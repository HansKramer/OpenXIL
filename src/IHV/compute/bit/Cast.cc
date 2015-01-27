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
//  File:	Cast.cc
//  Project:	XIL
//  Revision:	1.2
//  Last Mod:	10:09:38, 03/10/00
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
#pragma ident	"@(#)Cast.cc	1.2\t00/03/10  "

#include "XilDeviceManagerComputeBIT.hh"
#include "ComputeInfo.hh"

XilStatus
XilDeviceManagerComputeBIT::CastTo8(XilOp*       op,
				    unsigned int op_count,
				    XilRoi*      roi,
				    XilBoxList*  bl)
{
    ComputeInfoGENERAL ci(op, op_count, roi, bl);
    
    if (ci.isOK() == FALSE) {
        return ci.returnValue;
    }

    if (getCastByteTable() == XIL_FAILURE) {
	return XIL_FAILURE;
    }
    while (ci.hasMoreInfo()) {
	for (int band = 0; band < ci.src1NumBands; band++) {
	    Xil_unsigned8* src1_scanline = (Xil_unsigned8 *) ci.getSrc1Data(band); 
	    Xil_unsigned8* dest_scanline = (Xil_unsigned8 *) ci.getDestData(band);

	    unsigned int dest_pixel_stride = ci.getDestPixelStride(band);
	    unsigned int src1_scanline_stride = ci.getSrc1ScanlineStride(band);
	    unsigned int dest_scanline_stride = ci.getDestScanlineStride(band);
	    unsigned int begin = ci.getSrc1Offset(band);
	    unsigned int end = ci.xsize + begin;

	    for (int y=ci.ysize; y>0; y--) {
		Xil_unsigned8* src1 = src1_scanline;
		Xil_unsigned8* dest = dest_scanline;
		unsigned long start = begin;
		
		// Process leading bits and get on to a byte boundary
		if (start%8) {
		    while ((start < end) && (start%8)) {
			*dest = castByteTable[*src1].b[start%8];
			dest += dest_pixel_stride;
			start++;
		    }
		    src1++;
		}

		// Do the byte aligned bits
		unsigned long stop = end - (end % 8);
		if ((dest_pixel_stride != 1) || ((unsigned int)dest & 0x3)) {
		    while (start < stop) {
			Xil_unsigned8* bPtr = castByteTable[*src1].b;
			for (int l = 0; l < 8; l++) {
			    *dest = *bPtr;
			    dest += dest_pixel_stride;
			    bPtr++;
			}
			src1++;
			start += 8;
		    }
		} else {
		    while (start < stop) {
			((int *)dest)[0] = castByteUpperTable[*src1];
			((int *)dest)[1] = castByteLowerTable[*src1];
			 dest += 8;
			 src1++;
			 start += 8;
		    }
		}

		// do trailing bits
		while (start < end) {
		    *dest = castByteTable[*src1].b[start - stop];
		    dest += dest_pixel_stride;
		    start++;
		}

		src1_scanline += src1_scanline_stride;
		dest_scanline += dest_scanline_stride;
	    }
	}
    }
		    
    return XIL_SUCCESS;
}


