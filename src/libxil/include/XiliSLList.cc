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
//  File:	XiliSLList.cc
//  Project:	XIL
//  Revision:	1.11
//  Last Mod:	10:07:55, 03/10/00
//
//  Description:
//	
//	
//	
//	
//	
//	
//	
//	
//  MT-level:  UNSafe
//	
//------------------------------------------------------------------------
//	COPYRIGHT
//------------------------------------------------------------------------
#pragma ident	"@(#)XiliSLList.cc	1.11\t00/03/10  "

//
//  C++ Includes
//
#include "XiliSLList.hh"


//
//  CONSTRUCTOR/DESTRUCTOR
//
template<class Type>
XiliSLList<Type>::XiliSLList(XilSystemState* sys_state)
{
    systemState    = sys_state;

    baseEntry.next = NULL;
    freeList       = &baseEntry;
    arrayList      = NULL;

    listLength     = 0;
    listHead       = NULL;
    listTail       = NULL;
}

template<class Type>
XiliSLList<Type>::XiliSLList(XiliSLList<Type>& init_list)
{
    systemState    = init_list.systemState;

    baseEntry.next = NULL;
    freeList       = &baseEntry;
    arrayList      = NULL;

    listLength     = 0;
    listHead       = NULL;
    listTail       = NULL;

    Entry* lh = init_list.listHead;

    while(lh != NULL) {
        append(lh->entry);

        lh = lh->next;
    }
}

template<class Type>
XiliSLList<Type>::~XiliSLList()
{
    //
    //  Cleanup the storage we've allocated by freeing all of the EntryArray
    //  objects on our arrayList.
    //
    while(arrayList != NULL) {
        EntryArray* tmp = arrayList->next;

        delete arrayList;
        
        arrayList = tmp;
    }
}

//
//  Append a new entry to the list.
//
template<class Type>
XiliSLListPosition
XiliSLList<Type>::append(Type& new_entry)
{
    Entry* le = getNewEntry(new_entry);
    
    if(le == NULL) {
        return NULL;
    }

    if(listLength == 0) {
        //
        //  Empty list
        //
        listHead = le;
        listTail = le;
        
        listHead->next = NULL;
    } else {
        le->next       = NULL;
        
        listTail->next = le;
        listTail       = le;
    }
    
    listLength++;

    return listTail;
}

template<class Type>
XiliSLListPosition
XiliSLList<Type>::append()
{
    Entry* le = getNewEntry();
    
    if(le == NULL) {
        return NULL;
    }

    if(listLength == 0) {
        //
        //  Empty list
        //
        listHead = le;
        listTail = le;
        
        listHead->next = NULL;
    } else {
        le->next       = NULL;
        
        listTail->next = le;
        listTail       = le;
    }
    
    listLength++;

    return listTail;
}

//
//  Prepend a new entry to the list.
//
template<class Type>
XiliSLListPosition
XiliSLList<Type>::prepend(Type& new_entry)
{
    Entry* le = getNewEntry(new_entry);
    
    if(le == NULL) {
        return NULL;
    }

    if(listLength == 0) {
        //
        //  Empty list
        //
        listHead = le;
        listTail = le;
        
        listHead->next = NULL;
    } else {
        le->next       = listHead;
        listHead            = le;
    }
    
    listLength++;

    return listHead;
}

template<class Type>
XiliSLListPosition
XiliSLList<Type>::prepend()
{
    Entry* le = getNewEntry();
    
    if(le == NULL) {
        return NULL;
    }

    if(listLength == 0) {
        //
        //  Empty list
        //
        listHead = le;
        listTail = le;
        
        listHead->next = NULL;
    } else {
        le->next       = listHead;
        listHead            = le;
    }
    
    listLength++;

    return listTail;
}

