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
//  File:       Block.hh
//  Project:    XIL
//  Revision:   1.4
//  Last Mod:   10:23:35, 03/10/00
//
//  Description:
//
//  Declaration of and Block class templates
//
//  A Block maintins a two dimensional array of elements.
//  Block allows users to use the block class as they
//  would any almost any 2 dimensional array.  The [] operator for
//  Block actually returns a pointer to the row in the block
//
//  inline routines:
//
//   operator()(int i):
//                returns a reference to the value associated 
//      with the data in row i/height, column i%width of the block.
//
//   Block<>::operator[](int j):
//       Returns a pointer to row j
//
//  Note:  This originally contained a _Block_ base class for Block
//         which actually created the space for the block and handled
//         the last level of indexing.  That proved to be too much
//         for the HP Cfront based compiler and was not necessary
//         so it was removed
//
//------------------------------------------------------------------------
//    COPYRIGHT
//------------------------------------------------------------------------
#pragma ident   "@(#)Block.hh	1.4\t00/03/10  "

#ifndef BLOCK_HH
#define BLOCK_HH


template <class ElementType, int height, int width>
class Block {
public:
    Block(void) { }
    Block(const Block<ElementType, height, width>& b) { *this = b; }

    int    rows(void)    { return height; }
    int    columns(void) { return width;  }

    ElementType* addr(int i = 0, int j = 0) {
        return &blk[i][j];
    }

    void   Initialize(ElementType val);

    ElementType&     operator() (int i) { return blk[i/height][i%width]; }

	ElementType*     operator[] (int j) {
		return &blk[j][0];
	}

    Block<ElementType, height, width>&
    operator=  (const Block<ElementType, height, width>& rval);

    ElementType average();
    int         variance();  

protected:
    int          dummy;  //  This int at the beginning is here to
                          //   ensure that blk is 4-byte aligned
    ElementType  blk[height][width];
};

#endif // BLOCK_HH
