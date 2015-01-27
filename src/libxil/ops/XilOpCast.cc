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
//  File:	XilOpCast.cc
//  Project:	XIL
//  Revision:	1.9
//  Last Mod:	10:06:55, 03/10/00
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
#pragma ident	"@(#)XilOpCast.cc	1.9\t00/03/10  "

//
//  Standard Includes
//
#include <stdlib.h>
#include <string.h>

//
//  XIL C++ Includes
//
#include <xil/xilGPI.hh>

//
//  Local C++ Includes
//
#include "XilOpPoint.hh"
#include "XilOpCopy.hh"
#include "XiliUtils.hh"

class XilOpCast : public XilOpPoint {
public:
    static XilOp* create(char* function_name,
                         void* args[],
                         int count);
    
protected:
    XilOpCast(XilOpNumber op_num);
    virtual ~XilOpCast();
};

XilOp*
XilOpCast::create(char  function_name[],
                  void* args[],
                  int   )
{
    //
    //  Declare the op cache and indidate that there is 1 set of differences
    //  between source and destination datatypes.
    //
    static XilOpCache	cast_op_cache(1);
    
    //
    //  Check for NULL images.
    //
    XilImage* src = (XilImage*)args[0];
    XilImage* dst = (XilImage*)args[1];
    if(src == NULL || dst == NULL) {
        XIL_ERROR(NULL, XIL_ERROR_USER, "di-207", TRUE);
        return NULL;
    }

    
    //
    //  Check for invalid src image.
    //
    if(src->isValid() == FALSE) {
        XIL_ERROR(src->getSystemState(), XIL_ERROR_USER, "di-327", TRUE);
        return NULL;
    }

    //
    //  Check the number of bands between our images.
    //
    if(src->getNumBands() != dst->getNumBands()) {
        XIL_ERROR(dst->getSystemState(), XIL_ERROR_USER, "di-2", TRUE);
        return NULL;
    }

    //
    //  If the src and dst have the same datatype, then just call
    //  XilOpCopy::create() to create a copy operation.
    //
    XilDataType src_datatype = src->getDataType();
    XilDataType dst_datatype = dst->getDataType();

    if(src_datatype == dst_datatype) {
        return XilOpCopy::create("copy", args, 2);
    }
    
    XilOpNumber	op_number;
    if((op_number = xili_check_op_cache_cast(function_name, 
                                             &cast_op_cache, dst, src)) == -1) {
	return NULL;
    }

    XilOpCast* op = new XilOpCast(op_number);
    if(op == NULL) {
	XIL_ERROR(dst->getSystemState(), XIL_ERROR_RESOURCE, "di-1", TRUE);
	return NULL;
    }


    op->setSrc(1, src);
    op->setDst(1, dst);

    return op;
}

XilOpCast::XilOpCast(XilOpNumber op_num) : XilOpPoint(op_num) { }
XilOpCast::~XilOpCast() { }
