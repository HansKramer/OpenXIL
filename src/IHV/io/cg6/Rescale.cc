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
//  Revision:	1.2
//  Last Mod:	10:13:57, 03/10/00
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
#pragma ident	"@(#)Rescale.cc	1.2\t00/03/10  "

#include <stdio.h>
#include "XilDeviceIOcg6.hh"
#include "XiliUtils.hh"

XilStatus
XilDeviceIOcg6::rescaleDisplayPre(XilOp*        op,
                                  unsigned      ,
                                  XilRoi*       ,
                                  void**        compute_data,
                                  unsigned int* )
{
    //
    //  Get the multConst and addConst for this operation.
    //
    XilOp* rop = op->getOpList()[1];

    float* multConst;
    rop->getParam(1, (void**)&multConst);

    float* addConst;
    rop->getParam(2, (void**)&addConst);

    XilSystemState* state = op->getDstImage(1)->getSystemState();
    
    //
    //  Get the cache number we can use...
    //
    *compute_data =
        (void*)deviceManager->getRescaleTable(state, *multConst, *addConst);

    return XIL_SUCCESS;
}

XilStatus
XilDeviceIOcg6::rescaleDisplayPost(XilOp* ,
                                   void*  compute_data)
{
    deviceManager->releaseRescaleTable((int)compute_data);

    return XIL_SUCCESS;
}

XilStatus
XilDeviceIOcg6::rescaleDisplay(XilOp*       op,
                               unsigned int ,
                               XilRoi*      roi,
                               XilBoxList*  bl)
{
    int table = (int)op->getPreprocessData(this);
    if(table == -1) {
        return XIL_FAILURE;
    }

    return lookup8to8(op, roi, bl, deviceManager->refRescaleCache(table));
}


////////////////////////////////////////////////////////////////////////////
//
//  ADD CONST
//
////////////////////////////////////////////////////////////////////////////
XilStatus
XilDeviceIOcg6::addConstDisplayPre(XilOp*        op,
                                  unsigned      ,
                                  XilRoi*       ,
                                  void**        compute_data,
                                  unsigned int* )
{
    //
    //  Get the addConst for this operation.
    //
    Xil_signed16* add_consts;
    op->getOpList()[1]->getParam(1, (void **)&add_consts);

    XilSystemState* state = op->getDstImage(1)->getSystemState();
    
    //
    //  Get the cache number we can use...
    //
    *compute_data =
        (void*)deviceManager->getRescaleTable(state, 1.0F, (float)*add_consts);

    return XIL_SUCCESS;
}

XilStatus
XilDeviceIOcg6::addConstDisplayPost(XilOp* ,
                                   void*  compute_data)
{
    deviceManager->releaseRescaleTable((int)compute_data);

    return XIL_SUCCESS;
}

XilStatus
XilDeviceIOcg6::addConstDisplay(XilOp*       op,
                               unsigned int ,
                               XilRoi*      roi,
                               XilBoxList*  bl)
{
    int table = (int)op->getPreprocessData(this);
    if(table == -1) {
        return XIL_FAILURE;
    }

    return lookup8to8(op, roi, bl, deviceManager->refRescaleCache(table));
}

////////////////////////////////////////////////////////////////////////////
//
//  ADD CONST
//
////////////////////////////////////////////////////////////////////////////
XilStatus
XilDeviceIOcg6::mulConstDisplayPre(XilOp*        op,
                                  unsigned      ,
                                  XilRoi*       ,
                                  void**        compute_data,
                                  unsigned int* )
{
    //
    //  Get the mulConst for this operation.
    //
    float* mul_consts;
    op->getOpList()[1]->getParam(1, (void**)&mul_consts);

    XilSystemState* state = op->getDstImage(1)->getSystemState();
    
    //
    //  Get the cache number we can use...
    //
    *compute_data =
        (void*)deviceManager->getRescaleTable(state, *mul_consts, 0.0F);

    return XIL_SUCCESS;
}

XilStatus
XilDeviceIOcg6::mulConstDisplayPost(XilOp* ,
                                   void*  compute_data)
{
    deviceManager->releaseRescaleTable((int)compute_data);

    return XIL_SUCCESS;
}

XilStatus
XilDeviceIOcg6::mulConstDisplay(XilOp*       op,
                               unsigned int ,
                               XilRoi*      roi,
                               XilBoxList*  bl)
{
    int table = (int)op->getPreprocessData(this);
    if(table == -1) {
        return XIL_FAILURE;
    }

    return lookup8to8(op, roi, bl, deviceManager->refRescaleCache(table));
}


