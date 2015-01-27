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
//  File:	XilOpEdgeDetection.cc
//  Project:	XIL
//  Revision:	1.7
//  Last Mod:	10:07:51, 03/10/00
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
#pragma ident	"@(#)XilOpEdgeDetection.cc	1.7\t00/03/10  "

#include <stdlib.h>
#include <string.h>

#include <xil/xilGPI.hh>
#include "XilOpAreaKernel.hh"
#include "XiliOpUtils.hh"

class XilOpEdgeDetection : public XilOpAreaKernel {
public:
    static XilOp* create(char* function_name,
                         void* args[],
                         int   count);
    
protected:
    XilOpEdgeDetection(XilOpNumber op_num);
    virtual ~XilOpEdgeDetection();
};


//
// Create the EdgeDetection op class
//
XilOp*
XilOpEdgeDetection::create(char  function_name[],
			void* args[],
			int   )
{
    //
    //  Check the edge condition flag
    //
    XilImage*        dst = (XilImage*)args[1];
    XilEdgeDetection type = ((XilEdgeDetection)((int)args[2]));
    if(type != XIL_EDGE_DETECT_SOBEL) {
	XIL_ERROR(dst->getSystemState(), XIL_ERROR_USER, "di-343", TRUE);
	return NULL;
    }

    XilOpNumber		opnum;
    static XilOpCache	edge_detection_op_cache;
    XilImage*           src = (XilImage*)args[0];
    if((opnum = xili_verify_op_args(function_name, &edge_detection_op_cache,
                                    dst, src))== -1) {
        return NULL;
    }

    XilOpEdgeDetection* op = new XilOpEdgeDetection(opnum);
    if(op == NULL) {
	XIL_ERROR(dst->getSystemState(), XIL_ERROR_RESOURCE, "di-1", TRUE);
	return NULL;
    }

    op->setSrc(1, src);
    op->setDst(1, dst);
    op->setParam(1, type);

    //
    // Set the parameters for later use, must be done after src image
    // is set up.
    //
    if(type == XIL_EDGE_DETECT_SOBEL) {

        //
        //  If the source is smaller in either dimension than the kernel
        //  we must mark it as a special case
        //
        Xil_boolean small_source_flag = FALSE;
        
        if((src->getWidth() < 3) ||
           (src->getHeight() < 3)) {
            small_source_flag = TRUE;
        }

	//
	// Sobel is a 3*3 kernel with a key value of 1, 1, edge
	// type is EDGE_EXTEND
	//
	op->initializeAreaKernel(3, 3, 1, 1, XIL_EDGE_EXTEND, small_source_flag);
    }

    return op;
}

XilOpEdgeDetection::XilOpEdgeDetection(XilOpNumber op_num) : XilOpAreaKernel(op_num) { }
XilOpEdgeDetection::~XilOpEdgeDetection() { }
