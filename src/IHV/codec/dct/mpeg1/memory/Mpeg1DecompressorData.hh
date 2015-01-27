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

//This line lets emacs recognize this as -*- C++ -*- Code
//------------------------------------------------------------------------
//
//  File:       Mpeg1DecompressorData.hh
//  Project:    XIL
//  Revision:   1.2
//  Last Mod:   10:23:01, 03/10/00
//
//  Description:
//
//    Heade for the Mpeg1DecompressorData Internal Class
//
//
//------------------------------------------------------------------------
//    COPYRIGHT
//------------------------------------------------------------------------
#pragma ident   "@(#)Mpeg1DecompressorData.hh	1.2\t00/03/10  "

#ifndef MPEG1DECOMPRESSORDATA_HH
#define MPEG1DECOMPRESSORDATA_HH

#include <xil/xilGPI.hh>

#if 0
#include "IdctImager.hh"
#include "IdctMisc.hh"
#endif
#include "Mpeg1Decoder.hh"
#include "Mpeg1Splatter.hh"
#include "IdctRefFrame.hh"

//
// Data in 'current' array
//
const int DC_LUMA       = 0;  // DC Luminance offset (always 1024)
const int DC_Cb         = 1;  // DC Cb chrominance offset (always 1024)
const int DC_Cr         = 2;  // DC Cr chrominance offset (always 1024)
const int C_MBA         = 3;
const int L_MBA         = 4;
const int C_Q           = 5;
 
const int F_COUNT       = 10; // Total pixels
const int F_HORZ        = 11; // Pixel width
const int F_VERT        = 12; // Pixel height
const int F_TBLOCK      = 13; // Total blocks in image(or slice ?)
const int F_HBLOCK      = 14; // Macroblocks per image width
const int F_PERSEC      = 15; // Bit Rate
 
const int F_F_FPV       = 20; // Full Pel Forward Vector (0 or 1)
const int F_F_F_CODE    = 21; // Forward F code
const int F_B_FPV       = 22; // Full Pel Backward Vector (0 or 1)
const int F_B_F_CODE    = 23; // Backward F code

struct SC_state {
    Xil_boolean        non_zero;
    int                zeros;
};

// for CBM types
const int NON_MPEG1_FRAME_TYPE = 16;
const int MPEG1_NONBFRAME_TYPE = 2;

// for user_data types
enum Mpeg1FrameType {
    MPEG1_IFRAME_TYPE = 1,
    MPEG1_PFRAME_TYPE = 2,
    MPEG1_BFRAME_TYPE = 3,
    MPEG1_DFRAME_TYPE = 4
};

const int SEQ_SC  = 0xB3;
const int SEQ_END = 0xB7;
const int GOP_SC  = 0xB8;
const int PIC_SC  = 0x00;

const int SEQ_ST_CODE      = (0x000001b3); // Sequence start code
const int GOP_ST_CODE      = (0x000001b8); // Group of pics start code
const int USR_ST_CODE      = (0x000001b2); // User data start code
const int EXT_ST_CODE      = (0x000001b5); // Extension start code
const int PIC_ST_CODE      = (0x00000100); // Picture start code
const int END_ST_CODE      = (0x000001b7); // Sequence end code

enum Mpeg1AdjustType {
    NO_ADJUST,
    PREV_NONBFRAME,
    SEQUENCE_END
};

const int DISPLAY_ID_TBD = -1;

const int MAX_TEMP_REF   = 1024;

struct mpeg_user_data_t {
    int              display_id;
    Xil_unsigned16   temp_ref;
    Xil_unsigned8    frame_type;
    int              time_code;
    Xil_unsigned8    closed_gop;
    Xil_unsigned8    broken_link;
    Xil_unsigned8    aspect_ratio;
    Xil_unsigned8    picture_rate;
};

class Mpeg1DecompressorData : public Mpeg1Splatter, public Mpeg1Decoder {
public:
    int getDitheringFlag() 
        {return doDither; }

    void setDitheringFlag(int value) 
    {
        doDither = value; 
        if(value == 0) {
            validDitherState = 0;
        }
    }

#if 0 // Disable this for now

    void setValidDitherState() 
    { 
        validDitherState = 1;
        decoderValidCnt = imager.getImagerValidCnt(); 
    }


    //
    // TODO: Fix me to handle XIL image destinations also.
    // Need to check image version number to see if it what we saw last time
    // plus "number of ops in the molecule" (which is, of course not a const)
    //
    int  isValidDitherState()   
    { 
        return (validDitherState &&
                imager.getImagerValidCnt() == decoderValidCnt &&
                (!(imager.usingXil()))) ; 
    }

#endif

    void color_video_frame();
    void doIntra(MacroBlock* result, int quant);
    void doInter(int code, MacroBlock* result, int quant);
    void doSscB(int slice);
    void doSscI(int slice);
    void doSscP(int slice);
    void initIntraTable();
    void initNonintraTable();
    void deleteFrames();

