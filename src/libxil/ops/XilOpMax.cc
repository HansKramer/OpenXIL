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
//  File:	XilOpMax.cc
//  Project:	XIL
//  Revision:	1.4
//  Last Mod:	10:07:15, 03/10/00
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
#pragma ident	"@(#)XilOpMax.cc	1.4\t00/03/10  "

#include <stdlib.h>

#include <xil/xilGPI.hh>
#include "XilOpPoint.hh"
#include "XiliOpUtils.hh"

class XilOpMax : public XilOpPoint {
public:
    static XilOp* create(char* function_name,
                         void* args[],
                         int count);
    
protected:
    XilOpMax(XilOpNumber op_num);
    virtual ~XilOpMax();
};


XilOp*
XilOpMax::create(char  function_name[],
                  void* args[],
                  int)
{
    static XilOpCache  max_op_cache;
    
    XilOpNumber opnum;
    if((opnum = xili_verify_op_args(function_name, &max_op_cache,
                               (XilImage*)args[2],
                               (XilImage*)args[0],
			       (XilImage*)args[1]))== -1) {
        return NULL;
    }

    XilOpMax* op = new XilOpMax(opnum);

    op->setSrc(1,(XilImage*)args[0]);
    op->setSrc(2,(XilImage*)args[1]);
    op->setDst(1,(XilImage*)args[2]);

    return op;
}

XilOpMax::XilOpMax(XilOpNumber op_num) : XilOpPoint(op_num) { }
XilOpMax::~XilOpMax() { }
