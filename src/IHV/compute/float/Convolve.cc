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
//  File:	Convolve.cc
//  Project:	XIL
//  Revision:	1.15
//  Last Mod:	10:12:57, 03/10/00
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
#pragma ident	"@(#)Convolve.cc	1.15\t00/03/10  "

#include "XilDeviceManagerComputeFLOAT.hh"
#include "XiliUtils.hh"

//
//  Which type of optimization can we use in the center case.
//
enum XiliConvolveOptType {
    KERNEL_SQUARE,
    KERNEL_3X3,
    NO_OPT
};

static
XilStatus
xili_convolve_square_opt_1(Xil_float32*  dst_data,
                           unsigned int  dst_sstride,
                           unsigned int  dst_pstride,
                           Xil_float32*  src_data,
                           unsigned int  src_sstride,
                           unsigned int  src_pstride,
                           int           roi_x,
                           int           roi_y,
                           int           roi_xsize,
                           int           roi_ysize,
                           float*        kdata,
                           int           ksize);

static
XilStatus
xili_convolve_3x3_opt_1(Xil_float32*  dst_data,
                        unsigned int  dst_sstride,
                        unsigned int  dst_pstride,
                        Xil_float32*  src_data,
                        unsigned int  src_sstride,
                        unsigned int  src_pstride,
                        int           roi_x,
                        int           roi_y,
                        int           roi_xsize,
                        int           roi_ysize,
                        float*        kdata);



