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
//  File:	XilOpMultiplyConst.cc
//  Project:	XIL
//  Revision:	1.6
//  Last Mod:	10:07:19, 03/10/00
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
#pragma ident	"@(#)XilOpMultiplyConst.cc	1.6\t00/03/10  "

#include <stdlib.h>

#include <xil/xilGPI.hh>
#include "XilOpPoint.hh"
#include "XiliOpUtils.hh"

class XilOpMultiplyConst : public XilOpPoint {
public:
    static XilOp* create(char* function_name,
                         void* args[],
                         int count);
    
protected:
    XilOpMultiplyConst(XilOpNumber op_num);
    virtual ~XilOpMultiplyConst();
};


XilOp*
XilOpMultiplyConst::create(char  function_name[],
                  void* args[],
                  int)
{
    static XilOpCache  multiplyconst_op_cache;
    XilImage*          src    = (XilImage*)args[0];
    float*             values = (float*)args[1];
    XilImage*          dst    = (XilImage*)args[2];
    
    XilOpNumber opnum;
    if((opnum = xili_verify_op_args(function_name, &multiplyconst_op_cache,
                                    dst, src)) == -1) {
        return NULL;
    }

    //
    //  Verify values is ok
    //
    if(values == NULL) {
        XIL_ERROR(dst->getSystemState(), XIL_ERROR_RESOURCE, "di-259", TRUE);
        return NULL;
    }

    //
    //  Create a copy of the values
    //
    unsigned int nbands      = src->getNumBands();
    float*       const_array = new float[nbands];
    if(const_array == NULL) {
        XIL_ERROR(dst->getSystemState(), XIL_ERROR_RESOURCE,"di-1",TRUE);
        return NULL;
    }

    //
    //  Check validity of constants
    //
    for(unsigned int i=0; i<nbands; i++) {
        const_array[i] = values[i];

        if(const_array[i] != const_array[i]) {
            XIL_ERROR(dst->getSystemState(), XIL_ERROR_USER, "di-146", TRUE);
            delete const_array;
            return NULL;
        }
    }

    XilOpMultiplyConst* op = new XilOpMultiplyConst(opnum);
    if(op == NULL) {
	XIL_ERROR(dst->getSystemState(), XIL_ERROR_RESOURCE, "di-1", TRUE);
	return NULL;
    }

    op->setSrc(1, src);
    op->setParam(1, const_array);
    op->setDst(1, dst);

    return op;
}

XilOpMultiplyConst::XilOpMultiplyConst(XilOpNumber op_num) : XilOpPoint(op_num) { }
XilOpMultiplyConst::~XilOpMultiplyConst() { }
