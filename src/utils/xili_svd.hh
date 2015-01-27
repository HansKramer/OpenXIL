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
//  File:	xili_svd.hh
//  Project:	XIL
//  Revision:	1.2
//  Last Mod:	10:23:58, 03/10/00
//
//  Description:
//	
//	Provide data structures and function prototypes for "xili_svd.cc".
//	
//	
//	
//	
//	
//	
//  MT-level:  <??????>
//	
//------------------------------------------------------------------------
//	COPYRIGHT
//------------------------------------------------------------------------
#pragma ident	"@(#)xili_svd.hh	1.2\t00/03/10  "


void svsolve(int m, int n, float *a, float *b, float *x);
void svinvrt(int m,int n, float *a, float *ainv);
float svcond(int m,int n, float *a, float *sv);
void svdcmp(int m,int n, float *a,float *w,float *v);
void svbksb(int m,int n, float *u, float *w, float *v, float *b, float *x);
void svedit(int n, float *w);
