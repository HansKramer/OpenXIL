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
//  File:	XiliSLList.hh
//  Project:	XIL
//  Revision:	1.20
//  Last Mod:	10:21:11, 03/10/00
//
//  Description:
//	A template for a fast single link list.  The object allocates
//      memory in blocks and can be used to store the real objects or 
//      pointers to the objects.  The block allocation can be taken
//      advantage of by inserting or appending and then obtaining a
//      reference to the Type class within the block.
//
//      Tests show that this method of allocation makes manipulating
//      the list take less than 1/8 as much time compared with new and
//	delete to allocate and deallocate every entry being inserted
//      and then removed from the list.
//
//  MT-level:  UNsafe
//	
//------------------------------------------------------------------------
//	COPYRIGHT
//------------------------------------------------------------------------
#pragma ident	"@(#)XiliSLList.hh	1.20\t00/03/10  "

#ifndef _XILI_SLLIST_HH
#define _XILI_SLLIST_HH

#include "_XilDefines.h"
#include "_XilClasses.hh"

typedef void* XiliSLListPosition;

#define _XILI_SLLIST_INVALID_POSITION  NULL

#if !defined (GCC) && !defined (_WINDOWS) && !defined(IRIX) &&!defined(HPUX)
template <class Type> class XiliSLListIterator<Type>;
#endif

