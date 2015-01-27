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
//  File:	SqueezeRange.cc
//  Project:	XIL
//  Revision:	1.8
//  Last Mod:	10:09:25, 03/10/00
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
#pragma ident	"@(#)SqueezeRange.cc	1.8\t00/03/10  "

#include "XilDeviceManagerComputeBIT.hh"
#include "ComputeInfo.hh"
#include <string.h>

XilStatus
XilDeviceManagerComputeBIT::SqueezeRange(XilOp*       op,
					 unsigned     op_count,
					 XilRoi*      roi,
					 XilBoxList*  bl)
{
    ComputeInfoBIT  ci(op, op_count, roi, bl);

    if(ci.isOK() == FALSE) {
        return ci.returnValue;
    }

    // Initialize values
    Xil_unsigned8 flags[2] = { 0, 0 };
    unsigned int values_seen = 0;
    int bval;

    //
    // Squeeze range by definition only works on single band
    // images so we can remove all of the band loops, and hence
    // special case loops. The op checks that the image is single banded 
    // before we ever get here.
    //
    while(ci.hasMoreInfo()) {
	Xil_unsigned8* src1_scanline = ci.src1Data;
	unsigned int src1_offset = ci.src1Offset;

	for(int y=0; ((y<ci.ysize) && (values_seen <2)); y++) {
	    
	    for(int x=0; ((x<ci.xsize) && (values_seen < 2)); x++) {
		bval = XIL_BMAP_TST(src1_scanline, src1_offset + x) ? 1 : 0;
		if(!flags[bval]) {
		    flags[bval] = 1;
		    values_seen++;
		}
	    }

	    src1_scanline += ci.src1ScanlineStride;
	}
    }

    if(ci.returnValue != XIL_SUCCESS) {
        //
	//  Something went wrong, clean up and return
        //
	return ci.returnValue;
    }

    //
    // Set the imin and imax values appropriately
    //
    int imax;
    int imin;
    if(values_seen == 2) {
	imin = 0;
	imax = 1;
    } else {
	imin = bval;
	imax = bval;
    }

    void* results[3];
	results[0] = (void*)&imin;
	results[1] = (void*)&imax;
	results[2] = (void*)flags;
    return op->reportResults(results);
}
