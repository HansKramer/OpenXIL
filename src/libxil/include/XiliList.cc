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
//  File:	XiliList.cc
//  Project:	XIL
//  Revision:	1.26
//  Last Mod:	10:07:54, 03/10/00
//
//  Description:
//	Implementation of the general List class.
//
//      TODO: 11/19/95 jlf  Should this class not use XiliListEntry?
//
//  MT-level:  UNsafe
//	
//------------------------------------------------------------------------
//	COPYRIGHT
//------------------------------------------------------------------------
#pragma ident	"@(#)XiliList.cc	1.26\t00/03/10  "

//
//  System Includes
//
#include <stdlib.h>

//
//  C++ Includes
//
#include "XiliList.hh"

//
//  We need to special case these here because of the requisite template code.
//
#ifdef _XIL_OVERLOAD_NEW_AND_DELETE
template<class Type>
XiliListEntry<Type>* XiliListEntry<Type>::freeList= NULL;

template<class Type>
XilMutex             XiliListEntry<Type>::freeListMutex;

template<class Type>
unsigned int         XiliListEntry<Type>::freeListCount = 0;

template<class Type>
void*
XiliListEntry<Type>::operator new (size_t)
{
    XiliListEntry<Type>* entry;

    freeListMutex.lock();

    if(freeListCount > 2048) {
        entry = ::new XiliListEntry<Type>;
        entry->nextFree = (XiliListEntry<Type>*)-1;
    } else {
        entry = freeList;

        if(entry == NULL) {
            XiliListEntry<Type>* tmp = ::new XiliListEntry<Type>[16];

            for(entry=freeList=&tmp[16-1]; tmp<entry; entry--) {
                entry->nextFree = entry-1;
            }
            entry->nextFree = NULL;
                
            entry = freeList;
                
            freeListCount += 16;
        }
            
        freeList = entry->nextFree;
    }
        
    freeListMutex.unlock();
        
    return entry;
}
    
template<class Type>
void
XiliListEntry<Type>::operator delete (void* ptr, size_t)
{
    freeListMutex.lock();

    if(((XiliListEntry*)ptr)->nextFree == (XiliListEntry*)-1) {
        ::delete ptr;
    } else {
        ((XiliListEntry*)ptr)->nextFree   = freeList;
        freeList                          = (XiliListEntry*)ptr;
    }
        
    freeListMutex.unlock();
}
#endif

//
//  Destructor
//
template<class Type>
XiliList<Type>::~XiliList()
{
    emptyList();
}

//
//  Empty and _delete_ the elements from the list.
//
template<class Type>
void
XiliList<Type>::emptyList()
{
    XiliListEntry<Type>* tmpentry; 

    while(listHead) {
        tmpentry = listHead;
        listHead = listHead->nextEntry;

        delete tmpentry->entry;
        delete tmpentry;
    }
    
    listLength    = 0;
    listHead      = NULL;
    listTail      = NULL;
}

//
//  Insert the given entry before the given position
//
template<class Type>
XiliListPosition
XiliList<Type>::insertBefore(Type*            new_entry,
                             XiliListPosition before_position)
{
    XiliListEntry<Type>* list_position = (XiliListEntry<Type>*)before_position;
    XiliListEntry<Type>* le;
    
    if(listLength == 0) {
        //
        //  Empty list
        //
        le = listHead = listTail = new XiliListEntry<Type>(new_entry);
        
        listHead->nextEntry = NULL;
        listHead->prevEntry = NULL;
    } else {
        le = new XiliListEntry<Type>(new_entry);
        
        le->prevEntry       = list_position->prevEntry;
        le->nextEntry       = list_position;

        if(list_position->prevEntry) {
            list_position->prevEntry->nextEntry = le;
        }
        
        list_position->prevEntry                = le;
        
        if(listHead == list_position) {
            listHead = le;
        }
    }
    
    listLength++;

    return le;
}

//
//  Insert the given entry before the given position
//
template<class Type>
XiliListPosition
XiliList<Type>::insertAfter(Type*            new_entry,
                            XiliListPosition after_position)
{
    XiliListEntry<Type>* list_position = (XiliListEntry<Type>*)after_position;
    
    if(listLength == 0) {
        //
        //  Empty list
        //
        listHead = listTail = new XiliListEntry<Type>(new_entry);
        
        listHead->nextEntry = NULL;
        listHead->prevEntry = NULL;
    } else if(after_position == _XILI_LIST_INVALID_POSITION) {
        //
        //  Append
        //
        return append(new_entry);
    } else {
        XiliListEntry<Type>* le   = new XiliListEntry<Type>(new_entry);
        
        le->prevEntry       = list_position;
        le->nextEntry       = list_position->nextEntry;

        if(list_position->nextEntry) {
            list_position->nextEntry->prevEntry = le;
        }
        
        list_position->nextEntry                = le;
        
        if(listTail == list_position) {
            listTail = le;
        }
    }
    
    listLength++;

    return list_position->nextEntry;
}

