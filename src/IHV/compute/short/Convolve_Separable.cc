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
//  File:	Convolve_Separable.cc
//  Project:	XIL
//  Revision:	1.14
//  Last Mod:	10:12:30, 03/10/00
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
#pragma ident	"@(#)Convolve_Separable.cc	1.14\t00/03/10  "

#include "XilDeviceManagerComputeSHORT.hh"
#include "XiliUtils.hh"


static XilStatus
ConvolveSep(Xil_signed16* dest_data,
            int dest_scanline_stride,
            int dest_pixel_stride,
            Xil_signed16* src1_data,
            int src1_scanline_stride,
            int src1_pixel_stride,
            int src1_xsize,
            int src1_ysize,
            int nbands,
            float *kernel1,
            float *kernel2,
            int keyx,
            int keyy,
            int kwidth,
            int kheight);

XilStatus
XilDeviceManagerComputeSHORT::Convolve_SeparablePreprocess(XilOp*        op,
                                                           unsigned      ,
                                                           XilRoi*       ,
                                                           void**        ,
                                                           unsigned int* )
{
    //
    //  don't use separable kernels for 3x3 (better optimizations elsewhere)
    //
    XilKernel* kernel;
    op->getParam(1, (void**)&kernel);

    unsigned int kwidth  = kernel->getWidth();
    unsigned int kheight = kernel->getHeight();
    int          keyx    = kernel->getKeyX();
    int          keyy    = kernel->getKeyY();

    if( kwidth == 3 &&
        kheight == 3 ||
        keyx != kwidth>>1 ||
        keyy != kheight>>1 ) {
        return XIL_FAILURE;
    } else {
        return XIL_SUCCESS;
    }
}


