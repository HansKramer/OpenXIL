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
//  File:	And.cc
//  Project:	XIL
//  Revision:	1.4
//  Last Mod:	10:09:12, 03/10/00
//
//  Description:
//	
//	
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
#pragma ident	"@(#)And.cc	1.4\t00/03/10  "

#include "XilDeviceManagerComputeBIT.hh"
#include "ComputeInfo.hh"
#include "XiliUtils.hh"

#define CUTOFFVALUE 5

void
xili_bit_and(
    Xil_unsigned8* src1,
    Xil_unsigned8* src2,
    Xil_unsigned8* dest,
    int	width,
    int src1_offset,
    int src2_offset,
    int dest_offset);

XilStatus
XilDeviceManagerComputeBIT::And(XilOp*       op,
                                unsigned     op_count,
                                XilRoi*      roi,
                                XilBoxList*  bl)
{
    ComputeInfoBIT  ci(op, op_count, roi, bl);

    if(ci.isOK() == FALSE) {
        return XIL_CONTINUE;
    }

    while(ci.hasMoreInfo()) {
	for(int b=0; b<ci.nbands; b++) { // each band
	    Xil_unsigned8* src1_scanline = ci.getSrc1Data(b);
	    Xil_unsigned8* src2_scanline = ci.getSrc2Data(b);
	    Xil_unsigned8* dest_scanline = ci.getDestData(b);

	    for(int y=ci.ysize; y>0; y--) { // each scanline
	   
		xili_bit_and(src1_scanline, 
			src2_scanline,
			dest_scanline,
			ci.xsize, 
			ci.getSrc1Offset(b),
			ci.getSrc2Offset(b),
			ci.getDestOffset(b));
		
		src1_scanline += ci.getSrc1ScanlineStride(b);
		src2_scanline += ci.getSrc2ScanlineStride(b);
		dest_scanline += ci.getDestScanlineStride(b);
	    }
	}
    }
    
    return ci.returnValue;
}

