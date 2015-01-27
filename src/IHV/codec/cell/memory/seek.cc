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
//  File:   seek.cc
//  Project:    XIL
//  Revision:   1.2
//  Last Mod:   10:15:48, 03/10/00
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
#pragma ident   "@(#)seek.cc	1.2\t00/03/10  "

#include "XilDeviceCompressionCell.hh"

//------------------------------------------------------------------------
//
//  Function:    XilDeviceCompressionCell::seek
//
//  Description:
//    Since Cell compression must maintain history, we must seek to the
//    closest key-frame and then burn forward to the actual frame.
//    Unless "cis" function indicates that the history update is not
//      necessary.
//    
//  Parameters:
//    
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
//    
//------------------------------------------------------------------------

void
XilDeviceCompressionCell::seek(int framenumber, Xil_boolean history_update)
{
    int frames_to_burn;

    if (history_update == TRUE)
      frames_to_burn = cbm->seek(framenumber, XIL_CIS_CELL_KEY_FRAME);
    else
      frames_to_burn = cbm->seek(framenumber, XIL_CIS_NO_BURN_TYPE);

    if (frames_to_burn > 0) {
      burnFrames(frames_to_burn);
      // Burning frames updates the history buffer but not the frame buffer,
      // so this is a signal molecules which rely on the contents of
      // the frame buffer.
      decompData.redrawNeeded = 1;
    }

}
