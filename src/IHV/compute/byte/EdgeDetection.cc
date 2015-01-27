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
//  File:	EdgeDetection.cc
//  Project:	XIL
//  Revision:	1.13
//  Last Mod:	10:10:51, 03/10/00
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
#pragma ident	"@(#)EdgeDetection.cc	1.13\t00/03/10  "

#include "XilDeviceManagerComputeBYTE.hh"
#include "XiliUtils.hh"

XilStatus
XilDeviceManagerComputeBYTE::EdgeDetection(XilOp*       op,
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

    XilEdgeDetection    type;

    op->getParam(1, (int*)&type);
    if(type != XIL_EDGE_DETECT_SOBEL) {
	return XIL_FAILURE;
    }

    const unsigned int width  = 3;
    const unsigned int height = 3;
    const int          keyx   = 1;
    const int          keyy   = 1;

    //
    //  Get the images for our operation.
    //
    XilImage* src1Image = op->getSrcImage(1);
    XilImage* destImage = op->getDstImage(1);

    float* sq_lut = getSquaresOver4Table(destImage->getSystemState());
    if(sq_lut == NULL) {
        return XIL_FAILURE;
    }
    
    //
    //  Store away the number of bands for this operation.
    //    
    unsigned int nbands   = destImage->getNumBands();

    unsigned int i,j;
    
    //
    //  Loop over each of the boxes of storage we are to process.
    //
    XilBox* src1_box;
    XilBox* dest_box;
    while(bl->getNext(&src1_box, &dest_box)) {
        XilBoxAreaType tag = (XilBoxAreaType)((long)dest_box->getTag());
        switch (tag) {
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
            //  We do not currently support edge detect
            //  for sources smaller than the kernel.
            //  Don't mark the box as failed since there's no
            //  other implementation to fall to.
            //
            XIL_ERROR(destImage->getSystemState(), XIL_ERROR_INTERNAL,
                      "di-447", TRUE);
            continue;

          default :
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
        if ((src1Image->getStorage(&src1_storage, op, src1_box, "XilMemory",
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

        if(tag != XIL_AREA_CENTER) {
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
            // So it's an edge box and only edge extend is allowed in this routine
            //
            XilRectList  rl(roi, dest_box);
            
            int          x;
            int          y;
            unsigned int xsize;
            unsigned int ysize;
            while(rl.getNext(&x, &y, &xsize, &ysize)) {
                unsigned int band;
                switch (tag) {
                  case XIL_AREA_TOP_LEFT_CORNER:
                    //
                    //  Each Band...
                    //
                    for(band=0; band<nbands; band++) {
                        unsigned int   src1_pixel_stride;
                        unsigned int   src1_scanline_stride;
                        Xil_unsigned8* src1_data;
                        src1_storage.getStorageInfo(band,
                                                    &src1_pixel_stride,
                                                    &src1_scanline_stride,
                                                    NULL,
                                                    (void**)&src1_data);

                        unsigned int   dest_pixel_stride;
                        unsigned int   dest_scanline_stride;
                        Xil_unsigned8* dest_data;
                        dest_storage.getStorageInfo(band,
                                                    &dest_pixel_stride,
                                                    &dest_scanline_stride,
                                                    NULL,
                                                    (void**)&dest_data);
                        
                        Xil_unsigned8* src1_scanline = src1_data +
                            (y*src1_scanline_stride) + (x*src1_pixel_stride);
                        Xil_unsigned8* dest_scanline = dest_data +
                            (y*dest_scanline_stride) + (x*dest_pixel_stride);

                        int ptrX = src_box_x + x;
                        int ptrY = src_box_y + y;
                        int kulX = ptrX - keyx;
                        int kulY = ptrY - keyy;
                        //
                        // Clamp kernel upper left and since this is the
                        // upper left corner, both should clamp to 0.
                        //
                        if (kulX < 0) kulX = 0;
                        if (kulY < 0) kulY = 0;
                        Xil_unsigned8* up_left = src1_scanline +
                            ((kulX - ptrX) * src1_pixel_stride) +
                            ((kulY - ptrY) * src1_scanline_stride);

                        //
                        // Do the cases where the kernel extends above or meets the top
                        // of the image.
                        //
                        for (j = 0; j < ysize; j++) {
                            // point to the first pixel of the scanline 
                            Xil_unsigned8* dest_pixel = dest_scanline;

                            for (i = 0; i < xsize; i++) {
                                Xil_unsigned8* dest = dest_pixel;
                                
                                int s0 = *(up_left);
                                int s1 = *(up_left);
                                int s2 = *(up_left + src1_pixel_stride);
                                int s3 = *(up_left);
                                int s5 = *(up_left + src1_pixel_stride);
                                int s6 = *(up_left + src1_scanline_stride);
                                int s7 = *(up_left + src1_scanline_stride);
                                int s8 = *(up_left + src1_scanline_stride + src1_pixel_stride);

                                int vsum = s0 + (s3 << 1) + s6 - s2 - (s5 << 1) -s8;
                                int hsum = s0 + (s1 << 1) + s2 - s6 - (s7 << 1) - s8;

                                float fsum = (float)sqrt(sq_lut[vsum] + sq_lut[hsum]);
                                
                                *dest = _XILI_ROUND_U8(fsum);
                                
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
                        Xil_unsigned8* src1_data;
                        src1_storage.getStorageInfo(band,
                                                    &src1_pixel_stride,
                                                    &src1_scanline_stride,
                                                    NULL,
                                                    (void**)&src1_data);
            
                        unsigned int   dest_pixel_stride;
                        unsigned int   dest_scanline_stride;
                        Xil_unsigned8* dest_data;
                        dest_storage.getStorageInfo(band,
                                                    &dest_pixel_stride,
                                                    &dest_scanline_stride,
                                                    NULL,
                                                    (void**)&dest_data);
                        
                        Xil_unsigned8* src1_scanline = src1_data +
                            (y*src1_scanline_stride) + (x*src1_pixel_stride);

                        Xil_unsigned8* dest_scanline = dest_data +
                            (y*dest_scanline_stride) + (x*dest_pixel_stride);

                        //
                        // Do the cases where the kernel extends above or meets the top
                        // of the image.
                        //
                        for (j = 0; j < ysize; j++) {
                            // point to the first pixel of the scanline 
                            Xil_unsigned8* src1_pixel = src1_scanline;
                            Xil_unsigned8* dest_pixel = dest_scanline;
                            
                            for (i = 0; i < xsize; i++) {
                                Xil_unsigned8* dest = dest_pixel;

                                int s0 = *(src1_pixel - src1_pixel_stride);
                                int s1 = *src1_pixel;
                                int s2 = *(src1_pixel + src1_pixel_stride);
                                int s3 = *(src1_pixel - src1_pixel_stride);
                                int s5 = *(src1_pixel + src1_pixel_stride);
                                int s6 = *(src1_pixel + src1_scanline_stride - src1_pixel_stride);
                                int s7 = *(src1_pixel + src1_scanline_stride);
                                int s8 = *(src1_pixel + src1_scanline_stride + src1_pixel_stride);
                                
                                int vsum = s0 + (s3 << 1) + s6 - s2 - (s5 << 1) -s8;
                                int hsum = s0 + (s1 << 1) + s2 - s6 - (s7 << 1) - s8;

                                float fsum = (float)sqrt(sq_lut[vsum] + sq_lut[hsum]);
                                
                                *dest = _XILI_ROUND_U8(fsum);
                                
                                src1_pixel += src1_pixel_stride;
                                dest_pixel += dest_pixel_stride;
                            }
                            
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
                        Xil_unsigned8* src1_data;
                        src1_storage.getStorageInfo(band,
                                                    &src1_pixel_stride,
                                                    &src1_scanline_stride,
                                                    NULL,
                                                    (void**)&src1_data);
            
                        unsigned int   dest_pixel_stride;
                        unsigned int   dest_scanline_stride;
                        Xil_unsigned8* dest_data;
                        dest_storage.getStorageInfo(band,
                                                    &dest_pixel_stride,
                                                    &dest_scanline_stride,
                                                    NULL,
                                                    (void**)&dest_data);
                        
                        Xil_unsigned8* src1_scanline = src1_data +
                            (y*src1_scanline_stride) + (x*src1_pixel_stride);

                        Xil_unsigned8* dest_scanline = dest_data +
                            (y*dest_scanline_stride) + (x*dest_pixel_stride);

                        int ptrX = src_box_x + x;
                        int ptrY = src_box_y + y;
                        int kulX = ptrX + width - keyx - 1;
                        int kulY = ptrY - keyy;
                        //
                        // Clamp kernel top right        
                        // Y should be clamping to 0.
                        // X should clamp to image width
                        //
                        int imageWidth = src1Image->getWidth();
                        if (kulX > imageWidth - 1)
                            kulX = imageWidth - 1;
                        if (kulY < 0)
                            kulY = 0;
                        Xil_unsigned8* top_right = src1_scanline +
                            ((kulX - ptrX) * src1_pixel_stride) +
                            ((kulY - ptrY) * src1_scanline_stride);
                        
                        //
                        // Do the cases where the kernel extends above or meets the top
                        // of the image.
                        //
                        for (j = 0; j < ysize; j++) {
                            // point to the first pixel of the scanline 
                            Xil_unsigned8* dest_pixel = dest_scanline;
                            for (i = 0; i < xsize; i++) {
                                Xil_unsigned8* dest = dest_pixel;
                                
                                int s0 = *(top_right - src1_pixel_stride);
                                int s1 = *top_right;
                                int s2 = *(top_right);
                                int s3 = *(top_right - src1_pixel_stride);
                                int s5 = *(top_right);
                                int s6 = *(top_right + src1_scanline_stride - src1_pixel_stride);
                                int s7 = *(top_right + src1_scanline_stride);
                                int s8 = *(top_right + src1_scanline_stride);

                                int vsum = s0 + (s3 << 1) + s6 - s2 - (s5 << 1) -s8;
                                int hsum = s0 + (s1 << 1) + s2 - s6 - (s7 << 1) - s8;
                                
                                float fsum = (float)sqrt(sq_lut[vsum] + sq_lut[hsum]);

                                *dest = _XILI_ROUND_U8(fsum);
                                
                                dest_pixel += dest_pixel_stride;
                                }
                            
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
                        Xil_unsigned8* src1_data;
                        src1_storage.getStorageInfo(band,
                                                    &src1_pixel_stride,
                                                    &src1_scanline_stride,
                                                    NULL,
                                                    (void**)&src1_data);
                        
                        unsigned int   dest_pixel_stride;
                        unsigned int   dest_scanline_stride;
                        Xil_unsigned8* dest_data;
                        dest_storage.getStorageInfo(band,
                                                    &dest_pixel_stride,
                                                    &dest_scanline_stride,
                                                    NULL,
                                                    (void**)&dest_data);
                        
                        Xil_unsigned8* src1_scanline = src1_data +
                            + (x*src1_pixel_stride) + (y*src1_scanline_stride);

                        Xil_unsigned8* dest_scanline = dest_data +
                            + (x*dest_pixel_stride) + (y*dest_scanline_stride);

                        for (j = 0; j < ysize; j++) {
                            Xil_unsigned8* src1_pixel = src1_scanline;
                            Xil_unsigned8* dest_pixel = dest_scanline;
                            
                            for (i = 0; i < xsize; i++) {
                                Xil_unsigned8* dest = dest_pixel;
                                
                                int s0 = *(src1_pixel - src1_scanline_stride);
                                int s1 = *(src1_pixel - src1_scanline_stride);
                                int s2 = *(src1_pixel - src1_scanline_stride + src1_pixel_stride);
                                int s3 = *src1_pixel;
                                int s5 = *(src1_pixel + src1_pixel_stride);
                                int s6 = *(src1_pixel + src1_scanline_stride);
                                int s7 = *(src1_pixel + src1_scanline_stride);
                                int s8 = *(src1_pixel + src1_scanline_stride + src1_pixel_stride);

                                int vsum = s0 + (s3 << 1) + s6 - s2 - (s5 << 1) -s8;
                                int hsum = s0 + (s1 << 1) + s2 - s6 - (s7 << 1) - s8;
                                
                                float fsum = (float)sqrt(sq_lut[vsum] + sq_lut[hsum]);

                                *dest = _XILI_ROUND_U8(fsum);
                                
                                src1_pixel += src1_pixel_stride;
                                dest_pixel += dest_pixel_stride;
                            }
                            
                            src1_scanline += src1_scanline_stride;
                            dest_scanline += dest_scanline_stride;
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
                        Xil_unsigned8* src1_data;
                        src1_storage.getStorageInfo(band,
                                                    &src1_pixel_stride,
                                                    &src1_scanline_stride,
                                                    NULL,
                                                    (void**)&src1_data);
            
                        unsigned int   dest_pixel_stride;
                        unsigned int   dest_scanline_stride;
                        Xil_unsigned8* dest_data;
                        dest_storage.getStorageInfo(band,
                                                    &dest_pixel_stride,
                                                    &dest_scanline_stride,
                                                    NULL,
                                                    (void**)&dest_data);
                        
                        Xil_unsigned8* src1_scanline = src1_data +
                            (y*src1_scanline_stride) + (x*src1_pixel_stride);

                        Xil_unsigned8* dest_scanline = dest_data +
                            (y*dest_scanline_stride) + (x*dest_pixel_stride);

                        for (j = 0; j < ysize; j++) {
                            Xil_unsigned8* src1_pixel = src1_scanline;
                            Xil_unsigned8* dest_pixel = dest_scanline;
                            
                            for (i = 0; i < xsize; i++) {
                                Xil_unsigned8* dest = dest_pixel;
                                
                                int s0 = *(src1_pixel - src1_scanline_stride - src1_pixel_stride);
                                int s1 = *(src1_pixel - src1_scanline_stride);
                                int s2 = *(src1_pixel - src1_scanline_stride);
                                int s3 = *(src1_pixel - src1_pixel_stride);
                                int s5 = *src1_pixel;
                                int s6 = *(src1_pixel + src1_scanline_stride - src1_pixel_stride);
                                int s7 = *(src1_pixel + src1_scanline_stride);
                                int s8 = *(src1_pixel + src1_scanline_stride);

                                int vsum = s0 + (s3 << 1) + s6 - s2 - (s5 << 1) -s8;
                                int hsum = s0 + (s1 << 1) + s2 - s6 - (s7 << 1) - s8;
                                
                                float fsum = (float)sqrt(sq_lut[vsum] + sq_lut[hsum]);

                                *dest = _XILI_ROUND_U8(fsum);
                                
                                src1_pixel += src1_pixel_stride;
                                dest_pixel += dest_pixel_stride;
                            }
                            
                            src1_scanline += src1_scanline_stride;
                            dest_scanline += dest_scanline_stride;
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
                        Xil_unsigned8* src1_data;
                        src1_storage.getStorageInfo(band,
                                                    &src1_pixel_stride,
                                                    &src1_scanline_stride,
                                                    NULL,
                                                    (void**)&src1_data);
            
                        unsigned int   dest_pixel_stride;
                        unsigned int   dest_scanline_stride;
                        Xil_unsigned8* dest_data;
                        dest_storage.getStorageInfo(band,
                                                    &dest_pixel_stride,
                                                    &dest_scanline_stride,
                                                    NULL,
                                                    (void**)&dest_data);
                        
                        Xil_unsigned8* src1_scanline = src1_data +
                            (y*src1_scanline_stride) + (x*src1_pixel_stride);

                        Xil_unsigned8* dest_scanline = dest_data +
                            (y*dest_scanline_stride) + (x*dest_pixel_stride);

                        int ptrX = src_box_x + x;
                        int ptrY = src_box_y + y;
                        int kulX = ptrX - keyx;
                        int kulY = ptrY + height - keyy - 1;
                        //
                        // Clamp kernel lower_left
                        // Y should be clamping to image height
                        // X should clamp to 0
                        //
                        int imageHeight = src1Image->getHeight();
                        if (kulX < 0)
                            kulX = 0;
                        if (kulY > imageHeight - 1)
                            kulY = imageHeight - 1;
                        Xil_unsigned8* lower_left = src1_scanline +
                            ((kulX - ptrX) * src1_pixel_stride) +
                            ((kulY - ptrY) * src1_scanline_stride);

                        //
                        // Do the cases where the kernel extends above or meets the top
                        // of the image.
                        //
                        for (j = 0; j < ysize; j++) {
                            // point to the first pixel of the scanline 
                            Xil_unsigned8* dest_pixel = dest_scanline;
                            
                            for (i = 0; i < xsize; i++) {
                                Xil_unsigned8* dest = dest_pixel;
                                
                                int s0 = *(lower_left - src1_scanline_stride);
                                int s1 = *(lower_left - src1_scanline_stride);
                                int s2 = *(lower_left - src1_scanline_stride + src1_pixel_stride);
                                int s3 = *(lower_left);
                                int s5 = *(lower_left + src1_pixel_stride);
                                int s6 = *(lower_left);
                                int s7 = *(lower_left);
                                int s8 = *(lower_left + src1_pixel_stride);

                                int vsum = s0 + (s3 << 1) + s6 - s2 - (s5 << 1) -s8;
                                int hsum = s0 + (s1 << 1) + s2 - s6 - (s7 << 1) - s8;

                                float fsum = (float)sqrt(sq_lut[vsum] + sq_lut[hsum]);
                                
                                *dest = _XILI_ROUND_U8(fsum);
                                
                                dest_pixel += dest_pixel_stride;
                            }
                            
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
                        Xil_unsigned8* src1_data;
                        src1_storage.getStorageInfo(band,
                                                    &src1_pixel_stride,
                                                    &src1_scanline_stride,
                                                    NULL,
                                                    (void**)&src1_data);
            
                        unsigned int   dest_pixel_stride;
                        unsigned int   dest_scanline_stride;
                        Xil_unsigned8* dest_data;
                        dest_storage.getStorageInfo(band,
                                                    &dest_pixel_stride,
                                                    &dest_scanline_stride,
                                                    NULL,
                                                    (void**)&dest_data);
                        
                        Xil_unsigned8* src1_scanline = src1_data +
                            (y*src1_scanline_stride) + (x*src1_pixel_stride);

                        Xil_unsigned8* dest_scanline = dest_data +
                            (y*dest_scanline_stride) + (x*dest_pixel_stride);

                        //
                        // Do the cases where the kernel extends above or meets the top
                        // of the image.
                        //
                        for (j = 0; j < ysize; j++) {
                            // point to the first pixel of the scanline 
                            Xil_unsigned8* src1_pixel = src1_scanline;
                            Xil_unsigned8* dest_pixel = dest_scanline;
                            
                            for (i = 0; i < xsize; i++) {
                                Xil_unsigned8* src1 = src1_pixel;
                                Xil_unsigned8* dest = dest_pixel;
                                
                                int s0 = *(src1 - src1_scanline_stride - src1_pixel_stride);
                                int s1 = *(src1 - src1_scanline_stride);
                                int s2 = *(src1 - src1_scanline_stride + src1_pixel_stride);
                                int s3 = *(src1 - src1_pixel_stride);
                                int s5 = *(src1 + src1_pixel_stride);
                                int s6 = *(src1 - src1_pixel_stride);
                                int s7 = *(src1);
                                int s8 = *(src1 + src1_pixel_stride);

                                int vsum = s0 + (s3 << 1) + s6 - s2 - (s5 << 1) -s8;
                                int hsum = s0 + (s1 << 1) + s2 - s6 - (s7 << 1) - s8;

                                float fsum = (float)sqrt(sq_lut[vsum] + sq_lut[hsum]);
                                
                                *dest = _XILI_ROUND_U8(fsum);
                                
                                src1_pixel += src1_pixel_stride;
                                dest_pixel += dest_pixel_stride;
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
                        Xil_unsigned8* src1_data;
                        src1_storage.getStorageInfo(band,
                                                    &src1_pixel_stride,
                                                    &src1_scanline_stride,
                                                    NULL,
                                                    (void**)&src1_data);
            
                        unsigned int   dest_pixel_stride;
                        unsigned int   dest_scanline_stride;
                        Xil_unsigned8* dest_data;
                        dest_storage.getStorageInfo(band,
                                                    &dest_pixel_stride,
                                                    &dest_scanline_stride,
                                                    NULL,
                                                    (void**)&dest_data);
                        
                        Xil_unsigned8* src1_scanline = src1_data +
                            (y*src1_scanline_stride) + (x*src1_pixel_stride);

                        Xil_unsigned8* dest_scanline = dest_data +
                            (y*dest_scanline_stride) + (x*dest_pixel_stride);

                        int ptrX = src_box_x + x;
                        int ptrY = src_box_y + y;
                        int kulX = ptrX + width - keyx - 1;
                        int kulY = ptrY + height - keyy - 1;

                        //
                        // Clamp kernel lower right
                        // Y should be clamping to image height
                        // X should clamp to to image width
                        //
                        int imageWidth = src1Image->getWidth();
                        int imageHeight = src1Image->getHeight();
                        if (kulX > imageWidth - 1)
                            kulX = imageWidth - 1;
                        if (kulY > imageHeight - 1)
                            kulY = imageHeight - 1;
                        Xil_unsigned8* lower_right = src1_scanline +
                            ((kulX - ptrX) * src1_pixel_stride) +
                            ((kulY - ptrY) * src1_scanline_stride);
                        //
                        // Do the cases where the kernel extends above or meets the top
                        // of the image.
                        //
                        for (j = 0; j < ysize; j++) {
                            // point to the first pixel of the scanline 
                            Xil_unsigned8* dest_pixel = dest_scanline;
                            
                            for (i = 0; i < xsize; i++) {
                                Xil_unsigned8* dest = dest_pixel;
                                
                                int s0 = *(lower_right - src1_scanline_stride - src1_pixel_stride);
                                int s1 = *(lower_right - src1_scanline_stride);
                                int s2 = *(lower_right - src1_scanline_stride);
                                int s3 = *(lower_right - src1_pixel_stride);
                                int s5 = *lower_right;
                                int s6 = *(lower_right - src1_pixel_stride);
                                int s7 = *(lower_right);
                                int s8 = *(lower_right);

                                int vsum = s0 + (s3 << 1) + s6 - s2 - (s5 << 1) -s8;
                                int hsum = s0 + (s1 << 1) + s2 - s6 - (s7 << 1) - s8;
                                
                                float fsum = (float)sqrt(sq_lut[vsum] + sq_lut[hsum]);

                                *dest = _XILI_ROUND_U8(fsum);
                                
                                dest_pixel += dest_pixel_stride;
                            }
                            
                            dest_scanline += dest_scanline_stride;
                        }
                    }
                    break;
                }
            }
        } else {
            //
            //  Test to see if all of our storage is of type XIL_PIXEL_SEQUENTIAL.
            //  If so, implement an loop optimized fro pixel-sequential storage.
            //
            if((src1_storage.isType(XIL_PIXEL_SEQUENTIAL)) &&
               (dest_storage.isType(XIL_PIXEL_SEQUENTIAL))) {
                unsigned int   src1_pixel_stride;
                unsigned int   src1_scanline_stride;
                Xil_unsigned8* src1_data;
                src1_storage.getStorageInfo(&src1_pixel_stride,
                                            &src1_scanline_stride,
                                            NULL, NULL,
                                            (void**)&src1_data);
                unsigned int   dest_pixel_stride;
                unsigned int   dest_scanline_stride;
                Xil_unsigned8* dest_data;
                dest_storage.getStorageInfo(&dest_pixel_stride,
                                            &dest_scanline_stride,
                                            NULL, NULL,
                                            (void**)&dest_data);
                
                //
                //        Create a list of rectangles to loop over.  The resulting list
                //        of rectangles is the area left by intersecting the ROI with
                //        the destination box.
                //
                XilRectList           rl(roi, dest_box);

                int                   x;
                int                   y;
                unsigned int   xsize;
                unsigned int   ysize;
                while(rl.getNext(&x, &y, &xsize, &ysize)) {
                    Xil_unsigned8* src1_scanline = src1_data +
                        ((y - keyy)*src1_scanline_stride) +
                        ((x - keyx)*src1_pixel_stride);
                    
                    Xil_unsigned8* dest_scanline = dest_data +
                        (y*dest_scanline_stride) +
                        (x*dest_pixel_stride);

                    Xil_unsigned8* r1ptr = src1_scanline;
                    Xil_unsigned8* r2ptr = src1_scanline + src1_scanline_stride;
                    Xil_unsigned8* r3ptr = r2ptr + src1_scanline_stride;

                    unsigned int stopcolsave =
                        ((xsize - (xsize % 3)) + 1) * src1_pixel_stride;
                    unsigned int remainder = xsize % width;

                    for (j = 0; j < ysize; j++) {
                        for (i = 0; i < nbands; i++) {
                            Xil_unsigned8* dest  = dest_scanline + i;
                            
                            int s0 = *(r1ptr + i);
                            int s1 = *(r1ptr + i + src1_pixel_stride);
                            int s2 = *(r1ptr + i + src1_pixel_stride + src1_pixel_stride);
                            int s3 = *(r2ptr + i);
                            int s4 = *(r2ptr + i + src1_pixel_stride);
                            int s5 = *(r2ptr + i + src1_pixel_stride + src1_pixel_stride);
                            int s6 = *(r3ptr + i);
                            int s7 = *(r3ptr + i + src1_pixel_stride);
                            int s8 = *(r3ptr + i + src1_pixel_stride + src1_pixel_stride);

                            unsigned int col       = (src1_pixel_stride<<1) + i;
                            unsigned int stopcol   = stopcolsave + i;

                            while(col <= stopcol) {
                                //
                                //  Pass 1
                                //
                                int vsum = s0 + (s3 << 1) + s6 - s2 - (s5 << 1) - s8;
                                int hsum = s0 + (s1 << 1) + s2 - s6 - (s7 << 1) - s8;
                                
                                float fsum = (float)sqrt(sq_lut[vsum] + sq_lut[hsum]);

                                col += src1_pixel_stride;

                                *dest = _XILI_ROUND_U8(fsum);

                                s0 = *(r1ptr + col);
                                s3 = *(r2ptr + col);
                                s6 = *(r3ptr + col);

                                dest += dest_pixel_stride;

                                //
                                //  Pass 2
                                //
                                vsum = s1 + (s4 << 1) + s7 - s0 - (s3 << 1) - s6;
                                hsum = s1 + (s2 << 1) + s0 - s7 - (s8 << 1) - s6;

                                fsum = (float)sqrt(sq_lut[vsum] + sq_lut[hsum]);

                                col += src1_pixel_stride;

                                *dest = _XILI_ROUND_U8(fsum);

                                s1 = *(r1ptr + col);
                                s4 = *(r2ptr + col);
                                s7 = *(r3ptr + col);

                                dest += dest_pixel_stride;

                                //
                                //  Pass 3
                                //
                                vsum = s2 + (s5 << 1) + s8 - s1 - (s4 << 1) - s7;
                                hsum = s2 + (s0 << 1) + s1 - s8 - (s6 << 1) - s7;

                                fsum = (float)sqrt(sq_lut[vsum] + sq_lut[hsum]);

                                col += src1_pixel_stride;

                                *dest = _XILI_ROUND_U8(fsum);

                                s2 = *(r1ptr + col);
                                s5 = *(r2ptr + col);
                                s8 = *(r3ptr + col);

                                dest += dest_pixel_stride;
                            }

                            switch(remainder) {
                              case 1:
                              {
                                //
                                //  Pass 1
                                //
                                int vsum = s0 + (s3 << 1) + s6 - s2 - (s5 << 1) - s8;
                                int hsum = s0 + (s1 << 1) + s2 - s6 - (s7 << 1) - s8;
                                
                                float fsum = (float)sqrt(sq_lut[vsum] + sq_lut[hsum]);

                                col += src1_pixel_stride;

                                *dest = _XILI_ROUND_U8(fsum);

                                s0 = *(r1ptr + col);
                                s3 = *(r2ptr + col);
                                s6 = *(r3ptr + col);

                                dest += dest_pixel_stride;
                              }
                              break;

                              case 2:
                              {
                                //
                                //  Pass 1
                                //
                                int vsum = s0 + (s3 << 1) + s6 - s2 - (s5 << 1) - s8;
                                int hsum = s0 + (s1 << 1) + s2 - s6 - (s7 << 1) - s8;
                                
                                float fsum = (float)sqrt(sq_lut[vsum] + sq_lut[hsum]);

                                col += src1_pixel_stride;

                                *dest = _XILI_ROUND_U8(fsum);

                                s0 = *(r1ptr + col);
                                s3 = *(r2ptr + col);
                                s6 = *(r3ptr + col);

                                dest += dest_pixel_stride;

                                //
                                //  Pass 2
                                //
                                vsum = s1 + (s4 << 1) + s7 - s0 - (s3 << 1) - s6;
                                hsum = s1 + (s2 << 1) + s0 - s7 - (s8 << 1) - s6;

                                fsum = (float)sqrt(sq_lut[vsum] + sq_lut[hsum]);

                                col += src1_pixel_stride;

                                *dest = _XILI_ROUND_U8(fsum);

                                s1 = *(r1ptr + col);
                                s4 = *(r2ptr + col);
                                s7 = *(r3ptr + col);

                                dest += dest_pixel_stride;
                              }
                              break;
                            }
                        }

                        r1ptr += src1_scanline_stride;
                        r2ptr += src1_scanline_stride;
                        r3ptr += src1_scanline_stride;

                        dest_scanline += dest_scanline_stride;
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
                        Xil_unsigned8* src1_data;
                        src1_storage.getStorageInfo(band,
                                                    &src1_pixel_stride,
                                                    &src1_scanline_stride,
                                                    NULL,
                                                    (void**)&src1_data);
                        
                        unsigned int   dest_pixel_stride;
                        unsigned int   dest_scanline_stride;
                        Xil_unsigned8* dest_data;
                        dest_storage.getStorageInfo(band,
                                                    &dest_pixel_stride,
                                                    &dest_scanline_stride,
                                                    NULL,
                                                    (void**)&dest_data);
                        
                        Xil_unsigned8* src1_scanline = src1_data +
                            ((y-keyy)*src1_scanline_stride) + ((x-keyx)*src1_pixel_stride);
                        
                        Xil_unsigned8* dest_scanline = dest_data +
                            (y*dest_scanline_stride) + (x*dest_pixel_stride);
                        
                        Xil_unsigned8* r1ptr = src1_scanline;
                        Xil_unsigned8* r2ptr = src1_scanline + src1_scanline_stride;
                        Xil_unsigned8* r3ptr = r2ptr + src1_scanline_stride;
                        
                        unsigned long stopcolsave = (xsize + 1) * src1_pixel_stride;
                        
                        for (j = 0; j < ysize; j++) {
                            Xil_unsigned8* dest  = dest_scanline;
                                
                            int s0 = *(r1ptr);
                            int s1 = *(r1ptr + src1_pixel_stride);
                            int s2 = *(r1ptr + src1_pixel_stride + src1_pixel_stride);
                            int s3 = *(r2ptr);
                            int s4 = *(r2ptr + src1_pixel_stride);
                            int s5 = *(r2ptr + src1_pixel_stride + src1_pixel_stride);
                            int s6 = *(r3ptr);
                            int s7 = *(r3ptr + src1_pixel_stride);
                            int s8 = *(r3ptr + src1_pixel_stride + src1_pixel_stride);
                            
                            unsigned long col = src1_pixel_stride * 2;
                            unsigned long stopcol = stopcolsave;
                                
                            while (col <= stopcol) {
                                
                                int vsum = s0 + (s3 << 1) + s6 - s2 - (s5 << 1) - s8;
                                int hsum = s0 + (s1 << 1) + s2 - s6 - (s7 << 1) - s8;
                                
                                float fsum = (float)sqrt(sq_lut[vsum] + sq_lut[hsum]);
                                
                                *dest = _XILI_ROUND_U8(fsum);
                                
                                col += src1_pixel_stride;
                                
                                if (col > stopcol)
                                    break;
                                
                                s0 = *(r1ptr + col);
                                s3 = *(r2ptr + col);
                                s6 = *(r3ptr + col);
                                
                                dest += dest_pixel_stride;
                                
                                vsum = s1 + (s4 << 1) + s7 - s0 - (s3 << 1) - s6;
                                hsum = s1 + (s2 << 1) + s0 - s7 - (s8 << 1) - s6;
                                
                                fsum = (float)sqrt(sq_lut[vsum] + sq_lut[hsum]);
                                
                                *dest = _XILI_ROUND_U8(fsum);
                                
                                col += src1_pixel_stride;
                                
                                if (col > stopcol)
                                    break;
                                
                                s1 = *(r1ptr + col);
                                s4 = *(r2ptr + col);
                                s7 = *(r3ptr + col);
                                
                                dest += dest_pixel_stride;
                                
                                vsum = s2 + (s5 << 1) + s8 - s1 - (s4 << 1) - s7;
                                hsum = s2 + (s0 << 1) + s1 - s8 - (s6 << 1) - s7;
                                
                                fsum = (float)sqrt(sq_lut[vsum] + sq_lut[hsum]);
                                
                                *dest = _XILI_ROUND_U8(fsum);
                                
                                col += src1_pixel_stride;
                                
                                if (col > stopcol)
                                    break;
                                
                                s2 = *(r1ptr + col);
                                s5 = *(r2ptr + col);
                                s8 = *(r3ptr + col);
                                
                                dest += dest_pixel_stride;
                            }
                            
                            r1ptr += src1_scanline_stride;
                            r2ptr += src1_scanline_stride;
                            r3ptr += src1_scanline_stride;
                            
                            dest_scanline += dest_scanline_stride;
                        }
                    }
                }
            }
        }
    }

    return XIL_SUCCESS;
}

