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
//  File:	xili_svd.cc
//  Project:	XIL
//  Revision:	1.4
//  Last Mod:	10:16:34, 03/10/00
//
//  Description:
//	
//	Singular value decomposition functions.
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
#pragma ident	"@(#)xili_svd.cc	1.4\t00/03/10  "


/*************************************************************************
**									**
**      svd.c								**
**									**
**	Copyright (c) 1988 by Sun Microsystems, Inc., Raleigh, NC	**
**									**
**	Description:							**
**	------------							**
**	This is a suite of routines based on the singular value		**
**	decomposition for handling basic linear algebra	tasks.		**
**	The main module, routine svdcmp(), computes a singular value 	**
**	decomposition (SVD) for an m by n matrix A. This routine is a	**
**	modification of one given in Numerical Recipes in C.		**
**	(Press, et.al., Cambridge University Press, 1988)		**
**									**
**	- svsolve() uses svdcmp() to solve the linear system Ax=b	**
**		    by back substitution (svbksb()). If the system	**
**		    is overdetermined x represents the solution whose	**
**		    residual |Ax-b| is least squares minimal.		**
**									**
**	- svinvrt() uses svdcmp() and svedit to invert A.		**
**		    (If A is singular or A is not square the Moore-	**
**		    Penrose "pseudo-inverse" is produced.)		**
**									**
**	- svcond()  uses svdcmp() to compute the singular values	**
**		    and the condition number of A.			**
**                                                                      **
**	For "well behaved" square systems, the faster (but less robust)	**
**	versions of these routines which are based on lower/upper	**
**	triangular (LU) decomposition should be used. See lu.c.		**
**                                                                      **
**      Revision History:                                               **
**      -------- --------                                               **
**      Rev     Date       By   Description                             **
**      1.0     06/16/89   gyf  Original                                **
**                                                                      **
*************************************************************************/
#include <stdio.h>
#include <math.h>
#include "xili_svd.hh"
#define	 MAXSIZE	25
#define	 SVTINY		.00001
static long		infx[] = {0x7ff00000, 0x0};
#define	 inf		(*(double *)infx)

static float at,bt,ct;
#define PYTHAG(a,b) ((at=(float)fabs(a)) > (bt=(float)fabs(b)) ?    \
                            (ct=bt/at, at*sqrt(1.0+ct*ct)) : \
                            (bt ? (ct=at/bt, bt*sqrt(1.0+ct*ct)) : 0.0))
static float m1,m2;
#define MAX(a,b) ((m1=a, m2=b, m1 > m2) ? m1 : m2)
#define SIGN(a,b) ((b >= 0.0) ? fabs(a) : -fabs(a))

void svsolve(int m, int n, float *a, float *b, float *x)
{/* returns "least squares" solution (*x) of a (possibly overdetermined)    */
 /* linear system ax=b, i.e., |ax-b| is minimal. (*a and *b unaffected)	    */

   int i,j,ni;
   float u[MAXSIZE*MAXSIZE],v[MAXSIZE*MAXSIZE],w[MAXSIZE];

   for (i=0; i<m; i++) {		/* copy matrix into workspace	    */
	ni=n*i;   for(j=0; j<n; j++) u[ni+j] = a[ni+j];}
   svdcmp(m,n,u,w,v);			/* get singular value decomposition */
   svedit(n,w);				/* "edit" singular values	    */
   svbksb(m,n,u,w,v,b,x);		/* solve system			    */
   return;
}

void svinvrt(int m,int n, float *a, float *ainv)
{/* puts n by m pseudo-inverse of m by n matrix *a in *ainv. (*a unaffected)*/

   int i,j,k,ni,nj;
   float scale,sum,u[MAXSIZE*MAXSIZE],v[MAXSIZE*MAXSIZE],w[MAXSIZE];

   for (i=0; i<m; i++) {		/* copy matrix into workspace 	    */
	ni=n*i;
	for(j=0; j<n; j++) u[ni+j] = a[ni+j];}
   svdcmp(m,n,u,w,v);			/* get singular value decomposition */
   svedit(n,w);				/* "edit" singular values	    */
   for (j=0; j<n; j++) {
	if (w[j]) scale=1.0F/w[j];   else scale=0.0F;
	for (i=0; i<m; i++) u[n*i+j] *= scale;}
   for (j=0; j<n; j++) {
	nj=n*j;
	for (i=0; i<m; i++) {
	   sum=0.0;  ni=n*i;  
	   for (k=0; k<n; k++) sum += v[nj+k]*u[ni+k];
	   ainv[m*j+i]=sum;}}
   return;
}

