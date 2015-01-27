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
//  File:	H261Dither.cc
//  Project:	XIL
//  Revision:	1.3
//  Last Mod:	16:10:37, 31 Jan 1994
//
//  Description:
//	C++ versions of Dither routines for IDCT decompressors
//	
//------------------------------------------------------------------------
//
//	Copyright (c) 1992, 1993, 1994, by Sun Microsystems, Inc.
//
//------------------------------------------------------------------------

#pragma ident	"@(#)H261Dither.cc	1.3\t94/01/31  "


#include <stdio.h>
#include <math.h>

#include "Idct.hh"
#include "IdctDither.hh"
#include "IdctDitherSize.hh"

/* XXX
 * This file is exactly the same as cis/IdctDither.cc except that
 * yuv data pointers are chars instead of shorts:
 *
 * Should use the same source file and just recompile it twice???
 */

#ifdef LITTLEENDIAN
#define ADD_FIRST_UV_TO_0(packed,uv)	(packed + ((uv << 16)>>16))
#define ADD_FIRST_UV_TO_1(packed,uv)	(packed + (uv >> 16))
#define ADD_SECOND_UV_TO_0(packed,uv)	(packed + (uv << 16))
#define ADD_SECOND_UV_TO_1(packed,uv)	(packed + ((uv >> 16)<<16))
#else
#define ADD_FIRST_UV_TO_0(packed,uv)	(packed + (uv >> 16)<<16)
#define ADD_FIRST_UV_TO_1(packed,uv)	(packed + (uv << 16))
#define ADD_SECOND_UV_TO_0(packed,uv)	(packed + (uv >> 16))
#define ADD_SECOND_UV_TO_1(packed,uv)	(packed + ((uv << 16)>>16))
#endif

// C version of dither routine.
// The assembly version should be used for performance.
//
// Dither/colorspace convert a 8x8 pixel block from 4:1:1 YUV data
//
void XiliDitherblock(Xil_signed16* syptr, Xil_signed16* suptr, 
		     Xil_signed16* svptr, Xil_unsigned32* dest0, 
                     int stride, void** dptrs)
{
        Xil_unsigned8** dither_ptrs = (Xil_unsigned8**)dptrs;
	Xil_unsigned32 *dest1;
	Xil_unsigned32 uv;
	int packed0, packed1;
	Xil_unsigned8 *ydither = dither_ptrs[0];
	Xil_unsigned32 *udither = (Xil_unsigned32 *) dither_ptrs[1];
	Xil_unsigned32 *vdither = (Xil_unsigned32 *) dither_ptrs[2];
	int ditherstep = DITHER_STEP_SIZE;
	int i = 4;
	Xil_unsigned8* yptr = (Xil_unsigned8*)syptr;
	Xil_unsigned8* uptr = (Xil_unsigned8*)suptr;
	Xil_unsigned8* vptr = (Xil_unsigned8*)svptr;
	
	while (1) {
	    dest1 = dest0 + stride;
	    
	    /*
	     * Load up 4 pixels with luminance values for scanline n
	     * and 4 pixels with luminance values for scanline n+1
	     */
	    ASSIGN_FIRST_BYTE(packed0,ydither[yptr[0]]);
	    ASSIGN_SECOND_BYTE(packed0,ydither[(yptr[1])+TABLE_ELEMENTS]);
	    ASSIGN_THIRD_BYTE(packed0,ydither[(yptr[2])+TABLE_ELEMENTS_2]);
	    ASSIGN_FOURTH_BYTE(packed0,ydither[(yptr[3])+TABLE_ELEMENTS_3]);

	    ASSIGN_FIRST_BYTE(packed1,ydither[(yptr[8])+TABLE_ELEMENTS_4]);
	    ASSIGN_SECOND_BYTE(packed1,ydither[(yptr[9])+TABLE_ELEMENTS_5]);
	    ASSIGN_THIRD_BYTE(packed1,ydither[(yptr[10])+TABLE_ELEMENTS_6]);
	    ASSIGN_FOURTH_BYTE(packed1,ydither[(yptr[11])+TABLE_ELEMENTS_7]);

	    /*
	     * Compute UV component for first 2 pixels of each scanline
	     * (each lookup delivers 4 pixels, 2 each in each scanline.
	     *  add them in one operation to produce 4 pixels of UV data)
	     */
	    uv = udither[uptr[0]] + vdither[vptr[0]];

	    /*
	     * Add 2 pixels of UV into each scanline
	     */
	    packed0 = ADD_FIRST_UV_TO_0(packed0,uv);
	    packed1 = ADD_FIRST_UV_TO_1(packed1,uv);

	    /*
	     * Get UV component for second two pixels and add it in
	     */
	    uv = udither[(uptr[1])+TABLE_ELEMENTS] +
			  vdither[(vptr[1])+TABLE_ELEMENTS];
	    dest0[0] = ADD_SECOND_UV_TO_0(packed0,uv);
	    dest1[0] = ADD_SECOND_UV_TO_1(packed1,uv);

	    
	    /*
	     * Repeat for next 4 pixels in X, so that we cover 8 pixels
	     * in X each time through the loop
	     */
	    ASSIGN_FIRST_BYTE(packed0,ydither[yptr[4]]);
	    ASSIGN_SECOND_BYTE(packed0,ydither[(yptr[5])+TABLE_ELEMENTS]);
	    ASSIGN_THIRD_BYTE(packed0,ydither[(yptr[6])+TABLE_ELEMENTS_2]);
	    ASSIGN_FOURTH_BYTE(packed0,ydither[(yptr[7])+TABLE_ELEMENTS_3]);

	    ASSIGN_FIRST_BYTE(packed1,ydither[(yptr[12])+TABLE_ELEMENTS_4]);
	    ASSIGN_SECOND_BYTE(packed1,ydither[(yptr[13])+TABLE_ELEMENTS_5]);
	    ASSIGN_THIRD_BYTE(packed1,ydither[(yptr[14])+TABLE_ELEMENTS_6]);
	    ASSIGN_FOURTH_BYTE(packed1,ydither[(yptr[15])+TABLE_ELEMENTS_7]);

	    uv = udither[uptr[2]] + vdither[vptr[2]];
	    packed0 = ADD_FIRST_UV_TO_0(packed0,uv);
	    packed1 = ADD_FIRST_UV_TO_1(packed1,uv);
	    
	    uv = udither[(uptr[3])+TABLE_ELEMENTS] + 
			  vdither[(vptr[3])+TABLE_ELEMENTS];
	    dest0[1] = ADD_SECOND_UV_TO_0(packed0,uv);
	    dest1[1] = ADD_SECOND_UV_TO_1(packed1,uv);


	    if (--i == 0)
		    break;
	    
	    ydither += ditherstep;
	    udither = (Xil_unsigned32*)(((Xil_unsigned8*)udither) + ditherstep);
	    vdither = (Xil_unsigned32*)(((Xil_unsigned8*)vdither) + ditherstep);
	    ditherstep = -ditherstep;
	    dest0 = dest1 + stride;
	    yptr += 16;
	    uptr += 8;
	    vptr += 8;
	}
}
