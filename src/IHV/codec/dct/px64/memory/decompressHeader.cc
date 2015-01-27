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
//  File:	decompressHeader.cc
//  Project:	XIL
//  Revision:	1.3
//  Last Mod:	10:15:22, 03/10/00
//
//  Description:
//	
//	This file contains the decompressHeader routine which is responsible
//  for reading the H261 frame header prior to actually decompressing
//  the frame.  
//	
//	
//  MT-level:  <??????>
//	
//------------------------------------------------------------------------
//	COPYRIGHT
//------------------------------------------------------------------------
#pragma ident	"@(#)decompressHeader.cc	1.3\t00/03/10  "

#include "XilDeviceCompressionH261.hh"
#include "H261DecompressorData.hh"

XilStatus	
XilDeviceCompressionH261::decompressHeader(void)
{
    Xil_unsigned8	*bp, byte, *endOfBuffer;
    int			offset;
    unsigned int	*wp, bits, morebits, info;

    //
    // We need to clear any outstanding compressions because we must read the
    // header information here.
    //
    // In XIL 1.3, we don't.
    //
    //getCis()->sync();

    
    // bp is the byte pointer used to access the H261 bytestream; get its
    // initial value from the CIS buffer mgr
    if ((bp = (Xil_unsigned8 *)cbm->nextFrame(&endOfBuffer)) == NULL) { //No data
        XIL_ERROR(system_state, XIL_ERROR_SYSTEM, "di-100", TRUE);
	return XIL_FAILURE;
	}
    
    while ((byte = *bp) == 0) {
      if(bp++ == endOfBuffer) {		// get byte containing the "1"
        XIL_ERROR(system_state,XIL_ERROR_SYSTEM, "di-100", TRUE);
        return XIL_FAILURE;
      }
    }
    wp = (unsigned int *)((int)bp & 0xfffffffc);
    bits = wp[0];			// get word containing the "1"
    UNSCRAMBLE(bits)

    offset = 7;
    while (!((0x1 << offset) & byte))
	offset--;			// get offset (in byte) of "1" bit
    offset += 8*(3-((int)bp & 0x3))-5;	// get offset (in word) of TR

    if (offset > 7)			// all info present
	info = (bits >> (offset-8)) & 0x1ff;
    else {				// need more bits
	morebits = wp[1];
	UNSCRAMBLE(morebits)
	if (offset < 0)			// all info in 2nd word
	    info = (morebits >> (24+offset)) & 0x1ff;
	else				// info straddles words
	    switch (offset) {
		case 0: info = ((bits & 0x01) << 8) | (morebits >> 24); break;
		case 1: info = ((bits & 0x03) << 7) | (morebits >> 25); break;
		case 2: info = ((bits & 0x07) << 6) | (morebits >> 26); break;
		case 3: info = ((bits & 0x0f) << 5) | (morebits >> 27); break;
		case 4: info = ((bits & 0x1f) << 4) | (morebits >> 28); break;
		case 5: info = ((bits & 0x3f) << 3) | (morebits >> 29); break;
		case 6: info = ((bits & 0x7f) << 2) | (morebits >> 30); break;
		case 7: info = ((bits & 0xff) << 1) | (morebits >> 31); break;
		}
	}
    decompData.temporalReference = info >> 4;
    decompData.splitScreen = info & 0x8;
    decompData.documentCamera = info & 0x4;
    decompData.freezeFrame = info & 0x2;
    decompData.isCif = info & 0x1;
    if (decompData.getCisSize() == -1) {
	decompData.setCisSize(decompData.isCif ? CIS_CIF_SIZE : CIS_QCIF_SIZE);
    }

    return XIL_SUCCESS;
}
