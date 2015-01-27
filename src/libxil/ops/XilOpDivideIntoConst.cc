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
//  File:	XilOpDivideIntoConst.cc
//  Project:	XIL
//  Revision:	1.8
//  Last Mod:	10:07:16, 03/10/00
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
#pragma ident	"@(#)XilOpDivideIntoConst.cc	1.8\t00/03/10  "

#include <stdlib.h>

#include <xil/xilGPI.hh>
#include "XilOpPoint.hh"
#include "XiliOpUtils.hh"

class XilOpDivideIntoConst : public XilOpPoint {
public:
    static XilOp* create(char* function_name,
                         void* args[],
                         int count);
    
protected:
    XilOpDivideIntoConst(XilOpNumber op_num);
    virtual ~XilOpDivideIntoConst();
};


XilOp*
XilOpDivideIntoConst::create(char  function_name[],
                  void* args[],
                  int)
{
    static XilOpCache  divideintoconst_op_cache;
    float*             values = (float*)args[0];
    XilImage*          src    = (XilImage*)args[1];
    XilImage*          dst    = (XilImage*)args[2];
    
    XilOpNumber opnum;
    if((opnum = xili_verify_op_args(function_name, &divideintoconst_op_cache,
                                    dst, src))== -1) {
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
    //  Copy and round the values passed into the appropriate type of array
    //  for that data type.
    //
    //  We need to make sure that arithmetic can be performed correctly. ie
    //  that I can represent all of the information needed for dividing the
    //  given constant by the image's datatype.  For example, dividing by
    //  XIL_UNSIGNED_8 means the constants should be converted to
    //  XIL_UNSIGNED_16 to have it work correctly.
    //
    unsigned int    nbands = dst->getNumBands();
    XilDataType     dst_data_type = dst->getDataType();
    XilDataType     new_data_type;
    
    switch(dst_data_type) {
      case XIL_BIT:
      case XIL_UNSIGNED_4:
	new_data_type = XIL_UNSIGNED_8;
	break;
      case XIL_SIGNED_8:
	new_data_type = XIL_SIGNED_16;
        break;
      case XIL_UNSIGNED_8:
	new_data_type = XIL_UNSIGNED_16;
	break;
      case XIL_SIGNED_16:
	new_data_type = XIL_SIGNED_32;
	break;
      case XIL_UNSIGNED_16:
	new_data_type = XIL_UNSIGNED_32;
	break;

      default:
	new_data_type = XIL_FLOAT;
	break;
    }

    void* const_array = xili_round_op_values(new_data_type,
                                             values,
                                             dst->getNumBands());
    if(const_array == NULL) {
	XIL_ERROR(dst->getSystemState(), XIL_ERROR_RESOURCE, "d1-1", TRUE);
	return NULL;
    }

    XilOpDivideIntoConst* op = new XilOpDivideIntoConst(opnum);
    if(op == NULL) {
	XIL_ERROR(dst->getSystemState(), XIL_ERROR_RESOURCE, "di-1", TRUE);
	return NULL;
    }

    op->setSrc(1, src);
    op->setParam(1, const_array);
    op->setDst(1, dst);

    return op;
}

XilOpDivideIntoConst::XilOpDivideIntoConst(XilOpNumber op_num) : XilOpPoint(op_num) { }
XilOpDivideIntoConst::~XilOpDivideIntoConst() { }
