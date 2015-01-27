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
//  File:	Multiply.cc
//  Project:	XIL
//  Revision:	1.7
//  Last Mod:	10:11:36, 03/10/00
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
#pragma ident	"@(#)Multiply.cc	1.7\t00/03/10  "

#include "XilDeviceManagerComputeSHORT.hh"
#include "ComputeInfo.hh"

XilStatus
XilDeviceManagerComputeSHORT::Multiply(XilOp*       op,
                                       unsigned     op_count,
                                       XilRoi*      roi,
                                       XilBoxList*  bl)
{
    ComputeInfoSHORT  ci(op, op_count, roi, bl);

    if(ci.isOK() == FALSE) {
        return ci.returnValue;
    }

#if defined(_XIL_USE_TABLE_FLT_CNV) && !defined(_XIL_USE_INTMULDIV)
    Xil_float32* s2f = getShortToFloat(ci.getSystemState());
    if(s2f == NULL) {
        return XIL_FAILURE;
    }
#endif

    //
    // The Macro COMPUTE_GENERAL_S2_D1 traverses across the data.
    //
    while(ci.hasMoreInfo()) {
        int result;

#ifdef _XIL_USE_INTMULDIV
        COMPUTE_GENERAL_2S_1D(Xil_signed16, Xil_signed16, Xil_signed16,

                              result = (int)(*src1) * (int)(*src2);
                              *dest = _XILI_CLAMP_S16(result),

                              result = (int)(*(src1+1)) * (int)(*(src2+1));
                              *(dest+1) = _XILI_CLAMP_S16(result);
                              result = (int)(*(src1+2)) * (int)(*(src2+2));
                              *(dest+2) = _XILI_CLAMP_S16(result)
            );
#else
#ifdef _XIL_USE_TABLE_FLT_CNV
        COMPUTE_GENERAL_2S_1D(Xil_signed16, Xil_signed16, Xil_signed16,

                              result = (int)(s2f[*src1] * s2f[*src2]);
                              *dest = _XILI_CLAMP_S16(result),

                              result = (int)(s2f[*(src1+1)] * s2f[*(src2+1)]);
                              *(dest+1) = _XILI_CLAMP_S16(result);
                              result = (int)(s2f[*(src1+2)] * s2f[*(src2+2)]);
                              *(dest+2) = _XILI_CLAMP_S16(result)
            );
#else
        COMPUTE_GENERAL_2S_1D(Xil_signed16, Xil_signed16, Xil_signed16,

                              result = (int)(*src1) * (int)(*src2);
                              *dest = _XILI_CLAMP_S16(result),

                              result = (int)(*(src1+1)) * (int)(*(src2+1));
                              *(dest+1) = _XILI_CLAMP_S16(result);
                              result = (int)(*(src1+2)) * (int)(*(src2+2));
                              *(dest+2) = _XILI_CLAMP_S16(result)
            );
#endif // _XIL_USE_TABLE_FLT_CNV
#endif // _XIL_USE_INTMULDIV
    }
     
    return ci.returnValue;
}