    int  doSeq(int* ret_width, int* ret_height,
               int* ret_aspect_ratio, int* ret_picture_rate);
    int  doUsr();
    int  doExt();
    int  doGop();
    int  doPic();
    void doAdd(int cbp, int type, short* tmbf, short* tmbb,
               short* tmbd, short* tmbr);


    //
    //  Decompressor Supporting Member Functions
    //
    Xil_unsigned8* findSC(Xil_unsigned8* ptr, 
                          Xil_unsigned8* end, 
                          SC_state*      state);

    int            findNextFrameBoundary();
    void           seek(int framenumber);
    Xil_boolean    scanForward(int count, int framenumber);

    Xil_unsigned8* establishFrameInfo(Xil_unsigned8*     start, 
                                      Xil_unsigned8*     end,
                                      mpeg_user_data_t** data_ptr, 
                                      Mpeg1AdjustType*    adjust_id);

    int            getOutputType(unsigned int* width, 
                                 unsigned int* height, 
                                 unsigned int* nbands,
                                 XilDataType*  datatype);

    int            burnFrames(int nframes);
    int            checkHistoryBuffers();  
    void           outputLastBuffer();  
    int            establishLastBuffer(int curr, int burn_start, int next);
    Xil_boolean    hasFrame();
    void           addEos(int frame_id);

    // Constructor
    Mpeg1DecompressorData();
    ~Mpeg1DecompressorData();
    void           reset();
    void           resetDecompressorData();
    int            allocOk()    {return (isok && splatterOk);}
    //
    // Some useful inline functions for use by decompressor routines
    //

    //
    // Set the bytstream pointer.
    //
    int            setByteStreamPtr() 
    {
        return(((rdptr= cbm->nextFrame(&endOfBuffer,TRUE))!=NULL));
    }

    int            setByteStreamPtrIfHasFrame() 
    {
        return(((rdptr= cbm->nextFrame(&endOfBuffer,TRUE))!=NULL) &&
           (hasFrame()==TRUE));
    }

    //
    // Finish processing the bytestream after decompressing a frame
    //
    void           finishDecode() 
    {
        cbm->decompressedFrame(rdptr,cbm->getRFrameType());
    }

    void           SetWidthHeightData(int horz, int vert);

    //
    // Data members
    //
    XilCisBufferManager* cbm ;
    int                  version;
    Xil_boolean          isok;
    Xil_unsigned32       decoderValidCnt;
    Xil_boolean          validDitherState;

    //
    // Controls whether decompress routines dither into a buffer.
    // Turn off if not executing molecule
    //
    Xil_boolean          doDither;

    // display_id for base of group (used with temp_ref)
    int                  group_base;        

    // next frame # to scan in fwd dir
    int                  next_scan_id;      

    // next frame # to assign as display id order
    int                  curr_display_id;   

    // last P or I frame with undetermined display id
    int                  prev_nonbframe_id; 

    // last subgroup start frame # scanned in forward direction
    int                  subgroup_id;       

    //
    // Compressor attribute variables
    //
    Xil_unsigned32       aspectRatioC;
    Xil_unsigned32       pictureRateC;
    Xil_unsigned32       timeCodeC;
    XilMpeg1Pattern      compressionPattern;
    Xil_unsigned32       bitsPerSecond;
    Xil_unsigned8        intraQTable[64];    // in zig-zag order
    Xil_unsigned8        nonIntraQTable[64]; // in zig-zag order
    Xil_unsigned8*       intraQptr;
    Xil_unsigned8*       nonIntraQptr;
    Xil_unsigned32       slicesPerPicture;
    Xil_boolean          insertSequenceEnd;

    //
    // Decompressor attribute variables
    //
    Xil_unsigned32       decompressionQuality;
    Xil_unsigned32       aspectRatioD;
    Xil_unsigned32       pictureRateD;
    Xil_unsigned32       timeCodeD;
    Xil_boolean          closedGop;
    Xil_boolean          brokenLink;
    Xil_unsigned32       temporalReference;
    Xil_unsigned32       frameType;



    Xil_unsigned8        quantintra[64];
    Xil_unsigned8        quantnonin[64];
    int                  modulo[8][64];
    int                  current[32];

    Mpeg1ReferenceFrame  Copy0;
    Mpeg1ReferenceFrame  Copy1;
    Mpeg1ReferenceFrame  CopyB;
    Mpeg1ReferenceFrame* last;
    Mpeg1ReferenceFrame* bbbb;
    Mpeg1ReferenceFrame* next;
    int                  frametypeC; // TODO:  temporary hack
    int                  verbose;

    //
    // Class variables for tracking display id during scanForward
    //
    int                  time_code;
    int                  closed_gop;
    int                  broken_link;
    int                  aspect_ratio;
    int                  picture_rate;

    //
    // B frame decode Quality attribute
    //
    int                  bframequality;

    //
    // Imager object.  Maybe could be a reference someday instead of
    // static allocation of object.
    //

    // TODO: Re-enable this when imager is implemented
    // XiliIdctImager      imager;

    static Xil_unsigned8 quantIntraInit[64];

};

#endif // MPEG1DECOMPRESSORDATA_HH



