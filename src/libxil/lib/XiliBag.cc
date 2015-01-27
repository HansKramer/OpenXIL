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
//  File:	XiliBag.cc
//  Project:	XIL
//  Revision:	1.20
//  Last Mod:	10:08:27, 03/10/00
//
//  Description:
//	Implementation of the XilBag base class.
//	
//  MT-level:  UNsafe
//	
//------------------------------------------------------------------------
//	COPYRIGHT
//------------------------------------------------------------------------
#pragma ident	"@(#)XiliBag.cc	1.20\t00/03/10  "

//
//  System Includes
//
#include <stdlib.h>
#include <string.h>

//
//  XIL Includes
//
#include "_XilDefines.h"
#include "_XilSystemState.hh"
#include "XiliBag.hh"
#include "XiliUtils.hh"

//------------------------------------------------------------------------
//
//  Function:	doubleArraySize()
//
//  Description:
//	
//    Grow the size of the list by 2x.  The first time through, we've
//    used up all of the available entries on the stack.  So, we
//    allocate some space on the heap and copy from the stack into the
//    heap.  Calls other than the first time causes the heap to be
//    reallocated to twice its size.
//	
//------------------------------------------------------------------------
XilStatus
XiliBag::doubleArraySize()
{
    if(arraySize == 0) {
        //
        //  Special case the very common case of adding a single element to
        //  the bag.
        //
        arraySize      = 1;
        entryArray     = &entryStack[0];
        *entryArray    = NULL;
        *(entryArray+1)= NULL;
    } else if(arraySize < _XILI_BAG_INITIAL_SIZE) {
        //
        //  arraySize should be initialized to a power of 2!
        //
        arraySize      = _XILI_BAG_INITIAL_SIZE;

        //
        //  Point the array we use at the stack version for now.  If we need more
        //  space later, we'll allocate it from the heap
        //
        entryArray     = &entryStack[0];

        //
        //  Initialize our pointers to NULL.
        //
        //  NOTE:  We're starting at arraySize because we keep a hidden NULL
        //         element in an entry at the end of the array to stop our loops.
        //
        for(int i=arraySize; i>0; i--) {
            entryArray[i] = NULL;
        }
    } else {
        //
        //  Double size of array count
        //
        unsigned int new_size = arraySize<<1;

        //
        //  Pointer to the newly allocated array.
        //
        void** new_array;

        if(entryHeap == NULL) {
            //
            //  If we don't already have space on the heap, allocate a 
            //  new array of elements on the heap.  I use malloc() here
            //  so I can use realloc() to continue to grow the bag.
            //
            new_array = (void**)malloc((new_size+1)*sizeof(void*));
            if(new_array == NULL) {
                XIL_ERROR(systemState, XIL_ERROR_RESOURCE, "di-1", TRUE);
                return XIL_FAILURE;
            }
            
            //
            //  Copy contents from stack to heap storage.
            //
            xili_memcpy(new_array, entryArray, arraySize*sizeof(void*));
        } else {
            //
            //  Just realloc the array.
            //
            //  NOTE:  Admittedly, this is not the most efficient way to grow the
            //         array.  But, this case should be uncommon because the
            //         expected usage of the object is for a small number of
            //         entries in the bag.  If the expectations change, then
            //         modifying this call may be wise.
            //
            new_array = (void**)realloc(entryHeap, (new_size+1)*sizeof(void*));
            if(new_array == NULL) {
                XIL_ERROR(systemState, XIL_ERROR_RESOURCE, "di-1", TRUE);
                return XIL_FAILURE;
            }
        }
    
        //
        //  Initialize our new pointers to NULL.
        //
        xili_memset(&new_array[arraySize], 0, (arraySize+1)*sizeof(void*));

        entryHeap  = new_array;
        entryArray = entryHeap;

        arraySize  = new_size;
    }
        
    return XIL_SUCCESS;
}

//------------------------------------------------------------------------
//
//  Function:	remove()
//
//  Description:
//	
//	Remove all entries containing the given element from the bag.
//	
//------------------------------------------------------------------------
XilStatus
XiliBag::remove(void* remove_ptr)
{
    unsigned int current_index = 0;
    unsigned int max_to_visit  = bagCount;

    //
    //  The bagCount contains the maximum number of valid entries in the bag
    //  so we only have to check for as many that reside in the bag.
    //
    while(max_to_visit--) {
        //
        //  Skip over empty bag entries.
        //
        while(entryArray[current_index] == NULL) {
            current_index++;
        }

        //
        //  When we find an entry, we clear it from the bag and continue
        //  looking for potentially more entries.
        //
        if(entryArray[current_index] == remove_ptr) {
            entryArray[current_index] = NULL;
            bagCount--;

            //
            //  Update our "next" entry to be the one we just removed if we've
            //  removed one lower than the current "next" index.  This keeps
            //  the bag packed from the starting point.
            //
            if(current_index < nextOpenEntry) {
                nextOpenEntry = current_index;
            }
        }

        current_index++;
    }

    return XIL_SUCCESS;
}

//------------------------------------------------------------------------
//
//  Function:	clear()
//
//  Description:
//	
//	Remove all of the entries from the bag.
//	
//------------------------------------------------------------------------
XilStatus
XiliBag::clear()
{
    //
    //  Set all of our pointers to NULL.
    //
    xili_memset(entryArray, 0, arraySize*sizeof(void*));

    //
    //  Set the number of valid entries to 0.
    //
    bagCount = 0;

    return XIL_SUCCESS;
}

//------------------------------------------------------------------------
//
//  Function:	dump()
//
//  Description:
//	
//	Dump the contents of the bag to stderr.
//	
//------------------------------------------------------------------------
#ifdef DEBUG
void
XiliBag::dump() const
{
    fprintf(stderr, "DUMPING PTR LIST OBJECT:\n");

    for(unsigned int i=0; i<arraySize; i++) {
        fprintf(stderr, "%2d: %p\n", i, entryArray[i]);
    }
}
#endif
