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
//  File:	Not.cc
//  Project:	XIL
//  Revision:	1.7
//  Last Mod:	10:09:18, 03/10/00
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
//  MT-level:  <??????>
//	
//------------------------------------------------------------------------
//	COPYRIGHT
//------------------------------------------------------------------------
#pragma ident	"@(#)Not.cc	1.7\t00/03/10  "

#include "XilDeviceManagerComputeBIT.hh"
#include "ComputeInfo.hh"
#include "XiliUtils.hh"

XilStatus
XilDeviceManagerComputeBIT::Not(XilOp*       op,
                                unsigned     op_count,
                                XilRoi*      roi,
                                XilBoxList*  bl)
{
    ComputeInfoBIT  ci(op, op_count, roi, bl);

    if(ci.isOK() == FALSE) {
        return ci.returnValue;
    }

    unsigned int nbands = ci.destNumBands;

    while(ci.hasMoreInfo()) {
	for(int b=0; b<nbands; b++) {
	    Xil_unsigned8* src1_scanline      = ci.getSrc1Data(b);
	    Xil_unsigned8* dest_scanline      = ci.getDestData(b);

	    unsigned int   src1_offset        = ci.getSrc1Offset(b);
	    unsigned int   dest_offset        = ci.getDestOffset(b);

	    unsigned int src1_scanline_stride = ci.getSrc1ScanlineStride(b);
	    unsigned int dest_scanline_stride = ci.getDestScanlineStride(b);

	    for(int y=ci.ysize; y>0; y--) {

		xili_bit_not(src1_scanline,
                             dest_scanline,
                             ci.xsize,
                             src1_offset,
                             dest_offset);
		
		src1_scanline += src1_scanline_stride;
		dest_scanline += dest_scanline_stride;
	    }
	}
    }
    
    return ci.returnValue;
}

