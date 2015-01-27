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
//  File:	DivideIntoConst.cc
//  Project:	XIL
//  Revision:	1.9
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
//  MT-level:  SAFE
//	
//------------------------------------------------------------------------
//	COPYRIGHT
//------------------------------------------------------------------------
#pragma ident	"@(#)DivideIntoConst.cc	1.9\t00/03/10  "

#include "XilDeviceManagerComputeSHORT.hh"
#include "ComputeInfo.hh"

//
//  If we can't use integer division, then we do the math in floating point.
//
#ifndef _XIL_USE_INTMULDIV
XilStatus
XilDeviceManagerComputeSHORT::DivideIntoConstPreprocess(XilOp*        op,
                                                        unsigned      ,
                                                        XilRoi*       ,
                                                        void**        compute_data,
                                                        unsigned int* )
{
    Xil_signed32* consts;
    op->getParam(1, (void **)&consts);

    unsigned int    nbands  = op->getDstImage(1)->getNumBands();
    float*          fconsts = new float[nbands];
    if(fconsts == NULL) {
        XIL_ERROR(op->getDstImage(1)->getSystemState(),
                  XIL_ERROR_RESOURCE, "di-1", TRUE);
    }

    for(unsigned int b=0; b<nbands; b++) {
        *(fconsts+b) = (float)*(consts+b);
    }

    *compute_data = fconsts;

    return XIL_SUCCESS;
}

XilStatus
XilDeviceManagerComputeSHORT::DivideIntoConstPostprocess(XilOp*       ,
                                                         void*        compute_data)
{
    delete [] (float*)compute_data;

    return XIL_SUCCESS;
}
#endif // _XIL_USE_INTMULDIV

