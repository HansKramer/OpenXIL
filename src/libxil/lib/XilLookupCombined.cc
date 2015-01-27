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
//  File:    XilLookupCombined.cc
//  Project:    XIL
//  Revision:    1.17
//  Last Mod:    10:08:49, 03/10/00
//
//  Description:
//    Implementation of XilLookupCombined class
//    
//    
//------------------------------------------------------------------------
//  COPYRIGHT
//------------------------------------------------------------------------
#pragma ident   "@(#)XilLookupCombined.cc	1.17\t00/03/10  "
 
//
//  System Includes
//
#include <memory.h>
#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

//
//  C++ Includes
//
#include "_XilDefines.h"
#include "_XilLookupCombined.hh"
#include "_XilSystemState.hh"
#include "XiliUtils.hh"

XilLookupCombined::~XilLookupCombined()
{
    for(unsigned int i=0; i<inputNBands; i++) {
        //
        //  Destroying the XilLookup will take care of deleting the data
        //  pointer we aquired via getData() in our dataList.
        //
        lutList[i]->destroy();
    }

    delete [] dataList;
    delete [] offsetsList;
    delete [] entriesList;
    delete [] lutList;
}

//
// Constructor for a combined lookup.
// Build from a list of XilLookupSingle objects. 
// Bounds checking will already have been performed on the
// individual lookup objects.
//
XilLookupCombined::XilLookupCombined(XilSystemState*  system_state,
                                     XilLookupSingle* list[],
                                     unsigned int     num_lookups)
: XilLookup(system_state)
{
    //
    // Verify base class OK
    //
    if(! isOKFlag) {
        return;
    }

    isOKFlag = FALSE;
    
    lookupType = XIL_LOOKUP_COMBINED;

    this->entriesList = NULL;
    this->offsetsList = NULL;
    this->dataList    = NULL;
    
    if(num_lookups == 0 || list == NULL) {
        XIL_ERROR(system_state, XIL_ERROR_USER, "di-331", TRUE);
        return;
    }
    
    //
    // Verify that all luts are single-band with the same in/out types
    //
    inputType  = list[0]->getInputDataType();
    outputType = list[0]->getOutputDataType();
    bytesPerBand  = xili_sizeof(outputType);
    bytesPerEntry = xili_sizeof(outputType);

    unsigned int i;
    for(i=0; i<num_lookups; i++) {
        if(list[i]->getInputDataType() != inputType) {
            XIL_ERROR(system_state, XIL_ERROR_USER, "di-332", TRUE);
            return;
        }

        if(list[i]->getOutputDataType() != outputType) {
            XIL_ERROR(system_state, XIL_ERROR_USER, "di-333", TRUE);
            return;
        }

        if(list[i]->getOutputNBands() != 1) {
            XIL_ERROR(system_state, XIL_ERROR_USER, "di-334", TRUE);
            return;
        }
    }

    //
    //  Each lut is a one band to one band table
    //
    inputNBands   = num_lookups;
    outputNBands  = num_lookups;

    //
    //  Set up the extents of the lookup
    //
    offsetsList = new int[num_lookups];
    entriesList = new unsigned int[num_lookups];
    dataList    = new (const void*[num_lookups]);
    lutList     = new (XilLookupSingle*[num_lookups]);


    for(i=0; i<num_lookups; i++) {
        offsetsList[i] = list[i]->getOffset();
        entriesList[i] = list[i]->getNumEntries();
    }
    
    for(i=0; i<num_lookups; i++) {
        lutList[i] = (XilLookupSingle*) list[i]->createCopy();
        if( ! lutList[i]->isOK()) {
            XIL_ERROR(system_state, XIL_ERROR_RESOURCE, "di-1", TRUE);
            return;
        }
        dataList[i] = lutList[i]->getData();
    }


    isOKFlag = TRUE;
}

const unsigned int* 
XilLookupCombined::getEntriesList()
{
    return entriesList;
}

const int* 
XilLookupCombined::getOffsetsList()
{
    return offsetsList;
}

const void** 
XilLookupCombined::getDataList()
{
    return dataList;
}

//
//  Create a single lookup from the information contained in a single
//   band of a combined lookup.
//
XilLookupSingle* 
XilLookupCombined::getBandLookup(unsigned int band_num)
{
    if(band_num >= inputNBands) {
        XIL_ERROR(getSystemState(), XIL_ERROR_USER, "di-335", TRUE);
        return NULL;
    }

    return (XilLookupSingle*)(this->lutList[band_num]->createCopy());
}

XilLookup*
XilLookupCombined::convert(XilLookup*)
{
    // convert not valid for combined lookup tables
    XIL_OBJ_ERROR(getSystemState(), XIL_ERROR_USER, "di-398", TRUE,
                  (XilObject*)this);
    return NULL;
}

//
// Create a complete copy of the combined lookup 
//
XilObject* 
XilLookupCombined::createCopy()
{
    XilLookupCombined* new_lut =
        getSystemState()->createXilLookupCombined(lutList, inputNBands);
    
    if(new_lut == NULL) {
        XIL_ERROR(this->getSystemState(), XIL_ERROR_SYSTEM, "di-177", FALSE);
        return NULL;
    }

    //
    //  Give the copy the same version number
    //
    new_lut->copyVersionInfo(this);

    return new_lut;
}

