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
//  Revision:	1.25
//  Last Mod:	10:10:09, 03/10/00
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
#pragma ident	"@(#)Lookup.cc	1.25\t00/03/10  "

#include "XilDeviceManagerComputeBYTE.hh"
#include "ComputeInfo.hh"

struct LookupData {
    Xil_unsigned8*  values;
    unsigned char   allocated;
};
    
XilStatus
XilDeviceManagerComputeBYTE::Lookup8Preprocess(XilOp*        op,
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
    // If the table covrs the dynamic space (256 entries), just copy
    // the pointer.
    // Otherwise, if it's a 1 to N band table, copy table if just a few bands.
    // If the table has less than 65536 entries and is 1 to N band, do nothing.
    //
    unsigned int num_entries      = lookup->getNumEntries();
    if(num_entries != 256) {
        //
        //  Only copy the data if this is a one to one lookup
        //
#define XIL_LOOKUP_8_8_COPY_THRESHOLD 256
        if(num_output_bands < XIL_LOOKUP_8_8_COPY_THRESHOLD) {
            Xil_unsigned8* values = new Xil_unsigned8[256 * num_output_bands];
	    if(values == NULL) {
		XIL_ERROR(dst->getSystemState(), XIL_ERROR_RESOURCE,
			  "di-1", TRUE);
		return XIL_FAILURE;
	    }
	    lud->allocated = 1;
	    lud->values = values;
            
            Xil_unsigned8 *lut_data = (Xil_unsigned8 *) lookup->getData();
            
            // Start values pointer at the 0th index
            int i;
            unsigned int j;
            for (i = 0; i < offset; i++)
                for (j = 0; j < num_output_bands; j++)
                    values[i*num_output_bands + j] = lut_data[j];
            
            xili_memcpy(values+offset*num_output_bands,
                        lut_data,
                        num_entries * num_output_bands);

            Xil_unsigned8 *top =
                &(lut_data[(num_entries - 1) * num_output_bands]);
            for (i = num_entries + offset; i < 256; i++)
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
XilDeviceManagerComputeBYTE::Lookup8Postprocess(XilOp*       ,
						void*        compute_data)
{
    LookupData* lud = (LookupData*)compute_data;

    if (lud->allocated != 0)
	delete lud->values;
    
    delete lud;
    
    return XIL_SUCCESS;
}

XilStatus
XilDeviceManagerComputeBYTE::Lookup1(XilOp*       op,
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
    Xil_unsigned8* lut=(Xil_unsigned8*)lookup->getData() - offset*output_bands;

    //
    // Assuming a single lookup table
    //
    while(ci.hasMoreInfo()) {
	for (int band=0; band < output_bands; band++) {
	    Xil_unsigned8* src1_scanline=(Xil_unsigned8*)ci.src1Data;
	    Xil_unsigned8* dest_scanline=(Xil_unsigned8*)ci.getDestData(band);
	    unsigned int dest_offset = ci.getDestOffset(band);
	    for(int y=ci.ysize; y>0; y--) {
		Xil_unsigned8* src1 = src1_scanline;
		for(unsigned int x=0; x<ci.xsize; x++) {
		    Xil_unsigned8* dest = dest_scanline;
		    if (*src1 > num_entries + offset) {
			if (lut[(num_entries - 1) * output_bands + band] & 0x1)
			    XIL_BMAP_SET(dest, dest_offset + x);
			else
			    XIL_BMAP_CLR(dest, dest_offset + x);
		    } else if (*src1 < offset) {
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
XilDeviceManagerComputeBYTE::Lookup8(XilOp*       op,
				     unsigned     op_count,
				     XilRoi*      roi,
				     XilBoxList*  bl)
{
    ComputeInfoBYTE  ci(op, op_count, roi, bl);

    if(ci.isOK() == FALSE) {
        return ci.returnValue;
    }

    XilLookupSingle* lookup;
    op->getParam(1, (XilObject**) &lookup);
    int output_bands = lookup->getOutputNBands();
    int num_entries  = lookup->getNumEntries();
    int offset       = lookup->getOffset();
    
    LookupData*  lud = (LookupData*)op->getPreprocessData(this);

    //
    // Assuming a single lookup table
    //
    while(ci.hasMoreInfo()) {
	if ((ci.src1PixelStride == 1) &&
	    (ci.destPixelStride == 1) &&
            (ci.destNumBands    == 1)) {
	    Xil_unsigned8* src1_scanline = ci.src1Data;
	    Xil_unsigned8* dest_scanline = ci.destData;
	    for(int y=ci.ysize; y>0; y--) {
		fastLookupWrite(src1_scanline,
                                dest_scanline,
                                ci.xsize,
                                lud->values);
		src1_scanline += ci.src1ScanlineStride;
		dest_scanline += ci.destScanlineStride;
	    }
	} // end of 1 source band to 1 dest band lookup
	else if ((ci.destStorage.isType(XIL_PIXEL_SEQUENTIAL))) {
	    if (lud->values != NULL) {
		Xil_unsigned8* src1_scanline = ci.src1Data;
		Xil_unsigned8* dest_scanline = ci.destData;
		Xil_unsigned8* lut = lud->values;

                if(output_bands == 3) {
                    for(int y=ci.ysize; y!=0; y--) {
                        Xil_unsigned8* src1_pixel = src1_scanline;
                        Xil_unsigned8* dest_pixel = dest_scanline;
                        for(int x=ci.xsize; x!=0; x--) {
                            unsigned int gray = *src1_pixel;
                            Xil_unsigned8* lut_band = lut + 3*gray;

                            dest_pixel[0] = lut_band[0];
                            dest_pixel[1] = lut_band[1];
                            dest_pixel[2] = lut_band[2];
                            
                            src1_pixel += ci.src1PixelStride;
                            dest_pixel += ci.destPixelStride;
                        }
                        src1_scanline += ci.src1ScanlineStride;
                        dest_scanline += ci.destScanlineStride;
                    }
                } else {

                    for(int y=ci.ysize; y>0; y--) {
                        Xil_unsigned8* src1_pixel = src1_scanline;
                        Xil_unsigned8* dest_pixel = dest_scanline;
                        for(int x=ci.xsize; x>0; x--) {
                            Xil_unsigned8* src1 = src1_pixel;
                            Xil_unsigned8* dest = dest_pixel;
                            Xil_unsigned8 *lut_band = lut + (*src1 * output_bands);
                            
                            for (int band=0; band < output_bands; band++) {
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
	    } else {
		Xil_unsigned8* src1_scanline = ci.src1Data;
		Xil_unsigned8* dest_scanline = ci.destData;
		Xil_unsigned8* lut = (Xil_unsigned8*)lookup->getData() -
		    offset*output_bands;
		Xil_unsigned8* top = lut + (num_entries - 1) * output_bands;
		for(int y=ci.ysize; y>0; y--) {
		    Xil_unsigned8* src1_pixel = src1_scanline;
		    Xil_unsigned8* dest_pixel = dest_scanline;
		    for(int x=ci.xsize; x>0; x--) {
			Xil_unsigned8* src1 = src1_pixel;
			Xil_unsigned8* dest = dest_pixel;
			Xil_unsigned8 *lut_band = lut + (*src1 * output_bands);
			
			for (int band=0; band < output_bands; band++) {
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
		Xil_unsigned8* lut = lud->values;
		for(int band=0; band < output_bands; band++) { 
		    int destPixelStride = ci.getDestPixelStride(band);
		    int destScanlineStride = ci.getDestScanlineStride(band);
		    Xil_unsigned8* src1_scanline = ci.getSrc1Data(0);
		    Xil_unsigned8* dest_scanline = ci.getDestData(band);
                    if(output_bands == 3) {
                        for(int y=ci.ysize; y!=0; y--) { 
                            Xil_unsigned8* src1 = src1_scanline;
                            Xil_unsigned8* dest = dest_scanline;
                            for(int x=ci.xsize; x!=0; x--) {
                                unsigned int gray = *src1;
                                Xil_unsigned8* lut_band = lut + band +
                                                          gray+gray+gray;
                                *dest = *lut_band;
                                src1 += src1PixelStride;
                                dest += destPixelStride;
                            }
                            src1_scanline += src1ScanlineStride;
                            dest_scanline += destScanlineStride;
                        }
                    } else {
                        for(int y=ci.ysize; y>0; y--) { 
                            Xil_unsigned8* src1 = src1_scanline;
                            Xil_unsigned8* dest = dest_scanline;
                            for(int x=ci.xsize; x>0; x--) {
                                *dest = *(lut + (*src1 * output_bands) + band);
                                src1 += src1PixelStride;
                                dest += destPixelStride;
                            }
                            src1_scanline += src1ScanlineStride;
                            dest_scanline += destScanlineStride;
                        }
                    }
		}
	    } else {
		int src1PixelStride = ci.getSrc1PixelStride(0);
		int src1ScanlineStride = ci.getSrc1ScanlineStride(0);
		Xil_unsigned8* lut = (Xil_unsigned8*)lookup->getData() -
		    offset*output_bands;
		Xil_unsigned8* top = lut + (num_entries - 1) * output_bands;
		for(int band=0; band < output_bands; band++) { 
		    int destPixelStride = ci.getDestPixelStride(band);
		    int destScanlineStride = ci.getDestScanlineStride(band);
		    Xil_unsigned8* src1_scanline = ci.getSrc1Data(0);
		    Xil_unsigned8* dest_scanline = ci.getDestData(band);
		    for(int y=ci.ysize; y>0; y--) { 
			Xil_unsigned8* src1 = src1_scanline;
			Xil_unsigned8* dest = dest_scanline;
			for(int x=ci.xsize; x>0; x--) {
			    if (*src1 > num_entries + offset)
				*dest = top[band];
			    else if (*src1 < offset)
				*dest = lut[band];
			    else 
				*dest = *(lut + (*src1 * output_bands) + band);
			    
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

void
XilDeviceManagerComputeBYTE::fastLookupWrite(Xil_unsigned8* in_src,
                                             Xil_unsigned8* in_dst,
                                             unsigned int   xsize,
                                             Xil_unsigned8* lut)
{
    Xil_unsigned8* src = in_src;
    Xil_unsigned8* dst = in_dst;

    //
    //  Get dst aligned on the 8 byte boundary
    //
    int xmod = ((int)(dst) & 7);
    dst -= xmod;
    src -= xmod;

    //
    //  Fill in preceeding group of <8 bytes in order to get both
    //  src and dst 8-byte aligned.
    //
    switch(xmod) {
      case 1: dst[1] = lut[src[1]]; if(!--xsize) break;
      case 2: dst[2] = lut[src[2]]; if(!--xsize) break;
      case 3: dst[3] = lut[src[3]]; if(!--xsize) break;
      case 4: dst[4] = lut[src[4]]; if(!--xsize) break;
      case 5: dst[5] = lut[src[5]]; if(!--xsize) break;
      case 6: dst[6] = lut[src[6]]; if(!--xsize) break;
      case 7: dst[7] = lut[src[7]]; if(!--xsize) break;
    }

    //
    // adjust pointers to beginning of 8-byte boundary.
    if(xmod) {
	src += 8;
	dst += 8;
    }

    //
    //  Do all even multiples of 8; pack them to make writes faster.
    //
    int           xeven  = (xsize >> 3);
    unsigned int* intout = (unsigned int*)dst;
    while(xeven--) {
#ifdef XIL_LITTLE_ENDIAN
        *intout =
            (lut[src[3]] << 24) |
            (lut[src[2]] << 16) |
            (lut[src[1]] << 8)  |
             lut[src[0]];

        *(intout+1) =
            (lut[src[7]] << 24) |
            (lut[src[6]] << 16) |
            (lut[src[5]] << 8)  |
             lut[src[4]];

      	src    += 8;
      	intout += 2;
    }

    xmod = xsize & 7;

    //
    //  do 4 more if there are that many
    //
    if(xmod>3) {
        *intout =
            (lut[src[3]] << 24) |
            (lut[src[2]] << 16) |
            (lut[src[1]] << 8)  |
             lut[src[0]];

#else
        *intout =
            (lut[src[0]] << 24) |
            (lut[src[1]] << 16) |
            (lut[src[2]] << 8)  |
             lut[src[3]];

        *(intout+1) =
            (lut[src[4]] << 24) |
            (lut[src[5]] << 16) |
            (lut[src[6]] << 8)  |
             lut[src[7]];

        src    += 8;
        intout += 2;
    }

    xmod = xsize & 7;

    //
    //  do 4 more if there are that many
    //
    if(xmod>3) {
        *intout =
            (lut[src[0]] << 24) |
            (lut[src[1]] << 16) |
            (lut[src[2]] << 8)  |
             lut[src[3]];
#endif
    }

    //
    //  Do any remaining bytes
    //
    dst = (Xil_unsigned8*)intout;
    
    switch(xmod) {
      case 3: dst[2] = lut[src[2]]; // break ommited
      case 2: dst[1] = lut[src[1]]; // break ommited
      case 1: dst[0] = lut[src[0]]; break;
      case 7: dst[6] = lut[src[6]]; // break ommited
      case 6: dst[5] = lut[src[5]]; // break ommited
      case 5: dst[4] = lut[src[4]]; break;
    }
}

XilStatus
XilDeviceManagerComputeBYTE::Lookup16(XilOp*       op,
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
    Xil_signed16* lut = (Xil_signed16*)lookup->getData() - offset*output_bands;

    //
    // Assuming a single lookup table
    //

    while(ci.hasMoreInfo()) {
	if ((ci.src1Storage.isType(XIL_PIXEL_SEQUENTIAL)) &&
	    (ci.destStorage.isType(XIL_PIXEL_SEQUENTIAL))) {
	    Xil_unsigned8* src1_scanline = (Xil_unsigned8*) ci.src1Data;
	    Xil_signed16*  dest_scanline = (Xil_signed16*)  ci.destData;
	    for(int y=ci.ysize; y>0; y--) {
		Xil_unsigned8* src1_pixel = src1_scanline;
		Xil_signed16*  dest_pixel = dest_scanline;
		for(unsigned int x=0; x<ci.xsize; x++) {
		    Xil_unsigned8* src1 = src1_pixel;
		    Xil_signed16*  dest = dest_pixel;
		    Xil_signed16* lut_band = lut + *src1 * output_bands;
		    for (int band=0; band < output_bands; band++) {
			if (*src1 > num_entries + offset) {
			    *dest = lut[(num_entries - 1) * output_bands+band];
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
	    int src1PixelStride = ci.getSrc1PixelStride(0);
	    int src1ScanlineStride = ci.getSrc1ScanlineStride(0);
	    for(int band=0; band < output_bands; band++) { 
		int destPixelStride = ci.getDestPixelStride(band);
		int destScanlineStride = ci.getDestScanlineStride(band);
		Xil_unsigned8* src1_scanline=(Xil_unsigned8*)ci.getSrc1Data(0);
		Xil_signed16*  dest_scanline=(Xil_signed16*)
		    ci.getDestData(band);
		for(int y=ci.ysize; y>0; y--) { 
		    Xil_unsigned8* src1 = src1_scanline;
		    Xil_signed16*  dest = dest_scanline;
		    for(int x=ci.xsize; x>0; x--) {
			Xil_signed16  *lut_band = lut + (*src1 * output_bands);
			
			*dest = lut_band[band];
			
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
XilDeviceManagerComputeBYTE::Lookupf32(XilOp*       op,
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

    //
    // Assuming a single lookup table
    //

    while(ci.hasMoreInfo()) {
	if ((ci.src1Storage.isType(XIL_PIXEL_SEQUENTIAL)) &&
	    (ci.destStorage.isType(XIL_PIXEL_SEQUENTIAL))) {
	    Xil_unsigned8* src1_scanline = (Xil_unsigned8*) ci.src1Data;
	    Xil_float32*   dest_scanline = (Xil_float32*)   ci.destData;
	    for(int y=ci.ysize; y>0; y--) {
		Xil_unsigned8* src1_pixel = src1_scanline;
		Xil_float32*   dest_pixel = dest_scanline;
		for(unsigned int x=0; x<ci.xsize; x++) {
		    Xil_unsigned8* src1 = src1_pixel;
		    Xil_float32*   dest = dest_pixel;
		    Xil_float32*   lut_band = lut + *src1 * output_bands;
		    for (int band=0; band < output_bands; band++) {
			if (*src1 > num_entries + offset) {
			    *dest = lut[(num_entries - 1) * output_bands+band];
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
		Xil_unsigned8* src1_scanline=(Xil_unsigned8*)ci.getSrc1Data(0);
		Xil_float32*   dest_scanline=(Xil_float32*)
		    ci.getDestData(band);
		for(int y=ci.ysize; y>0; y--) { 
		    Xil_unsigned8* src1 = src1_scanline;
		    Xil_float32*   dest = dest_scanline;
		    for(int x=ci.xsize; x>0; x--) {
			Xil_float32 *lut_band = lut + (*src1 * output_bands);
			
			*dest = lut_band[band];
			
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


