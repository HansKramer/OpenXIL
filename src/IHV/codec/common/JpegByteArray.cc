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
//This line lets emacs recognize this as -*- C++ -*- Code
//------------------------------------------------------------------------
//
//  File:       JpegByteArray.cc
//  Project:    XIL
//  Revision:   1.2
//  Last Mod:   10:16:17, 03/10/00
//
//  Description:
//
//    JpegByteArray Class. 
//    A very simple array class, which tracks its own length.
//    The max size must be specified a construction time.
//
//
//------------------------------------------------------------------------
//    COPYRIGHT
//------------------------------------------------------------------------
#pragma ident   "@(#)JpegByteArray.cc	1.2\t00/03/10  "


#include "JpegByteArray.hh"

//
// Constructor
// Allocate the max size given
//
JpegByteArray::JpegByteArray(int size)
{
    isOKFlag = FALSE;

    array_size = 0;
    current_length = 0;

    theArray = new Xil_unsigned8[size];
    if(theArray == NULL) {
        return;
    }

    // Successful, so set the size
    array_size = size;

    isOKFlag = TRUE;
}

//
// Destructor - dealloc array and reset values
//
JpegByteArray::~JpegByteArray()
{
    array_size = 0;
    current_length = 0;
    delete [] theArray;
    theArray = NULL;
}

//
// Append method - increment the length
// Tests if its safe to do the insertion
//
XilStatus
JpegByteArray::append(Xil_unsigned8 element)
{
    if(current_length < array_size) {
        theArray[current_length++] = element;
        return XIL_SUCCESS;
    }

    return XIL_FAILURE;
}

//
// Get back an element from the array
// TODO: what should be returned on an error
//
Xil_unsigned8
JpegByteArray::retrieve(int position)
{
    if(position < current_length) {
        return theArray[position];
    }

    return 0xff; // ???
}

int
JpegByteArray::length()
{
    return current_length;
}

Xil_boolean
JpegByteArray::isOK()
{
    if(this == NULL) { 
        return FALSE; 
    } else { 
        if(isOKFlag == TRUE) { 
            return TRUE; 
        } else { 
            delete this; 
            return FALSE; 
        } 
    }
}
