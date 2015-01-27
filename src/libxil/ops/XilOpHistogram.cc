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
//  File:	XilOpHistogram.cc
//  Project:	XIL
//  Revision:	1.16
//  Last Mod:	10:07:14, 03/10/00
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
#pragma ident	"@(#)XilOpHistogram.cc	1.16\t00/03/10  "

#include <stdlib.h>
#include <string.h>

#include <xil/xilGPI.hh>
#include "XilOpDataCollect.hh"
#include "XiliOpUtils.hh"

class XilOpHistogram : public XilOpDataCollect {
public:
    static XilOp* create(char* function_name,
                         void* args[],
                         int   count);

    XilStatus     vReportResults(void* results[]);
    
protected:
                  XilOpHistogram(XilOpNumber   op_num,
                                 XilHistogram* histogram);
    virtual       ~XilOpHistogram();

private:
    //
    //  Pointer to the data in the histogram object
    //
    unsigned int* histo_data;

    unsigned int  histo_size;
    XilMutex      mutex;
};


XilOp*
XilOpHistogram::create(char  function_name[],
                       void* args[],
                       int   )
{
    XilImage*     src       = (XilImage*)args[0];
    XilHistogram* histogram = (XilHistogram*)args[1];
    unsigned int  skip_x    = (unsigned int)args[2];
    unsigned int  skip_y    = (unsigned int)args[3];
    
    static XilOpCache  histogram_op_cache;
    XilOpNumber opnum;
    if((opnum = xili_verify_op_args(function_name, &histogram_op_cache,
                                    src))== -1) {
        return NULL;
    }

    //
    //  Check that the histogram object is valid
    //
    if(histogram == NULL) {
        XIL_ERROR(src->getSystemState(), XIL_ERROR_USER, "di-265", TRUE);
        return NULL;
    }

    if(histogram->getNumBands() != src->getNumBands()) {
        XIL_OBJ_ERROR(histogram->getSystemState(), XIL_ERROR_USER,
                      "di-234", TRUE, histogram);
        return NULL;
    }

    //
    //  Skip values must be > 0
    //
    if((skip_x <= 0) || (skip_y <= 0)) {
        XIL_ERROR(src->getSystemState(), XIL_ERROR_USER, "di-354", TRUE);
        return NULL;
    }

    XilOpHistogram* op = new XilOpHistogram(opnum, histogram);
    if(op == NULL) {
	XIL_ERROR(src->getSystemState(), XIL_ERROR_RESOURCE, "di-1", TRUE);
        return NULL;
    }

    op->setSrc(1, src);
    op->setParam(1, histogram, XIL_DONT_DELETE);
    op->setParam(2, skip_x);
    op->setParam(3, skip_y);

    return op;
}

//
// Implement the virtual reportResults method
// We won't need a completeResults method
// Only one argument is passed here, so don't need varargs
//
XilStatus
XilOpHistogram::vReportResults(void* results[])
{
    //
    //  Lock around the histogram update
    //
    mutex.lock();

    unsigned int  count     = histo_size + 1;

    unsigned int* total     = histo_data;
    unsigned int* sub_total = (unsigned int*)results[0];

    //
    // Accumulate into the grand total
    //
    while(--count) {
        *total++ += *sub_total++;
    }

    mutex.unlock();

    return XIL_SUCCESS;
}

//
// Constructor
// Set the private data variables so we can accumulate
// the histogram results from multiple threads
//
XilOpHistogram::XilOpHistogram(XilOpNumber op_num,
                               XilHistogram* histogram)
    : XilOpDataCollect(op_num)
{
    histo_size = histogram->getNumElements();
    histo_data = (unsigned int*)histogram->getData();
}

XilOpHistogram::~XilOpHistogram() { }
