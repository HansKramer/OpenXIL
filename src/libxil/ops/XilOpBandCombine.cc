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
//  File:	XilOpBandCombine.cc
//  Project:	XIL
//  Revision:	1.6
//  Last Mod:	10:07:44, 03/10/00
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
#pragma ident	"@(#)XilOpBandCombine.cc	1.6\t00/03/10  "

//
//  Standard Includes
//
#include <stdlib.h>

#include <xil/xilGPI.hh>
#include "XilOpPoint.hh"
#include "XiliOpUtils.hh"

class XilOpBandCombine : public XilOpPoint {
public:
    static XilOp* create(char* function_name,
                         void* args[],
                         int count);
    
protected:
    XilOpBandCombine(XilOpNumber op_num);
    virtual ~XilOpBandCombine();
};


XilOp*
XilOpBandCombine::create(char  function_name[],
			 void* args[],
			 int   )
{
    //
    //  Source and destination bands can differ do our own verification.
    //
    XilImage* src = (XilImage*)args[0];
    XilImage* dst = (XilImage*)args[1];

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
        XIL_ERROR(src->getSystemState(), XIL_ERROR_USER, "di-327", TRUE);
        return NULL;
    }

    //
    // Same datatypes
    //
    if(src->getDataType() != dst->getDataType()) {
        XIL_ERROR(dst->getSystemState(),XIL_ERROR_USER,"di-2",TRUE);
	return NULL;
    }

    //
    // Get an opnum
    //
    static XilOpCache	band_combine_op_cache;
    XilOpNumber 	opnum;
    
    opnum = xili_check_op_cache(function_name, &band_combine_op_cache, dst);
    if(opnum == -1) {
	return NULL;
    }

    //
    //  Make a copy of the parameters
    //
    if(args[2] == NULL) {
	XIL_ERROR(dst->getSystemState(), XIL_ERROR_USER, "di-146", TRUE);
	return NULL;
    }

    //
    //  Check the matrix size
    //
    XilKernel*   kernel = (XilKernel*)args[2];
    unsigned int sbands = src->getNumBands();
    unsigned int dbands = dst->getNumBands();

    if((sbands+1) != kernel->getWidth() || (dbands != kernel->getHeight())) {
	XIL_ERROR(dst->getSystemState(), XIL_ERROR_USER, "di-326", TRUE);
	return NULL;
    }

    //
    //  Set the parameters
    //
    XilOpBandCombine* op = new XilOpBandCombine(opnum);
    if(op == NULL) {
	XIL_ERROR(dst->getSystemState(), XIL_ERROR_RESOURCE, "di-1", TRUE);
	return NULL;
    }

    //
    // Set the images on the op
    //
    op->setSrc(1, src);
    op->setDst(1, dst);

    //
    //  Reference the Kernel -- actually, setup a reference to the kernel from
    //  the op to the kernel and vice versa.  If the kernel changes before the
    //  op is executed, only then will a copy be made.
    //
    kernel = (XilKernel*)(kernel->aquireDefRef(op));
    if(kernel == NULL) {
	XIL_ERROR(dst->getSystemState(),
                  XIL_ERROR_SYSTEM, "di-189", FALSE);
        op->destroy();
	return NULL;
    }

    op->setParam(1, kernel, XIL_RELEASE_REF);

    return op;
}

XilOpBandCombine::XilOpBandCombine(XilOpNumber op_num) : XilOpPoint(op_num) { }
XilOpBandCombine::~XilOpBandCombine() { }
