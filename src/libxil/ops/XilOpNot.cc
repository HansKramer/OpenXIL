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
//  File:	XilOpNot.cc
//  Project:	XIL
//  Revision:	1.5
//  Last Mod:	10:07:00, 03/10/00
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
#pragma ident	"@(#)XilOpNot.cc	1.5\t00/03/10  "

//
//  Standard Includes
//
#include <stdlib.h>

#include <xil/xilGPI.hh>
#include "XilOpPoint.hh"
#include "XiliOpUtils.hh"

class XilOpNot : public XilOpPoint {
public:
    static XilOp* create(char* function_name,
                         void* args[],
                         int count);
    
protected:
    XilOpNot(XilOpNumber op_num);
    virtual ~XilOpNot();
};


XilOp*
XilOpNot::create(char  function_name[],
		 void* args[],
		 int)
{
    static XilOpCache	not_op_cache;
    XilOpNumber 	opnum;

    opnum = xili_verify_op_args(function_name,
			    &not_op_cache,
			    (XilImage*)args[1],
			    (XilImage*)args[0]);

    if(opnum == -1)
	return NULL;

    XilOpNot* op = new XilOpNot(opnum);

    op->setSrc(1,(XilImage*)args[0]);
    op->setDst(1,(XilImage*)args[1]);

    return op;
}

XilOpNot::XilOpNot(XilOpNumber op_num) : XilOpPoint(op_num) { }
XilOpNot::~XilOpNot() { }
