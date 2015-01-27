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
//  File:   AffineGeneral.cc
//  Project:   XIL
//  Revision:  1.25
//  Last Mod:  10:10:51, 03/10/00
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
// COPYRIGHT
//------------------------------------------------------------------------
#pragma ident  "@(#)AffineGeneral.cc	1.25\t00/03/10  "

#include "XilDeviceManagerComputeBYTE.hh"
#include "ComputeInfo.hh"
#include "xili_geom_utils.hh"
#include "xili_interp_utils.hh"

//
//  Forward declarations
//
static XilStatus xili_affine_pixel_sequential_GL(AffineData affine_data);
static XilStatus xili_affine_general_storage_GL(AffineData affine_data);

XilStatus
XilDeviceManagerComputeBYTE::AffineGeneral(XilOp*       op,
					   unsigned     ,
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
    //  Get the basic data, assuming that the src image is a BYTE image.
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
XilDeviceManagerComputeBYTE::affineGeneral(XilBoxList* bl,
					   AffineData  ad)
{
    XilOp*    op  = ad.op;

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

        //
        //  Test to see if all of our storage is of type XIL_PIXEL_SEQUENTIAL.
        //  If so, optimize for pixel-sequential storage.
        //
        if((src_storage.isType(XIL_PIXEL_SEQUENTIAL)) &&
           (dst_storage.isType(XIL_PIXEL_SEQUENTIAL))) {
            if(xili_affine_pixel_sequential_GL(ad) == XIL_FAILURE) {
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
        } else {
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
    }

    return XIL_SUCCESS;
}

//
//  Performs affine transform of dst values for n banded image
//  assuming pixel sequential image.
//
static
XilStatus
xili_affine_pixel_sequential_GL(AffineData affine_data)
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
    
    Xil_unsigned8* src_data;
    unsigned int   src_pstride;
    unsigned int   src_sstride;
    src_storage->getStorageInfo(&src_pstride,
                                &src_sstride,
                                NULL, NULL,
                                (void**)&src_data);

    Xil_unsigned8* dst_data;
    unsigned int   dst_pstride;
    unsigned int   dst_sstride;
    dst_storage->getStorageInfo(&dst_pstride,
                                &dst_sstride,
                                NULL, NULL,
                                (void**)&dst_data);

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
    // Initialize scanline walking
    //
    XILI_SCANLINE_INIT_BOX_EXACT;
    Xil_unsigned8* src_data_min = src_data -
                                      (v_key*src_sstride + h_key*src_pstride);
    Xil_unsigned8* src_data_max = src_data_min +
                                      src_box_y2*src_sstride +
                                      src_box_x2*src_pstride;

    //
    //  Loop over convex regions in the destination.
    //
    XilConvexRegionList crl(roi, dst_box);

    unsigned int  num_pts;
    const double* dst_xarray;
    const double* dst_yarray;
    while(crl.getNext(&dst_xarray, &dst_yarray, &num_pts)) {    
        //
        //  Create scanline list
        //
        XilScanlineList dst_scanlines(dst_box,
                                      dst_xarray, dst_yarray, num_pts);

        //
        //  Loop over scanlines.
        //
        unsigned int y;    
        unsigned int dst_scan_start;
        unsigned int dst_scan_end;
        while(dst_scanlines.getNext(&y, &dst_scan_start, &dst_scan_end)) {
            //
            // Single-step through the pixels
            //
            // Get destination pixel address to map to
            //
            Xil_unsigned8* dst_pixel = dst_data +
                y              * dst_sstride +
                dst_scan_start * dst_pstride;

            //
            // Set scanline walking to the current line
            //
            Xil_unsigned8* src_pixel;
            XILI_SCANLINE_INIT_LINE_EXACT;
            src_pixel -= (v_key*src_sstride + h_key*src_pstride);

            if(nbands == 1) {
                //
                //  Single banded pixel sequential image
                //
                for(unsigned int i = dst_scan_start; i <= dst_scan_end; i++) {
#ifdef AFFINE_DEBUG                    
fprintf(stderr, "dst x=%d, y=%d maps to ", i, y);                   
fprintf(stderr, "src x=%d, y=%d\n", isx, isy);                    
#endif                  
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
                    Xil_unsigned8* src_pixel_clip = xili_srcbox_clip(src_pixel,
                                                        src_data_min,
                                                        src_data_max);

                    //
                    //  Do general interpolation...
                    //
                    XILI_INTERP_GENERAL_BYTE(src_pixel_clip,
                                             src_sstride,
                                             src_pstride,
                                             v_ptr, h_ptr,
                                             v_size, h_size,
                                             dst_pixel);

                    dst_pixel += dst_pstride;
                    //
                    // Update source image coordinates for the next pixel
                    //
                    XILI_SCANLINE_SRC_INCREMENT_EXACT;
                }
            } else {
                //
                // Multi-banded pixel sequential image
                //
                for(unsigned int i = dst_scan_start; i <= dst_scan_end; i++) {
#ifdef AFFINE_DEBUG
fprintf(stderr, "nbands= %d:  dst x=%d, y=%d maps to ", nbands, i, y);                   
fprintf(stderr, "src x=%d, y=%d, v_key= %d; h_key= %d\n",
        isx, isy, v_key, h_key);
#endif
                    //
                    //  Compute which (seperable) interpolation kernel to use
                    //  from our table.
                    //
                    const float* h_ptr =
                        h_data + ((int)(fracx * h_samples)) * h_size;
                    const float* v_ptr =
                        v_data + ((int)(fracy * v_samples)) * v_size;

                    //
                    //  Copy pixel values to destination
                    //
                    unsigned int   band_count = nbands;
                    Xil_unsigned8* dst_band = dst_pixel;
                    Xil_unsigned8* src_band = xili_srcbox_clip(src_pixel,
                                                  src_data_min,
                                                  src_data_max);

                    do {
                        //
                        //  Do general interpolation...
                        //
                        XILI_INTERP_GENERAL_BYTE(src_band,
                                                 src_sstride,
                                                 src_pstride,
                                                 v_ptr, h_ptr,
                                                 v_size, h_size,
                                                 dst_band);
                        dst_band++;
                        src_band++;
                    } while(--band_count);

                    dst_pixel += dst_pstride;
                    //
                    // Update source image coordinates for the next pixel
                    //
                    XILI_SCANLINE_SRC_INCREMENT_EXACT;
                }
            }            
        } 
    }
    
    return XIL_SUCCESS;
}

static XilStatus
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
            Xil_unsigned8* src_data    =
                (Xil_unsigned8*)src_storage->getDataPtr(b);
            unsigned int   src_pstride = src_storage->getPixelStride(b);
            unsigned int   src_sstride = src_storage->getScanlineStride(b);

            Xil_unsigned8* dst_data    =
                (Xil_unsigned8*)dst_storage->getDataPtr(b);
            unsigned int   dst_pstride = dst_storage->getPixelStride(b);
            unsigned int   dst_sstride = dst_storage->getScanlineStride(b);

            //
            // Initialize scanline walking
            //
            XILI_SCANLINE_INIT_BOX_EXACT;
            Xil_unsigned8* src_data_min = src_data -
                                              (v_key*src_sstride +
                                               h_key*src_pstride);
            Xil_unsigned8* src_data_max = src_data_min +
                                              src_box_y2*src_sstride +
                                              src_box_x2*src_pstride;

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
                //  Single-step through the pixels
                //
                //  Get destination pixel address to map to
                //
                Xil_unsigned8* dst_pixel = dst_data +
                    y              * dst_sstride +
                    dst_scan_start * dst_pstride;

                //
                // Set scanline walking to the current line
                //
                Xil_unsigned8* src_pixel;
                XILI_SCANLINE_INIT_LINE_EXACT;
                src_pixel -= (v_key*src_sstride + h_key*src_pstride);

                for(unsigned int i = dst_scan_start; i <= dst_scan_end; i++) {
#ifdef AFFINE_DEBUG                    
fprintf(stderr, "dst x=%d, y=%d maps to ", i, y);                   
fprintf(stderr, "src x=%d, y=%d\n", isx, isy);                    
#endif                  
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
                    Xil_unsigned8* src_pixel_clip = xili_srcbox_clip(src_pixel,
                                                        src_data_min,
                                                        src_data_max);

                    //
                    //  Do general interpolation...
                    //
                    XILI_INTERP_GENERAL_BYTE(src_pixel_clip,
                                             src_sstride,
                                             src_pstride,
                                             v_ptr, h_ptr,
                                             v_size, h_size,
                                             dst_pixel);

                    dst_pixel += dst_pstride;
                    //
                    // Update source image coordinates for the next pixel
                    //
                    XILI_SCANLINE_SRC_INCREMENT_EXACT;
                }
            }
        } 
    }
    
    return XIL_SUCCESS;
}
