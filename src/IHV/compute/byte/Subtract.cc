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
//  File:	Subtract.cc
//  Project:	XIL
//  Revision:	1.11
//  Last Mod:	10:10:06, 03/10/00
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
#pragma ident	"@(#)Subtract.cc	1.11\t00/03/10  "

#include "XilDeviceManagerComputeBYTE.hh"
#include "ComputeInfo.hh"

XilStatus
XilDeviceManagerComputeBYTE::Subtract(XilOp*       op,
                                      unsigned     op_count,
                                      XilRoi*      roi,
                                      XilBoxList*  bl)
{
    ComputeInfoBYTE  ci(op, op_count, roi, bl);

    if(ci.isOK() == FALSE) {
        return ci.returnValue;
    }
    
    Xil_unsigned8*  subt_clamp = getSubtractClampArray(ci.getSystemState());
    if(subt_clamp == NULL) {
        return XIL_FAILURE;
    }

    while(ci.hasMoreInfo()) {
        COMPUTE_GENERAL_2S_1D(Xil_unsigned8, Xil_unsigned8, Xil_unsigned8,

                              *dest = *(subt_clamp + (*src1 - *src2)),
                              
                              *(dest+1) = *(subt_clamp + (*(src1+1) - *(src2+1)));
                              *(dest+2) = *(subt_clamp + (*(src1+2) - *(src2+2)))
            );
    }
    
    return ci.returnValue;
}

