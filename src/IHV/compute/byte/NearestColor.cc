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
//  File:	NearestColor.cc
//  Project:	XIL
//  Revision:	1.21
//  Last Mod:	10:10:13, 03/10/00
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
//  MT-level:  SAFE
//	
//------------------------------------------------------------------------
//	COPYRIGHT
//------------------------------------------------------------------------
#pragma ident	"@(#)NearestColor.cc	1.21\t00/03/10  "

#include "XilDeviceManagerComputeBYTE.hh"
#include "ComputeInfo.hh"
#include "XiliNearestIndexSelector.hh"

static XilMutex nearestColorMutex;

struct NearestColorData {
    Xil_unsigned8*            grid;
    XiliNearestIndexSelector* indexSelector;
};
    
static XilStatus
singleBand8(XilOp*            op,
	    unsigned          op_count,
	    XilRoi*           roi,
	    XilBoxList*       bl,
            NearestColorData* ncd);

static XilStatus
threeBand8(XilOp*            op,
           unsigned          op_count,
           XilRoi*           roi,
           XilBoxList*       bl,
            NearestColorData* ncd);

static XilStatus
default8(XilOp*          op,
	 unsigned        op_count,
	 XilRoi*         roi,
	 XilBoxList*     bl,
	 Xil_unsigned32* squares_table);

XilStatus
XilDeviceManagerComputeBYTE::NearestColor1(XilOp*       op,
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

    Xil_unsigned32 * squares_table = getSquaresTable(ci.getSystemState());
    
    unsigned int   lut_size = lookup->getNumEntries();
    Xil_unsigned8* lut=(Xil_unsigned8*)lookup->getData();
    unsigned short lut_nbands = lookup->getOutputNBands();
    short          lut_offset = lookup->getOffset();
    int            entry_size = lut_nbands * sizeof (Xil_unsigned8),
	           max_index  = lut_size * entry_size;

    while(ci.hasMoreInfo()) {
	int dR_offset = ci.destOffset;
	//
	// destination is always single banded so don't check it's storage
	//
        if ((ci.src1Storage.isType(XIL_PIXEL_SEQUENTIAL))) {
            Xil_unsigned8* src1_scanline = (Xil_unsigned8*) ci.src1Data;
            Xil_unsigned8* dest_scanline = (Xil_unsigned8*) ci.destData;
            for(int y=ci.ysize; y>0; y--) {
                Xil_unsigned8* src1 = src1_scanline;
                Xil_unsigned8* dest = dest_scanline;
		int closest_index;
                for(unsigned int x = 0; x < ci.xsize; x++) {
		    int closest_dist = INT_MAX;
                    for(int b=0; b < max_index; b += entry_size) {
			// go throught the whole LUT
			int distance = 0;
			for (int i = lut_nbands - 1; i >= 0; i--) {
			    int diff = (int) (*(src1 + i) - lut[b + i]);
			    distance += squares_table[diff];
			}
			// compare if distance is shorter than previous
			if (distance < closest_dist) {
			    closest_dist = distance;
			    closest_index = b;
			}
		    } // for colormap size

		    if ((lut_offset + closest_index / entry_size))
			XIL_BMAP_SET(dest, dR_offset + x);
		    else
			XIL_BMAP_CLR(dest, dR_offset + x);

                    src1 += ci.src1PixelStride;
                }
                src1_scanline += ci.src1ScanlineStride;
                dest_scanline += ci.destScanlineStride;
            }
        } else {
	    unsigned int band;
	    // Create source data pointer tables
	    Xil_unsigned8** srcPtrTable =
		new Xil_unsigned8*[ci.src1NumBands];
	    Xil_unsigned8** srcStartLineTable =
		new Xil_unsigned8*[ci.src1NumBands];
	    for (unsigned int i = 0; i < ci.src1NumBands; i++)
		srcStartLineTable[i] = (Xil_unsigned8*) ci.getSrc1Data(i);
	    
	    Xil_unsigned8* dest_scanline = (Xil_unsigned8*) ci.destData;

	    for(unsigned int y=ci.ysize; y>0; y--) { // each scanline
		for (band = 0; band < ci.src1NumBands; band++)
		    srcPtrTable[band] = srcStartLineTable[band];
		
		Xil_unsigned8* dest = dest_scanline;

		for (unsigned int x=0; x < ci.xsize; x++) { // each pixel
		    int closest_index;
		    int closest_dist = INT_MAX;
		    
		    for(int idx=0; idx < max_index; idx += entry_size) {
			int distance = 0;
			
			for (band = 0; band < ci.src1NumBands; band++) {
			    int diff = *(srcPtrTable[band]) - lut[band+idx];
			    distance += squares_table[diff];
			}
			
			// compare if distance is shorter than previous
			if (distance < closest_dist) {
			    closest_dist = distance;
			    closest_index  = idx;
			}
		    }

		    if ((lut_offset + closest_index / entry_size))
			XIL_BMAP_SET(dest, dR_offset + x);
		    else
			XIL_BMAP_CLR(dest, dR_offset + x);

		    for (band = 0; band < ci.src1NumBands; band++)
			srcPtrTable[band] += ci.getSrc1PixelStride(band);
		}
			
		for (band = 0; band < ci.src1NumBands; band++)
		    srcStartLineTable[band] += ci.getSrc1ScanlineStride(band);
			
		dest_scanline += ci.destScanlineStride;
            }
	    
	    delete srcPtrTable;
	    delete srcStartLineTable;
        }
    }

    return ci.returnValue;

}


