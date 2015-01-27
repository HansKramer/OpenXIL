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
//  File:	XiliBag.hh
//  Project:	XIL
//  Revision:	1.20
//  Last Mod:	10:21:01, 03/10/00
//
//  Description:
//	This header file contains the classes for managing a bag of
//      unsorted elements.  They are non-ordered and no effort is made
//      to verify the same entry is not inserted twice.
//
//      The class is optimized for a relatively small number of elements.
//      Although it can handle as many elements as necessary, it manages
//      memory such that no new or delete occurs as long as the initial
//      number of entries is not exceeded.
//
//      We allocate an additional entry at the end of the data array
//      for a NULL element.  This means we don't have to test against
//      arraySize when looking because we know we'll always be
//      guarenteed to have a stopping element at the end of the array.
//      This NULL element is set by doubleArraySize() and is not taken
//      into account with the arraySize variable.
//
//      NOTE:  The bag is not intended to store NULL entries.  Inserting
//             a NULL entry is an error.  This is not tested for in the
//             current implementation for performance reasons.
//      
//  MT-level:  UNsafe
//	
//------------------------------------------------------------------------
//	COPYRIGHT
//------------------------------------------------------------------------
#pragma ident	"@(#)XiliBag.hh	1.20\t00/03/10  "

#ifndef _XILI_BAG_HH
#define _XILI_BAG_HH

//
//  For free()
//
#include <stdlib.h>

#include "_XilDefines.h"

//
//  The initial size of the bag indicates how many elements are stored on the
//  "stack" or, more specifcially, as part of the class.  When the number of
//  elements stored in the bag grow beyond this initial size, the class will
//  use the heap to aquire more space.
//
#define _XILI_BAG_INITIAL_SIZE    8

//
//  Forward declare our iterator.
//
class XiliBagIterator;

class XiliBag {
public:
    //
    //  Status methods
    //
    Xil_boolean         isEmpty()  const
    {
        return bagCount == 0;
    }

    int                 length()   const
    {
        return bagCount;
    }

    //
    //  Insert a new pointer into the unordered container.
    //
    XilStatus           insert(void* insert_ptr)
    {
        if(bagCount == arraySize) {
            //
            //  If the bag is full, then allocate more space from the heap.
            //
            if(doubleArraySize() == XIL_FAILURE) {
                return XIL_FAILURE;
            }
        }

        //
        //  Fill in the next spot in the bag
        //
        entryArray[nextOpenEntry] = insert_ptr;

        //
        //  Grow the list by finding the next available entry in the bag.
        //
        while(entryArray[++nextOpenEntry] != NULL);

        bagCount++;

        return XIL_SUCCESS;
    }

    //
    //  Remove ALL instances of a pointer from the bag.
    //
    XilStatus           remove(void* remove_ptr);

    //
    //  Test whether the given pointer is in the bag.
    //
    Xil_boolean         isIn(void* element_ptr)
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
            //  When we find an entry, return indicating it exists.
            //
            if(entryArray[current_index] == element_ptr) {
                return TRUE;
            }

            current_index++;
        }

        return FALSE;
    }

    //
    //  Clear all of the entries from the bag.
    //
    XilStatus           clear();

#ifdef DEBUG
    //
    //  Display the contents of the unordered contained to stderr.
    //
    virtual void        dump() const;
#endif

    //
    //  Constructor/Destructor
    //
                        XiliBag(XilSystemState* sys_state = NULL) :
                            systemState(sys_state)
    {
        //
        //  An empty bag has no entries and the open entry is the first one.
        //
        nextOpenEntry  = 0;
        bagCount       = 0;
        arraySize      = 0;
        entryHeap      = NULL;
    }

                        ~XiliBag()
    {
        //
        //  We use malloc() to allocate entryHeap.
        //
        if(entryHeap != NULL) {
            free(entryHeap);
        }
    }

    
protected:
    //
    //  Private utility functions
    //
    XilStatus           doubleArraySize();

    //
    //  entryArray is a pointer to the current array which contains the bag
    //  infomation.   It will alternatively point at entryStack and then
    //  entryHeap when all of the entryStack entries have been utilized.
    //
    void**              entryArray;
    void*               entryStack[_XILI_BAG_INITIAL_SIZE + 1];
    void**              entryHeap;

    //
    //  Next location for a new pointer
    //
    unsigned int        nextOpenEntry;

    //
    //  Count indicating the number of valid entries
    //
    unsigned int        bagCount;

    //
    //  The current size of the actual storage array
    //
    unsigned int        arraySize;

    //
    //  The system state for error reporting.
    //
    XilSystemState*     systemState;
    
    //
    //  Befriend the iterator.
    //
    friend class XiliBagIterator;
};


//------------------------------------------------------------------------
//
//  Class:	XiliBagIterator
//
//  Description:
//	
//	Iterates through all of the elements in a bag.
//
//      See the bag routines in XiliBag.cc for more information on how
//      to iterate through the bag.
//	
//  MT-level:  Un-Safe
//	
//------------------------------------------------------------------------
class XiliBagIterator {
public:
    //
    //  Get the next entry in the bag.  This routine returns NULL when there
    //  are no more valid entries in the bag.
    //
    void*             getNext()
    {
        //
        //  We also test against bagCount just in case entries are removed
        //  from the bag while we're interating causing an endless loop
        //  below.
        //
        if(toVisit-- <= 0 || bag->bagCount == 0) {
            return NULL;
        }

        //
        //  Skip over the empty NULL entries.  Since toVisit is not zero, we
        //  know there is one out there.
        //
        while(bag->entryArray[currentIndex] == NULL) {
            currentIndex++;
        }

        return bag->entryArray[currentIndex++];
    }

                      XiliBagIterator(XiliBag* initial_bag)
    {
        bag          = initial_bag;
        currentIndex = 0;
        toVisit      = bag->bagCount;
    }

private:
    XiliBag*          bag;
    unsigned int      currentIndex;
    unsigned int      toVisit;
};

#endif   // _XILI_BAG_HH


