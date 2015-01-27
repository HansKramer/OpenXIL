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
//  File:       symmetry.hh
//  Project:    XIL
//  Revision:   1.2
//  Last Mod:   10:23:40, 03/10/00
//
//  Description:
//
//	Macros for idct splat (used in scale2)
//
//
//------------------------------------------------------------------------
//    COPYRIGHT
//------------------------------------------------------------------------
#pragma ident   "@(#)symmetry.hh	1.2\t00/03/10  "


#ifndef SYMMETRY_H
#define SYMMETRY_H

#ifdef XIL_LITTLE_ENDIAN
#define PACK_SHORTS(x,y)	(((y)<<16) | ((x)&0xffff))
#else
#define PACK_SHORTS(x,y)	(((x)<<16) | ((y)&0xffff))
#endif

#define symmetry_11111111(t, x, c0)		\
    {						\
	register long u0;			\
	register long *t0 = (long *) & t[x];	\
	u0 = PACK_SHORTS(c0,c0);		\
	t0[0] = t0[0] + u0;			\
	t0[1] = t0[1] + u0;			\
	t0[2] = t0[2] + u0;			\
	t0[3] = t0[3] + u0;			\
    }

#define symmetry_00000000(t, x, c0)		\
    {						\
	register long u0;			\
	register long *t0 = (long *) & t[x];	\
	u0 = PACK_SHORTS((-c0),(-c0));		\
	t0[0] = t0[0] + u0;			\
	t0[1] = t0[1] + u0;			\
	t0[2] = t0[2] + u0;			\
	t0[3] = t0[3] + u0;			\
    }

#define symmetry_11110000(t, x, c0, c1, c2, c3)	\
    {						\
	register long u0;			\
	register long *t0 = (long *) & t[x];	\
	u0 = PACK_SHORTS(c0,c1);		\
	t0[0] = t0[0] + u0;			\
	u0 = PACK_SHORTS(c2,c3);		\
	t0[1] = t0[1] + u0;			\
	u0 = PACK_SHORTS((-c3),(-c2));		\
	t0[2] = t0[2] + u0;			\
	u0 = PACK_SHORTS((-c1),(-c0));		\
	t0[3] = t0[3] + u0;			\
    }

#define symmetry_00001111(t, x, c0, c1, c2, c3)	\
    {						\
	register long u0;			\
	register long *t0 = (long *) & t[x];	\
	u0 = PACK_SHORTS((-c0),(-c1));		\
	t0[0] = t0[0] + u0;			\
	u0 = PACK_SHORTS((-c2),(-c3));		\
	t0[1] = t0[1] + u0;			\
	u0 = PACK_SHORTS(c3,c2);		\
	t0[2] = t0[2] + u0;			\
	u0 = PACK_SHORTS(c1,c0);		\
	t0[3] = t0[3] + u0;			\
    }

#define symmetry_11000011(t, x, c0, c1, c2, c3)	\
    {						\
	register long u0,u1,u2;			\
	register long *t0 = (long *) & t[x];	\
	u1 = -c2;				\
	u2 = -c3;				\
	u0 = PACK_SHORTS(c0,c1);		\
	t0[0] = t0[0] + u0;			\
	u0 = PACK_SHORTS(u1,u2);		\
	t0[1] = t0[1] + u0;			\
	u0 = PACK_SHORTS(u2,u1);		\
	t0[2] = t0[2] + u0;			\
	u0 = PACK_SHORTS(c1,c0);		\
	t0[3] = t0[3] + u0;			\
    }

#define symmetry_00111100(t, x, c0, c1, c2, c3)	\
    {						\
	register long u0,u1,u2;			\
	register long *t0 = (long *) & t[x];	\
	u1 = -c0;				\
	u2 = -c1;				\
	u0 = PACK_SHORTS(u1,u2);		\
	t0[0] = t0[0] + u0;			\
	u0 = PACK_SHORTS(c2,c3);		\
	t0[1] = t0[1] + u0;			\
	u0 = PACK_SHORTS(c3,c2);		\
	t0[2] = t0[2] + u0;			\
	u0 = PACK_SHORTS(u2,u1);		\
	t0[3] = t0[3] + u0;			\
    }

#define symmetry_10011001(t, x, c0, c1, c2, c3)	\
    {						\
	register long u0,u1,u2;			\
	register long *t0 = (long *) & t[x];	\
	u1 = -c1;				\
	u2 = -c2;				\
	u0 = PACK_SHORTS(c0,u1);		\
	t0[0] = t0[0] + u0;			\
	u0 = PACK_SHORTS(u2,c3);		\
	t0[1] = t0[1] + u0;			\
	u0 = PACK_SHORTS(c3,u2);		\
	t0[2] = t0[2] + u0;			\
	u0 = PACK_SHORTS(u1,c0);		\
	t0[3] = t0[3] + u0;			\
    }

#define symmetry_01100110(t, x, c0, c1, c2, c3)	\
    {						\
	register long u0,u1,u2;			\
	register long *t0 = (long *) & t[x];	\
	u1 = -c0;				\
	u2 = -c3;				\
	u0 = PACK_SHORTS(u1,c1);		\
	t0[0] = t0[0] + u0;			\
	u0 = PACK_SHORTS(c2,u2);		\
	t0[1] = t0[1] + u0;			\
	u0 = PACK_SHORTS(u2,c2);		\
	t0[2] = t0[2] + u0;			\
	u0 = PACK_SHORTS(c1,u1);		\
	t0[3] = t0[3] + u0;			\
    }