XilStatus
XilDeviceManagerComputeBYTE::NearestColor8Preprocess(XilOp*        op,
                                                     unsigned      ,
                                                     XilRoi*       ,
                                                     void**        compute_data,
                                                     unsigned int* )
{
    XilImage* dst = op->getDstImage(1);

    //
    // Create a lookup structure no matter what
    //
    NearestColorData* ncd = new NearestColorData;
    if(ncd == NULL) {
        XIL_ERROR(dst->getSystemState(), XIL_ERROR_RESOURCE, "di-1", TRUE);
        return XIL_FAILURE;
    }
    ncd->grid = NULL;

    //
    // Assume we have a single lookup.
    // If it was a colorcube, it would have been trapped in the Op
    // If it was a combined, then we shouldn't be in nearest color because
    // this op only allows single band output, thus only 1 input band LUTs.
    //
    XilLookupSingle* lookup;
    op->getParam(1, (XilObject**) &lookup);
    unsigned int num_output_bands = lookup->getOutputNBands();

    if(num_output_bands == 1) {
	Xil_unsigned8* values = new Xil_unsigned8[256];
	if(values == NULL) {
	    XIL_ERROR(dst->getSystemState(), XIL_ERROR_RESOURCE,
		      "di-1", TRUE);
	    return XIL_FAILURE;
	}
	Xil_unsigned8* lut_data   = (Xil_unsigned8*) lookup->getData();
	int            lut_size   = lookup->getNumEntries();
	int            lut_offset = lookup->getOffset();
	int byte_index, closest_byte_entry,dist;
	for (int i = 0; i < 256; i++) {
	    int closest_dist = INT_MAX;
	    for (byte_index = 0; byte_index < lut_size; byte_index++) {
		if ((dist = abs(i - (int)lut_data[byte_index]))<closest_dist){
		    closest_dist = dist;
		    closest_byte_entry = byte_index;
		}
	    }
	    values[i] = lut_offset + closest_byte_entry;
	}
	ncd->grid          = values;
        ncd->indexSelector = NULL;
    } else if(num_output_bands == 3) {
        //
        //  We can potentially use the optimized index selector...
        //
        XilLookupSingle* lut;
        op->getParam(1, (XilObject**)&lut);

        aquireNearestIndexSelector(dst->getSystemState(), op, lut);

        ncd->indexSelector = nearestIndexSelector;
    } else {
        ncd->indexSelector = NULL;
    }

    *compute_data = ncd;

    return XIL_SUCCESS;
}

XilStatus
XilDeviceManagerComputeBYTE::NearestColor8Postprocess(XilOp*      op,
						      void*       compute_data)
{
    NearestColorData* ncd = (NearestColorData*)compute_data;

    if(ncd->indexSelector != NULL) {
        releaseNearestIndexSelector(op);
    }
        
    delete ncd->grid;
    delete ncd;

    return XIL_SUCCESS;
}