//
//  Insert the given entry before the given position
//
template<class Type>
XiliSLListPosition
XiliSLList<Type>::insertBefore(Type&              new_entry,
                               XiliSLListPosition before_position)
{
    Entry* le = getNewEntry(new_entry);
    
    if(le == NULL) {
        return NULL;
    }

    if(listLength == 0) {
        //
        //  Empty list
        //
        listHead = le;
        listTail = le;
        
        listHead->next = NULL;
    } else {
        if(listHead == (Entry*)before_position) {
            le->next        = listHead;
            
            listHead = le;
        } else {
            Entry* tmpentry = listHead;

            while(tmpentry) {
                if(tmpentry->next == (Entry*)before_position) {
                    le->next        = tmpentry->next;
                    tmpentry->next  = le;
                    
                    break;
                }
                tmpentry = tmpentry->next;
            }
        }
    }
    listLength++;

    return le;
}

template<class Type>
XiliSLListPosition
XiliSLList<Type>::insertBefore(XiliSLListPosition before_position)
{
    Entry* le = getNewEntry();
    
    if(le == NULL) {
        return NULL;
    }

    if(listLength == 0) {
        //
        //  Empty list
        //
        listHead = le;
        listTail = le;
        
        listHead->next = NULL;
    } else {
        if(listHead == (Entry*)before_position) {
            le->next        = listHead;
            
            listHead = le;
        } else {
            Entry* tmpentry = listHead;

            while(tmpentry) {
                if(tmpentry->next == (Entry*)before_position) {
                    le->next        = tmpentry->next;
                    tmpentry->next  = le;
                    
                    break;
                }
                tmpentry = tmpentry->next;
            }
        }
    }
    listLength++;

    return le;
}

//
//  Insert the given entry before the given position
//
template<class Type>
XiliSLListPosition
XiliSLList<Type>::insertAfter(Type&              new_entry,
                              XiliSLListPosition after_position)
{
    Entry* le = getNewEntry(new_entry);
    
    if(le == NULL) {
        return NULL;
    }

    if(listLength == 0) {
        //
        //  Empty list
        //
        listHead = le;
        listTail = le;
        
        listHead->next = NULL;
    } else {
        le->next                       = ((Entry*)after_position)->next;
        ((Entry*)after_position)->next = le;

        if(listTail == (Entry*)after_position) {
            listTail = le;
        }
    }
    
    listLength++;

    return le;
}

template<class Type>
XiliSLListPosition
XiliSLList<Type>::insertAfter(XiliSLListPosition after_position)
{
    Entry* le = getNewEntry();

    if(le == NULL) {
        return NULL;
    }

    if(listLength == 0) {
        //
        //  Empty list
        //
        listHead = le;
        listTail = le;
        
        listHead->next = NULL;
    } else {
        le->next                       = ((Entry*)after_position)->next;
        ((Entry*)after_position)->next = le;

        if(listTail == (Entry*)after_position) {
            listTail = le;
        }
    }
    
    listLength++;

    return le;
}

//
//  Remove the entry at the given position and return the removed entry.
//
template<class Type>
XilStatus
XiliSLList<Type>::remove(XiliSLListPosition remove_position,
                         Type&              removed_entry)
{
    XilStatus ret_val = XIL_FAILURE;

    if(listLength == 0 || remove_position == _XILI_SLLIST_INVALID_POSITION) {
        return ret_val;
    }

    if(listHead == remove_position) {
        removed_entry = listHead->entry;
        listHead      = listHead->next;
        ret_val       = XIL_SUCCESS;
    } else {
        //
        //  For removal, we must start at the beginning because we need to
        //  update the previous entry's next pointer.
        //
        Entry* tmpentry = listHead;

        while(tmpentry) {
            if(tmpentry->next == (Entry*)remove_position) {
                //
                //  Found the entry to remove in the list.
                //
                removed_entry  = tmpentry->next->entry;
                tmpentry->next = tmpentry->next->next;
                ret_val        = XIL_SUCCESS;

                if(listTail == (Entry*)remove_position) {
                    listTail = tmpentry;
                }
                break;
            }
            
            tmpentry = tmpentry->next;
        }
    }

    freeEntry((Entry*)remove_position);
    
    listLength--;

    return ret_val;
}