XilStatus
XilDeviceManagerComputeFLOAT::Convolve(XilOp*       op,
				      unsigned     ,
				      XilRoi*      roi,
				      XilBoxList*  bl)
{
    //
    //  Split the list of XilBoxes to take tile boundaries into account.  This
    //  will work to ensure that no cobbling of the data is required because
    //  the boxes will not cross tile boundaries in the source images.
    //
    if(op->splitOnTileBoundaries(bl) == XIL_FAILURE) {
        return XIL_FAILURE;
    }

    //
    //  Get the images for our operation.
    //
    XilImage* src1Image = op->getSrcImage(1);
    XilImage* destImage = op->getDstImage(1);

    XilKernel*          kernel;
    op->getParam(1, (void**)&kernel);
    //
    // XIL_EDGE_NO_WRITE
    // XIL_EDGE_ZERO_FILL
    // XIL_EDGE_EXTEND
    //
    XilEdgeCondition    edge_condition;
    op->getParam(2, (int*)&edge_condition);

    //
    //  Verify it's an edge condition we support and know about...
    //
    switch(edge_condition) {
      case XIL_EDGE_NO_WRITE:
      case XIL_EDGE_EXTEND:
      case XIL_EDGE_ZERO_FILL:
        break;

      default:
        return XIL_FAILURE;
    }

    unsigned int kwidth  = kernel->getWidth();
    unsigned int kheight = kernel->getHeight();
    int keyx   = (int)kernel->getKeyX();
    int keyy   = (int)kernel->getKeyY();
    float* kdata  = (float *) kernel->getData();

    //
    //  Store away the number of bands for this operation.
    //    
    unsigned int nbands   = destImage->getNumBands();

    //
    //  Determine what type of optimization we can do with this kernel.
    //
    XiliConvolveOptType opt_type = NO_OPT;
    if((kwidth == kheight)     &&
       (keyx   == (kwidth>>1)) &&
       (keyy   == keyx)        &&
       (kwidth&1)) {
        if(kwidth == 3) {
            //
            //  3x3 special case
            //
            opt_type = KERNEL_3X3;
        } else {
            //
            //  A square, odd size kernel with the key at the center.
            //
            opt_type = KERNEL_SQUARE;
        }
    }

    //
    //  Miscellaneous Counters
    //
    int i;
    int j;
    
    //
    //  Loop over each of the boxes of storage we are to process.
    //
    XilBox* src1_box;
    XilBox* dest_box;
    while(bl->getNext(&src1_box, &dest_box)) {
        //
        //  Get the tag associated with this box which indicates where the box
        //  resides in the image.  Verify that the tag is of a type we know.
        //
	XilBoxAreaType tag = (XilBoxAreaType) ((long)dest_box->getTag());
	switch(tag) {
	  case XIL_AREA_TOP_LEFT_CORNER:
	  case XIL_AREA_TOP_EDGE:
	  case XIL_AREA_TOP_RIGHT_CORNER:
	  case XIL_AREA_RIGHT_EDGE:
	  case XIL_AREA_CENTER:
	  case XIL_AREA_LEFT_EDGE:
	  case XIL_AREA_BOTTOM_LEFT_CORNER:
	  case XIL_AREA_BOTTOM_EDGE:
	  case XIL_AREA_BOTTOM_RIGHT_CORNER:
	    break;

          case XIL_AREA_SMALL_SOURCE:
            //
            //  We do not currently support convolve implementation
            //  for sources smaller than the kernel.
            //  Don't mark the box as failed since there's no
            //  other implementation to fall to.
            //
            XIL_ERROR(destImage->getSystemState(), XIL_ERROR_INTERNAL,
                      "di-447", TRUE);
            continue;

	  default:
	    if(bl->markAsFailed() == XIL_FAILURE) {
		return XIL_FAILURE;
	    } else {
		continue;
	    }
	}

        //
        //  Aquire our storage from the images.  The storage returned is valid
        //  for the box given.  Thus, any origins or child offsets have been
        //  taken into account.
        //
	XilStorage  src1_storage(src1Image);
        XilStorage  dest_storage(destImage);
        if((src1Image->getStorage(&src1_storage, op, src1_box, "XilMemory",
                             XIL_READ_ONLY) == XIL_FAILURE) || 
	    (destImage->getStorage(&dest_storage, op, dest_box, "XilMemory",
			      XIL_WRITE_ONLY) == XIL_FAILURE)) {
	    //
	    //  Mark this box entry as having failed.  If marking the box
	    //  returns XIL_FAILURE, then we return XIL_FAILURE.
	    //
	    if(bl->markAsFailed() == XIL_FAILURE) {
		return XIL_FAILURE;
	    } else {
		continue;
	    }
	}


	//
	// If the edge condition is no write and this box is in an edge area
	// then simple continue on to the next box.
	//
        //  TODO: 7/12/96 jlf  The XIL core shouldn't generate edge conditions
        //                     in this case so we shouldn't need to test for
        //                     them here.
        //
	if((edge_condition == XIL_EDGE_NO_WRITE) &&
	    (tag != XIL_AREA_CENTER)) {
	    continue;
	}

        //
        //  Get the image coordinates of our source from the box.
        //
        int          src_box_x;
        int          src_box_y;
        unsigned int src_box_xsize;
        unsigned int src_box_ysize;
        src1_box->getAsRect(&src_box_x, &src_box_y,
                            &src_box_xsize, &src_box_ysize);

	//
	// If the edge condition is no write and this box is in an edge area
	// then simple continue on to the next box.
	//
	if((edge_condition == XIL_EDGE_ZERO_FILL) &&
           (tag != XIL_AREA_CENTER)) {
	    //
            // General Storage Implementation.
            //
            XilRectList  rl(roi, dest_box);
	    
            int          x;
            int          y;
            unsigned int xsize;
            unsigned int ysize;
            while(rl.getNext(&x, &y, &xsize, &ysize)) {
                //
                //  Each Band...
                //
                for(unsigned int band=0; band<nbands; band++) {
                    unsigned int   dest_pixel_stride;
                    unsigned int   dest_scanline_stride;
                    Xil_float32* dest_data;
                    dest_storage.getStorageInfo(band,
                                                &dest_pixel_stride,
                                                &dest_scanline_stride,
                                                NULL,
                                                (void**)&dest_data);
		    
                    Xil_float32* dest_scanline = dest_data +
                        (y*dest_scanline_stride) + (x*dest_pixel_stride);
		    
                    unsigned int scanline_count = ysize;
                    
                    //
                    //  Each Scanline...
                    //
                    do {
                        Xil_float32* dest_pixel = dest_scanline;
			
                        unsigned int pixel_count = xsize;
			
                        //
                        //  Each Pixel...
                        //
                        do {
                            *dest_pixel = 0;
                            dest_pixel += dest_pixel_stride;
			    
			} while(--pixel_count);
			
                        dest_scanline += dest_scanline_stride;
                    } while(--scanline_count);
                }
            }
	    continue;
        }

	if(tag != XIL_AREA_CENTER) {
	    //
	    // So it's an edge box and since we have handled the other types
	    // of edge conditions above, this must be an edge extend case.
	    //
	    XilRectList  rl(roi, dest_box);
	    
	    int          x;
	    int          y;
	    unsigned int xsize;
	    unsigned int ysize;
	    while(rl.getNext(&x, &y, &xsize, &ysize)) {
		int band;
		switch(tag) {
		  case XIL_AREA_TOP_LEFT_CORNER:
		    //
		    //  Each Band...
		    //
		    for(band=0; band<nbands; band++) {
			unsigned int   src1_pixel_stride;
			unsigned int   src1_scanline_stride;
			Xil_float32* src1_data;
			src1_storage.getStorageInfo(band,
						    &src1_pixel_stride,
						    &src1_scanline_stride,
						    NULL,
						    (void**)&src1_data);

			unsigned int   dest_pixel_stride;
			unsigned int   dest_scanline_stride;
			Xil_float32* dest_data;
			dest_storage.getStorageInfo(band,
						    &dest_pixel_stride,
						    &dest_scanline_stride,
						    NULL,
						    (void**)&dest_data);
			
			Xil_float32* src1_scanline = src1_data +
			    (y*src1_scanline_stride) + (x*src1_pixel_stride);

			Xil_float32* dest_scanline = dest_data +
			    (y*dest_scanline_stride) + (x*dest_pixel_stride);

			int ptrX = src_box_x + x;
			int ptrY = src_box_y + y;
			int kulX = ptrX - keyx;
			int kulY = ptrY - keyy;

			//
			// Clamp kernel upper left and since this is the
			// upper left corner, both should clamp to 0.
			//
			if(kulX < 0) kulX = 0;
			if(kulY < 0) kulY = 0;

			Xil_float32* up_left = src1_scanline +
			    ((kulX - ptrX) * src1_pixel_stride) +
			    ((kulY - ptrY) * src1_scanline_stride);
			
			//
			// Do the cases where the kernel extends above or meets the top
			// of the image.
			//
			for(j = 0; j < ysize; j++) {
			    //
			    // point to the first pixel of the scanline 
			    //
			    Xil_float32* src1_pixel = src1_scanline;
			    Xil_float32* dest_pixel = dest_scanline;
			    
			    for(i = 0; i < xsize; i++) {
				Xil_float32* dest = dest_pixel;
				
				double fsum = 0.0;
				float *kptr = kdata;

				Xil_float32* corner = up_left;
				Xil_float32* sptr;
				int kh;
				int kw;
				for(kh = 0; kh < keyy - ptrY - j; kh++) {
				    sptr = corner;
				    for(kw = 0; kw < keyx - ptrX - i; kw++) {
					fsum += (double)(*sptr) * (double)*(kptr + kh * kwidth + kw);
				    }
				    
				    for(kw = keyx - ptrX - i; kw < kwidth; kw++) {
					fsum += (double)(*sptr) * (double)*(kptr + kh * kwidth + kw);
					sptr += src1_pixel_stride;
				    }
				}

				sptr = corner;
				for(kh = keyy - ptrY - j; kh < kheight; kh++) {
				    for(kw = 0; kw < keyx - ptrX - i; kw++) {
				        fsum += (double)(*sptr) * (double)*(kptr + kh * kwidth + kw);
				    }
					
				    for(kw = keyx - ptrX - i; kw < kwidth; kw++) {
				        fsum += (double)(*sptr) * (double)*(kptr + kh * kwidth + kw);
					sptr += src1_pixel_stride;
				    }
					
				    sptr = (corner += src1_scanline_stride);
				}
				
				*dest = fsum;
				
				/* move to the next pixel */
				src1_pixel += src1_pixel_stride;
				dest_pixel += dest_pixel_stride;
			    }
			    
			    /* move to the next scanline */
			    src1_scanline += src1_scanline_stride;
			    dest_scanline += dest_scanline_stride;
			}
		    }
		    break; 
		  case XIL_AREA_TOP_EDGE:
		    //
		    //  Each Band...
		    //
		    for(band=0; band<nbands; band++) {
			unsigned int   src1_pixel_stride;
			unsigned int   src1_scanline_stride;
			Xil_float32* src1_data;
			src1_storage.getStorageInfo(band,
						    &src1_pixel_stride,
						    &src1_scanline_stride,
						    NULL,
						    (void**)&src1_data);
	    
			unsigned int   dest_pixel_stride;
			unsigned int   dest_scanline_stride;
			Xil_float32* dest_data;
			dest_storage.getStorageInfo(band,
						    &dest_pixel_stride,
						    &dest_scanline_stride,
						    NULL,
						    (void**)&dest_data);
			
			Xil_float32* src1_scanline = src1_data +
			    (y*src1_scanline_stride) + (x*src1_pixel_stride);

			Xil_float32* dest_scanline = dest_data +
			    (y*dest_scanline_stride) + (x*dest_pixel_stride);

			int ptrX = src_box_x + x;
			int ptrY = src_box_y + y;
			int kulX = ptrX - keyx;
			int kulY = ptrY - keyy;
			//
			// Clamp kernel upper left.
			// Y should be clamping to 0.
			// X might hit zero but shouldn't ever go below.
			//
			if(kulX < 0) kulX = 0;
			if(kulY < 0) kulY = 0;
			Xil_float32* kernel_start = src1_scanline +
			    ((kulX - ptrX) * src1_pixel_stride) +
			    ((kulY - ptrY) * src1_scanline_stride);
			
			//
			// Do the cases where the kernel extends above or meets the top
			// of the image.
			//
			for(j = 0; j < ysize; j++) {
			    // point to the first pixel of the scanline 
			    Xil_float32* src1_pixel = src1_scanline;
			    Xil_float32* dest_pixel = dest_scanline;
			    
			    for(i = 0; i < xsize; i++) {
				Xil_float32* dest = dest_pixel;
				
				double fsum = 0.0;
				float *kptr = kdata;
				
				Xil_float32* sptr;
				Xil_float32* corner = kernel_start + (i * src1_pixel_stride);
				int kh;
				int kw;
				for(kh = 0; kh < keyy - ptrY - j; kh++) {
				    sptr = corner;
				    for(kw = 0; kw < kwidth; kw++) {
					fsum += (double)(*sptr) * (double)*(kptr + kh * kwidth + kw);
					sptr += src1_pixel_stride;
				    }
				}

				sptr = corner;
				for(kh = keyy - ptrY - j; kh < kheight; kh++) {
				    for(kw = 0; kw < kwidth; kw++) {
					fsum += (double)(*sptr) * (double)*(kptr + kh * kwidth + kw);
					sptr += src1_pixel_stride;
				    }
				    
				    sptr = (corner += src1_scanline_stride);
				}
				
				*dest = fsum;
				
				/* move to the next pixel */
				src1_pixel += src1_pixel_stride;
				dest_pixel += dest_pixel_stride;
			    }
			    
			    /* move to the next scanline */
			    src1_scanline += src1_scanline_stride;
			    dest_scanline += dest_scanline_stride;
			}
		    }
		    break;
		  case XIL_AREA_TOP_RIGHT_CORNER:
		    //
		    //  Each Band...
		    //
		    for(band=0; band<nbands; band++) {
			unsigned int   src1_pixel_stride;
			unsigned int   src1_scanline_stride;
			Xil_float32* src1_data;
			src1_storage.getStorageInfo(band,
						    &src1_pixel_stride,
						    &src1_scanline_stride,
						    NULL,
						    (void**)&src1_data);
	    
			unsigned int   dest_pixel_stride;
			unsigned int   dest_scanline_stride;
			Xil_float32* dest_data;
			dest_storage.getStorageInfo(band,
						    &dest_pixel_stride,
						    &dest_scanline_stride,
						    NULL,
						    (void**)&dest_data);
			
			Xil_float32* src1_scanline = src1_data +
			    (y*src1_scanline_stride) + (x*src1_pixel_stride);

			Xil_float32* dest_scanline = dest_data +
			    (y*dest_scanline_stride) + (x*dest_pixel_stride);

			int ptrX = src_box_x + x;
			int ptrY = src_box_y + y;
			int kulX = ptrX + kwidth - keyx - 1;
			int kulY = ptrY - keyy;
			//
			// Clamp kernel top right	
			// Y should be clamping to 0.
			// X should clamp to image width
			//
			int imageWidth = src1Image->getWidth();
			if(kulX > imageWidth - 1)
			    kulX = imageWidth - 1;
			if(kulY < 0)
			    kulY = 0;
			Xil_float32* top_right = src1_scanline +
			    ((kulX - ptrX) * src1_pixel_stride) +
			    ((kulY - ptrY) * src1_scanline_stride);

			//
			// Do the cases where the kernel extends above or meets the top
			// of the image.
			//
			for(j = 0; j < ysize; j++) {
			    // point to the first pixel of the scanline 
			    Xil_float32* src1_pixel = src1_scanline;
			    Xil_float32* dest_pixel = dest_scanline;
			    for(i = 0; i < xsize; i++) {
				Xil_float32* dest = dest_pixel;
				
				double fsum = 0.0;
				float *kptr = kdata;

				Xil_float32* corner = top_right;
				Xil_float32* sptr;
				int kh;
				int kw;
				for(kh = 0; kh < keyy - ptrY - j; kh++) {
				    sptr = corner;
				    for(kw = keyx + imageWidth - ptrX - i; kw < kwidth; kw++) {
					fsum += (double)(*sptr) * (double)*(kptr + kh * kwidth + kw);
				    }
				    
				    for(kw = keyx + imageWidth - ptrX - i - 1; kw >= 0; kw--) {
					fsum += (double)(*sptr) * (double)*(kptr + kh * kwidth + kw);
					sptr -= src1_pixel_stride;
				    }
				}
				
				sptr = corner;
				for(kh = keyy - ptrY - j; kh < kheight; kh++) {
				    for(kw = keyx + imageWidth - ptrX - i; kw < kwidth; kw++) {
					fsum += (double)(*sptr) * (double)*(kptr + kh * kwidth + kw);
				    }
				    
				    for(kw = keyx + imageWidth - ptrX - i - 1; kw >= 0; kw--) {
					fsum += (double)(*sptr) * (double)*(kptr + kh * kwidth + kw);
					sptr -= src1_pixel_stride;
				    }
				    
				    sptr = (corner += src1_scanline_stride);
				}
				
				*dest = fsum;
				
				/* move to the next pixel */
				src1_pixel += src1_pixel_stride;
				dest_pixel += dest_pixel_stride;
			    }
			    
			    /* move to the next scanline */
			    src1_scanline += src1_scanline_stride;
			    dest_scanline += dest_scanline_stride;
			}
		    }
		    break;
		  case XIL_AREA_LEFT_EDGE:
		    //
		    //  Each Band...
		    //
		    for(band=0; band<nbands; band++) {
			unsigned int   src1_pixel_stride;
			unsigned int   src1_scanline_stride;
			Xil_float32* src1_data;
			src1_storage.getStorageInfo(band,
						    &src1_pixel_stride,
						    &src1_scanline_stride,
						    NULL,
						    (void**)&src1_data);
			
			unsigned int   dest_pixel_stride;
			unsigned int   dest_scanline_stride;
			Xil_float32* dest_data;
			dest_storage.getStorageInfo(band,
						    &dest_pixel_stride,
						    &dest_scanline_stride,
						    NULL,
						    (void**)&dest_data);
			
			Xil_float32* src1_scanline = src1_data +
			    + (x*src1_pixel_stride) + (y*src1_scanline_stride);

			Xil_float32* dest_scanline = dest_data +
			    + (x*dest_pixel_stride) + (y*dest_scanline_stride);

			int ptrX = src_box_x + x;
			int ptrY = src_box_y + y;
			int kulX = ptrX - keyx;
			int kulY = ptrY - keyy;
			//
			// Clamp kernel top left
			// Y should be clamping to 0.
			// X should clamp to 0
			//
			if(kulX < 0)
			    kulX = 0;
			if(kulY < 0)
			    kulY = 0;
			Xil_float32* kernel_start = src1_scanline +
			    ((kulX - ptrX) * src1_pixel_stride) +
			    ((kulY - ptrY) * src1_scanline_stride);
			
			for(j = 0; j < ysize; j++) {
			    // point to the first pixel of the scanline 
			    Xil_float32* dest_pixel = dest_scanline;
			    
			    for(i = 0; i < xsize; i++) {
				Xil_float32* dest = dest_pixel;
				
				double fsum = 0.0;
				float *kptr = kdata;

				Xil_float32* sptr = kernel_start;
				Xil_float32* sptr_save = sptr;
				int kh;
				int kw;
				for(kh = 0; kh < kheight; kh++) {
				    for(kw = 0; kw < keyx - ptrX - i; kw++) {
					fsum += (double)(*sptr) * (double)*(kptr + kh * kwidth + kw);
				    }
				    
				    for(kw = keyx - ptrX - i; kw < kwidth; kw++) {
					fsum += (double)(*sptr) * (double)*(kptr + kh * kwidth + kw);
					sptr += src1_pixel_stride;
				    }
				    
				    sptr = (sptr_save += src1_scanline_stride);
				}

				*dest = fsum;
				
				/* move to the next pixel */
				dest_pixel += dest_pixel_stride;
			    }
			    
			    /* move to the next scanline */
			    src1_scanline += src1_scanline_stride;
			    dest_scanline += dest_scanline_stride;
			    kernel_start += src1_scanline_stride;
			}
		    }
		    break;
		  case XIL_AREA_RIGHT_EDGE:
		    //
		    //  Each Band...
		    //
		    for(band=0; band<nbands; band++) {
			unsigned int   src1_pixel_stride;
			unsigned int   src1_scanline_stride;
			Xil_float32* src1_data;
			src1_storage.getStorageInfo(band,
						    &src1_pixel_stride,
						    &src1_scanline_stride,
						    NULL,
						    (void**)&src1_data);
	    
			unsigned int   dest_pixel_stride;
			unsigned int   dest_scanline_stride;
			Xil_float32* dest_data;
			dest_storage.getStorageInfo(band,
						    &dest_pixel_stride,
						    &dest_scanline_stride,
						    NULL,
						    (void**)&dest_data);
			
			Xil_float32* src1_scanline = src1_data +
			    (y*src1_scanline_stride) + (x*src1_pixel_stride);

			Xil_float32* dest_scanline = dest_data +
			    (y*dest_scanline_stride) + (x*dest_pixel_stride);

			int ptrX = src_box_x + x;
			int ptrY = src_box_y + y;
			int kulX = ptrX + kwidth - keyx - 1;
			int kulY = ptrY - keyy;
			//
			// Clamp kernel top right
			// Y should be clamping to 0.
			// X should clamp to image width
			//
			if(kulX > src1Image->getWidth() - 1)
			    kulX = src1Image->getWidth() - 1;
			if(kulY < 0)
			    kulY = 0;
			Xil_float32* kernel_start = src1_scanline +
			    ((kulX - ptrX) * src1_pixel_stride) +
			    ((kulY - ptrY) * src1_scanline_stride);

			for(j = 0; j < ysize; j++) {
			    // point to the first pixel of the scanline 
			    Xil_float32* dest_pixel = dest_scanline;
			    
			    for(i = 0; i < xsize; i++) {
				Xil_float32* dest = dest_pixel;
				
				double fsum = 0.0;
				float *kptr = kdata;

				Xil_float32* sptr = kernel_start;
				Xil_float32* sptr_save = sptr;
				int kh;
				int kw;
				for(kh = 0; kh < kheight; kh++) {
                                    for(kw = kulX - (ptrX-keyx) - i + 1; kw < kwidth; kw++) {
					fsum += (double)(*sptr) * (double)*(kptr + kh * kwidth + kw);
				    }
				    
                                    for(kw = kulX - (ptrX-keyx) - i; kw >= 0; kw--) {
					fsum += (double)(*sptr) * (double)*(kptr + kh * kwidth + kw);
					sptr -= src1_pixel_stride;
				    }
				    
				    sptr = (sptr_save += src1_scanline_stride);
				}
				*dest = fsum;
				
				/* move to the next pixel */
				dest_pixel += dest_pixel_stride;
			    }
			    
			    /* move to the next scanline */
			    src1_scanline += src1_scanline_stride;
			    dest_scanline += dest_scanline_stride;
			    kernel_start  += src1_scanline_stride;
			}
		    }
		    break;
		  case XIL_AREA_BOTTOM_LEFT_CORNER:
		    //
		    //  Each Band...
		    //
		    for(band=0; band<nbands; band++) {
			unsigned int   src1_pixel_stride;
			unsigned int   src1_scanline_stride;
			Xil_float32* src1_data;
			src1_storage.getStorageInfo(band,
						    &src1_pixel_stride,
						    &src1_scanline_stride,
						    NULL,
						    (void**)&src1_data);
	    
			unsigned int   dest_pixel_stride;
			unsigned int   dest_scanline_stride;
			Xil_float32* dest_data;
			dest_storage.getStorageInfo(band,
						    &dest_pixel_stride,
						    &dest_scanline_stride,
						    NULL,
						    (void**)&dest_data);
			
			Xil_float32* src1_scanline = src1_data +
			    (y*src1_scanline_stride) + (x*src1_pixel_stride);

			Xil_float32* dest_scanline = dest_data +
			    (y*dest_scanline_stride) + (x*dest_pixel_stride);

			int ptrX = src_box_x + x;
			int ptrY = src_box_y + y;
			int kulX = ptrX - keyx;
			int kulY = ptrY + kheight - keyy - 1;
			//
			// Clamp kernel lower_left
			// Y should be clamping to image height
			// X should clamp to 0
			//
			int imageHeight = src1Image->getHeight();
			if(kulX < 0)
			    kulX = 0;

			if(kulY > imageHeight - 1)
			    kulY = imageHeight - 1;

			Xil_float32* lower_left = src1_scanline +
			    ((kulX - ptrX) * src1_pixel_stride) +
			    ((kulY - ptrY) * src1_scanline_stride);

			//
			// Do the cases where the kernel extends above or meets the top
			// of the image.
			//
			for(j = 0; j < ysize; j++) {
			    // point to the first pixel of the scanline 
			    Xil_float32* src1_pixel = src1_scanline;
			    Xil_float32* dest_pixel = dest_scanline;
			    
			    for(i = 0; i < xsize; i++) {
				Xil_float32* dest = dest_pixel;
				
				double fsum = 0.0;
				float *kptr = kdata;

				Xil_float32* corner = lower_left;
				Xil_float32* sptr = corner;
				Xil_float32* sptr_save = corner;
				int kh;
				int kw;
				for(kh = keyy + imageHeight - ptrY - j - 1; kh >= 0; kh--) {
				    for(kw = 0; kw < keyx - ptrX - i; kw++) {
					fsum += (double)(*sptr) * (double)*(kptr + kh * kwidth + kw);
				    }
				    
				    for(kw = keyx - ptrX - i; kw < kwidth; kw++) {
					fsum += (double)(*sptr) * (double)*(kptr + kh * kwidth + kw);
					sptr += src1_pixel_stride;
				    }

				    sptr = (sptr_save -= src1_scanline_stride);
				}

				for(kh = keyy + imageHeight - ptrY - j; kh < kheight; kh++) {
					sptr = corner;
					for(kw = 0; kw < keyx - ptrX - i; kw++)
					    fsum += (double)(*sptr) * (double)*(kptr + kh * kwidth + kw);
					
					for(kw = keyx - ptrX - i; kw < kwidth; kw++) {
					    fsum += (double)(*sptr) * (double)*(kptr + kh * kwidth + kw);
					    sptr += src1_pixel_stride;
					}
				    }
				
				*dest = fsum;
				
				/* move to the next pixel */
				src1_pixel += src1_pixel_stride;
				dest_pixel += dest_pixel_stride;
			    }
			    
			    /* move to the next scanline */
			    src1_scanline += src1_scanline_stride;
			    dest_scanline += dest_scanline_stride;
			}
		    }
		    break; 
		  case XIL_AREA_BOTTOM_EDGE:
		    //
		    //  Each Band...
		    //
		    for(band=0; band<nbands; band++) {
			unsigned int   src1_pixel_stride;
			unsigned int   src1_scanline_stride;
			Xil_float32* src1_data;
			src1_storage.getStorageInfo(band,
						    &src1_pixel_stride,
						    &src1_scanline_stride,
						    NULL,
						    (void**)&src1_data);
	    
			unsigned int   dest_pixel_stride;
			unsigned int   dest_scanline_stride;
			Xil_float32* dest_data;
			dest_storage.getStorageInfo(band,
						    &dest_pixel_stride,
						    &dest_scanline_stride,
						    NULL,
						    (void**)&dest_data);
			
			Xil_float32* src1_scanline = src1_data +
			    (y*src1_scanline_stride) + (x*src1_pixel_stride);

			Xil_float32* dest_scanline = dest_data +
			    (y*dest_scanline_stride) + (x*dest_pixel_stride);

			int ptrX = src_box_x + x;
			int ptrY = src_box_y + y;
			int kulX = ptrX - keyx;
			int kulY = ptrY + kheight - keyy - 1;
			//
			// Clamp kernel lower left
			// Y should be clamping to image height
			// X should clamp to 0
			//
			if(kulX < 0)
			    kulX = 0;
			if(kulY > src1Image->getHeight() - 1)
			    kulY = src1Image->getHeight() - 1;
			Xil_float32* kernel_start = src1_scanline +
			    ((kulX - ptrX) * src1_pixel_stride) +
			    ((kulY - ptrY) * src1_scanline_stride);

			//
			// Do the cases where the kernel extends above or meets the top
			// of the image.
			//
			for(j = 0; j < ysize; j++) {
			    // point to the first pixel of the scanline 
			    Xil_float32* src1_pixel = src1_scanline;
			    Xil_float32* dest_pixel = dest_scanline;
			    Xil_float32* lower_left = kernel_start;
			    
			    for(i = 0; i < xsize; i++) {
				Xil_float32* dest = dest_pixel;
				
				double fsum = 0.0;
				float *kptr = kdata;

				Xil_float32* sptr = lower_left;
				Xil_float32* sptr_save = lower_left;
				int kh;
				int kw;
                                for(kh = kulY - (ptrY-keyy) - j; kh >= 0; kh--) {
				    for(kw = 0; kw < kwidth; kw++) {
					fsum += (double)(*sptr) * (double)*(kptr + kh * kwidth + kw);
					sptr += src1_pixel_stride;
				    }
				    sptr = (sptr_save -= src1_scanline_stride);
				}

                                for(kh = kulY - (ptrY-keyy) - j + 1; kh < kheight; kh++) {
				    sptr = lower_left;
				    for(kw = 0; kw < kwidth; kw++) {
					fsum += (double)(*sptr) * (double)*(kptr + kh * kwidth + kw);
					sptr += src1_pixel_stride;
				    }
				}

				*dest = fsum;
				
				/* move to the next pixel */
				src1_pixel += src1_pixel_stride;
				dest_pixel += dest_pixel_stride;
				lower_left += src1_pixel_stride;
			    }
			    
			    /* move to the next scanline */
			    src1_scanline += src1_scanline_stride;
			    dest_scanline += dest_scanline_stride;
			}
		    }
		    break;
		  case XIL_AREA_BOTTOM_RIGHT_CORNER:
		    //
		    //  Each Band...
		    //
		    for(band=0; band<nbands; band++) {
			unsigned int   src1_pixel_stride;
			unsigned int   src1_scanline_stride;
			Xil_float32* src1_data;
			src1_storage.getStorageInfo(band,
						    &src1_pixel_stride,
						    &src1_scanline_stride,
						    NULL,
						    (void**)&src1_data);
	    
			unsigned int   dest_pixel_stride;
			unsigned int   dest_scanline_stride;
			Xil_float32* dest_data;
			dest_storage.getStorageInfo(band,
						    &dest_pixel_stride,
						    &dest_scanline_stride,
						    NULL,
						    (void**)&dest_data);
			
			Xil_float32* src1_scanline = src1_data +
			    (y*src1_scanline_stride) + (x*src1_pixel_stride);

			Xil_float32* dest_scanline = dest_data +
			    (y*dest_scanline_stride) + (x*dest_pixel_stride);

			int ptrX = src_box_x + x;
			int ptrY = src_box_y + y;
			int kulX = ptrX + kwidth - keyx - 1;
			int kulY = ptrY + kheight - keyy - 1;

			//
			// Clamp kernel lower right
			// Y should be clamping to image height
			// X should clamp to to image width
			//
			int imageWidth = src1Image->getWidth();
			int imageHeight = src1Image->getHeight();
			if(kulX > imageWidth - 1)
			    kulX = imageWidth - 1;
			if(kulY > imageHeight - 1)
			    kulY = imageHeight - 1;
			Xil_float32* lower_right = src1_scanline +
			    ((kulX - ptrX) * src1_pixel_stride) +
			    ((kulY - ptrY) * src1_scanline_stride);
			//
			// Do the cases where the kernel extends above or meets the top
			// of the image.
			//
			for(j = 0; j < ysize; j++) {
			    // point to the first pixel of the scanline 
			    Xil_float32* src1_pixel = src1_scanline;
			    Xil_float32* dest_pixel = dest_scanline;
			    
			    for(i = 0; i < xsize; i++) {
				Xil_float32* dest = dest_pixel;
				
				double fsum = 0.0;
				float *kptr = kdata;

				Xil_float32* corner = lower_right;
				Xil_float32* sptr = corner;
				Xil_float32* sptr_save = corner;
				int kh;
				int kw;
				for(kh = keyy + imageHeight - ptrY - j - 1; kh>=0; kh--) {
				    for(kw = keyx + imageWidth - ptrX - i; kw < kwidth; kw++) {
					fsum += (double)(*sptr) * (double)*(kptr + kh * kwidth + kw);
				    }
				    
				    for(kw = keyx + imageWidth - ptrX - i - 1; kw >= 0; kw--) {
					fsum += (double)(*sptr) * (double)*(kptr + kh * kwidth + kw);
					sptr -= src1_pixel_stride;
				    }
				    sptr = (sptr_save -= src1_scanline_stride);
				}
				
				for(kh = keyy + imageHeight - ptrY - j; kh < kheight; kh++) {
				    sptr = corner;
				    for(kw =keyx + imageWidth - ptrX - i ; kw < kwidth; kw++){
					fsum += (double)(*sptr) * (double)*(kptr + kh * kwidth + kw);
				    }
					
				    for(kw =keyx + imageWidth - ptrX - i - 1; kw >= 0; kw--) {
					fsum += (double)(*sptr) * (double)*(kptr + kh * kwidth + kw);
					sptr -= src1_pixel_stride;
				    }
					
				}
				
				*dest = fsum;
				
				/* move to the next pixel */
				src1_pixel += src1_pixel_stride;
				dest_pixel += dest_pixel_stride;
			    }
			    
			    /* move to the next scanline */
			    src1_scanline += src1_scanline_stride;
			    dest_scanline += dest_scanline_stride;
			}
		    }
		    break;
		}
	    }
	} else {
	    //
	    //  The XIL_AREA_CENTER Case.
	    //

	    //
	    //  Test to see if all of our storage is of type XIL_PIXEL_SEQUENTIAL.
	    //  If so, implement an loop optimized for pixel-sequential storage.
	    //
	    if((src1_storage.isType(XIL_PIXEL_SEQUENTIAL)) &&
	       (dest_storage.isType(XIL_PIXEL_SEQUENTIAL))) {
		unsigned int   src1_pixel_stride;
		unsigned int   src1_scanline_stride;
		Xil_float32* src1_data;
		src1_storage.getStorageInfo(&src1_pixel_stride,
					    &src1_scanline_stride,
					    NULL, NULL,
					    (void**)&src1_data);
		unsigned int   dest_pixel_stride;
		unsigned int   dest_scanline_stride;
		Xil_float32* dest_data;
		dest_storage.getStorageInfo(&dest_pixel_stride,
					    &dest_scanline_stride,
					    NULL, NULL,
					    (void**)&dest_data);

                if(opt_type != NO_OPT) {
                    //
                    //  The optimized routines do not support rectangles
                    //  less than the kernel width/height.  We test here
                    //  which limits their usage to rectangles > the
                    //  kernel size.
                    //
                    {
                        XilRectList  rl(roi, dest_box);

                        int          x;
                        int          y;
                        unsigned int xsize;
                        unsigned int ysize;
                        while(rl.getNext(&x, &y, &xsize, &ysize) &&
                              opt_type != NO_OPT) {
                            if(xsize <= kwidth || ysize <= kheight) {
                                opt_type = NO_OPT;
                            }
                        }
                    }

                    if(nbands == 1) {
                        if(opt_type == KERNEL_SQUARE &&
                           src1_pixel_stride == 1 &&
                           dest_pixel_stride == 1) {
                            XilRectList  rl(roi, dest_box);

                            int          x;
                            int          y;
                            unsigned int xsize;
                            unsigned int ysize;
                            while(rl.getNext(&x, &y, &xsize, &ysize)) {
                                if(xili_convolve_square_opt_1(dest_data,
                                                              dest_scanline_stride,
                                                              dest_pixel_stride,
                                                              src1_data,
                                                              src1_scanline_stride,
                                                              src1_pixel_stride,
                                                              x, y, xsize, ysize,
                                                              kdata, kwidth) == XIL_FAILURE) {
                                    //
                                    //  Mark this box entry as having failed.
                                    //  If marking the box returns
                                    //  XIL_FAILURE, then we return
                                    //  XIL_FAILURE.
                                    //
                                    if(bl->markAsFailed() == XIL_FAILURE) {
                                        return XIL_FAILURE;
                                    } else {
                                        continue;
                                    }
                                }
                            }
                            continue;
                        } else if(opt_type == KERNEL_3X3) {
                            XilRectList  rl(roi, dest_box);

                            int          x;
                            int          y;
                            unsigned int xsize;
                            unsigned int ysize;
                            while(rl.getNext(&x, &y, &xsize, &ysize)) {
                                if(xili_convolve_3x3_opt_1(dest_data,
                                                           dest_scanline_stride,
                                                           dest_pixel_stride,
                                                           src1_data,
                                                           src1_scanline_stride,
                                                           src1_pixel_stride,
                                                           x, y, xsize, ysize,
                                                           kdata) == XIL_FAILURE) {
                                    //
                                    //  Mark this box entry as having failed.
                                    //  If marking the box returns
                                    //  XIL_FAILURE, then we return
                                    //  XIL_FAILURE.
                                    //
                                    if(bl->markAsFailed() == XIL_FAILURE) {
                                        return XIL_FAILURE;
                                    } else {
                                        continue;
                                    }
                                }
                            }
                            continue;
                        }
                    } else {
                        if(opt_type == KERNEL_3X3) {
                            XilRectList  rl(roi, dest_box);

                            int          x;
                            int          y;
                            unsigned int xsize;
                            unsigned int ysize;
                            while(rl.getNext(&x, &y, &xsize, &ysize)) {
                                for(unsigned int b = 0; b < nbands; b++) {
                                    if(xili_convolve_3x3_opt_1(dest_data + b,
                                                               dest_scanline_stride,
                                                               dest_pixel_stride,
                                                               src1_data + b,
                                                               src1_scanline_stride,
                                                               src1_pixel_stride,
                                                               x, y, xsize, ysize,
                                                               kdata) == XIL_FAILURE) {
                                        //
                                        //  Mark this box entry as having failed.
                                        //  If marking the box returns
                                        //  XIL_FAILURE, then we return
                                        //  XIL_FAILURE.
                                        //
                                        if(bl->markAsFailed() == XIL_FAILURE) {
                                            return XIL_FAILURE;
                                        } else {
                                            continue;
                                        }
                                    }
                                }
                            }
                            continue;
                        }
                    }
                }

                if(nbands == 1) {
                    //
                    //  Create a list of rectangles to loop over.  The
                    //  resulting list of rectangles is the area left by
                    //  intersecting the ROI with the destination box.
                    //
                    XilRectList rl(roi, dest_box);

                    int          x;
                    int          y;
                    unsigned int xsize;
                    unsigned int ysize;
                    while(rl.getNext(&x, &y, &xsize, &ysize)) {
                        Xil_float32* src1_scanline = src1_data +
                            (y*src1_scanline_stride) + (x*src1_pixel_stride);

                        Xil_float32* dest_scanline = dest_data +
                            (y*dest_scanline_stride) + (x*dest_pixel_stride);

                        Xil_float32* kernel_scanline  = src1_scanline -
                            (keyx * src1_pixel_stride) - (keyy * src1_scanline_stride);

                        do {
                            Xil_float32* src1_pixel   = src1_scanline;
                            Xil_float32* dest_pixel   = dest_scanline;
                            Xil_float32* kernel_pixel = kernel_scanline;
                            unsigned int  pixel_count = xsize;

                            do {
                                double  fsum               = 0.0;
                                float* kptr                = kdata;
                                Xil_float32* sptr_save     = kernel_pixel;
                                Xil_float32* sptr          = sptr_save;
                                unsigned int  src1_sstride = src1_scanline_stride;
                                unsigned int  src1_pstride = src1_pixel_stride;

                                for(unsigned int kh = kheight; kh != 0; kh--) {
                                    for(unsigned int kw = kwidth; kw != 0; kw--) {
                                        fsum += (double)(*sptr) * (double)*kptr;

                                        sptr += src1_pstride;
                                        kptr++;
                                    }


                                    sptr = (sptr_save += src1_sstride);
                                }

                                *dest_pixel = fsum;

                                src1_pixel   += src1_pstride;
                                dest_pixel   += dest_pixel_stride;
                                kernel_pixel += src1_pstride;
                            } while(--pixel_count);

                            src1_scanline   += src1_scanline_stride;
                            dest_scanline   += dest_scanline_stride;
                            kernel_scanline += src1_scanline_stride;
                        } while(--ysize);
                    }
                } else if(nbands == 3) {
                    //
                    //  Create a list of rectangles to loop over.  The resulting list
                    //  of rectangles is the area left by intersecting the ROI with
                    //  the destination box.
                    //
                    XilRectList rl(roi, dest_box);

                    int          x;
                    int          y;
                    unsigned int xsize;
                    unsigned int ysize;
                    while(rl.getNext(&x, &y, &xsize, &ysize)) {
                        Xil_float32* src1_scanline = src1_data +
                            (y*src1_scanline_stride) + (x*src1_pixel_stride);

                        Xil_float32* dest_scanline = dest_data +
                            (y*dest_scanline_stride) + (x*dest_pixel_stride);

                        Xil_float32* kernel_scanline  = src1_scanline -
                            (keyx * src1_pixel_stride) - (keyy * src1_scanline_stride);

                        do {
                            Xil_float32* src1_pixel   = src1_scanline;
                            Xil_float32* dest_pixel   = dest_scanline;
                            Xil_float32* kernel_pixel = kernel_scanline;
                            unsigned int pixel_count  = xsize;

                            do {
                                double fsum0              = 0.0;
                                double fsum1              = 0.0;
                                double fsum2              = 0.0;
                                float* kptr               = kdata;
                                Xil_float32* sptr_save    = kernel_pixel;
                                Xil_float32* sptr         = sptr_save;
                                unsigned int src1_sstride = src1_scanline_stride;
                                unsigned int src1_pstride = src1_pixel_stride;

                                for(unsigned int kh = kheight; kh != 0; kh--) {
                                    for(unsigned int kw = kwidth; kw != 0; kw--) {
                                        fsum0 += (double)(*sptr)     * (double)*kptr;
                                        fsum1 += (double)(*(sptr+1)) * (double)*kptr;
                                        fsum2 += (double)(*(sptr+2)) * (double)*kptr;

                                        sptr  += src1_pstride;
                                        kptr++;
                                    }

                                    sptr_save += src1_sstride;
                                    sptr       = sptr_save;
                                }

                                *dest_pixel     = fsum0;
                                *(dest_pixel+1) = fsum1;
                                *(dest_pixel+2) = fsum2;

                                src1_pixel   += src1_pstride;
                                dest_pixel   += dest_pixel_stride;
                                kernel_pixel += src1_pstride;
                            } while(--pixel_count);

                            src1_scanline   += src1_scanline_stride;
                            dest_scanline   += dest_scanline_stride;
                            kernel_scanline += src1_scanline_stride;
                        } while(--ysize);
                    }
                } else {
		    //
		    //	Create a list of rectangles to loop over.  The resulting list
		    //	of rectangles is the area left by intersecting the ROI with
		    //	the destination box.
		    //
		    XilRectList	   rl(roi, dest_box);

		    int		   x;
		    int		   y;
		    unsigned int   xsize;
		    unsigned int   ysize;
		    while(rl.getNext(&x, &y, &xsize, &ysize)) {
		        Xil_float32* src1_scanline = src1_data +
			    (y*src1_scanline_stride) + (x*src1_pixel_stride);
		    
		        Xil_float32* dest_scanline = dest_data +
			    (y*dest_scanline_stride) + (x*dest_pixel_stride);
		    
		        Xil_float32* kernel_scanline  = src1_scanline -
			    (keyx * src1_pixel_stride) - (keyy * src1_scanline_stride);

		        do {
			    Xil_float32* src1_pixel   = src1_scanline;
			    Xil_float32* dest_pixel   = dest_scanline;
			    Xil_float32* kernel_pixel = kernel_scanline;
			    unsigned int  pixel_count = xsize;
			
			    do {
			        Xil_float32* src1_band   = src1_pixel;
			        Xil_float32* dest_band   = dest_pixel;
			        Xil_float32* kernel_band = kernel_pixel;
			        unsigned int band_count  = nbands;

			        do {
				    double fsum = 0.0;
				    float* kptr = kdata;
				    Xil_float32* sptr_save = kernel_band;
				    Xil_float32* sptr = sptr_save;
				
				    for(int kh = 0; kh < kheight; kh++) {
				        for(int kw = 0; kw < kwidth; kw++) {
					    fsum += (double)(*sptr) * (double)*kptr;
					    sptr += src1_pixel_stride;
					    kptr++;
				        }
				        sptr = (sptr_save += src1_scanline_stride);
				    }
				
				    *dest_band = fsum;
				
				    kernel_band++;
				    src1_band++;
				    dest_band++;
			        } while(--band_count);
                            
			        src1_pixel += src1_pixel_stride;
			        dest_pixel += dest_pixel_stride;
			        kernel_pixel += src1_pixel_stride;
			    } while(--pixel_count);
                        
			    src1_scanline += src1_scanline_stride;
			    dest_scanline += dest_scanline_stride;
			    kernel_scanline += src1_scanline_stride;
		        } while(--ysize);
		    }
		}
	    } else {
		//
		// General Storage Implementation.
		//
		XilRectList  rl(roi, dest_box);
		
		int          x;
		int          y;
		unsigned int xsize;
		unsigned int ysize;
		while(rl.getNext(&x, &y, &xsize, &ysize)) {
		    //
		    //  Each Band...
		    //
		    for(unsigned int band=0; band<nbands; band++) {
			unsigned int   src1_pixel_stride;
			unsigned int   src1_scanline_stride;
			Xil_float32* src1_data;
			src1_storage.getStorageInfo(band,
						    &src1_pixel_stride,
						    &src1_scanline_stride,
						    NULL,
						    (void**)&src1_data);

			unsigned int   dest_pixel_stride;
			unsigned int   dest_scanline_stride;
			Xil_float32* dest_data;
			dest_storage.getStorageInfo(band,
						    &dest_pixel_stride,
						    &dest_scanline_stride,
						    NULL,
						    (void**)&dest_data);
			
			Xil_float32* src1_scanline = src1_data +
			    (y*src1_scanline_stride) + (x*src1_pixel_stride);
			
			Xil_float32* dest_scanline = dest_data +
			    (y*dest_scanline_stride) + (x*dest_pixel_stride);
			
			unsigned int scanline_count = ysize;
			
			Xil_float32* kernel_scanline  = src1_scanline -
			    (keyx * src1_pixel_stride) - (keyy * src1_scanline_stride);
			//
			//  Each Scanline...
			//
			do {
			    Xil_float32* src1_pixel   = src1_scanline;
			    Xil_float32* dest_pixel   = dest_scanline;
			    Xil_float32* kernel_pixel = kernel_scanline;
			    unsigned int pixel_count  = xsize;
			    
			    //
			    //  Each Pixel...
			    //
			    do {
				double fsum = 0.0;
				float* kptr = kdata;
				Xil_float32* sptr = kernel_pixel;
				Xil_float32* sptr_save  = sptr;
				
				for(int kh = 0; kh < kheight; kh++) {
				    for(int kw = 0; kw < kwidth; kw++) {
					fsum += (double)(*sptr) * (double)*kptr;
					sptr += src1_pixel_stride;
					kptr++;
				    }
				    sptr = (sptr_save += src1_scanline_stride);
				}
				
				*dest_pixel  = fsum;

				src1_pixel   += src1_pixel_stride;
				dest_pixel   += dest_pixel_stride;
				kernel_pixel += src1_pixel_stride;
			    } while(--pixel_count);
			    
			    src1_scanline   += src1_scanline_stride;
			    dest_scanline   += dest_scanline_stride;
			    kernel_scanline += src1_scanline_stride;
			} while(--scanline_count);
		    }
		}
	    }
	}
    }

    return XIL_SUCCESS;
}


