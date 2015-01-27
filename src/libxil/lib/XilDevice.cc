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
//  File:	XilDevice.cc
//  Project:	XIL
//  Revision:	1.12
//  Last Mod:	10:08:44, 03/10/00
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
//  MT-level:  UNSAFE
//	
//------------------------------------------------------------------------
//	COPYRIGHT
//------------------------------------------------------------------------
#pragma ident	"@(#)XilDevice.cc	1.12\t00/03/10  "

//
//  System includes
//
#include <string.h>
#include <stdlib.h>

//
//  XIL Includes
//
#include "_XilDefines.h"
#include "_XilSystemState.hh"
#include "_XilDevice.hh"
#include "XiliUtils.hh"

const unsigned int _XILI_INITIAL_ATTR_TBL_SIZE = 8;

//
//  Constructors...
//
XilDevice::XilDevice(XilSystemState* state,
                     const char*     device_name) :
    XilNonDeferrableObject(state, XIL_DEVICE),
    deviceName(strdup(device_name))
{
    isOKFlag = FALSE;
    arraySize = 0;
    numAttrs  = 0;
    attrTable = NULL;
    isOKFlag = TRUE;
}

XilDevice::~XilDevice()
{
    free(deviceName);
}

const char*
XilDevice::getDeviceName()
{
    return deviceName;
}

const XilAttributeData*
XilDevice::getAttributes(unsigned int* num_attrs)
{
    if(num_attrs == NULL) {
        XIL_ERROR(getSystemState(), XIL_ERROR_USER, "di-259", TRUE);
        return NULL;
    }

    *num_attrs = numAttrs;

    return attrTable;
}


void
XilDevice::setAttribute(const char* attr_name,
                        void*       attr_value)
{
    if(attr_name == NULL) {
        XIL_ERROR(getSystemState(), XIL_ERROR_USER, "di-259", TRUE);
        return;
    }

    if(arraySize == 0) {
        //
        //  Allocate a new table.
        //
        attrTable = new XilAttributeData[_XILI_INITIAL_ATTR_TBL_SIZE];

        if(attrTable == NULL) {
            XIL_ERROR(getSystemState(), XIL_ERROR_RESOURCE, "di-1", TRUE);
            return;
        }

        arraySize = _XILI_INITIAL_ATTR_TBL_SIZE;
    } else if(numAttrs == arraySize) {
        //
        //  Allocate a new table of 2x size and 
        //  copy the old data into the new table.
        //
        XilAttributeData* tmp_table = new XilAttributeData[arraySize<<1];

        if(tmp_table == NULL) {
            XIL_ERROR(getSystemState(), XIL_ERROR_RESOURCE, "di-1", TRUE);
            return;
        }

        xili_memcpy(tmp_table, attrTable,
                    sizeof(XilAttributeData)*arraySize);

        //
        // Now delete the old table and set attrTable ptr 
        // to point to the new table .
        //
        delete [] attrTable;
        attrTable = tmp_table;

        arraySize = arraySize<<1;
    }

    attrTable[numAttrs].name  = attr_name;
    attrTable[numAttrs].value = attr_value;

    numAttrs++;
}

XilObject*
XilDevice::createCopy()
{
    //
    //  TODO:  implement this routine.
    //
    return NULL;
}
