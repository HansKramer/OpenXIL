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
//  File:       JpegHuffmanDecoder.hh
//  Project:    XIL
//  Revision:   1.3
//  Last Mod:   10:22:47, 03/10/00
//
//  Description:
//
//      Class Declaration for JpegHuffmanDecoder, and macro 
//      definitions for fast decode algorithms.
//
//
//------------------------------------------------------------------------
//    COPYRIGHT
//------------------------------------------------------------------------
#pragma ident   "@(#)JpegHuffmanDecoder.hh	1.3\t00/03/10  "



#ifndef JPEGHUFFMANDECODER
#define JPEGHUFFMANDECODER

#include "JpegHuffmanDecoderBase.hh"

class JpegHuffmanDecoder : public JpegHuffmanDecoderBase {
public:  
  // Constructor
  JpegHuffmanDecoder() {};
  ~JpegHuffmanDecoder() {};

  int decode(Xil_unsigned8** ppstream, int *results,
		int *dctable, int* actable,
		Xil_unsigned8* pend, int* history);
};

#endif  // JPEGHUFFMANDECODER
