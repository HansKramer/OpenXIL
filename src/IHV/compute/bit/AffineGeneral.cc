/***********************************************************************


            EXHIBIT A - XIL 1.4.1 (OPEN SOURCE VERSION) License


The contents of this file are subject to the XIL 1.4.1 (Open Source
Version) License Agreement Version 1.0 (the "License").  You may not
use this file except in compliance with the License.  You may obtain a
copy of the License at:

    http://www.sun.com/software/imaging/XIL/xilsrc.html

Software distributed under the License is distributed on an "AS IS"
basis, WITHOUT WARRANTY OF ANY KIND, either express or implied.  See
the License for the specific language governing rights and limitations
under the License.

The Original Code is XIL 1.4.1 (Open Source Version).
The Initial Developer of the Original Code is: Sun Microsystems, Inc..
Portions created by:_______________________________________________
are Copyright(C):__________________________________________________
All Rights Reserved.
Contributor(s):____________________________________________________


***********************************************************************/
//------------------------------------------------------------------------
//
//  File:	AffineGeneral.cc
//  Project:	XIL
//  Revision:	1.3
//  Last Mod:	16:54:58, 05/07/96
//
//  Description:
//	
//	
//	
//	
//	
//	
//	
//	
//  MT-level:  SAFE
//	
//------------------------------------------------------------------------
//	COPYRIGHT
//------------------------------------------------------------------------
#pragma ident	"@(#)AffineGeneral.cc	1.3\t96/05/07  "

#include "XilDeviceManagerComputeBIT.hh"
#include "ComputeInfo.hh"
#include "XiliUtils.hh"
#include "xili_geom_utils.hh"
#include "xili_interp_utils.hh"

//
//  Forward declarations
//
static XilStatus xili_affine_general_storage_GL(AffineData affine_data);

XilStatus
XilDeviceManagerComputeBIT::AffineGeneral(XilOp*       op,
                                          unsigned        ,
                                          XilRoi*      roi,
                                          XilBoxList*  bl)
{
    XilStatus status = XIL_SUCCESS;
    AffineData ad;

    ad.op  = op;
    ad.roi = roi;
    
    //
    //  Get the transformation matrix. Translational part of
    //  the matrix contains destination origin. src_origin
    //  coordinates are passed as the 2nd and 3rd parameter.
    //
    op->getParam(1, (void**)&(ad.matrix));

    op->getParam(4, (XilObject**)&(ad.htable));
    
    op->getParam(5, (XilObject**)&(ad.vtable));

    //
    //  Get the basic data, assuming that the src image is a BIT image.
    //  roi is the complete image ROI. This means that src and dst ROI's are
    //  already taken into account, and the roi passed is the intersection
    //  of these. 
    //
    if(op->splitOnTileBoundaries(bl) == XIL_FAILURE) {
        return XIL_FAILURE;
    }

    status = affineGeneral(bl, ad);

    return status;
}

XilStatus
XilDeviceManagerComputeBIT::affineGeneral(XilBoxList* bl,
                                          AffineData  ad)
{
    XilOp*     op = ad.op;

    //
    //  Get the images for our operation.
    //
    XilImage* src = op->getSrcImage(1);
    XilImage* dst = op->getDstImage(1);

    ad.nbands = dst->getNumBands();

    //
    // Each dst box is covered with several convex regions. Each of
    // the convex regions is bound to lie in only one src box.
    //
    XilBox*   src_box;
    XilBox*   dst_box;
    while(bl->getNext(&src_box, &dst_box)) {
        //
        //  Aquire our storage from the images.  The storage returned is valid
        //  for the box given.  Thus, any origins or child offsets have been
        //  taken into account. Currently we only think in terms of "inner"
        //  tiles and this code works only for the case of entire image.
        //
        XilStorage  src_storage(src);
        XilStorage  dst_storage(dst);
        if((src->getStorage(&src_storage, op, src_box, "XilMemory",
                XIL_READ_ONLY)  == XIL_FAILURE) ||
           (dst->getStorage(&dst_storage, op, dst_box, "XilMemory",
                XIL_WRITE_ONLY) == XIL_FAILURE)) {
            //
            //  Mark this box as failed and if that succeeds, continue
            //  processing the next box.  Otherwise, return XIL_FAILURE now.
            //
            if(bl->markAsFailed() == XIL_FAILURE) {
                return XIL_FAILURE;
            } else {
                continue;
            }
        }

        ad.src_storage = &src_storage;
        ad.dst_storage = &dst_storage;
        ad.src_box     = src_box;
        ad.dst_box     = dst_box;

        if(xili_affine_general_storage_GL(ad) == XIL_FAILURE) {
            //
            //  Mark this box as failed and if that succeeds, continue 
            //  processing the next box.  Otherwise, return XIL_FAILURE
            //  now. 
            //
            if(bl->markAsFailed() == XIL_FAILURE) {
                return XIL_FAILURE;
            } else {
                continue;
            }
        }
    }

    return XIL_SUCCESS;
}