//
// This is a generalized convolve for square kernels with the key at the center.
// It also requires that the region being convolved is as large as the kernel.
//
// This function handles the edge extension while maintaining a simple left to
// right, top down processing order. Doing the edge extension inline allows
// better use of memory and disk cache.
//
// The basic approach is a forward rendering model allowin each source pixel to
// be read only once. All of the math is done in floating point because most
// FPUs can pipeline their multiplies allowing one multiply per processor cycle.
// This code attempts to reduce the problem to one of multiplying a kernel by
// a source pixel and accumulating the results into a buffer allowing the
// code to stay relatively simple while approaching a near-optimal algorithm.
//
// THE ACCUMULATION BUFFER
//    Obviously it is not practical for the accumlation buffer (called "result"
//    throughout the code) to be as large as the image since that be very
//    wasteful of memory. All that is really required is a buffer which is as
//    wide as the image and as tall as the kernel. The scanlines in the buffer
//    are reused by wrapping around whenever the bottom is reached. This
//    leaves the problem of the bottom part of a kernel needing to wrap around
//    to the top of the accumulation buffer. Rather than having logic in the
//    accumulation portion of the code to handle wrapping two copies of the
//    kernel are put end to end so that the kernel data can wrap.
//
//  Double kernel
//     0 -1  0
//    -1  5 -1    Wrapped kernel
//     0 -1  0 <->  0 -1  0
//     0 -1  0 <->  0 -1  0
//    -1  5 -1 <-> -1  5 -1
//     0 -1  0
//
//    The only time it is necessary to wrap during accumulation is on the bottom
//    edge where the entire kernel isn't used yet it may still straddle the
//    bottom edge of the accumulation buffer.
//
// EDGE HANDLING
//    Horizontal edges are handled by having separate loops for use of the
//    edge pixels, simply repeating the pixel when necessary. Vertical edges
//    are handled by reusing entire scanlines when necessary.
//
//
// TODO: 7/12/96 jlf  Updated the comments or acutally use this routine's edge
//                    conditions for full images.
//
//
   
