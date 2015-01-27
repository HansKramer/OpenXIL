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
//  File:	Dilate.cc
//  Project:	XIL
//  Revision:	1.13
//  Last Mod:	10:10:30, 03/10/00
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
#pragma ident	"@(#)Dilate.cc	1.13\t00/03/10  "

#include "XilDeviceManagerComputeBYTE.hh"

void dlate_core (int ipdx,
                 int ipdy,
                 int l_x,
                 int l_y,
                 unsigned int nbands,
                 unsigned int  src_pixel_stride,
                 unsigned int  src_scanline_stride,
                 Xil_unsigned8 *src_scanline,
                 Xil_unsigned8 *mmax);

XilStatus
XilDeviceManagerComputeBYTE::Dilate(XilOp*       op,
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
    // Get parameters for Dilate
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

    Xil_unsigned8 *mmax = new Xil_unsigned8[nbands];
    if (mmax == NULL) {
      delete [] xdeltas;
      delete [] ydeltas;
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
            //  We do not currently support dilate
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
        for (unsigned int i = 0; i < height; i++) {
          for (unsigned int j = 0; j < width; j++) {
            if (*tmp_data++) {
              ydeltas[sel_size] = i - keyy;
              xdeltas[sel_size] = j - keyx;
              sel_size++;
            }
          }
        }

        //
        //  Test to see if all of our storage is of type XIL_PIXEL_SEQUENTIAL.
        //  If so, implement an loop optimized for pixel-sequential storage.
        //
        if((src_storage.isType(XIL_PIXEL_SEQUENTIAL)) &&
           (dest_storage.isType(XIL_PIXEL_SEQUENTIAL))) {

            unsigned int   src_pixel_stride;
            unsigned int   src_scanline_stride;
            Xil_unsigned8* src_data;
            src_storage.getStorageInfo(&src_pixel_stride,
                                        &src_scanline_stride,
                                        NULL, NULL,
                                        (void**)&src_data);

            unsigned int   dest_pixel_stride;
            unsigned int   dest_scanline_stride;
            Xil_unsigned8* dest_data;
            dest_storage.getStorageInfo(&dest_pixel_stride,
                                        &dest_scanline_stride,
                                        NULL, NULL,
                                        (void**)&dest_data);

            //
            //  Create a list of rectangles to loop over.  The resulting list
            //  of rectangles is the area left by intersecting the ROI with
            //  the destination box.
            //
            XilRectList    rl(roi, dest_box);

            int            x;
            int            y;
            unsigned int   xsize;
            unsigned int   ysize;

            while(rl.getNext(&x, &y, &xsize, &ysize)) {
              if (dest_box_type == XIL_AREA_CENTER) {
                // Process for Center Box

                Xil_unsigned8* src_scanline = src_data +
                    (y*src_scanline_stride) + (x*src_pixel_stride);

                Xil_unsigned8* dest_scanline = dest_data +
                    (y*dest_scanline_stride) + (x*dest_pixel_stride);

                for (int l_y = 0; l_y < ysize; l_y++) {

                  Xil_unsigned8* src_pixel = src_scanline;
                  Xil_unsigned8* dest_pixel = dest_scanline;

                  for (int l_x = 0; l_x < xsize; l_x++) {

                    if (nbands == 1) {
                      mmax[0] = XIL_MINBYTE;
                    } else {
                      for (unsigned int k1=0; k1<nbands; k1++)
                        mmax[k1] = XIL_MINBYTE;
                    } 

                    for (unsigned int z=0; z<sel_size; z++) {
                      int ipdx = xdeltas[z];
                      int ipdy = ydeltas[z];

                      Xil_unsigned8 *src_band = src_pixel +
                                         (ipdy * src_scanline_stride) +
                                         (ipdx * src_pixel_stride);

                      if (nbands == 1) {
                        if (*src_band > mmax[0])
                          mmax[0] = *src_band;
                      } else {
                        // loop over bands
                        for (unsigned int k2 = 0; k2 < nbands; k2++) {
                          if (*src_band > mmax[k2])
                            mmax[k2] = *src_band;
                          src_band++;
                        }
                      }
                    }

                    if (nbands == 1) {
                      *dest_pixel = mmax[0];
                    } else {
                      Xil_unsigned8* dest_band = dest_pixel;
                      for (unsigned int k3 = 0; k3 < nbands; k3++) {
                        *dest_band = mmax[k3];
                        dest_band++;
                      }
                    }

                    // Move to the next pixel
                    src_pixel += src_pixel_stride;
                    dest_pixel += dest_pixel_stride;
                  }

                  // Move to the next line
                  src_scanline += src_scanline_stride;
                  dest_scanline += dest_scanline_stride;
                }

              } else {
                // Other Box cases

                Xil_unsigned8* src_scanline = src_data +
                    (y*src_scanline_stride) + (x*src_pixel_stride);

                Xil_unsigned8* dest_scanline = dest_data +
                    (y*dest_scanline_stride) + (x*dest_pixel_stride);

                for (int l_y = 0; l_y < ysize; l_y++) {

                  Xil_unsigned8* dest_pixel = dest_scanline;

                  for (int l_x = 0; l_x < xsize; l_x++) {

                    if (nbands == 1) {
                      mmax[0] = XIL_MINBYTE;
                    } else {
                      for (unsigned int k1=0; k1<nbands; k1++)
                        mmax[k1] = XIL_MINBYTE;
                    } 

                    for (unsigned int z=0; z<sel_size; z++) {

                      int ipdx = xdeltas[z];
                      int ipdy = ydeltas[z];

                      switch (dest_box_type) {
                          //
                          // NOTE that if l_x or l_y become
                          // unsigned ints, they will need
                          // explicit casting or the following
                          // tests will fail, because ipdx, ipdy
                          // CAN be negative!
                          //
                        case XIL_AREA_TOP_EDGE:
                            if (ipdy + l_y >= 0) {
                              dlate_core(ipdx,
                                         ipdy,
                                         l_x,
                                         l_y,
                                         nbands,
                                         src_pixel_stride,
                                         src_scanline_stride,
                                         src_scanline,
                                         mmax);
                            }
                            break;

                        case XIL_AREA_BOTTOM_EDGE:
                            if (ipdy + l_y < (int) ysize) {
                              dlate_core(ipdx,
                                         ipdy,
                                         l_x,
                                         l_y,
                                         nbands,
                                         src_pixel_stride,
                                         src_scanline_stride,
                                         src_scanline,
                                         mmax);
                            }
                            break;

                        case XIL_AREA_LEFT_EDGE:
                          if (ipdx + l_x >= 0) {
                              dlate_core(ipdx,
                                         ipdy,
                                         l_x,
                                         l_y,
                                         nbands,
                                         src_pixel_stride,
                                         src_scanline_stride,
                                         src_scanline,
                                         mmax);
                            }
                            break;

                        case XIL_AREA_TOP_LEFT_CORNER:
                            if ((ipdx + l_x >= 0) &&
                                ((ipdy + l_y) >= 0)) {
                              dlate_core(ipdx,
                                         ipdy,
                                         l_x,
                                         l_y,
                                         nbands,
                                         src_pixel_stride,
                                         src_scanline_stride,
                                         src_scanline,
                                         mmax);
                            }
                            break;

                        case XIL_AREA_BOTTOM_LEFT_CORNER:
                            if ((ipdx + l_x >= 0) &&
                                ((ipdy + l_y) < (int) ysize)) {
                              dlate_core(ipdx,
                                         ipdy,
                                         l_x,
                                         l_y,
                                         nbands,
                                         src_pixel_stride,
                                         src_scanline_stride,
                                         src_scanline,
                                         mmax);
                            }
                            break;

                        case XIL_AREA_RIGHT_EDGE:
                          if (ipdx + l_x < (int) xsize) {
                              dlate_core(ipdx,
                                         ipdy,
                                         l_x,
                                         l_y,
                                         nbands,
                                         src_pixel_stride,
                                         src_scanline_stride,
                                         src_scanline,
                                         mmax);
                            }
                            break;

                        case XIL_AREA_TOP_RIGHT_CORNER:
                            if ((ipdx + l_x < (int) xsize ) &&
                                ((ipdy + l_y) >= 0)) {
                              dlate_core(ipdx,
                                         ipdy,
                                         l_x,
                                         l_y,
                                         nbands,
                                         src_pixel_stride,
                                         src_scanline_stride,
                                         src_scanline,
                                         mmax);
                            }
                            break;

                        case XIL_AREA_BOTTOM_RIGHT_CORNER:
                            if ((ipdx + l_x < (int) xsize) &&
                                ((ipdy + l_y) < (int) ysize)) {
                              dlate_core(ipdx,
                                         ipdy,
                                         l_x,
                                         l_y,
                                         nbands,
                                         src_pixel_stride,
                                         src_scanline_stride,
                                         src_scanline,
                                         mmax);
                            }
                            break;

                        default: break;
                      } // end Switch

                    }

                    if (nbands == 1) {
                      *dest_pixel = mmax[0];
                    } else {
                      Xil_unsigned8* dest_band = dest_pixel;
                      for (unsigned int k3 = 0; k3 < nbands; k3++) {
                        *dest_band = mmax[k3];
                        dest_band++;
                      }
                    }

                    // Move to the next pixel
                    dest_pixel += dest_pixel_stride;
                  }

                  // Move to the next line
                  dest_scanline += dest_scanline_stride;
                }

              }

            } // while r1.getNext

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

                  unsigned int   src_pixel_stride;
                  unsigned int   src_scanline_stride;
                  Xil_unsigned8* src_data;
                  src_storage.getStorageInfo(band,
                                             &src_pixel_stride,
                                             &src_scanline_stride,
                                             NULL,
                                             (void**)&src_data);

                  unsigned int   dest_pixel_stride;
                  unsigned int   dest_scanline_stride;
                  Xil_unsigned8* dest_data;
                  dest_storage.getStorageInfo(band,
                                              &dest_pixel_stride,
                                              &dest_scanline_stride,
                                              NULL,
                                              (void**)&dest_data);

                 if (dest_box_type == XIL_AREA_CENTER) {
                   // Process for Center Box

                   Xil_unsigned8* src_scanline = src_data +
                       (y*src_scanline_stride) + (x*src_pixel_stride);

                   Xil_unsigned8* dest_scanline = dest_data +
                       (y*dest_scanline_stride) + (x*dest_pixel_stride);

                   for (int l_y = 0; l_y < ysize; l_y++) {

                     Xil_unsigned8* src_pixel = src_scanline;
                     Xil_unsigned8* dest_pixel = dest_scanline;

                     for (int l_x = 0; l_x < xsize; l_x++) {

                       mmax[0] = XIL_MINBYTE;

                       for (unsigned int z=0; z<sel_size; z++) {
                         int ipdx = xdeltas[z];
                         int ipdy = ydeltas[z];

                         Xil_unsigned8 *src_band = src_pixel +
                                          (ipdy * src_scanline_stride) +
                                          (ipdx * src_pixel_stride);

                         if (*src_band > mmax[0])
                           mmax[0] = *src_band;
                       }

                       *dest_pixel = mmax[0];

                       // Move to the next pixel
                       src_pixel += src_pixel_stride;
                       dest_pixel += dest_pixel_stride;
                     }

                     // Move to the next line
                     src_scanline += src_scanline_stride;
                     dest_scanline += dest_scanline_stride;
                  }

                } else {
                  // Other Box cases

                  Xil_unsigned8* src_scanline = src_data +
                      (y*src_scanline_stride) + (x*src_pixel_stride);

                  Xil_unsigned8* dest_scanline = dest_data +
                      (y*dest_scanline_stride) + (x*dest_pixel_stride);

                  for (int l_y = 0; l_y < ysize; l_y++) {

                    Xil_unsigned8* dest_pixel = dest_scanline;

                    for (int l_x = 0; l_x < xsize; l_x++) {

                      mmax[0] = XIL_MINBYTE;

                      for (unsigned int z=0; z<sel_size; z++) {

                        int ipdx = xdeltas[z];
                        int ipdy = ydeltas[z];

                        switch (dest_box_type) {
                          //
                          // NOTE that if l_x or l_y become
                          // unsigned ints, they will need
                          // explicit casting or the following
                          // tests will fail, because ipdx, ipdy
                          // CAN be negative!
                          //
                          case XIL_AREA_TOP_EDGE:
                            if (ipdy + l_y >= 0) {
                              Xil_unsigned8 *src_band = src_scanline +
                                          ((ipdy + l_y) * src_scanline_stride) +
                                           ((ipdx + l_x) * src_pixel_stride);

                              if (*src_band > mmax[0])
                                mmax[0] = *src_band;
                            }
                            break;

                          case XIL_AREA_BOTTOM_EDGE:
                            if (ipdy + l_y < (int) ysize) {
                              Xil_unsigned8 *src_band = src_scanline +
                                          ((ipdy + l_y) * src_scanline_stride) +
                                           ((ipdx + l_x) * src_pixel_stride);

                              if (*src_band > mmax[0])
                                mmax[0] = *src_band;
                            }
                            break;

                          case XIL_AREA_LEFT_EDGE:
                            if (ipdx + l_x >= 0) {
                              Xil_unsigned8 *src_band = src_scanline +
                                          ((ipdy + l_y) * src_scanline_stride) +
                                           ((ipdx + l_x) * src_pixel_stride);

                              if (*src_band > mmax[0])
                                mmax[0] = *src_band;
                            }
                            break;

                          case XIL_AREA_TOP_LEFT_CORNER:
                            if ((ipdx + l_x >= 0) &&
                                ((ipdy + l_y) >= 0)) {
                              Xil_unsigned8 *src_band = src_scanline +
                                          ((ipdy + l_y) * src_scanline_stride) +
                                           ((ipdx + l_x) * src_pixel_stride);

                              if (*src_band > mmax[0])
                                mmax[0] = *src_band;
                            }
                            break;

                          case XIL_AREA_BOTTOM_LEFT_CORNER:
                            if ((ipdx + l_x >= 0) &&
                                ((ipdy + l_y) < (int) ysize)) {
                              Xil_unsigned8 *src_band = src_scanline +
                                          ((ipdy + l_y) * src_scanline_stride) +
                                           ((ipdx + l_x) * src_pixel_stride);

                              if (*src_band > mmax[0])
                                mmax[0] = *src_band;
                            }
                            break;

                          case XIL_AREA_RIGHT_EDGE:
                            if (ipdx + l_x < (int) xsize) {
                              Xil_unsigned8 *src_band = src_scanline +
                                          ((ipdy + l_y) * src_scanline_stride) +
                                           ((ipdx + l_x) * src_pixel_stride);

                              if (*src_band > mmax[0])
                                mmax[0] = *src_band;
                            }
                            break;

                          case XIL_AREA_TOP_RIGHT_CORNER:
                            if ((ipdx + l_x < (int) xsize) &&
                                ((ipdy + l_y) >= 0)) {
                              Xil_unsigned8 *src_band = src_scanline +
                                          ((ipdy + l_y) * src_scanline_stride) +
                                           ((ipdx + l_x) * src_pixel_stride);

                              if (*src_band > mmax[0])
                                mmax[0] = *src_band;
                            }
                            break;

                          case XIL_AREA_BOTTOM_RIGHT_CORNER:
                            if ((ipdx + l_x < (int) xsize) &&
                                ((ipdy + l_y) < (int) ysize)) {
                              Xil_unsigned8 *src_band = src_scanline +
                                          ((ipdy + l_y) * src_scanline_stride) +
                                           ((ipdx + l_x) * src_pixel_stride);

                              if (*src_band > mmax[0])
                                mmax[0] = *src_band;
                            }
                            break;

                          default: break;
                        } // end Switch

                      }

                      *dest_pixel = mmax[0];

                      // Move to the next pixel
                      dest_pixel += dest_pixel_stride;
                    }

                    // Move to the next line
                    dest_scanline += dest_scanline_stride;
                  }

                }

              } // for bands

            } // while r1.getNext

        }  // else pixel_type

    } // while b1->getNext

    // Release resources
    delete [] mmax;
    delete [] ydeltas;
    delete [] xdeltas;

    return XIL_SUCCESS;
}

void dlate_core (int ipdx,
                 int ipdy,
                 int l_x,
                 int l_y,
                 unsigned int nbands,
                 unsigned int  src_pixel_stride,
                 unsigned int  src_scanline_stride,
                 Xil_unsigned8 *src_scanline,
                 Xil_unsigned8 *mmax)
{
   Xil_unsigned8 *src_band = src_scanline +
                             ((ipdy + l_y) * src_scanline_stride) +
                             ((ipdx + l_x) * src_pixel_stride);

   if (nbands == 1) {
     if (*src_band > mmax[0])
       mmax[0] = *src_band;
   } else {
     // loop over bands
     for (unsigned int k2 = 0; k2 < nbands; k2++) {
       if (*src_band > mmax[k2])
         mmax[k2] = *src_band;
       src_band++;
     }
   }

   return;
}

