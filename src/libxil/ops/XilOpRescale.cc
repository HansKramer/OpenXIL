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
//  File:	XilOpRescale.cc
//  Project:	XIL
//  Revision:	1.6
//  Last Mod:	10:07:06, 03/10/00
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
#pragma ident	"@(#)XilOpRescale.cc	1.6\t00/03/10  "

//
//  Standard Includes
//
#include <stdlib.h>

#include <xil/xilGPI.hh>
#include "XilOpPoint.hh"
#include "XiliOpUtils.hh"

class XilOpRescale : public XilOpPoint {
public:
    static XilOp* create(char* function_name,
                         void* args[],
                         int count);
    
protected:
    XilOpRescale(XilOpNumber op_num);
    virtual ~XilOpRescale();
};


XilOp*
XilOpRescale::create(char  function_name[],
                     void* args[],
                     int   )
{

    static XilOpCache rescale_op_cache;
    XilImage*         src   = (XilImage*)args[0];
    XilImage*         dst   = (XilImage*)args[1];
    XilOpNumber       opnum = xili_verify_op_args(function_name,
                                                  &rescale_op_cache,
                                                  dst, src);
    if(opnum == -1) {
	return NULL;
    }


    //
    // Make a copy of the parameters
    //
    if((args[2] == NULL) || (args[3] == NULL)) {
	XIL_ERROR(dst->getSystemState(), XIL_ERROR_USER, "di-259", TRUE);
	return NULL;
    }
    unsigned int nbands = src->getNumBands();

    //
    //  Scale values
    //
    float* scale = new float[nbands];
    if(scale == NULL) {
	XIL_ERROR(dst->getSystemState(), XIL_ERROR_RESOURCE, "d1-1", TRUE);
	return (NULL);
    }

    float* tmp = (float*)args[2];
    for(unsigned int i=0; i<nbands; i++) {
	scale[i] = tmp[i];
    }

    //
    //  Offset values
    //
    float* offset = new float[nbands];
    if(offset == NULL) {
	XIL_ERROR(dst->getSystemState(), XIL_ERROR_RESOURCE, "d1-1", TRUE);
	delete scale;
	return NULL;
    }

    tmp = (float*)args[3];
    for(i=0; i<nbands; i++) {
	offset[i] = tmp[i];
    }

    XilOpRescale* op = new XilOpRescale(opnum);
    if(op == NULL) {
	XIL_ERROR(dst->getSystemState(), XIL_ERROR_RESOURCE, "di-1", TRUE);
	return NULL;
    }

    //
    // Set the parameters
    //
    op->setSrc(1, src);
    op->setDst(1, dst);
    op->setParam(1, scale);
    op->setParam(2, offset);

    return op;
}

XilOpRescale::XilOpRescale(XilOpNumber op_num) : XilOpPoint(op_num) { }
XilOpRescale::~XilOpRescale() { }
