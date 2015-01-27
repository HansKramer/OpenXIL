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
//  File:   intraEncodeImage.cc
//  Project:    XIL
//  Revision:   1.4
//  Last Mod:   10:15:58, 03/10/00
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
#pragma ident   "@(#)intraEncodeImage.cc	1.4\t00/03/10  "

#include "XilDeviceCompressionCell.hh"
#include "InFrame.hh"

//------------------------------------------------------------------------
//
//  Function:    XilDeviceComputeTypeCellMemory::intraEncodeCurrentImage
//
//  Description:
//    This function runs through the image and compresses every 4x4
//    block using the selected encoding algorithm.
//    
//    Each block is also flipped if the top bit is 1.
//    
//    Finally, every block is ordered by luminance to make the
//    interframe encoding work properly (most effectively).  A flag
//    is set in the frame to indicate that a Cell has been flipped.
//    
//  Parameters:
//    void
//    
//  Returns:
//    XIL_SUCCESS or XIL_FAILURE
//    
//  Side Effects:
//    
//    
//  Notes:
//    
//    
//  Deficiencies/ToDo:
//    
//    
//------------------------------------------------------------------------

int
XilDeviceCompressionCell::intraEncodeCurrentImage(CellFrame& curFrame)
{
    //
    //  Compute the luminance of the current colormap
    //
    compData.currentCmap.computeLum();

    ColorValue  block[16];   // a 4x4 block of pixels
    Cell        cell;        // an encoded cell

    curFrame.clearFlipped();
    
    for (int y = 0; y < compData.cellHeight; y++) {
      for (int x = 0; x < compData.cellWidth; x++) {
        //
        //  Actually code to a cell
        //
        if (cellAttribs.encodingType == DITHER) {
          compData.inFrame->getRGBBlock(block);
          curFrame[y][x] = encodeDither(block);
        } else {
          compData.inFrame->getYUVBlock(block);
          curFrame[y][x] = encodeBTC(block);
        }
        
        if (curFrame[y][x].MASK() & 0x8000)
          curFrame[y][x].flipCell();

        //
        //  Order the colors by luminance, then by index
        //
        double lum0 = compData.currentCmap.lum(curFrame[y][x].C0());
        double lum1 = compData.currentCmap.lum(curFrame[y][x].C1());

        if ((lum0 == lum1 &&
             curFrame[y][x].C0() > curFrame[y][x].C1()) ||
             (lum0 >  lum1)) {
          curFrame.flipCell(y, x);
        }
      }
    }

    return XIL_SUCCESS;
}
