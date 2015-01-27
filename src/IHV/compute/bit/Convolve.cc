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
//  Revision:	1.10
//  Last Mod:	10:09:45, 03/10/00
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
#pragma ident	"@(#)Convolve.cc	1.10\t00/03/10  "

#include "XilDeviceManagerComputeBIT.hh"
#include "XiliUtils.hh"

XilStatus
XilDeviceManagerComputeBIT::Convolve(XilOp*       op,
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
    XilEdgeCondition    edge_condition;
    op->getParam(1, (void**)&kernel);
    //
    // XIL_EDGE_NO_WRITE
    // XIL_EDGE_ZERO_FILL
    // XIL_EDGE_EXTEND
    //
    op->getParam(2, (int*)&edge_condition);

    unsigned int width  = kernel->getWidth();
    unsigned int height = kernel->getHeight();
    int keyx   = (int)kernel->getKeyX();
    int keyy   = (int)kernel->getKeyY();
    float* kdata  = (float *) kernel->getData();

    //
    //  Store away the number of bands for this operation.
    //    
    unsigned int nbands   = destImage->getNumBands();

    int i,j;

    /* allocate space for the src_scan */
    bit_base_t** src_scan = new bit_base_t* [height];
    if (src_scan == NULL) {
        XIL_ERROR(destImage->getSystemState(), XIL_ERROR_RESOURCE, "di-1", TRUE);
        return XIL_FAILURE;
    }

    //
    //  Loop over each of the boxes of storage we are to process.
    //
    XilBox* src1_box;
    XilBox* dest_box;
    while(bl->getNext(&src1_box, &dest_box)) {
	XilBoxAreaType tag = (XilBoxAreaType) ((long)dest_box->getTag());
	switch(tag) {
	  case XIL_AREA_TOP_LEFT_CORNER:
	  case XIL_AREA_TOP_EDGE:
	  case XIL_AREA_TOP_RIGHT_CORNER:
	  case XIL_AREA_RIGHT_EDGE:
	  case XIL_AREA_CENTER:
	  case XIL_AREA_LEFT_EDGE:
	  case XIL_AREA_BOTTOM_EDGE:
 	  case XIL_AREA_BOTTOM_LEFT_CORNER:
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

	  default :
	    if(bl->markAsFailed() == XIL_FAILURE) {
		delete [] src_scan;
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
        if ((src1Image->getStorage(&src1_storage, op, src1_box, "XilMemory",
                             XIL_READ_ONLY) == XIL_FAILURE) || 
	    (destImage->getStorage(&dest_storage, op, dest_box, "XilMemory",
			      XIL_WRITE_ONLY) == XIL_FAILURE)) {
	    //
	    //  Mark this box entry as having failed.  If marking the box
	    //  returns XIL_FAILURE, then we return XIL_FAILURE.
	    //
	    if(bl->markAsFailed() == XIL_FAILURE) {
		delete [] src_scan;
		return XIL_FAILURE;
	    } else {
		continue;
	    }
	}


	//
	// If the edge condition is no write and this box is in an edge area
	// then simple continue on to the next box.
	//
	if ((edge_condition == XIL_EDGE_NO_WRITE) &&
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
		    unsigned int   dest_offset;
                    Xil_unsigned8* dest_data;
                    dest_storage.getStorageInfo(band,
                                                &dest_pixel_stride,
                                                &dest_scanline_stride,
                                                &dest_offset,
                                                (void**)&dest_data);
		    
                    Xil_unsigned8* dest_scanline = dest_data +
                        (y*dest_scanline_stride) + ((x + dest_offset) / 8);

		    dest_offset = (x + dest_offset) % 8;
		    
                    //
                    //  Each Scanline...
                    //
		    for(j = 0; j < ysize; j++) {
			for(i = 0; i < xsize; i++) {
			    XIL_BMAP_CLR(dest_scanline, dest_offset + i);
			}
			dest_scanline += dest_scanline_stride;
		    }
		}
	    }
	    continue;
        }

	if (tag != XIL_AREA_CENTER) {
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
			unsigned int   src1_offsetUI;
			Xil_unsigned8* src1_data;
			src1_storage.getStorageInfo(band,
						    &src1_pixel_stride,
						    &src1_scanline_stride,
						    &src1_offsetUI,
						    (void**)&src1_data);
			int src1_offset = src1_offsetUI;

			unsigned int   dest_pixel_stride;
			unsigned int   dest_scanline_stride;
			unsigned int   dest_offset;
			Xil_unsigned8* dest_data;
			dest_storage.getStorageInfo(band,
						    &dest_pixel_stride,
						    &dest_scanline_stride,
						    &dest_offset,
						    (void**)&dest_data);
			
			int ptrX = src_box_x + x;
			int ptrY = src_box_y + y;
			Xil_unsigned8* src1 = src1_data +
			    (y * src1_scanline_stride) + ((x + src1_offset) / 8);
			src1_offset = (x + src1_offset) % 8;

			//
			// Get to top.
			// Y should be clamping to 0.
			//
			int kulY = ptrY - keyy;
			if (kulY < 0)
			    kulY = 0;
			src1 -= (ptrY - kulY) * src1_scanline_stride;

			Xil_unsigned8* dest = dest_data +
			    (y*dest_scanline_stride) + ((x + dest_offset) / 8);
			dest_offset = (x + dest_offset) % 8;
		    
			//
			//  Each Scanline...
			//
			float* kptr = kdata;
			for(j = 0; j < ysize; j++) {
			    int kh;
			    int counter;
			    for(kh = 0; kh < keyy - ptrY - j; kh++) {
				src_scan[kh] = (bit_base_t*) src1;			
			    }
			    for(kh = keyy - ptrY - j, counter = 0; kh < height; kh++, counter++) {
				src_scan[kh] = (bit_base_t*) (src1 + counter * src1_scanline_stride);
			    }
			    
			    //
			    //  Each Pixel...
			    //
			    for(i = 0; i < xsize; i++) {
				int kw;
				float fsum = 0.0;
				int bit_offset = src1_offset + i - keyx;
				
				for(kh = 0; kh < height; kh++) {
				    for(kw = 0; kw < keyx - ptrX - i; kw++) {
					if (XIL_BMAP_TST(src_scan[kh], src1_offset)) {
					    fsum += *(kptr + kh * width + kw);
					}
					    
				    }
				    for(kw = keyx - ptrX - i; kw < width; kw++) {
					if (XIL_BMAP_TST(src_scan[kh], bit_offset + kw)) {
					    fsum += *(kptr + kh * width + kw);
					}
				    }
				}
			    
				if (_XILI_ROUND_1(fsum) == 0) {
				    XIL_BMAP_CLR(dest, dest_offset + i);
				} else {
				    XIL_BMAP_SET(dest, dest_offset + i);
				}
			    }
			
			    //src1 += src1_scanline_stride;
			dest += dest_scanline_stride;
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
			unsigned int   src1_offsetUI;
			Xil_unsigned8* src1_data;
			src1_storage.getStorageInfo(band,
						    &src1_pixel_stride,
						    &src1_scanline_stride,
						    &src1_offsetUI,
						    (void**)&src1_data);
			int src1_offset = src1_offsetUI;

			unsigned int   dest_pixel_stride;
			unsigned int   dest_scanline_stride;
			unsigned int   dest_offset;
			Xil_unsigned8* dest_data;
			dest_storage.getStorageInfo(band,
						    &dest_pixel_stride,
						    &dest_scanline_stride,
						    &dest_offset,
						    (void**)&dest_data);
			
			int sx1,sy1,sx2,sy2;
			src1_box->getAsCorners(&sx1, &sy1, &sx2, &sy2);

			Xil_unsigned8* src1 = src1_data
			    - (sy1 * src1_scanline_stride)
			    + ((x + src1_offset) / 8);
			src1_offset = (x + src1_offset) % 8;
			while(src1_offset - keyx < 0) {
			    src1--;
			    src1_offset += 8;
			}
		    
			Xil_unsigned8* dest = dest_data +
			    (y*dest_scanline_stride) + ((x + dest_offset) / 8);
			dest_offset = (x + dest_offset) % 8;
		    
			//
			//  Each Scanline...
			//
			for(j = 0; j < ysize; j++) {
			    int kh;
			    for(kh = 0; kh < keyy - j - sy1; kh++) {
				src_scan[kh] = (bit_base_t*) src1;			
			    }
			    int counter;
			    for(kh = keyy - j - sy1, counter = 0; kh < height; kh++, counter++) {
				src_scan[kh] = (bit_base_t*)(src1 + src1_scanline_stride * counter);
			    }
			    
			    //
			    //  Each Pixel...
			    //
			    for(i = 0; i < xsize; i++) {
				int kw;
				float fsum = 0.0;
				float* kptr = kdata;
				int bit_offset = src1_offset + i - keyx ;

				for(kh = 0; kh < height; kh++) {
				    for(kw = 0; kw < width; kw++) {
					if (XIL_BMAP_TST(src_scan[kh], bit_offset + kw)) {
					    fsum += *kptr;
					}
					kptr++;
				    }
				}

				if (_XILI_ROUND_1(fsum) == 0) {
				    XIL_BMAP_CLR(dest, dest_offset + i);
				} else {
				    XIL_BMAP_SET(dest, dest_offset + i);
				}
			    }
			
//			    src1 += src1_scanline_stride;
			    dest += dest_scanline_stride;
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
			unsigned int   src1_offsetUI;
			Xil_unsigned8* src1_data;
			src1_storage.getStorageInfo(band,
						    &src1_pixel_stride,
						    &src1_scanline_stride,
						    &src1_offsetUI,
						    (void**)&src1_data);
			int src1_offset = src1_offsetUI;

			unsigned int   dest_pixel_stride;
			unsigned int   dest_scanline_stride;
			unsigned int   dest_offset;
			Xil_unsigned8* dest_data;
			dest_storage.getStorageInfo(band,
						    &dest_pixel_stride,
						    &dest_scanline_stride,
						    &dest_offset,
						    (void**)&dest_data);
			
			int ptrX = src_box_x + x;
			int ptrY = src_box_y + y;
			Xil_unsigned8* src1 = src1_data +
			    (y * src1_scanline_stride) + ((x + src1_offset) / 8);
			src1_offset = (x + src1_offset) % 8;

			//
			// Get to top.
			// Y should be clamping to 0.
			//
			int topY = ptrY - keyy;
			if (topY < 0)
			    topY = 0;
			src1 -= (ptrY - topY) * src1_scanline_stride;

			while(src1_offset - keyx < 0) {
			    src1--;
			    src1_offset += 8;
			}
			
			Xil_unsigned8* dest = dest_data +
			    (y*dest_scanline_stride) + ((x + dest_offset) / 8);
			dest_offset = (x + dest_offset) % 8;
		    
			//
			//  Each Scanline...
			//
			int imageWidth = src1Image->getWidth();
			int last = imageWidth - ptrX - 1 + src1_offset;
			for(j = 0; j < ysize; j++) {
			    int kh;
			    for(kh = 0; kh < keyy - j; kh++) {
				src_scan[kh] = (bit_base_t*) src1;			
			    }
			    int counter;
			    for(kh = keyy - j, counter = 0; kh < height; kh++, counter++) {
				src_scan[kh] = (bit_base_t*) (src1 + counter * src1_scanline_stride);
			    }
			    
			    //
			    //  Each Pixel...
			    //
			    for(i = 0; i < xsize; i++) {
				int kw;
				float fsum = 0.0;
				float* kptr = kdata;
				int bit_offset = src1_offset + i - keyx;
				
				for(kh = 0; kh < height; kh++) {
				    for(kw = 0; kw < keyx + imageWidth - ptrX - i; kw++) {
					if (XIL_BMAP_TST(src_scan[kh], bit_offset + kw)) {
					    fsum += *kptr;
					}
					kptr++;
				    }
				    for(kw = keyx + imageWidth - ptrX - i; kw < width; kw++) {
					if (XIL_BMAP_TST(src_scan[kh], last)) {
					    fsum += *kptr;
					}
					kptr++;
				    }
				}
			    
				if (_XILI_ROUND_1(fsum) == 0) {
				    XIL_BMAP_CLR(dest, dest_offset + i);
				} else {
				    XIL_BMAP_SET(dest, dest_offset + i);
				}
			    }
			
			    //src1 += src1_scanline_stride;
			dest += dest_scanline_stride;
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
			unsigned int   src1_offsetUI;
			Xil_unsigned8* src1_data;
			src1_storage.getStorageInfo(band,
						    &src1_pixel_stride,
						    &src1_scanline_stride,
						    &src1_offsetUI,
						    (void**)&src1_data);
			int src1_offset = src1_offsetUI;

			unsigned int   dest_pixel_stride;
			unsigned int   dest_scanline_stride;
			unsigned int   dest_offset;
			Xil_unsigned8* dest_data;
			dest_storage.getStorageInfo(band,
						    &dest_pixel_stride,
						    &dest_scanline_stride,
						    &dest_offset,
						    (void**)&dest_data);
			
			int ptrX = src_box_x + x;
			int ptrY = src_box_y + y;
			Xil_unsigned8* src1 = src1_data +
			    (y * src1_scanline_stride) + ((x + src1_offset) / 8);
			src1_offset = (x + src1_offset) % 8;

			//
			// Get to top left corner.
			// Y should be clamping to 0.
			// X should clamp to 0
			//
			int kulX = ptrX - keyx;
			if (kulX < 0)
			    kulX = 0;
			int kulY = ptrY - keyy;
			if (kulY < 0)
			    kulY = 0;
			src1 -= (ptrY - kulY) * src1_scanline_stride;
			//
			// src1_offset damn well better be negative.  If not, then this pixel
			// belongs in another box type.
			//
			src1_offset -= ptrX - kulX;
			while(src1_offset < 0) {
			    src1--;
			    src1_offset += 8;
			}
		    
			Xil_unsigned8* dest = dest_data +
			    (y*dest_scanline_stride) + ((x + dest_offset) / 8);
			dest_offset = (x + dest_offset) % 8;
		    
			//
			//  Each Scanline...
			//
			float* kptr = kdata;
			for(j = 0; j < ysize; j++) {
			    int kh;
			    src_scan[0] = (bit_base_t*) src1;			
			    for(kh = 1; kh < height; kh++) {
				src_scan[kh] = src_scan[kh - 1] + src1_scanline_stride;
			    }
			    
			    //
			    //  Each Pixel...
			    //
			    for(i = 0; i < xsize; i++) {
				int kw;
				float fsum = 0.0;
				
				for(kh = 0; kh < height; kh++) {
				    int tmp = XIL_BMAP_TST(src_scan[kh], src1_offset);
				    for(kw = 0; kw < keyx - ptrX - i; kw++) {
					if (tmp) {
					    fsum += *(kptr + kh * width + kw);
					}
				    }
				    for(kw = keyx - ptrX - i,tmp=0; kw < width; kw++,tmp++) {
					if (XIL_BMAP_TST(src_scan[kh], src1_offset+tmp)) {
					    fsum += *(kptr + kh * width + kw);
					}
				    }
				}
			    
				if (_XILI_ROUND_1(fsum) == 0) {
				    XIL_BMAP_CLR(dest, dest_offset + i);
				} else {
				    XIL_BMAP_SET(dest, dest_offset + i);
				}
			    }
			
			src1 += src1_scanline_stride;
			dest += dest_scanline_stride;
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
			unsigned int   src1_offsetUI;
			Xil_unsigned8* src1_data;
			src1_storage.getStorageInfo(band,
						    &src1_pixel_stride,
						    &src1_scanline_stride,
						    &src1_offsetUI,
						    (void**)&src1_data);
			int src1_offset = src1_offsetUI;

			unsigned int   dest_pixel_stride;
			unsigned int   dest_scanline_stride;
			unsigned int   dest_offset;
			Xil_unsigned8* dest_data;
			dest_storage.getStorageInfo(band,
						    &dest_pixel_stride,
						    &dest_scanline_stride,
						    &dest_offset,
						    (void**)&dest_data);
			
			int ptrX = src_box_x + x;
			Xil_unsigned8* src1 = src1_data +
			    (y * src1_scanline_stride) + ((x + src1_offset) / 8);
			src1_offset = (x + src1_offset) % 8;

			while(src1_offset - keyx < 0) {
			    src1--;
			    src1_offset += 8;
			}
			
			Xil_unsigned8* dest = dest_data +
			    (y*dest_scanline_stride) + ((x + dest_offset) / 8);
			dest_offset = (x + dest_offset) % 8;
		    
			//
			//  Each Scanline...
			//
			int imageWidth = src1Image->getWidth();
			int last = imageWidth - ptrX - 1 + src1_offset;
			for(j = 0; j < ysize; j++) {
			    int kh;
			    for(kh = 0; kh < height; kh++) {
				src_scan[kh] = (bit_base_t*) (src1 + (kh - keyy) * src1_scanline_stride);
			    }
			    
			    //
			    //  Each Pixel...
			    //
			    for(i = 0; i < xsize; i++) {
				int kw;
				float fsum = 0.0;
				float* kptr = kdata;
				int bit_offset = src1_offset + i - keyx;
				
				for(kh = 0; kh < height; kh++) {
				    for(kw = 0; kw < keyx + imageWidth - ptrX - i; kw++) {
					if (XIL_BMAP_TST(src_scan[kh], bit_offset + kw)) {
					    fsum += *kptr;
					}
					kptr++;
				    }
				    for(kw = keyx + imageWidth - ptrX - i; kw < width; kw++) {
					if (XIL_BMAP_TST(src_scan[kh], last)) {
					    fsum += *kptr;
					}
					kptr++;
				    }
				}
			    
				if (_XILI_ROUND_1(fsum) == 0) {
				    XIL_BMAP_CLR(dest, dest_offset + i);
				} else {
				    XIL_BMAP_SET(dest, dest_offset + i);
				}
			    }
			
			src1 += src1_scanline_stride;
			dest += dest_scanline_stride;
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
			unsigned int   src1_offsetUI;
			Xil_unsigned8* src1_data;
			src1_storage.getStorageInfo(band,
						    &src1_pixel_stride,
						    &src1_scanline_stride,
						    &src1_offsetUI,
						    (void**)&src1_data);
			int src1_offset = src1_offsetUI;

			unsigned int   dest_pixel_stride;
			unsigned int   dest_scanline_stride;
			unsigned int   dest_offset;
			Xil_unsigned8* dest_data;
			dest_storage.getStorageInfo(band,
						    &dest_pixel_stride,
						    &dest_scanline_stride,
						    &dest_offset,
						    (void**)&dest_data);
			
			int ptrX = src_box_x + x;
			int ptrY = src_box_y + y;
			Xil_unsigned8* src1 = src1_data +
			    (y * src1_scanline_stride) + ((x + src1_offset) / 8);
			src1_offset = (x + src1_offset) % 8;

			//
			// Get to top
			// Y should be clamping to 0.
			//
			Xil_unsigned8* dest = dest_data +
			    (y*dest_scanline_stride) + ((x + dest_offset) / 8);
			dest_offset = (x + dest_offset) % 8;
		    
			int imageHeight = src1Image->getHeight();
			//
			//  Each Scanline...
			//
			float* kptr = kdata;
			for(j = 0; j < ysize; j++) {
			    int kh;
			    for(kh = 0; kh < imageHeight - ptrY + keyy - j; kh++) {
				src_scan[kh] = (bit_base_t*) (src1 + (kh - keyy) * src1_scanline_stride);
			    }
			    for(kh = imageHeight - ptrY + keyy - j; kh < height; kh++) {
				src_scan[kh] = src_scan[kh - 1];
			    }
			    
			    //
			    //  Each Pixel...
			    //
			    for(i = 0; i < xsize; i++) {
				int kw;
				float fsum = 0.0;
				int bit_offset = src1_offset + i - keyx;
				
				for(kh = 0; kh < height; kh++) {
				    for(kw = 0; kw < keyx - ptrX - i; kw++) {
					if (XIL_BMAP_TST(src_scan[kh], src1_offset)) {
					    fsum += *(kptr + kh * width + kw);
					}
				    }
				    for(kw = keyx - ptrX - i; kw < width; kw++) {
					if (XIL_BMAP_TST(src_scan[kh], bit_offset + kw)) {
					    fsum += *(kptr + kh * width + kw);
					}
				    }
				}
			    
				if (_XILI_ROUND_1(fsum) == 0) {
				    XIL_BMAP_CLR(dest, dest_offset + i);
				} else {
				    XIL_BMAP_SET(dest, dest_offset + i);
				}
			    }
			
			src1 += src1_scanline_stride;
			dest += dest_scanline_stride;
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
			unsigned int   src1_offsetUI;
			Xil_unsigned8* src1_data;
			src1_storage.getStorageInfo(band,
						    &src1_pixel_stride,
						    &src1_scanline_stride,
						    &src1_offsetUI,
						    (void**)&src1_data);
			int src1_offset = src1_offsetUI;

			unsigned int   dest_pixel_stride;
			unsigned int   dest_scanline_stride;
			unsigned int   dest_offset;
			Xil_unsigned8* dest_data;
			dest_storage.getStorageInfo(band,
						    &dest_pixel_stride,
						    &dest_scanline_stride,
						    &dest_offset,
						    (void**)&dest_data);
			
			int ptrY = src_box_y + y;
			Xil_unsigned8* src1 = src1_data +
			    (y * src1_scanline_stride) + ((x + src1_offset) / 8);
			src1_offset = (x + src1_offset) % 8;

			while(src1_offset - keyx < 0) {
			    src1--;
			    src1_offset += 8;
			}
		    
			Xil_unsigned8* dest = dest_data +
			    (y*dest_scanline_stride) + ((x + dest_offset) / 8);
			dest_offset = (x + dest_offset) % 8;

			int imageHeight = src1Image->getHeight();
			//
			//  Each Scanline...
			//
			float* kptr = kdata;
			for(j = 0; j < ysize; j++) {
			    int kh;
			    for(kh = 0; kh < (imageHeight - ptrY) + keyy - j; kh++) {
				src_scan[kh] = (bit_base_t*) (src1 + src1_scanline_stride * (kh - keyy));
			    }
			    for(kh = (imageHeight - ptrY) + keyy - j; kh < height; kh++) {
				src_scan[kh] = src_scan[kh - 1];
			    }
			    
			    //
			    //  Each Pixel...
			    //
			    for(i = 0; i < xsize; i++) {
				int kw;
				float fsum = 0.0;
				int bit_offset = src1_offset + i - keyx;
				
				for(kh = 0; kh < height; kh++) {
				    for(kw = 0; kw < width; kw++) {
					if (XIL_BMAP_TST(src_scan[kh], bit_offset + kw)) {
					    fsum += *(kptr + kh * width + kw);
					}
				    }
				}
			    
				if (_XILI_ROUND_1(fsum) == 0) {
				    XIL_BMAP_CLR(dest, dest_offset + i);
				} else {
				    XIL_BMAP_SET(dest, dest_offset + i);
				}
			    }
			
			src1 += src1_scanline_stride;
			dest += dest_scanline_stride;
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
			unsigned int   src1_offsetUI;
			Xil_unsigned8* src1_data;
			src1_storage.getStorageInfo(band,
						    &src1_pixel_stride,
						    &src1_scanline_stride,
						    &src1_offsetUI,
						    (void**)&src1_data);
			int src1_offset = src1_offsetUI;
			
			unsigned int   dest_pixel_stride;
			unsigned int   dest_scanline_stride;
			unsigned int   dest_offset;
			Xil_unsigned8* dest_data;
			dest_storage.getStorageInfo(band,
						    &dest_pixel_stride,
						    &dest_scanline_stride,
						    &dest_offset,
						    (void**)&dest_data);
			
			int ptrX = src_box_x + x;
			int ptrY = src_box_y + y;
			Xil_unsigned8* src1 = src1_data +
			    (y * src1_scanline_stride) + ((x + src1_offset) / 8);
			src1_offset = (x + src1_offset) % 8;

			//
			// Get to top left corner.
			// Y should be clamping to 0.
			//
			int kulY = ptrY - keyy;
			if (kulY < 0)
			    kulY = 0;
			src1 -= (ptrY - kulY) * src1_scanline_stride;

			while(src1_offset - keyx < 0) {
			    src1--;
			    src1_offset += 8;
			}

			Xil_unsigned8* dest = dest_data +
			    (y*dest_scanline_stride) + ((x + dest_offset) / 8);
			dest_offset = (x + dest_offset) % 8;

			int imageHeight = src1Image->getHeight();
			//
			//  Each Scanline...
			//
			int imageWidth = src1Image->getWidth();
			int last = imageWidth - ptrX - 1 + src1_offset;
			for(j = 0; j < ysize; j++) {
			    int kh;
			    src_scan[0] = (bit_base_t*) src1;
			    for(kh = 1; kh <= (imageHeight - ptrY) + keyy - j; kh++) {
				src_scan[kh] = src_scan[kh - 1] + src1_scanline_stride;
			    }
			    for(kh = (imageHeight - ptrY) + keyy - j; kh < height; kh++) {
				src_scan[kh] = src_scan[kh - 1];
			    }
			    
			    //
			    //  Each Pixel...
			    //
			    for(i = 0; i < xsize; i++) {
				int kw;
				float fsum = 0.0;
				float* kptr = kdata;
				int bit_offset = src1_offset + i - keyx;
				
				for(kh = 0; kh < height; kh++) {
				    for(kw = 0; kw < keyx + imageWidth - ptrX - i; kw++) {
					if (XIL_BMAP_TST(src_scan[kh], bit_offset + kw))
					    fsum += *kptr;
					kptr++;
				    }
				    for(kw = keyx + imageWidth - ptrX - i; kw < width; kw++) {
					if (XIL_BMAP_TST(src_scan[kh], last))
					    fsum += *kptr;
					kptr++;
				    }
				}
			    
				if (_XILI_ROUND_1(fsum) == 0)
				    XIL_BMAP_CLR(dest, dest_offset + i);
				else
				    XIL_BMAP_SET(dest, dest_offset + i);
			    }
			
			src1 += src1_scanline_stride;
			dest += dest_scanline_stride;
			}
		    }
		    break;
		}
	    }
	} else {
	    //
	    // Processing Center box
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
		    unsigned int   src1_offsetUI;
		    Xil_unsigned8* src1_data;
		    src1_storage.getStorageInfo(band,
						&src1_pixel_stride,
						&src1_scanline_stride,
						&src1_offsetUI,
						(void**)&src1_data);
		    int src1_offset = src1_offsetUI;
		    
		    unsigned int   dest_pixel_stride;
		    unsigned int   dest_scanline_stride;
		    unsigned int   dest_offset;
		    Xil_unsigned8* dest_data;
		    dest_storage.getStorageInfo(band,
						&dest_pixel_stride,
						&dest_scanline_stride,
						&dest_offset,
						(void**)&dest_data);
		    
		    Xil_unsigned8* src1 = src1_data +
			(y*src1_scanline_stride) + ((x + src1_offset) / 8) -
			(keyy * src1_scanline_stride);
		    int kernel_offset = ((x + src1_offset) % 8) - keyx;
		    if (kernel_offset < 0) {
			src1--;
			kernel_offset += 8;
		    }
		    
		    Xil_unsigned8* dest = dest_data +
			(y*dest_scanline_stride) + ((x + dest_offset) / 8);
		    dest_offset = (x + dest_offset) % 8;
		    
		    unsigned int scanline_count = ysize;
		    
		    //
		    //  Each Scanline...
		    //
		    do {
			int kh;
			src_scan[0] = (bit_base_t*) src1;			
			for(kh = 1; kh < height; kh++) {
			    src_scan[kh] = src_scan[kh - 1] + src1_scanline_stride;
			}
			
			//
			//  Each Pixel...
			//
			for(i = 0; i < xsize; i++) {
			    int kw;
			    float fsum = 0.0;
			    float* kptr = kdata;
			    int bit_offset = kernel_offset + i;
				
			    for(kh = 0; kh < height; kh++) {
				for(kw = 0; kw < width; kw++) {
				    if (XIL_BMAP_TST(src_scan[kh], bit_offset + kw))
					fsum += *kptr;
				    kptr++;
				}
			    }
			    
			    if (_XILI_ROUND_1(fsum) == 0)
				XIL_BMAP_CLR(dest, dest_offset + i);
			    else
				XIL_BMAP_SET(dest, dest_offset + i);
			}
			
			src1 += src1_scanline_stride;
			dest += dest_scanline_stride;
		    } while(--scanline_count);
		}
	    }
	}
    }

    delete [] src_scan;
    return XIL_SUCCESS;
}

