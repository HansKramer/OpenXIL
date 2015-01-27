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
//  File:	Lookup.cc
//  Project:	XIL
//  Revision:	1.8
//  Last Mod:	10:58:23, 11/28/95
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
#pragma ident	"@(#)Lookup.cc	1.8\t95/11/28  "

#include "XilDeviceManagerComputeSHORT.hh"
#include "ComputeInfo.hh"

struct LookupData {
    Xil_unsigned8*  values;
    unsigned char   allocated;
};
    
XilStatus
XilDeviceManagerComputeSHORT::Lookup8Preprocess(XilOp*        op,
						unsigned      ,
						XilRoi*       ,
						void**        compute_data,
                                                unsigned int* )
{
    XilImage* dst = op->getDstImage(1);

    //
    // Create a lookup structure no matter what
    //
    LookupData* lud = new LookupData;
    if(lud == NULL) {
        XIL_ERROR(dst->getSystemState(), XIL_ERROR_RESOURCE, "di-1", TRUE);
        return XIL_FAILURE;
    }

    XilLookupSingle* lookup;
    op->getParam(1, (XilObject**) &lookup);
    unsigned int num_output_bands = lookup->getOutputNBands();
    int offset                    = lookup->getOffset();

    //
    // If the table covrs the dynamic space (65536 entries), just copy
    // the pointer (altering it to point at the 0th element).
    // Otherwise, if it's a 1 to 1 band table, copy table.
    // If the table has less than 65536 entries and is 1 to N band, do nothing.
    //
    unsigned int num_entries      = lookup->getNumEntries();
    if (num_entries != 65536) {
	// Only copy the data if this is a one to one lookup
#define XIL_LOOKUP_16_8_COPY_THRESHOLD 9
	if (num_output_bands < XIL_LOOKUP_16_8_COPY_THRESHOLD) {
	    Xil_unsigned8* values = new Xil_unsigned8[65536* num_output_bands];
	    if(values == NULL) {
		XIL_ERROR(dst->getSystemState(),
			  XIL_ERROR_RESOURCE, "di-1", TRUE);
		return XIL_FAILURE;
	    }
	    values += 32768 * num_output_bands;
	    lud->allocated = 1;
	    lud->values = values;
	    
	    Xil_unsigned8 *lut_data = (Xil_unsigned8 *) lookup->getData();
	    
	    // Start values pointer at the 0th index
	    int i,j;
	    for (i = -32768; i < offset; i++)
		for (j = 0; j < num_output_bands; j++)
		    values[i*num_output_bands + j] = lut_data[j];
	    
	    xili_memcpy(values+offset*num_output_bands,
			lut_data,
			num_entries * num_output_bands);

	    Xil_unsigned8 *top =
		&(lut_data[(num_entries - 1) * num_output_bands]);
	    for (i = num_entries + offset; i < 32768; i++)
		for (j = 0; j < num_output_bands; j++)
		    values[i*num_output_bands + j] = top[j];
	} else {
	    lud->values = 0;
	    lud->allocated = 0;
	}
    } else {
	lud->allocated = 0;
	lud->values = (Xil_unsigned8*) lookup->getData() -
	    offset * num_output_bands;
    }

    *compute_data = lud;

    return XIL_SUCCESS;
}

XilStatus
XilDeviceManagerComputeSHORT::Lookup8Postprocess(XilOp*       op,
						 void*        compute_data)
{
    XilLookupSingle* lookup;
    op->getParam(1, (XilObject**) &lookup);

    LookupData* lud = (LookupData*)compute_data;

    if (lud->allocated != 0)
	delete (lud->values - 32768 * lookup->getOutputNBands());
    
    delete lud;

    return XIL_SUCCESS;
}

