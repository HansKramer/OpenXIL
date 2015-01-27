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
//  File:       IdctCoeffSplat.cc
//  Project:    XIL
//  Revision:   1.3
//  Last Mod:   10:16:12, 03/10/00
//
//  Description:
//
//    From splat.c 
//
//
//------------------------------------------------------------------------
//    COPYRIGHT
//------------------------------------------------------------------------
#pragma ident   "@(#)IdctCoeffSplat.cc	1.3\t00/03/10  "



#include <xil/xilGPI.hh>
#include "IdctSplatter.hh"

int splat(int *result, int count, int *coefflist[64])
{
    register int tmp;
    register int *coeff;
    register int q0, q1, q2, q3;
    register int q4, q5, q6, q7;
    register int flag = 0;


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

    q0 = 0;     q1 = 0;     q2 = 0;     q3 = 0;
    q4 = 0;     q5 = 0;     q6 = 0;     q7 = 0;

    if ((coeff = coefflist[0]) != NULL) {
	tmp = coeff[0];	/* C44 */
	q0 += tmp;    	q1 += tmp;    	q2 += tmp;    	q3 += tmp;    
	q4 += tmp;    	q5 += tmp;    	q6 += tmp;    	q7 += tmp;    
	flag |= 1;
    }


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

    if (count < 3)
	goto Lsave0;

    if ((coeff = coefflist[3]) != NULL) {
	tmp = coeff[0];	/* C24 */
	q0 += tmp;    	q1 += tmp;    	q2 += tmp;    	q3 += tmp;    
	tmp = coeff[1];	/* C46 */
	q4 += tmp;    	q5 += tmp;    	q6 += tmp;    	q7 += tmp;    
	flag |= 1;
    }


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

    if (count < 5)
	goto Lsave0;

    if ((coeff = coefflist[5]) != NULL) {
	tmp = coeff[0];	/* C24 */
	q0 += tmp;    	q3 -= tmp;    
	q4 += tmp;    	q7 -= tmp;    
	tmp = coeff[1];	/* C46 */
	q1 += tmp;    	q2 -= tmp;    
	q5 += tmp;    	q6 -= tmp;    
	flag |= 1;
    }


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

    if (count < 10)
	goto Lsave0;

    if ((coeff = coefflist[10]) != NULL) {
	tmp = coeff[0];	/* C44 */
	q0 += tmp;    	q1 += tmp;    	q2 += tmp;    	q3 += tmp;    
	q4 -= tmp;    	q5 -= tmp;    	q6 -= tmp;    	q7 -= tmp;    
	flag |= 1;
    }


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

    if (count < 12)
	goto Lsave0;

    if ((coeff = coefflist[12]) != NULL) {
	tmp = coeff[0];	/* C22 */
	q0 += tmp;    	q3 -= tmp;    
	tmp = coeff[1];	/* C26 */
	q1 += tmp;    	q2 -= tmp;    
	q4 += tmp;    	q7 -= tmp;    
	tmp = coeff[2];	/* C66 */
	q5 += tmp;    	q6 -= tmp;    
	flag |= 1;
    }


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

    if (count < 14)
	goto Lsave0;

    if ((coeff = coefflist[14]) != NULL) {
	tmp = coeff[0];	/* C44 */
	q0 += tmp;    	q1 -= tmp;    	q2 -= tmp;    	q3 += tmp;    
	q4 += tmp;    	q5 -= tmp;    	q6 -= tmp;    	q7 += tmp;    
	flag |= 1;
    }


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

    if (count < 21)
	goto Lsave0;

    if ((coeff = coefflist[21]) != NULL) {
	tmp = coeff[0];	/* C24 */
	q4 -= tmp;    	q5 -= tmp;    	q6 -= tmp;    	q7 -= tmp;    
	tmp = coeff[1];	/* C46 */
	q0 += tmp;    	q1 += tmp;    	q2 += tmp;    	q3 += tmp;    
	flag |= 1;
    }


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

    if (count < 23)
	goto Lsave0;

    if ((coeff = coefflist[23]) != NULL) {
	tmp = coeff[0];	/* C24 */
	q0 += tmp;    	q3 -= tmp;    
	q4 -= tmp;    	q7 += tmp;    
	tmp = coeff[1];	/* C46 */
	q1 += tmp;    	q2 -= tmp;    
	q5 -= tmp;    	q6 += tmp;    
	flag |= 1;
    }


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

    if (count < 25)
	goto Lsave0;

    if ((coeff = coefflist[25]) != NULL) {
	tmp = coeff[0];	/* C24 */
	q0 += tmp;    	q1 -= tmp;    	q2 -= tmp;    	q3 += tmp;    
	tmp = coeff[1];	/* C46 */
	q4 += tmp;    	q5 -= tmp;    	q6 -= tmp;    	q7 += tmp;    
	flag |= 1;
    }


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

    if (count < 27)
	goto Lsave0;

    if ((coeff = coefflist[27]) != NULL) {
	tmp = coeff[0];	/* C24 */
	q1 -= tmp;    	q2 += tmp;    
	q5 -= tmp;    	q6 += tmp;    
	tmp = coeff[1];	/* C46 */
	q0 += tmp;    	q3 -= tmp;    
	q4 += tmp;    	q7 -= tmp;    
	flag |= 1;
    }


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

    if (count < 37)
	goto Lsave0;

    if ((coeff = coefflist[37]) != NULL) {
	tmp = coeff[0];	/* C22 */
	q4 -= tmp;    	q7 += tmp;    
	tmp = coeff[1];	/* C26 */
	q0 += tmp;    	q3 -= tmp;    
	q5 -= tmp;    	q6 += tmp;    
	tmp = coeff[2];	/* C66 */
	q1 += tmp;    	q2 -= tmp;    
	flag |= 1;
    }


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

    if (count < 39)
	goto Lsave0;

    if ((coeff = coefflist[39]) != NULL) {
	tmp = coeff[0];	/* C44 */
	q0 += tmp;    	q1 -= tmp;    	q2 -= tmp;    	q3 += tmp;    
	q4 -= tmp;    	q5 += tmp;    	q6 += tmp;    	q7 -= tmp;    
	flag |= 1;
    }


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

    if (count < 41)
	goto Lsave0;

    if ((coeff = coefflist[41]) != NULL) {
	tmp = coeff[0];	/* C22 */
	q1 -= tmp;    	q2 += tmp;    
	tmp = coeff[1];	/* C26 */
	q0 += tmp;    	q3 -= tmp;    
	q5 -= tmp;    	q6 += tmp;    
	tmp = coeff[2];	/* C66 */
	q4 += tmp;    	q7 -= tmp;    
	flag |= 1;
    }


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

    if (count < 50)
	goto Lsave0;

    if ((coeff = coefflist[50]) != NULL) {
	tmp = coeff[0];	/* C24 */
	q4 -= tmp;    	q5 += tmp;    	q6 += tmp;    	q7 -= tmp;    
	tmp = coeff[1];	/* C46 */
	q0 += tmp;    	q1 -= tmp;    	q2 -= tmp;    	q3 += tmp;    
	flag |= 1;
    }


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

    if (count < 52)
	goto Lsave0;

    if ((coeff = coefflist[52]) != NULL) {
	tmp = coeff[0];	/* C24 */
	q1 -= tmp;    	q2 += tmp;    
	q5 += tmp;    	q6 -= tmp;    
	tmp = coeff[1];	/* C46 */
	q0 += tmp;    	q3 -= tmp;    
	q4 -= tmp;    	q7 += tmp;    
	flag |= 1;
    }


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

    if (count < 59)
	goto Lsave0;

    if ((coeff = coefflist[59]) != NULL) {
	tmp = coeff[0];	/* C22 */
	q5 += tmp;    	q6 -= tmp;    
	tmp = coeff[1];	/* C26 */
	q1 -= tmp;    	q2 += tmp;    
	q4 -= tmp;    	q7 += tmp;    
	tmp = coeff[2];	/* C66 */
	q0 += tmp;    	q3 -= tmp;    
	flag |= 1;
    }


Lsave0: 
    if ((flag & 1) == 0)
	goto Lsplatq1;
    result[ 0] = q0;
    result[ 1] = q1;
    result[ 2] = q2;
    result[ 3] = q3;
    result[ 4] = q4;
    result[ 5] = q5;
    result[ 6] = q6;
    result[ 7] = q7;




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

    q0 = 0;     q1 = 0;     q2 = 0;     q3 = 0;
    q4 = 0;     q5 = 0;     q6 = 0;     q7 = 0;

    if ((coeff = coefflist[0]) != NULL) {
	tmp = coeff[0];	/* C44 */
	q0 += tmp;    	q1 += tmp;    	q2 += tmp;    	q3 += tmp;    
	q4 += tmp;    	q5 += tmp;    	q6 += tmp;    	q7 += tmp;    
    }


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

    if (count < 3)
	goto Lsave1;

    if ((coeff = coefflist[3]) != NULL) {
	tmp = coeff[0];	/* C24 */
	q4 -= tmp;    	q5 -= tmp;    	q6 -= tmp;    	q7 -= tmp;    
	tmp = coeff[1];	/* C46 */
	q0 -= tmp;    	q1 -= tmp;    	q2 -= tmp;    	q3 -= tmp;    
    }


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

    if (count < 5)
	goto Lsave1;

    if ((coeff = coefflist[5]) != NULL) {
	tmp = coeff[0];	/* C24 */
	q0 += tmp;    	q3 -= tmp;    
	q4 += tmp;    	q7 -= tmp;    
	tmp = coeff[1];	/* C46 */
	q1 += tmp;    	q2 -= tmp;    
	q5 += tmp;    	q6 -= tmp;    
    }


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

    if (count < 10)
	goto Lsave1;

    if ((coeff = coefflist[10]) != NULL) {
	tmp = coeff[0];	/* C44 */
	q0 -= tmp;    	q1 -= tmp;    	q2 -= tmp;    	q3 -= tmp;    
	q4 += tmp;    	q5 += tmp;    	q6 += tmp;    	q7 += tmp;    
    }


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

    if (count < 12)
	goto Lsave1;

    if ((coeff = coefflist[12]) != NULL) {
	tmp = coeff[0];	/* C22 */
	q4 -= tmp;    	q7 += tmp;    
	tmp = coeff[1];	/* C26 */
	q0 -= tmp;    	q3 += tmp;    
	q5 -= tmp;    	q6 += tmp;    
	tmp = coeff[2];	/* C66 */
	q1 -= tmp;    	q2 += tmp;    
    }


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

    if (count < 14)
	goto Lsave1;

    if ((coeff = coefflist[14]) != NULL) {
	tmp = coeff[0];	/* C44 */
	q0 += tmp;    	q1 -= tmp;    	q2 -= tmp;    	q3 += tmp;    
	q4 += tmp;    	q5 -= tmp;    	q6 -= tmp;    	q7 += tmp;    
    }


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

    if (count < 21)
	goto Lsave1;

    if ((coeff = coefflist[21]) != NULL) {
	tmp = coeff[0];	/* C24 */
	q0 += tmp;    	q1 += tmp;    	q2 += tmp;    	q3 += tmp;    
	tmp = coeff[1];	/* C46 */
	q4 -= tmp;    	q5 -= tmp;    	q6 -= tmp;    	q7 -= tmp;    
    }


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

    if (count < 23)
	goto Lsave1;

    if ((coeff = coefflist[23]) != NULL) {
	tmp = coeff[0];	/* C24 */
	q0 -= tmp;    	q3 += tmp;    
	q4 += tmp;    	q7 -= tmp;    
	tmp = coeff[1];	/* C46 */
	q1 -= tmp;    	q2 += tmp;    
	q5 += tmp;    	q6 -= tmp;    
    }


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

    if (count < 25)
	goto Lsave1;

    if ((coeff = coefflist[25]) != NULL) {
	tmp = coeff[0];	/* C24 */
	q4 -= tmp;    	q5 += tmp;    	q6 += tmp;    	q7 -= tmp;    
	tmp = coeff[1];	/* C46 */
	q0 -= tmp;    	q1 += tmp;    	q2 += tmp;    	q3 -= tmp;    
    }


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

    if (count < 27)
	goto Lsave1;

    if ((coeff = coefflist[27]) != NULL) {
	tmp = coeff[0];	/* C24 */
	q1 -= tmp;    	q2 += tmp;    
	q5 -= tmp;    	q6 += tmp;    
	tmp = coeff[1];	/* C46 */
	q0 += tmp;    	q3 -= tmp;    
	q4 += tmp;    	q7 -= tmp;    
    }


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

    if (count < 37)
	goto Lsave1;

    if ((coeff = coefflist[37]) != NULL) {
	tmp = coeff[0];	/* C22 */
	q0 += tmp;    	q3 -= tmp;    
	tmp = coeff[1];	/* C26 */
	q1 += tmp;    	q2 -= tmp;    
	q4 -= tmp;    	q7 += tmp;    
	tmp = coeff[2];	/* C66 */
	q5 -= tmp;    	q6 += tmp;    
    }


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

    if (count < 39)
	goto Lsave1;

    if ((coeff = coefflist[39]) != NULL) {
	tmp = coeff[0];	/* C44 */
	q0 -= tmp;    	q1 += tmp;    	q2 += tmp;    	q3 -= tmp;    
	q4 += tmp;    	q5 -= tmp;    	q6 -= tmp;    	q7 += tmp;    
    }


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

    if (count < 41)
	goto Lsave1;

    if ((coeff = coefflist[41]) != NULL) {
	tmp = coeff[0];	/* C22 */
	q5 += tmp;    	q6 -= tmp;    
	tmp = coeff[1];	/* C26 */
	q1 += tmp;    	q2 -= tmp;    
	q4 -= tmp;    	q7 += tmp;    
	tmp = coeff[2];	/* C66 */
	q0 -= tmp;    	q3 += tmp;    
    }


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

    if (count < 50)
	goto Lsave1;

    if ((coeff = coefflist[50]) != NULL) {
	tmp = coeff[0];	/* C24 */
	q0 += tmp;    	q1 -= tmp;    	q2 -= tmp;    	q3 += tmp;    
	tmp = coeff[1];	/* C46 */
	q4 -= tmp;    	q5 += tmp;    	q6 += tmp;    	q7 -= tmp;    
    }


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

    if (count < 52)
	goto Lsave1;

    if ((coeff = coefflist[52]) != NULL) {
	tmp = coeff[0];	/* C24 */
	q1 += tmp;    	q2 -= tmp;    
	q5 -= tmp;    	q6 += tmp;    
	tmp = coeff[1];	/* C46 */
	q0 -= tmp;    	q3 += tmp;    
	q4 += tmp;    	q7 -= tmp;    
    }


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

    if (count < 59)
	goto Lsave1;

    if ((coeff = coefflist[59]) != NULL) {
	tmp = coeff[0];	/* C22 */
	q1 -= tmp;    	q2 += tmp;    
	tmp = coeff[1];	/* C26 */
	q0 += tmp;    	q3 -= tmp;    
	q5 += tmp;    	q6 -= tmp;    
	tmp = coeff[2];	/* C66 */
	q4 -= tmp;    	q7 += tmp;    
    }

Lsave1:
    result[ 8] = q0;
    result[ 9] = q1;
    result[10] = q2;
    result[11] = q3;
    result[12] = q4;
    result[13] = q5;
    result[14] = q6;
    result[15] = q7;

Lsplatq1:
    result += 16;

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

    if (count < 1)
	goto Lsplatq2;

    q0 = 0;     q1 = 0;     q2 = 0;     q3 = 0;
    q4 = 0;     q5 = 0;     q6 = 0;     q7 = 0;

    if ((coeff = coefflist[1]) != NULL) {
	tmp = coeff[0];	/* C14 */
	q0 += tmp;    
	q4 += tmp;    
	tmp = coeff[1];	/* C34 */
	q1 += tmp;    
	q5 += tmp;    
	tmp = coeff[2];	/* C45 */
	q2 += tmp;    
	q6 += tmp;    
	tmp = coeff[3];	/* C47 */
	q3 += tmp;    
	q7 += tmp;    
	flag |= 2;
    }


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

    if (count < 6)
	goto Lsave2;

    if ((coeff = coefflist[6]) != NULL) {
	tmp = coeff[0];	/* C14 */
	q2 -= tmp;    
	q6 -= tmp;    
	tmp = coeff[1];	/* C34 */
	q0 += tmp;    
	q4 += tmp;    
	tmp = coeff[2];	/* C45 */
	q3 -= tmp;    
	q7 -= tmp;    
	tmp = coeff[3];	/* C47 */
	q1 -= tmp;    
	q5 -= tmp;    
	flag |= 2;
    }


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

    if (count < 8)
	goto Lsave2;

    if ((coeff = coefflist[8]) != NULL) {
	tmp = coeff[0];	/* C12 */
	q0 += tmp;    
	tmp = coeff[4];	/* C16 */
	q4 += tmp;    
	tmp = coeff[1];	/* C23 */
	q1 += tmp;    
	tmp = coeff[2];	/* C25 */
	q2 += tmp;    
	tmp = coeff[3];	/* C27 */
	q3 += tmp;    
	tmp = coeff[5];	/* C36 */
	q5 += tmp;    
	tmp = coeff[6];	/* C56 */
	q6 += tmp;    
	tmp = coeff[7];	/* C67 */
	q7 += tmp;    
	flag |= 2;
    }


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

    if (count < 15)
	goto Lsave2;

    if ((coeff = coefflist[15]) != NULL) {
	tmp = coeff[0];	/* C14 */
	q1 -= tmp;    
	q5 -= tmp;    
	tmp = coeff[1];	/* C34 */
	q3 += tmp;    
	q7 += tmp;    
	tmp = coeff[2];	/* C45 */
	q0 += tmp;    
	q4 += tmp;    
	tmp = coeff[3];	/* C47 */
	q2 += tmp;    
	q6 += tmp;    
	flag |= 2;
    }


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

    if (count < 17)
	goto Lsave2;

    if ((coeff = coefflist[17]) != NULL) {
	tmp = coeff[0];	/* C12 */
	q2 -= tmp;    
	tmp = coeff[4];	/* C16 */
	q6 -= tmp;    
	tmp = coeff[1];	/* C23 */
	q0 += tmp;    
	tmp = coeff[2];	/* C25 */
	q3 -= tmp;    
	tmp = coeff[3];	/* C27 */
	q1 -= tmp;    
	tmp = coeff[5];	/* C36 */
	q4 += tmp;    
	tmp = coeff[6];	/* C56 */
	q7 -= tmp;    
	tmp = coeff[7];	/* C67 */
	q5 -= tmp;    
	flag |= 2;
    }


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

    if (count < 19)
	goto Lsave2;

    if ((coeff = coefflist[19]) != NULL) {
	tmp = coeff[0];	/* C14 */
	q0 += tmp;    
	q4 -= tmp;    
	tmp = coeff[1];	/* C34 */
	q1 += tmp;    
	q5 -= tmp;    
	tmp = coeff[2];	/* C45 */
	q2 += tmp;    
	q6 -= tmp;    
	tmp = coeff[3];	/* C47 */
	q3 += tmp;    
	q7 -= tmp;    
	flag |= 2;
    }


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

    if (count < 28)
	goto Lsave2;

    if ((coeff = coefflist[28]) != NULL) {
	tmp = coeff[0];	/* C14 */
	q3 -= tmp;    
	q7 -= tmp;    
	tmp = coeff[1];	/* C34 */
	q2 += tmp;    
	q6 += tmp;    
	tmp = coeff[2];	/* C45 */
	q1 -= tmp;    
	q5 -= tmp;    
	tmp = coeff[3];	/* C47 */
	q0 += tmp;    
	q4 += tmp;    
	flag |= 2;
    }


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

    if (count < 30)
	goto Lsave2;

    if ((coeff = coefflist[30]) != NULL) {
	tmp = coeff[0];	/* C12 */
	q1 -= tmp;    
	tmp = coeff[4];	/* C16 */
	q5 -= tmp;    
	tmp = coeff[1];	/* C23 */
	q3 += tmp;    
	tmp = coeff[2];	/* C25 */
	q0 += tmp;    
	tmp = coeff[3];	/* C27 */
	q2 += tmp;    
	tmp = coeff[5];	/* C36 */
	q7 += tmp;    
	tmp = coeff[6];	/* C56 */
	q4 += tmp;    
	tmp = coeff[7];	/* C67 */
	q6 += tmp;    
	flag |= 2;
    }


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

    if (count < 32)
	goto Lsave2;

    if ((coeff = coefflist[32]) != NULL) {
	tmp = coeff[0];	/* C14 */
	q2 -= tmp;    
	q6 += tmp;    
	tmp = coeff[1];	/* C34 */
	q0 += tmp;    
	q4 -= tmp;    
	tmp = coeff[2];	/* C45 */
	q3 -= tmp;    
	q7 += tmp;    
	tmp = coeff[3];	/* C47 */
	q1 -= tmp;    
	q5 += tmp;    
	flag |= 2;
    }


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

    if (count < 34)
	goto Lsave2;

    if ((coeff = coefflist[34]) != NULL) {
	tmp = coeff[0];	/* C12 */
	q4 -= tmp;    
	tmp = coeff[4];	/* C16 */
	q0 += tmp;    
	tmp = coeff[1];	/* C23 */
	q5 -= tmp;    
	tmp = coeff[2];	/* C25 */
	q6 -= tmp;    
	tmp = coeff[3];	/* C27 */
	q7 -= tmp;    
	tmp = coeff[5];	/* C36 */
	q1 += tmp;    
	tmp = coeff[6];	/* C56 */
	q2 += tmp;    
	tmp = coeff[7];	/* C67 */
	q3 += tmp;    
	flag |= 2;
    }


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

    if (count < 43)
	goto Lsave2;

    if ((coeff = coefflist[43]) != NULL) {
	tmp = coeff[0];	/* C12 */
	q3 -= tmp;    
	tmp = coeff[4];	/* C16 */
	q7 -= tmp;    
	tmp = coeff[1];	/* C23 */
	q2 += tmp;    
	tmp = coeff[2];	/* C25 */
	q1 -= tmp;    
	tmp = coeff[3];	/* C27 */
	q0 += tmp;    
	tmp = coeff[5];	/* C36 */
	q6 += tmp;    
	tmp = coeff[6];	/* C56 */
	q5 -= tmp;    
	tmp = coeff[7];	/* C67 */
	q4 += tmp;    
	flag |= 2;
    }


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

    if (count < 45)
	goto Lsave2;

    if ((coeff = coefflist[45]) != NULL) {
	tmp = coeff[0];	/* C14 */
	q1 -= tmp;    
	q5 += tmp;    
	tmp = coeff[1];	/* C34 */
	q3 += tmp;    
	q7 -= tmp;    
	tmp = coeff[2];	/* C45 */
	q0 += tmp;    
	q4 -= tmp;    
	tmp = coeff[3];	/* C47 */
	q2 += tmp;    
	q6 -= tmp;    
	flag |= 2;
    }


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

    if (count < 47)
	goto Lsave2;

    if ((coeff = coefflist[47]) != NULL) {
	tmp = coeff[0];	/* C12 */
	q6 += tmp;    
	tmp = coeff[4];	/* C16 */
	q2 -= tmp;    
	tmp = coeff[1];	/* C23 */
	q4 -= tmp;    
	tmp = coeff[2];	/* C25 */
	q7 += tmp;    
	tmp = coeff[3];	/* C27 */
	q5 += tmp;    
	tmp = coeff[5];	/* C36 */
	q0 += tmp;    
	tmp = coeff[6];	/* C56 */
	q3 -= tmp;    
	tmp = coeff[7];	/* C67 */
	q1 -= tmp;    
	flag |= 2;
    }


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

    if (count < 54)
	goto Lsave2;

    if ((coeff = coefflist[54]) != NULL) {
	tmp = coeff[0];	/* C14 */
	q3 -= tmp;    
	q7 += tmp;    
	tmp = coeff[1];	/* C34 */
	q2 += tmp;    
	q6 -= tmp;    
	tmp = coeff[2];	/* C45 */
	q1 -= tmp;    
	q5 += tmp;    
	tmp = coeff[3];	/* C47 */
	q0 += tmp;    
	q4 -= tmp;    
	flag |= 2;
    }


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

    if (count < 56)
	goto Lsave2;

    if ((coeff = coefflist[56]) != NULL) {
	tmp = coeff[0];	/* C12 */
	q5 += tmp;    
	tmp = coeff[4];	/* C16 */
	q1 -= tmp;    
	tmp = coeff[1];	/* C23 */
	q7 -= tmp;    
	tmp = coeff[2];	/* C25 */
	q4 -= tmp;    
	tmp = coeff[3];	/* C27 */
	q6 -= tmp;    
	tmp = coeff[5];	/* C36 */
	q3 += tmp;    
	tmp = coeff[6];	/* C56 */
	q0 += tmp;    
	tmp = coeff[7];	/* C67 */
	q2 += tmp;    
	flag |= 2;
    }


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

    if (count < 61)
	goto Lsave2;

    if ((coeff = coefflist[61]) != NULL) {
	tmp = coeff[0];	/* C12 */
	q7 += tmp;    
	tmp = coeff[4];	/* C16 */
	q3 -= tmp;    
	tmp = coeff[1];	/* C23 */
	q6 -= tmp;    
	tmp = coeff[2];	/* C25 */
	q5 += tmp;    
	tmp = coeff[3];	/* C27 */
	q4 -= tmp;    
	tmp = coeff[5];	/* C36 */
	q2 += tmp;    
	tmp = coeff[6];	/* C56 */
	q1 -= tmp;    
	tmp = coeff[7];	/* C67 */
	q0 += tmp;    
	flag |= 2;
    }

Lsave2:
    if ((flag & 2) == 0)
	goto Lsplatq2;
    result[ 0] = q0;
    result[ 1] = q1;
    result[ 2] = q2;
    result[ 3] = q3;
    result[ 4] = q4;
    result[ 5] = q5;
    result[ 6] = q6;
    result[ 7] = q7;




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

    q0 = 0;     q1 = 0;     q2 = 0;     q3 = 0;
    q4 = 0;     q5 = 0;     q6 = 0;     q7 = 0;

    if ((coeff = coefflist[1]) != NULL) {
	tmp = coeff[0];	/* C14 */
	q0 += tmp;    
	q4 += tmp;    
	tmp = coeff[1];	/* C34 */
	q1 += tmp;    
	q5 += tmp;    
	tmp = coeff[2];	/* C45 */
	q2 += tmp;    
	q6 += tmp;    
	tmp = coeff[3];	/* C47 */
	q3 += tmp;    
	q7 += tmp;    
    }


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

    if (count < 6)
	goto Lsave3;

    if ((coeff = coefflist[6]) != NULL) {
	tmp = coeff[0];	/* C14 */
	q2 -= tmp;    
	q6 -= tmp;    
	tmp = coeff[1];	/* C34 */
	q0 += tmp;    
	q4 += tmp;    
	tmp = coeff[2];	/* C45 */
	q3 -= tmp;    
	q7 -= tmp;    
	tmp = coeff[3];	/* C47 */
	q1 -= tmp;    
	q5 -= tmp;    
    }


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

    if (count < 8)
	goto Lsave3;

    if ((coeff = coefflist[8]) != NULL) {
	tmp = coeff[0];	/* C12 */
	q4 -= tmp;    
	tmp = coeff[4];	/* C16 */
	q0 -= tmp;    
	tmp = coeff[1];	/* C23 */
	q5 -= tmp;    
	tmp = coeff[2];	/* C25 */
	q6 -= tmp;    
	tmp = coeff[3];	/* C27 */
	q7 -= tmp;    
	tmp = coeff[5];	/* C36 */
	q1 -= tmp;    
	tmp = coeff[6];	/* C56 */
	q2 -= tmp;    
	tmp = coeff[7];	/* C67 */
	q3 -= tmp;    
    }


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

    if (count < 15)
	goto Lsave3;

    if ((coeff = coefflist[15]) != NULL) {
	tmp = coeff[0];	/* C14 */
	q1 -= tmp;    
	q5 -= tmp;    
	tmp = coeff[1];	/* C34 */
	q3 += tmp;    
	q7 += tmp;    
	tmp = coeff[2];	/* C45 */
	q0 += tmp;    
	q4 += tmp;    
	tmp = coeff[3];	/* C47 */
	q2 += tmp;    
	q6 += tmp;    
    }


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

    if (count < 17)
	goto Lsave3;

    if ((coeff = coefflist[17]) != NULL) {
	tmp = coeff[0];	/* C12 */
	q6 += tmp;    
	tmp = coeff[4];	/* C16 */
	q2 += tmp;    
	tmp = coeff[1];	/* C23 */
	q4 -= tmp;    
	tmp = coeff[2];	/* C25 */
	q7 += tmp;    
	tmp = coeff[3];	/* C27 */
	q5 += tmp;    
	tmp = coeff[5];	/* C36 */
	q0 -= tmp;    
	tmp = coeff[6];	/* C56 */
	q3 += tmp;    
	tmp = coeff[7];	/* C67 */
	q1 += tmp;    
    }


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

    if (count < 19)
	goto Lsave3;

    if ((coeff = coefflist[19]) != NULL) {
	tmp = coeff[0];	/* C14 */
	q0 -= tmp;    
	q4 += tmp;    
	tmp = coeff[1];	/* C34 */
	q1 -= tmp;    
	q5 += tmp;    
	tmp = coeff[2];	/* C45 */
	q2 -= tmp;    
	q6 += tmp;    
	tmp = coeff[3];	/* C47 */
	q3 -= tmp;    
	q7 += tmp;    
    }


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

    if (count < 28)
	goto Lsave3;

    if ((coeff = coefflist[28]) != NULL) {
	tmp = coeff[0];	/* C14 */
	q3 -= tmp;    
	q7 -= tmp;    
	tmp = coeff[1];	/* C34 */
	q2 += tmp;    
	q6 += tmp;    
	tmp = coeff[2];	/* C45 */
	q1 -= tmp;    
	q5 -= tmp;    
	tmp = coeff[3];	/* C47 */
	q0 += tmp;    
	q4 += tmp;    
    }


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

    if (count < 30)
	goto Lsave3;

    if ((coeff = coefflist[30]) != NULL) {
	tmp = coeff[0];	/* C12 */
	q5 += tmp;    
	tmp = coeff[4];	/* C16 */
	q1 += tmp;    
	tmp = coeff[1];	/* C23 */
	q7 -= tmp;    
	tmp = coeff[2];	/* C25 */
	q4 -= tmp;    
	tmp = coeff[3];	/* C27 */
	q6 -= tmp;    
	tmp = coeff[5];	/* C36 */
	q3 -= tmp;    
	tmp = coeff[6];	/* C56 */
	q0 -= tmp;    
	tmp = coeff[7];	/* C67 */
	q2 -= tmp;    
    }


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

    if (count < 32)
	goto Lsave3;

    if ((coeff = coefflist[32]) != NULL) {
	tmp = coeff[0];	/* C14 */
	q2 += tmp;    
	q6 -= tmp;    
	tmp = coeff[1];	/* C34 */
	q0 -= tmp;    
	q4 += tmp;    
	tmp = coeff[2];	/* C45 */
	q3 += tmp;    
	q7 -= tmp;    
	tmp = coeff[3];	/* C47 */
	q1 += tmp;    
	q5 -= tmp;    
    }


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

    if (count < 34)
	goto Lsave3;

    if ((coeff = coefflist[34]) != NULL) {
	tmp = coeff[0];	/* C12 */
	q0 += tmp;    
	tmp = coeff[4];	/* C16 */
	q4 -= tmp;    
	tmp = coeff[1];	/* C23 */
	q1 += tmp;    
	tmp = coeff[2];	/* C25 */
	q2 += tmp;    
	tmp = coeff[3];	/* C27 */
	q3 += tmp;    
	tmp = coeff[5];	/* C36 */
	q5 -= tmp;    
	tmp = coeff[6];	/* C56 */
	q6 -= tmp;    
	tmp = coeff[7];	/* C67 */
	q7 -= tmp;    
    }


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

    if (count < 43)
	goto Lsave3;

    if ((coeff = coefflist[43]) != NULL) {
	tmp = coeff[0];	/* C12 */
	q7 += tmp;    
	tmp = coeff[4];	/* C16 */
	q3 += tmp;    
	tmp = coeff[1];	/* C23 */
	q6 -= tmp;    
	tmp = coeff[2];	/* C25 */
	q5 += tmp;    
	tmp = coeff[3];	/* C27 */
	q4 -= tmp;    
	tmp = coeff[5];	/* C36 */
	q2 -= tmp;    
	tmp = coeff[6];	/* C56 */
	q1 += tmp;    
	tmp = coeff[7];	/* C67 */
	q0 -= tmp;    
    }


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

    if (count < 45)
	goto Lsave3;

    if ((coeff = coefflist[45]) != NULL) {
	tmp = coeff[0];	/* C14 */
	q1 += tmp;    
	q5 -= tmp;    
	tmp = coeff[1];	/* C34 */
	q3 -= tmp;    
	q7 += tmp;    
	tmp = coeff[2];	/* C45 */
	q0 -= tmp;    
	q4 += tmp;    
	tmp = coeff[3];	/* C47 */
	q2 -= tmp;    
	q6 += tmp;    
    }


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

    if (count < 47)
	goto Lsave3;

    if ((coeff = coefflist[47]) != NULL) {
	tmp = coeff[0];	/* C12 */
	q2 -= tmp;    
	tmp = coeff[4];	/* C16 */
	q6 += tmp;    
	tmp = coeff[1];	/* C23 */
	q0 += tmp;    
	tmp = coeff[2];	/* C25 */
	q3 -= tmp;    
	tmp = coeff[3];	/* C27 */
	q1 -= tmp;    
	tmp = coeff[5];	/* C36 */
	q4 -= tmp;    
	tmp = coeff[6];	/* C56 */
	q7 += tmp;    
	tmp = coeff[7];	/* C67 */
	q5 += tmp;    
    }


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

    if (count < 54)
	goto Lsave3;

    if ((coeff = coefflist[54]) != NULL) {
	tmp = coeff[0];	/* C14 */
	q3 += tmp;    
	q7 -= tmp;    
	tmp = coeff[1];	/* C34 */
	q2 -= tmp;    
	q6 += tmp;    
	tmp = coeff[2];	/* C45 */
	q1 += tmp;    
	q5 -= tmp;    
	tmp = coeff[3];	/* C47 */
	q0 -= tmp;    
	q4 += tmp;    
    }


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

    if (count < 56)
	goto Lsave3;

    if ((coeff = coefflist[56]) != NULL) {
	tmp = coeff[0];	/* C12 */
	q1 -= tmp;    
	tmp = coeff[4];	/* C16 */
	q5 += tmp;    
	tmp = coeff[1];	/* C23 */
	q3 += tmp;    
	tmp = coeff[2];	/* C25 */
	q0 += tmp;    
	tmp = coeff[3];	/* C27 */
	q2 += tmp;    
	tmp = coeff[5];	/* C36 */
	q7 -= tmp;    
	tmp = coeff[6];	/* C56 */
	q4 -= tmp;    
	tmp = coeff[7];	/* C67 */
	q6 -= tmp;    
    }


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

    if (count < 61)
	goto Lsave3;

    if ((coeff = coefflist[61]) != NULL) {
	tmp = coeff[0];	/* C12 */
	q3 -= tmp;    
	tmp = coeff[4];	/* C16 */
	q7 += tmp;    
	tmp = coeff[1];	/* C23 */
	q2 += tmp;    
	tmp = coeff[2];	/* C25 */
	q1 -= tmp;    
	tmp = coeff[3];	/* C27 */
	q0 += tmp;    
	tmp = coeff[5];	/* C36 */
	q6 -= tmp;    
	tmp = coeff[6];	/* C56 */
	q5 += tmp;    
	tmp = coeff[7];	/* C67 */
	q4 -= tmp;    
    }

Lsave3:
    result[ 8] = q0;
    result[ 9] = q1;
    result[10] = q2;
    result[11] = q3;
    result[12] = q4;
    result[13] = q5;
    result[14] = q6;
    result[15] = q7;

Lsplatq2:
    result += 16;

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

    if (count < 2)
	goto Lsplatq3;

    q0 = 0;     q1 = 0;     q2 = 0;     q3 = 0;
    q4 = 0;     q5 = 0;     q6 = 0;     q7 = 0;

    if ((coeff = coefflist[2]) != NULL) {
	tmp = coeff[0];	/* C14 */
	q0 += tmp;    	q1 += tmp;    	q2 += tmp;    	q3 += tmp;    
	tmp = coeff[1];	/* C34 */
	q4 += tmp;    	q5 += tmp;    	q6 += tmp;    	q7 += tmp;    
	flag |= 4;
    }


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

    if (count < 7)
	goto Lsave4;

    if ((coeff = coefflist[7]) != NULL) {
	tmp = coeff[0];	/* C12 */
	q0 += tmp;    	q3 -= tmp;    
	tmp = coeff[4];	/* C16 */
	q1 += tmp;    	q2 -= tmp;    
	tmp = coeff[1];	/* C23 */
	q4 += tmp;    	q7 -= tmp;    
	tmp = coeff[5];	/* C36 */
	q5 += tmp;    	q6 -= tmp;    
	flag |= 4;
    }


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

    if (count < 9)
	goto Lsave4;

    if ((coeff = coefflist[9]) != NULL) {
	tmp = coeff[1];	/* C34 */
	q0 += tmp;    	q1 += tmp;    	q2 += tmp;    	q3 += tmp;    
	tmp = coeff[3];	/* C47 */
	q4 -= tmp;    	q5 -= tmp;    	q6 -= tmp;    	q7 -= tmp;    
	flag |= 4;
    }


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

    if (count < 16)
	goto Lsave4;

    if ((coeff = coefflist[16]) != NULL) {
	tmp = coeff[0];	/* C14 */
	q0 += tmp;    	q1 -= tmp;    	q2 -= tmp;    	q3 += tmp;    
	tmp = coeff[1];	/* C34 */
	q4 += tmp;    	q5 -= tmp;    	q6 -= tmp;    	q7 += tmp;    
	flag |= 4;
    }


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

    if (count < 18)
	goto Lsave4;

    if ((coeff = coefflist[18]) != NULL) {
	tmp = coeff[1];	/* C23 */
	q0 += tmp;    	q3 -= tmp;    
	tmp = coeff[3];	/* C27 */
	q4 -= tmp;    	q7 += tmp;    
	tmp = coeff[5];	/* C36 */
	q1 += tmp;    	q2 -= tmp;    
	tmp = coeff[7];	/* C67 */
	q5 -= tmp;    	q6 += tmp;    
	flag |= 4;
    }


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

    if (count < 20)
	goto Lsave4;

    if ((coeff = coefflist[20]) != NULL) {
	tmp = coeff[0];	/* C14 */
	q4 -= tmp;    	q5 -= tmp;    	q6 -= tmp;    	q7 -= tmp;    
	tmp = coeff[2];	/* C45 */
	q0 += tmp;    	q1 += tmp;    	q2 += tmp;    	q3 += tmp;    
	flag |= 4;
    }


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

    if (count < 29)
	goto Lsave4;

    if ((coeff = coefflist[29]) != NULL) {
	tmp = coeff[0];	/* C12 */
	q1 -= tmp;    	q2 += tmp;    
	tmp = coeff[4];	/* C16 */
	q0 += tmp;    	q3 -= tmp;    
	tmp = coeff[1];	/* C23 */
	q5 -= tmp;    	q6 += tmp;    
	tmp = coeff[5];	/* C36 */
	q4 += tmp;    	q7 -= tmp;    
	flag |= 4;
    }


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

    if (count < 31)
	goto Lsave4;

    if ((coeff = coefflist[31]) != NULL) {
	tmp = coeff[1];	/* C34 */
	q0 += tmp;    	q1 -= tmp;    	q2 -= tmp;    	q3 += tmp;    
	tmp = coeff[3];	/* C47 */
	q4 -= tmp;    	q5 += tmp;    	q6 += tmp;    	q7 -= tmp;    
	flag |= 4;
    }


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

    if (count < 33)
	goto Lsave4;

    if ((coeff = coefflist[33]) != NULL) {
	tmp = coeff[0];	/* C12 */
	q4 -= tmp;    	q7 += tmp;    
	tmp = coeff[4];	/* C16 */
	q5 -= tmp;    	q6 += tmp;    
	tmp = coeff[2];	/* C25 */
	q0 += tmp;    	q3 -= tmp;    
	tmp = coeff[6];	/* C56 */
	q1 += tmp;    	q2 -= tmp;    
	flag |= 4;
    }


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

    if (count < 35)
	goto Lsave4;

    if ((coeff = coefflist[35]) != NULL) {
	tmp = coeff[2];	/* C45 */
	q4 -= tmp;    	q5 -= tmp;    	q6 -= tmp;    	q7 -= tmp;    
	tmp = coeff[3];	/* C47 */
	q0 += tmp;    	q1 += tmp;    	q2 += tmp;    	q3 += tmp;    
	flag |= 4;
    }


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

    if (count < 44)
	goto Lsave4;

    if ((coeff = coefflist[44]) != NULL) {
	tmp = coeff[1];	/* C23 */
	q1 -= tmp;    	q2 += tmp;    
	tmp = coeff[3];	/* C27 */
	q5 += tmp;    	q6 -= tmp;    
	tmp = coeff[5];	/* C36 */
	q0 += tmp;    	q3 -= tmp;    
	tmp = coeff[7];	/* C67 */
	q4 -= tmp;    	q7 += tmp;    
	flag |= 4;
    }


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

    if (count < 46)
	goto Lsave4;

    if ((coeff = coefflist[46]) != NULL) {
	tmp = coeff[0];	/* C14 */
	q4 -= tmp;    	q5 += tmp;    	q6 += tmp;    	q7 -= tmp;    
	tmp = coeff[2];	/* C45 */
	q0 += tmp;    	q1 -= tmp;    	q2 -= tmp;    	q3 += tmp;    
	flag |= 4;
    }


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

    if (count < 48)
	goto Lsave4;

    if ((coeff = coefflist[48]) != NULL) {
	tmp = coeff[2];	/* C25 */
	q4 -= tmp;    	q7 += tmp;    
	tmp = coeff[3];	/* C27 */
	q0 += tmp;    	q3 -= tmp;    
	tmp = coeff[6];	/* C56 */
	q5 -= tmp;    	q6 += tmp;    
	tmp = coeff[7];	/* C67 */
	q1 += tmp;    	q2 -= tmp;    
	flag |= 4;
    }


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

    if (count < 55)
	goto Lsave4;

    if ((coeff = coefflist[55]) != NULL) {
	tmp = coeff[0];	/* C12 */
	q5 += tmp;    	q6 -= tmp;    
	tmp = coeff[4];	/* C16 */
	q4 -= tmp;    	q7 += tmp;    
	tmp = coeff[2];	/* C25 */
	q1 -= tmp;    	q2 += tmp;    
	tmp = coeff[6];	/* C56 */
	q0 += tmp;    	q3 -= tmp;    
	flag |= 4;
    }


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

    if (count < 57)
	goto Lsave4;

    if ((coeff = coefflist[57]) != NULL) {
	tmp = coeff[2];	/* C45 */
	q4 -= tmp;    	q5 += tmp;    	q6 += tmp;    	q7 -= tmp;    
	tmp = coeff[3];	/* C47 */
	q0 += tmp;    	q1 -= tmp;    	q2 -= tmp;    	q3 += tmp;    
	flag |= 4;
    }


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

    if (count < 62)
	goto Lsave4;

    if ((coeff = coefflist[62]) != NULL) {
	tmp = coeff[2];	/* C25 */
	q5 += tmp;    	q6 -= tmp;    
	tmp = coeff[3];	/* C27 */
	q1 -= tmp;    	q2 += tmp;    
	tmp = coeff[6];	/* C56 */
	q4 -= tmp;    	q7 += tmp;    
	tmp = coeff[7];	/* C67 */
	q0 += tmp;    	q3 -= tmp;    
	flag |= 4;
    }

Lsave4:
    if ((flag & 4) == 0)
	goto Lsplatq3;
    result[ 0] = q0;
    result[ 1] = q1;
    result[ 2] = q2;
    result[ 3] = q3;
    result[ 4] = q4;
    result[ 5] = q5;
    result[ 6] = q6;
    result[ 7] = q7;




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

    q0 = 0;     q1 = 0;     q2 = 0;     q3 = 0;
    q4 = 0;     q5 = 0;     q6 = 0;     q7 = 0;

    if ((coeff = coefflist[2]) != NULL) {
	tmp = coeff[2];	/* C45 */
	q0 += tmp;    	q1 += tmp;    	q2 += tmp;    	q3 += tmp;    
	tmp = coeff[3];	/* C47 */
	q4 += tmp;    	q5 += tmp;    	q6 += tmp;    	q7 += tmp;    
    }


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

    if (count < 7)
	goto Lsave5;

    if ((coeff = coefflist[7]) != NULL) {
	tmp = coeff[2];	/* C25 */
	q0 += tmp;    	q3 -= tmp;    
	tmp = coeff[3];	/* C27 */
	q4 += tmp;    	q7 -= tmp;    
	tmp = coeff[6];	/* C56 */
	q1 += tmp;    	q2 -= tmp;    
	tmp = coeff[7];	/* C67 */
	q5 += tmp;    	q6 -= tmp;    
    }


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

    if (count < 9)
	goto Lsave5;

    if ((coeff = coefflist[9]) != NULL) {
	tmp = coeff[0];	/* C14 */
	q0 -= tmp;    	q1 -= tmp;    	q2 -= tmp;    	q3 -= tmp;    
	tmp = coeff[2];	/* C45 */
	q4 -= tmp;    	q5 -= tmp;    	q6 -= tmp;    	q7 -= tmp;    
    }


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

    if (count < 16)
	goto Lsave5;

    if ((coeff = coefflist[16]) != NULL) {
	tmp = coeff[2];	/* C45 */
	q0 += tmp;    	q1 -= tmp;    	q2 -= tmp;    	q3 += tmp;    
	tmp = coeff[3];	/* C47 */
	q4 += tmp;    	q5 -= tmp;    	q6 -= tmp;    	q7 += tmp;    
    }


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

    if (count < 18)
	goto Lsave5;

    if ((coeff = coefflist[18]) != NULL) {
	tmp = coeff[0];	/* C12 */
	q0 -= tmp;    	q3 += tmp;    
	tmp = coeff[4];	/* C16 */
	q1 -= tmp;    	q2 += tmp;    
	tmp = coeff[2];	/* C25 */
	q4 -= tmp;    	q7 += tmp;    
	tmp = coeff[6];	/* C56 */
	q5 -= tmp;    	q6 += tmp;    
    }


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

    if (count < 20)
	goto Lsave5;

    if ((coeff = coefflist[20]) != NULL) {
	tmp = coeff[1];	/* C34 */
	q4 += tmp;    	q5 += tmp;    	q6 += tmp;    	q7 += tmp;    
	tmp = coeff[3];	/* C47 */
	q0 += tmp;    	q1 += tmp;    	q2 += tmp;    	q3 += tmp;    
    }


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

    if (count < 29)
	goto Lsave5;

    if ((coeff = coefflist[29]) != NULL) {
	tmp = coeff[2];	/* C25 */
	q1 -= tmp;    	q2 += tmp;    
	tmp = coeff[3];	/* C27 */
	q5 -= tmp;    	q6 += tmp;    
	tmp = coeff[6];	/* C56 */
	q0 += tmp;    	q3 -= tmp;    
	tmp = coeff[7];	/* C67 */
	q4 += tmp;    	q7 -= tmp;    
    }


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

    if (count < 31)
	goto Lsave5;

    if ((coeff = coefflist[31]) != NULL) {
	tmp = coeff[0];	/* C14 */
	q0 -= tmp;    	q1 += tmp;    	q2 += tmp;    	q3 -= tmp;    
	tmp = coeff[2];	/* C45 */
	q4 -= tmp;    	q5 += tmp;    	q6 += tmp;    	q7 -= tmp;    
    }


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

    if (count < 33)
	goto Lsave5;

    if ((coeff = coefflist[33]) != NULL) {
	tmp = coeff[1];	/* C23 */
	q4 += tmp;    	q7 -= tmp;    
	tmp = coeff[3];	/* C27 */
	q0 += tmp;    	q3 -= tmp;    
	tmp = coeff[5];	/* C36 */
	q5 += tmp;    	q6 -= tmp;    
	tmp = coeff[7];	/* C67 */
	q1 += tmp;    	q2 -= tmp;    
    }


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

    if (count < 35)
	goto Lsave5;

    if ((coeff = coefflist[35]) != NULL) {
	tmp = coeff[0];	/* C14 */
	q4 -= tmp;    	q5 -= tmp;    	q6 -= tmp;    	q7 -= tmp;    
	tmp = coeff[1];	/* C34 */
	q0 += tmp;    	q1 += tmp;    	q2 += tmp;    	q3 += tmp;    
    }


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

    if (count < 44)
	goto Lsave5;

    if ((coeff = coefflist[44]) != NULL) {
	tmp = coeff[0];	/* C12 */
	q1 += tmp;    	q2 -= tmp;    
	tmp = coeff[4];	/* C16 */
	q0 -= tmp;    	q3 += tmp;    
	tmp = coeff[2];	/* C25 */
	q5 += tmp;    	q6 -= tmp;    
	tmp = coeff[6];	/* C56 */
	q4 -= tmp;    	q7 += tmp;    
    }


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

    if (count < 46)
	goto Lsave5;

    if ((coeff = coefflist[46]) != NULL) {
	tmp = coeff[1];	/* C34 */
	q4 += tmp;    	q5 -= tmp;    	q6 -= tmp;    	q7 += tmp;    
	tmp = coeff[3];	/* C47 */
	q0 += tmp;    	q1 -= tmp;    	q2 -= tmp;    	q3 += tmp;    
    }


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

    if (count < 48)
	goto Lsave5;

    if ((coeff = coefflist[48]) != NULL) {
	tmp = coeff[0];	/* C12 */
	q4 -= tmp;    	q7 += tmp;    
	tmp = coeff[4];	/* C16 */
	q5 -= tmp;    	q6 += tmp;    
	tmp = coeff[1];	/* C23 */
	q0 += tmp;    	q3 -= tmp;    
	tmp = coeff[5];	/* C36 */
	q1 += tmp;    	q2 -= tmp;    
    }


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

    if (count < 55)
	goto Lsave5;

    if ((coeff = coefflist[55]) != NULL) {
	tmp = coeff[1];	/* C23 */
	q5 -= tmp;    	q6 += tmp;    
	tmp = coeff[3];	/* C27 */
	q1 -= tmp;    	q2 += tmp;    
	tmp = coeff[5];	/* C36 */
	q4 += tmp;    	q7 -= tmp;    
	tmp = coeff[7];	/* C67 */
	q0 += tmp;    	q3 -= tmp;    
    }


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

    if (count < 57)
	goto Lsave5;

    if ((coeff = coefflist[57]) != NULL) {
	tmp = coeff[0];	/* C14 */
	q4 -= tmp;    	q5 += tmp;    	q6 += tmp;    	q7 -= tmp;    
	tmp = coeff[1];	/* C34 */
	q0 += tmp;    	q1 -= tmp;    	q2 -= tmp;    	q3 += tmp;    
    }


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

    if (count < 62)
	goto Lsave5;

    if ((coeff = coefflist[62]) != NULL) {
	tmp = coeff[0];	/* C12 */
	q5 += tmp;    	q6 -= tmp;    
	tmp = coeff[4];	/* C16 */
	q4 -= tmp;    	q7 += tmp;    
	tmp = coeff[1];	/* C23 */
	q1 -= tmp;    	q2 += tmp;    
	tmp = coeff[5];	/* C36 */
	q0 += tmp;    	q3 -= tmp;    
    }

Lsave5:
    result[ 8] = q0;
    result[ 9] = q1;
    result[10] = q2;
    result[11] = q3;
    result[12] = q4;
    result[13] = q5;
    result[14] = q6;
    result[15] = q7;

Lsplatq3:
    result += 16;

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

    if (count < 4)
	goto Lexit;

    q0 = 0;     q1 = 0;     q2 = 0;     q3 = 0;
    q4 = 0;     q5 = 0;     q6 = 0;     q7 = 0;

    if ((coeff = coefflist[4]) != NULL) {
	tmp = coeff[0];	/* C11 */
	q0 += tmp;    
	tmp = coeff[1];	/* C13 */
	q1 += tmp;    
	q4 += tmp;    
	tmp = coeff[2];	/* C15 */
	q2 += tmp;    
	tmp = coeff[3];	/* C17 */
	q3 += tmp;    
	tmp = coeff[4];	/* C33 */
	q5 += tmp;    
	tmp = coeff[5];	/* C35 */
	q6 += tmp;    
	tmp = coeff[6];	/* C37 */
	q7 += tmp;    
	flag |= 8;
    }


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

    if (count < 11)
	goto Lsave6;

    if ((coeff = coefflist[11]) != NULL) {
	tmp = coeff[1];	/* C13 */
	q0 += tmp;    
	tmp = coeff[3];	/* C17 */
	q4 -= tmp;    
	tmp = coeff[4];	/* C33 */
	q1 += tmp;    
	tmp = coeff[5];	/* C35 */
	q2 += tmp;    
	tmp = coeff[6];	/* C37 */
	q3 += tmp;    
	q5 -= tmp;    
	tmp = coeff[8];	/* C57 */
	q6 -= tmp;    
	tmp = coeff[9];	/* C77 */
	q7 -= tmp;    
	flag |= 8;
    }


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

    if (count < 13)
	goto Lsave6;

    if ((coeff = coefflist[13]) != NULL) {
	tmp = coeff[0];	/* C11 */
	q2 -= tmp;    
	tmp = coeff[1];	/* C13 */
	q0 += tmp;    
	q6 -= tmp;    
	tmp = coeff[2];	/* C15 */
	q3 -= tmp;    
	tmp = coeff[3];	/* C17 */
	q1 -= tmp;    
	tmp = coeff[4];	/* C33 */
	q4 += tmp;    
	tmp = coeff[5];	/* C35 */
	q7 -= tmp;    
	tmp = coeff[6];	/* C37 */
	q5 -= tmp;    
	flag |= 8;
    }


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

    if (count < 22)
	goto Lsave6;

    if ((coeff = coefflist[22]) != NULL) {
	tmp = coeff[0];	/* C11 */
	q4 -= tmp;    
	tmp = coeff[1];	/* C13 */
	q5 -= tmp;    
	tmp = coeff[2];	/* C15 */
	q0 += tmp;    
	q6 -= tmp;    
	tmp = coeff[3];	/* C17 */
	q7 -= tmp;    
	tmp = coeff[5];	/* C35 */
	q1 += tmp;    
	tmp = coeff[7];	/* C55 */
	q2 += tmp;    
	tmp = coeff[8];	/* C57 */
	q3 += tmp;    
	flag |= 8;
    }


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

    if (count < 24)
	goto Lsave6;

    if ((coeff = coefflist[24]) != NULL) {
	tmp = coeff[1];	/* C13 */
	q2 -= tmp;    
	tmp = coeff[3];	/* C17 */
	q6 += tmp;    
	tmp = coeff[4];	/* C33 */
	q0 += tmp;    
	tmp = coeff[5];	/* C35 */
	q3 -= tmp;    
	tmp = coeff[6];	/* C37 */
	q1 -= tmp;    
	q4 -= tmp;    
	tmp = coeff[8];	/* C57 */
	q7 += tmp;    
	tmp = coeff[9];	/* C77 */
	q5 += tmp;    
	flag |= 8;
    }


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

    if (count < 26)
	goto Lsave6;

    if ((coeff = coefflist[26]) != NULL) {
	tmp = coeff[0];	/* C11 */
	q1 -= tmp;    
	tmp = coeff[1];	/* C13 */
	q3 += tmp;    
	q5 -= tmp;    
	tmp = coeff[2];	/* C15 */
	q0 += tmp;    
	tmp = coeff[3];	/* C17 */
	q2 += tmp;    
	tmp = coeff[4];	/* C33 */
	q7 += tmp;    
	tmp = coeff[5];	/* C35 */
	q4 += tmp;    
	tmp = coeff[6];	/* C37 */
	q6 += tmp;    
	flag |= 8;
    }


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

    if (count < 36)
	goto Lsave6;

    if ((coeff = coefflist[36]) != NULL) {
	tmp = coeff[2];	/* C15 */
	q4 -= tmp;    
	tmp = coeff[3];	/* C17 */
	q0 += tmp;    
	tmp = coeff[5];	/* C35 */
	q5 -= tmp;    
	tmp = coeff[6];	/* C37 */
	q1 += tmp;    
	tmp = coeff[7];	/* C55 */
	q6 -= tmp;    
	tmp = coeff[8];	/* C57 */
	q2 += tmp;    
	q7 -= tmp;    
	tmp = coeff[9];	/* C77 */
	q3 += tmp;    
	flag |= 8;
    }


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

    if (count < 38)
	goto Lsave6;

    if ((coeff = coefflist[38]) != NULL) {
	tmp = coeff[0];	/* C11 */
	q6 += tmp;    
	tmp = coeff[1];	/* C13 */
	q4 -= tmp;    
	tmp = coeff[2];	/* C15 */
	q2 -= tmp;    
	q7 += tmp;    
	tmp = coeff[3];	/* C17 */
	q5 += tmp;    
	tmp = coeff[5];	/* C35 */
	q0 += tmp;    
	tmp = coeff[7];	/* C55 */
	q3 -= tmp;    
	tmp = coeff[8];	/* C57 */
	q1 -= tmp;    
	flag |= 8;
    }


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

    if (count < 40)
	goto Lsave6;

    if ((coeff = coefflist[40]) != NULL) {
	tmp = coeff[1];	/* C13 */
	q1 -= tmp;    
	tmp = coeff[3];	/* C17 */
	q5 += tmp;    
	tmp = coeff[4];	/* C33 */
	q3 += tmp;    
	tmp = coeff[5];	/* C35 */
	q0 += tmp;    
	tmp = coeff[6];	/* C37 */
	q2 += tmp;    
	q7 -= tmp;    
	tmp = coeff[8];	/* C57 */
	q4 -= tmp;    
	tmp = coeff[9];	/* C77 */
	q6 -= tmp;    
	flag |= 8;
    }


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

    if (count < 42)
	goto Lsave6;

    if ((coeff = coefflist[42]) != NULL) {
	tmp = coeff[0];	/* C11 */
	q3 -= tmp;    
	tmp = coeff[1];	/* C13 */
	q2 += tmp;    
	q7 -= tmp;    
	tmp = coeff[2];	/* C15 */
	q1 -= tmp;    
	tmp = coeff[3];	/* C17 */
	q0 += tmp;    
	tmp = coeff[4];	/* C33 */
	q6 += tmp;    
	tmp = coeff[5];	/* C35 */
	q5 -= tmp;    
	tmp = coeff[6];	/* C37 */
	q4 += tmp;    
	flag |= 8;
    }


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

    if (count < 49)
	goto Lsave6;

    if ((coeff = coefflist[49]) != NULL) {
	tmp = coeff[2];	/* C15 */
	q6 += tmp;    
	tmp = coeff[3];	/* C17 */
	q2 -= tmp;    
	tmp = coeff[5];	/* C35 */
	q4 -= tmp;    
	tmp = coeff[6];	/* C37 */
	q0 += tmp;    
	tmp = coeff[7];	/* C55 */
	q7 += tmp;    
	tmp = coeff[8];	/* C57 */
	q3 -= tmp;    
	q5 += tmp;    
	tmp = coeff[9];	/* C77 */
	q1 -= tmp;    
	flag |= 8;
    }


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

    if (count < 51)
	goto Lsave6;

    if ((coeff = coefflist[51]) != NULL) {
	tmp = coeff[0];	/* C11 */
	q5 += tmp;    
	tmp = coeff[1];	/* C13 */
	q7 -= tmp;    
	tmp = coeff[2];	/* C15 */
	q1 -= tmp;    
	q4 -= tmp;    
	tmp = coeff[3];	/* C17 */
	q6 -= tmp;    
	tmp = coeff[5];	/* C35 */
	q3 += tmp;    
	tmp = coeff[7];	/* C55 */
	q0 += tmp;    
	tmp = coeff[8];	/* C57 */
	q2 += tmp;    
	flag |= 8;
    }


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

    if (count < 53)
	goto Lsave6;

    if ((coeff = coefflist[53]) != NULL) {
	tmp = coeff[1];	/* C13 */
	q3 -= tmp;    
	tmp = coeff[3];	/* C17 */
	q7 += tmp;    
	tmp = coeff[4];	/* C33 */
	q2 += tmp;    
	tmp = coeff[5];	/* C35 */
	q1 -= tmp;    
	tmp = coeff[6];	/* C37 */
	q0 += tmp;    
	q6 -= tmp;    
	tmp = coeff[8];	/* C57 */
	q5 += tmp;    
	tmp = coeff[9];	/* C77 */
	q4 -= tmp;    
	flag |= 8;
    }


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

    if (count < 58)
	goto Lsave6;

    if ((coeff = coefflist[58]) != NULL) {
	tmp = coeff[2];	/* C15 */
	q5 += tmp;    
	tmp = coeff[3];	/* C17 */
	q1 -= tmp;    
	tmp = coeff[5];	/* C35 */
	q7 -= tmp;    
	tmp = coeff[6];	/* C37 */
	q3 += tmp;    
	tmp = coeff[7];	/* C55 */
	q4 -= tmp;    
	tmp = coeff[8];	/* C57 */
	q0 += tmp;    
	q6 -= tmp;    
	tmp = coeff[9];	/* C77 */
	q2 += tmp;    
	flag |= 8;
    }


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

    if (count < 60)
	goto Lsave6;

    if ((coeff = coefflist[60]) != NULL) {
	tmp = coeff[0];	/* C11 */
	q7 += tmp;    
	tmp = coeff[1];	/* C13 */
	q6 -= tmp;    
	tmp = coeff[2];	/* C15 */
	q3 -= tmp;    
	q5 += tmp;    
	tmp = coeff[3];	/* C17 */
	q4 -= tmp;    
	tmp = coeff[5];	/* C35 */
	q2 += tmp;    
	tmp = coeff[7];	/* C55 */
	q1 -= tmp;    
	tmp = coeff[8];	/* C57 */
	q0 += tmp;    
	flag |= 8;
    }


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

    if (count < 63)
	goto Lsave6;

    if ((coeff = coefflist[63]) != NULL) {
	tmp = coeff[2];	/* C15 */
	q7 += tmp;    
	tmp = coeff[3];	/* C17 */
	q3 -= tmp;    
	tmp = coeff[5];	/* C35 */
	q6 -= tmp;    
	tmp = coeff[6];	/* C37 */
	q2 += tmp;    
	tmp = coeff[7];	/* C55 */
	q5 += tmp;    
	tmp = coeff[8];	/* C57 */
	q1 -= tmp;    
	q4 -= tmp;    
	tmp = coeff[9];	/* C77 */
	q0 += tmp;    
	flag |= 8;
    }

Lsave6:
    if ((flag & 8) == 0)
	goto Lexit;
    result[ 0] = q0;
    result[ 1] = q1;
    result[ 2] = q2;
    result[ 3] = q3;
    result[ 4] = q4;
    result[ 5] = q5;
    result[ 6] = q6;
    result[ 7] = q7;




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

    q0 = 0;     q1 = 0;     q2 = 0;     q3 = 0;
    q4 = 0;     q5 = 0;     q6 = 0;     q7 = 0;

    if ((coeff = coefflist[4]) != NULL) {
	tmp = coeff[2];	/* C15 */
	q0 += tmp;    
	tmp = coeff[3];	/* C17 */
	q4 += tmp;    
	tmp = coeff[5];	/* C35 */
	q1 += tmp;    
	tmp = coeff[6];	/* C37 */
	q5 += tmp;    
	tmp = coeff[7];	/* C55 */
	q2 += tmp;    
	tmp = coeff[8];	/* C57 */
	q3 += tmp;    
	q6 += tmp;    
	tmp = coeff[9];	/* C77 */
	q7 += tmp;    
    }


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

    if (count < 11)
	goto Lsave7;

    if ((coeff = coefflist[11]) != NULL) {
	tmp = coeff[0];	/* C11 */
	q0 -= tmp;    
	tmp = coeff[1];	/* C13 */
	q1 -= tmp;    
	tmp = coeff[2];	/* C15 */
	q2 -= tmp;    
	q4 -= tmp;    
	tmp = coeff[3];	/* C17 */
	q3 -= tmp;    
	tmp = coeff[5];	/* C35 */
	q5 -= tmp;    
	tmp = coeff[7];	/* C55 */
	q6 -= tmp;    
	tmp = coeff[8];	/* C57 */
	q7 -= tmp;    
    }


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

    if (count < 13)
	goto Lsave7;

    if ((coeff = coefflist[13]) != NULL) {
	tmp = coeff[2];	/* C15 */
	q2 -= tmp;    
	tmp = coeff[3];	/* C17 */
	q6 -= tmp;    
	tmp = coeff[5];	/* C35 */
	q0 += tmp;    
	tmp = coeff[6];	/* C37 */
	q4 += tmp;    
	tmp = coeff[7];	/* C55 */
	q3 -= tmp;    
	tmp = coeff[8];	/* C57 */
	q1 -= tmp;    
	q7 -= tmp;    
	tmp = coeff[9];	/* C77 */
	q5 -= tmp;    
    }


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

    if (count < 22)
	goto Lsave7;

    if ((coeff = coefflist[22]) != NULL) {
	tmp = coeff[1];	/* C13 */
	q4 += tmp;    
	tmp = coeff[3];	/* C17 */
	q0 += tmp;    
	tmp = coeff[4];	/* C33 */
	q5 += tmp;    
	tmp = coeff[5];	/* C35 */
	q6 += tmp;    
	tmp = coeff[6];	/* C37 */
	q1 += tmp;    
	q7 += tmp;    
	tmp = coeff[8];	/* C57 */
	q2 += tmp;    
	tmp = coeff[9];	/* C77 */
	q3 += tmp;    
    }


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

    if (count < 24)
	goto Lsave7;

    if ((coeff = coefflist[24]) != NULL) {
	tmp = coeff[0];	/* C11 */
	q2 += tmp;    
	tmp = coeff[1];	/* C13 */
	q0 -= tmp;    
	tmp = coeff[2];	/* C15 */
	q3 += tmp;    
	q6 += tmp;    
	tmp = coeff[3];	/* C17 */
	q1 += tmp;    
	tmp = coeff[5];	/* C35 */
	q4 -= tmp;    
	tmp = coeff[7];	/* C55 */
	q7 += tmp;    
	tmp = coeff[8];	/* C57 */
	q5 += tmp;    
    }


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

    if (count < 26)
	goto Lsave7;

    if ((coeff = coefflist[26]) != NULL) {
	tmp = coeff[2];	/* C15 */
	q1 -= tmp;    
	tmp = coeff[3];	/* C17 */
	q5 -= tmp;    
	tmp = coeff[5];	/* C35 */
	q3 += tmp;    
	tmp = coeff[6];	/* C37 */
	q7 += tmp;    
	tmp = coeff[7];	/* C55 */
	q0 += tmp;    
	tmp = coeff[8];	/* C57 */
	q2 += tmp;    
	q4 += tmp;    
	tmp = coeff[9];	/* C77 */
	q6 += tmp;    
    }


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

    if (count < 36)
	goto Lsave7;

    if ((coeff = coefflist[36]) != NULL) {
	tmp = coeff[0];	/* C11 */
	q4 -= tmp;    
	tmp = coeff[1];	/* C13 */
	q0 += tmp;    
	q5 -= tmp;    
	tmp = coeff[2];	/* C15 */
	q6 -= tmp;    
	tmp = coeff[3];	/* C17 */
	q7 -= tmp;    
	tmp = coeff[4];	/* C33 */
	q1 += tmp;    
	tmp = coeff[5];	/* C35 */
	q2 += tmp;    
	tmp = coeff[6];	/* C37 */
	q3 += tmp;    
    }


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

    if (count < 38)
	goto Lsave7;

    if ((coeff = coefflist[38]) != NULL) {
	tmp = coeff[1];	/* C13 */
	q6 -= tmp;    
	tmp = coeff[3];	/* C17 */
	q2 -= tmp;    
	tmp = coeff[4];	/* C33 */
	q4 += tmp;    
	tmp = coeff[5];	/* C35 */
	q7 -= tmp;    
	tmp = coeff[6];	/* C37 */
	q0 += tmp;    
	q5 -= tmp;    
	tmp = coeff[8];	/* C57 */
	q3 -= tmp;    
	tmp = coeff[9];	/* C77 */
	q1 -= tmp;    
    }


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

    if (count < 40)
	goto Lsave7;

    if ((coeff = coefflist[40]) != NULL) {
	tmp = coeff[0];	/* C11 */
	q1 += tmp;    
	tmp = coeff[1];	/* C13 */
	q3 -= tmp;    
	tmp = coeff[2];	/* C15 */
	q0 -= tmp;    
	q5 += tmp;    
	tmp = coeff[3];	/* C17 */
	q2 -= tmp;    
	tmp = coeff[5];	/* C35 */
	q7 -= tmp;    
	tmp = coeff[7];	/* C55 */
	q4 -= tmp;    
	tmp = coeff[8];	/* C57 */
	q6 -= tmp;    
    }


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

    if (count < 42)
	goto Lsave7;

    if ((coeff = coefflist[42]) != NULL) {
	tmp = coeff[2];	/* C15 */
	q3 -= tmp;    
	tmp = coeff[3];	/* C17 */
	q7 -= tmp;    
	tmp = coeff[5];	/* C35 */
	q2 += tmp;    
	tmp = coeff[6];	/* C37 */
	q6 += tmp;    
	tmp = coeff[7];	/* C55 */
	q1 -= tmp;    
	tmp = coeff[8];	/* C57 */
	q0 += tmp;    
	q5 -= tmp;    
	tmp = coeff[9];	/* C77 */
	q4 += tmp;    
    }


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

    if (count < 49)
	goto Lsave7;

    if ((coeff = coefflist[49]) != NULL) {
	tmp = coeff[0];	/* C11 */
	q6 += tmp;    
	tmp = coeff[1];	/* C13 */
	q2 -= tmp;    
	q4 -= tmp;    
	tmp = coeff[2];	/* C15 */
	q7 += tmp;    
	tmp = coeff[3];	/* C17 */
	q5 += tmp;    
	tmp = coeff[4];	/* C33 */
	q0 += tmp;    
	tmp = coeff[5];	/* C35 */
	q3 -= tmp;    
	tmp = coeff[6];	/* C37 */
	q1 -= tmp;    
    }


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

    if (count < 51)
	goto Lsave7;

    if ((coeff = coefflist[51]) != NULL) {
	tmp = coeff[1];	/* C13 */
	q5 -= tmp;    
	tmp = coeff[3];	/* C17 */
	q1 -= tmp;    
	tmp = coeff[4];	/* C33 */
	q7 += tmp;    
	tmp = coeff[5];	/* C35 */
	q4 += tmp;    
	tmp = coeff[6];	/* C37 */
	q3 += tmp;    
	q6 += tmp;    
	tmp = coeff[8];	/* C57 */
	q0 += tmp;    
	tmp = coeff[9];	/* C77 */
	q2 += tmp;    
    }


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

    if (count < 53)
	goto Lsave7;

    if ((coeff = coefflist[53]) != NULL) {
	tmp = coeff[0];	/* C11 */
	q3 += tmp;    
	tmp = coeff[1];	/* C13 */
	q2 -= tmp;    
	tmp = coeff[2];	/* C15 */
	q1 += tmp;    
	q7 += tmp;    
	tmp = coeff[3];	/* C17 */
	q0 -= tmp;    
	tmp = coeff[5];	/* C35 */
	q6 -= tmp;    
	tmp = coeff[7];	/* C55 */
	q5 += tmp;    
	tmp = coeff[8];	/* C57 */
	q4 -= tmp;    
    }


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

    if (count < 58)
	goto Lsave7;

    if ((coeff = coefflist[58]) != NULL) {
	tmp = coeff[0];	/* C11 */
	q5 += tmp;    
	tmp = coeff[1];	/* C13 */
	q1 -= tmp;    
	q7 -= tmp;    
	tmp = coeff[2];	/* C15 */
	q4 -= tmp;    
	tmp = coeff[3];	/* C17 */
	q6 -= tmp;    
	tmp = coeff[4];	/* C33 */
	q3 += tmp;    
	tmp = coeff[5];	/* C35 */
	q0 += tmp;    
	tmp = coeff[6];	/* C37 */
	q2 += tmp;    
    }


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

    if (count < 60)
	goto Lsave7;

    if ((coeff = coefflist[60]) != NULL) {
	tmp = coeff[1];	/* C13 */
	q7 -= tmp;    
	tmp = coeff[3];	/* C17 */
	q3 -= tmp;    
	tmp = coeff[4];	/* C33 */
	q6 += tmp;    
	tmp = coeff[5];	/* C35 */
	q5 -= tmp;    
	tmp = coeff[6];	/* C37 */
	q2 += tmp;    
	q4 += tmp;    
	tmp = coeff[8];	/* C57 */
	q1 -= tmp;    
	tmp = coeff[9];	/* C77 */
	q0 += tmp;    
    }


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

    if (count < 63)
	goto Lsave7;

    if ((coeff = coefflist[63]) != NULL) {
	tmp = coeff[0];	/* C11 */
	q7 += tmp;    
	tmp = coeff[1];	/* C13 */
	q3 -= tmp;    
	q6 -= tmp;    
	tmp = coeff[2];	/* C15 */
	q5 += tmp;    
	tmp = coeff[3];	/* C17 */
	q4 -= tmp;    
	tmp = coeff[4];	/* C33 */
	q2 += tmp;    
	tmp = coeff[5];	/* C35 */
	q1 -= tmp;    
	tmp = coeff[6];	/* C37 */
	q0 += tmp;    
    }

Lsave7:
    result[ 8] = q0;
    result[ 9] = q1;
    result[10] = q2;
    result[11] = q3;
    result[12] = q4;
    result[13] = q5;
    result[14] = q6;
    result[15] = q7;

Lexit:
    return flag;
}


