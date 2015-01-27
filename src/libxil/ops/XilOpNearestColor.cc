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
//  File:	XilOpNearestColor.cc
//  Project:	XIL
//  Revision:	1.13
//  Last Mod:	10:07:25, 03/10/00
//
//  Description:
//	XilOp object for the Lookup operation.
//	Derived from XilOpPoint. This op differs
//	from other "point" ops in that it involves
//      an auxilaiary object, the XilLookup object.
//      This contents of the Lookup object must be checked
//      for validity.
//	
//	
//------------------------------------------------------------------------
//	COPYRIGHT
//------------------------------------------------------------------------
#pragma ident	"@(#)XilOpNearestColor.cc	1.13\t00/03/10  "

#include <stdlib.h>

#include <xil/xilGPI.hh>

#include "XilOpPoint.hh"
#include "XiliOpUtils.hh"
#include "XiliUtils.hh"

class XilOpNearestColor : public XilOpPoint {
public:
    static XilOp* create(char* function_name,
                         void* args[],
                         int count);
    
protected:
    XilOpNearestColor(XilOpNumber op_num);
    virtual ~XilOpNearestColor();
};


//
//  Source and destination bands can differ do our own verification
//
XilOp*
XilOpNearestColor::create(char  function_name[],
                          void* args[],
                          int   )
{
    //
    //  Check for NULL images.
    //
    XilImage* src = (XilImage*)args[0];
    XilImage* dst = (XilImage*)args[1];
    if(dst == NULL) {
        XIL_ERROR(NULL, XIL_ERROR_USER, "di-207", TRUE);
        return NULL;
    }
    
    if(src == NULL) {
        XIL_ERROR(dst->getSystemState(), XIL_ERROR_USER, "di-207", TRUE);
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
    //  Check that the Lookup table is valid
    //
    XilLookupSingle* lookup = (XilLookupSingle*)args[2];
    if(lookup == NULL) {
        XIL_ERROR(dst->getSystemState(), XIL_ERROR_USER, "di-131", TRUE);
        return NULL;
    }

    //
    //  src datatype matches lookup input datatype
    //
    if(src->getDataType() != lookup->getOutputDataType()) {
        XIL_ERROR(dst->getSystemState(), XIL_ERROR_USER, "di-175", TRUE);
        return NULL;
    }

    //
    //  dst datatype must match that of look-up table's input.
    //
    if(dst->getDataType() != lookup->getInputDataType()) {
        XIL_ERROR(dst->getSystemState(), XIL_ERROR_USER, "di-174", TRUE);
        return NULL;
    }

    //
    //  src and lookup table output must have the same number of bands
    //
    if(src->getNumBands() != lookup->getOutputNBands()) {
        XIL_ERROR(dst->getSystemState(), XIL_ERROR_USER, "di-173", TRUE);
        return NULL;
    }

    //
    //  destination image should be single-banded
    //
    if(dst->getNumBands() != 1) {
        XIL_ERROR(dst->getSystemState(), XIL_ERROR_USER, "di-176", TRUE);
        return NULL;
    }

    static XilOpCache	nearest_color_op_cache(1);
    XilOpNumber		opnum =
        xili_check_op_cache_cast(function_name, &nearest_color_op_cache, dst, src);
    if(opnum == -1) {
	return NULL;
    }

    XilOpNearestColor* op = new XilOpNearestColor(opnum);
    if(op == NULL) {
	XIL_ERROR(dst->getSystemState(), XIL_ERROR_RESOURCE, "di-1", TRUE);
	return NULL;
    }	

    op->setSrc(1, src);
    op->setDst(1, dst);

    //
    //  Reference the lookup -- actually, setup a reference to the lookup from
    //  the op to the lookup and vice versa.  If the lookup changes before the
    //  op is executed, only then will a copy be made.
    //
    lookup = (XilLookupSingle*)(lookup->aquireDefRef(op));
    if(lookup == NULL) {
        op->destroy();
	XIL_OBJ_ERROR(lookup->getSystemState(),
                      XIL_ERROR_SYSTEM, "di-177", FALSE, lookup);
	return NULL;
    }
    op->setParam(1, lookup, XIL_RELEASE_REF);

    return op;
}

XilOpNearestColor::XilOpNearestColor(XilOpNumber op_num) : XilOpPoint(op_num) { }
XilOpNearestColor::~XilOpNearestColor() { }
