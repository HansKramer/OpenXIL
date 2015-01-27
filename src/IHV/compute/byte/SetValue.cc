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
//  File:	SetValue.cc
//  Project:	XIL
//  Revision:	1.18
//  Last Mod:	10:10:08, 03/10/00
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
#pragma ident	"@(#)SetValue.cc	1.18\t00/03/10  "

#include "XilDeviceManagerComputeBYTE.hh"
#include "XiliUtils.hh"
#include <string.h>

XilStatus
XilDeviceManagerComputeBYTE::SetValuePreprocess(XilOp*        op,
                                                unsigned      ,
                                                XilRoi*       ,
                                                void**        compute_data,
                                                unsigned int* )
{
    unsigned int nbands = op->getDstImage(1)->getNumBands();

    if(nbands == 1) {
        *compute_data = (void*)TRUE;
    } else {
        //
        //  Check to see if the bands are contiguous.
        //
        Xil_unsigned8* in_vals;
        op->getParam(1, (void**)&in_vals);

        Xil_boolean    contiguous_values = TRUE;
        Xil_unsigned8  prev_value        = in_vals[0];

        for(int i=1; i<nbands; i++) {
            if(in_vals[i] != prev_value) {
                contiguous_values = FALSE;
            }
        }

        *compute_data = (void*)contiguous_values;
    }

    return XIL_SUCCESS;
}

XilStatus
XilDeviceManagerComputeBYTE::SetValue(XilOp*       op,
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
    //  Preprocess routine sets whether the values are contiguous.
    //
    Xil_boolean contiguous_values = FALSE; (Xil_boolean)op->getPreprocessData(this);

    //
    //  Get the image for our operation.
    //
    XilImage* dst = op->getDstImage(1);

    //
    //  Store away the number of bands for this operation.
    //    
    unsigned int nbands   = dst->getNumBands();

    //
    //  The values we're setting.
    //
    Xil_unsigned8* int_values;
    op->getParam(1, (void**)&int_values);

    //
    //  Loop over each of the boxes of storage we are to process.
    //
    XilBox* dst_box;
    while(bl->getNext(&dst_box)) {
        //
        //  Aquire storage from the images.  The storage returned is valid
        //  for the box given.  Thus, any origins or child offsets have been
        //  taken into account.
        //
        XilStorage dst_storage(dst);
        if(dst->getStorage(&dst_storage, op, dst_box, "XilMemory",
                           XIL_WRITE_ONLY) == XIL_FAILURE) {
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
        //  Test to see if our storage is of type XIL_PIXEL_SEQUENTIAL.
        //  If so, implement an loop optimized fro pixel-sequential storage.
        //
        if(dst_storage.isType(XIL_PIXEL_SEQUENTIAL)) {
            //
            //  The storage is pixel sequential
            //
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
                Xil_unsigned8* dst_scanline = dst_data +
                    (y*dst_scanline_stride) + (x*dst_pixel_stride);

                //
                //  Handle some special cases that greatly accelerate the
                //  operation for many common cases.
                //
                if(dst_pixel_stride == nbands) {
                    //
                    //  Packed data.  So, there are no inbetween pixels we need to
                    //  worry about maintaining.
                    //
                    if(nbands == 1) {
                        unsigned int value = int_values[0];

                        if(dst_scanline_stride == xsize) {
                            xili_memset(dst_scanline, value, xsize*ysize);
                        } else {
                            for(unsigned int i=ysize; i!=0; i--) {
                                xili_memset(dst_scanline, value, xsize);

                                dst_scanline += dst_scanline_stride;
                            }
                        }
                    } else {
                        Xil_unsigned8* first_scanline;
                        Xil_unsigned8* tmp_scanline;
                        Xil_unsigned8* next_scanline;
                        unsigned int   scanline_len   = xsize*nbands;

                        first_scanline = tmp_scanline = dst_scanline;

                        next_scanline  = first_scanline + dst_scanline_stride;

                        if(contiguous_values) {
                            unsigned int value = int_values[0];

                            if(dst_scanline_stride == scanline_len) {
                                xili_memset(dst_scanline, value, ysize*scanline_len);
                            } else {
                                //
                                //  Set each scanline
                                //
                                for(unsigned int i=ysize; i!=0; i--) {
                                    xili_memset(dst_scanline, value, scanline_len);

                                    dst_scanline += dst_scanline_stride;
                                }
                            }
                        } else {
                            //
                            //  Fill in 1st scanline
                            //
                            for(unsigned int i=xsize; i!=0; i--) {
                                for(unsigned int j=0; j<nbands; j++) {
                                    *(tmp_scanline + j) = *(int_values + j);
                                }

                                tmp_scanline += dst_pixel_stride;
                            }

                            //
                            //  Replicate scanline buffer
                            //
                            for(i=ysize - 1; i!=0; i--) {
                                xili_memcpy(next_scanline, first_scanline,
                                            scanline_len);

                                next_scanline += dst_scanline_stride;
                            }
                        }
                    }
                } else {
                    for(unsigned int j=ysize; j!=0; j--) {
                        Xil_unsigned8* dst_pixel = dst_scanline;

                        for(unsigned int i=xsize; i!=0; i--) {
                            Xil_unsigned8* dst_band = dst_pixel;

                            for(unsigned int b=0; b<nbands; b++) {
                                *dst_band = int_values[b];

                                dst_band++;
                            }

                            dst_pixel += dst_pixel_stride;
                        }

                        dst_scanline += dst_scanline_stride;
                    }
                }
            }
        } else {
            //
            //  General storage
            //
            for(unsigned int b=0; b<nbands; b++) {
                unsigned int   dst_pixel_stride;
                unsigned int   dst_scanline_stride;
                Xil_unsigned8* dst_data;
                dst_storage.getStorageInfo(b,
                                           &dst_pixel_stride,
                                           &dst_scanline_stride,
                                           NULL,
                                           (void**)&dst_data);

                //
                //  The value for this band.
                //
                unsigned int value = int_values[b];

                //
                //  Create a list of rectangles to loop over.  The resulting
                //  list of rectangles is the area left by intersecting the
                //  ROI with the destination box.
                //
                XilRectList    rl(roi, dst_box);
	    
                int            x;
                int            y;
                unsigned int   xsize;
                unsigned int   ysize;
                while(rl.getNext(&x, &y, &xsize, &ysize)) {
                    Xil_unsigned8* dst_scanline = dst_data +
                        (y*dst_scanline_stride) + (x*dst_pixel_stride);

                    if(dst_pixel_stride == 1) {
                        if(dst_scanline_stride == xsize) {
                            xili_memset(dst_scanline, value, xsize*ysize);
                        } else {
                            for(unsigned int j=ysize; j!=0; j--) {
                                xili_memset(dst_scanline, value, xsize);

                                dst_scanline += dst_scanline_stride;
                            }
                        }
                    } else {
                        for(unsigned int j=ysize; j!=0; j--) {
                            Xil_unsigned8* dst_pixel = dst_scanline;
                
                            for(unsigned int i=xsize; i!=0; i--) {
                                *dst_pixel = value;
                            
                                dst_pixel += dst_pixel_stride;
                            }

                            dst_scanline += dst_scanline_stride;
                        }
                    }
                }
            }
        }
    }

    return XIL_SUCCESS;
}

