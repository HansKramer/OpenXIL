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
//  File:	XilOpCopyWithPlanemask.cc
//  Project:	XIL
//  Revision:	1.12
//  Last Mod:	10:07:19, 03/10/00
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
#pragma ident	"@(#)XilOpCopyWithPlanemask.cc	1.12\t00/03/10  "

#include <stdlib.h>

#include <xil/xilGPI.hh>
#include "XilOpPoint.hh"
#include "XiliOpUtils.hh"

class XilOpCopyWithPlanemask : public XilOpPoint {
public:
    static XilOp* create(char* function_name,
                         void* args[],
                         int count);
    
protected:
    XilOpCopyWithPlanemask(XilOpNumber op_num);
    virtual ~XilOpCopyWithPlanemask();
};


XilOp*
XilOpCopyWithPlanemask::create(char  function_name[],
                               void* args[],
                               int   )
{
    static XilOpCache	copy_with_planemask_op_cache;
    XilImage*           src = (XilImage*)args[0];
    XilImage*           dst = (XilImage*)args[1];
    XilOpNumber         opnum;
    if((opnum = xili_verify_op_args(function_name,
                                    &copy_with_planemask_op_cache,
                                    dst, src)) == -1) {
        return NULL;
    }

    //
    //  Make our own internal copy of array of unsigned int.
    //
    if(args[2] == NULL) {
	XIL_ERROR(dst->getSystemState(), XIL_ERROR_RESOURCE, "d1-1", TRUE);
	return NULL;
    }

    unsigned int  nbands       = src->getNumBands();
    unsigned int* const_array  = new unsigned int[nbands];
    if(const_array == NULL) {
	XIL_ERROR(dst->getSystemState(), XIL_ERROR_RESOURCE,"di-1",TRUE);
	return NULL;
    }

    //
    // TODO: put this in utility code
    // This is how 1.2 did the clamping
    //
    unsigned int* values = (unsigned int*)args[2];
    for(unsigned int i=0; i<nbands; i++) {
	switch(((XilImage*)args[0])->getDataType()) {
	  case XIL_BIT:
	    if(values[i] >= 1) {
		const_array[i] = 1;
	    } else {
		const_array[i] = 0;
	    }
	    break;

	  case XIL_BYTE:
	    if(values[i] > XIL_MAXBYTE) {
		const_array[i] = XIL_MAXBYTE;
	    } else {
		const_array[i] = values[i];
	    }
	    break;

	  case XIL_SHORT:
	    if(values[i] > XIL_MAXSHORT) {
		const_array[i] = XIL_MAXSHORT;
            } else {
		const_array[i] = values[i];
	    }
            break;

	  default:
	    const_array[i] = values[i];
	    break;
	}
    }
    
    XilOpCopyWithPlanemask* op = new XilOpCopyWithPlanemask(opnum);
    if(op == NULL) {
	XIL_ERROR(dst->getSystemState(), XIL_ERROR_RESOURCE, "di-1", TRUE);
	return NULL;
    }


    op->setSrc(1, src);
    op->setDst(1, dst);
    op->setParam(1, const_array);

    return op;
}

XilOpCopyWithPlanemask::XilOpCopyWithPlanemask(XilOpNumber op_num) : XilOpPoint(op_num) { }
XilOpCopyWithPlanemask::~XilOpCopyWithPlanemask() { }
