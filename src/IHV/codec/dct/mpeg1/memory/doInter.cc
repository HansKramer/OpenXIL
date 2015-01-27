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
//  File:       doInter.cc
//  Project:    XIL
//  Revision:   1.2
//  Last Mod:   10:14:53, 03/10/00
//
//  Description:
//
//    Reconstruct an Mpeg1 Inter-frame coded Macroblock
//
//
//------------------------------------------------------------------------
//    COPYRIGHT
//------------------------------------------------------------------------
#pragma ident   "@(#)doInter.cc	1.2\t00/03/10  "

#include "Mpeg1DecompressorData.hh"

void Mpeg1DecompressorData::doInter(int         code, 
                                    MacroBlock* result, 
                                    int         quant)
{
    int    room[65];
    int    valu;
    int    zero;
    int    indx;
    int    word;
    int    ilast;

    //
    // Scan through each bit of the Coded Block Pattern (cbp)
    //
    int* rptr = (int*)(result->block88);
    int* mtx  = room;
    for(int block=32; block>0; block >>= 1) {
        //
        // Process this 8x8 block if its CBP bit is set
        //
        if((code & block) != 0) {
            indx = -1;
            ilast = 0;

            //
            // .... there are two encodings of the 0-length-run-
            // level-1 case depending on whether the coefficient
            // is the first one in a block; so, we initialize the
            // huffman        decode table to expect the shorter [0,1]
            // symbol first ....
            //
            word = decode(ACF_HuffTable);

            //
            // .... for subsequent coefficients we expect the
            // longer [0,1] symbol ....
            //

            while(1) {
                if((word & 0x8000) == 0) {
                    zero = word >> 8;
                    valu = (word << 24) >> 16;
                } else if(word == ESCAPE) {
                    GETBITS(6, zero);
                    GETBITS(8, valu);
                    if(valu & 0x7f) {
                        valu = (valu << 24) >> 16;
                    } else if(valu == 0x00) {
                        GETBITS(8, valu);
                        valu <<= 8;
                    } else if(valu == 0x80) {
                        GETBITS(8, valu);
                        valu = (valu - 256) << 8;
                    }
                } else if(word == EOBP) {
                    // End of Macroblock 
                    break;
                } else { // (decoded_coeff == ILLEGAL_CODE)
                    return;
                }

                if(ilast > 63) {
                    return;
                }
                indx = indx + 1 + zero;
                mtx[ilast++] = valu + indx;
                word = decode(ACR_HuffTable);
            }

            if(ilast > 0) {
                mtx[ilast] = 0;
                dequantize(quant, mtx, quantnonin, 1);
                idct(rptr, mtx);
            } else {
                int* iptr = rptr;
                for(indx= 0; indx<8; indx++) {
                    iptr[0] = 0;
                    iptr[1] = 0;
                    iptr[2] = 0;
                    iptr[3] = 0;
                    iptr += 4;
                }
            }
        }
        rptr += 32;
    }

  //
  // Escape point for errors from GETBITS
  //
  ErrorReturn:
    return;
}
