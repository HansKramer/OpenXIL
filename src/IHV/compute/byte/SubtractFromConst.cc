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
//  File:	SubtractFromConst.cc
//  Project:	XIL
//  Revision:	1.11
//  Last Mod:	10:10:25, 03/10/00
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
#pragma ident	"@(#)SubtractFromConst.cc	1.11\t00/03/10  "

#include "XilDeviceManagerComputeBYTE.hh"
#include "ComputeInfo.hh"

XilStatus
XilDeviceManagerComputeBYTE::SubtractFromConst(XilOp*       op,
					       unsigned     op_count,
					       XilRoi*      roi,
					       XilBoxList*  bl)
{
    ComputeInfoBYTE  ci(op, op_count, roi, bl);

    if(ci.isOK() == FALSE) {
        return ci.returnValue;
    }

    Xil_signed16* op_constants;
    op->getParam(1, (void**)&op_constants);

    int           const0 = op_constants[0];
    int           const1 = op_constants[1];
    int           const2 = op_constants[2];
    
    Xil_unsigned8*  subt_clamp = getSubtractClampArray(ci.getSystemState());
    if(subt_clamp == NULL) {
        return XIL_FAILURE;
    }

    while(ci.hasMoreInfo()) {
        COMPUTE_GENERAL_1S_1D_W_BAND(Xil_unsigned8, Xil_unsigned8,

                                     *dest = *(subt_clamp +
                                               (const0 - (int)*src1)),

                                     *(dest+1) = *(subt_clamp +
                                                   (const1 - (int)*(src1+1)));
                                     *(dest+2) = *(subt_clamp +
                                                   (const2 - (int)*(src1+2))),

                                     *dest = *(subt_clamp +
                                               ((int)*(op_constants+band) - (int)*src1))
            );
    }

    return ci.returnValue;
}

