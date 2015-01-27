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
//  File:       doSscP.cc
//  Project:    XIL
//  Revision:   1.2
//  Last Mod:   10:14:52, 03/10/00
//
//  Description:
//
//    Reconstruct a slice within a P frame
//
//
//------------------------------------------------------------------------
//    COPYRIGHT
//------------------------------------------------------------------------
#pragma ident   "@(#)doSscP.cc	1.2\t00/03/10  "

#include "Mpeg1DecompressorData.hh"
#include "XiliUtils.hh"

void 
Mpeg1DecompressorData::doSscP(int slice)
{
    double      mbroom[2*6*32];
    MacroBlock* tmbF = (MacroBlock*)&mbroom[000];
    MacroBlock* tmbD = (MacroBlock*)&mbroom[192];
    MacroBlock* tmbf;
    int         done = 2;
    int         quant;
    int         j;
    int         mba, mbt;
    int         mfh, mfv;
    int         ffpv;
    int         *ff;
    int         cbp;
    int         x, y;
    int         framecode;
    int         cmba, fhb;

    fhb  = current[F_HBLOCK];
    cmba = current[C_MBA];

    ffpv = current[F_F_FPV];
    ff   = modulo[current[F_F_F_CODE]];

    mfh = 0;
    mfv = 0;

    current[DC_LUMA] = 1024;
    current[DC_Cr]   = 1024;
    current[DC_Cb]   = 1024;

    GETBITS(5, quant);
    current[C_Q]    = quant;

    GETBITS(1, j);
    while(j == 1) {
        GETBITS(8, j);
        GETBITS(1, j);
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
            y = cmba / fhb;
            x = cmba - y*fhb;
        } else {
            if(mba > 1) {
                mfh = 0;
                mfv = 0;
                xili_memcpy(next->getMacroBlock(cmba+1),
                            last->getMacroBlock(cmba+1),
                            (mba-1)*sizeof(MacroBlock));
            }
            cmba += mba;
            x    += mba;
            while(x >= fhb) {
                x -= fhb;
                y++;
            }
        }

        mbt = getMBTP();
        if(mbt == INVALID_DECODE_RETURN) {
            return;
        }
        framecode = (mbt>>16);
        mbt  = (mbt<<16)>>16;
        if(framecode & CODE_Q) {
            GETBITS(5, quant);
            current[C_Q] = quant;
        }
        if(framecode & CODE_I) {
            if(cmba != current[L_MBA]+1) {
                current[DC_LUMA] = 1024;
                current[DC_Cr]   = 1024;
                current[DC_Cb]   = 1024;
            }
            current[L_MBA] = cmba;

            doIntra(next->getMacroBlock(cmba), current[C_Q]);
            mfh = 0;
            mfv = 0;
        } else {
            if(framecode & CODE_F) {
                mfh = getMVD(mfh, ff);
                mfv = getMVD(mfv, ff);
                if(ffpv) {
                    mfh <<= 1;
                    mfv <<= 1;
                }

                if((framecode & CODE_M) == CODE_F) {
                    tmbf = next->getMacroBlock(cmba);
                } else {
                    tmbf = tmbF;
                }

                last->getMVReference(tmbf, x, y, mfh, mfv, 7);
            } else {
                mfh = 0;
                mfv = 0;
                tmbf = last->getMacroBlock(cmba);
            }
            if(framecode & CODE_C) {
                cbp = getMVC();
                doInter(cbp, tmbD, current[C_Q]);
            }
            if((framecode & CODE_M) != CODE_F) {
                doAdd(cbp, framecode&CODE_M,
                      (Xil_signed16*)tmbf, NULL,
                      (Xil_signed16*)tmbD,
                      (Xil_signed16*)next->getMacroBlock(cmba));
            }
        }
        done = checkforstart();
    }

    current[C_MBA] = cmba;

  ErrorReturn:
    return;
}
