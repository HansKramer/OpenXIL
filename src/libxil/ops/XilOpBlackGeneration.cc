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
//  File:	XilOpBlackGeneration.cc
//  Project:	XIL
//  Revision:	1.7
//  Last Mod:	10:07:04, 03/10/00
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
//------------------------------------------------------------------------
//	COPYRIGHT
//------------------------------------------------------------------------
#pragma ident	"@(#)XilOpBlackGeneration.cc	1.7\t00/03/10  "

//
//  Standard Includes
//
#include <stdlib.h>

#include <xil/xilGPI.hh>
#include "XilOpPoint.hh"
#include "XiliOpUtils.hh"

class XilOpBlackGeneration : public XilOpPoint {
public:
    static XilOp* create(char* function_name,
                         void* args[],
                         int count);
    
protected:
    XilOpBlackGeneration(XilOpNumber op_num);
    virtual ~XilOpBlackGeneration();
};


XilOp*
XilOpBlackGeneration::create(char  function_name[],
                             void* args[],
                             int   )
{
    static XilOpCache	black_generation_op_cache;
    XilOpNumber		opnum;
    XilImage*           src = (XilImage*)args[0];
    XilImage*           dst = (XilImage*)args[1];

    opnum = xili_verify_op_args(function_name,
                                &black_generation_op_cache,
                                dst, src);
    if(opnum == -1) {
	return NULL;
    }

    //
    //  Check float* parameters
    //
    if((args[2] == NULL) || (args[3] == NULL)) {
	XIL_ERROR(dst->getSystemState(), XIL_ERROR_USER, "di-146", TRUE);
	return NULL;
    }

    //
    //  Verify correct number of bands...
    //
    if(src->getNumBands() != 4) {
        XIL_OBJ_ERROR(src->getSystemState(), XIL_ERROR_USER, "di-164", TRUE, src);
        return NULL;
    }

    if(dst->getNumBands() != 4) {
        XIL_OBJ_ERROR(dst->getSystemState(), XIL_ERROR_USER, "di-164", TRUE, dst);
        return NULL;
    }
    
    //
    //  Create the op and set the images and parameters
    //
    XilOpBlackGeneration* op = new XilOpBlackGeneration(opnum);
    if(op == NULL) {
	XIL_ERROR(dst->getSystemState(), XIL_ERROR_RESOURCE, "di-1", TRUE);
	return NULL;
    }

    op->setSrc(1, (XilImage*)args[0]);
    op->setDst(1, (XilImage*)args[1]);
    op->setParam(1, *(float*)args[2]);  // black
    op->setParam(2, *(float*)args[3]);  // undercolor

    return op;
}

XilOpBlackGeneration::XilOpBlackGeneration(XilOpNumber op_num) : XilOpPoint(op_num) { }
XilOpBlackGeneration::~XilOpBlackGeneration() { }
