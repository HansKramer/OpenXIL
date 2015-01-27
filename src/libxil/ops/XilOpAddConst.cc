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
//  File:	XilOpAddConst.cc
//  Project:	XIL
//  Revision:	1.10
//  Last Mod:	10:07:10, 03/10/00
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
#pragma ident	"@(#)XilOpAddConst.cc	1.10\t00/03/10  "

#include <stdlib.h>

#include <xil/xilGPI.hh>
#include "XilOpPoint.hh"
#include "XiliOpUtils.hh"

class XilOpAddConst : public XilOpPoint {
public:
    static XilOp* create(char* function_name,
                         void* args[],
                         int count);
    
protected:
    XilOpAddConst(XilOpNumber op_num);
    virtual ~XilOpAddConst();
};


XilOp*
XilOpAddConst::create(char  function_name[],
                      void* args[],
                      int)
{
    static XilOpCache  addconst_op_cache;
    XilImage*          src    = (XilImage*)args[0];
    float*             values = (float*)args[1];
    XilImage*          dst    = (XilImage*)args[2];
    
    XilOpNumber opnum;
    if((opnum = xili_verify_op_args(function_name,
                                    &addconst_op_cache, dst, src)) == -1) {
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
    // that arithmetic can be performed correctly. ie that I can
    // subtract -255 from a byte image and have it work correctly,
    // hence the data type for BYTE images is SHORT.
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
    // data types.
    //
    unsigned int i;
    switch(dst_data_type) {
      // Tested cases
      case XIL_BIT: {
	Xil_signed8* array = (Xil_signed8*)const_array;
	for(i=0; i<nbands; i++) {
	    if((int)array[i] < -1) {
		array[i] = -1;
	    } else if(array[i] > 1) {
		array[i] = 1;
	    }
	}
	break; }
      case XIL_UNSIGNED_4: {
	Xil_signed8* array = (Xil_signed8*)const_array;
	for(i=0; i<nbands; i++) {
	    if((int)array[i] < -15) {
		array[i] = -1;
	    } else if(array[i] > 15) {
		array[i] = 1;
	    }
	}
	break; }
      case XIL_BYTE:
      case XIL_SIGNED_8: {
	Xil_signed16* array = (Xil_signed16*)const_array;
	for(i=0; i<nbands; i++) {
	    if(array[i] < -255) {
		array[i] = -255;
	    } else if(array[i] > 255) {
		array[i] = 255;
	    }
	}
	break; }
      case XIL_SHORT:
      case XIL_UNSIGNED_16: {
	Xil_signed32* array = (Xil_signed32*)const_array;
	for(i=0; i<nbands; i++) {
	    if(array[i] < -65535) {
		array[i] = -65535;
	    } else if(array[i] > 65535) {
		array[i] = 65535;
	    }
	}
	break; }
      default:
	break;
    }

    XilOpAddConst* op = new XilOpAddConst(opnum);
    if(op == NULL) {
	XIL_ERROR(dst->getSystemState(), XIL_ERROR_RESOURCE, "di-1", TRUE);
        return NULL;
    }

    op->setSrc(1, src);
    op->setParam(1, const_array);
    op->setDst(1, dst);

    return op;
}

XilOpAddConst::XilOpAddConst(XilOpNumber op_num) : XilOpPoint(op_num) { }
XilOpAddConst::~XilOpAddConst() { }
