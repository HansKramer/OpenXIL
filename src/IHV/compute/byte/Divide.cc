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
//  File:	Divide.cc
//  Project:	XIL
//  Revision:	1.13
//  Last Mod:	10:10:16, 03/10/00
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
#pragma ident	"@(#)Divide.cc	1.13\t00/03/10  "

#include "XilDeviceManagerComputeBYTE.hh"
#include "ComputeInfo.hh"

XilStatus
XilDeviceManagerComputeBYTE::Divide(XilOp*       op,
				    unsigned     op_count,
				    XilRoi*      roi,
				    XilBoxList*  bl)
{
    ComputeInfoBYTE  ci(op, op_count, roi, bl);

    if(ci.isOK() == FALSE) {
        return ci.returnValue;
    }

    Xil_boolean  divide_by_zero_error = 0;

    while(ci.hasMoreInfo()) {
#ifdef _XIL_USE_INTMULDIV
        COMPUTE_GENERAL_2S_1D(Xil_unsigned8, Xil_unsigned8, Xil_unsigned8, 

                              if(*src2 == 0) {
                                  divide_by_zero_error = TRUE;

                                  *dest = *src1 ? 0 : XIL_MAXBYTE;
                              } else {
                                  *dest = *src1 / *src2;
                              },

                              if(*(src2+1) == 0) {
                                  divide_by_zero_error = TRUE;

                                  *(dest+1) = *(src1+1) ? 0 : XIL_MAXBYTE;
                              } else {
                                  *(dest+1) = *(src1+1) / *(src2+1);
                              }

                              if(*(src2+2) == 0) {
                                  divide_by_zero_error = TRUE;

                                  *(dest+2) = *(src1+2) ? 0 : XIL_MAXBYTE;
                              } else {
                                  *(dest+2) = *(src1+2) / *(src2+2);
                              }
            );
#else
        //
        //  NOTE:  We're using CLAMP because we're simulating integer division
        //         in floating point.  Thus, we want to truncate the floating
        //         point value to its integer component.
        //
        COMPUTE_GENERAL_2S_1D(Xil_unsigned8, Xil_unsigned8, Xil_unsigned8, 

                              if(*src2 == 0) {
                                  divide_by_zero_error = TRUE;

                                  *dest = *src1 ? 0 : XIL_MAXBYTE;
                              } else {
                                  float result = (_XILI_B2F(*src1) /
                                                  _XILI_B2F(*src2));
                                  *dest = _XILI_CLAMP_U8(result);
                              },

                              if(*(src2+1) == 0) {
                                  divide_by_zero_error = TRUE;

                                  *(dest+1) = *(src1+1) ? 0 : XIL_MAXBYTE;
                              } else {
                                  float result = (_XILI_B2F(*(src1+1)) /
                                                  _XILI_B2F(*(src2+1)));
                                  *(dest+1) = _XILI_CLAMP_U8(result);
                              }

                              if(*(src2+2) == 0) {
                                  divide_by_zero_error = TRUE;

                                  *(dest+2) = *(src1+2) ? 0 : XIL_MAXBYTE;
                              } else {
                                  float result = (_XILI_B2F(*(src1+2)) /
                                                  _XILI_B2F(*(src2+2)));
                                  *(dest+2) = _XILI_CLAMP_U8(result);
                              }
            );
#endif // _XIL_USE_INTMULDIV
    }

    if(divide_by_zero_error) {
        XIL_ERROR(ci.getSystemState(), XIL_ERROR_USER, "di-171", TRUE);
    }

    return ci.returnValue;
}

