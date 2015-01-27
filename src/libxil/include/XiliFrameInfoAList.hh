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
//  File:	XilFrameInfoAList.h
//  Project:	XIL
//  Revision:	1.3
//  Last Mod:	14:35:20, 10/30/94
//
//  Description:
//		
//	Creation of definition for FrameInfo AList
//	
//------------------------------------------------------------------------
//
//	Copyright (c) 1992, 1993, 1994, by Sun Microsystems, Inc.
//
//------------------------------------------------------------------------


#ifndef XILIFRAMEINFOALIST_H
#define XILIFRAMEINFOALIST_H

#include "XiliFrameInfo.hh"

//	
//  The name of the class is a macro that is replaced by true a class
//  name. This class is an array list that can grow if the initial
//  size is not large enough. The list has an initial size and grows
//  by a given amount when required. This amount can be changed at any
//  time. After each growth, the factor doubles. The list is best used
//  when the approximate size of the list is known at construction since
//  accesses will O(1). If the size is unkown, or you expect the list to
//  grow and shrink constantly, the GLList may be a better choice.
//

class XiliFrameInfoAList {
public:

    //-----------------------------------------------------------
    // Constructor: default initial size and growth factor
    //-----------------------------------------------------------
  
  XiliFrameInfoAList (unsigned init_size = 10, unsigned init_growth = 5);
  ~XiliFrameInfoAList (void);

  XiliFrameInfoAList * ok();

    //-----------------------------------------------------------
    // make the list large by the growth amount. returns true
    // if allocation successful, otherwise false is returned.
    //-----------------------------------------------------------
  
  int     grow();

    //-----------------------------------------------------------
    // set growth factor to new growth amount
    //-----------------------------------------------------------
  
  void    setGrowthFactor(unsigned new_gr) { growth_factor = new_gr; }

      //-----------------------------------------------------------
      // append appends the element to the end of the list and
      // returns the position/node into which the element was
      // added into.
      //-----------------------------------------------------------

  int append(XiliFrameInfo* elem_to_append);
  
      //---------------------------------------------------------
      // insert inserts the element Before the given position and
      // returns the position/node into which the element was
      // added into.
      //-----------------------------------------------------------
  
  int insert(XiliFrameInfo* elem_to_insert,
                                  int pos);

  
  int insertBefore(XiliFrameInfo* elem_to_insert,
                                        int pos)
    { return insert(elem_to_insert, pos); }    

      //-----------------------------------------------------------
      // insertAfter inserts the element After the given position
      // and returns the position/node into which the element was
      // added into.
      //-----------------------------------------------------------
  
  int insertAfter(XiliFrameInfo* elem_to_insert,
                                       int pos);
  
      //-----------------------------------------------------------
      // remove removes the node associated with the given position
      // and returns the element associated with this node.
      //-----------------------------------------------------------
  
  XiliFrameInfo* remove(int pos);

      //-------------------------------------------------------------
      // store stores the element at the node associated with the
      // given position.  If integer position is not valid,
      // 0 is returned - else successful 1 returned.
      //-------------------------------------------------------------
  
  int     store(XiliFrameInfo* elem_to_store,
                int pos);
  

      //-------------------------------------------------------------
      // retrieve retrieves the element at the node associated with
      // the given position. A null element is returned if the position
      // is not valid.
      //-------------------------------------------------------------
  
  
  XiliFrameInfo*  retrieve(int pos);

      //-------------------------------------------------------------
      // locate transverses the list looking for the given element
      // and returns the position type of the first node that contains
      // the element. Null is returned if the element is not found.
      //--------------------------------------------------------------
    
  int locate(XiliFrameInfo* elem_to_locate);

      //-------------------------------------------------------------
      // start returns the head of the list
      // end returns the tail of the list
      //--------------------------------------------------------------
    
  int start(void) const { return 0; }
  int end(void)   const { return tail; }

      //-------------------------------------------------------------
      // next returns the next position from the specified position
      // previous returns the previous position from the specified
      // position
      //--------------------------------------------------------------
    
  int next(int position)
    const {return position + 1; }
  int previous(int position)
    const {return position - 1;}

      //--------------------------------------------------------------
      // length returns the number of elements
      // empty returns true if the number of elements is zero
      // makeNull deletes all nodes on the list : Note this does
      //   not delete the elements at the nodes
      //--------------------------------------------------------------
    
  unsigned length(void) const { return num_of_elements; }
  int      empty(void);
  void     makeNull(void);


      //--------------------------------------------------------------
      // deletePtrElements deletes all the elements in the list
      //--------------------------------------------------------------
  
  void     deletePtrElements();

      //--------------------------------------------------------------
      // adjustStartTo adjust the start of the list to the desired
      // position. This routine does peform deletions on the removed
      // objects.
      //--------------------------------------------------------------
  
  void     adjustStartTo(int position);

private:
  Xil_boolean isok;	// constructor creation flag

  XiliFrameInfo**  list_alloc;
  XiliFrameInfo**  list;
  int  tail;
  
  unsigned       num_of_elements;
  unsigned       growth_factor;
  unsigned       current_size;  

  //-----------------------------------------------------------
  // True if position is valid (<num of elements) else false
  //-----------------------------------------------------------
  
  int     validPosition(int p)
    const { return (p<num_of_elements && p>=0); }

  //-----------------------------------------------------------
  // True if position is within the current size of the list
  //-----------------------------------------------------------
  
  int     inBounds(int p)
    const { return p < current_size; }  
  
};

typedef int XiliFrameInfoAListPositionType;

#endif // XILFRAMEINFOALIST_H