XilStatus
XilDeviceManagerComputeBIT::CastTo16(XilOp*       op,
				     unsigned int op_count,
				     XilRoi*      roi,
				     XilBoxList*  bl)
{
    ComputeInfoGENERAL ci(op, op_count, roi, bl);
    
    if (ci.isOK() == FALSE) {
        return ci.returnValue;
    }

    if (getCastShortTable() == XIL_FAILURE) {
	return XIL_FAILURE;
    }
    
    while (ci.hasMoreInfo()) {
	for (int band = 0; band < ci.src1NumBands; band++) {
	    Xil_unsigned8* src1_scanline = (Xil_unsigned8 *) ci.getSrc1Data(band); 
	    Xil_signed16*  dest_scanline = (Xil_signed16 *)  ci.getDestData(band);

	    unsigned int dest_pixel_stride = ci.getDestPixelStride(band);
	    unsigned int src1_scanline_stride = ci.getSrc1ScanlineStride(band);
	    unsigned int dest_scanline_stride = ci.getDestScanlineStride(band);
	    unsigned int begin = ci.getSrc1Offset(band);
	    unsigned int end   = begin + ci.xsize;

	    for (int y=ci.ysize; y>0; y--) {
		Xil_unsigned8* src1 = src1_scanline;
		Xil_signed16*  dest = dest_scanline;
		unsigned int start = begin;
		
		// Process leading bits and get on to a byte boundary
		if (start%8) {
		    while ((start < end) && (start%8)) {
			*dest = castShortTable[*src1].b[start%8];
			dest += dest_pixel_stride;
			start++;
		    }
		    src1++;
		}

		// Do the byte aligned bits
		unsigned int stop = end - (end%8);
		while (start < stop) {
		    Xil_signed16* bPtr = castShortTable[*src1].b;
		    for (int l = 0; l < 8; l++) {
			*dest = *bPtr;
			dest += dest_pixel_stride;
			bPtr++;
		    }
		    src1++;
		    start += 8;
		}

		// do trailing bits
		while (start < end) {
		    *dest = castShortTable[*src1].b[start - stop];
		    dest += dest_pixel_stride;
		    start++;
		}

		src1_scanline += src1_scanline_stride;
		dest_scanline += dest_scanline_stride;
	    }
	}
    }
		    
    return XIL_SUCCESS;
}


XilStatus
XilDeviceManagerComputeBIT::CastTof32(XilOp*       op,
				      unsigned int  op_count,
				      XilRoi*      roi,
				      XilBoxList*  bl)
{
    ComputeInfoGENERAL ci(op, op_count, roi, bl);
    
    if (ci.isOK() == FALSE) {
        return ci.returnValue;
    }

    if (getCastFloat32Table() == XIL_FAILURE) {
	return XIL_FAILURE;
    }
    
    while (ci.hasMoreInfo()) {
	for (int band = 0; band < ci.src1NumBands; band++) {
	    Xil_unsigned8* src1_scanline = (Xil_unsigned8 *) ci.getSrc1Data(band); 
	    Xil_float32*   dest_scanline = (Xil_float32 *)   ci.getDestData(band);

	    unsigned int dest_pixel_stride = ci.getDestPixelStride(band);
	    unsigned int src1_scanline_stride = ci.getSrc1ScanlineStride(band);
	    unsigned int dest_scanline_stride = ci.getDestScanlineStride(band);
	    unsigned int begin = ci.getSrc1Offset(band);
	    unsigned int end   = begin  + ci.xsize;

	    for (int y=ci.ysize; y>0; y--) {
		Xil_unsigned8* src1  = src1_scanline;
		Xil_float32*   dest  = dest_scanline;
		unsigned int   start = begin;
		
		// Process leading bits and get on to a byte boundary
		if (start%8) {
		    while ((start < end) && (start%8)) {
			*dest = castFloat32Table[*src1].b[start%8];
			dest += dest_pixel_stride;
			start++;
		    }
		    src1++;
		}

		// Do the byte aligned bits
		unsigned long stop = end - (end%8);
		while (start < stop) {
		    Xil_float32* bPtr = castFloat32Table[*src1].b;
		    for (int l = 0; l < 8; l++) {
			*dest = *bPtr;
			dest += dest_pixel_stride;
			bPtr++;
		    }
		    src1++;
		    start += 8;
		}

		// do trailing bits
		while (start < end) {
		    *dest = castFloat32Table[*src1].b[start - stop];
		    dest += dest_pixel_stride;
		    start++;
		}

		src1_scanline += src1_scanline_stride;
		dest_scanline += dest_scanline_stride;
	    }
	}
    }
		    
    return XIL_SUCCESS;
}

