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
//  Revision:	1.16
//  Last Mod:	10:11:47, 03/10/00
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
#pragma ident	"@(#)Extrema.cc	1.16\t00/03/10  "

#include <stdlib.h>
#if defined(_WINDOWS)
#include <malloc.h>
#elif !defined(HPUX)
#include <alloca.h>
#endif
#include "XilDeviceManagerComputeSHORT.hh"
#include "ComputeInfo.hh"
#include <stdlib.h>

XilStatus
XilDeviceManagerComputeSHORT::Extrema(XilOp*       op,
                                      unsigned     op_count,
                                      XilRoi*      roi,
                                      XilBoxList*  bl)
{
    ComputeInfoSHORT  ci(op, op_count, roi, bl);

    if(ci.isOK() == FALSE) {
        return ci.returnValue;
    }

    //
    //  "nbands" is also required for COMPUTE_GENERAL_1S_W_BAND().
    //
    const unsigned int nbands = ci.src1NumBands;

    //
    //  If nbands is reasonable, then attempt to allocate the arrays on the stack using
    //  alloca() instead of malloc().  (Except for HP, which alloca confuses.)
    //
    //  Create the required data arrays and check memory
    //  allocation before preceeding.
    //
    Xil_signed16* max;
    Xil_signed16* min;
    Xil_boolean   used_new;

#if !defined(HPUX)
    if(nbands < 256) {
        max  = (Xil_signed16*)alloca(nbands*sizeof(Xil_signed16));
        min  = (Xil_signed16*)alloca(nbands*sizeof(Xil_signed16));
        used_new = FALSE;
    } else {
#endif // !HPUX
        max = new Xil_signed16[nbands];
        if(max == NULL) {
            XIL_ERROR(ci.getSystemState(), XIL_ERROR_RESOURCE, "di-1", TRUE);
            return XIL_FAILURE;
        }
    
        min = new Xil_signed16[nbands];
        if(min == NULL) {
            delete [] max;
            XIL_ERROR(ci.getSystemState(), XIL_ERROR_RESOURCE, "di-1", TRUE);
            return XIL_FAILURE;
        }

        used_new = TRUE;
#if !defined(HPUX)
    }
#endif // !HPUX

    for(int i=0; i<nbands; i++) {
        min[i] = XIL_MAXSHORT;
        max[i] = XIL_MINSHORT;
    }

    while(ci.hasMoreInfo()) {
        COMPUTE_GENERAL_1S_W_BAND(Xil_signed16,

                                  if(*src1 > *max) {
                                      *max = *src1;
                                  }
                                  if(*src1 < *min) {
                                      *min = *src1;
                                  },

                                  if(*(src1+1) > *(max+1)) {
                                      *(max+1) = *(src1+1);
                                  }
                                  if(*(src1+1) < *(min+1)) {
                                      *(min+1) = *(src1+1);
                                  }
                                  if(*(src1+2) > *(max+2)) {
                                      *(max+2) = *(src1+2);
                                  }
                                  if(*(src1+2) < *(min+2)) {
                                      *(min+2) = *(src1+2);
                                  },

                                  if(*src1 > *(max+band)) {
                                      *(max+band) = *src1;
                                  }
                                  if(*src1 < *(min+band)) {
                                      *(min+band) = *src1;
                                  }
            );
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
