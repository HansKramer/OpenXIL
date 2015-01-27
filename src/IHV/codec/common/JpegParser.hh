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
//  File:       JpegParser.hh
//  Project:    XIL
//  Revision:   1.4
//  Last Mod:   10:23:43, 03/10/00
//
//  Description:
//
//    The JpegParser class is one of the parent classes of JpegDecompressorData
//    and also the parent of JPEGLLdecompressor.  This class came about, when 
//    the merge of the JpegLLParse.cc and JpegParse.cc files.  Many of the
//    functions in this file repeated in both Jpeg and JpegLL codecs.  This
//    class takes care of all the bitstream parsing for jpeg, and it also 
//    generates the fast decode tables for Huffman decoding.
//
//
//------------------------------------------------------------------------
//    COPYRIGHT
//------------------------------------------------------------------------
#pragma ident   "@(#)JpegParser.hh	1.4\t00/03/10  "



#ifndef JPEGPARSER
#define JPEGPARSER

#include "xil/xilGPI.hh"
#include "Huffman_Code.hh"

#define JPEG_ERROR(category, id, primary, read_invalid, ptr) \
    cbm->byteError(ptr);                                 \
    XIL_CIS_ERROR(category, id, primary,                 \
                  cbm->getXilDeviceCompression(), \
                  read_invalid, FALSE);

#define MAX_DECODER_TABLES   32
#define QUANT_TABLES                4
#define QUANTIZER_VALUES        64
                  

enum jpeg_mode {  JPEG,  JPEGLL };
                  
typedef struct {
    Xil_unsigned8 id;                // component id (from bitstream)
    Xil_unsigned8 h, v, p;        // h,v: horizontal and vertical sampling
    // factors (from bitstream)
    // p: # blocks of this band per MCU
    Xil_unsigned8 qindex;                // quantization table id
    int* dctable;                        // Huffman table for decodeing DC
    int* actable;                        // Huffman table for decodeing AC
    int* table;                   //  Huffman table for JpegLL
    int history;                        // last DC coefficient value
} banddata;

typedef struct {

    int* decoder[MAX_DECODER_TABLES]; // Fast huffman decode tables
    int width, height;                    // w,h of the image
    int bands;                            // # bands in the image
    int precision;                    // bit precision of all components
    int maxh, maxv;                    // maximum horizontal and vertical
    // sampling rates for current frame
    int scanbands;                    // # bands in the current scan
    Xil_unsigned8 scancomponents[4];  // component ids for current scan
    int scan_pt_transform;     // pt transform for current scan
    int scan_selection;           // selection of predictor for current scan
    int restartInterval;                    // Number of MCU in restart interval
    // 0 means restart interval disabled
    banddata* band_data;                    // Variable length Array of banddata
    // length == # bands
} Header;
             

class JpegParser {
public:

    // Constructor / Destructor
    JpegParser(jpeg_mode mode);
    ~JpegParser();

    //
    // Public methods
    //
    Xil_boolean          isOK();
    void                 reset();
    int                  readtoscan();
    void                 free_decode_table(int *decode_table);
    void                 useBufferManager(XilCisBufferManager* bmanager) 
                         { cbm = bmanager; }

    void                 burnFrames(int nframes);
    int                  getOutputType(unsigned int* width, 
                                       unsigned int* height, 
                                       unsigned int* nbands, 
                                       XilDataType*  datatype);
    void                 CheckSeekable();


    //
    // Public data
    //
    banddata             bandInformation;
    Header               header;
    XilCisBufferManager* cbm;
    Xil_unsigned8*       rdptr;
    short                quantizer[QUANT_TABLES][QUANTIZER_VALUES];
    Xil_unsigned8*       endOfBuffer;

    Xil_unsigned32       definedInFrame;
    Xil_unsigned32       reDefinedInCis;
    Xil_boolean          notSeekable;


private:
    jpeg_mode   decode_mode;
    Xil_boolean isOKFlag;
    static int* create_decode_table(Huffman_Code* huffman, int size);

};

inline void JpegParser::CheckSeekable()
{
    if(cbm->getRFrameId() == 1)  {
        reDefinedInCis = definedInFrame;
    } else if(reDefinedInCis != definedInFrame && cbm->getRFrameId() > 1) {
        notSeekable = TRUE;
    }
}

#endif   //  JPEGPARSER



