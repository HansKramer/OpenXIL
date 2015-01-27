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
//  File:	SubtractConst.cc
//  Project:	XIL
//  Revision:	1.8
//  Last Mod:	10:11:40, 03/10/00
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
#pragma ident	"@(#)SubtractConst.cc	1.8\t00/03/10  "

#include "XilDeviceManagerComputeSHORT.hh"
#include "ComputeInfo.hh"

XilStatus
XilDeviceManagerComputeSHORT::SubtractConst(XilOp*       op,
                                            unsigned     op_count,
                                            XilRoi*      roi,
                                            XilBoxList*  bl)
{
    ComputeInfoSHORT  ci(op, op_count, roi, bl);

    if(ci.isOK() == FALSE) {
        return ci.returnValue;
    }

    Xil_signed32* op_constants;
    op->getParam(1, (void**)&op_constants);

    while(ci.hasMoreInfo()) {
        Xil_signed32* consts = op_constants;
        int           result;

        COMPUTE_GENERAL_1S_1D_W_BAND(Xil_signed16, Xil_signed16,

                                     result = *src1 - *consts;
                                     *dest = _XILI_CLAMP_S16(result),

                                     result = *(src1+1) - *(consts+1);
                                     *(dest+1) = _XILI_CLAMP_S16(result);
                                     result = *(src1+2) - *(consts+2);
                                     *(dest+2) = _XILI_CLAMP_S16(result),
                                      
                                     result = *src1 - *(consts+band);
                                     *dest  = _XILI_CLAMP_S16(result)
            );
    }

    return ci.returnValue;
}
