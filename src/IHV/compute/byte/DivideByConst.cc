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
//  File:	DivideByConst.cc
//  Project:	XIL
//  Revision:	1.9
//  Last Mod:	10:10:32, 03/10/00
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
#pragma ident	"@(#)DivideByConst.cc	1.9\t00/03/10  "

#include "XilDeviceManagerComputeBYTE.hh"
#include "ComputeInfo.hh"

XilStatus
XilDeviceManagerComputeBYTE::DivideByConstPreprocess(XilOp*        op,
                                                     unsigned      ,
                                                     XilRoi*       ,
                                                     void**        compute_data,
                                                     unsigned int* )
{
    unsigned int    nbands     = op->getDstImage(1)->getNumBands();


    if(nbands < _XILI_NUM_RESCALE_TABLES) {
        //
        //  Attempt to get tables...
        //
        float mul_consts[_XILI_NUM_RESCALE_TABLES];

        float* div_consts;
        op->getParam(1, (void **)&div_consts);

        for(unsigned int b=0; b<nbands; b++) {
            *(mul_consts+b) = (float) (1.0F / *(div_consts+b));
        }

        XilSystemState* state  = op->getDstImage(1)->getSystemState();
    
        //
        //  Get the cache number we can use...
        //
        *compute_data = (void*)getRescaleTables(state, mul_consts, NULL, nbands);
    } else {
        *compute_data = NULL;
    }

    return XIL_SUCCESS;
}

XilStatus
XilDeviceManagerComputeBYTE::DivideByConstPostprocess(XilOp* op,
                                                      void*  compute_data)
{
    releaseRescaleTables((int*)compute_data,
                         op->getDstImage(1)->getNumBands());

    return XIL_SUCCESS;
}

XilStatus
XilDeviceManagerComputeBYTE::DivideByConst(XilOp*       op,
                                           unsigned     op_count,
                                           XilRoi*      roi,
                                           XilBoxList*  bl)
{
    ComputeInfoBYTE  ci(op, op_count, roi, bl);

    if(ci.isOK() == FALSE) {
        return ci.returnValue;
    }

    int* tables = (int*)op->getPreprocessData(this);
    if(tables == NULL) {
        //
        //  Straightforward Method
        //
        float* div_consts;
        op->getParam(1, (void**)&div_consts);
        
        while(ci.hasMoreInfo()) {
            float*  consts = div_consts;

            COMPUTE_GENERAL_1S_1D_W_BAND(Xil_unsigned8, Xil_unsigned8,

                                         *dest =
                                             _XILI_ROUND_U8(_XILI_B2F(*src1) /
                                                            consts[0]),

                                         *(dest+1) =
                                             _XILI_ROUND_U8(_XILI_B2F(*(src1+1)) /
                                                            consts[1]);
                                         *(dest+2) =
                                             _XILI_ROUND_U8(_XILI_B2F(*(src1+2)) /
                                                            consts[2]),

                                         *dest =
                                             _XILI_ROUND_U8(_XILI_B2F(*src1) /
                                                            consts[band])
                );
        }
    } else {
        tableRescale(ci, tables);
    }

    return ci.returnValue;
}
