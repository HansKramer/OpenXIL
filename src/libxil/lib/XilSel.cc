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
//  File:    XilSel.cc
//  Project:    XIL
//  Revision:    1.14
//  Last Mod:    10:08:12, 03/10/00
//
//  Description:
//    Implementation of XilSel class
//    
//    
//------------------------------------------------------------------------
//  COPYRIGHT
//------------------------------------------------------------------------
#pragma ident   "@(#)XilSel.cc	1.14\t00/03/10  "
 
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
#include "_XilSel.hh"
#include "_XilSystemState.hh"
#include "XiliUtils.hh"

XilSel::~XilSel()
{
    delete [] data;
}

//
//  Constructor 
//
XilSel::XilSel(XilSystemState* system_state,
               unsigned int    xsize,
               unsigned int    ysize,
               int	       key_x,
               int             key_y,
               unsigned int*   input_data)
: XilNonDeferrableObject(system_state, XIL_SEL)
{
    isOKFlag = FALSE;

    //
    //  Initialize to NULL just in case we fail.
    //
    data = NULL;

    if(input_data == NULL) {
       XIL_ERROR(getSystemState(), XIL_ERROR_USER, "di-259", TRUE);
       return;
    }

    if((xsize == 0) || (ysize == 0)) {
       XIL_ERROR(getSystemState(), XIL_ERROR_USER, "di-146", TRUE);
       return;
    }

    if(((unsigned int)key_x >= xsize) || ((unsigned int)key_y >= ysize)) {
       XIL_ERROR(getSystemState(), XIL_ERROR_USER, "di-146", TRUE);
       return;
    }

    if((INT_MAX/sizeof(float)/xsize) < ysize) {
       XIL_ERROR(getSystemState(), XIL_ERROR_RESOURCE, "di-129", TRUE);
       return;
    }

    //
    //  Check the sel for invalid values...
    //
    for(unsigned int i=0; i<(xsize*ysize); i++) {
        if(input_data[i] > 1) {
            XIL_ERROR(getSystemState(), XIL_ERROR_USER, "di-273", TRUE);
            return;
        }
    }

    data = new unsigned int[xsize*ysize];
    if(data == NULL) {
       XIL_ERROR(getSystemState(), XIL_ERROR_RESOURCE, "di-1", TRUE);
       return;
    }
    memcpy(data, input_data, sizeof(unsigned int)*xsize*ysize);

    width  = xsize;
    height = ysize;
    keyX   = key_x;
    keyY   = key_y;

    //
    // AFTER THIS POINT AN ERROR MUST delete data and set it to NULL 
    //

    isOKFlag = TRUE;
}


//
//  Create a copy of the sel
//
XilObject*
XilSel::createCopy()
{
    XilSel* new_copy =
        getSystemState()->createXilSel(width, height, keyX, keyY, data);
    if(new_copy == NULL) {
	XIL_ERROR(getSystemState(), XIL_ERROR_SYSTEM, "di-170", FALSE);
	return NULL;
    }

    new_copy->copyVersionInfo(this);

    return new_copy;
}

unsigned int 
XilSel::getWidth()
{
    return width;
}

unsigned int 
XilSel::getHeight()
{
    return height;
}

int 
XilSel::getKeyX()
{
    return keyX;
}

int 
XilSel::getKeyY()
{
    return keyY;
}

const unsigned int*
XilSel::getData()
{
    return data;
}

//
// Copy the Sel values to the user-supplied buffer
//
void
XilSel::getValues(unsigned int* data_arg)
{
    if(data_arg == NULL) {
        XIL_ERROR(getSystemState(), XIL_ERROR_USER, "di-259", TRUE);
        return;
    }

    xili_memcpy(data_arg, data, width*height*sizeof(unsigned int));
}
