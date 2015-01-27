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
//  File:	XilOpOr.cc
//  Project:	XIL
//  Revision:	1.8
//  Last Mod:	10:07:02, 03/10/00
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
#pragma ident	"@(#)XilOpOr.cc	1.8\t00/03/10  "

//
//  Standard Includes
//
#include <stdlib.h>

#include <xil/xilGPI.hh>
#include "XilOpPoint.hh"
#include "XiliOpUtils.hh"

class XilOpOr : public XilOpPoint {
public:
    static XilOp* create(char* function_name,
                         void* args[],
                         int count);
    
protected:
    XilOpOr(XilOpNumber op_num);
    virtual ~XilOpOr();
};


XilOp*
XilOpOr::create(char  function_name[],
		 void* args[],
		 int)
{
    XilImage*    src1 = (XilImage*)args[0];
    XilImage*    src2 = (XilImage*)args[1];
    XilImage*    dst = (XilImage*)args[2];

    //
    //  Verify args and get an opnum
    //
    static XilOpCache	or_op_cache;
    XilOpNumber		opnum = xili_verify_op_args(function_name,
                                                    &or_op_cache,
                                                    dst, src1, src2);

    if(opnum == -1) {
        return NULL;
    }

    //
    //  Verify that none of the images are invalid for logical operations.
    //
    if(xili_verify_op_logicals(src1) == FALSE) {
	XIL_ERROR(dst->getSystemState(), XIL_ERROR_USER, "di-362", TRUE);
	return NULL;
    }

    if(xili_verify_op_logicals(src2) == FALSE) {
	XIL_ERROR(dst->getSystemState(), XIL_ERROR_USER, "di-362", TRUE);
	return NULL;
    }

    if(xili_verify_op_logicals(dst) == FALSE) {
	XIL_ERROR(dst->getSystemState(), XIL_ERROR_USER, "di-362", TRUE);
	return NULL;
    }

    XilOpOr* op = new XilOpOr(opnum);
    if(op == NULL) {
	XIL_ERROR(dst->getSystemState(), XIL_ERROR_RESOURCE, "di-1", TRUE);
	return NULL;
    }	

    op->setSrc(1, src1);
    op->setSrc(2, src2);
    op->setDst(1, dst);

    return op;
}

XilOpOr::XilOpOr(XilOpNumber op_num) : XilOpPoint(op_num) { }
XilOpOr::~XilOpOr() { }
