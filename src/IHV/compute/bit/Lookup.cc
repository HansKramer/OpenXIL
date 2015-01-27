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
//  File:	Lookup.cc
//  Project:	XIL
//  Revision:	1.9
//  Last Mod:	10:09:22, 03/10/00
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
#pragma ident	"@(#)Lookup.cc	1.9\t00/03/10  "

#include "XilDeviceManagerComputeBIT.hh"
#include "ComputeInfo.hh"
#include "XiliUtils.hh"

XilStatus
XilDeviceManagerComputeBIT::LookupTo1(XilOp*       op,
				      unsigned     op_count,
				      XilRoi*      roi,
				      XilBoxList*  bl)
{
    ComputeInfoBIT  ci(op, op_count, roi, bl);

    if(ci.isOK() == FALSE) {
        return ci.returnValue;
    }

    //
    // TODO:   make multibanded lut work...
    //
    XilLookupSingle* lookup;
    op->getParam(1, (XilObject**) &lookup);

    Xil_unsigned8* lut = 
	(Xil_unsigned8*)lookup->getData() + lookup->getOffset();

    while(ci.hasMoreInfo()) {
	for(int b=0; b<ci.destNumBands; b++) { // each band

	    Xil_unsigned8 zero_val = lut[b];
	    if (zero_val > 1) zero_val = 1;
	    
	    Xil_unsigned8 one_val = lut[b+ci.destNumBands];
	    if (one_val > 1) one_val = 1;
    
	    Xil_unsigned8* src1_scanline = ci.src1Data;
	    Xil_unsigned8* dest_scanline = ci.getDestData(b);

	    unsigned int src1_offset = ci.src1Offset;
	    unsigned int dest_offset = ci.getDestOffset(b);

	    int src1_scanline_stride = ci.src1ScanlineStride;
	    int dest_scanline_stride = ci.getDestScanlineStride(b);

	    if(zero_val == 0) {
		if(one_val == 0) {
		    for(int y=ci.ysize; y>0; y--) { // each scanline
			xili_bit_setvalue(dest_scanline,
				0,
				ci.xsize, dest_offset);
			
			dest_scanline += dest_scanline_stride;
		    }
		} else { // one_val == 1
		    for(int y=ci.ysize; y>0; y--) { // each scanline
	       
			xili_bit_memcpy(src1_scanline, dest_scanline,
				ci.xsize, src1_offset, dest_offset);
			
			src1_scanline += src1_scanline_stride;
			dest_scanline += dest_scanline_stride;
		    }
		}
	    } else { // zero_val == 1
		if(one_val == 0) {
		    for(int y=ci.ysize; y>0; y--) { // each scanline
	       
			xili_bit_not(src1_scanline, dest_scanline,
				ci.xsize, src1_offset, dest_offset);
			
			src1_scanline += src1_scanline_stride;
			dest_scanline += dest_scanline_stride;
		    }
		} else { // one_val == 1
		    for(int y=ci.ysize; y>0; y--) { // each scanline
			xili_bit_setvalue(dest_scanline,
				1,
				ci.xsize, dest_offset);
			
			dest_scanline += dest_scanline_stride;
		    }
		}
	    }
	}
    }
    
    return ci.returnValue;
}



