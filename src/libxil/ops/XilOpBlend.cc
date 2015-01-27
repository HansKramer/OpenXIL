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
//  File:	XilOpBlend.cc
//  Project:	XIL
//  Revision:	1.10
//  Last Mod:	10:06:58, 03/10/00
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
#pragma ident	"@(#)XilOpBlend.cc	1.10\t00/03/10  "

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
//  Local Includes
//
#include "XilOpPoint.hh"
#include "XiliUtils.hh"

class XilOpBlend : public XilOpPoint {
public:
    static XilOp* create(char* function_name,
                         void* args[],
                         int count);
    
protected:
    XilOpBlend(XilOpNumber op_num);
    virtual ~XilOpBlend();
};


XilOp*
XilOpBlend::create(char  function_name[],
                   void* args[],
                   int   )
{
    XilImage*  src1    = (XilImage*)args[0];
    XilImage*  src2    = (XilImage*)args[1];
    XilImage*  dest    = (XilImage*)args[2];
    XilImage*  alpha   = (XilImage*)args[3];

    //
    //  Check for NULL images.
    //
    if(dest == NULL) {
        XIL_ERROR(NULL, XIL_ERROR_USER, "di-207", TRUE);
        return NULL;
    }
    
    if(src1 == NULL || src2 == NULL || alpha == NULL) {
        XIL_ERROR(dest->getSystemState(), XIL_ERROR_USER, "di-207", TRUE);
        return NULL;
    }

    //
    //  Check for invalid src images.
    //
    if(src1->isValid()  == FALSE) {
        XIL_ERROR(src1->getSystemState(), XIL_ERROR_USER, "di-327", TRUE);
        return NULL;
    }

    if(src2->isValid()  == FALSE) {
        XIL_ERROR(src2->getSystemState(), XIL_ERROR_USER, "di-327", TRUE);
        return NULL;
    }

    if(alpha->isValid() == FALSE) {
        XIL_ERROR(alpha->getSystemState(), XIL_ERROR_USER, "di-327", TRUE);
        return NULL;
    }
        

    //
    //  Check the number of bands between our images.
    //
    if(src1->getNumBands() != dest->getNumBands() ||
       src2->getNumBands() != dest->getNumBands()) {
        XIL_ERROR(dest->getSystemState(), XIL_ERROR_USER, "di-2", TRUE);
        return NULL;
    }

    //
    //  Verify the datatypes are the same between srcs and dest.
    //
    XilDataType op_datatype  = dest->getDataType();
    if(src1->getDataType() != op_datatype) {
        XIL_OBJ_ERROR(dest->getSystemState(), XIL_ERROR_USER, "di-434", TRUE, src1);
        return NULL;
    }

    if(src2->getDataType() != op_datatype) {
        XIL_OBJ_ERROR(dest->getSystemState(), XIL_ERROR_USER, "di-435", TRUE, src2);
        return NULL;
    }

    //
    //  Check the number of bands in the alpha image -- it must be 1
    //
    if(alpha->getNumBands() != 1) {
        XIL_ERROR(dest->getSystemState(), XIL_ERROR_USER, "di-165", TRUE);
        return NULL;
    }

    //
    //  Declare the op cache and indidate that there is 1 set of differences
    //  between source and destination datatypes.
    //
    static XilOpCache  blend_op_cache(1);
    XilDataType        alpha_datatype = alpha->getDataType();
    XilOpNumber        op_number      =
	blend_op_cache.lookup(op_datatype, alpha_datatype);

    if(op_number < 0) {
        //
        //  Create a temporary buffer to sprintf the name into.  It's the
        //  length of the base + 3 * the max size of a datatype string + 4 for
        //  the intermediate characters and 1 for the NULL character at the
        //  end.
        //
        char* tmpbuf = new char[strlen(function_name) +
                                4*xili_maxlen_of_datatype_string() + 6];

        sprintf(tmpbuf, "%s;%s,%s,%s->%s", function_name,
                xili_datatype_to_string(op_datatype),
                xili_datatype_to_string(op_datatype),
		xili_datatype_to_string(alpha_datatype),
		xili_datatype_to_string(op_datatype));

        //
        //  Lookup the compute op number and store the result in the
        //    given cache...
        //
        XilGlobalState* xgs = XilGlobalState::getXilGlobalState();
        if((op_number = blend_op_cache.set(op_datatype, alpha_datatype,
                                           xgs->lookupOpNumber(tmpbuf))) < 0) {
            delete tmpbuf;
            XIL_ERROR(dest->getSystemState(),
                      XIL_ERROR_CONFIGURATION, "di-5", TRUE);
            return NULL;
        }

        delete tmpbuf;
    }

    XilOpBlend* op = new XilOpBlend(op_number);
    if(op == NULL) {
	XIL_ERROR(dest->getSystemState(), XIL_ERROR_RESOURCE, "di-1", TRUE);
	return NULL;
    }

    op->setSrc(1, src1);
    op->setSrc(2, src2);
    op->setSrc(3, alpha);
    op->setDst(1, dest);

    return op;
}

XilOpBlend::XilOpBlend(XilOpNumber op_num) : XilOpPoint(op_num) { }
XilOpBlend::~XilOpBlend() { }