//
//  Multiply a pixel by a kernel and add the results to a buffer
//
static
inline
void
madd(double       pixel,
     double*      kernel,
     int          ksize,
     int          rsize,
     int          width,
     int          height,
     double*      results)
{
    int x,y;

    for(y=0; y<height; y++) {
        for(x=0; x<width; x++) {
            results[x] += pixel * kernel[x];
        }

        results+=rsize;
        kernel+=ksize;
    }
}

//
// This version of madd does 4 pixels at once. This reduces the number of reads
// and writes because the contribution of each of the 4 pixels can be added before
// the results are written.
//
static
inline
void
madd4(Xil_float32*  pixels,
      unsigned int  pstride,
      double*       kernel,
      int           ksize,
      int           rsize,
      int           width,
      int           height,
      double*       results)
{
    int x,y;

    double fpixel1= (double)pixels[0];
    double fpixel2= (double)pixels[pstride];
    double fpixel3= (double)pixels[pstride * 2];
    double fpixel4= (double)pixels[pstride * 3];
    double k;
    double results1;
    double results2;
    double results3;
    double results4;

    for(y=0; y<height; y++) {
        results1= results[0];
        results2= results[1];
        results3= results[2];

        //
        // this is four copies of the same operation except that the results variables
        // rotate through the four positions in the operation to allow the least
        // amount of data movement.
        //
        for(x=0; x< (width>>2); x++) {
            results4= results[3];

            k= kernel[0];
            results1  += fpixel1 * k;
            results2  += fpixel2 * k;
            results3  += fpixel3 * k;
            results4  += fpixel4 * k;

            results[0]= results1;
            results1= results[4];

            k= kernel[1];
            results2  += fpixel1 * k;
            results3  += fpixel2 * k;
            results4  += fpixel3 * k;
            results1  += fpixel4 * k;

            results[1]= results2;
            results2= results[5];

            k= kernel[2];
            results3  += fpixel1 * k;
            results4  += fpixel2 * k;
            results1  += fpixel3 * k;
            results2  += fpixel4 * k;

            results[2]= results3;
            results3= results[6];

            k= kernel[3];
            results4  += fpixel1 * k;
            results1  += fpixel2 * k;
            results2  += fpixel3 * k;
            results3  += fpixel4 * k;

            results[3]= results4;

            results+= 4;
            kernel +=4;
        }

        //
        // The cleanup of the loop is large but not very complex.
        // Each case handles different fractional amounts of
        // the loop.
        //
        switch(width & 3) {
          case 3:
            results4= results[3];

            k= kernel[0];
            results1  += fpixel1 * k;
            results2  += fpixel2 * k;
            results3  += fpixel3 * k;
            results4  += fpixel4 * k;

            results[0]= results1;
            results1= results[4];

            k= kernel[1];
            results2  += fpixel1 * k;
            results3  += fpixel2 * k;
            results4  += fpixel3 * k;
            results1  += fpixel4 * k;

            results[1]= results2;
            results2= results[5];

            k= kernel[2];
            results3  += fpixel1 * k;
            results4  += fpixel2 * k;
            results1  += fpixel3 * k;
            results2  += fpixel4 * k;

            results[2]= results3;
            results[3]= results4;
            results[4]= results1;
            results[5]= results2;

            results+= 3;
            kernel += 3;

            break;

          case 2:
            results4= results[3];

            k= kernel[0];
            results1  += fpixel1 * k;
            results2  += fpixel2 * k;
            results3  += fpixel3 * k;
            results4  += fpixel4 * k;

            results[0]= results1;
            results1= results[4];

            k= kernel[1];
            results2  += fpixel1 * k;
            results3  += fpixel2 * k;
            results4  += fpixel3 * k;
            results1  += fpixel4 * k;

            results[1]= results2;
            results[2]= results3;
            results[3]= results4;
            results[4]= results1;

            results+= 2;
            kernel += 2;

            break;

          case 1:
            results4= results[3];

            k= kernel[0];
            results1  += fpixel1 * k;
            results2  += fpixel2 * k;
            results3  += fpixel3 * k;
            results4  += fpixel4 * k;
            results[0]= results1;
            results[1]= results2;
            results[2]= results3;
            results[3]= results4;

            results+= 1;
            kernel += 1;

            break;

          case 0:
            results[0]= results1;
            results[1]= results2;
            results[2]= results3;

            break;
        }

        results=results + rsize - width;
        kernel=kernel + ksize - width;
    }
}

//
//  The results are rounded and saved in a separate pass. The assumption
//  is that since the kernel is at least 5x5 that an extra read and write
//  is not considerable compared to the cost of the math. This function
//  also automatically zeros the row out for reuse.
//
static
inline
void
finish(double*       src,
       Xil_float32*  dst,
       unsigned int  dst_pstride,
       int           count)
{
    int i;

    for(i=0; i<count; i++) {
        *dst = src[i];
        dst += dst_pstride;

        src[i] = 0.0F;
    }
}

static
XilStatus
xili_convolve_square_opt_1(Xil_float32*  dst_data,
                           unsigned int  dst_sstride,
                           unsigned int  dst_pstride,
                           Xil_float32*  src_data,
                           unsigned int  src_sstride,
                           unsigned int  src_pstride,
                           int           roi_x,
                           int           roi_y,
                           int           roi_xsize,
                           int           roi_ysize,
                           float*        kdata,
                           int           ksize)
{
    double* results;              // a pointer to the beginning of the result buffer
    double* resultsend;           // a pointer to the end of the result buffer
    double* resultptr;            // a ptr to the area to add the pixel contribution
    double* finishedptr;          // points to the results row which has been finished
    unsigned int half_ksize   = ksize/2;        // for performance and convenience
    unsigned int ksize_sqrd   = ksize*ksize;    // for performance and convenience
    unsigned int src_4pstride = src_pstride*4; // for performance and convenience

    //
    //  Fix the source image to point at the first pixel within the ROI
    //
    src_data = src_data + (roi_y * src_sstride) + (roi_x * src_pstride);
    dst_data = dst_data + (roi_y * dst_sstride) + (roi_x * dst_pstride);

    //
    //  The results buffer is used to accumulate the contribution of each
    //  source pixel. It only needs to be ksize pixels high since the
    //  scanlines can be reused.
    //
    results = new double[roi_xsize*ksize];
    if(results == NULL) {
        return XIL_FAILURE;
    }

    resultsend = results + roi_xsize*ksize;
    xili_memset(results, 0, roi_xsize*ksize*sizeof(double));

    //
    //  Create a buffer which has two copies of the kernel so that a wrapped
    //  version of it with any phase can be used
    //
    double* localkernel = new double[ksize_sqrd<<1];
    if(localkernel == NULL) {
        delete [] results;
        return XIL_FAILURE;
    }

    for(int i=0; i<ksize_sqrd; i++) {
        int j = ksize_sqrd - i - 1;
        localkernel[j]              = (double)kdata[i];
        localkernel[ksize_sqrd + j] = (double)kdata[i];
    }

    //
    //  Point to the start of the second copy of the kernel, rather then
    //  creaping down through resultptr I'll creap up through the kernel.
    //  That way I won't need to worry about wrapping around in the middle
    //  of madd()
    //
    finishedptr = results;

    //
    //  Point to the start of the second copy of the kernel, rather then
    //  creaping down through resultptr I'll creap up through the kernel.
    //  That way I won't need to worry about wrapping around in the middle
    //  of madd()
    //
    double* kernelptr = localkernel+ksize_sqrd-1;

    //
    //  The middle section doesn't need to deal with top or bottom edges
    //
    unsigned int size4      = (roi_xsize-ksize+1)>>2;
    unsigned int remaining  = ((roi_xsize-ksize+1)&3);
    Xil_float32* srcptr;
    Xil_float32* dstptr     = dst_data;

    //
    //  Point to the upper left corner of the source data contributing to the
    //  result.  Pixels outside of the src ROI can be used.
    //
    src_data = src_data - (half_ksize * src_sstride) - (half_ksize * src_pstride);

    unsigned int y;
    for(y=1; y < ksize; y++) {
        //
        //  Start with the upper left corner of the results buffer
        //
        srcptr    = src_data;
        resultptr = results;

        unsigned int x;
        for(x=1; x<ksize; x++) {
            madd((double)*srcptr, kernelptr--, ksize, roi_xsize, x, y, resultptr);
            srcptr += src_pstride;
        }

        for(x=0; x < (roi_xsize-ksize+1); x++) {
            madd((double)*srcptr, kernelptr, ksize, roi_xsize, ksize, y, resultptr++);
            srcptr += src_pstride;
        }

        for(x=(ksize-1); x>0; x--) {
            madd((double)*srcptr, kernelptr, ksize, roi_xsize, x, y, resultptr++);
            srcptr += src_pstride;
        }

        kernelptr -= 1;

        src_data += src_sstride;
    }

    kernelptr = localkernel+ksize_sqrd;

    for(y=roi_ysize; y != 0; y--) {
        //
        //  Start with the upper left corner of the results buffer
        //
        srcptr    = src_data;
        resultptr = results;

        unsigned int x;

        //
        //  Point the the rightmost column of the kernel. More and more of the
        //  right side of the kernel will be used as we move right through the
        //  source data
        //
        kernelptr += ksize-1;

        //
        //  Left portion kernel usage...
        //
        for(x=1; x < ksize; x++) {
            madd((double)*srcptr, kernelptr--, ksize, roi_xsize, x, ksize, resultptr);
            srcptr += src_pstride;
        }

        //
        //  The center portion uses the entire kernel. We'll do the operation
        //  4 pixels at I time when we can
        //
        for(x=size4; x != 0; x--) {
            madd4(srcptr, src_pstride, kernelptr, ksize,
                  roi_xsize, ksize, ksize, resultptr);

            srcptr   += src_4pstride;
            resultptr+= 4;
        }

        for(x=remaining; x != 0; x--) {
            madd((double)*srcptr, kernelptr, ksize, roi_xsize, ksize, ksize, resultptr++);
            srcptr += src_pstride;
        }

        //
        //  Right portion kernel usage...
        //
        for(x=(ksize-1); x>0; x--) {
            madd((double)*srcptr, kernelptr, ksize, roi_xsize, x, ksize, resultptr++);
            srcptr += src_pstride;
        }

        //
        //  Finish up the convolve (clipping and rounding) and move on to the
        //  next scanline. Wrap back to the beginning of the results buffer.
        //
        finish(finishedptr, dstptr, dst_pstride, roi_xsize);
        finishedptr += roi_xsize;

        if(finishedptr == resultsend) {
            finishedptr = results;
        }

        //
        //  Move up through the double copy of the kernel. Wrap back to the
        //  middle (the end of the first copy)
        //
        kernelptr -= ksize;
        if(kernelptr == localkernel) {
            kernelptr += ksize_sqrd;
        }
 
        dstptr   += dst_sstride;
        src_data += src_sstride;
    }

    //
    //  Free all allocated memory
    //
    delete [] results;
    delete [] localkernel;

    return XIL_SUCCESS;
}

