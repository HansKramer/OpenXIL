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
//  File:	XilOpErrorDiffusion.cc
//  Project:	XIL
//  Revision:	1.12
//  Last Mod:	10:07:49, 03/10/00
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
#pragma ident	"@(#)XilOpErrorDiffusion.cc	1.12\t00/03/10  "

#include <stdlib.h>
#include <string.h>

#include <xil/xilGPI.hh>
#include "XilOpAreaFill.hh"
#include "XiliOpUtils.hh"

//
// We derive from AreaFill as we make use of the
// fact that it over-rides the moveIntoObjectSpace
// method to always supply storage for the whole source image.
//
class XilOpErrorDiffusion : public XilOpAreaFill {
public:
    static XilOp* create(char* function_name,
                         void* args[],
                         int count);
    
protected:
    XilOpErrorDiffusion(XilOpNumber op_num,
                        XilImage*   source);
    virtual ~XilOpErrorDiffusion();
};


//
// Create the ordered dither op class
//
XilOp*
XilOpErrorDiffusion::create(char  function_name[],
                            void* args[],
                            int)
{
    static XilOpCache  error_diffusion_op_cache(1);

    XilImage*          src    = (XilImage*)args[0];
    XilImage*          dst    = (XilImage*)args[1];
    XilLookup*         cmap   = (XilLookup*)args[2];
    XilKernel*         kernel = (XilKernel*)args[3];

    //
    //  Check for NULL Images
    //
    if(dst == NULL) {
        if(src != NULL) {
            XIL_ERROR(src->getSystemState(), XIL_ERROR_USER, "di-207", TRUE);
        } else {
            XIL_ERROR(NULL, XIL_ERROR_USER, "di-207", TRUE);
        }

	return NULL;
    }

    if(src == NULL) {
	XIL_ERROR(dst->getSystemState(),  XIL_ERROR_USER, "di-207", TRUE);
	return NULL;
    }

    //
    //  Check the lut and kernel parameters for NULL
    //
    if(cmap == NULL || kernel == NULL) {
	XIL_ERROR(dst->getSystemState(), XIL_ERROR_USER, "di-146", TRUE);
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
    //  Input datatype of colormap must be the same as the destination data
    //
    if(dst->getDataType() != cmap->getInputDataType()) {
	XIL_ERROR(dst->getSystemState(), XIL_ERROR_USER, "di-174", TRUE);
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
    //  Use "cast" routine to generate function name
    //
    XilOpNumber opnum;						   
    if((opnum = xili_check_op_cache_cast(function_name, 
                                         &error_diffusion_op_cache,
                                         dst, src)) == -1) {
	return NULL;
    }

    XilOpErrorDiffusion* op = new XilOpErrorDiffusion(opnum, src);
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
	XIL_OBJ_ERROR(cmap->getSystemState(),
                      XIL_ERROR_SYSTEM, "di-177", FALSE, cmap);
	return NULL;
    }

    //
    //  Reference the kernel
    //
    kernel = (XilKernel*)(kernel->aquireDefRef(op));
    if(kernel == NULL) {
        op->destroy();
        cmap->destroy();
	XIL_ERROR(dst->getSystemState(), XIL_ERROR_RESOURCE, "di-1", TRUE);
	return NULL;
    }

    op->setSrc(1, src);
    op->setDst(1, dst);
    op->setParam(1, cmap, XIL_RELEASE_REF);
    op->setParam(2, kernel, XIL_RELEASE_REF);

    return op;
}

XilOpErrorDiffusion::XilOpErrorDiffusion(XilOpNumber op_num,
                                         XilImage*   source) :
    XilOpAreaFill(op_num, source)
{
}

XilOpErrorDiffusion::~XilOpErrorDiffusion()
{
}
