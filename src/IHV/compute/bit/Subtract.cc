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
//  File:	Subtract.cc
//  Project:	XIL
//  Revision:	1.7
//  Last Mod:	10:09:13, 03/10/00
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
#pragma ident	"@(#)Subtract.cc	1.7\t00/03/10  "

#include "XilDeviceManagerComputeBIT.hh"
#include "ComputeInfo.hh"
#include "XiliUtils.hh"

XilStatus
XilDeviceManagerComputeBIT::Subtract(XilOp*       op,
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

            for(int y=ci.ysize; y>0; y--) {
                xili_bit_subtract(src1_scanline, src2_scanline, dest_scanline,
                                  ci.xsize,
                                  src1_offset, src2_offset, dest_offset);

                src1_scanline += src1_scanline_stride;
                src2_scanline += src2_scanline_stride;
                dest_scanline += dest_scanline_stride;
            }
        }
    }
    
    return XIL_SUCCESS;
}

#define CUTOFFVALUE 5

void
XilDeviceManagerComputeBIT::xili_bit_subtract(Xil_unsigned8* src1,
                                              Xil_unsigned8* src2,
                                              Xil_unsigned8* dest,
                                              int            width,
                                              int            src1_offset,
                                              int            src2_offset,
                                              int            dest_offset)
{
#ifdef XIL_LITTLE_ENDIAN
    for(int j=0; j<width; j++) {
        if(XIL_BMAP_TST(src1, src1_offset + j) &&
           !XIL_BMAP_TST(src2, src2_offset + j))
            XIL_BMAP_SET(dest, dest_offset + j);
        else
            XIL_BMAP_CLR(dest, dest_offset + j);
    }
#else // XIL_BIG_ENDIAN
    if(width < CUTOFFVALUE) {
        for(int j=0; j<width; j++) {
            if(XIL_BMAP_TST(src1, src1_offset + j) &&
               !XIL_BMAP_TST(src2, src2_offset + j))
                XIL_BMAP_SET(dest, dest_offset + j);
            else
                XIL_BMAP_CLR(dest, dest_offset + j);
        }
    } else {
        //
        //  Find the number of bits until a 32-bit boundary
        //
        int edge_bits_left =
            (-(((unsigned int)dest & 0x3)<<3) - dest_offset) & 0x1f;

        //
        //  Number of unmasked aligned 32-bit words to process
        //
        int n32_bit_words = (width - edge_bits_left) / 32;

        //
        //  Number of extra bits on right side
        //
        int edge_bits_right =
            width - edge_bits_left - (n32_bit_words << 5);

        //
        //  Get the 32-bit aligned start for srcs and dest
        //
        Xil_unsigned32* src1_aligned = (Xil_unsigned32*)
            ((Xil_unsigned32)(src1 +
                              (src1_offset + edge_bits_left) / 8) & (~0x3));

        Xil_unsigned32* src2_aligned = (Xil_unsigned32*)
            ((Xil_unsigned32)(src2 +
                              (src2_offset + edge_bits_left) / 8) & (~0x3));

        //
        //  TODO: 7/1/96 jlf  Seems like a bug here?  Why doesn't dest_aligned
        //                    need to be & ~0x3?
        //
        Xil_unsigned32* dest_aligned = (Xil_unsigned32*)
            ((Xil_unsigned32)(dest + (dest_offset + edge_bits_left) / 8));

        //
        //  Amount to shift for alignment
        //
        int shift1 =
            (src1_offset + edge_bits_left) - ((int)src1_aligned - (int)src1)*8;
        int shift2 =
            (src2_offset + edge_bits_left) - ((int)src2_aligned - (int)src2)*8;

        Xil_unsigned32  src1_shifted;
        Xil_unsigned32  src2_shifted;
        Xil_unsigned32  imask;

        //
        //  Compute the shifted sources
        //
        if(width <= edge_bits_left) {
            //
            //  All dest bits in one 32-bit word
            //
            if(shift1 > 0) {
                if(shift1 < edge_bits_left) {
                    if(shift1 > (edge_bits_left - width)) {
                        //
                        //  Bits in next src word?
                        //
                        src1_shifted = ((src1_aligned[-1] << shift1) |
                                        (src1_aligned[0] >> (32 - shift1)));
                    } else {
                        //
                        //  No bits in next src word
                        //
                        src1_shifted = (src1_aligned[-1] << shift1);
                    }
                } else {
                    src1_shifted = src1_aligned[0] >> (32 - shift1);
                }
            } else {
                //
                //  shift1 == 0
                //
                src1_shifted = src1_aligned[-1];
            }
                
            if(shift2 > 0) {
                if(shift2 < edge_bits_left) {
                    if(shift2 > (edge_bits_left - width)) {
                        //
                        //  Bits in next src word?
                        //
                        src2_shifted =
                            ((src2_aligned[-1] << shift2) |
                             (src2_aligned[0] >> (32 - shift2)));
                    } else {
                        //
                        //  No bits in next src word
                        //
                        src2_shifted = (src2_aligned[-1] << shift2);
                    }
                } else {
                    src2_shifted = src2_aligned[0] >> (32 - shift2);
                }
            } else {
                //
                //  shift2 == 0
                //
                src2_shifted = src2_aligned[-1];
            }

            //
            //  Compute the write mask
            //
            imask =
                (0xffffffff >> (32 - edge_bits_left)) &
                (0xffffffff << (32 - edge_bits_right));

            //
            //  Or in the result
            //
            dest_aligned[-1] =
                (dest_aligned[-1] & ~imask) |
                ((src1_shifted & ~src2_shifted) & imask);
        } else {
            //
            //  dest is more than 1 32-bit word
            //
            //
            //  Process the left edge if needed
            //
            if(edge_bits_left != 0) {
                if(shift1 > 0) {
                    if(shift1 < edge_bits_left) {
                        src1_shifted =
                            (src1_aligned[-1] << shift1) |
                            (src1_aligned[0] >> (32 - shift1));
                    } else {
                        src1_shifted =
                            src1_aligned[0] >> (32 - shift1);
                    }
                } else {
                    //
                    //  shift1 == 0
                    //
                    src1_shifted = src1_aligned[-1];
                }
                
                if(shift2 > 0) {
                    if(shift2 < edge_bits_left) {
                        src2_shifted =
                            (src2_aligned[-1] << shift2) |
                            (src2_aligned[0]  >> (32 - shift2));
                    } else {
                        src2_shifted =
                            src2_aligned[0] >> (32 - shift2);
                    }
                } else {
                    //
                    //  shift2 == 0
                    //
                    src2_shifted = src2_aligned[-1];
                }
                
                //
                //  compute edge mask
                //
                imask = 0xffffffff >> (32 - edge_bits_left);
                
                dest_aligned[-1] =
                    (dest_aligned[-1] & ~imask) |
                    ((src1_shifted & ~src2_shifted) & imask);
            }

            //
            //  Process the central portion of 32-bit quantities
            //
            for(int j = 0; j < n32_bit_words; j++) {
                if(shift1 > 0) {
                    //
                    //  shift left to dest
                    //
                    src1_shifted =
                        (src1_aligned[j] << shift1) |
                        (src1_aligned[j+1] >> (32 - shift1));
                } else {
                    //
                    //  no shift
                    //
                    src1_shifted = src1_aligned[j];
                }

                if(shift2 > 0) {
                    //
                    //  shift left to dest
                    //
                    src2_shifted =
                        (src2_aligned[j] << shift2) |
                        (src2_aligned[j+1] >> (32 - shift2));
                } else {
                    //
                    //  no shift
                    //
                    src2_shifted = src2_aligned[j];
                }

                dest_aligned[j] = src1_shifted & ~src2_shifted;
            }

            //
            //  Process the right edge if needed
            //
            if(edge_bits_right != 0) {
                if(shift1 > 0) {
                    if(shift1 > (32 - edge_bits_right)) {
                        src1_shifted =
                            (src1_aligned[n32_bit_words] << shift1) |
                            (src1_aligned[n32_bit_words+1] >> (32 - shift1));
                    } else {
                        src1_shifted = src1_aligned[n32_bit_words] << shift1;
                    }
                } else {
                    //
                    //  no shift
                    //
                    src1_shifted = src1_aligned[n32_bit_words];
                }
                
                if(shift2 > 0) {
                    if(shift2 > (32 - edge_bits_right)) {
                        src2_shifted = (src2_aligned[n32_bit_words] << shift2) |
                            (src2_aligned[n32_bit_words+1] >> (32 - shift2));
                    } else {
                        src2_shifted = src2_aligned[n32_bit_words] << shift2;
                    }
                } else {
                    //
                    //  no shift
                    //
                    src2_shifted = src2_aligned[n32_bit_words];
                }
                
                imask = 0xffffffff << (32 - edge_bits_right);
                
                dest_aligned[n32_bit_words] = 
                    (dest_aligned[n32_bit_words] & ~imask) |
                    ((src1_shifted & ~src2_shifted) & imask);
            }
        }
    }
#endif // XIL_LITTLE_ENDIAN
}

