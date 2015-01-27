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
//  File:   byteStreamError.cc
//  Project:    XIL
//  Revision:   1.2
//  Last Mod:   10:15:54, 03/10/00
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
#pragma ident   "@(#)byteStreamError.cc	1.2\t00/03/10  "

#include "XilDeviceCompressionCell.hh"

//------------------------------------------------------------------------
//
//  Function:    byteStreamError
//
//  Description:
//    
//    Called if an error is detected in the Cell bytestream:
//       - invalid escape code
//       - runlength or interframe skip which exceeds the end of the scanline
//       - colormap index exceeds the size of the current colormap (TODO)
//    
//  Parameters:
//    
//    Pointer to the first byte in the Cell bytestream beyond the error.
//    
//  Returns:
//    
//    Always returns XIL_FAILURE.
//    
//  Side Effects:
//    
//    Tells the CIS buffer manager that the end of a frame was found.
//    
//  Deficiencies/ToDo:
//    
//    Should we set decompData.bp to NULL?  The whole use of this function
//    needs to be re-visited after the error plan is resolved.
//    
//------------------------------------------------------------------------

XilStatus
XilDeviceCompressionCell::byteStreamError(Xil_unsigned8* bp)
{
    // Tell the CIS buffer mgr where the end of the frame is.
    // decompressHeader() will set frameType to indicate whether
    // or not this is a key frame.
    cbm->decompressedFrame(bp, decompData.frameType);

    // XIL_ERROR_CIS_DATA();
    XIL_ERROR( NULL, XIL_ERROR_SYSTEM, "di-97", FALSE);
    return XIL_FAILURE;
}
