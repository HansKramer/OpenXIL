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
//  File:	Cast.cc
//  Project:	XIL
//  Revision:	1.20
//  Last Mod:	10:10:20, 03/10/00
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
//  MT-level:  Safe
//	
//------------------------------------------------------------------------
//	COPYRIGHT
//------------------------------------------------------------------------
#pragma ident	"@(#)Cast.cc	1.20\t00/03/10  "

#include "XilDeviceManagerComputeBYTE.hh"
#include "ComputeInfo.hh"

XilStatus
XilDeviceManagerComputeBYTE::CastTo1(XilOp*       op,
				     unsigned int op_count,
				     XilRoi*      roi,
				     XilBoxList*  bl)
{
    ComputeInfoGENERAL ci(op, op_count, roi, bl);

    if(ci.isOK() == FALSE) {
        return ci.returnValue;
    }

    unsigned int nbands = ci.destNumBands;

    while(ci.hasMoreInfo()) {
	if((ci.src1Storage.isType(XIL_PIXEL_SEQUENTIAL)) &&
           (ci.destStorage.isType(XIL_BAND_SEQUENTIAL))) {
	    Xil_unsigned8* src1_scanline = (Xil_unsigned8*) ci.src1Data; 
	    Xil_unsigned8* dest_scanline = (Xil_unsigned8*) ci.destData;

            unsigned int   src1_sstride  = ci.src1ScanlineStride;
            unsigned int   dest_sstride  = ci.destScanlineStride;

            unsigned int   src1_pstride  = ci.src1PixelStride;
            unsigned int   dest_bstride  = ci.destBandStride;

            //
            //  If it's small, use older loop instead of byte-by-byte loop
            //  which only supports distances > 8.
            //
            if(ci.xsize < 8) {
                for(unsigned int band = 0; band < ci.destNumBands; band++) {
                    Xil_unsigned8* src1_band = src1_scanline + band;
                    Xil_unsigned8* dest_band = dest_scanline + (band * dest_bstride);

                    for(unsigned int height = 0; height < ci.ysize; height++) {
                        Xil_unsigned8* src1 = src1_band;
                        Xil_unsigned8* dest = dest_band;

                        for(unsigned int width = 0; width < ci.xsize; width++) {
                            if(*src1 & 0x1) {
                                XIL_BMAP_SET(dest, ci.destOffset + width);
                            } else {
                                XIL_BMAP_CLR(dest, ci.destOffset + width);
                            }

                            src1 += src1_pstride;
                        }

                        src1_band += src1_sstride;
                        dest_band += dest_sstride;
                    }
                }
            } else {
                //
                //  Calc start and end bit positions (left to right)
                //
                int start_bit  = ci.destOffset;
                int end_bit    = start_bit + ci.xsize - 1;
                int full_bytes = (end_bit - ((start_bit + 7) & (~7)) + 1) / 8;

                //
                //  Cvt to bit position within byte
                //
                end_bit   = end_bit % 8;

                if(src1_pstride == 1) {
                    for(int y = ci.ysize; y != 0; y--) {
                        for(unsigned int j=0; j<nbands; j++) {
                            Xil_unsigned8* src1_pixel =
                                src1_scanline + j;
                            Xil_unsigned8* dest_pixel =
                                dest_scanline + (j * dest_bstride);

                            //
                            //  Do first (possibly) partial byte
                            //
                            if(start_bit != 0) {
                                for(int k=start_bit; k<8; k++) {
                                    if(*src1_pixel++ & 0x1) {
                                        XIL_BMAP_SET(dest_pixel, k);
                                    } else {
                                        XIL_BMAP_CLR(dest_pixel, k);
                                    }
                                }

                                dest_pixel++;
                            }

                            //
                            //  Do all the complete bytes.
                            //
                            //  Unroll the loop to do all 8 bits without loop
                            //  overhead.
                            //
                            //  Do one bit shift as an add (x86 shifts are 2
                            //  or 3 clocks).
                            //
                            //  Write to destination only when byte is
                            //  complete.  This could be extended to 32 bits,
                            //  but there would need to be separate code for
                            //  big and little endian cases.
                            //
                            int count = full_bytes;

                            while(count-- > 0) {
                                unsigned int reg8 = (*(src1_pixel+0) & 0x1);

                                reg8 = (reg8+reg8) | (*(src1_pixel+1) & 0x1);
                                reg8 = (reg8+reg8) | (*(src1_pixel+2) & 0x1);
                                reg8 = (reg8+reg8) | (*(src1_pixel+3) & 0x1);
                                reg8 = (reg8+reg8) | (*(src1_pixel+4) & 0x1);
                                reg8 = (reg8+reg8) | (*(src1_pixel+5) & 0x1);
                                reg8 = (reg8+reg8) | (*(src1_pixel+6) & 0x1);
                                reg8 = (reg8+reg8) | (*(src1_pixel+7) & 0x1);

                                src1_pixel += 8;

                                *dest_pixel++ = (Xil_unsigned8)reg8;
                            }

                            //
                            //  Do last (possibly) partial byte
                            //
                            if(end_bit != 7) {
                                for(int k=0; k<=end_bit; k++) {
                                    if(*src1_pixel++ & 0x1) {
                                        XIL_BMAP_SET(dest_pixel, k);
                                    } else {
                                        XIL_BMAP_CLR(dest_pixel, k);
                                    }
                                }
                            }
                        }

                        src1_scanline += src1_sstride;
                        dest_scanline += dest_sstride;
                    }
                } else {                
                    for(int y = ci.ysize; y != 0; y--) {
                        for(unsigned int j=0; j<nbands; j++) {
                            Xil_unsigned8* src1_pixel = src1_scanline + j;
                            Xil_unsigned8* dest_pixel =
                                dest_scanline + (j * dest_bstride);

                            //
                            //  Do first (possibly) partial byte
                            //
                            if(start_bit != 0) {
                                for(int k=start_bit; k<8; k++) {
                                    if(*src1_pixel & 0x1) {
                                        XIL_BMAP_SET(dest_pixel, k);
                                    } else {
                                        XIL_BMAP_CLR(dest_pixel, k);
                                    }

                                    src1_pixel += src1_pstride;
                                }

                                dest_pixel++;
                            }

                            //
                            //  Do all the complete bytes.
                            //
                            //  Unroll the loop to do all 8 bits without loop
                            //  overhead.
                            //
                            //  Do one bit shift as an add (x86 shifts are 2
                            //  or 3 clocks).
                            //
                            //  Write to destination only when byte is
                            //  complete.  This could be extended to 32 bits,
                            //  but there would need to be separate code for
                            //  big and little endian cases.
                            //
                            int count = full_bytes;

                            //
                            //  General multi-band case
                            //
                            while(count-- > 0) {
                                unsigned int reg8 = (*src1_pixel & 0x1);
                                src1_pixel += src1_pstride;

                                reg8 = (reg8+reg8) | (*src1_pixel & 0x1);
                                src1_pixel += src1_pstride;

                                reg8 = (reg8+reg8) | (*src1_pixel & 0x1);
                                src1_pixel += src1_pstride;

                                reg8 = (reg8+reg8) | (*src1_pixel & 0x1);
                                src1_pixel += src1_pstride;

                                reg8 = (reg8+reg8) | (*src1_pixel & 0x1);
                                src1_pixel += src1_pstride;

                                reg8 = (reg8+reg8) | (*src1_pixel & 0x1);
                                src1_pixel += src1_pstride;

                                reg8 = (reg8+reg8) | (*src1_pixel & 0x1);
                                src1_pixel += src1_pstride;

                                reg8 = (reg8+reg8) | (*src1_pixel & 0x1);
                                src1_pixel += src1_pstride;

                                *dest_pixel++ = (Xil_unsigned8)reg8;
                            }

                            //
                            //  Do last (possibly) partial byte
                            //
                            if(end_bit != 7) {
                                for(int k=0; k<=end_bit; k++) {
                                    if(*src1_pixel & 0x1) {
                                        XIL_BMAP_SET(dest_pixel, k);
                                    } else {
                                        XIL_BMAP_CLR(dest_pixel, k);
                                    }

                                    src1_pixel += src1_pstride;
                                }
                            }
                        }

                        src1_scanline += src1_sstride;
                        dest_scanline += dest_sstride;
                    }
                }
            }
	} else {
	    for(unsigned int band = 0; band<nbands; band++) {
		Xil_unsigned8* src1_scanline = (Xil_unsigned8*) ci.getSrc1Data(band); 
		Xil_unsigned8* dest          = (Xil_unsigned8*) ci.getDestData(band);
		
		unsigned int src1_scanline_stride = ci.getSrc1ScanlineStride(band);
		unsigned int dest_scanline_stride = ci.getDestScanlineStride(band);
		unsigned int src1_pixel_stride    = ci.getSrc1PixelStride(band);
		unsigned int dest_offset          = ci.getDestOffset(band);
		
		for(unsigned int height = 0; height < ci.ysize; height++) {
		    Xil_unsigned8 *src1 = src1_scanline;

		    for(unsigned int width = 0; width < ci.xsize; width++) {
			if(*src1 & 0x1) {
			    XIL_BMAP_SET(dest, dest_offset + width);
			} else {
			    XIL_BMAP_CLR(dest, dest_offset + width);
			}

			src1 += src1_pixel_stride;
		    }

		    src1_scanline += src1_scanline_stride;
		    dest          += dest_scanline_stride;
		}
	    }
	}
    }
    
    return XIL_SUCCESS;
}