float svcond(int m,int n, float *a, float *sv)
{	/* puts singular values of m by n matrix *a in *sv (*a unaffected)  */
	/* and returns condition number of a.				    */

   int i,j,ni;
   float max,min,u[MAXSIZE*MAXSIZE],v[MAXSIZE*MAXSIZE];

   for (i=0; i<m; i++) {		/* copy matrix into workspace 	    */
	ni=n*i;
	for(j=0; j<n; j++) u[ni+j] = a[ni+j];}
   svdcmp(m,n,u,sv,v);			/* get singular value decomposition */
   max=min=sv[0];
   for (i=1; i<n; i++) {
	if (sv[i] > max) max=sv[i];	/* get max singular value	    */
	if (sv[i] < min) min=sv[i];}	/* get min singular value	    */
   if (min == 0) return (float) inf;		/* return condition number	    */
   else return (max/min);
}

void svdcmp(int m,int n, float *a,float *w,float *v)
{/* Computes singular value decomposition of m by n matrix. On entry a	  */
 /* points to the matrix A to be decomposed as A=UWV where U is m by n	  */
 /* with orthonormal columns, W is a diagonal matrix of singular values,  */
 /* and V is n by n orthogonal. On exit a points to U, w points to a 	  */
 /* vector containing the diagonal elements of W, and v points to the	  */
 /* transpose of V.							  */

int flag,i,its,j,jj,k,l,nm;
float c,f,h,s,x,y,z,rv1[MAXSIZE];
float anorm=0, g=0, scale=0;

for (i=0; i<n; i++) {
    l=i+1;   rv1[i]=scale*g;   g=s=scale=0.0;
    if (i < m) {
	for (k=i; k<m; k++)  scale += (float) fabs(a[k*n+i]);
	if (scale) {
	    for (k=i; k<m; k++) {a[k*n+i] /= scale;   s += a[k*n+i]*a[k*n+i];}
	    f = a[i*n+i];
            g = (float) -SIGN(sqrt(s),f);
            h = f*g-s;
            a[i*n+i] = f-g;
	    if (i != n-1) {
		for (j=l; j<n; j++) {
		    for (s=0, k=i; k<m; k++) s += a[k*n+i]*a[k*n+j];
		    f=s/h;
		    for (k=i; k<m; k++) a[k*n+j] += f*a[k*n+i];}}
	    for (k=i; k<m; k++) a[k*n+i] *= scale;}}
    w[i] = scale*g;   g=s=scale=0.0;
    if (i < m  &&  i != n-1) {
	for (k=l; k<n; k++)  scale += (float)fabs(a[i*n+k]);
	if (scale) {
	    for (k=l; k<n; k++) {a[i*n+k] /= scale;   s += a[i*n+k]*a[i*n+k];}
	    f = a[i*n+l];
            g =(float) -SIGN(sqrt(s),f);
            h = f*g-s;
            a[i*n+l] = f-g;
	    for (k=l; k<n; k++) rv1[k] = a[i*n+k]/h;
	    if (i != m-1) {
		for (j=l; j<m; j++) {
		    for (s=0, k=l; k<n; k++) s += a[j*n+k]*a[i*n+k];
		    for (k=l; k<n; k++) a[j*n+k] += s*rv1[k];}}
	    for (k=l; k<n; k++) a[i*n+k] *= scale;}}
    anorm = MAX(anorm, (float) (fabs(w[i])+fabs(rv1[i])));}

for (i=n-1; i>=0; i--) {
    if (i < n-1) {
	if (g) {
	    for (j=l; j<n; j++)  v[j*n+i] = (a[i*n+j]/a[i*n+l])/g;
	    for (j=l; j<n; j++){
		for (s=0, k=l; k<n; k++)  s += a[i*n+k]*v[k*n+j];
		for (k=l; k<n; k++)  v[k*n+j] += s*v[k*n+i];}}
	for (j=l; j<n; j++)  v[i*n+j]=v[j*n+i]=0.0;}
    v[i*n+i]=1.0;   g=rv1[i];   l=i;}

for (i=n-1; i>=0; i--) {
    l=i+1;   g=w[i];
    if (i < n-1) for (j=l; j<n; j++)  a[i*n+j]=0.0;
    if (g) {
	g = 1.0F/g;
	if (i != n) {
	    for (j=l; j<n; j++){
		for (s=0, k=l; k<m; k++) s += a[k*n+i]*a[k*n+j];
		f = (s/a[i*n+i])*g;
		for (k=i; k<m; k++) a[k*n+j] += f*a[k*n+i];}}
	for (j=i; j<m; j++)  a[j*n+i] *= g;}
    else for(j=i; j<m; j++)  a[j*n+i] = 0.0;
    ++a[i*n+i];}

for (k=n-1; k>=0; k--) {
    for (its=0; its<30; its++) {
	flag = 1;
	for (l=k; l>=0; l--) {
	    nm=l-1;
	    if (fabs(rv1[l])+anorm == anorm) { flag = 0;  break;}
	    if (fabs(w[nm])+anorm == anorm) break;}
	if (flag) {
	    c = 0.0;  s = 1.0;
	    for (i=l; i<=k; i++) {
		f = s*rv1[i];
		if (fabs(f)+anorm != anorm) {
		    g = w[i];
                    h = (float) PYTHAG(f,g);  
                    w[i]=h;  h =1.0F/h;  c=g*h;  s=(-f)*h;
		    for (j=0; j<m; j++) {
			y=a[j*n+nm]; z=a[j*n+i];
			a[j*n+nm]=y*c+z*s; a[j*n+i]=z*c-y*s;}}}}
	z = w[k];
	if (l == k) {
	    if (z < 0.0) { w[k]=(-z);  for (j=0; j<n; j++) v[j*n+k]=(-v[j*n+k]);}
	    break;}
	if (its == 29) printf(" no convergence when k=%d\n",k);
	x=w[l];  nm=k-1;  y=w[nm];  g=rv1[nm];  h=rv1[k];
	f=((y-z)*(y+z)+(g-h)*(g+h))/(2*h*y);   g =(float) PYTHAG(f,1.0);
	f= (float)(((x-z)*(x+z)+h*((y/(f+SIGN(g,f)))-h))/x);   c=s=1.0F;
	for (j=l; j<k; j++) {
	    i=j+1;  g=rv1[i];  y=w[i];  h=s*g;  g=c*g;  z = (float) PYTHAG(f,h);
	    rv1[j]=z;  c=f/z;  s=h/z;  f=x*c+g*s;  g=g*c-x*s;  h=y*s;  y=y*c;
	    for (jj=0; jj<n; jj++) {
		x=v[jj*n+j]; z=v[jj*n+i]; v[jj*n+j]=x*c+z*s; v[jj*n+i]=z*c-x*s;}
	    z = (float) PYTHAG(f,h);  w[j]=z;  
	    if (z) { z=1.0F/z;  c=f*z;  s=h*z; }
	    f=c*g+s*y;   x=c*y-s*g;
	    for (jj=0; jj<m; jj++) {
		y=a[jj*n+j];  z=a[jj*n+i];
		a[jj*n+j]=y*c+z*s;  a[jj*n+i]=z*c-y*s;}}
	rv1[l]=0.0;   rv1[k]=f;   w[k]=x;}}

return;
}

void svbksb(int m,int n, float *u, float *w, float *v, float *b, float *x)
{/* Solves system UVWx=b. On entry u, w, and v are pointers to U, W,	*/
 /* and V, the factors of a singular value decomposition as obtained	*/
 /* svdcmp(), and b points to the right hand side vector. On exit x	*/
 /* points to a vector which is the best "least squares" solution to	*/
 /* the system.								*/

   int i,j,k,nj;
   float s,tmp[MAXSIZE];

   for (j=0; j<n; j++) {
	s=0.0;
	if (w[j]) { for (i=0; i<m; i++) s += u[n*i+j]*b[i];   s /= w[j];}
	tmp[j]=s;}
   for (j=0; j<n; j++) {
	s=0.0;   nj=n*j;
	for (k=0; k<n; k++) s += v[nj+k]*tmp[k];
	x[j]=s;}
   return;
}

void svedit(int n, float *w)
{/* zeros out prohibitively small singular values to "improve" behavior	   */
 /* of pseudo-inverse for ill-conditioned systems.			   */

   int i;
   float max;

   max = 0.0;   for (i=0; i<n; i++) if (w[i] > max) max=w[i];
   for (i=0; i<n; i++) if (w[i]/max < SVTINY) w[i]=0.0;
   return;
}
