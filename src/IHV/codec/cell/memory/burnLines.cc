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
//  File:   burnLines.cc
//  Project:    XIL
//  Revision:   1.2
//  Last Mod:   10:15:52, 03/10/00
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
#pragma ident   "@(#)burnLines.cc	1.2\t00/03/10  "

#include "XilDeviceCompressionCell.hh"

//------------------------------------------------------------------------
//
//  Function:    XilDeviceCompressionCell::burnLines
//
//  Description:
//    Quickly read through the Cell bytestream while updating the CellFrame
//    history buffer.  This is called from the GX molecules when the
//    clip shape does not include all of the lines through the bottom of
//    the window.
//    
//  Parameters:
//    number of lines to burn
//    
//  Returns:
//    void
//    
//  Side Effects:
//    updates decompData.bp
//    
//  Notes:
//    
//    
//  Deficiencies/ToDo:
//    TODO: bail out when bytestream error occurs
//    
//------------------------------------------------------------------------
void
XilDeviceCompressionCell::burnLines(int nlines)
{
  //
  // The bytestream is processed to update the contents of a CellFrame
  // object, which is a two-dimensional array of Cell objects.  
  //
  CellFrame& cell_frame = (*decompData.cellFrame);

  // compute the number of rows in columns of 4x4 cells in the image
  int max_row = cell_frame.getFrameHeight();
  int max_col = cell_frame.getFrameWidth();

  // bp is the byte pointer used to access the Cell bytestream; get its
  // initial value from decompData.bp which was set by the caller
  Xil_unsigned8* bp = decompData.bp;

  int row, col;
  for (row = max_row - nlines; row < max_row; row++) {
    col = 0;
    Cell* cell_line = cell_frame[row];

    do {
      unsigned int code = bp[0];
      if ((code & 0x80) == 0) {
        // this is either a regular 4x4 Cell or a runlength code
        code = (code << 8) | bp[1];
        if ((code) != 0) {
          // this is a regular 4x4 Cell
          cell_line[col].setCell(code, bp[3], bp[2]);
          col++;
        } else {
          // this is a runlength code
          int length = bp[3] + 1;

          if (col + length > max_col) {
            XIL_ERROR( NULL, XIL_ERROR_SYSTEM, "di-97", TRUE);
            byteStreamError(bp+4);
          }
          while (length--) {
            cell_line[col].setCell(0, bp[2], bp[2]);
            col++;
          }
        }
        bp += 4;
      } else {
        // this must be an escape code
        if ((code & 0x40) == 0) {
          // 10NNNNNN = interframe skip N+1 blocks
          col += (code & 0x3F) + 1;

          if (col > max_col) {
            XIL_ERROR( NULL, XIL_ERROR_SYSTEM, "di-97", TRUE);
            byteStreamError(bp+1);
          }
          bp++;
        } else {
          switch (code) {
            case CELL_COLORS_ONLY:
              cell_line[col].C0() = bp[1];
              cell_line[col].C1() = bp[2];
              bp += 3;
              break;

            case CELL_COLOR0_ONLY:
              cell_line[col].C0() = bp[1];
              bp += 2;
              break;

            case CELL_COLOR1_ONLY:
              cell_line[col].C1() = bp[1];
              bp += 2;
              break;

            case CELL_MASK_ONLY:
              cell_line[col].MASK() = (bp[1] << 8) | bp[2];
              bp += 3;
              break;

            default:
              XIL_ERROR( NULL, XIL_ERROR_SYSTEM, "di-97", TRUE); 
              byteStreamError(bp+1);
          }
          col++;
        }
      }
    } while (col < max_col);
  }
  decompData.bp = bp;
}
