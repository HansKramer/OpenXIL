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
//------------------------------------------------------------------------
//
//  File:       doIntra.cc
//  Project:    XIL
//  Revision:   1.3
//  Last Mod:   10:15:22, 03/10/00
//
//  Description:
//
//    Intra-frame parsing
//
//
//------------------------------------------------------------------------
//    COPYRIGHT
//------------------------------------------------------------------------
#pragma ident   "@(#)doIntra.cc	1.3\t00/03/10  "

#include "H261DecompressorData.hh"

void H261DecompressorData::doIntra(Xil_signed16* result[6], int quant)
{
    int		coeffs[65];
    int		index;
    int		block;
    int		decoded_coeff;
    int		value;
    int		zeros;
    int		last;

    for (block=0;block<6;block++)
    {
	if((value = getbits(8)) == BAD_BIT_VALUE)
	    return;
	if (value == 255) value = 128;
	coeffs[0] = value << 3;

	index = 0;
	last = 1;
	do {
	    decoded_coeff = decode(useDCTTable());

	    if (!(decoded_coeff & 0x8000))
	    {
		zeros = (decoded_coeff >> 8);
		value = (decoded_coeff<<24)>>16;
	    }
	    else if (decoded_coeff == ESC_CODE)
	    {
		GETBITS(14,value);
		zeros = value>>8;
		value = (value<<24)>>16;
	    }
	    else if (decoded_coeff == EOB_CODE)
	    {
		break;
	    }
	    else //(decoded_coeff == ILLEGAL_CODE)
	    {
		return;
	    }
	    //Check for index out-of-range!
	    if (last > 63)
		return;
	    index += 1 + zeros;
	    coeffs[last++] = value + index;
	} while (1);

	coeffs[last] = 0;
	dequantize(quant,coeffs,0);
	idct((int*)result[block], coeffs);
    }

ErrorReturn:
    return;
}