XilStatus
XilDeviceManagerComputeBYTE::NearestColor8(XilOp*       op,
					   unsigned     op_count,
					   XilRoi*      roi,
					   XilBoxList*  bl)
{
    XilStatus returnValue;
    
    XilLookupSingle* lookup;
    op->getParam(1, (XilObject**) &lookup);

    unsigned short lut_nbands = lookup->getOutputNBands();

    switch (lut_nbands) {
      case 1:
      {
          NearestColorData* ncd =
              (NearestColorData*)op->getPreprocessData(this);

          returnValue = singleBand8(op, op_count, roi, bl, ncd);
      }
      break;

      case 3:
      {
          NearestColorData* ncd =
              (NearestColorData*)op->getPreprocessData(this);

          if(ncd->indexSelector != NULL) {
              //
              //  Remember, a "break" in here won't do what you want it to
              //  do...so, we just return from here directly.
              //
              return threeBand8(op, op_count, roi, bl, ncd);
          }
      }

      default:
	Xil_unsigned32* squares_table =
            getSquaresTable(op->getDstImage(1)->getSystemState());
	if(squares_table == NULL) {
	    return XIL_FAILURE;
        }

	returnValue = default8(op, op_count, roi, bl, squares_table);
	break;
    }

    return returnValue;
}


static
XilStatus
singleBand8(XilOp*            op,
	    unsigned          op_count,
	    XilRoi*           roi,
	    XilBoxList*       bl,
            NearestColorData* ncd)
{
    ComputeInfoBYTE  ci(op, op_count, roi, bl);

    if(ci.isOK() == FALSE) {
        return ci.returnValue;
    }

    while(ci.hasMoreInfo()) {
	Xil_unsigned8* src1_scanline = ci.src1Data;
	Xil_unsigned8* dest_scanline = ci.destData;
	for(int y=ci.ysize; y>0; y--) {
	    Xil_unsigned8* src1 = src1_scanline;
	    Xil_unsigned8* dest = dest_scanline;
	    for(int x=ci.xsize; x>0; x--) {
		*dest = ncd->grid[*src1];
		dest += ci.destPixelStride;
		src1 += ci.src1PixelStride;
	    }
	    src1_scanline += ci.src1ScanlineStride;
	    dest_scanline += ci.destScanlineStride;
	}
    }

    return ci.returnValue;
    
}

static XilStatus
threeBand8(XilOp*            op,
           unsigned          op_count,
           XilRoi*           roi,
           XilBoxList*       bl,
           NearestColorData* ncd)
{
    ComputeInfoBYTE  ci(op, op_count, roi, bl);

    if(ci.isOK() == FALSE) {
        return ci.returnValue;
    }

    XiliNearestIndexSelector* sel = ncd->indexSelector;

    // fixes core dump in selectCheck() routine
    nearestColorMutex.lock();

    while(ci.hasMoreInfo()) {
        if((ci.src1Storage.isType(XIL_PIXEL_SEQUENTIAL))) {
            Xil_unsigned8* src_scanline = ci.src1Data;
            Xil_unsigned8* dst_scanline = ci.destData;

            for(int y=ci.ysize; y>0; y--) {
                Xil_unsigned8* src = src_scanline;
                Xil_unsigned8* dst = dst_scanline;

                for(int x=ci.xsize; x>0; x--) {
                    //
                    //  Use the index selector to find the closest index.
                    //
		    *dst = sel->selectIndex(*src, *(src+1), *(src+2));

                    src += ci.src1PixelStride;
                    dst += ci.destPixelStride;
                }

                src_scanline += ci.src1ScanlineStride;
                dst_scanline += ci.destScanlineStride;
            }
        } else {
            Xil_unsigned8* src_scanline0 = ci.getSrc1Data(0);
            Xil_unsigned8* src_scanline1 = ci.getSrc1Data(1);
            Xil_unsigned8* src_scanline2 = ci.getSrc1Data(2);

            unsigned int   src_band0_ps  = ci.getSrc1PixelStride(0);
            unsigned int   src_band1_ps  = ci.getSrc1PixelStride(1);
            unsigned int   src_band2_ps  = ci.getSrc1PixelStride(2);

            unsigned int   src_band0_ss  = ci.getSrc1ScanlineStride(0);
            unsigned int   src_band1_ss  = ci.getSrc1ScanlineStride(1);
            unsigned int   src_band2_ss  = ci.getSrc1ScanlineStride(2);

            Xil_unsigned8* dst_scanline = ci.destData;
            for(int y=ci.ysize; y>0; y--) {
                Xil_unsigned8* src_band0 = src_scanline0;
                Xil_unsigned8* src_band1 = src_scanline1;
                Xil_unsigned8* src_band2 = src_scanline2;
                Xil_unsigned8* dst       = dst_scanline;

                for(int x=ci.xsize; x>0; x--) {
                    //
                    //  Use the index selector to find the closest index.
                    //
		    *dst = sel->selectIndex(*src_band0,
                                            *src_band1,
                                            *src_band2);

                    dst       += ci.destPixelStride;
                    src_band0 += src_band0_ps;
                    src_band1 += src_band1_ps;
                    src_band2 += src_band2_ps;
                }

                dst_scanline  += ci.destScanlineStride;
                src_scanline0 += src_band0_ss;
                src_scanline1 += src_band1_ss;
                src_scanline2 += src_band2_ss;
            }
        }
    }

    nearestColorMutex.unlock();
    return ci.returnValue;
}

