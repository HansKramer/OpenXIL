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
//  File:       Dct_S.cc
//  Project:    XIL
//  Revision:   1.5
//  Last Mod:   10:16:08, 03/10/00
//
//  Description:
//    For SPARC platforms , this provides an interface to the 
//    SPARC assembler version of the 1-dimensional forward DCT.
//    A C language layer performs the 8x8 DCT in a separable manner.
//
//    First, the 1-D DCT is done in place on each row. Pixel stride is 1.
//    Second, the 1-D DCT is done in place on the resulting columns, 
//    where pixel stride is 8.
//
//    An assembly version of Lee's 1-D DCT is used.
//
//  Parameters:
//    Block b: Pointer to a Block of data
//
//  Returns:
//    A pointer to the modified Block.
//	
//  Side Effects:
//    The DCT is performed on the supplied Block. The results are
//    placed into the supplied Block.
//

//------------------------------------------------------------------------
//    COPYRIGHT
//------------------------------------------------------------------------
#pragma ident   "@(#)Dct_S.cc	1.5\t00/03/10  "

#include "xil/xilGPI.hh"
#include "Dct.hh"

//-----------------------------
// externed assembly routine
//-----------------------------

extern "C" void dct8(int*, int);


int* Dct8x8(int* b) {
    //
    // DCT each row.
    // Pixel stride is 1.
    //
    dct8(b, 1);
    dct8(b+8, 1);
    dct8(b+16, 1);
    dct8(b+24, 1);
    dct8(b+32, 1);
    dct8(b+40, 1);
    dct8(b+48, 1);
    dct8(b+56, 1);
    
    //
    // DCT each resultant column.
    // Pixel stride is 8.
    //
    dct8(b, 8);
    dct8(b+1, 8);
    dct8(b+2, 8);
    dct8(b+3, 8);
    dct8(b+4, 8);
    dct8(b+5, 8);
    dct8(b+6, 8);
    dct8(b+7, 8);
    
    return b;
}
