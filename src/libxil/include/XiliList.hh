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
//  File:	XiliList.hh
//  Project:	XIL
//  Revision:	1.33
//  Last Mod:	10:20:57, 03/10/00
//
//  Description:
//	A template for a general list object.  Currently implemented as
//	a doubly linked list.
//	
//  MT-level:  UNsafe
//	
//------------------------------------------------------------------------
//	COPYRIGHT
//------------------------------------------------------------------------
#pragma ident	"@(#)XiliList.hh	1.33\t00/03/10  "

#include "_XilDefines.h"

#ifndef _XILI_LIST_HH
#define _XILI_LIST_HH

typedef void* XiliListPosition;

#define _XILI_LIST_INVALID_POSITION  NULL

#if !defined(GCC) && !defined(_WINDOWS) && !defined(IRIX) &&!defined(HPUX)
template <class Type> class XiliListIterator<Type>;
template <class Type> class XiliListIteratorReverse<Type>;
#endif

template <class Type>
class XiliListEntry {
public:
    XiliListEntry(Type* init_entry)
    {
        entry   = init_entry;
        isValid = TRUE;
    }

    ~XiliListEntry()
    {
        isValid = FALSE;
    }

    XiliListEntry<Type>* nextEntry;
    XiliListEntry<Type>* prevEntry;
    Xil_boolean          isValid;

    Type*                entry;

    _XIL_NEW_DELETE_OVERLOAD_PUBLIC(XiliListEntry<Type>)

private:
    _XIL_NEW_DELETE_OVERLOAD_PRIVATE(XiliListEntry<Type>)

    XiliListEntry()
    {
        isValid = TRUE;
    }
};

//
//  GENERAL USAGE NOTE:
//    All XiliListEntry references are only guarenteed to be valid
//    until the next additive or removal method.
//
//    TODO:  should put some mechanism to permit someone to test if
//           the reference has been made invalid.
//
template<class Type> class XiliList
{
public:
    //
    //  List Status Information Methods
    //
    Xil_boolean      isEmpty()  const { return listLength == 0;  }
    int              length()   const { return listLength;       }
    XiliListPosition head()     const { return listHead;         }
    XiliListPosition tail()     const { return listTail;         }


    //
    //  Adding Elements to the List
    //
    XiliListPosition append(Type* new_entry)
    {
        if(listLength == 0) {
            //
            //  Empty list
            //
            listHead = listTail = new XiliListEntry<Type>(new_entry);
        
            listHead->nextEntry = NULL;
            listHead->prevEntry = NULL;

        } else {
            XiliListEntry<Type>* le   = new XiliListEntry<Type>(new_entry);
            le->prevEntry       = listTail;
            le->nextEntry       = NULL;
        
            listTail->nextEntry = le;
            listTail            = le;
        }
    
        listLength++;

        return listTail;
    }
        
    XiliListPosition prepend(Type* new_entry)
    {
        if(listLength == 0) {
            //
            //  Empty list
            //
            listHead = listTail = new XiliListEntry<Type>(new_entry);

            listHead->nextEntry = NULL;
            listHead->prevEntry = NULL;
        } else {
            XiliListEntry<Type>* le   = new XiliListEntry<Type>(new_entry);
            le->prevEntry       = NULL;
            le->nextEntry       = listHead;

            listHead->prevEntry = le;
            listHead            = le;
        }
    
        listLength++;

        return listHead;
    }

    XiliListPosition insertBefore(Type*            new_entry,
                                  XiliListPosition before_position);
    XiliListPosition insertAfter(Type*            new_entry,
                                 XiliListPosition after_position);

    //
    //  Element Removal
    //
    void             remove(XiliListPosition remove_position)
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

            delete tmp;
        }
    }

    Type*            removeEntry(XiliListPosition remove_position);

    //
    //  Element Retrieval Methods
    //
    Type*            reference(XiliListPosition position)
    {
        return ((XiliListEntry<Type>*)position)->entry;
    }

    Type*            reference(unsigned int     entry_number);

    //
    //  Element Locator Methods
    //
    XiliListPosition find(Type* entry);

    //
    //  List Traversal Methods
    //
    //  NOTE:  It's best to use the iterators for moving through the list
    //         since the underlying implementation may change and the
    //         performance of these may change dramatically.
    //
    XiliListPosition next(XiliListPosition position) const
    {
        return ((XiliListEntry<Type>*)position)->nextEntry;
    }
    
    XiliListPosition previous(XiliListPosition position) const
    {
        return ((XiliListEntry<Type>*)position)->prevEntry;
    }

    //
    //  Equal operator
    //
    int              operator == (XiliList<Type>& rval) const;
    XiliList<Type>&  operator =  (XiliList<Type>& rval);
    
    //
    //  Constructor/Destructor
    //
                     XiliList()
    {
        listLength    = 0;
        listHead      = NULL;
        listTail      = NULL;
    }

                     ~XiliList();

    //
    //  Empty the elements from the list.
    //
    void             emptyList();

#ifndef _WINDOWS
private:
#endif
    
    XiliListEntry<Type>* listHead;
    XiliListEntry<Type>* listTail;
    unsigned int         listLength;
    
    //
    //  Iterator Friends
    //
#if 0
#if !defined(_WINDOWS) && !defined(IRIX) && !defined(HPUX) && !defined(GCC)
    friend XiliListIterator<Type>;
    friend XiliListIteratorReverse<Type>;
#endif
#endif
};


//------------------------------------------------------------------------
//
//  Class:	XiliListIterator/XiliListIteratorReverse
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
//  Notes:
//	
//	
//  Deficiencies/ToDo:
//	
//	
//------------------------------------------------------------------------
template <class Type>
class XiliListIterator {
public:
    Type*             getNext();

    XiliListPosition  getCurrentPosition()
    {
        return curPos;
    }

    XiliListIterator(XiliList<Type>*  initial_list)
    {
        list    = initial_list;
        curPos  = _XILI_LIST_INVALID_POSITION;
    }

    XiliListIterator(XiliList<Type>*  initial_list,
                     XiliListPosition start_position)
    {
        list    = initial_list;
        curPos  = start_position;
    }

protected:
    XiliList<Type>*   list;
    XiliListPosition  curPos; 
};

template <class Type>
class XiliListIteratorReverse {
public:
    //
    //  TODO: 2/7/96 jlf  Another compiler bug!
    //
    //    This is here due to another compiler bug!  If the method is left
    //    inline in XiliList.hh then any test of curPos causes it to test a
    //    wrong value and act as if it's set even if it and list->head() are
    //    NULL. 
    //
    Type*             getNext();

    XiliListPosition  getCurrentPosition()
    {
        return curPos;
    }

    void                setCurrentPosition(XiliListPosition pos)
    {
	curPos = pos;
    }

    XiliListIteratorReverse(XiliList<Type>* initial_list)
    {
        list    = initial_list;
	curPos  = _XILI_LIST_INVALID_POSITION;
    }

    XiliListIteratorReverse(XiliList<Type>*  initial_list,
                            XiliListPosition start_position)
    {
        list    = initial_list;
	curPos  = start_position;
    }

protected:
    XiliList<Type>*   list;
    XiliListPosition  curPos; 
};
#endif // _XILI_LIST_HH
