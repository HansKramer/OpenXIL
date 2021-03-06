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
//  File:	XilOpSubsampleBinaryToGray.cc
//  Project:	XIL
//  Revision:	1.18
//  Last Mod:	10:07:38, 03/10/00
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
#pragma ident	"@(#)XilOpSubsampleBinaryToGray.cc	1.18\t00/03/10  "

#include "XilOpSubsample.hh"
#include "XilOpCopy.hh"

class XilOpSubsampleBinaryToGray : public XilOpSubsample {
public:
    static XilOp* create(char* function_name,
                         void* args[],
                         int count);

protected:
    //
    //  Subsample binary to gray has the storage characteristics of
    //  nearest neighbor scale, so we specifiy that as our interpolation type
    //  to the scale op.
    //
                  XilOpSubsampleBinaryToGray(XilOpNumber            op_num,
                                             XilImage*              src_image,
                                             XilImage*              dst_image,
                                             float                  xscale,
                                             float                  yscale) :
        XilOpSubsample(op_num, src_image, dst_image, xscale, yscale)
    {
    }


    virtual       ~XilOpSubsampleBinaryToGray()
    {
    }

    XiliRectDbl srcBoxScaledToDst;
};


XilOp*
XilOpSubsampleBinaryToGray::create(char  function_name[],
                               void* args[],
                               int   )
{
    static XilOpCache subsample_binary_to_gray_op_cache;
    XilImage*         src    = (XilImage*)args[0];
    XilImage*         dst    = (XilImage*)args[1];
    float             xscale = *((float*)args[2]);
    float             yscale = *((float*)args[3]);
    
    //
    //  Check that images are valid
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
    // Verify that src is a binary image, and dst is a byte image
    //
    if(src->getDataType() != XIL_BIT) {
        XIL_OBJ_ERROR(src->getSystemState(), XIL_ERROR_USER, "di-434", TRUE, src
);
        return NULL;
    }
    if(dst->getDataType() != XIL_BYTE) {
        XIL_OBJ_ERROR(dst->getSystemState(), XIL_ERROR_USER, "di-437", TRUE, dst
);
        return NULL;
    }

    //
    // Verify that src and dst have the same number of bands
    //
    if(dst->getNumBands() != src->getNumBands()) {
        // invalid source image
        XIL_OBJ_ERROR(src->getSystemState(), XIL_ERROR_USER, "di-2", TRUE, src
);
        return NULL;
    }

    //
    //  Scale factors must be in the range [0.0, 1.0]
    //
    if((xscale < 0.0F) || (xscale > 1.0F)) {
	XIL_ERROR(dst->getSystemState(), XIL_ERROR_USER, "di-367", TRUE);
	return NULL;
    }
    if((yscale < 0.0F) || (yscale > 1.0F)) {
	XIL_ERROR(dst->getSystemState(), XIL_ERROR_USER, "di-368", TRUE);
	return NULL;
    }

    XilOpNumber opnum;
    if((opnum = xili_check_op_cache(function_name,
                                    &subsample_binary_to_gray_op_cache,
                                    dst)) == -1) {
        return NULL;
    }
    
    //
    //  Create the op.
    //
    XilOpSubsampleBinaryToGray* op =
        new XilOpSubsampleBinaryToGray(opnum, src, dst, xscale, yscale);
    if(op == NULL) {
	XIL_ERROR(dst->getSystemState(),XIL_ERROR_RESOURCE,"di-1",TRUE);
	return NULL;
    }

    op->setSrc(1, src);
    op->setDst(1, dst);
    op->setParam(1, xscale);
    op->setParam(2, yscale);

    return op;
}
