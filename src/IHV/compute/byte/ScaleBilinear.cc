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
//This line lets emacs recognize this as -*- C++ -*- Code
//------------------------------------------------------------------------
//
//  File:	ScaleBilinear.cc
//  Project:	XIL
//  Revision:	1.14
//  Last Mod:	10:10:43, 03/10/00
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
//  MT-level:  <??????>
//	
//------------------------------------------------------------------------
//	COPYRIGHT
//------------------------------------------------------------------------
#pragma ident	"@(#)ScaleBilinear.cc	1.14\t00/03/10  "

#include "XilDeviceManagerComputeBYTE.hh"
#include "ComputeInfo.hh"
#include "xili_geom_utils.hh"

#define SUBSAMPLE2X_ENABLED 1

static XilStatus scaleZoom2xBilinear(XilBoxList* bl, AffineData ad);
static XilStatus scale_zoom2x_pixel_sequential_BL(AffineData affine_data);
static XilStatus scale_zoom2x_general_storage_BL(AffineData affine_data);

#ifdef SUBSAMPLE2X_ENABLED
static XilStatus scaleSubsample2xBilinear(XilBoxList* bl, AffineData ad);
static XilStatus scale_subsample2x_pixel_sequential_BL(AffineData affine_data);
static XilStatus scale_subsample2x_general_storage_BL(AffineData affine_data);
#endif

XilStatus
XilDeviceManagerComputeBYTE::ScaleBilinear(XilOp*       op,
					   unsigned     ,
					   XilRoi*      roi,
					   XilBoxList*  bl)

{
    XilStatus status;
    AffineData ad;

    ad.op  = op;
    ad.roi = roi;
    //
    //  Get the transformation matrix. Translational part of
    //  the matrix contains destination origin, currently
    //  this is not supplied by the core.
    //
    float	xscale, yscale;
    
    op->getParam(1, &xscale);
    op->getParam(2, &yscale);

    ad.roi = roi;

    //
    // Get the basic data, assuming that the src image is a BYTE image.
    // roi is the complete image ROI. This means that src and dst ROI's are
    // already taken into account, and the roi passed is the intersection
    // of these. 
    //
    if(op->splitOnTileBoundaries(bl) == XIL_FAILURE) {
        return XIL_FAILURE;
    }

    //
    // construct affine matrix
    //
    float matrix[6];
    xili_afftr_to_array(xili_scale(xscale, yscale), matrix);
    ad.matrix = matrix;

    if(xili_scale_is_zoom2x(xscale, yscale)) {
        //
        // 2x replicate zoom
        //
        status = scaleZoom2xBilinear(bl, ad);
#ifdef SUBSAMPLE2X_ENABLED
    //
    // TODO  bpb  02/25/1997  0.5x scale optimization from XIL 1.2.
    // Due to the change in pixel origin definition this optimization is
    // incompatible with the equivalent affine transform in XIL 1.3.
    //
    } else if(xili_scale_is_subsample2x(xscale, yscale)) {
        //
        // subsample by 2
        //
        status = scaleSubsample2xBilinear(bl, ad);
#endif
    } else {
        //
        // If nothing else works, call affine transform
        //
        status = affineBilinear(bl, ad);
    }

    return status;  
}

