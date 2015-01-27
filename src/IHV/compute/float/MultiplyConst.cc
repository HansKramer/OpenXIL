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
//  File:	MultiplyConst.cc
//  Project:	XIL
//  Revision:	1.6
//  Last Mod:	10:12:47, 03/10/00
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
#pragma ident	"@(#)MultiplyConst.cc	1.6\t00/03/10  "

#include "XilDeviceManagerComputeFLOAT.hh"
#include "ComputeInfo.hh"

XilStatus
XilDeviceManagerComputeFLOAT::MultiplyConst(XilOp*       op,
                                            unsigned     op_count,
                                            XilRoi*      roi,
                                            XilBoxList*  bl)
{
    ComputeInfoFLOAT  ci(op, op_count, roi, bl);

    if(ci.isOK() == FALSE) {
        return ci.returnValue;
    }
  
    float* op_constants;
    op->getParam(1, (void**)&op_constants);

    while(ci.hasMoreInfo()) {
        float* consts = op_constants;

        COMPUTE_GENERAL_1S_1D_W_BAND(Xil_float32, Xil_float32,

                                     *dest = *src1 * *consts,

                                     *(dest+1) = *(src1+1) * *(consts+1);
                                     *(dest+2) = *(src1+2) * *(consts+2),
                                      
                                     *dest = *src1 * *(consts+band)
            );
    }

    return ci.returnValue;
}

 


