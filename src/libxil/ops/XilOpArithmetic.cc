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
//This line lets emacs recognize this as -*- C++ -*- Code
//------------------------------------------------------------------------
//
//  File:	XilOpArithmetic.cc
//  Project:	XIL
//  Revision:	1.6
//  Last Mod:	10:06:57, 03/10/00
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
#pragma ident	"@(#)XilOpArithmetic.cc	1.6\t00/03/10  "

//
//  Standard Includes
//
#include <stdlib.h>

#include <xil/XilError.hh>
#include <xil/XilImage.hh>
#include <xil/XilGlobalState.hh>
#include "XilOpBasic.hh"

class XilOpArithmetic : public XilOpBasic {
public:
    static XilOp* create(char* function_name,
                         void* args[],
                         int count);
    
protected:
    XilOpArithmetic(XilOpNumber op_num);
    ~XilOpArithmetic();
};


XilOp*
XilOpArithmetic::create(char  function_name[],
                        void* args[],
                        int)
{
    static XilOpCache  add_op_cache;
    static XilOpCache  sub_op_cache;
    static XilOpCache  mul_op_cache;
    static XilOpCache  div_op_cache;
    
    XilOpNumber opnum;
    switch(function_name[0]) {
        case 'a': // add
        if((opnum = XilVerifyOpArgs(function_name,
                                    &add_op_cache,
                                    (XilImage*)args[2],
                                    (XilImage*)args[0],
                                    (XilImage*)args[1])) == -1) {
            return NULL;
        }
        break;
        case 's': // subtract
        if((opnum = XilVerifyOpArgs(function_name,
                                    &sub_op_cache,
                                    (XilImage*)args[2],
                                    (XilImage*)args[0],
                                    (XilImage*)args[1])) == -1) {
            return NULL;
        }
        break;

        case 'm': // multiply
        if((opnum = XilVerifyOpArgs(function_name,
                                    &mul_op_cache,
                                    (XilImage*)args[2],
                                    (XilImage*)args[0],
                                    (XilImage*)args[1])) == -1) {
            return NULL;
        }
        break;

        case 'd': // divide
        if((opnum = XilVerifyOpArgs(function_name,
                                    &div_op_cache,
                                    (XilImage*)args[2],
                                    (XilImage*)args[0],
                                    (XilImage*)args[1])) == -1) {
            return NULL;
        }
        break;

        default:
        XIL_ERROR(((XilImage*)args[0])->getSystemState(),
                  XIL_ERROR_INTERNAL, "di-???", TRUE);
        return NULL;
    }


    XilOpArithmetic* op = new XilOpArithmetic(opnum);

    op->setSrc(1,(XilImage*)args[0]);
    op->setSrc(2,(XilImage*)args[1]);
    op->setDst(1,(XilImage*)args[2]);

    return op;
}

XilOpArithmetic::XilOpArithmetic(XilOpNumber op_num) : XilOpBasic(op_num) { }
XilOpArithmetic::~XilOpArithmetic() { }
