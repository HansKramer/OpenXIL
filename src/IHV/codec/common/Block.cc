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
//  File:       Block.cc
//  Project:    XIL
//  Revision:   1.3
//  Last Mod:   10:16:04, 03/10/00
//
//  Description:
//
//    Contains the template definitions for the Block class member functions.
//
//
//------------------------------------------------------------------------
//    COPYRIGHT
//------------------------------------------------------------------------
#pragma ident   "@(#)Block.cc	1.3\t00/03/10  "


#include "xil/xilGPI.hh"
#include "Block.hh"

//------------------------------------------------------------------------
//
//  Function:operator=
//  Created:92/04/20
//
//  Description:
//    Copies the Block rval into the current class such that the
//   current class is equal to the rval.
//
//  Parameters:
//    const Block<ElementType, height, width>&:
//    The Block to equate.
//
//  Returns:
//    Block<ElementType, height, width>&:
//    A reference to this class for use in further computations.
//
//  Side Effects:
//    The contents of this Block class will be overwritten.
//
//  Deficiencies/ToDo:
//
//
//------------------------------------------------------------------------
template <class ElementType, int height, int width>
Block<ElementType, height, width>&
Block<ElementType, height, width>::
operator=(const Block<ElementType, height, width>& b)
{
    //  Copy Block...
    for(int i = 0; i<height; i++) {
        for(int j = 0; j<width; j++) {
            blk[i][j] = b.blk[i][j];
        }
    }

    return *this;
}


//------------------------------------------------------------------------
//
//  Function:Initialize(ElementType val)
//  Created:92/04/20
//
//  Description:
//    Initializes all of the elements in the Block to the given
//    value.  This function requires the = operator to be defined
//    for ElementType if it is used.
//
//  Parameters:
//    ElementType val:  The value with which the Block will be
//    initialized.
//
//  Returns:
//    None.
//
//  Side Effects:
//    The contents of this Block class will be overwritten.
//
//  Deficiencies/ToDo:
//
//
//------------------------------------------------------------------------
template <class ElementType, int height, int width>
void
Block<ElementType, height, width>::Initialize(ElementType val)
{
    //
    // Fill block with val
    //
    for(int i=0; i<height; i++) {
        for(int j=0; j<width; j++) {
            blk[i][j] = val;
        }
    }
}
