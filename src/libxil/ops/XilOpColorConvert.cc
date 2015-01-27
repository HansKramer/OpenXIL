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
//  File:	XilOpColorConvert.cc
//  Project:	XIL
//  Revision:	1.9
//  Last Mod:	10:07:53, 03/10/00
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
#pragma ident	"@(#)XilOpColorConvert.cc	1.9\t00/03/10  "

#include <stdlib.h>

#include <xil/xilGPI.hh>
#include "XilOpColorConvert.hh"
#include "XilOpColorCorrect.hh"
#include "XilOpCopy.hh"

XilOp*
XilOpColorConvert::create(char  function_name[],
			  void* args[],
			  int   argc)
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
	XIL_OBJ_ERROR(src->getSystemState(), XIL_ERROR_USER, "di-434", TRUE, src);
        return NULL;
    }

    //
    //  Check the datatype (float is not supported)
    //
    if(src->getDataType() == XIL_FLOAT ||
       dst->getDataType() == XIL_FLOAT) {
        XIL_ERROR(src->getSystemState(), XIL_ERROR_USER, "di-385", TRUE);
        return NULL;
    }
    
    //
    // Get the colorspaces of the images
    //

    //
    // We compare the argument count to distinguish if we were called
    // directly via the api or internally from color_correct.
    //
    XilColorspace* src_cs;
    XilColorspace* dst_cs;

    if(argc == 2) {
        //
	//  Called from the API
        //
	if((src_cs = src->getColorspace()) == NULL) {
	    XIL_ERROR(src->getSystemState(),XIL_ERROR_USER,"di-202",TRUE);
	    return NULL;
	}
	
	if((dst_cs = dst->getColorspace()) == NULL) {
	    XIL_ERROR(dst->getSystemState(),XIL_ERROR_USER,"di-203",TRUE);
	    src_cs->destroy();
	    return NULL;
	}

        //
        //  If either of the types are not XIL_COLORSPACE_NAME, then create a
        //  colorspace list object and call color correct.
        //
        if(src_cs->getColorspaceType() != XIL_COLORSPACE_NAME ||
           dst_cs->getColorspaceType() != XIL_COLORSPACE_NAME) {
            XilColorspace*     cspaces[2] = { src_cs, dst_cs };
            XilColorspaceList* cspacelist =
                dst->getSystemState()->createXilColorspaceList(cspaces, 2);
            if(cspacelist == NULL) {
                return NULL;
            }

            void*  ccargs[4] = { (void*)src, (void*)dst, (void*)cspacelist, 
								  NULL };
            XilOp* op        = XilOpColorCorrect::create("color_correct",
                                                         ccargs, 3);

            cspacelist->destroy();

            return op;
        }
    } else {
	//
	//  Called from color_correct, get the spaces from the list
       	//
	XilColorspaceList* cspaceList = (XilColorspaceList*)args[2];
	
	src_cs = cspaceList->getColorspace(0);
	dst_cs = cspaceList->getColorspace(1);

	if(src_cs == NULL) {
	    XIL_ERROR(src->getSystemState(),XIL_ERROR_USER,"di-202",TRUE);
	    return NULL;
	}

	if(dst_cs == NULL) {
	    XIL_ERROR(dst->getSystemState(),XIL_ERROR_USER,"di-202",TRUE);
	    src_cs->destroy();
	    return NULL;
	}
    }

    //
    // Compare colorspaces if they are the same do a copy
    //
    char* src_name = src_cs->getName();
    char* dst_name = dst_cs->getName();
    int compare = strcmp(src_name, dst_name);

    //
    //  Must use free here since XilObject::getName()
    //  uses strdup which uses malloc
    //
    free(src_name);
    free(dst_name);

    if(compare == 0) {
	if(argc == 2) {
	    // the image made a copy destroy the color spaces
	    src_cs->destroy();
	    dst_cs->destroy();
	}

	return XilOpCopy::create("copy", args, 2);
    }

    static XilOpCache  color_convert_op_cache;
    XilOpNumber        opnum = xili_check_op_cache(function_name,
						   &color_convert_op_cache,
						   dst);
    XilOpColorConvert* op = new XilOpColorConvert(opnum);
    if(op == NULL) {
	XIL_ERROR(dst->getSystemState(), XIL_ERROR_RESOURCE, "di-1", TRUE);
	src_cs->destroy();
	dst_cs->destroy();
        return NULL;
    }

    op->setSrc(1,src);
    op->setDst(1,dst);
    if(argc == 2) {
	// the image made a copy ok to destroy the color spaces
	op->setParam(1, src_cs);
	op->setParam(2, dst_cs);
    } else {
	op->setParam(1, src_cs, XIL_DONT_DELETE);
	op->setParam(2, dst_cs, XIL_DONT_DELETE);
    }
    return op;
}

XilOpColorConvert::XilOpColorConvert(XilOpNumber op_num) : XilOpPoint(op_num) { }
XilOpColorConvert::~XilOpColorConvert() { }
