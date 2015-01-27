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
//  File:       Idct.cc
//  Project:    XIL
//  Revision:   1.3
//  Last Mod:   10:16:10, 03/10/00
//
//  Description:
//
//    C Implementation of the Inverse DCT using
//    the splatter algorithm
//
//
//------------------------------------------------------------------------
//    COPYRIGHT
//------------------------------------------------------------------------
#pragma ident   "@(#)Idct.cc	1.3\t00/03/10  "

#define HALF 32768

#include "xil/xilGPI.hh"
#include "IdctSplatter.hh"

#ifdef XIL_LITTLE_ENDIAN
#define PUT_SHORTS_INTO_INT(a,b)	((b << 16) + a)
#else
#define PUT_SHORTS_INTO_INT(a,b)	((a << 16) + b)
#endif

void idct(int *result, int *coefflist)

{
    int i, t;
    int *coeff;
    int a0, a1, a2, a3;
    int b0, b1, b2, b3;
    int c0, c1, c2, c3;
    int d0, d1, d2, d3;

    i = 0;
    a0 = a1 = a2 = a3 = HALF;
    b0 = b1 = b2 = b3 = 0;
    c0 = c1 = c2 = c3 = 0;
    d0 = d1 = d2 = d3 = 0;

    t = coefflist[0];
    do {
	coeff = (int *)(t & ~63);
	t = t & 63;

	switch (t) {
	  case 0:
	    /*
	      Coefficient[0][0]

	      C44  C44  C44  C44  C44  C44  C44  C44 
	      C44  C44  C44  C44  C44  C44  C44  C44 
	      C44  C44  C44  C44  C44  C44  C44  C44 
	      C44  C44  C44  C44  C44  C44  C44  C44 
	      C44  C44  C44  C44  C44  C44  C44  C44 
	      C44  C44  C44  C44  C44  C44  C44  C44 
	      C44  C44  C44  C44  C44  C44  C44  C44 
	      C44  C44  C44  C44  C44  C44  C44  C44 
	    */
	    t = coeff[0];	/* C44 */
	    a0 += t;
	    a1 += t;
	    a2 += t;
	    a3 += t;    
	    break;


	  case 1:
	    /*
	      Coefficient[0][1]

	       C14  C34  C45  C47 -C47 -C45 -C34 -C14 
	       C14  C34  C45  C47 -C47 -C45 -C34 -C14 
	       C14  C34  C45  C47 -C47 -C45 -C34 -C14 
	       C14  C34  C45  C47 -C47 -C45 -C34 -C14 
	       C14  C34  C45  C47 -C47 -C45 -C34 -C14 
	       C14  C34  C45  C47 -C47 -C45 -C34 -C14 
	       C14  C34  C45  C47 -C47 -C45 -C34 -C14 
	       C14  C34  C45  C47 -C47 -C45 -C34 -C14 
	    */
	    t = coeff[4];	/* C14 */
	    b0 += t;    
	    t = coeff[5];	/* C34 */
	    b1 += t;    
	    t = coeff[6];	/* C45 */
	    b2 += t;    
	    t = coeff[7];	/* C47 */
	    b3 += t;
	    break;

	  case 2:
	    /*
	      Coefficient[1][0]

	       C14  C14  C14  C14  C14  C14  C14  C14 
	       C34  C34  C34  C34  C34  C34  C34  C34 
	       C45  C45  C45  C45  C45  C45  C45  C45 
	       C47  C47  C47  C47  C47  C47  C47  C47 
	      -C47 -C47 -C47 -C47 -C47 -C47 -C47 -C47 
	      -C45 -C45 -C45 -C45 -C45 -C45 -C45 -C45 
	      -C34 -C34 -C34 -C34 -C34 -C34 -C34 -C34
	      -C14 -C14 -C14 -C14 -C14 -C14 -C14 -C14 
	    */
	    t = coeff[4];	/* C14 */
	    c0 += t;  
	    c1 += t;
	    c2 += t;
	    c3 += t;    
	    break;

	  case 3:
	    /*
	      Coefficient[2][0]

	       C24  C24  C24  C24  C24  C24  C24  C24 
	       C46  C46  C46  C46  C46  C46  C46  C46 
	      -C46 -C46 -C46 -C46 -C46 -C46 -C46 -C46 
	      -C24 -C24 -C24 -C24 -C24 -C24 -C24 -C24 
	      -C24 -C24 -C24 -C24 -C24 -C24 -C24 -C24 
	      -C46 -C46 -C46 -C46 -C46 -C46 -C46 -C46 
	       C46  C46  C46  C46  C46  C46  C46  C46 
	       C24  C24  C24  C24  C24  C24  C24  C24 
	    */
	    t = coeff[2];	/* C24 */
	    a0 += t;
	    a1 += t;
	    a2 += t;
	    a3 += t;
	    break;


	  case 4:
	    /*
	      Coefficient[1][1]

	       C11  C13  C15  C17 -C17 -C15 -C13 -C11 
	       C13  C33  C35  C37 -C37 -C35 -C33 -C13 
	       C15  C35  C55  C57 -C57 -C55 -C35 -C15 
	       C17  C37  C57  C77 -C77 -C57 -C37 -C17 
	      -C17 -C37 -C57 -C77  C77  C57  C37  C17 
	      -C15 -C35 -C55 -C57  C57  C55  C35  C15 
	      -C13 -C33 -C35 -C37  C37  C35  C33  C13 
	      -C11 -C13 -C15 -C17  C17  C15  C13  C11 
	    */
	    t = coeff[20];	/* C11 */
	    d0 += t;    
	    t = coeff[21];	/* C13 */
	    d1 += t;    
	    t = coeff[22];	/* C15 */
	    d2 += t;    
	    t = coeff[23];	/* C17 */
	    d3 += t;    
	    break;

	  case 5:
	    /*
	      Coefficient[0][2]

	       C24  C46 -C46 -C24 -C24 -C46  C46  C24 
	       C24  C46 -C46 -C24 -C24 -C46  C46  C24 
	       C24  C46 -C46 -C24 -C24 -C46  C46  C24 
	       C24  C46 -C46 -C24 -C24 -C46  C46  C24 
	       C24  C46 -C46 -C24 -C24 -C46  C46  C24 
	       C24  C46 -C46 -C24 -C24 -C46  C46  C24 
	       C24  C46 -C46 -C24 -C24 -C46  C46  C24 
	       C24  C46 -C46 -C24 -C24 -C46  C46  C24 
	    */
	    t = coeff[2];	/* C24 */
	    a0 += t;
	    a3 -= t;    
	    t = coeff[3];	/* C46 */
	    a1 += t;
	    a2 -= t;    
	    break;

	  case 6:
	    /*
	      Coefficient[0][3]

	       C34 -C47 -C14 -C45  C45  C14  C47 -C34 
	       C34 -C47 -C14 -C45  C45  C14  C47 -C34 
	       C34 -C47 -C14 -C45  C45  C14  C47 -C34 
	       C34 -C47 -C14 -C45  C45  C14  C47 -C34 
	       C34 -C47 -C14 -C45  C45  C14  C47 -C34 
	       C34 -C47 -C14 -C45  C45  C14  C47 -C34 
	       C34 -C47 -C14 -C45  C45  C14  C47 -C34 
	       C34 -C47 -C14 -C45  C45  C14  C47 -C34 
	    */
	    t = coeff[4];	/* C14 */
	    b2 -= t;    
	    t = coeff[5];	/* C34 */
	    b0 += t;    
	    t = coeff[6];	/* C45 */
	    b3 -= t;    
	    t = coeff[7];	/* C47 */
	    b1 -= t;    
	    break;

	  case 7:
	    /*
	      Coefficient[1][2]

	       C12  C16 -C16 -C12 -C12 -C16  C16  C12 
	       C23  C36 -C36 -C23 -C23 -C36  C36  C23 
	       C25  C56 -C56 -C25 -C25 -C56  C56  C25 
	       C27  C67 -C67 -C27 -C27 -C67  C67  C27 
	      -C27 -C67  C67  C27  C27  C67 -C67 -C27 
	      -C25 -C56  C56  C25  C25  C56 -C56 -C25 
	      -C23 -C36  C36  C23  C23  C36 -C36 -C23 
	      -C12 -C16  C16  C12  C12  C16 -C16 -C12 
	    */
	    t = coeff[8];	/* C12 */
	    c0 += t;
	    c3 -= t; 
	    t = coeff[12];	/* C16 */
	    c1 += t;
	    c2 -= t;
	    break;

	  case 8:
	    /*
	      Coefficient[2][1]

	       C12  C23  C25  C27 -C27 -C25 -C23 -C12 
	       C16  C36  C56  C67 -C67 -C56 -C36 -C16 
	      -C16 -C36 -C56 -C67  C67  C56  C36  C16 
	      -C12 -C23 -C25 -C27  C27  C25  C23  C12 
	      -C12 -C23 -C25 -C27  C27  C25  C23  C12 
	      -C16 -C36 -C56 -C67  C67  C56  C36  C16 
	       C16  C36  C56  C67 -C67 -C56 -C36 -C16 
	       C12  C23  C25  C27 -C27 -C25 -C23 -C12 
	    */
	    t = coeff[8];	/* C12 */
	    b0 += t;    
	    t = coeff[9];	/* C23 */
	    b1 += t;    
	    t = coeff[10];	/* C25 */
	    b2 += t;    
	    t = coeff[11];	/* C27 */
	    b3 += t;    
	    break;

	  case 9:
	    /*
	      Coefficient[3][0]

	       C34  C34  C34  C34  C34  C34  C34  C34 
	      -C47 -C47 -C47 -C47 -C47 -C47 -C47 -C47 
	      -C14 -C14 -C14 -C14 -C14 -C14 -C14 -C14 
	      -C45 -C45 -C45 -C45 -C45 -C45 -C45 -C45 
	       C45  C45  C45  C45  C45  C45  C45  C45 
	       C14  C14  C14  C14  C14  C14  C14  C14 
	       C47  C47  C47  C47  C47  C47  C47  C47 
	      -C34 -C34 -C34 -C34 -C34 -C34 -C34 -C34 
	    */
	    t = coeff[5];	/* C34 */
	    c0 += t;
	    c1 += t;
	    c2 += t;
	    c3 += t;    
	    break;

	  case 10:
	    /*
	      Coefficient[4][0]
	      
	       C44  C44  C44  C44  C44  C44  C44  C44 
	      -C44 -C44 -C44 -C44 -C44 -C44 -C44 -C44 
	      -C44 -C44 -C44 -C44 -C44 -C44 -C44 -C44 
	       C44  C44  C44  C44  C44  C44  C44  C44 
	       C44  C44  C44  C44  C44  C44  C44  C44 
	      -C44 -C44 -C44 -C44 -C44 -C44 -C44 -C44 
	      -C44 -C44 -C44 -C44 -C44 -C44 -C44 -C44 
	       C44  C44  C44  C44  C44  C44  C44  C44 
	    */
	    t = coeff[0];	/* C44 */
	    a0 += t;
	    a1 += t;
	    a2 += t;
	    a3 += t;    
	    break;

	  case 11:
	    /*
	      Coefficient[3][1]

	       C13  C33  C35  C37 -C37 -C35 -C33 -C13 
	      -C17 -C37 -C57 -C77  C77  C57  C37  C17 
	      -C11 -C13 -C15 -C17  C17  C15  C13  C11 
	      -C15 -C35 -C55 -C57  C57  C55  C35  C15 
	       C15  C35  C55  C57 -C57 -C55 -C35 -C15 
	       C11  C13  C15  C17 -C17 -C15 -C13 -C11 
	       C17  C37  C57  C77 -C77 -C57 -C37 -C17 
	      -C13 -C33 -C35 -C37  C37  C35  C33  C13 
	   */
	    t = coeff[21];	/* C13 */
	    d0 += t;    
	    t = coeff[24];	/* C33 */
	    d1 += t;    
	    t = coeff[25];	/* C35 */
	    d2 += t;    
	    t = coeff[26];	/* C37 */
	    d3 += t;
	    break;

	  case 12:
	    /*
	      Coefficient[2][2]

	       C22  C26 -C26 -C22 -C22 -C26  C26  C22 
	       C26  C66 -C66 -C26 -C26 -C66  C66  C26 
	      -C26 -C66  C66  C26  C26  C66 -C66 -C26 
	      -C22 -C26  C26  C22  C22  C26 -C26 -C22 
	      -C22 -C26  C26  C22  C22  C26 -C26 -C22 
	      -C26 -C66  C66  C26  C26  C66 -C66 -C26 
	       C26  C66 -C66 -C26 -C26 -C66  C66  C26 
	       C22  C26 -C26 -C22 -C22 -C26  C26  C22 
	    */
	    t = coeff[16];	/* C22 */
	    a0 += t;
	    a3 -= t;    
	    t = coeff[17];	/* C26 */
	    a1 += t;
	    a2 -= t;    
	    break;

	  case 13:
	    /*
	      Coefficient[1][3]

	       C13 -C17 -C11 -C15  C15  C11  C17 -C13 
	       C33 -C37 -C13 -C35  C35  C13  C37 -C33 
	       C35 -C57 -C15 -C55  C55  C15  C57 -C35 
	       C37 -C77 -C17 -C57  C57  C17  C77 -C37 
	      -C37  C77  C17  C57 -C57 -C17 -C77  C37 
	      -C35  C57  C15  C55 -C55 -C15 -C57  C35 
	      -C33  C37  C13  C35 -C35 -C13 -C37  C33 
	      -C13  C17  C11  C15 -C15 -C11 -C17  C13 
	    */
	    t = coeff[20];	/* C11 */
	    d2 -= t;    
	    t = coeff[21];	/* C13 */
	    d0 += t;    
	    t = coeff[22];	/* C15 */
	    d3 -= t;    
	    t = coeff[23];	/* C17 */
	    d1 -= t;    
	    break;

	  case 14:
	    /*
	      Coefficient[0][4]

	       C44 -C44 -C44  C44  C44 -C44 -C44  C44 
 	       C44 -C44 -C44  C44  C44 -C44 -C44  C44 
	       C44 -C44 -C44  C44  C44 -C44 -C44  C44 
	       C44 -C44 -C44  C44  C44 -C44 -C44  C44 
	       C44 -C44 -C44  C44  C44 -C44 -C44  C44 
	       C44 -C44 -C44  C44  C44 -C44 -C44  C44 
	       C44 -C44 -C44  C44  C44 -C44 -C44  C44 
	       C44 -C44 -C44  C44  C44 -C44 -C44  C44 
	    */
	    t = coeff[0];	/* C44 */
	    a0 += t;
	    a1 -= t;
	    a2 -= t;
	    a3 += t;    
	    break;

	  case 15:
	    /*
	      Coefficient[0][5]

	       C45 -C14  C47  C34 -C34 -C47  C14 -C45 
	       C45 -C14  C47  C34 -C34 -C47  C14 -C45 
	       C45 -C14  C47  C34 -C34 -C47  C14 -C45 
	       C45 -C14  C47  C34 -C34 -C47  C14 -C45 
	       C45 -C14  C47  C34 -C34 -C47  C14 -C45 
	       C45 -C14  C47  C34 -C34 -C47  C14 -C45 
	       C45 -C14  C47  C34 -C34 -C47  C14 -C45 
	       C45 -C14  C47  C34 -C34 -C47  C14 -C45 
	    */
	    t = coeff[4];	/* C14 */
	    b1 -= t;    
	    t = coeff[5];	/* C34 */
	    b3 += t;    
	    t = coeff[6];	/* C45 */
	    b0 += t;    
	    t = coeff[7];	/* C47 */
	    b2 += t;    
	    break;

	  case 16:
	    /*
	      Coefficient[1][4]

	       C14 -C14 -C14  C14  C14 -C14 -C14  C14 
	       C34 -C34 -C34  C34  C34 -C34 -C34  C34 
	       C45 -C45 -C45  C45  C45 -C45 -C45  C45 
	       C47 -C47 -C47  C47  C47 -C47 -C47  C47 
	      -C47  C47  C47 -C47 -C47  C47  C47 -C47 
	      -C45  C45  C45 -C45 -C45  C45  C45 -C45 
	      -C34  C34  C34 -C34 -C34  C34  C34 -C34 
	      -C14  C14  C14 -C14 -C14  C14  C14 -C14 
	    */
	    t = coeff[4];	/* C14 */
	    c0 += t;
	    c1 -= t;
	    c2 -= t;
	    c3 += t;
	    break;

	  case 17:
	    /*
	      Coefficient[2][3]

	       C23 -C27 -C12 -C25  C25  C12  C27 -C23 
	       C36 -C67 -C16 -C56  C56  C16  C67 -C36 
	      -C36  C67  C16  C56 -C56 -C16 -C67  C36 
	      -C23  C27  C12  C25 -C25 -C12 -C27  C23 
	      -C23  C27  C12  C25 -C25 -C12 -C27  C23 
	      -C36  C67  C16  C56 -C56 -C16 -C67  C36 
	       C36 -C67 -C16 -C56  C56  C16  C67 -C36 
	       C23 -C27 -C12 -C25  C25  C12  C27 -C23 
	    */
	    t = coeff[8];	/* C12 */
	    b2 -= t;    
	    t = coeff[9];	/* C23 */
	    b0 += t;    
	    t = coeff[10];	/* C25 */
	    b3 -= t;    
	    t = coeff[11];	/* C27 */
	    b1 -= t;
	    break;

	  case 18:
	    /*
	      Coefficient[3][2]

 	       C23  C36 -C36 -C23 -C23 -C36  C36  C23 
	      -C27 -C67  C67  C27  C27  C67 -C67 -C27 
	      -C12 -C16  C16  C12  C12  C16 -C16 -C12 
	      -C25 -C56  C56  C25  C25  C56 -C56 -C25 
	       C25  C56 -C56 -C25 -C25 -C56  C56  C25 
	       C12  C16 -C16 -C12 -C12 -C16  C16  C12 
	       C27  C67 -C67 -C27 -C27 -C67  C67  C27 
	      -C23 -C36  C36  C23  C23  C36 -C36 -C23 
	    */
	    t = coeff[9];	/* C23 */
	    c0 += t;
	    c3 -= t;
	    t = coeff[13];	/* C36 */
	    c1 += t;
	    c2 -= t;
	    break;

	  case 19:
	    /*
	      Coefficient[4][1]

	       C14  C34  C45  C47 -C47 -C45 -C34 -C14 
	      -C14 -C34 -C45 -C47  C47  C45  C34  C14 
	      -C14 -C34 -C45 -C47  C47  C45  C34  C14 
	       C14  C34  C45  C47 -C47 -C45 -C34 -C14 
	       C14  C34  C45  C47 -C47 -C45 -C34 -C14 
	      -C14 -C34 -C45 -C47  C47  C45  C34  C14 
	      -C14 -C34 -C45 -C47  C47  C45  C34  C14 
	       C14  C34  C45  C47 -C47 -C45 -C34 -C14 
	    */
	    t = coeff[4];	/* C14 */
	    b0 += t;    
	    t = coeff[5];	/* C34 */
	    b1 += t;    
	    t = coeff[6];	/* C45 */
	    b2 += t;    
	    t = coeff[7];	/* C47 */
	    b3 += t;    
	    break;

	  case 20:
	    /*
	      Coefficient[5][0]

	       C45  C45  C45  C45  C45  C45  C45  C45 
	      -C14 -C14 -C14 -C14 -C14 -C14 -C14 -C14 
	       C47  C47  C47  C47  C47  C47  C47  C47 
	       C34  C34  C34  C34  C34  C34  C34  C34 
	      -C34 -C34 -C34 -C34 -C34 -C34 -C34 -C34 
	      -C47 -C47 -C47 -C47 -C47 -C47 -C47 -C47 
	       C14  C14  C14  C14  C14  C14  C14  C14 
	      -C45 -C45 -C45 -C45 -C45 -C45 -C45 -C45 
	    */
	    t = coeff[6];	/* C45 */
	    c0 += t;
	    c1 += t;
	    c2 += t;
	    c3 += t;
	    break;
	    
	  case 21:
	    /*
	      Coefficient[6][0]

	       C46  C46  C46  C46  C46  C46  C46  C46 
	      -C24 -C24 -C24 -C24 -C24 -C24 -C24 -C24 
	       C24  C24  C24  C24  C24  C24  C24  C24 
	      -C46 -C46 -C46 -C46 -C46 -C46 -C46 -C46 
	      -C46 -C46 -C46 -C46 -C46 -C46 -C46 -C46 
	       C24  C24  C24  C24  C24  C24  C24  C24 
	      -C24 -C24 -C24 -C24 -C24 -C24 -C24 -C24 
	       C46  C46  C46  C46  C46  C46  C46  C46 
	    */
	    t = coeff[3];	/* C46 */
	    a0 += t;
	    a1 += t;
	    a2 += t;
	    a3 += t;    
	    break;

	  case 22:
	    /*
	      Coefficient[5][1]

	       C15  C35  C55  C57 -C57 -C55 -C35 -C15 
	      -C11 -C13 -C15 -C17  C17  C15  C13  C11 
	       C17  C37  C57  C77 -C77 -C57 -C37 -C17 
	       C13  C33  C35  C37 -C37 -C35 -C33 -C13 
	      -C13 -C33 -C35 -C37  C37  C35  C33  C13 
	      -C17 -C37 -C57 -C77  C77  C57  C37  C17 
	       C11  C13  C15  C17 -C17 -C15 -C13 -C11 
	      -C15 -C35 -C55 -C57  C57  C55  C35  C15 
	    */
	    t = coeff[22];	/* C15 */
	    d0 += t;    
	    t = coeff[25];	/* C35 */
	    d1 += t;    
	    t = coeff[27];	/* C55 */
	    d2 += t;    
	    t = coeff[28];	/* C57 */
	    d3 += t;    
	    break;

	  case 23:
	    /*
	      Coefficient[4][2]

	       C24  C46 -C46 -C24 -C24 -C46  C46  C24 
	      -C24 -C46  C46  C24  C24  C46 -C46 -C24 
	      -C24 -C46  C46  C24  C24  C46 -C46 -C24 
	       C24  C46 -C46 -C24 -C24 -C46  C46  C24 
	       C24  C46 -C46 -C24 -C24 -C46  C46  C24 
	      -C24 -C46  C46  C24  C24  C46 -C46 -C24 
	      -C24 -C46  C46  C24  C24  C46 -C46 -C24 
	       C24  C46 -C46 -C24 -C24 -C46  C46  C24 
	    */
	    t = coeff[2];	/* C24 */
	    a0 += t;
	    a3 -= t;    
	    t = coeff[3];	/* C46 */
	    a1 += t;
	    a2 -= t;    
	    break;

	  case 24:
	    /*
	      Coefficient[3][3]

	       C33 -C37 -C13 -C35  C35  C13  C37 -C33 
	      -C37  C77  C17  C57 -C57 -C17 -C77  C37 
	      -C13  C17  C11  C15 -C15 -C11 -C17  C13 
	      -C35  C57  C15  C55 -C55 -C15 -C57  C35 
	       C35 -C57 -C15 -C55  C55  C15  C57 -C35 
	       C13 -C17 -C11 -C15  C15  C11  C17 -C13 
	       C37 -C77 -C17 -C57  C57  C17  C77 -C37 
	      -C33  C37  C13  C35 -C35 -C13 -C37  C33 
	    */
	    t = coeff[21];	/* C13 */
	    d2 -= t;    
	    t = coeff[24];	/* C33 */
	    d0 += t;    
	    t = coeff[25];	/* C35 */
	    d3 -= t;    
	    t = coeff[26];	/* C37 */
	    d1 -= t;    
	    break;

	  case 25:
	    /*
	      Coefficient[2][4]

	       C24 -C24 -C24  C24  C24 -C24 -C24  C24 
	       C46 -C46 -C46  C46  C46 -C46 -C46  C46 
	      -C46  C46  C46 -C46 -C46  C46  C46 -C46 
	      -C24  C24  C24 -C24 -C24  C24  C24 -C24 
	      -C24  C24  C24 -C24 -C24  C24  C24 -C24 
	      -C46  C46  C46 -C46 -C46  C46  C46 -C46 
	       C46 -C46 -C46  C46  C46 -C46 -C46  C46 
	       C24 -C24 -C24  C24  C24 -C24 -C24  C24 
	    */
	    t = coeff[2];	/* C24 */
	    a0 += t;
	    a1 -= t;
	    a2 -= t;
	    a3 += t;
	    break;

	  case 26:
	    /*
	      Coefficient[1][5]

	       C15 -C11  C17  C13 -C13 -C17  C11 -C15 
	       C35 -C13  C37  C33 -C33 -C37  C13 -C35 
	       C55 -C15  C57  C35 -C35 -C57  C15 -C55 
	       C57 -C17  C77  C37 -C37 -C77  C17 -C57 
	      -C57  C17 -C77 -C37  C37  C77 -C17  C57 
	      -C55  C15 -C57 -C35  C35  C57 -C15  C55 
	      -C35  C13 -C37 -C33  C33  C37 -C13  C35 
	      -C15  C11 -C17 -C13  C13  C17 -C11  C15 
	    */
	    t = coeff[20];	/* C11 */
	    d1 -= t;    
	    t = coeff[21];	/* C13 */
	    d3 += t;    
	    t = coeff[22];	/* C15 */
	    d0 += t;    
	    t = coeff[23];	/* C17 */
	    d2 += t;    
	    break;

	  case 27:
	    /*
	      Coefficient[0][6]

	       C46 -C24  C24 -C46 -C46  C24 -C24  C46 
	       C46 -C24  C24 -C46 -C46  C24 -C24  C46 
	       C46 -C24  C24 -C46 -C46  C24 -C24  C46 
	       C46 -C24  C24 -C46 -C46  C24 -C24  C46 
	       C46 -C24  C24 -C46 -C46  C24 -C24  C46 
	       C46 -C24  C24 -C46 -C46  C24 -C24  C46 
	       C46 -C24  C24 -C46 -C46  C24 -C24  C46 
	       C46 -C24  C24 -C46 -C46  C24 -C24  C46 
	    */
	    t = coeff[2];	/* C24 */
	    a1 -= t;
	    a2 += t;
	    t = coeff[3];	/* C46 */
	    a0 += t;
	    a3 -= t;
	    break;

	  case 28:
	    /*
	      Coefficient[0][7]

	       C47 -C45  C34 -C14  C14 -C34  C45 -C47 
	       C47 -C45  C34 -C14  C14 -C34  C45 -C47 
	       C47 -C45  C34 -C14  C14 -C34  C45 -C47 
	       C47 -C45  C34 -C14  C14 -C34  C45 -C47 
	       C47 -C45  C34 -C14  C14 -C34  C45 -C47 
	       C47 -C45  C34 -C14  C14 -C34  C45 -C47 
	       C47 -C45  C34 -C14  C14 -C34  C45 -C47 
	       C47 -C45  C34 -C14  C14 -C34  C45 -C47 
	    */
	    t = coeff[4];	/* C14 */
	    b3 -= t;    
	    t = coeff[5];	/* C34 */
	    b2 += t;    
	    t = coeff[6];	/* C45 */
	    b1 -= t;    
	    t = coeff[7];	/* C47 */
	    b0 += t;    
	    break;

	  case 29:
	    /*
	      Coefficient[1][6]

	       C16 -C12  C12 -C16 -C16  C12 -C12  C16 
	       C36 -C23  C23 -C36 -C36  C23 -C23  C36 
	       C56 -C25  C25 -C56 -C56  C25 -C25  C56 
	       C67 -C27  C27 -C67 -C67  C27 -C27  C67 
	      -C67  C27 -C27  C67  C67 -C27  C27 -C67 
	      -C56  C25 -C25  C56  C56 -C25  C25 -C56 
	      -C36  C23 -C23  C36  C36 -C23  C23 -C36 
	      -C16  C12 -C12  C16  C16 -C12  C12 -C16 
	    */
	    t = coeff[8];	/* C12 */
	    c1 -= t;
	    c2 += t;
	    t = coeff[12];	/* C16 */
	    c0 += t;
	    c3 -= t;
	    break;

	  case 30:
	    /*
	      Coefficient[2][5]

	       C25 -C12  C27  C23 -C23 -C27  C12 -C25 
	       C56 -C16  C67  C36 -C36 -C67  C16 -C56 
	      -C56  C16 -C67 -C36  C36  C67 -C16  C56 
	      -C25  C12 -C27 -C23  C23  C27 -C12  C25 
	      -C25  C12 -C27 -C23  C23  C27 -C12  C25 
	      -C56  C16 -C67 -C36  C36  C67 -C16  C56 
	       C56 -C16  C67  C36 -C36 -C67  C16 -C56 
	       C25 -C12  C27  C23 -C23 -C27  C12 -C25 
	    */
	    t = coeff[8];	/* C12 */
	    b1 -= t;    
	    t = coeff[9];	/* C23 */
	    b3 += t;    
	    t = coeff[10];	/* C25 */
	    b0 += t;    
	    t = coeff[11];	/* C27 */
	    b2 += t;    
	    break;

	  case 31:
	    /*
	      Coefficient[3][4]

	       C34 -C34 -C34  C34  C34 -C34 -C34  C34 
	      -C47  C47  C47 -C47 -C47  C47  C47 -C47 
	      -C14  C14  C14 -C14 -C14  C14  C14 -C14 
	      -C45  C45  C45 -C45 -C45  C45  C45 -C45 
	       C45 -C45 -C45  C45  C45 -C45 -C45  C45 
	       C14 -C14 -C14  C14  C14 -C14 -C14  C14 
	       C47 -C47 -C47  C47  C47 -C47 -C47  C47 
	      -C34  C34  C34 -C34 -C34  C34  C34 -C34 
	    */
	    t = coeff[5];	/* C34 */
	    c0 += t;
	    c1 -= t;
	    c2 -= t;
	    c3 += t;
	    break;

	  case 32:
	    /*
	      Coefficient[4][3]

	       C34 -C47 -C14 -C45  C45  C14  C47 -C34 
	      -C34  C47  C14  C45 -C45 -C14 -C47  C34 
	      -C34  C47  C14  C45 -C45 -C14 -C47  C34 
	       C34 -C47 -C14 -C45  C45  C14  C47 -C34 
	       C34 -C47 -C14 -C45  C45  C14  C47 -C34 
	      -C34  C47  C14  C45 -C45 -C14 -C47  C34 
	      -C34  C47  C14  C45 -C45 -C14 -C47  C34 
	       C34 -C47 -C14 -C45  C45  C14  C47 -C34 
	    */
	    t = coeff[4];	/* C14 */
	    b2 -= t;    
	    t = coeff[5];	/* C34 */
	    b0 += t;    
	    t = coeff[6];	/* C45 */
	    b3 -= t;    
	    t = coeff[7];	/* C47 */
	    b1 -= t;    
	    break;

	  case 33:
	    /*
	      Coefficient[5][2]

	       C25  C56 -C56 -C25 -C25 -C56  C56  C25 
	      -C12 -C16  C16  C12  C12  C16 -C16 -C12 
	       C27  C67 -C67 -C27 -C27 -C67  C67  C27 
	       C23  C36 -C36 -C23 -C23 -C36  C36  C23 
	      -C23 -C36  C36  C23  C23  C36 -C36 -C23 
	      -C27 -C67  C67  C27  C27  C67 -C67 -C27 
	       C12  C16 -C16 -C12 -C12 -C16  C16  C12 
	      -C25 -C56  C56  C25  C25  C56 -C56 -C25 
	    */
	    t = coeff[10];	/* C25 */
	    c0 += t;
	    c3 -= t;
	    t = coeff[14];	/* C56 */
	    c1 += t;
	    c2 -= t;
	    break;

	  case 34:
	    /*
	      Coefficient[6][1]

	       C16  C36  C56  C67 -C67 -C56 -C36 -C16 
	      -C12 -C23 -C25 -C27  C27  C25  C23  C12 
	       C12  C23  C25  C27 -C27 -C25 -C23 -C12 
	      -C16 -C36 -C56 -C67  C67  C56  C36  C16 
	      -C16 -C36 -C56 -C67  C67  C56  C36  C16 
	       C12  C23  C25  C27 -C27 -C25 -C23 -C12 
	      -C12 -C23 -C25 -C27  C27  C25  C23  C12 
	       C16  C36  C56  C67 -C67 -C56 -C36 -C16 
	    */
	    t = coeff[12];	/* C16 */
	    b0 += t;    
	    t = coeff[13];	/* C36 */
	    b1 += t;    
	    t = coeff[14];	/* C56 */
	    b2 += t;    
	    t = coeff[15];	/* C67 */
	    b3 += t;    
	    break;

	  case 35:
	    /*
	      Coefficient[7][0]

	       C47  C47  C47  C47  C47  C47  C47  C47 
	      -C45 -C45 -C45 -C45 -C45 -C45 -C45 -C45 
	       C34  C34  C34  C34  C34  C34  C34  C34 
	      -C14 -C14 -C14 -C14 -C14 -C14 -C14 -C14 
	       C14  C14  C14  C14  C14  C14  C14  C14 
	      -C34 -C34 -C34 -C34 -C34 -C34 -C34 -C34 
	       C45  C45  C45  C45  C45  C45  C45  C45 
	      -C47 -C47 -C47 -C47 -C47 -C47 -C47 -C47 
	    */
	    t = coeff[7];	/* C47 */
	    c0 += t;
	    c1 += t;
	    c2 += t;
	    c3 += t;    
	    break;

	  case 36:
	    /*
	      Coefficient[7][1]

	       C17  C37  C57  C77 -C77 -C57 -C37 -C17 
	      -C15 -C35 -C55 -C57  C57  C55  C35  C15 
	       C13  C33  C35  C37 -C37 -C35 -C33 -C13 
	      -C11 -C13 -C15 -C17  C17  C15  C13  C11 
	       C11  C13  C15  C17 -C17 -C15 -C13 -C11 
	      -C13 -C33 -C35 -C37  C37  C35  C33  C13 
	       C15  C35  C55  C57 -C57 -C55 -C35 -C15 
	      -C17 -C37 -C57 -C77  C77  C57  C37  C17 
	    */
	    t = coeff[23];	/* C17 */
	    d0 += t;    
	    t = coeff[26];	/* C37 */
	    d1 += t;    
	    t = coeff[28];	/* C57 */
	    d2 += t;    
	    t = coeff[29];	/* C77 */
	    d3 += t;    
	    break;

	  case 37:
	    /*
	      Coefficient[6][2]

	       C26  C66 -C66 -C26 -C26 -C66  C66  C26 
	      -C22 -C26  C26  C22  C22  C26 -C26 -C22 
	       C22  C26 -C26 -C22 -C22 -C26  C26  C22 
	      -C26 -C66  C66  C26  C26  C66 -C66 -C26 
	      -C26 -C66  C66  C26  C26  C66 -C66 -C26 
	       C22  C26 -C26 -C22 -C22 -C26  C26  C22 
	      -C22 -C26  C26  C22  C22  C26 -C26 -C22 
	       C26  C66 -C66 -C26 -C26 -C66  C66  C26 
	    */
	    t = coeff[18];	/* C26 */
	    a0 += t;
	    a3 -= t;
	    t = coeff[19];	/* C66 */
	    a1 += t;
	    a2 -= t;    
	    break;

	  case 38:
	    /*
	      Coefficient[5][3]

	       C35 -C57 -C15 -C55  C55  C15  C57 -C35 
	      -C13  C17  C11  C15 -C15 -C11 -C17  C13 
	       C37 -C77 -C17 -C57  C57  C17  C77 -C37 
	       C33 -C37 -C13 -C35  C35  C13  C37 -C33 
	      -C33  C37  C13  C35 -C35 -C13 -C37  C33 
	      -C37  C77  C17  C57 -C57 -C17 -C77  C37 
	       C13 -C17 -C11 -C15  C15  C11  C17 -C13 
	      -C35  C57  C15  C55 -C55 -C15 -C57  C35 
	    */
	    t = coeff[22];	/* C15 */
	    d2 -= t;    
	    t = coeff[25];	/* C35 */
	    d0 += t;    
	    t = coeff[27];	/* C55 */
	    d3 -= t;    
	    t = coeff[28];	/* C57 */
	    d1 -= t;    
	    break;

	  case 39:
	    /*
	      Coefficient[4][4]

	       C44 -C44 -C44  C44  C44 -C44 -C44  C44 
	      -C44  C44  C44 -C44 -C44  C44  C44 -C44 
	      -C44  C44  C44 -C44 -C44  C44  C44 -C44 
	       C44 -C44 -C44  C44  C44 -C44 -C44  C44 
	       C44 -C44 -C44  C44  C44 -C44 -C44  C44 
	      -C44  C44  C44 -C44 -C44  C44  C44 -C44 
	      -C44  C44  C44 -C44 -C44  C44  C44 -C44 
	       C44 -C44 -C44  C44  C44 -C44 -C44  C44 
	    */
	    t = coeff[0];	/* C44 */
	    a0 += t;
	    a1 -= t;
	    a2 -= t;
	    a3 += t;
	    break;

	  case 40:
	    /*
	      Coefficient[3][5]

	       C35 -C13  C37  C33 -C33 -C37  C13 -C35 
	      -C57  C17 -C77 -C37  C37  C77 -C17  C57 
	      -C15  C11 -C17 -C13  C13  C17 -C11  C15 
	      -C55  C15 -C57 -C35  C35  C57 -C15  C55 
	       C55 -C15  C57  C35 -C35 -C57  C15 -C55 
	       C15 -C11  C17  C13 -C13 -C17  C11 -C15 
	       C57 -C17  C77  C37 -C37 -C77  C17 -C57 
	      -C35  C13 -C37 -C33  C33  C37 -C13  C35 
	    */
	    t = coeff[21];	/* C13 */
	    d1 -= t;    
	    t = coeff[24];	/* C33 */
	    d3 += t;    
	    t = coeff[25];	/* C35 */
	    d0 += t;    
	    t = coeff[26];	/* C37 */
	    d2 += t;    
	    break;

	  case 41:
	    /*
	      Coefficient[2][6]

	       C26 -C22  C22 -C26 -C26  C22 -C22  C26 
	       C66 -C26  C26 -C66 -C66  C26 -C26  C66 
	      -C66  C26 -C26  C66  C66 -C26  C26 -C66 
	      -C26  C22 -C22  C26  C26 -C22  C22 -C26 
	      -C26  C22 -C22  C26  C26 -C22  C22 -C26 
	      -C66  C26 -C26  C66  C66 -C26  C26 -C66 
	       C66 -C26  C26 -C66 -C66  C26 -C26  C66 
	       C26 -C22  C22 -C26 -C26  C22 -C22  C26 
	    */
	    t = coeff[16];	/* C22 */
	    a1 -= t;
	    a2 += t;
	    t = coeff[17];	/* C26 */
	    a0 += t;
	    a3 -= t;
	    break;

	  case 42:
	    /*
	       Coefficient[1][7]

	        C17 -C15  C13 -C11  C11 -C13  C15 -C17 
	        C37 -C35  C33 -C13  C13 -C33  C35 -C37 
	        C57 -C55  C35 -C15  C15 -C35  C55 -C57 
	        C77 -C57  C37 -C17  C17 -C37  C57 -C77 
	       -C77  C57 -C37  C17 -C17  C37 -C57  C77 
	       -C57  C55 -C35  C15 -C15  C35 -C55  C57 
	       -C37  C35 -C33  C13 -C13  C33 -C35  C37 
	       -C17  C15 -C13  C11 -C11  C13 -C15  C17 
	    */
	    t = coeff[20];	/* C11 */
	    d3 -= t;    
	    t = coeff[21];	/* C13 */
	    d2 += t;    
	    t = coeff[22];	/* C15 */
	    d1 -= t;    
	    t = coeff[23];	/* C17 */
	    d0 += t;    
	    break;

	  case 43:
	    /*
	      Coefficient[2][7]

	       C27 -C25  C23 -C12  C12 -C23  C25 -C27 
	       C67 -C56  C36 -C16  C16 -C36  C56 -C67 
	      -C67  C56 -C36  C16 -C16  C36 -C56  C67 
	      -C27  C25 -C23  C12 -C12  C23 -C25  C27 
	      -C27  C25 -C23  C12 -C12  C23 -C25  C27 
	      -C67  C56 -C36  C16 -C16  C36 -C56  C67 
	       C67 -C56  C36 -C16  C16 -C36  C56 -C67 
	       C27 -C25  C23 -C12  C12 -C23  C25 -C27 
	    */
	    t = coeff[8];	/* C12 */
	    b3 -= t;    
	    t = coeff[9];	/* C23 */
	    b2 += t;    
	    t = coeff[10];	/* C25 */
	    b1 -= t;    
	    t = coeff[11];	/* C27 */
	    b0 += t;    
	    break;

	  case 44:
	    /*
	      Coefficient[3][6]

	       C36 -C23  C23 -C36 -C36  C23 -C23  C36 
	      -C67  C27 -C27  C67  C67 -C27  C27 -C67 
	      -C16  C12 -C12  C16  C16 -C12  C12 -C16 
	      -C56  C25 -C25  C56  C56 -C25  C25 -C56 
	       C56 -C25  C25 -C56 -C56  C25 -C25  C56 
	       C16 -C12  C12 -C16 -C16  C12 -C12  C16 
	       C67 -C27  C27 -C67 -C67  C27 -C27  C67 
	      -C36  C23 -C23  C36  C36 -C23  C23 -C36 
	    */
	    t = coeff[9];	/* C23 */
	    c1 -= t;
	    c2 += t;
	    t = coeff[13];	/* C36 */
	    c0 += t;
	    c3 -= t;
	    break;

	  case 45:
	    /*
	      Coefficient[4][5]

	       C45 -C14  C47  C34 -C34 -C47  C14 -C45 
	      -C45  C14 -C47 -C34  C34  C47 -C14  C45 
	      -C45  C14 -C47 -C34  C34  C47 -C14  C45 
	       C45 -C14  C47  C34 -C34 -C47  C14 -C45 
	       C45 -C14  C47  C34 -C34 -C47  C14 -C45 
	      -C45  C14 -C47 -C34  C34  C47 -C14  C45 
	      -C45  C14 -C47 -C34  C34  C47 -C14  C45 
	       C45 -C14  C47  C34 -C34 -C47  C14 -C45 
	    */
	    t = coeff[4];	/* C14 */
	    b1 -= t;    
	    t = coeff[5];	/* C34 */
	    b3 += t;    
	    t = coeff[6];	/* C45 */
	    b0 += t;    
	    t = coeff[7];	/* C47 */
	    b2 += t;    
	    break;

	  case 46:
	    /*
	      Coefficient[5][4]

	       C45 -C45 -C45  C45  C45 -C45 -C45  C45 
	      -C14  C14  C14 -C14 -C14  C14  C14 -C14 
	       C47 -C47 -C47  C47  C47 -C47 -C47  C47 
	       C34 -C34 -C34  C34  C34 -C34 -C34  C34 
	      -C34  C34  C34 -C34 -C34  C34  C34 -C34 
	      -C47  C47  C47 -C47 -C47  C47  C47 -C47 
	       C14 -C14 -C14  C14  C14 -C14 -C14  C14 
	      -C45  C45  C45 -C45 -C45  C45  C45 -C45 
	    */
	    t = coeff[6];	/* C45 */
	    c0 += t;
	    c1 -= t;
	    c2 -= t;
	    c3 += t;
	    break;

	  case 47:
	    /*
	      Coefficient[6][3]

	       C36 -C67 -C16 -C56  C56  C16  C67 -C36 
	      -C23  C27  C12  C25 -C25 -C12 -C27  C23 
	       C23 -C27 -C12 -C25  C25  C12  C27 -C23 
	      -C36  C67  C16  C56 -C56 -C16 -C67  C36 
	      -C36  C67  C16  C56 -C56 -C16 -C67  C36 
	       C23 -C27 -C12 -C25  C25  C12  C27 -C23 
	      -C23  C27  C12  C25 -C25 -C12 -C27  C23 
	       C36 -C67 -C16 -C56  C56  C16  C67 -C36 
	    */
	    t = coeff[12];	/* C16 */
	    b2 -= t;    
	    t = coeff[13];	/* C36 */
	    b0 += t;    
	    t = coeff[14];	/* C56 */
	    b3 -= t;    
	    t = coeff[15];	/* C67 */
	    b1 -= t;    
	    break;

	  case 48:
	    /*
	      Coefficient[7][2]

	       C27  C67 -C67 -C27 -C27 -C67  C67  C27 
	      -C25 -C56  C56  C25  C25  C56 -C56 -C25 
	       C23  C36 -C36 -C23 -C23 -C36  C36  C23 
	      -C12 -C16  C16  C12  C12  C16 -C16 -C12 
	       C12  C16 -C16 -C12 -C12 -C16  C16  C12 
	      -C23 -C36  C36  C23  C23  C36 -C36 -C23 
	       C25  C56 -C56 -C25 -C25 -C56  C56  C25 
	      -C27 -C67  C67  C27  C27  C67 -C67 -C27 
	    */
	    t = coeff[11];	/* C27 */
	    c0 += t;
	    c3 -= t;
	    t = coeff[15];	/* C67 */
	    c1 += t;
	    c2 -= t;
	    break;

	  case 49:
	    /*
	      Coefficient[7][3]
	      
	       C37 -C77 -C17 -C57  C57  C17  C77 -C37 
	      -C35  C57  C15  C55 -C55 -C15 -C57  C35 
	       C33 -C37 -C13 -C35  C35  C13  C37 -C33 
	      -C13  C17  C11  C15 -C15 -C11 -C17  C13 
	       C13 -C17 -C11 -C15  C15  C11  C17 -C13 
	      -C33  C37  C13  C35 -C35 -C13 -C37  C33 
	       C35 -C57 -C15 -C55  C55  C15  C57 -C35 
	      -C37  C77  C17  C57 -C57 -C17 -C77  C37 
	    */
	    t = coeff[23];	/* C17 */
	    d2 -= t;    
	    t = coeff[26];	/* C37 */
	    d0 += t;    
	    t = coeff[28];	/* C57 */
	    d3 -= t;    
	    t = coeff[29];	/* C77 */
	    d1 -= t;    
	    break;

	  case 50:
	    /*
	      Coefficient[6][4]

	       C46 -C46 -C46  C46  C46 -C46 -C46  C46 
	      -C24  C24  C24 -C24 -C24  C24  C24 -C24 
	       C24 -C24 -C24  C24  C24 -C24 -C24  C24 
	      -C46  C46  C46 -C46 -C46  C46  C46 -C46 
	      -C46  C46  C46 -C46 -C46  C46  C46 -C46 
	       C24 -C24 -C24  C24  C24 -C24 -C24  C24 
	      -C24  C24  C24 -C24 -C24  C24  C24 -C24 
	       C46 -C46 -C46  C46  C46 -C46 -C46  C46 
	    */
	    t = coeff[3];	/* C46 */
	    a0 += t;
	    a1 -= t;
	    a2 -= t;
	    a3 += t;
	    break;

	  case 51:
	    /*
	      Coefficient[5][5]

	       C55 -C15  C57  C35 -C35 -C57  C15 -C55 
	      -C15  C11 -C17 -C13  C13  C17 -C11  C15 
	       C57 -C17  C77  C37 -C37 -C77  C17 -C57 
	       C35 -C13  C37  C33 -C33 -C37  C13 -C35 
	      -C35  C13 -C37 -C33  C33  C37 -C13  C35 
	      -C57  C17 -C77 -C37  C37  C77 -C17  C57 
	       C15 -C11  C17  C13 -C13 -C17  C11 -C15 
	      -C55  C15 -C57 -C35  C35  C57 -C15  C55 
	    */
	    t = coeff[22];	/* C15 */
	    d1 -= t;    
	    t = coeff[25];	/* C35 */
	    d3 += t;    
	    t = coeff[27];	/* C55 */
	    d0 += t;    
	    t = coeff[28];	/* C57 */
	    d2 += t;    
	    break;

	  case 52:
	    /*
	      Coefficient[4][6]

	       C46 -C24  C24 -C46 -C46  C24 -C24  C46 
	      -C46  C24 -C24  C46  C46 -C24  C24 -C46 
	      -C46  C24 -C24  C46  C46 -C24  C24 -C46 
	       C46 -C24  C24 -C46 -C46  C24 -C24  C46 
	       C46 -C24  C24 -C46 -C46  C24 -C24  C46 
	      -C46  C24 -C24  C46  C46 -C24  C24 -C46 
	      -C46  C24 -C24  C46  C46 -C24  C24 -C46 
	      C46 -C24  C24 -C46 -C46  C24 -C24  C46 
	    */
	    t = coeff[2];	/* C24 */
	    a1 -= t;
	    a2 += t;
	    t = coeff[3];	/* C46 */
	    a0 += t;
	    a3 -= t;
	    break;

	  case 53:
	    /*
	      Coefficient[3][7]

	       C37 -C35  C33 -C13  C13 -C33  C35 -C37 
	      -C77  C57 -C37  C17 -C17  C37 -C57  C77 
	      -C17  C15 -C13  C11 -C11  C13 -C15  C17 
	      -C57  C55 -C35  C15 -C15  C35 -C55  C57 
	       C57 -C55  C35 -C15  C15 -C35  C55 -C57 
	       C17 -C15  C13 -C11  C11 -C13  C15 -C17 
	       C77 -C57  C37 -C17  C17 -C37  C57 -C77 
	      -C37  C35 -C33  C13 -C13  C33 -C35  C37 
	    */
	    t = coeff[21];	/* C13 */
	    d3 -= t;    
	    t = coeff[24];	/* C33 */
	    d2 += t;    
	    t = coeff[25];	/* C35 */
	    d1 -= t;    
	    t = coeff[26];	/* C37 */
	    d0 += t;    
	    break;

	  case 54:
	    /*
	      Coefficient[4][7]

	       C47 -C45  C34 -C14  C14 -C34  C45 -C47 
	      -C47  C45 -C34  C14 -C14  C34 -C45  C47 
	      -C47  C45 -C34  C14 -C14  C34 -C45  C47 
	       C47 -C45  C34 -C14  C14 -C34  C45 -C47 
	       C47 -C45  C34 -C14  C14 -C34  C45 -C47 
	      -C47  C45 -C34  C14 -C14  C34 -C45  C47 
	      -C47  C45 -C34  C14 -C14  C34 -C45  C47 
	       C47 -C45  C34 -C14  C14 -C34  C45 -C47 
	    */
	    t = coeff[4];	/* C14 */
	    b3 -= t;    
	    t = coeff[5];	/* C34 */
	    b2 += t;    
	    t = coeff[6];	/* C45 */
	    b1 -= t;    
	    t = coeff[7];	/* C47 */
	    b0 += t;    
	    break;

	  case 55:
	    /*
	      Coefficient[5][6]

	       C56 -C25  C25 -C56 -C56  C25 -C25  C56 
	      -C16  C12 -C12  C16  C16 -C12  C12 -C16 
	       C67 -C27  C27 -C67 -C67  C27 -C27  C67 
	       C36 -C23  C23 -C36 -C36  C23 -C23  C36 
	      -C36  C23 -C23  C36  C36 -C23  C23 -C36 
	      -C67  C27 -C27  C67  C67 -C27  C27 -C67 
	       C16 -C12  C12 -C16 -C16  C12 -C12  C16 
	      -C56  C25 -C25  C56  C56 -C25  C25 -C56 
	    */
	    t = coeff[10];	/* C25 */
	    c1 -= t;
	    c2 += t;
	    t = coeff[14];	/* C56 */
	    c0 += t;
	    c3 -= t;
	    break;

	  case 56:
	    /*
	      Coefficient[6][5]

	       C56 -C16  C67  C36 -C36 -C67  C16 -C56 
	      -C25  C12 -C27 -C23  C23  C27 -C12  C25 
	       C25 -C12  C27  C23 -C23 -C27  C12 -C25 
	      -C56  C16 -C67 -C36  C36  C67 -C16  C56 
	      -C56  C16 -C67 -C36  C36  C67 -C16  C56 
	       C25 -C12  C27  C23 -C23 -C27  C12 -C25 
	      -C25  C12 -C27 -C23  C23  C27 -C12  C25 
	       C56 -C16  C67  C36 -C36 -C67  C16 -C56 
	    */
	    t = coeff[12];	/* C16 */
	    b1 -= t;    
	    t = coeff[13];	/* C36 */
	    b3 += t;    
	    t = coeff[14];	/* C56 */
	    b0 += t;    
	    t = coeff[15];	/* C67 */
	    b2 += t;    
	    break;

	  case 57:
	    /*
	      Coefficient[7][4]

	       C47 -C47 -C47  C47  C47 -C47 -C47  C47 
	      -C45  C45  C45 -C45 -C45  C45  C45 -C45 
	       C34 -C34 -C34  C34  C34 -C34 -C34  C34 
	      -C14  C14  C14 -C14 -C14  C14  C14 -C14 
	       C14 -C14 -C14  C14  C14 -C14 -C14  C14 
	      -C34  C34  C34 -C34 -C34  C34  C34 -C34 
	       C45 -C45 -C45  C45  C45 -C45 -C45  C45 
	      -C47  C47  C47 -C47 -C47  C47  C47 -C47 
	    */
	    t = coeff[7];	/* C47 */
	    c0 += t;
	    c1 -= t;
	    c2 -= t;
	    c3 += t;
	    break;

	  case 58:
	    /*
	      Coefficient[7][5]

	       C57 -C17  C77  C37 -C37 -C77  C17 -C57 
	      -C55  C15 -C57 -C35  C35  C57 -C15  C55 
	       C35 -C13  C37  C33 -C33 -C37  C13 -C35 
	      -C15  C11 -C17 -C13  C13  C17 -C11  C15 
	       C15 -C11  C17  C13 -C13 -C17  C11 -C15 
	      -C35  C13 -C37 -C33  C33  C37 -C13  C35 
	       C55 -C15  C57  C35 -C35 -C57  C15 -C55 
	      -C57  C17 -C77 -C37  C37  C77 -C17  C57 
	    */
	    t = coeff[23];	/* C17 */
	    d1 -= t;    
	    t = coeff[26];	/* C37 */
	    d3 += t;    
	    t = coeff[28];	/* C57 */
	    d0 += t;    
	    t = coeff[29];	/* C77 */
	    d2 += t;    
	    break;

	  case 59:
	    /*
	      Coefficient[6][6]

	       C66 -C26  C26 -C66 -C66  C26 -C26  C66 
	      -C26  C22 -C22  C26  C26 -C22  C22 -C26 
	       C26 -C22  C22 -C26 -C26  C22 -C22  C26 
	      -C66  C26 -C26  C66  C66 -C26  C26 -C66 
	      -C66  C26 -C26  C66  C66 -C26  C26 -C66 
	       C26 -C22  C22 -C26 -C26  C22 -C22  C26 
	      -C26  C22 -C22  C26  C26 -C22  C22 -C26 
	       C66 -C26  C26 -C66 -C66  C26 -C26  C66 
	    */
	    t = coeff[18];	/* C26 */
	    a1 -= t;
	    a2 += t;
	    t = coeff[19];	/* C66 */
	    a0 += t;
	    a3 -= t;
	    break;

	  case 60:
	    /*
	      Coefficient[5][7]

	       C57 -C55  C35 -C15  C15 -C35  C55 -C57 
	      -C17  C15 -C13  C11 -C11  C13 -C15  C17 
	       C77 -C57  C37 -C17  C17 -C37  C57 -C77 
	       C37 -C35  C33 -C13  C13 -C33  C35 -C37 
	      -C37  C35 -C33  C13 -C13  C33 -C35  C37 
	      -C77  C57 -C37  C17 -C17  C37 -C57  C77 
	       C17 -C15  C13 -C11  C11 -C13  C15 -C17 
	      -C57  C55 -C35  C15 -C15  C35 -C55  C57 
	    */
	    t = coeff[22];	/* C15 */
	    d3 -= t;    
	    t = coeff[25];	/* C35 */
	    d2 += t;    
	    t = coeff[27];	/* C55 */
	    d1 -= t;    
	    t = coeff[28];	/* C57 */
	    d0 += t;    
	    break;

	  case 61:
	    /*
	      Coefficient[6][7]

	       C67 -C56  C36 -C16  C16 -C36  C56 -C67 
	      -C27  C25 -C23  C12 -C12  C23 -C25  C27 
	       C27 -C25  C23 -C12  C12 -C23  C25 -C27 
	      -C67  C56 -C36  C16 -C16  C36 -C56  C67 
	      -C67  C56 -C36  C16 -C16  C36 -C56  C67 
	       C27 -C25  C23 -C12  C12 -C23  C25 -C27 
	      -C27  C25 -C23  C12 -C12  C23 -C25  C27 
	       C67 -C56  C36 -C16  C16 -C36  C56 -C67 
	    */
	    t = coeff[12];	/* C16 */
	    b3 -= t;    
	    t = coeff[13];	/* C36 */
	    b2 += t;    
	    t = coeff[14];	/* C56 */
	    b1 -= t;    
	    t = coeff[15];	/* C67 */
	    b0 += t;    
	    break;

	  case 62:
	    /*
	      Coefficient[7][6]

	       C67 -C27  C27 -C67 -C67  C27 -C27  C67 
	      -C56  C25 -C25  C56  C56 -C25  C25 -C56 
	       C36 -C23  C23 -C36 -C36  C23 -C23  C36 
	      -C16  C12 -C12  C16  C16 -C12  C12 -C16 
	       C16 -C12  C12 -C16 -C16  C12 -C12  C16 
	      -C36  C23 -C23  C36  C36 -C23  C23 -C36 
	       C56 -C25  C25 -C56 -C56  C25 -C25  C56 
	      -C67  C27 -C27  C67  C67 -C27  C27 -C67 
	    */
	    t = coeff[11];	/* C27 */
	    c1 -= t;
	    c2 += t;
	    t = coeff[15];	/* C67 */
	    c0 += t;
	    c3 -= t;
	    break;

	  case 63:
	    /*
	      Coefficient[7][7]

	       C77 -C57  C37 -C17  C17 -C37  C57 -C77 
	      -C57  C55 -C35  C15 -C15  C35 -C55  C57 
	       C37 -C35  C33 -C13  C13 -C33  C35 -C37 
	      -C17  C15 -C13  C11 -C11  C13 -C15  C17 
	       C17 -C15  C13 -C11  C11 -C13  C15 -C17 
	      -C37  C35 -C33  C13 -C13  C33 -C35  C37 
	       C57 -C55  C35 -C15  C15 -C35  C55 -C57 
	      -C77  C57 -C37  C17 -C17  C37 -C57  C77 
	    */
	    t = coeff[23];	/* C17 */
	    d3 -= t;    
	    t = coeff[26];	/* C37 */
	    d2 += t;    
	    t = coeff[28];	/* C57 */
	    d1 -= t;    
	    t = coeff[29];	/* C77 */
	    d0 += t;    
	    break;
	}

	i += 1;
	t = coefflist[i];

    } while (t != 0);

    t = a0 + b0;
    i = a0 - b0;
    a0 = c0 + d0;
    b0 = c0 - d0;
    c0 = t - a0;
    a0 = t + a0;
    d0 = i - b0;
    b0 = i + b0;
    a0 = ((unsigned int) a0) >> 16;
    b0 = ((unsigned int) b0) >> 16;
    c0 = ((unsigned int) c0) >> 16;
    d0 = ((unsigned int) d0) >> 16;

    t = a1 + b1;
    i = a1 - b1;
    a1 = c1 + d1;
    b1 = c1 - d1;
    c1 = t - a1;
    a1 = t + a1;
    d1 = i - b1;
    b1 = i + b1;
    a1 = ((unsigned int) a1) >> 16;
    b1 = ((unsigned int) b1) >> 16;
    c1 = ((unsigned int) c1) >> 16;
    d1 = ((unsigned int) d1) >> 16;

    t = a2 + b2;
    i = a2 - b2;
    a2 = c2 + d2;
    b2 = c2 - d2;
    c2 = t - a2;
    a2 = t + a2;
    d2 = i - b2;
    b2 = i + b2;
    a2 = ((unsigned int) a2) >> 16;
    b2 = ((unsigned int) b2) >> 16;
    c2 = ((unsigned int) c2) >> 16;
    d2 = ((unsigned int) d2) >> 16;

    t = a3 + b3;
    i = a3 - b3;
    a3 = c3 + d3;
    b3 = c3 - d3;
    c3 = t - a3;
    a3 = t + a3;
    d3 = i - b3;
    b3 = i + b3;
    a3 = ((unsigned int) a3) >> 16;
    b3 = ((unsigned int) b3) >> 16;
    c3 = ((unsigned int) c3) >> 16;
    d3 = ((unsigned int) d3) >> 16;

    a0 = PUT_SHORTS_INTO_INT(a0,a1);
    result[0] = a0;
    a2 = PUT_SHORTS_INTO_INT(a2,a3);
    result[1] = a2;
    b2 = PUT_SHORTS_INTO_INT(b3,b2);
    result[2] = b2;
    b0 = PUT_SHORTS_INTO_INT(b1,b0);
    result[3] = b0;
    c0 = PUT_SHORTS_INTO_INT(c0,c1);
    result[28] = c0;
    c2 = PUT_SHORTS_INTO_INT(c2,c3);
    result[29] = c2;
    d2 = PUT_SHORTS_INTO_INT(d3,d2);
    result[30] = d2;
    d0 = PUT_SHORTS_INTO_INT(d1,d0);
    result[31] = d0;

    i = 0;
    a0 = a1 = a2 = a3 = HALF;
    b0 = b1 = b2 = b3 = 0;
    c0 = c1 = c2 = c3 = 0;
    d0 = d1 = d2 = d3 = 0;

    t = coefflist[0];
    do {
	coeff = (int *)(t & ~63);
	t = t & 63;

	switch (t) {
	  case 0:
	    /*
	      Coefficient[0][0]

	      C44  C44  C44  C44  C44  C44  C44  C44 
	      C44  C44  C44  C44  C44  C44  C44  C44 
	      C44  C44  C44  C44  C44  C44  C44  C44 
	      C44  C44  C44  C44  C44  C44  C44  C44 
	      C44  C44  C44  C44  C44  C44  C44  C44 
	      C44  C44  C44  C44  C44  C44  C44  C44 
	      C44  C44  C44  C44  C44  C44  C44  C44 
	      C44  C44  C44  C44  C44  C44  C44  C44 
	    */
	    t = coeff[0];	/* C44 */
	    a0 += t;
	    a1 += t;
	    a2 += t;
	    a3 += t;    
	    break;


	  case 1:
	    /*
	      Coefficient[0][1]

	       C14  C34  C45  C47 -C47 -C45 -C34 -C14 
	       C14  C34  C45  C47 -C47 -C45 -C34 -C14 
	       C14  C34  C45  C47 -C47 -C45 -C34 -C14 
	       C14  C34  C45  C47 -C47 -C45 -C34 -C14 
	       C14  C34  C45  C47 -C47 -C45 -C34 -C14 
	       C14  C34  C45  C47 -C47 -C45 -C34 -C14 
	       C14  C34  C45  C47 -C47 -C45 -C34 -C14 
	       C14  C34  C45  C47 -C47 -C45 -C34 -C14 
	    */
	    t = coeff[4];	/* C14 */
	    b0 += t;    
	    t = coeff[5];	/* C34 */
	    b1 += t;    
	    t = coeff[6];	/* C45 */
	    b2 += t;    
	    t = coeff[7];	/* C47 */
	    b3 += t;
	    break;

	  case 2:
	    /*
	      Coefficient[1][0]

	       C14  C14  C14  C14  C14  C14  C14  C14 
	       C34  C34  C34  C34  C34  C34  C34  C34 
	       C45  C45  C45  C45  C45  C45  C45  C45 
	       C47  C47  C47  C47  C47  C47  C47  C47 
	      -C47 -C47 -C47 -C47 -C47 -C47 -C47 -C47 
	      -C45 -C45 -C45 -C45 -C45 -C45 -C45 -C45 
	      -C34 -C34 -C34 -C34 -C34 -C34 -C34 -C34
	      -C14 -C14 -C14 -C14 -C14 -C14 -C14 -C14 
	    */
	    t = coeff[5];	/* C34 */
	    c0 += t;  
	    c1 += t;
	    c2 += t;
	    c3 += t;    
	    break;

	  case 3:
	    /*
	      Coefficient[2][0]

	       C24  C24  C24  C24  C24  C24  C24  C24 
	       C46  C46  C46  C46  C46  C46  C46  C46 
	      -C46 -C46 -C46 -C46 -C46 -C46 -C46 -C46 
	      -C24 -C24 -C24 -C24 -C24 -C24 -C24 -C24 
	      -C24 -C24 -C24 -C24 -C24 -C24 -C24 -C24 
	      -C46 -C46 -C46 -C46 -C46 -C46 -C46 -C46 
	       C46  C46  C46  C46  C46  C46  C46  C46 
	       C24  C24  C24  C24  C24  C24  C24  C24 
	    */
	    t = coeff[3];	/* C46 */
	    a0 += t;
	    a1 += t;
	    a2 += t;
	    a3 += t;
	    break;


	  case 4:
	    /*
	      Coefficient[1][1]

	       C11  C13  C15  C17 -C17 -C15 -C13 -C11 
	       C13  C33  C35  C37 -C37 -C35 -C33 -C13 
	       C15  C35  C55  C57 -C57 -C55 -C35 -C15 
	       C17  C37  C57  C77 -C77 -C57 -C37 -C17 
	      -C17 -C37 -C57 -C77  C77  C57  C37  C17 
	      -C15 -C35 -C55 -C57  C57  C55  C35  C15 
	      -C13 -C33 -C35 -C37  C37  C35  C33  C13 
	      -C11 -C13 -C15 -C17  C17  C15  C13  C11 
	    */
	    t = coeff[21];	/* C13 */
	    d0 += t;    
	    t = coeff[24];	/* C33 */
	    d1 += t;    
	    t = coeff[25];	/* C35 */
	    d2 += t;    
	    t = coeff[26];	/* C37 */
	    d3 += t;    
	    break;

	  case 5:
	    /*
	      Coefficient[0][2]

	       C24  C46 -C46 -C24 -C24 -C46  C46  C24 
	       C24  C46 -C46 -C24 -C24 -C46  C46  C24 
	       C24  C46 -C46 -C24 -C24 -C46  C46  C24 
	       C24  C46 -C46 -C24 -C24 -C46  C46  C24 
	       C24  C46 -C46 -C24 -C24 -C46  C46  C24 
	       C24  C46 -C46 -C24 -C24 -C46  C46  C24 
	       C24  C46 -C46 -C24 -C24 -C46  C46  C24 
	       C24  C46 -C46 -C24 -C24 -C46  C46  C24 
	    */
	    t = coeff[2];	/* C24 */
	    a0 += t;
	    a3 -= t;    
	    t = coeff[3];	/* C46 */
	    a1 += t;
	    a2 -= t;    
	    break;

	  case 6:
	    /*
	      Coefficient[0][3]

	       C34 -C47 -C14 -C45  C45  C14  C47 -C34 
	       C34 -C47 -C14 -C45  C45  C14  C47 -C34 
	       C34 -C47 -C14 -C45  C45  C14  C47 -C34 
	       C34 -C47 -C14 -C45  C45  C14  C47 -C34 
	       C34 -C47 -C14 -C45  C45  C14  C47 -C34 
	       C34 -C47 -C14 -C45  C45  C14  C47 -C34 
	       C34 -C47 -C14 -C45  C45  C14  C47 -C34 
	       C34 -C47 -C14 -C45  C45  C14  C47 -C34 
	    */
	    t = coeff[4];	/* C14 */
	    b2 -= t;    
	    t = coeff[5];	/* C34 */
	    b0 += t;    
	    t = coeff[6];	/* C45 */
	    b3 -= t;    
	    t = coeff[7];	/* C47 */
	    b1 -= t;    
	    break;

	  case 7:
	    /*
	      Coefficient[1][2]

	       C12  C16 -C16 -C12 -C12 -C16  C16  C12 
	       C23  C36 -C36 -C23 -C23 -C36  C36  C23 
	       C25  C56 -C56 -C25 -C25 -C56  C56  C25 
	       C27  C67 -C67 -C27 -C27 -C67  C67  C27 
	      -C27 -C67  C67  C27  C27  C67 -C67 -C27 
	      -C25 -C56  C56  C25  C25  C56 -C56 -C25 
	      -C23 -C36  C36  C23  C23  C36 -C36 -C23 
	      -C12 -C16  C16  C12  C12  C16 -C16 -C12 
	    */
	    t = coeff[9];	/* C23 */
	    c0 += t;
	    c3 -= t; 
	    t = coeff[13];	/* C36 */
	    c1 += t;
	    c2 -= t;
	    break;

	  case 8:
	    /*
	      Coefficient[2][1]

	       C12  C23  C25  C27 -C27 -C25 -C23 -C12 
	       C16  C36  C56  C67 -C67 -C56 -C36 -C16 
	      -C16 -C36 -C56 -C67  C67  C56  C36  C16 
	      -C12 -C23 -C25 -C27  C27  C25  C23  C12 
	      -C12 -C23 -C25 -C27  C27  C25  C23  C12 
	      -C16 -C36 -C56 -C67  C67  C56  C36  C16 
	       C16  C36  C56  C67 -C67 -C56 -C36 -C16 
	       C12  C23  C25  C27 -C27 -C25 -C23 -C12 
	    */
	    t = coeff[12];	/* C16 */
	    b0 += t;    
	    t = coeff[13];	/* C36 */
	    b1 += t;    
	    t = coeff[14];	/* C56 */
	    b2 += t;    
	    t = coeff[15];	/* C67 */
	    b3 += t;    
	    break;

	  case 9:
	    /*
	      Coefficient[3][0]

	       C34  C34  C34  C34  C34  C34  C34  C34 
	      -C47 -C47 -C47 -C47 -C47 -C47 -C47 -C47 
	      -C14 -C14 -C14 -C14 -C14 -C14 -C14 -C14 
	      -C45 -C45 -C45 -C45 -C45 -C45 -C45 -C45 
	       C45  C45  C45  C45  C45  C45  C45  C45 
	       C14  C14  C14  C14  C14  C14  C14  C14 
	       C47  C47  C47  C47  C47  C47  C47  C47 
	      -C34 -C34 -C34 -C34 -C34 -C34 -C34 -C34 
	    */
	    t = coeff[7];	/* C47 */
	    c0 -= t;
	    c1 -= t;
	    c2 -= t;
	    c3 -= t;    
	    break;

	  case 10:
	    /*
	      Coefficient[4][0]
	      
	       C44  C44  C44  C44  C44  C44  C44  C44 
	      -C44 -C44 -C44 -C44 -C44 -C44 -C44 -C44 
	      -C44 -C44 -C44 -C44 -C44 -C44 -C44 -C44 
	       C44  C44  C44  C44  C44  C44  C44  C44 
	       C44  C44  C44  C44  C44  C44  C44  C44 
	      -C44 -C44 -C44 -C44 -C44 -C44 -C44 -C44 
	      -C44 -C44 -C44 -C44 -C44 -C44 -C44 -C44 
	       C44  C44  C44  C44  C44  C44  C44  C44 
	    */
	    t = coeff[0];	/* C44 */
	    a0 -= t;
	    a1 -= t;
	    a2 -= t;
	    a3 -= t;    
	    break;

	  case 11:
	    /*
	      Coefficient[3][1]

	       C13  C33  C35  C37 -C37 -C35 -C33 -C13 
	      -C17 -C37 -C57 -C77  C77  C57  C37  C17 
	      -C11 -C13 -C15 -C17  C17  C15  C13  C11 
	      -C15 -C35 -C55 -C57  C57  C55  C35  C15 
	       C15  C35  C55  C57 -C57 -C55 -C35 -C15 
	       C11  C13  C15  C17 -C17 -C15 -C13 -C11 
	       C17  C37  C57  C77 -C77 -C57 -C37 -C17 
	      -C13 -C33 -C35 -C37  C37  C35  C33  C13 
	   */
	    t = coeff[23];	/* C17 */
	    d0 -= t;    
	    t = coeff[26];	/* C37 */
	    d1 -= t;    
	    t = coeff[28];	/* C57 */
	    d2 -= t;    
	    t = coeff[29];	/* C77 */
	    d3 -= t;
	    break;

	  case 12:
	    /*
	      Coefficient[2][2]

	       C22  C26 -C26 -C22 -C22 -C26  C26  C22 
	       C26  C66 -C66 -C26 -C26 -C66  C66  C26 
	      -C26 -C66  C66  C26  C26  C66 -C66 -C26 
	      -C22 -C26  C26  C22  C22  C26 -C26 -C22 
	      -C22 -C26  C26  C22  C22  C26 -C26 -C22 
	      -C26 -C66  C66  C26  C26  C66 -C66 -C26 
	       C26  C66 -C66 -C26 -C26 -C66  C66  C26 
	       C22  C26 -C26 -C22 -C22 -C26  C26  C22 
	    */
	    t = coeff[18];	/* C26 */
	    a0 += t;
	    a3 -= t;    
	    t = coeff[19];	/* C66 */
	    a1 += t;
	    a2 -= t;    
	    break;

	  case 13:
	    /*
	      Coefficient[1][3]

	       C13 -C17 -C11 -C15  C15  C11  C17 -C13 
	       C33 -C37 -C13 -C35  C35  C13  C37 -C33 
	       C35 -C57 -C15 -C55  C55  C15  C57 -C35 
	       C37 -C77 -C17 -C57  C57  C17  C77 -C37 
	      -C37  C77  C17  C57 -C57 -C17 -C77  C37 
	      -C35  C57  C15  C55 -C55 -C15 -C57  C35 
	      -C33  C37  C13  C35 -C35 -C13 -C37  C33 
	      -C13  C17  C11  C15 -C15 -C11 -C17  C13 
	    */
	    t = coeff[21];	/* C13 */
	    d2 -= t;    
	    t = coeff[24];	/* C33 */
	    d0 += t;    
	    t = coeff[25];	/* C35 */
	    d3 -= t;    
	    t = coeff[26];	/* C37 */
	    d1 -= t;    
	    break;

	  case 14:
	    /*
	      Coefficient[0][4]

	       C44 -C44 -C44  C44  C44 -C44 -C44  C44 
 	       C44 -C44 -C44  C44  C44 -C44 -C44  C44 
	       C44 -C44 -C44  C44  C44 -C44 -C44  C44 
	       C44 -C44 -C44  C44  C44 -C44 -C44  C44 
	       C44 -C44 -C44  C44  C44 -C44 -C44  C44 
	       C44 -C44 -C44  C44  C44 -C44 -C44  C44 
	       C44 -C44 -C44  C44  C44 -C44 -C44  C44 
	       C44 -C44 -C44  C44  C44 -C44 -C44  C44 
	    */
	    t = coeff[0];	/* C44 */
	    a0 += t;
	    a1 -= t;
	    a2 -= t;
	    a3 += t;    
	    break;

	  case 15:
	    /*
	      Coefficient[0][5]

	       C45 -C14  C47  C34 -C34 -C47  C14 -C45 
	       C45 -C14  C47  C34 -C34 -C47  C14 -C45 
	       C45 -C14  C47  C34 -C34 -C47  C14 -C45 
	       C45 -C14  C47  C34 -C34 -C47  C14 -C45 
	       C45 -C14  C47  C34 -C34 -C47  C14 -C45 
	       C45 -C14  C47  C34 -C34 -C47  C14 -C45 
	       C45 -C14  C47  C34 -C34 -C47  C14 -C45 
	       C45 -C14  C47  C34 -C34 -C47  C14 -C45 
	    */
	    t = coeff[4];	/* C14 */
	    b1 -= t;    
	    t = coeff[5];	/* C34 */
	    b3 += t;    
	    t = coeff[6];	/* C45 */
	    b0 += t;    
	    t = coeff[7];	/* C47 */
	    b2 += t;    
	    break;

	  case 16:
	    /*
	      Coefficient[1][4]

	       C14 -C14 -C14  C14  C14 -C14 -C14  C14 
	       C34 -C34 -C34  C34  C34 -C34 -C34  C34 
	       C45 -C45 -C45  C45  C45 -C45 -C45  C45 
	       C47 -C47 -C47  C47  C47 -C47 -C47  C47 
	      -C47  C47  C47 -C47 -C47  C47  C47 -C47 
	      -C45  C45  C45 -C45 -C45  C45  C45 -C45 
	      -C34  C34  C34 -C34 -C34  C34  C34 -C34 
	      -C14  C14  C14 -C14 -C14  C14  C14 -C14 
	    */
	    t = coeff[5];	/* C34 */
	    c0 += t;
	    c1 -= t;
	    c2 -= t;
	    c3 += t;
	    break;

	  case 17:
	    /*
	      Coefficient[2][3]

	       C23 -C27 -C12 -C25  C25  C12  C27 -C23 
	       C36 -C67 -C16 -C56  C56  C16  C67 -C36 
	      -C36  C67  C16  C56 -C56 -C16 -C67  C36 
	      -C23  C27  C12  C25 -C25 -C12 -C27  C23 
	      -C23  C27  C12  C25 -C25 -C12 -C27  C23 
	      -C36  C67  C16  C56 -C56 -C16 -C67  C36 
	       C36 -C67 -C16 -C56  C56  C16  C67 -C36 
	       C23 -C27 -C12 -C25  C25  C12  C27 -C23 
	    */
	    t = coeff[12];	/* C16 */
	    b2 -= t;    
	    t = coeff[13];	/* C36 */
	    b0 += t;    
	    t = coeff[14];	/* C56 */
	    b3 -= t;    
	    t = coeff[15];	/* C67 */
	    b1 -= t;
	    break;

	  case 18:
	    /*
	      Coefficient[3][2]

 	       C23  C36 -C36 -C23 -C23 -C36  C36  C23 
	      -C27 -C67  C67  C27  C27  C67 -C67 -C27 
	      -C12 -C16  C16  C12  C12  C16 -C16 -C12 
	      -C25 -C56  C56  C25  C25  C56 -C56 -C25 
	       C25  C56 -C56 -C25 -C25 -C56  C56  C25 
	       C12  C16 -C16 -C12 -C12 -C16  C16  C12 
	       C27  C67 -C67 -C27 -C27 -C67  C67  C27 
	      -C23 -C36  C36  C23  C23  C36 -C36 -C23 
	    */
	    t = coeff[11];	/* C27 */
	    c0 -= t;
	    c3 += t;
	    t = coeff[15];	/* C67 */
	    c1 -= t;
	    c2 += t;
	    break;

	  case 19:
	    /*
	      Coefficient[4][1]

	       C14  C34  C45  C47 -C47 -C45 -C34 -C14 
	      -C14 -C34 -C45 -C47  C47  C45  C34  C14 
	      -C14 -C34 -C45 -C47  C47  C45  C34  C14 
	       C14  C34  C45  C47 -C47 -C45 -C34 -C14 
	       C14  C34  C45  C47 -C47 -C45 -C34 -C14 
	      -C14 -C34 -C45 -C47  C47  C45  C34  C14 
	      -C14 -C34 -C45 -C47  C47  C45  C34  C14 
	       C14  C34  C45  C47 -C47 -C45 -C34 -C14 
	    */
	    t = coeff[4];	/* C14 */
	    b0 -= t;    
	    t = coeff[5];	/* C34 */
	    b1 -= t;    
	    t = coeff[6];	/* C45 */
	    b2 -= t;    
	    t = coeff[7];	/* C47 */
	    b3 -= t;    
	    break;

	  case 20:
	    /*
	      Coefficient[5][0]

	       C45  C45  C45  C45  C45  C45  C45  C45 
	      -C14 -C14 -C14 -C14 -C14 -C14 -C14 -C14 
	       C47  C47  C47  C47  C47  C47  C47  C47 
	       C34  C34  C34  C34  C34  C34  C34  C34 
	      -C34 -C34 -C34 -C34 -C34 -C34 -C34 -C34 
	      -C47 -C47 -C47 -C47 -C47 -C47 -C47 -C47 
	       C14  C14  C14  C14  C14  C14  C14  C14 
	      -C45 -C45 -C45 -C45 -C45 -C45 -C45 -C45 
	    */
	    t = coeff[4];	/* C14 */
	    c0 -= t;
	    c1 -= t;
	    c2 -= t;
	    c3 -= t;
	    break;
	    
	  case 21:
	    /*
	      Coefficient[6][0]

	       C46  C46  C46  C46  C46  C46  C46  C46 
	      -C24 -C24 -C24 -C24 -C24 -C24 -C24 -C24 
	       C24  C24  C24  C24  C24  C24  C24  C24 
	      -C46 -C46 -C46 -C46 -C46 -C46 -C46 -C46 
	      -C46 -C46 -C46 -C46 -C46 -C46 -C46 -C46 
	       C24  C24  C24  C24  C24  C24  C24  C24 
	      -C24 -C24 -C24 -C24 -C24 -C24 -C24 -C24 
	       C46  C46  C46  C46  C46  C46  C46  C46 
	    */
	    t = coeff[2];	/* C24 */
	    a0 -= t;
	    a1 -= t;
	    a2 -= t;
	    a3 -= t;    
	    break;

	  case 22:
	    /*
	      Coefficient[5][1]

	       C15  C35  C55  C57 -C57 -C55 -C35 -C15 
	      -C11 -C13 -C15 -C17  C17  C15  C13  C11 
	       C17  C37  C57  C77 -C77 -C57 -C37 -C17 
	       C13  C33  C35  C37 -C37 -C35 -C33 -C13 
	      -C13 -C33 -C35 -C37  C37  C35  C33  C13 
	      -C17 -C37 -C57 -C77  C77  C57  C37  C17 
	       C11  C13  C15  C17 -C17 -C15 -C13 -C11 
	      -C15 -C35 -C55 -C57  C57  C55  C35  C15 
	    */
	    t = coeff[20];	/* C11 */
	    d0 -= t;    
	    t = coeff[21];	/* C13 */
	    d1 -= t;    
	    t = coeff[22];	/* C15 */
	    d2 -= t;    
	    t = coeff[23];	/* C17 */
	    d3 -= t;    
	    break;

	  case 23:
	    /*
	      Coefficient[4][2]

	       C24  C46 -C46 -C24 -C24 -C46  C46  C24 
	      -C24 -C46  C46  C24  C24  C46 -C46 -C24 
	      -C24 -C46  C46  C24  C24  C46 -C46 -C24 
	       C24  C46 -C46 -C24 -C24 -C46  C46  C24 
	       C24  C46 -C46 -C24 -C24 -C46  C46  C24 
	      -C24 -C46  C46  C24  C24  C46 -C46 -C24 
	      -C24 -C46  C46  C24  C24  C46 -C46 -C24 
	       C24  C46 -C46 -C24 -C24 -C46  C46  C24 
	    */
	    t = coeff[2];	/* C24 */
	    a0 -= t;
	    a3 += t;    
	    t = coeff[3];	/* C46 */
	    a1 -= t;
	    a2 += t;    
	    break;

	  case 24:
	    /*
	      Coefficient[3][3]

	       C33 -C37 -C13 -C35  C35  C13  C37 -C33 
	      -C37  C77  C17  C57 -C57 -C17 -C77  C37 
	      -C13  C17  C11  C15 -C15 -C11 -C17  C13 
	      -C35  C57  C15  C55 -C55 -C15 -C57  C35 
	       C35 -C57 -C15 -C55  C55  C15  C57 -C35 
	       C13 -C17 -C11 -C15  C15  C11  C17 -C13 
	       C37 -C77 -C17 -C57  C57  C17  C77 -C37 
	      -C33  C37  C13  C35 -C35 -C13 -C37  C33 
	    */
	    t = coeff[23];	/* C17 */
	    d2 += t;    
	    t = coeff[26];	/* C37 */
	    d0 -= t;    
	    t = coeff[28];	/* C57 */
	    d3 += t;    
	    t = coeff[29];	/* C77 */
	    d1 += t;    
	    break;

	  case 25:
	    /*
	      Coefficient[2][4]

	       C24 -C24 -C24  C24  C24 -C24 -C24  C24 
	       C46 -C46 -C46  C46  C46 -C46 -C46  C46 
	      -C46  C46  C46 -C46 -C46  C46  C46 -C46 
	      -C24  C24  C24 -C24 -C24  C24  C24 -C24 
	      -C24  C24  C24 -C24 -C24  C24  C24 -C24 
	      -C46  C46  C46 -C46 -C46  C46  C46 -C46 
	       C46 -C46 -C46  C46  C46 -C46 -C46  C46 
	       C24 -C24 -C24  C24  C24 -C24 -C24  C24 
	    */
	    t = coeff[3];	/* C46 */
	    a0 += t;
	    a1 -= t;
	    a2 -= t;
	    a3 += t;
	    break;

	  case 26:
	    /*
	      Coefficient[1][5]

	       C15 -C11  C17  C13 -C13 -C17  C11 -C15 
	       C35 -C13  C37  C33 -C33 -C37  C13 -C35 
	       C55 -C15  C57  C35 -C35 -C57  C15 -C55 
	       C57 -C17  C77  C37 -C37 -C77  C17 -C57 
	      -C57  C17 -C77 -C37  C37  C77 -C17  C57 
	      -C55  C15 -C57 -C35  C35  C57 -C15  C55 
	      -C35  C13 -C37 -C33  C33  C37 -C13  C35 
	      -C15  C11 -C17 -C13  C13  C17 -C11  C15 
	    */
	    t = coeff[21];	/* C13 */
	    d1 -= t;    
	    t = coeff[24];	/* C33 */
	    d3 += t;    
	    t = coeff[25];	/* C35 */
	    d0 += t;    
	    t = coeff[26];	/* C37 */
	    d2 += t;    
	    break;

	  case 27:
	    /*
	      Coefficient[0][6]

	       C46 -C24  C24 -C46 -C46  C24 -C24  C46 
	       C46 -C24  C24 -C46 -C46  C24 -C24  C46 
	       C46 -C24  C24 -C46 -C46  C24 -C24  C46 
	       C46 -C24  C24 -C46 -C46  C24 -C24  C46 
	       C46 -C24  C24 -C46 -C46  C24 -C24  C46 
	       C46 -C24  C24 -C46 -C46  C24 -C24  C46 
	       C46 -C24  C24 -C46 -C46  C24 -C24  C46 
	       C46 -C24  C24 -C46 -C46  C24 -C24  C46 
	    */
	    t = coeff[2];	/* C24 */
	    a1 -= t;
	    a2 += t;
	    t = coeff[3];	/* C46 */
	    a0 += t;
	    a3 -= t;
	    break;

	  case 28:
	    /*
	      Coefficient[0][7]

	       C47 -C45  C34 -C14  C14 -C34  C45 -C47 
	       C47 -C45  C34 -C14  C14 -C34  C45 -C47 
	       C47 -C45  C34 -C14  C14 -C34  C45 -C47 
	       C47 -C45  C34 -C14  C14 -C34  C45 -C47 
	       C47 -C45  C34 -C14  C14 -C34  C45 -C47 
	       C47 -C45  C34 -C14  C14 -C34  C45 -C47 
	       C47 -C45  C34 -C14  C14 -C34  C45 -C47 
	       C47 -C45  C34 -C14  C14 -C34  C45 -C47 
	    */
	    t = coeff[4];	/* C14 */
	    b3 -= t;    
	    t = coeff[5];	/* C34 */
	    b2 += t;    
	    t = coeff[6];	/* C45 */
	    b1 -= t;    
	    t = coeff[7];	/* C47 */
	    b0 += t;    
	    break;

	  case 29:
	    /*
	      Coefficient[1][6]

	       C16 -C12  C12 -C16 -C16  C12 -C12  C16 
	       C36 -C23  C23 -C36 -C36  C23 -C23  C36 
	       C56 -C25  C25 -C56 -C56  C25 -C25  C56 
	       C67 -C27  C27 -C67 -C67  C27 -C27  C67 
	      -C67  C27 -C27  C67  C67 -C27  C27 -C67 
	      -C56  C25 -C25  C56  C56 -C25  C25 -C56 
	      -C36  C23 -C23  C36  C36 -C23  C23 -C36 
	      -C16  C12 -C12  C16  C16 -C12  C12 -C16 
	    */
	    t = coeff[9];	/* C23 */
	    c1 -= t;
	    c2 += t;
	    t = coeff[13];	/* C36 */
	    c0 += t;
	    c3 -= t;
	    break;

	  case 30:
	    /*
	      Coefficient[2][5]

	       C25 -C12  C27  C23 -C23 -C27  C12 -C25 
	       C56 -C16  C67  C36 -C36 -C67  C16 -C56 
	      -C56  C16 -C67 -C36  C36  C67 -C16  C56 
	      -C25  C12 -C27 -C23  C23  C27 -C12  C25 
	      -C25  C12 -C27 -C23  C23  C27 -C12  C25 
	      -C56  C16 -C67 -C36  C36  C67 -C16  C56 
	       C56 -C16  C67  C36 -C36 -C67  C16 -C56 
	       C25 -C12  C27  C23 -C23 -C27  C12 -C25 
	    */
	    t = coeff[12];	/* C16 */
	    b1 -= t;    
	    t = coeff[13];	/* C36 */
	    b3 += t;    
	    t = coeff[14];	/* C56 */
	    b0 += t;    
	    t = coeff[15];	/* C67 */
	    b2 += t;    
	    break;

	  case 31:
	    /*
	      Coefficient[3][4]

	       C34 -C34 -C34  C34  C34 -C34 -C34  C34 
	      -C47  C47  C47 -C47 -C47  C47  C47 -C47 
	      -C14  C14  C14 -C14 -C14  C14  C14 -C14 
	      -C45  C45  C45 -C45 -C45  C45  C45 -C45 
	       C45 -C45 -C45  C45  C45 -C45 -C45  C45 
	       C14 -C14 -C14  C14  C14 -C14 -C14  C14 
	       C47 -C47 -C47  C47  C47 -C47 -C47  C47 
	      -C34  C34  C34 -C34 -C34  C34  C34 -C34 
	    */
	    t = coeff[7];	/* C47 */
	    c0 -= t;
	    c1 += t;
	    c2 += t;
	    c3 -= t;
	    break;

	  case 32:
	    /*
	      Coefficient[4][3]

	       C34 -C47 -C14 -C45  C45  C14  C47 -C34 
	      -C34  C47  C14  C45 -C45 -C14 -C47  C34 
	      -C34  C47  C14  C45 -C45 -C14 -C47  C34 
	       C34 -C47 -C14 -C45  C45  C14  C47 -C34 
	       C34 -C47 -C14 -C45  C45  C14  C47 -C34 
	      -C34  C47  C14  C45 -C45 -C14 -C47  C34 
	      -C34  C47  C14  C45 -C45 -C14 -C47  C34 
	       C34 -C47 -C14 -C45  C45  C14  C47 -C34 
	    */
	    t = coeff[4];	/* C14 */
	    b2 += t;    
	    t = coeff[5];	/* C34 */
	    b0 -= t;    
	    t = coeff[6];	/* C45 */
	    b3 += t;    
	    t = coeff[7];	/* C47 */
	    b1 += t;    
	    break;

	  case 33:
	    /*
	      Coefficient[5][2]

	       C25  C56 -C56 -C25 -C25 -C56  C56  C25 
	      -C12 -C16  C16  C12  C12  C16 -C16 -C12 
	       C27  C67 -C67 -C27 -C27 -C67  C67  C27 
	       C23  C36 -C36 -C23 -C23 -C36  C36  C23 
	      -C23 -C36  C36  C23  C23  C36 -C36 -C23 
	      -C27 -C67  C67  C27  C27  C67 -C67 -C27 
	       C12  C16 -C16 -C12 -C12 -C16  C16  C12 
	      -C25 -C56  C56  C25  C25  C56 -C56 -C25 
	    */
	    t = coeff[8];	/* C12 */
	    c0 -= t;
	    c3 += t;
	    t = coeff[12];	/* C16 */
	    c1 -= t;
	    c2 += t;
	    break;

	  case 34:
	    /*
	      Coefficient[6][1]

	       C16  C36  C56  C67 -C67 -C56 -C36 -C16 
	      -C12 -C23 -C25 -C27  C27  C25  C23  C12 
	       C12  C23  C25  C27 -C27 -C25 -C23 -C12 
	      -C16 -C36 -C56 -C67  C67  C56  C36  C16 
	      -C16 -C36 -C56 -C67  C67  C56  C36  C16 
	       C12  C23  C25  C27 -C27 -C25 -C23 -C12 
	      -C12 -C23 -C25 -C27  C27  C25  C23  C12 
	       C16  C36  C56  C67 -C67 -C56 -C36 -C16 
	    */
	    t = coeff[8];	/* C12 */
	    b0 -= t;    
	    t = coeff[9];	/* C23 */
	    b1 -= t;    
	    t = coeff[10];	/* C25 */
	    b2 -= t;    
	    t = coeff[11];	/* C27 */
	    b3 -= t;    
	    break;

	  case 35:
	    /*
	      Coefficient[7][0]

	       C47  C47  C47  C47  C47  C47  C47  C47 
	      -C45 -C45 -C45 -C45 -C45 -C45 -C45 -C45 
	       C34  C34  C34  C34  C34  C34  C34  C34 
	      -C14 -C14 -C14 -C14 -C14 -C14 -C14 -C14 
	       C14  C14  C14  C14  C14  C14  C14  C14 
	      -C34 -C34 -C34 -C34 -C34 -C34 -C34 -C34 
	       C45  C45  C45  C45  C45  C45  C45  C45 
	      -C47 -C47 -C47 -C47 -C47 -C47 -C47 -C47 
	    */
	    t = coeff[6];	/* C45 */
	    c0 -= t;
	    c1 -= t;
	    c2 -= t;
	    c3 -= t;    
	    break;

	  case 36:
	    /*
	      Coefficient[7][1]

	       C17  C37  C57  C77 -C77 -C57 -C37 -C17 
	      -C15 -C35 -C55 -C57  C57  C55  C35  C15 
	       C13  C33  C35  C37 -C37 -C35 -C33 -C13 
	      -C11 -C13 -C15 -C17  C17  C15  C13  C11 
	       C11  C13  C15  C17 -C17 -C15 -C13 -C11 
	      -C13 -C33 -C35 -C37  C37  C35  C33  C13 
	       C15  C35  C55  C57 -C57 -C55 -C35 -C15 
	      -C17 -C37 -C57 -C77  C77  C57  C37  C17 
	    */
	    t = coeff[22];	/* C15 */
	    d0 -= t;    
	    t = coeff[25];	/* C35 */
	    d1 -= t;    
	    t = coeff[27];	/* C55 */
	    d2 -= t;    
	    t = coeff[28];	/* C57 */
	    d3 -= t;    
	    break;

	  case 37:
	    /*
	      Coefficient[6][2]

	       C26  C66 -C66 -C26 -C26 -C66  C66  C26 
	      -C22 -C26  C26  C22  C22  C26 -C26 -C22 
	       C22  C26 -C26 -C22 -C22 -C26  C26  C22 
	      -C26 -C66  C66  C26  C26  C66 -C66 -C26 
	      -C26 -C66  C66  C26  C26  C66 -C66 -C26 
	       C22  C26 -C26 -C22 -C22 -C26  C26  C22 
	      -C22 -C26  C26  C22  C22  C26 -C26 -C22 
	       C26  C66 -C66 -C26 -C26 -C66  C66  C26 
	    */
	    t = coeff[16];	/* C22 */
	    a0 -= t;
	    a3 += t;
	    t = coeff[17];	/* C26 */
	    a1 -= t;
	    a2 += t;    
	    break;

	  case 38:
	    /*
	      Coefficient[5][3]

	       C35 -C57 -C15 -C55  C55  C15  C57 -C35 
	      -C13  C17  C11  C15 -C15 -C11 -C17  C13 
	       C37 -C77 -C17 -C57  C57  C17  C77 -C37 
	       C33 -C37 -C13 -C35  C35  C13  C37 -C33 
	      -C33  C37  C13  C35 -C35 -C13 -C37  C33 
	      -C37  C77  C17  C57 -C57 -C17 -C77  C37 
	       C13 -C17 -C11 -C15  C15  C11  C17 -C13 
	      -C35  C57  C15  C55 -C55 -C15 -C57  C35 
	    */
	    t = coeff[20];	/* C11 */
	    d2 += t;    
	    t = coeff[21];	/* C13 */
	    d0 -= t;    
	    t = coeff[22];	/* C15 */
	    d3 += t;    
	    t = coeff[23];	/* C17 */
	    d1 += t;    
	    break;

	  case 39:
	    /*
	      Coefficient[4][4]

	       C44 -C44 -C44  C44  C44 -C44 -C44  C44 
	      -C44  C44  C44 -C44 -C44  C44  C44 -C44 
	      -C44  C44  C44 -C44 -C44  C44  C44 -C44 
	       C44 -C44 -C44  C44  C44 -C44 -C44  C44 
	       C44 -C44 -C44  C44  C44 -C44 -C44  C44 
	      -C44  C44  C44 -C44 -C44  C44  C44 -C44 
	      -C44  C44  C44 -C44 -C44  C44  C44 -C44 
	       C44 -C44 -C44  C44  C44 -C44 -C44  C44 
	    */
	    t = coeff[0];	/* C44 */
	    a0 -= t;
	    a1 += t;
	    a2 += t;
	    a3 -= t;
	    break;

	  case 40:
	    /*
	      Coefficient[3][5]

	       C35 -C13  C37  C33 -C33 -C37  C13 -C35 
	      -C57  C17 -C77 -C37  C37  C77 -C17  C57 
	      -C15  C11 -C17 -C13  C13  C17 -C11  C15 
	      -C55  C15 -C57 -C35  C35  C57 -C15  C55 
	       C55 -C15  C57  C35 -C35 -C57  C15 -C55 
	       C15 -C11  C17  C13 -C13 -C17  C11 -C15 
	       C57 -C17  C77  C37 -C37 -C77  C17 -C57 
	      -C35  C13 -C37 -C33  C33  C37 -C13  C35 
	    */
	    t = coeff[23];	/* C17 */
	    d1 += t;    
	    t = coeff[26];	/* C37 */
	    d3 -= t;    
	    t = coeff[28];	/* C57 */
	    d0 -= t;    
	    t = coeff[29];	/* C77 */
	    d2 -= t;    
	    break;

	  case 41:
	    /*
	      Coefficient[2][6]

	       C26 -C22  C22 -C26 -C26  C22 -C22  C26 
	       C66 -C26  C26 -C66 -C66  C26 -C26  C66 
	      -C66  C26 -C26  C66  C66 -C26  C26 -C66 
	      -C26  C22 -C22  C26  C26 -C22  C22 -C26 
	      -C26  C22 -C22  C26  C26 -C22  C22 -C26 
	      -C66  C26 -C26  C66  C66 -C26  C26 -C66 
	       C66 -C26  C26 -C66 -C66  C26 -C26  C66 
	       C26 -C22  C22 -C26 -C26  C22 -C22  C26 
	    */
	    t = coeff[18];	/* C26 */
	    a1 -= t;
	    a2 += t;
	    t = coeff[19];	/* C66 */
	    a0 += t;
	    a3 -= t;
	    break;

	  case 42:
	    /*
	       Coefficient[1][7]

	        C17 -C15  C13 -C11  C11 -C13  C15 -C17 
	        C37 -C35  C33 -C13  C13 -C33  C35 -C37 
	        C57 -C55  C35 -C15  C15 -C35  C55 -C57 
	        C77 -C57  C37 -C17  C17 -C37  C57 -C77 
	       -C77  C57 -C37  C17 -C17  C37 -C57  C77 
	       -C57  C55 -C35  C15 -C15  C35 -C55  C57 
	       -C37  C35 -C33  C13 -C13  C33 -C35  C37 
	       -C17  C15 -C13  C11 -C11  C13 -C15  C17 
	    */
	    t = coeff[21];	/* C13 */
	    d3 -= t;    
	    t = coeff[24];	/* C33 */
	    d2 += t;    
	    t = coeff[25];	/* C35 */
	    d1 -= t;    
	    t = coeff[26];	/* C37 */
	    d0 += t;    
	    break;

	  case 43:
	    /*
	      Coefficient[2][7]

	       C27 -C25  C23 -C12  C12 -C23  C25 -C27 
	       C67 -C56  C36 -C16  C16 -C36  C56 -C67 
	      -C67  C56 -C36  C16 -C16  C36 -C56  C67 
	      -C27  C25 -C23  C12 -C12  C23 -C25  C27 
	      -C27  C25 -C23  C12 -C12  C23 -C25  C27 
	      -C67  C56 -C36  C16 -C16  C36 -C56  C67 
	       C67 -C56  C36 -C16  C16 -C36  C56 -C67 
	       C27 -C25  C23 -C12  C12 -C23  C25 -C27 
	    */
	    t = coeff[12];	/* C16 */
	    b3 -= t;    
	    t = coeff[13];	/* C36 */
	    b2 += t;    
	    t = coeff[14];	/* C56 */
	    b1 -= t;    
	    t = coeff[15];	/* C67 */
	    b0 += t;    
	    break;

	  case 44:
	    /*
	      Coefficient[3][6]

	       C36 -C23  C23 -C36 -C36  C23 -C23  C36 
	      -C67  C27 -C27  C67  C67 -C27  C27 -C67 
	      -C16  C12 -C12  C16  C16 -C12  C12 -C16 
	      -C56  C25 -C25  C56  C56 -C25  C25 -C56 
	       C56 -C25  C25 -C56 -C56  C25 -C25  C56 
	       C16 -C12  C12 -C16 -C16  C12 -C12  C16 
	       C67 -C27  C27 -C67 -C67  C27 -C27  C67 
	      -C36  C23 -C23  C36  C36 -C23  C23 -C36 
	    */
	    t = coeff[11];	/* C27 */
	    c1 += t;
	    c2 -= t;
	    t = coeff[15];	/* C67 */
	    c0 -= t;
	    c3 += t;
	    break;

	  case 45:
	    /*
	      Coefficient[4][5]

	       C45 -C14  C47  C34 -C34 -C47  C14 -C45 
	      -C45  C14 -C47 -C34  C34  C47 -C14  C45 
	      -C45  C14 -C47 -C34  C34  C47 -C14  C45 
	       C45 -C14  C47  C34 -C34 -C47  C14 -C45 
	       C45 -C14  C47  C34 -C34 -C47  C14 -C45 
	      -C45  C14 -C47 -C34  C34  C47 -C14  C45 
	      -C45  C14 -C47 -C34  C34  C47 -C14  C45 
	       C45 -C14  C47  C34 -C34 -C47  C14 -C45 
	    */
	    t = coeff[4];	/* C14 */
	    b1 += t;    
	    t = coeff[5];	/* C34 */
	    b3 -= t;    
	    t = coeff[6];	/* C45 */
	    b0 -= t;    
	    t = coeff[7];	/* C47 */
	    b2 -= t;    
	    break;

	  case 46:
	    /*
	      Coefficient[5][4]

	       C45 -C45 -C45  C45  C45 -C45 -C45  C45 
	      -C14  C14  C14 -C14 -C14  C14  C14 -C14 
	       C47 -C47 -C47  C47  C47 -C47 -C47  C47 
	       C34 -C34 -C34  C34  C34 -C34 -C34  C34 
	      -C34  C34  C34 -C34 -C34  C34  C34 -C34 
	      -C47  C47  C47 -C47 -C47  C47  C47 -C47 
	       C14 -C14 -C14  C14  C14 -C14 -C14  C14 
	      -C45  C45  C45 -C45 -C45  C45  C45 -C45 
	    */
	    t = coeff[4];	/* C14 */
	    c0 -= t;
	    c1 += t;
	    c2 += t;
	    c3 -= t;
	    break;

	  case 47:
	    /*
	      Coefficient[6][3]

	       C36 -C67 -C16 -C56  C56  C16  C67 -C36 
	      -C23  C27  C12  C25 -C25 -C12 -C27  C23 
	       C23 -C27 -C12 -C25  C25  C12  C27 -C23 
	      -C36  C67  C16  C56 -C56 -C16 -C67  C36 
	      -C36  C67  C16  C56 -C56 -C16 -C67  C36 
	       C23 -C27 -C12 -C25  C25  C12  C27 -C23 
	      -C23  C27  C12  C25 -C25 -C12 -C27  C23 
	       C36 -C67 -C16 -C56  C56  C16  C67 -C36 
	    */
	    t = coeff[8];	/* C12 */
	    b2 += t;    
	    t = coeff[9];	/* C23 */
	    b0 -= t;    
	    t = coeff[10];	/* C25 */
	    b3 += t;    
	    t = coeff[11];	/* C27 */
	    b1 += t;    
	    break;

	  case 48:
	    /*
	      Coefficient[7][2]

	       C27  C67 -C67 -C27 -C27 -C67  C67  C27 
	      -C25 -C56  C56  C25  C25  C56 -C56 -C25 
	       C23  C36 -C36 -C23 -C23 -C36  C36  C23 
	      -C12 -C16  C16  C12  C12  C16 -C16 -C12 
	       C12  C16 -C16 -C12 -C12 -C16  C16  C12 
	      -C23 -C36  C36  C23  C23  C36 -C36 -C23 
	       C25  C56 -C56 -C25 -C25 -C56  C56  C25 
	      -C27 -C67  C67  C27  C27  C67 -C67 -C27 
	    */
	    t = coeff[10];	/* C25 */
	    c0 -= t;
	    c3 += t;
	    t = coeff[14];	/* C56 */
	    c1 -= t;
	    c2 += t;
	    break;

	  case 49:
	    /*
	      Coefficient[7][3]
	      
	       C37 -C77 -C17 -C57  C57  C17  C77 -C37 
	      -C35  C57  C15  C55 -C55 -C15 -C57  C35 
	       C33 -C37 -C13 -C35  C35  C13  C37 -C33 
	      -C13  C17  C11  C15 -C15 -C11 -C17  C13 
	       C13 -C17 -C11 -C15  C15  C11  C17 -C13 
	      -C33  C37  C13  C35 -C35 -C13 -C37  C33 
	       C35 -C57 -C15 -C55  C55  C15  C57 -C35 
	      -C37  C77  C17  C57 -C57 -C17 -C77  C37 
	    */
	    t = coeff[22];	/* C15 */
	    d2 += t;    
	    t = coeff[25];	/* C35 */
	    d0 -= t;    
	    t = coeff[27];	/* C55 */
	    d3 += t;    
	    t = coeff[28];	/* C57 */
	    d1 += t;    
	    break;

	  case 50:
	    /*
	      Coefficient[6][4]

	       C46 -C46 -C46  C46  C46 -C46 -C46  C46 
	      -C24  C24  C24 -C24 -C24  C24  C24 -C24 
	       C24 -C24 -C24  C24  C24 -C24 -C24  C24 
	      -C46  C46  C46 -C46 -C46  C46  C46 -C46 
	      -C46  C46  C46 -C46 -C46  C46  C46 -C46 
	       C24 -C24 -C24  C24  C24 -C24 -C24  C24 
	      -C24  C24  C24 -C24 -C24  C24  C24 -C24 
	       C46 -C46 -C46  C46  C46 -C46 -C46  C46 
	    */
	    t = coeff[2];	/* C24 */
	    a0 -= t;
	    a1 += t;
	    a2 += t;
	    a3 -= t;
	    break;

	  case 51:
	    /*
	      Coefficient[5][5]

	       C55 -C15  C57  C35 -C35 -C57  C15 -C55 
	      -C15  C11 -C17 -C13  C13  C17 -C11  C15 
	       C57 -C17  C77  C37 -C37 -C77  C17 -C57 
	       C35 -C13  C37  C33 -C33 -C37  C13 -C35 
	      -C35  C13 -C37 -C33  C33  C37 -C13  C35 
	      -C57  C17 -C77 -C37  C37  C77 -C17  C57 
	       C15 -C11  C17  C13 -C13 -C17  C11 -C15 
	      -C55  C15 -C57 -C35  C35  C57 -C15  C55 
	    */
	    t = coeff[20];	/* C11 */
	    d1 += t;    
	    t = coeff[21];	/* C13 */
	    d3 -= t;    
	    t = coeff[22];	/* C15 */
	    d0 -= t;    
	    t = coeff[23];	/* C17 */
	    d2 -= t;    
	    break;

	  case 52:
	    /*
	      Coefficient[4][6]

	       C46 -C24  C24 -C46 -C46  C24 -C24  C46 
	      -C46  C24 -C24  C46  C46 -C24  C24 -C46 
	      -C46  C24 -C24  C46  C46 -C24  C24 -C46 
	       C46 -C24  C24 -C46 -C46  C24 -C24  C46 
	       C46 -C24  C24 -C46 -C46  C24 -C24  C46 
	      -C46  C24 -C24  C46  C46 -C24  C24 -C46 
	      -C46  C24 -C24  C46  C46 -C24  C24 -C46 
	      C46 -C24  C24 -C46 -C46  C24 -C24  C46 
	    */
	    t = coeff[2];	/* C24 */
	    a1 += t;
	    a2 -= t;
	    t = coeff[3];	/* C46 */
	    a0 -= t;
	    a3 += t;
	    break;

	  case 53:
	    /*
	      Coefficient[3][7]

	       C37 -C35  C33 -C13  C13 -C33  C35 -C37 
	      -C77  C57 -C37  C17 -C17  C37 -C57  C77 
	      -C17  C15 -C13  C11 -C11  C13 -C15  C17 
	      -C57  C55 -C35  C15 -C15  C35 -C55  C57 
	       C57 -C55  C35 -C15  C15 -C35  C55 -C57 
	       C17 -C15  C13 -C11  C11 -C13  C15 -C17 
	       C77 -C57  C37 -C17  C17 -C37  C57 -C77 
	      -C37  C35 -C33  C13 -C13  C33 -C35  C37 
	    */
	    t = coeff[23];	/* C17 */
	    d3 += t;    
	    t = coeff[26];	/* C37 */
	    d2 -= t;    
	    t = coeff[28];	/* C57 */
	    d1 += t;    
	    t = coeff[29];	/* C77 */
	    d0 -= t;    
	    break;

	  case 54:
	    /*
	      Coefficient[4][7]

	       C47 -C45  C34 -C14  C14 -C34  C45 -C47 
	      -C47  C45 -C34  C14 -C14  C34 -C45  C47 
	      -C47  C45 -C34  C14 -C14  C34 -C45  C47 
	       C47 -C45  C34 -C14  C14 -C34  C45 -C47 
	       C47 -C45  C34 -C14  C14 -C34  C45 -C47 
	      -C47  C45 -C34  C14 -C14  C34 -C45  C47 
	      -C47  C45 -C34  C14 -C14  C34 -C45  C47 
	       C47 -C45  C34 -C14  C14 -C34  C45 -C47 
	    */
	    t = coeff[4];	/* C14 */
	    b3 += t;    
	    t = coeff[5];	/* C34 */
	    b2 -= t;    
	    t = coeff[6];	/* C45 */
	    b1 += t;    
	    t = coeff[7];	/* C47 */
	    b0 -= t;    
	    break;

	  case 55:
	    /*
	      Coefficient[5][6]

	       C56 -C25  C25 -C56 -C56  C25 -C25  C56 
	      -C16  C12 -C12  C16  C16 -C12  C12 -C16 
	       C67 -C27  C27 -C67 -C67  C27 -C27  C67 
	       C36 -C23  C23 -C36 -C36  C23 -C23  C36 
	      -C36  C23 -C23  C36  C36 -C23  C23 -C36 
	      -C67  C27 -C27  C67  C67 -C27  C27 -C67 
	       C16 -C12  C12 -C16 -C16  C12 -C12  C16 
	      -C56  C25 -C25  C56  C56 -C25  C25 -C56 
	    */
	    t = coeff[8];	/* C12 */
	    c1 += t;
	    c2 -= t;
	    t = coeff[12];	/* C16 */
	    c0 -= t;
	    c3 += t;
	    break;

	  case 56:
	    /*
	      Coefficient[6][5]

	       C56 -C16  C67  C36 -C36 -C67  C16 -C56 
	      -C25  C12 -C27 -C23  C23  C27 -C12  C25 
	       C25 -C12  C27  C23 -C23 -C27  C12 -C25 
	      -C56  C16 -C67 -C36  C36  C67 -C16  C56 
	      -C56  C16 -C67 -C36  C36  C67 -C16  C56 
	       C25 -C12  C27  C23 -C23 -C27  C12 -C25 
	      -C25  C12 -C27 -C23  C23  C27 -C12  C25 
	       C56 -C16  C67  C36 -C36 -C67  C16 -C56 
	    */
	    t = coeff[8];	/* C12 */
	    b1 += t;    
	    t = coeff[9];	/* C23 */
	    b3 -= t;    
	    t = coeff[10];	/* C25 */
	    b0 -= t;    
	    t = coeff[11];	/* C27 */
	    b2 -= t;    
	    break;

	  case 57:
	    /*
	      Coefficient[7][4]

	       C47 -C47 -C47  C47  C47 -C47 -C47  C47 
	      -C45  C45  C45 -C45 -C45  C45  C45 -C45 
	       C34 -C34 -C34  C34  C34 -C34 -C34  C34 
	      -C14  C14  C14 -C14 -C14  C14  C14 -C14 
	       C14 -C14 -C14  C14  C14 -C14 -C14  C14 
	      -C34  C34  C34 -C34 -C34  C34  C34 -C34 
	       C45 -C45 -C45  C45  C45 -C45 -C45  C45 
	      -C47  C47  C47 -C47 -C47  C47  C47 -C47 
	    */
	    t = coeff[6];	/* C45 */
	    c0 -= t;
	    c1 += t;
	    c2 += t;
	    c3 -= t;
	    break;

	  case 58:
	    /*
	      Coefficient[7][5]

	       C57 -C17  C77  C37 -C37 -C77  C17 -C57 
	      -C55  C15 -C57 -C35  C35  C57 -C15  C55 
	       C35 -C13  C37  C33 -C33 -C37  C13 -C35 
	      -C15  C11 -C17 -C13  C13  C17 -C11  C15 
	       C15 -C11  C17  C13 -C13 -C17  C11 -C15 
	      -C35  C13 -C37 -C33  C33  C37 -C13  C35 
	       C55 -C15  C57  C35 -C35 -C57  C15 -C55 
	      -C57  C17 -C77 -C37  C37  C77 -C17  C57 
	    */
	    t = coeff[22];	/* C15 */
	    d1 += t;    
	    t = coeff[25];	/* C35 */
	    d3 -= t;    
	    t = coeff[27];	/* C55 */
	    d0 -= t;    
	    t = coeff[28];	/* C57 */
	    d2 -= t;    
	    break;

	  case 59:
	    /*
	      Coefficient[6][6]

	       C66 -C26  C26 -C66 -C66  C26 -C26  C66 
	      -C26  C22 -C22  C26  C26 -C22  C22 -C26 
	       C26 -C22  C22 -C26 -C26  C22 -C22  C26 
	      -C66  C26 -C26  C66  C66 -C26  C26 -C66 
	      -C66  C26 -C26  C66  C66 -C26  C26 -C66 
	       C26 -C22  C22 -C26 -C26  C22 -C22  C26 
	      -C26  C22 -C22  C26  C26 -C22  C22 -C26 
	       C66 -C26  C26 -C66 -C66  C26 -C26  C66 
	    */
	    t = coeff[16];	/* C22 */
	    a1 += t;
	    a2 -= t;
	    t = coeff[17];	/* C26 */
	    a0 -= t;
	    a3 += t;
	    break;

	  case 60:
	    /*
	      Coefficient[5][7]

	       C57 -C55  C35 -C15  C15 -C35  C55 -C57 
	      -C17  C15 -C13  C11 -C11  C13 -C15  C17 
	       C77 -C57  C37 -C17  C17 -C37  C57 -C77 
	       C37 -C35  C33 -C13  C13 -C33  C35 -C37 
	      -C37  C35 -C33  C13 -C13  C33 -C35  C37 
	      -C77  C57 -C37  C17 -C17  C37 -C57  C77 
	       C17 -C15  C13 -C11  C11 -C13  C15 -C17 
	      -C57  C55 -C35  C15 -C15  C35 -C55  C57 
	    */
	    t = coeff[20];	/* C11 */
	    d3 += t;    
	    t = coeff[21];	/* C13 */
	    d2 -= t;    
	    t = coeff[22];	/* C15 */
	    d1 += t;    
	    t = coeff[23];	/* C17 */
	    d0 -= t;    
	    break;

	  case 61:
	    /*
	      Coefficient[6][7]

	       C67 -C56  C36 -C16  C16 -C36  C56 -C67 
	      -C27  C25 -C23  C12 -C12  C23 -C25  C27 
	       C27 -C25  C23 -C12  C12 -C23  C25 -C27 
	      -C67  C56 -C36  C16 -C16  C36 -C56  C67 
	      -C67  C56 -C36  C16 -C16  C36 -C56  C67 
	       C27 -C25  C23 -C12  C12 -C23  C25 -C27 
	      -C27  C25 -C23  C12 -C12  C23 -C25  C27 
	       C67 -C56  C36 -C16  C16 -C36  C56 -C67 
	    */
	    t = coeff[8];	/* C12 */
	    b3 += t;    
	    t = coeff[9];	/* C23 */
	    b2 -= t;    
	    t = coeff[10];	/* C25 */
	    b1 += t;    
	    t = coeff[11];	/* C27 */
	    b0 -= t;    
	    break;

	  case 62:
	    /*
	      Coefficient[7][6]

	       C67 -C27  C27 -C67 -C67  C27 -C27  C67 
	      -C56  C25 -C25  C56  C56 -C25  C25 -C56 
	       C36 -C23  C23 -C36 -C36  C23 -C23  C36 
	      -C16  C12 -C12  C16  C16 -C12  C12 -C16 
	       C16 -C12  C12 -C16 -C16  C12 -C12  C16 
	      -C36  C23 -C23  C36  C36 -C23  C23 -C36 
	       C56 -C25  C25 -C56 -C56  C25 -C25  C56 
	      -C67  C27 -C27  C67  C67 -C27  C27 -C67 
	    */
	    t = coeff[10];	/* C25 */
	    c1 += t;
	    c2 -= t;
	    t = coeff[14];	/* C56 */
	    c0 -= t;
	    c3 += t;
	    break;

	  case 63:
	    /*
	      Coefficient[7][7]

	       C77 -C57  C37 -C17  C17 -C37  C57 -C77 
	      -C57  C55 -C35  C15 -C15  C35 -C55  C57 
	       C37 -C35  C33 -C13  C13 -C33  C35 -C37 
	      -C17  C15 -C13  C11 -C11  C13 -C15  C17 
	       C17 -C15  C13 -C11  C11 -C13  C15 -C17 
	      -C37  C35 -C33  C13 -C13  C33 -C35  C37 
	       C57 -C55  C35 -C15  C15 -C35  C55 -C57 
	      -C77  C57 -C37  C17 -C17  C37 -C57  C77 
	    */
	    t = coeff[22];	/* C15 */
	    d3 += t;    
	    t = coeff[25];	/* C35 */
	    d2 -= t;    
	    t = coeff[27];	/* C55 */
	    d1 += t;    
	    t = coeff[28];	/* C57 */
	    d0 -= t;    
	    break;
	}
	
	i += 1;
	t = coefflist[i];

    } while (t != 0);

    t = a0 + b0;
    i = a0 - b0;
    a0 = c0 + d0;
    b0 = c0 - d0;
    c0 = t - a0;
    a0 = t + a0;
    d0 = i - b0;
    b0 = i + b0;
    a0 = ((unsigned int) a0) >> 16;
    b0 = ((unsigned int) b0) >> 16;
    c0 = ((unsigned int) c0) >> 16;
    d0 = ((unsigned int) d0) >> 16;

    t = a1 + b1;
    i = a1 - b1;
    a1 = c1 + d1;
    b1 = c1 - d1;
    c1 = t - a1;
    a1 = t + a1;
    d1 = i - b1;
    b1 = i + b1;
    a1 = ((unsigned int) a1) >> 16;
    b1 = ((unsigned int) b1) >> 16;
    c1 = ((unsigned int) c1) >> 16;
    d1 = ((unsigned int) d1) >> 16;

    t = a2 + b2;
    i = a2 - b2;
    a2 = c2 + d2;
    b2 = c2 - d2;
    c2 = t - a2;
    a2 = t + a2;
    d2 = i - b2;
    b2 = i + b2;
    a2 = ((unsigned int) a2) >> 16;
    b2 = ((unsigned int) b2) >> 16;
    c2 = ((unsigned int) c2) >> 16;
    d2 = ((unsigned int) d2) >> 16;

    t = a3 + b3;
    i = a3 - b3;
    a3 = c3 + d3;
    b3 = c3 - d3;
    c3 = t - a3;
    a3 = t + a3;
    d3 = i - b3;
    b3 = i + b3;
    a3 = ((unsigned int) a3) >> 16;
    b3 = ((unsigned int) b3) >> 16;
    c3 = ((unsigned int) c3) >> 16;
    d3 = ((unsigned int) d3) >> 16;

    a0 = PUT_SHORTS_INTO_INT(a0,a1);
    result[4] = a0;
    a2 = PUT_SHORTS_INTO_INT(a2,a3);
    result[5] = a2;
    b2 = PUT_SHORTS_INTO_INT(b3,b2);
    result[6] = b2;
    b0 = PUT_SHORTS_INTO_INT(b1,b0);
    result[7] = b0;
    c0 = PUT_SHORTS_INTO_INT(c0,c1);
    result[24] = c0;
    c2 = PUT_SHORTS_INTO_INT(c2,c3);
    result[25] = c2;
    d2 = PUT_SHORTS_INTO_INT(d3,d2);
    result[26] = d2;
    d0 = PUT_SHORTS_INTO_INT(d1,d0);
    result[27] = d0;


    i = 0;
    a0 = a1 = a2 = a3 = HALF;
    b0 = b1 = b2 = b3 = 0;
    c0 = c1 = c2 = c3 = 0;
    d0 = d1 = d2 = d3 = 0;

    t = coefflist[0];
    do {
	coeff = (int *)(t & ~63);
	t = t & 63;

	switch (t) {
	  case 0:
	    /*
	      Coefficient[0][0]

	      C44  C44  C44  C44  C44  C44  C44  C44 
	      C44  C44  C44  C44  C44  C44  C44  C44 
	      C44  C44  C44  C44  C44  C44  C44  C44 
	      C44  C44  C44  C44  C44  C44  C44  C44 
	      C44  C44  C44  C44  C44  C44  C44  C44 
	      C44  C44  C44  C44  C44  C44  C44  C44 
	      C44  C44  C44  C44  C44  C44  C44  C44 
	      C44  C44  C44  C44  C44  C44  C44  C44 
	    */
	    t = coeff[0];	/* C44 */
	    a0 += t;
	    a1 += t;
	    a2 += t;
	    a3 += t;    
	    break;


	  case 1:
	    /*
	      Coefficient[0][1]

	       C14  C34  C45  C47 -C47 -C45 -C34 -C14 
	       C14  C34  C45  C47 -C47 -C45 -C34 -C14 
	       C14  C34  C45  C47 -C47 -C45 -C34 -C14 
	       C14  C34  C45  C47 -C47 -C45 -C34 -C14 
	       C14  C34  C45  C47 -C47 -C45 -C34 -C14 
	       C14  C34  C45  C47 -C47 -C45 -C34 -C14 
	       C14  C34  C45  C47 -C47 -C45 -C34 -C14 
	       C14  C34  C45  C47 -C47 -C45 -C34 -C14 
	    */
	    t = coeff[4];	/* C14 */
	    b0 += t;    
	    t = coeff[5];	/* C34 */
	    b1 += t;    
	    t = coeff[6];	/* C45 */
	    b2 += t;    
	    t = coeff[7];	/* C47 */
	    b3 += t;
	    break;

	  case 2:
	    /*
	      Coefficient[1][0]

	       C14  C14  C14  C14  C14  C14  C14  C14 
	       C34  C34  C34  C34  C34  C34  C34  C34 
	       C45  C45  C45  C45  C45  C45  C45  C45 
	       C47  C47  C47  C47  C47  C47  C47  C47 
	      -C47 -C47 -C47 -C47 -C47 -C47 -C47 -C47 
	      -C45 -C45 -C45 -C45 -C45 -C45 -C45 -C45 
	      -C34 -C34 -C34 -C34 -C34 -C34 -C34 -C34
	      -C14 -C14 -C14 -C14 -C14 -C14 -C14 -C14 
	    */
	    t = coeff[6];	/* C45 */
	    c0 += t;  
	    c1 += t;
	    c2 += t;
	    c3 += t;    
	    break;

	  case 3:
	    /*
	      Coefficient[2][0]

	       C24  C24  C24  C24  C24  C24  C24  C24 
	       C46  C46  C46  C46  C46  C46  C46  C46 
	      -C46 -C46 -C46 -C46 -C46 -C46 -C46 -C46 
	      -C24 -C24 -C24 -C24 -C24 -C24 -C24 -C24 
	      -C24 -C24 -C24 -C24 -C24 -C24 -C24 -C24 
	      -C46 -C46 -C46 -C46 -C46 -C46 -C46 -C46 
	       C46  C46  C46  C46  C46  C46  C46  C46 
	       C24  C24  C24  C24  C24  C24  C24  C24 
	    */
	    t = coeff[3];	/* C46 */
	    a0 -= t;
	    a1 -= t;
	    a2 -= t;
	    a3 -= t;
	    break;


	  case 4:
	    /*
	      Coefficient[1][1]

	       C11  C13  C15  C17 -C17 -C15 -C13 -C11 
	       C13  C33  C35  C37 -C37 -C35 -C33 -C13 
	       C15  C35  C55  C57 -C57 -C55 -C35 -C15 
	       C17  C37  C57  C77 -C77 -C57 -C37 -C17 
	      -C17 -C37 -C57 -C77  C77  C57  C37  C17 
	      -C15 -C35 -C55 -C57  C57  C55  C35  C15 
	      -C13 -C33 -C35 -C37  C37  C35  C33  C13 
	      -C11 -C13 -C15 -C17  C17  C15  C13  C11 
	    */
	    t = coeff[22];	/* C15 */
	    d0 += t;    
	    t = coeff[25];	/* C35 */
	    d1 += t;    
	    t = coeff[27];	/* C55 */
	    d2 += t;    
	    t = coeff[28];	/* C57 */
	    d3 += t;    
	    break;

	  case 5:
	    /*
	      Coefficient[0][2]

	       C24  C46 -C46 -C24 -C24 -C46  C46  C24 
	       C24  C46 -C46 -C24 -C24 -C46  C46  C24 
	       C24  C46 -C46 -C24 -C24 -C46  C46  C24 
	       C24  C46 -C46 -C24 -C24 -C46  C46  C24 
	       C24  C46 -C46 -C24 -C24 -C46  C46  C24 
	       C24  C46 -C46 -C24 -C24 -C46  C46  C24 
	       C24  C46 -C46 -C24 -C24 -C46  C46  C24 
	       C24  C46 -C46 -C24 -C24 -C46  C46  C24 
	    */
	    t = coeff[2];	/* C24 */
	    a0 += t;
	    a3 -= t;    
	    t = coeff[3];	/* C46 */
	    a1 += t;
	    a2 -= t;    
	    break;

	  case 6:
	    /*
	      Coefficient[0][3]

	       C34 -C47 -C14 -C45  C45  C14  C47 -C34 
	       C34 -C47 -C14 -C45  C45  C14  C47 -C34 
	       C34 -C47 -C14 -C45  C45  C14  C47 -C34 
	       C34 -C47 -C14 -C45  C45  C14  C47 -C34 
	       C34 -C47 -C14 -C45  C45  C14  C47 -C34 
	       C34 -C47 -C14 -C45  C45  C14  C47 -C34 
	       C34 -C47 -C14 -C45  C45  C14  C47 -C34 
	       C34 -C47 -C14 -C45  C45  C14  C47 -C34 
	    */
	    t = coeff[4];	/* C14 */
	    b2 -= t;    
	    t = coeff[5];	/* C34 */
	    b0 += t;    
	    t = coeff[6];	/* C45 */
	    b3 -= t;    
	    t = coeff[7];	/* C47 */
	    b1 -= t;    
	    break;

	  case 7:
	    /*
	      Coefficient[1][2]

	       C12  C16 -C16 -C12 -C12 -C16  C16  C12 
	       C23  C36 -C36 -C23 -C23 -C36  C36  C23 
	       C25  C56 -C56 -C25 -C25 -C56  C56  C25 
	       C27  C67 -C67 -C27 -C27 -C67  C67  C27 
	      -C27 -C67  C67  C27  C27  C67 -C67 -C27 
	      -C25 -C56  C56  C25  C25  C56 -C56 -C25 
	      -C23 -C36  C36  C23  C23  C36 -C36 -C23 
	      -C12 -C16  C16  C12  C12  C16 -C16 -C12 
	    */
	    t = coeff[10];	/* C25 */
	    c0 += t;
	    c3 -= t; 
	    t = coeff[14];	/* C56 */
	    c1 += t;
	    c2 -= t;
	    break;

	  case 8:
	    /*
	      Coefficient[2][1]

	       C12  C23  C25  C27 -C27 -C25 -C23 -C12 
	       C16  C36  C56  C67 -C67 -C56 -C36 -C16 
	      -C16 -C36 -C56 -C67  C67  C56  C36  C16 
	      -C12 -C23 -C25 -C27  C27  C25  C23  C12 
	      -C12 -C23 -C25 -C27  C27  C25  C23  C12 
	      -C16 -C36 -C56 -C67  C67  C56  C36  C16 
	       C16  C36  C56  C67 -C67 -C56 -C36 -C16 
	       C12  C23  C25  C27 -C27 -C25 -C23 -C12 
	    */
	    t = coeff[12];	/* C16 */
	    b0 -= t;    
	    t = coeff[13];	/* C36 */
	    b1 -= t;    
	    t = coeff[14];	/* C56 */
	    b2 -= t;    
	    t = coeff[15];	/* C67 */
	    b3 -= t;    
	    break;

	  case 9:
	    /*
	      Coefficient[3][0]

	       C34  C34  C34  C34  C34  C34  C34  C34 
	      -C47 -C47 -C47 -C47 -C47 -C47 -C47 -C47 
	      -C14 -C14 -C14 -C14 -C14 -C14 -C14 -C14 
	      -C45 -C45 -C45 -C45 -C45 -C45 -C45 -C45 
	       C45  C45  C45  C45  C45  C45  C45  C45 
	       C14  C14  C14  C14  C14  C14  C14  C14 
	       C47  C47  C47  C47  C47  C47  C47  C47 
	      -C34 -C34 -C34 -C34 -C34 -C34 -C34 -C34 
	    */
	    t = coeff[4];	/* C14 */
	    c0 -= t;
	    c1 -= t;
	    c2 -= t;
	    c3 -= t;    
	    break;

	  case 10:
	    /*
	      Coefficient[4][0]
	      
	       C44  C44  C44  C44  C44  C44  C44  C44 
	      -C44 -C44 -C44 -C44 -C44 -C44 -C44 -C44 
	      -C44 -C44 -C44 -C44 -C44 -C44 -C44 -C44 
	       C44  C44  C44  C44  C44  C44  C44  C44 
	       C44  C44  C44  C44  C44  C44  C44  C44 
	      -C44 -C44 -C44 -C44 -C44 -C44 -C44 -C44 
	      -C44 -C44 -C44 -C44 -C44 -C44 -C44 -C44 
	       C44  C44  C44  C44  C44  C44  C44  C44 
	    */
	    t = coeff[0];	/* C44 */
	    a0 -= t;
	    a1 -= t;
	    a2 -= t;
	    a3 -= t;    
	    break;

	  case 11:
	    /*
	      Coefficient[3][1]

	       C13  C33  C35  C37 -C37 -C35 -C33 -C13 
	      -C17 -C37 -C57 -C77  C77  C57  C37  C17 
	      -C11 -C13 -C15 -C17  C17  C15  C13  C11 
	      -C15 -C35 -C55 -C57  C57  C55  C35  C15 
	       C15  C35  C55  C57 -C57 -C55 -C35 -C15 
	       C11  C13  C15  C17 -C17 -C15 -C13 -C11 
	       C17  C37  C57  C77 -C77 -C57 -C37 -C17 
	      -C13 -C33 -C35 -C37  C37  C35  C33  C13 
	   */
	    t = coeff[20];	/* C11 */
	    d0 -= t;    
	    t = coeff[21];	/* C13 */
	    d1 -= t;    
	    t = coeff[22];	/* C15 */
	    d2 -= t;    
	    t = coeff[23];	/* C17 */
	    d3 -= t;
	    break;

	  case 12:
	    /*
	      Coefficient[2][2]

	       C22  C26 -C26 -C22 -C22 -C26  C26  C22 
	       C26  C66 -C66 -C26 -C26 -C66  C66  C26 
	      -C26 -C66  C66  C26  C26  C66 -C66 -C26 
	      -C22 -C26  C26  C22  C22  C26 -C26 -C22 
	      -C22 -C26  C26  C22  C22  C26 -C26 -C22 
	      -C26 -C66  C66  C26  C26  C66 -C66 -C26 
	       C26  C66 -C66 -C26 -C26 -C66  C66  C26 
	       C22  C26 -C26 -C22 -C22 -C26  C26  C22 
	    */
	    t = coeff[18];	/* C26 */
	    a0 -= t;
	    a3 += t;    
	    t = coeff[19];	/* C66 */
	    a1 -= t;
	    a2 += t;    
	    break;

	  case 13:
	    /*
	      Coefficient[1][3]

	       C13 -C17 -C11 -C15  C15  C11  C17 -C13 
	       C33 -C37 -C13 -C35  C35  C13  C37 -C33 
	       C35 -C57 -C15 -C55  C55  C15  C57 -C35 
	       C37 -C77 -C17 -C57  C57  C17  C77 -C37 
	      -C37  C77  C17  C57 -C57 -C17 -C77  C37 
	      -C35  C57  C15  C55 -C55 -C15 -C57  C35 
	      -C33  C37  C13  C35 -C35 -C13 -C37  C33 
	      -C13  C17  C11  C15 -C15 -C11 -C17  C13 
	    */
	    t = coeff[22];	/* C15 */
	    d2 -= t;    
	    t = coeff[25];	/* C35 */
	    d0 += t;    
	    t = coeff[27];	/* C55 */
	    d3 -= t;    
	    t = coeff[28];	/* C57 */
	    d1 -= t;    
	    break;

	  case 14:
	    /*
	      Coefficient[0][4]

	       C44 -C44 -C44  C44  C44 -C44 -C44  C44 
 	       C44 -C44 -C44  C44  C44 -C44 -C44  C44 
	       C44 -C44 -C44  C44  C44 -C44 -C44  C44 
	       C44 -C44 -C44  C44  C44 -C44 -C44  C44 
	       C44 -C44 -C44  C44  C44 -C44 -C44  C44 
	       C44 -C44 -C44  C44  C44 -C44 -C44  C44 
	       C44 -C44 -C44  C44  C44 -C44 -C44  C44 
	       C44 -C44 -C44  C44  C44 -C44 -C44  C44 
	    */
	    t = coeff[0];	/* C44 */
	    a0 += t;
	    a1 -= t;
	    a2 -= t;
	    a3 += t;    
	    break;

	  case 15:
	    /*
	      Coefficient[0][5]

	       C45 -C14  C47  C34 -C34 -C47  C14 -C45 
	       C45 -C14  C47  C34 -C34 -C47  C14 -C45 
	       C45 -C14  C47  C34 -C34 -C47  C14 -C45 
	       C45 -C14  C47  C34 -C34 -C47  C14 -C45 
	       C45 -C14  C47  C34 -C34 -C47  C14 -C45 
	       C45 -C14  C47  C34 -C34 -C47  C14 -C45 
	       C45 -C14  C47  C34 -C34 -C47  C14 -C45 
	       C45 -C14  C47  C34 -C34 -C47  C14 -C45 
	    */
	    t = coeff[4];	/* C14 */
	    b1 -= t;    
	    t = coeff[5];	/* C34 */
	    b3 += t;    
	    t = coeff[6];	/* C45 */
	    b0 += t;    
	    t = coeff[7];	/* C47 */
	    b2 += t;    
	    break;

	  case 16:
	    /*
	      Coefficient[1][4]

	       C14 -C14 -C14  C14  C14 -C14 -C14  C14 
	       C34 -C34 -C34  C34  C34 -C34 -C34  C34 
	       C45 -C45 -C45  C45  C45 -C45 -C45  C45 
	       C47 -C47 -C47  C47  C47 -C47 -C47  C47 
	      -C47  C47  C47 -C47 -C47  C47  C47 -C47 
	      -C45  C45  C45 -C45 -C45  C45  C45 -C45 
	      -C34  C34  C34 -C34 -C34  C34  C34 -C34 
	      -C14  C14  C14 -C14 -C14  C14  C14 -C14 
	    */
	    t = coeff[6];	/* C45 */
	    c0 += t;
	    c1 -= t;
	    c2 -= t;
	    c3 += t;
	    break;

	  case 17:
	    /*
	      Coefficient[2][3]

	       C23 -C27 -C12 -C25  C25  C12  C27 -C23 
	       C36 -C67 -C16 -C56  C56  C16  C67 -C36 
	      -C36  C67  C16  C56 -C56 -C16 -C67  C36 
	      -C23  C27  C12  C25 -C25 -C12 -C27  C23 
	      -C23  C27  C12  C25 -C25 -C12 -C27  C23 
	      -C36  C67  C16  C56 -C56 -C16 -C67  C36 
	       C36 -C67 -C16 -C56  C56  C16  C67 -C36 
	       C23 -C27 -C12 -C25  C25  C12  C27 -C23 
	    */
	    t = coeff[12];	/* C16 */
	    b2 += t;    
	    t = coeff[13];	/* C36 */
	    b0 -= t;    
	    t = coeff[14];	/* C56 */
	    b3 += t;    
	    t = coeff[15];	/* C67 */
	    b1 += t;
	    break;

	  case 18:
	    /*
	      Coefficient[3][2]

 	       C23  C36 -C36 -C23 -C23 -C36  C36  C23 
	      -C27 -C67  C67  C27  C27  C67 -C67 -C27 
	      -C12 -C16  C16  C12  C12  C16 -C16 -C12 
	      -C25 -C56  C56  C25  C25  C56 -C56 -C25 
	       C25  C56 -C56 -C25 -C25 -C56  C56  C25 
	       C12  C16 -C16 -C12 -C12 -C16  C16  C12 
	       C27  C67 -C67 -C27 -C27 -C67  C67  C27 
	      -C23 -C36  C36  C23  C23  C36 -C36 -C23 
	    */
	    t = coeff[8];	/* C12 */
	    c0 -= t;
	    c3 += t;
	    t = coeff[12];	/* C16 */
	    c1 -= t;
	    c2 += t;
	    break;

	  case 19:
	    /*
	      Coefficient[4][1]

	       C14  C34  C45  C47 -C47 -C45 -C34 -C14 
	      -C14 -C34 -C45 -C47  C47  C45  C34  C14 
	      -C14 -C34 -C45 -C47  C47  C45  C34  C14 
	       C14  C34  C45  C47 -C47 -C45 -C34 -C14 
	       C14  C34  C45  C47 -C47 -C45 -C34 -C14 
	      -C14 -C34 -C45 -C47  C47  C45  C34  C14 
	      -C14 -C34 -C45 -C47  C47  C45  C34  C14 
	       C14  C34  C45  C47 -C47 -C45 -C34 -C14 
	    */
	    t = coeff[4];	/* C14 */
	    b0 -= t;    
	    t = coeff[5];	/* C34 */
	    b1 -= t;    
	    t = coeff[6];	/* C45 */
	    b2 -= t;    
	    t = coeff[7];	/* C47 */
	    b3 -= t;    
	    break;

	  case 20:
	    /*
	      Coefficient[5][0]

	       C45  C45  C45  C45  C45  C45  C45  C45 
	      -C14 -C14 -C14 -C14 -C14 -C14 -C14 -C14 
	       C47  C47  C47  C47  C47  C47  C47  C47 
	       C34  C34  C34  C34  C34  C34  C34  C34 
	      -C34 -C34 -C34 -C34 -C34 -C34 -C34 -C34 
	      -C47 -C47 -C47 -C47 -C47 -C47 -C47 -C47 
	       C14  C14  C14  C14  C14  C14  C14  C14 
	      -C45 -C45 -C45 -C45 -C45 -C45 -C45 -C45 
	    */
	    t = coeff[7];	/* C47 */
	    c0 += t;
	    c1 += t;
	    c2 += t;
	    c3 += t;
	    break;
	    
	  case 21:
	    /*
	      Coefficient[6][0]

	       C46  C46  C46  C46  C46  C46  C46  C46 
	      -C24 -C24 -C24 -C24 -C24 -C24 -C24 -C24 
	       C24  C24  C24  C24  C24  C24  C24  C24 
	      -C46 -C46 -C46 -C46 -C46 -C46 -C46 -C46 
	      -C46 -C46 -C46 -C46 -C46 -C46 -C46 -C46 
	       C24  C24  C24  C24  C24  C24  C24  C24 
	      -C24 -C24 -C24 -C24 -C24 -C24 -C24 -C24 
	       C46  C46  C46  C46  C46  C46  C46  C46 
	    */
	    t = coeff[2];	/* C24 */
	    a0 += t;
	    a1 += t;
	    a2 += t;
	    a3 += t;    
	    break;

	  case 22:
	    /*
	      Coefficient[5][1]

	       C15  C35  C55  C57 -C57 -C55 -C35 -C15 
	      -C11 -C13 -C15 -C17  C17  C15  C13  C11 
	       C17  C37  C57  C77 -C77 -C57 -C37 -C17 
	       C13  C33  C35  C37 -C37 -C35 -C33 -C13 
	      -C13 -C33 -C35 -C37  C37  C35  C33  C13 
	      -C17 -C37 -C57 -C77  C77  C57  C37  C17 
	       C11  C13  C15  C17 -C17 -C15 -C13 -C11 
	      -C15 -C35 -C55 -C57  C57  C55  C35  C15 
	    */
	    t = coeff[23];	/* C17 */
	    d0 += t;    
	    t = coeff[26];	/* C37 */
	    d1 += t;    
	    t = coeff[28];	/* C57 */
	    d2 += t;    
	    t = coeff[29];	/* C77 */
	    d3 += t;    
	    break;

	  case 23:
	    /*
	      Coefficient[4][2]

	       C24  C46 -C46 -C24 -C24 -C46  C46  C24 
	      -C24 -C46  C46  C24  C24  C46 -C46 -C24 
	      -C24 -C46  C46  C24  C24  C46 -C46 -C24 
	       C24  C46 -C46 -C24 -C24 -C46  C46  C24 
	       C24  C46 -C46 -C24 -C24 -C46  C46  C24 
	      -C24 -C46  C46  C24  C24  C46 -C46 -C24 
	      -C24 -C46  C46  C24  C24  C46 -C46 -C24 
	       C24  C46 -C46 -C24 -C24 -C46  C46  C24 
	    */
	    t = coeff[2];	/* C24 */
	    a0 -= t;
	    a3 += t;    
	    t = coeff[3];	/* C46 */
	    a1 -= t;
	    a2 += t;    
	    break;

	  case 24:
	    /*
	      Coefficient[3][3]

	       C33 -C37 -C13 -C35  C35  C13  C37 -C33 
	      -C37  C77  C17  C57 -C57 -C17 -C77  C37 
	      -C13  C17  C11  C15 -C15 -C11 -C17  C13 
	      -C35  C57  C15  C55 -C55 -C15 -C57  C35 
	       C35 -C57 -C15 -C55  C55  C15  C57 -C35 
	       C13 -C17 -C11 -C15  C15  C11  C17 -C13 
	       C37 -C77 -C17 -C57  C57  C17  C77 -C37 
	      -C33  C37  C13  C35 -C35 -C13 -C37  C33 
	    */
	    t = coeff[20];	/* C11 */
	    d2 += t;    
	    t = coeff[21];	/* C13 */
	    d0 -= t;    
	    t = coeff[22];	/* C15 */
	    d3 += t;    
	    t = coeff[23];	/* C17 */
	    d1 += t;    
	    break;

	  case 25:
	    /*
	      Coefficient[2][4]

	       C24 -C24 -C24  C24  C24 -C24 -C24  C24 
	       C46 -C46 -C46  C46  C46 -C46 -C46  C46 
	      -C46  C46  C46 -C46 -C46  C46  C46 -C46 
	      -C24  C24  C24 -C24 -C24  C24  C24 -C24 
	      -C24  C24  C24 -C24 -C24  C24  C24 -C24 
	      -C46  C46  C46 -C46 -C46  C46  C46 -C46 
	       C46 -C46 -C46  C46  C46 -C46 -C46  C46 
	       C24 -C24 -C24  C24  C24 -C24 -C24  C24 
	    */
	    t = coeff[3];	/* C46 */
	    a0 -= t;
	    a1 += t;
	    a2 += t;
	    a3 -= t;
	    break;

	  case 26:
	    /*
	      Coefficient[1][5]

	       C15 -C11  C17  C13 -C13 -C17  C11 -C15 
	       C35 -C13  C37  C33 -C33 -C37  C13 -C35 
	       C55 -C15  C57  C35 -C35 -C57  C15 -C55 
	       C57 -C17  C77  C37 -C37 -C77  C17 -C57 
	      -C57  C17 -C77 -C37  C37  C77 -C17  C57 
	      -C55  C15 -C57 -C35  C35  C57 -C15  C55 
	      -C35  C13 -C37 -C33  C33  C37 -C13  C35 
	      -C15  C11 -C17 -C13  C13  C17 -C11  C15 
	    */
	    t = coeff[22];	/* C15 */
	    d1 -= t;    
	    t = coeff[25];	/* C35 */
	    d3 += t;    
	    t = coeff[27];	/* C55 */
	    d0 += t;    
	    t = coeff[28];	/* C57 */
	    d2 += t;    
	    break;

	  case 27:
	    /*
	      Coefficient[0][6]

	       C46 -C24  C24 -C46 -C46  C24 -C24  C46 
	       C46 -C24  C24 -C46 -C46  C24 -C24  C46 
	       C46 -C24  C24 -C46 -C46  C24 -C24  C46 
	       C46 -C24  C24 -C46 -C46  C24 -C24  C46 
	       C46 -C24  C24 -C46 -C46  C24 -C24  C46 
	       C46 -C24  C24 -C46 -C46  C24 -C24  C46 
	       C46 -C24  C24 -C46 -C46  C24 -C24  C46 
	       C46 -C24  C24 -C46 -C46  C24 -C24  C46 
	    */
	    t = coeff[2];	/* C24 */
	    a1 -= t;
	    a2 += t;
	    t = coeff[3];	/* C46 */
	    a0 += t;
	    a3 -= t;
	    break;

	  case 28:
	    /*
	      Coefficient[0][7]

	       C47 -C45  C34 -C14  C14 -C34  C45 -C47 
	       C47 -C45  C34 -C14  C14 -C34  C45 -C47 
	       C47 -C45  C34 -C14  C14 -C34  C45 -C47 
	       C47 -C45  C34 -C14  C14 -C34  C45 -C47 
	       C47 -C45  C34 -C14  C14 -C34  C45 -C47 
	       C47 -C45  C34 -C14  C14 -C34  C45 -C47 
	       C47 -C45  C34 -C14  C14 -C34  C45 -C47 
	       C47 -C45  C34 -C14  C14 -C34  C45 -C47 
	    */
	    t = coeff[4];	/* C14 */
	    b3 -= t;    
	    t = coeff[5];	/* C34 */
	    b2 += t;    
	    t = coeff[6];	/* C45 */
	    b1 -= t;    
	    t = coeff[7];	/* C47 */
	    b0 += t;    
	    break;

	  case 29:
	    /*
	      Coefficient[1][6]

	       C16 -C12  C12 -C16 -C16  C12 -C12  C16 
	       C36 -C23  C23 -C36 -C36  C23 -C23  C36 
	       C56 -C25  C25 -C56 -C56  C25 -C25  C56 
	       C67 -C27  C27 -C67 -C67  C27 -C27  C67 
	      -C67  C27 -C27  C67  C67 -C27  C27 -C67 
	      -C56  C25 -C25  C56  C56 -C25  C25 -C56 
	      -C36  C23 -C23  C36  C36 -C23  C23 -C36 
	      -C16  C12 -C12  C16  C16 -C12  C12 -C16 
	    */
	    t = coeff[10];	/* C25 */
	    c1 -= t;
	    c2 += t;
	    t = coeff[14];	/* C56 */
	    c0 += t;
	    c3 -= t;
	    break;

	  case 30:
	    /*
	      Coefficient[2][5]

	       C25 -C12  C27  C23 -C23 -C27  C12 -C25 
	       C56 -C16  C67  C36 -C36 -C67  C16 -C56 
	      -C56  C16 -C67 -C36  C36  C67 -C16  C56 
	      -C25  C12 -C27 -C23  C23  C27 -C12  C25 
	      -C25  C12 -C27 -C23  C23  C27 -C12  C25 
	      -C56  C16 -C67 -C36  C36  C67 -C16  C56 
	       C56 -C16  C67  C36 -C36 -C67  C16 -C56 
	       C25 -C12  C27  C23 -C23 -C27  C12 -C25 
	    */
	    t = coeff[12];	/* C16 */
	    b1 += t;    
	    t = coeff[13];	/* C36 */
	    b3 -= t;    
	    t = coeff[14];	/* C56 */
	    b0 -= t;    
	    t = coeff[15];	/* C67 */
	    b2 -= t;    
	    break;

	  case 31:
	    /*
	      Coefficient[3][4]

	       C34 -C34 -C34  C34  C34 -C34 -C34  C34 
	      -C47  C47  C47 -C47 -C47  C47  C47 -C47 
	      -C14  C14  C14 -C14 -C14  C14  C14 -C14 
	      -C45  C45  C45 -C45 -C45  C45  C45 -C45 
	       C45 -C45 -C45  C45  C45 -C45 -C45  C45 
	       C14 -C14 -C14  C14  C14 -C14 -C14  C14 
	       C47 -C47 -C47  C47  C47 -C47 -C47  C47 
	      -C34  C34  C34 -C34 -C34  C34  C34 -C34 
	    */
	    t = coeff[4];	/* C14 */
	    c0 -= t;
	    c1 += t;
	    c2 += t;
	    c3 -= t;
	    break;

	  case 32:
	    /*
	      Coefficient[4][3]

	       C34 -C47 -C14 -C45  C45  C14  C47 -C34 
	      -C34  C47  C14  C45 -C45 -C14 -C47  C34 
	      -C34  C47  C14  C45 -C45 -C14 -C47  C34 
	       C34 -C47 -C14 -C45  C45  C14  C47 -C34 
	       C34 -C47 -C14 -C45  C45  C14  C47 -C34 
	      -C34  C47  C14  C45 -C45 -C14 -C47  C34 
	      -C34  C47  C14  C45 -C45 -C14 -C47  C34 
	       C34 -C47 -C14 -C45  C45  C14  C47 -C34 
	    */
	    t = coeff[4];	/* C14 */
	    b2 += t;    
	    t = coeff[5];	/* C34 */
	    b0 -= t;    
	    t = coeff[6];	/* C45 */
	    b3 += t;    
	    t = coeff[7];	/* C47 */
	    b1 += t;    
	    break;

	  case 33:
	    /*
	      Coefficient[5][2]

	       C25  C56 -C56 -C25 -C25 -C56  C56  C25 
	      -C12 -C16  C16  C12  C12  C16 -C16 -C12 
	       C27  C67 -C67 -C27 -C27 -C67  C67  C27 
	       C23  C36 -C36 -C23 -C23 -C36  C36  C23 
	      -C23 -C36  C36  C23  C23  C36 -C36 -C23 
	      -C27 -C67  C67  C27  C27  C67 -C67 -C27 
	       C12  C16 -C16 -C12 -C12 -C16  C16  C12 
	      -C25 -C56  C56  C25  C25  C56 -C56 -C25 
	    */
	    t = coeff[11];	/* C27 */
	    c0 += t;
	    c3 -= t;
	    t = coeff[15];	/* C67 */
	    c1 += t;
	    c2 -= t;
	    break;

	  case 34:
	    /*
	      Coefficient[6][1]

	       C16  C36  C56  C67 -C67 -C56 -C36 -C16 
	      -C12 -C23 -C25 -C27  C27  C25  C23  C12 
	       C12  C23  C25  C27 -C27 -C25 -C23 -C12 
	      -C16 -C36 -C56 -C67  C67  C56  C36  C16 
	      -C16 -C36 -C56 -C67  C67  C56  C36  C16 
	       C12  C23  C25  C27 -C27 -C25 -C23 -C12 
	      -C12 -C23 -C25 -C27  C27  C25  C23  C12 
	       C16  C36  C56  C67 -C67 -C56 -C36 -C16 
	    */
	    t = coeff[8];	/* C12 */
	    b0 += t;    
	    t = coeff[9];	/* C23 */
	    b1 += t;    
	    t = coeff[10];	/* C25 */
	    b2 += t;    
	    t = coeff[11];	/* C27 */
	    b3 += t;    
	    break;

	  case 35:
	    /*
	      Coefficient[7][0]

	       C47  C47  C47  C47  C47  C47  C47  C47 
	      -C45 -C45 -C45 -C45 -C45 -C45 -C45 -C45 
	       C34  C34  C34  C34  C34  C34  C34  C34 
	      -C14 -C14 -C14 -C14 -C14 -C14 -C14 -C14 
	       C14  C14  C14  C14  C14  C14  C14  C14 
	      -C34 -C34 -C34 -C34 -C34 -C34 -C34 -C34 
	       C45  C45  C45  C45  C45  C45  C45  C45 
	      -C47 -C47 -C47 -C47 -C47 -C47 -C47 -C47 
	    */
	    t = coeff[5];	/* C34 */
	    c0 += t;
	    c1 += t;
	    c2 += t;
	    c3 += t;    
	    break;

	  case 36:
	    /*
	      Coefficient[7][1]

	       C17  C37  C57  C77 -C77 -C57 -C37 -C17 
	      -C15 -C35 -C55 -C57  C57  C55  C35  C15 
	       C13  C33  C35  C37 -C37 -C35 -C33 -C13 
	      -C11 -C13 -C15 -C17  C17  C15  C13  C11 
	       C11  C13  C15  C17 -C17 -C15 -C13 -C11 
	      -C13 -C33 -C35 -C37  C37  C35  C33  C13 
	       C15  C35  C55  C57 -C57 -C55 -C35 -C15 
	      -C17 -C37 -C57 -C77  C77  C57  C37  C17 
	    */
	    t = coeff[21];	/* C13 */
	    d0 += t;    
	    t = coeff[24];	/* C33 */
	    d1 += t;    
	    t = coeff[25];	/* C35 */
	    d2 += t;    
	    t = coeff[26];	/* C37 */
	    d3 += t;    
	    break;

	  case 37:
	    /*
	      Coefficient[6][2]

	       C26  C66 -C66 -C26 -C26 -C66  C66  C26 
	      -C22 -C26  C26  C22  C22  C26 -C26 -C22 
	       C22  C26 -C26 -C22 -C22 -C26  C26  C22 
	      -C26 -C66  C66  C26  C26  C66 -C66 -C26 
	      -C26 -C66  C66  C26  C26  C66 -C66 -C26 
	       C22  C26 -C26 -C22 -C22 -C26  C26  C22 
	      -C22 -C26  C26  C22  C22  C26 -C26 -C22 
	       C26  C66 -C66 -C26 -C26 -C66  C66  C26 
	    */
	    t = coeff[16];	/* C22 */
	    a0 += t;
	    a3 -= t;
	    t = coeff[17];	/* C26 */
	    a1 += t;
	    a2 -= t;    
	    break;

	  case 38:
	    /*
	      Coefficient[5][3]

	       C35 -C57 -C15 -C55  C55  C15  C57 -C35 
	      -C13  C17  C11  C15 -C15 -C11 -C17  C13 
	       C37 -C77 -C17 -C57  C57  C17  C77 -C37 
	       C33 -C37 -C13 -C35  C35  C13  C37 -C33 
	      -C33  C37  C13  C35 -C35 -C13 -C37  C33 
	      -C37  C77  C17  C57 -C57 -C17 -C77  C37 
	       C13 -C17 -C11 -C15  C15  C11  C17 -C13 
	      -C35  C57  C15  C55 -C55 -C15 -C57  C35 
	    */
	    t = coeff[23];	/* C17 */
	    d2 -= t;    
	    t = coeff[26];	/* C37 */
	    d0 += t;    
	    t = coeff[28];	/* C57 */
	    d3 -= t;    
	    t = coeff[29];	/* C77 */
	    d1 -= t;    
	    break;

	  case 39:
	    /*
	      Coefficient[4][4]

	       C44 -C44 -C44  C44  C44 -C44 -C44  C44 
	      -C44  C44  C44 -C44 -C44  C44  C44 -C44 
	      -C44  C44  C44 -C44 -C44  C44  C44 -C44 
	       C44 -C44 -C44  C44  C44 -C44 -C44  C44 
	       C44 -C44 -C44  C44  C44 -C44 -C44  C44 
	      -C44  C44  C44 -C44 -C44  C44  C44 -C44 
	      -C44  C44  C44 -C44 -C44  C44  C44 -C44 
	       C44 -C44 -C44  C44  C44 -C44 -C44  C44 
	    */
	    t = coeff[0];	/* C44 */
	    a0 -= t;
	    a1 += t;
	    a2 += t;
	    a3 -= t;
	    break;

	  case 40:
	    /*
	      Coefficient[3][5]

	       C35 -C13  C37  C33 -C33 -C37  C13 -C35 
	      -C57  C17 -C77 -C37  C37  C77 -C17  C57 
	      -C15  C11 -C17 -C13  C13  C17 -C11  C15 
	      -C55  C15 -C57 -C35  C35  C57 -C15  C55 
	       C55 -C15  C57  C35 -C35 -C57  C15 -C55 
	       C15 -C11  C17  C13 -C13 -C17  C11 -C15 
	       C57 -C17  C77  C37 -C37 -C77  C17 -C57 
	      -C35  C13 -C37 -C33  C33  C37 -C13  C35 
	    */
	    t = coeff[20];	/* C11 */
	    d1 += t;    
	    t = coeff[21];	/* C13 */
	    d3 -= t;    
	    t = coeff[22];	/* C15 */
	    d0 -= t;    
	    t = coeff[23];	/* C17 */
	    d2 -= t;    
	    break;

	  case 41:
	    /*
	      Coefficient[2][6]

	       C26 -C22  C22 -C26 -C26  C22 -C22  C26 
	       C66 -C26  C26 -C66 -C66  C26 -C26  C66 
	      -C66  C26 -C26  C66  C66 -C26  C26 -C66 
	      -C26  C22 -C22  C26  C26 -C22  C22 -C26 
	      -C26  C22 -C22  C26  C26 -C22  C22 -C26 
	      -C66  C26 -C26  C66  C66 -C26  C26 -C66 
	       C66 -C26  C26 -C66 -C66  C26 -C26  C66 
	       C26 -C22  C22 -C26 -C26  C22 -C22  C26 
	    */
	    t = coeff[18];	/* C26 */
	    a1 += t;
	    a2 -= t;
	    t = coeff[19];	/* C66 */
	    a0 -= t;
	    a3 += t;
	    break;

	  case 42:
	    /*
	       Coefficient[1][7]

	        C17 -C15  C13 -C11  C11 -C13  C15 -C17 
	        C37 -C35  C33 -C13  C13 -C33  C35 -C37 
	        C57 -C55  C35 -C15  C15 -C35  C55 -C57 
	        C77 -C57  C37 -C17  C17 -C37  C57 -C77 
	       -C77  C57 -C37  C17 -C17  C37 -C57  C77 
	       -C57  C55 -C35  C15 -C15  C35 -C55  C57 
	       -C37  C35 -C33  C13 -C13  C33 -C35  C37 
	       -C17  C15 -C13  C11 -C11  C13 -C15  C17 
	    */
	    t = coeff[22];	/* C15 */
	    d3 -= t;    
	    t = coeff[25];	/* C35 */
	    d2 += t;    
	    t = coeff[27];	/* C55 */
	    d1 -= t;    
	    t = coeff[28];	/* C57 */
	    d0 += t;    
	    break;

	  case 43:
	    /*
	      Coefficient[2][7]

	       C27 -C25  C23 -C12  C12 -C23  C25 -C27 
	       C67 -C56  C36 -C16  C16 -C36  C56 -C67 
	      -C67  C56 -C36  C16 -C16  C36 -C56  C67 
	      -C27  C25 -C23  C12 -C12  C23 -C25  C27 
	      -C27  C25 -C23  C12 -C12  C23 -C25  C27 
	      -C67  C56 -C36  C16 -C16  C36 -C56  C67 
	       C67 -C56  C36 -C16  C16 -C36  C56 -C67 
	       C27 -C25  C23 -C12  C12 -C23  C25 -C27 
	    */
	    t = coeff[12];	/* C16 */
	    b3 += t;    
	    t = coeff[13];	/* C36 */
	    b2 -= t;    
	    t = coeff[14];	/* C56 */
	    b1 += t;    
	    t = coeff[15];	/* C67 */
	    b0 -= t;    
	    break;

	  case 44:
	    /*
	      Coefficient[3][6]

	       C36 -C23  C23 -C36 -C36  C23 -C23  C36 
	      -C67  C27 -C27  C67  C67 -C27  C27 -C67 
	      -C16  C12 -C12  C16  C16 -C12  C12 -C16 
	      -C56  C25 -C25  C56  C56 -C25  C25 -C56 
	       C56 -C25  C25 -C56 -C56  C25 -C25  C56 
	       C16 -C12  C12 -C16 -C16  C12 -C12  C16 
	       C67 -C27  C27 -C67 -C67  C27 -C27  C67 
	      -C36  C23 -C23  C36  C36 -C23  C23 -C36 
	    */
	    t = coeff[8];	/* C12 */
	    c1 += t;
	    c2 -= t;
	    t = coeff[12];	/* C16 */
	    c0 -= t;
	    c3 += t;
	    break;

	  case 45:
	    /*
	      Coefficient[4][5]

	       C45 -C14  C47  C34 -C34 -C47  C14 -C45 
	      -C45  C14 -C47 -C34  C34  C47 -C14  C45 
	      -C45  C14 -C47 -C34  C34  C47 -C14  C45 
	       C45 -C14  C47  C34 -C34 -C47  C14 -C45 
	       C45 -C14  C47  C34 -C34 -C47  C14 -C45 
	      -C45  C14 -C47 -C34  C34  C47 -C14  C45 
	      -C45  C14 -C47 -C34  C34  C47 -C14  C45 
	       C45 -C14  C47  C34 -C34 -C47  C14 -C45 
	    */
	    t = coeff[4];	/* C14 */
	    b1 += t;    
	    t = coeff[5];	/* C34 */
	    b3 -= t;    
	    t = coeff[6];	/* C45 */
	    b0 -= t;    
	    t = coeff[7];	/* C47 */
	    b2 -= t;    
	    break;

	  case 46:
	    /*
	      Coefficient[5][4]

	       C45 -C45 -C45  C45  C45 -C45 -C45  C45 
	      -C14  C14  C14 -C14 -C14  C14  C14 -C14 
	       C47 -C47 -C47  C47  C47 -C47 -C47  C47 
	       C34 -C34 -C34  C34  C34 -C34 -C34  C34 
	      -C34  C34  C34 -C34 -C34  C34  C34 -C34 
	      -C47  C47  C47 -C47 -C47  C47  C47 -C47 
	       C14 -C14 -C14  C14  C14 -C14 -C14  C14 
	      -C45  C45  C45 -C45 -C45  C45  C45 -C45 
	    */
	    t = coeff[7];	/* C47 */
	    c0 += t;
	    c1 -= t;
	    c2 -= t;
	    c3 += t;
	    break;

	  case 47:
	    /*
	      Coefficient[6][3]

	       C36 -C67 -C16 -C56  C56  C16  C67 -C36 
	      -C23  C27  C12  C25 -C25 -C12 -C27  C23 
	       C23 -C27 -C12 -C25  C25  C12  C27 -C23 
	      -C36  C67  C16  C56 -C56 -C16 -C67  C36 
	      -C36  C67  C16  C56 -C56 -C16 -C67  C36 
	       C23 -C27 -C12 -C25  C25  C12  C27 -C23 
	      -C23  C27  C12  C25 -C25 -C12 -C27  C23 
	       C36 -C67 -C16 -C56  C56  C16  C67 -C36 
	    */
	    t = coeff[8];	/* C12 */
	    b2 -= t;    
	    t = coeff[9];	/* C23 */
	    b0 += t;    
	    t = coeff[10];	/* C25 */
	    b3 -= t;    
	    t = coeff[11];	/* C27 */
	    b1 -= t;    
	    break;

	  case 48:
	    /*
	      Coefficient[7][2]

	       C27  C67 -C67 -C27 -C27 -C67  C67  C27 
	      -C25 -C56  C56  C25  C25  C56 -C56 -C25 
	       C23  C36 -C36 -C23 -C23 -C36  C36  C23 
	      -C12 -C16  C16  C12  C12  C16 -C16 -C12 
	       C12  C16 -C16 -C12 -C12 -C16  C16  C12 
	      -C23 -C36  C36  C23  C23  C36 -C36 -C23 
	       C25  C56 -C56 -C25 -C25 -C56  C56  C25 
	      -C27 -C67  C67  C27  C27  C67 -C67 -C27 
	    */
	    t = coeff[9];	/* C23 */
	    c0 += t;
	    c3 -= t;
	    t = coeff[13];	/* C36 */
	    c1 += t;
	    c2 -= t;
	    break;

	  case 49:
	    /*
	      Coefficient[7][3]
	      
	       C37 -C77 -C17 -C57  C57  C17  C77 -C37 
	      -C35  C57  C15  C55 -C55 -C15 -C57  C35 
	       C33 -C37 -C13 -C35  C35  C13  C37 -C33 
	      -C13  C17  C11  C15 -C15 -C11 -C17  C13 
	       C13 -C17 -C11 -C15  C15  C11  C17 -C13 
	      -C33  C37  C13  C35 -C35 -C13 -C37  C33 
	       C35 -C57 -C15 -C55  C55  C15  C57 -C35 
	      -C37  C77  C17  C57 -C57 -C17 -C77  C37 
	    */
	    t = coeff[21];	/* C13 */
	    d2 -= t;    
	    t = coeff[24];	/* C33 */
	    d0 += t;    
	    t = coeff[25];	/* C35 */
	    d3 -= t;    
	    t = coeff[26];	/* C37 */
	    d1 -= t;    
	    break;

	  case 50:
	    /*
	      Coefficient[6][4]

	       C46 -C46 -C46  C46  C46 -C46 -C46  C46 
	      -C24  C24  C24 -C24 -C24  C24  C24 -C24 
	       C24 -C24 -C24  C24  C24 -C24 -C24  C24 
	      -C46  C46  C46 -C46 -C46  C46  C46 -C46 
	      -C46  C46  C46 -C46 -C46  C46  C46 -C46 
	       C24 -C24 -C24  C24  C24 -C24 -C24  C24 
	      -C24  C24  C24 -C24 -C24  C24  C24 -C24 
	       C46 -C46 -C46  C46  C46 -C46 -C46  C46 
	    */
	    t = coeff[2];	/* C24 */
	    a0 += t;
	    a1 -= t;
	    a2 -= t;
	    a3 += t;
	    break;

	  case 51:
	    /*
	      Coefficient[5][5]

	       C55 -C15  C57  C35 -C35 -C57  C15 -C55 
	      -C15  C11 -C17 -C13  C13  C17 -C11  C15 
	       C57 -C17  C77  C37 -C37 -C77  C17 -C57 
	       C35 -C13  C37  C33 -C33 -C37  C13 -C35 
	      -C35  C13 -C37 -C33  C33  C37 -C13  C35 
	      -C57  C17 -C77 -C37  C37  C77 -C17  C57 
	       C15 -C11  C17  C13 -C13 -C17  C11 -C15 
	      -C55  C15 -C57 -C35  C35  C57 -C15  C55 
	    */
	    t = coeff[23];	/* C17 */
	    d1 -= t;    
	    t = coeff[26];	/* C37 */
	    d3 += t;    
	    t = coeff[28];	/* C57 */
	    d0 += t;    
	    t = coeff[29];	/* C77 */
	    d2 += t;    
	    break;

	  case 52:
	    /*
	      Coefficient[4][6]

	       C46 -C24  C24 -C46 -C46  C24 -C24  C46 
	      -C46  C24 -C24  C46  C46 -C24  C24 -C46 
	      -C46  C24 -C24  C46  C46 -C24  C24 -C46 
	       C46 -C24  C24 -C46 -C46  C24 -C24  C46 
	       C46 -C24  C24 -C46 -C46  C24 -C24  C46 
	      -C46  C24 -C24  C46  C46 -C24  C24 -C46 
	      -C46  C24 -C24  C46  C46 -C24  C24 -C46 
	      C46 -C24  C24 -C46 -C46  C24 -C24  C46 
	    */
	    t = coeff[2];	/* C24 */
	    a1 += t;
	    a2 -= t;
	    t = coeff[3];	/* C46 */
	    a0 -= t;
	    a3 += t;
	    break;

	  case 53:
	    /*
	      Coefficient[3][7]

	       C37 -C35  C33 -C13  C13 -C33  C35 -C37 
	      -C77  C57 -C37  C17 -C17  C37 -C57  C77 
	      -C17  C15 -C13  C11 -C11  C13 -C15  C17 
	      -C57  C55 -C35  C15 -C15  C35 -C55  C57 
	       C57 -C55  C35 -C15  C15 -C35  C55 -C57 
	       C17 -C15  C13 -C11  C11 -C13  C15 -C17 
	       C77 -C57  C37 -C17  C17 -C37  C57 -C77 
	      -C37  C35 -C33  C13 -C13  C33 -C35  C37 
	    */
	    t = coeff[20];	/* C11 */
	    d3 += t;    
	    t = coeff[21];	/* C13 */
	    d2 -= t;    
	    t = coeff[22];	/* C15 */
	    d1 += t;    
	    t = coeff[23];	/* C17 */
	    d0 -= t;    
	    break;

	  case 54:
	    /*
	      Coefficient[4][7]

	       C47 -C45  C34 -C14  C14 -C34  C45 -C47 
	      -C47  C45 -C34  C14 -C14  C34 -C45  C47 
	      -C47  C45 -C34  C14 -C14  C34 -C45  C47 
	       C47 -C45  C34 -C14  C14 -C34  C45 -C47 
	       C47 -C45  C34 -C14  C14 -C34  C45 -C47 
	      -C47  C45 -C34  C14 -C14  C34 -C45  C47 
	      -C47  C45 -C34  C14 -C14  C34 -C45  C47 
	       C47 -C45  C34 -C14  C14 -C34  C45 -C47 
	    */
	    t = coeff[4];	/* C14 */
	    b3 += t;    
	    t = coeff[5];	/* C34 */
	    b2 -= t;    
	    t = coeff[6];	/* C45 */
	    b1 += t;    
	    t = coeff[7];	/* C47 */
	    b0 -= t;    
	    break;

	  case 55:
	    /*
	      Coefficient[5][6]

	       C56 -C25  C25 -C56 -C56  C25 -C25  C56 
	      -C16  C12 -C12  C16  C16 -C12  C12 -C16 
	       C67 -C27  C27 -C67 -C67  C27 -C27  C67 
	       C36 -C23  C23 -C36 -C36  C23 -C23  C36 
	      -C36  C23 -C23  C36  C36 -C23  C23 -C36 
	      -C67  C27 -C27  C67  C67 -C27  C27 -C67 
	       C16 -C12  C12 -C16 -C16  C12 -C12  C16 
	      -C56  C25 -C25  C56  C56 -C25  C25 -C56 
	    */
	    t = coeff[11];	/* C27 */
	    c1 -= t;
	    c2 += t;
	    t = coeff[15];	/* C67 */
	    c0 += t;
	    c3 -= t;
	    break;

	  case 56:
	    /*
	      Coefficient[6][5]

	       C56 -C16  C67  C36 -C36 -C67  C16 -C56 
	      -C25  C12 -C27 -C23  C23  C27 -C12  C25 
	       C25 -C12  C27  C23 -C23 -C27  C12 -C25 
	      -C56  C16 -C67 -C36  C36  C67 -C16  C56 
	      -C56  C16 -C67 -C36  C36  C67 -C16  C56 
	       C25 -C12  C27  C23 -C23 -C27  C12 -C25 
	      -C25  C12 -C27 -C23  C23  C27 -C12  C25 
	       C56 -C16  C67  C36 -C36 -C67  C16 -C56 
	    */
	    t = coeff[8];	/* C12 */
	    b1 -= t;    
	    t = coeff[9];	/* C23 */
	    b3 += t;    
	    t = coeff[10];	/* C25 */
	    b0 += t;    
	    t = coeff[11];	/* C27 */
	    b2 += t;    
	    break;

	  case 57:
	    /*
	      Coefficient[7][4]

	       C47 -C47 -C47  C47  C47 -C47 -C47  C47 
	      -C45  C45  C45 -C45 -C45  C45  C45 -C45 
	       C34 -C34 -C34  C34  C34 -C34 -C34  C34 
	      -C14  C14  C14 -C14 -C14  C14  C14 -C14 
	       C14 -C14 -C14  C14  C14 -C14 -C14  C14 
	      -C34  C34  C34 -C34 -C34  C34  C34 -C34 
	       C45 -C45 -C45  C45  C45 -C45 -C45  C45 
	      -C47  C47  C47 -C47 -C47  C47  C47 -C47 
	    */
	    t = coeff[5];	/* C34 */
	    c0 += t;
	    c1 -= t;
	    c2 -= t;
	    c3 += t;
	    break;

	  case 58:
	    /*
	      Coefficient[7][5]

	       C57 -C17  C77  C37 -C37 -C77  C17 -C57 
	      -C55  C15 -C57 -C35  C35  C57 -C15  C55 
	       C35 -C13  C37  C33 -C33 -C37  C13 -C35 
	      -C15  C11 -C17 -C13  C13  C17 -C11  C15 
	       C15 -C11  C17  C13 -C13 -C17  C11 -C15 
	      -C35  C13 -C37 -C33  C33  C37 -C13  C35 
	       C55 -C15  C57  C35 -C35 -C57  C15 -C55 
	      -C57  C17 -C77 -C37  C37  C77 -C17  C57 
	    */
	    t = coeff[21];	/* C13 */
	    d1 -= t;    
	    t = coeff[24];	/* C33 */
	    d3 += t;    
	    t = coeff[25];	/* C35 */
	    d0 += t;    
	    t = coeff[26];	/* C37 */
	    d2 += t;    
	    break;

	  case 59:
	    /*
	      Coefficient[6][6]

	       C66 -C26  C26 -C66 -C66  C26 -C26  C66 
	      -C26  C22 -C22  C26  C26 -C22  C22 -C26 
	       C26 -C22  C22 -C26 -C26  C22 -C22  C26 
	      -C66  C26 -C26  C66  C66 -C26  C26 -C66 
	      -C66  C26 -C26  C66  C66 -C26  C26 -C66 
	       C26 -C22  C22 -C26 -C26  C22 -C22  C26 
	      -C26  C22 -C22  C26  C26 -C22  C22 -C26 
	       C66 -C26  C26 -C66 -C66  C26 -C26  C66 
	    */
	    t = coeff[16];	/* C22 */
	    a1 -= t;
	    a2 += t;
	    t = coeff[17];	/* C26 */
	    a0 += t;
	    a3 -= t;
	    break;

	  case 60:
	    /*
	      Coefficient[5][7]

	       C57 -C55  C35 -C15  C15 -C35  C55 -C57 
	      -C17  C15 -C13  C11 -C11  C13 -C15  C17 
	       C77 -C57  C37 -C17  C17 -C37  C57 -C77 
	       C37 -C35  C33 -C13  C13 -C33  C35 -C37 
	      -C37  C35 -C33  C13 -C13  C33 -C35  C37 
	      -C77  C57 -C37  C17 -C17  C37 -C57  C77 
	       C17 -C15  C13 -C11  C11 -C13  C15 -C17 
	      -C57  C55 -C35  C15 -C15  C35 -C55  C57 
	    */
	    t = coeff[23];	/* C17 */
	    d3 -= t;    
	    t = coeff[26];	/* C37 */
	    d2 += t;    
	    t = coeff[28];	/* C57 */
	    d1 -= t;    
	    t = coeff[29];	/* C77 */
	    d0 += t;    
	    break;

	  case 61:
	    /*
	      Coefficient[6][7]

	       C67 -C56  C36 -C16  C16 -C36  C56 -C67 
	      -C27  C25 -C23  C12 -C12  C23 -C25  C27 
	       C27 -C25  C23 -C12  C12 -C23  C25 -C27 
	      -C67  C56 -C36  C16 -C16  C36 -C56  C67 
	      -C67  C56 -C36  C16 -C16  C36 -C56  C67 
	       C27 -C25  C23 -C12  C12 -C23  C25 -C27 
	      -C27  C25 -C23  C12 -C12  C23 -C25  C27 
	       C67 -C56  C36 -C16  C16 -C36  C56 -C67 
	    */
	    t = coeff[8];	/* C12 */
	    b3 -= t;    
	    t = coeff[9];	/* C23 */
	    b2 += t;    
	    t = coeff[10];	/* C25 */
	    b1 -= t;    
	    t = coeff[11];	/* C27 */
	    b0 += t;    
	    break;

	  case 62:
	    /*
	      Coefficient[7][6]

	       C67 -C27  C27 -C67 -C67  C27 -C27  C67 
	      -C56  C25 -C25  C56  C56 -C25  C25 -C56 
	       C36 -C23  C23 -C36 -C36  C23 -C23  C36 
	      -C16  C12 -C12  C16  C16 -C12  C12 -C16 
	       C16 -C12  C12 -C16 -C16  C12 -C12  C16 
	      -C36  C23 -C23  C36  C36 -C23  C23 -C36 
	       C56 -C25  C25 -C56 -C56  C25 -C25  C56 
	      -C67  C27 -C27  C67  C67 -C27  C27 -C67 
	    */
	    t = coeff[9];	/* C23 */
	    c1 -= t;
	    c2 += t;
	    t = coeff[13];	/* C36 */
	    c0 += t;
	    c3 -= t;
	    break;

	  case 63:
	    /*
	      Coefficient[7][7]

	       C77 -C57  C37 -C17  C17 -C37  C57 -C77 
	      -C57  C55 -C35  C15 -C15  C35 -C55  C57 
	       C37 -C35  C33 -C13  C13 -C33  C35 -C37 
	      -C17  C15 -C13  C11 -C11  C13 -C15  C17 
	       C17 -C15  C13 -C11  C11 -C13  C15 -C17 
	      -C37  C35 -C33  C13 -C13  C33 -C35  C37 
	       C57 -C55  C35 -C15  C15 -C35  C55 -C57 
	      -C77  C57 -C37  C17 -C17  C37 -C57  C77 
	    */
	    t = coeff[21];	/* C13 */
	    d3 -= t;    
	    t = coeff[24];	/* C33 */
	    d2 += t;    
	    t = coeff[25];	/* C35 */
	    d1 -= t;    
	    t = coeff[26];	/* C37 */
	    d0 += t;    
	    break;
	}
	
	i += 1;
	t = coefflist[i];

    } while (t != 0);

    t = a0 + b0;
    i = a0 - b0;
    a0 = c0 + d0;
    b0 = c0 - d0;
    c0 = t - a0;
    a0 = t + a0;
    d0 = i - b0;
    b0 = i + b0;
    a0 = ((unsigned int) a0) >> 16;
    b0 = ((unsigned int) b0) >> 16;
    c0 = ((unsigned int) c0) >> 16;
    d0 = ((unsigned int) d0) >> 16;

    t = a1 + b1;
    i = a1 - b1;
    a1 = c1 + d1;
    b1 = c1 - d1;
    c1 = t - a1;
    a1 = t + a1;
    d1 = i - b1;
    b1 = i + b1;
    a1 = ((unsigned int) a1) >> 16;
    b1 = ((unsigned int) b1) >> 16;
    c1 = ((unsigned int) c1) >> 16;
    d1 = ((unsigned int) d1) >> 16;

    t = a2 + b2;
    i = a2 - b2;
    a2 = c2 + d2;
    b2 = c2 - d2;
    c2 = t - a2;
    a2 = t + a2;
    d2 = i - b2;
    b2 = i + b2;
    a2 = ((unsigned int) a2) >> 16;
    b2 = ((unsigned int) b2) >> 16;
    c2 = ((unsigned int) c2) >> 16;
    d2 = ((unsigned int) d2) >> 16;

    t = a3 + b3;
    i = a3 - b3;
    a3 = c3 + d3;
    b3 = c3 - d3;
    c3 = t - a3;
    a3 = t + a3;
    d3 = i - b3;
    b3 = i + b3;
    a3 = ((unsigned int) a3) >> 16;
    b3 = ((unsigned int) b3) >> 16;
    c3 = ((unsigned int) c3) >> 16;
    d3 = ((unsigned int) d3) >> 16;

    a0 = PUT_SHORTS_INTO_INT(a0,a1);
    result[8] = a0;
    a2 = PUT_SHORTS_INTO_INT(a2,a3);
    result[9] = a2;
    b2 = PUT_SHORTS_INTO_INT(b3,b2);
    result[10] = b2;
    b0 = PUT_SHORTS_INTO_INT(b1,b0);
    result[11] = b0;
    c0 = PUT_SHORTS_INTO_INT(c0,c1);
    result[20] = c0;
    c2 = PUT_SHORTS_INTO_INT(c2,c3);
    result[21] = c2;
    d2 = PUT_SHORTS_INTO_INT(d3,d2);
    result[22] = d2;
    d0 = PUT_SHORTS_INTO_INT(d1,d0);
    result[23] = d0;


    i = 0;
    a0 = a1 = a2 = a3 = HALF;
    b0 = b1 = b2 = b3 = 0;
    c0 = c1 = c2 = c3 = 0;
    d0 = d1 = d2 = d3 = 0;

    t = coefflist[0];
    do {
	coeff = (int *)(t & ~63);
	t = t & 63;

	switch (t) {
	  case 0:
	    /*
	      Coefficient[0][0]

	      C44  C44  C44  C44  C44  C44  C44  C44 
	      C44  C44  C44  C44  C44  C44  C44  C44 
	      C44  C44  C44  C44  C44  C44  C44  C44 
	      C44  C44  C44  C44  C44  C44  C44  C44 
	      C44  C44  C44  C44  C44  C44  C44  C44 
	      C44  C44  C44  C44  C44  C44  C44  C44 
	      C44  C44  C44  C44  C44  C44  C44  C44 
	      C44  C44  C44  C44  C44  C44  C44  C44 
	    */
	    t = coeff[0];	/* C44 */
	    a0 += t;
	    a1 += t;
	    a2 += t;
	    a3 += t;    
	    break;


	  case 1:
	    /*
	      Coefficient[0][1]

	       C14  C34  C45  C47 -C47 -C45 -C34 -C14 
	       C14  C34  C45  C47 -C47 -C45 -C34 -C14 
	       C14  C34  C45  C47 -C47 -C45 -C34 -C14 
	       C14  C34  C45  C47 -C47 -C45 -C34 -C14 
	       C14  C34  C45  C47 -C47 -C45 -C34 -C14 
	       C14  C34  C45  C47 -C47 -C45 -C34 -C14 
	       C14  C34  C45  C47 -C47 -C45 -C34 -C14 
	       C14  C34  C45  C47 -C47 -C45 -C34 -C14 
	    */
	    t = coeff[4];	/* C14 */
	    b0 += t;    
	    t = coeff[5];	/* C34 */
	    b1 += t;    
	    t = coeff[6];	/* C45 */
	    b2 += t;    
	    t = coeff[7];	/* C47 */
	    b3 += t;
	    break;

	  case 2:
	    /*
	      Coefficient[1][0]

	       C14  C14  C14  C14  C14  C14  C14  C14 
	       C34  C34  C34  C34  C34  C34  C34  C34 
	       C45  C45  C45  C45  C45  C45  C45  C45 
	       C47  C47  C47  C47  C47  C47  C47  C47 
	      -C47 -C47 -C47 -C47 -C47 -C47 -C47 -C47 
	      -C45 -C45 -C45 -C45 -C45 -C45 -C45 -C45 
	      -C34 -C34 -C34 -C34 -C34 -C34 -C34 -C34
	      -C14 -C14 -C14 -C14 -C14 -C14 -C14 -C14 
	    */
	    t = coeff[7];	/* C47 */
	    c0 += t;  
	    c1 += t;
	    c2 += t;
	    c3 += t;    
	    break;

	  case 3:
	    /*
	      Coefficient[2][0]

	       C24  C24  C24  C24  C24  C24  C24  C24 
	       C46  C46  C46  C46  C46  C46  C46  C46 
	      -C46 -C46 -C46 -C46 -C46 -C46 -C46 -C46 
	      -C24 -C24 -C24 -C24 -C24 -C24 -C24 -C24 
	      -C24 -C24 -C24 -C24 -C24 -C24 -C24 -C24 
	      -C46 -C46 -C46 -C46 -C46 -C46 -C46 -C46 
	       C46  C46  C46  C46  C46  C46  C46  C46 
	       C24  C24  C24  C24  C24  C24  C24  C24 
	    */
	    t = coeff[2];	/* C24 */
	    a0 -= t;
	    a1 -= t;
	    a2 -= t;
	    a3 -= t;
	    break;


	  case 4:
	    /*
	      Coefficient[1][1]

	       C11  C13  C15  C17 -C17 -C15 -C13 -C11 
	       C13  C33  C35  C37 -C37 -C35 -C33 -C13 
	       C15  C35  C55  C57 -C57 -C55 -C35 -C15 
	       C17  C37  C57  C77 -C77 -C57 -C37 -C17 
	      -C17 -C37 -C57 -C77  C77  C57  C37  C17 
	      -C15 -C35 -C55 -C57  C57  C55  C35  C15 
	      -C13 -C33 -C35 -C37  C37  C35  C33  C13 
	      -C11 -C13 -C15 -C17  C17  C15  C13  C11 
	    */
	    t = coeff[23];	/* C17 */
	    d0 += t;    
	    t = coeff[26];	/* C37 */
	    d1 += t;    
	    t = coeff[28];	/* C57 */
	    d2 += t;    
	    t = coeff[29];	/* C77 */
	    d3 += t;    
	    break;

	  case 5:
	    /*
	      Coefficient[0][2]

	       C24  C46 -C46 -C24 -C24 -C46  C46  C24 
	       C24  C46 -C46 -C24 -C24 -C46  C46  C24 
	       C24  C46 -C46 -C24 -C24 -C46  C46  C24 
	       C24  C46 -C46 -C24 -C24 -C46  C46  C24 
	       C24  C46 -C46 -C24 -C24 -C46  C46  C24 
	       C24  C46 -C46 -C24 -C24 -C46  C46  C24 
	       C24  C46 -C46 -C24 -C24 -C46  C46  C24 
	       C24  C46 -C46 -C24 -C24 -C46  C46  C24 
	    */
	    t = coeff[2];	/* C24 */
	    a0 += t;
	    a3 -= t;    
	    t = coeff[3];	/* C46 */
	    a1 += t;
	    a2 -= t;    
	    break;

	  case 6:
	    /*
	      Coefficient[0][3]

	       C34 -C47 -C14 -C45  C45  C14  C47 -C34 
	       C34 -C47 -C14 -C45  C45  C14  C47 -C34 
	       C34 -C47 -C14 -C45  C45  C14  C47 -C34 
	       C34 -C47 -C14 -C45  C45  C14  C47 -C34 
	       C34 -C47 -C14 -C45  C45  C14  C47 -C34 
	       C34 -C47 -C14 -C45  C45  C14  C47 -C34 
	       C34 -C47 -C14 -C45  C45  C14  C47 -C34 
	       C34 -C47 -C14 -C45  C45  C14  C47 -C34 
	    */
	    t = coeff[4];	/* C14 */
	    b2 -= t;    
	    t = coeff[5];	/* C34 */
	    b0 += t;    
	    t = coeff[6];	/* C45 */
	    b3 -= t;    
	    t = coeff[7];	/* C47 */
	    b1 -= t;    
	    break;

	  case 7:
	    /*
	      Coefficient[1][2]

	       C12  C16 -C16 -C12 -C12 -C16  C16  C12 
	       C23  C36 -C36 -C23 -C23 -C36  C36  C23 
	       C25  C56 -C56 -C25 -C25 -C56  C56  C25 
	       C27  C67 -C67 -C27 -C27 -C67  C67  C27 
	      -C27 -C67  C67  C27  C27  C67 -C67 -C27 
	      -C25 -C56  C56  C25  C25  C56 -C56 -C25 
	      -C23 -C36  C36  C23  C23  C36 -C36 -C23 
	      -C12 -C16  C16  C12  C12  C16 -C16 -C12 
	    */
	    t = coeff[11];	/* C27 */
	    c0 += t;
	    c3 -= t; 
	    t = coeff[15];	/* C67 */
	    c1 += t;
	    c2 -= t;
	    break;

	  case 8:
	    /*
	      Coefficient[2][1]

	       C12  C23  C25  C27 -C27 -C25 -C23 -C12 
	       C16  C36  C56  C67 -C67 -C56 -C36 -C16 
	      -C16 -C36 -C56 -C67  C67  C56  C36  C16 
	      -C12 -C23 -C25 -C27  C27  C25  C23  C12 
	      -C12 -C23 -C25 -C27  C27  C25  C23  C12 
	      -C16 -C36 -C56 -C67  C67  C56  C36  C16 
	       C16  C36  C56  C67 -C67 -C56 -C36 -C16 
	       C12  C23  C25  C27 -C27 -C25 -C23 -C12 
	    */
	    t = coeff[8];	/* C12 */
	    b0 -= t;    
	    t = coeff[9];	/* C23 */
	    b1 -= t;    
	    t = coeff[10];	/* C25 */
	    b2 -= t;    
	    t = coeff[11];	/* C27 */
	    b3 -= t;    
	    break;

	  case 9:
	    /*
	      Coefficient[3][0]

	       C34  C34  C34  C34  C34  C34  C34  C34 
	      -C47 -C47 -C47 -C47 -C47 -C47 -C47 -C47 
	      -C14 -C14 -C14 -C14 -C14 -C14 -C14 -C14 
	      -C45 -C45 -C45 -C45 -C45 -C45 -C45 -C45 
	       C45  C45  C45  C45  C45  C45  C45  C45 
	       C14  C14  C14  C14  C14  C14  C14  C14 
	       C47  C47  C47  C47  C47  C47  C47  C47 
	      -C34 -C34 -C34 -C34 -C34 -C34 -C34 -C34 
	    */
	    t = coeff[6];	/* C45 */
	    c0 -= t;
	    c1 -= t;
	    c2 -= t;
	    c3 -= t;    
	    break;

	  case 10:
	    /*
	      Coefficient[4][0]
	      
	       C44  C44  C44  C44  C44  C44  C44  C44 
	      -C44 -C44 -C44 -C44 -C44 -C44 -C44 -C44 
	      -C44 -C44 -C44 -C44 -C44 -C44 -C44 -C44 
	       C44  C44  C44  C44  C44  C44  C44  C44 
	       C44  C44  C44  C44  C44  C44  C44  C44 
	      -C44 -C44 -C44 -C44 -C44 -C44 -C44 -C44 
	      -C44 -C44 -C44 -C44 -C44 -C44 -C44 -C44 
	       C44  C44  C44  C44  C44  C44  C44  C44 
	    */
	    t = coeff[0];	/* C44 */
	    a0 += t;
	    a1 += t;
	    a2 += t;
	    a3 += t;    
	    break;

	  case 11:
	    /*
	      Coefficient[3][1]

	       C13  C33  C35  C37 -C37 -C35 -C33 -C13 
	      -C17 -C37 -C57 -C77  C77  C57  C37  C17 
	      -C11 -C13 -C15 -C17  C17  C15  C13  C11 
	      -C15 -C35 -C55 -C57  C57  C55  C35  C15 
	       C15  C35  C55  C57 -C57 -C55 -C35 -C15 
	       C11  C13  C15  C17 -C17 -C15 -C13 -C11 
	       C17  C37  C57  C77 -C77 -C57 -C37 -C17 
	      -C13 -C33 -C35 -C37  C37  C35  C33  C13 
	   */
	    t = coeff[22];	/* C15 */
	    d0 -= t;    
	    t = coeff[25];	/* C35 */
	    d1 -= t;    
	    t = coeff[27];	/* C55 */
	    d2 -= t;    
	    t = coeff[28];	/* C57 */
	    d3 -= t;
	    break;

	  case 12:
	    /*
	      Coefficient[2][2]

	       C22  C26 -C26 -C22 -C22 -C26  C26  C22 
	       C26  C66 -C66 -C26 -C26 -C66  C66  C26 
	      -C26 -C66  C66  C26  C26  C66 -C66 -C26 
	      -C22 -C26  C26  C22  C22  C26 -C26 -C22 
	      -C22 -C26  C26  C22  C22  C26 -C26 -C22 
	      -C26 -C66  C66  C26  C26  C66 -C66 -C26 
	       C26  C66 -C66 -C26 -C26 -C66  C66  C26 
	       C22  C26 -C26 -C22 -C22 -C26  C26  C22 
	    */
	    t = coeff[16];	/* C22 */
	    a0 -= t;
	    a3 += t;    
	    t = coeff[17];	/* C26 */
	    a1 -= t;
	    a2 += t;    
	    break;

	  case 13:
	    /*
	      Coefficient[1][3]

	       C13 -C17 -C11 -C15  C15  C11  C17 -C13 
	       C33 -C37 -C13 -C35  C35  C13  C37 -C33 
	       C35 -C57 -C15 -C55  C55  C15  C57 -C35 
	       C37 -C77 -C17 -C57  C57  C17  C77 -C37 
	      -C37  C77  C17  C57 -C57 -C17 -C77  C37 
	      -C35  C57  C15  C55 -C55 -C15 -C57  C35 
	      -C33  C37  C13  C35 -C35 -C13 -C37  C33 
	      -C13  C17  C11  C15 -C15 -C11 -C17  C13 
	    */
	    t = coeff[23];	/* C17 */
	    d2 -= t;    
	    t = coeff[26];	/* C37 */
	    d0 += t;    
	    t = coeff[28];	/* C57 */
	    d3 -= t;    
	    t = coeff[29];	/* C77 */
	    d1 -= t;    
	    break;

	  case 14:
	    /*
	      Coefficient[0][4]

	       C44 -C44 -C44  C44  C44 -C44 -C44  C44 
 	       C44 -C44 -C44  C44  C44 -C44 -C44  C44 
	       C44 -C44 -C44  C44  C44 -C44 -C44  C44 
	       C44 -C44 -C44  C44  C44 -C44 -C44  C44 
	       C44 -C44 -C44  C44  C44 -C44 -C44  C44 
	       C44 -C44 -C44  C44  C44 -C44 -C44  C44 
	       C44 -C44 -C44  C44  C44 -C44 -C44  C44 
	       C44 -C44 -C44  C44  C44 -C44 -C44  C44 
	    */
	    t = coeff[0];	/* C44 */
	    a0 += t;
	    a1 -= t;
	    a2 -= t;
	    a3 += t;    
	    break;

	  case 15:
	    /*
	      Coefficient[0][5]

	       C45 -C14  C47  C34 -C34 -C47  C14 -C45 
	       C45 -C14  C47  C34 -C34 -C47  C14 -C45 
	       C45 -C14  C47  C34 -C34 -C47  C14 -C45 
	       C45 -C14  C47  C34 -C34 -C47  C14 -C45 
	       C45 -C14  C47  C34 -C34 -C47  C14 -C45 
	       C45 -C14  C47  C34 -C34 -C47  C14 -C45 
	       C45 -C14  C47  C34 -C34 -C47  C14 -C45 
	       C45 -C14  C47  C34 -C34 -C47  C14 -C45 
	    */
	    t = coeff[4];	/* C14 */
	    b1 -= t;    
	    t = coeff[5];	/* C34 */
	    b3 += t;    
	    t = coeff[6];	/* C45 */
	    b0 += t;    
	    t = coeff[7];	/* C47 */
	    b2 += t;    
	    break;

	  case 16:
	    /*
	      Coefficient[1][4]

	       C14 -C14 -C14  C14  C14 -C14 -C14  C14 
	       C34 -C34 -C34  C34  C34 -C34 -C34  C34 
	       C45 -C45 -C45  C45  C45 -C45 -C45  C45 
	       C47 -C47 -C47  C47  C47 -C47 -C47  C47 
	      -C47  C47  C47 -C47 -C47  C47  C47 -C47 
	      -C45  C45  C45 -C45 -C45  C45  C45 -C45 
	      -C34  C34  C34 -C34 -C34  C34  C34 -C34 
	      -C14  C14  C14 -C14 -C14  C14  C14 -C14 
	    */
	    t = coeff[7];	/* C47 */
	    c0 += t;
	    c1 -= t;
	    c2 -= t;
	    c3 += t;
	    break;

	  case 17:
	    /*
	      Coefficient[2][3]

	       C23 -C27 -C12 -C25  C25  C12  C27 -C23 
	       C36 -C67 -C16 -C56  C56  C16  C67 -C36 
	      -C36  C67  C16  C56 -C56 -C16 -C67  C36 
	      -C23  C27  C12  C25 -C25 -C12 -C27  C23 
	      -C23  C27  C12  C25 -C25 -C12 -C27  C23 
	      -C36  C67  C16  C56 -C56 -C16 -C67  C36 
	       C36 -C67 -C16 -C56  C56  C16  C67 -C36 
	       C23 -C27 -C12 -C25  C25  C12  C27 -C23 
	    */
	    t = coeff[8];	/* C12 */
	    b2 += t;    
	    t = coeff[9];	/* C23 */
	    b0 -= t;    
	    t = coeff[10];	/* C25 */
	    b3 += t;    
	    t = coeff[11];	/* C27 */
	    b1 += t;
	    break;

	  case 18:
	    /*
	      Coefficient[3][2]

 	       C23  C36 -C36 -C23 -C23 -C36  C36  C23 
	      -C27 -C67  C67  C27  C27  C67 -C67 -C27 
	      -C12 -C16  C16  C12  C12  C16 -C16 -C12 
	      -C25 -C56  C56  C25  C25  C56 -C56 -C25 
	       C25  C56 -C56 -C25 -C25 -C56  C56  C25 
	       C12  C16 -C16 -C12 -C12 -C16  C16  C12 
	       C27  C67 -C67 -C27 -C27 -C67  C67  C27 
	      -C23 -C36  C36  C23  C23  C36 -C36 -C23 
	    */
	    t = coeff[10];	/* C25 */
	    c0 -= t;
	    c3 += t;
	    t = coeff[14];	/* C56 */
	    c1 -= t;
	    c2 += t;
	    break;

	  case 19:
	    /*
	      Coefficient[4][1]

	       C14  C34  C45  C47 -C47 -C45 -C34 -C14 
	      -C14 -C34 -C45 -C47  C47  C45  C34  C14 
	      -C14 -C34 -C45 -C47  C47  C45  C34  C14 
	       C14  C34  C45  C47 -C47 -C45 -C34 -C14 
	       C14  C34  C45  C47 -C47 -C45 -C34 -C14 
	      -C14 -C34 -C45 -C47  C47  C45  C34  C14 
	      -C14 -C34 -C45 -C47  C47  C45  C34  C14 
	       C14  C34  C45  C47 -C47 -C45 -C34 -C14 
	    */
	    t = coeff[4];	/* C14 */
	    b0 += t;    
	    t = coeff[5];	/* C34 */
	    b1 += t;    
	    t = coeff[6];	/* C45 */
	    b2 += t;    
	    t = coeff[7];	/* C47 */
	    b3 += t;    
	    break;

	  case 20:
	    /*
	      Coefficient[5][0]

	       C45  C45  C45  C45  C45  C45  C45  C45 
	      -C14 -C14 -C14 -C14 -C14 -C14 -C14 -C14 
	       C47  C47  C47  C47  C47  C47  C47  C47 
	       C34  C34  C34  C34  C34  C34  C34  C34 
	      -C34 -C34 -C34 -C34 -C34 -C34 -C34 -C34 
	      -C47 -C47 -C47 -C47 -C47 -C47 -C47 -C47 
	       C14  C14  C14  C14  C14  C14  C14  C14 
	      -C45 -C45 -C45 -C45 -C45 -C45 -C45 -C45 
	    */
	    t = coeff[5];	/* C34 */
	    c0 += t;
	    c1 += t;
	    c2 += t;
	    c3 += t;
	    break;
	    
	  case 21:
	    /*
	      Coefficient[6][0]

	       C46  C46  C46  C46  C46  C46  C46  C46 
	      -C24 -C24 -C24 -C24 -C24 -C24 -C24 -C24 
	       C24  C24  C24  C24  C24  C24  C24  C24 
	      -C46 -C46 -C46 -C46 -C46 -C46 -C46 -C46 
	      -C46 -C46 -C46 -C46 -C46 -C46 -C46 -C46 
	       C24  C24  C24  C24  C24  C24  C24  C24 
	      -C24 -C24 -C24 -C24 -C24 -C24 -C24 -C24 
	       C46  C46  C46  C46  C46  C46  C46  C46 
	    */
	    t = coeff[3];	/* C46 */
	    a0 -= t;
	    a1 -= t;
	    a2 -= t;
	    a3 -= t;    
	    break;

	  case 22:
	    /*
	      Coefficient[5][1]

	       C15  C35  C55  C57 -C57 -C55 -C35 -C15 
	      -C11 -C13 -C15 -C17  C17  C15  C13  C11 
	       C17  C37  C57  C77 -C77 -C57 -C37 -C17 
	       C13  C33  C35  C37 -C37 -C35 -C33 -C13 
	      -C13 -C33 -C35 -C37  C37  C35  C33  C13 
	      -C17 -C37 -C57 -C77  C77  C57  C37  C17 
	       C11  C13  C15  C17 -C17 -C15 -C13 -C11 
	      -C15 -C35 -C55 -C57  C57  C55  C35  C15 
	    */
	    t = coeff[21];	/* C13 */
	    d0 += t;    
	    t = coeff[24];	/* C33 */
	    d1 += t;    
	    t = coeff[25];	/* C35 */
	    d2 += t;    
	    t = coeff[26];	/* C37 */
	    d3 += t;    
	    break;

	  case 23:
	    /*
	      Coefficient[4][2]

	       C24  C46 -C46 -C24 -C24 -C46  C46  C24 
	      -C24 -C46  C46  C24  C24  C46 -C46 -C24 
	      -C24 -C46  C46  C24  C24  C46 -C46 -C24 
	       C24  C46 -C46 -C24 -C24 -C46  C46  C24 
	       C24  C46 -C46 -C24 -C24 -C46  C46  C24 
	      -C24 -C46  C46  C24  C24  C46 -C46 -C24 
	      -C24 -C46  C46  C24  C24  C46 -C46 -C24 
	       C24  C46 -C46 -C24 -C24 -C46  C46  C24 
	    */
	    t = coeff[2];	/* C24 */
	    a0 += t;
	    a3 -= t;    
	    t = coeff[3];	/* C46 */
	    a1 += t;
	    a2 -= t;    
	    break;

	  case 24:
	    /*
	      Coefficient[3][3]

	       C33 -C37 -C13 -C35  C35  C13  C37 -C33 
	      -C37  C77  C17  C57 -C57 -C17 -C77  C37 
	      -C13  C17  C11  C15 -C15 -C11 -C17  C13 
	      -C35  C57  C15  C55 -C55 -C15 -C57  C35 
	       C35 -C57 -C15 -C55  C55  C15  C57 -C35 
	       C13 -C17 -C11 -C15  C15  C11  C17 -C13 
	       C37 -C77 -C17 -C57  C57  C17  C77 -C37 
	      -C33  C37  C13  C35 -C35 -C13 -C37  C33 
	    */
	    t = coeff[22];	/* C15 */
	    d2 += t;    
	    t = coeff[25];	/* C35 */
	    d0 -= t;    
	    t = coeff[27];	/* C55 */
	    d3 += t;    
	    t = coeff[28];	/* C57 */
	    d1 += t;    
	    break;

	  case 25:
	    /*
	      Coefficient[2][4]

	       C24 -C24 -C24  C24  C24 -C24 -C24  C24 
	       C46 -C46 -C46  C46  C46 -C46 -C46  C46 
	      -C46  C46  C46 -C46 -C46  C46  C46 -C46 
	      -C24  C24  C24 -C24 -C24  C24  C24 -C24 
	      -C24  C24  C24 -C24 -C24  C24  C24 -C24 
	      -C46  C46  C46 -C46 -C46  C46  C46 -C46 
	       C46 -C46 -C46  C46  C46 -C46 -C46  C46 
	       C24 -C24 -C24  C24  C24 -C24 -C24  C24 
	    */
	    t = coeff[2];	/* C24 */
	    a0 -= t;
	    a1 += t;
	    a2 += t;
	    a3 -= t;
	    break;

	  case 26:
	    /*
	      Coefficient[1][5]

	       C15 -C11  C17  C13 -C13 -C17  C11 -C15 
	       C35 -C13  C37  C33 -C33 -C37  C13 -C35 
	       C55 -C15  C57  C35 -C35 -C57  C15 -C55 
	       C57 -C17  C77  C37 -C37 -C77  C17 -C57 
	      -C57  C17 -C77 -C37  C37  C77 -C17  C57 
	      -C55  C15 -C57 -C35  C35  C57 -C15  C55 
	      -C35  C13 -C37 -C33  C33  C37 -C13  C35 
	      -C15  C11 -C17 -C13  C13  C17 -C11  C15 
	    */
	    t = coeff[23];	/* C17 */
	    d1 -= t;    
	    t = coeff[26];	/* C37 */
	    d3 += t;    
	    t = coeff[28];	/* C57 */
	    d0 += t;    
	    t = coeff[29];	/* C77 */
	    d2 += t;    
	    break;

	  case 27:
	    /*
	      Coefficient[0][6]

	       C46 -C24  C24 -C46 -C46  C24 -C24  C46 
	       C46 -C24  C24 -C46 -C46  C24 -C24  C46 
	       C46 -C24  C24 -C46 -C46  C24 -C24  C46 
	       C46 -C24  C24 -C46 -C46  C24 -C24  C46 
	       C46 -C24  C24 -C46 -C46  C24 -C24  C46 
	       C46 -C24  C24 -C46 -C46  C24 -C24  C46 
	       C46 -C24  C24 -C46 -C46  C24 -C24  C46 
	       C46 -C24  C24 -C46 -C46  C24 -C24  C46 
	    */
	    t = coeff[2];	/* C24 */
	    a1 -= t;
	    a2 += t;
	    t = coeff[3];	/* C46 */
	    a0 += t;
	    a3 -= t;
	    break;

	  case 28:
	    /*
	      Coefficient[0][7]

	       C47 -C45  C34 -C14  C14 -C34  C45 -C47 
	       C47 -C45  C34 -C14  C14 -C34  C45 -C47 
	       C47 -C45  C34 -C14  C14 -C34  C45 -C47 
	       C47 -C45  C34 -C14  C14 -C34  C45 -C47 
	       C47 -C45  C34 -C14  C14 -C34  C45 -C47 
	       C47 -C45  C34 -C14  C14 -C34  C45 -C47 
	       C47 -C45  C34 -C14  C14 -C34  C45 -C47 
	       C47 -C45  C34 -C14  C14 -C34  C45 -C47 
	    */
	    t = coeff[4];	/* C14 */
	    b3 -= t;    
	    t = coeff[5];	/* C34 */
	    b2 += t;    
	    t = coeff[6];	/* C45 */
	    b1 -= t;    
	    t = coeff[7];	/* C47 */
	    b0 += t;    
	    break;

	  case 29:
	    /*
	      Coefficient[1][6]

	       C16 -C12  C12 -C16 -C16  C12 -C12  C16 
	       C36 -C23  C23 -C36 -C36  C23 -C23  C36 
	       C56 -C25  C25 -C56 -C56  C25 -C25  C56 
	       C67 -C27  C27 -C67 -C67  C27 -C27  C67 
	      -C67  C27 -C27  C67  C67 -C27  C27 -C67 
	      -C56  C25 -C25  C56  C56 -C25  C25 -C56 
	      -C36  C23 -C23  C36  C36 -C23  C23 -C36 
	      -C16  C12 -C12  C16  C16 -C12  C12 -C16 
	    */
	    t = coeff[11];	/* C27 */
	    c1 -= t;
	    c2 += t;
	    t = coeff[15];	/* C67 */
	    c0 += t;
	    c3 -= t;
	    break;

	  case 30:
	    /*
	      Coefficient[2][5]

	       C25 -C12  C27  C23 -C23 -C27  C12 -C25 
	       C56 -C16  C67  C36 -C36 -C67  C16 -C56 
	      -C56  C16 -C67 -C36  C36  C67 -C16  C56 
	      -C25  C12 -C27 -C23  C23  C27 -C12  C25 
	      -C25  C12 -C27 -C23  C23  C27 -C12  C25 
	      -C56  C16 -C67 -C36  C36  C67 -C16  C56 
	       C56 -C16  C67  C36 -C36 -C67  C16 -C56 
	       C25 -C12  C27  C23 -C23 -C27  C12 -C25 
	    */
	    t = coeff[8];	/* C12 */
	    b1 += t;    
	    t = coeff[9];	/* C23 */
	    b3 -= t;    
	    t = coeff[10];	/* C25 */
	    b0 -= t;    
	    t = coeff[11];	/* C27 */
	    b2 -= t;    
	    break;

	  case 31:
	    /*
	      Coefficient[3][4]

	       C34 -C34 -C34  C34  C34 -C34 -C34  C34 
	      -C47  C47  C47 -C47 -C47  C47  C47 -C47 
	      -C14  C14  C14 -C14 -C14  C14  C14 -C14 
	      -C45  C45  C45 -C45 -C45  C45  C45 -C45 
	       C45 -C45 -C45  C45  C45 -C45 -C45  C45 
	       C14 -C14 -C14  C14  C14 -C14 -C14  C14 
	       C47 -C47 -C47  C47  C47 -C47 -C47  C47 
	      -C34  C34  C34 -C34 -C34  C34  C34 -C34 
	    */
	    t = coeff[6];	/* C45 */
	    c0 -= t;
	    c1 += t;
	    c2 += t;
	    c3 -= t;
	    break;

	  case 32:
	    /*
	      Coefficient[4][3]

	       C34 -C47 -C14 -C45  C45  C14  C47 -C34 
	      -C34  C47  C14  C45 -C45 -C14 -C47  C34 
	      -C34  C47  C14  C45 -C45 -C14 -C47  C34 
	       C34 -C47 -C14 -C45  C45  C14  C47 -C34 
	       C34 -C47 -C14 -C45  C45  C14  C47 -C34 
	      -C34  C47  C14  C45 -C45 -C14 -C47  C34 
	      -C34  C47  C14  C45 -C45 -C14 -C47  C34 
	       C34 -C47 -C14 -C45  C45  C14  C47 -C34 
	    */
	    t = coeff[4];	/* C14 */
	    b2 -= t;    
	    t = coeff[5];	/* C34 */
	    b0 += t;    
	    t = coeff[6];	/* C45 */
	    b3 -= t;    
	    t = coeff[7];	/* C47 */
	    b1 -= t;    
	    break;

	  case 33:
	    /*
	      Coefficient[5][2]

	       C25  C56 -C56 -C25 -C25 -C56  C56  C25 
	      -C12 -C16  C16  C12  C12  C16 -C16 -C12 
	       C27  C67 -C67 -C27 -C27 -C67  C67  C27 
	       C23  C36 -C36 -C23 -C23 -C36  C36  C23 
	      -C23 -C36  C36  C23  C23  C36 -C36 -C23 
	      -C27 -C67  C67  C27  C27  C67 -C67 -C27 
	       C12  C16 -C16 -C12 -C12 -C16  C16  C12 
	      -C25 -C56  C56  C25  C25  C56 -C56 -C25 
	    */
	    t = coeff[9];	/* C23 */
	    c0 += t;
	    c3 -= t;
	    t = coeff[13];	/* C36 */
	    c1 += t;
	    c2 -= t;
	    break;

	  case 34:
	    /*
	      Coefficient[6][1]

	       C16  C36  C56  C67 -C67 -C56 -C36 -C16 
	      -C12 -C23 -C25 -C27  C27  C25  C23  C12 
	       C12  C23  C25  C27 -C27 -C25 -C23 -C12 
	      -C16 -C36 -C56 -C67  C67  C56  C36  C16 
	      -C16 -C36 -C56 -C67  C67  C56  C36  C16 
	       C12  C23  C25  C27 -C27 -C25 -C23 -C12 
	      -C12 -C23 -C25 -C27  C27  C25  C23  C12 
	       C16  C36  C56  C67 -C67 -C56 -C36 -C16 
	    */
	    t = coeff[12];	/* C16 */
	    b0 -= t;    
	    t = coeff[13];	/* C36 */
	    b1 -= t;    
	    t = coeff[14];	/* C56 */
	    b2 -= t;    
	    t = coeff[15];	/* C67 */
	    b3 -= t;    
	    break;

	  case 35:
	    /*
	      Coefficient[7][0]

	       C47  C47  C47  C47  C47  C47  C47  C47 
	      -C45 -C45 -C45 -C45 -C45 -C45 -C45 -C45 
	       C34  C34  C34  C34  C34  C34  C34  C34 
	      -C14 -C14 -C14 -C14 -C14 -C14 -C14 -C14 
	       C14  C14  C14  C14  C14  C14  C14  C14 
	      -C34 -C34 -C34 -C34 -C34 -C34 -C34 -C34 
	       C45  C45  C45  C45  C45  C45  C45  C45 
	      -C47 -C47 -C47 -C47 -C47 -C47 -C47 -C47 
	    */
	    t = coeff[4];	/* C14 */
	    c0 -= t;
	    c1 -= t;
	    c2 -= t;
	    c3 -= t;    
	    break;

	  case 36:
	    /*
	      Coefficient[7][1]

	       C17  C37  C57  C77 -C77 -C57 -C37 -C17 
	      -C15 -C35 -C55 -C57  C57  C55  C35  C15 
	       C13  C33  C35  C37 -C37 -C35 -C33 -C13 
	      -C11 -C13 -C15 -C17  C17  C15  C13  C11 
	       C11  C13  C15  C17 -C17 -C15 -C13 -C11 
	      -C13 -C33 -C35 -C37  C37  C35  C33  C13 
	       C15  C35  C55  C57 -C57 -C55 -C35 -C15 
	      -C17 -C37 -C57 -C77  C77  C57  C37  C17 
	    */
	    t = coeff[20];	/* C11 */
	    d0 -= t;    
	    t = coeff[21];	/* C13 */
	    d1 -= t;    
	    t = coeff[22];	/* C15 */
	    d2 -= t;    
	    t = coeff[23];	/* C17 */
	    d3 -= t;    
	    break;

	  case 37:
	    /*
	      Coefficient[6][2]

	       C26  C66 -C66 -C26 -C26 -C66  C66  C26 
	      -C22 -C26  C26  C22  C22  C26 -C26 -C22 
	       C22  C26 -C26 -C22 -C22 -C26  C26  C22 
	      -C26 -C66  C66  C26  C26  C66 -C66 -C26 
	      -C26 -C66  C66  C26  C26  C66 -C66 -C26 
	       C22  C26 -C26 -C22 -C22 -C26  C26  C22 
	      -C22 -C26  C26  C22  C22  C26 -C26 -C22 
	       C26  C66 -C66 -C26 -C26 -C66  C66  C26 
	    */
	    t = coeff[18];	/* C26 */
	    a0 -= t;
	    a3 += t;
	    t = coeff[19];	/* C66 */
	    a1 -= t;
	    a2 += t;    
	    break;

	  case 38:
	    /*
	      Coefficient[5][3]

	       C35 -C57 -C15 -C55  C55  C15  C57 -C35 
	      -C13  C17  C11  C15 -C15 -C11 -C17  C13 
	       C37 -C77 -C17 -C57  C57  C17  C77 -C37 
	       C33 -C37 -C13 -C35  C35  C13  C37 -C33 
	      -C33  C37  C13  C35 -C35 -C13 -C37  C33 
	      -C37  C77  C17  C57 -C57 -C17 -C77  C37 
	       C13 -C17 -C11 -C15  C15  C11  C17 -C13 
	      -C35  C57  C15  C55 -C55 -C15 -C57  C35 
	    */
	    t = coeff[21];	/* C13 */
	    d2 -= t;    
	    t = coeff[24];	/* C33 */
	    d0 += t;    
	    t = coeff[25];	/* C35 */
	    d3 -= t;    
	    t = coeff[26];	/* C37 */
	    d1 -= t;    
	    break;

	  case 39:
	    /*
	      Coefficient[4][4]

	       C44 -C44 -C44  C44  C44 -C44 -C44  C44 
	      -C44  C44  C44 -C44 -C44  C44  C44 -C44 
	      -C44  C44  C44 -C44 -C44  C44  C44 -C44 
	       C44 -C44 -C44  C44  C44 -C44 -C44  C44 
	       C44 -C44 -C44  C44  C44 -C44 -C44  C44 
	      -C44  C44  C44 -C44 -C44  C44  C44 -C44 
	      -C44  C44  C44 -C44 -C44  C44  C44 -C44 
	       C44 -C44 -C44  C44  C44 -C44 -C44  C44 
	    */
	    t = coeff[0];	/* C44 */
	    a0 += t;
	    a1 -= t;
	    a2 -= t;
	    a3 += t;
	    break;

	  case 40:
	    /*
	      Coefficient[3][5]

	       C35 -C13  C37  C33 -C33 -C37  C13 -C35 
	      -C57  C17 -C77 -C37  C37  C77 -C17  C57 
	      -C15  C11 -C17 -C13  C13  C17 -C11  C15 
	      -C55  C15 -C57 -C35  C35  C57 -C15  C55 
	       C55 -C15  C57  C35 -C35 -C57  C15 -C55 
	       C15 -C11  C17  C13 -C13 -C17  C11 -C15 
	       C57 -C17  C77  C37 -C37 -C77  C17 -C57 
	      -C35  C13 -C37 -C33  C33  C37 -C13  C35 
	    */
	    t = coeff[22];	/* C15 */
	    d1 += t;    
	    t = coeff[25];	/* C35 */
	    d3 -= t;    
	    t = coeff[27];	/* C55 */
	    d0 -= t;    
	    t = coeff[28];	/* C57 */
	    d2 -= t;    
	    break;

	  case 41:
	    /*
	      Coefficient[2][6]

	       C26 -C22  C22 -C26 -C26  C22 -C22  C26 
	       C66 -C26  C26 -C66 -C66  C26 -C26  C66 
	      -C66  C26 -C26  C66  C66 -C26  C26 -C66 
	      -C26  C22 -C22  C26  C26 -C22  C22 -C26 
	      -C26  C22 -C22  C26  C26 -C22  C22 -C26 
	      -C66  C26 -C26  C66  C66 -C26  C26 -C66 
	       C66 -C26  C26 -C66 -C66  C26 -C26  C66 
	       C26 -C22  C22 -C26 -C26  C22 -C22  C26 
	    */
	    t = coeff[16];	/* C22 */
	    a1 += t;
	    a2 -= t;
	    t = coeff[17];	/* C26 */
	    a0 -= t;
	    a3 += t;
	    break;

	  case 42:
	    /*
	       Coefficient[1][7]

	        C17 -C15  C13 -C11  C11 -C13  C15 -C17 
	        C37 -C35  C33 -C13  C13 -C33  C35 -C37 
	        C57 -C55  C35 -C15  C15 -C35  C55 -C57 
	        C77 -C57  C37 -C17  C17 -C37  C57 -C77 
	       -C77  C57 -C37  C17 -C17  C37 -C57  C77 
	       -C57  C55 -C35  C15 -C15  C35 -C55  C57 
	       -C37  C35 -C33  C13 -C13  C33 -C35  C37 
	       -C17  C15 -C13  C11 -C11  C13 -C15  C17 
	    */
	    t = coeff[23];	/* C17 */
	    d3 -= t;    
	    t = coeff[26];	/* C37 */
	    d2 += t;    
	    t = coeff[28];	/* C57 */
	    d1 -= t;    
	    t = coeff[29];	/* C77 */
	    d0 += t;    
	    break;

	  case 43:
	    /*
	      Coefficient[2][7]

	       C27 -C25  C23 -C12  C12 -C23  C25 -C27 
	       C67 -C56  C36 -C16  C16 -C36  C56 -C67 
	      -C67  C56 -C36  C16 -C16  C36 -C56  C67 
	      -C27  C25 -C23  C12 -C12  C23 -C25  C27 
	      -C27  C25 -C23  C12 -C12  C23 -C25  C27 
	      -C67  C56 -C36  C16 -C16  C36 -C56  C67 
	       C67 -C56  C36 -C16  C16 -C36  C56 -C67 
	       C27 -C25  C23 -C12  C12 -C23  C25 -C27 
	    */
	    t = coeff[8];	/* C12 */
	    b3 += t;    
	    t = coeff[9];	/* C23 */
	    b2 -= t;    
	    t = coeff[10];	/* C25 */
	    b1 += t;    
	    t = coeff[11];	/* C27 */
	    b0 -= t;    
	    break;

	  case 44:
	    /*
	      Coefficient[3][6]

	       C36 -C23  C23 -C36 -C36  C23 -C23  C36 
	      -C67  C27 -C27  C67  C67 -C27  C27 -C67 
	      -C16  C12 -C12  C16  C16 -C12  C12 -C16 
	      -C56  C25 -C25  C56  C56 -C25  C25 -C56 
	       C56 -C25  C25 -C56 -C56  C25 -C25  C56 
	       C16 -C12  C12 -C16 -C16  C12 -C12  C16 
	       C67 -C27  C27 -C67 -C67  C27 -C27  C67 
	      -C36  C23 -C23  C36  C36 -C23  C23 -C36 
	    */
	    t = coeff[10];	/* C25 */
	    c1 += t;
	    c2 -= t;
	    t = coeff[14];	/* C56 */
	    c0 -= t;
	    c3 += t;
	    break;

	  case 45:
	    /*
	      Coefficient[4][5]

	       C45 -C14  C47  C34 -C34 -C47  C14 -C45 
	      -C45  C14 -C47 -C34  C34  C47 -C14  C45 
	      -C45  C14 -C47 -C34  C34  C47 -C14  C45 
	       C45 -C14  C47  C34 -C34 -C47  C14 -C45 
	       C45 -C14  C47  C34 -C34 -C47  C14 -C45 
	      -C45  C14 -C47 -C34  C34  C47 -C14  C45 
	      -C45  C14 -C47 -C34  C34  C47 -C14  C45 
	       C45 -C14  C47  C34 -C34 -C47  C14 -C45 
	    */
	    t = coeff[4];	/* C14 */
	    b1 -= t;    
	    t = coeff[5];	/* C34 */
	    b3 += t;    
	    t = coeff[6];	/* C45 */
	    b0 += t;    
	    t = coeff[7];	/* C47 */
	    b2 += t;    
	    break;

	  case 46:
	    /*
	      Coefficient[5][4]

	       C45 -C45 -C45  C45  C45 -C45 -C45  C45 
	      -C14  C14  C14 -C14 -C14  C14  C14 -C14 
	       C47 -C47 -C47  C47  C47 -C47 -C47  C47 
	       C34 -C34 -C34  C34  C34 -C34 -C34  C34 
	      -C34  C34  C34 -C34 -C34  C34  C34 -C34 
	      -C47  C47  C47 -C47 -C47  C47  C47 -C47 
	       C14 -C14 -C14  C14  C14 -C14 -C14  C14 
	      -C45  C45  C45 -C45 -C45  C45  C45 -C45 
	    */
	    t = coeff[5];	/* C34 */
	    c0 += t;
	    c1 -= t;
	    c2 -= t;
	    c3 += t;
	    break;

	  case 47:
	    /*
	      Coefficient[6][3]

	       C36 -C67 -C16 -C56  C56  C16  C67 -C36 
	      -C23  C27  C12  C25 -C25 -C12 -C27  C23 
	       C23 -C27 -C12 -C25  C25  C12  C27 -C23 
	      -C36  C67  C16  C56 -C56 -C16 -C67  C36 
	      -C36  C67  C16  C56 -C56 -C16 -C67  C36 
	       C23 -C27 -C12 -C25  C25  C12  C27 -C23 
	      -C23  C27  C12  C25 -C25 -C12 -C27  C23 
	       C36 -C67 -C16 -C56  C56  C16  C67 -C36 
	    */
	    t = coeff[12];	/* C16 */
	    b2 += t;    
	    t = coeff[13];	/* C36 */
	    b0 -= t;    
	    t = coeff[14];	/* C56 */
	    b3 += t;    
	    t = coeff[15];	/* C67 */
	    b1 += t;    
	    break;

	  case 48:
	    /*
	      Coefficient[7][2]

	       C27  C67 -C67 -C27 -C27 -C67  C67  C27 
	      -C25 -C56  C56  C25  C25  C56 -C56 -C25 
	       C23  C36 -C36 -C23 -C23 -C36  C36  C23 
	      -C12 -C16  C16  C12  C12  C16 -C16 -C12 
	       C12  C16 -C16 -C12 -C12 -C16  C16  C12 
	      -C23 -C36  C36  C23  C23  C36 -C36 -C23 
	       C25  C56 -C56 -C25 -C25 -C56  C56  C25 
	      -C27 -C67  C67  C27  C27  C67 -C67 -C27 
	    */
	    t = coeff[8];	/* C12 */
	    c0 -= t;
	    c3 += t;
	    t = coeff[12];	/* C16 */
	    c1 -= t;
	    c2 += t;
	    break;

	  case 49:
	    /*
	      Coefficient[7][3]
	      
	       C37 -C77 -C17 -C57  C57  C17  C77 -C37 
	      -C35  C57  C15  C55 -C55 -C15 -C57  C35 
	       C33 -C37 -C13 -C35  C35  C13  C37 -C33 
	      -C13  C17  C11  C15 -C15 -C11 -C17  C13 
	       C13 -C17 -C11 -C15  C15  C11  C17 -C13 
	      -C33  C37  C13  C35 -C35 -C13 -C37  C33 
	       C35 -C57 -C15 -C55  C55  C15  C57 -C35 
	      -C37  C77  C17  C57 -C57 -C17 -C77  C37 
	    */
	    t = coeff[20];	/* C11 */
	    d2 += t;    
	    t = coeff[21];	/* C13 */
	    d0 -= t;    
	    t = coeff[22];	/* C15 */
	    d3 += t;    
	    t = coeff[23];	/* C17 */
	    d1 += t;    
	    break;

	  case 50:
	    /*
	      Coefficient[6][4]

	       C46 -C46 -C46  C46  C46 -C46 -C46  C46 
	      -C24  C24  C24 -C24 -C24  C24  C24 -C24 
	       C24 -C24 -C24  C24  C24 -C24 -C24  C24 
	      -C46  C46  C46 -C46 -C46  C46  C46 -C46 
	      -C46  C46  C46 -C46 -C46  C46  C46 -C46 
	       C24 -C24 -C24  C24  C24 -C24 -C24  C24 
	      -C24  C24  C24 -C24 -C24  C24  C24 -C24 
	       C46 -C46 -C46  C46  C46 -C46 -C46  C46 
	    */
	    t = coeff[3];	/* C46 */
	    a0 -= t;
	    a1 += t;
	    a2 += t;
	    a3 -= t;
	    break;

	  case 51:
	    /*
	      Coefficient[5][5]

	       C55 -C15  C57  C35 -C35 -C57  C15 -C55 
	      -C15  C11 -C17 -C13  C13  C17 -C11  C15 
	       C57 -C17  C77  C37 -C37 -C77  C17 -C57 
	       C35 -C13  C37  C33 -C33 -C37  C13 -C35 
	      -C35  C13 -C37 -C33  C33  C37 -C13  C35 
	      -C57  C17 -C77 -C37  C37  C77 -C17  C57 
	       C15 -C11  C17  C13 -C13 -C17  C11 -C15 
	      -C55  C15 -C57 -C35  C35  C57 -C15  C55 
	    */
	    t = coeff[21];	/* C13 */
	    d1 -= t;    
	    t = coeff[24];	/* C33 */
	    d3 += t;    
	    t = coeff[25];	/* C35 */
	    d0 += t;    
	    t = coeff[26];	/* C37 */
	    d2 += t;    
	    break;

	  case 52:
	    /*
	      Coefficient[4][6]

	       C46 -C24  C24 -C46 -C46  C24 -C24  C46 
	      -C46  C24 -C24  C46  C46 -C24  C24 -C46 
	      -C46  C24 -C24  C46  C46 -C24  C24 -C46 
	       C46 -C24  C24 -C46 -C46  C24 -C24  C46 
	       C46 -C24  C24 -C46 -C46  C24 -C24  C46 
	      -C46  C24 -C24  C46  C46 -C24  C24 -C46 
	      -C46  C24 -C24  C46  C46 -C24  C24 -C46 
	      C46 -C24  C24 -C46 -C46  C24 -C24  C46 
	    */
	    t = coeff[2];	/* C24 */
	    a1 -= t;
	    a2 += t;
	    t = coeff[3];	/* C46 */
	    a0 += t;
	    a3 -= t;
	    break;

	  case 53:
	    /*
	      Coefficient[3][7]

	       C37 -C35  C33 -C13  C13 -C33  C35 -C37 
	      -C77  C57 -C37  C17 -C17  C37 -C57  C77 
	      -C17  C15 -C13  C11 -C11  C13 -C15  C17 
	      -C57  C55 -C35  C15 -C15  C35 -C55  C57 
	       C57 -C55  C35 -C15  C15 -C35  C55 -C57 
	       C17 -C15  C13 -C11  C11 -C13  C15 -C17 
	       C77 -C57  C37 -C17  C17 -C37  C57 -C77 
	      -C37  C35 -C33  C13 -C13  C33 -C35  C37 
	    */
	    t = coeff[22];	/* C15 */
	    d3 += t;    
	    t = coeff[25];	/* C35 */
	    d2 -= t;    
	    t = coeff[27];	/* C55 */
	    d1 += t;    
	    t = coeff[28];	/* C57 */
	    d0 -= t;    
	    break;

	  case 54:
	    /*
	      Coefficient[4][7]

	       C47 -C45  C34 -C14  C14 -C34  C45 -C47 
	      -C47  C45 -C34  C14 -C14  C34 -C45  C47 
	      -C47  C45 -C34  C14 -C14  C34 -C45  C47 
	       C47 -C45  C34 -C14  C14 -C34  C45 -C47 
	       C47 -C45  C34 -C14  C14 -C34  C45 -C47 
	      -C47  C45 -C34  C14 -C14  C34 -C45  C47 
	      -C47  C45 -C34  C14 -C14  C34 -C45  C47 
	       C47 -C45  C34 -C14  C14 -C34  C45 -C47 
	    */
	    t = coeff[4];	/* C14 */
	    b3 -= t;    
	    t = coeff[5];	/* C34 */
	    b2 += t;    
	    t = coeff[6];	/* C45 */
	    b1 -= t;    
	    t = coeff[7];	/* C47 */
	    b0 += t;    
	    break;

	  case 55:
	    /*
	      Coefficient[5][6]

	       C56 -C25  C25 -C56 -C56  C25 -C25  C56 
	      -C16  C12 -C12  C16  C16 -C12  C12 -C16 
	       C67 -C27  C27 -C67 -C67  C27 -C27  C67 
	       C36 -C23  C23 -C36 -C36  C23 -C23  C36 
	      -C36  C23 -C23  C36  C36 -C23  C23 -C36 
	      -C67  C27 -C27  C67  C67 -C27  C27 -C67 
	       C16 -C12  C12 -C16 -C16  C12 -C12  C16 
	      -C56  C25 -C25  C56  C56 -C25  C25 -C56 
	    */
	    t = coeff[9];	/* C23 */
	    c1 -= t;
	    c2 += t;
	    t = coeff[13];	/* C36 */
	    c0 += t;
	    c3 -= t;
	    break;

	  case 56:
	    /*
	      Coefficient[6][5]

	       C56 -C16  C67  C36 -C36 -C67  C16 -C56 
	      -C25  C12 -C27 -C23  C23  C27 -C12  C25 
	       C25 -C12  C27  C23 -C23 -C27  C12 -C25 
	      -C56  C16 -C67 -C36  C36  C67 -C16  C56 
	      -C56  C16 -C67 -C36  C36  C67 -C16  C56 
	       C25 -C12  C27  C23 -C23 -C27  C12 -C25 
	      -C25  C12 -C27 -C23  C23  C27 -C12  C25 
	       C56 -C16  C67  C36 -C36 -C67  C16 -C56 
	    */
	    t = coeff[12];	/* C16 */
	    b1 += t;    
	    t = coeff[13];	/* C36 */
	    b3 -= t;    
	    t = coeff[14];	/* C56 */
	    b0 -= t;    
	    t = coeff[15];	/* C67 */
	    b2 -= t;    
	    break;

	  case 57:
	    /*
	      Coefficient[7][4]

	       C47 -C47 -C47  C47  C47 -C47 -C47  C47 
	      -C45  C45  C45 -C45 -C45  C45  C45 -C45 
	       C34 -C34 -C34  C34  C34 -C34 -C34  C34 
	      -C14  C14  C14 -C14 -C14  C14  C14 -C14 
	       C14 -C14 -C14  C14  C14 -C14 -C14  C14 
	      -C34  C34  C34 -C34 -C34  C34  C34 -C34 
	       C45 -C45 -C45  C45  C45 -C45 -C45  C45 
	      -C47  C47  C47 -C47 -C47  C47  C47 -C47 
	    */
	    t = coeff[4];	/* C14 */
	    c0 -= t;
	    c1 += t;
	    c2 += t;
	    c3 -= t;
	    break;

	  case 58:
	    /*
	      Coefficient[7][5]

	       C57 -C17  C77  C37 -C37 -C77  C17 -C57 
	      -C55  C15 -C57 -C35  C35  C57 -C15  C55 
	       C35 -C13  C37  C33 -C33 -C37  C13 -C35 
	      -C15  C11 -C17 -C13  C13  C17 -C11  C15 
	       C15 -C11  C17  C13 -C13 -C17  C11 -C15 
	      -C35  C13 -C37 -C33  C33  C37 -C13  C35 
	       C55 -C15  C57  C35 -C35 -C57  C15 -C55 
	      -C57  C17 -C77 -C37  C37  C77 -C17  C57 
	    */
	    t = coeff[20];	/* C11 */
	    d1 += t;    
	    t = coeff[21];	/* C13 */
	    d3 -= t;    
	    t = coeff[22];	/* C15 */
	    d0 -= t;    
	    t = coeff[23];	/* C17 */
	    d2 -= t;    
	    break;

	  case 59:
	    /*
	      Coefficient[6][6]

	       C66 -C26  C26 -C66 -C66  C26 -C26  C66 
	      -C26  C22 -C22  C26  C26 -C22  C22 -C26 
	       C26 -C22  C22 -C26 -C26  C22 -C22  C26 
	      -C66  C26 -C26  C66  C66 -C26  C26 -C66 
	      -C66  C26 -C26  C66  C66 -C26  C26 -C66 
	       C26 -C22  C22 -C26 -C26  C22 -C22  C26 
	      -C26  C22 -C22  C26  C26 -C22  C22 -C26 
	       C66 -C26  C26 -C66 -C66  C26 -C26  C66 
	    */
	    t = coeff[18];	/* C26 */
	    a1 += t;
	    a2 -= t;
	    t = coeff[19];	/* C66 */
	    a0 -= t;
	    a3 += t;
	    break;

	  case 60:
	    /*
	      Coefficient[5][7]

	       C57 -C55  C35 -C15  C15 -C35  C55 -C57 
	      -C17  C15 -C13  C11 -C11  C13 -C15  C17 
	       C77 -C57  C37 -C17  C17 -C37  C57 -C77 
	       C37 -C35  C33 -C13  C13 -C33  C35 -C37 
	      -C37  C35 -C33  C13 -C13  C33 -C35  C37 
	      -C77  C57 -C37  C17 -C17  C37 -C57  C77 
	       C17 -C15  C13 -C11  C11 -C13  C15 -C17 
	      -C57  C55 -C35  C15 -C15  C35 -C55  C57 
	    */
	    t = coeff[21];	/* C13 */
	    d3 -= t;    
	    t = coeff[24];	/* C33 */
	    d2 += t;    
	    t = coeff[25];	/* C35 */
	    d1 -= t;    
	    t = coeff[26];	/* C37 */
	    d0 += t;    
	    break;

	  case 61:
	    /*
	      Coefficient[6][7]

	       C67 -C56  C36 -C16  C16 -C36  C56 -C67 
	      -C27  C25 -C23  C12 -C12  C23 -C25  C27 
	       C27 -C25  C23 -C12  C12 -C23  C25 -C27 
	      -C67  C56 -C36  C16 -C16  C36 -C56  C67 
	      -C67  C56 -C36  C16 -C16  C36 -C56  C67 
	       C27 -C25  C23 -C12  C12 -C23  C25 -C27 
	      -C27  C25 -C23  C12 -C12  C23 -C25  C27 
	       C67 -C56  C36 -C16  C16 -C36  C56 -C67 
	    */
	    t = coeff[12];	/* C16 */
	    b3 += t;    
	    t = coeff[13];	/* C36 */
	    b2 -= t;    
	    t = coeff[14];	/* C56 */
	    b1 += t;    
	    t = coeff[15];	/* C67 */
	    b0 -= t;    
	    break;

	  case 62:
	    /*
	      Coefficient[7][6]

	       C67 -C27  C27 -C67 -C67  C27 -C27  C67 
	      -C56  C25 -C25  C56  C56 -C25  C25 -C56 
	       C36 -C23  C23 -C36 -C36  C23 -C23  C36 
	      -C16  C12 -C12  C16  C16 -C12  C12 -C16 
	       C16 -C12  C12 -C16 -C16  C12 -C12  C16 
	      -C36  C23 -C23  C36  C36 -C23  C23 -C36 
	       C56 -C25  C25 -C56 -C56  C25 -C25  C56 
	      -C67  C27 -C27  C67  C67 -C27  C27 -C67 
	    */
	    t = coeff[8];	/* C12 */
	    c1 += t;
	    c2 -= t;
	    t = coeff[12];	/* C16 */
	    c0 -= t;
	    c3 += t;
	    break;

	  case 63:
	    /*
	      Coefficient[7][7]

	       C77 -C57  C37 -C17  C17 -C37  C57 -C77 
	      -C57  C55 -C35  C15 -C15  C35 -C55  C57 
	       C37 -C35  C33 -C13  C13 -C33  C35 -C37 
	      -C17  C15 -C13  C11 -C11  C13 -C15  C17 
	       C17 -C15  C13 -C11  C11 -C13  C15 -C17 
	      -C37  C35 -C33  C13 -C13  C33 -C35  C37 
	       C57 -C55  C35 -C15  C15 -C35  C55 -C57 
	      -C77  C57 -C37  C17 -C17  C37 -C57  C77 
	    */
	    t = coeff[20];	/* C11 */
	    d3 += t;    
	    t = coeff[21];	/* C13 */
	    d2 -= t;    
	    t = coeff[22];	/* C15 */
	    d1 += t;    
	    t = coeff[23];	/* C17 */
	    d0 -= t;    
	    break;
	}
	
	i += 1;
	t = coefflist[i];

    } while (t != 0);

    t = a0 + b0;
    i = a0 - b0;
    a0 = c0 + d0;
    b0 = c0 - d0;
    c0 = t - a0;
    a0 = t + a0;
    d0 = i - b0;
    b0 = i + b0;
    a0 = ((unsigned int) a0) >> 16;
    b0 = ((unsigned int) b0) >> 16;
    c0 = ((unsigned int) c0) >> 16;
    d0 = ((unsigned int) d0) >> 16;

    t = a1 + b1;
    i = a1 - b1;
    a1 = c1 + d1;
    b1 = c1 - d1;
    c1 = t - a1;
    a1 = t + a1;
    d1 = i - b1;
    b1 = i + b1;
    a1 = ((unsigned int) a1) >> 16;
    b1 = ((unsigned int) b1) >> 16;
    c1 = ((unsigned int) c1) >> 16;
    d1 = ((unsigned int) d1) >> 16;

    t = a2 + b2;
    i = a2 - b2;
    a2 = c2 + d2;
    b2 = c2 - d2;
    c2 = t - a2;
    a2 = t + a2;
    d2 = i - b2;
    b2 = i + b2;
    a2 = ((unsigned int) a2) >> 16;
    b2 = ((unsigned int) b2) >> 16;
    c2 = ((unsigned int) c2) >> 16;
    d2 = ((unsigned int) d2) >> 16;

    t = a3 + b3;
    i = a3 - b3;
    a3 = c3 + d3;
    b3 = c3 - d3;
    c3 = t - a3;
    a3 = t + a3;
    d3 = i - b3;
    b3 = i + b3;
    a3 = ((unsigned int) a3) >> 16;
    b3 = ((unsigned int) b3) >> 16;
    c3 = ((unsigned int) c3) >> 16;
    d3 = ((unsigned int) d3) >> 16;

    a0 = PUT_SHORTS_INTO_INT(a0,a1);
    result[12] = a0;
    a2 = PUT_SHORTS_INTO_INT(a2,a3);
    result[13] = a2;
    b2 = PUT_SHORTS_INTO_INT(b3,b2);
    result[14] = b2;
    b0 = PUT_SHORTS_INTO_INT(b1,b0);
    result[15] = b0;
    c0 = PUT_SHORTS_INTO_INT(c0,c1);
    result[16] = c0;
    c2 = PUT_SHORTS_INTO_INT(c2,c3);
    result[17] = c2;
    d2 = PUT_SHORTS_INTO_INT(d3,d2);
    result[18] = d2;
    d0 = PUT_SHORTS_INTO_INT(d1,d0);
    result[19] = d0;
}
