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
//  File:       H261DecompressorData.cc
//  Project:    XIL
//  Revision:   1.6
//  Last Mod:   10:15:16, 03/10/00
//
//  Description:
//
//      Internal functions for H261 decompressor.
//      TODO: Incorporate these into the DeviceCompression
//    
//
//
//------------------------------------------------------------------------
//    COPYRIGHT
//------------------------------------------------------------------------
#pragma ident   "@(#)H261DecompressorData.cc	1.6\t00/03/10  "

#include <stdio.h>
#include "H261DecompressorData.hh"

void H261DecompressorData::reset()
{
    validDitherState = FALSE;
    decoderValidCnt = 0;
    version++;
    validDitherState = 0;
    rdptr = NULL;
    cisSize = -1;
    temporalReference = 0;
    splitScreen = FALSE;
    documentCamera = FALSE;
    freezeFrame = FALSE;
    isCif = TRUE;
    ignoreHistory = FALSE;
    curr_id = -1;
    prev_id = -1;
    frame_id = -1;
    H261Decoder::reset();
}

void H261DecompressorData::burnFrames(int nframes)
{
   for(int i=0; i<nframes; i++) {
     if ( !setByteStreamPtr()) {
       // We have a condition where 
       //   decompress: no frame from buffer mgr
       XIL_ERROR( NULL, XIL_ERROR_SYSTEM, "di-110", TRUE);
       return;
     }
    
     //
     // Execute the decompression without updating
     // the output (update_output flag is FALSE)
     //
     color_video_frame(NULL);

     finishDecode();
    
   }
   return;
}

void H261DecompressorData::skipFrames(int nframes)
{
   for(int i=0; i<nframes; i++) {
     if ( !setByteStreamPtr()) {
       // We have a condition where 
       //   decompress: no frame from buffer mgr
       XIL_ERROR( NULL, XIL_ERROR_SYSTEM, "di-110", TRUE);
       return;
     }
     rdptr = findNextPSC();
     finishDecode();
   }
   validDitherState = FALSE;
   return;
}

H261DecompressorData::H261DecompressorData()
{
    isok = 0;
    if (!(H261Decoder::allocOk())) {
      return;
    }  
    version = 0;
    reset();

    //
    // Declare enough space for two CIF-sized buffers for 411 YCbCr,
    // plus a 24 bit and an 8 bit intenal output image buffer (CIF size)
    // Make it 8 byte aligned
    //
    unsigned int cif_size = 352 * 288;
    buffer_space = new double[(2*cif_size*3/2 +
                               3*cif_size + 
                               cif_size)/sizeof(double)];

    if(buffer_space == NULL) {
        XIL_ERROR(NULL, XIL_ERROR_RESOURCE, "di-1", TRUE);
        return;
    }

    //
    // Set pointers to each of the buffers within the buffer space.
    //
    ycptr = (Xil_unsigned8*)buffer_space;
    ucptr = ycptr + cif_size;
    vcptr = ucptr + cif_size/4;
    ypptr = vcptr + cif_size/4;
    upptr = ypptr + cif_size;
    vpptr = upptr + cif_size/4;
    buf24 = vpptr + cif_size/4;
    buf8  = buf24 + cif_size*3;

    isok = 1;
}

H261DecompressorData::~H261DecompressorData()
{
    //
    // Delete the space for all the internal buffers.
    // The decode tables freed in H261Decoder destructor.
    //
    delete buffer_space;
}

void H261DecompressorData::adjustRdptr()
{
   // we only need 20 bits for PSC, and the rdptr must point to the
   // first complete byte of the PSC.  If there are 4 or more 
   // extra bits, we are 3 bytes past the first complete byte.
   // Else, we are 2 bytes past the first complete byte.
   if (nbits >= 4)
      rdptr -= 3;
    else
      rdptr -= 2; 
}

int
H261DecompressorData::getOutputType(int* width, int* height, int* nbands,
                                       XilDataType* datatype)
{
  Xil_unsigned8* pend;
  Xil_unsigned32 tempword;

  *nbands = 3;
  *datatype = XIL_BYTE;

  if (bitstreamWidth != 0) {
    *width = bitstreamWidth;
    *height = bitstreamHeight;

    return 1;
  }

// Careful about messing around with data pointer and saved
// bits.

  if ((rdptr = cbm->nextFrame(&pend))==NULL){
    return 0;
  }
  // Do this because endofBuffer is used by getBits
  endOfBuffer = pend;

  /*
   * Look for Picture Start Code.
   * sync up with the first "1" followed by 4 zeros,
   * Some of the leading "0" PSC bits may be part of the previous frame!
   * Then return unused bits to savedBits
   */
  if (syncPSC() == XIL_FAILURE) {
ErrorReturn:
    XIL_ERROR(NULL, XIL_ERROR_CIS_DATA,"di-314",TRUE);
    return 0;
  }

  // grab next 11 bits = temp reference (10:6) |  ptype bit vector (5:0)
  GETBITS(11,tempword);
  if (tempword & 0x04) {
      // CIF
      *width = 352; *height = 288;
      if (getCisSize() == -1)
	  setCisSize(CIS_CIF_SIZE);
  } else {
      // QCIF
      *width = 176; *height = 144;
      if (getCisSize() == -1)
	  setCisSize(CIS_QCIF_SIZE);
  }

  initParser();		// reset nbits, savedBits
  return 1;

}

void H261DecompressorData::setCisSize(int value)
{
    cisSize = value;

    if (value == CIS_CIF_SIZE) {
      bitstreamWidth = 352; bitstreamHeight = 288;
    } else {
      bitstreamWidth = 176; bitstreamHeight = 144;
    }
}


void H261DecompressorData::swapCurrPrevBlockPtrs()
{
  Xil_unsigned8	*tptr;

  // check before swap--no swap in case re-decompress this frame
  if ((curr_id == -1) || (curr_id != frame_id)) {
    tptr  = ycptr; ycptr = ypptr; ypptr = tptr;
    tptr  = ucptr; ucptr = upptr; upptr = tptr;
    tptr  = vcptr; vcptr = vpptr; vpptr = tptr;

    prev_id = curr_id;
    curr_id = frame_id;
  }
  return;
}