static
XilStatus
scaleZoom2xBilinear(XilBoxList* bl,
                    AffineData ad)
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

        //
        //  Test to see if all of our storage is of type XIL_PIXEL_SEQUENTIAL.
        //  If so, optimize for pixel-sequential storage.
        //
        if((src_storage.isType(XIL_PIXEL_SEQUENTIAL)) &&
           (dst_storage.isType(XIL_PIXEL_SEQUENTIAL))) {
            if(scale_zoom2x_pixel_sequential_BL(ad) == XIL_FAILURE) {
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
            if(scale_zoom2x_general_storage_BL(ad) == XIL_FAILURE) {
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

#ifdef SUBSAMPLE2X_ENABLED
static
XilStatus
scaleSubsample2xBilinear(XilBoxList* bl,
                         AffineData ad)
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

        //
        //  Test to see if all of our storage is of type XIL_PIXEL_SEQUENTIAL.
        //  If so, optimize for pixel-sequential storage.
        //
        if((src_storage.isType(XIL_PIXEL_SEQUENTIAL)) &&
           (dst_storage.isType(XIL_PIXEL_SEQUENTIAL))) {
            if(scale_subsample2x_pixel_sequential_BL(ad) == XIL_FAILURE) {
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
            if(scale_subsample2x_general_storage_BL(ad) == XIL_FAILURE) {
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
#endif

static XilStatus scale_zoom2x_pixel_sequential_BL(AffineData affine_data)
{
    XilOp*       op            = affine_data.op;
    XilStorage*  src_storage   = affine_data.src_storage;
    XilStorage*  dst_storage   = affine_data.dst_storage;
    XilBox*      src_box       = affine_data.src_box;
    XilBox*      dst_box       = affine_data.dst_box;
    XilRoi*      roi           = affine_data.roi;
    unsigned int nbands        = affine_data.nbands;

    //
    // Get the storage information
    //
    Xil_unsigned8* src_base_addr;
    unsigned int   src_nx_pixel;
    unsigned int   src_nx_scan;
    src_storage->getStorageInfo(&src_nx_pixel,
                                &src_nx_scan,
                                NULL, NULL,
                                (void**)&src_base_addr);

    Xil_unsigned8* dst_base_addr;
    unsigned int   dst_nx_pixel;
    unsigned int   dst_nx_scan;
    dst_storage->getStorageInfo(&dst_nx_pixel,
                                &dst_nx_scan,
                                NULL, NULL,
                                (void**)&dst_base_addr);

    //
    // Get the image coordinates of the source box for later use.
    //
    int src_box_x0_image;
    int src_box_y0_image;
    {
        unsigned int src_box_size_x;
        unsigned int src_box_size_y;
        src_box->getAsRect(&src_box_x0_image,
                           &src_box_y0_image,
                           &src_box_size_x,
                           &src_box_size_y);
    }

    //
    // Retrieve the source image width and height for later use.
    //
    int src_xsize;
    int src_ysize;
    {
        XilImage* src = src_storage->getImage();
        src_xsize = src->getWidth();
        src_ysize = src->getHeight();
    }

    //
    // Create a rectangle list that is an intersection of
    // the intersected roi and dst_box. The list is
    // returned in the dst_box coordinate space (not
    // in the dst image coordinate space).
    //
    XilRectList rl(roi, dst_box);

    //
    //  Well, special case city here.  Feel free to add to the list as
    //    you want more performance...
    //
    //  1)  1 BAND -> 1 BAND (no band children)
    //  2)  3 BAND -> 3 BAND (no band children)
    //  3   N BAND -> N BAND (everything)
    //
    
    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    //
    //      1 CONTIGUOUS BAND OPTIMIZED CASE
    //
    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    if(nbands == 1 && src_nx_pixel == 1 && dst_nx_pixel == 1) {
        //
        //  Loop over the rectangle list
        //
        int dstR_x;
        int dstR_y;
        unsigned int dstR_xsize;
        unsigned int dstR_ysize;
        while(rl.getNext(&dstR_x, &dstR_y, &dstR_xsize, &dstR_ysize)) {
            //
            //  The starting point in the destination.
            //
            Xil_unsigned8* dst_scanline = dst_base_addr +
                (dstR_y * dst_nx_scan) +
                (dstR_x * dst_nx_pixel);
            Xil_unsigned8* dst_pixel = dst_scanline;

            //
            // Backward map the upper left destination rectangle corner.
            // This mapping includes the effect of src and dst origins.
            //
            double sx;
            double sy;
            op->backwardMap(dst_box, (double)dstR_x, (double)dstR_y,
                            src_box, &sx, &sy);

            //
            //  Since we do the first line seperately and then do a
            //   two-line algorithm there is either 1 or 2 lines left
            //   depending upon whether we're odd or even.
            //
            //  This variable is set to 1 if it's odd which means we
            //   add an extra 1 to our loop to complete the last lines.
            //  
            unsigned int  odd_start_line =
                (_XILI_ROUND(sy) == _XILI_ROUND(sy + 0.5F)) ? 1 : 0;
            unsigned int  odd_start_pixel =
                (_XILI_ROUND(sx) == _XILI_ROUND(sx + 0.5F)) ? 1 : 0;
            int odd_lines = dstR_ysize % 2;
            int end_pixels     = dstR_xsize % 4;
            Xil_unsigned8* line_buffer  = NULL;
        
            int dstR_ysize_2 = dstR_ysize/2 + odd_lines;
            int dstR_xsize_4 = dstR_xsize/4;

            //
            //  Check for edge conditions (in image coordinate space)
            //
            {
                //
                // Backward map the lower right corner of the dst rectangle.
                // The results are in src_box coordinate space.
                //
                double src_x;
                double src_y;
                op->backwardMap(dst_box,
                                (double)(dstR_x + (int)dstR_xsize - 1),
                                (double)(dstR_y + (int)dstR_ysize - 1),
                                src_box,
                                &src_x,
                                &src_y);

                //
                // Check the edge conditions in src image space.
                //
                if((src_x + src_box_x0_image) > (double)(src_xsize - 1)) {
                    dstR_xsize_4--;
                    end_pixels = 4;
                }

                if((src_y + src_box_y0_image) > (double)(src_ysize - 1)) {
                    dstR_ysize -= 1;
                    odd_lines = 1; // this forces it not to touch bottom line
                }
            }

            //
            //  The starting point in the source.
            //
	    if (sx < 0 && odd_start_pixel)
		sx -= 1.0;
	    if (sy < 0 && odd_start_line)
		sy -= 1.0;
            Xil_unsigned8* src_scanline = src_base_addr + 
                ((int)sy * src_nx_scan) +
                ((int)sx * src_nx_pixel);
            Xil_unsigned8* src_pixel = src_scanline;

            //
            //  If we're starting on an odd line, then we must
            //   allocate our own buffer.
            //
            if(odd_start_line) {
                odd_lines = !odd_lines;
                dstR_ysize_2 += odd_lines;
                line_buffer    = new Xil_unsigned8[dstR_xsize];
                dst_pixel      = line_buffer;
            }
            
            if(odd_start_pixel) {
                unsigned int interp0 =
                    (unsigned int)(*src_pixel + *(src_pixel+1))>>1;

                *dst_pixel++ = interp0;
                src_pixel++;
                
                if(end_pixels>0) {
                    end_pixels--;
                } else {
                    end_pixels = 3;
                    dstR_xsize_4--;
                }
            }

            //
            //  Special case the first scanline.  I compute this and then
            //  do a more general two-line algorithm for the rest of the image.
            //
            int i;
            for(i=0; i<dstR_xsize_4; i++) {
                //
                //  Calculate the interpolation points and set
                //   the corresponding four pixels in the destination.
                //
                int  interp0 =
                    (int)(*src_pixel + *(src_pixel+1))>>1;
                int  interp1 =
                    (int)(*(src_pixel+1) + *(src_pixel+2))>>1;
                
                *dst_pixel      = *src_pixel;
                *(dst_pixel+1)  = interp0;
                *(dst_pixel+2)  = *(src_pixel+1);
                *(dst_pixel+3)  = interp1;
                
                dst_pixel += 4;
                src_pixel += 2;
            }
            
            switch(end_pixels) {
                case 4:
                case 3:
                {
                    //
                    //  Calculate the interpolation points and set
                    //   the corresponding three pixels in the destination.
                    //
                    int interp0 =
                        (int)(*src_pixel + *(src_pixel+1))>>1;
                    
                    *dst_pixel      = *src_pixel;
                    *(dst_pixel+1)  = interp0;
                    *(dst_pixel+2)  = *(src_pixel+1);
                }
                break;
                
                case 2:
                {
                    //
                    //  Calculate the interpolation points and set
                    //   the corresponding two pixels in the destination.
                    //
                    int  interp0 =
                        (int)(*src_pixel + *(src_pixel+1))>>1;
                    
                    *dst_pixel      = *src_pixel;
                    *(dst_pixel+1)  = interp0;
                }                    
                break;
                
                case 1:
                *dst_pixel      = *src_pixel;
                break;
            }
            
            //
            //  Initialize the variables which we'll use to move through
            //   the image.
            //
            //  The destination is incremented by two since we're doing
            //   two lines of the destination for every line of the
            //   source.  This saves an add per scanline.
            //
            unsigned int   dst_nx_scan2 = dst_nx_scan + dst_nx_scan;
            
            //
            //  Here I set a pointer to the scanline we just computed.
            //
            Xil_unsigned8* dst_p_scanline = dst_scanline;
            Xil_unsigned8* dst_p_pixel    = dst_scanline;

            //
            //  Only increment each by one scanline this time since we've
            //   only completed a single scanline in the destination.
            //
            if(odd_start_line) {
                dst_p_pixel = dst_p_scanline = line_buffer;
                dst_pixel = dst_scanline;
            } else {
                dst_pixel = dst_scanline += dst_nx_scan;
            }
            src_pixel = src_scanline += src_nx_scan;

            Xil_unsigned8* dst_n_scanline = dst_scanline + dst_nx_scan;
            Xil_unsigned8* dst_n_pixel    = dst_n_scanline;


            for(i=1; i<dstR_ysize_2; i++) {
                if(odd_start_pixel) {
                    unsigned int  interp0 =
                        (unsigned int)(*src_pixel + *(src_pixel+1))>>1;
                    unsigned int  interp1 =
                        (unsigned int)(*dst_p_pixel + interp0)>>1;
                    
                    *dst_pixel++   = interp1;
                    *dst_n_pixel++ = interp0;
                    
                    dst_p_pixel++;
                    src_pixel++;
                }
                
                for(int j=0; j<dstR_xsize_4; j++) {
                    //
                    //  Below is the layout of the interpolated values.
                    //   - The *'s refer to the previous scanline in the
                    //       destination.
                    //   - The numbers correspond to the different
                    //       interpolated variables. 
                    //   - The s corresponds to a pixel in the source
                    //       image.
                    //
                    //  dst:   *  *  *  *
                    //         1  3  2  5
                    //         s  0  s  4
                    //
                    
                    //
                    //  Calculate the interpolation points for band 0 and
                    //   set the corresponding eight pixels in the destination.
                    //
                    unsigned int  interp0 =
                        (unsigned int)(*src_pixel + *(src_pixel+1))>>1;
                    unsigned int  interp1 =
                        (unsigned int)(*dst_p_pixel + *src_pixel)>>1;
                    unsigned int  interp2 =
                        (unsigned int)(*(dst_p_pixel+2) + *(src_pixel+1))>>1;
                    unsigned int  interp3 =
                        (unsigned int)(*(dst_p_pixel+1) + interp0)>>1;
                    unsigned int  interp4 =
                        (unsigned int)(*(src_pixel+1) + *(src_pixel+2))>>1;
                    unsigned int  interp5 =
                        (unsigned int)(*(dst_p_pixel+3) + interp4)>>1;

                    *dst_pixel       = interp1;
                    *(dst_pixel+1)   = interp3;
                    *(dst_pixel+2)   = interp2;
                    *(dst_pixel+3)   = interp5;
                    
                    *dst_n_pixel      = *src_pixel;
                    *(dst_n_pixel+1)  = interp0;
                    *(dst_n_pixel+2)  = *(src_pixel + 1);
                    *(dst_n_pixel+3)  = interp4;
                    
                    dst_p_pixel += 4;
                    dst_n_pixel += 4;
                    dst_pixel   += 4;
                    src_pixel   += 2;
                }
                
                switch(end_pixels) {
                    case 4:
                    case 3:
                    {
                        //
                        //  Calculate the interpolation points for band 0 and
                        //   set the corresponding pixels in the destination.
                        //
                        unsigned int  interp0 =
                            (unsigned int)(*src_pixel + *(src_pixel+1))>>1;
                        unsigned int  interp1 =
                            (unsigned int)(*dst_p_pixel + *src_pixel)>>1;
                        unsigned int  interp2 =
                            (unsigned int)(*(dst_p_pixel+2)+*(src_pixel+1))>>1;
                        unsigned int  interp3 =
                            (unsigned int)(*(dst_p_pixel+1) + interp0)>>1;
                        
                        *dst_pixel       = interp1;
                        *(dst_pixel+1)   = interp3;
                        *(dst_pixel+2)   = interp2;
                        
                        *dst_n_pixel      = *src_pixel;
                        *(dst_n_pixel+1)  = interp0;
                        *(dst_n_pixel+2)  = *(src_pixel + 1);
                    }
                    break;
                    
                    case 2:
                    {
                        //
                        //  Calculate the interpolation points for band 0 and
                        //   set the corresponding pixels in the destination.
                        //
                        unsigned int  interp0 =
                            (unsigned int)(*src_pixel + *(src_pixel+1))>>1;
                        unsigned int  interp1 =
                            (unsigned int)(*dst_p_pixel + *src_pixel)>>1;
                        unsigned int  interp3 =
                            (unsigned int)(*(dst_p_pixel+1) + interp0)>>1;
                        
                        *dst_pixel       = interp1;
                        *(dst_pixel+1)   = interp3;
                        
                        *dst_n_pixel      = *src_pixel;
                        *(dst_n_pixel+1)  = interp0;
                    }
                    break;
                    
                    case 1:
                    {
                        //
                        //  Calculate the interpolation points for band 0 and
                        //   set the corresponding pixels in the destination.
                        //
                        unsigned int  interp1 =
                            (unsigned int)(*dst_p_pixel + *src_pixel)>>1;
                        
                        *dst_pixel       = interp1;
                        *dst_n_pixel      = *src_pixel;
                    }
                    break;
                }
                
                //
                //  Reset everything back to the beginning of the
                //   next scanline.
                //
                src_scanline += src_nx_scan;
                src_pixel = src_scanline;

                dst_p_pixel = dst_p_scanline = dst_n_scanline;
                
                dst_scanline += dst_nx_scan2;
                dst_pixel = dst_scanline;
                
                dst_n_scanline += dst_nx_scan2;
                dst_n_pixel    = dst_n_scanline;
            }

            if(!odd_lines) {
                if(odd_start_pixel) {
                    unsigned int  interp0 =
                        (unsigned int)(*src_pixel + *(src_pixel+1))>>1;
                    unsigned int  interp1 =
                        (unsigned int)(*dst_p_pixel + interp0)>>1;

                    *dst_pixel++   = interp1;

                    dst_p_pixel++;
                    src_pixel++;
                }
                for(int j=0; j<dstR_xsize_4; j++) {
                    //
                    //  Calculate the interpolation points for band 0 and set
                    //   the corresponding eight pixels in the destination.
                    //
                    unsigned int  interp0 =
                        (unsigned int)(*src_pixel + *(src_pixel+1))>>1;
                    unsigned int  interp1 =
                        (unsigned int)(*dst_p_pixel + *src_pixel)>>1;
                    unsigned int  interp2 =
                        (unsigned int)(*(dst_p_pixel+2) + *(src_pixel+1))>>1;
                    unsigned int  interp3 =
                        (unsigned int)(*(dst_p_pixel+1) + interp0)>>1;
                    unsigned int  interp4 =
                        (unsigned int)(*(src_pixel+1) + *(src_pixel+2))>>1;
                    unsigned int  interp5 =
                        (unsigned int)(*(dst_p_pixel+3) + interp4)>>1;
                    
                    *dst_pixel       = interp1;
                    *(dst_pixel+1)   = interp3;
                    *(dst_pixel+2)   = interp2;
                    *(dst_pixel+3)   = interp5;

                    dst_p_pixel += 4;
                    dst_n_pixel += 4;
                    dst_pixel   += 4;
                    src_pixel   += 2;
                }
                
                switch(end_pixels) {
                    case 3:
                    {
                        //
                        //  Calculate the interpolation points for band 0 and
                        //   set the corresponding pixels in the destination.
                        //
                        unsigned int  interp0 =
                            (unsigned int)(*src_pixel + *(src_pixel+1))>>1;
                        unsigned int  interp1 =
                            (unsigned int)(*dst_p_pixel + *src_pixel)>>1;
                        unsigned int  interp2 =
                            (unsigned int)(*(dst_p_pixel+2)+*(src_pixel+1))>>1;
                        unsigned int  interp3 =
                            (unsigned int)(*(dst_p_pixel+1) + interp0)>>1;
                        
                        *dst_pixel       = interp1;
                        *(dst_pixel+1)   = interp3;
                        *(dst_pixel+2)   = interp2;
                    }
                    break;
                    
                    case 2:
                    {
                        //
                        //  Calculate the interpolation points for band 0 and
                        //   set the corresponding pixels in the destination.
                        //
                        unsigned int  interp0 =
                            (unsigned int)(*src_pixel + *(src_pixel+1))>>1;
                        unsigned int  interp1 =
                            (unsigned int)(*dst_p_pixel + *src_pixel)>>1;
                        unsigned int  interp3 =
                            (unsigned int)(*(dst_p_pixel+1) + interp0)>>1;

                        *dst_pixel       = interp1;
                        *(dst_pixel+1)   = interp3;
                    }
                    break;
                    
                    case 1:
                    {
                        //
                        //  Calculate the interpolation points for band 0 and
                        //   set the corresponding pixels in the destination.
                        //
                        unsigned int  interp1 =
                            (unsigned int)(*dst_p_pixel + *src_pixel)>>1;
                        
                        *dst_pixel       = interp1;
                    }
                    break;
                }
            }
            delete line_buffer;
        }
    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    //
    //      3 CONTIGUOUS BAND OPTIMIZED CASE
    //
    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    } else if(nbands == 3 && src_nx_pixel == 3 && dst_nx_pixel == 3) {
        //
        //  Loop over the rectangle list
        //
        int dstR_x;
        int dstR_y;
        unsigned int dstR_xsize;
        unsigned int dstR_ysize;
        while(rl.getNext(&dstR_x, &dstR_y, &dstR_xsize, &dstR_ysize)) {
            //
            //  The starting point in the destination.
            //
            Xil_unsigned8* dst_scanline = dst_base_addr +
                (dstR_y * dst_nx_scan) +
                (dstR_x * dst_nx_pixel);
            Xil_unsigned8* dst_pixel = dst_scanline;
            
            //
            // Backward map the upper left destination rectangle corner.
            // This mapping includes the effect of src and dst origins.
            //
            double sx;
            double sy;
            op->backwardMap(dst_box, (double)dstR_x, (double)dstR_y,
                            src_box, &sx, &sy);

            //
            //  Since we do the first line seperately and then do a
            //   two-line algorithm there is either 1 or 2 lines left
            //   depending upon whether we're odd or even.
            //
            //  This variable is set to 1 if it's odd which means we
            //   add an extra 1 to our loop to complete the last lines.
            //  
            unsigned int  odd_start_line =
                (_XILI_ROUND(sy) == _XILI_ROUND(sy + 0.5F)) ? 1 : 0;
            unsigned int  odd_start_pixel =
                (_XILI_ROUND(sx) == _XILI_ROUND(sx + 0.5F)) ? 1 : 0;
            int odd_lines = dstR_ysize % 2;
            int end_pixels = dstR_xsize % 4;
            Xil_unsigned8* line_buffer  = NULL;

            int dstR_ysize_2 = dstR_ysize/2 + odd_lines;
            int dstR_xsize_4 = dstR_xsize/4;

            //
            //  Check for edge conditions (in image coordinate space)
            //
            {
                //
                // Backward map the lower right corner of the dst rectangle.
                // The results are in src_box coordinate space.
                //
                double src_x;
                double src_y;
                op->backwardMap(dst_box,
                                (double)(dstR_x + (int)dstR_xsize - 1),
                                (double)(dstR_y + (int)dstR_ysize - 1),
                                src_box,
                                &src_x,
                                &src_y);

                //
                // Check the edge conditions in src image space.
                //
                if((src_x + src_box_x0_image) > (double)(src_xsize - 1)) {
                    dstR_xsize_4--;
                    end_pixels = 4;
                }

                if((src_y + src_box_y0_image) > (double)(src_ysize - 1)) {
                    dstR_ysize -= 1;
                    odd_lines = 1; // this forces it not to touch bottom line
                }
            }

            //
            //  The starting point in the source.
            //
	    if (sx < 0 && odd_start_pixel)
		sx -= 1.0;
	    if (sy < 0 && odd_start_line)
		sy -= 1.0;
            Xil_unsigned8* src_scanline = src_base_addr + 
                ((int)sy * src_nx_scan) +
                ((int)sx * src_nx_pixel);
            Xil_unsigned8* src_pixel = src_scanline;

            //
            //  If we're starting on an odd line, then we must
            //   allocate our own buffer.
            //
            if(odd_start_line) {
                odd_lines = !odd_lines;
                dstR_ysize_2 += odd_lines;
                line_buffer    = new Xil_unsigned8[dstR_xsize*3];
                dst_pixel      = line_buffer;
            }
            
            if(odd_start_pixel) {
                unsigned int interp0 =
                    (unsigned int)(*src_pixel + *(src_pixel+3))>>1;
                *dst_pixel     = interp0;
                    
                interp0 =
                (unsigned int)(*(src_pixel+1) + *(src_pixel+4))>>1;
                *(dst_pixel+1) = interp0;

                interp0 =
                    (unsigned int)(*(src_pixel+2) + *(src_pixel+5))>>1;
                *(dst_pixel+2) = interp0;
                
                dst_pixel += 3;
                src_pixel += 3;
                
                if(end_pixels>0) {
                    end_pixels--;
                } else {
                    end_pixels = 3;
                    dstR_xsize_4--;
                }
            }
            //
            //  Special case the first scanline.  I compute this and then
            //  do a more general two-line algorithm for the rest of the image.
            //
            int i;
            for(i=0; i<dstR_xsize_4; i++) {
                //
                //  Calculate the interpolation points for band 0 and set
                //   the corresponding four pixels in the destination.
                //
                int  interp0 =
                    (int)(*src_pixel + *(src_pixel+3))>>1;
                int  interp1 =
                    (int)(*(src_pixel+3) + *(src_pixel+6))>>1;
                
                *dst_pixel      = *src_pixel;
                *(dst_pixel+3)  = interp0;
                *(dst_pixel+6)  = *(src_pixel+3);
                *(dst_pixel+9)  = interp1;
                
                //
                //  Calculate the interpolation points for band 1 and set
                //   the corresponding four pixels in the destination.
                //
                interp0 =
                (int)(*(src_pixel+1) + *(src_pixel+4))>>1;
                interp1 =
                    (int)(*(src_pixel+4) + *(src_pixel+7))>>1;
                
                *(dst_pixel+1)  = *(src_pixel+1);
                *(dst_pixel+4)  = interp0;
                *(dst_pixel+7)  = *(src_pixel+4);
                *(dst_pixel+10) = interp1;
                
                //
                //  Calculate the interpolation points for band 2 and set
                //   the corresponding four pixels in the destination.
                //
                interp0 =
                    (int)(*(src_pixel+2) + *(src_pixel+5))>>1;
                interp1 =
                    (int)(*(src_pixel+5) + *(src_pixel+8))>>1;
                
                *(dst_pixel+2)  = *(src_pixel+2);
                *(dst_pixel+5)  = interp0;
                *(dst_pixel+8)  = *(src_pixel+5);
                *(dst_pixel+11) = interp1;
                
                dst_pixel += 12;
                src_pixel += 6;
            }
            
            switch(end_pixels) {
                case 4:
                case 3:
                {
                    //
                    //  Calculate the interpolation points for band 0 and set
                    //   the corresponding three pixels in the destination.
                    //
                    int interp0 =
                        (int)(*src_pixel + *(src_pixel+3))>>1;
                    
                    *dst_pixel      = *src_pixel;
                    *(dst_pixel+3)  = interp0;
                    *(dst_pixel+6)  = *(src_pixel+3);
                    
                    //
                    //  Calculate the interpolation points for band 1 and set
                    //   the corresponding three pixels in the destination.
                    //
                    interp0 =
                        (int)(*(src_pixel+1) + *(src_pixel+4))>>1;
                    
                    *(dst_pixel+1)  = *(src_pixel+1);
                    *(dst_pixel+4)  = interp0;
                    *(dst_pixel+7)  = *(src_pixel+4);
                    
                    //
                    //  Calculate the interpolation points for band 2 and set
                    //   the corresponding three pixels in the destination.
                    //
                    interp0 =
                        (int)(*(src_pixel+2) + *(src_pixel+5))>>1;
                    
                    *(dst_pixel+2)  = *(src_pixel+2);
                    *(dst_pixel+5)  = interp0;
                    *(dst_pixel+8)  = *(src_pixel+5);
                }
                break;
                
                case 2:
                {
                    //
                    //  Calculate the interpolation points for band 0 and set
                    //   the corresponding two pixels in the destination.
                    //
                    int  interp0 =
                        (int)(*src_pixel + *(src_pixel+3))>>1;
                    
                    *dst_pixel      = *src_pixel;
                    *(dst_pixel+3)  = interp0;
                    
                    //
                    //  Calculate the interpolation points for band 1 and set
                    //   the corresponding two pixels in the destination.
                    //
                    interp0 =
                        (int)(*(src_pixel+1) + *(src_pixel+4))>>1;
                    
                    *(dst_pixel+1)  = *(src_pixel+1);
                    *(dst_pixel+4)  = interp0;
                    
                    //
                    //  Calculate the interpolation points for band 2 and set
                    //   the corresponding two pixels in the destination.
                    //
                    interp0 =
                        (int)(*(src_pixel+2) + *(src_pixel+5))>>1;
                    
                    *(dst_pixel+2)  = *(src_pixel+2);
                    *(dst_pixel+5)  = interp0;
                }
                break;
                
                case 1:
                //
                //  Set the corresponding pixel in the destination.
                //
                *dst_pixel      = *src_pixel;
                *(dst_pixel+1)  = *(src_pixel+1);
                *(dst_pixel+2)  = *(src_pixel+2);
                break;
            }
            
            //
            //  Initialize the variables which we'll use to move through
            //   the image.
            //
            //  The destination is incremented by two since we're doing
            //   two lines of the destination for every line of the
            //   source.  This saves an add per scanline.
            //
            unsigned int  dst_nx_scan2 = dst_nx_scan + dst_nx_scan;
            
            //
            //  Here I set a pointer to the scanline we just computed.
            //
            Xil_unsigned8* dst_p_scanline = dst_scanline;
            Xil_unsigned8* dst_p_pixel    = dst_scanline;
            
            //
            //  Only increment each by one scanline this time since we've
            //   only completed a single scanline in the destination.
            //
            if(odd_start_line) {
                dst_p_pixel = dst_p_scanline = line_buffer;
                dst_pixel = dst_scanline;
            } else {
                dst_pixel = dst_scanline += dst_nx_scan;
            }
            src_pixel = src_scanline += src_nx_scan;
            
            Xil_unsigned8* dst_n_scanline = dst_scanline + dst_nx_scan;
            Xil_unsigned8* dst_n_pixel    = dst_n_scanline;
            
            for(i=1; i<dstR_ysize_2; i++) {
                if(odd_start_pixel) {
                    unsigned int  interp0 =
                        (unsigned int)(*src_pixel + *(src_pixel+3))>>1;
                    unsigned int  interp1 =
                        (unsigned int)(*dst_p_pixel + interp0)>>1;
                    *dst_pixel       = interp1;
                    *dst_n_pixel     = interp0;

                    interp0 =
                        (unsigned int)(*(src_pixel+1) + *(src_pixel+4))>>1;
                    interp1 =
                        (unsigned int)(*(dst_p_pixel+1) + interp0)>>1;
                    *(dst_pixel+1)   = interp1;
                    *(dst_n_pixel+1) = interp0;

                    interp0 =
                        (unsigned int)(*(src_pixel+2) + *(src_pixel+5))>>1;
                    interp1 =
                        (unsigned int)(*(dst_p_pixel+2) + interp0)>>1;
                    *(dst_pixel+2)   = interp1;
                    *(dst_n_pixel+2) = interp0;

                    dst_pixel   += 3;
                    dst_p_pixel += 3;
                    dst_n_pixel += 3;
                    src_pixel   += 3;
                }
                
                for(int j=0; j<dstR_xsize_4; j++) {
                    //
                    //  Below is the layout of the interpolated values.
                    //   - The *'s refer to the previous scanline in the
                    //       destination.
                    //   - The numbers correspond to the different
                    //       interpolated variables. 
                    //   - The s corresponds to a pixel in the source
                    //       image.
                    //
                    //  dst:   *  *  *  *
                    //         1  3  2  5
                    //         s  0  s  4
                    //
                    
                    //
                    //  Calculate the interpolation points for band 0 and
                    //   set the corresponding eight pixels in the destination.
                    //
                    unsigned int  interp0 =
                        (unsigned int)(*src_pixel + *(src_pixel+3))>>1;
                    unsigned int  interp1 =
                        (unsigned int)(*dst_p_pixel + *src_pixel)>>1;
                    unsigned int  interp2 =
                        (unsigned int)(*(dst_p_pixel+6) + *(src_pixel+3))>>1;
                    unsigned int  interp3 =
                        (unsigned int)(*(dst_p_pixel+3) + interp0)>>1;
                    unsigned int  interp4 =
                        (unsigned int)(*(src_pixel+3) + *(src_pixel+6))>>1;
                    unsigned int  interp5 =
                        (unsigned int)(*(dst_p_pixel+9) + interp4)>>1;
                    
                    *dst_pixel       = interp1;
                    *(dst_pixel+3)   = interp3;
                    *(dst_pixel+6)   = interp2;
                    *(dst_pixel+9)   = interp5;
                    
                    *dst_n_pixel      = *src_pixel;
                    *(dst_n_pixel+3)  = interp0;
                    *(dst_n_pixel+6)  = *(src_pixel + 3);
                    *(dst_n_pixel+9)  = interp4;
                    
                    //
                    //  Calculate the interpolation points for band 1 and set
                    //   the corresponding eight pixels in the destination.
                    //
                    interp0 =
                        (unsigned int)(*(src_pixel+1) + *(src_pixel+4))>>1;
                    interp1 =
                        (unsigned int)(*(dst_p_pixel+1) + *(src_pixel+1))>>1;
                    interp2 =
                        (unsigned int)(*(dst_p_pixel+7) + *(src_pixel+4))>>1;
                    interp3 =
                        (unsigned int)(*(dst_p_pixel+4) + interp0)>>1;
                    interp4 =
                        (unsigned int)(*(src_pixel+4) + *(src_pixel+7))>>1;
                    interp5 =
                        (unsigned int)(*(dst_p_pixel+10) + interp4)>>1;
                    
                    *(dst_pixel+1)    = interp1;
                    *(dst_pixel+4)    = interp3;
                    *(dst_pixel+7)    = interp2;
                    *(dst_pixel+10)   = interp5;
                    
                    *(dst_n_pixel+1)  = *(src_pixel+1);
                    *(dst_n_pixel+4)  = interp0;
                    *(dst_n_pixel+7)  = *(src_pixel+4);
                    *(dst_n_pixel+10) = interp4;
                    
                    //
                    //  Calculate the interpolation points for band 2 and set
                    //   the corresponding eight pixels in the destination.
                    //
                    interp0 =
                        (unsigned int)(*(src_pixel+2) + *(src_pixel+5))>>1;
                    interp1 =
                        (unsigned int)(*(dst_p_pixel+2) + *(src_pixel+2))>>1;
                    interp2 =
                        (unsigned int)(*(dst_p_pixel+8) + *(src_pixel+5))>>1;
                    interp3 =
                        (unsigned int)(*(dst_p_pixel+5) + interp0)>>1;
                    interp4 =
                        (unsigned int)(*(src_pixel+5) + *(src_pixel+8))>>1;
                    interp5 =
                        (unsigned int)(*(dst_p_pixel+11) + interp4)>>1;
                    
                    *(dst_pixel+2)    = interp1;
                    *(dst_pixel+5)    = interp3;
                    *(dst_pixel+8)    = interp2;
                    *(dst_pixel+11)   = interp5;
                    
                    *(dst_n_pixel+2)  = *(src_pixel+2);
                    *(dst_n_pixel+5)  = interp0;
                    *(dst_n_pixel+8)  = *(src_pixel+5);
                    *(dst_n_pixel+11) = interp4;
                    
                    dst_p_pixel += 12;
                    dst_n_pixel += 12;
                    dst_pixel   += 12;
                    src_pixel   += 6;
                }
                
                switch(end_pixels) {
                    case 4:
                    case 3:
                    {
                        //
                        //  Calculate the interpolation points for band 0 and
                        //   set the corresponding pixels in the destination.
                        //
                        unsigned int  interp0 =
                            (unsigned int)(*src_pixel + *(src_pixel+3))>>1;
                        unsigned int  interp1 =
                            (unsigned int)(*dst_p_pixel + *src_pixel)>>1;
                        unsigned int  interp2 =
                            (unsigned int)(*(dst_p_pixel+6)+*(src_pixel+3))>>1;
                        unsigned int  interp3 =
                            (unsigned int)(*(dst_p_pixel+3) + interp0)>>1;
                        
                        *dst_pixel       = interp1;
                        *(dst_pixel+3)   = interp3;
                        *(dst_pixel+6)   = interp2;
                        
                        *dst_n_pixel      = *src_pixel;
                        *(dst_n_pixel+3)  = interp0;
                        *(dst_n_pixel+6)  = *(src_pixel + 3);
                        
                        //
                        //  Calculate the interpolation points for band 1 and
                        //   set the corresponding pixels in the destination.
                        //
                        interp0 =
                            (unsigned int)(*(src_pixel+1) + *(src_pixel+4))>>1;
                        interp1 =
                            (unsigned int)(*(dst_p_pixel+1)+*(src_pixel+1))>>1;
                        interp2 =
                            (unsigned int)(*(dst_p_pixel+7)+*(src_pixel+4))>>1;
                        interp3 =
                            (unsigned int)(*(dst_p_pixel+4) + interp0)>>1;
                        
                        *(dst_pixel+1)    = interp1;
                        *(dst_pixel+4)    = interp3;
                        *(dst_pixel+7)    = interp2;
                        
                        *(dst_n_pixel+1)  = *(src_pixel+1);
                        *(dst_n_pixel+4)  = interp0;
                        *(dst_n_pixel+7)  = *(src_pixel+4);
                        
                        //
                        //  Calculate the interpolation points for band 2 and
                        //   set the corresponding pixels in the destination.
                        //
                        interp0 =
                            (unsigned int)(*(src_pixel+2) + *(src_pixel+5))>>1;
                        interp1 =
                            (unsigned int)(*(dst_p_pixel+2)+*(src_pixel+2))>>1;
                        interp2 =
                            (unsigned int)(*(dst_p_pixel+8)+*(src_pixel+5))>>1;
                        interp3 =
                            (unsigned int)(*(dst_p_pixel+5) + interp0)>>1;
                        
                        *(dst_pixel+2)    = interp1;
                        *(dst_pixel+5)    = interp3;
                        *(dst_pixel+8)    = interp2;
                        
                        *(dst_n_pixel+2)  = *(src_pixel+2);
                        *(dst_n_pixel+5)  = interp0;
                        *(dst_n_pixel+8)  = *(src_pixel+5);
                    }
                    break;
                    
                    case 2:
                    {
                        //
                        //  Calculate the interpolation points for band 0 and
                        //   set the corresponding pixels in the destination.
                        //
                        unsigned int  interp0 =
                            (unsigned int)(*src_pixel + *(src_pixel+3))>>1;
                        unsigned int  interp1 =
                            (unsigned int)(*dst_p_pixel + *src_pixel)>>1;
                        unsigned int  interp3 =
                            (unsigned int)(*(dst_p_pixel+3) + interp0)>>1;
                        
                        *dst_pixel       = interp1;
                        *(dst_pixel+3)   = interp3;
                        
                        *dst_n_pixel      = *src_pixel;
                        *(dst_n_pixel+3)  = interp0;
                        
                        //
                        //  Calculate the interpolation points for band 1 and
                        //   set the corresponding pixels in the destination.
                        //
                        interp0 =
                            (unsigned int)(*(src_pixel+1) + *(src_pixel+4))>>1;
                        interp1 =
                            (unsigned int)(*(dst_p_pixel+1)+*(src_pixel+1))>>1;
                        interp3 =
                            (unsigned int)(*(dst_p_pixel+4) + interp0)>>1;
                        
                        *(dst_pixel+1)    = interp1;
                        *(dst_pixel+4)    = interp3;
                        
                        *(dst_n_pixel+1)  = *(src_pixel+1);
                        *(dst_n_pixel+4)  = interp0;
                        
                        //
                        //  Calculate the interpolation points for band 2 and
                        //   set the corresponding pixels in the destination.
                        //
                        interp0 =
                            (unsigned int)(*(src_pixel+2) + *(src_pixel+5))>>1;
                        interp1 =
                            (unsigned int)(*(dst_p_pixel+2)+*(src_pixel+2))>>1;
                        interp3 =
                            (unsigned int)(*(dst_p_pixel+5) + interp0)>>1;
                        
                        *(dst_pixel+2)    = interp1;
                        *(dst_pixel+5)    = interp3;
                        
                        *(dst_n_pixel+2)  = *(src_pixel+2);
                        *(dst_n_pixel+5)  = interp0;
                    }
                    break;
                    
                    case 1:
                    {
                        //
                        //  Calculate the interpolation points for band 0 and
                        //   set the corresponding pixels in the destination.
                        //
                        unsigned int  interp1 =
                            (unsigned int)(*dst_p_pixel + *src_pixel)>>1;
                        
                        *dst_pixel       = interp1;
                        *dst_n_pixel      = *src_pixel;
                        
                        //
                        //  Calculate the interpolation points for band 1 and
                        //   set the corresponding pixels in the destination.
                        //
                        interp1 =
                            (unsigned int)(*(dst_p_pixel+1)+*(src_pixel+1))>>1;
                        
                        *(dst_pixel+1)    = interp1;
                        *(dst_n_pixel+1)  = *(src_pixel+1);
                        
                        //
                        //  Calculate the interpolation points for band 2 and
                        //   set the corresponding pixels in the destination.
                        //
                        interp1 =
                            (unsigned int)(*(dst_p_pixel+2)+*(src_pixel+2))>>1;
                        
                        *(dst_pixel+2)    = interp1;
                        *(dst_n_pixel+2)  = *(src_pixel+2);
                    }
                    break;
                }
                
                //
                //  Reset everything back to the beginning of the
                //   next scanline.
                //
                src_scanline += src_nx_scan;
                src_pixel = src_scanline;
                
                dst_p_scanline += dst_nx_scan2;
                dst_p_pixel    = dst_n_scanline;
                
                dst_scanline += dst_nx_scan2;
                dst_pixel = dst_scanline;
                
                dst_n_scanline += dst_nx_scan2;
                dst_n_pixel    = dst_n_scanline;
            }

            if(!odd_lines) {
                if(odd_start_pixel) {
                    unsigned int  interp0 =
                        (unsigned int)(*src_pixel + *(src_pixel+3))>>1;
                    unsigned int  interp1 =
                        (unsigned int)(*dst_p_pixel + interp0)>>1;
                    *dst_pixel       = interp1;

                    interp0 =
                        (unsigned int)(*(src_pixel+1) + *(src_pixel+4))>>1;
                    interp1 =
                        (unsigned int)(*(dst_p_pixel+1) + interp0)>>1;
                    *(dst_pixel+1)   = interp1;

                    interp0 =
                        (unsigned int)(*(src_pixel+2) + *(src_pixel+5))>>1;
                    interp1 =
                        (unsigned int)(*(dst_p_pixel+2) + interp0)>>1;
                    *(dst_pixel+2)   = interp1;

                    dst_pixel   += 3;
                    dst_p_pixel += 3;
                    src_pixel   += 3;
                }
                
                for(unsigned int j=0; j<dstR_xsize/4; j++) {
                    //
                    //  Calculate the interpolation points for band 0 and set
                    //   the corresponding eight pixels in the destination.
                    //
                    unsigned int  interp0 =
                        (unsigned int)(*src_pixel + *(src_pixel+3))>>1;
                    unsigned int  interp1 =
                        (unsigned int)(*dst_p_pixel + *src_pixel)>>1;
                    unsigned int  interp2 =
                        (unsigned int)(*(dst_p_pixel+6) + *(src_pixel+3))>>1;
                    unsigned int  interp3 =
                        (unsigned int)(*(dst_p_pixel+3) + interp0)>>1;
                    unsigned int  interp4 =
                        (unsigned int)(*(src_pixel+3) + *(src_pixel+6))>>1;
                    unsigned int  interp5 =
                        (unsigned int)(*(dst_p_pixel+9) + interp4)>>1;
                    
                    *dst_pixel       = interp1;
                    *(dst_pixel+3)   = interp3;
                    *(dst_pixel+6)   = interp2;
                    *(dst_pixel+9)   = interp5;
                    
                    //
                    //  Calculate the interpolation points for band 1 and set
                    //   the corresponding eight pixels in the destination.
                    //
                    interp0 =
                        (unsigned int)(*(src_pixel+1) + *(src_pixel+4))>>1;
                    interp1 =
                        (unsigned int)(*(dst_p_pixel+1) + *(src_pixel+1))>>1;
                    interp2 =
                        (unsigned int)(*(dst_p_pixel+7) + *(src_pixel+4))>>1;
                    interp3 =
                        (unsigned int)(*(dst_p_pixel+4) + interp0)>>1;
                    interp4 =
                        (unsigned int)(*(src_pixel+4) + *(src_pixel+7))>>1;
                    interp5 =
                        (unsigned int)(*(dst_p_pixel+10) + interp4)>>1;
                    
                    *(dst_pixel+1)    = interp1;
                    *(dst_pixel+4)    = interp3;
                    *(dst_pixel+7)    = interp2;
                    *(dst_pixel+10)   = interp5;
                    
                    //
                    //  Calculate the interpolation points for band 2 and set
                    //   the corresponding eight pixels in the destination.
                    //
                    interp0 =
                        (unsigned int)(*(src_pixel+2) + *(src_pixel+5))>>1;
                    interp1 =
                        (unsigned int)(*(dst_p_pixel+2) + *(src_pixel+2))>>1;
                    interp2 =
                        (unsigned int)(*(dst_p_pixel+8) + *(src_pixel+5))>>1;
                    interp3 =
                        (unsigned int)(*(dst_p_pixel+5) + interp0)>>1;
                    interp4 =
                        (unsigned int)(*(src_pixel+5) + *(src_pixel+8))>>1;
                    interp5 =
                        (unsigned int)(*(dst_p_pixel+11) + interp4)>>1;
                    
                    *(dst_pixel+2)    = interp1;
                    *(dst_pixel+5)    = interp3;
                    *(dst_pixel+8)    = interp2;
                    *(dst_pixel+11)   = interp5;
                    
                    dst_p_pixel += 12;
                    dst_n_pixel += 12;
                    dst_pixel   += 12;
                    src_pixel   += 6;
                }
                
                switch(end_pixels) {
                    case 4:
                    case 3:
                    {
                        //
                        //  Calculate the interpolation points for band 0 and
                        //   set the corresponding pixels in the destination.
                        //
                        unsigned int  interp0 =
                            (unsigned int)(*src_pixel + *(src_pixel+3))>>1;
                        unsigned int  interp1 =
                            (unsigned int)(*dst_p_pixel + *src_pixel)>>1;
                        unsigned int  interp2 =
                            (unsigned int)(*(dst_p_pixel+6)+*(src_pixel+3))>>1;
                        unsigned int  interp3 =
                            (unsigned int)(*(dst_p_pixel+3) + interp0)>>1;
                        
                        *dst_pixel       = interp1;
                        *(dst_pixel+3)   = interp3;
                        *(dst_pixel+6)   = interp2;
                        
                        //
                        //  Calculate the interpolation points for band 1 and
                        //   set the corresponding pixels in the destination.
                        //
                        interp0 =
                            (unsigned int)(*(src_pixel+1) + *(src_pixel+4))>>1;
                        interp1 =
                            (unsigned int)(*(dst_p_pixel+1)+*(src_pixel+1))>>1;
                        interp2 =
                            (unsigned int)(*(dst_p_pixel+7)+*(src_pixel+4))>>1;
                        interp3 =
                            (unsigned int)(*(dst_p_pixel+4) + interp0)>>1;
                        
                        *(dst_pixel+1)    = interp1;
                        *(dst_pixel+4)    = interp3;
                        *(dst_pixel+7)    = interp2;
                        
                        //
                        //  Calculate the interpolation points for band 2 and
                        //   set the corresponding pixels in the destination.
                        //
                        interp0 =
                            (unsigned int)(*(src_pixel+2) + *(src_pixel+5))>>1;
                        interp1 =
                            (unsigned int)(*(dst_p_pixel+2)+*(src_pixel+2))>>1;
                        interp2 =
                            (unsigned int)(*(dst_p_pixel+8)+*(src_pixel+5))>>1;
                        interp3 =
                            (unsigned int)(*(dst_p_pixel+5) + interp0)>>1;
                        
                        *(dst_pixel+2)    = interp1;
                        *(dst_pixel+5)    = interp3;
                        *(dst_pixel+8)    = interp2;
                    }
                    break;
                    
                    case 2:
                    {
                        //
                        //  Calculate the interpolation points for band 0 and
                        //   set the corresponding pixels in the destination.
                        //
                        unsigned int  interp0 =
                            (unsigned int)(*src_pixel + *(src_pixel+3))>>1;
                        unsigned int  interp1 =
                            (unsigned int)(*dst_p_pixel + *src_pixel)>>1;
                        unsigned int  interp3 =
                            (unsigned int)(*(dst_p_pixel+3) + interp0)>>1;
                        
                        *dst_pixel       = interp1;
                        *(dst_pixel+3)   = interp3;
                        
                        //
                        //  Calculate the interpolation points for band 1 and
                        //   set the corresponding pixels in the destination.
                        //
                        interp0 =
                            (unsigned int)(*(src_pixel+1) + *(src_pixel+4))>>1;
                        interp1 =
                            (unsigned int)(*(dst_p_pixel+1)+*(src_pixel+1))>>1;
                        interp3 =
                            (unsigned int)(*(dst_p_pixel+4) + interp0)>>1;
                        
                        *(dst_pixel+1)    = interp1;
                        *(dst_pixel+4)    = interp3;
                        
                        //
                        //  Calculate the interpolation points for band 2 and
                        //   the corresponding pixels in the destination.
                        //
                        interp0 =
                            (unsigned int)(*(src_pixel+2) + *(src_pixel+5))>>1;
                        interp1 =
                            (unsigned int)(*(dst_p_pixel+2)+*(src_pixel+2))>>1;
                        interp3 =
                            (unsigned int)(*(dst_p_pixel+5) + interp0)>>1;
                        
                        *(dst_pixel+2)    = interp1;
                        *(dst_pixel+5)    = interp3;
                    }
                    break;
                    
                    case 1:
                    {
                        //
                        //  Calculate the interpolation points for band 0 and
                        //   set the corresponding pixels in the destination.
                        //
                        unsigned int  interp1 =
                            (unsigned int)(*dst_p_pixel + *src_pixel)>>1;
                        
                        *dst_pixel       = interp1;
                        
                        //
                        //  Calculate the interpolation points for band 1 and
                        //   set the corresponding pixels in the destination.
                        //
                        interp1 =
                            (unsigned int)(*(dst_p_pixel+1)+*(src_pixel+1))>>1;
                        
                        *(dst_pixel+1)    = interp1;
                        
                        //
                        //  Calculate the interpolation points for band 2 and
                        //   set the corresponding pixels in the destination.
                        //
                        interp1 =
                            (unsigned int)(*(dst_p_pixel+2)+*(src_pixel+2))>>1;
                        
                        *(dst_pixel+2)    = interp1;
                    }
                    break;
                }
            }
            delete line_buffer;
        }
    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    //
    //      N BAND CASE
    //
    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    } else {
        //
        //  Loop over the rectangle list
        //
        int dstR_x;
        int dstR_y;
        unsigned int dstR_xsize;
        unsigned int dstR_ysize;
        while(rl.getNext(&dstR_x, &dstR_y, &dstR_xsize, &dstR_ysize)) {
            //
            //  The starting point in the destination.
            //
            Xil_unsigned8* dst_scanline = dst_base_addr +
                (dstR_y * dst_nx_scan) +
                (dstR_x * dst_nx_pixel);
            Xil_unsigned8* dst_pixel = dst_scanline;
            Xil_unsigned8* dst_band  = dst_scanline;

            //
            // Backward map the upper left destination rectangle corner.
            // This mapping includes the effect of src and dst origins.
            //
            double sx;
            double sy;
            op->backwardMap(dst_box, (double)dstR_x, (double)dstR_y,
                            src_box, &sx, &sy);

            //
            //  Since we do the first line seperately and then do a
            //   two-line algorithm there is either 1 or 2 lines left
            //   depending upon whether we're odd or even.
            //
            //  This variable is set to 1 if it's odd which means we
            //   add an extra 1 to our loop to complete the last lines.
            //  
            unsigned int  odd_start_line =
                (_XILI_ROUND(sy) == _XILI_ROUND(sy + 0.5F)) ? 1 : 0;
            unsigned int  odd_start_pixel =
                (_XILI_ROUND(sx) == _XILI_ROUND(sx + 0.5F)) ? 1 : 0;
            int odd_lines = dstR_ysize % 2;
            int end_pixels = dstR_xsize % 4;
            Xil_unsigned8* line_buffer  = NULL;
        
            int dstR_ysize_2 = dstR_ysize/2 + odd_lines;
            int dstR_xsize_4 = dstR_xsize/4;

            //
            //  Check for edge conditions (in image coordinate space)
            //
            {
                //
                // Backward map the lower right corner of the dst rectangle.
                // The results are in src_box coordinate space.
                //
                double src_x;
                double src_y;
                op->backwardMap(dst_box,
                                (double)(dstR_x + (int)dstR_xsize - 1),
                                (double)(dstR_y + (int)dstR_ysize - 1),
                                src_box,
                                &src_x,
                                &src_y);

                //
                // Check the edge conditions in src image space.
                //
                if((src_x + src_box_x0_image) > (double)(src_xsize - 1)) {
                    dstR_xsize_4--;
                    end_pixels = 4;
                }

                if((src_y + src_box_y0_image) > (double)(src_ysize - 1)) {
                    dstR_ysize -= 1;
                    odd_lines = 1; // this forces it not to touch bottom line
                }
            }
            
            //
            //  The starting point in the source.
            //
	    if (sx < 0 && odd_start_pixel)
		sx -= 1.0;
	    if (sy < 0 && odd_start_line)
		sy -= 1.0;
            Xil_unsigned8* src_scanline = src_base_addr + 
                ((int)sy * src_nx_scan) +
                ((int)sx * src_nx_pixel);
            Xil_unsigned8* src_pixel = src_scanline;
            Xil_unsigned8* src_band  = src_scanline;

            //
            //  Pre-compute the increments...
            //
            unsigned int   dst_nx_pixel4 = dst_nx_pixel<<2; 
            unsigned int   dst_nx_pixel3 = (dst_nx_pixel<<1) + dst_nx_pixel; 
            unsigned int   dst_nx_pixel2 = dst_nx_pixel<<1; 
            unsigned int   src_nx_pixel2 = src_nx_pixel<<1;
            
            //
            //  If we're starting on an odd line, then we must
            //   allocate our own buffer.
            //
            if(odd_start_line) {
                odd_lines = !odd_lines;
                dstR_ysize_2 += odd_lines;
                
                line_buffer = new Xil_unsigned8[dstR_xsize*dst_nx_pixel];
                dst_band = dst_pixel = line_buffer;
            }
            
            if(odd_start_pixel) {
                for(unsigned int b=0; b<nbands; b++) {
                    unsigned int  interp0 =
                        (unsigned int)(*src_band +
                                       *(src_band+src_nx_pixel))>>1;
                    
                    *dst_band++   = interp0;

                    src_band++;
                }

                dst_pixel   += dst_nx_pixel;
                src_pixel   += src_nx_pixel;
                
                src_band   = src_pixel;
                dst_band   = dst_pixel;

                if(end_pixels>0) {
                    end_pixels--;
                } else {
                    end_pixels = 3;
                    dstR_xsize_4--;
                }
            }

            //
            //  Special case the first scanline.  I compute this and then
            //  do a more general two-line algorithm for the rest of the image.
            //
            int i;
            unsigned int b;
            for(i=0; i<dstR_xsize_4; i++) {
                for(b=0; b<nbands; b++) {
                    //
                    //  Calculate the interpolation points and set
                    //   the corresponding four pixels in the destination.
                    //
                    int  interp0 =
                        (int)(*src_band + *(src_band+src_nx_pixel))>>1;
                    int  interp1 =
                        (int)(*(src_band+src_nx_pixel) +
                              *(src_band+src_nx_pixel2))>>1;
                
                    *dst_band                  = *src_band;
                    *(dst_band+dst_nx_pixel)   = interp0;
                    *(dst_band+dst_nx_pixel2)  = *(src_band+src_nx_pixel);
                    *(dst_band+dst_nx_pixel3)  = interp1;

                    dst_band++;
                    src_band++;
                }
                
                dst_pixel += dst_nx_pixel4;
                src_pixel += src_nx_pixel2;
                
                src_band = src_pixel;
                dst_band = dst_pixel;
            }

            for(b=0; b<nbands; b++) {
                switch(end_pixels) {
                    case 4:
                    case 3:
                    {
                        //
                        //  Calculate the interpolation points and set
                        //   the corresponding three pixels in the destination.
                        //
                        int interp0 =
                            (int)(*src_band + *(src_band+src_nx_pixel))>>1;
                        
                        *dst_band                 = *src_band;
                        *(dst_band+dst_nx_pixel)  = interp0;
                        *(dst_band+dst_nx_pixel2) = *(src_band+src_nx_pixel);
                    }
                    break;
                
                    case 2:
                    {
                        //
                        //  Calculate the interpolation points and set
                        //   the corresponding two pixels in the destination.
                        //
                        int  interp0 =
                            (int)(*src_band + *(src_band+src_nx_pixel))>>1;
                        
                        *dst_band                 = *src_band;
                        *(dst_band+dst_nx_pixel)  = interp0;
                    }                    
                    break;
                    
                    case 1:
                    *dst_band      = *src_band;
                    break;
                }

                dst_band++;
                src_band++;
            }
            
            //
            //  Initialize the variables which we'll use to move through
            //   the image.
            //
            //  Here I set a pointer to the scanline we just computed.
            //
            Xil_unsigned8* dst_p_scanline = dst_scanline;
            Xil_unsigned8* dst_p_pixel    = dst_scanline;
            Xil_unsigned8* dst_p_band     = dst_scanline;
                
            //
            //  The destination is incremented by two since we're doing
            //   two lines of the destination for every line of the
            //   source.  This saves an add per scanline.
            //
            unsigned int  dst_nx_scan2 = dst_nx_scan + dst_nx_scan;
            
            //
            //  Only increment each by one scanline this time since we've
            //   only completed a single scanline in the destination.
            //
            if(odd_start_line) {
                dst_p_band = dst_p_pixel = dst_p_scanline = line_buffer;
                dst_band   = dst_pixel   = dst_scanline;
            } else {
                dst_band   = dst_pixel   = dst_scanline += dst_nx_scan;
            }
            src_band = src_pixel = src_scanline += src_nx_scan;
            
            Xil_unsigned8* dst_n_scanline = dst_scanline + dst_nx_scan;
            Xil_unsigned8* dst_n_pixel    = dst_n_scanline;
            Xil_unsigned8* dst_n_band     = dst_n_scanline;
            
            for(i=1; i<dstR_ysize_2; i++) {
                if(odd_start_pixel) {
                    for(unsigned int band_num=0; band_num<nbands; band_num++) {
                        unsigned int  interp0 =
                            (unsigned int)(*src_band +
                                           *(src_band+src_nx_pixel))>>1;
                        unsigned int  interp1 =
                            (unsigned int)(*dst_p_band + interp0)>>1;
                    
                        *dst_band++   = interp1;
                        *dst_n_band++ = interp0;
                        dst_p_band++;
                        src_band++;
                    }

                    dst_p_pixel += dst_nx_pixel;
                    dst_n_pixel += dst_nx_pixel;
                    dst_pixel   += dst_nx_pixel;
                    src_pixel   += src_nx_pixel;

                    src_band   = src_pixel;
                    dst_band   = dst_pixel;
                    dst_n_band = dst_n_pixel;
                    dst_p_band = dst_p_pixel;
                }
                
                for(int j=0; j<dstR_xsize_4; j++) {
                    for(unsigned int band_num=0; band_num<nbands; band_num++) {
                        //
                        //  Below is the layout of the interpolated values.
                        //   - The *'s refer to the previous scanline in the
                        //       destination.
                        //   - The numbers correspond to the different
                        //       interpolated variables. 
                        //   - The s corresponds to a pixel in the source
                        //       image.
                        //
                        //  dst:   *  *  *  *
                        //         1  3  2  5
                        //         s  0  s  4
                        //
                        
                        //
                        //  Calculate the interpolation points for band 0 and
                        //   set the corresponding eight pixels in
                        //   the destination.
                        //
                        unsigned int  interp0 = (unsigned int)
                            (*src_band + *(src_band+src_nx_pixel))>>1;
                        unsigned int  interp1 = (unsigned int)
                            (*dst_p_band + *src_band)>>1;
                        unsigned int  interp2 = (unsigned int)
                            (*(dst_p_band+dst_nx_pixel2) +
                             *(src_band+src_nx_pixel))>>1;
                        unsigned int  interp3 = (unsigned int)
                            (*(dst_p_band+dst_nx_pixel) + interp0)>>1;
                        unsigned int  interp4 = (unsigned int)
                            (*(src_band+src_nx_pixel) +
                             *(src_band+src_nx_pixel2))>>1;
                        unsigned int  interp5 = (unsigned int)
                            (*(dst_p_band+dst_nx_pixel3) + interp4)>>1;
                        
                        *dst_band                   = interp1;
                        *(dst_band+dst_nx_pixel)    = interp3;
                        *(dst_band+dst_nx_pixel2)   = interp2;
                        *(dst_band+dst_nx_pixel3)   = interp5;
                        
                        *dst_n_band                 = *src_band;
                        *(dst_n_band+dst_nx_pixel)  = interp0;
                        *(dst_n_band+dst_nx_pixel2) = *(src_band+src_nx_pixel);
                        *(dst_n_band+dst_nx_pixel3) = interp4;

                        src_band++;
                        dst_n_band++;
                        dst_band++;
                        dst_p_band++;
                    }
                    dst_p_pixel += dst_nx_pixel4;
                    dst_n_pixel += dst_nx_pixel4;
                    dst_pixel   += dst_nx_pixel4;
                    src_pixel   += src_nx_pixel2;

                    src_band   = src_pixel;
                    dst_band   = dst_pixel;
                    dst_n_band = dst_n_pixel;
                    dst_p_band = dst_p_pixel;
                }

                for(unsigned int band_num=0; band_num<nbands; band_num++) {
                    switch(end_pixels) {
                        case 4:
                        {
                            //
                            //  Calculate the interpolation points for
                            //   band 0 and set the corresponding
                            //   pixels in the destination. 
                            //
                            unsigned int  interp0 = (unsigned int)
                                (*src_band + *(src_band+src_nx_pixel))>>1;
                            unsigned int  interp1 = (unsigned int)
                                (*dst_p_band + *src_band)>>1;
                            unsigned int  interp2 = (unsigned int)
                                (*(dst_p_band+dst_nx_pixel2) +
                                 *(src_band+src_nx_pixel))>>1;
                            unsigned int  interp3 = (unsigned int)
                                (*(dst_p_band+dst_nx_pixel) + interp0)>>1;
                            
                            *dst_band                   = interp1;
                            *(dst_band+dst_nx_pixel)    = interp3;
                            *(dst_band+dst_nx_pixel2)   = interp2;
                            *(dst_band+dst_nx_pixel3)   = interp2;
                            
                            *dst_n_band                 = *src_band;
                            *(dst_n_band+dst_nx_pixel)  = interp0;
                            *(dst_n_band+dst_nx_pixel2) =
                                *(src_band+src_nx_pixel);
                            *(dst_n_band+dst_nx_pixel3) =
                                *(src_band+src_nx_pixel);
                        }
                        break;
                        
                        case 3:
                        {
                            //
                            //  Calculate the interpolation points for
                            //   band 0 and set the corresponding
                            //   pixels in the destination. 
                            //
                            unsigned int  interp0 = (unsigned int)
                                (*src_band + *(src_band+src_nx_pixel))>>1;
                            unsigned int  interp1 = (unsigned int)
                                (*dst_p_band + *src_band)>>1;
                            unsigned int  interp2 = (unsigned int)
                                (*(dst_p_band+dst_nx_pixel2) +
                                 *(src_band+src_nx_pixel))>>1;
                            unsigned int  interp3 = (unsigned int)
                                (*(dst_p_band+dst_nx_pixel) + interp0)>>1;
                            
                            *dst_band                   = interp1;
                            *(dst_band+dst_nx_pixel)    = interp3;
                            *(dst_band+dst_nx_pixel2)   = interp2;
                            
                            *dst_n_band                 = *src_band;
                            *(dst_n_band+dst_nx_pixel)  = interp0;
                            *(dst_n_band+dst_nx_pixel2) =
                                *(src_band+src_nx_pixel);
                        }
                        break;
                        
                        case 2:
                        {
                            //
                            //  Calculate the interpolation points for
                            //   band 0 and set the corresponding
                            //   pixels in the destination. 
                            //
                            unsigned int  interp0 = (unsigned int)
                                (*src_band + *(src_band+src_nx_pixel))>>1;
                            unsigned int  interp1 = (unsigned int)
                                (*dst_p_band + *src_band)>>1;
                            unsigned int  interp3 = (unsigned int)
                                (*(dst_p_band+dst_nx_pixel) + interp0)>>1;
                            
                            *dst_band                   = interp1;
                            *(dst_band+dst_nx_pixel)    = interp3;
                            
                            *dst_n_band                 = *src_band;
                            *(dst_n_band+dst_nx_pixel)  = interp0;
                        }
                        break;
                        
                        case 1:
                        {
                            //
                            //  Calculate the interpolation points for
                            //   band 0 and set the corresponding
                            //   pixels in the destination. 
                            //
                            unsigned int  interp1 = (unsigned int)
                                (*dst_p_band + *src_band)>>1;
                            
                            *dst_band                   = interp1;
                            
                            *dst_n_band                 = *src_band;
                        }
                        break;
                    }
                    
                    src_band++;
                    dst_band++;
                    dst_n_band++;
                    dst_p_band++;
                }
                
                //
                //  Reset everything back to the beginning of the
                //   next scanline.
                //
                src_scanline += src_nx_scan;
                src_pixel = src_scanline;
                src_band  = src_scanline;
                
                dst_p_scanline += dst_nx_scan2;
                dst_p_pixel = dst_n_scanline;
                dst_p_band = dst_n_scanline;
                
                dst_scanline += dst_nx_scan2;
                dst_pixel = dst_scanline;
                dst_band  = dst_scanline;
                
                dst_n_scanline += dst_nx_scan2;
                dst_n_pixel = dst_n_scanline;
                dst_n_band  = dst_n_scanline;
            }

            if(!odd_lines) {
                if(odd_start_pixel) {
                    for(unsigned int band_num=0; band_num<nbands; band_num++) {
                        unsigned int  interp0 =
                            (unsigned int)(*src_band +
                                           *(src_band+src_nx_pixel))>>1;
                        unsigned int  interp1 =
                            (unsigned int)(*dst_p_band + interp0)>>1;
                    
                        *dst_band++   = interp1;

                        dst_p_band++;
                        src_band++;
                    }

                    dst_p_pixel += dst_nx_pixel;
                    dst_pixel   += dst_nx_pixel;
                    src_pixel   += src_nx_pixel;

                    src_band   = src_pixel;
                    dst_band   = dst_pixel;
                    dst_p_band = dst_p_pixel;
                }
                
                for(unsigned int j=0; j<dstR_xsize/4; j++) {
                    for(unsigned int band_num=0; band_num<nbands; band_num++) {
                        //
                        //  Calculate the interpolation points for band 0 and
                        //   set the corresponding eight pixels in
                        //   the destination.
                        //
                        unsigned int  interp0 = (unsigned int)
                            (*src_band + *(src_band+src_nx_pixel))>>1;
                        unsigned int  interp1 = (unsigned int)
                            (*dst_p_band + *src_band)>>1;
                        unsigned int  interp2 = (unsigned int)
                            (*(dst_p_band+dst_nx_pixel2) +
                             *(src_band+src_nx_pixel))>>1;
                        unsigned int  interp3 = (unsigned int)
                            (*(dst_p_band+dst_nx_pixel) + interp0)>>1;
                        unsigned int  interp4 = (unsigned int)
                            (*(src_band+src_nx_pixel) +
                             *(src_band+src_nx_pixel2))>>1;
                        unsigned int  interp5 = (unsigned int)
                            (*(dst_p_band+dst_nx_pixel3) + interp4)>>1;
                        
                        *dst_band                   = interp1;
                        *(dst_band+dst_nx_pixel)    = interp3;
                        *(dst_band+dst_nx_pixel2)   = interp2;
                        *(dst_band+dst_nx_pixel3)   = interp5;
                        
                        dst_band++;
                        src_band++;
                        dst_p_band++;
                    }
                    dst_p_pixel += dst_nx_pixel4;
                    dst_pixel   += dst_nx_pixel4;
                    src_pixel   += src_nx_pixel2;
                    
                    src_band   = src_pixel;
                    dst_band   = dst_pixel;
                    dst_p_band = dst_p_pixel;
                }
                
                for(unsigned int band_num=0; band_num<nbands; band_num++) {
                    switch(end_pixels) {
                        case 3:
                        {
                            //
                            //  Calculate the interpolation points for
                            //   band 0 and set the corresponding
                            //   pixels in the destination. 
                            //
                            unsigned int  interp0 = (unsigned int)
                                (*src_band + *(src_band+src_nx_pixel))>>1;
                            unsigned int  interp1 = (unsigned int)
                                (*dst_p_band + *src_band)>>1;
                            unsigned int  interp2 = (unsigned int)
                                (*(dst_p_band+dst_nx_pixel2) +
                                 *(src_band+src_nx_pixel))>>1;
                            unsigned int  interp3 = (unsigned int)
                                (*(dst_p_band+dst_nx_pixel) + interp0)>>1;
                            
                            *dst_band                   = interp1;
                            *(dst_band+dst_nx_pixel)    = interp3;
                            *(dst_band+dst_nx_pixel2)   = interp2;
                        }
                        break;
                        
                        case 2:
                        {
                            //
                            //  Calculate the interpolation points for
                            //   band 0 and set the corresponding
                            //   pixels in the destination. 
                            //
                            unsigned int  interp0 = (unsigned int)
                                (*src_band + *(src_band+src_nx_pixel))>>1;
                            unsigned int  interp1 = (unsigned int)
                                (*dst_p_band + *src_band)>>1;
                            unsigned int  interp3 = (unsigned int)
                                (*(dst_p_band+dst_nx_pixel) + interp0)>>1;
                            
                            *dst_band                   = interp1;
                            *(dst_band+dst_nx_pixel)    = interp3;
                        }
                        break;
                        
                        case 1:
                        {
                            //
                            //  Calculate the interpolation points for
                            //   band 0 and set the corresponding
                            //   pixels in the destination. 
                            //
                            unsigned int  interp1 = (unsigned int)
                                (*dst_p_band + *src_band)>>1;
                            
                            *dst_band                   = interp1;
                        }
                        break;
                    }
                    
                    src_band++;
                    dst_band++;
                    dst_p_band++;
                }
            }
            delete line_buffer;
        }
    }
    
    return XIL_SUCCESS;
}

//
// For general storage just use the general N-band implementatino from above.
//
static XilStatus scale_zoom2x_general_storage_BL(AffineData affine_data)
{
    XilOp*       op            = affine_data.op;
    XilStorage*  src_storage   = affine_data.src_storage;
    XilStorage*  dst_storage   = affine_data.dst_storage;
    XilBox*      src_box       = affine_data.src_box;
    XilBox*      dst_box       = affine_data.dst_box;
    XilRoi*      roi           = affine_data.roi;
    unsigned int nbands        = affine_data.nbands;

    //
    // Get the image coordinates of the source box for later use.
    //
    int src_box_x0_image;
    int src_box_y0_image;
    {
        unsigned int src_box_size_x;
        unsigned int src_box_size_y;
        src_box->getAsRect(&src_box_x0_image,
                           &src_box_y0_image,
                           &src_box_size_x,
                           &src_box_size_y);
    }

    //
    // Retrieve the source image width and height for later use.
    //
    int src_xsize;
    int src_ysize;
    {
        XilImage* src = src_storage->getImage();
        src_xsize = src->getWidth();
        src_ysize = src->getHeight();
    }

    //
    // Create a rectangle list that is an intersection of
    // the intersected roi and dst_box. The list is
    // returned in the dst_box coordinate space (not
    // in the dst image coordinate space).
    //
    XilRectList rl(roi, dst_box);

    //
    //  Loop over the rectangle list
    //
    int dstR_x;
    int dstR_y;
    unsigned int dstR_xsize;
    unsigned int dstR_ysize;
    while(rl.getNext(&dstR_x, &dstR_y, &dstR_xsize, &dstR_ysize)) {
        int num_bands = 1; // TODO  bpb  02/13/1997  Remove this jerry rigging
        for(unsigned int bandn = 0; bandn < nbands; bandn++) {
            //
            // Get the storage information
            //
            Xil_unsigned8* src_base_addr =
                (Xil_unsigned8*) src_storage->getDataPtr(bandn);
            unsigned int   src_nx_pixel = src_storage->getPixelStride(bandn);
            unsigned int   src_nx_scan = src_storage->getScanlineStride(bandn);

            Xil_unsigned8* dst_base_addr =
                (Xil_unsigned8*) dst_storage->getDataPtr(bandn);
            unsigned int   dst_nx_pixel = dst_storage->getPixelStride(bandn);
            unsigned int   dst_nx_scan = dst_storage->getScanlineStride(bandn);

            //
            //  The starting point in the destination.
            //
            Xil_unsigned8* dst_scanline = dst_base_addr +
                (dstR_y * dst_nx_scan) +
                (dstR_x * dst_nx_pixel);
            Xil_unsigned8* dst_pixel = dst_scanline;
            Xil_unsigned8* dst_band  = dst_scanline;

            //
            // Backward map the upper left destination rectangle corner.
            // This mapping includes the effect of src and dst origins.
            //
            double sx;
            double sy;
            op->backwardMap(dst_box, (double)dstR_x, (double)dstR_y,
                            src_box, &sx, &sy);

            //
            //  Since we do the first line seperately and then do a
            //   two-line algorithm there is either 1 or 2 lines left
            //   depending upon whether we're odd or even.
            //
            //  This variable is set to 1 if it's odd which means we
            //   add an extra 1 to our loop to complete the last lines.
            //  
            unsigned int  odd_start_line =
                (_XILI_ROUND(sy) == _XILI_ROUND(sy + 0.5F)) ? 1 : 0;
            unsigned int  odd_start_pixel =
                (_XILI_ROUND(sx) == _XILI_ROUND(sx + 0.5F)) ? 1 : 0;
            int odd_lines = dstR_ysize % 2;
            int end_pixels = dstR_xsize % 4;
            Xil_unsigned8* line_buffer  = NULL;
        
            int dstR_ysize_2 = dstR_ysize/2 + odd_lines;
            int dstR_xsize_4 = dstR_xsize/4;

            //
            //  Check for edge conditions (in image coordinate space)
            //
            {
                //
                // Backward map the lower right corner of the dst rectangle.
                // The results are in src_box coordinate space.
                //
                double src_x;
                double src_y;
                op->backwardMap(dst_box,
                                (double)(dstR_x + (int)dstR_xsize - 1),
                                (double)(dstR_y + (int)dstR_ysize - 1),
                                src_box,
                                &src_x,
                                &src_y);

                //
                // Check the edge conditions in src image space.
                //
                if((src_x + src_box_x0_image) > (double)(src_xsize - 1)) {
                    dstR_xsize_4--;
                    end_pixels = 4;
                }

                if((src_y + src_box_y0_image) > (double)(src_ysize - 1)) {
                    dstR_ysize -= 1;
                    odd_lines = 1; // this forces it not to touch bottom line
                }
            }
            
            //
            //  The starting point in the source.
            //
	    if (sx < 0 && odd_start_pixel)
		sx -= 1.0;
	    if (sy < 0 && odd_start_line)
		sy -= 1.0;
            Xil_unsigned8* src_scanline = src_base_addr + 
                ((int)sy * src_nx_scan) +
                ((int)sx * src_nx_pixel);
            Xil_unsigned8* src_pixel = src_scanline;
            Xil_unsigned8* src_band  = src_scanline;

            //
            //  Pre-compute the increments...
            //
            unsigned int   dst_nx_pixel4 = dst_nx_pixel<<2; 
            unsigned int   dst_nx_pixel3 = (dst_nx_pixel<<1) + dst_nx_pixel; 
            unsigned int   dst_nx_pixel2 = dst_nx_pixel<<1; 
            unsigned int   src_nx_pixel2 = src_nx_pixel<<1;
            
            //
            //  If we're starting on an odd line, then we must
            //   allocate our own buffer.
            //
            if(odd_start_line) {
                odd_lines = !odd_lines;
                dstR_ysize_2 += odd_lines;
                
                line_buffer = new Xil_unsigned8[dstR_xsize*dst_nx_pixel];
                dst_band = dst_pixel = line_buffer;
            }
            
            if(odd_start_pixel) {
                for(int b=0; b<num_bands; b++) {
                    unsigned int  interp0 =
                        (unsigned int)(*src_band +
                                       *(src_band+src_nx_pixel))>>1;
                    
                    *dst_band++   = interp0;

                    src_band++;
                }

                dst_pixel   += dst_nx_pixel;
                src_pixel   += src_nx_pixel;
                
                src_band   = src_pixel;
                dst_band   = dst_pixel;

                if(end_pixels>0) {
                    end_pixels--;
                } else {
                    end_pixels = 3;
                    dstR_xsize_4--;
                }
            }

            //
            //  Special case the first scanline.  I compute this and then
            //  do a more general two-line algorithm for the rest of the image.
            //
            int i, b;
            for(i=0; i<dstR_xsize_4; i++) {
                for(b=0; b<num_bands; b++) {
                    //
                    //  Calculate the interpolation points and set
                    //   the corresponding four pixels in the destination.
                    //
                    int  interp0 =
                        (int)(*src_band + *(src_band+src_nx_pixel))>>1;
                    int  interp1 =
                        (int)(*(src_band+src_nx_pixel) +
                              *(src_band+src_nx_pixel2))>>1;
                
                    *dst_band                  = *src_band;
                    *(dst_band+dst_nx_pixel)   = interp0;
                    *(dst_band+dst_nx_pixel2)  = *(src_band+src_nx_pixel);
                    *(dst_band+dst_nx_pixel3)  = interp1;

                    dst_band++;
                    src_band++;
                }
                
                dst_pixel += dst_nx_pixel4;
                src_pixel += src_nx_pixel2;
                
                src_band = src_pixel;
                dst_band = dst_pixel;
            }

            for(b=0; b<num_bands; b++) {
                switch(end_pixels) {
                    case 4:
                    case 3:
                    {
                        //
                        //  Calculate the interpolation points and set
                        //   the corresponding three pixels in the destination.
                        //
                        int interp0 =
                            (int)(*src_band + *(src_band+src_nx_pixel))>>1;
                        
                        *dst_band                 = *src_band;
                        *(dst_band+dst_nx_pixel)  = interp0;
                        *(dst_band+dst_nx_pixel2) = *(src_band+src_nx_pixel);
                    }
                    break;
                
                    case 2:
                    {
                        //
                        //  Calculate the interpolation points and set
                        //   the corresponding two pixels in the destination.
                        //
                        int  interp0 =
                            (int)(*src_band + *(src_band+src_nx_pixel))>>1;
                        
                        *dst_band                 = *src_band;
                        *(dst_band+dst_nx_pixel)  = interp0;
                    }                    
                    break;
                    
                    case 1:
                    *dst_band      = *src_band;
                    break;
                }

                dst_band++;
                src_band++;
            }
            
            //
            //  Initialize the variables which we'll use to move through
            //   the image.
            //
            //  Here I set a pointer to the scanline we just computed.
            //
            Xil_unsigned8* dst_p_scanline = dst_scanline;
            Xil_unsigned8* dst_p_pixel    = dst_scanline;
            Xil_unsigned8* dst_p_band     = dst_scanline;
                
            //
            //  The destination is incremented by two since we're doing
            //   two lines of the destination for every line of the
            //   source.  This saves an add per scanline.
            //
            unsigned int  dst_nx_scan2 = dst_nx_scan + dst_nx_scan;
            
            //
            //  Only increment each by one scanline this time since we've
            //   only completed a single scanline in the destination.
            //
            if(odd_start_line) {
                dst_p_band = dst_p_pixel = dst_p_scanline = line_buffer;
                dst_band   = dst_pixel   = dst_scanline;
            } else {
                dst_band   = dst_pixel   = dst_scanline += dst_nx_scan;
            }
            src_band = src_pixel = src_scanline += src_nx_scan;
            
            Xil_unsigned8* dst_n_scanline = dst_scanline + dst_nx_scan;
            Xil_unsigned8* dst_n_pixel    = dst_n_scanline;
            Xil_unsigned8* dst_n_band     = dst_n_scanline;
            
            for(i=1; i<dstR_ysize_2; i++) {
                if(odd_start_pixel) {
                    for(int band_num=0; band_num<num_bands; band_num++) {
                        unsigned int  interp0 =
                            (unsigned int)(*src_band +
                                           *(src_band+src_nx_pixel))>>1;
                        unsigned int  interp1 =
                            (unsigned int)(*dst_p_band + interp0)>>1;
                    
                        *dst_band++   = interp1;
                        *dst_n_band++ = interp0;
                        dst_p_band++;
                        src_band++;
                    }

                    dst_p_pixel += dst_nx_pixel;
                    dst_n_pixel += dst_nx_pixel;
                    dst_pixel   += dst_nx_pixel;
                    src_pixel   += src_nx_pixel;

                    src_band   = src_pixel;
                    dst_band   = dst_pixel;
                    dst_n_band = dst_n_pixel;
                    dst_p_band = dst_p_pixel;
                }
                
                for(int j=0; j<dstR_xsize_4; j++) {
                    for(int band_num=0; band_num<num_bands; band_num++) {
                        //
                        //  Below is the layout of the interpolated values.
                        //   - The *'s refer to the previous scanline in the
                        //       destination.
                        //   - The numbers correspond to the different
                        //       interpolated variables. 
                        //   - The s corresponds to a pixel in the source
                        //       image.
                        //
                        //  dst:   *  *  *  *
                        //         1  3  2  5
                        //         s  0  s  4
                        //
                        
                        //
                        //  Calculate the interpolation points for band 0 and
                        //   set the corresponding eight pixels in
                        //   the destination.
                        //
                        unsigned int  interp0 = (unsigned int)
                            (*src_band + *(src_band+src_nx_pixel))>>1;
                        unsigned int  interp1 = (unsigned int)
                            (*dst_p_band + *src_band)>>1;
                        unsigned int  interp2 = (unsigned int)
                            (*(dst_p_band+dst_nx_pixel2) +
                             *(src_band+src_nx_pixel))>>1;
                        unsigned int  interp3 = (unsigned int)
                            (*(dst_p_band+dst_nx_pixel) + interp0)>>1;
                        unsigned int  interp4 = (unsigned int)
                            (*(src_band+src_nx_pixel) +
                             *(src_band+src_nx_pixel2))>>1;
                        unsigned int  interp5 = (unsigned int)
                            (*(dst_p_band+dst_nx_pixel3) + interp4)>>1;
                        
                        *dst_band                   = interp1;
                        *(dst_band+dst_nx_pixel)    = interp3;
                        *(dst_band+dst_nx_pixel2)   = interp2;
                        *(dst_band+dst_nx_pixel3)   = interp5;
                        
                        *dst_n_band                 = *src_band;
                        *(dst_n_band+dst_nx_pixel)  = interp0;
                        *(dst_n_band+dst_nx_pixel2) = *(src_band+src_nx_pixel);
                        *(dst_n_band+dst_nx_pixel3) = interp4;

                        src_band++;
                        dst_n_band++;
                        dst_band++;
                        dst_p_band++;
                    }
                    dst_p_pixel += dst_nx_pixel4;
                    dst_n_pixel += dst_nx_pixel4;
                    dst_pixel   += dst_nx_pixel4;
                    src_pixel   += src_nx_pixel2;

                    src_band   = src_pixel;
                    dst_band   = dst_pixel;
                    dst_n_band = dst_n_pixel;
                    dst_p_band = dst_p_pixel;
                }

                for(int band_num=0; band_num<num_bands; band_num++) {
                    switch(end_pixels) {
                        case 4:
                        {
                            //
                            //  Calculate the interpolation points for
                            //   band 0 and set the corresponding
                            //   pixels in the destination. 
                            //
                            unsigned int  interp0 = (unsigned int)
                                (*src_band + *(src_band+src_nx_pixel))>>1;
                            unsigned int  interp1 = (unsigned int)
                                (*dst_p_band + *src_band)>>1;
                            unsigned int  interp2 = (unsigned int)
                                (*(dst_p_band+dst_nx_pixel2) +
                                 *(src_band+src_nx_pixel))>>1;
                            unsigned int  interp3 = (unsigned int)
                                (*(dst_p_band+dst_nx_pixel) + interp0)>>1;
                            
                            *dst_band                   = interp1;
                            *(dst_band+dst_nx_pixel)    = interp3;
                            *(dst_band+dst_nx_pixel2)   = interp2;
                            *(dst_band+dst_nx_pixel3)   = interp2;
                            
                            *dst_n_band                 = *src_band;
                            *(dst_n_band+dst_nx_pixel)  = interp0;
                            *(dst_n_band+dst_nx_pixel2) =
                                *(src_band+src_nx_pixel);
                            *(dst_n_band+dst_nx_pixel3) =
                                *(src_band+src_nx_pixel);
                        }
                        break;
                        
                        case 3:
                        {
                            //
                            //  Calculate the interpolation points for
                            //   band 0 and set the corresponding
                            //   pixels in the destination. 
                            //
                            unsigned int  interp0 = (unsigned int)
                                (*src_band + *(src_band+src_nx_pixel))>>1;
                            unsigned int  interp1 = (unsigned int)
                                (*dst_p_band + *src_band)>>1;
                            unsigned int  interp2 = (unsigned int)
                                (*(dst_p_band+dst_nx_pixel2) +
                                 *(src_band+src_nx_pixel))>>1;
                            unsigned int  interp3 = (unsigned int)
                                (*(dst_p_band+dst_nx_pixel) + interp0)>>1;
                            
                            *dst_band                   = interp1;
                            *(dst_band+dst_nx_pixel)    = interp3;
                            *(dst_band+dst_nx_pixel2)   = interp2;
                            
                            *dst_n_band                 = *src_band;
                            *(dst_n_band+dst_nx_pixel)  = interp0;
                            *(dst_n_band+dst_nx_pixel2) =
                                *(src_band+src_nx_pixel);
                        }
                        break;
                        
                        case 2:
                        {
                            //
                            //  Calculate the interpolation points for
                            //   band 0 and set the corresponding
                            //   pixels in the destination. 
                            //
                            unsigned int  interp0 = (unsigned int)
                                (*src_band + *(src_band+src_nx_pixel))>>1;
                            unsigned int  interp1 = (unsigned int)
                                (*dst_p_band + *src_band)>>1;
                            unsigned int  interp3 = (unsigned int)
                                (*(dst_p_band+dst_nx_pixel) + interp0)>>1;
                            
                            *dst_band                   = interp1;
                            *(dst_band+dst_nx_pixel)    = interp3;
                            
                            *dst_n_band                 = *src_band;
                            *(dst_n_band+dst_nx_pixel)  = interp0;
                        }
                        break;
                        
                        case 1:
                        {
                            //
                            //  Calculate the interpolation points for
                            //   band 0 and set the corresponding
                            //   pixels in the destination. 
                            //
                            unsigned int  interp1 = (unsigned int)
                                (*dst_p_band + *src_band)>>1;
                            
                            *dst_band                   = interp1;
                            
                            *dst_n_band                 = *src_band;
                        }
                        break;
                    }
                    
                    src_band++;
                    dst_band++;
                    dst_n_band++;
                    dst_p_band++;
                }
                
                //
                //  Reset everything back to the beginning of the
                //   next scanline.
                //
                src_scanline += src_nx_scan;
                src_pixel = src_scanline;
                src_band  = src_scanline;
                
                dst_p_scanline += dst_nx_scan2;
                dst_p_pixel = dst_n_scanline;
                dst_p_band = dst_n_scanline;
                
                dst_scanline += dst_nx_scan2;
                dst_pixel = dst_scanline;
                dst_band  = dst_scanline;
                
                dst_n_scanline += dst_nx_scan2;
                dst_n_pixel = dst_n_scanline;
                dst_n_band  = dst_n_scanline;
            }

            if(!odd_lines) {
                if(odd_start_pixel) {
                    for(int band_num=0; band_num<num_bands; band_num++) {
                        unsigned int  interp0 =
                            (unsigned int)(*src_band +
                                           *(src_band+src_nx_pixel))>>1;
                        unsigned int  interp1 =
                            (unsigned int)(*dst_p_band + interp0)>>1;
                    
                        *dst_band++   = interp1;

                        dst_p_band++;
                        src_band++;
                    }

                    dst_p_pixel += dst_nx_pixel;
                    dst_pixel   += dst_nx_pixel;
                    src_pixel   += src_nx_pixel;

                    src_band   = src_pixel;
                    dst_band   = dst_pixel;
                    dst_p_band = dst_p_pixel;
                }
                
                for(unsigned int j=0; j<dstR_xsize/4; j++) {
                    for(int band_num=0; band_num<num_bands; band_num++) {
                        //
                        //  Calculate the interpolation points for band 0 and
                        //   set the corresponding eight pixels in
                        //   the destination.
                        //
                        unsigned int  interp0 = (unsigned int)
                            (*src_band + *(src_band+src_nx_pixel))>>1;
                        unsigned int  interp1 = (unsigned int)
                            (*dst_p_band + *src_band)>>1;
                        unsigned int  interp2 = (unsigned int)
                            (*(dst_p_band+dst_nx_pixel2) +
                             *(src_band+src_nx_pixel))>>1;
                        unsigned int  interp3 = (unsigned int)
                            (*(dst_p_band+dst_nx_pixel) + interp0)>>1;
                        unsigned int  interp4 = (unsigned int)
                            (*(src_band+src_nx_pixel) +
                             *(src_band+src_nx_pixel2))>>1;
                        unsigned int  interp5 = (unsigned int)
                            (*(dst_p_band+dst_nx_pixel3) + interp4)>>1;
                        
                        *dst_band                   = interp1;
                        *(dst_band+dst_nx_pixel)    = interp3;
                        *(dst_band+dst_nx_pixel2)   = interp2;
                        *(dst_band+dst_nx_pixel3)   = interp5;
                        
                        dst_band++;
                        src_band++;
                        dst_p_band++;
                    }
                    dst_p_pixel += dst_nx_pixel4;
                    dst_pixel   += dst_nx_pixel4;
                    src_pixel   += src_nx_pixel2;
                    
                    src_band   = src_pixel;
                    dst_band   = dst_pixel;
                    dst_p_band = dst_p_pixel;
                }
                
                for(int band_num=0; band_num<num_bands; band_num++) {
                    switch(end_pixels) {
                        case 3:
                        {
                            //
                            //  Calculate the interpolation points for
                            //   band 0 and set the corresponding
                            //   pixels in the destination. 
                            //
                            unsigned int  interp0 = (unsigned int)
                                (*src_band + *(src_band+src_nx_pixel))>>1;
                            unsigned int  interp1 = (unsigned int)
                                (*dst_p_band + *src_band)>>1;
                            unsigned int  interp2 = (unsigned int)
                                (*(dst_p_band+dst_nx_pixel2) +
                                 *(src_band+src_nx_pixel))>>1;
                            unsigned int  interp3 = (unsigned int)
                                (*(dst_p_band+dst_nx_pixel) + interp0)>>1;
                            
                            *dst_band                   = interp1;
                            *(dst_band+dst_nx_pixel)    = interp3;
                            *(dst_band+dst_nx_pixel2)   = interp2;
                        }
                        break;
                        
                        case 2:
                        {
                            //
                            //  Calculate the interpolation points for
                            //   band 0 and set the corresponding
                            //   pixels in the destination. 
                            //
                            unsigned int  interp0 = (unsigned int)
                                (*src_band + *(src_band+src_nx_pixel))>>1;
                            unsigned int  interp1 = (unsigned int)
                                (*dst_p_band + *src_band)>>1;
                            unsigned int  interp3 = (unsigned int)
                                (*(dst_p_band+dst_nx_pixel) + interp0)>>1;
                            
                            *dst_band                   = interp1;
                            *(dst_band+dst_nx_pixel)    = interp3;
                        }
                        break;
                        
                        case 1:
                        {
                            //
                            //  Calculate the interpolation points for
                            //   band 0 and set the corresponding
                            //   pixels in the destination. 
                            //
                            unsigned int  interp1 = (unsigned int)
                                (*dst_p_band + *src_band)>>1;
                            
                            *dst_band                   = interp1;
                        }
                        break;
                    }
                    
                    src_band++;
                    dst_band++;
                    dst_p_band++;
                }
            }
            delete line_buffer;
        } // band for loop
    } // rect list while loop

    return XIL_SUCCESS;
}

#ifdef SUBSAMPLE2X_ENABLED
static XilStatus scale_subsample2x_pixel_sequential_BL(AffineData affine_data)
{
    XilStorage*  src_storage   = affine_data.src_storage;
    XilStorage*  dst_storage   = affine_data.dst_storage;
    XilBox*      dst_box       = affine_data.dst_box;
    XilRoi*      roi           = affine_data.roi;
    unsigned int nbands        = affine_data.nbands;

    //
    // Get the storage information
    //
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
    // Set up some variables used in the accelerated algorithm
    //
    Xil_unsigned8* src_base_addr = src_data;
    unsigned int   src_nx_pixel = src_pstride;
    unsigned int   src_nx_scan = src_sstride;

    Xil_unsigned8* dst_base_addr = dst_data;
    unsigned int   dst_nx_pixel = dst_pstride;
    unsigned int   dst_nx_scan = dst_sstride;

    //
    // TODO  bpb  02/13/97  Delete the following variables if possible
    //
    int src_xorig = 0, src_yorig = 0;
    int dst_xorig = 0, dst_yorig = 0;
    XilImage* src = src_storage->getImage();
    int src_xsize = src->getWidth();
    int src_ysize = src->getHeight();
    XilImage* dst = dst_storage->getImage();
    int dst_xsize = dst->getWidth();
    int dst_ysize = dst->getHeight();

    //
    // Create a rectangle list that is an intersection of
    // the intersected roi and dst_box. The list is
    // returned in the dst_box coordinate space (not
    // in the dst image coordinate space).
    //
    XilRectList rl(roi, dst_box);

    //
    //  Well, special case city here.  Feel free to add to the list as
    //    you want more performance...
    //
    //  1)  1 BAND -> 1 BAND (no band children)
    //  2)  3 BAND -> 3 BAND (no band children)
    //  3   N BAND -> N BAND (everything)
    //
    
    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    //
    //      1 CONTIGUOUS BAND OPTIMIZED CASE
    //
    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    if(nbands == 1 && src_nx_pixel == 1 && dst_nx_pixel == 1) {
        //
        //  Loop over the rectangle list
        //
        int          dstR_x, dstR_y;
        unsigned int dstR_xsize, dstR_ysize;
        while(rl.getNext(&dstR_x, &dstR_y, &dstR_xsize, &dstR_ysize)) {

            //
            //  The starting point in the destination.
            //
            Xil_unsigned8* dst_scanline = dst_base_addr +
                ((dstR_y + dst_yorig) * dst_nx_scan) +
                ((dstR_x + dst_xorig) * dst_nx_pixel);
            Xil_unsigned8* dst_pixel = dst_scanline;

            //
            //  The starting point in the source.
            //
            double sx = (double)dstR_x * 2.0;
            double sy = (double)dstR_y * 2.0;
            Xil_unsigned8* src_scanline = src_base_addr + 
                (((int)sy + src_yorig) * src_nx_scan) +
                (((int)sx + src_xorig) * src_nx_pixel);
            Xil_unsigned8* src_pixel = src_scanline;

            //
            //  Initialize the variables which we'll use to move through
            //   the image.
            //
            //  The source is incremented by two since we're doing
            //   two lines of the source for every line of the
            //   destination.  This saves an add per scanline.
            //
            unsigned int   src_nx_scan2 = src_nx_scan + src_nx_scan;
            
            //
            //  Here I set a pointer to the scanline we just computed.
            //
            Xil_unsigned8* src_n_scanline = src_scanline + src_nx_scan;
            Xil_unsigned8* src_n_pixel    = src_n_scanline;

            do {
                //
                //  Handle non word-aligned destination
                //
                unsigned int dstR_xbegin = 0;
                unsigned int tmp_xsize = dstR_xsize;
                while(((unsigned long)dst_pixel) & 0x3 &&
                      tmp_xsize--) {
                    *dst_pixel++ = ((unsigned int)
                        (*src_pixel     + *src_n_pixel     +
                         *(src_pixel+1) + *(src_n_pixel+1)))>>2;
                    
                    src_pixel  += 2;
                    src_n_pixel+= 2;

                    dstR_xbegin++;
                }
                
                unsigned int dstR_xsize_4  = (dstR_xsize - dstR_xbegin)>>2;
                unsigned int dstR_xend     =
                    dstR_xsize - (dstR_xsize_4<<2) - dstR_xbegin;
                
                while(dstR_xsize_4--) {
                    //
                    //  Calculate the interpolation points for band 0 and
                    //   set the corresponding pixels in the destination.
                    //
                    unsigned int  interp1 = ((unsigned int)
                        (*src_pixel     + *src_n_pixel     +
                         *(src_pixel+1) + *(src_n_pixel+1)))>>2;

                    unsigned int  interp2 = ((unsigned int)
                        (*(src_pixel+2) + *(src_n_pixel+2) +
                         *(src_pixel+3) + *(src_n_pixel+3)))>>2;

                    unsigned int  interp3 = ((unsigned int)
                        (*(src_pixel+4) + *(src_n_pixel+4) +
                         *(src_pixel+5) + *(src_n_pixel+5)))>>2;

                    unsigned int  interp4 = ((unsigned int)
                        (*(src_pixel+6) + *(src_n_pixel+6) +
                         *(src_pixel+7) + *(src_n_pixel+7)))>>2;

#ifndef XIL_LITTLE_ENDIAN
                    *((unsigned int*)dst_pixel) =
                         interp1<<24 |
                         interp2<<16 |
                         interp3<<8  |
                         interp4;
#else
                    *((unsigned int*)dst_pixel) =
                         interp1     |
                         interp2<<8  |
                         interp3<<16 |
                         interp4<<24;
#endif

                    dst_pixel  += 4;
                    src_pixel  += 8;
                    src_n_pixel+= 8;
                }

                //
                //  Any left over pixels...
                //
                while(dstR_xend--) {
                    *dst_pixel++ = ((unsigned int)
                        (*src_pixel     + *src_n_pixel     +
                         *(src_pixel+1) + *(src_n_pixel+1)))>>2;
                    
                    src_pixel  += 2;
                    src_n_pixel+= 2;
                }
                
                //
                //  Reset everything back to the beginning of the
                //   next scanline.
                //
                src_scanline   += src_nx_scan2;
                src_pixel       = src_scanline;

                src_n_scanline += src_nx_scan2;
                src_n_pixel     = src_n_scanline;

                dst_scanline   += dst_nx_scan;
                dst_pixel       = dst_scanline;
            } while(--dstR_ysize);
        }
    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    //
    //      3 CONTIGUOUS BAND OPTIMIZED CASE
    //
    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    } else if(nbands == 3 && src_nx_pixel == 3 && dst_nx_pixel == 3) {
        //
        //  Loop over the rectangle list
        //
        int          dstR_x, dstR_y;
        unsigned int dstR_xsize, dstR_ysize;
        while(rl.getNext(&dstR_x, &dstR_y, &dstR_xsize, &dstR_ysize)) {
            //
            //  The starting point in the destination.
            //
            Xil_unsigned8* dst_scanline = dst_base_addr +
                ((dstR_y + dst_yorig) * dst_nx_scan) +
                ((dstR_x + dst_xorig) * dst_nx_pixel);
            Xil_unsigned8* dst_pixel = dst_scanline;

            //
            //  The starting point in the source.
            //
            double sx = (double)dstR_x * 2.0;
            double sy = (double)dstR_y * 2.0;
            Xil_unsigned8* src_scanline = src_base_addr + 
                (((int)sy + src_yorig) * src_nx_scan) +
                (((int)sx + src_xorig) * src_nx_pixel);
            Xil_unsigned8* src_pixel = src_scanline;

            //
            //  Initialize the variables which we'll use to move through
            //   the image.
            //
            //  The source is incremented by two since we're doing
            //   two lines of the source for every line of the
            //   destination.  This saves an add per scanline.
            //
            unsigned int   src_nx_scan2 = src_nx_scan + src_nx_scan;
            
            //
            //  Here I set a pointer to the scanline we just computed.
            //
            Xil_unsigned8* src_n_scanline = src_scanline + src_nx_scan;
            Xil_unsigned8* src_n_pixel    = src_n_scanline;

            do {
                //
                //  Handle non word-aligned destination
                //
                unsigned int dstR_xbegin = 0;
                unsigned int tmp_xsize = dstR_xsize;
                while(((unsigned long)dst_pixel) & 0x3 &&
                      tmp_xsize--) {
                    *dst_pixel++ = ((unsigned int)
                        (*src_pixel     + *src_n_pixel     +
                         *(src_pixel+3) + *(src_n_pixel+3)))>>2;
                    
                    *dst_pixel++ = ((unsigned int)
                        (*(src_pixel+1) + *(src_n_pixel+1) +
                         *(src_pixel+4) + *(src_n_pixel+4)))>>2;
                    
                    *dst_pixel++ = ((unsigned int)
                        (*(src_pixel+2) + *(src_n_pixel+2) +
                         *(src_pixel+5) + *(src_n_pixel+5)))>>2;
                    
                    src_pixel  += 6;
                    src_n_pixel+= 6;

                    dstR_xbegin++;
                }
                
                unsigned int dstR_xsize_4  = (dstR_xsize - dstR_xbegin)>>2;
                unsigned int dstR_xend     =
                    dstR_xsize - (dstR_xsize_4<<2) - dstR_xbegin;

                while(dstR_xsize_4--) {
                    //
                    //  Calculate the interpolation points for band 0 and
                    //   set the corresponding pixels in the destination.
                    //
                    unsigned int  interp1_0 = ((unsigned int)
                        (*src_pixel     + *src_n_pixel     +
                         *(src_pixel+3) + *(src_n_pixel+3)))>>2;

                    unsigned int  interp1_1 = ((unsigned int)
                        (*(src_pixel+1) + *(src_n_pixel+1) +
                         *(src_pixel+4) + *(src_n_pixel+4)))>>2;

                    unsigned int  interp1_2 = ((unsigned int)
                        (*(src_pixel+2) + *(src_n_pixel+2) +
                         *(src_pixel+5) + *(src_n_pixel+5)))>>2;

                    unsigned int  interp2_0 = ((unsigned int)
                        (*(src_pixel+6) + *(src_n_pixel+6) +
                         *(src_pixel+9) + *(src_n_pixel+9)))>>2;

#ifndef XIL_LITTLE_ENDIAN
                    *((unsigned int*)dst_pixel) =
                         interp1_0<<24 |
                         interp1_1<<16 |
                         interp1_2<<8  |
                         interp2_0;
#else
                    *((unsigned int*)dst_pixel) =
                         interp1_0     |
                         interp1_1<<8  |
                         interp1_2<<16 |
                         interp2_0<<24;
#endif
                    dst_pixel += 4;
                    
                    unsigned int  interp2_1 = ((unsigned int)
                        (*(src_pixel+7)  + *(src_n_pixel+7) +
                         *(src_pixel+10) + *(src_n_pixel+10)))>>2;

                    unsigned int  interp2_2 = ((unsigned int)
                        (*(src_pixel+8) + *(src_n_pixel+8) +
                         *(src_pixel+11) + *(src_n_pixel+11)))>>2;

                    unsigned int  interp3_0 = ((unsigned int)
                        (*(src_pixel+12) + *(src_n_pixel+12) +
                         *(src_pixel+15) + *(src_n_pixel+15)))>>2;

                    unsigned int  interp3_1 = ((unsigned int)
                        (*(src_pixel+13) + *(src_n_pixel+13) +
                         *(src_pixel+16) + *(src_n_pixel+16)))>>2;


#ifndef XIL_LITTLE_ENDIAN
                    *((unsigned int*)dst_pixel) =
                         interp2_1<<24 |
                         interp2_2<<16 |
                         interp3_0<<8  |
                         interp3_1;
#else
                    *((unsigned int*)dst_pixel) =
                         interp2_1     |
                         interp2_2<<8  |
                         interp3_0<<16 |
                         interp3_1<<24;
#endif
                    dst_pixel += 4;
                    
                    unsigned int  interp3_2 = ((unsigned int)
                        (*(src_pixel+14) + *(src_n_pixel+14) +
                         *(src_pixel+17) + *(src_n_pixel+17)))>>2;

                    unsigned int  interp4_0 = ((unsigned int)
                        (*(src_pixel+18) + *(src_n_pixel+18) +
                         *(src_pixel+21) + *(src_n_pixel+21)))>>2;

                    unsigned int  interp4_1 = ((unsigned int)
                        (*(src_pixel+19) + *(src_n_pixel+19) +
                         *(src_pixel+22) + *(src_n_pixel+22)))>>2;

                    unsigned int  interp4_2 = ((unsigned int)
                        (*(src_pixel+20) + *(src_n_pixel+20) +
                         *(src_pixel+23) + *(src_n_pixel+23)))>>2;

#ifndef XIL_LITTLE_ENDIAN
                    *((unsigned int*)dst_pixel) =
                         interp3_2<<24 |
                         interp4_0<<16 |
                         interp4_1<<8  |
                         interp4_2;
#else
                    *((unsigned int*)dst_pixel) =
                         interp3_2     |
                         interp4_0<<8  |
                         interp4_1<<16 |
                         interp4_2<<24;
#endif
                    dst_pixel  += 4;
                    
                    src_pixel  += 24;
                    src_n_pixel+= 24;
                }

                //
                //  Any left over pixels...
                //
                while(dstR_xend--) {
                    *dst_pixel++ = ((unsigned int)
                        (*src_pixel     + *src_n_pixel     +
                         *(src_pixel+3) + *(src_n_pixel+3)))>>2;
                    
                    *dst_pixel++ = ((unsigned int)
                        (*(src_pixel+1) + *(src_n_pixel+1) +
                         *(src_pixel+4) + *(src_n_pixel+4)))>>2;
                    
                    *dst_pixel++ = ((unsigned int)
                        (*(src_pixel+2) + *(src_n_pixel+2) +
                         *(src_pixel+5) + *(src_n_pixel+5)))>>2;
                    
                    src_pixel  += 6;
                    src_n_pixel+= 6;
                }
                
                //
                //  Reset everything back to the beginning of the
                //   next scanline.
                //
                src_scanline   += src_nx_scan2;
                src_pixel       = src_scanline;

                src_n_scanline += src_nx_scan2;
                src_n_pixel     = src_n_scanline;

                dst_scanline   += dst_nx_scan;
                dst_pixel       = dst_scanline;
            } while(--dstR_ysize);
        }
    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    //
    //      4 CONTIGUOUS BAND OPTIMIZED CASE
    //
    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    } else if(nbands == 4 && src_nx_pixel == 4 && dst_nx_pixel == 4) {
        //
        //  Loop over the rectangle list
        //
        int          dstR_x, dstR_y;
        unsigned int dstR_xsize, dstR_ysize;
        while(rl.getNext(&dstR_x, &dstR_y, &dstR_xsize, &dstR_ysize)) {
            //
            //  The starting point in the destination.
            //
            Xil_unsigned8* dst_scanline = dst_base_addr +
                ((dstR_y + dst_yorig) * dst_nx_scan) +
                ((dstR_x + dst_xorig) * dst_nx_pixel);
            Xil_unsigned8* dst_pixel = dst_scanline;

            //
            //  The starting point in the source.
            //
            double sx = (double)dstR_x * 2.0;
            double sy = (double)dstR_y * 2.0;
            Xil_unsigned8* src_scanline = src_base_addr + 
                (((int)sy + src_yorig) * src_nx_scan) +
                (((int)sx + src_xorig) * src_nx_pixel);
            Xil_unsigned8* src_pixel = src_scanline;

            //
            //  Initialize the variables which we'll use to move through
            //   the image.
            //
            //  The source is incremented by two since we're doing
            //   two lines of the source for every line of the
            //   destination.  This saves an add per scanline.
            //
            unsigned int   src_nx_scan2 = src_nx_scan + src_nx_scan;
            
            //
            //  Here I set a pointer to the scanline we just computed.
            //
            Xil_unsigned8* src_n_scanline = src_scanline + src_nx_scan;
            Xil_unsigned8* src_n_pixel    = src_n_scanline;

            do {
                //
                //  Handle non word-aligned destination
                //
                unsigned int dstR_xbegin = 0;
                unsigned int tmp_xsize = dstR_xsize;
                while(((unsigned long)dst_pixel) & 0x3 &&
                      tmp_xsize--) {
                    *dst_pixel++ = ((unsigned int)
                        (*src_pixel     + *src_n_pixel     +
                         *(src_pixel+4) + *(src_n_pixel+4)))>>2;
                    
                    *dst_pixel++ = ((unsigned int)
                        (*(src_pixel+1) + *(src_n_pixel+1) +
                         *(src_pixel+5) + *(src_n_pixel+5)))>>2;
                    
                    *dst_pixel++ = ((unsigned int)
                        (*(src_pixel+2) + *(src_n_pixel+2) +
                         *(src_pixel+6) + *(src_n_pixel+6)))>>2;
                    
                    *dst_pixel++ = ((unsigned int)
                        (*(src_pixel+3) + *(src_n_pixel+3) +
                         *(src_pixel+7) + *(src_n_pixel+7)))>>2;
                    
                    src_pixel  += 8;
                    src_n_pixel+= 8;

                    dstR_xbegin++;
                }
                
                unsigned int dstR_xsize_4  = (dstR_xsize - dstR_xbegin)>>2;
                unsigned int dstR_xend     =
                    dstR_xsize - (dstR_xsize_4<<2) - dstR_xbegin;

                while(dstR_xsize_4--) {
                    //
                    //  Calculate the interpolation points for band 0 and
                    //   set the corresponding pixels in the destination.
                    //
                    unsigned int  interp1_0 = ((unsigned int)
                        (*src_pixel     + *src_n_pixel     +
                         *(src_pixel+4) + *(src_n_pixel+4)))>>2;

                    unsigned int  interp1_1 = ((unsigned int)
                        (*(src_pixel+1) + *(src_n_pixel+1) +
                         *(src_pixel+5) + *(src_n_pixel+5)))>>2;

                    unsigned int  interp1_2 = ((unsigned int)
                        (*(src_pixel+2) + *(src_n_pixel+2) +
                         *(src_pixel+6) + *(src_n_pixel+6)))>>2;

                    unsigned int  interp1_3 = ((unsigned int)
                        (*(src_pixel+3) + *(src_n_pixel+3) +
                         *(src_pixel+7) + *(src_n_pixel+7)))>>2;

#ifndef XIL_LITTLE_ENDIAN
                    *((unsigned int*)dst_pixel) =
                         interp1_0<<24 |
                         interp1_1<<16 |
                         interp1_2<<8  |
                         interp1_3;
#else
                    *((unsigned int*)dst_pixel) =
                         interp1_0     |
                         interp1_1<<8  |
                         interp1_2<<16 |
                         interp1_3<<24;
#endif
                    dst_pixel += 4;
                    
                    unsigned int  interp2_0 = ((unsigned int)
                        (*(src_pixel+8) + *(src_n_pixel+8) +
                         *(src_pixel+12) + *(src_n_pixel+12)))>>2;

                    unsigned int  interp2_1 = ((unsigned int)
                        (*(src_pixel+9)  + *(src_n_pixel+9) +
                         *(src_pixel+13) + *(src_n_pixel+13)))>>2;

                    unsigned int  interp2_2 = ((unsigned int)
                        (*(src_pixel+10) + *(src_n_pixel+10) +
                         *(src_pixel+14) + *(src_n_pixel+14)))>>2;

                    unsigned int  interp2_3 = ((unsigned int)
                        (*(src_pixel+11) + *(src_n_pixel+11) +
                         *(src_pixel+15) + *(src_n_pixel+15)))>>2;

#ifndef XIL_LITTLE_ENDIAN
                    *((unsigned int*)dst_pixel) =
                         interp2_0<<24 |
                         interp2_1<<16 |
                         interp2_2<<8  |
                         interp2_3;
#else
                    *((unsigned int*)dst_pixel) =
                         interp2_0     |
                         interp2_1<<8  |
                         interp2_2<<16 |
                         interp2_3<<24;
#endif
                    dst_pixel += 4;

                    unsigned int  interp3_0 = ((unsigned int)
                        (*(src_pixel+16) + *(src_n_pixel+16) +
                         *(src_pixel+20) + *(src_n_pixel+20)))>>2;

                    unsigned int  interp3_1 = ((unsigned int)
                        (*(src_pixel+17) + *(src_n_pixel+17) +
                         *(src_pixel+21) + *(src_n_pixel+21)))>>2;

                    unsigned int  interp3_2 = ((unsigned int)
                        (*(src_pixel+18) + *(src_n_pixel+18) +
                         *(src_pixel+22) + *(src_n_pixel+22)))>>2;

                    unsigned int  interp3_3 = ((unsigned int)
                        (*(src_pixel+19) + *(src_n_pixel+19) +
                         *(src_pixel+23) + *(src_n_pixel+23)))>>2;

#ifndef XIL_LITTLE_ENDIAN
                    *((unsigned int*)dst_pixel) =
                         interp3_0<<24 |
                         interp3_1<<16 |
                         interp3_2<<8  |
                         interp3_3;
#else
                    *((unsigned int*)dst_pixel) =
                         interp3_0     |
                         interp3_1<<8  |
                         interp3_2<<16 |
                         interp3_3<<24;
#endif
                    dst_pixel += 4;

                    unsigned int  interp4_0 = ((unsigned int)
                        (*(src_pixel+24) + *(src_n_pixel+24) +
                         *(src_pixel+28) + *(src_n_pixel+28)))>>2;

                    unsigned int  interp4_1 = ((unsigned int)
                        (*(src_pixel+25) + *(src_n_pixel+25) +
                         *(src_pixel+29) + *(src_n_pixel+29)))>>2;

                    unsigned int  interp4_2 = ((unsigned int)
                        (*(src_pixel+26) + *(src_n_pixel+26) +
                         *(src_pixel+30) + *(src_n_pixel+30)))>>2;

                    unsigned int  interp4_3 = ((unsigned int)
                        (*(src_pixel+27) + *(src_n_pixel+27) +
                         *(src_pixel+31) + *(src_n_pixel+31)))>>2;

#ifndef XIL_LITTLE_ENDIAN
                    *((unsigned int*)dst_pixel) =
                         interp4_0<<24 |
                         interp4_1<<16 |
                         interp4_2<<8  |
                         interp4_3;
#else
                    *((unsigned int*)dst_pixel) =
                         interp4_0     |
                         interp4_1<<8  |
                         interp4_2<<16 |
                         interp4_3<<24;
#endif
                    dst_pixel += 4;
                    
                    src_pixel  += 32;
                    src_n_pixel+= 32;
                }

                //
                //  Any left over pixels...
                //
                while(dstR_xend--) {
                    *dst_pixel++ = ((unsigned int)
                        (*src_pixel     + *src_n_pixel     +
                         *(src_pixel+4) + *(src_n_pixel+4)))>>2;
                    
                    *dst_pixel++ = ((unsigned int)
                        (*(src_pixel+1) + *(src_n_pixel+1) +
                         *(src_pixel+5) + *(src_n_pixel+5)))>>2;
                    
                    *dst_pixel++ = ((unsigned int)
                        (*(src_pixel+2) + *(src_n_pixel+2) +
                         *(src_pixel+6) + *(src_n_pixel+6)))>>2;
                    
                    *dst_pixel++ = ((unsigned int)
                        (*(src_pixel+3) + *(src_n_pixel+3) +
                         *(src_pixel+7) + *(src_n_pixel+7)))>>2;
                    
                    src_pixel  += 8;
                    src_n_pixel+= 8;
                }
                
                //
                //  Reset everything back to the beginning of the
                //   next scanline.
                //
                src_scanline   += src_nx_scan2;
                src_pixel       = src_scanline;

                src_n_scanline += src_nx_scan2;
                src_n_pixel     = src_n_scanline;

                dst_scanline   += dst_nx_scan;
                dst_pixel       = dst_scanline;
            } while(--dstR_ysize);
        }
    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    //
    //      N BAND CASE
    //
    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    } else {
        //
        //  Loop over the rectangle list
        //
        int          dstR_x, dstR_y;
        unsigned int dstR_xsize, dstR_ysize;
        while(rl.getNext(&dstR_x, &dstR_y, &dstR_xsize, &dstR_ysize)) {
            //
            //  The starting point in the destination.
            //
            Xil_unsigned8* dst_scanline = dst_base_addr +
                ((dstR_y + dst_yorig) * dst_nx_scan) +
                ((dstR_x + dst_xorig) * dst_nx_pixel);
            Xil_unsigned8* dst_pixel = dst_scanline;
            Xil_unsigned8* dst_band  = dst_scanline;

            //
            //  The starting point in the source.
            //
            double sx = (double)dstR_x * 2.0;
            double sy = (double)dstR_y * 2.0;
            Xil_unsigned8* src_scanline = src_base_addr + 
                (((int)sy + src_yorig) * src_nx_scan) +
                (((int)sx + src_xorig) * src_nx_pixel);
            Xil_unsigned8* src_pixel = src_scanline;
            Xil_unsigned8* src_band  = src_scanline;

            //
            //  Initialize the variables which we'll use to move through
            //   the image.
            //
            //  The source is incremented by two since we're doing
            //   two lines of the source for every line of the
            //   destination.  This saves an add per scanline.
            //
            unsigned int   src_nx_scan2  = src_nx_scan<<1;
            unsigned int   src_nx_pixel2 = src_nx_pixel<<1; 
            
            //  Here I set a pointer to the scanline we just computed.
            //
            Xil_unsigned8* src_n_scanline = src_scanline + src_nx_scan;
            Xil_unsigned8* src_n_pixel    = src_n_scanline;
            Xil_unsigned8* src_n_band     = src_n_scanline;

            do {
                int xsize = dstR_xsize;
                
                src_pixel       = src_scanline;
                src_n_pixel     = src_n_scanline;
                dst_pixel       = dst_scanline;

                do {
                    int bands = nbands;

                    src_band   = src_pixel;
                    src_n_band = src_n_pixel;
                    dst_band   = dst_pixel;
                    
                    do {
                        *dst_band = ((unsigned int)
                                     (*src_band                 +
                                      *src_n_band               +
                                      *(src_band+src_nx_pixel)  +
                                      *(src_n_band+src_nx_pixel)))>>2;

                        dst_band++;
                        src_band++;
                        src_n_band++;
                    } while(--bands);
                    
                    src_pixel   += src_nx_pixel2;
                    src_n_pixel += src_nx_pixel2;
                    dst_pixel   += dst_nx_pixel;
                } while(--xsize);

                //
                //  Reset everything back to the beginning of the
                //   next scanline.
                //
                src_scanline   += src_nx_scan2;
                src_n_scanline += src_nx_scan2;
                dst_scanline   += dst_nx_scan;
            } while(--dstR_ysize);
        } 
    }
    
    return XIL_SUCCESS;
}

static XilStatus scale_subsample2x_general_storage_BL(AffineData affine_data)
{
    XilStorage*  src_storage   = affine_data.src_storage;
    XilStorage*  dst_storage   = affine_data.dst_storage;
    XilBox*      dst_box       = affine_data.dst_box;
    XilRoi*      roi           = affine_data.roi;
    unsigned int nbands        = affine_data.nbands;

    //
    // Create a rectangle list that is an intersection of
    // the intersected roi and dst_box. The list is
    // returned in the dst_box coordinate space (not
    // in the dst image coordinate space).
    //
    XilRectList rl(roi, dst_box);

    //
    //  Loop over the rectangle list
    //
    int          dstR_x, dstR_y;
    unsigned int dstR_xsize, dstR_ysize;
    while(rl.getNext(&dstR_x, &dstR_y, &dstR_xsize, &dstR_ysize)) {
        for(int bandn = 0; bandn < nbands; bandn++) {
            unsigned int ysize = dstR_ysize;
            //
            // Get the storage information
            //
            Xil_unsigned8* src_data =
                (Xil_unsigned8*) src_storage->getDataPtr(bandn);
            unsigned int   src_pstride = src_storage->getPixelStride(bandn);
            unsigned int   src_sstride = src_storage->getScanlineStride(bandn);

            Xil_unsigned8* dst_data =
                (Xil_unsigned8*) dst_storage->getDataPtr(bandn);
            unsigned int   dst_pstride = dst_storage->getPixelStride(bandn);
            unsigned int   dst_sstride = dst_storage->getScanlineStride(bandn);

            //
            // Set up some variables used in the accelerated algorithm
            //
            Xil_unsigned8* src_base_addr = src_data;
            unsigned int   src_nx_pixel = src_pstride;
            unsigned int   src_nx_scan = src_sstride;

            Xil_unsigned8* dst_base_addr = dst_data;
            unsigned int   dst_nx_pixel = dst_pstride;
            unsigned int   dst_nx_scan = dst_sstride;

            //
            // TODO  bpb  02/13/97  Delete the following variables if possible
            //
            int src_xorig = 0, src_yorig = 0;
            int dst_xorig = 0, dst_yorig = 0;
            XilImage* src = src_storage->getImage();
            int src_xsize = src->getWidth();
            int src_ysize = src->getHeight();
            XilImage* dst = dst_storage->getImage();
            int dst_xsize = dst->getWidth();
            int dst_ysize = dst->getHeight();

            //
            //  The starting point in the destination.
            //
            Xil_unsigned8* dst_scanline = dst_base_addr +
                ((dstR_y + dst_yorig) * dst_nx_scan) +
                ((dstR_x + dst_xorig) * dst_nx_pixel);
            Xil_unsigned8* dst_pixel = dst_scanline;
            Xil_unsigned8* dst_band  = dst_scanline;

            //
            //  The starting point in the source.
            //
            double sx = (double)dstR_x * 2.0;
            double sy = (double)dstR_y * 2.0;
            Xil_unsigned8* src_scanline = src_base_addr + 
                (((int)sy + src_yorig) * src_nx_scan) +
                (((int)sx + src_xorig) * src_nx_pixel);
            Xil_unsigned8* src_pixel = src_scanline;
            Xil_unsigned8* src_band  = src_scanline;

            //
            //  Initialize the variables which we'll use to move through
            //   the image.
            //
            //  The source is incremented by two since we're doing
            //   two lines of the source for every line of the
            //   destination.  This saves an add per scanline.
            //
            unsigned int   src_nx_scan2  = src_nx_scan<<1;
            unsigned int   src_nx_pixel2 = src_nx_pixel<<1; 
            
            //  Here I set a pointer to the scanline we just computed.
            //
            Xil_unsigned8* src_n_scanline = src_scanline + src_nx_scan;
            Xil_unsigned8* src_n_pixel    = src_n_scanline;
            Xil_unsigned8* src_n_band     = src_n_scanline;

            do {
                int xsize = dstR_xsize;
                
                src_pixel       = src_scanline;
                src_n_pixel     = src_n_scanline;
                dst_pixel       = dst_scanline;

                do {
                    int bands = 1; // TODO  bpb  02/13/97  Remove this kludge

                    src_band   = src_pixel;
                    src_n_band = src_n_pixel;
                    dst_band   = dst_pixel;
                    
                    do {
                        *dst_band = ((unsigned int)
                                     (*src_band                 +
                                      *src_n_band               +
                                      *(src_band+src_nx_pixel)  +
                                      *(src_n_band+src_nx_pixel)))>>2;

                        dst_band++;
                        src_band++;
                        src_n_band++;
                    } while(--bands);
                    
                    src_pixel   += src_nx_pixel2;
                    src_n_pixel += src_nx_pixel2;
                    dst_pixel   += dst_nx_pixel;
                } while(--xsize);

                //
                //  Reset everything back to the beginning of the
                //   next scanline.
                //
                src_scanline   += src_nx_scan2;
                src_n_scanline += src_nx_scan2;
                dst_scanline   += dst_nx_scan;
            } while(--ysize);
        }
    } 
    
    return XIL_SUCCESS;
}
#endif
