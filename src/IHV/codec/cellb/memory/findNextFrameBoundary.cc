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
//  File:   findNextFrameBoundary.cc
//  Project:    XIL
//  Revision:   1.2
//  Last Mod:   10:15:37, 03/10/00
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
#pragma ident   "@(#)findNextFrameBoundary.cc	1.2\t00/03/10  "


#include "XilDeviceCompressionCellB.hh"

//------------------------------------------------------------------------
//
//  Function:    XilDeviceCompressionCellB::findNextFrameBoundary
//  Created:    92/12/08
//
//  Description:
//    Fast scan of the CellB bytestream to find the next frame boundary.
//    
//  Parameters:
//    none
//    
//  Returns:
//    XIL_SUCCESS if a frame boundary found.
//    XIL_FAILURE if getNextByte returned a NULL before we reached a frame
//    boundary, if a bytestream error was detected, or if
//    foundNextFrameBoundary returned XIL_FAILURE.
//    
//  Side Effects:
//    
//  Notes:
//    We may have to traverse multiple "chunks" of data to get to the next
//    frame boundary.
//
//    Since a CellB code might straddle two "chunks" of data, we encapsulate
//    the getNextBytes function with a macro.
//    
//  Deficiencies/ToDo:
//    TODO: This implementation is somewhat slower than burnFrames.
//    Some analysis is needed to determine why.
//
//    TODO: the XIL_SIMULATE_FAILURE numbers were copied from the Cell-A
//    version of this function.
//------------------------------------------------------------------------

//
// At the point of calling this macro, nbytes = number of remaining bytes
// including *bp, which we've already used, so nbytes is actually one more
// than the number of ready-to-use bytes.
//
// This is why nbytes is initialized to 1.  The first time through,
// negative_shortage will be 0, and bp will end up pointing at the first byte.
//

#define GET_NEXT_BYTES_OR_FAIL(n)                        \
    if ((nbytes -= n) > 0) {                             \
    bp += n;                                             \
    } else {                                             \
        int negative_shortage = nbytes;                  \
    if ((bp = cbm->getNextBytes(&nbytes)) == NULL) {      \
            return XIL_FAILURE;                          \
    }                                                    \
    while (negative_shortage++ < 0) {                    \
        if (--nbytes > 0) {                              \
            bp++;                                        \
            } else {                                     \
        if ((bp = cbm->getNextBytes(&nbytes)) == NULL) {  \
            return XIL_FAILURE;                          \
        }                                                \
            }                                            \
        }                                                \
    }

int
XilDeviceCompressionCellB::findNextFrameBoundary()
{
    //
    // Instead of counting by rows & columns separately, we'll just count
    // total cells.
    //

    // 16 pixels / cell
    int cell_count = (int)(inputType->getHeight() * inputType->getWidth()) >> 4;

    if (cell_count == 0) {
      // see if we just have not derived the input type yet.
      XilImageFormat* ot = getOutputType();
      cell_count = (int)(inputType->getHeight() * inputType->getWidth()) >> 4;
    }

    if (cell_count <= 0) {
      // width and height must be defined before parsing Cis
      XIL_ERROR(getCis()->getSystemState(), XIL_ERROR_USER,"di-358",TRUE);
      return XIL_FAILURE;
    }
    
    //
    // Get the first byte
    //
    Xil_unsigned8* bp = NULL; // init to avoid warning that bp used before set
    int nbytes = 1;           // see macro comments above 
    GET_NEXT_BYTES_OR_FAIL(1);

    while (1) {
      if (*bp >= SKIPCODE) {
        cell_count -= *bp - (SKIPCODE - 1);
      } else {
        // normal 4x4 cell
        cell_count--;
        GET_NEXT_BYTES_OR_FAIL(3);
      }
    
      if (cell_count > 0) {
        GET_NEXT_BYTES_OR_FAIL(1);
      } else if (cell_count == 0) {
        return cbm->foundNextFrameBoundary(bp + 1);
      } else {
        // The cell_count is negative.  This could happen if an
        // interframe skip code ran past the end of the last row.
        // The error could have occurred earlier in this frame, or even
        // in an earlier frame; there is no way to be sure.
        
        // TODO: CellB message here?
        // (di-97 just says bytestream error)
        XIL_ERROR( NULL, XIL_ERROR_SYSTEM, "di-97", TRUE);  
        // byteStreamError(bp + 1);

        return XIL_FAILURE;
      }
    }
}