XilStatus
XilDeviceManagerComputeBIT::LookupTo8(XilOp*       op,
				      unsigned     op_count,
				      XilRoi*      roi,
				      XilBoxList*  bl)
{
    ComputeInfoGENERAL  ci(op, op_count, roi, bl);

    if(ci.isOK() == FALSE) {
        return ci.returnValue;
    }

    XilLookupSingle* lookup;
    op->getParam(1, (XilObject**) &lookup);

    Xil_unsigned8* lut = 
	(Xil_unsigned8*)lookup->getData() + lookup->getOffset();

    while (ci.hasMoreInfo()) {
	if (ci.src1Storage.isType(XIL_BAND_SEQUENTIAL) &&
	    ci.destStorage.isType(XIL_PIXEL_SEQUENTIAL)) {
            Xil_unsigned8* src1_scanline = (Xil_unsigned8 *) ci.src1Data;
            Xil_unsigned8* dest_scanline = (Xil_unsigned8 *) ci.destData;
	    
	    unsigned int dest_pixel_stride = ci.destPixelStride;
            unsigned int src1_scanline_stride = ci.src1ScanlineStride;
            unsigned int dest_scanline_stride = ci.destScanlineStride;
            unsigned int src1_offset = ci.src1Offset;
	    
            for (int y=0; y<ci.ysize; y++) {
		Xil_unsigned8* dest_pixel = dest_scanline;
		
		for (int x=0; x<ci.xsize; x++) {
		    Xil_unsigned8* val;
		    
		    if (XIL_BMAP_TST(src1_scanline, src1_offset + x))
			val = lut + ci.destNumBands;
		    else
			val = lut;
		    
		    Xil_unsigned8* dest = dest_pixel;
		    for (int band = 0; band < ci.destNumBands; band++) {
			*dest++ = val[band];
		    }
		    dest_pixel += dest_pixel_stride;
		}
		dest_scanline += dest_scanline_stride;
		src1_scanline += src1_scanline_stride;
	    }
	} else {
	    for (int band = 0; band < ci.destNumBands; band++) {
		Xil_unsigned8* src1_scanline = (Xil_unsigned8 *) ci.src1Data;
		Xil_unsigned8* dest_scanline = (Xil_unsigned8 *) ci.getDestData(band);
		
		unsigned int src1_offset = ci.src1Offset;
		unsigned int src1_scanline_stride = ci.src1ScanlineStride;
		
		unsigned int dest_pixel_stride = ci.getDestPixelStride(band);
		unsigned int dest_scanline_stride = ci.getDestScanlineStride(band);
		
		Xil_unsigned8 val0 = lut[band];
		Xil_unsigned8 val1 = lut[band + ci.destNumBands];
		
		for (int y=0; y<ci.ysize; y++) {
		    Xil_unsigned8* dest = dest_scanline;
		    
		    for (int x=0; x<ci.xsize; x++) {
			if (XIL_BMAP_TST(src1_scanline, src1_offset + x))
			    *dest = val1;
			else
			    *dest = val0;
			
			dest += dest_pixel_stride;
		    }
		    dest_scanline += dest_scanline_stride;
		    src1_scanline += src1_scanline_stride;
		}
	    }
	}
    }
    
    return ci.returnValue;
}
    
XilStatus
XilDeviceManagerComputeBIT::LookupTo16(XilOp*       op,
				       unsigned     op_count,
				       XilRoi*      roi,
				       XilBoxList*  bl)
{
    ComputeInfoGENERAL  ci(op, op_count, roi, bl);

    if(ci.isOK() == FALSE) {
        return ci.returnValue;
    }

    XilLookupSingle* lookup;
    op->getParam(1, (XilObject**) &lookup);

    Xil_signed16* lut = 
	(Xil_signed16*)lookup->getData() + lookup->getOffset();

    while (ci.hasMoreInfo()) {
	if (ci.src1Storage.isType(XIL_BAND_SEQUENTIAL) &&
	    ci.destStorage.isType(XIL_PIXEL_SEQUENTIAL)) {
            Xil_unsigned8* src1_scanline = (Xil_unsigned8 *) ci.src1Data;
            Xil_signed16* dest_scanline = (Xil_signed16 *) ci.destData;
	    
	    unsigned int dest_pixel_stride = ci.destPixelStride;
            unsigned int src1_scanline_stride = ci.src1ScanlineStride;
            unsigned int dest_scanline_stride = ci.destScanlineStride;
            unsigned int src1_offset = ci.src1Offset;
	    
            for (int y=0; y<ci.ysize; y++) {
		Xil_signed16* dest_pixel = dest_scanline;
		
		for (int x=0; x<ci.xsize; x++) {
		    Xil_signed16* val;
		    
		    if (XIL_BMAP_TST(src1_scanline, src1_offset + x))
			val = lut + ci.destNumBands;
		    else
			val = lut;

		    Xil_signed16* dest = dest_pixel;
		    for (int band = 0; band < ci.destNumBands; band++) {
			*dest++ = val[band];
		    }
		    dest_pixel += dest_pixel_stride;
		}
		dest_scanline += dest_scanline_stride;
		src1_scanline += src1_scanline_stride;
	    }
	} else {
	    for (int band = 0; band < ci.destNumBands; band++) {
		Xil_unsigned8* src1_scanline = (Xil_unsigned8 *) ci.src1Data;
		Xil_signed16* dest_scanline = (Xil_signed16 *) ci.getDestData(band);
		
		unsigned int src1_offset = ci.src1Offset;
		unsigned int src1_scanline_stride = ci.src1ScanlineStride;
		
		unsigned int dest_pixel_stride = ci.getDestPixelStride(band);
		unsigned int dest_scanline_stride = ci.getDestScanlineStride(band);
		
		Xil_signed16 val0 = lut[band];
		Xil_signed16 val1 = lut[band + ci.destNumBands];
		
		for (int y=0; y<ci.ysize; y++) {
		    Xil_signed16* dest = dest_scanline;
		    
		    for (int x=0; x<ci.xsize; x++) {
			if (XIL_BMAP_TST(src1_scanline, src1_offset + x))
			    *dest = val1;
			else
			    *dest = val0;
			
			dest += dest_pixel_stride;
		    }
		    dest_scanline += dest_scanline_stride;
		    src1_scanline += src1_scanline_stride;
		}
	    }
	}
    }
    
    return ci.returnValue;
}

