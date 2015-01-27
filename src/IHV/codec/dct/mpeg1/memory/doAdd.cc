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
//  File:       doAdd.cc
//  Project:    XIL
//  Revision:   1.2
//  Last Mod:   10:14:56, 03/10/00
//
//  Description:
//
//    Perform the addition of the reference macroblock to
//    reconstruct a macroblock for a P or B slice.
//
//
//------------------------------------------------------------------------
//    COPYRIGHT
//------------------------------------------------------------------------
#pragma ident   "@(#)doAdd.cc	1.2\t00/03/10  "

#include "Mpeg1DecompressorData.hh"

void 
Mpeg1DecompressorData::doAdd(int    cbp, 
                             int    type,
	                     short* tmbf, 
                             short* tmbb, 
                             short* tmbd, 
                             short* tmbr)
{
    int         block_select;
    int		j;
    int         sign;
    int         mask;
    int         round;
    int         *imbr; // Reconstructed 8x8 block (result)
    int         *imbf; // Forward (newer) block
    int         *imbb; // Backward (older) block
    int         *imbd; // Deltas for current block

    //
    // The type code is a 3 bit code, set as follows:
    //   Bit 2 - Backward prediction, if set
    //   Bit 1 - Forward prediction, if set
    //   Bit 0 - DCT coding used, if set
    //
    // Bidirectional (interpolated) prediction is indicated by 
    // having both bits 2 and 1 set (cases 6 and 7).
    // 
    // The Index stepping from 32 (2^5) to 1 (2^0) by halving
    // creates masks to test the bits of the Coded Block Pattern (cbp).
    //
    // The (cbp & i) tests whether a bit is set in the 
    // Coded Block Pattern (cbp). Each bit (5 ... 0), if set,
    // indicates whether that block in the Macroblock is coded,
    // i.e. has undergone DCT and Huff coding. Bits 5 ... 2
    // represent the luminance blocks, bit 1 the Cb and bit 0 the Cr.
    //
    // TODO:(lperry) This codec implementation, from XIL 1.2, 
    //               has the Cr block as the first chroma block.
    //               Mpeg has the Cb block first. However, I think
    //               this is only a labelling error. After everything
    //               is working again, go through and do a global change.
    // 
    switch(type) {
      case 1:
      case 3:
	imbr = (int *) tmbr;
	imbf = (int *) tmbf;
	imbd = (int *) tmbd;
	sign = 0x80008000;
	for (block_select=32; block_select>0; block_select>>=1) {
	    if (cbp & block_select) {
                //
                // This block is coded.
                // So add its deltas in.
                //
		for(j=0;j<32;j+=8) {
		    imbr[0] = (imbf[0] + (imbd[0] ^ sign)) ^ sign;
		    imbr[1] = (imbf[1] + (imbd[1] ^ sign)) ^ sign;
		    imbr[2] = (imbf[2] + (imbd[2] ^ sign)) ^ sign;
		    imbr[3] = (imbf[3] + (imbd[3] ^ sign)) ^ sign;
		    imbr[4] = (imbf[4] + (imbd[4] ^ sign)) ^ sign;
		    imbr[5] = (imbf[5] + (imbd[5] ^ sign)) ^ sign;
		    imbr[6] = (imbf[6] + (imbd[6] ^ sign)) ^ sign;
		    imbr[7] = (imbf[7] + (imbd[7] ^ sign)) ^ sign;
		    imbr += 8;
		    imbf += 8;
		    imbd += 8;
		}
	    } else {
                //
                // This block was not DCT coded.
                // So just use the values of the motion-compensated block,
                // in this case the forward block.
                //
		for(j=0;j<32;j+=8) {
		    imbr[0] = imbf[0];
		    imbr[1] = imbf[1];
		    imbr[2] = imbf[2];
		    imbr[3] = imbf[3];
		    imbr[4] = imbf[4];
		    imbr[5] = imbf[5];
		    imbr[6] = imbf[6];
		    imbr[7] = imbf[7];
		    imbr += 8;
		    imbf += 8;
		}
		imbd += 32;
	    }
	}
	break;

      case 2:
	imbr = (int *) tmbr;
	imbf = (int *) tmbf;
	for(j=0;j<6*32;j+=8) {
	    imbr[0] = imbf[0];
	    imbr[1] = imbf[1];
	    imbr[2] = imbf[2];
	    imbr[3] = imbf[3];
	    imbr[4] = imbf[4];
	    imbr[5] = imbf[5];
	    imbr[6] = imbf[6];
	    imbr[7] = imbf[7];
	    imbr += 8;
	    imbf += 8;
	}
	break;

      case 4:
	imbr = (int *) tmbr;
	imbb = (int *) tmbb;
	for(j=0;j<6*32;j+=8) {
	    imbr[0] = imbb[0];
	    imbr[1] = imbb[1];
	    imbr[2] = imbb[2];
	    imbr[3] = imbb[3];
	    imbr[4] = imbb[4];
	    imbr[5] = imbb[5];
	    imbr[6] = imbb[6];
	    imbr[7] = imbb[7];
	    imbr += 8;
	    imbb += 8;
	}
	break;

      case 5:
	imbr = (int *) tmbr;
	imbb = (int *) tmbb;
	imbd = (int *) tmbd;
	sign = 0x80008000;
	for (block_select=32; block_select>0; block_select>>=1) {
	    if (cbp & block_select) {
		for(j=0;j<32;j+=8) {
		    imbr[0] = (imbb[0] + (imbd[0] ^ sign)) ^ sign;
		    imbr[1] = (imbb[1] + (imbd[1] ^ sign)) ^ sign;
		    imbr[2] = (imbb[2] + (imbd[2] ^ sign)) ^ sign;
		    imbr[3] = (imbb[3] + (imbd[3] ^ sign)) ^ sign;
		    imbr[4] = (imbb[4] + (imbd[4] ^ sign)) ^ sign;
		    imbr[5] = (imbb[5] + (imbd[5] ^ sign)) ^ sign;
		    imbr[6] = (imbb[6] + (imbd[6] ^ sign)) ^ sign;
		    imbr[7] = (imbb[7] + (imbd[7] ^ sign)) ^ sign;
		    imbr += 8;
		    imbb += 8;
		    imbd += 8;
		}
	    } else {
		for(j=0;j<32;j+=8) {
		    imbr[0] = imbb[0];
		    imbr[1] = imbb[1];
		    imbr[2] = imbb[2];
		    imbr[3] = imbb[3];
		    imbr[4] = imbb[4];
		    imbr[5] = imbb[5];
		    imbr[6] = imbb[6];
		    imbr[7] = imbb[7];
		    imbr += 8;
		    imbb += 8;
		}
		imbd += 32;
	    }
	}
	break;

      case 6:
	imbr = (int *) tmbr;
	imbb = (int *) tmbb;
	imbf = (int *) tmbf;
	mask = 0x00ff00ff;
	round = 0x00010001;
	for (j=0; j < 6*32; j+=8) {
	    imbr[0] = ((imbb[0] + imbf[0] + round) >> 1) & mask;
	    imbr[1] = ((imbb[1] + imbf[1] + round) >> 1) & mask;
	    imbr[2] = ((imbb[2] + imbf[2] + round) >> 1) & mask;
	    imbr[3] = ((imbb[3] + imbf[3] + round) >> 1) & mask;
	    imbr[4] = ((imbb[4] + imbf[4] + round) >> 1) & mask;
	    imbr[5] = ((imbb[5] + imbf[5] + round) >> 1) & mask;
	    imbr[6] = ((imbb[6] + imbf[6] + round) >> 1) & mask;
	    imbr[7] = ((imbb[7] + imbf[7] + round) >> 1) & mask;
	    imbr += 8;
	    imbb += 8;
	    imbf += 8;
	}
	break;

      case 7:
	imbr = (int *) tmbr;
	imbb = (int *) tmbb;
	imbf = (int *) tmbf;
	imbd = (int *) tmbd;
	mask = 0x00ff00ff;
	round = 0x00010001;
	sign = 0x80008000;
	for (block_select=32; block_select>0; block_select>>=1) {
	    if (cbp & block_select) {
		for (j=0; j<32; j+=8) {
		    imbr[0] = ((((imbb[0] + imbf[0] + round) >> 1) & mask)
			    + (imbd[0] ^ sign)) ^ sign;
		    imbr[1] = ((((imbb[1] + imbf[1] + round) >> 1) & mask) 
			    + (imbd[1] ^ sign)) ^ sign;
		    imbr[2] = ((((imbb[2] + imbf[2] + round) >> 1) & mask) 
			    + (imbd[2] ^ sign)) ^ sign;
		    imbr[3] = ((((imbb[3] + imbf[3] + round) >> 1) & mask)
			    + (imbd[3] ^ sign)) ^ sign;
		    imbr[4] = ((((imbb[4] + imbf[4] + round) >> 1) & mask) 
			    + (imbd[4] ^ sign)) ^ sign;
		    imbr[5] = ((((imbb[5] + imbf[5] + round) >> 1) & mask) 
                            + (imbd[5] ^ sign)) ^ sign;
		    imbr[6] = ((((imbb[6] + imbf[6] + round) >> 1) & mask) 
                            + (imbd[6] ^ sign)) ^ sign;
		    imbr[7] = ((((imbb[7] + imbf[7] + round) >> 1) & mask) 
                            + (imbd[7] ^ sign)) ^ sign;
		    imbr += 8;
		    imbb += 8;
		    imbf += 8;
		    imbd += 8;
		}
	    } else {
		for (j=0; j<32; j+=8) {
		    imbr[0] = ((imbb[0] + imbf[0] + round) >> 1) & mask;
		    imbr[1] = ((imbb[1] + imbf[1] + round) >> 1) & mask;
		    imbr[2] = ((imbb[2] + imbf[2] + round) >> 1) & mask;
		    imbr[3] = ((imbb[3] + imbf[3] + round) >> 1) & mask;
		    imbr[4] = ((imbb[4] + imbf[4] + round) >> 1) & mask;
		    imbr[5] = ((imbb[5] + imbf[5] + round) >> 1) & mask;
		    imbr[6] = ((imbb[6] + imbf[6] + round) >> 1) & mask;
		    imbr[7] = ((imbb[7] + imbf[7] + round) >> 1) & mask;
		    imbr += 8;
		    imbb += 8;
		    imbf += 8;
		}
		imbd += 32;
	    }
	}
	break;
    }

    return;
}
