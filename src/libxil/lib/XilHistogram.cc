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
//  File:       XilHistogram.cc
//  Project:    XIL
//  Revision:   1.33
//  Last Mod:   10:08:16, 03/10/00
//
//  Description:
//    Implementation of XilHistogram class
//
//
//------------------------------------------------------------------------
//  COPYRIGHT
//------------------------------------------------------------------------
#pragma ident   "@(#)XilHistogram.cc	1.33\t00/03/10  "

//
//  System Includes
//
#include <limits.h>

//
//  C++ Includes
//
#include "_XilDefines.h"
#include "_XilHistogram.hh"
#include "_XilLookupSingle.hh"
#include "_XilSystemState.hh"
#include "XiliUtils.hh"

XilHistogram::~XilHistogram()
{
    delete [] nBins;
    delete [] lowValue;
    delete [] highValue;
    delete [] data;
}

XilHistogram::XilHistogram(XilSystemState* system_state,
                           unsigned int    nbands,
                           unsigned int*   nbins,
                           float*          low_value,
                           float*          high_value)
: XilNonDeferrableObject(system_state, XIL_HISTOGRAM)
{
    isOKFlag  = FALSE;
    
    data      = NULL;
    lowValue  = NULL;
    highValue = NULL;
    nBins     = NULL;

    if((high_value == NULL) || (low_value == NULL) || (nbins == NULL)) {
        XIL_ERROR(system_state, XIL_ERROR_USER, "di-259", TRUE);
        return;
    }
    
    if((nbands == 0) || (nbands > 65535)) {
        XIL_ERROR(system_state, XIL_ERROR_USER, "di-146", TRUE);
        return;
    }
 
    this->nBands = nbands;
 
    //
    //  Create and load the per-band arrays
    //
    nBins     = new unsigned int[nbands];
    if(nBins==NULL) {
        XIL_ERROR(system_state, XIL_ERROR_RESOURCE,"di-1",TRUE);
        return;
    }
    lowValue  = new float[nbands];
    if(lowValue==NULL) {
        delete [] nBins;
        XIL_ERROR(system_state, XIL_ERROR_RESOURCE,"di-1",TRUE);
        return;
    }
    highValue = new float[nbands];
    if(highValue==NULL) {
        delete [] nBins;
        delete [] lowValue;
        XIL_ERROR(system_state, XIL_ERROR_RESOURCE,"di-1",TRUE);
        return;
    }
 
    xili_memcpy(this->nBins, nbins, nbands*sizeof(unsigned int));
    xili_memcpy(this->lowValue, low_value, nbands*sizeof(float));
    xili_memcpy(this->highValue, high_value, nbands*sizeof(float));
 
    //
    //  Compute the size of the data array and create it
    //
    unsigned int array_size = 1;
    for(unsigned int i = 0; i<nbands; i++) {
        if((INT_MAX/sizeof(unsigned int)/nbins[i]) < array_size) {
            XIL_ERROR(system_state, XIL_ERROR_RESOURCE, "di-129", TRUE);
            return;
        }  
        array_size *= nbins[i];
    }
 
    //
    //  Was zero passed as a bin size?
    //
    if(array_size == 0) {
        XIL_ERROR(system_state, XIL_ERROR_OTHER,"di-235", TRUE);
        return;
    }

    //
    //  Initialize the size of the historgram
    //
    nElements = array_size;

    //
    //  Create the histogram data array
    //
    data = new unsigned int[array_size];
    if(data == NULL) {
        delete [] nBins;
        delete [] lowValue;
        delete [] highValue;
        XIL_ERROR(system_state, XIL_ERROR_RESOURCE, "di-1", TRUE);
        return;
    }
 
    //
    //  Go ahead and zero the array on creation
    //
    xili_memset(data, 0, sizeof(unsigned int)*array_size);

    isOKFlag = TRUE;
}

//
// Create a copy of the histogram
//
XilObject*
XilHistogram::createCopy()
{
    XilHistogram* new_copy =
        getSystemState()->createXilHistogram(nBands, nBins, lowValue, highValue); 
    if(new_copy == NULL) {
        XIL_ERROR(this->getSystemState(), XIL_ERROR_SYSTEM, "di-391", FALSE);
	return NULL;
    }

    // 
    // Copy the data from this histogram to the copy
    //
    xili_memcpy(new_copy->data, this->data, nElements*sizeof(data[0]));

    new_copy->copyVersionInfo(this);

    return new_copy;
}

const unsigned int*
XilHistogram::getData()
{
    return this->data;
}

void
XilHistogram::getValues(unsigned int* data_arg)
{
    if(data_arg) {
        xili_memcpy(data_arg, this->data, nElements*sizeof(unsigned int));
    }
}

unsigned int    
XilHistogram::getNumElements()
{
    return nElements;
}

unsigned int    
XilHistogram::getNumBands()
{
    return nBands;
}

//
// Support for API calls
//
void 
XilHistogram::getNumBins(unsigned int* nbins)
{
    if(nbins) {
       xili_memcpy(nbins, this->nBins, nBands*sizeof(unsigned int));
    }
}

void 
XilHistogram::getLowValues(float* low_values)
{
    if(low_values) {
        xili_memcpy(low_values, this->lowValue, nBands*sizeof(float));
    }
}


void 
XilHistogram::getHighValues(float* high_values)
{
    if(high_values) {
        xili_memcpy(high_values, this->highValue, nBands*sizeof(float));
    }
}

//
// These calls are used by the GPI, so we can just reference
// the data ptrs
//
const float* 
XilHistogram::getLowValues()
{
    return lowValue;
}

const float* 
XilHistogram::getHighValues()
{
    return highValue;
}

const unsigned int* 
XilHistogram::getNumBins()
{
    return nBins;
}

