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
//  File:	SetValue.cc
//  Project:	XIL
//  Revision:	1.9
//  Last Mod:	10:09:11, 03/10/00
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
#pragma ident	"@(#)SetValue.cc	1.9\t00/03/10  "

#include "XilDeviceManagerComputeBIT.hh"
#include "ComputeInfo.hh"
#include "XiliUtils.hh"

XilStatus
XilDeviceManagerComputeBIT::SetValue(XilOp*       op,
                                     unsigned     op_count,
                                     XilRoi*      roi,
                                     XilBoxList*  bl)
{
    Xil_unsigned8* band_vals;
	
    ComputeInfoBIT  ci(op, op_count, roi, bl);

    if(ci.isOK() == FALSE) {
        return ci.returnValue;
    }

    op->getParam(1, (void **)&band_vals);

    unsigned int nbands = ci.destNumBands;

    while(ci.hasMoreInfo()) {
	for(int b=0; b<nbands; b++) {
	    Xil_unsigned8* dest_scanline    = ci.getDestData(b);
            unsigned int   dest_offset      = ci.getDestOffset(b);
            unsigned int   dest_scan_stride = ci.getDestScanlineStride(b);

	    unsigned int   cur_band_val     = (unsigned int)band_vals[b];

	    for(int y=ci.ysize; y>0; y--) {
		xili_bit_setvalue(dest_scanline,
                                  cur_band_val,
                                  ci.xsize, 
                                  dest_offset);

		dest_scanline += dest_scan_stride;
	    }
	}
    }
    
    return ci.returnValue;
}

