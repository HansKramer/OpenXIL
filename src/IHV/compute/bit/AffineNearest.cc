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
//  File:	AffineNearest.cc
//  Project:	XIL
//  Revision:	1.8
//  Last Mod:	15:21:42, 05/07/96
//
//  Description:
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
#pragma ident	"@(#)AffineNearest.cc	1.8\t96/05/07  "

#include "XilDeviceManagerComputeBIT.hh"
#include "ComputeInfo.hh"
#include "XiliUtils.hh"
#include "xili_geom_utils.hh"

//
//  Forward declarations
//
static XilStatus xili_affine_general_storage_NN(AffineData affine_data);

XilStatus
XilDeviceManagerComputeBIT::AffineNearest(XilOp*       op,
                                          unsigned        ,
                                          XilRoi*      roi,
                                          XilBoxList*  bl)
{
    XilStatus status = XIL_SUCCESS;
    AffineData ad;

    ad.op  = op;
    ad.roi = roi;

    //
    //  Get the transformation matrix.
    //
    op->getParam(1, (void**)&(ad.matrix));
    
    //
    //  Get the basic data, assuming that the src image is a BIT image.
    //  roi is the complete image ROI. This means that src and dst ROI's are
    //  already taken into account, and the roi passed is the intersection
    //  of these. 
    //
    if(op->splitOnTileBoundaries(bl) == XIL_FAILURE) {
        return XIL_FAILURE;
    }

    status = affineNearest(bl, ad);

    return status;
}

XilStatus
XilDeviceManagerComputeBIT::affineNearest(XilBoxList* bl,
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

        if(xili_affine_general_storage_NN(ad) == XIL_FAILURE) {
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
xili_affine_general_storage_NN(AffineData affine_data)
{
    XilOp*       op            = affine_data.op;
    XilStorage*  src_storage   = affine_data.src_storage;
    XilStorage*  dst_storage   = affine_data.dst_storage;
    XilBox*      src_box       = affine_data.src_box;
    XilBox*      dst_box       = affine_data.dst_box;
    XilRoi*      roi           = affine_data.roi;
    unsigned int nbands        = affine_data.nbands;
    
    //
    //  Compute the right and bottom edges of our box -- in box space.  Box
    //  space always starts at (0, 0) within a box.  So, we want the
    //  (width - 1) and (height - 1) which are our corners.
    //
    int src_box_x2;
    int src_box_y2;
    {
        int          src_box_x;
        int          src_box_y;
        unsigned int src_box_w;
        unsigned int src_box_h;

        src_box->getAsRect(&src_box_x, &src_box_y, &src_box_w, &src_box_h);

        src_box_x2 = src_box_w - 1;
        src_box_y2 = src_box_h - 1;
    }

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

            //
            // Initialize scanline walking
            //
            XILI_SCANLINE_INIT_BOX_BIT;
            Xil_unsigned8* src_line_max = src_data + src_box_y2*src_sstride;
            unsigned int   src_bit_max  = src_offset + src_box_x2;

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
                    //  Get source pixel address to map from.
                    //
                    Xil_unsigned8* src_pixel = src_scanline;
                    unsigned int   pixel_offset = src_bit;
                
                    //
                    //  Clip the pixel to be within the source data buffer
                    //
                    src_pixel = xili_srcbox_clip(src_pixel,
                                                 src_data,
                                                 src_line_max);
                    pixel_offset = xili_srcbox_clip(pixel_offset,
                                                    src_offset,
                                                    src_bit_max);

                    if(XIL_BMAP_TST(src_pixel, pixel_offset)) {
                        XIL_BMAP_SET(dst_scanline, dst_offset + x);
                    } else {
                        XIL_BMAP_CLR(dst_scanline, dst_offset + x);
                    }

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
