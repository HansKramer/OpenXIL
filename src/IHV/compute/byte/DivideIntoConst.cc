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
//  Revision:	1.14
//  Last Mod:	10:10:23, 03/10/00
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
#pragma ident	"@(#)DivideIntoConst.cc	1.14\t00/03/10  "

#include "XilDeviceManagerComputeBYTE.hh"
#include "ComputeInfo.hh"

//
//  If we can't use integer division, then we do the math in floating point.
//
#ifndef _XIL_USE_INTMULDIV
XilStatus
XilDeviceManagerComputeBYTE::DivideIntoConstPreprocess(XilOp*        op,
                                                       unsigned      ,
                                                       XilRoi*       ,
                                                       void**        compute_data,
                                                       unsigned int* )
{
    Xil_unsigned16* consts;
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
XilDeviceManagerComputeBYTE::DivideIntoConstPostprocess(XilOp*       ,
                                                        void*        compute_data)
{
    delete [] (float*)compute_data;

    return XIL_SUCCESS;
}
#endif // _XIL_USE_INTMULDIV

XilStatus
XilDeviceManagerComputeBYTE::DivideIntoConst(XilOp*       op,
					     unsigned     op_count,
					     XilRoi*      roi,
					     XilBoxList*  bl)
{
    ComputeInfoBYTE  ci(op, op_count, roi, bl);

    if(ci.isOK() == FALSE) {
        return ci.returnValue;
    }

    Xil_boolean  divide_by_zero_error = 0;

    Xil_unsigned16* op_constants;
    op->getParam(1, (void**)&op_constants);

#ifndef _XIL_USE_INTMULDIV
    float*          op_pre_data = (float*)op->getPreprocessData(this);
#endif

    while(ci.hasMoreInfo()) {
        Xil_unsigned16* consts  = op_constants;
        float*          fconsts = op_pre_data;
#ifdef _XIL_USE_INTMULDIV
        COMPUTE_GENERAL_1S_1D_W_BAND(Xil_unsigned8, Xil_unsigned8,

                                     if(*src1 == 0) {
                                         divide_by_zero_error = 1;
                                         
                                         *consts ?
                                             *dest = XIL_MAXBYTE :
                                             *dest = 0;
                                     } else {
                                         *dest = *consts / *src1;
                                     },

                                     if(*(src1+1) == 0) {
                                         divide_by_zero_error = 1;
                                         
                                         *(consts+1) ?
                                             *(dest+1) = XIL_MAXBYTE :
                                             *(dest+1) = 0;
                                     } else {
                                         *(dest+1) = *(consts+1) / *(src1+1);
                                     }
                              
                                     if(*(src1+2) == 0) {
                                         divide_by_zero_error = 1;
                                         
                                         *(consts+2) ?
                                             *(dest+2) = XIL_MAXBYTE :
                                             *(dest+2) = 0;
                                     } else {
                                         *(dest+2) = *(consts+2) / *(src1+2);
                                     },
                                     if(*src1 == 0) {
                                         divide_by_zero_error = 1;
                                         
                                         *(consts+band) ?
                                             *dest = XIL_MAXBYTE : *dest = 0;
                                     } else {
                                         *dest = *(consts+band) / *src1;
                                     }
            );
#else
        COMPUTE_GENERAL_1S_1D_W_BAND(Xil_unsigned8, Xil_unsigned8,

                                     if(*src1 == 0) {
                                         divide_by_zero_error = 1;
                                         
                                         *consts ?
                                             *dest = XIL_MAXBYTE :
                                             *dest = 0;
                                     } else {
                                         float result =
                                             *fconsts / _XILI_B2F(*src1);
                                         *dest = _XILI_CLAMP_U8(result);
                                     },

                                     if(*(src1+1) == 0) {
                                         divide_by_zero_error = 1;
                                         
                                         *(consts+1) ?
                                             *(dest+1) = XIL_MAXBYTE :
                                             *(dest+1) = 0;
                                     } else {
                                         float result = (*(fconsts+1) /
                                                         _XILI_B2F(*(src1+1)));
                                         *(dest+1) = _XILI_CLAMP_U8(result);
                                     }
                              
                                     if(*(src1+2) == 0) {
                                         divide_by_zero_error = 1;
                                         
                                         *(consts+2) ?
                                             *(dest+2) = XIL_MAXBYTE :
                                             *(dest+2) = 0;
                                     } else {
                                         float result = (*(fconsts+2) /
                                                         _XILI_B2F(*(src1+2)));
                                         *(dest+2) = _XILI_CLAMP_U8(result);
                                     },
                                     if(*src1 == 0) {
                                         divide_by_zero_error = 1;
                                         
                                         *(consts+band) ?
                                             *dest = XIL_MAXBYTE : *dest = 0;
                                     } else {
                                         float result = (*(fconsts+band) /
                                                         _XILI_B2F(*src1));
                                         *dest = _XILI_CLAMP_U8(result);
                                     }
            );
#endif // _XIL_USE_INTMULDIV
    }

    if(divide_by_zero_error) {
        XIL_ERROR(ci.getSystemState(), XIL_ERROR_USER, "di-171", TRUE);
    }

    return ci.returnValue;
}