static XilStatus
default8(XilOp*          op,
	 unsigned        op_count,
	 XilRoi*         roi,
	 XilBoxList*     bl,
	 Xil_unsigned32* squares_table)
{
    ComputeInfoBYTE  ci(op, op_count, roi, bl);

    if(ci.isOK() == FALSE) {
        return ci.returnValue;
    }

    XilLookupSingle* lookup;
    op->getParam(1, (XilObject**) &lookup);

    unsigned int   lut_size = lookup->getNumEntries();
    Xil_unsigned8* lut=(Xil_unsigned8*)lookup->getData();
    unsigned short lut_nbands = lookup->getOutputNBands();
    short          lut_offset = lookup->getOffset();
    int            entry_size = lut_nbands * sizeof (Xil_unsigned8),
	           max_index  = lut_size * entry_size;

    while(ci.hasMoreInfo()) {
        if ((ci.src1Storage.isType(XIL_PIXEL_SEQUENTIAL))) {
            Xil_unsigned8* src1_scanline = ci.src1Data;
            Xil_unsigned8* dest_scanline = ci.destData;
            for(int y=ci.ysize; y>0; y--) {
                Xil_unsigned8* src1 = src1_scanline;
                Xil_unsigned8* dest = dest_scanline;
		int closest_entry;
                for(int x=ci.xsize; x>0; x--) {
		    int closest_dist = INT_MAX;
                    for(int b=0; b < max_index; b += entry_size) {
			// go throught the whole LUT
			int distance = 0;
			for (int i = lut_nbands - 1; i >= 0; i--) {
			    int diff = (int) (*(src1 + i) - lut[b + i]);
			    distance += squares_table[diff];
			}
			// compare if distance is shorter than previous
			if (distance < closest_dist) {
			    closest_dist = distance;
			    closest_entry = b / entry_size;
			}
		    } // for colormap size

		    *dest = lut_offset + closest_entry;

                    src1 += ci.src1PixelStride;
                    dest += ci.destPixelStride;
                }
                src1_scanline += ci.src1ScanlineStride;
                dest_scanline += ci.destScanlineStride;
            }
        } else {
	    unsigned int band;
	    // Create source data pointer tables
	    Xil_unsigned8** srcPtrTable = new Xil_unsigned8*[ci.src1NumBands];
	    Xil_unsigned8** srcStartLineTable = new Xil_unsigned8*[ci.src1NumBands];
	    for (unsigned int i = 0; i < ci.src1NumBands; i++)
		srcStartLineTable[i] = ci.getSrc1Data(i);
	    
	    Xil_unsigned8* dest_scanline = ci.destData;

	    for(int y=ci.ysize; y>0; y--) { // each scanline
		for (band = 0; band < ci.src1NumBands; band++)
		    srcPtrTable[band] = srcStartLineTable[band];
		
		Xil_unsigned8* dest = dest_scanline;
		
		for(int x=ci.xsize; x>0; x--) { // each pixel
		    int closest_entry;
		    int closest_dist = INT_MAX;
		    
		    for(int idx=0; idx < max_index; idx += entry_size) {
			int distance = 0;
			
			for (band = 0; band < ci.src1NumBands; band++) {
			    int diff = *(srcPtrTable[band]) - lut[band+idx];
			    distance += squares_table[diff];
			}
			
			// compare if distance is shorter than previous
			if (distance < closest_dist) {
			    closest_dist = distance;
			    closest_entry = idx / entry_size;
			}
		    }

		    *dest = lut_offset + closest_entry;
		    for (band = 0; band < ci.src1NumBands; band++)
			srcPtrTable[band] += ci.getSrc1PixelStride(band);
		    dest += ci.destPixelStride;
		}
			
		for (band = 0; band < ci.src1NumBands; band++)
		    srcStartLineTable[band] += ci.getSrc1ScanlineStride(band);
			
		dest_scanline += ci.destScanlineStride;
            }
	    
	    delete srcPtrTable;
	    delete srcStartLineTable;
        }
    }

    return ci.returnValue;
}

