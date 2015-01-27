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
//  File:	XilOpErode.cc
//  Project:	XIL
//  Revision:	1.14
//  Last Mod:	10:07:33, 03/10/00
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
#pragma ident	"@(#)XilOpErode.cc	1.14\t00/03/10  "

#include <stdlib.h>
#include <string.h>

#include <xil/xilGPI.hh>
#include "XilOpAreaKernel.hh"
#include "XiliOpUtils.hh"

class XilOpErode : public XilOpAreaKernel {
public:
    static XilOp* create(char* function_name,
                         void* args[],
                         int count);
    
protected:
    XilOpErode(XilOpNumber op_num);
    virtual ~XilOpErode();
};


//
// Create the erode op class
//
XilOp*
XilOpErode::create(char  function_name[],
		   void* args[],
		   int)
{
    static XilOpCache	erode_op_cache;
    XilImage*           src = (XilImage*)args[0];
    XilImage*           dst = (XilImage*)args[1];

    XilOpNumber		opnum;						   
    if((opnum = xili_verify_op_args(function_name, &erode_op_cache,
                                    dst, src))== -1) {
        return NULL;
    }

    //
    //  Check the parameters for NULL
    //
    if(args[2] == NULL) {
	XIL_ERROR(dst->getSystemState(), XIL_ERROR_USER, "di-146", TRUE);
	return NULL;
    }

    XilOpErode* op = new XilOpErode(opnum);
    if(op == NULL) {
	XIL_ERROR(dst->getSystemState(), XIL_ERROR_RESOURCE, "di-1", TRUE);
	return NULL;
    }

    //
    //  Reference the Sel -- actually, setup a reference to the sel from
    //  the op to the sel and vice versa.  If the sel changes before the
    //  op is executed, only then will a copy be made.
    //
    XilSel* sel = (XilSel*)args[2];
    sel = (XilSel*)(sel->aquireDefRef(op));
    if(sel == NULL) {
        op->destroy();
	XIL_OBJ_ERROR(dst->getSystemState(),
                      XIL_ERROR_SYSTEM, "di-191", FALSE, sel);
	return NULL;
    }

    op->setSrc(1, src);
    op->setDst(1, dst);
    op->setParam(1, sel, XIL_RELEASE_REF);

    //
    //  If the source is smaller in either dimension than the sel
    //  we must mark it as a special case
    //
    Xil_boolean small_source_flag = FALSE;

    if((src->getWidth() < sel->getWidth()) ||
       (src->getHeight() < sel->getHeight())) {
        small_source_flag = TRUE;
    }
        

    //
    // Set the parameters for later use, make sure image source is set
    // first.
    //
    op->initializeAreaKernel(sel->getWidth(), sel->getHeight(),
			     sel->getKeyX(), sel->getKeyY(),
			     XIL_EDGE_EXTEND, small_source_flag);
    return op;
}

XilOpErode::XilOpErode(XilOpNumber op_num) : XilOpAreaKernel(op_num) { }
XilOpErode::~XilOpErode() { }
