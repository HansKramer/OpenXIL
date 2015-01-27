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
//  File:	XilOpSetValue.cc
//  Project:	XIL
//  Revision:	1.10
//  Last Mod:	10:07:11, 03/10/00
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
#pragma ident	"@(#)XilOpSetValue.cc	1.10\t00/03/10  "

#include <xil/xilGPI.hh>
#include "XilOpSetValue.hh"
#include "XilOpPoint.hh"
#include "XiliOpUtils.hh"


XilOp*
XilOpSetValue::create(char  function_name[],
                  void* args[],
                  int)
{
    static XilOpCache	set_value_op_cache;
    XilOpNumber		opnum;
    XilImage*           dst    = (XilImage*)args[0];
    float*              values = (float*)args[1];

    if((opnum = xili_verify_op_args(function_name,
                                    &set_value_op_cache, dst)) == -1) {
        return NULL;
    }

    //
    //  Check the values
    //
    if(values == NULL) {
	XIL_ERROR(dst->getSystemState(), XIL_ERROR_USER, "di-259", TRUE);
	return NULL;
    }

    //
    // Copy and round the values passed into the appropriate
    // type of array for that data type
    //
    void* const_array = xili_round_op_values(dst->getDataType(),
                                             values,
                                             dst->getNumBands());
    if(const_array == NULL) {
	XIL_ERROR(dst->getSystemState(), XIL_ERROR_RESOURCE, "di-1", TRUE);
	return NULL;
    }

    //
    //  Now create the op and set the required pieces
    //
    XilOpSetValue* op = new XilOpSetValue(opnum);
    if(op == NULL) {
	XIL_ERROR(dst->getSystemState(), XIL_ERROR_RESOURCE, "di-1", TRUE);
	return NULL;
    }
    
    op->setDst(1, dst);
    op->setParam(1, const_array);

    // Return the op
    return op;
}

XilOpSetValue::XilOpSetValue(XilOpNumber op_num) : XilOpPoint(op_num) { }
XilOpSetValue::~XilOpSetValue() { }
