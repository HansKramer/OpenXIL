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
//  File:       doSscB.cc
//  Project:    XIL
//  Revision:   1.2
//  Last Mod:   10:14:48, 03/10/00
//
//  Description:
//
//    Reconstruct a slice within a B frame
//
//
//------------------------------------------------------------------------
//    COPYRIGHT
//------------------------------------------------------------------------
#pragma ident   "@(#)doSscB.cc	1.2\t00/03/10  "

#include "Mpeg1DecompressorData.hh"

void 
Mpeg1DecompressorData::doSscB(int slice)
{
    double        mbroom[3*6*32];
    MacroBlock*   tmbF = (MacroBlock *)&mbroom[000];
    MacroBlock*   tmbB = (MacroBlock *)&mbroom[192];
    MacroBlock*   tmbD = (MacroBlock *)&mbroom[384];
    MacroBlock*   tmbf = NULL;
    MacroBlock*   tmbb = NULL;
    MacroBlock*   tmbR;

    int  j;
    int  mba; // Macroblock address
    int  mbt;
    int  cbp; // Coded Block Pattern
    int  x;
    int  y;
    int  code = 0;
    int  done = 2;

    int  fhb  = current[F_HBLOCK];
    int  cmba = current[C_MBA];

    int  ffpv = current[F_F_FPV];
    int  bfpv = current[F_B_FPV];
    int* ff   = modulo[current[F_F_F_CODE]];
    int* bf   = modulo[current[F_B_F_CODE]];

    int  mfh  = 0;
    int  mfv  = 0;
    int  mbh  = 0;
    int  mbv  = 0;

    current[DC_LUMA] = 1024;
    current[DC_Cr]   = 1024;
    current[DC_Cb]   = 1024;

    int  quant;
    GETBITS(5, quant);
    current[C_Q]    = quant;

    GETBITS(1, j);
    while(j == 1) {
        GETBITS(8, j);
        GETBITS(1, j);
    }


    while(done != 1) {
        cbp = 0;
        mba = getMBA();
        if(mba == INVALID_DECODE_RETURN) {
            return;
        }
        if(done == 2) {
            cmba = (slice-1)*current[F_HBLOCK]-1 + mba;
            y = cmba / fhb;
            x = cmba - y*fhb;
            if(cmba >= current[F_TBLOCK]) {
                return;
            }
            tmbR  = bbbb->getMacroBlock(cmba);
        } else {
            if(mba > 1) {
                for(j=1;j<mba;j++) {
                    tmbR++;
                    cbp = 0;
                    x++;
                    if(x >= fhb) {
                        x = 0;
                        y++;
                    }
                    if(code & CODE_F) {
                        if((code&CODE_M) == CODE_F) {
                            tmbf = tmbR;
                        } else {
                            tmbf = tmbF;
                        }
                        last->getMVReference(tmbf, x, y, mfh, mfv, 
                                             bframequality);
                    }
                    if(code & CODE_B) {
                        if((code&CODE_M) == CODE_B) {
                            tmbb = tmbR;
                        } else {
                            tmbb = tmbB;
                        }
                        next->getMVReference(tmbb, x, y, mbh, mbv, 
                                             bframequality);
                    }
                    if(((code&CODE_M) != CODE_F) &&
                        ((code&CODE_M) != CODE_B)) {
                        doAdd(cbp, code&CODE_M,(Xil_signed16*)tmbf,
                                (Xil_signed16*)tmbb, NULL,
                                (Xil_signed16*)tmbR);
                    }
                }
            }
            cmba += mba;
            tmbR++;
            x++;
            if(x >= fhb) {
                x = 0;
                y++;
            }
        }

        mbt = getMBTB();
        if(mbt == INVALID_DECODE_RETURN) {
            return;
        }
        code = (mbt >> 16);
        mbt  = (mbt << 16) >> 16;
        if(code & CODE_Q) {
            GETBITS(5, quant);
            current[C_Q] = quant;
        }

        if(code & CODE_I) {
            if(cmba != current[L_MBA]+1) {
                current[DC_LUMA] = 1024;
                current[DC_Cr]   = 1024;
                current[DC_Cb]   = 1024;
            }
            current[L_MBA] = cmba;

            doIntra(bbbb->getMacroBlock(cmba), quant);

            mfh = 0;
            mfv = 0;
            mbh = 0;
            mbv = 0;
        } else {
            if(code & CODE_F) {
                mfh = getMVD(mfh, ff);
                mfv = getMVD(mfv, ff);
                if(ffpv) {
                    mfh <<= 1;
                    mfv <<= 1;
                }

                if((code&CODE_M) == CODE_F) {
                    tmbf = tmbR;
                } else {
                    tmbf = tmbF;
                }
                last->getMVReference(tmbf, x, y, mfh, mfv,
                                     bframequality);
            }
            if(code & CODE_B) {
                mbh = getMVD(mbh, bf);
                mbv = getMVD(mbv, bf);
                if(bfpv) {
                    mbh <<= 1;
                    mbv <<= 1;
                }

                if((code&CODE_M) == CODE_B) {
                    tmbb = tmbR;
                } else {
                    tmbb = tmbB;
                }
                next->getMVReference(tmbb, x, y, mbh, mbv, 
                                     bframequality);
            }
            if(code & CODE_C) {
                cbp = getMVC();
                if(cbp == INVALID_DECODE_RETURN) {
                    return;
                }
                doInter(cbp, tmbD, quant);
            }
            if(((code&CODE_M) != CODE_F) &&
                ((code&CODE_M) != CODE_B)) {
                doAdd(cbp, code&CODE_M, (Xil_signed16*)tmbf,
                        (Xil_signed16*)tmbb, 
                        (Xil_signed16*)tmbD, 
                        (Xil_signed16*)tmbR);
            }
        }
        done = checkforstart();
    }

    current[C_MBA] = cmba;

  ErrorReturn:
    return;
}

