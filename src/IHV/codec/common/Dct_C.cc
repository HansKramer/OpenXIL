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
//  File:       Dct_C.cc
//  Project:    XIL
//  Revision:   1.4
//  Last Mod:   10:16:19, 03/10/00
//
//  Description:
//
//    Contains the C version of the Dct8 routine.
//
//
//------------------------------------------------------------------------
//    COPYRIGHT
//------------------------------------------------------------------------
#pragma ident   "@(#)Dct_C.cc	1.4\t00/03/10  "


#include "xil/xilGPI.hh"
#include "Dct.hh"

// integer cosine table used by both forward and inverse
//  dct routines:
//     cosine[i] = (int)((cos((Pi * i)/16) * 65536.0) + 0.5)

#define COS_0  65536	   /* 0x10000 */
#define COS_1	64277	   /* 0x0fb15 */
#define COS_2	60547	   /* 0x0ec83 */
#define COS_3	54491	   /* 0x0d4db */
#define COS_4  46341	   /* 0x0b505 */
#define COS_5	36410   	/* 0x08e3a */
#define COS_6	25080	   /* 0x061f8 */
#define COS_7	12785    /* 0x031f1 */

void Dct8(int* b, int i, int j, int dx);

//------------------------------------------------------------------------
//
//  Function:	Dct8x8
//  Created:	92/03/24
//
//  Description:
//	
//   Perform a 2D forward dct transfrom on the passed Block as
//   defined by Lee's C version of the forward DCT.
//
//  Parameters:
//	
//   int* b: Pointer to a Block of data
//
//  Returns:
//	
//    A pointer to the modified Block is returned
//	
//  Side Effects:
//	
//    The DCT is performed on the supplied Block. The results are
//    placed into the supplied Block.
//
//------------------------------------------------------------------------


int* Dct8x8(int* b)
{
  Dct8(b, 0, 0, 1);
  Dct8(b, 1, 0, 1);
  Dct8(b, 2, 0, 1);
  Dct8(b, 3, 0, 1);
  Dct8(b, 4, 0, 1);
  Dct8(b, 5, 0, 1);
  Dct8(b, 6, 0, 1);
  Dct8(b, 7, 0, 1);
  
  Dct8(b, 0, 0, 8);
  Dct8(b, 0, 1, 8);
  Dct8(b, 0, 2, 8);
  Dct8(b, 0, 3, 8);
  Dct8(b, 0, 4, 8);
  Dct8(b, 0, 5, 8);
  Dct8(b, 0, 6, 8);
  Dct8(b, 0, 7, 8);
  
  return b;
}

//------------------------------------------------------------------------
//
//  Function:	Dct8
//  Created:	92/03/24
//
//  Description:
//
//  Parameters:
//	
//    int* b:    Pointer to a Block of data
//    int i:     row index into Block
//    int j:     col index into Block
//    int dx:    stride (delta) to next element in Block
//
//  Side Effects:
//
//    Values within the Block are changed
//
//------------------------------------------------------------------------

void Dct8(int* b, int i, int j, int dx)
{
  
  int *xptr = b + i*8 + j;
  int *x    = xptr;
  int a0, a1, a2, a3, a4, a5, a6, a7;
  int b0, b1, b2, b3, b4, b5, b6, b7;
  int half = 32768;
  
  // uses 27 adds and 16 mults per 8 element dct
  
  b0 = *xptr;	xptr += dx;
  b1 = *xptr;	xptr += dx;
  b2 = *xptr;	xptr += dx;
  b3 = *xptr;	xptr += dx;
  b4 = *xptr;	xptr += dx;
  b5 = *xptr;	xptr += dx;
  b6 = *xptr;	xptr += dx;
  b7 = *xptr;
  
  a0 = b0 + b7;
  a1 = b1 + b6;
  a2 = b2 + b5;
  a3 = b3 + b4;
  a4 = b3 - b4;
  a5 = b2 - b5;
  a6 = b1 - b6;
  a7 = b0 - b7;
  
  b0 = a0 + a3;
  b1 = a1 + a2;
  b2 = a1 - a2;
  b3 = a0 - a3;
  
  b5 = ((a6 - a5) * COS_4 + half) >> 16;
  b7 = ((a5 + a6) * COS_4 + half) >> 16;

  
  b4 = a4 + b5;
  b5 = a4 - b5;
  b6 = a7 - b7;
  b7 = b7 + a7;
  
  a0 = (b0 + b1) * COS_4;
  *x = (a0 + half) >> 16;
  x += dx;
  a1 = b4 * COS_7 + b7 * COS_1;
  *x = (a1 + half) >> 16;
  x += dx;
  a2 = b2 * COS_6 + b3 * COS_2;
  
  *x = (a2 + half) >> 16;
  x += dx;
  a3 = b6 * COS_3 - b5 * COS_5;
  *x = (a3 + half) >> 16;
  x += dx;
  a4 = (b0 - b1) * COS_4;
  *x = (a4 + half) >> 16;
  x += dx;
  a5 = b5 * COS_3 + b6 * COS_5;
  *x = (a5 + half) >> 16;
  x += dx;
  a6 = b3 * COS_6 - b2 * COS_2;
  *x = (a6 + half) >> 16;
  x += dx;
  a7 = b7 * COS_7 - b4 * COS_1;
  *x = (a7 + half) >> 16;
}


