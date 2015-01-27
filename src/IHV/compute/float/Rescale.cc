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
//  File:	Rescale.cc
//  Project:	XIL
//  Revision:	1.4
//  Last Mod:	11:47:55, 10/03/95
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
#pragma ident	"@(#)Rescale.cc	1.4\t95/10/03  "

#include "XilDeviceManagerComputeFLOAT.hh"
#include "ComputeInfo.hh"

XilStatus
XilDeviceManagerComputeFLOAT::Rescale(XilOp*       op,
				      unsigned     op_count,
				      XilRoi*      roi,
				      XilBoxList*  bl)
{
    ComputeInfoFLOAT ci(op, op_count, roi, bl);

    if(ci.isOK() == FALSE) {
        return ci.returnValue;
    }

    float*  op_mul_constants;
    op->getParam(1, (void**)&op_mul_constants);

    float*  op_add_constants;
    op->getParam(2, (void**)&op_add_constants);

    while(ci.hasMoreInfo()) {
        float* mc = op_mul_constants;
        float* ac = op_add_constants;

        COMPUTE_GENERAL_1S_1D_W_BAND(Xil_float32, Xil_float32,

                                     *dest = *src1 * *mc + *ac,

                                     *(dest+1) = *(src1+1) * *(mc+1) + *(ac+1);
                                     *(dest+2) = *(src1+2) * *(mc+2) + *(ac+2),
                                      
                                     *dest = *src1 * *(mc+band) + *(ac+band)
            );
    }

    return ci.returnValue;
}
