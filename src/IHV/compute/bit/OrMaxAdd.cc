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
//  File:	OrMaxAdd.cc
//  Project:	XIL
//  Revision:	1.7
//  Last Mod:	10:09:14, 03/10/00
//
//  Description:
//	Same functionality for xil_max(), xil_or(), xil_add() for 1-bit
//	data.
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
#pragma ident	"@(#)OrMaxAdd.cc	1.7\t00/03/10  "

#include "XilDeviceManagerComputeBIT.hh"
#include "ComputeInfo.hh"
#include "XiliUtils.hh"

#define CUTOFFVALUE 5

enum Alignment {
    BOTH, SRC1, SRC2, NEITHER
};

XilStatus
XilDeviceManagerComputeBIT::OrMaxAdd(XilOp*       op,
                                     unsigned     op_count,
                                     XilRoi*      roi,
                                     XilBoxList*  bl)
{
    ComputeInfoBIT  ci(op, op_count, roi, bl);

    if(ci.isOK() == FALSE) {
        return ci.returnValue;
    }

    unsigned int nbands = ci.destNumBands;

    while(ci.hasMoreInfo()) {
	for(int b=0; b<nbands; b++) {
	    Xil_unsigned8* src1_scanline        = ci.getSrc1Data(b);
	    Xil_unsigned8* src2_scanline        = ci.getSrc2Data(b);
	    Xil_unsigned8* dest_scanline        = ci.getDestData(b);

	    unsigned int   src1_offset          = ci.getSrc1Offset(b);
	    unsigned int   src2_offset          = ci.getSrc2Offset(b);
	    unsigned int   dest_offset          = ci.getDestOffset(b);

	    unsigned int   src1_scanline_stride = ci.getSrc1ScanlineStride(b);
	    unsigned int   src2_scanline_stride = ci.getSrc2ScanlineStride(b);
	    unsigned int   dest_scanline_stride = ci.getDestScanlineStride(b);

	    for(int y=ci.ysize; y!=0; y--) {
		xili_bit_or(src1_scanline, src2_scanline, dest_scanline,
                            ci.xsize, 
                            src1_offset, src2_offset, dest_offset);
		
		src1_scanline += src1_scanline_stride;
		src2_scanline += src2_scanline_stride;
		dest_scanline += dest_scanline_stride;
	    }
	}
    }
    
    return ci.returnValue;
}

