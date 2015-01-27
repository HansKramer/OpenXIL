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
//  File:	XilOpSubsampleAdaptive.cc
//  Project:	XIL
//  Revision:	1.11
//  Last Mod:	10:07:35, 03/10/00
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
#pragma ident	"@(#)XilOpSubsampleAdaptive.cc	1.11\t00/03/10  "

#include "XilOpSubsample.hh"
#include "XilOpCopy.hh"

class XilOpSubsampleAdaptive : public XilOpSubsample {
public:
    static XilOp* create(char* function_name,
                         void* args[],
                         int count);

protected:
    //
    //  Subsample adaptive has the storage characteristics of
    //  nearest neighbor scale, so we specifiy that as our interpolation type
    //  to the scale op.
    //
                  XilOpSubsampleAdaptive(XilOpNumber            op_num,
                                         XilImage*              src_image,
                                         XilImage*              dst_image,
                                         float                  xscale,
                                         float                  yscale) :
        XilOpSubsample(op_num, src_image, dst_image, xscale, yscale)
    {
    }


    virtual       ~XilOpSubsampleAdaptive()
    {
    }
};


XilOp*
XilOpSubsampleAdaptive::create(char  function_name[],
                               void* args[],
                               int   )
{
    static XilOpCache subsample_adaptive_op_cache;
    XilImage*         src    = (XilImage*)args[0];
    XilImage*         dst    = (XilImage*)args[1];
    float             xscale = *((float*)args[2]);
    float             yscale = *((float*)args[3]);
    
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

    //
    //  Can it be considered a copy?
    //
    if(XILI_FLT_EQ(xscale, 1.0F) && XILI_FLT_EQ(yscale, 1.0F)) {
	return XilOpCopy::create("copy", args, 2);
    }
    
    XilOpNumber opnum;
    if((opnum = xili_verify_op_args(function_name, &subsample_adaptive_op_cache,
                                    dst, src)) == -1) {
        return NULL;
    }
    
    //
    //  Create the op.
    //
    XilOpSubsampleAdaptive* op =
        new XilOpSubsampleAdaptive(opnum, src, dst, xscale, yscale);
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
