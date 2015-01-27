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
//  File:       JpegLL_Huffman_Encoder.hh
//  Project:    XIL
//  Revision:   1.3
//  Last Mod:   10:23:05, 03/10/00
//
//  Description:
//
//   Definition of JpegLL_Huffman_Encoder Class
//
//   This class, derived off of the Jpeg_Huffman_Encoder_Base base class,
//   defines the encoding and output routines specific to Jpeg lossless.
//
//------------------------------------------------------------------------
//    COPYRIGHT
//------------------------------------------------------------------------
#pragma ident   "@(#)JpegLL_Huffman_Encoder.hh	1.3\t00/03/10  "


#ifndef JPEGLL_HUFFMAN_ENCODER_H
#define JPEGLL_HUFFMAN_ENCODER_H

#include "Jpeg_Huffman_Encoder_Base.hh"
#include "JpegBandInfo.hh"

class JpegLL_Huffman_Encoder : public Jpeg_Huffman_Encoder_Base {
public:
    JpegLL_Huffman_Encoder(unsigned int  nt, 
                           unsigned int  nc,
                           Xil_unsigned8 m = 0, 
                           SingleBuffer* buf = NULL);

    ~JpegLL_Huffman_Encoder();

    void Encode_ll(Xil_signed16* diff, unsigned int width, unsigned int nbands,
                   JpegBandInfo* banddata);

    void Reset();
};

#endif // JPEGLL_HUFFMAN_ENCODER_H

