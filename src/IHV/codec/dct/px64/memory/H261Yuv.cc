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
//  File:	H261Yuv.cc
//  Project:	XIL
//  Revision:	1.5
//  Last Mod:	16:10:35, 31 Jan 1994
//
//  Description:
//	
//	
//	
//	
//	
//------------------------------------------------------------------------
//
//	Copyright (c) 1992, 1993, 1994, by Sun Microsystems, Inc.
//
//------------------------------------------------------------------------

#pragma ident	"@(#)H261Yuv.cc	1.5\t94/01/31  "


#include <xil/xilGPI.hh>

#define HI_SHORT_SHIFT	8

#ifdef XIL_LITLE_ENDIAN
#define GET_FIRST_BYTE(short_value)	(short_value & 255)
#define GET_SECOND_BYTE(short_value)	(short_value>>HI_SHORT_SHIFT)
#else
#define GET_FIRST_BYTE(short_value)	(short_value>>HI_SHORT_SHIFT)
#define GET_SECOND_BYTE(short_value)	(short_value & 255)
#endif

#define INPUT_MAGNITUDE 255

#define CLAMP(X) (((X) & ~255) ? (((X) < 0) ? 0 : 255) : (X))

/*
 * 2 adjacent pixels in X
 */
#define YUV2RGB(src_index,dest_addr)		\
    yvalSave = syptr[src_index];		\
    yval = ytable[GET_FIRST_BYTE(yvalSave)];	\
    blue = (yval + blueC) >> 4;			\
    dest_addr[0] = CLAMP(blue);			\
    green = (yval + greenC) >> 4;		\
    dest_addr[1] = CLAMP(green);		\
    red = (yval + redC) >> 4;			\
    dest_addr[2] = CLAMP(red);			\
    dest_addr += pixel_stride;			\
						\
    yval = ytable[GET_SECOND_BYTE(yvalSave)];	\
    blue = (yval + blueC) >> 4;			\
    dest_addr[0] = CLAMP(blue);			\
    green = (yval + greenC) >> 4;		\
    dest_addr[1] = CLAMP(green);		\
    red = (yval + redC) >> 4;			\
    dest_addr[2] = CLAMP(red);

/*
 * yuv2rgbblock -- convert 8 X 8 yuv 4:1:1 image to rgb image
 *
 * Source image:
 *	Represents an 8 X 8 block of YUV pixels, 4:1:1
 *
 */
void yuv2rgbblock(
    Xil_signed16 *syptr,	/* vector of 64 Y values */
    Xil_signed16 *suptr,   /* vector of 32 U values (we use only 16 of them) */
    Xil_signed16 *svptr,   /* vector of 32 V values (we use only 16 of them) */
    Xil_unsigned32 *destL, /* destination address for first pixel */
    int stride,		   /* size of a dest scanline (in bytes) */
    void** color_ptrs)     /* table of colorconvert table pointers */
{
    int redC, greenC, blueC;
    int yval, uval, vval;
    Xil_unsigned16 yvalSave;
    int red, green, blue;
    Xil_unsigned8* dest0;
    Xil_unsigned8* dest1;
    Xil_unsigned8* uptr = (Xil_unsigned8*)suptr;
    Xil_unsigned8* vptr = (Xil_unsigned8*)svptr;
    int *redtable = (int*)color_ptrs[0];
    int pixel_stride = (int)color_ptrs[1];
    int *bluetable = redtable + (2 * INPUT_MAGNITUDE);
    int *ytable = bluetable + (2 * INPUT_MAGNITUDE);
    int loop_count = 16;

    dest0 = ((Xil_unsigned8*) destL);
    dest1 = dest0 + stride;
    stride = (stride<<1) - ((pixel_stride << 3) - pixel_stride);
    /*
     * Convert 8 lines x 8 pixels  (loop executes 4X)
     */
    do {
	/*
	 * Convert 2 lines x 8 pixels  (loop executes 4X)
	 */
	do {
	    /*
	     * Convert 2 x 2  pixel block in inner loop
	     */

	    /*
	     * compute the chrominance values which will be used
	     * for all 4 pixels in this loop
	     */
	    vval = *vptr;
	    uval = *uptr;
	    redC = redtable[vval];
	    blueC = bluetable[uval];
	    greenC = (redC>>16) + (blueC>>16);
	    redC = (redC << 16) >> 16;
	    blueC = (blueC << 16) >> 16;

	    /*
	     * 2 adjacent pixels in X on scanline n
	     */
	    YUV2RGB(0,dest0);

	    /*
	     * 2 adjacent pixels in X on scanline n+1
	     */
	    YUV2RGB(4,dest1);

	    if (!(--loop_count & 3))
		break;
	    uptr++; vptr++;
	    dest0 += pixel_stride; dest1 += pixel_stride;
	    syptr++;
	} while (1);
	if (!loop_count)
	    break;
	uptr += 5; vptr += 5;
	dest0 += stride; dest1 += stride;
	syptr += 5;
    } while (1);

}
