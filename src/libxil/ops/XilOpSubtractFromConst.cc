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
//  File:	XilOpSubtractFromConst.cc
//  Project:	XIL
//  Revision:	1.11
//  Last Mod:	10:07:17, 03/10/00
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
#pragma ident	"@(#)XilOpSubtractFromConst.cc	1.11\t00/03/10  "

#include <stdlib.h>

#include <xil/xilGPI.hh>
#include "XilOpPoint.hh"
#include "XiliOpUtils.hh"

class XilOpSubtractFromConst : public XilOpPoint {
public:
    static XilOp* create(char* function_name,
                         void* args[],
                         int count);
    
protected:
    XilOpSubtractFromConst(XilOpNumber op_num);
    virtual ~XilOpSubtractFromConst();
};


XilOp*
XilOpSubtractFromConst::create(char  function_name[],
                  void* args[],
                  int)
{
    static XilOpCache  subtractfromconst_op_cache;
    float*             values = (float*)args[0];
    XilImage*          src    = (XilImage*)args[1];
    XilImage*          dst    = (XilImage*)args[2];
    
    XilOpNumber opnum;
    if((opnum = xili_verify_op_args(function_name, &subtractfromconst_op_cache,
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
    // Copy and round the values passed into the appropriate
    // type of array for that data type. We need to make sure
    // that arithmetic can be performed correctly, hence byte data
    // is converted to short arrays.
    //
    unsigned int    nbands = dst->getNumBands();
    XilDataType     dst_data_type = dst->getDataType();
    XilDataType     new_data_type;

    switch(dst_data_type) {
      case XIL_BIT:
      case XIL_UNSIGNED_4:
	new_data_type = XIL_SIGNED_8;
	break;
      case XIL_SIGNED_8:
      case XIL_BYTE:
	new_data_type = XIL_SHORT;
	break;
      case XIL_SHORT:
      case XIL_UNSIGNED_16:
      case XIL_SIGNED_32:
      case XIL_UNSIGNED_32:
	new_data_type = XIL_SIGNED_32;
	break;
      default:
	new_data_type = XIL_FLOAT;
	break;
    }
    
    void* const_array = xili_round_op_values(new_data_type,
                                             values,
                                             nbands);
    if(const_array == NULL) {
	XIL_ERROR(dst->getSystemState(), XIL_ERROR_RESOURCE, "di-1", TRUE);
	return NULL;
    }

    //
    // Now clamp to the appropriate range, for non-float
    // data types. Note that because we subtract_from_const
    // the ranges are different from subtract_const.
    // eg 400-255 = 145, produces a valid result. Hence the
    // ranges for the types are 0 to 2*maxval, for the byte
    // case. Short is a signed type and the range is -65536
    // to 65534.
    //
    unsigned int i;
    switch(dst_data_type) {
      // Tested cases
      case XIL_BIT: {
	Xil_signed8* array = (Xil_signed8*)const_array;
	for(i=0; i<nbands; i++) {
	    if((int)array[i] < 0) {
		array[i] = 0;
	    } else if(array[i] > 2) {
		array[i] = 2;
	    }
	}
	break; }
      case XIL_UNSIGNED_4: {
	Xil_signed8* array = (Xil_signed8*)const_array;
	for(i=0; i<nbands; i++) {
	    if((int)array[i] < 0) {
		array[i] = 0;
	    } else if(array[i] > 30) {
		array[i] = 1;
	    }
	}
	break; }
      case XIL_BYTE:
      case XIL_SIGNED_8: {
	Xil_signed16* array = (Xil_signed16*)const_array;
	for(i=0; i<nbands; i++) {
	    if(array[i] < XIL_MINBYTE) {
		array[i] = XIL_MINBYTE;
	    } else if(array[i] > (2*XIL_MAXBYTE)) {
		array[i] = 510;
	    }
	}
	break; }
      case XIL_SHORT:
      case XIL_UNSIGNED_16: {
	Xil_signed32* array = (Xil_signed32*)const_array;
	for(i=0; i<nbands; i++) {
	    if(array[i] < -65536) {
		array[i] = -65536;
	    } else if(array[i] > 65534) {
		array[i] = 65534;
	    }
	}
	break; }
      default:
	break;
    }

    XilOpSubtractFromConst* op = new XilOpSubtractFromConst(opnum);
    if(op == NULL) {
	XIL_ERROR(dst->getSystemState(), XIL_ERROR_RESOURCE, "di-1", TRUE);
        return NULL;
    }

    op->setSrc(1, src);
    op->setParam(1, const_array);
    op->setDst(1, dst);

    return op;
}

XilOpSubtractFromConst::XilOpSubtractFromConst(XilOpNumber op_num) : XilOpPoint(op_num) { }
XilOpSubtractFromConst::~XilOpSubtractFromConst() { }
