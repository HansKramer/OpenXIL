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
//  File:       H261DecompressorData.hh
//  Project:    XIL
//  Revision:   1.7
//  Last Mod:   10:23:13, 03/10/00
//
//  Description:
//
//
//
//------------------------------------------------------------------------
//    COPYRIGHT
//------------------------------------------------------------------------
#pragma ident   "@(#)H261DecompressorData.hh	1.7\t00/03/10  "

#ifndef H261DECOMPRESSORDATA_H
#define H261DECOMPRESSORDATA_H

#include "xil/xilGPI.hh"
#include "H261Decoder.hh"
#include "H261Splatter.hh"
#include "DecompressInfo.hh"
#include "DctBlockUtils.hh"

#define CIS_CIF_SIZE	1
#define CIS_QCIF_SIZE	0

#ifdef XIL_LITTLE_ENDIAN
#define UNSCRAMBLE(word) 				\
		word = (word << 24) | 			\
		       ((word & 0xff00) << 8) | 	\
		       ((word >> 8) & 0xff00) | 	\
		       (word >> 24);
#else
#define UNSCRAMBLE(word)
#endif

#define GETWORD(word, ptr)				\
		word = *ptr++;  UNSCRAMBLE(word)

class H261DecompressorData : public H261Splatter, public H261Decoder {
  
private:
    //
    // CIF size or QCIF size
    //
    int			cisSize;

    //
    // Pointer to the space allocated to hold the internal history buffers>
    //
    double* buffer_space;

    Xil_boolean validDitherState;

public:
    XilCisBufferManager* cbm ;

    //
    // Create a block handling object.
    // This includes internal storage for one macroblock
    // in both 16 bit and 8 bit depths
    //
    BlockMan blok;

    int isok;
    int version;
    Xil_unsigned32 decoderValidCnt;
    int doDither;	// Controls whether or not decompress routines dither
			// results into a buffer.  Turn this off if we are
			// not executing a display molecule.

    int		temporalReference;
    Xil_boolean	splitScreen;
    Xil_boolean	documentCamera;
    Xil_boolean	freezeFrame;
    Xil_boolean	isCif;
    Xil_boolean	ignoreHistory;


    //
    // Pointers to current image history raster
    //
    Xil_unsigned8* ycptr;
    Xil_unsigned8* ucptr;
    Xil_unsigned8* vcptr;

    //
    // Pointers to previous image history raster
    //
    Xil_unsigned8* ypptr;
    Xil_unsigned8* upptr;
    Xil_unsigned8* vpptr;

    //
    // Pointer to 24 bit internal image buffer (CIF size)
    // (for results of color conversion)
    //
    Xil_unsigned8* buf24;

    //
    // Pointer to 8 bit internal image buffer (CIF size)
    // (for results of dither operation)
    //
    Xil_unsigned8* buf8;


    // frame id of curr and prev blocks
    // allows us to check for re-decompress last frame
    int    curr_id, prev_id, frame_id;

  // before next decompress, swap curr and prev block pointers
  void swapCurrPrevBlockPtrs();
  
  // Imager object.  Maybe could be a reference someday instead of
  // static allocation of object.
  //  XiliIdctImager imager;
  

  int getDitheringFlag()		{return doDither; }
  Xil_boolean getIgnoreHistoryFlag()    {return ignoreHistory; }

  void setDitheringFlag(int value)	{
					 doDither = value; 
					 if (value == 0)
					    validDitherState = 0;
					}

  void color_video_frame(DecompressInfo* di);
  void doCBP(short cbp, Xil_signed16 *blsh[6], short mbType, int mquant);
  void catchup(DecompressInfo* di, int skip_blocks,
               int start_block,int end_block,int basey,int basex,int width);
  
  void doInter(int code, Xil_signed16* result[6], int quant);
  void doIntra(Xil_signed16* result[6], int quant);
  
  //
  //  Decompressor Supporting Member Functions
  //
//  void burnFrames(int nframes, DecompressInfo* di);
  void burnFrames(int nframes);
  void skipFrames(int nframes);
  int  findNextFrameBoundary();
  Xil_unsigned8* findPSC();
  Xil_unsigned8* findNextPSC();
  int getCisSize()			{return cisSize; }
  void setCisSize(int value);
  void adjustRdptr();
  int syncPSC();
  int getOutputType(int* width, int* height, int* nbands,
		      XilDataType* datatype);

  void useBufferManager(XilCisBufferManager* bmanager) { cbm = bmanager; }
  
  //
  // Constructor / Destructor
  //
  H261DecompressorData();
  ~H261DecompressorData();

  void  reset();
  int allocOk()    {return isok; }

  //
  // Some useful inline functions for use by decompressor routines
  //
  
  //
  // Set the bytstream pointer.
  //
  int setByteStreamPtr() {
     rdptr= cbm->nextFrame(&endOfBuffer);
     frame_id = cbm->getRFrameId();
     return(rdptr!=NULL);
  }
  
  //
  // Finish processing the bytestream after decompressing a frame
  //
  void finishDecode() {
    cbm->decompressedFrame(rdptr);
  }
  
};


#endif /* H261DECOMPRESSORDATA_H */
