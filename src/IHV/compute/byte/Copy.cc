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
//  File:	Copy.cc
//  Project:	XIL
//  Revision:	1.14
//  Last Mod:	10:10:05, 03/10/00
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
#pragma ident	"@(#)Copy.cc	1.14\t00/03/10  "

#include <string.h>
#include "XilDeviceManagerComputeBYTE.hh"
#include "ComputeInfo.hh"

XilStatus
XilDeviceManagerComputeBYTE::Copy(XilOp*       op,
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
    XilImage* src = op->getSrcImage(1);
    XilImage* dst = op->getDstImage(1);

    //
    //  Store away the number of bands for this operation.
    //    
    unsigned int nbands   = dst->getNumBands();

    //
    //  Loop over each of the boxes of storage we are to process.
    //
    XilBox* src_box;
    XilBox* dst_box;
    while(bl->getNext(&src_box, &dst_box)) {
        //
        //  Aquire storage from the images.  The storage returned is valid
        //  for the box given.  Thus, any origins or child offsets have been
        //  taken into account.
        //
        XilStorage src_storage(src);
        XilStorage dst_storage(dst);
        if((src->getStorage(&src_storage, op, src_box, "XilMemory",
                            XIL_READ_ONLY)  == XIL_FAILURE) ||
           (dst->getStorage(&dst_storage, op, dst_box, "XilMemory",
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
        //  Initialize the direction flags.
        //
	int bottom_up     = FALSE;
	int right_to_left = FALSE;

	//
        //  Test to see if all of our storage is of type XIL_PIXEL_SEQUENTIAL.
        //  If so, implement an loop optimized fro pixel-sequential storage.
        //
        if((src_storage.isType(XIL_PIXEL_SEQUENTIAL)) &&
	   (dst_storage.isType(XIL_PIXEL_SEQUENTIAL))) {
            //
            //  Both source and destination are pixel sequential
            //
            unsigned int   src_pixel_stride;
            unsigned int   src_scanline_stride;
            Xil_unsigned8* src_data;
            src_storage.getStorageInfo(&src_pixel_stride,
                                       &src_scanline_stride,
                                       NULL, NULL,
                                       (void**)&src_data);
            
            unsigned int   dst_pixel_stride;
            unsigned int   dst_scanline_stride;
            Xil_unsigned8* dst_data;
            dst_storage.getStorageInfo(&dst_pixel_stride,
                                        &dst_scanline_stride,
                                        NULL, NULL,
                                        (void**)&dst_data);

            //
            //  Create a list of rectangles to loop over.  The resulting list
            //  of rectangles is the area left by intersecting the ROI with
            //  the destination box.
            //
            XilRectList    rl(roi, dst_box);
	    
            int            x;
            int            y;
            unsigned int   xsize;
            unsigned int   ysize;
            while(rl.getNext(&x, &y, &xsize, &ysize)) {
                Xil_unsigned8* src_scanline = src_data +
                    (y*src_scanline_stride) + (x*src_pixel_stride);
                
                Xil_unsigned8* dst_scanline = dst_data +
                    (y*dst_scanline_stride) + (x*dst_pixel_stride);

                //
                //  Check to see if we should proceed top to bottom OR bottom to top
                //  Check to see if we should proceed left to right OR right to left
                //
                //  NOTE: You cannot go bottom to top AND right to left.
                //        This is because left to right is only important when the
                //        areas to be copied are on the same "scanline" otherwise
                //        the vertical direction is all that matters.
                //
                unsigned int src_end =
                    (unsigned int)(src_scanline +
                                   (ysize - 1) * src_scanline_stride + 
                                   (xsize - 1) * src_pixel_stride    +
                                   (nbands - 1));

                //
                //  Default copy is top to bottom and left to right.
                //
                //  If the destination overlaps the source and the destination
                //  has a lower address in memory, then copy either bottom to
                //  top OR right to left. 
                //
                if(((unsigned int)dst_scanline <= src_end) &&
                   ((unsigned int)dst_scanline >= (unsigned int)src_scanline)) {
                    unsigned int src_end_scanline =
                        (unsigned int)(src_scanline +
                                       (xsize - 1) * src_pixel_stride +
                                       (nbands - 1));

                    if((unsigned int)dst_scanline <= src_end_scanline) {
                        right_to_left = TRUE;
                    } else {
                        bottom_up = TRUE;
                    }
                }

                if(bottom_up) {
                    src_scanline += (ysize - 1) * src_scanline_stride; 
                    dst_scanline += (ysize - 1) * dst_scanline_stride;

                    src_scanline_stride = -src_scanline_stride;
                    dst_scanline_stride = -dst_scanline_stride;
                }

                //
                //  Special cases for packed n-band image.
                //
                if((nbands == src_pixel_stride) &&
                   (nbands == dst_pixel_stride)) {
                    unsigned int scan_size =
                        xsize * nbands * sizeof(Xil_unsigned8);

                    //
                    //  Can we block copy?
                    //
                    if((scan_size == src_scanline_stride*sizeof(Xil_unsigned8))
                       &&
                       (scan_size == dst_scanline_stride*sizeof(Xil_unsigned8))) {
                        //
                        //  Use memcpy to do the whole block.
                        //
                        if(right_to_left || bottom_up) {
                            memmove(dst_scanline, src_scanline, scan_size*ysize);
                        } else {
                            xili_memcpy(dst_scanline, src_scanline, scan_size*ysize);
                        }
                    } else {
                        //
                        //  Use xili_memcpy() to do scanline at a time.
                        //
                        if(right_to_left) {
                            do {
                                memmove(dst_scanline, src_scanline, scan_size);

                                src_scanline += src_scanline_stride;
                                dst_scanline += dst_scanline_stride;
                            } while (--ysize);
                        } else {
                            do {
                                xili_memcpy(dst_scanline, src_scanline, scan_size);

                                src_scanline += src_scanline_stride;
                                dst_scanline += dst_scanline_stride;
                            } while (--ysize);
                        }
                    }
                } else {
                    //
                    //  Not a packed image.  More special cases for special
                    //  band types...
                    //
                    if(nbands == 1) {
                        Xil_unsigned8* src_pixel;
                        Xil_unsigned8* dst_pixel;

                        if(right_to_left) {
                            src_scanline += src_scanline_stride - src_pixel_stride;
                            dst_scanline += dst_scanline_stride - dst_pixel_stride;

                            for(int i = ysize; i > 0; i--) {
                                src_pixel = src_scanline;
                                dst_pixel = dst_scanline;

                                for(int j = xsize; j > 0; j--) {
                                    *dst_pixel = *src_pixel;

                                    src_pixel -= src_pixel_stride;
                                    dst_pixel -= dst_pixel_stride;
                                }
                                src_scanline += src_scanline_stride;
                                dst_scanline += dst_scanline_stride;
                            }
                        } else {
                            for(int i = ysize; i > 0; i--) {
                                src_pixel = src_scanline;
                                dst_pixel = dst_scanline;

                                for(int j = xsize; j > 0; j--) {
                                    *dst_pixel = *src_pixel;

                                    src_pixel += src_pixel_stride;
                                    dst_pixel += dst_pixel_stride;
                                }
                                
                                src_scanline += src_scanline_stride;
                                dst_scanline += dst_scanline_stride;
                            }
                        }
                    } else if(nbands == 3) {
                        Xil_unsigned8* src_pixel;
                        Xil_unsigned8* dst_pixel;

                        if(right_to_left) {
                            src_scanline += src_scanline_stride - src_pixel_stride;
                            dst_scanline += dst_scanline_stride - dst_pixel_stride;

                            for(int i = ysize; i > 0; i--) {
                                src_pixel = src_scanline;
                                dst_pixel = dst_scanline;

                                for(int j = xsize; j > 0; j--) {
                                    *(dst_pixel+2) = *(src_pixel+2);
                                    *(dst_pixel+1) = *(src_pixel+1);
                                    *dst_pixel     = *src_pixel;

                                    src_pixel -= src_pixel_stride;
                                    dst_pixel -= dst_pixel_stride;
                                }
                                
                                src_scanline += src_scanline_stride;
                                dst_scanline += dst_scanline_stride;
                            }
                        } else {
                            for(int i = ysize; i > 0; i--) {
                                src_pixel = src_scanline;
                                dst_pixel = dst_scanline;

                                for(int j = xsize; j > 0; j--) {
                                    *dst_pixel     = *src_pixel;
                                    *(dst_pixel+1) = *(src_pixel+1);
                                    *(dst_pixel+2) = *(src_pixel+2);

                                    src_pixel += src_pixel_stride;
                                    dst_pixel += dst_pixel_stride;
                                }
                                
                                src_scanline += src_scanline_stride;
                                dst_scanline += dst_scanline_stride;
                            }
                        }
                    } else {
                        Xil_unsigned8* src_pixel;
                        Xil_unsigned8* src_band;
                        Xil_unsigned8* dst_pixel;
                        Xil_unsigned8* dst_band;

                        if(right_to_left) {
                            src_scanline += src_scanline_stride - src_pixel_stride;
                            dst_scanline += dst_scanline_stride - dst_pixel_stride;

                            for(int i = ysize; i > 0; i--) {
                                src_pixel = src_scanline;
                                dst_pixel = dst_scanline;

                                for(int j = xsize; j > 0; j--) {
                                    src_band = src_pixel;
                                    dst_band = dst_pixel;

                                    for(int band = nbands; band > 0; band--) {
                                        *dst_band++ = *src_band++;
                                    }

                                    src_pixel -= src_pixel_stride;
                                    dst_pixel -= dst_pixel_stride;
                                }
                                src_scanline += src_scanline_stride;
                                dst_scanline += dst_scanline_stride;
                            }
                        } else {
                            for(int i = ysize; i > 0; i--) {
                                src_pixel = src_scanline;
                                dst_pixel = dst_scanline;

                                for(int j = xsize; j > 0; j--) {
                                    src_band = src_pixel;
                                    dst_band = dst_pixel;
                                    
                                    for(int band = nbands; band > 0; band--) {
                                        *dst_band++ = *src_band++;
                                    }

                                    src_pixel += src_pixel_stride;
                                    dst_pixel += dst_pixel_stride;
                                }
                                
                                src_scanline += src_scanline_stride;
                                dst_scanline += dst_scanline_stride;
                            }
                        }
                    }
                }
            }
        } else if((src_storage.isType(XIL_BAND_SEQUENTIAL)) &&
                  (dst_storage.isType(XIL_BAND_SEQUENTIAL))) {
            //
            //  Since the images are band sequential, we can copy a line at a
            //  time.  Much faster than general where we can only copy a pixel
            //  at a time.
            //
            //
            //
            //  BAND_SEQUENTIAL and GENERAL storage
            //
            XilRectList  rl(roi, dst_box);

            int          x;
            int          y;
            unsigned int xsize;
            unsigned int ysize;
            while(rl.getNext(&x, &y, &xsize, &ysize)) {
                //
                //  Each Band...
                //
                for(unsigned int band=0; band<nbands; band++) {
                    unsigned int   src_pixel_stride;
                    unsigned int   src_scanline_stride;
                    Xil_unsigned8* src_data;
                    src_storage.getStorageInfo(band,
                                                &src_pixel_stride,
                                                &src_scanline_stride,
                                                NULL,
                                                (void**)&src_data);
            
                    unsigned int   dst_pixel_stride;
                    unsigned int   dst_scanline_stride;
                    Xil_unsigned8* dst_data;
                    dst_storage.getStorageInfo(band,
                                                &dst_pixel_stride,
                                                &dst_scanline_stride,
                                                NULL,
                                                (void**)&dst_data);

                    Xil_unsigned8* src_scanline = src_data +
                        (y*src_scanline_stride) + (x*src_pixel_stride);
                    
                    Xil_unsigned8* dst_scanline = dst_data +
                        (y*dst_scanline_stride) + (x*dst_pixel_stride);

                    unsigned int scanline_count = ysize;
                    
                    //
                    //  Check to see if we should proceed top to bottom OR
                    //    bottom to top.
                    //
                    //  Check to see if we should proceed left to right OR
                    //    right to left
                    //
                    //  NOTE: You cannot go bottom to top AND right to left.
                    //        This is because left to right is only important
                    //        when the areas to be copied are on the same
                    //        "scanline" otherwise the vertical direction is
                    //        all that matters. 
                    //
                    //
                    unsigned int src_end =
                        (unsigned int)(src_scanline +
                                       (ysize - 1) * src_scanline_stride + 
                                       (xsize - 1) * src_pixel_stride);

                    //
                    //  Default copy is top to bottom and left to right.
                    //
                    //  If the destination overlaps the source and the
                    //  destination has a lower address in memory, then copy
                    //  either bottom to top OR right to left. 
                    //
                    if(((unsigned int)dst_scanline <= src_end) &&
                       ((unsigned int)dst_scanline >= (unsigned int)src_scanline)) {
                        unsigned int src_end_scanline =
                            (unsigned int)(src_scanline +
                                           (xsize - 1) * src_pixel_stride);

                        if((unsigned int)dst_scanline <= src_end_scanline) {
                            right_to_left = TRUE;
                        } else {
                            bottom_up = TRUE;
                        }
                    }

                    if(bottom_up) {
                        src_scanline += (ysize - 1) * src_scanline_stride;
                        dst_scanline += (ysize - 1) * dst_scanline_stride;

                        src_scanline_stride = -src_scanline_stride;
                        dst_scanline_stride = -dst_scanline_stride;
                    }

                    unsigned int scan_size = xsize * sizeof(Xil_unsigned8);
                    
                    if(right_to_left) {
                        do {
                            memmove(dst_scanline, src_scanline, scan_size);

                            src_scanline += src_scanline_stride;
                            dst_scanline += dst_scanline_stride;
                        } while (--scanline_count);
                    } else {
                        do {
                            xili_memcpy(dst_scanline, src_scanline, scan_size);

                            src_scanline += src_scanline_stride;
                            dst_scanline += dst_scanline_stride;
                        } while (--scanline_count);
                    }
                }
            }
        } else {
            //
            //  And finally, GENERAL storage...
            //
            XilRectList  rl(roi, dst_box);

            int          x;
            int          y;
            unsigned int xsize;
            unsigned int ysize;
            while(rl.getNext(&x, &y, &xsize, &ysize)) {
                //
                //  Each Band...
                //
                for(unsigned int band=0; band<nbands; band++) {
                    unsigned int   src_pixel_stride;
                    unsigned int   src_scanline_stride;
                    Xil_unsigned8* src_data;
                    src_storage.getStorageInfo(band,
                                                &src_pixel_stride,
                                                &src_scanline_stride,
                                                NULL,
                                                (void**)&src_data);
            
                    unsigned int   dst_pixel_stride;
                    unsigned int   dst_scanline_stride;
                    Xil_unsigned8* dst_data;
                    dst_storage.getStorageInfo(band,
                                                &dst_pixel_stride,
                                                &dst_scanline_stride,
                                                NULL,
                                                (void**)&dst_data);

                    Xil_unsigned8* src_scanline = src_data +
                        (y*src_scanline_stride) + (x*src_pixel_stride);
                    
                    Xil_unsigned8* dst_scanline = dst_data +
                        (y*dst_scanline_stride) + (x*dst_pixel_stride);

                    //
                    //  Check to see if we should proceed top to bottom OR
                    //    bottom to top.
                    //
                    //  Check to see if we should proceed left to right OR
                    //    right to left
                    //
                    //  NOTE: You cannot go bottom to top AND right to left.
                    //        This is because left to right is only important
                    //        when the areas to be copied are on the same
                    //        "scanline" otherwise the vertical direction is
                    //        all that matters. 
                    //
                    //
                    unsigned int src_end =
                        (unsigned int)(src_scanline +
                                       (ysize - 1) * src_scanline_stride + 
                                       (xsize - 1) * src_pixel_stride);

                    //
                    //  Default copy is top to bottom and left to right.
                    //
                    //  If the destination overlaps the source and the
                    //  destination has a lower address in memory, then copy
                    //  either bottom to top OR right to left. 
                    //
                    if(((unsigned int)dst_scanline <= src_end) &&
                       ((unsigned int)dst_scanline >= (unsigned int)src_scanline)) {
                        unsigned int src_end_scanline =
                            (unsigned int)(src_scanline +
                                           (xsize - 1) * src_pixel_stride);

                        if((unsigned int)dst_scanline <= src_end_scanline) {
                            right_to_left = TRUE;
                        } else {
                            bottom_up = TRUE;
                        }
                    }

                    if(bottom_up) {
                        src_scanline += (ysize - 1) * src_scanline_stride;
                        dst_scanline += (ysize - 1) * dst_scanline_stride;

                        src_scanline_stride = -src_scanline_stride;
                        dst_scanline_stride = -dst_scanline_stride;
                    }

                    if(right_to_left) {
                        src_scanline += (xsize - 1) * src_pixel_stride;
                        dst_scanline += (xsize - 1) * dst_pixel_stride;

			for(int i = ysize; i > 0; i--) {
			    Xil_unsigned8* src_pixel = src_scanline;
			    Xil_unsigned8* dst_pixel = dst_scanline;

			    for(int j = xsize; j > 0; j--) {
				*dst_pixel = *src_pixel;

				src_pixel -= src_pixel_stride;
				dst_pixel -= dst_pixel_stride;
			    }

			    src_scanline += src_scanline_stride;
			    dst_scanline += dst_scanline_stride;
			}
		    } else {
                        for(int i = ysize; i > 0; i--) {
                            Xil_unsigned8* src_pixel = src_scanline;
                            Xil_unsigned8* dst_pixel = dst_scanline;

                            for(int j = xsize; j > 0; j--) {
				*dst_pixel = *src_pixel;

				src_pixel += src_pixel_stride;
				dst_pixel += dst_pixel_stride;
			    }
			    src_scanline += src_scanline_stride;
			    dst_scanline += dst_scanline_stride;
			}
		    }
		}
	    }
	}
    }

    return XIL_SUCCESS;
}

