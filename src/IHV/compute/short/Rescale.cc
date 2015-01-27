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
//  File:	Rescale.cc
//  Project:	XIL
//  Revision:	1.4
//  Last Mod:	11:47:55, 10/03/95
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
#pragma ident	"@(#)Rescale.cc	1.4\t95/10/03  "

#include "XilDeviceManagerComputeSHORT.hh"
#include "ComputeInfo.hh"

XilStatus
XilDeviceManagerComputeSHORT::RescalePreprocess(XilOp*        op,
                                               unsigned      ,
                                               XilRoi*       ,
                                               void**        compute_data,
                                               unsigned int* )
{
    //
    //  Get the multConst and addConst for this operation.
    //
    float* multConst;
    op->getParam(1, (void **)&multConst);

    float* addConst;
    op->getParam(2, (void **)&addConst);

    unsigned int    nbands = op->getDstImage(1)->getNumBands();
    XilSystemState* state  = op->getDstImage(1)->getSystemState();
    
    //
    //  Get the cache number we can use...
    //
    *compute_data = (void*)getRescaleTables(state, multConst, addConst, nbands);

    return XIL_SUCCESS;
}

XilStatus
XilDeviceManagerComputeSHORT::RescalePostprocess(XilOp*       op,
                                                void*        compute_data)
{
    releaseRescaleTables((int*)compute_data,
                         op->getDstImage(1)->getNumBands());

    return XIL_SUCCESS;
}

XilStatus
XilDeviceManagerComputeSHORT::Rescale(XilOp*       op,
				     unsigned     op_count,
				     XilRoi*      roi,
				     XilBoxList*  bl)
{
    ComputeInfoSHORT  ci(op, op_count, roi, bl);

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

        float* add_const;
        op->getParam(2, (void**)&add_const);

#ifdef _XIL_USE_TABLE_FLT_CNV
        float* s2f = getShortToFloat(ci.getSystemState());

        if(s2f != NULL) {
            float* mc  = mult_const;
            float* ac  = add_const;

            while(ci.hasMoreInfo()) {
                COMPUTE_GENERAL_1S_1D_W_BAND(Xil_signed16, Xil_signed16,

                                             *dest =
                                                 _XILI_ROUND_S16(s2f[*src1] * *mc + *ac),

                                             *(dest+1) =
                                                 _XILI_ROUND_S16(s2f[*(src1+1)] *
                                                                 *(mc+1) + *(ac+1));
                                             *(dest+2) =
                                                 _XILI_ROUND_S16(s2f[*(src1+2)] *
                                                                 *(mc+2) + *(ac+2)),

                                             *dest  =
                                                 _XILI_ROUND_S16(s2f[*src1] *
                                                                 *(mc+band) + *(ac+band))
                    );
            }
        } else {
#endif // _XIL_USE_TABLE_FLT_CNV
            float* mc  = mult_const;
            float* ac  = add_const;

            while(ci.hasMoreInfo()) {
                COMPUTE_GENERAL_1S_1D_W_BAND(Xil_signed16, Xil_signed16,

                                             *dest = _XILI_ROUND_S16(((float)*src1) *
                                                                     *mc + *ac),

                                             *(dest+1) =
                                                 _XILI_ROUND_S16(((float)*(src1+1)) *
                                                                 *(mc+1) + *(ac+1));
                                             *(dest+2) =
                                                 _XILI_ROUND_S16(((float)*(src1+2)) *
                                                                 *(mc+2) + *(ac+2)),
                                      
                                             *dest  =
                                                 _XILI_ROUND_S16(((float)*src1) *
                                                                 *(mc+band) + *(ac+band))
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

//
// Passing ci by reference because I want to use the macros
//
void
XilDeviceManagerComputeSHORT::tableRescale(ComputeInfoSHORT& ci,
                                           int*              tables)
{
    unsigned int nbands = ci.destNumBands;

    while(ci.hasMoreInfo()) {
        Xil_signed16* r_lut_0;
        Xil_signed16* r_lut_1;
        Xil_signed16* r_lut_2;

        switch(nbands) {
          case 3:
            r_lut_1 = rescaleCache[tables[1]];
            r_lut_2 = rescaleCache[tables[2]];

          case 1:
            r_lut_0 = rescaleCache[tables[0]];
        }

        COMPUTE_GENERAL_1S_1D_W_BAND(Xil_signed16, Xil_signed16,

                                     *dest     = r_lut_0[*src1],

                                     *(dest+1) = r_lut_1[*(src1+1)];
                                     *(dest+2) = r_lut_2[*(src1+2)],

                                     *dest     = rescaleCache[tables[band]][*src1]
            );
    }
}
