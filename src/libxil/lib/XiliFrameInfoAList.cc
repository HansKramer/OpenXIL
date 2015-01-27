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
//  File:       XiliFrameInfoAList.cc
//  Project:    XIL
//  Revision:   1.2
//  Last Mod:   10:09:09, 03/10/00
//
//  Description:
//
//	Implementation for XiliFrameInfoAList object.
//      USes an array to store XiliFrameInfo objects.
//
//------------------------------------------------------------------------
//    COPYRIGHT
//------------------------------------------------------------------------
#pragma ident   "@(#)XiliFrameInfoAList.cc	1.2\t00/03/10  "

#include "_XilSystemState.hh"
#include "XiliFrameInfoAList.hh"

//-----------------------------------------------------------
// Constructor: default initial size and growth factor
//-----------------------------------------------------------
  
XiliFrameInfoAList::XiliFrameInfoAList (unsigned init_size, 
                                        unsigned init_growth)
{
  isok = FALSE;
  tail            = 0;
  num_of_elements = 0;
  growth_factor = (init_growth) ? init_growth : (init_size/2 + 5);
  current_size = init_size;  
  list_alloc = list = NULL;  

  list_alloc = list = new XiliFrameInfo*[init_size];
  if (list_alloc == NULL) {
    XIL_ERROR( NULL, XIL_ERROR_RESOURCE,"di-1",TRUE);
    return;
  }
  isok = TRUE;
}


//-----------------------------------------------------------
// ok: check creation of object
//-----------------------------------------------------------
  
XiliFrameInfoAList* 
XiliFrameInfoAList::ok ()
{
  if (this==NULL) {		
    return NULL;	// allocation of memory for object failed
  }
  else {			// allocation succeeded,  was object created?
    if (isok == FALSE) {
      delete this;		// problems with object creation, not OK
      return NULL;		// return NULL
    }
    else {
      return this;		// object created, return ptr to object
    }
  }
}

XiliFrameInfoAList::~XiliFrameInfoAList (void)
{
  delete []list_alloc;
  list = list_alloc = NULL;
}

//--------------------------------------------------------------
// makeNull removes all nodes on the list : Note this does
//   not delete the elements at the nodes. This is as simple
//   as setting num_of_elements and tail to 0.
//--------------------------------------------------------------

void 
XiliFrameInfoAList::makeNull(void)
{
  num_of_elements = tail = 0;
}

//-----------------------------------------------------------
// make the list large by the growth amount. returns true
// if allocation successful, otherwise false is returned.
//-----------------------------------------------------------

int  
XiliFrameInfoAList::grow()
{
  XiliFrameInfo** new_list = new XiliFrameInfo*[current_size + growth_factor];

  if (new_list == NULL) {
    XIL_ERROR( NULL, XIL_ERROR_RESOURCE,"di-1",TRUE);
    return 0;
  }
  
  int p;
  
  for (p = start(); p != end(); p = next(p))
    new_list[p] = list[p];
  
  delete []list_alloc;
  
  list_alloc = list = new_list;
  
  current_size += growth_factor;
  growth_factor *= 2;
  
  return 1;
}


int  
XiliFrameInfoAList::empty(void)
{
  if (num_of_elements == 0)
    return 1;
  else
    return 0;
}


//-----------------------------------------------------------
// append appends the element to the end of the list and
// returns the position/node into which the element was
// added into.
//-----------------------------------------------------------

int 
XiliFrameInfoAList::append(XiliFrameInfo* element_to_append)
{
  if (!inBounds(tail))
    if (!grow()) {
      // problem growing to new size, already reported, just return
      return -1;
    }
  
  list[tail++] = element_to_append;
  num_of_elements++;

  return (tail - 1);
}


//---------------------------------------------------------
// insert inserts the element Before the given position and
// returns the position/node into which the element was
// added into.
//-----------------------------------------------------------