XilStatus
XilDeviceManagerComputeSHORT::DivideIntoConst(XilOp*       op,
                                              unsigned     op_count,
                                              XilRoi*      roi,
                                              XilBoxList*  bl)
{
    ComputeInfoSHORT  ci(op, op_count, roi, bl);

    if(ci.isOK() == FALSE) {
        return ci.returnValue;
    }

#ifdef _XIL_USE_TABLE_FLT_CNV
    Xil_float32* s2f = getShortToFloat(ci.getSystemState());
    if(s2f == NULL) {
        return XIL_FAILURE;
    }
#endif

    Xil_signed32* consts;
    op->getParam(1, (void**)&consts);

#ifndef _XIL_USE_INTMULDIV
    float*          fconsts = (float*)op->getPreprocessData(this);
#endif

    Xil_boolean     divide_by_zero_error = FALSE;

    while(ci.hasMoreInfo()) {
#ifdef _XIL_USE_INTMULDIV
        COMPUTE_GENERAL_1S_1D_W_BAND(Xil_signed16, Xil_signed16,

                                     if(*src1 == 0) {
                                         divide_by_zero_error = TRUE;
                                         
                                         if(*consts == 0) {
                                             *dest = 0;
                                         } else if(*consts < 0) {
                                             *dest = XIL_MINSHORT;
                                         } else {
                                             *dest = XIL_MAXSHORT;
                                         }
                                     } else {
                                         *dest = *consts / *srcTRUE;
                                     },

                                     if(*(src1+1) == 0) {
                                         divide_by_zero_error = TRUE;
                                         
                                         if(*(consts+1) == 0) {
                                             *(dest+1) = 0;
                                         } else if(*(consts+1) < 0) {
                                             *(dest+1) = XIL_MINSHORT;
                                         } else {
                                             *(dest+1) = XIL_MAXSHORT;
                                         }
                                     } else {
                                         *(dest+1) = *(consts+1) / *(src1+1);
                                     }
                              
                                     if(*(src1+2) == 0) {
                                         divide_by_zero_error = TRUE;
                                         
                                         if(*(consts+2) == 0) {
                                             *(dest+2) = 0;
                                         } else if(*(consts+2) < 0) {
                                             *(dest+2) = XIL_MINSHORT;
                                         } else {
                                             *(dest+2) = XIL_MAXSHORT;
                                         }
                                     } else {
                                         *(dest+2) = *(consts+2) / *(src1+2);
                                     },
                                     if(*src1 == 0) {
                                         divide_by_zero_error = TRUE;
                                         
                                         if(*(consts+band) == 0) {
                                             *dest = 0;
                                         } else if(*(consts+band) < 0) {
                                             *dest = XIL_MINSHORT;
                                         } else {
                                             *dest = XIL_MAXSHORT;
                                         }
                                     } else {
                                         *dest = *(consts+band) / *src1;
                                     }
            );
#else

        //
        //  NOTE:  We're using CLAMP because we're simulating integer division
        //         in floating point.  Thus, we want to truncate the floating
        //         point value to its integer component.
        //
#ifdef _XIL_USE_TABLE_FLT_CNV
        COMPUTE_GENERAL_1S_1D_W_BAND(Xil_signed16, Xil_signed16,

                                     if(*src1 == 0) {
                                         divide_by_zero_error = TRUE;
                                         
                                         if(*consts == 0) {
                                             *dest = 0;
                                         } else if(*consts < 0) {
                                             *dest = XIL_MINSHORT;
                                         } else {
                                             *dest = XIL_MAXSHORT;
                                         }
                                     } else {
                                         float result =
                                             *fconsts / s2f[*src1];
                                         *dest = _XILI_CLAMP_S16(result);
                                     },

                                     if(*(src1+1) == 0) {
                                         divide_by_zero_error = TRUE;
                                         
                                         if(*(consts+1) == 0) {
                                             *(dest+1) = 0;
                                         } else if(*(consts+1) < 0) {
                                             *(dest+1) = XIL_MINSHORT;
                                         } else {
                                             *(dest+1) = XIL_MAXSHORT;
                                         }
                                     } else {
                                         float result =
                                             *(fconsts+1) / s2f[*(src1+1)];
                                         *(dest+1) = _XILI_CLAMP_S16(result);
                                     }
                                     if(*(src1+2) == 0) {
                                         divide_by_zero_error = TRUE;
                                         
                                         if(*(consts+2) == 0) {
                                             *(dest+2) = 0;
                                         } else if(*(consts+2) < 0) {
                                             *(dest+2) = XIL_MINSHORT;
                                         } else {
                                             *(dest+2) = XIL_MAXSHORT;
                                         }
                                     } else {
                                         float result =
                                             *(fconsts+2) / s2f[*(src1+2)];
                                         *(dest+2) = _XILI_CLAMP_S16(result);
                                     },

                                     if(*src1 == 0) {
                                         divide_by_zero_error = TRUE;
                                         
                                         if(*(consts+band) == 0) {
                                             *dest = 0;
                                         } else if(*(consts+band) < 0) {
                                             *dest = XIL_MINSHORT;
                                         } else {
                                             *dest = XIL_MAXSHORT;
                                         }
                                     } else {
                                         float result =
                                             *(fconsts+band) / s2f[*src1];
                                         *dest = _XILI_CLAMP_S16(result);
                                     }
            );
#else
        COMPUTE_GENERAL_1S_1D_W_BAND(Xil_signed16, Xil_signed16,

                                     if(*src1 == 0) {
                                         divide_by_zero_error = TRUE;
                                         
                                         if(*consts == 0) {
                                             *dest = 0;
                                         } else if(*consts < 0) {
                                             *dest = XIL_MINSHORT;
                                         } else {
                                             *dest = XIL_MAXSHORT;
                                         }
                                     } else {
                                         float result =
                                             *fconsts / (float)*src1;
                                         *dest = _XILI_CLAMP_S16(result);
                                     },

                                     if(*(src1+1) == 0) {
                                         divide_by_zero_error = TRUE;
                                         
                                         if(*(consts+1) == 0) {
                                             *(dest+1) = 0;
                                         } else if(*(consts+1) < 0) {
                                             *(dest+1) = XIL_MINSHORT;
                                         } else {
                                             *(dest+1) = XIL_MAXSHORT;
                                         }
                                     } else {
                                         float result =
                                             *(fconsts+1) / (float)*(src1+1);
                                         *(dest+1) = _XILI_CLAMP_S16(result);
                                     }
                              
                                     if(*(src1+2) == 0) {
                                         divide_by_zero_error = TRUE;
                                         
                                         if(*(consts+2) == 0) {
                                             *(dest+2) = 0;
                                         } else if(*(consts+2) < 0) {
                                             *(dest+2) = XIL_MINSHORT;
                                         } else {
                                             *(dest+2) = XIL_MAXSHORT;
                                         }
                                     } else {
                                         float result =
                                             *(fconsts+2) / (float)*(src1+2);
                                         *(dest+2) = _XILI_CLAMP_S16(result);
                                     },
                                     if(*src1 == 0) {
                                         divide_by_zero_error = TRUE;
                                         
                                         if(*(consts+band) == 0) {
                                             *dest = 0;
                                         } else if(*(consts+band) < 0) {
                                             *dest = XIL_MINSHORT;
                                         } else {
                                             *dest = XIL_MAXSHORT;
                                         }
                                     } else {
                                         float result =
                                             *(fconsts+band) / (float)*src1;
                                         *dest = _XILI_CLAMP_S16(result);
                                     }
            );
#endif // _XIL_USE_TABLE_FLT_CNV       
#endif // _XIL_USE_INTMULDIV
    }

    if(divide_by_zero_error) {
      XIL_ERROR((op->getSrcImage(1))->getSystemState(), 
          XIL_ERROR_USER, "di-171", TRUE);
    }
    
    return ci.returnValue;
}