XilStatus
XilDeviceManagerComputeSHORT::Lookup1(XilOp*       op,
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

    int output_bands = lookup->getOutputNBands();
    int num_entries  = lookup->getNumEntries();
    int offset       = lookup->getOffset();
    Xil_unsigned8* lut = (Xil_unsigned8*)lookup->getData() -
	offset * output_bands;

    //
    // Assuming a single lookup table
    //
    while(ci.hasMoreInfo()) {
	for (int band=0; band < output_bands; band++) {
	    Xil_signed16*  src1_scanline=(Xil_signed16*)ci.src1Data;
	    Xil_unsigned8* dest_scanline=(Xil_unsigned8*)ci.getDestData(band);
	    unsigned int dest_offset = ci.getDestOffset(band);
	    for(int y=ci.ysize; y>0; y--) {
		Xil_signed16* src1 = src1_scanline;
		for(int x=0; x<ci.xsize; x++) {
		    Xil_unsigned8* dest = dest_scanline;
		    if (*src1 > num_entries + offset) {
			if (lut[(num_entries - 1) * output_bands + band] &0x1)
			    XIL_BMAP_SET(dest, dest_offset + x);
			else
			    XIL_BMAP_CLR(dest, dest_offset + x);
		    } else if ( *src1 <  offset) {
			if (lut[band] & 0x1) 
			    XIL_BMAP_SET(dest, dest_offset + x);
			else
			    XIL_BMAP_CLR(dest, dest_offset + x);
		    } else {
			if (*(lut + (*src1 * output_bands) + band) & 0x1)
			    XIL_BMAP_SET(dest, dest_offset + x);
			else
			    XIL_BMAP_CLR(dest, dest_offset + x);
		    }
		    src1 += ci.src1PixelStride;
		}
		src1_scanline += ci.src1ScanlineStride;
		dest_scanline += ci.destScanlineStride;
	    }
	}
    } // end while has more info

    return ci.returnValue;
}


