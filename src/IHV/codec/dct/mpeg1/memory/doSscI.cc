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
//  File:       doSscI.cc
//  Project:    XIL
//  Revision:   1.2
//  Last Mod:   10:14:57, 03/10/00
//
//  Description:
//
//    Reconstruct a slice within an I frame
//
//
//------------------------------------------------------------------------
//    COPYRIGHT
//------------------------------------------------------------------------
#pragma ident   "@(#)doSscI.cc	1.2\t00/03/10  "

#include "Mpeg1DecompressorData.hh"

void Mpeg1DecompressorData::doSscI(int slice)
{
    int                done = 2;
    int                quant;
    int                j;
    int                mba,mbt;
    int                framecode;
    int                cmba;

    cmba = current[C_MBA];

    current[DC_LUMA] = 1024;
    current[DC_Cr]   = 1024;
    current[DC_Cb]   = 1024;

    GETBITS(5,quant);

    GETBITS(1,j);
    while(j == 1) {
        GETBITS(8,j);
        GETBITS(1,j);
    }

    while(done != 1) {
        mba = getMBA();
        if(mba == INVALID_DECODE_RETURN) {
            return;
        }
        if(done == 2) {
            cmba = (slice-1)*current[F_HBLOCK]-1 + mba;
            if(cmba >= current[F_TBLOCK]) {
                return;
            }
        } else {
            cmba += mba;
        }

        mbt = getMBTI();
        if(mbt == INVALID_DECODE_RETURN) {
            return;
        }
        framecode = (mbt>>16);
        mbt   = (mbt<<16)>>16;
        if(framecode & CODE_Q) {
            GETBITS(5,quant);
        }
        doIntra(next->getMacroBlock(cmba), quant);
        done = checkforstart();
    }

    current[C_MBA] = cmba;
    current[C_Q] = quant;

  ErrorReturn:
    return;
}
