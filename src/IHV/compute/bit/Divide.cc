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
//  File:	Divide.cc
//  Project:	XIL
//  Revision:	1.6
//  Last Mod:	10:09:16, 03/10/00
//
//  Description:
//	
//	
//	
//	
//	
//	
//	
//	
//  MT-level:  SAFE
//	
//------------------------------------------------------------------------
//	COPYRIGHT
//------------------------------------------------------------------------
#pragma ident	"@(#)Divide.cc	1.6\t00/03/10  "

#include "XilDeviceManagerComputeBIT.hh"
#include "ComputeInfo.hh"
#include "XiliUtils.hh"

XilStatus
XilDeviceManagerComputeBIT::Divide(XilOp*       op,
                                   unsigned     op_count,
                                   XilRoi*      roi,
                                   XilBoxList*  bl)
{
    ComputeInfoBIT  ci(op, op_count, roi, bl);

    if(ci.isOK() == FALSE) {
        return XIL_FAILURE;
    }

    unsigned int nbands = ci.destNumBands;

    Xil_boolean  div_by_zero = FALSE;

    while(ci.hasMoreInfo()) {
	for(int b=0; b<nbands; b++) {
            Xil_unsigned8* src1_scanline      = ci.getSrc1Data(b);
            Xil_unsigned8* src2_scanline      = ci.getSrc2Data(b);
            Xil_unsigned8* dest_scanline      = ci.getDestData(b);

            unsigned int src1_offset          = ci.getSrc1Offset(b);
            unsigned int src2_offset          = ci.getSrc2Offset(b);
            unsigned int dest_offset          = ci.getDestOffset(b);

            unsigned int src1_scanline_stride = ci.getSrc1ScanlineStride(b);
            unsigned int src2_scanline_stride = ci.getSrc2ScanlineStride(b);
            unsigned int dest_scanline_stride = ci.getDestScanlineStride(b);

            unsigned int xsize                = ci.xsize;

	    for(int y=ci.ysize; y>0; y--) {
                //
                //  Check for division by zero in the second source.
                //
                //  If there is a division by zero, then set a flag indicating
                //  we discovered an error which we'll report at the
                //  conclusion of the operation.
                //
                if(xili_bit_check_for_zero(src2_scanline, xsize, src2_offset)) {
                    div_by_zero = TRUE;
                }

                //
                //  Divide of bits is a copy...
                //
                //    0/0 -> ? (w/error above)
                //    1/0 -> ? (w/error above)
                //    0/1 -> 0
                //    1/1 -> 1
                //
                xili_bit_memcpy(src1_scanline,
                                dest_scanline,
                                xsize,
                                src1_offset,
                                dest_offset);

		src1_scanline += src1_scanline_stride;
		src2_scanline += src2_scanline_stride;
		dest_scanline += dest_scanline_stride;
	    }
	}
    }

    if(div_by_zero) {
        XIL_ERROR(ci.getSystemState(), XIL_ERROR_USER, "di-171", TRUE);
    }

    return ci.returnValue;
}