#define symmetry_10001110(t, x, c0, c1, c2, c3)	\
    {						\
	register long u0;			\
	register long *t0 = (long *) & t[x];	\
	u0 = PACK_SHORTS(c0,-c1);		\
	t0[0] = t0[0] + u0;			\
	u0 = PACK_SHORTS(-c2,-c3);		\
	t0[1] = t0[1] + u0;			\
	u0 = PACK_SHORTS(c3,c2);		\
	t0[2] = t0[2] + u0;			\
	u0 = PACK_SHORTS(c1,-c0);		\
	t0[3] = t0[3] + u0;			\
    }

#define symmetry_01110001(t, x, c0, c1, c2, c3)	\
    {						\
	register long u0;			\
	register long *t0 = (long *) & t[x];	\
	u0 = PACK_SHORTS(-c0,c1);		\
	t0[0] = t0[0] + u0;			\
	u0 = PACK_SHORTS(c2,c3);		\
	t0[1] = t0[1] + u0;			\
	u0 = PACK_SHORTS(-c3,-c2);		\
	t0[2] = t0[2] + u0;			\
	u0 = PACK_SHORTS(-c1,c0);		\
	t0[3] = t0[3] + u0;			\
    }

#define symmetry_10110010(t, x, c0, c1, c2, c3)	\
    {						\
	register long u0;			\
	register long *t0 = (long *) & t[x];	\
	u0 = PACK_SHORTS(c0,-c1);		\
	t0[0] = t0[0] + u0;			\
	u0 = PACK_SHORTS(c2,c3);		\
	t0[1] = t0[1] + u0;			\
	u0 = PACK_SHORTS(-c3,-c2);		\
	t0[2] = t0[2] + u0;			\
	u0 = PACK_SHORTS(c1,-c0);		\
	t0[3] = t0[3] + u0;			\
    }

#define symmetry_01001101(t, x, c0, c1, c2, c3)	\
    {						\
	register long u0;			\
	register long *t0 = (long *) & t[x];	\
	u0 = PACK_SHORTS(-c0,c1);		\
	t0[0] = t0[0] + u0;			\
	u0 = PACK_SHORTS(-c2,-c3);		\
	t0[1] = t0[1] + u0;			\
	u0 = PACK_SHORTS(c3,c2);		\
	t0[2] = t0[2] + u0;			\
	u0 = PACK_SHORTS(-c1,c0);		\
	t0[3] = t0[3] + u0;			\
    }

#define symmetry_10100101(t, x, c0, c1, c2, c3)	\
    {						\
	register long u0;			\
	register long *t0 = (long *) & t[x];	\
	u0 = PACK_SHORTS(c0,-c1);		\
	t0[0] = t0[0] + u0;			\
	u0 = PACK_SHORTS(c2,-c3);		\
	t0[1] = t0[1] + u0;			\
	u0 = PACK_SHORTS(-c3,c2);		\
	t0[2] = t0[2] + u0;			\
	u0 = PACK_SHORTS(-c1,c0);		\
	t0[3] = t0[3] + u0;			\
    }

#define symmetry_01011010(t, x, c0, c1, c2, c3)	\
    {						\
	register long u0;			\
	register long *t0 = (long *) & t[x];	\
	u0 = PACK_SHORTS(-c0,c1);		\
	t0[0] = t0[0] + u0;			\
	u0 = PACK_SHORTS(-c2,c3);		\
	t0[1] = t0[1] + u0;			\
	u0 = PACK_SHORTS(c3,-c2);		\
	t0[2] = t0[2] + u0;			\
	u0 = PACK_SHORTS(c1,-c0);		\
	t0[3] = t0[3] + u0;			\
    }

#define symmetry_10101010(t, x, c0, c1, c2, c3)	\
    {						\
	register long u0;			\
	register long *t0 = (long *) & t[x];	\
	u0 = PACK_SHORTS(c0,-c1);		\
	t0[0] = t0[0] + u0;			\
	u0 = PACK_SHORTS(c2,-c3);		\
	t0[1] = t0[1] + u0;			\
	u0 = PACK_SHORTS(c3,-c2);		\
	t0[2] = t0[2] + u0;			\
	u0 = PACK_SHORTS(c1,-c0);		\
	t0[3] = t0[3] + u0;			\
    }

#define symmetry_01010101(t, x, c0, c1, c2, c3)	\
    {						\
	register long u0;			\
	register long *t0 = (long *) & t[x];	\
	u0 = PACK_SHORTS(-c0,c1);		\
	t0[0] = t0[0] + u0;			\
	u0 = PACK_SHORTS(-c2,c3);		\
	t0[1] = t0[1] + u0;			\
	u0 = PACK_SHORTS(-c3,c2);		\
	t0[2] = t0[2] + u0;			\
	u0 = PACK_SHORTS(-c1,c0);		\
	t0[3] = t0[3] + u0;			\
    }

#endif // SYMMETRY_H
