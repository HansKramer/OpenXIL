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
//  File:       SingleBuffer.hh
//  Project:    XIL
//  Revision:   1.2
//  Last Mod:   10:23:47, 03/10/00
//
//  Description:
//
//    TODO: Enter some descriptive text here
//
//
//------------------------------------------------------------------------
//    COPYRIGHT
//------------------------------------------------------------------------
#pragma ident   "@(#)SingleBuffer.hh	1.2\t00/03/10  "

#ifndef SINGLE_BUFFER_H
#define SINGLE_BUFFER_H

#include <stdlib.h>
#include <xil/xilGPI.hh>

typedef int (*GrowFn)(int old_size, void* ptr);

class SingleBuffer {
private:
    unsigned int    buffer_size;   // buffer size
    Xil_boolean     can_realloc;   // Can we realloc 'buffer'?
    Xil_unsigned8*  wptr;          // ptr to next writable byte in buffer
    Xil_unsigned8*  buffer;        // ptr to start of buffer
    GrowFn          grow_fn;       // returns new size for buffer
    void*           user_grow_ptr; // uninterpreted user data pointer

public:
    Xil_unsigned8*  init(Xil_unsigned8* user_data,
                         int size, GrowFn grow_fn, void* ptr);

    int             getNumBytes()   const  { return wptr - buffer; }
    int             getBufferSize() const  { return buffer_size; }
    Xil_unsigned8*  getDataPtr()        {return buffer;}

    void            addByte(int b);
    void            addShort(int s)  { addByte((s) >> 8); addByte(s); }
};
#endif // SINGLE_BUFFER_H