XilStatus
XilDeviceManagerComputeBIT::LookupTof32(XilOp*       op,
					unsigned     op_count,
					XilRoi*      roi,
					XilBoxList*  bl)
{
    ComputeInfoGENERAL  ci(op, op_count, roi, bl);

    if(ci.isOK() == FALSE) {
        return ci.returnValue;
    }

    XilLookupSingle* lookup;
    op->getParam(1, (XilObject**) &lookup);

    Xil_float32* lut = 
	(Xil_float32*)lookup->getData() + lookup->getOffset();

    while (ci.hasMoreInfo()) {
	if (ci.src1Storage.isType(XIL_BAND_SEQUENTIAL) &&
	    ci.destStorage.isType(XIL_PIXEL_SEQUENTIAL)) {
            Xil_unsigned8* src1_scanline = (Xil_unsigned8 *) ci.src1Data;
            Xil_float32* dest_scanline = (Xil_float32 *) ci.destData;
	    
	    unsigned int dest_pixel_stride = ci.destPixelStride;
            unsigned int src1_scanline_stride = ci.src1ScanlineStride;
            unsigned int dest_scanline_stride = ci.destScanlineStride;
            unsigned int src1_offset = ci.src1Offset;
	    
            for (int y=0; y<ci.ysize; y++) {
		Xil_float32* dest_pixel = dest_scanline;
		
		for (int x=0; x<ci.xsize; x++) {
		    Xil_float32* val;
		    
		    if (XIL_BMAP_TST(src1_scanline, src1_offset + x))
			val = (lut + ci.destNumBands);
		    else
			val = lut;

		    Xil_float32* dest = dest_pixel;
		    for (int band = 0; band < ci.destNumBands; band++) {
			*dest++ = val[band];
		    }
		    dest_pixel += dest_pixel_stride;
		}
		dest_scanline += dest_scanline_stride;
		src1_scanline += src1_scanline_stride;
	    }
	} else {
	    for (int band = 0; band < ci.destNumBands; band++) {
		Xil_unsigned8* src1_scanline = (Xil_unsigned8 *) ci.src1Data;
		Xil_float32* dest_scanline = (Xil_float32 *) ci.getDestData(band);
		
		unsigned int src1_offset = ci.src1Offset;
		unsigned int src1_scanline_stride = ci.src1ScanlineStride;
		
		unsigned int dest_pixel_stride = ci.getDestPixelStride(band);
		unsigned int dest_scanline_stride = ci.getDestScanlineStride(band);
		
		Xil_float32 val0 = lut[band];
		Xil_float32 val1 = lut[band + ci.destNumBands];
		
		for (int y=0; y<ci.ysize; y++) {
		    Xil_float32* dest = dest_scanline;
		    
		    for (int x=0; x<ci.xsize; x++) {
			if (XIL_BMAP_TST(src1_scanline, src1_offset + x))
			    *dest = val1;
			else
			    *dest = val0;
			
			dest += dest_pixel_stride;
		    }
		    dest_scanline += dest_scanline_stride;
		    src1_scanline += src1_scanline_stride;
		}
	    }
	}
    }
    
 return ci.returnValue;
}

