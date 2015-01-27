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
//  File:       JpegDecompressorData.hh
//  Project:    XIL
//  Revision:   1.9
//  Last Mod:   10:22:53, 03/10/00
//
//  Description:
//
//    Header for object containing info
//    specific to this decompresssion
//
//
//------------------------------------------------------------------------
//    COPYRIGHT
//------------------------------------------------------------------------
#pragma ident   "@(#)JpegDecompressorData.hh	1.9\t00/03/10  "

#ifndef JPEGDECOMPRESSORDATA_H
#define JPEGDECOMPRESSORDATA_H

#include <xil/xilGPI.hh>

#include "IdctSplatter.hh"
#include "JpegHuffmanDecoder.hh"
#include "JpegParser.hh"

class JpegDecompressorData : public IdctSplatter, public JpegParser {
public:
    //
    // Constructor/Destructor
    //
              JpegDecompressorData();
              ~JpegDecompressorData();

    void      reset();

    int       decode8x8(Xil_signed16* tile,
                        int           dcpred,
                        int           qindex,
                        int*          dctable,
                        int*          actable);

    //
    //  Decompressor Supporting Member Functions
    //
    XilStatus findNextFrameBoundary();

    void      setmaxkount(int count);

    int       allocOk();

    //
    // Set the bytstream pointer and Parse the header.
    // Only do this if some other decompress module has not already
    // done it (and then failed because the bytestream couldn't be
    // decompressed by that module, e.g. an optimized path failed)
    //
    int       setByteStreamPtr();

    int       parseByteStreamHeader();

    //
    // Finish processing the bytestream after decompressing a frame
    //
    void      finishDecode();

    void      doRestart();

    int       is411Frame();
    int       is422Frame();

    //
    // Instance variables
    //
    int       maxkount;   // Max number of AC coeffs to process in each IDCT
    int       already_readtoscan;
    int       version;
    int       seekFrameType;
    int       ignoreHistory;
    int       historyIgnored;

    //
    // Huffman Decoder object - allocate on stack as part of this object
    //
    JpegHuffmanDecoder huffmandecoder;

};

#endif // JPEGDECOMPRESSORDATA_H
