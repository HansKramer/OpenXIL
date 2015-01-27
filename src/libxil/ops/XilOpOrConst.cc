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
//  File:	XilOpOrConst.cc
//  Project:	XIL
//  Revision:	1.11
//  Last Mod:	10:06:59, 03/10/00
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
#pragma ident	"@(#)XilOpOrConst.cc	1.11\t00/03/10  "

//
//  Standard Includes
//
#include <stdlib.h>

#include <xil/xilGPI.hh>
#include "XilOpPoint.hh"
#include "XiliOpUtils.hh"

class XilOpOrConst : public XilOpPoint {
public:
    static XilOp* create(char* function_name,
                         void* args[],
                         int count);
    
protected:
    XilOpOrConst(XilOpNumber op_num);
    virtual ~XilOpOrConst();
};


XilOp*
XilOpOrConst::create(char  function_name[],
		     void* args[],
		     int)
{
    XilImage* src = (XilImage*)args[0];
    XilImage* dst = (XilImage*)args[2];
    
    //
    //  Verify the rest of the information and get an opnum
    //
    static XilOpCache	or_const_op_cache;
    XilOpNumber 	opnum =
        xili_verify_op_args(function_name, &or_const_op_cache, dst, src);
    if(opnum == -1) {
	return NULL;
    }

    //
    //  Verify that none of the images are invalid for logical operations.
    //
    if(xili_verify_op_logicals(src) == FALSE) {
	XIL_ERROR(dst->getSystemState(), XIL_ERROR_USER, "di-362", TRUE);
	return NULL;
    }
    if(xili_verify_op_logicals(dst) == FALSE) {
	XIL_ERROR(dst->getSystemState(), XIL_ERROR_USER, "di-362", TRUE);
	return NULL;
    }

    //
    //  Copy and clamp the parameters
    //
    unsigned int* values = (unsigned int*)args[1];
    if(values == NULL) {
	XIL_ERROR(dst->getSystemState(), XIL_ERROR_USER, "di-146", TRUE);
	return NULL;
    }

    XilDataType    dtype = dst->getDataType();
    void*          const_array;

    const_array = xili_clamp_op_logical(dtype, values, dst->getNumBands());
    if(const_array == NULL) {
	XIL_ERROR(dst->getSystemState(), XIL_ERROR_RESOURCE, "d1-1", TRUE);
	return NULL;
    }

    //
    //  Create an op
    //
    XilOpOrConst* op = new XilOpOrConst(opnum);
    if(op == NULL) {
	delete const_array;
	XIL_ERROR(dst->getSystemState(), XIL_ERROR_RESOURCE, "di-1", TRUE);
	return NULL;
    }

    op->setSrc(1, src);
    op->setDst(1, dst);
    op->setParam(1, const_array);

    return op;
}

XilOpOrConst::XilOpOrConst(XilOpNumber op_num) : XilOpPoint(op_num) { }
XilOpOrConst::~XilOpOrConst() { }
