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
//  File:	XilOpSoftFill.cc
//  Project:	XIL
//  Revision:	1.11
//  Last Mod:	10:07:50, 03/10/00
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
#pragma ident	"@(#)XilOpSoftFill.cc	1.11\t00/03/10  "

#include <stdlib.h>

#include <xil/xilGPI.hh>
#include "XilOpAreaFill.hh"
#include "XiliOpUtils.hh"
#include "XiliUtils.hh"

class XilOpSoftFill : public XilOpAreaFill {
public:
    static XilOp* create(char* function_name,
                         void* args[],
                         int count);
    
protected:
    XilOpSoftFill(XilOpNumber op_num,
                  XilImage*   source);
    virtual ~XilOpSoftFill();
};


XilOp*
XilOpSoftFill::create(char  function_name[],
		      void* args[],
		      int)
{
    XilImage*          src = (XilImage*)args[0];
    XilImage*          dst = (XilImage*)args[1];
    static XilOpCache  soft_fill_op_cache;
    
    XilOpNumber opnum = xili_verify_op_args(function_name,
                                            &soft_fill_op_cache,
                                            dst, src);
    if(opnum == -1) {
        return NULL;
    }

    //
    // Copy and round the xseed and yseed values.
    // Adjust the seed values for the source image
    // origins.
    //
    float        tmp;
    float        originx, originy;
    unsigned int xseed;
    unsigned int yseed;
    
    src->getOrigin(&originx, &originy);
    tmp = (float) _XILI_ROUND((*(float*)args[2]) + originx);
        if(tmp < 0) {
	xseed = 0;
    } else {
	xseed = (unsigned int)tmp;
    }

    tmp = (float) _XILI_ROUND((*(float*)args[3]) + originy);
    if(tmp < 0) {
	yseed = 0;
    } else {
	yseed = (unsigned int)tmp;
    }
    
    //
    // Copy the foreground color
    //
    unsigned int nbands = src->getNumBands();
    void*        fg_color = xili_round_op_values(src->getDataType(),
						 (float*)args[4],
						 nbands);
    if(fg_color == NULL) {
	XIL_ERROR(src->getSystemState(), XIL_ERROR_RESOURCE,"di-1",TRUE);
	return NULL;
    }

    // Number of background colors
    unsigned int    nbgcolors = (unsigned int)args[5];
    
    //
    // Copy the background colors
    //
    void* bg_color = xili_round_op_values(src->getDataType(),
					  (float*)args[6],
					  nbgcolors*nbands);
    if(bg_color==NULL) {
	XIL_ERROR(src->getSystemState(), XIL_ERROR_RESOURCE,"di-1",TRUE);
        delete fg_color;
	return NULL;
    }
    
    //
    // Copy the fill_color array
    //
    void* fill_color = xili_round_op_values(src->getDataType(),
					    (float*)args[7],
					    nbands);
    if(fill_color==NULL) {
	XIL_ERROR(src->getSystemState(), XIL_ERROR_RESOURCE,"di-1",TRUE);
        delete bg_color;
        delete fg_color;
	return NULL;
    }
    
    XilOpSoftFill* op = new XilOpSoftFill(opnum, src);
    if(op == NULL) {
	XIL_ERROR(dst->getSystemState(), XIL_ERROR_RESOURCE, "di-1", TRUE);
        delete fill_color;
        delete bg_color;
        delete fg_color;
	return NULL;
    }

    op->setSrc(1, src);
    op->setDst(1, dst);

    op->setParam(1, xseed);
    op->setParam(2, yseed);
    op->setParam(3, fg_color);
    op->setParam(4, nbgcolors);
    op->setParam(5, bg_color);
    op->setParam(6, fill_color);

    return op;
}

XilOpSoftFill::XilOpSoftFill(XilOpNumber op_num,
                             XilImage*   source) :
    XilOpAreaFill(op_num, source)
{
}

XilOpSoftFill::~XilOpSoftFill()
{
}
