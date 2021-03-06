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
//  File:	AffineBilinear.cc
//  Project:	XIL
//  Revision:	1.21
//  Last Mod:	10:13:02, 03/10/00
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
#pragma ident	"@(#)AffineBilinear.cc	1.21\t00/03/10  "

#include "XilDeviceManagerComputeFLOAT.hh"
#include "XiliUtils.hh"
#include "xili_geom_utils.hh"
#include "xili_interp_utils.hh"

static XilStatus xili_affine_pixel_sequential_BL(AffineData affine_data);
static XilStatus xili_affine_general_storage_BL(AffineData affine_data);

XilStatus
XilDeviceManagerComputeFLOAT::AffineBilinear(XilOp*       op,
                                             unsigned       ,
                                             XilRoi*      roi,
                                             XilBoxList*  bl)
{
    XilStatus status;
    AffineData ad;
    ad.roi = roi;
    ad.op  = op;

    //
    //  Get the transformation matrix.
    //
    op->getParam(1, (void**)&(ad.matrix));

    //
    // Get the basic data, assuming that the src image is a FLOAT image.
    // roi is the complete image ROI. This means that src and dst ROI's are
    // already taken into account, and the roi passed is the intersection
    // of these. 
    //
    if(op->splitOnTileBoundaries(bl) == XIL_FAILURE) {
        return XIL_FAILURE;
    }

    status = affineBilinear(bl, ad);

    return status;    
}

    
XilStatus
XilDeviceManagerComputeFLOAT::affineBilinear(XilBoxList* bl,
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
            if(xili_affine_pixel_sequential_BL(ad) == XIL_FAILURE) {
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
            if(xili_affine_general_storage_BL(ad) == XIL_FAILURE) {
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
xili_affine_pixel_sequential_BL(AffineData affine_data)
{
    XilOp*       op            = affine_data.op;
    XilStorage*  src_storage   = affine_data.src_storage;
    XilStorage*  dst_storage   = affine_data.dst_storage;
    XilBox*      src_box       = affine_data.src_box;
    XilBox*      dst_box       = affine_data.dst_box;
    XilRoi*      roi           = affine_data.roi;
    unsigned int nbands        = affine_data.nbands;
    
    Xil_float32*   src_data;
    unsigned int   src_pstride;
    unsigned int   src_sstride;
    src_storage->getStorageInfo(&src_pstride,
                                &src_sstride,
                                NULL, NULL,
                                (void**)&src_data);
            
    Xil_float32*   dst_data;
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
    Xil_float32* src_data_max = src_data +
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
            Xil_float32*   dst_pixel = dst_data +
                y              * dst_sstride +
                dst_scan_start * dst_pstride;

            //
            // Set scanline walking to the current line
            //
            Xil_float32* src_pixel;
            XILI_SCANLINE_INIT_LINE_EXACT;

            if(nbands == 1) {
                //
                //  Single banded pixel sequential image
                //
                for(unsigned int i = dst_scan_start; i <= dst_scan_end; i++) {
#ifdef AFFINE_DEBUG                    
fprintf (stderr, "dst x=%d, y=%d maps to ", i, y);                   
fprintf (stderr, "src x=%d, y=%d\n", isx, isy);                    
#endif                  
                    //
                    //  Clip the pixel to be within the source data buffer
                    //
                    Xil_float32* src_pixel_clip = xili_srcbox_clip(src_pixel,
                                                      src_data,
                                                      src_data_max);

                    *dst_pixel = xili_interp_bilinear(src_pixel_clip,
                                                      src_pstride,
                                                      src_sstride,
                                                      fracx,
                                                      fracy);

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
fprintf (stderr, "dst x=%d, y=%d maps to ", i, y);                   
fprintf (stderr, "src x=%d, y=%d\n", isx, isy);                    
#endif                  
                    //
                    //  Copy pixel values to destination
                    //
                    unsigned int band_count = nbands;
                    Xil_float32*   dst_band = dst_pixel;
                    Xil_float32*   src_band = xili_srcbox_clip(src_pixel,
                                                               src_data,
                                                               src_data_max);

                    do {
                        *dst_band++ = xili_interp_bilinear(src_band++,
                                                           src_pstride,
                                                           src_sstride,
                                                           fracx,
                                                           fracy);
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


static
XilStatus
xili_affine_general_storage_BL(AffineData affine_data)
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
            Xil_float32*   src_data    =
                (Xil_float32*)src_storage->getDataPtr(b);
            unsigned int   src_pstride = src_storage->getPixelStride(b);
            unsigned int   src_sstride = src_storage->getScanlineStride(b);

            Xil_float32*   dst_data    =
                (Xil_float32*)dst_storage->getDataPtr(b);
            unsigned int   dst_pstride = dst_storage->getPixelStride(b);
            unsigned int   dst_sstride = dst_storage->getScanlineStride(b);

            //
            //  Create scanline list
            //
            XilScanlineList dst_scanlines(dst_box,
                                          dst_xarray, dst_yarray, num_pts);

            //
            // Initialize scanline walking
            //
            XILI_SCANLINE_INIT_BOX_EXACT;
            Xil_float32* src_data_max = src_data +
                                            src_box_y2*src_sstride +
                                            src_box_x2*src_pstride;

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
                Xil_float32*   dst_pixel = dst_data +
                    y              * dst_sstride +
                    dst_scan_start * dst_pstride;

                //
                // Set scanline walking to the current line
                //
                Xil_float32* src_pixel;
                XILI_SCANLINE_INIT_LINE_EXACT;

                for(unsigned int i = dst_scan_start; i <= dst_scan_end; i++) {
#ifdef AFFINE_DEBUG                    
fprintf(stderr, "dst x=%d, y=%d maps to ", i, y);                   
fprintf(stderr, "src x=%d, y=%d\n", isx, isy);                    
#endif                  
                    //
                    //  Clip the pixel to be within the source data buffer
                    //
                    Xil_float32* src_pixel_clip = xili_srcbox_clip(src_pixel,
                                                      src_data,
                                                      src_data_max);

                    *dst_pixel = xili_interp_bilinear(src_pixel_clip,
                                                      src_pstride,
                                                      src_sstride,
                                                      fracx,
                                                      fracy);

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
