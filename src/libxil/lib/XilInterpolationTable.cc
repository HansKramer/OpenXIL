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
//  File:	XilInterpolationTable.cc
//  Project:	XIL
//  Revision:	1.15
//  Last Mod:	10:09:00, 03/10/00
//
//  Description:
//	
//	
//	
//	
//	
//------------------------------------------------------------------------
//	COPYRIGHT
//------------------------------------------------------------------------
#pragma ident	"@(#)XilInterpolationTable.cc	1.15\t00/03/10  "

#include "_XilDefines.h"
#include "_XilSystemState.hh"
#include "_XilInterpolationTable.hh"

#include "XiliUtils.hh"

XilInterpolationTable::XilInterpolationTable(XilSystemState* state,
                                             unsigned int    kernel_size,
                                             unsigned int    num_subsamples,
                                             float*          init_data) :
    XilNonDeferrableObject(state, XIL_INTERPOLATION_TABLE)
{
    isOKFlag = FALSE;

    //
    //  So destructor doesn't barf.
    //
    data = NULL;

    if(init_data == NULL) {
        XIL_ERROR(getSystemState(), XIL_ERROR_USER, "di-259", TRUE);
        return;
    }

    if(kernel_size == 0) {
        XIL_ERROR(getSystemState(), XIL_ERROR_USER, "di-146", TRUE);
        return;
    }

    if(num_subsamples == 0) {
        XIL_ERROR(getSystemState(), XIL_ERROR_USER, "di-146", TRUE);
        return;
    }

    //
    //  Fill-in the members of the class.
    //
    kernelSize = kernel_size;
    subSamples = num_subsamples;

    //
    //  Copy the data given to us by the user.
    //
    unsigned int data_size = subSamples * kernelSize;

    data = new float[data_size];
    if(data == NULL) {
        XIL_ERROR(getSystemState(), XIL_ERROR_RESOURCE, "di-1", TRUE);
        return;
    }

    xili_memcpy(data, init_data, data_size*sizeof(float));
    
    isOKFlag = TRUE;
}

XilInterpolationTable::~XilInterpolationTable()
{
    delete [] data;
}

//
//  Member function implementations.
//
unsigned int
XilInterpolationTable::getNumSubsamples()
{
    return subSamples;
}

unsigned int
XilInterpolationTable::getKernelSize()
{
    return kernelSize;
}

const float*
XilInterpolationTable::getData()
{
    return data;
}

void
XilInterpolationTable::getValues(float* buffer)
{
    if(buffer == NULL) {
        XIL_ERROR(getSystemState(), XIL_ERROR_USER, "di-259", TRUE);
        return;
    }

    xili_memcpy(buffer, data, kernelSize*subSamples*sizeof(float));
}

//
//  Create a copy of the interpolation table
//
XilObject*
XilInterpolationTable::createCopy()
{
    XilInterpolationTable* new_copy =
        getSystemState()->createXilInterpolationTable(kernelSize,
                                                      subSamples, data);
    if(new_copy == NULL) {
	XIL_ERROR(this->getSystemState(), XIL_ERROR_SYSTEM, "di-442", FALSE);
	return NULL;
    }

    new_copy->copyVersionInfo(this);

    return new_copy;
}
