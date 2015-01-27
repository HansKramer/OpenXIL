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
//  File:	XilOpAbsolute.cc
//  Project:	XIL
//  Revision:	1.6
//  Last Mod:	10:07:09, 03/10/00
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
#pragma ident	"@(#)XilOpAbsolute.cc	1.6\t00/03/10  "

#include <stdlib.h>

#include <xil/xilGPI.hh>
#include "XilOpPoint.hh"
#include "XilOpCopy.hh"
#include "XiliOpUtils.hh"

class XilOpAbsolute : public XilOpPoint {
public:
    static XilOp* create(char* function_name,
                         void* args[],
                         int count);
    
protected:
    XilOpAbsolute(XilOpNumber op_num);
    virtual ~XilOpAbsolute();
};


XilOp*
XilOpAbsolute::create(char  function_name[],
			   void* args[],
			   int)
{
    XilImage*	src = (XilImage*)args[0];
    XilImage*	dst = (XilImage*)args[1];

    //
    //  Check for NULL images.
    //
    if(src == NULL || dst == NULL) {
        XIL_ERROR(NULL, XIL_ERROR_USER, "di-207", TRUE);
        return NULL;
    }

    //
    //  Check for invalid src image.
    //
    if(src->isValid() == FALSE) {
        XIL_ERROR(dst->getSystemState(), XIL_ERROR_USER, "di-327", TRUE);
        return NULL;
    }

    //
    //  Check the number of bands and data types between our images.
    //
    if(src->getNumBands() != dst->getNumBands()) {
        XIL_ERROR(dst->getSystemState(), XIL_ERROR_USER, "di-2", TRUE);
        return NULL;
    }
    if(src->getDataType() != dst->getDataType()) {
        XIL_ERROR(dst->getSystemState(), XIL_ERROR_USER, "di-2", TRUE);
        return NULL;
    }

    //
    //  If we are dealing with unsigned data
    //  call XilOpCopy::create() to create a copy operation.
    //
    switch(src->getDataType()) {
      case XIL_BIT:
      case XIL_BYTE:
      case XIL_UNSIGNED_4:
      case XIL_UNSIGNED_16:
      case XIL_UNSIGNED_32:
        return XilOpCopy::create("copy", args, 2);
    }

    // Not a copy, create an absolute op
    XilOpNumber		opnum;
    static XilOpCache	absolute_op_cache;
    if((opnum = xili_check_op_cache(function_name, &absolute_op_cache, dst)) == -1) {
        return NULL;
    }

    XilOpAbsolute* op = new XilOpAbsolute(opnum);
    if(op == NULL) {
	XIL_ERROR(dst->getSystemState(), XIL_ERROR_RESOURCE, "di-1", TRUE);
        return NULL;
    }

    op->setSrc(1, src);
    op->setDst(1, dst);

    return op;
}

XilOpAbsolute::XilOpAbsolute(XilOpNumber op_num) : XilOpPoint(op_num) { }
XilOpAbsolute::~XilOpAbsolute() { }
