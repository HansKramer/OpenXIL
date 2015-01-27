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
//  File:	XilOpColorCorrect.cc
//  Project:	XIL
//  Revision:	1.8
//  Last Mod:	10:07:51, 03/10/00
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
#pragma ident	"@(#)XilOpColorCorrect.cc	1.8\t00/03/10  "

#include <stdlib.h>
#include "XilOpColorCorrect.hh"
#include "XilOpColorConvert.hh"

XilOp*
XilOpColorCorrect::create(char  function_name[],
			  void* args[],
			  int   )
{
    XilImage* src = (XilImage*)args[0];
    XilImage* dst = (XilImage*)args[1];

    //
    //  Check for NULL images.
    //
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
    //  Check for invalid dst image.
    //
    if(dst->isValid() == FALSE) {
        XIL_ERROR(src->getSystemState(), XIL_ERROR_USER, "di-327", TRUE);
        return NULL;
    }

    //
    //  Check the datatype compatibility of images.
    //
    if(src->getDataType() != dst->getDataType()) {
	XIL_OBJ_ERROR(src->getSystemState(),
                      XIL_ERROR_USER,
                      "di-434",
                      TRUE,
                      src);
        return NULL;
    }
    
    //
    // Get the colorspace list passed in
    //
    XilColorspaceList* cspaceList = (XilColorspaceList*)args[2];

    if(cspaceList == NULL) {
	XIL_ERROR(src->getSystemState(), XIL_ERROR_USER, "di-372", TRUE);
        return NULL;
    }

    int nspaces = cspaceList->getNumColorspaces();
    if(nspaces == 0) {
        //
        // Application should never do this. But to be on the
        // safe side.
        //
	    XIL_ERROR(src->getSystemState(), XIL_ERROR_USER, "di-372", TRUE);
	    return NULL;
    }

    //
    // Check all the colorspaces in the list to be non NULL
    //
    for(int i=0; i<nspaces; i++) {
	if(cspaceList->getColorspace(i) == NULL) {
	    XIL_ERROR(src->getSystemState(), XIL_ERROR_USER, "di-373", TRUE);
	    return NULL;
	}
    }

    //
    // Can we just call color_convert
    //
    if((nspaces == 2) &&
       ((cspaceList->getColorspace(0))->getColorspaceType()
        == XIL_COLORSPACE_NAME) &&
       ((cspaceList->getColorspace(1))->getColorspaceType()
        == XIL_COLORSPACE_NAME)) {
	return XilOpColorConvert::create("color_convert", args, 3);
    }
    
    //
    // TODO : Venu. 04/15/97
    //
    // ColorCorrect currently is supported for only XIL_BYTE
    // at this point. Reason being KCMS cannot handle any other
    // datatype besides XIL_BYTE. If and when KCMS supports other
    // datatypes, then this can be deleted.
    //
    // Also, we are checking here rather than earlier because
    // there is a possibility that we could have done color convert
    // (previous condition) which is valid for most datatypes.
    //
    // We need only check for source datatype because datatype compatibility
    // between source and destination is already done earlier.
    //
    if(src->getDataType() != XIL_BYTE) {
        XIL_OBJ_ERROR(src->getSystemState(),
                      XIL_ERROR_USER,
                      "di-134",
                      TRUE,
                      src);
        return NULL;
    }
    
    //
    // Check that the number of bands in the first and last
    // entries match source and destination bands
    //
    XilColorspace* first = cspaceList->getColorspace(0);
    XilColorspace* last = cspaceList->getColorspace(nspaces-1);
       
    if(first->getNBands() != src->getNumBands()) {
	XIL_OBJ_ERROR(src->getSystemState(),
                      XIL_ERROR_USER,
                      "di-295",
                      TRUE,
                      first);
	return NULL;
    }
    if(last->getNBands() != dst->getNumBands()) {
	XIL_OBJ_ERROR(src->getSystemState(),
                      XIL_ERROR_USER,
                      "di-295",
                      TRUE,
                      last);
	return NULL;
    }
        
    static XilOpCache  color_correct_op_cache;
    XilOpNumber        opnum = xili_check_op_cache(function_name,
						   &color_correct_op_cache,
						   dst);
    XilOpColorCorrect* op = new XilOpColorCorrect(opnum);
    if(op == NULL) {
	XIL_ERROR(dst->getSystemState(), XIL_ERROR_RESOURCE, "di-1", TRUE);
	return NULL;
    }

    op->setSrc(1,src);
    op->setDst(1,dst);

    cspaceList = (XilColorspaceList*)cspaceList->aquireDefRef(op);
    if(cspaceList == NULL) {
        op->destroy();
	XIL_ERROR(dst->getSystemState(), XIL_ERROR_RESOURCE, "di-1", FALSE);
	return NULL;
    }	

    op->setParam(1, cspaceList, XIL_RELEASE_REF);

    return op;
}

XilOpColorCorrect::XilOpColorCorrect(XilOpNumber op_num) : XilOpPoint(op_num) { }
XilOpColorCorrect::~XilOpColorCorrect() { }