void
XilDeviceManagerComputeBIT::xili_bit_or(Xil_unsigned8* src1,
					Xil_unsigned8* src2,
					Xil_unsigned8* dest,
					int	       width,
					int            src1_offset,
					int            src2_offset,
					int            dest_offset)
{
    if(width < CUTOFFVALUE) {
	for(int j=0; j<width; j++) {
	    if(XIL_BMAP_TST(src1, src1_offset + j) ||
	       XIL_BMAP_TST(src2, src2_offset + j)) {
		XIL_BMAP_SET(dest, dest_offset + j);
	    } else {
		XIL_BMAP_CLR(dest, dest_offset + j);
	    }
	}
    } else {
#ifdef XIL_LITTLE_ENDIAN
	Xil_unsigned32* s1   = (Xil_unsigned32*) ((int)src1 & ~0x3);
	unsigned int    s1os = src1_offset + (((int)src1 - (int)s1) << 3);
	
	Xil_unsigned32* s2    = (Xil_unsigned32*) ((int)src2 & ~0x3);
	unsigned int    s2os  = src2_offset + (((int)src2 - (int)s2) << 3);
	
	Xil_unsigned32* d   = (Xil_unsigned32*) ((int)dest & ~0x3);
	unsigned int    dos = dest_offset + (((int)dest - (int)d) << 3);

	Xil_unsigned32 s1buf  = *s1++;
	s1buf = unscramble(s1buf);
	
	Xil_unsigned32 s1buf2 = *s1++;
	s1buf2 = unscramble(s1buf2);
	
	Xil_unsigned32 s2buf  = *s2++;
	s2buf = unscramble(s2buf);
	
	Xil_unsigned32 s2buf2 = *s2++;
	s2buf2 = unscramble(s2buf2);
	
	Xil_unsigned32 dbuf;
                        
	unsigned int remaining = width;
	unsigned int tbit = 0x80000000;
	
	if(dos % 32) {	// get to 1st word boundary in dst
	    dbuf = *d;
	    dbuf = unscramble(dbuf);
	    while((dos % 32) && (remaining > 0)) {
		if(((s1buf << s1os++) | (s2buf << s2os++)) & tbit)

		    dbuf |= tbit >> dos;
		else
		    dbuf &= ~(tbit >> dos);
                                    
		if(s1os == 32){
		    s1buf = s1buf2;
		    s1buf2 = *s1++;
		    s1buf2 = unscramble(s1buf2);
		    s1os = 0;
		}
		if(s2os == 32) {
		    s2buf = s2buf2;
		    s2buf2 = *s2++;
		    s2buf2 = unscramble(s2buf2);
		    s2os = 0;
		}
		dos++;
		remaining--;
	    }
	    dbuf = unscramble(dbuf);
	    *d++ = dbuf;

	}

	if(s1os) {
	    if(s2os) {	// src1 & src2 out of line with dst
		while(remaining > 31) {
		    dbuf =
			(((s1buf << s1os) | (s1buf2 >> (32 - s1os))) |
			 ((s2buf << s2os) | (s2buf2 >> (32 - s2os))));
		    
		    dbuf = unscramble(dbuf);
		    *d++ = dbuf;
		    s1buf = s1buf2;
		    s1buf2 = *s1++;
		    s1buf2 = unscramble(s1buf2);
		    s2buf = s2buf2;
		    s2buf2 = *s2++;
		    s2buf2 = unscramble(s2buf2);
		    remaining -= 32;
		}
	    } else {		//only src2 in line with dst
		s2--;
		while(remaining > 31) {
		    dbuf =
			(((s1buf << s1os) | (s1buf2 >> (32 - s1os))) | s2buf);
		    
		    dbuf = unscramble(dbuf);
		    *d++ = dbuf;
		    s1buf = s1buf2;
		    s1buf2 = *s1++;
		    s1buf2 = unscramble(s1buf2);
		    s2buf = *s2++;
		    s2buf = unscramble(s2buf);
		    remaining -= 32;
		}
		s2buf2 = *s2++;
	    }
	} else {
	    if(s2os) {	// only src1 in line with dst
		s1--;
		while(remaining > 31) {
		    dbuf =
			(s1buf | ((s2buf << s2os) | (s2buf2 >> (32 - s2os))));
		    dbuf = unscramble(dbuf);
		    *d++ = dbuf;
		    s1buf = *s1++;
		    s1buf = unscramble(s1buf);
		    s2buf = s2buf2;
		    s2buf2 = *s2++;
		    s2buf2 = unscramble(s2buf2);
		    remaining -= 32;
		}
		s1buf2 = *s1++;
	    } else {		// src1 & src2 aligned with dst
		s1--;  s2--;
		s1buf = unscramble(s1buf);
		s2buf = unscramble(s2buf);
                                    
		while(remaining > 31) {
		    *d++ = (s1buf | s2buf);
		    
		    s1buf = *s1++;
		    s2buf = *s2++;
		    remaining -= 32;
		}
		s1buf = unscramble(s1buf);
		s2buf = unscramble(s2buf);
		s1buf2 = *s1++;
		s2buf2 = *s2++;
		s1buf2 = unscramble(s1buf2);
		s2buf2 = unscramble(s2buf2);
	    }
	}
	
	if(remaining) {
	    dbuf = *d;
	    dbuf = unscramble(dbuf);
	    dos = 0;
	    while(remaining > 0) {	// finish up extra bits
		if(((s1buf << s1os++) | (s2buf << s2os++)) & tbit)

		    dbuf |= tbit >> dos;
		else
		    dbuf &= ~(tbit >> dos);
		
		if(s1os == 32){
		    s1buf = s1buf2;
		    s1buf2 = *s1++;
		    s1buf2 = unscramble(s1buf2);
		    s1os = 0;
		}
                                
		if(s2os == 32){
		    s2buf = s2buf2;
		    s2buf2 = *s2++;
		    s2buf2 = unscramble(s2buf2);
		    s2os = 0;
		}
		dos++;
		remaining--;
	    }
	    dbuf = unscramble(dbuf);
	    *d = dbuf;
	}
#else // XIL_BIG_ENDIAN
	
	//
	// This code was considered sparc specific in 1.2 that
	// check may need to be re-introduced when XIL 1.3 is ported
	// to a non sparc big endian architecture.
	//
	int             edge_bits_left;
	int             edge_bits_right;
	int             n32_bit_words;
	int             shift1;
	int             shift2;
	Xil_unsigned32  src1_shifted;
	Xil_unsigned32  src2_shifted;
	Xil_unsigned32  imask;
	Xil_unsigned32* src1_aligned;
	Xil_unsigned32* src2_aligned;
	Xil_unsigned32* dest_aligned;
	
	// find the number of bits until a 32-bit boundary
	edge_bits_left = (-(((unsigned int)dest & 0x3)<<3) - 
		dest_offset) & 0x1f;

	// number of unmasked aligned 32-bit words to process
	n32_bit_words = (width - edge_bits_left) / 32;

	// number of extra bits on right side
	edge_bits_right = width - edge_bits_left - (n32_bit_words << 5);

	// get the 32-bit aligned start for srcs and dest
	src1_aligned = (Xil_unsigned32*) ((Xil_unsigned32)
		(src1 + (src1_offset + edge_bits_left) / 8) &
		(~0x3));
	src2_aligned = (Xil_unsigned32*) ((Xil_unsigned32)
		(src2 + (src2_offset + edge_bits_left) / 8) &
		(~0x3));
	dest_aligned = (Xil_unsigned32*) ((Xil_unsigned32)
		(dest + (dest_offset + edge_bits_left) / 8));

	// amount to shift for alignment 
	shift1 = (src1_offset + edge_bits_left) -
		((int) src1_aligned - (int) src1) * 8;
	shift2 = (src2_offset + edge_bits_left) -
		((int) src2_aligned - (int) src2) * 8;

        Alignment align_type;
        if(shift1 == 0) {
            if(shift2 == 0) {
                align_type = BOTH;
            } else {
                align_type = SRC1;
            }
        } else {
            if(shift2 == 0) {
                align_type = SRC2;
            } else {
                align_type = NEITHER;
            }
        }

	// compute the shifted sources

	if(width <= edge_bits_left) {
	    // all dest bits in one 32-bit word

	    if(shift1 > 0) {
		if(shift1 < edge_bits_left) {
		    if(shift1 > (edge_bits_left - width)) {
			// bits in next src word?
			src1_shifted = ((src1_aligned[-1] << shift1) |
				(src1_aligned[0] >> (32 - shift1)));
		    } else {
			// no bits in next src word
			src1_shifted = (src1_aligned[-1] << shift1);
		    }
		} else {
		    src1_shifted = src1_aligned[0] >> (32 - shift1);
		}
		
	    } else { // shift1 == 0
		src1_shifted = src1_aligned[-1];
	    }
	    
	    if(shift2 > 0) {
		if(shift2 < edge_bits_left) {
		    if(shift2 > (edge_bits_left - width)) {
			// bits in next src word?
			src2_shifted = ((src2_aligned[-1] << shift2) |
				(src2_aligned[0] >> (32 - shift2)));
		    } else {
			// no bits in next src word
			src2_shifted = (src2_aligned[-1] << shift2);
		    }
		} else {
		    src2_shifted = src2_aligned[0] >> (32 - shift2);
		}
		
	    } else { // shift2 == 0
		src2_shifted = src2_aligned[-1];
	    }


	    // compute the write mask
	    imask = (0xffffffff >> (32 - edge_bits_left)) &
		    (0xffffffff << (32 - edge_bits_right));

	    // or in the result
	    dest_aligned[-1] = (dest_aligned[-1] & ~imask) |
		    ((src1_shifted | src2_shifted) & imask);

	} else { // dest is more than 1 32-bit word

	    // process the left edge if needed
	    if(edge_bits_left != 0) {
		if(shift1 > 0) {
		    if (shift1 < edge_bits_left) {
			src1_shifted = (src1_aligned[-1] << shift1) |
				(src1_aligned[0] >> (32 - shift1));
		    } else {
			src1_shifted = src1_aligned[0] >> (32 - shift1);
		    }
		} else { // shift1 == 0
		    src1_shifted = src1_aligned[-1];
		}

		if(shift2 > 0) {
		    if(shift2 < edge_bits_left) {
			src2_shifted = (src2_aligned[-1] << shift2) |
				(src2_aligned[0] >> (32 - shift2));
		    } else {
			src2_shifted = src2_aligned[0] >> (32 - shift2);
		    }
		} else {// shift2 == 0
		    src2_shifted = src2_aligned[-1];
		}

		// compute edge mask
		imask = 0xffffffff >> (32 - edge_bits_left);

		dest_aligned[-1] =
			(dest_aligned[-1] & ~imask) |
			((src1_shifted | src2_shifted) & imask);

	    }

            //
	    // Process the central portion of 32-bit quantities.
            // Four cases done separately, to avoid if-test in
            // the middle of the loop
            //

            int j;
            unsigned int rshift1 = (32 - shift1);
            unsigned int rshift2 = (32 - shift2);
            Xil_unsigned32* psrc1 = src1_aligned;
            Xil_unsigned32* psrc2 = src2_aligned;
            Xil_unsigned32* pdst = dest_aligned;
            switch(align_type) {
              case BOTH:
                for(j=0; j<n32_bit_words; j++) {
		    src1_shifted = psrc1[j];
		    src2_shifted = psrc2[j];
                    pdst[j] = src1_shifted | src2_shifted;
                }
                break;
              case SRC1:
                for(j=0; j<n32_bit_words; j++) {
		    src1_shifted = psrc1[j];
		    src2_shifted = (psrc2[j] << shift2) |
			    (psrc2[j+1] >> rshift2);
                    pdst[j] = src1_shifted | src2_shifted;
                }
                break;
              case SRC2:
                for(j=0; j<n32_bit_words; j++) {
		    src1_shifted = (psrc1[j] << shift1) |
			    (psrc1[j+1] >> rshift1);
		    src2_shifted = psrc2[j];
                    pdst[j] = src1_shifted | src2_shifted;
                }
                break;
              case NEITHER:
                for(j=0; j<n32_bit_words; j++) {
		    src1_shifted = (psrc1[j] << shift1) |
			    (psrc1[j+1] >> rshift1);
		    src2_shifted = (psrc2[j] << shift2) |
			    (psrc2[j+1] >> rshift2);
                    pdst[j] = src1_shifted | src2_shifted;
                }
                break;
            }

	    // process the right edge if needed
	    if(edge_bits_right != 0) {
		if(shift1 > 0) {
		    if(shift1 > (32 - edge_bits_right)) {
			src1_shifted = (src1_aligned[n32_bit_words] << shift1) |
			    (src1_aligned[n32_bit_words+1] >> (32 - shift1));
		    } else {
			src1_shifted = src1_aligned[n32_bit_words] << shift1;
		    }
		} else { // no shift
		    src1_shifted = src1_aligned[n32_bit_words];
		}

		if(shift2 > 0) {
		    if(shift2 > (32 - edge_bits_right)) {
			src2_shifted = (src2_aligned[n32_bit_words] << shift2) |
			    (src2_aligned[n32_bit_words+1] >> (32 - shift2));
		    } else {
			src2_shifted = src2_aligned[n32_bit_words] << shift2;
		    }
		} else { // no shift
		    src2_shifted = src2_aligned[n32_bit_words];
		}

		imask = 0xffffffff << (32 - edge_bits_right);

		dest_aligned[n32_bit_words] =
		    (dest_aligned[n32_bit_words] & ~imask) |
		    ((src1_shifted | src2_shifted) & imask);
	    }
	}
#endif // XIL_LITTLE_ENDIAN	
    }
}

