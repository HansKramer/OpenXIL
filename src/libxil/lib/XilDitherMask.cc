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
//  File:    XilDitherMask.cc
//  Project:    XIL
//  Revision:    1.17
//  Last Mod:    10:08:46, 03/10/00
//
//  Description:
//    Implementation of XilDitherMask class
//    
//    
//------------------------------------------------------------------------
//  COPYRIGHT
//------------------------------------------------------------------------
#pragma ident   "@(#)XilDitherMask.cc	1.17\t00/03/10  "
 
//
//  System Includes
//
#include <memory.h>
#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

//
//  C++ Inclues
//
#include "_XilDefines.h"
#include "_XilDitherMask.hh"
#include "_XilSystemState.hh"

#include "XiliUtils.hh"

XilDitherMask::~XilDitherMask()
{
    delete [] data;
}

//
//  Constructor 
//
XilDitherMask::XilDitherMask(XilSystemState* system_state,
                             unsigned int    xsize,
                             unsigned int    ysize,
                             unsigned int    nbands,
                             float*          input_data)
: XilNonDeferrableObject(system_state, XIL_DITHER_MASK)
{
    isOKFlag = FALSE;
    
    this->width       = 0;
    this->height      = 0;
    this->nBands      = 0;
    this->data        = NULL;

    //
    //  See if the XilObject construction worked
    //
    if(getSystemState() == NULL) {
        //
        //  XilObject will have already generated enough of an error.
        //
        return;
    }


    if(input_data == NULL) {
        XIL_ERROR(getSystemState(), XIL_ERROR_USER, "di-259", TRUE);
        return;
    }

    //
    //  For XIL 1.3, the upper bound on the size is not limited to 2^16.
    //
    if((xsize==0) || (ysize==0) || (nbands==0)) {
       XIL_ERROR(getSystemState(), XIL_ERROR_USER, "di-245",TRUE);
       return;
    }

    //
    //  Make sure size will not overflow 32 bits
    //
    if(INT_MAX/xsize/ysize < nbands) {
        XIL_ERROR(getSystemState(), XIL_ERROR_RESOURCE, "di-129", TRUE);
        return;
    }

    //
    //  Check for invalid values in the dithermask
    //
    for(unsigned int i=0; i<(xsize*ysize*nbands); i++) {
        if(input_data[i] != input_data[i]) {
            XIL_ERROR(getSystemState(), XIL_ERROR_USER, "di-271", TRUE);
            return;
        } else if((input_data[i] < 0.0) || (input_data[i] > 1.0)) {
            XIL_ERROR(getSystemState(), XIL_ERROR_USER, "di-272", TRUE);
            return;
        }
    }

    this->data = new float[xsize*ysize*nbands];
    if(this->data == NULL) {
        XIL_ERROR(getSystemState(), XIL_ERROR_RESOURCE, "di-1", TRUE);
        return;
    }
    memcpy(this->data, input_data, sizeof(float)*xsize*ysize*nbands);

    this->width  = xsize;
    this->height = ysize;
    this->nBands = nbands;

    //
    // AFTER THIS POINT AN ERROR MUST delete data and set it to NULL 
    //

    isOKFlag = TRUE;
}


//
// Create a copy of the dithermask
//
XilObject*
XilDitherMask::createCopy()
{
    XilDitherMask* new_copy =
        getSystemState()->createXilDitherMask(width, height, nBands, data);
    if(new_copy == NULL) {
	XIL_ERROR(getSystemState(), XIL_ERROR_SYSTEM, "di-267", FALSE);
	return NULL;
    }

    new_copy->copyVersionInfo(this);

    return new_copy;
}

unsigned int 
XilDitherMask::getWidth()
{
    return width;
}

unsigned int 
XilDitherMask::getHeight()
{
    return height;
}

unsigned int 
XilDitherMask::getNumBands()
{
    return nBands;
}

const float*
XilDitherMask::getData()
{
    return data;
}

//
// Copy the DitherMask data to the user-supplied buffer
//
void
XilDitherMask::getValues(float* data_arg)
{
    if(data_arg == NULL) {
        XIL_ERROR(getSystemState(), XIL_ERROR_USER, "di-259", TRUE);
        return;
    }

    xili_memcpy(data_arg, data, nBands*width*height*sizeof(float));
}


