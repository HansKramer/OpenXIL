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
//  File:       doInter.cc
//  Project:    XIL
//  Revision:   1.3
//  Last Mod:   10:15:10, 03/10/00
//
//  Description:
//
//    Process an inter-frame coded macroblock
//
//
//------------------------------------------------------------------------
//    COPYRIGHT
//------------------------------------------------------------------------
#pragma ident   "@(#)doInter.cc	1.3\t00/03/10  "


#include "H261DecompressorData.hh"

void 
H261DecompressorData::doInter(int           code, 
                              Xil_signed16* result[6], 
                              int           quant)
{
    int		index;
    int		block;
    int		decoded_coeff;
    int		value;
    int		zeros;
    int		last;
    int		coeffs[65];

    /*
         .... scan through each bit of cbp ....
    */
    for (block = 32; block > 0; block >>= 1) {
	if (code & block) {
	    index = -1;
	    last = 0;
	    decoded_coeff = decode(useFSTTable());
	    while (1) {
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
		    break;
		else // (decoded_coeff == ILLEGAL_CODE)
		    return;

		//Check for coeff out-of-range!
		if (last > 63)
		    return;
		index += 1 + zeros;
		coeffs[last++] = value + index;
		decoded_coeff = decode(useDCTTable());
	    }

	    coeffs[last] = 0;
	    dequantize(quant, coeffs, 1);
	    idct((int*) result[0], coeffs);
	}
	result++;
    }

ErrorReturn:
    return;

}
