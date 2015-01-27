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
//  File:       JpegHuffmanDecoderBase.cc
//  Project:    XIL
//  Revision:   1.2
//  Last Mod:   10:16:19, 03/10/00
//
//  Description:
//
//    Jpeg Bitstream decoding
//    Functions common to Jpeg and JpegLL
//
//
//------------------------------------------------------------------------
//    COPYRIGHT
//------------------------------------------------------------------------
#pragma ident   "@(#)JpegHuffmanDecoderBase.cc	1.2\t00/03/10  "

#include "JpegHuffmanDecoderBase.hh"
 

// Reset Huffman internal decoding state
void  
JpegHuffmanDecoderBase::initdecode()
{
     int *bptr = bitsleftover;
     
     bptr[0] = 0;
     bptr[1] = 0;
}


void  
JpegHuffmanDecoderBase::finishdecode(Xil_unsigned8** ppstream,
                                     Xil_unsigned8*  pend)
{
    int tmp;

    // Look for EndOfImage marker and gobble it up
    GETANYBYTE(tmp);
    do {
        if (tmp == MARKER) {
            GETANYBYTE(tmp);
            if (tmp == EOI) {
                break;
            }
        } else {
            GETANYBYTE(tmp);
        }
    } while (1);

ErrorReturn:
    return;

}

void  
JpegHuffmanDecoderBase::finishRstInterval(Xil_unsigned8** ppstream,
                                          Xil_unsigned8*  pend)
{
    int        tmp;

    // Look for Restart marker and gobble it up
    // We either have MARKER [MARKER]* RST?, which we want to gobble,
    // or we have something else which we don't want to process
    //
    GETANYBYTE(tmp);
    if (tmp == MARKER) {
        int count = 0;
        while (tmp == MARKER) {
            count++;
            GETANYBYTE(tmp);
        }
        if (tmp >= RST(0) && tmp <= RST(7))
            return;
        else {
            for (int i=0; i < count; i++) {
                BACKUPBYTE();
            }
        }
    }

    BACKUPBYTE();

ErrorReturn:
    return;

}

