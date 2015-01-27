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
//  File:   burnFrames.cc
//  Project:    XIL
//  Revision:   1.2
//  Last Mod:   10:15:33, 03/10/00
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
//  COPYRIGHT
//------------------------------------------------------------------------
#pragma ident   "@(#)burnFrames.cc	1.2\t00/03/10  "


#include "XilDeviceCompressionCellB.hh"

//------------------------------------------------------------------------
//
//  Function:    XilDeviceCompressionCellB::burnFrames
//  Created:    92/12/02
//
//  Description:
//    Quickly read through the CellB bytestream while updating the CellBFrame
//    history buffer.  This is called from the seek function when, for
//    example, the user wishes to skip some frames in a movie. It is also
//    called when doing unaccelerated memory-to-memory decompression.
//    
//  Parameters:
//    number of frames to burn
//    
//  Returns:
//    void
//    
//  Side Effects:
//    
//    
//  Notes:
//    
//    
//  Deficiencies/ToDo:
//    
//------------------------------------------------------------------------
void
XilDeviceCompressionCellB::burnFrames(int nframes)
{
    // compute the number of rows and columns of 4x4 cells in the image
    int w = inputType->getWidth();
    int h = inputType->getHeight();
    int height = h / 4;
    int width = w / 4;

    //
    // The bytestream is processed to update the contents of a CellBFrame
    // object, which is a two-dimensional array of CellB objects.  
    // Get a pointer to the CellBFrame object.  Supply the width and height
    // of the image in case the CellBFrame has not yet been allocated.
    //
    CellBFrame* cellb_frame = decompData.getCellBFrame(w,h);
    if (cellb_frame == NULL) {
      // TODO: getCellBFrame has already reported an error, but do we need
      // to generate a secondary error?
      return;
    }

    for (int i = nframes; i > 0; i--) {
      // get a pointer to the bytestream
      Xil_unsigned8* bp = (Xil_unsigned8*)cbm->nextFrame();
      if (bp == NULL) {
        // XilCis: No data to decompress
        XIL_ERROR(getCis()->getSystemState(), XIL_ERROR_SYSTEM, "di-100", TRUE);
        return;
      }

      int row = 0;
      int col = 0;

      while (row < height) {
        unsigned int pattern = bp[0];
        if (pattern >= SKIPCODE) {
          bp++;
          pattern -= (SKIPCODE - 1);    // pattern = # of cells to skip
          col      += pattern;
          while (col >= width) {
            row++;
            col -= width;
          }
        } else {
          //
          // Build a 32-bit value from the next 4 bytes in the
          // bytestream: a 16 bit of mask (MMMM), an 8 bit uv index (UV),
          // and an 8 bit yy index (YY).  The pointer bp may not be
          // word-aligned, so we have to get it a byte at a time.
          //
          cellb_frame->cellB(row, col).setCellB((pattern << 24) |
                             (bp[1]   << 16) | 
                             (bp[2]   <<  8) |
                             bp[3]);
          bp += 4;

          if (++col >= width) {
            col = 0;
            row++;
          }
        }
      }

      // Tell the CIS buffer mgr where the end of the frame is.
      // 2nd arg is 0 because we don't care about frame type for CellB
      cbm->decompressedFrame(bp, 0);
    }
}