XilStatus
XilDeviceManagerComputeBYTE::NearestColor16(XilOp*       op,
					    unsigned     op_count,
					    XilRoi*      roi,
					    XilBoxList*  bl)
{
    ComputeInfoGENERAL  ci(op, op_count, roi, bl);

    if(ci.isOK() == FALSE) {
        return ci.returnValue;
    }

    //
    // Assuming number of entries in colormap is not greater than 65536
    // GPG (giant programming guy) says this is impossible.
    //
    XilLookupSingle* lookup;
    op->getParam(1, (XilObject**) &lookup);

    Xil_unsigned32 * squares_table = getSquaresTable(ci.getSystemState());
    
    unsigned int   lut_size = lookup->getNumEntries();
    Xil_unsigned8*  lut=(Xil_unsigned8*)lookup->getData();
    unsigned short lut_nbands = lookup->getOutputNBands();
    short          lut_offset = lookup->getOffset();
    int            entry_size = lut_nbands * sizeof (Xil_unsigned8),
	           max_index  = lut_size * entry_size;

    while(ci.hasMoreInfo()) {
	//
	// destination is always single banded so don't check it's storage
	//
        if ((ci.src1Storage.isType(XIL_PIXEL_SEQUENTIAL))) {
            Xil_unsigned8* src1_scanline = (Xil_unsigned8*) ci.src1Data;
            Xil_signed16*  dest_scanline = (Xil_signed16*)  ci.destData;
            for(int y=ci.ysize; y>0; y--) {
                Xil_unsigned8* src1 = src1_scanline;
                Xil_signed16*  dest = dest_scanline;
		int closest_entry;
                for(int x=ci.xsize; x>0; x--) {
		    int closest_dist = INT_MAX;
                    for(int b=0; b < max_index; b += entry_size) {
			// go throught the whole LUT
			int distance = 0;
			for (int i = lut_nbands - 1; i >= 0; i--) {
			    int diff = (int) (*(src1 + i) - lut[b + i]);
			    distance += squares_table[diff];
			}
			// compare if distance is shorter than previous
			if (distance < closest_dist) {
			    closest_dist = distance;
			    closest_entry = b / entry_size;
			}
		    } // for colormap size
		    *dest = lut_offset + closest_entry;
                    src1 += ci.src1PixelStride;
                    dest += ci.destPixelStride;
                }
                src1_scanline += ci.src1ScanlineStride;
                dest_scanline += ci.destScanlineStride;
            }
        } else {
	    unsigned int band;
	    // Create source data pointer tables
	    Xil_unsigned8** srcPtrTable =
		new Xil_unsigned8*[ci.src1NumBands];
	    Xil_unsigned8** srcStartLineTable =
		new Xil_unsigned8*[ci.src1NumBands];
	    for (unsigned int i = 0; i < ci.src1NumBands; i++)
		srcStartLineTable[i] = (Xil_unsigned8*) ci.getSrc1Data(i);
	    
	    Xil_signed16* dest_scanline = (Xil_signed16*) ci.destData;

	    for(int y=ci.ysize; y>0; y--) { // each scanline
		for (band = 0; band < ci.src1NumBands; band++)
		    srcPtrTable[band] = srcStartLineTable[band];
		
		Xil_signed16* dest = dest_scanline;
		
		for(int x=ci.xsize; x>0; x--) { // each pixel
		    int closest_entry;
		    int closest_dist = INT_MAX;
		    
		    for(int idx=0; idx < max_index; idx += entry_size) {
			int distance = 0;
			
			for (band = 0; band < ci.src1NumBands; band++) {
			    int diff = *(srcPtrTable[band]) - lut[band+idx];
			    distance += squares_table[diff];
			}
			
			// compare if distance is shorter than previous
			if (distance < closest_dist) {
			    closest_dist = distance;
			    closest_entry = idx / entry_size;
			}
		    }

		    *dest = lut_offset + closest_entry;
		    for (band = 0; band < ci.src1NumBands; band++)
			srcPtrTable[band] += ci.getSrc1PixelStride(band);
		    dest += ci.destPixelStride;
		}
			
		for (band = 0; band < ci.src1NumBands; band++)
		    srcStartLineTable[band] += ci.getSrc1ScanlineStride(band);
			
		dest_scanline += ci.destScanlineStride;
            }
	    
	    delete srcPtrTable;
	    delete srcStartLineTable;
        }
    }

    return ci.returnValue;
}

