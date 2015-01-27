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
//  File:	XilOpOrderedDither.cc
//  Project:	XIL
//  Revision:	1.10
//  Last Mod:	10:07:41, 03/10/00
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
#pragma ident	"@(#)XilOpOrderedDither.cc	1.10\t00/03/10  "

//
//  System headers
//
#include <stdlib.h>
#include <string.h>

//
//  XIL headers
//
#include <xil/xilGPI.hh>

//
//  Local ops headers
//
#include "XilOpOrderedDither.hh"
#include "XiliOpUtils.hh"

//
// Create the ordered dither op class
//
XilOp*
XilOpOrderedDither::create(char  function_name[],
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
    //  Check for NULL parameters.
    //
    XilLookup* cmap = (XilLookup*)args[2];
    if(cmap == NULL) {
        XIL_ERROR(dst->getSystemState(), XIL_ERROR_USER, "di-131", TRUE);
        return NULL;
    }

    XilDitherMask* dmask = (XilDitherMask*)args[3];
    if(dmask == NULL) {
        XIL_ERROR(dst->getSystemState(), XIL_ERROR_USER, "di-222", TRUE);
        return NULL;
    }

    //
    //  Makesure the colormap is a colorcube
    //
    if(cmap->isColorcube() == FALSE) {
        XIL_ERROR(dst->getSystemState(), XIL_ERROR_USER, "di-206", TRUE);
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
    //  Destination image must be single banded
    //
    if(dst->getNumBands() != 1) {
	XIL_ERROR(dst->getSystemState(), XIL_ERROR_USER, "di-176", TRUE);
	return NULL;
    }

    //
    //  Number of output bands in the colormap must equal the number of 
    //  bands in the source image
    //
    if(src->getNumBands() != cmap->getOutputNBands()) {
	XIL_ERROR(dst->getSystemState(), XIL_ERROR_USER, "di-173", TRUE);
	return NULL;
    }

    //
    //  Output datatype of colormap must be the same as the source data
    //
    if(src->getDataType() != cmap->getOutputDataType()) {
	XIL_ERROR(dst->getSystemState(), XIL_ERROR_USER, "di-175", TRUE);
	return NULL;
    }

    //
    //  Input datatype of colormap must be the same as the destination data
    //
    if(dst->getDataType() != cmap->getInputDataType()) {
	XIL_ERROR(dst->getSystemState(), XIL_ERROR_USER, "di-174", TRUE);
	return NULL;
    }

    //
    //  Mask bands must match source bands
    //
    if(src->getNumBands() != dmask->getNumBands()) {
	XIL_ERROR(dst->getSystemState(), XIL_ERROR_USER, "di-251", TRUE);
	return NULL;
    }

    //
    //  Use "cast" routine to generate function name
    //
    static XilOpCache	ordered_dither_op_cache(1);
    XilOpNumber		opnum;						   
    if((opnum = xili_check_op_cache_cast(function_name, 
                                         &ordered_dither_op_cache,
                                         dst, src)) == -1) {
	return NULL;
    }

    XilOpOrderedDither* op = new XilOpOrderedDither(opnum);
    if(op == NULL) {
	XIL_ERROR(dst->getSystemState(), XIL_ERROR_RESOURCE, "di-1", TRUE);
	return NULL;
    }

    //
    //  Reference the lookup -- actually, setup a reference to the lookup from
    //  the op to the lookup and vice versa.  If the lookup changes before the
    //  op is executed, only then will a copy be made.
    //
    cmap = (XilLookup*)(cmap->aquireDefRef(op));
    if(cmap == NULL) {
        op->destroy();
	XIL_OBJ_ERROR(cmap->getSystemState(),
                      XIL_ERROR_SYSTEM, "di-177", FALSE, cmap);
	return NULL;
    }

    //
    //  Reference the dithermask
    //
    dmask = (XilDitherMask*)(dmask->aquireDefRef(op));
    if(dmask == NULL) {
        op->destroy();
        cmap->destroy();
	XIL_OBJ_ERROR(dmask->getSystemState(),
                      XIL_ERROR_SYSTEM, "di-267", FALSE, dmask);
	return NULL;
    }

    op->setSrc(1, src);
    op->setDst(1, dst);
    op->setParam(1, cmap, XIL_RELEASE_REF);
    op->setParam(2, dmask, XIL_RELEASE_REF);

    return op;
}

XilOpOrderedDither::XilOpOrderedDither(XilOpNumber op_num) : XilOpPoint(op_num) { }
XilOpOrderedDither::~XilOpOrderedDither() { }