XilStatus
XilDeviceManagerComputeSHORT::Convolve_Separable(XilOp*        op,
                                                 unsigned        ,
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
    int          keyx    = kernel->getKeyX();
    int          keyy    = kernel->getKeyY();
    float*       kdata   = (float*)kernel->getData();

    //
    //  Get separable kernel data
    //
    float *kernel1;
    float *kernel2;
    kernel->getSeparableData((const float **)&kernel1, (const float **)&kernel2);

    //
    //  Store away the number of bands for this operation.
    //    
    unsigned int nbands = destImage->getNumBands();

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
	//  If the edge condition is no write and this box is in an edge area
	//  then simple continue on to the next box.
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
                    Xil_signed16* dest_data;
                    dest_storage.getStorageInfo(band,
                                                &dest_pixel_stride,
                                                &dest_scanline_stride,
                                                NULL,
                                                (void**)&dest_data);
		    
                    Xil_signed16* dest_scanline = dest_data +
                        (y*dest_scanline_stride) + (x*dest_pixel_stride);
		    
                    unsigned int scanline_count = ysize;
                    
                    //
                    //  Each Scanline...
                    //
                    do {
                        Xil_signed16* dest_pixel = dest_scanline;
			
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
			Xil_signed16* src1_data;
			src1_storage.getStorageInfo(band,
						    &src1_pixel_stride,
						    &src1_scanline_stride,
						    NULL,
						    (void**)&src1_data);

			unsigned int   dest_pixel_stride;
			unsigned int   dest_scanline_stride;
			Xil_signed16* dest_data;
			dest_storage.getStorageInfo(band,
						    &dest_pixel_stride,
						    &dest_scanline_stride,
						    NULL,
						    (void**)&dest_data);

			Xil_signed16* src1_scanline = src1_data +
			    (y*src1_scanline_stride) + (x*src1_pixel_stride);

			Xil_signed16* dest_scanline = dest_data +
			    (y*dest_scanline_stride) + (x*dest_pixel_stride);

			int ptrX = src_box_x + x;
			int ptrY = src_box_y + y;
			int kulX = ptrX - keyx;
			int kulY = ptrY - keyy;

			//
			// Clamp kernel upper left and since this is the
			// upper left corner, both should clamp to 0.
			//
			if(kulX < 0) {
                            kulX = 0;
                        }
			if(kulY < 0) {
                            kulY = 0;
                        }

			Xil_signed16* up_left = src1_scanline +
			    ((kulX - ptrX) * src1_pixel_stride) +
			    ((kulY - ptrY) * src1_scanline_stride);
			
			//
			// Do the cases where the kernel extends above or meets the top
			// of the image.
			//
			for(j = 0; j < ysize; j++) {
                            //
			    //  point to the first pixel of the scanline
                            //
			    Xil_signed16* src1_pixel = src1_scanline;
			    Xil_signed16* dest_pixel = dest_scanline;
			    
			    for(i = 0; i < xsize; i++) {
				Xil_signed16* dest = dest_pixel;
				
				float fsum  = 0.0;
				float *kptr = kdata;

				Xil_signed16* corner = up_left;
				Xil_signed16* sptr;
				int kh;
				int kw;

				//  for the 0th row to < keyy row
				for(kh = 0; kh < keyy - ptrY - j; kh++) {
				    sptr = corner;

				    // for the upper left quadrant
				    for(kw = 0; kw < keyx - ptrX - i; kw++) {
					fsum += ((float)(*sptr) * *(kptr + kh * kwidth + kw));
				    }
				    
				    // for the upper right quadrant
				    for(kw = keyx - ptrX - i; kw < kwidth; kw++) {
					fsum += ((float)(*sptr) * *(kptr + kh * kwidth + kw));
					sptr += src1_pixel_stride;
				    }
				}

				sptr = corner;
				for(kh = keyy - ptrY - j; kh < kheight; kh++) {
				    for(kw = 0; kw < keyx - ptrX - i; kw++) {
				        fsum += ((float)(*sptr) * *(kptr + kh * kwidth + kw));
				    }

				    for(kw = keyx - ptrX - i; kw < kwidth; kw++) {
				        fsum += ((float)(*sptr) * *(kptr + kh * kwidth + kw));
				        sptr += src1_pixel_stride;
				    }

				    corner += src1_scanline_stride;
				    sptr    = corner;
				}
				
				*dest = _XILI_ROUND_S16(fsum);
				
				src1_pixel += src1_pixel_stride;
				dest_pixel += dest_pixel_stride;
			    }
			    
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
			Xil_signed16* src1_data;
			src1_storage.getStorageInfo(band,
						    &src1_pixel_stride,
						    &src1_scanline_stride,
						    NULL,
						    (void**)&src1_data);
	    
			unsigned int   dest_pixel_stride;
			unsigned int   dest_scanline_stride;
			Xil_signed16* dest_data;
			dest_storage.getStorageInfo(band,
						    &dest_pixel_stride,
						    &dest_scanline_stride,
						    NULL,
						    (void**)&dest_data);
			
			Xil_signed16* src1_scanline = src1_data +
			    (y*src1_scanline_stride) + (x*src1_pixel_stride);

			Xil_signed16* dest_scanline = dest_data +
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
			Xil_signed16* kernel_start = src1_scanline +
			    ((kulX - ptrX) * src1_pixel_stride) +
			    ((kulY - ptrY) * src1_scanline_stride);
			
			//
			// Do the cases where the kernel extends above or meets the top
			// of the image.
			//
			for(j = 0; j < ysize; j++) {
			    // point to the first pixel of the scanline 
			    Xil_signed16* src1_pixel = src1_scanline;
			    Xil_signed16* dest_pixel = dest_scanline;
			    
			    for(i = 0; i < xsize; i++) {
				Xil_signed16* dest = dest_pixel;
				
				float fsum = 0.0;
				float *kptr = kdata;
				
				Xil_signed16* sptr;
				Xil_signed16* corner = kernel_start + (i * src1_pixel_stride);

				int kh;
				int kw;
				for(kh = 0; kh < keyy - ptrY - j; kh++) {
				    sptr = corner;
				    for(kw = 0; kw < kwidth; kw++) {
					fsum += ((float)(*sptr) * *(kptr + kh * kwidth + kw));
					sptr += src1_pixel_stride;
				    }
				}

				sptr = corner;
				for(kh = keyy - ptrY - j; kh < kheight; kh++) {
				    for(kw = 0; kw < kwidth; kw++) {
					fsum += ((float)(*sptr) * *(kptr + kh * kwidth + kw));
					sptr += src1_pixel_stride;
				    }

                                    corner += src1_scanline_stride;
                                    sptr   = corner;
				}
				
				*dest = _XILI_ROUND_S16(fsum);

				//
				//  Move to the next pixel
                                //
				src1_pixel += src1_pixel_stride;
				dest_pixel += dest_pixel_stride;
			    }

                            //
			    //  Move to the next scanline
                            //
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
			Xil_signed16* src1_data;
			src1_storage.getStorageInfo(band,
						    &src1_pixel_stride,
						    &src1_scanline_stride,
						    NULL,
						    (void**)&src1_data);
	    
			unsigned int   dest_pixel_stride;
			unsigned int   dest_scanline_stride;
			Xil_signed16* dest_data;
			dest_storage.getStorageInfo(band,
						    &dest_pixel_stride,
						    &dest_scanline_stride,
						    NULL,
						    (void**)&dest_data);
			
			Xil_signed16* src1_scanline = src1_data +
			    (y*src1_scanline_stride) + (x*src1_pixel_stride);

			Xil_signed16* dest_scanline = dest_data +
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
			Xil_signed16* top_right = src1_scanline +
			    ((kulX - ptrX) * src1_pixel_stride) +
			    ((kulY - ptrY) * src1_scanline_stride);
			
			//
			// Do the cases where the kernel extends above or meets the top
			// of the image.
			//
			for(j = 0; j < ysize; j++) {
			    // point to the first pixel of the scanline 
			    Xil_signed16* src1_pixel = src1_scanline;
			    Xil_signed16* dest_pixel = dest_scanline;
			    for(i = 0; i < xsize; i++) {
				Xil_signed16* dest = dest_pixel;
				
				float fsum = 0.0;
				float *kptr = kdata;

				Xil_signed16* corner = top_right;
				Xil_signed16* sptr;
				int kh;
				int kw;

				for(kh = 0; kh < keyy - ptrY - j; kh++) {
				    sptr = corner;
				    for(kw = keyx + imageWidth - ptrX - i; kw < kwidth; kw++) {
					fsum += ((float)(*sptr) * *(kptr + kh * kwidth + kw));
				    }
				    
				    for(kw = keyx + imageWidth - ptrX - i - 1; kw >=0; kw--) {
					fsum += ((float)(*sptr) * *(kptr + kh * kwidth + kw));
					sptr -= src1_pixel_stride;
				    }
				}
				
				sptr = corner;
				for(kh = keyy - ptrY - j; kh < kheight; kh++) {
				    for(kw = keyx + imageWidth - ptrX - i; kw < kwidth; kw++) {
					fsum += ((float)(*sptr) * *(kptr + kh * kwidth + kw));
				    }
				    
				    for(kw = keyx + imageWidth - ptrX - i - 1; kw >=0; kw--) {
					fsum += ((float)(*sptr) * *(kptr + kh * kwidth + kw));
					sptr -= src1_pixel_stride;
				    }
				    
				    sptr = (corner += src1_scanline_stride);
				}
				
				*dest = _XILI_ROUND_S16(fsum);
				
				src1_pixel += src1_pixel_stride;
				dest_pixel += dest_pixel_stride;
				}
			    
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
			Xil_signed16* src1_data;
			src1_storage.getStorageInfo(band,
						    &src1_pixel_stride,
						    &src1_scanline_stride,
						    NULL,
						    (void**)&src1_data);
			
			unsigned int   dest_pixel_stride;
			unsigned int   dest_scanline_stride;
			Xil_signed16* dest_data;
			dest_storage.getStorageInfo(band,
						    &dest_pixel_stride,
						    &dest_scanline_stride,
						    NULL,
						    (void**)&dest_data);
			
			Xil_signed16* src1_scanline = src1_data +
			    + (x*src1_pixel_stride) + (y*src1_scanline_stride);

			Xil_signed16* dest_scanline = dest_data +
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
			Xil_signed16* kernel_start = src1_scanline +
			    ((kulX - ptrX) * src1_pixel_stride) +
			    ((kulY - ptrY) * src1_scanline_stride);
			
			for(j = 0; j < ysize; j++) {
			    // point to the first pixel of the scanline 
			    Xil_signed16* dest_pixel = dest_scanline;
			    
			    for(i = 0; i < xsize; i++) {
				Xil_signed16* dest = dest_pixel;
				
				float fsum = 0.0;
				float *kptr = kdata;

				Xil_signed16* sptr = kernel_start;
				Xil_signed16* sptr_save = sptr;
				int kh;
				int kw;
				for(kh = 0; kh < kheight; kh++) {
				    for(kw = 0; kw < keyx - ptrX - i; kw++) {
					fsum += ((float)(*sptr) * *(kptr + kh * kwidth + kw));
				    }
				    
				    for(kw = keyx - ptrX - i; kw < kwidth; kw++) {
					fsum += ((float)(*sptr) * *(kptr + kh * kwidth + kw));
					sptr += src1_pixel_stride;
				    }

                                    sptr_save += src1_scanline_stride;
                                    sptr      =  sptr_save;
				}

				*dest = _XILI_ROUND_S16(fsum);
				
				dest_pixel += dest_pixel_stride;
			    }
			    
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
			Xil_signed16* src1_data;
			src1_storage.getStorageInfo(band,
						    &src1_pixel_stride,
						    &src1_scanline_stride,
						    NULL,
						    (void**)&src1_data);
	    
			unsigned int   dest_pixel_stride;
			unsigned int   dest_scanline_stride;
			Xil_signed16* dest_data;
			dest_storage.getStorageInfo(band,
						    &dest_pixel_stride,
						    &dest_scanline_stride,
						    NULL,
						    (void**)&dest_data);
			
			Xil_signed16* src1_scanline = src1_data +
			    (y*src1_scanline_stride) + (x*src1_pixel_stride);

			Xil_signed16* dest_scanline = dest_data +
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
			Xil_signed16* kernel_start = src1_scanline +
			    ((kulX - ptrX) * src1_pixel_stride) +
			    ((kulY - ptrY) * src1_scanline_stride);

			for(j = 0; j < ysize; j++) {
			    // point to the first pixel of the scanline 
			    Xil_signed16* dest_pixel = dest_scanline;
			    
			    for(i = 0; i < xsize; i++) {
				Xil_signed16* dest = dest_pixel;
				
				float fsum = 0.0;
				float *kptr = kdata;

				Xil_signed16* sptr = kernel_start;
				Xil_signed16* sptr_save = sptr;
				int kh;
				int kw;
				for(kh = 0; kh < kheight; kh++) {
				    for(kw = kulX - (ptrX-keyx) - i + 1; kw < kwidth; kw++) {
					fsum += ((float)(*sptr) * *(kptr + kh * kwidth + kw));
				    }
				    
				    for(kw = kulX - (ptrX-keyx) - i; kw >= 0; kw--) {
					fsum += ((float)(*sptr) * *(kptr + kh * kwidth + kw));
					sptr -= src1_pixel_stride;
				    }

                                    sptr_save += src1_scanline_stride;
				    sptr       = sptr_save;
				}

				*dest = _XILI_ROUND_S16(fsum);
				
				dest_pixel += dest_pixel_stride;
			    }
			    
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
			Xil_signed16* src1_data;
			src1_storage.getStorageInfo(band,
						    &src1_pixel_stride,
						    &src1_scanline_stride,
						    NULL,
						    (void**)&src1_data);
	    
			unsigned int   dest_pixel_stride;
			unsigned int   dest_scanline_stride;
			Xil_signed16* dest_data;
			dest_storage.getStorageInfo(band,
						    &dest_pixel_stride,
						    &dest_scanline_stride,
						    NULL,
						    (void**)&dest_data);
			
			Xil_signed16* src1_scanline = src1_data +
			    (y*src1_scanline_stride) + (x*src1_pixel_stride);

			Xil_signed16* dest_scanline = dest_data +
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

			Xil_signed16* lower_left = src1_scanline +
			    ((kulX - ptrX) * src1_pixel_stride) +
			    ((kulY - ptrY) * src1_scanline_stride);

			//
			// Do the cases where the kernel extends above or meets the top
			// of the image.
			//
			for(j = 0; j < ysize; j++) {
			    // point to the first pixel of the scanline 
			    Xil_signed16* src1_pixel = src1_scanline;
			    Xil_signed16* dest_pixel = dest_scanline;
			    
			    for(i = 0; i < xsize; i++) {
				Xil_signed16* dest = dest_pixel;
				
				float fsum = 0.0;
				float *kptr = kdata;

				Xil_signed16* corner = lower_left;
				Xil_signed16* sptr = corner;
				Xil_signed16* sptr_save = corner;
				int kh;
				int kw;
				for(kh = keyy + imageHeight - ptrY - j - 1; kh >= 0; kh--) {
				    for(kw = 0; kw < keyx - ptrX - i; kw++) {
					fsum += ((float)(*sptr) * *(kptr + kh * kwidth + kw));
				    }
				    
				    for(kw = keyx - ptrX - i; kw < kwidth; kw++) {
					fsum += ((float)(*sptr) * *(kptr + kh * kwidth + kw));
					sptr += src1_pixel_stride;
				    }

                                    sptr_save -= src1_scanline_stride;
				    sptr       = sptr_save;
				}

				for(kh = keyy + imageHeight - ptrY - j; kh < kheight; kh++) {
					sptr = corner;
					for(kw = 0; kw < keyx - ptrX - i; kw++)
					    fsum += ((float)(*sptr) * *(kptr + kh * kwidth + kw));

					for(kw = keyx - ptrX - i; kw < kwidth; kw++) {
					    fsum += ((float)(*sptr) * *(kptr + kh * kwidth + kw));
					    sptr += src1_pixel_stride;
					}
				    }
				
				*dest = _XILI_ROUND_S16(fsum);
				
				src1_pixel += src1_pixel_stride;
				dest_pixel += dest_pixel_stride;
			    }
			    
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
			Xil_signed16* src1_data;
			src1_storage.getStorageInfo(band,
						    &src1_pixel_stride,
						    &src1_scanline_stride,
						    NULL,
						    (void**)&src1_data);
	    
			unsigned int   dest_pixel_stride;
			unsigned int   dest_scanline_stride;
			Xil_signed16* dest_data;
			dest_storage.getStorageInfo(band,
						    &dest_pixel_stride,
						    &dest_scanline_stride,
						    NULL,
						    (void**)&dest_data);
			
			Xil_signed16* src1_scanline = src1_data +
			    (y*src1_scanline_stride) + (x*src1_pixel_stride);

			Xil_signed16* dest_scanline = dest_data +
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
			Xil_signed16* kernel_start = src1_scanline +
			    ((kulX - ptrX) * src1_pixel_stride) +
			    ((kulY - ptrY) * src1_scanline_stride);

			//
			// Do the cases where the kernel extends below or meets the bottom
			// of the image.
			//
			for(j = 0; j < ysize; j++) {
			    // point to the first pixel of the scanline 
			    Xil_signed16* src1_pixel = src1_scanline;
			    Xil_signed16* dest_pixel = dest_scanline;
			    Xil_signed16* lower_left = kernel_start;
			    
			    for(i = 0; i < xsize; i++) {
				Xil_signed16* dest = dest_pixel;
				
				float fsum  = 0.0;
				float *kptr = kdata;

				Xil_signed16* sptr = lower_left;
				Xil_signed16* sptr_save = lower_left;
				int kh;
				int kw;
				for(kh = kulY - (ptrY-keyy) - j; kh >= 0; kh--) {
				    for(kw = 0; kw < kwidth; kw++) {
					fsum += ((float)(*sptr) * *(kptr + kh * kwidth + kw));
					sptr += src1_pixel_stride;
				    }

                                    sptr_save -= src1_scanline_stride;
				    sptr       = sptr_save;
				}

				for(kh = kulY - (ptrY-keyy) - j + 1; kh < kheight; kh++) {
				    sptr = lower_left;

				    for(kw = 0; kw < kwidth; kw++) {
					fsum += ((float)(*sptr) * *(kptr + kh * kwidth + kw));

					sptr += src1_pixel_stride;
				    }
				}

				*dest = _XILI_ROUND_S16(fsum);

				src1_pixel += src1_pixel_stride;
				dest_pixel += dest_pixel_stride;
				lower_left += src1_pixel_stride;
			    }

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
			Xil_signed16* src1_data;
			src1_storage.getStorageInfo(band,
						    &src1_pixel_stride,
						    &src1_scanline_stride,
						    NULL,
						    (void**)&src1_data);
	    
			unsigned int   dest_pixel_stride;
			unsigned int   dest_scanline_stride;
			Xil_signed16* dest_data;
			dest_storage.getStorageInfo(band,
						    &dest_pixel_stride,
						    &dest_scanline_stride,
						    NULL,
						    (void**)&dest_data);
			
			Xil_signed16* src1_scanline = src1_data +
			    (y*src1_scanline_stride) + (x*src1_pixel_stride);

			Xil_signed16* dest_scanline = dest_data +
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
			Xil_signed16* lower_right = src1_scanline +
			    ((kulX - ptrX) * src1_pixel_stride) +
			    ((kulY - ptrY) * src1_scanline_stride);
			//
			// Do the cases where the kernel extends above or meets the top
			// of the image.
			//
			for(j = 0; j < ysize; j++) {
			    Xil_signed16* src1_pixel = src1_scanline;
			    Xil_signed16* dest_pixel = dest_scanline;
			    
			    for(i = 0; i < xsize; i++) {
				Xil_signed16* dest = dest_pixel;
				
				float fsum = 0.0;
				float *kptr = kdata;

				Xil_signed16* corner = lower_right;
				Xil_signed16* sptr = corner;
				Xil_signed16* sptr_save = corner;
				int kh;
				int kw;
				for(kh = keyy + imageHeight - ptrY - j - 1; kh >= 0; kh--) {
				    for(kw =  keyx + imageWidth - ptrX - i; kw < kwidth; kw++) {
					fsum += ((float)(*sptr) * *(kptr + kh * kwidth + kw));
				    }
				    
				    for(kw = keyx + imageWidth - ptrX - i - 1 ; kw >= 0; kw--) {
					fsum += ((float)(*sptr) * *(kptr + kh * kwidth + kw));
					sptr -= src1_pixel_stride;
				    }

                                    sptr_save -= src1_scanline_stride;
				    sptr       = sptr_save;
				}

				for(kh = keyy + imageHeight - ptrY - j; kh < kheight; kh++) {
				    sptr = corner;
				    for(kw =  keyx + imageWidth - ptrX - i; kw < kwidth; kw++)
					fsum += ((float)(*sptr) * *(kptr + kh * kwidth + kw));
					
				    for(kw = keyx + imageWidth - ptrX - i - 1 ; kw >= 0; kw--) {
					fsum += ((float)(*sptr) * *(kptr + kh * kwidth + kw));
					sptr -= src1_pixel_stride;
				    }
				}
				
				*dest = _XILI_ROUND_S16(fsum);
				
				src1_pixel += src1_pixel_stride;
				dest_pixel += dest_pixel_stride;
			    }
			    
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
	    //  Test to see if all of our storage is of type
            //  XIL_PIXEL_SEQUENTIAL.
            //
	    //  If so, implement an loop optimized for pixel-sequential
            //  storage. 
	    //
	    if((src1_storage.isType(XIL_PIXEL_SEQUENTIAL)) &&
	       (dest_storage.isType(XIL_PIXEL_SEQUENTIAL))) {
		unsigned int   src1_pixel_stride;
		unsigned int   src1_scanline_stride;
		Xil_signed16* src1_data;
		src1_storage.getStorageInfo(&src1_pixel_stride,
					    &src1_scanline_stride,
					    NULL, NULL,
					    (void**)&src1_data);

		unsigned int   dest_pixel_stride;
		unsigned int   dest_scanline_stride;
		Xil_signed16* dest_data;
		dest_storage.getStorageInfo(&dest_pixel_stride,
					    &dest_scanline_stride,
					    NULL, NULL,
					    (void**)&dest_data);

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
                    Xil_signed16* src1_scanline = src1_data +
                        (y*src1_scanline_stride) + (x*src1_pixel_stride);

                    Xil_signed16* dest_scanline = dest_data +
                        (y*dest_scanline_stride) + (x*dest_pixel_stride);

                    ConvolveSep(dest_scanline,
                                dest_scanline_stride,
                                dest_pixel_stride,
                                src1_scanline,
                                src1_scanline_stride,
                                src1_pixel_stride,
                                xsize,
                                ysize,
                                nbands,
                                kernel1,
                                kernel2,
                                keyx,
                                keyy,
                                kwidth,
                                kheight);
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
			Xil_signed16* src1_data;
			src1_storage.getStorageInfo(band,
						    &src1_pixel_stride,
						    &src1_scanline_stride,
						    NULL,
						    (void**)&src1_data);

			unsigned int   dest_pixel_stride;
			unsigned int   dest_scanline_stride;
			Xil_signed16* dest_data;
			dest_storage.getStorageInfo(band,
						    &dest_pixel_stride,
						    &dest_scanline_stride,
						    NULL,
						    (void**)&dest_data);
			
			Xil_signed16* src1_scanline = src1_data +
			    (y*src1_scanline_stride) + (x*src1_pixel_stride);
			
			Xil_signed16* dest_scanline = dest_data +
			    (y*dest_scanline_stride) + (x*dest_pixel_stride);
			
			unsigned int scanline_count = ysize;
			
			Xil_signed16* kernel_scanline  = src1_scanline -
			    (keyx * src1_pixel_stride) - (keyy * src1_scanline_stride);
			//
			//  Each Scanline...
			//
			do {
			    Xil_signed16* src1_pixel = src1_scanline;
			    Xil_signed16* dest_pixel = dest_scanline;
			    Xil_signed16* kernel_pixel = kernel_scanline;
			    unsigned int pixel_count = xsize;
			    
			    //
			    //  Each Pixel...
			    //
			    do {
				float fsum = 0.0;
				float* kptr = kdata;
				Xil_signed16* sptr = kernel_pixel;
				Xil_signed16* sptr_save  = sptr;
				
				for(int kh = 0; kh < kheight; kh++) {
				    for(int kw = 0; kw < kwidth; kw++) {
					fsum += (float)(*sptr) * *kptr;

					sptr += src1_pixel_stride;
					kptr++;
				    }

                                    sptr_save += src1_scanline_stride;
				    sptr       = sptr_save;
				}
				
				*dest_pixel  = _XILI_ROUND_S16(fsum);

				src1_pixel  +=  src1_pixel_stride;
				dest_pixel  +=  dest_pixel_stride;
				kernel_pixel += src1_pixel_stride;
			    } while(--pixel_count);
			    
			    src1_scanline += src1_scanline_stride;
			    dest_scanline += dest_scanline_stride;
			    kernel_scanline += src1_scanline_stride;
			} while(--scanline_count);
		    }
		}
            }
	}
    }

    return XIL_SUCCESS;
}



/*center area*/
static XilStatus
ConvolveSep(Xil_signed16* dest_data,
            int dest_scanline_stride,
            int dest_pixel_stride,
            Xil_signed16* src1_data,
            int src1_scanline_stride,
            int src1_pixel_stride,
            int src1_xsize,
            int src1_ysize,
            int nbands,
            float *kernel1,
            float *kernel2,
            int keyx,
            int keyy,
            int kwidth,
            int kheight)
{
    int i, j, line;
    float fsum;
    Xil_signed16 *srcptr;
    Xil_signed16 *dstptr;

    //
    //  3x3 kernels are not supported here
    //  uses other 3x3 optimizations
    //
    float *result = new float[src1_xsize*kheight];
    float *tmpsav = result;

    for( int b = 0; b < nbands; b++ ) {
        srcptr = src1_data - (keyy*src1_scanline_stride) - (keyx*src1_pixel_stride) + b;
        dstptr = dest_data + b;
        result = tmpsav;

        //
        //  unroll loops for 5x5
        //
        if( kwidth == 5 && kheight == 5 ) {
            //
            //  Separable Convolution (pixel sequential)
            //
            Xil_signed16 *src = srcptr;
            Xil_signed16 *dst = dstptr;
            Xil_signed16 *tmp = srcptr;

            int roll0 = 0;
            int roll1 = 1;
            int roll2 = 2;
            int roll3 = 3;
            int roll4 = 4;

            float kx0 = kernel1[0];
            float kx1 = kernel1[1];
            float kx2 = kernel1[2];
            float kx3 = kernel1[3];
            float kx4 = kernel1[4];

            //
            //  first (kheight x src1_xsize) block (horizontal convolve)
            //  probably not worth unrolling this loop
            //
            int kh = kheight;
            while( kh-- ) {
                for( j = 0; j < src1_xsize; j++ ) {
                    fsum  = (float)(*src) * kx0;  src += src1_pixel_stride;
                    fsum += (float)(*src) * kx1;  src += src1_pixel_stride;
                    fsum += (float)(*src) * kx2;  src += src1_pixel_stride;
                    fsum += (float)(*src) * kx3;  src += src1_pixel_stride;
                    fsum += (float)(*src) * kx4;

                    *result++ = fsum;

                    tmp += src1_pixel_stride;
                    src  = tmp;
                }

                srcptr += src1_scanline_stride;
                src = srcptr;
                tmp = src;
            }

            result = tmpsav;

            //
            //  first block (vertical convolve)
            //
            for( i = 0; i < src1_xsize; i++ ) {
                fsum  = *(result + i) * kernel2[0];  result += src1_xsize;
                fsum += *(result + i) * kernel2[1];  result += src1_xsize;
                fsum += *(result + i) * kernel2[2];  result += src1_xsize;
                fsum += *(result + i) * kernel2[3];  result += src1_xsize;
                fsum += *(result + i) * kernel2[4];

                *dst = _XILI_ROUND_S16(fsum);

                dst += dest_pixel_stride;
                result = tmpsav;
            }

            //
            //  sliding window
            //
            dstptr += dest_scanline_stride;
            dst = dstptr;
            src = srcptr;
            tmp = srcptr;

            for( i = 0; i < src1_ysize-1; i++ ) {
                //
                //  horizontal pass (line emulates a rolling buffer)
                //
                line = i % kheight;

                for( j = 0; j < src1_xsize; j++ ) {
                    fsum  = (float)(*src) * kx0;  src += src1_pixel_stride;
                    fsum += (float)(*src) * kx1;  src += src1_pixel_stride;
                    fsum += (float)(*src) * kx2;  src += src1_pixel_stride;
                    fsum += (float)(*src) * kx3;  src += src1_pixel_stride;
                    fsum += (float)(*src) * kx4;

                    *(result + line*src1_xsize + j) = fsum;

                    tmp += src1_pixel_stride;
                    src  = tmp;
                }

                //
                //  vertical
                //
                roll0 -= 1;   if( roll0 < 0 ) roll0 = kheight-1;
                roll1 -= 1;   if( roll1 < 0 ) roll1 = kheight-1;
                roll2 -= 1;   if( roll2 < 0 ) roll2 = kheight-1;
                roll3 -= 1;   if( roll3 < 0 ) roll3 = kheight-1;
                roll4 -= 1;   if( roll4 < 0 ) roll4 = kheight-1;

                for( j = 0; j < src1_xsize; j++ ) {
                    fsum  = *(result + j) * kernel2[roll0];  result += src1_xsize;
                    fsum += *(result + j) * kernel2[roll1];  result += src1_xsize;
                    fsum += *(result + j) * kernel2[roll2];  result += src1_xsize;
                    fsum += *(result + j) * kernel2[roll3];  result += src1_xsize;
                    fsum += *(result + j) * kernel2[roll4];

                    *dst = _XILI_ROUND_S16(fsum);

                    dst += dest_pixel_stride;
                    result = tmpsav;
                }

                srcptr += src1_scanline_stride;
                dstptr += dest_scanline_stride;

                src = srcptr;
                dst = dstptr;
                tmp = src;
            }
        } else {
            //
            //  Separable Convolution (pixel sequential)
            //
            float *kptr;
            Xil_signed16 *src = srcptr;
            Xil_signed16 *dst = dstptr;
            Xil_signed16 *tmp = srcptr;

            int *roll = new int[kheight];

            for( j = 0; j < kheight; j++ ) {
                roll[j] = j;
            }

            //
            //  first (kheight x src1_xsize) block (horizontal convolve)
            //
            int kw;
            int kh = kheight;
            while( kh-- ) {
                for( j = 0; j < src1_xsize; j++ ) {
                    fsum = 0.0;
                    kw   = kwidth;
                    kptr = kernel1;

                    while( kw-- ) {
                        fsum += (float)(*src) * (*kptr++);
                        src  += src1_pixel_stride;
                    }

                    *result = fsum;

                    result += 1;
                    tmp    += src1_pixel_stride;
                    src     = tmp;
                }

                srcptr += src1_scanline_stride;
                src = srcptr;
                tmp = src;
            }

            result = tmpsav;

            //
            //  first block (vertical convolve)
            //
            for( i = 0; i < src1_xsize; i++ ) {
                fsum = 0.0;
                kptr = kernel2;
   
                for( j = 0; j < kheight; j++ ) {
                    fsum += (*kptr++) * *(result + j*src1_xsize + i);
                }

                *dst = _XILI_ROUND_S16(fsum);

                dst += dest_pixel_stride;
            }

            //
            //  sliding window
            //
            dstptr += dest_scanline_stride;
            dst = dstptr;
            src = srcptr;
            tmp = srcptr;

            for( i = 0; i < src1_ysize-1; i++ ) {
                //
                //  horizontal pass (line emulates a rolling buffer)
                //
                line = i % kheight;

                for( j = 0; j < src1_xsize; j++ ) {
                    fsum = 0.0;
                    kptr = kernel1;
                    kw   = kwidth;

                    while( kw-- ) {
                        fsum += (float)(*src) * (*kptr++);
                        src  += src1_pixel_stride;
                    }

                    *(result + line*src1_xsize + j) = fsum;

                    tmp += src1_pixel_stride;
                    src  = tmp;
                }

                //
                //  vertical
                //
                for( j = 0; j < kheight; j++ ) {
                    roll[j] -= 1;

                    if( roll[j] < 0 ) {
                        roll[j] = kheight-1;
                    }
                }

                for( j = 0; j < src1_xsize; j++ ) {
                    fsum = 0.0;

                    for( int m = 0; m < kheight; m++ ) {
                        fsum += *(result + m*src1_xsize + j) * kernel2[roll[m]];
                    }

                    *dst = _XILI_ROUND_S16(fsum);
                    dst += dest_pixel_stride;
                }

                srcptr += src1_scanline_stride;
                dstptr += dest_scanline_stride;

                src = srcptr;
                dst = dstptr;
                tmp = src;
            }

            delete[] roll;
        }
    }

    result = tmpsav;
    delete[] result;

    return XIL_SUCCESS;
}
