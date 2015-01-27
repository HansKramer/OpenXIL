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
//  File:       JpegHuffmanDecoderBase.hh
//  Project:    XIL
//  Revision:   1.2
//  Last Mod:   10:23:51, 03/10/00
//
//  Description:
//
//    Class Declaration for JpegHuffmanDecoderBase.
//    This contains the functions common to Jpeg and JpegLL.
//
//
//------------------------------------------------------------------------
//    COPYRIGHT
//------------------------------------------------------------------------
#pragma ident   "@(#)JpegHuffmanDecoderBase.hh	1.2\t00/03/10  "


#ifndef JPEGHUFFMANDECODERBASE
#define JPEGHUFFMANDECODERBASE
#include "xil/xilGPI.hh"
#include "JpegMacros.hh"

//   .... the function reads a byte from the incoming variable-length
//  bit-stream, if a marker word is encountered (byte-aligned 0xff)
//  then it steps to the next byte in stream to check for a padding
//  value (byte-aligned 0x00) indicating a NULL marker.  If a 0x00 is
//  found, it is skipped.

#define MAX_DIFF_ENCODED -32768
#define GETBYTE_INIT        \
    register Xil_unsigned8 *dptr = *ppstream;

#define GETBYTE(rval) {  \
    if (dptr >= pend)                        \
        goto ErrorReturn;                \
    if ((rval = *dptr) == MARKER) {           \
        if (dptr[1] == 0)   \
            dptr += 2;             \
    } else    \
        dptr++;   \
}
#define GETBYTE_END        \
    *ppstream = dptr;

#define GETANYBYTE(rval) { \
    if (*ppstream >= pend)                        \
        goto ErrorReturn;                \
    rval = *(*ppstream)++;                \
    }
#define BACKUPBYTE()                {(*ppstream)--;}

//  .... the following macro reads the next 8-bits from
//  the input stream and returns them in rval, (Note:
//  lastbyte my get as many as 15 leftover bits) ....

//  rval is AND'd with 0xff because we left the old index value (which was used
//  in the initial decode lookup) in lastbyte.  it will be removed later.

#define GETINDEX(rval)   \
{   \
    if (leftover < 8) {   \
        GETBYTE(rval);  \
        lastbyte = (lastbyte << 8) | rval;  \
    } else {   \
        leftover -= 8;  \
    }  \
    rval = (lastbyte >> leftover) & 0xff;  \
    }

class JpegHuffmanDecoderBase {
public:  
  // Constructor
  JpegHuffmanDecoderBase() { initdecode(); }

  void initdecode();

  void finishdecode(Xil_unsigned8**, Xil_unsigned8* pend);

  void finishRstInterval(Xil_unsigned8**, Xil_unsigned8* pend);
  
protected:

  //
  // Huffman decoding state
  //
  
  int bitsleftover[2];

};

#endif  // JPEGHUFFMANDECODERBASE
