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
//  File:       SingleBuffer.cc
//  Project:    XIL
//  Revision:   1.2
//  Last Mod:   10:16:21, 03/10/00
//
//  Description:
//
//    TODO: Enter some descriptive text here
//
//
//------------------------------------------------------------------------
//    COPYRIGHT
//------------------------------------------------------------------------
#pragma ident   "@(#)SingleBuffer.cc	1.2\t00/03/10  "

#include "SingleBuffer.hh"

Xil_unsigned8* 
SingleBuffer::init(Xil_unsigned8* user_data,
                   int            size, 
                   GrowFn         get_grow_increment, 
                   void*          user_ptr)
{
    buffer_size = size;
    if(user_data != NULL) {
        buffer = wptr = user_data;
        can_realloc = FALSE;
    } else {
        buffer = wptr = (Xil_unsigned8*) malloc(size);
        can_realloc = TRUE;
    }
    grow_fn = get_grow_increment;
    user_grow_ptr = user_ptr;

    return (buffer);
}

void 
SingleBuffer::addByte(int b)
{
    Xil_unsigned8* tmp;
    int new_size;

    //
    // lperry
    // TODO: This check seems unnecessary. the init routine and
    //       the realloc routine must check for valid memory
    //       allocations. Checking on every byte just slows things.
    //
    if(buffer == NULL) {
        return;
    }
    if(buffer_size == getNumBytes()) {
        if(!can_realloc) {
            buffer = 0;
            buffer_size = 0;
            XIL_ERROR( NULL, XIL_ERROR_USER, "di-104", TRUE);
            return;
        }
        //
        // Attempted write beyond allocated buffer space
        //
        new_size = (*grow_fn)( buffer_size, user_grow_ptr);
        tmp = (Xil_unsigned8*) realloc(buffer, new_size);
        if(tmp == NULL) {
            if(buffer != NULL) {
                free((void*)buffer);
            }
            buffer = 0;
            buffer_size = 0;
            XIL_ERROR( NULL, XIL_ERROR_RESOURCE,"di-1",TRUE);
            return;
        }
        wptr = tmp + buffer_size;
        buffer = tmp;
        buffer_size = new_size;

    }

    *wptr++ = b;
}

