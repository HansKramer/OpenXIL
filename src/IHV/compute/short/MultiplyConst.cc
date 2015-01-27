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
//  Revision:	1.10
//  Last Mod:	10:11:39, 03/10/00
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
#pragma ident	"@(#)MultiplyConst.cc	1.10\t00/03/10  "

#include "XilDeviceManagerComputeSHORT.hh"
#include "ComputeInfo.hh"

XilStatus
XilDeviceManagerComputeSHORT::MultiplyConstPreprocess(XilOp*        op,
                                                      unsigned      ,
                                                      XilRoi*       ,
                                                      void**        compute_data,
                                                      unsigned int* )
{
    unsigned int nbands = op->getDstImage(1)->getNumBands();

    float* mul_consts;
    op->getParam(1, (void**)&mul_consts);

    XilSystemState* state  = op->getDstImage(1)->getSystemState();

    //
    //  Get the cache number we can use...
    //
    *compute_data = (void*)getRescaleTables(state, mul_consts, NULL, nbands);

    return XIL_SUCCESS;
}

XilStatus
XilDeviceManagerComputeSHORT::MultiplyConstPostprocess(XilOp* op,
                                                       void*  compute_data)
{
    releaseRescaleTables((int*)compute_data,
                         op->getDstImage(1)->getNumBands());

    return XIL_SUCCESS;
}

XilStatus
XilDeviceManagerComputeSHORT::MultiplyConst(XilOp*       op,
                                            unsigned     op_count,
                                            XilRoi*      roi,
                                            XilBoxList*  bl)
{
    ComputeInfoSHORT ci(op, op_count, roi, bl);

    if(ci.isOK() == FALSE) {
        return ci.returnValue;
    }

    int* tables = (int*)op->getPreprocessData(this);
    if(tables == NULL) {
        //
        //  Do operation old-style...
        //
        float* mult_const;
        op->getParam(1, (void**)&mult_const);

#ifdef _XIL_USE_TABLE_FLT_CNV
        float* s2f = getShortToFloat(ci.getSystemState());

        if(s2f != NULL) {
            float* mc  = mult_const;

            while(ci.hasMoreInfo()) {
                COMPUTE_GENERAL_1S_1D_W_BAND(Xil_signed16, Xil_signed16,

                                             *dest = _XILI_ROUND_S16(s2f[*src1] * *mc),

                                             
                                             *(dest+1) =
                                                 _XILI_ROUND_S16(s2f[*(src1+1)] * *(mc+1));
                                             *(dest+2) =
                                                 _XILI_ROUND_S16(s2f[*(src1+2)] * *(mc+2)),


                                             *dest = _XILI_ROUND_S16(s2f[*src1] * *(mc+band))
                    );
            }
        } else {
#endif // _XIL_USE_TABLE_FLT_CNV
            float* mc  = mult_const;

            while(ci.hasMoreInfo()) {
                COMPUTE_GENERAL_1S_1D_W_BAND(Xil_signed16, Xil_signed16,

                                             *dest = _XILI_ROUND_S16(((float)*src1) * *mc),

                                             
                                             *(dest+1) =
                                                 _XILI_ROUND_S16(((float)*(src1+1)) * *(mc+1));
                                             *(dest+2) =
                                                 _XILI_ROUND_S16(((float)*(src1+2)) * *(mc+2)),


                                             *dest = _XILI_ROUND_S16(((float)*src1) * *(mc+band))
                    );
            }
#ifdef _XIL_USE_TABLE_FLT_CNV
        }
#endif
    } else {
        tableRescale(ci, tables);
    }

    return ci.returnValue;
}