//
// This is a special case convolve for 3x3 kernels with the key at the center.
//
// This function handles the edge extension while maintaining a simple left to
// right, top down processing order. Doing the edge extension inline allows better
// use of memory and disk cache.
//
// It uses a forward rendering model which doesn't do any intermediate storing.
// A band of 6 source scanlines are used to produce 4 scanlines of destination.
// This means that half of the scanlines are read twice and the other half are
// read only once.
//
// All of the math is done in floating point because most FPUs can pipeline
// multiplies allowing one multiply per processor cycle.
//
// The results are accumulated into 12 registers. The registers are quickly
// reused as soon as they are saved so that results for the destination pixels
// are computed in the registers like this:
//
//     r1a r1b r1c r1a r1b r1c r1a r1b r1c ...
//     r2a r2b r2c r2a r2b r2c r2a r2b r2c ...
//     r3a r3b r3c r3a r3b r3c r3a r3b r3c ...
//     r4a r4b r4c r4a r4b r4c r4a r4b r4c ...
//
// The main loop finishes a 3x4 chunk of destination pixels using partial results
// from the last time through the loop and producing partial results for the next
// time through the loop. This requires loop setup and teardown. The loop exits
// when there are only two more columns of results to produce regardless of where
// that is in the loop, moving the data around as necessary to put the partial
// results into standard registers for the cleanup code.
//
// REGISTER USAGE
//
//   9 for the kernel
//   12 for the results
//   3 temps to make things flow better
//   1 for the source pixel
//
// POSSIBLE IMPROVEMENTS
//
//   A basic way to improve the performance of this code without making
//   major changes is to increase the number of scanlines that are processed
//   at a time to reduce the number of scanlines which are read more than
//   once. Without convincing the compiler to make better use of the floating
//   point registers more registers can't really be used. By reusing the
//   registers more quickly is may be possible to use the same 12 result registers
//   to produce 5 scanlines of results rather than 4 which would mean that source
//   pixels would be read an average of 1.4 times rather than 1.5 times which
//   probably wouldn't help very much given the amount of computation involved.
//
//
///////////////////////////////////////////////////////////////////////////
//
//  1-banded abitrary pixel-stride images
//
///////////////////////////////////////////////////////////////////////////
static
XilStatus
xili_convolve_3x3_opt_1(Xil_float32*  dst_data,
                        unsigned int  dst_sstride,
                        unsigned int  dst_pstride,
                        Xil_float32*  src_data,
                        unsigned int  src_sstride,
                        unsigned int  src_pstride,
                        int           roi_x,
                        int           roi_y,
                        int           roi_xsize,
                        int           roi_ysize,
                        float*        kdata)
{
    int x,y;

    unsigned int drow1= dst_sstride;
    unsigned int drow2= drow1+dst_sstride;
    unsigned int drow3= drow2+dst_sstride;

    //
    //  Load the kernel -- remember XIL 1.3 inverts it for us already.
    //
    float k11 = kdata[8];
    float k21 = kdata[7];
    float k31 = kdata[6];
    float k12 = kdata[5];
    float k22 = kdata[4];
    float k32 = kdata[3];
    float k13 = kdata[2];
    float k23 = kdata[1];
    float k33 = kdata[0];

    src_data = src_data + (roi_y - 1) * src_sstride + (roi_x - 1) * src_pstride;
    dst_data = dst_data + roi_y * dst_sstride + roi_x * dst_pstride;

    unsigned int dst_skip= dst_sstride - roi_xsize * dst_pstride;
    unsigned int src_skip= src_sstride - (roi_xsize + 2) * src_pstride;

    float r1a,r1b,r1c;
    float r2a,r2b,r2c;
    float r3a,r3b,r3c;
    float r4a,r4b,r4c;

    unsigned int ycount= (roi_ysize>>2);
    for(y=ycount; y!=0; y--) {
        unsigned int src_next = src_skip + 3 * src_sstride;

        unsigned int srow1 = src_sstride;
        unsigned int srow2 = srow1+src_sstride;
        unsigned int srow3 = srow2+src_sstride;
        unsigned int srow4 = srow3+src_sstride;
        unsigned int srow5 = srow4+src_sstride;

        float inA;
        float temp1,temp2,temp3;

        //
        //  Pass 2
        //
        inA= src_data[0];

        temp3 = inA * k33;
        r1a  = temp3;

        inA= src_data[srow1];

        temp3 = inA * k32;
        r1a += temp3;

        temp3 = inA * k33;
        r2a  = temp3;

        inA= src_data[srow2];
 
        temp3 = inA * k31;
        r1a += temp3;
 
        temp3 = inA * k32; 
        r2a += temp3; 
 
        temp3 = inA * k33; 
        r3a  = temp3; 
 
        inA= src_data[srow3]; 
  
        temp3 = inA * k31; 
        r2a += temp3; 
  
        temp3 = inA * k32;  
        r3a += temp3;
  
        temp3 = inA * k33;
        r4a  = temp3;

        inA= src_data[srow4];

        temp3 = inA * k31;  
        r3a += temp3;  
   
        temp3 = inA * k32;   
        r4a += temp3;   
   
        inA= src_data[srow5];
    
        temp3 = inA * k31;   
        r4a += temp3;   
    
        src_data += src_pstride;

        //
        //  pass 3 (if the region is only 1 pixel wide then this does
        //  unecessary work that is an extrem case)
        //
        inA= src_data[0];

        temp2= inA * k23; temp3 = inA * k33;
        r1a += temp2; r1b = temp3;

        inA= src_data[srow1];

        temp2= inA * k22; temp3 = inA * k32;
        r1a += temp2; r1b += temp3;

        temp2= inA * k23; temp3 = inA * k33;
        r2a += temp2; r2b  = temp3;

        inA= src_data[srow2];
 
        temp2= inA * k21; temp3 = inA * k31;
        r1a += temp2; r1b += temp3;
 
        temp2= inA * k22; temp3 = inA * k32; 
        r2a += temp2; r2b += temp3; 
 
        temp2= inA * k23; temp3 = inA * k33; 
        r3a += temp2; r3b  = temp3; 
 
        inA= src_data[srow3];

        temp2= inA * k21; temp3 = inA * k31; 
        r2a += temp2; r2b += temp3; 
  
        temp2= inA * k22; temp3 = inA * k32;  
        r3a += temp2; r3b += temp3;  
  
        temp2= inA * k23; temp3 = inA * k33;
        r4a += temp2; r4b  = temp3;

        inA= src_data[srow4];
   
        temp2= inA * k21; temp3 = inA * k31;  
        r3a += temp2; r3b += temp3;  
   
        temp2= inA * k22; temp3 = inA * k32;   
        r4a += temp2; r4b += temp3;   
   
        inA= src_data[srow5];
    
        temp2= inA * k21; temp3 = inA * k31;   
        r4a += temp2; r4b += temp3;   
    
        src_data += src_pstride;

        //
        // Each pass through the loop produces a 3 x 4 block of destination
        // pixels
        //
        x= roi_xsize;
        while(1) {
            //
            // Pass 1
            //
            if(x==0) break;

            inA= src_data[0];

            temp1 = inA * k13; temp2= inA * k23; temp3 = inA * k33;
            r1a += temp1; r1b += temp2; r1c  = temp3;

            inA= src_data[srow1];

            temp1 = inA * k12; temp2= inA * k22; temp3 = inA * k32;
            r1a += temp1; r1b += temp2; r1c += temp3;

            temp1 = inA * k13; temp2= inA * k23; temp3 = inA * k33;
            r2a += temp1; r2b += temp2; r2c  = temp3;

            inA= src_data[srow2];
 
            temp1 = inA * k11; temp2= inA * k21; temp3 = inA * k31;
            r1a += temp1; r1b += temp2; r1c += temp3;
 
            dst_data[0]= (r1a);

            temp1 = inA * k12; temp2= inA * k22; temp3 = inA * k32; 
            r2a += temp1; r2b += temp2; r2c += temp3; 
 
            temp1 = inA * k13; temp2= inA * k23; temp3 = inA * k33; 
            r3a += temp1; r3b += temp2; r3c  = temp3; 
 
            inA= src_data[srow3]; 
  
            temp1 = inA * k11; temp2= inA * k21; temp3 = inA * k31; 
            r2a += temp1; r2b += temp2; r2c += temp3; 
  
            dst_data[drow1]= (r2a);

            temp1 = inA * k12; temp2= inA * k22; temp3 = inA * k32;  
            r3a += temp1; r3b += temp2; r3c += temp3;  
  
            temp1 = inA * k13; temp2= inA * k23; temp3 = inA * k33;
            r4a += temp1; r4b += temp2; r4c  = temp3;

            inA= src_data[srow4];
   
            temp1 = inA * k11; temp2= inA * k21; temp3 = inA * k31;  
            r3a += temp1; r3b += temp2; r3c += temp3;  
   
            dst_data[drow2]= (r3a);

            temp1 = inA * k12; temp2= inA * k22; temp3 = inA * k32;   
            r4a += temp1; r4b += temp2; r4c += temp3;   
   
            inA= src_data[srow5];
    
            temp1 = inA * k11; temp2= inA * k21; temp3 = inA * k31;   
            r4a += temp1; r4b += temp2; r4c += temp3;   
    
            dst_data[drow3]= (r4a);

            src_data += src_pstride;
            dst_data += dst_pstride;
            x--;

            //
            //  Pass 2
            //
            if(x==0) break;

            inA= src_data[0];

            temp1 = inA * k13; temp2= inA * k23; temp3 = inA * k33;
            r1b += temp1; r1c += temp2; r1a  = temp3;

            inA= src_data[srow1];

            temp1 = inA * k12; temp2= inA * k22; temp3 = inA * k32;
            r1b += temp1; r1c += temp2; r1a += temp3;

            temp1 = inA * k13; temp2= inA * k23; temp3 = inA * k33;
            r2b += temp1; r2c += temp2; r2a  = temp3;

            inA= src_data[srow2];
 
            temp1 = inA * k11; temp2= inA * k21; temp3 = inA * k31;
            r1b += temp1; r1c += temp2; r1a += temp3;
 
            dst_data[0]= (r1b);

            temp1 = inA * k12; temp2= inA * k22; temp3 = inA * k32; 
            r2b += temp1; r2c += temp2; r2a += temp3; 
 
            temp1 = inA * k13; temp2= inA * k23; temp3 = inA * k33; 
            r3b += temp1; r3c += temp2; r3a  = temp3; 
 
            inA= src_data[srow3]; 
  
            temp1 = inA * k11; temp2= inA * k21; temp3 = inA * k31; 
            r2b += temp1; r2c += temp2; r2a += temp3; 
  
            dst_data[drow1]= (r2b);

            temp1 = inA * k12; temp2= inA * k22; temp3 = inA * k32;  
            r3b += temp1; r3c += temp2; r3a += temp3;
  
            temp1 = inA * k13; temp2= inA * k23; temp3 = inA * k33;
            r4b += temp1; r4c += temp2; r4a  = temp3;

            inA= src_data[srow4];

            temp1 = inA * k11; temp2= inA * k21; temp3 = inA * k31;  
            r3b += temp1; r3c += temp2; r3a += temp3;  
   
            dst_data[drow2]= (r3b);

            temp1 = inA * k12; temp2= inA * k22; temp3 = inA * k32;   
            r4b += temp1; r4c += temp2; r4a += temp3;   
   
            inA= src_data[srow5];
    
            temp1 = inA * k11; temp2= inA * k21; temp3 = inA * k31;   
            r4b += temp1; r4c += temp2; r4a += temp3;   
    
            dst_data[drow3]= (r4b);

            src_data += src_pstride;
            dst_data += dst_pstride;
            x--;

            //
            //  Pass 3
            //
            if(x==0) break;

            inA= src_data[0];

            temp1 = inA * k13; temp2= inA * k23; temp3 = inA * k33;
            r1c += temp1; r1a += temp2; r1b  = temp3;

            inA= src_data[srow1];

            temp1 = inA * k12; temp2= inA * k22; temp3 = inA * k32;
            r1c += temp1; r1a += temp2; r1b += temp3;

            temp1 = inA * k13; temp2= inA * k23; temp3 = inA * k33;
            r2c += temp1; r2a += temp2; r2b  = temp3;

            inA= src_data[srow2];
 
            temp1 = inA * k11; temp2= inA * k21; temp3 = inA * k31;
            r1c += temp1; r1a += temp2; r1b += temp3;
 
            dst_data[0]= (r1c);

            temp1 = inA * k12; temp2= inA * k22; temp3 = inA * k32; 
            r2c += temp1; r2a += temp2; r2b += temp3; 
 
            temp1 = inA * k13; temp2= inA * k23; temp3 = inA * k33; 
  
            r3c += temp1; r3a += temp2; r3b  = temp3; 
 
            inA= src_data[srow3];
  
            temp1 = inA * k11; temp2= inA * k21; temp3 = inA * k31; 
            r2c += temp1; r2a += temp2; r2b += temp3; 

            dst_data[drow1]= (r2c);

            temp1 = inA * k12; temp2= inA * k22; temp3 = inA * k32;  
            r3c += temp1; r3a += temp2; r3b += temp3;  
  
            temp1 = inA * k13; temp2= inA * k23; temp3 = inA * k33;
            r4c += temp1; r4a += temp2; r4b  = temp3;

            inA= src_data[srow4];
   
            temp1 = inA * k11; temp2= inA * k21; temp3 = inA * k31;  
            r3c += temp1; r3a += temp2; r3b += temp3;  
   
            dst_data[drow2]= (r3c);

            temp1 = inA * k12; temp2= inA * k22; temp3 = inA * k32;   
            r4c += temp1; r4a += temp2; r4b += temp3;   
   
            inA= src_data[srow5];
    
            temp1 = inA * k11; temp2= inA * k21; temp3 = inA * k31;   
            r4c += temp1; r4a += temp2; r4b += temp3;   
    
            dst_data[drow3]= (r4c);

            src_data += src_pstride;
            dst_data += dst_pstride;
            x--;
        }

        src_data += src_next;
        dst_data += dst_skip + 3 * dst_sstride;
    }

    ycount= roi_ysize & 3;
    for(y=ycount; y > 0; y--) {
        unsigned int src_next = src_skip;
        unsigned int srow1    = src_sstride;
        unsigned int srow2    = srow1+src_sstride;

        float inA;
        float temp1,temp2,temp3;

        //
        //  Pass 2
        //
        inA= src_data[0];

        temp3 = inA * k33;
        r1a  = temp3;

        inA= src_data[srow1];

        temp3 = inA * k32;
        r1a += temp3;

        inA= src_data[srow2];
 
        temp3 = inA * k31;
        r1a += temp3;
 
        src_data += src_pstride;

        //
        //  Pass 3 (if the region is only 1 pixel wide then this does
        //          unecessary work that is an extreme case)
        //
        inA= src_data[0];

        temp2= inA * k23; temp3 = inA * k33;
        r1a += temp2; r1b = temp3;

        inA= src_data[srow1];

        temp2= inA * k22; temp3 = inA * k32;
        r1a += temp2; r1b += temp3;

        inA= src_data[srow2];
 
        temp2= inA * k21; temp3 = inA * k31;
        r1a += temp2; r1b += temp3;
 
        temp2= inA * k21; temp3 = inA * k31; 
        r2a += temp2; r2b += temp3; 
  
        src_data += src_pstride;

        //
        //  Each pass through the loop produces a 3 x 4 block of destination
        //  pixels
        //
        x= roi_xsize;
        while(1) {
            //
            //  Pass 1
            //
            if(x==0) break;

            inA= src_data[0];

            temp1 = inA * k13; temp2= inA * k23; temp3 = inA * k33;
            r1a += temp1; r1b += temp2; r1c  = temp3;

            inA= src_data[srow1];

            temp1 = inA * k12; temp2= inA * k22; temp3 = inA * k32;
            r1a += temp1; r1b += temp2; r1c += temp3;

            inA= src_data[srow2];
 
            temp1 = inA * k11; temp2= inA * k21; temp3 = inA * k31;
            r1a += temp1; r1b += temp2; r1c += temp3;
 
            dst_data[0]= (r1a);

            src_data += src_pstride;
            dst_data += dst_pstride;
            x--;

            //
            //  Pass 2
            //
            if(x==0) break;

            inA= src_data[0];

            temp1 = inA * k13; temp2= inA * k23; temp3 = inA * k33;
            r1b += temp1; r1c += temp2; r1a  = temp3;

            inA= src_data[srow1];

            temp1 = inA * k12; temp2= inA * k22; temp3 = inA * k32;
            r1b += temp1; r1c += temp2; r1a += temp3;

            inA= src_data[srow2];
 
            temp1 = inA * k11; temp2= inA * k21; temp3 = inA * k31;
            r1b += temp1; r1c += temp2; r1a += temp3;
 
            dst_data[0]= (r1b);

            src_data += src_pstride;
            dst_data += dst_pstride;
            x--;

            //
            //  Pass 3
            //
            if(x==0) break;

            inA= src_data[0];

            temp1 = inA * k13; temp2= inA * k23; temp3 = inA * k33;
            r1c += temp1; r1a += temp2; r1b  = temp3;

            inA= src_data[srow1];

            temp1 = inA * k12; temp2= inA * k22; temp3 = inA * k32;
            r1c += temp1; r1a += temp2; r1b += temp3;

            inA= src_data[srow2];
 
            temp1 = inA * k11; temp2= inA * k21; temp3 = inA * k31;
            r1c += temp1; r1a += temp2; r1b += temp3;
 
            dst_data[0]= (r1c);

            src_data += src_pstride;
            dst_data += dst_pstride;
            x--;
        }

        src_data += src_next;
        dst_data += dst_skip;
    }

    return XIL_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////
//
//  3-banded abitrary pixel-stride images
//
//  Appears to be significantly slower than calling the above 1-banded
//  case for each band.
//
///////////////////////////////////////////////////////////////////////////
static
XilStatus
xili_convolve_3x3_opt_3(Xil_float32*  dst_data,
                        unsigned int  dst_sstride,
                        unsigned int  dst_pstride,
                        Xil_float32*  src_data,
                        unsigned int  src_sstride,
                        unsigned int  src_pstride,
                        int           roi_x,
                        int           roi_y,
                        int           roi_xsize,
                        int           roi_ysize,
                        float*        kdata)
{
    int x,y;

    unsigned int drow1= dst_sstride;
    unsigned int drow2= drow1+dst_sstride;
    unsigned int drow3= drow2+dst_sstride;

    //
    //  Load the kernel -- remember XIL 1.3 inverts it for us already.
    //
    float k11 = kdata[8];
    float k21 = kdata[7];
    float k31 = kdata[6];
    float k12 = kdata[5];
    float k22 = kdata[4];
    float k32 = kdata[3];
    float k13 = kdata[2];
    float k23 = kdata[1];
    float k33 = kdata[0];

    src_data = src_data + (roi_y - 1) * src_sstride + (roi_x - 1) * src_pstride;
    dst_data = dst_data + roi_y * dst_sstride + roi_x * dst_pstride;

    unsigned int dst_skip= dst_sstride - roi_xsize * dst_pstride;
    unsigned int src_skip= src_sstride - (roi_xsize + 2) * src_pstride;

    unsigned int ycount= (roi_ysize>>2);
    for(y=ycount; y!=0; y--) {
        unsigned int src_next = src_skip + 3 * src_sstride;

        unsigned int srow1 = src_sstride;
        unsigned int srow2 = srow1+src_sstride;
        unsigned int srow3 = srow2+src_sstride;
        unsigned int srow4 = srow3+src_sstride;
        unsigned int srow5 = srow4+src_sstride;

        float r1a0, r1b0, r1c0;
        float r2a0, r2b0, r2c0;
        float r3a0, r3b0, r3c0;
        float r4a0, r4b0, r4c0;

        float r1a1, r1b1, r1c1;
        float r2a1, r2b1, r2c1;
        float r3a1, r3b1, r3c1;
        float r4a1, r4b1, r4c1;

        float r1a2, r1b2, r1c2;
        float r2a2, r2b2, r2c2;
        float r3a2, r3b2, r3c2;
        float r4a2, r4b2, r4c2;

        float inA;
        float temp1, temp2, temp3;

        //
        //  Pass 2
        //
        inA = src_data[0];

        temp3 = inA * k33;
        r1a0  = temp3;

        inA = src_data[1];

        temp3 = inA * k33;
        r1a1  = temp3;

        inA = src_data[2];

        temp3 = inA * k33;
        r1a2  = temp3;


        inA = src_data[srow1];

        temp3 = inA * k32;
        r1a0 += temp3;

        temp3 = inA * k33;
        r2a0  = temp3;

        inA = src_data[srow1+1];

        temp3 = inA * k32;
        r1a1 += temp3;

        temp3 = inA * k33;
        r2a1  = temp3;

        inA = src_data[srow1+2];

        temp3 = inA * k32;
        r1a2 += temp3;

        temp3 = inA * k33;
        r2a2  = temp3;


        inA = src_data[srow2];
 
        temp3 = inA * k31;
        r1a0 += temp3;
 
        temp3 = inA * k32; 
        r2a0 += temp3; 
 
        temp3 = inA * k33; 
        r3a0  = temp3; 
 
        inA = src_data[srow2 + 1];
 
        temp3 = inA * k31;
        r1a1 += temp3;
 
        temp3 = inA * k32; 
        r2a1 += temp3; 
 
        temp3 = inA * k33; 
        r3a1  = temp3; 
 
        inA = src_data[srow2 + 2];
 
        temp3 = inA * k31;
        r1a2 += temp3;
 
        temp3 = inA * k32; 
        r2a2 += temp3; 
 
        temp3 = inA * k33; 
        r3a2  = temp3; 
 

        inA= src_data[srow3]; 
  
        temp3 = inA * k31; 
        r2a0 += temp3; 
  
        temp3 = inA * k32;  
        r3a0 += temp3;
  
        temp3 = inA * k33;
        r4a0  = temp3;

        inA= src_data[srow3 + 1]; 
  
        temp3 = inA * k31; 
        r2a1 += temp3; 
  
        temp3 = inA * k32;  
        r3a1 += temp3;
  
        temp3 = inA * k33;
        r4a1  = temp3;

        inA= src_data[srow3 + 2];
  
        temp3 = inA * k31; 
        r2a2 += temp3; 
  
        temp3 = inA * k32;  
        r3a2 += temp3;
  
        temp3 = inA * k33;
        r4a2  = temp3;

        
        inA= src_data[srow4];

        temp3 = inA * k31;  
        r3a0 += temp3;  
   
        temp3 = inA * k32;   
        r4a0 += temp3;   
   
        inA= src_data[srow4+1];

        temp3 = inA * k31;  
        r3a1 += temp3;  
   
        temp3 = inA * k32;   
        r4a1 += temp3;   
   
        inA= src_data[srow4+2];

        temp3 = inA * k31;  
        r3a2 += temp3;  
   
        temp3 = inA * k32;   
        r4a2 += temp3;   
   

        inA= src_data[srow5];
    
        temp3 = inA * k31;   
        r4a0 += temp3;   

        inA= src_data[srow5+1];
    
        temp3 = inA * k31;   
        r4a1 += temp3;   

        inA= src_data[srow5+2];
    
        temp3 = inA * k31;   
        r4a2 += temp3;   
    
        src_data += src_pstride;

        //
        //  pass 3 (if the region is only 1 pixel wide then this does
        //  unecessary work that is an extrem case)
        //
        inA= src_data[0];

        temp2 = inA * k23; temp3 = inA * k33;
        r1a0 += temp2; r1b0 = temp3;

        inA= src_data[1];

        temp2 = inA * k23; temp3 = inA * k33;
        r1a1 += temp2; r1b1 = temp3;

        inA= src_data[2];

        temp2 = inA * k23; temp3 = inA * k33;
        r1a2 += temp2; r1b2 = temp3;


        inA= src_data[srow1];

        temp2 = inA * k22; temp3 = inA * k32;
        r1a0 += temp2; r1b0 += temp3;

        temp2= inA * k23; temp3 = inA * k33;
        r2a0 += temp2; r2b0  = temp3;

        inA= src_data[srow1 + 1];

        temp2 = inA * k22; temp3 = inA * k32;
        r1a1 += temp2; r1b1 += temp3;

        temp2= inA * k23; temp3 = inA * k33;
        r2a1 += temp2; r2b1  = temp3;

        inA= src_data[srow1 + 2];

        temp2 = inA * k22; temp3 = inA * k32;
        r1a2 += temp2; r1b2 += temp3;

        temp2= inA * k23; temp3 = inA * k33;
        r2a2 += temp2; r2b2  = temp3;


        inA= src_data[srow2];
 
        temp2 = inA * k21; temp3 = inA * k31;
        r1a0 += temp2; r1b0 += temp3;
 
        temp2 = inA * k22; temp3 = inA * k32; 
        r2a0 += temp2; r2b0 += temp3; 
 
        temp2 = inA * k23; temp3 = inA * k33; 
        r3a0 += temp2; r3b0  = temp3; 
 
        inA= src_data[srow2 + 1];

        temp2 = inA * k21; temp3 = inA * k31;
        r1a1 += temp2; r1b1 += temp3;
 
        temp2 = inA * k22; temp3 = inA * k32; 
        r2a1 += temp2; r2b1 += temp3; 
 
        temp2 = inA * k23; temp3 = inA * k33; 
        r3a1 += temp2; r3b1  = temp3; 
 
        inA= src_data[srow2 + 2];
 
        temp2 = inA * k21; temp3 = inA * k31;
        r1a2 += temp2; r1b2 += temp3;
 
        temp2 = inA * k22; temp3 = inA * k32; 
        r2a2 += temp2; r2b2 += temp3; 
 
        temp2 = inA * k23; temp3 = inA * k33; 
        r3a2 += temp2; r3b2  = temp3; 
 

        inA= src_data[srow3];

        temp2 = inA * k21; temp3 = inA * k31; 
        r2a0 += temp2; r2b0 += temp3; 
  
        temp2 = inA * k22; temp3 = inA * k32;  
        r3a0 += temp2; r3b0 += temp3;  
  
        temp2 = inA * k23; temp3 = inA * k33;
        r4a0 += temp2; r4b0  = temp3;

        inA= src_data[srow3 + 1];

        temp2 = inA * k21; temp3 = inA * k31; 
        r2a1 += temp2; r2b1 += temp3; 
  
        temp2 = inA * k22; temp3 = inA * k32;  
        r3a1 += temp2; r3b1 += temp3;  
  
        temp2 = inA * k23; temp3 = inA * k33;
        r4a1 += temp2; r4b1  = temp3;

        inA= src_data[srow3 + 2];

        temp2 = inA * k21; temp3 = inA * k31; 
        r2a2 += temp2; r2b2 += temp3; 
  
        temp2 = inA * k22; temp3 = inA * k32;  
        r3a2 += temp2; r3b2 += temp3;  
  
        temp2 = inA * k23; temp3 = inA * k33;
        r4a2 += temp2; r4b2  = temp3;


        inA= src_data[srow4];
   
        temp2 = inA * k21; temp3 = inA * k31;  
        r3a0 += temp2; r3b0 += temp3;  
   
        temp2 = inA * k22; temp3 = inA * k32;   
        r4a0 += temp2; r4b0 += temp3;   
   
        inA= src_data[srow4 + 1];
   
        temp2 = inA * k21; temp3 = inA * k31;  
        r3a1 += temp2; r3b1 += temp3;  
   
        temp2 = inA * k22; temp3 = inA * k32;   
        r4a1 += temp2; r4b1 += temp3;   
   
        inA= src_data[srow4 + 2];
   
        temp2 = inA * k21; temp3 = inA * k31;  
        r3a2 += temp2; r3b2 += temp3;  
   
        temp2 = inA * k22; temp3 = inA * k32;   
        r4a2 += temp2; r4b2 += temp3;   
   

        inA= src_data[srow5];

        temp2 = inA * k21; temp3 = inA * k31;   
        r4a0 += temp2; r4b0 += temp3;   
    
        inA= src_data[srow5 + 1];

        temp2 = inA * k21; temp3 = inA * k31;   
        r4a1 += temp2; r4b1 += temp3;   
    
        inA= src_data[srow5 + 2];

        temp2 = inA * k21; temp3 = inA * k31;   
        r4a2 += temp2; r4b2 += temp3;   
    
        src_data += src_pstride;

        //
        //  Each pass through the loop produces a 3 x 4 block of destination
        //  pixels
        //
        x= roi_xsize;
        while(1) {
            //
            // Pass 1
            //
            if(x==0) break;

            inA= src_data[0];

            temp1 = inA * k13; temp2 = inA * k23; temp3 = inA * k33;
            r1a0 += temp1; r1b0 += temp2; r1c0  = temp3;

            inA= src_data[1];

            temp1 = inA * k13; temp2 = inA * k23; temp3 = inA * k33;
            r1a1 += temp1; r1b1 += temp2; r1c1  = temp3;

            inA= src_data[2];

            temp1 = inA * k13; temp2 = inA * k23; temp3 = inA * k33;
            r1a2 += temp1; r1b2 += temp2; r1c2  = temp3;

            
            inA= src_data[srow1];

            temp1 = inA * k12; temp2 = inA * k22; temp3 = inA * k32;
            r1a0 += temp1; r1b0 += temp2; r1c0 += temp3;

            temp1 = inA * k13; temp2 = inA * k23; temp3 = inA * k33;
            r2a0 += temp1; r2b0 += temp2; r2c0  = temp3;

            inA= src_data[srow1 + 1];

            temp1 = inA * k12; temp2 = inA * k22; temp3 = inA * k32;
            r1a1 += temp1; r1b1 += temp2; r1c1 += temp3;

            temp1 = inA * k13; temp2 = inA * k23; temp3 = inA * k33;
            r2a1 += temp1; r2b1 += temp2; r2c1  = temp3;

            inA= src_data[srow1 + 2];

            temp1 = inA * k12; temp2 = inA * k22; temp3 = inA * k32;
            r1a2 += temp1; r1b2 += temp2; r1c2 += temp3;

            temp1 = inA * k13; temp2 = inA * k23; temp3 = inA * k33;
            r2a2 += temp1; r2b2 += temp2; r2c2  = temp3;

            
            inA= src_data[srow2];
 
            temp1 = inA * k11; temp2 = inA * k21; temp3 = inA * k31;
            r1a0 += temp1; r1b0 += temp2; r1c0 += temp3;

            dst_data[0]= (r1a0);

            temp1 = inA * k12; temp2 = inA * k22; temp3 = inA * k32; 
            r2a0 += temp1; r2b0 += temp2; r2c0 += temp3; 
 
            temp1 = inA * k13; temp2 = inA * k23; temp3 = inA * k33; 
            r3a0 += temp1; r3b0 += temp2; r3c0  = temp3; 
 
            inA= src_data[srow2 + 1];
 
            temp1 = inA * k11; temp2 = inA * k21; temp3 = inA * k31;
            r1a1 += temp1; r1b1 += temp2; r1c1 += temp3;

            dst_data[1]= (r1a1);

            temp1 = inA * k12; temp2 = inA * k22; temp3 = inA * k32; 
            r2a1 += temp1; r2b1 += temp2; r2c1 += temp3; 
 
            temp1 = inA * k13; temp2 = inA * k23; temp3 = inA * k33; 
            r3a1 += temp1; r3b1 += temp2; r3c1  = temp3; 
 
            inA= src_data[srow2 + 2];
 
            temp1 = inA * k11; temp2 = inA * k21; temp3 = inA * k31;
            r1a2 += temp1; r1b2 += temp2; r1c2 += temp3;

            dst_data[2]= (r1a2);

            temp1 = inA * k12; temp2 = inA * k22; temp3 = inA * k32; 
            r2a2 += temp1; r2b2 += temp2; r2c2 += temp3; 
 
            temp1 = inA * k13; temp2 = inA * k23; temp3 = inA * k33; 
            r3a2 += temp1; r3b2 += temp2; r3c2  = temp3; 
 

            inA= src_data[srow3]; 
  
            temp1 = inA * k11; temp2 = inA * k21; temp3 = inA * k31; 
            r2a0 += temp1; r2b0 += temp2; r2c0 += temp3; 
  
            dst_data[drow1]= (r2a0);

            temp1 = inA * k12; temp2 = inA * k22; temp3 = inA * k32;  
            r3a0 += temp1; r3b0 += temp2; r3c0 += temp3;  
  
            temp1 = inA * k13; temp2 = inA * k23; temp3 = inA * k33;
            r4a0 += temp1; r4b0 += temp2; r4c0  = temp3;

            inA= src_data[srow3 + 1]; 
  
            temp1 = inA * k11; temp2 = inA * k21; temp3 = inA * k31; 
            r2a1 += temp1; r2b1 += temp2; r2c1 += temp3; 
  
            dst_data[drow1+1]= (r2a1);

            temp1 = inA * k12; temp2 = inA * k22; temp3 = inA * k32;  
            r3a1 += temp1; r3b1 += temp2; r3c1 += temp3;  
  
            temp1 = inA * k13; temp2 = inA * k23; temp3 = inA * k33;
            r4a1 += temp1; r4b1 += temp2; r4c1  = temp3;

            inA= src_data[srow3 + 2]; 
  
            temp1 = inA * k11; temp2 = inA * k21; temp3 = inA * k31; 
            r2a2 += temp1; r2b2 += temp2; r2c2 += temp3; 
  
            dst_data[drow1+2]= (r2a2);

            temp1 = inA * k12; temp2 = inA * k22; temp3 = inA * k32;  
            r3a2 += temp1; r3b2 += temp2; r3c2 += temp3;  
  
            temp1 = inA * k13; temp2 = inA * k23; temp3 = inA * k33;
            r4a2 += temp1; r4b2 += temp2; r4c2  = temp3;

            
            inA= src_data[srow4];
   
            temp1 = inA * k11; temp2 = inA * k21; temp3 = inA * k31;  
            r3a0 += temp1; r3b0 += temp2; r3c0 += temp3;  
   
            dst_data[drow2] = (r3a0);

            temp1 = inA * k12; temp2 = inA * k22; temp3 = inA * k32;   
            r4a0 += temp1; r4b0 += temp2; r4c0 += temp3;   

            inA= src_data[srow4 + 1];
   
            temp1 = inA * k11; temp2 = inA * k21; temp3 = inA * k31;  
            r3a1 += temp1; r3b1 += temp2; r3c1 += temp3;  
   
            dst_data[drow2 + 1] = (r3a1);

            temp1 = inA * k12; temp2 = inA * k22; temp3 = inA * k32;   
            r4a1 += temp1; r4b1 += temp2; r4c1 += temp3;   

            inA= src_data[srow4 + 2];
   
            temp1 = inA * k11; temp2 = inA * k21; temp3 = inA * k31;  
            r3a2 += temp1; r3b2 += temp2; r3c2 += temp3;  
   
            dst_data[drow2 + 2] = (r3a2);

            temp1 = inA * k12; temp2 = inA * k22; temp3 = inA * k32;   
            r4a2 += temp1; r4b2 += temp2; r4c2 += temp3;   


            inA= src_data[srow5];
    
            temp1 = inA * k11; temp2 = inA * k21; temp3 = inA * k31;   
            r4a0 += temp1; r4b0 += temp2; r4c0 += temp3;   
    
            dst_data[drow3]= (r4a0);

            inA= src_data[srow5 + 1];
    
            temp1 = inA * k11; temp2 = inA * k21; temp3 = inA * k31;   
            r4a1 += temp1; r4b1 += temp2; r4c1 += temp3;   
    
            dst_data[drow3 + 1]= (r4a1);

            inA= src_data[srow5 + 2];
    
            temp1 = inA * k11; temp2 = inA * k21; temp3 = inA * k31;   
            r4a2 += temp1; r4b2 += temp2; r4c2 += temp3;   
    
            dst_data[drow3 + 2]= (r4a2);


            src_data += src_pstride;
            dst_data += dst_pstride;
            x--;

            //
            //  Pass 2
            //
            if(x==0) break;

            inA= src_data[0];

            temp1 = inA * k13; temp2 = inA * k23; temp3 = inA * k33;
            r1b0 += temp1; r1c0 += temp2; r1a0  = temp3;

            inA= src_data[1];

            temp1 = inA * k13; temp2 = inA * k23; temp3 = inA * k33;
            r1b1 += temp1; r1c1 += temp2; r1a1  = temp3;

            inA= src_data[2];

            temp1 = inA * k13; temp2 = inA * k23; temp3 = inA * k33;
            r1b2 += temp1; r1c2 += temp2; r1a2  = temp3;


            inA= src_data[srow1];

            temp1 = inA * k12; temp2 = inA * k22; temp3 = inA * k32;
            r1b0 += temp1; r1c0 += temp2; r1a0 += temp3;

            temp1 = inA * k13; temp2 = inA * k23; temp3 = inA * k33;
            r2b0 += temp1; r2c0 += temp2; r2a0  = temp3;

            inA= src_data[srow1 + 1];

            temp1 = inA * k12; temp2 = inA * k22; temp3 = inA * k32;
            r1b1 += temp1; r1c1 += temp2; r1a1 += temp3;

            temp1 = inA * k13; temp2 = inA * k23; temp3 = inA * k33;
            r2b1 += temp1; r2c1 += temp2; r2a1  = temp3;

            inA= src_data[srow1 + 2];

            temp1 = inA * k12; temp2 = inA * k22; temp3 = inA * k32;
            r1b2 += temp1; r1c2 += temp2; r1a2 += temp3;

            temp1 = inA * k13; temp2 = inA * k23; temp3 = inA * k33;
            r2b2 += temp1; r2c2 += temp2; r2a2  = temp3;


            inA= src_data[srow2];
 
            temp1 = inA * k11; temp2 = inA * k21; temp3 = inA * k31;
            r1b0 += temp1; r1c0 += temp2; r1a0 += temp3;
 
            dst_data[0]= (r1b0);

            temp1 = inA * k12; temp2 = inA * k22; temp3 = inA * k32; 
            r2b0 += temp1; r2c0 += temp2; r2a0 += temp3; 
 
            temp1 = inA * k13; temp2 = inA * k23; temp3 = inA * k33; 
            r3b0 += temp1; r3c0 += temp2; r3a0  = temp3; 
 
            inA= src_data[srow2 + 1];
 
            temp1 = inA * k11; temp2 = inA * k21; temp3 = inA * k31;
            r1b1 += temp1; r1c1 += temp2; r1a1 += temp3;
 
            dst_data[1]= (r1b1);

            temp1 = inA * k12; temp2 = inA * k22; temp3 = inA * k32; 
            r2b1 += temp1; r2c1 += temp2; r2a1 += temp3; 
 
            temp1 = inA * k13; temp2 = inA * k23; temp3 = inA * k33; 
            r3b1 += temp1; r3c1 += temp2; r3a1  = temp3; 
 
            inA= src_data[srow2 + 2];
 
            temp1 = inA * k11; temp2 = inA * k21; temp3 = inA * k31;
            r1b2 += temp1; r1c2 += temp2; r1a2 += temp3;
 
            dst_data[2]= (r1b2);

            temp1 = inA * k12; temp2 = inA * k22; temp3 = inA * k32; 
            r2b2 += temp1; r2c2 += temp2; r2a2 += temp3; 
 
            temp1 = inA * k13; temp2 = inA * k23; temp3 = inA * k33; 
            r3b2 += temp1; r3c2 += temp2; r3a2  = temp3; 
 

            inA= src_data[srow3];
  
            temp1 = inA * k11; temp2 = inA * k21; temp3 = inA * k31; 
            r2b0 += temp1; r2c0 += temp2; r2a0 += temp3; 
  
            dst_data[drow1]= (r2b0);

            temp1 = inA * k12; temp2 = inA * k22; temp3 = inA * k32;  
            r3b0 += temp1; r3c0 += temp2; r3a0 += temp3;
  
            temp1 = inA * k13; temp2 = inA * k23; temp3 = inA * k33;
            r4b0 += temp1; r4c0 += temp2; r4a0  = temp3;

            inA= src_data[srow3 + 1];
  
            temp1 = inA * k11; temp2 = inA * k21; temp3 = inA * k31; 
            r2b1 += temp1; r2c1 += temp2; r2a1 += temp3; 
  
            dst_data[drow1 + 1]= (r2b1);

            temp1 = inA * k12; temp2 = inA * k22; temp3 = inA * k32;  
            r3b1 += temp1; r3c1 += temp2; r3a1 += temp3;
  
            temp1 = inA * k13; temp2 = inA * k23; temp3 = inA * k33;
            r4b1 += temp1; r4c1 += temp2; r4a1  = temp3;

            inA= src_data[srow3 + 2];
  
            temp1 = inA * k11; temp2 = inA * k21; temp3 = inA * k31; 
            r2b2 += temp1; r2c2 += temp2; r2a2 += temp3; 
  
            dst_data[drow1 + 2]= (r2b2);

            temp1 = inA * k12; temp2 = inA * k22; temp3 = inA * k32;  
            r3b2 += temp1; r3c2 += temp2; r3a2 += temp3;
  
            temp1 = inA * k13; temp2 = inA * k23; temp3 = inA * k33;
            r4b2 += temp1; r4c2 += temp2; r4a2  = temp3;


            inA= src_data[srow4];

            temp1 = inA * k11; temp2 = inA * k21; temp3 = inA * k31;  
            r3b0 += temp1; r3c0 += temp2; r3a0 += temp3;  
   
            dst_data[drow2]= (r3b0);

            temp1 = inA * k12; temp2 = inA * k22; temp3 = inA * k32;   
            r4b0 += temp1; r4c0 += temp2; r4a0 += temp3;   
   
            inA= src_data[srow4 + 1];

            temp1 = inA * k11; temp2 = inA * k21; temp3 = inA * k31;  
            r3b1 += temp1; r3c1 += temp2; r3a1 += temp3;  
   
            dst_data[drow2 + 1]= (r3b1);

            temp1 = inA * k12; temp2 = inA * k22; temp3 = inA * k32;   
            r4b1 += temp1; r4c1 += temp2; r4a1 += temp3;   
   
            inA= src_data[srow4 + 2];

            temp1 = inA * k11; temp2 = inA * k21; temp3 = inA * k31;  
            r3b2 += temp1; r3c2 += temp2; r3a2 += temp3;  
   
            dst_data[drow2 + 2]= (r3b2);

            temp1 = inA * k12; temp2 = inA * k22; temp3 = inA * k32;   
            r4b2 += temp1; r4c2 += temp2; r4a2 += temp3;   
   

            inA= src_data[srow5];
    
            temp1 = inA * k11; temp2 = inA * k21; temp3 = inA * k31;   
            r4b0 += temp1; r4c0 += temp2; r4a0 += temp3;   
    
            dst_data[drow3]= (r4b0);

            inA= src_data[srow5 + 1];
    
            temp1 = inA * k11; temp2 = inA * k21; temp3 = inA * k31;   
            r4b1 += temp1; r4c1 += temp2; r4a1 += temp3;   
    
            dst_data[drow3 + 1]= (r4b1);

            inA= src_data[srow5 + 2];
    
            temp1 = inA * k11; temp2 = inA * k21; temp3 = inA * k31;   
            r4b2 += temp1; r4c2 += temp2; r4a2 += temp3;   
    
            dst_data[drow3 + 2]= (r4b2);


            src_data += src_pstride;
            dst_data += dst_pstride;
            x--;

            //
            //  Pass 3
            //
            if(x==0) break;

            inA= src_data[0];

            temp1 = inA * k13; temp2 = inA * k23; temp3 = inA * k33;
            r1c0 += temp1; r1a0 += temp2; r1b0  = temp3;

            inA= src_data[1];

            temp1 = inA * k13; temp2 = inA * k23; temp3 = inA * k33;
            r1c1 += temp1; r1a1 += temp2; r1b1  = temp3;

            inA= src_data[2];

            temp1 = inA * k13; temp2 = inA * k23; temp3 = inA * k33;
            r1c2 += temp1; r1a2 += temp2; r1b2  = temp3;


            inA= src_data[srow1];

            temp1 = inA * k12; temp2 = inA * k22; temp3 = inA * k32;
            r1c0 += temp1; r1a0 += temp2; r1b0 += temp3;

            temp1 = inA * k13; temp2 = inA * k23; temp3 = inA * k33;
            r2c0 += temp1; r2a0 += temp2; r2b0  = temp3;

            inA= src_data[srow1 + 1];

            temp1 = inA * k12; temp2 = inA * k22; temp3 = inA * k32;
            r1c1 += temp1; r1a1 += temp2; r1b1 += temp3;

            temp1 = inA * k13; temp2 = inA * k23; temp3 = inA * k33;
            r2c1 += temp1; r2a1 += temp2; r2b1  = temp3;

            inA= src_data[srow1 + 2];

            temp1 = inA * k12; temp2 = inA * k22; temp3 = inA * k32;
            r1c2 += temp1; r1a2 += temp2; r1b2 += temp3;

            temp1 = inA * k13; temp2 = inA * k23; temp3 = inA * k33;
            r2c2 += temp1; r2a2 += temp2; r2b2  = temp3;


            inA= src_data[srow2];
 
            temp1 = inA * k11; temp2 = inA * k21; temp3 = inA * k31;
            r1c0 += temp1; r1a0 += temp2; r1b0 += temp3;
 
            dst_data[0]= (r1c0);

            temp1 = inA * k12; temp2 = inA * k22; temp3 = inA * k32; 
            r2c0 += temp1; r2a0 += temp2; r2b0 += temp3; 
 
            temp1 = inA * k13; temp2 = inA * k23; temp3 = inA * k33; 
            r3c0 += temp1; r3a0 += temp2; r3b0  = temp3; 

            inA= src_data[srow2 + 1];
 
            temp1 = inA * k11; temp2 = inA * k21; temp3 = inA * k31;
            r1c1 += temp1; r1a1 += temp2; r1b1 += temp3;
 
            dst_data[1]= (r1c1);

            temp1 = inA * k12; temp2 = inA * k22; temp3 = inA * k32; 
            r2c1 += temp1; r2a1 += temp2; r2b1 += temp3; 
 
            temp1 = inA * k13; temp2 = inA * k23; temp3 = inA * k33; 
            r3c1 += temp1; r3a1 += temp2; r3b1  = temp3; 

            inA= src_data[srow2 + 2];
 
            temp1 = inA * k11; temp2 = inA * k21; temp3 = inA * k31;
            r1c2 += temp1; r1a2 += temp2; r1b2 += temp3;
 
            dst_data[2]= (r1c2);

            temp1 = inA * k12; temp2 = inA * k22; temp3 = inA * k32; 
            r2c2 += temp1; r2a2 += temp2; r2b2 += temp3; 
 
            temp1 = inA * k13; temp2 = inA * k23; temp3 = inA * k33; 
            r3c2 += temp1; r3a2 += temp2; r3b2  = temp3; 


            inA= src_data[srow3];
  
            temp1 = inA * k11; temp2 = inA * k21; temp3 = inA * k31; 
            r2c0 += temp1; r2a0 += temp2; r2b0 += temp3; 

            dst_data[drow1]= (r2c0);

            temp1 = inA * k12; temp2 = inA * k22; temp3 = inA * k32;  
            r3c0 += temp1; r3a0 += temp2; r3b0 += temp3;  
  
            temp1 = inA * k13; temp2 = inA * k23; temp3 = inA * k33;
            r4c0 += temp1; r4a0 += temp2; r4b0  = temp3;

            inA= src_data[srow3 + 1];
  
            temp1 = inA * k11; temp2 = inA * k21; temp3 = inA * k31; 
            r2c1 += temp1; r2a1 += temp2; r2b1 += temp3; 

            dst_data[drow1 + 1]= (r2c1);

            temp1 = inA * k12; temp2 = inA * k22; temp3 = inA * k32;  
            r3c1 += temp1; r3a1 += temp2; r3b1 += temp3;  
  
            temp1 = inA * k13; temp2 = inA * k23; temp3 = inA * k33;
            r4c1 += temp1; r4a1 += temp2; r4b1  = temp3;

            inA= src_data[srow3 + 2];
  
            temp1 = inA * k11; temp2 = inA * k21; temp3 = inA * k31; 
            r2c2 += temp1; r2a2 += temp2; r2b2 += temp3; 

            dst_data[drow1 + 2]= (r2c2);

            temp1 = inA * k12; temp2 = inA * k22; temp3 = inA * k32;  
            r3c2 += temp1; r3a2 += temp2; r3b2 += temp3;  
  
            temp1 = inA * k13; temp2 = inA * k23; temp3 = inA * k33;
            r4c2 += temp1; r4a2 += temp2; r4b2  = temp3;


            inA= src_data[srow4];
   
            temp1 = inA * k11; temp2 = inA * k21; temp3 = inA * k31;  
            r3c0 += temp1; r3a0 += temp2; r3b0 += temp3;  
   
            dst_data[drow2]= (r3c0);

            temp1 = inA * k12; temp2 = inA * k22; temp3 = inA * k32;   
            r4c0 += temp1; r4a0 += temp2; r4b0 += temp3;   

            inA= src_data[srow4 + 1];
   
            temp1 = inA * k11; temp2 = inA * k21; temp3 = inA * k31;  
            r3c1 += temp1; r3a1 += temp2; r3b1 += temp3;  
   
            dst_data[drow2 + 1]= (r3c1);

            temp1 = inA * k12; temp2 = inA * k22; temp3 = inA * k32;   
            r4c1 += temp1; r4a1 += temp2; r4b1 += temp3;   

            inA= src_data[srow4 + 2];
   
            temp1 = inA * k11; temp2 = inA * k21; temp3 = inA * k31;  
            r3c2 += temp1; r3a2 += temp2; r3b2 += temp3;  
   
            dst_data[drow2 + 2]= (r3c2);

            temp1 = inA * k12; temp2 = inA * k22; temp3 = inA * k32;   
            r4c2 += temp1; r4a2 += temp2; r4b2 += temp3;   


            inA= src_data[srow5];

            temp1 = inA * k11; temp2 = inA * k21; temp3 = inA * k31;   
            r4c0 += temp1; r4a0 += temp2; r4b0 += temp3;   
    
            dst_data[drow3]= (r4c0);

            inA= src_data[srow5 + 1];

            temp1 = inA * k11; temp2 = inA * k21; temp3 = inA * k31;   
            r4c1 += temp1; r4a1 += temp2; r4b1 += temp3;   
    
            dst_data[drow3 + 1]= (r4c1);

            inA= src_data[srow5 + 2];

            temp1 = inA * k11; temp2 = inA * k21; temp3 = inA * k31;   
            r4c2 += temp1; r4a2 += temp2; r4b2 += temp3;   
    
            dst_data[drow3 + 2]= (r4c2);

            src_data += src_pstride;
            dst_data += dst_pstride;
            x--;
        }

        src_data += src_next;
        dst_data += dst_skip + 3 * dst_sstride;
    }

    ycount= roi_ysize & 3;
    for(y=ycount; y > 0; y--) {
        unsigned int src_next = src_skip;
        unsigned int srow1    = src_sstride;
        unsigned int srow2    = srow1+src_sstride;

        float r1a0 = 0, r1b0 = 0, r1c0 = 0;
        float r2a0 = 0, r2b0 = 0;

        float r1a1 = 0, r1b1 = 0, r1c1 = 0;
        float r2a1 = 0, r2b1 = 0;

        float r1a2 = 0, r1b2 = 0, r1c2 = 0;
        float r2a2 = 0, r2b2 = 0;

        float inA = 0;
        float temp1 = 0,temp2 = 0,temp3 = 0;

        //
        //  Pass 2
        //
        inA= src_data[0];

        temp3 = inA * k33;
        r1a0  = temp3;

        inA= src_data[1];

        temp3 = inA * k33;
        r1a1  = temp3;

        inA= src_data[2];

        temp3 = inA * k33;
        r1a2  = temp3;


        inA= src_data[srow1];

        temp3 = inA * k32;
        r1a0 += temp3;

        inA= src_data[srow1 + 1];

        temp3 = inA * k32;
        r1a1 += temp3;

        inA= src_data[srow1 + 2];

        temp3 = inA * k32;
        r1a2 += temp3;


        inA= src_data[srow2];
 
        temp3 = inA * k31;
        r1a0 += temp3;
 
        inA= src_data[srow2 + 1];
 
        temp3 = inA * k31;
        r1a1 += temp3;
 
        inA= src_data[srow2 + 2];
 
        temp3 = inA * k31;
        r1a2 += temp3;

        src_data += src_pstride;

        //
        //  Pass 3 (if the region is only 1 pixel wide then this does
        //          unecessary work that is an extreme case)
        //
        inA= src_data[0];

        temp2 = inA * k23; temp3 = inA * k33;
        r1a0 += temp2; r1b0 = temp3;

        inA= src_data[1];

        temp2 = inA * k23; temp3 = inA * k33;
        r1a1 += temp2; r1b1 = temp3;

        inA= src_data[2];

        temp2 = inA * k23; temp3 = inA * k33;
        r1a2 += temp2; r1b2 = temp3;


        inA= src_data[srow1];

        temp2 = inA * k22; temp3 = inA * k32;
        r1a0 += temp2; r1b0 += temp3;

        inA= src_data[srow1 + 1];

        temp2 = inA * k22; temp3 = inA * k32;
        r1a1 += temp2; r1b1 += temp3;

        inA= src_data[srow1 + 2];

        temp2 = inA * k22; temp3 = inA * k32;
        r1a2 += temp2; r1b2 += temp3;


        inA= src_data[srow2];
 
        temp2 = inA * k21; temp3 = inA * k31;
        r1a0 += temp2; r1b0 += temp3;
 
        temp2 = inA * k21; temp3 = inA * k31; 
        r2a0 += temp2; r2b0 += temp3; 

        inA= src_data[srow2 + 1];
 
        temp2 = inA * k21; temp3 = inA * k31;
        r1a1 += temp2; r1b1 += temp3;
 
        temp2 = inA * k21; temp3 = inA * k31; 
        r2a1 += temp2; r2b1 += temp3; 

        inA= src_data[srow2 + 2];
 
        temp2 = inA * k21; temp3 = inA * k31;
        r1a2 += temp2; r1b2 += temp3;
 
        temp2 = inA * k21; temp3 = inA * k31; 
        r2a2 += temp2; r2b2 += temp3; 


        src_data += src_pstride;

        //
        //  Each pass through the loop produces a 3 x 4 block of destination
        //  pixels
        //
        x= roi_xsize;
        while(1) {
            //
            //  Pass 1
            //
            if(x==0) break;

            inA= src_data[0];

            temp1 = inA * k13; temp2 = inA * k23; temp3 = inA * k33;
            r1a0 += temp1; r1b0 += temp2; r1c0  = temp3;

            inA= src_data[1];

            temp1 = inA * k13; temp2 = inA * k23; temp3 = inA * k33;
            r1a1 += temp1; r1b1 += temp2; r1c1  = temp3;

            inA= src_data[2];

            temp1 = inA * k13; temp2 = inA * k23; temp3 = inA * k33;
            r1a2 += temp1; r1b2 += temp2; r1c2  = temp3;


            inA= src_data[srow1];

            temp1 = inA * k12; temp2 = inA * k22; temp3 = inA * k32;
            r1a0 += temp1; r1b0 += temp2; r1c0 += temp3;

            inA= src_data[srow1 + 1];

            temp1 = inA * k12; temp2 = inA * k22; temp3 = inA * k32;
            r1a1 += temp1; r1b1 += temp2; r1c1 += temp3;

            inA= src_data[srow1 + 2];

            temp1 = inA * k12; temp2 = inA * k22; temp3 = inA * k32;
            r1a2 += temp1; r1b2 += temp2; r1c2 += temp3;


            inA= src_data[srow2];
 
            temp1 = inA * k11; temp2 = inA * k21; temp3 = inA * k31;
            r1a0 += temp1; r1b0 += temp2; r1c0 += temp3;
 
            inA= src_data[srow2 + 1];
 
            temp1 = inA * k11; temp2 = inA * k21; temp3 = inA * k31;
            r1a1 += temp1; r1b1 += temp2; r1c1 += temp3;
 
            inA= src_data[srow2 + 2];
 
            temp1 = inA * k11; temp2 = inA * k21; temp3 = inA * k31;
            r1a2 += temp1; r1b2 += temp2; r1c2 += temp3;
 

            dst_data[0]= (r1a0);
            dst_data[1]= (r1a1);
            dst_data[2]= (r1a2);

            src_data += src_pstride;
            dst_data += dst_pstride;
            x--;

            //
            //  Pass 2
            //
            if(x==0) break;

            inA= src_data[0];

            temp1 = inA * k13; temp2 = inA * k23; temp3 = inA * k33;
            r1b0 += temp1; r1c0 += temp2; r1a0  = temp3;

            inA= src_data[1];

            temp1 = inA * k13; temp2 = inA * k23; temp3 = inA * k33;
            r1b1 += temp1; r1c1 += temp2; r1a1  = temp3;

            inA= src_data[2];

            temp1 = inA * k13; temp2 = inA * k23; temp3 = inA * k33;
            r1b2 += temp1; r1c2 += temp2; r1a2  = temp3;


            inA= src_data[srow1];

            temp1 = inA * k12; temp2 = inA * k22; temp3 = inA * k32;
            r1b0 += temp1; r1c0 += temp2; r1a0 += temp3;

            inA= src_data[srow1 + 1];

            temp1 = inA * k12; temp2 = inA * k22; temp3 = inA * k32;
            r1b1 += temp1; r1c1 += temp2; r1a1 += temp3;

            inA= src_data[srow1 + 2];

            temp1 = inA * k12; temp2 = inA * k22; temp3 = inA * k32;
            r1b2 += temp1; r1c2 += temp2; r1a2 += temp3;


            inA= src_data[srow2];
 
            temp1 = inA * k11; temp2 = inA * k21; temp3 = inA * k31;
            r1b0 += temp1; r1c0 += temp2; r1a0 += temp3;

            inA= src_data[srow2 + 1];
 
            temp1 = inA * k11; temp2 = inA * k21; temp3 = inA * k31;
            r1b1 += temp1; r1c1 += temp2; r1a1 += temp3;

            inA= src_data[srow2 + 2];
 
            temp1 = inA * k11; temp2 = inA * k21; temp3 = inA * k31;
            r1b2 += temp1; r1c2 += temp2; r1a2 += temp3;


            dst_data[0]= (r1b0);
            dst_data[1]= (r1b1);
            dst_data[2]= (r1b2);

            src_data += src_pstride;
            dst_data += dst_pstride;
            x--;

            //
            //  Pass 3
            //
            if(x==0) break;

            inA= src_data[0];

            temp1 = inA * k13; temp2 = inA * k23; temp3 = inA * k33;
            r1c0 += temp1; r1a0 += temp2; r1b0  = temp3;

            inA= src_data[1];

            temp1 = inA * k13; temp2 = inA * k23; temp3 = inA * k33;
            r1c1 += temp1; r1a1 += temp2; r1b1  = temp3;

            inA= src_data[2];

            temp1 = inA * k13; temp2 = inA * k23; temp3 = inA * k33;
            r1c2 += temp1; r1a2 += temp2; r1b2  = temp3;


            inA= src_data[srow1];

            temp1 = inA * k12; temp2 = inA * k22; temp3 = inA * k32;
            r1c0 += temp1; r1a0 += temp2; r1b0 += temp3;

            inA= src_data[srow1 + 1];

            temp1 = inA * k12; temp2 = inA * k22; temp3 = inA * k32;
            r1c1 += temp1; r1a1 += temp2; r1b1 += temp3;

            inA= src_data[srow1 + 2];

            temp1 = inA * k12; temp2 = inA * k22; temp3 = inA * k32;
            r1c2 += temp1; r1a2 += temp2; r1b2 += temp3;


            inA= src_data[srow2];
 
            temp1 = inA * k11; temp2 = inA * k21; temp3 = inA * k31;
            r1c0 += temp1; r1a0 += temp2; r1b0 += temp3;
 
            inA= src_data[srow2 + 1];
 
            temp1 = inA * k11; temp2 = inA * k21; temp3 = inA * k31;
            r1c1 += temp1; r1a1 += temp2; r1b1 += temp3;
 
            inA= src_data[srow2 + 2];
 
            temp1 = inA * k11; temp2 = inA * k21; temp3 = inA * k31;
            r1c2 += temp1; r1a2 += temp2; r1b2 += temp3;
 
            dst_data[0]= (r1c0);
            dst_data[1]= (r1c1);
            dst_data[2]= (r1c2);

            src_data += src_pstride;
            dst_data += dst_pstride;
            x--;
        }

        src_data += src_next;
        dst_data += dst_skip;
    }

    return XIL_SUCCESS;
}