XilStatus
XilDeviceManagerComputeBYTE::CastTo16(XilOp*       op,
				     unsigned int  op_count,
				     XilRoi*      roi,
				     XilBoxList*  bl)
{
    ComputeInfoGENERAL ci(op, op_count, roi, bl);
    
    if(ci.isOK() == FALSE) {
        return ci.returnValue;
    }
    
    while(ci.hasMoreInfo()) {
        COMPUTE_GENERAL_1S_1D(Xil_unsigned8, Xil_signed16,
                              
                              *dest = (Xil_signed16)*src1,

                              *(dest+1) = (Xil_signed16)*(src1+1);
                              *(dest+2) = (Xil_signed16)*(src1+2)
            );
    }


    return ci.returnValue;
}


XilStatus
XilDeviceManagerComputeBYTE::CastTof32(XilOp*       op,
                                       unsigned int  op_count,
                                       XilRoi*      roi,
                                       XilBoxList*  bl)
{
    ComputeInfoGENERAL  ci(op, op_count, roi, bl);

    if(ci.isOK() == FALSE) {
        return ci.returnValue;
    }

    while(ci.hasMoreInfo()) {
        COMPUTE_GENERAL_1S_1D(Xil_unsigned8, Xil_float32,
                              
                              *dest = _XILI_B2F(*src1),

                              *(dest+1) = _XILI_B2F(*(src1+1));
                              *(dest+2) = _XILI_B2F(*(src1+2))
            );
    }

    return ci.returnValue;
}

