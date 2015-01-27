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
//  File:       doIntra.cc
//  Project:    XIL
//  Revision:   1.2
//  Last Mod:   10:14:49, 03/10/00
//
//  Description:
//
//    Reconstruct an Intra-coded Macroblock
//
//
//------------------------------------------------------------------------
//    COPYRIGHT
//------------------------------------------------------------------------
#pragma ident   "@(#)doIntra.cc	1.2\t00/03/10  "

#include "Mpeg1DecompressorData.hh"

void 
Mpeg1DecompressorData::doIntra(MacroBlock* result, int quant)
{
    int  room[65];
    int* mtx;
    int  indx;
    int  block;
    int  word;
    int  valu;
    int  zero;
    int  which;
    int  ilast;
    int* hptr;

    //
    // Establish an integer ptr to the block of doubles
    //
    int* dct_blk_ptr = (int*)(result->block88);

    hptr  = DCL_HuffTable;
    which = DC_LUMA;
    for(block=0; block<6; block++) {
        if(block > 3) {
            if(block == 4) {
                hptr = DCC_HuffTable;
                which = DC_Cb;
            } else {
                which = DC_Cr;
            }
        }

        mtx = room;
        word = decode(hptr);
        if(word > 0) {
            GETBITS(word,valu);
            if((valu & (1 << (word - 1))) == 0) {
                valu = (valu + 1) - (1 << word);
            }
            valu = valu * 8 + current[which];
            current[which] = valu;
        } else {
            valu = current[which];
        }

        mtx[0] = valu << 8;

        indx = 0;
        ilast = 1;
        while(1) {
            word = decode(ACR_HuffTable);

            if(!(word & 0x8000)) {
                zero = word >> 8;
                valu = (word << 24) >> 16;
            } else if(word == ESCAPE) {
                GETBITS(6,zero);
                GETBITS(8,valu);
                if(valu & 0x7f) {
                    valu = (valu << 24) >> 16;
                } else if(valu == 0x00) {
                    GETBITS(8,valu);
                    valu <<= 8;
                } else if(valu == 0x80) {
                    GETBITS(8,valu);
                    valu = (valu - 256) << 8;
                }
                //
                // TODO: There is no default "else" case above.
                //       What happens if 'valu' is not one of these 3 values
                //       (lperry)
                //
            } else if(word == EOBP) {
                break;
            } else { // (decoded_coeff == ILLEGAL_CODE)
                return;
            }

            //Check for index out-of-range!
            if(ilast > 63) {
                return;
            }

            indx = indx + 1 + zero;
            mtx[ilast++] = valu + indx;
        }
        mtx[ilast] = 0;
        dequantize(quant, mtx, quantintra, 0);
        idct(dct_blk_ptr, mtx);
        dct_blk_ptr += 32;
    }

  ErrorReturn:
    return;
}