XilStatus
XilDeviceManagerComputeSHORT::Lookup8(XilOp*       op,
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

    LookupData*  lud = (LookupData*)op->getPreprocessData(this);
 
    //
    // Assuming a single lookup table
    //
    while(ci.hasMoreInfo()) {
	if ((ci.src1PixelStride == 1) &&
	    (ci.destPixelStride == 1) &&
            (ci.destNumBands    == 1)) {
	    Xil_signed16*  src1_scanline = (Xil_signed16*) ci.src1Data;
	    Xil_unsigned8* dest_scanline = (Xil_unsigned8*) ci.destData;
	    Xil_unsigned8* lut = (Xil_unsigned8*)lud->values;

	    int ySize = ci.ysize;
	    do {
		Xil_signed16*  src1 = src1_scanline;
		Xil_unsigned8* dest = dest_scanline;
		//
		//  Handle non word-aligned destination
		//
		unsigned int dstR_xbegin = 0;
		unsigned int tmp_xsize = ci.xsize;
		while(((unsigned int)dest) & 0x3 && tmp_xsize--) {
		    *dest++ = *(lut + (*src1++));
		    dstR_xbegin++;
		}
		
		unsigned int dstR_xsize_4  = (ci.xsize - dstR_xbegin)>>2;
		unsigned int dstR_xend     = ci.xsize - (dstR_xsize_4 << 2)
		    - dstR_xbegin;

		while(dstR_xsize_4--) {
#ifdef XIL_LITTLE_ENDIAN
		    unsigned int value = *(lut + src1[3]);
		    value = (value<<8) | *(lut + src1[2]);
		    value = (value<<8) | *(lut + src1[1]);
		    *((unsigned int*)dest) = (value<<8) | *(lut + src1[0]);
#else
		    unsigned int value = *(lut + src1[0]);
		    value = (value<<8) | *(lut + src1[1]);
		    value = (value<<8) | *(lut + src1[2]);
		    *((unsigned int*)dest) =(value<<8) | *(lut + src1[3]);
#endif
		    dest+=4;
		    src1+=4;
		}
		
		switch(dstR_xend) {
		  case 3:
		    dest[0] = *(lut + src1[0]);
		    dest[1] = *(lut + src1[1]);
		    dest[2] = *(lut + src1[2]);
		    dest+=3;
		    src1+=3;
		    break;
		    
		  case 2:
		    dest[0] = *(lut + src1[0]);
		    dest[1] = *(lut + src1[1]);
		    dest+=2;
		    src1+=2;
		    break;
		    
		  case 1:
		    dest[0] = *(lut + src1[0]);
		    dest++;
		    src1++;
		    break;
		}
		
		src1_scanline += ci.src1ScanlineStride;
		dest_scanline += ci.destScanlineStride;
	    } while(--ySize);
	} // end of 1 source band to 1 dest band lookup
	else if ((ci.destStorage.isType(XIL_PIXEL_SEQUENTIAL))) {
	    Xil_signed16*  src1_scanline = (Xil_signed16*) ci.src1Data;
	    Xil_unsigned8* dest_scanline = (Xil_unsigned8*) ci.destData;
	    if (lud->values != NULL) {
		Xil_unsigned8* lut = (Xil_unsigned8*)lud->values;
		for(int y=ci.ysize; y>0; y--) {
		    Xil_signed16*  src1_pixel = src1_scanline;
		    Xil_unsigned8* dest_pixel = dest_scanline;
		    for(int x=ci.xsize; x>0; x--) {
			Xil_signed16*  src1 = src1_pixel;
			Xil_unsigned8* dest = dest_pixel;
			Xil_unsigned8 *lut_band = lut +(*src1*ci.destNumBands);

			for (int band=0; band < ci.destNumBands; band++) {
			    *dest = lut_band[band];
			    dest ++;
			}
			
			src1_pixel += ci.src1PixelStride;
			dest_pixel += ci.destPixelStride;
		    }
		    src1_scanline += ci.src1ScanlineStride;
		    dest_scanline += ci.destScanlineStride;
		}
	    } else {
		int offset       = lookup->getOffset();
		int num_entries  = lookup->getNumEntries();
		Xil_unsigned8* lut = ((Xil_unsigned8*) lookup->getData())
		    - offset * lookup->getOutputNBands();
		Xil_unsigned8* top = lut + ((num_entries-1) * ci.destNumBands);
		for(int y=ci.ysize; y>0; y--) {
		    Xil_signed16*  src1_pixel = src1_scanline;
		    Xil_unsigned8* dest_pixel = dest_scanline;
		    for(int x=ci.xsize; x>0; x--) {
			Xil_signed16*  src1 = src1_pixel;
			Xil_unsigned8* dest = dest_pixel;
			Xil_unsigned8 *lut_band = lut +(*src1*ci.destNumBands);

			for (int band=0; band < ci.destNumBands; band++) {
			    if (*src1 > num_entries + offset)
				*dest = top[band];
			    else if (*src1 < offset)
				*dest = lut[band];
			    else 
				*dest = lut_band[band];
			    dest ++;
			}
			
			src1_pixel += ci.src1PixelStride;
			dest_pixel += ci.destPixelStride;
		    }
		    src1_scanline += ci.src1ScanlineStride;
		    dest_scanline += ci.destScanlineStride;
		}
	    }
	}  // end 1 source band to pixel sequential N banded lookup
	else {
	    if (lud->values != NULL) {
		int src1PixelStride = ci.getSrc1PixelStride(0);
		int src1ScanlineStride = ci.getSrc1ScanlineStride(0);
		Xil_unsigned8* lut = (Xil_unsigned8*)lud->values;
		for(int band=0; band < ci.destNumBands; band++) { 
		    int destPixelStride = ci.getDestPixelStride(band);
		    int destScanlineStride = ci.getDestScanlineStride(band);
		    Xil_signed16*  src1_scanline =
			(Xil_signed16*) ci.getSrc1Data(0);
		    Xil_unsigned8* dest_scanline =
			(Xil_unsigned8*) ci.getDestData(band);
		    for(int y=ci.ysize; y>0; y--) { 
			Xil_signed16*  src1 = src1_scanline;
			Xil_unsigned8* dest = dest_scanline;
			for(int x=ci.xsize; x>0; x--) {
			    *dest = *(lut + (*src1 * ci.destNumBands) + band);
			    
			    src1 += src1PixelStride;
			    dest += destPixelStride;
			}
			src1_scanline += src1ScanlineStride;
			dest_scanline += destScanlineStride;
		    }
		}
	    } else {	
		int src1PixelStride = ci.getSrc1PixelStride(0);
		int src1ScanlineStride = ci.getSrc1ScanlineStride(0);

		int offset       = lookup->getOffset();
		int num_entries  = lookup->getNumEntries();
		Xil_unsigned8* lut = ((Xil_unsigned8*) lookup->getData())
		    - offset * lookup->getOutputNBands();
		Xil_unsigned8* top = lut + ((num_entries-1) * ci.destNumBands);
		for(int band=0; band < ci.destNumBands; band++) { 
		    int destPixelStride = ci.getDestPixelStride(band);
		    int destScanlineStride = ci.getDestScanlineStride(band);
		    Xil_signed16*  src1_scanline =
			(Xil_signed16*) ci.getSrc1Data(0);
		    Xil_unsigned8* dest_scanline =
			(Xil_unsigned8*) ci.getDestData(band);
		    for(int y=ci.ysize; y>0; y--) { 
			Xil_signed16*  src1 = src1_scanline;
			Xil_unsigned8* dest = dest_scanline;
			for(int x=ci.xsize; x>0; x--) {
			    if (*src1 > num_entries + offset)
				*dest = top[band];
			    else if (*src1 < offset)
				*dest = lut[band];
			    else 
				*dest = *(lut +(*src1*ci.destNumBands) + band);
			
			    src1 += src1PixelStride;
			    dest += destPixelStride;
			}
			src1_scanline += src1ScanlineStride;
			dest_scanline += destScanlineStride;
		    }
		}
	    }
	} // end 1 source band to non-pixel sequential N banded lookup
    } // end while has more info

    return ci.returnValue;
}