int 
XiliFrameInfoAList::insert(XiliFrameInfo* element_to_insert,
                           int            position)
{
  
  if (position == tail)
    return append(element_to_insert);
  else {
    if (validPosition(position)){
      if (!inBounds(tail))
        if (!grow()) {
          // problem growing to new size, already reported, just return
          return -1;
        }
      
      for (int tmp = tail; tmp != position; tmp--)
        list[tmp] = list[tmp-1];
      
      list[position] = element_to_insert;
      tail++;
      num_of_elements++;
    } else {
      XIL_ERROR(NULL,XIL_ERROR_USER,"di-185",TRUE);
      return -1;
    }
  }
  return position;
}


//-----------------------------------------------------------
// insertAfter inserts the element After the given position
// and returns the position/node into which the element was
// added into.
//-----------------------------------------------------------

int 
XiliFrameInfoAList::insertAfter(XiliFrameInfo* element_to_insert,
                                int            position)
{
  
  if (position == tail-1)
    return append(element_to_insert);
  else {
    if (validPosition(position)){
      if (!inBounds(tail))
        if (!grow()) {
          // problem growing to new size, already reported, just return
          return -1;
        }
      
      for (int tmp = tail; tmp != position+1; tmp--)
        list[tmp] = list[tmp-1];
      
      list[position+1] = element_to_insert;
      tail++;
      num_of_elements++;
    } else {
      XIL_ERROR(NULL,XIL_ERROR_USER,"di-185",TRUE);
      return -1;
    }
  }
  return position+1;
}


//-------------------------------------------------------------
// store stores the element at the node associated with the
// given position.  If integer position is not valid,
// 0 is returned - else successful 1 returned.
//-------------------------------------------------------------
  
int  
XiliFrameInfoAList::store(XiliFrameInfo* element_to_store,
                          int            position)
{
  if (validPosition(position)){
    list[position] = element_to_store;
    return 1;
  } else {
    XIL_ERROR(NULL,XIL_ERROR_USER,"di-185",TRUE);
    return 0;
  }
}


//-----------------------------------------------------------
// remove removes the node associated with the given position
// and returns the element associated with this node.
//-----------------------------------------------------------
  
XiliFrameInfo* 
XiliFrameInfoAList::remove(int position)
{
  if (position == tail && tail > 0){
    num_of_elements--;
    tail--;
    return list[position];
  } else {
    if (validPosition(position)){
      
      XiliFrameInfo* elem_rm = list[position];
      int end = tail-1;
      
      for (int tmp = position; tmp != end; tmp++)
        list[tmp] = list[tmp+1];
      
      tail--;
      num_of_elements--;
      
      return elem_rm;
      
    } else {
      XIL_ERROR(NULL,XIL_ERROR_USER,"di-186",TRUE);
      return (XiliFrameInfo*)(0);
    }
    
  }
}

//-------------------------------------------------------------
// retrieve retrieves the element at the node associated with
// the given position. A null element is returned if the position
// is not valid.
//-------------------------------------------------------------
  
XiliFrameInfo* 
XiliFrameInfoAList::retrieve(int position)
{
  if (validPosition(position)){
    return list[position];
  } else {
    return (XiliFrameInfo*)(0);
  }
}



//-------------------------------------------------------------
// locate traverses the list looking for the given element
// and returns the position type of the first node that contains
// the element. Null is returned if the element is not found.
//--------------------------------------------------------------

int 
XiliFrameInfoAList::locate(XiliFrameInfo* element_to_locate)
{
  int tmp = 0;
  
  while (tmp != tail)
    if (list[tmp] == element_to_locate)
      return (tmp);
    else
      tmp = tmp + 1;;
  
  return (int)(-1);  // Element was Not Found
}

//--------------------------------------------------------------
// deletePtrElements deletes all the elements in the list
//--------------------------------------------------------------

void 
XiliFrameInfoAList::deletePtrElements()
{
  int p = 0;
  
  while (p != tail) {
    delete list[p];
    list[p] = NULL;
    p = p + 1;
  }
}

//--------------------------------------------------------------
// adjustStartTo adjust the start of the list to the desired
// position. This routine does peform deletions on the removed
// objects.
//--------------------------------------------------------------

void 
XiliFrameInfoAList::adjustStartTo(int position)
{
  int p = 0;
  
  while (p != position) {
    delete list[p];
    list[p] = NULL;
    p = p + 1;
  }
  list += position;
  num_of_elements -= position;
  current_size -= position;
  tail = num_of_elements;
}
