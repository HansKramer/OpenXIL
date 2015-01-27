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
//This line lets emacs recognize this as -*- C++ -*- Code
//------------------------------------------------------------------------
//
//  File:	BandCombine.cc
//  Project:	XIL
//  Revision:	1.4
//  Last Mod:	10:09:27, 03/10/00
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
#pragma ident	"@(#)BandCombine.cc	1.4\t00/03/10  "

#include "XiliUtils.hh"
#include "XilDeviceManagerComputeBIT.hh"

XilStatus
XilDeviceManagerComputeBIT::BandCombine(XilOp*       op,
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

    XilKernel*    kernel;
    op->getParam(1, (void**)&kernel);

    //
    // Get Kernel information
    //
    unsigned int mwidth  = kernel->getWidth();
    unsigned int mheight = kernel->getHeight();
    const float *mdata = kernel->getData();

    //
    //  Store away the number of bands for this operation.
    //
    unsigned int src_bands   = src->getNumBands();
    unsigned int dest_bands   = dest->getNumBands();

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

           if ((src_storage.isType(XIL_BAND_SEQUENTIAL)) &&
               (dest_storage.isType(XIL_BAND_SEQUENTIAL))) {

             //
             // BandSequential Implementation
             //

             unsigned int   src_scanline_stride;
             unsigned int   src_storage_offset;
             Xil_unsigned8* src_data;
             src_storage.getStorageInfo((unsigned int) 0,
                                        NULL,
                                        &src_scanline_stride,
                                        &src_storage_offset,
                                        (void**)&src_data);

             unsigned int   dest_scanline_stride;
             unsigned int   dest_storage_offset;
             Xil_unsigned8* dest_data;
             dest_storage.getStorageInfo((unsigned int) 0,
                                         NULL,
                                         &dest_scanline_stride,
                                         &dest_storage_offset,
                                         (void**)&dest_data);

             Xil_unsigned8* dest_scanline = dest_data +
                       (y*dest_scanline_stride) +
                       ((dest_storage_offset + x) / XIL_BIT_ALIGNMENT);

             int dest_offset =
                       ((dest_storage_offset + x) % XIL_BIT_ALIGNMENT);

             Xil_unsigned8* src_scanline = src_data +
                       (y*src_scanline_stride) +
                       ((src_storage_offset + x) / XIL_BIT_ALIGNMENT);
               
             int src_offset =
                       ((src_storage_offset + x) % XIL_BIT_ALIGNMENT);

             for (int l_y = 0; l_y < ysize; l_y++) {

               for (int l_x = 0; l_x < xsize; l_x++) {

                 const float* mptr = mdata;

                 Xil_unsigned8* dest_band = dest_scanline;

                 //
                 // Loop over dest bands
                 //
                 for (int d_band = 0; d_band < mheight; d_band++) {
                   float sum = 0.0;

                   //
                   // Loop over source bands
                   //

                   Xil_unsigned8* src_band = src_scanline;

                   for (int s_band = 0; s_band < mwidth - 1; s_band++) {
                     if (XIL_BMAP_TST(src_band, src_offset + l_x)) {
                       sum += *mptr;
                     }
                     mptr ++;

                     // Go to next band
                     src_band += src_storage.getBandStride(); 
                   }

                   //
                   // Add in constant value
                   //
                   sum += *mptr++;

                   //
                   // Write out to dst bands
                   //
                   if (sum > 0.5)
                     XIL_BMAP_SET(dest_band, dest_offset + l_x);
                   else
                     XIL_BMAP_CLR(dest_band, dest_offset + l_x);

                   // Go to next Band
                   dest_band += dest_storage.getBandStride();
                 }
              }

              // Go to next scanline
              src_scanline += src_scanline_stride;
              dest_scanline += dest_scanline_stride;
            }

          } else {

            //
            // General Storage Implementation.
            //

            // 
            // Painfully slow implementation, but there is no
            // other way for General storage. 
            // 

            for (int l_y = 0; l_y < ysize; l_y++) {

              for (int l_x = 0; l_x < xsize; l_x++) {

                const float* mptr = mdata;

                //
                // Loop over dest bands
                //
                for (int dest_band = 0; dest_band < mheight; dest_band++) {

                  //
                  // Get all info for current destination band's scanline
                  //

                  unsigned int   dest_scanline_stride;
                  unsigned int   dest_storage_offset;
                  Xil_unsigned8* dest_data;
                  dest_storage.getStorageInfo((unsigned int) dest_band,
                                              NULL,
                                              &dest_scanline_stride,
                                              &dest_storage_offset,
                                              (void**)&dest_data);

                  Xil_unsigned8* dest_scanline = dest_data +
                          ((y + l_y) * dest_scanline_stride) +
                          ((dest_storage_offset + x) / XIL_BIT_ALIGNMENT);
               
                  int dest_offset =
                          ((dest_storage_offset + x) % XIL_BIT_ALIGNMENT);

                  float sum = 0.0;

                  //
                  // Loop over source bands
                  //

                  for (int src_band = 0; src_band < mwidth - 1; src_band++) {

                    //
                    // Get all info for current source band's scanline
                    //

                    unsigned int   src_scanline_stride;
                    unsigned int   src_storage_offset;
                    Xil_unsigned8* src_data;
                    src_storage.getStorageInfo((unsigned int) src_band,
                                               NULL,
                                               &src_scanline_stride,
                                               &src_storage_offset,
                                               (void**)&src_data);

                    Xil_unsigned8* src_scanline = src_data +
                           ((y + l_y) * src_scanline_stride) +
                           ((src_storage_offset + x) / XIL_BIT_ALIGNMENT);
               
                    int src_offset =
                           ((src_storage_offset + x) % XIL_BIT_ALIGNMENT);

                    if (XIL_BMAP_TST(src_scanline, src_offset + l_x)) {
                      sum += *mptr;
                    }

                    mptr ++;
                  }

                  //
                  // Add in constant value
                  //
                  sum += *mptr++;

                  //
                  // Write out to dst bands
                  //
                  if (sum > 0.5)
                    XIL_BMAP_SET(dest_scanline, dest_offset + l_x);
                  else
                    XIL_BMAP_CLR(dest_scanline, dest_offset + l_x);
                }

              }

            }

         } // General Storage	
 
      } // Next Rectangle

    } // while b1->getNext

    return XIL_SUCCESS;
}

