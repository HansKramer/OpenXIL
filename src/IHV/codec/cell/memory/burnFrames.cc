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
//  Revision:   1.3
//  Last Mod:   10:15:45, 03/10/00
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
#pragma ident   "@(#)burnFrames.cc	1.3\t00/03/10  "

#include "XilDeviceCompressionCell.hh"

//------------------------------------------------------------------------
//
//  Function:    XilDeviceCompressionCell::burnFrames
//
//  Description:
//    Quickly read through the Cell bytestream while updating the CellFrame
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
//    TODO: bail out when bytestream error occurs
//    
//------------------------------------------------------------------------
void
XilDeviceCompressionCell::burnFrames(int nframes)
{
  //
  // Under certain conditions, the image dimensions might be unknown at this
  // point.  For example, the sequence [putBitsPtr, seek, decompress].  If so,
  // we must derive the dimensions from the first frame header.
  //

  if (imageHeight == 0) {
    if (deriveOutputType() != XIL_SUCCESS) {

      // Couldn't burn frames
      // Unable to complete seek by skipping (burn) frames

      XIL_ERROR( NULL, XIL_ERROR_SYSTEM, "di-109", FALSE);
      return;
    }
  }

  // compute the number of rows in columns of 4x4 cells in the image
  int max_row = imageHeight / 4;
  int max_col = imageWidth / 4;

  // fprintf(stderr, "In burnFrames: %d\n", nframes);
  
#if 0
  printf("burnFrames, nframes = %d, bp = %08x\n", nframes, decompData.bp);
#endif

  //
  // The bytestream is processed to update the contents of a CellFrame
  // object, which is a two-dimensional array of Cell objects.  
  // If the CellFrame object pointed to by decompData.cellFrame has not
  // already been constructed in deriveOutputType(), then we'll construct
  // it here.
  //

  if (decompData.cellFrame == NULL) {

    // If the user does a sequence like [putBitsPtr, compress, seek, 
    // decompress] then we would end up here because imageWidth and 
    // imageHeight would be set when this function was called but 
    // deriveOutputType would not have been called yet.

    decompData.cellFrame = new CellFrame(max_col, max_row);
    if (decompData.cellFrame == NULL) {
      XIL_ERROR( NULL, XIL_ERROR_RESOURCE,"di-1",TRUE);
      return;
    }
  }
  CellFrame& cell_frame = (*decompData.cellFrame);

  // When burning more than one frame, we postpone updates to the user's
  // colormap until reading the header for the last frame.  This flag is
  // checked in decompressHeader().
  //
  decompData.updateUserColormapEnabled = 0;

  for (int i = nframes; i > 0; i--) {

    // If this for loop is modified to have an early termination (for example,
    // after a bytestream error), then this flag must be reset back to 1
    // before leaving this function.
    //
    if (i == 1) decompData.updateUserColormapEnabled = 1;

    if(decompressHeader() == XIL_FAILURE) {
        XIL_OBJ_ERROR(system_state, XIL_ERROR_SYSTEM,
                      "di-311", FALSE, getCis());
        return;
    }
    
    // bp is the byte pointer used to access the Cell bytestream; get its
    // initial value from decompData.bp which was set by decompressHeader()
    Xil_unsigned8* bp = decompData.bp;

    if (bp[0] == CELL_SKIP_ENTIRE_FRAME) {
      // This one-byte code means the entire frame is the same as
      // the previous one, and is utilized by the bit-rate control stuff
      // in the Cell compressor code.
      bp++;
    } else {
      int row, col;
      for (row = 0; row < max_row; row++) {
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
    }

    // Tell the CIS buffer mgr where the end of the frame is.
    // decompressHeader() will set frameType to indicate whether
    // or not this is a key frame.
    cbm->decompressedFrame(bp, decompData.frameType);

    // Setting this to NULL is a signal to decompressHeader() that we've read
    // the frame.
    decompData.bp = NULL;
  }
}
