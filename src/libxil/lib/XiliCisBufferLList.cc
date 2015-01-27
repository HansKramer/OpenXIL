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
//  File:       XiliCisBufferLList.cc
//  Project:    XIL
//  Revision:   1.3
//  Last Mod:   10:09:08, 03/10/00
//
//  Description:
//
//    Implementation of a linked list of XilCisBuffer objects.
//    Used by XilCisBufferManager.
//
//
//-----------------------------------------------------------------------
//    COPYRIGHT
//-----------------------------------------------------------------------
#pragma ident   "@(#)XiliCisBufferLList.cc	1.3\t00/03/10  "

#include "_XilSystemState.hh"
#include "XiliCisBufferLList.hh"

XiliCisBufferLList::XiliCisBufferLList(void)
{
  head            = new CBListNode;
  if (head == NULL) {
    XIL_ERROR( NULL, XIL_ERROR_RESOURCE,"di-1",TRUE);
    tail = NULL;
    return;
  }
  
  tail            = head;
  head->next      = 0;
  head->previous  = 0;
  head->element   = (XilCisBuffer*)NULL;  
  num_of_elements = 0;
}

XiliCisBufferLList* XiliCisBufferLList::ok()
{
   if (this) {
      if (head==NULL) {
         delete this;
         return NULL;
      } else {
         return this;
      }
   } else {
      return NULL;
   }
}

XiliCisBufferLList::~XiliCisBufferLList(void)
{
  makeNull ();
  delete head;
  head = NULL;
}

//--------------------------------------------------------------
// makeNull deletes all nodes on the list : Note this does
//   not delete the elements at the nodes
//--------------------------------------------------------------

void 
XiliCisBufferLList::makeNull(void)
{
  CBListNode* tmp = head;
  
  while (tmp != tail){
    CBListNode* tmp1 = tmp;
    tmp = tmp->next;
    delete tmp1;
  }
  
  head = tmp;
  num_of_elements = 0;
  if (head) {
    head->next = NULL;
    head->previous = NULL;
  }
}

//--------------------------------------------------------------
// empty returns true if the number of elements is zero
//--------------------------------------------------------------

Xil_boolean 
XiliCisBufferLList::empty(void)
{
  return (num_of_elements == 0);
}


//-----------------------------------------------------------
// append appends the element to the end of the list and
// returns the position/node into which the element was
// added into.
//-----------------------------------------------------------
  
CBListNode* 
XiliCisBufferLList::append(XilCisBuffer* element_to_append)
{
  CBListNode* tmp = new CBListNode;
  if (tmp == NULL) {
    XIL_ERROR( NULL, XIL_ERROR_RESOURCE,"di-1",TRUE);
    return (CBListNode*)NULL;
  }
  
  tail->element = element_to_append;
  tail->next = tmp;
  tmp->previous = tail;
  tail = tmp;
  
  num_of_elements++;

  return tail->previous;
}


//-----------------------------------------------------------
// insert inserts the element Before the given position and
// returns the position/node into which the element was
// added into.
//-----------------------------------------------------------
  
CBListNode* 
XiliCisBufferLList::insert(XilCisBuffer*          element_to_insert,
                          CBListNode* position)
{
  if (position == tail)
    return append(element_to_insert);
  else {
      CBListNode* tmp = position->next;
      
      position->next = new CBListNode;
      if (position->next == NULL) {
        XIL_ERROR( NULL, XIL_ERROR_RESOURCE,"di-1",TRUE);
        return (CBListNode*)NULL;
      }

      position->next->element = position->element;
      position->next->next = tmp;
      position->element = element_to_insert;
      position->next->previous = position;
      tmp->previous = position->next;
      num_of_elements++;

      return position;
    }
}


//-------------------------------------------------------------
// insertAfter inserts the element After the given position
// and returns the position/node into which the element was
// added into.
//-----------------------------------------------------------

CBListNode* 
XiliCisBufferLList::insertAfter(XilCisBuffer*          element_to_insert,
                               CBListNode* position)
{
  if (position == tail->previous)
    return append(element_to_insert);
  else {
      CBListNode* tmp = position->next;
      
      position->next = new CBListNode;
      if (position->next == NULL) {
        XIL_ERROR( NULL, XIL_ERROR_RESOURCE,"di-1",TRUE);
        return (CBListNode*)NULL;
      }
      
      position->next->element = element_to_insert;
      position->next->next = tmp;
      position->next->previous = position;
      tmp->previous = position->next;
      num_of_elements++;

      return position->next;
    }
}

//-------------------------------------------------------------
// store stores the element at the node associated with the
// given position. The position may be given as an integer.
// In this case, the element is stored at the that integer
// position in the list. If integer position is not valid,
// 0 is returned - else successful 1 returned.
//-------------------------------------------------------------

void 
XiliCisBufferLList::store(XilCisBuffer*          element_to_store,
                         CBListNode* position)
{
  position->element = element_to_store;
}


#if 0
int 
XiliCisBufferLList::store(XilCisBuffer* element_to_store,
                         int            position)
                      
{
  CBListNode* tmp = getPosition(position);
  
  if (tmp==0){
    // Error in new implementation of Store::XiliCisBufferLList
    // Internal error. (UNREACHABLE)
    XIL_ERROR( NULL, XIL_ERROR_SYSTEM, "di-95", FALSE);
    return 0;
  }
  
  tmp->element = element_to_store;
  return 1;
}
#endif

//-----------------------------------------------------------
// remove removes the node associated with the given position
// and returns the element at this removed node.
//-----------------------------------------------------------

XilCisBuffer* 
XiliCisBufferLList::remove(CBListNode* position)
{
  if (position == tail){

     // Can't Delete The End of the XiliCisBufferLList
     // Internal error. (UNREACHABLE)
     XIL_ERROR( NULL, XIL_ERROR_SYSTEM, "di-95", FALSE);

     return (XilCisBuffer*)NULL;  // No element removed
  }

  XilCisBuffer* removed_elem = position->element;

  position->next->previous = position->previous;

  if (position->previous)
    position->previous->next = position->next;
  else
    head = position->next;
  
  delete position;
  num_of_elements--;

  return removed_elem;
}


//-------------------------------------------------------------
// retrieve retrieves the element at the node associated with
// the given position. As with store, an integer may be used as
// an index. A null element is returned if the position is
// not valid.
//-------------------------------------------------------------
  
XilCisBuffer* 
XiliCisBufferLList::retrieve(CBListNode* pos)
{
  if (pos != tail)
    return pos->element;
  else
    return (XilCisBuffer*)NULL;
}

#if 0
XilCisBuffer* 
XiliCisBufferLList::retrieve(int num)
{
  CBListNode* tmp = head;
  int i=0;
  
  while (tmp != tail)
    if (i==num)
      return (tmp->element);
    else{
      tmp = tmp->next;
      i++;
    }
  return (XilCisBuffer*)NULL;  // Element was Not Found
}
#endif

//--------------------------------------------------------------
// deletePtrElements deletes all the elements in the list
//--------------------------------------------------------------
  
void 
XiliCisBufferLList::deletePtrElements()
{
  CBListNode* tmp = head;
  
  while (tmp != tail) {
    delete tmp->element;
    tmp->element = NULL;
    tmp = tmp->next;
  }
}