//
//  Remove the entry at the given position.
//
template<class Type>
XilStatus
XiliSLList<Type>::remove(XiliSLListPosition remove_position)
{
    XilStatus ret_val = XIL_FAILURE;

    if(listLength == 0 || remove_position == _XILI_SLLIST_INVALID_POSITION) {
        return ret_val;
    }

    if(listHead == remove_position) {
        listHead      = listHead->next;
        ret_val       = XIL_SUCCESS;
    } else {
        //
        //  For removal, we must start at the beginning because we need to
        //  update the previous entry's next pointer.
        //
        Entry* tmpentry = listHead;

        while(tmpentry) {
            if(tmpentry->next == (Entry*)remove_position) {
                //
                //  Found the entry to remove in the list.
                //
                tmpentry->next = tmpentry->next->next;
                ret_val        = XIL_SUCCESS;

                if(listTail == (Entry*)remove_position) {
                    listTail = tmpentry;
                }
                break;
            }
            
            tmpentry = tmpentry->next;
        }
    }

    freeEntry((Entry*)remove_position);
    
    listLength--;

    return ret_val;
}


//
//  Return the position of the next entry in the list
//
template<class Type>
XiliSLListPosition
XiliSLList<Type>::next(XiliSLListPosition position) const
{
    return ((Entry*)position)->next;
}

//
//  Return the position of the previous entry in the list
//
template<class Type>
XiliSLListPosition
XiliSLList<Type>::previous(XiliSLListPosition position) const
{
    if(listHead == (Entry*)position) {
        return NULL;
    } else {
        Entry* tmpentry = listHead;

        while(tmpentry) {
            if(tmpentry->next == (Entry*)position) {
                return tmpentry;
            }
            
            tmpentry = tmpentry->next;
        }
    }
    
    return NULL;
}

//
//  Return a reference to the nth entry in the list
//
template<class Type>
Type&
XiliSLList<Type>::reference(unsigned int entry_num)
{
    if(entry_num > listLength) {
        return *((Type *) NULL);
    }

    Entry* tmpentry = listHead;
    unsigned int i=0;
    while(i++ < entry_num) {
        tmpentry = tmpentry->next;
    }

    return tmpentry->entry;
}

//
//  Find the first entry in the list that is equal to the given entry
//    and return its position.
//
template<class Type>
XiliSLListPosition
XiliSLList<Type>::find(Type& entry)
{
    Entry* tmpentry = listHead;

    while(tmpentry) {
        if(tmpentry->entry == entry) {
            return tmpentry;
        }
        
        tmpentry = tmpentry->next;
    }

    return NULL;
}

//
//  TODO: 2/29/96 jlf  Another compiler bug!
//
//    This is here due to another compiler bug!  If the method is left inline
//    in XiliSLList.hh then any test of curPos causes it to test a wrong value
//    and act as if it's set even if it and list->head() are NULL.
//
//    The specific case where this fails is the use of the XiliListIterator
//    by XilSystemState::getObjectByName().
//
template<class Type>
XilStatus
XiliSLListIterator<Type>::getNext(Type& entry)
{
    if(curPos == _XILI_LIST_INVALID_POSITION) {
        //
        //  At beginning of the list -- just starting.
        //
        if((curPos = list->head()) == _XILI_LIST_INVALID_POSITION) {
            return XIL_FAILURE;
        }
    } else if((curPos = list->next(curPos)) == _XILI_LIST_INVALID_POSITION) {
        //
        //  At end of the list -- so we're done.
        //
        return XIL_FAILURE;
    }

    entry = list->reference(curPos);

    return XIL_SUCCESS;
}

