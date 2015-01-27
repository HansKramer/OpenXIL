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
//  File:	Erode.cc
//  Project:	XIL
//  Revision:	1.10
//  Last Mod:	10:09:28, 03/10/00
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
#pragma ident	"@(#)Erode.cc	1.10\t00/03/10  "

#include "XiliUtils.hh"
#include "XilDeviceManagerComputeBIT.hh"

XilStatus
XilDeviceManagerComputeBIT::Erode(XilOp*       op,
                                  unsigned int    ,
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
    XilImage* dest = op->getDstImage(1);

    //
    // Get parameters for Erode
    //

    XilSel*    sel;
    op->getParam(1, (void**)&sel);

    //
    // Get Kernel information
    //
    unsigned int width  = sel->getWidth();
    unsigned int height = sel->getHeight();
    unsigned int keyx = sel->getKeyX();
    unsigned int keyy = sel->getKeyY();
    const unsigned int *data = sel->getData();

    //
    //  Store away the number of bands for this operation.
    //
    unsigned int nbands   = dest->getNumBands();

    // Local allocations
    int *xdeltas = new int [width*height];
    if (xdeltas == NULL) {
      XIL_ERROR(dest->getSystemState(), XIL_ERROR_RESOURCE, "di-1", TRUE);
      return XIL_FAILURE;
    }

    int *ydeltas = new int [width*height];
    if (ydeltas == NULL) {
      delete [] xdeltas;
      XIL_ERROR(dest->getSystemState(), XIL_ERROR_RESOURCE, "di-1", TRUE);
      return XIL_FAILURE;
    }

    //
    //  Loop over each of the boxes of storage we are to process.
    //
    XilBox* src_box;
    XilBox* dest_box;
    while(bl->getNext(&src_box, &dest_box)) {
        //
        //  Aquire our storage from the images.  The storage returned is valid
        //  for the box given.  Thus, any origins or child offsets have been
        //  taken into account.
        //
        XilStorage  src_storage(src);
        XilStorage  dest_storage(dest);
        if((src->getStorage(&src_storage, op, src_box, "XilMemory",
                             XIL_READ_ONLY)  == XIL_FAILURE) ||
           (dest->getStorage(&dest_storage, op, dest_box, "XilMemory",
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

        // Get type of Box
        XilBoxAreaType dest_box_type;
        dest_box_type = (XilBoxAreaType) ((int)dest_box->getTag());

        if(dest_box_type == XIL_AREA_SMALL_SOURCE) {
            //
            //  We do not currently support erode
            //  for sources smaller than the kernel.
            //  Don't mark the box as failed since there's no
            //  other implementation to fall to.
            //
            XIL_ERROR(dest->getSystemState(), XIL_ERROR_INTERNAL,
                      "di-447", TRUE);
            continue;
        }

        // Get kernel data
        unsigned int sel_size = 0;
        const unsigned int *tmp_data = data;
        for (int i = 0; i < height; i++) {
          for (int j = 0; j < width; j++) {
            if (*tmp_data++) {
              ydeltas[sel_size] = i - keyy;
              xdeltas[sel_size] = j - keyx;
              sel_size++;
            }
          }
        }

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

             unsigned int   src_scanline_stride;
             unsigned int   src_storage_offset;
             Xil_unsigned8* src_data;
             src_storage.getStorageInfo(band,
                                        NULL,
                                        &src_scanline_stride,
                                        &src_storage_offset,
                                        (void**)&src_data);

             unsigned int   dest_scanline_stride;
             unsigned int   dest_storage_offset;
             Xil_unsigned8* dest_data;
             dest_storage.getStorageInfo(band,
                                         NULL,
                                         &dest_scanline_stride,
                                         &dest_storage_offset,
                                         (void**)&dest_data);

             if (dest_box_type == XIL_AREA_CENTER) {
               // Process for Center Box

               // Get scanline and offset
               Xil_unsigned8* src_scanline = src_data +
                       (y*src_scanline_stride) +
                       ((src_storage_offset + x) / XIL_BIT_ALIGNMENT);
               
               int src_offset =
                       ((src_storage_offset + x) % XIL_BIT_ALIGNMENT);

               Xil_unsigned8* dest_scanline = dest_data +
                       (y*dest_scanline_stride) +
                       ((dest_storage_offset + x) / XIL_BIT_ALIGNMENT);
               
               int dest_offset =
                       ((dest_storage_offset + x) % XIL_BIT_ALIGNMENT);

               for (int l_y = 0; l_y < ysize; l_y++) {

                 Xil_unsigned8* dest_pixel = dest_scanline;

                 for (int l_x = 0; l_x < xsize; l_x++) {

                   int found = 0;

                   for (int z=0; z<sel_size; z++) {
                     int ipdx = l_x + xdeltas[z];
                     int ipdy = l_y + ydeltas[z];

                     //
                     // If src_offset is 0, we could be reading pixels
                     // in previous byte(s). So, we need to position our
                     // source pointer and offset accordingly
                     //
                     int tmp = ipdx + src_offset;

                     Xil_unsigned8 *src_pixel;
                     int new_src_offset;

                     if (tmp < 0) {
                      src_pixel = src_scanline +
                                          (ipdy * src_scanline_stride) -
                                          ((XIL_BIT_ALIGNMENT - tmp) / XIL_BIT_ALIGNMENT);
                       new_src_offset = (XIL_BIT_ALIGNMENT + tmp) % XIL_BIT_ALIGNMENT;
                     } else {
                       src_pixel = src_scanline +
                                          (ipdy * src_scanline_stride) +
                                          (tmp / XIL_BIT_ALIGNMENT);
                       new_src_offset = tmp % XIL_BIT_ALIGNMENT;
                     } 

                     if (!XIL_BMAP_TST(src_pixel, new_src_offset)) {
                       found = 1;
                       break;
                     }
                   }

                   if (found)
                     XIL_BMAP_CLR(dest_pixel, dest_offset + l_x);
                   else
                     XIL_BMAP_SET(dest_pixel, dest_offset + l_x);
                 }

                 // Move to the next line
                 dest_scanline += dest_scanline_stride;
               }

             } else {
               // Other Box cases

               // Get scanline and offset
               Xil_unsigned8* src_scanline = src_data +
                       (y*src_scanline_stride) +
                       ((src_storage_offset + x) / XIL_BIT_ALIGNMENT);
               
               int src_offset =
                       ((src_storage_offset + x) % XIL_BIT_ALIGNMENT);

               Xil_unsigned8* dest_scanline = dest_data +
                       (y*dest_scanline_stride) +
                       ((dest_storage_offset + x) / XIL_BIT_ALIGNMENT);
               
               int dest_offset =
                       ((dest_storage_offset + x) % XIL_BIT_ALIGNMENT);

               for (int l_y = 0; l_y < ysize; l_y++) {

                 Xil_unsigned8* dest_pixel = dest_scanline;

                 for (int l_x = 0; l_x < xsize; l_x++) {

                    int found = 0;

                    for (int z=0; z<sel_size; z++) {

                       int ipdx = l_x + xdeltas[z];
                       int ipdy = l_y + ydeltas[z];

                       switch (dest_box_type) {

                          case XIL_AREA_TOP_LEFT_CORNER:

                             if ((ipdx >= 0) && (ipdy >= 0)) {
                               int tmp = ipdx + src_offset;

                               Xil_unsigned8* src_pixel = src_scanline +
                                     (ipdy * src_scanline_stride) +
                                     (tmp / XIL_BIT_ALIGNMENT);

                               int new_src_offset = (tmp % XIL_BIT_ALIGNMENT);

                               if (!XIL_BMAP_TST(src_pixel, new_src_offset)) {
                                 found = 1;
                               }
                             }
                             break;

                          case XIL_AREA_TOP_EDGE:

                             if (ipdy >= 0) {
                               //
                               // If src_offset is 0, we could be reading
                               // pixels in previous byte(s). So, we need
                               // to position our source pointer and offset
                               // accordingly
                               //

                               int tmp = ipdx + src_offset;

                               Xil_unsigned8 *src_pixel;
                               int new_src_offset;

                               if (tmp < 0) {
                                src_pixel = src_scanline +
                                          (ipdy * src_scanline_stride) -
                                          ((XIL_BIT_ALIGNMENT - tmp) / XIL_BIT_ALIGNMENT);
                                 new_src_offset = (XIL_BIT_ALIGNMENT + tmp) % XIL_BIT_ALIGNMENT;
                               } else {
                                 src_pixel = src_scanline +
                                          (ipdy * src_scanline_stride) +
                                          (tmp / XIL_BIT_ALIGNMENT);
                                 new_src_offset = tmp % XIL_BIT_ALIGNMENT;
                               } 

                               if (!XIL_BMAP_TST(src_pixel, new_src_offset)) {
                                 found = 1;
                               }
                             }
                             break;

                          case XIL_AREA_TOP_RIGHT_CORNER:

                             if ((ipdx < (int)xsize) && (ipdy >= 0)) {
                               //
                               // If src_offset is 0, we could be reading
                               // pixels in previous byte(s). So, we need
                               // to position our source pointer and offset
                               // accordingly
                               //

                               int tmp = ipdx + src_offset;

                               Xil_unsigned8 *src_pixel;
                               int new_src_offset;

                               if (tmp < 0) {
                                src_pixel = src_scanline +
                                          (ipdy * src_scanline_stride) -
                                          ((XIL_BIT_ALIGNMENT - tmp) / XIL_BIT_ALIGNMENT);
                                 new_src_offset = (XIL_BIT_ALIGNMENT + tmp) % XIL_BIT_ALIGNMENT;
                               } else {
                                 src_pixel = src_scanline +
                                          (ipdy * src_scanline_stride) +
                                          (tmp / XIL_BIT_ALIGNMENT);
                                 new_src_offset = tmp % XIL_BIT_ALIGNMENT;
                               } 

                               if (!XIL_BMAP_TST(src_pixel, new_src_offset)) {
                                 found = 1;
                               }
                             }
                             break;

                          case XIL_AREA_LEFT_EDGE:

                            if (ipdx >= 0) {
                               int tmp = ipdx + src_offset;

                               Xil_unsigned8* src_pixel = src_scanline +
                                     (ipdy * src_scanline_stride) +
                                     (tmp / XIL_BIT_ALIGNMENT);

                               int new_src_offset = (tmp % XIL_BIT_ALIGNMENT);

                               if (!XIL_BMAP_TST(src_pixel, new_src_offset)) {
                                 found = 1;
                               }
                             }
                             break;

                          case XIL_AREA_RIGHT_EDGE:

                            if (ipdx  < (int) xsize) {
                               //
                               // If src_offset is 0, we could be reading
                               // pixels in previous byte(s). So, we need
                               // to position our source pointer and offset
                               // accordingly
                               //

                               int tmp = ipdx + src_offset;

                               Xil_unsigned8 *src_pixel;
                               int new_src_offset;

                               if (tmp < 0) {
                                src_pixel = src_scanline +
                                          (ipdy * src_scanline_stride) -
                                          ((XIL_BIT_ALIGNMENT - tmp) / XIL_BIT_ALIGNMENT);
                                 new_src_offset = (XIL_BIT_ALIGNMENT + tmp) % XIL_BIT_ALIGNMENT;
                               } else {
                                 src_pixel = src_scanline +
                                          (ipdy * src_scanline_stride) +
                                          (tmp / XIL_BIT_ALIGNMENT);
                                 new_src_offset = tmp % XIL_BIT_ALIGNMENT;
                               } 

                               if (!XIL_BMAP_TST(src_pixel, new_src_offset)) {
                                 found = 1;
                               }
                             }
                             break;

                          case XIL_AREA_BOTTOM_LEFT_CORNER:

                             if((ipdx >= 0) && (ipdy < (int) ysize)) {
                               int tmp = ipdx + src_offset;

                               Xil_unsigned8* src_pixel = src_scanline +
                                     (ipdy * src_scanline_stride) +
                                     (tmp / XIL_BIT_ALIGNMENT);

                               int new_src_offset = (tmp % XIL_BIT_ALIGNMENT);

                               if (!XIL_BMAP_TST(src_pixel, new_src_offset)) {
                                 found = 1;
                               }
                             }
                             break;

                          case XIL_AREA_BOTTOM_EDGE:

                             if ( ipdy < (int) ysize) {
                               //
                               // If src_offset is 0, we could be reading
                               // pixels in previous byte(s). So, we need
                               // to position our source pointer and offset
                               // accordingly
                               //

                               int tmp = ipdx + src_offset;

                               Xil_unsigned8 *src_pixel;
                               int new_src_offset;

                               if (tmp < 0) {
                                src_pixel = src_scanline +
                                          (ipdy * src_scanline_stride) -
                                          ((XIL_BIT_ALIGNMENT - tmp) / XIL_BIT_ALIGNMENT);
                                 new_src_offset = (XIL_BIT_ALIGNMENT + tmp) % XIL_BIT_ALIGNMENT;
                               } else {
                                 src_pixel = src_scanline +
                                          (ipdy * src_scanline_stride) +
                                          (tmp / XIL_BIT_ALIGNMENT);
                                 new_src_offset = tmp % XIL_BIT_ALIGNMENT;
                               } 

                               if (!XIL_BMAP_TST(src_pixel, new_src_offset)) {
                                 found = 1;
                               }
                             }
                             break;

                          case XIL_AREA_BOTTOM_RIGHT_CORNER:

                             if ( (ipdx < (int) xsize) && (ipdy < (int) ysize)) {
                               //
                               // If src_offset is 0, we could be reading
                               // pixels in previous byte(s). So, we need
                               // to position our source pointer and offset
                               // accordingly
                               //

                               int tmp = ipdx + src_offset;

                               Xil_unsigned8 *src_pixel;
                               int new_src_offset;

                               if (tmp < 0) {
                                src_pixel = src_scanline +
                                          (ipdy * src_scanline_stride) -
                                          ((XIL_BIT_ALIGNMENT - tmp) / XIL_BIT_ALIGNMENT);
                                 new_src_offset = (XIL_BIT_ALIGNMENT + tmp) % XIL_BIT_ALIGNMENT;
                               } else {
                                 src_pixel = src_scanline +
                                          (ipdy * src_scanline_stride) +
                                          (tmp / XIL_BIT_ALIGNMENT);
                                 new_src_offset = tmp % XIL_BIT_ALIGNMENT;
                               } 

                               if (!XIL_BMAP_TST(src_pixel, new_src_offset)) {
                                 found = 1;
                               }
                             }
                             break;

                          default:
                             break;
                       }

                       // Break out of for loop
                       if (found) break;
                    }

                    if (found)
                      XIL_BMAP_CLR(dest_pixel, dest_offset + l_x);
                    else
                      XIL_BMAP_SET(dest_pixel, dest_offset + l_x);
                 }

                 // Move to the next line
                 dest_scanline += dest_scanline_stride;
              }

            } // dest_box_type

          } // for bands

       } // while r1.getNext

    } // while b1->getNext

    // Release resources
    delete [] ydeltas;
    delete [] xdeltas;

    return XIL_SUCCESS;
}

