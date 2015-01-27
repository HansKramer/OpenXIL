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
//  File:       JpegDecompressorData.cc
//  Project:    XIL
//  Revision:   1.6
//  Last Mod:   10:14:30, 03/10/00
//
//  Description:
//
//    Functions to manipulate this frame
//
//
//------------------------------------------------------------------------
//    COPYRIGHT
//------------------------------------------------------------------------
#pragma ident   "@(#)JpegDecompressorData.cc	1.6\t00/03/10  "


#include "JpegDecompressorData.hh"

void 
JpegDecompressorData::reset()
{
    //
    // Max number of AC coefficients to process in each IDCT
    //
    maxkount               = 63;

    already_readtoscan     = 0;
    header.restartInterval = 0;
    header.width           = 0;
    header.height          = 0;
    seekFrameType          = JPEG_FRAME_TYPE;
    ignoreHistory          = FALSE;
    historyIgnored         = FALSE;
    rdptr                  = NULL;

    version++;

    JpegParser::reset();

}

JpegDecompressorData::JpegDecompressorData() : JpegParser(JPEG)
{
    version = 0;
    reset();
}

JpegDecompressorData::~JpegDecompressorData()
{
    for(int i=0; i<MAX_DECODER_TABLES; i++) {
        if(header.decoder[i]) {
            free_decode_table(header.decoder[i]);
            header.decoder[i] = 0;
        }
    }

    if(header.band_data != NULL) {
        delete [] header.band_data;
        header.band_data = NULL;
    }

}

void 
JpegDecompressorData::setmaxkount(int count) 
{
    if(count < 0) {
        maxkount = 0;
    } else if(count > 63) {
        maxkount = 63;
    } else {
        maxkount = count;
    }
}

int 
JpegDecompressorData::allocOk()  
{
    return splatterOk;
}

//
// Set the bytstream pointer and Parse the header.
// Only do this if some other decompress module has not already
// done it (and then failed because the bytestream couldn't be
// decompressed by that module, e.g. an optimized path failed)
//
int 
JpegDecompressorData::setByteStreamPtr() 
{
    return(already_readtoscan || ((rdptr= cbm->nextFrame(&endOfBuffer))!=NULL));
}

int 
JpegDecompressorData::parseByteStreamHeader() 
{
    if(!already_readtoscan) {
        definedInFrame = 0;
        already_readtoscan = readtoscan();
    }

    return already_readtoscan;
}

//
// Finish processing the bytestream after decompressing a frame
//
void 
JpegDecompressorData::finishDecode() 
{
    CheckSeekable();
    huffmandecoder.finishdecode(&rdptr,endOfBuffer);
    cbm->decompressedFrame(rdptr,JPEG_FRAME_TYPE);
    already_readtoscan = 0;
}

void 
JpegDecompressorData::doRestart() 
{
    huffmandecoder.finishRstInterval(&rdptr,endOfBuffer);
    huffmandecoder.initdecode();
}

//
// Check to see if this image is 411 sampled.
// All 3 bands must be interleaved in first scan
// Ordering is Y then U then V
//
int 
JpegDecompressorData::is411Frame() 
{
    return (header.bands          == 3 &&
            header.scanbands      == 3 &&
            header.band_data[0].h == 2 &&
            header.band_data[0].v == 2 &&
            header.band_data[1].h == 1 &&
            header.band_data[1].v == 1 &&
            header.band_data[2].h == 1 &&
            header.band_data[2].v == 1);
}

//
// Check to see if this image is 422 sampled.
// All 3 bands must be interleaved in first scan
// Ordering is Y then U then V
//
int 
JpegDecompressorData::is422Frame() 
{
    return (header.bands          == 3 &&
            header.scanbands      == 3 &&
            header.band_data[0].h == 2 &&
            header.band_data[0].v == 1 &&
            header.band_data[1].h == 1 &&
            header.band_data[1].v == 1 &&
            header.band_data[2].h == 1 &&
            header.band_data[2].v == 1);
}

