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
//  File:       GOB.cc
//  Project:    XIL
//  Revision:   1.3
//  Last Mod:   10:16:05, 03/10/00
//
//  Description:
//
//    Contains the template definitions for the GOB class member functions.
//
//
//------------------------------------------------------------------------
//    COPYRIGHT
//------------------------------------------------------------------------
#pragma ident   "@(#)GOB.cc	1.3\t00/03/10  "

#include "xil/xilGPI.hh"
#include "GOB.hh"

//------------------------------------------------------------------------
//
//  Function:	operator=
//  Created:	92/04/20
//
//  Description:
//	Copies the GOB (Group Of Blocks) rval into the current class 
//	such that the current class is equal to the rval.
//	
//  Parameters:
//	const GOB<ElementType, nb, height, width>&:
//	    The GOB to equate.
//	
//  Returns:
//	GOB<ElementType, nb, height, width>&:
//	    A reference to this class for use in further computations.
//	
//  Side Effects:
//	The contents of this GOB class will be overwritten.
//	
//  Deficiencies/ToDo:
//	
//	
//------------------------------------------------------------------------
template <class ElementType,  int height, int width>
GOB<ElementType, height, width>&
GOB<ElementType, height, width>::
operator=(const GOB<ElementType, height, width>& b)
{
    //  Copy GOB...
    for(int i = 0; i<nblks; i++)
      *gob[i] = *b.gob[i];

    return *this;
}
    

//------------------------------------------------------------------------
//
//  Function:	Initialize(ElementType val)
//  Created:	92/04/20
//
//  Description:
//	Initializes all of the elements in the GOB to the given
//	value.  This function requires the = operator to be defined
//	for ElementType if it is used.
//	
//  Parameters:
//	ElementType val:  The value with which the GOB will be
//		initialized.
//	
//  Returns:
//	None.
//	
//  Side Effects:
//	The contents of this GOB class will be overwritten.
//	
//  Deficiencies/ToDo:
//	
//	
//------------------------------------------------------------------------
template <class ElementType, int height, int width>
void
GOB<ElementType, height, width>::Initialize(ElementType val)
{
  // fill groups of blocks with val
  
  for(int i=0; i<nblks; i++)
    gob[i]->Initialize(val);
}

//------------------------------------------------------------------------
//
//  Function:	constructor for GOB
//  Created:	92/04/20
//
//  Description:
//      Constructs the GOB
//	
//  Parameters:
//	
//  Returns:
//	None.
//	
//  Side Effects:
//	
//  Deficiencies/ToDo:
//	
//	
//------------------------------------------------------------------------
template <class ElementType, int height, int width>
GOB<ElementType, height, width>::GOB( int nb )
{
  // fill groups of blocks with val
  
  nblks = nb;
  gob = new Block<ElementType, height, width>*[nblks];
  for(int i=0; i<nblks; i++)
    gob[i] = new Block<ElementType, height, width>;
}



//------------------------------------------------------------------------
//
//  Function:	deconstructor for GOB
//  Created:	92/04/20
//
//  Description:
//      Constructs the GOB
//	
//  Parameters:
//	
//  Returns:
//	None.
//	
//  Side Effects:
//	
//  Deficiencies/ToDo:
//	
//	
//------------------------------------------------------------------------
template <class ElementType, int height, int width>
GOB<ElementType, height, width>::~GOB()
{
  // fill groups of blocks with val
  for(int i=0; i<nblks; i++)
    delete gob[i];
  delete gob;
}
