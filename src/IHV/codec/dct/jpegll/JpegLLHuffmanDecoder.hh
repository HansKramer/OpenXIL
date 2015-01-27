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
//  File:       JpegLLHuffmanDecoder.hh
//  Project:    XIL
//  Revision:   1.3
//  Last Mod:   10:23:02, 03/10/00
//
//  Description:
//
//    Header for the Lossless Variant of the Jpeg Huffman Decoder
//
//
//------------------------------------------------------------------------
//    COPYRIGHT
//------------------------------------------------------------------------
#pragma ident   "@(#)JpegLLHuffmanDecoder.hh	1.3\t00/03/10  "

#ifndef JPEGLLHUFFMANDECODER
#define JPEGLLHUFFMANDECODER

#include "JpegHuffmanDecoderBase.hh"

class JpegLLHuffmanDecoder : public JpegHuffmanDecoderBase {
public:  
    JpegLLHuffmanDecoder() {bitstreamPtr = NULL;}
    ~JpegLLHuffmanDecoder() {}

    void           decode_ll(Xil_unsigned8**, 
                             int*, Xil_signed16*,unsigned int,
                             Xil_unsigned8* pend);
  
    Xil_unsigned8* getBitstreamPtr();
    void           setBitstreamPtr(Xil_unsigned8* address);

private:
    Xil_unsigned8* bitstreamPtr;

};

#endif // JPEGLLHUFFMANDECODER
