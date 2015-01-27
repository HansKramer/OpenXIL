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
//  Revision:	1.9
//  Last Mod:	10:09:52, 03/10/00
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
#pragma ident	"@(#)EdgeDetection.cc	1.9\t00/03/10  "

#include "XilDeviceManagerComputeBIT.hh"
#include "XiliUtils.hh"

XilStatus
XilDeviceManagerComputeBIT::EdgeDetection(XilOp*       op,
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

    XilEdgeCondition    edge_condition;
    op->getParam(1, (int*)&edge_condition);

    Xil_unsigned8** tbl = getEdgeDetectTable();
    if(tbl == NULL) {
        return XIL_FAILURE;
    }

    //
    //  Store away the number of bands for this operation.
    //    
    unsigned int nbands   = destImage->getNumBands();

    int i,j;

    //
    //  Loop over each of the boxes of storage we are to process.
    //
    XilBox* src1_box;
    XilBox* dest_box;
    while(bl->getNext(&src1_box, &dest_box)) {
        XilBoxAreaType tag = (XilBoxAreaType) ((long)dest_box->getTag());
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
                switch (tag) {
                  case XIL_AREA_TOP_LEFT_CORNER:
                    //
                    //  Each Band...
                    //
                    for(band=0; band<nbands; band++) {
                        unsigned int   src1_pixel_stride;
                        unsigned int   src1_scanline_stride;
                        unsigned int   src1_offset;
                        Xil_unsigned8* src1_data;
                        src1_storage.getStorageInfo(band,
                                                    &src1_pixel_stride,
                                                    &src1_scanline_stride,
                                                    &src1_offset,
                                                    (void**)&src1_data);

                        unsigned int   dest_pixel_stride;
                        unsigned int   dest_scanline_stride;
                        unsigned int   dest_offset;
                        Xil_unsigned8* dest_data;
                        dest_storage.getStorageInfo(band,
                                                    &dest_pixel_stride,
                                                    &dest_scanline_stride,
                                                    &dest_offset,
                                                    (void**)&dest_data);
                        
                        //
                        //  Correctly position us in the x and y direction by
                        //  selecting the right scanline and taking the
                        //  src1_offset into account. 
                        //
                        Xil_unsigned8* src1 = src1_data + (y * src1_scanline_stride);
                        src1_offset         += x;

                        Xil_unsigned8* dest = dest_data + (y * dest_scanline_stride);
                        dest_offset         += x;

                        //
                        //  Each Scanline...
                        //
                        for (j = 0; j < ysize; j++) {
                            Xil_unsigned8* src_scan0 = src1;
                            Xil_unsigned8* src_scan1 = src1;
                            Xil_unsigned8* src_scan2 = src1 + src1_scanline_stride;

                            //
                            //  Each Pixel...
                            //
                            for (i = 0; i < xsize; i++) {
                                int bit_offset = src1_offset + i;

                                int s0 = (XIL_BMAP_TST(src_scan0, bit_offset));
                                int s1 = (XIL_BMAP_TST(src_scan0, bit_offset)) << 1;
                                int s2 = (XIL_BMAP_TST(src_scan0, bit_offset + 1));
                                int s3 = (XIL_BMAP_TST(src_scan1, bit_offset)) << 1;
                                int s4 = (XIL_BMAP_TST(src_scan1, bit_offset)) << 1;
                                int s5 = (XIL_BMAP_TST(src_scan1, bit_offset + 1)) << 1;
                                int s6 = (XIL_BMAP_TST(src_scan2, bit_offset));
                                int s7 = (XIL_BMAP_TST(src_scan2, bit_offset)) << 1;
                                int s8 = (XIL_BMAP_TST(src_scan2, bit_offset + 1));
                                
                                //
                                //  vsum and hsum can range from -4 to 4.
                                //
                                int vsum = (s0 + s3 + s6 - s2 - s5 - s8);
                                int hsum = (s0 + s1 + s2 - s6 - s7 - s8);

                                if(tbl[hsum][vsum]) {
                                    XIL_BMAP_SET(dest, dest_offset + i);
                                } else {
                                    XIL_BMAP_CLR(dest, dest_offset + i);
                                }
                            }
                        
                            src1 += src1_scanline_stride;
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
                        unsigned int   src1_offset;
                        Xil_unsigned8* src1_data;
                        src1_storage.getStorageInfo(band,
                                                    &src1_pixel_stride,
                                                    &src1_scanline_stride,
                                                    &src1_offset,
                                                    (void**)&src1_data);

                        unsigned int   dest_pixel_stride;
                        unsigned int   dest_scanline_stride;
                        unsigned int   dest_offset;
                        Xil_unsigned8* dest_data;
                        dest_storage.getStorageInfo(band,
                                                    &dest_pixel_stride,
                                                    &dest_scanline_stride,
                                                    &dest_offset,
                                                    (void**)&dest_data);
                        
                        //
                        //  Correctly position us in the x and y direction by
                        //  selecting the right scanline and taking the
                        //  src1_offset into account. 
                        //
                        Xil_unsigned8* src1 = src1_data + (y * src1_scanline_stride);
                        src1_offset         += x;

                        Xil_unsigned8* dest = dest_data + (y * dest_scanline_stride);
                        dest_offset         += x;

                        if(src1_offset == 0) {
                            src1--;
                            src1_offset = 8;
                        }

                        //
                        //  Each Scanline...
                        //
                        for (j = 0; j < ysize; j++) {
                            Xil_unsigned8* src_scan0 = src1;
                            Xil_unsigned8* src_scan1 = src1;
                            Xil_unsigned8* src_scan2 = src1 + src1_scanline_stride;

                            //
                            //  Each Pixel...
                            //
                            for (i = 0; i < xsize; i++) {
                                int bit_offset = src1_offset + i;

                                int s0 = (XIL_BMAP_TST(src_scan0, bit_offset - 1));
                                int s1 = (XIL_BMAP_TST(src_scan0, bit_offset)) << 1;
                                int s2 = (XIL_BMAP_TST(src_scan0, bit_offset + 1));
                                int s3 = (XIL_BMAP_TST(src_scan1, bit_offset - 1)) << 1;
                                int s4 = (XIL_BMAP_TST(src_scan1, bit_offset)) << 1;
                                int s5 = (XIL_BMAP_TST(src_scan1, bit_offset + 1)) << 1;
                                int s6 = (XIL_BMAP_TST(src_scan2, bit_offset - 1));
                                int s7 = (XIL_BMAP_TST(src_scan2, bit_offset)) << 1;
                                int s8 = (XIL_BMAP_TST(src_scan2, bit_offset + 1));

                                //
                                //  vsum and hsum can range from -4 to 4.
                                //
                                int vsum = (s0 + s3 + s6 - s2 - s5 - s8);
                                int hsum = (s0 + s1 + s2 - s6 - s7 - s8);

                                if(tbl[hsum][vsum]) {
                                    XIL_BMAP_SET(dest, dest_offset + i);
                                } else {
                                    XIL_BMAP_CLR(dest, dest_offset + i);
                                }
                            }
                        
                            src1 += src1_scanline_stride;
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
                        unsigned int   src1_offset;
                        Xil_unsigned8* src1_data;
                        src1_storage.getStorageInfo(band,
                                                    &src1_pixel_stride,
                                                    &src1_scanline_stride,
                                                    &src1_offset,
                                                    (void**)&src1_data);

                        unsigned int   dest_pixel_stride;
                        unsigned int   dest_scanline_stride;
                        unsigned int   dest_offset;
                        Xil_unsigned8* dest_data;
                        dest_storage.getStorageInfo(band,
                                                    &dest_pixel_stride,
                                                    &dest_scanline_stride,
                                                    &dest_offset,
                                                    (void**)&dest_data);
                        
                        //
                        //  Correctly position us in the x and y direction by
                        //  selecting the right scanline and taking the
                        //  src1_offset into account. 
                        //
                        Xil_unsigned8* src1 = src1_data + (y * src1_scanline_stride);
                        src1_offset         += x;

                        Xil_unsigned8* dest = dest_data + (y * dest_scanline_stride);
                        dest_offset         += x;

                        if(src1_offset == 0) {
                            src1--;
                            src1_offset = 8;
                        }

                        //
                        //  Each Scanline...
                        //
                        for (j = 0; j < ysize; j++) {
                            Xil_unsigned8* src_scan0 = src1;
                            Xil_unsigned8* src_scan1 = src1;
                            Xil_unsigned8* src_scan2 = src1 + src1_scanline_stride;

                            //
                            //  Each Pixel...
                            //
                            for (i = 0; i < xsize; i++) {
                                int bit_offset = src1_offset + i;
                                
                                int s0 = (XIL_BMAP_TST(src_scan0, bit_offset - 1));
                                int s1 = (XIL_BMAP_TST(src_scan0, bit_offset)) << 1;
                                int s2 = (XIL_BMAP_TST(src_scan0, bit_offset));
                                int s3 = (XIL_BMAP_TST(src_scan1, bit_offset - 1)) << 1;
                                int s4 = (XIL_BMAP_TST(src_scan1, bit_offset)) << 1;
                                int s5 = (XIL_BMAP_TST(src_scan1, bit_offset)) << 1;
                                int s6 = (XIL_BMAP_TST(src_scan2, bit_offset - 1));
                                int s7 = (XIL_BMAP_TST(src_scan2, bit_offset)) << 1;
                                int s8 = (XIL_BMAP_TST(src_scan2, bit_offset));
                                
                                //
                                //  vsum and hsum can range from -4 to 4.
                                //
                                int vsum = (s0 + s3 + s6 - s2 - s5 - s8);
                                int hsum = (s0 + s1 + s2 - s6 - s7 - s8);

                                if(tbl[hsum][vsum]) {
                                    XIL_BMAP_SET(dest, dest_offset + i);
                                } else {
                                    XIL_BMAP_CLR(dest, dest_offset + i);
                                }
                            }
                        
                            src1 += src1_scanline_stride;
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
                        unsigned int   src1_offset;
                        Xil_unsigned8* src1_data;
                        src1_storage.getStorageInfo(band,
                                                    &src1_pixel_stride,
                                                    &src1_scanline_stride,
                                                    &src1_offset,
                                                    (void**)&src1_data);

                        unsigned int   dest_pixel_stride;
                        unsigned int   dest_scanline_stride;
                        unsigned int   dest_offset;
                        Xil_unsigned8* dest_data;
                        dest_storage.getStorageInfo(band,
                                                    &dest_pixel_stride,
                                                    &dest_scanline_stride,
                                                    &dest_offset,
                                                    (void**)&dest_data);

                        //
                        //  Correctly position us in the x and y direction by
                        //  selecting the right scanline and taking the
                        //  src1_offset into account. 
                        //
                        Xil_unsigned8* src1 = src1_data + (y * src1_scanline_stride);
                        src1_offset         += x;

                        Xil_unsigned8* dest = dest_data + (y * dest_scanline_stride);
                        dest_offset         += x;

                        //
                        //  Each Scanline...
                        //
                        for (j = 0; j < ysize; j++) {
                            Xil_unsigned8* src_scan0 = src1 - src1_scanline_stride;
                            Xil_unsigned8* src_scan1 = src1;
                            Xil_unsigned8* src_scan2 = src1 + src1_scanline_stride;

                            //
                            //  Each Pixel...
                            //
                            for (i = 0; i < xsize; i++) {
                                int bit_offset = src1_offset + i;
                                
                                int s0 = (XIL_BMAP_TST(src_scan0, bit_offset));
                                int s1 = (XIL_BMAP_TST(src_scan0, bit_offset)) << 1;
                                int s2 = (XIL_BMAP_TST(src_scan0, bit_offset + 1));
                                int s3 = (XIL_BMAP_TST(src_scan1, bit_offset)) << 1;
                                int s4 = (XIL_BMAP_TST(src_scan1, bit_offset)) << 1;
                                int s5 = (XIL_BMAP_TST(src_scan1, bit_offset + 1)) << 1;
                                int s6 = (XIL_BMAP_TST(src_scan2, bit_offset));
                                int s7 = (XIL_BMAP_TST(src_scan2, bit_offset)) << 1;
                                int s8 = (XIL_BMAP_TST(src_scan2, bit_offset + 1));
                                
                                //
                                //  vsum and hsum can range from -4 to 4.
                                //
                                int vsum = (s0 + s3 + s6 - s2 - s5 - s8);
                                int hsum = (s0 + s1 + s2 - s6 - s7 - s8);

                                if(tbl[hsum][vsum]) {
                                    XIL_BMAP_SET(dest, dest_offset + i);
                                } else {
                                    XIL_BMAP_CLR(dest, dest_offset + i);
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
                        unsigned int   src1_offset;
                        Xil_unsigned8* src1_data;
                        src1_storage.getStorageInfo(band,
                                                    &src1_pixel_stride,
                                                    &src1_scanline_stride,
                                                    &src1_offset,
                                                    (void**)&src1_data);

                        unsigned int   dest_pixel_stride;
                        unsigned int   dest_scanline_stride;
                        unsigned int   dest_offset;
                        Xil_unsigned8* dest_data;
                        dest_storage.getStorageInfo(band,
                                                    &dest_pixel_stride,
                                                    &dest_scanline_stride,
                                                    &dest_offset,
                                                    (void**)&dest_data);
                        
                        //
                        //  Correctly position us in the x and y direction by
                        //  selecting the right scanline and taking the
                        //  src1_offset into account. 
                        //
                        Xil_unsigned8* src1 = src1_data + (y * src1_scanline_stride);
                        src1_offset         += x;

                        Xil_unsigned8* dest = dest_data + (y * dest_scanline_stride);
                        dest_offset         += x;

                        if(src1_offset == 0) {
                            src1--;
                            src1_offset = 8;
                        }

                        //
                        //  Each Scanline...
                        //
                        for (j = 0; j < ysize; j++) {
                            Xil_unsigned8* src_scan0 = src1 - src1_scanline_stride;
                            Xil_unsigned8* src_scan1 = src1;
                            Xil_unsigned8* src_scan2 = src1 + src1_scanline_stride;

                            //
                            //  Each Pixel...
                            //
                            for (i = 0; i < xsize; i++) {
                                int bit_offset = src1_offset + i;
                                
                                int s0 = (XIL_BMAP_TST(src_scan0, bit_offset - 1));
                                int s1 = (XIL_BMAP_TST(src_scan0, bit_offset)) << 1;
                                int s2 = (XIL_BMAP_TST(src_scan0, bit_offset));
                                int s3 = (XIL_BMAP_TST(src_scan1, bit_offset - 1)) << 1;
                                int s4 = (XIL_BMAP_TST(src_scan1, bit_offset)) << 1;
                                int s5 = (XIL_BMAP_TST(src_scan1, bit_offset)) << 1;
                                int s6 = (XIL_BMAP_TST(src_scan2, bit_offset - 1));
                                int s7 = (XIL_BMAP_TST(src_scan2, bit_offset)) << 1;
                                int s8 = (XIL_BMAP_TST(src_scan2, bit_offset));
                                
                                //
                                //  vsum and hsum can range from -4 to 4.
                                //
                                int vsum = (s0 + s3 + s6 - s2 - s5 - s8);
                                int hsum = (s0 + s1 + s2 - s6 - s7 - s8);

                                if(tbl[hsum][vsum]) {
                                    XIL_BMAP_SET(dest, dest_offset + i);
                                } else {
                                    XIL_BMAP_CLR(dest, dest_offset + i);
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
                        unsigned int   src1_offset;
                        Xil_unsigned8* src1_data;
                        src1_storage.getStorageInfo(band,
                                                    &src1_pixel_stride,
                                                    &src1_scanline_stride,
                                                    &src1_offset,
                                                    (void**)&src1_data);

                        unsigned int   dest_pixel_stride;
                        unsigned int   dest_scanline_stride;
                        unsigned int   dest_offset;
                        Xil_unsigned8* dest_data;
                        dest_storage.getStorageInfo(band,
                                                    &dest_pixel_stride,
                                                    &dest_scanline_stride,
                                                    &dest_offset,
                                                    (void**)&dest_data);
                        
                        //
                        //  Correctly position us in the x and y direction by
                        //  selecting the right scanline and taking the
                        //  src1_offset into account. 
                        //
                        Xil_unsigned8* src1 = src1_data + (y * src1_scanline_stride);
                        src1_offset         += x;

                        Xil_unsigned8* dest = dest_data + (y * dest_scanline_stride);
                        dest_offset         += x;

                        //
                        //  Each Scanline...
                        //
                        for (j = 0; j < ysize; j++) {
                            Xil_unsigned8* src_scan0 = src1 - src1_scanline_stride;
                            Xil_unsigned8* src_scan1 = src1;
                            Xil_unsigned8* src_scan2 = src1;

                            //
                            //  Each Pixel...
                            //
                            for (i = 0; i < xsize; i++) {
                                int bit_offset = src1_offset + i;
                                
                                int s0 = (XIL_BMAP_TST(src_scan0, bit_offset));
                                int s1 = (XIL_BMAP_TST(src_scan0, bit_offset)) << 1;
                                int s2 = (XIL_BMAP_TST(src_scan0, bit_offset + 1));
                                int s3 = (XIL_BMAP_TST(src_scan1, bit_offset)) << 1;
                                int s4 = (XIL_BMAP_TST(src_scan1, bit_offset)) << 1;
                                int s5 = (XIL_BMAP_TST(src_scan1, bit_offset + 1)) << 1;
                                int s6 = (XIL_BMAP_TST(src_scan2, bit_offset));
                                int s7 = (XIL_BMAP_TST(src_scan2, bit_offset)) << 1;
                                int s8 = (XIL_BMAP_TST(src_scan2, bit_offset + 1));
                                
                                //
                                //  vsum and hsum can range from -4 to 4.
                                //
                                int vsum = (s0 + s3 + s6 - s2 - s5 - s8);
                                int hsum = (s0 + s1 + s2 - s6 - s7 - s8);

                                if(tbl[hsum][vsum]) {
                                    XIL_BMAP_SET(dest, dest_offset + i);
                                } else {
                                    XIL_BMAP_CLR(dest, dest_offset + i);
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
                        unsigned int   src1_offset;
                        Xil_unsigned8* src1_data;
                        src1_storage.getStorageInfo(band,
                                                    &src1_pixel_stride,
                                                    &src1_scanline_stride,
                                                    &src1_offset,
                                                    (void**)&src1_data);

                        unsigned int   dest_pixel_stride;
                        unsigned int   dest_scanline_stride;
                        unsigned int   dest_offset;
                        Xil_unsigned8* dest_data;
                        dest_storage.getStorageInfo(band,
                                                    &dest_pixel_stride,
                                                    &dest_scanline_stride,
                                                    &dest_offset,
                                                    (void**)&dest_data);
                        
                        //
                        //  Correctly position us in the x and y direction by
                        //  selecting the right scanline and taking the
                        //  src1_offset into account. 
                        //
                        Xil_unsigned8* src1 = src1_data + (y * src1_scanline_stride);
                        src1_offset         += x;

                        Xil_unsigned8* dest = dest_data + (y * dest_scanline_stride);
                        dest_offset         += x;

                        if(src1_offset == 0) {
                            src1--;
                            src1_offset = 8;
                        }

                        //
                        //  Each Scanline...
                        //
                        for (j = 0; j < ysize; j++) {
                            Xil_unsigned8* src_scan0 = src1 - src1_scanline_stride;
                            Xil_unsigned8* src_scan1 = src1;
                            Xil_unsigned8* src_scan2 = src1;

                            //
                            //  Each Pixel...
                            //
                            for (i = 0; i < xsize; i++) {
                                int bit_offset = src1_offset + i;
                                
                                int s0 = (XIL_BMAP_TST(src_scan0, bit_offset - 1));
                                int s1 = (XIL_BMAP_TST(src_scan0, bit_offset)) << 1;
                                int s2 = (XIL_BMAP_TST(src_scan0, bit_offset + 1));
                                int s3 = (XIL_BMAP_TST(src_scan1, bit_offset - 1)) << 1;
                                int s4 = (XIL_BMAP_TST(src_scan1, bit_offset)) << 1;
                                int s5 = (XIL_BMAP_TST(src_scan1, bit_offset + 1)) << 1;
                                int s6 = (XIL_BMAP_TST(src_scan2, bit_offset - 1));
                                int s7 = (XIL_BMAP_TST(src_scan2, bit_offset)) << 1;
                                int s8 = (XIL_BMAP_TST(src_scan2, bit_offset + 1));
                                
                                //
                                //  vsum and hsum can range from -4 to 4.
                                //
                                int vsum = (s0 + s3 + s6 - s2 - s5 - s8);
                                int hsum = (s0 + s1 + s2 - s6 - s7 - s8);

                                if(tbl[hsum][vsum]) {
                                    XIL_BMAP_SET(dest, dest_offset + i);
                                } else {
                                    XIL_BMAP_CLR(dest, dest_offset + i);
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
                        unsigned int   src1_offset;
                        Xil_unsigned8* src1_data;
                        src1_storage.getStorageInfo(band,
                                                    &src1_pixel_stride,
                                                    &src1_scanline_stride,
                                                    &src1_offset,
                                                    (void**)&src1_data);
                        
                        unsigned int   dest_pixel_stride;
                        unsigned int   dest_scanline_stride;
                        unsigned int   dest_offset;
                        Xil_unsigned8* dest_data;
                        dest_storage.getStorageInfo(band,
                                                    &dest_pixel_stride,
                                                    &dest_scanline_stride,
                                                    &dest_offset,
                                                    (void**)&dest_data);
                        //
                        //  Correctly position us in the x and y direction by
                        //  selecting the right scanline and taking the
                        //  src1_offset into account. 
                        //
                        Xil_unsigned8* src1 = src1_data + (y * src1_scanline_stride);
                        src1_offset         += x;

                        Xil_unsigned8* dest = dest_data + (y * dest_scanline_stride);
                        dest_offset         += x;

                        if(src1_offset == 0) {
                            src1--;
                            src1_offset = 8;
                        }

                        //
                        //  Each Scanline...
                        //
                        for (j = 0; j < ysize; j++) {
                            Xil_unsigned8* src_scan0 = src1 - src1_scanline_stride;
                            Xil_unsigned8* src_scan1 = src1;
                            Xil_unsigned8* src_scan2 = src1;

                            //
                            //  Each Pixel...
                            //
                            for (i = 0; i < xsize; i++) {
                                int bit_offset = src1_offset + i;
                                
                                int s0 = (XIL_BMAP_TST(src_scan0, bit_offset - 1));
                                int s1 = (XIL_BMAP_TST(src_scan0, bit_offset)) << 1;
                                int s2 = (XIL_BMAP_TST(src_scan0, bit_offset));
                                int s3 = (XIL_BMAP_TST(src_scan1, bit_offset - 1)) << 1;
                                int s4 = (XIL_BMAP_TST(src_scan1, bit_offset)) << 1;
                                int s5 = (XIL_BMAP_TST(src_scan1, bit_offset)) << 1;
                                int s6 = (XIL_BMAP_TST(src_scan2, bit_offset - 1));
                                int s7 = (XIL_BMAP_TST(src_scan2, bit_offset)) << 1;
                                int s8 = (XIL_BMAP_TST(src_scan2, bit_offset));
                                
                                //
                                //  vsum and hsum can range from -4 to 4.
                                //
                                int vsum = (s0 + s3 + s6 - s2 - s5 - s8);
                                int hsum = (s0 + s1 + s2 - s6 - s7 - s8);

                                if(tbl[hsum][vsum]) {
                                    XIL_BMAP_SET(dest, dest_offset + i);
                                } else {
                                    XIL_BMAP_CLR(dest, dest_offset + i);
                                }
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
                    unsigned int   src1_offset;
                    Xil_unsigned8* src1_data;
                    src1_storage.getStorageInfo(band,
                                                &src1_pixel_stride,
                                                &src1_scanline_stride,
                                                &src1_offset,
                                                (void**)&src1_data);
                    
                    unsigned int   dest_pixel_stride;
                    unsigned int   dest_scanline_stride;
                    unsigned int   dest_offset;
                    Xil_unsigned8* dest_data;
                    dest_storage.getStorageInfo(band,
                                                &dest_pixel_stride,
                                                &dest_scanline_stride,
                                                &dest_offset,
                                                (void**)&dest_data);
                    
                    //
                    //  Correctly position us in the x and y direction by
                    //  selecting the right scanline and taking the
                    //  src1_offset into account. 
                    //
                    Xil_unsigned8* src1 = src1_data + (y * src1_scanline_stride);
                    src1_offset         += x;

                    Xil_unsigned8* dest = dest_data + (y * dest_scanline_stride);
                    dest_offset         += x;

                    if(src1_offset == 0) {
                        src1--;
                        src1_offset = 8;
                    }

                    //
                    //  Each Scanline...
                    //
                    for(j=ysize; j!=0; j--) {
                        Xil_unsigned8* src_scan0 = src1 - src1_scanline_stride;
                        Xil_unsigned8* src_scan1 = src1;
                        Xil_unsigned8* src_scan2 = src1 + src1_scanline_stride;

                        //
                        //  Each Pixel...
                        //
                        for(i = 0; i < xsize; i++) {
                            int bit_offset = src1_offset + i;

                            int s0 = (XIL_BMAP_TST(src_scan0, bit_offset - 1));
                            int s1 = (XIL_BMAP_TST(src_scan0, bit_offset)) << 1;
                            int s2 = (XIL_BMAP_TST(src_scan0, bit_offset + 1));
                            int s3 = (XIL_BMAP_TST(src_scan1, bit_offset - 1)) << 1;
                            int s4 = (XIL_BMAP_TST(src_scan1, bit_offset)) << 1;
                            int s5 = (XIL_BMAP_TST(src_scan1, bit_offset + 1)) << 1;
                            int s6 = (XIL_BMAP_TST(src_scan2, bit_offset - 1));
                            int s7 = (XIL_BMAP_TST(src_scan2, bit_offset)) << 1;
                            int s8 = (XIL_BMAP_TST(src_scan2, bit_offset + 1));

                            //
                            //  vsum and hsum can range from -4 to 4.
                            //
                            int vsum = (s0 + s3 + s6 - s2 - s5 - s8);
                            int hsum = (s0 + s1 + s2 - s6 - s7 - s8);

                            if(tbl[hsum][vsum]) {
                                XIL_BMAP_SET(dest, dest_offset + i);
                            } else {
                                XIL_BMAP_CLR(dest, dest_offset + i);
                            }
                        }
                        
                        src1 += src1_scanline_stride;
                        dest += dest_scanline_stride;
                    }
                }
            }
        }
    }

    return XIL_SUCCESS;
}

