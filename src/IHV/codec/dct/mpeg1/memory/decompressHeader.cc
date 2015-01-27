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
//  File:       decompressHeader.cc
//  Project:    XIL
//  Revision:   1.4
//  Last Mod:   10:14:50, 03/10/00
//
//  Description:
//
//    Decompress the header of an Mpeg1 Video stream
//
//
//------------------------------------------------------------------------
//    COPYRIGHT
//------------------------------------------------------------------------
#pragma ident   "@(#)decompressHeader.cc	1.4\t00/03/10  "

#include "XilDeviceCompressionMpeg1.hh"
#include "Mpeg1DecompressorData.hh"

XilStatus        
XilDeviceCompressionMpeg1::decompressHeader(void)
{
    Xil_unsigned8*    bp;
    Xil_unsigned8*    endOfBuffer;
    mpeg_user_data_t* user_ptr;

    XilCisBufferManager* theCBM = getCisBufferManager();

    //
    // We need to clear any outstanding compressions 
    // because we must read the header information here.
    //
    // In 1.3, we don't.
    //
    //cis->sync();

    //
    // Check for frame avail in current subgroup
    //
    if(hasFrame() == FALSE) {
        XIL_ERROR(getSystemState(), XIL_ERROR_SYSTEM, "di-100", TRUE);
        return XIL_FAILURE;
    }
    //
    // bp is the byte pointer used to access the Mpeg1 bytestream.
    // Get its initial value from the CIS buffer mgr
    //
    if((bp = (Xil_unsigned8*)theCBM->nextFrame(&endOfBuffer)) == NULL) {
        XIL_ERROR(getSystemState(), XIL_ERROR_SYSTEM, "di-100", TRUE);
        return XIL_FAILURE;
    }

    user_ptr = (mpeg_user_data_t *)theCBM->getRFrameUserPtr();
    if(user_ptr != NULL) {
        decompData.temporalReference = user_ptr->temp_ref;
        decompData.frameType         = user_ptr->frame_type;
        decompData.timeCodeD         = user_ptr->time_code;
        decompData.closedGop         = user_ptr->closed_gop;
        decompData.brokenLink        = user_ptr->broken_link;
        decompData.aspectRatioD      = user_ptr->aspect_ratio;
        decompData.pictureRateD      = user_ptr->picture_rate;
        return XIL_SUCCESS;
    }

    //
    // Here if no header data found even though the cbm thought there
    // was a frame to process...must be incomplete or corrupted frame.
    // Issue error message but
    // return XIL_SUCCESS so the decompress will update the read_frame
    //
    XIL_ERROR(getSystemState(), XIL_ERROR_SYSTEM, "di-329", TRUE);

    return XIL_SUCCESS;
}
