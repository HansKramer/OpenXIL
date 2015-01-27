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
//  File:       GOB.hh
//  Project:    XIL
//  Revision:   1.3
//  Last Mod:   10:23:34, 03/10/00
//
//  Description:
//
//           Declaration of _GOB_ and GOB class templates
//
//
//------------------------------------------------------------------------
//    COPYRIGHT
//------------------------------------------------------------------------
#pragma ident   "@(#)GOB.hh	1.3\t00/03/10  "

#ifndef GOB_H
#define GOB_H

#include "Block.hh"

//------------------------------------------------------------------------
//
//  Class:        GOB
//
//  Description of Class:
//        
//  A GOB maintins a three dimensional array of elements.
//  The _GOB_ base-class maintains the data and GOB is derived off
//  of this class.  This allows users to use the GOB class as they
//  would any almost any 2 dimensional array.  The [] operator for
//  GOB actually returns a _GOB_ which has the [] operator defined
//  to return the actuall element contained in the array.  Using GOB
//  with a single [] operator doesn't make much sense since it returns
//  inline routines:
//
//   operator()(int i):
//                returns a reference to the value associated 
//      with the data in row i/height, column i%width of the GOB.
//
//   GOB<>::operator[](int j):
//                sets an offset value for the _GOB_::operator[] to
//        use and then returns a _GOB_ object which usually calls the
//        _GOB_::operator[] to return the ElementType.
//
//   _GOB_<>::operator[](int i):
//                returns the element given by [i][offset] where offset
//        was set in GOB<>::operator[]
//                               
//------------------------------------------------------------------------

template <class ElementType, int height, int width>
class _GOB_ {
protected:
    Block < ElementType, height, width >** gob;   
    int nblks;
    _GOB_(void) { }

public:
    Block < ElementType, height, width >& operator[] (int i) {

        return *gob[i];
    }
};
        
template <class ElementType, int height, int width>
class GOB : public _GOB_<ElementType, height, width> {
protected:
    _GOB_<ElementType, height, width>::gob;

public:
    GOB(int nb);
    ~GOB();

    int    rows(void)    { return height; }
    int    columns(void) { return width;  }
    int    numblocks(void) {return nblks; }

    ElementType* addr( int nb, int i = 0, int j = 0) {
        return gob[nb]->addr(i,j);
    }

    void   Initialize(ElementType val);

    ElementType&     operator() (int nb, int i) { 
        return (*gob[nb]) ( i );
    }


    GOB<ElementType, height, width>&
    operator=  (const GOB<ElementType, height, width>& rval);
};

#endif