XilStatus
XilDeviceManagerComputeSHORT::Lookup16(XilOp*       op,
				      unsigned     op_count,
				      XilRoi*      roi,
				      XilBoxList*  bl)
{
    ComputeInfoSHORT  ci(op, op_count, roi, bl);

    if(ci.isOK() == FALSE) {
        return ci.returnValue;
    }

    XilLookupSingle* lookup;
    op->getParam(1, (XilObject**) &lookup);

    int output_bands = lookup->getOutputNBands();
    int num_entries  = lookup->getNumEntries();
    int offset       = lookup->getOffset();
    Xil_signed16* lut = (Xil_signed16*)lookup->getData() - offset*output_bands;
    Xil_signed16* top = lut + (num_entries - 1) * output_bands;

    //
    // Assuming a single lookup table
    //

    while(ci.hasMoreInfo()) {
	if ((ci.src1Storage.isType(XIL_PIXEL_SEQUENTIAL)) &&
	    (ci.destStorage.isType(XIL_PIXEL_SEQUENTIAL))) {
	    Xil_signed16* src1_scanline = ci.src1Data;
	    Xil_signed16* dest_scanline = (Xil_signed16*) ci.destData;
	    for(int y=ci.ysize; y>0; y--) {
		Xil_signed16* src1_pixel = src1_scanline;
		Xil_signed16* dest_pixel = dest_scanline;
		for(int x=0; x<ci.xsize; x++) {
		    Xil_signed16* src1 = src1_pixel;
		    Xil_signed16* dest = dest_pixel;
		    Xil_signed16* lut_band = lut + *src1 * output_bands;
		    for (int band=0; band < output_bands; band++) {
			if (*src1 > num_entries + offset) {
			    *dest = top[band];
			} else if ( *src1 <  offset) {
			    *dest = lut[band];
			} else {
			    *dest = lut_band[band];
			}
			dest++;
		    }
		    src1_pixel += ci.src1PixelStride;
		    dest_pixel += ci.destPixelStride;
		}
		src1_scanline += ci.src1ScanlineStride;
		dest_scanline += ci.destScanlineStride;
	    }
	} else {
	    int src1PixelStride = ci.getSrc1PixelStride(0);
	    int src1ScanlineStride = ci.getSrc1ScanlineStride(0);
	    for(int band=0; band < output_bands; band++) { 
		int destPixelStride = ci.getDestPixelStride(band);
		int destScanlineStride = ci.getDestScanlineStride(band);
		Xil_signed16* src1_scanline = ci.getSrc1Data(0);
		Xil_signed16*  dest_scanline = (Xil_signed16 *)
		    ci.getDestData(band);
		for(int y=ci.ysize; y>0; y--) { 
		    Xil_signed16* src1 = src1_scanline;
		    Xil_signed16*  dest = dest_scanline;
		    for(int x=ci.xsize; x>0; x--) {
			if (*src1 > num_entries + offset) {
			    *dest = top[band];
			} else if ( *src1 < offset) {
			    *dest = lut[band];
			} else {
			    *dest = *(lut + (*src1 * output_bands) + band);
			}
			
			src1 += src1PixelStride;
			dest += destPixelStride;
		    }
		    src1_scanline += src1ScanlineStride;
		    dest_scanline += destScanlineStride;
		}
	    }
	}
    } // end while has more info

    return ci.returnValue;
}