template<class Type>
Type*
XiliList<Type>::removeEntry(XiliListPosition remove_position)
{
    XiliListEntry<Type>* tmp   = (XiliListEntry<Type>*)remove_position;

    if(listHead != NULL && tmp->isValid) {
        if(tmp->nextEntry) {
            tmp->nextEntry->prevEntry = tmp->prevEntry;
        }

        if(tmp->prevEntry) {
            tmp->prevEntry->nextEntry = tmp->nextEntry;
        }

        if(listHead == tmp) {
            listHead = tmp->nextEntry;
        }

        if(listTail == tmp) {
            listTail = tmp->prevEntry;
        }

        listLength--;

        Type* entry = tmp->entry;
            
        delete tmp;

        return entry;
    } else {
        return _XILI_LIST_INVALID_POSITION;
    }
}


//
//  Return a reference to the nth entry in the list
//
template<class Type>
Type*
XiliList<Type>::reference(unsigned int entry_num)
{
    if(entry_num > listLength) {
        return NULL;
    }

    XiliListEntry<Type>* tmp = listHead;
    unsigned int i=0;
    while(i++ < entry_num) {
        tmp = tmp->nextEntry;
    }
        
    return tmp->entry;
}

//
//  Find the first entry in the list that is equal to the given entry
//    and return its position.
//
template<class Type>
XiliListPosition
XiliList<Type>::find(Type* entry)
{
    XiliListEntry<Type>* tmp = listHead;

    while(tmp) {
        if(*tmp->entry == *entry) {
            return tmp;
        }
        
        tmp = tmp->nextEntry;
    }

    return NULL;
}

//
//  Test two lists for equality
//
template<class Type>
int
XiliList<Type>::operator == (XiliList<Type>& other) const
{
    XiliListEntry<Type>* e1 = listHead;
    XiliListEntry<Type>* e2 = other.listHead;

    while(e1 != _XILI_LIST_INVALID_POSITION &&
          e2 != _XILI_LIST_INVALID_POSITION) {

        if(!(*(e1->entry) == *(e2->entry))) {
            return FALSE;
        }
        
        e1 = e1->nextEntry;
        e2 = e2->nextEntry;
    }
    
    return TRUE;
}

template<class Type>
XiliList<Type>&
XiliList<Type>::operator = (XiliList<Type>& other)
{
    //
    //  Empty/Destroy all our entries.
    //
    emptyList();
    
    XiliListEntry<Type>* lh        = other.listHead;
    XiliListEntry<Type>* prev      = _XILI_LIST_INVALID_POSITION;
    XiliListEntry<Type>* current   = _XILI_LIST_INVALID_POSITION;
    
    while(lh != _XILI_LIST_INVALID_POSITION) {
        Type* tmp_entry = new Type;

        *tmp_entry = *lh->entry;
        
        current = new XiliListEntry<Type>(tmp_entry);

        current->nextEntry = NULL;
        current->prevEntry = prev;

        if(prev != _XILI_LIST_INVALID_POSITION) {
            prev->nextEntry = current;
        }

        prev               = current;
        
        if(listHead == _XILI_LIST_INVALID_POSITION) {
            listHead = current;
        }
        
        lh            = lh->nextEntry;
    }

    listTail = current;

    listLength = other.listLength;

    return *this;
}

//
//  XiliListIterator methods
//
//
//  TODO: 2/7/96 jlf  Another compiler bug!
//
//    This is here due to another compiler bug!  If the method is left inline
//    in XiliList.hh then any test of curPos causes it to test a wrong value
//    and act as if it's set even if it and list->head() are NULL.
//
//    The specific case where this fails is the use of the XiliListIterator
//    by XilSystemState::getObjectByName().
//
template <class Type>
Type*
XiliListIterator<Type>::getNext()
{
    if(curPos == _XILI_LIST_INVALID_POSITION) {
        //
        //  At beginning of the list -- just starting.
        //
        if((curPos = list->head()) == _XILI_LIST_INVALID_POSITION) {
            return NULL;
        }
    } else if((curPos = list->next(curPos)) == _XILI_LIST_INVALID_POSITION) {
        //
        //  At end of the list -- so we're done.
        //
        return NULL;
    }

    return list->reference(curPos);
}

template <class Type>
Type*
XiliListIteratorReverse<Type>::getNext()
{
    if(curPos == _XILI_LIST_INVALID_POSITION) {
        //
        // At tail of reverse list -- just starting.
        //
        if((curPos = list->tail()) == _XILI_LIST_INVALID_POSITION) {
            return NULL;
        }
    } else if((curPos = list->previous(curPos)) == _XILI_LIST_INVALID_POSITION) {
        //
        //  At beginning (head) of reverse list -- so we're done.
        //
        return NULL;
    }

    return list->reference(curPos);
}