void
xili_bit_and(
    Xil_unsigned8* src1,
    Xil_unsigned8* src2,
    Xil_unsigned8* dest,
    int	width,
    int src1_offset,
    int src2_offset,
    int dest_offset)
{
    int edge_bits_left, edge_bits_right;
    int nlongs;
    int shift1, shift2;
    unsigned long src1_shifted, src2_shifted;
    unsigned long imask;
    unsigned long *src1_aligned, *src2_aligned, *dest_aligned;




    if (width < CUTOFFVALUE) {
	for (int j=0; j<width; j++) {
	    if (XIL_BMAP_TST(src1, src1_offset + j) &&
		XIL_BMAP_TST(src2, src2_offset + j))
		XIL_BMAP_SET(dest, dest_offset + j);
	    else
		XIL_BMAP_CLR(dest, dest_offset + j);
	}
    
    }
    else {
	// find the number of bits until a 32-bit boundary
	edge_bits_left = (-(((unsigned int)dest & 0x3)<<3) - 
		dest_offset) & 0x1f;

	// number of whole longs to process
	nlongs = (width - edge_bits_left) / 32;

	// number of extra bits on right side
	edge_bits_right = width - edge_bits_left - (nlongs << 5);

	// get the 32-bit aligned start for srcs and dest
	src1_aligned = (unsigned long *) ((unsigned long)
		(src1 + (src1_offset + edge_bits_left) / 8) &
		(~0x3));
	src2_aligned = (unsigned long *) ((unsigned long)
		(src2 + (src2_offset + edge_bits_left) / 8) &
		(~0x3));
	dest_aligned = (unsigned long *) ((unsigned long)
		(dest + (dest_offset + edge_bits_left) / 8));

	// amount to shift for alignment
	shift1 = (src1_offset + edge_bits_left) -
		((long) src1_aligned - (long) src1) * 8;
	shift2 = (src2_offset + edge_bits_left) -
		((long) src2_aligned - (long) src2) * 8;

	// compute the shifted sources

	if (width <= edge_bits_left) {
	    // all dest bits in one 32-bit word

	    if (shift1 > 0) 
		if (shift1 < edge_bits_left) 
		    if (shift1 > (edge_bits_left - width)) 
			// bits in next src word?
			src1_shifted = ((src1_aligned[-1] << shift1) |
				(src1_aligned[0] >> (32 - shift1)));
		    else
			// no bits in next src word
			src1_shifted = (src1_aligned[-1] << shift1);
		else
		    src1_shifted = src1_aligned[0] >> (32 - shift1);
		
	    else // shift1 == 0
		src1_shifted = src1_aligned[-1];

	    if (shift2 > 0) 
		if (shift2 < edge_bits_left) 
		    if (shift2 > (edge_bits_left - width)) 
			// bits in next src word?
			src2_shifted = ((src2_aligned[-1] << shift2) |
				(src2_aligned[0] >> (32 - shift2)));
		    else
			// no bits in next src word
			src2_shifted = (src2_aligned[-1] << shift2);
		else
		    src2_shifted = src2_aligned[0] >> (32 - shift2);
		
	    else // shift2 == 0
		src2_shifted = src2_aligned[-1];


	    // compute the write mask
	    imask = (0xffffffff >> (32 - edge_bits_left)) &
		    (0xffffffff << (32 - edge_bits_right));

	    // or in the result
	    dest_aligned[-1] = (dest_aligned[-1] & ~imask) |
		    ((src1_shifted & src2_shifted) & imask);

	}
	else { // dest take more than 1 32-bit word

	    // process the left edge if needed
	    if (edge_bits_left != 0) {
		if (shift1 > 0) 
		    if (shift1 < edge_bits_left) 
			src1_shifted = (src1_aligned[-1] << shift1) |
				(src1_aligned[0] >> (32 - shift1));
		    else
			src1_shifted = src1_aligned[0] >> (32 - shift1);
		else // shift1 == 0
		    src1_shifted = src1_aligned[-1];

		if (shift2 > 0) 
		    if (shift2 < edge_bits_left) 
			src2_shifted = (src2_aligned[-1] << shift2) |
				(src2_aligned[0] >> (32 - shift2));
		    else
			src2_shifted = src2_aligned[0] >> (32 - shift2);
		else // shift2 == 0
		    src2_shifted = src2_aligned[-1];

		// compute edge mask
		imask = 0xffffffff >> (32 - edge_bits_left);

		dest_aligned[-1] =
			(dest_aligned[-1] & ~imask) |
			((src1_shifted & src2_shifted) & imask);

	    }

	    // process the central portion of 32-bit quantities
	    for (int j = 0; j < nlongs; j++) {
		if (shift1 > 0)  // shift left to dest
		    src1_shifted = (src1_aligned[j] << shift1) |
			    (src1_aligned[j+1] >> (32 - shift1));
		else // no shift
		    src1_shifted = src1_aligned[j];

		if (shift2 > 0)  // shift left to dest
		    src2_shifted = (src2_aligned[j] << shift2) |
			    (src2_aligned[j+1] >> (32 - shift2));
		else // no shift
		    src2_shifted = src2_aligned[j];
		
		dest_aligned[j] = src1_shifted & src2_shifted;
	    }

	    // process the right edge if needed
	    if (edge_bits_right != 0) {
		if (shift1 > 0)
		    if (shift1 > (32 - edge_bits_right))
			src1_shifted = (src1_aligned[nlongs] << shift1) |
				(src1_aligned[nlongs+1] >> (32 - shift1));
		    else
			src1_shifted = src1_aligned[nlongs] << shift1;
		else // no shift
		    src1_shifted = src1_aligned[nlongs];

		if (shift2 > 0)
		    if (shift2 > (32 - edge_bits_right))
			src2_shifted = (src2_aligned[nlongs] << shift2) |
				(src2_aligned[nlongs+1] >> (32 - shift2));
		    else
			src2_shifted = src2_aligned[nlongs] << shift2;
		else // no shift
		    src2_shifted = src2_aligned[nlongs];

		imask = 0xffffffff << (32 - edge_bits_right);

		dest_aligned[nlongs] = 
			(dest_aligned[nlongs] & ~imask) |
			((src1_shifted & src2_shifted) & imask);
	    }
	}
    }
}