XilStatus
XilDeviceManagerComputeSHORT::Lookupf32(XilOp*       op,
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

    int output_bands = lookup->getOutputNBands();
    int num_entries  = lookup->getNumEntries();
    int offset       = lookup->getOffset();
    Xil_float32* lut = (Xil_float32*)lookup->getData() - offset*output_bands;
    Xil_float32* top = lut + (num_entries - 1) * output_bands;
    //
    // Assuming a single lookup table
    //

    while(ci.hasMoreInfo()) {
	if ((ci.src1Storage.isType(XIL_PIXEL_SEQUENTIAL)) &&
	    (ci.destStorage.isType(XIL_PIXEL_SEQUENTIAL))) {
	    Xil_signed16* src1_scanline = (Xil_signed16*)ci.src1Data;
	    Xil_float32*  dest_scanline = (Xil_float32*) ci.destData;
	    for(int y=ci.ysize; y>0; y--) {
		Xil_signed16* src1_pixel = src1_scanline;
		Xil_float32*  dest_pixel = dest_scanline;
		for(int x=0; x<ci.xsize; x++) {
		    Xil_signed16* src1 = src1_pixel;
		    Xil_float32*  dest = dest_pixel;
		    Xil_float32*  lut_band = lut + *src1 * output_bands;
		    for (int band=0; band < output_bands; band++) {
			if (*src1 > num_entries + offset) {
			    *dest = top[band];
			} else if (*src1 < offset) {
			    *dest = lut[band];
			} else {
			    *dest = lut_band[band];
			}
			dest++;
		    }
		    src1_pixel += ci.src1PixelStride;
		    dest_pixel += ci.destPixelStride;
		}
		src1_scanline += ci.src1ScanlineStride;
		dest_scanline += ci.destScanlineStride;
	    }
	} else {
	    int src1PixelStride    = ci.getSrc1PixelStride(0);
	    int src1ScanlineStride = ci.getSrc1ScanlineStride(0);
	    for(int band=0; band < output_bands; band++) { 
		int destPixelStride    = ci.getDestPixelStride(band);
		int destScanlineStride = ci.getDestScanlineStride(band);
		Xil_signed16* src1_scanline = (Xil_signed16*)ci.getSrc1Data(0);
		Xil_float32*   dest_scanline = (Xil_float32*)
		    ci.getDestData(band);
		for(int y=ci.ysize; y>0; y--) { 
		    Xil_signed16* src1 = src1_scanline;
		    Xil_float32*   dest = dest_scanline;
		    for(int x=ci.xsize; x>0; x--) {
			if (*src1 > num_entries + offset) {
			    *dest = top[band];
			} else if (*src1 < offset) {
			    *dest = lut[band];
			} else {
			    *dest = *(lut + (*src1 * output_bands) + band);
			}
			
			src1 += src1PixelStride;
			dest += destPixelStride;
		    }
		    src1_scanline += src1ScanlineStride;
		    dest_scanline += destScanlineStride;
		}
	    }
	}
    } // end while has more info

    return ci.returnValue;
}


