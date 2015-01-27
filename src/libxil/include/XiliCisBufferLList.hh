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
//  File:       XiliCisBufferLList.hh
//  Project:    XIL
//  Revision:   1.2
//  Last Mod:   10:22:17, 03/10/00
//
//  Description:
//
//    Class definition for the XiliCisBufferLList class
//
//
//------------------------------------------------------------------------
//    COPYRIGHT
//------------------------------------------------------------------------
#pragma ident   "@(#)XiliCisBufferLList.hh	1.2\t00/03/10  "

#ifndef XILICISBUFFERLLIST_H
#define XILICISBUFFERLLIST_H

#include "_XilDefines.h"
#include "_XilCisBuffer.hh"
#include "XilError.hh"

//
// Define a simple node to hold a ptr to the XilCisBuffer
// and next/previous ptrs
//
class CBListNode {

friend class XiliCisBufferLList;
  
private:
  CBListNode*   next;
  CBListNode*   previous;
  XilCisBuffer* element;
  
};

class XiliCisBufferLList {
public:
  
                       XiliCisBufferLList(void);
                       ~XiliCisBufferLList(void);

    XiliCisBufferLList* ok();
  
    //-----------------------------------------------------------
    // append appends the element to the end of the list and
    // returns the position/node into which the element was
    // added into.
    //-----------------------------------------------------------
    CBListNode* append(XilCisBuffer* element_to_append);

    //-----------------------------------------------------------
    // insert inserts the element Before the given position and
    // returns the position/node into which the element was
    // added into.
    //-----------------------------------------------------------
    CBListNode* insert(XilCisBuffer* elem_to_insert,
                       CBListNode*   pos);

    CBListNode* insertBefore(XilCisBuffer* elem_to_insert,
                             CBListNode*   pos)
        { return insert(elem_to_insert, pos); }
  
    //-------------------------------------------------------------
    // insertAfter inserts the element After the given position
    // and returns the position/node into which the element was
    // added into.
    //-----------------------------------------------------------
    CBListNode* insertAfter(XilCisBuffer* elem_to_insert,
                            CBListNode*   pos);
  
    //-----------------------------------------------------------
    // remove removes the node associated with the given position
    // and returns the element at this removed node.
    //-----------------------------------------------------------
    XilCisBuffer* remove(CBListNode* positon);

    //-------------------------------------------------------------
    // store stores the element at the node associated with the
    // given position. The position may be given as an integer.
    // In this case, the element is stored at the that integer
    // position in the list. If integer position is not valid,
    // 0 is returned - else successful 1 returned.
    //-------------------------------------------------------------
    void    store(XilCisBuffer* element_to_store,
                  CBListNode*   position);
  
#if 0
    int     store(XilCisBuffer* element_to_store, int position);
#endif

    //-------------------------------------------------------------
    // retrieve retrieves the element at the node associated with
    // the given position. As with store, an integer may be used as
    // an index. A null element is returned if the position is
    // not valid.
    //-------------------------------------------------------------
    XilCisBuffer* retrieve(CBListNode* position);
#if 0
    XilCisBuffer* retrieve(int);
#endif
  
    //-------------------------------------------------------------
    // start returns the head of the list
    // end returns the tail of the list
    //--------------------------------------------------------------
    CBListNode* start(void) const 
        { return head; }

    CBListNode* end(void)   const 
        { return tail; }

    //-------------------------------------------------------------
    // next returns the next position from the specified position
    // previous returns the previous position from the specified
    // position
    //--------------------------------------------------------------
    CBListNode* next(CBListNode* position) const 
        {return position->next; }

    CBListNode* previous(CBListNode* position) const 
        {return position->previous; };

    unsigned int length(void) const 
        { return num_of_elements; }

    Xil_boolean  empty(void);

    //
    // Delete all nodes, but not node contents
    //
    void         makeNull (void);

  

    //
    // deletePtrElements deletes all the elements in the list
    //
    void     deletePtrElements();
  
private:
    CBListNode*  head;
    CBListNode*  tail;
    unsigned int num_of_elements;
  

};

typedef CBListNode* XiliCisBufferLListPositionType;


#endif // XILICISBUFFERLLIST_H
