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
//  File:	XilOpThreshold.cc
//  Project:	XIL
//  Revision:	1.9
//  Last Mod:	10:07:07, 03/10/00
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
#pragma ident	"@(#)XilOpThreshold.cc	1.9\t00/03/10  "

//
//  Standard Includes
//
#include <stdlib.h>

#include <xil/xilGPI.hh>
#include "XilOpPoint.hh"
#include "XiliOpUtils.hh"

class XilOpThreshold : public XilOpPoint {
public:
    static XilOp* create(char* function_name,
                         void* args[],
                         int count);
    
protected:
    XilOpThreshold(XilOpNumber op_num);
    virtual ~XilOpThreshold();
};


XilOp*
XilOpThreshold::create(char  function_name[],
		 void* args[],
		 int)
{
    static XilOpCache	threshold_op_cache;
    void*		low;
    void*		high;
    void*		map;
    XilImage*           src = (XilImage*)args[0];
    XilImage*           dst = (XilImage*)args[1];

    XilOpNumber opnum = xili_verify_op_args(function_name,
                                            &threshold_op_cache,
                                            dst, src);
    if(opnum == -1) {
	return NULL;
    }

    //
    //  Number of bands set size of parameter arrays
    //
    if((args[2] == NULL) || (args[3] == NULL) || (args[4] == NULL)) {
	XIL_ERROR(dst->getSystemState(), XIL_ERROR_USER, "di-259", TRUE);
	return (NULL);
    }
    unsigned int nbands = src->getNumBands();
    XilDataType  dtype  = src->getDataType();

    //
    //  Copy and round low values
    //
    low = xili_round_op_values(dtype, (float*)args[2], nbands);
    if(low == NULL) {
	XIL_ERROR(dst->getSystemState(), XIL_ERROR_RESOURCE,"di-1",TRUE);
	return NULL;
    }

    //
    //  Copy and round high values
    //
    high = xili_round_op_values(dtype, (float*)args[3], nbands);
    if(high == NULL) {
	XIL_ERROR(dst->getSystemState(), XIL_ERROR_RESOURCE,"di-1",TRUE);
	delete low;
	return NULL;
    }

    //
    //  Copy and round map values
    //
    map = xili_round_op_values(dtype, (float*)args[4], nbands);
    if(map == NULL) {
	XIL_ERROR(dst->getSystemState(), XIL_ERROR_RESOURCE,"di-1",TRUE);
	delete low;
	delete high;
	return (NULL);
    }

    XilOpThreshold* op = new XilOpThreshold(opnum);
    if(op == NULL) {
	XIL_ERROR(dst->getSystemState(), XIL_ERROR_RESOURCE, "di-1", TRUE);
	return NULL;
    }

    op->setSrc(1, src);
    op->setDst(1, dst);
    op->setParam(1, low);
    op->setParam(2, high);
    op->setParam(3, map);

    return op;
}

XilOpThreshold::XilOpThreshold(XilOpNumber op_num) : XilOpPoint(op_num) { }
XilOpThreshold::~XilOpThreshold() { }
