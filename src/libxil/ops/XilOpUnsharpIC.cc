#include <stdio.h>
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
//  File:	XilOpUnsharpIC.cc
//  Project:	XIL
//  Revision:	
//  Last Mod:	
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
#pragma ident	"@(#)XilOpUnsharpIC.cc	1.18\t00/03/10  "

#include "_XilDefines.h"
#include <stdlib.h>
#include <string.h>

#include <xil/xilGPI.hh>
#include "XilOpAreaKernel.hh"
#include "XiliOpUtils.hh"

class XilOpUnsharpIC : public XilOpAreaKernel {
public:
    static XilOp* create(char* function_name,
                         void* args[],
                         int   count);

protected:
                  XilOpUnsharpIC(XilOpNumber op_num);
    virtual       ~XilOpUnsharpIC();

private:
};


//
// Create the convolve op class
//
XilOp*
XilOpUnsharpIC::create(char  function_name[],
                  void* args[],
                  int   argc)
{
    static XilOpCache unsharp_op_cache;

    XilImage* src = (XilImage*) args[0];
    XilImage* dst = (XilImage*) args[1];

    XilOpNumber	opnum = xili_verify_op_args(function_name, &unsharp_op_cache, dst, src);
    if (opnum == -1)
        return NULL;

    XilOpUnsharpIC* op = new XilOpUnsharpIC(opnum);
    if(op == NULL) {
        XIL_ERROR(dst->getSystemState(), XIL_ERROR_RESOURCE, "di-1", TRUE);
        return NULL;
    }

    op->setSrc(1, (XilImage*) args[0]);
    op->setDst(1, dst);

    unsigned int size = *((unsigned int *) args[2]);
    op->setParam(1, size);

    float alpha = *((float *) args[3]);
    float beta  = *((float *) args[4]);
    float gamma = *((float *) args[5]);

    XilUnsharpMaskingType type   = *((XilUnsharpMaskingType *) args[6]);
    int                   window = *((int *) args[7]); 
    int                   level  = *((int *) args[8]);

    op->setParam(2, alpha);
    op->setParam(3, beta);
    op->setParam(4, gamma);
    op->setParam(5, type);
    op->setParam(6, window);
    op->setParam(7, level);

    XilUnsharpMasking mode = dst->getSystemState()->get_unsharp_mode();
    if ((mode & XIL_UNSHARP_EDGE_OPT) == XIL_UNSHARP_EDGE_OPT) 
        size = 0;
    op->setParam(8, mode);

    op->initializeAreaKernel(size, size, size/2, size/2, XIL_EDGE_EXTEND, FALSE);

    return op;
}


XilOpUnsharpIC::XilOpUnsharpIC(XilOpNumber op_num) : XilOpAreaKernel(op_num) { }

XilOpUnsharpIC::~XilOpUnsharpIC() { }
