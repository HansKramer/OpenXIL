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
//  Revision:	1.11
//  Last Mod:	10:11:33, 03/10/00
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
#pragma ident	"@(#)Divide.cc	1.11\t00/03/10  "

#include "XilDeviceManagerComputeSHORT.hh"
#include "ComputeInfo.hh"

XilStatus
XilDeviceManagerComputeSHORT::Divide(XilOp*       op,
                                     unsigned     op_count,
                                     XilRoi*      roi,
                                     XilBoxList*  bl)
{
    Xil_boolean divide_by_zero = FALSE;

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

    while(ci.hasMoreInfo()) {
#ifdef _XIL_USE_INTMULDIV
        int result;
        COMPUTE_GENERAL_2S_1D(Xil_signed16, Xil_signed16, Xil_signed16, 
                              if(*src2 == 0) {
                                  divide_by_zero = TRUE;
                                  if(*src1 == 0) {
                                      *dest = 0;
                                  } else if(*src1 < 0) {
                                      *dest = XIL_MINSHORT;
                                  } else {
                                      *dest = XIL_MAXSHORT;
                                  }
                              } else {
                                  result = (int)(*src1) / (int)(*src2);
                                  *dest = _XILI_CLAMP_S16(result);
                              },
                              
                              if(*(src2+1) == 0) {
                                  divide_by_zero = TRUE;
                                  if(*(src1+1) == 0) {
                                      *dest = 0;
                                  } else if(*(src1+1) < 0) {
                                      *dest = XIL_MINSHORT;
                                  } else {
                                      *dest = XIL_MAXSHORT;
                                  }
                              } else {
                                  result    = (int)(*(src1+1)) / (int)(*(src2+1));
                                  *(dest+1) = _XILI_CLAMP_S16(result);
                              }
                              
                              if(*(src2+2) == 0) {
                                  divide_by_zero = TRUE;
                                  if(*(src1+2) == 0) {
                                      *dest = 0;
                                  } else if(*(src1+2) < 0) {
                                      *dest = XIL_MINSHORT;
                                  } else {
                                      *dest = XIL_MAXSHORT;
                                  }
                              } else {
                                  result    = (int)(*(src1+2)) / (int)(*(src2+2));
                                  *(dest+2) = _XILI_CLAMP_S16(result);
                              }
            );
#else
        //
        //  NOTE:  We're using CLAMP because we're simulating integer division
        //         in floating point.  Thus, we want to truncate the floating
        //         point value to its integer component.
        //
#ifdef _XIL_USE_TABLE_FLT_CNV
        float result;
        COMPUTE_GENERAL_2S_1D(Xil_signed16, Xil_signed16, Xil_signed16, 
                              if(*src2 == 0) {
                                  divide_by_zero = TRUE;
                                  if(*src1 == 0) {
                                      *dest = 0;
                                  } else if(*src1 < 0) {
                                      *dest = XIL_MINSHORT;
                                  } else {
                                      *dest = XIL_MAXSHORT;
                                  }
                              } else {
                                  result = s2f[*src1] / s2f[*src2];
                                  *dest  = _XILI_CLAMP_S16(result);
                              },

                              if(*(src2+1) == 0) {
                                  divide_by_zero = TRUE;
                                  if(*(src1+1) == 0) {
                                      *dest = 0;
                                  } else if(*(src1+1) < 0) {
                                      *dest = XIL_MINSHORT;
                                  } else {
                                      *dest = XIL_MAXSHORT;
                                  }
                              } else {
                                  result    = s2f[*(src1+1)] / s2f[*(src2+1)];
                                  *(dest+1) = _XILI_CLAMP_S16(result);
                              }

                              if(*(src2+2) == 0) {
                                  divide_by_zero = TRUE;
                                  if(*(src1+2) == 0) {
                                      *dest = 0;
                                  } else if(*(src1+2) < 0) {
                                      *dest = XIL_MINSHORT;
                                  } else {
                                      *dest = XIL_MAXSHORT;
                                  }
                              } else {
                                  result    = s2f[*(src1+2)] / s2f[*(src2+2)];
                                  *(dest+2) = _XILI_CLAMP_S16(result);
                            }
          );
#else
        float result;
        COMPUTE_GENERAL_2S_1D(Xil_signed16, Xil_signed16, Xil_signed16, 
                              if(*src2 == 0) {
                                  divide_by_zero = TRUE;
                                  if(*src1 == 0) {
                                      *dest = 0;
                                  } else if(*src1 < 0) {
                                      *dest = XIL_MINSHORT;
                                  } else {
                                      *dest = XIL_MAXSHORT;
                                  }
                              } else {
                                  result = (float)(*src1) / (float)(*src2);
                                  *dest = _XILI_CLAMP_S16(result);
                              },
                              
                              if(*(src2+1) == 0) {
                                  divide_by_zero = TRUE;
                                  if(*(src1+1) == 0) {
                                      *dest = 0;
                                  } else if(*(src1+1) < 0) {
                                      *dest = XIL_MINSHORT;
                                  } else {
                                      *dest = XIL_MAXSHORT;
                                  }
                              } else {
                                  result    = (float)(*(src1+1)) / (float)(*(src2+1));
                                  *(dest+1) = _XILI_CLAMP_S16(result);
                              }
                              
                              if(*(src2+2) == 0) {
                                  divide_by_zero = TRUE;
                                  if(*(src1+2) == 0) {
                                      *dest = 0;
                                  } else if(*(src1+2) < 0) {
                                      *dest = XIL_MINSHORT;
                                  } else {
                                      *dest = XIL_MAXSHORT;
                                  }
                              } else {
                                  result    = (float)(*(src1+2)) / (float)(*(src2+2));
                                  *(dest+2) = _XILI_CLAMP_S16(result);
                              }
            );
#endif // _XIL_USE_TABLE_FLT_CNV
#endif // _XIL_USE_INTMULDIV
    }

    if(divide_by_zero) {
        XIL_ERROR((op->getSrcImage(1))->getSystemState(),
                  XIL_ERROR_USER, "di-171", TRUE);
    }

    return ci.returnValue;
}

