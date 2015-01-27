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
//  File:    XilLookup.cc
//  Project:    XIL
//  Revision:    1.25
//  Last Mod:    10:08:05, 03/10/00
//
//  Description:
//    Implementation of XilLookup Base class
//    This contains functions which are common
//    to all of the lookup table variants
//    
//    
//------------------------------------------------------------------------
//  COPYRIGHT
//------------------------------------------------------------------------
#pragma ident   "@(#)XilLookup.cc	1.25\t00/03/10  "
 
//
//  System Includes
//
#include <stdio.h>
#include <stdlib.h>

//
//  XIL Includes
//
#include "_XilDefines.h"
#include "_XilLookup.hh"
#include "_XilSystemState.hh"
#include "XiliUtils.hh"

XilLookupType
XilLookup::getLookupType()
{
    return lookupType;
}

//
// These should require no explanation 
//
XilDataType    
XilLookup::getInputDataType()
{
    return inputType;
}

XilDataType    
XilLookup::getOutputDataType()
{
    return outputType;
}

unsigned int    
XilLookup::getInputNBands()
{
    return inputNBands;
}

unsigned int    
XilLookup::getOutputNBands()
{
    return outputNBands;
}

unsigned int
XilLookup::getBytesPerEntry()
{
    return bytesPerEntry;
}

unsigned int
XilLookup::getBytesPerBand()
{
    return bytesPerBand;
}

Xil_boolean    
XilLookup::isColorcube()
{
    return isColorcubeFlag;
}

//
//  Constructors for base lookup.
//
XilLookup::XilLookup(XilSystemState* system_state,
                     XilDataType     input_type,
                     XilDataType     output_type)
: XilNonDeferrableObject(system_state, XIL_LOOKUP)
{
    isOKFlag  = FALSE;

    //
    //  Set initial values for private data members
    //
    maxSize = 0;

    //
    //  Check the input type and figure out the maximum size
    //
    switch(input_type) {
      case XIL_BIT:
      case XIL_BYTE:
      case XIL_SIGNED_8:
      case XIL_SHORT:
      case XIL_UNSIGNED_16:
        maxSize = xili_get_datatype_max(input_type) + 1;
        break;
      default: // unsupported input type
        XIL_ERROR(system_state, XIL_ERROR_USER, "di-133", TRUE);
        return;
    }

    //
    // Check the output type.
    // Since we actually have to GENERATE values of the
    // output data type to fill in the colorcube, we're
    // going to restrict the types to those which can
    // actually be represented on Solaris now. So its
    // all integer types up to 32 bits plus float and double.
    // Aggregate types (complex, etc) don't make sense.
    // TODO: Add 64 bit integer types when Solaris supports them.
    //
    switch(output_type) {
      case XIL_BIT:
      case XIL_UNSIGNED_4:
      case XIL_BYTE:
      case XIL_SIGNED_8:
      case XIL_SHORT:
      case XIL_UNSIGNED_16:
      case XIL_FLOAT:
      case XIL_FLOAT_64:
      case XIL_SIGNED_32:
      case XIL_UNSIGNED_32:
        // These types are supported
        break;
      default:
        XIL_ERROR(system_state, XIL_ERROR_USER, "di-132", TRUE);
        return;
    }

    // Set up the type of the lookup 
    inputType        = input_type;
    outputType       = output_type;

    bytesPerBand  = xili_sizeof(output_type);

    //
    //  Set this as the default - overridden by colorcube constructor
    //
    isColorcubeFlag = FALSE;

    isOKFlag = TRUE;
}


XilLookup::XilLookup(XilSystemState* system_state)
: XilNonDeferrableObject(system_state, XIL_LOOKUP)
{
    isOKFlag  = FALSE;

    //
    //  Set initial values for private data members
    //
    maxSize = 0;

    //
    //  Set this as the default - overridden by colorcube constructor
    //
    isColorcubeFlag = FALSE;

    isOKFlag = TRUE;
}

//
// Destructor
//
XilLookup::~XilLookup()
{
  // Nothing to do here.
  // All memory allocation/deallocation is done in subclasses.
}
 