static
XilStatus
xili_affine_general_storage_GL(AffineData affine_data)
{
    XilOp*                 op          = affine_data.op;
    XilStorage*            src_storage = affine_data.src_storage;
    XilStorage*            dst_storage = affine_data.dst_storage;
    XilBox*                src_box     = affine_data.src_box;
    XilBox*                dst_box     = affine_data.dst_box;
    XilInterpolationTable* htable      = affine_data.htable;
    XilInterpolationTable* vtable      = affine_data.vtable;
    XilRoi*                roi         = affine_data.roi;
    unsigned int           nbands      = affine_data.nbands;

//#define AFFINE_DEBUG 1
    //
    //  Interpolation table information.
    //
    unsigned int h_samples = htable->getNumSubsamples();
    unsigned int v_samples = vtable->getNumSubsamples();
    unsigned int h_size    = htable->getKernelSize();
    unsigned int v_size    = vtable->getKernelSize();
    const float* h_data    = htable->getData();
    const float* v_data    = vtable->getData();
    unsigned int h_key     = (h_size - 1)/2;
    unsigned int v_key     = (v_size - 1)/2;

    //
    //  Compute the right and bottom edges of our box -- in box space.  Box
    //  space always starts at (0, 0) within a box.  So, we want the
    //  (width - 1) and (height - 1) which are our corners.
    //
    int          src_box_x;
    int          src_box_y;
    unsigned int src_box_w;
    unsigned int src_box_h;

    src_box->getAsRect(&src_box_x, &src_box_y, &src_box_w, &src_box_h);

    int          src_box_x2 = src_box_w - 1;
    int          src_box_y2 = src_box_h - 1;

    //
    //  Loop over convex regions in the destination.
    //
    XilConvexRegionList crl(roi, dst_box);

    unsigned int  num_pts;
    const double* dst_xarray;
    const double* dst_yarray;
    while(crl.getNext(&dst_xarray, &dst_yarray, &num_pts)) {    
        for(unsigned int b = 0; b < nbands; b++) {
            //
            //  Get the pointers for this band in the source and
            //  destination.
            //
            Xil_unsigned8* src_data =
                (Xil_unsigned8*)src_storage->getDataPtr(b);
            unsigned int   src_sstride  = src_storage->getScanlineStride(b);
            unsigned int   src_offset   = src_storage->getOffset(b);

            Xil_unsigned8* dst_data =
                (Xil_unsigned8*)dst_storage->getDataPtr(b);
            unsigned int   dst_sstride  = dst_storage->getScanlineStride(b);
            unsigned int   dst_offset   = dst_storage->getOffset(b);
            
#ifdef AFFINE_DEBUG
fprintf(stderr, "src_data = 0x%X, src_sstride = %d, src_offset = %d\n",
        src_data, src_sstride, src_offset);
fprintf(stderr, "dst_data = 0x%X, dst_sstride = %d, dst_offset = %d\n",
        dst_data, dst_sstride, dst_offset);
#endif
            
            //
            //  Adjust src_data and src_offset so we compute from beginning of
            //  storage instead of beginning of src box.  We need to do this so
            //  we're not trying to subtract bits from the src pointer in the
            //  inner loops.
            //
            int new_offset = (int)src_offset - (int)h_key;

#ifdef AFFINE_DEBUG
fprintf(stderr, "h_key = %d, v_key = %d, new_offset = %d\n",
        h_key, v_key, new_offset);
#endif
                        
            //
            // Adjusted source data
            //
            src_data = src_data - v_key * src_sstride;

            if(new_offset < 0) {
                //
                // Have to readjust src_data
                //
                src_data += ((new_offset / XIL_BIT_ALIGNMENT) - 1);
                
                //
                //  For negative offsets, we need to do more work to get the
                //  correct effect...
                //
                //    -1 --> 7, -2 --> 6, etc.
                //
                new_offset %= XIL_BIT_ALIGNMENT;                
                src_offset = 8 + new_offset;
            } else {
                src_offset = new_offset;
            }

#ifdef AFFINE_DEBUG
fprintf(stderr, "New src_data = 0x%X, src_offset = %d\n",
        src_data, src_offset);
#endif
 
            //
            // Initialize scanline walking
            //
            XILI_SCANLINE_INIT_BOX_BIT;
            Xil_unsigned8* src_line_max = src_data + src_box_y2*src_sstride;
            unsigned int   src_bit_max = src_offset + src_box_x2;

            //
            //  Create scanline list
            //
            XilScanlineList dst_scanlines(dst_box,
                                          dst_xarray, dst_yarray, num_pts);

            //
            //  Loop over scanlines.
            //
            unsigned int y;    
            unsigned int dst_scan_start, dst_scan_end;
            while(dst_scanlines.getNext(&y, &dst_scan_start, &dst_scan_end)) {
                //
                //  Get destination pixel address to map to
                //
                Xil_unsigned8* dst_scanline = dst_data +
                    y * dst_sstride;

                //
                // Set scanline walking to the current line
                //
                XILI_SCANLINE_INIT_LINE_BIT;

                for(unsigned int x = dst_scan_start; x <= dst_scan_end; x++) {
                    //
                    //  Compute which (seperable) interpolation kernel to use
                    //  from our table.
                    //
                    const float* h_ptr =
                        h_data + ((int)(fracx * h_samples)) * h_size;
                    const float* v_ptr =
                        v_data + ((int)(fracy * v_samples)) * v_size;

                    //
                    //  Clip the pixel to be within the source data buffer
                    //
                    Xil_unsigned8* src_scanline_clip = xili_srcbox_clip(
                                                        src_scanline,
                                                        src_data,
                                                        src_line_max);
                    int src_bit_clip = xili_srcbox_clip(src_bit,
                                                        (int)src_offset,
                                                        (int)src_bit_max);

		    //
		    //  Do general interpolation
		    //
                    XILI_INTERP_GENERAL_BIT(src_scanline_clip,
                                            src_sstride,
                                            src_bit_clip,
                                            v_ptr, h_ptr,
                                            v_size, h_size,
                                            dst_scanline, dst_offset + x);

                    //
                    // Update the source position
                    //
                    XILI_SCANLINE_SRC_INCREMENT_BIT;
                }
            }
        } 
    }
    
    return XIL_SUCCESS;
}
