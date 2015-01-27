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
//  File:	Transpose.cc
//  Project:	XIL
//  Revision:	1.9
//  Last Mod:	10:13:04, 03/10/00
//
//  Description:
//
//
//  Transpose() retrieves float src and dst images, a fliptype and
//  ROIs.  It then "flips" the src image, around image center,
//  into the dst image in the following manner:
//
//  If "fliptype" = XIL_FLIP_X_AXIS - horizontal, across the X axis
//                  XIL_FLIP_Y_AXIS - vertical, across the Y axis
//                  XIL_FLIP_MAIN_DIAGONAL - transpose across the main diagonal
//                  XIL_FLIP_ANTIDIAGONAL - transpose across the anti diagonal
//                  XIL_FLIP_90  - rotate counterclockwise 90 degress
//                  XIL_FLIP_180 - rotate counterclockwise 180 degress
//                  XIL_FLIP_270 - rotate counterclockwise 270 degress
//
//	
//  MT-level:  SAFE
//	
//------------------------------------------------------------------------
//	COPYRIGHT
//------------------------------------------------------------------------
#pragma ident	"@(#)Transpose.cc	1.9\t00/03/10  "

#include "XilDeviceManagerComputeFLOAT.hh"
#include "ComputeInfo.hh"


XilStatus
XilDeviceManagerComputeFLOAT::Transpose(XilOp* op,
					unsigned ,
					XilRoi*      roi,
					XilBoxList*  bl)
{
    //
    // Get the basic data, assuming that the src image is a FLOAT image.
    // roi is the complete image ROI. This means that src and dst ROI's are
    // already taken into account, and the roi passed is the intersection
    // of these. 
    //
    if(op->splitOnTileBoundaries(bl) == XIL_FAILURE) {
        return XIL_FAILURE;
    }

    //
    //  Get the images for our operation.
    //
    XilImage* src = op->getSrcImage(1);
    XilImage* dst = op->getDstImage(1);

    //
    //  Get the fliptype for the transpose operation
    //
    XilFlipType fliptype;
    op->getParam(1, (int *)&fliptype);

    //  
    // Get number of image bands
    //
    unsigned int nbands = src->getNumBands();

    //
    //  Loop over each of the boxes of storage we are to process.
    //
    XilBox* src_box;
    XilBox* dst_box;
    
    while(bl->getNext(&src_box, &dst_box)) {
	//
        //  Aquire our storage from the images.  
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


	//
	//  If the images are PIXEL_SEQUENTIAL
	//
	if((src_storage.isType(XIL_PIXEL_SEQUENTIAL)) &&
	   (dst_storage.isType(XIL_PIXEL_SEQUENTIAL))) {

	    
	    Xil_float32*   src_data;
	    unsigned int   src_pixel_stride;
	    unsigned int   src_scanline_stride;

	    src_storage.getStorageInfo(&src_pixel_stride,
				       &src_scanline_stride,
				       NULL, NULL,
				       (void**)&src_data);
            
	    unsigned int   dst_pixel_stride;
	    unsigned int   dst_scanline_stride;
	    Xil_float32*   dst_data;
	    dst_storage.getStorageInfo(&dst_pixel_stride,
				       &dst_scanline_stride,
				       NULL, NULL,
				       (void**)&dst_data);

	    int            	dstR_x;
	    int             dstR_y;	        // dst region location 
	    unsigned int   	dstR_xsize;
	    unsigned int    dstR_ysize;	// dst region size 
	    int            	srcR_x;
	    int             srcR_y;	        // src region location 
	    Xil_float32*    src_scanline;
	    Xil_float32*    dst_scanline;	// ptr to current scanline 
	    Xil_float32*    src_pixel;
	    Xil_float32*    dst_pixel;	// ptr to current pixel 
	    Xil_float32*    src_band; 
	    Xil_float32*    dst_band; 	// ptr to the current band 
	    unsigned int   	src_next_band = 1;
	    unsigned int    dst_next_band = 1;	
 
	    int  	        incr1, incr2;

	//
	// Increments can be set up once per box pair
	//
	    switch (fliptype) {
	      case XIL_FLIP_X_AXIS:
		incr1 = src_pixel_stride;
		incr2 = -src_scanline_stride;
		break;

	      case XIL_FLIP_Y_AXIS:
		incr1 = -src_pixel_stride;
		incr2 = src_scanline_stride;
		break;

	      case XIL_FLIP_MAIN_DIAGONAL:
		incr1 = src_scanline_stride;
		incr2 = src_pixel_stride;
		break;

	      case XIL_FLIP_ANTIDIAGONAL:
		incr1 = -src_scanline_stride;
		incr2 = -src_pixel_stride;
		break;
	    
	      case XIL_FLIP_90:
		incr1 = src_scanline_stride;
		incr2 = -src_pixel_stride;
		break;
	    
	      case XIL_FLIP_180:
		incr1 = -src_pixel_stride;
		incr2 = -src_scanline_stride;
		break;
	    
	      case XIL_FLIP_270:
		incr1 = -src_scanline_stride;
		incr2 = src_pixel_stride;
		break;
	    }

	    //
	    //  Create a list of rectangles to loop over.  The resulting list
	    //  of rectangles is the area created by intersecting the ROI with
	    //  the destination box.
	    //
	    XilRectList    rl(roi, dst_box);
	    //
	    // loop over the list of rectangles
	    //
	    while (rl.getNext(&dstR_x, &dstR_y, &dstR_xsize, &dstR_ysize)) {
		//
		// The rectangle in the list applies to the dst, so we have
		// to find appropriate rectangle in the src according to
		// the fliptype.
		//
		// The op does the backward map for us.
		//
                {
                    double srcx;
                    double srcy;
                    op->backwardMap(dst_box, dstR_x, dstR_y,
                                    src_box, &srcx, &srcy);

                    srcR_x = (int)srcx;
                    srcR_y = (int)srcy;
                }

		//
		// setup data pointers
		//
		src_scanline = src_data + (srcR_y * src_scanline_stride)
		    + (srcR_x * src_pixel_stride);
		dst_scanline = dst_data + (dstR_y * dst_scanline_stride)
		    + (dstR_x * dst_pixel_stride);

		//
		// step pixel by pixel through the dst copying appropriately
		// from the src rectangle
		//
		src_band = src_pixel = src_scanline;
		dst_band = dst_pixel = dst_scanline;

		unsigned int i, j, k;	        // loop counters
		for (i = 0; i < dstR_ysize; i++) {	// loop over scanlines 
		    for (j = 0; j < dstR_xsize; j++) {	// loop over pixels 
			for (k = 0; k < nbands; k++) {	// loop over bands 
			    *dst_band = *src_band;
			    src_band += src_next_band;
			    dst_band += dst_next_band;
			}

			src_pixel += incr1;
			dst_pixel += dst_pixel_stride;
			src_band = src_pixel;
			dst_band = dst_pixel;
		    }

		    src_scanline += incr2;
		    dst_scanline += dst_scanline_stride;
		    src_pixel = src_scanline;
		    dst_pixel = dst_scanline;
		    src_band = src_pixel;
		    dst_band = dst_pixel;
		}
	    }
	} else {		// XIL_GENERAL storage type

	    
	    Xil_float32*   src_data;
	    unsigned int   src_pixel_stride;
	    unsigned int   src_scanline_stride;

	    unsigned int   dst_pixel_stride;
	    unsigned int   dst_scanline_stride;
	    Xil_float32*   dst_data;

	    int             dstR_x;
	    int             dstR_y;	        // dst region location 
	    unsigned int    dstR_xsize;
	    unsigned int    dstR_ysize;	// dst region size 
	    int             srcR_x;
	    int             srcR_y;	        // src region location 
	    Xil_float32*    src_scanline;
	    Xil_float32*    dst_scanline;	// ptr to current scanline 
	    Xil_float32*    src_pixel;
	    Xil_float32*    dst_pixel;	// ptr to current pixel 
	    Xil_float32*    src_band; 
	    Xil_float32*    dst_band; 	// ptr to the current band 
 
	    int  	    incr1, incr2;

	    //
	    //  Each Band...
	    //
	    for(unsigned int band = 0; band < nbands; band++) {
		
		src_storage.getStorageInfo(band,
					   &src_pixel_stride,
					   &src_scanline_stride,
					   NULL,
					   (void**)&src_data);
		
		dst_storage.getStorageInfo(band,
					   &dst_pixel_stride,
					   &dst_scanline_stride,
					   NULL,
					   (void**)&dst_data);


		//
		// Increments can be set up once per box pair
		//
		switch (fliptype) {
		  case XIL_FLIP_X_AXIS:
		    incr1 = src_pixel_stride;
		    incr2 = -src_scanline_stride;
		    break;

		  case XIL_FLIP_Y_AXIS:
		    incr1 = -src_pixel_stride;
		    incr2 = src_scanline_stride;
		    break;

		  case XIL_FLIP_MAIN_DIAGONAL:
		    incr1 = src_scanline_stride;
		    incr2 = src_pixel_stride;
		    break;

		  case XIL_FLIP_ANTIDIAGONAL:
		    incr1 = -src_scanline_stride;
		    incr2 = -src_pixel_stride;
		    break;
	    
		  case XIL_FLIP_90:
		    incr1 = src_scanline_stride;
		    incr2 = -src_pixel_stride;
		    break;
	    
		  case XIL_FLIP_180:
		    incr1 = -src_pixel_stride;
		    incr2 = -src_scanline_stride;
		    break;
	    
		  case XIL_FLIP_270:
		    incr1 = -src_scanline_stride;
		    incr2 = src_pixel_stride;
		    break;
		}

		//
		//  Create a list of rectangles to loop over.  The list
		//  of rectangles is the area created by intersecting the
		//  ROI with the destination box.
		//
		XilRectList    rl(roi, dst_box);
		//
		// loop over the list of rectangles
		//
		while (rl.getNext(&dstR_x, &dstR_y, &dstR_xsize, &dstR_ysize)){
		    //
		    // The rectangle in the list applies to the dst, so we 
		    // find appropriate rectangle in the src according to
		    // the fliptype.
		    //
		    // The op does the backward map for us.
		    //
                    {
                        double srcx;
                        double srcy;
                        op->backwardMap(dst_box, dstR_x, dstR_y,
                                        src_box, &srcx, &srcy);

                        srcR_x = (int)srcx;
                        srcR_y = (int)srcy;
                    }

		    //
		    // setup data pointers
		    //
		    src_scanline = src_data + (srcR_y * src_scanline_stride)
			+ (srcR_x * src_pixel_stride);
		    dst_scanline = dst_data + (dstR_y * dst_scanline_stride)
			+ (dstR_x * dst_pixel_stride);

		    //
		    // step pixel by pixel through dst copying appropriately
		    // from the src rectangle
		    //
		    src_band = src_pixel = src_scanline;
		    dst_band = dst_pixel = dst_scanline;

		    unsigned int i, j;	        
		    for (i = 0; i < dstR_ysize; i++) {	// loop over scanlines 
			for (j = 0; j < dstR_xsize; j++) {
			    *dst_band = *src_band;
			    src_pixel += incr1;
			    dst_pixel += dst_pixel_stride;
			    src_band = src_pixel;
			    dst_band = dst_pixel;
			}
			src_scanline += incr2;
			dst_scanline += dst_scanline_stride;
			src_pixel = src_scanline;
			dst_pixel = dst_scanline;
			src_band = src_pixel;
			dst_band = dst_pixel;
		    }
		}
	    }
	}
    }
    return XIL_SUCCESS;
}


	