template<class Type> class XiliSLList
{
public:
    //
    //  List Status Information Methods
    //
    Xil_boolean        isEmpty()  const { return listLength == 0;  }
    int                length()   const { return listLength;       }
    XiliSLListPosition head()     const { return listHead;         }
    XiliSLListPosition tail()     const { return listTail;         }


    //
    //  Adding Elements to the List...
    //
    //  Both append and prepend are very fast because this object does keep
    //  track of both the beginning and the end of the list.
    //
    XiliSLListPosition append(Type& new_entry);
    XiliSLListPosition prepend(Type& new_entry);

    //
    //  For a single-linked list, insertBefore() is a slow operation and
    //  should be avoided except in certain instances (like inserting at the
    //  head of the list -- use prepend())
    //
    XiliSLListPosition insertBefore(Type&              new_entry,
                                    XiliSLListPosition before_position);

    //
    //  This is a fast operation on a single-linked list.
    //
    XiliSLListPosition insertAfter(Type&              new_entry,
                                   XiliSLListPosition after_position);
    //
    //  These which don't take arguments allocate a new entry on the list and
    //  provide the position of that entry.  The entry can then be referenced
    //  and it's contents modified.  See notes above about the relative
    //  performance of these operations.
    //
    XiliSLListPosition append();
    XiliSLListPosition prepend();
    XiliSLListPosition insertBefore(XiliSLListPosition before_position);
    XiliSLListPosition insertAfter(XiliSLListPosition after_position);

    //
    //  Element Removal
    //
    XilStatus          remove(XiliSLListPosition remove_position,
                              Type&              removed_entry);
    XilStatus          remove(XiliSLListPosition remove_position);

    //
    //  Element Retrieval Methods
    //
    Type&              reference(XiliSLListPosition position)
    {
        return ((Entry*)position)->entry;
    }
    
    Type&              reference(unsigned int       entry_number);

    //
    //  Element Locator Methods
    //
    XiliSLListPosition find(Type& entry);

    //
    //  Constructor/Destructor
    //
                       XiliSLList(XiliSLList<Type>& init_list);
                       XiliSLList(XilSystemState* sys_state = NULL);
                       ~XiliSLList();

    //
    //  List Traversal Methods
    //
    //  NOTE:  It's best to use the iterators for moving through the list
    //         since the underlying implementation may change and the
    //         performance of these may change dramatically.
    //
    XiliSLListPosition next(XiliSLListPosition position) const;

    //
    //  Using previous() on a single-linked list is a very slow operation and
    //  should be avoided or the list should be double-linked list.
    //
    XiliSLListPosition previous(XiliSLListPosition position) const;

    //
    //  This must be public so EntryArray can actually see it.
    //
    class Entry {
    public:
        Entry* next;
        Type   entry;
    };

#define _XILI_SLLIST_DEFAULT_BLOCK_SIZE 8
    class EntryArray {
    public:
        Entry       array[_XILI_SLLIST_DEFAULT_BLOCK_SIZE];
        EntryArray* next;
    };

#ifndef _WINDOWS
private:
#endif
    //
    //  Gets a new entry from our freeList.
    //
    Entry*          getNewEntry()
    {
        Entry* entry;
        if(freeList == NULL) {
            EntryArray* tmp_array_list = arrayList;

            //
            //  Allocate a new EntryArray class which contains the array of
            //  Entry objects we'll put onto the freeList.
            //
            arrayList = new EntryArray;
            if(arrayList == NULL) {
                return NULL;
            }

            //
            //  Link them together...
            //
            arrayList->next = tmp_array_list;

            //
            //  Put the elements onto the freeList.
            //
            for(entry = freeList =
                    &arrayList->array[_XILI_SLLIST_DEFAULT_BLOCK_SIZE - 1];
                &arrayList->array[0] < entry; entry--) {
                entry->next = entry-1;
            }

            entry->next = NULL;
        }

        entry    = freeList;
        freeList = freeList->next;

        return entry;
    }

    //
    //  Gets a new entry from our freeList and fills it in with the given user
    //  entry.
    //
    Entry*          getNewEntry(Type& new_entry)
    {
        Entry* tmp = getNewEntry();

        tmp->entry = new_entry;
        
        return tmp;
    }

    //
    //  Frees the given entry by placing it on our freeList
    //
    void            freeEntry(Entry* entry)
    {
        entry->next = freeList;
        freeList    = entry;
    }

    //
    //  arrayList is a list of the arrays that we have allocated.  This is how
    //  we know how to free them when the list goes away.
    //
    //  TODO: 2/25/96  jlf  Should these linger beyond the life of the list?
    //
    //    For some uses of the list, it seems like they may be created and
    //    destroyed often.  If this is the case, then we would do better to
    //    keep a static pool of Entry objects instead of reallocating them
    //    every time a list is constructed.
    //
    EntryArray*     arrayList;

    //
    //  This is the freeList which keeps a linked list of the available Entry
    //  objects.  It points to entries in the arrayList.
    //
    Entry*          freeList;

    //
    //  The actual list.
    //
    //  TODO: 2/25/96 jlf  Should we spend the time to keep track of listTail?
    //
    //    It's very useful for append(), but it does cost time.
    //
    Entry*          listHead;
    Entry*          listTail;

    //
    //  The number of elements on the list.
    //
    unsigned int    listLength;

    //
    //  The XilSystemState for generating errors.
    //
    XilSystemState* systemState;

    //
    //  The first set of entries are kept as part of the list.
    //
    Entry           baseEntry;
    
    //
    //  Iterator Friend
    //
#if 0 
#if  !defined(_WINDOWS) && !defined(IRIX) &&!defined(HPUX) && !defined(GCC)
    friend XiliSLListIterator<Type>;
#endif
#endif
};

template <class Type> class XiliSLListIterator {
public:
    //
    //  TODO: 2/29/96 jlf  Another compiler bug!
    //
    //    This is here due to another compiler bug!  If the method is left
    //    inline in XiliSLList.hh then any test of curPos causes it to test a
    //    wrong value and act as if it's set even if it and list->head() are
    //    NULL. 
    //
    XilStatus           getNext(Type& entry);

    XiliSLListPosition  getCurrentPosition()
    {
        return curPos;
    }

    void                setCurrentPosition(XiliSLListPosition pos)
    {
	curPos = pos;
    }

    XiliSLListIterator(XiliSLList<Type>* initial_list)
    {
        list    = initial_list;
        curPos  = _XILI_SLLIST_INVALID_POSITION;
    }

    XiliSLListIterator(XiliSLList<Type>*  initial_list,
                       XiliSLListPosition start_position)
    {
        list    = initial_list;
        curPos  = start_position;
    }

protected:
    XiliSLList<Type>*   list;
    XiliSLListPosition  curPos; 
};

#endif // _XILI_SLLIST_HH
