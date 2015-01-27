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
//  File:	Extrema.cc
//  Project:	XIL
//  Revision:	1.14
//  Last Mod:	10:09:30, 03/10/00
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
#pragma ident	"@(#)Extrema.cc	1.14\t00/03/10  "

#if defined(_WINDOWS)
#include <malloc.h>
#elif !defined(HPUX)
#include <alloca.h>
#endif
#include "XilDeviceManagerComputeBIT.hh"
#include "ComputeInfo.hh"

XilStatus
XilDeviceManagerComputeBIT::Extrema(XilOp*       op,
                                      unsigned     op_count,
                                      XilRoi*      roi,
                                      XilBoxList*  bl)
{
    ComputeInfoBIT  ci(op, op_count, roi, bl);

    if(ci.isOK() == FALSE) {
        return ci.returnValue;
    }
    //
    //  "nbands" is also required for COMPUTE_GENERAL_1S_W_BAND().
    //
    const unsigned int nbands = ci.src1NumBands;

    //
    //  If nbands is reasonable, then attempt to allocate the arrays on the stack using
    //  alloca() instead of malloc(). (Except for HP where it doesn't work.)
    //
    //  Create the required data arrays and check memory
    //  allocation before preceeding.
    //
    Xil_unsigned8* max = NULL;
    Xil_unsigned8* min = NULL;
    Xil_boolean    used_new;

#if !defined(HPUX)
    if(nbands < 256) {
        max  = (Xil_unsigned8*)malloc(nbands*sizeof(Xil_unsigned8));
        min  = (Xil_unsigned8*)malloc(nbands*sizeof(Xil_unsigned8));
        used_new = FALSE;
    } else {
#endif // !HPUX
        max = new Xil_unsigned8[nbands];
        if(max == NULL) {
            XIL_ERROR(ci.getSystemState(), XIL_ERROR_RESOURCE, "di-1", TRUE);
            return XIL_FAILURE;
        }
    
        min = new Xil_unsigned8[nbands];
        if(min == NULL) {
            delete [] max;
            XIL_ERROR(ci.getSystemState(), XIL_ERROR_RESOURCE, "di-1", TRUE);
            return XIL_FAILURE;
        }

        used_new = TRUE;
#if !defined(HPUX)
    }
#endif // !HPUX

    //
    //  For each band, we assume the min is 1 and max is 0.  If we find a 0
    //  bit in the band, then we mark min to 0 and continue looking for a 1.
    //  If we find a 1, then we mark the max to 1 and move onto the next
    //  band.
    //
    for(int i=0; i<nbands; i++) {
        min[i] = 1;
        max[i] = 0;
    }

    while(ci.hasMoreInfo()) {
	for(int b=0; b<nbands; b++) {
	    Xil_unsigned8* src1_scanline        = ci.getSrc1Data(b);
            unsigned int   src1_scanline_stride = ci.getSrc1ScanlineStride(b);
	    unsigned int   src1_offset          = ci.getSrc1Offset(b);

            Xil_boolean    found_0 = FALSE;
            Xil_boolean    found_1 = FALSE;

	    for(int y=0; y<ci.ysize; y++) {
		for(int x=0; x<ci.xsize; x++) {
                    if(XIL_BMAP_TST(src1_scanline, src1_offset + x)) {
                        found_1 = TRUE;
                        max[b]  = 1;
                        if(found_0) {
                            goto next_band;
                        }
                    } else {
                        found_0 = TRUE;
                        min[b]  = 0;
                        if(found_1) {
                            goto next_band;
                        }
                    }
		}

		src1_scanline += src1_scanline_stride;
	    }

    next_band:
            ;
        }
    }

    if(ci.returnValue != XIL_SUCCESS) {
        //
        //  Something went wrong, clean up and return
        //
        if(used_new) {
            delete [] max;
            delete [] min;
        }

	return ci.returnValue;
    }

    //
    //  Report the results and return the status
    //
    void* results[2] = { (void*)min, (void*)max };
    XilStatus status = op->reportResults(results);

    //
    //  Clean up and return
    //
    if(used_new) {
        delete [] max;
        delete [] min;
    }

    return status;
}
