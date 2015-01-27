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
//  Revision:	1.1
//  Last Mod:	16:53:06, 02/07/96
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
#pragma ident	"@(#)NearestColor.cc	1.1\t96/02/07  "

#ifdef _WINDOWS
#include "time.h"
#else
#include "sys/time.h"
#endif
#include "XilDeviceManagerComputeBIT.hh"
#include "ComputeInfo.hh"

struct NearestColor8Data {
    Xil_unsigned8* values;
};
    
struct NearestColor16Data {
    Xil_signed16*  values;
};
    
static XilStatus
singleBand1(XilOp*          op,
	    unsigned        op_count,
	    XilRoi*         roi,
	    XilBoxList*     bl);

static XilStatus
default1(XilOp*          op,
	 unsigned        op_count,
	 XilRoi*         roi,
	 XilBoxList*     bl,
	 Xil_unsigned32* squares_table);

static XilStatus
singleBand8(XilOp*             op,
	    unsigned           op_count,
	    XilRoi*            roi,
	    XilBoxList*        bl,
            NearestColor8Data* ncd);

static XilStatus
default8(XilOp*          op,
	 unsigned        op_count,
	 XilRoi*         roi,
	 XilBoxList*     bl,
	 Xil_unsigned32* squares_table);

static XilStatus
singleBand16(XilOp*              op,
	     unsigned            op_count,
	     XilRoi*             roi,
	     XilBoxList*         bl,
             NearestColor16Data* ncd);

static XilStatus
default16(XilOp*          op,
	  unsigned        op_count,
	  XilRoi*         roi,
	  XilBoxList*     bl,
	  Xil_unsigned32* squares_table);

XilStatus
XilDeviceManagerComputeBIT::NearestColor1(XilOp*       op,
					   unsigned     op_count,
					   XilRoi*      roi,
					   XilBoxList*  bl)
{
    XilStatus returnValue;
    
    XilLookupSingle* lookup;
    op->getParam(1, (XilObject**) &lookup);

    unsigned short lut_nbands = lookup->getOutputNBands();

    Xil_unsigned32 * squares_table = getSquaresTable();

    if (lut_nbands == 1)
	returnValue = singleBand1(op, op_count, roi, bl);
    else
	//
	// TODO : call into the fast NC lut routine from 1.2
	//        if bands is < 32. oconnor 1-Feb-96
        //
	returnValue = default1(op, op_count, roi, bl, squares_table);
    
    return returnValue;
}


static XilStatus
singleBand1(XilOp*          op,
	    unsigned        op_count,
	    XilRoi*         roi,
	    XilBoxList*     bl)
{
    ComputeInfoBIT  ci(op, op_count, roi, bl);

    if(ci.isOK() == FALSE) {
        return ci.returnValue;
    }

    XilLookupSingle* lookup;
    op->getParam(1, (XilObject**) &lookup);

    Xil_unsigned8* lut=(Xil_unsigned8*)lookup->getData();
    int offset = lookup->getOffset();

    while(ci.hasMoreInfo()) {
	int sR_offset = ci.src1Offset;
	int dR_offset = ci.destOffset;
	Xil_unsigned8* src1_scanline = ci.src1Data;
	Xil_unsigned8* dest_scanline = ci.destData;
	for(int y=ci.ysize; y>0; y--) { // each scanline
	    Xil_unsigned8* src1 = src1_scanline;
	    Xil_unsigned8* dest = dest_scanline;

	    for (int x=0; x<ci.xsize; x++) { // each pixel
		int pix = (Xil_unsigned8)((XIL_BMAP_TST(src1, (sR_offset + x))) ? 1 : 0);
                if (offset + ((pix == (*lut & 0x1)) ? (0) : (1)))
		    XIL_BMAP_SET(dest, dR_offset + x);
		else
		    XIL_BMAP_CLR(dest, dR_offset + x);
	    }
	    src1_scanline += ci.src1ScanlineStride;
	    dest_scanline += ci.destScanlineStride;
	}
    }

    return ci.returnValue;
}
static XilStatus
default1(XilOp*          op,
	 unsigned        op_count,
	 XilRoi*         roi,
	 XilBoxList*     bl,
	 Xil_unsigned32* squares_table)
{
    ComputeInfoGENERAL  ci(op, op_count, roi, bl);

    if(ci.isOK() == FALSE) {
        return ci.returnValue;
    }

    XilLookupSingle* lookup;
    op->getParam(1, (XilObject**) &lookup);

    unsigned int   lut_size = lookup->getNumEntries();
    Xil_unsigned8* lut=(Xil_unsigned8*)lookup->getData();
    unsigned short lut_nbands = lookup->getOutputNBands();
    short          lut_offset = lookup->getOffset();

    while(ci.hasMoreInfo()) {
	int sR_offset = ci.src1Offset;
	int dR_offset = ci.destOffset;
	int band;
	// Create source data pointer tables
	Xil_unsigned8** srcPtrTable = new Xil_unsigned8*[ci.src1NumBands];
	Xil_unsigned8** srcStartLineTable = new Xil_unsigned8*[ci.src1NumBands];
	for (int i = 0; i < ci.src1NumBands; i++)
	    srcStartLineTable[i] = (Xil_unsigned8*)ci.getSrc1Data(i);
	
	Xil_unsigned8* dest_scanline = (Xil_unsigned8*)ci.destData;
	
	for(int y=ci.ysize; y>0; y--) { // each scanline
	    for (band = 0; band < ci.src1NumBands; band++)
		srcPtrTable[band] = srcStartLineTable[band];
	    
	    Xil_unsigned8* dest = dest_scanline;

	    for (int x=0; x<ci.xsize; x++) { // each pixel
		int distance0 = 0;
		int distance1 = 0;
		
		for (band = 0; band < ci.src1NumBands; band++) {
		    int pix= XIL_BMAP_TST(srcPtrTable[band], sR_offset + x) ? 1 : 0;
		    distance0 += squares_table[pix -(lut[band] & 0x1)];
		    distance1 += squares_table[pix -(lut[band+1] & 0x1)];
		}

		if (lut_offset + ((distance1 < distance0) ? 1 : 0))
		    XIL_BMAP_SET(dest, dR_offset + x);
		else
		    XIL_BMAP_CLR(dest, dR_offset + x);
	    }
	    
	    for (band = 0; band < ci.src1NumBands; band++)
		srcStartLineTable[band] += ci.getSrc1ScanlineStride(band);
	    dest_scanline += ci.destScanlineStride;
	}
	
	delete srcPtrTable;
	delete srcStartLineTable;
    }

    return ci.returnValue;
}

XilStatus
XilDeviceManagerComputeBIT::NearestColor8Preprocess(XilOp*        op,
						    unsigned      ,
						    XilRoi*       ,
						    void**        compute_data,
                                                    unsigned int* )
{
    XilImage* dst = op->getDstImage(1);

    //
    // Create a NearestColor structure no matter what
    //
    NearestColor8Data* ncd = new NearestColor8Data;
    if(ncd == NULL) {
        XIL_ERROR(dst->getSystemState(), XIL_ERROR_RESOURCE, "di-1", TRUE);
        return XIL_FAILURE;
    }
    ncd->values = NULL;

    XilLookupSingle* lookup;
    op->getParam(1, (XilObject**) &lookup);
    unsigned int num_output_bands = lookup->getOutputNBands();

    //
    // Only set these up if there is only one band
    //
    if (num_output_bands == 1) {
	Xil_unsigned8* values = new Xil_unsigned8[2];
	if(values == NULL) {
	    XIL_ERROR(dst->getSystemState(), XIL_ERROR_RESOURCE,
		      "di-1", TRUE);
	    return XIL_FAILURE;
	}

	int num_entries = lookup->getNumEntries();
	Xil_unsigned8* lut_data = (Xil_unsigned8*) lookup->getData();

	//
	// Look for the first low bit is zero entry
	//
	int found = -1;
	for (int j =0; j < num_entries; j++) {
	    if ((lut_data[j] & 0x1) == 0) {
		found = j;
		break;
	    }
	}
	values[0] = found;

	//
	// Look for the first low bit is one entry
	found = -1;
	for (j =0; j < num_entries; j++) {
	    if ((lut_data[j] & 0x1) == 1) {
		found = j;
		break;
	    }
	}

	//
	// if didn't find a one, then just use the value for the zero
	// entry which is of course 0. QED.
	if (found == -1)
	    values[1] = values[0];
	else {
	    //
	    // We have a valid value for finding a one, check the entry
	    // for zero, and set it to the entry for finding a one which
	    // would of course be 0.  QED.
	    values[1] = found;
	    if ((int)values[0] == -1)
		values[0] = values[1];
	}

	//
	// Add the offset here, otherwise, it will be added every time
	// in the compute routine
	values[0] += lookup->getOffset();
	values[1] += lookup->getOffset();
	ncd->values = values;
    }

    *compute_data = ncd;
    
    return XIL_SUCCESS;
}

XilStatus
XilDeviceManagerComputeBIT::NearestColor8Postprocess(XilOp*     ,
						     void*      compute_data)
{
    NearestColor8Data* ncd = (NearestColor8Data*)compute_data;

    if (ncd->values != NULL)
	delete ncd->values;
    delete ncd;
    
    return XIL_SUCCESS;
}


XilStatus
XilDeviceManagerComputeBIT::NearestColor8(XilOp*       op,
					   unsigned     op_count,
					   XilRoi*      roi,
					   XilBoxList*  bl)
{
    XilStatus returnValue;
    
    XilLookupSingle* lookup;
    op->getParam(1, (XilObject**) &lookup);

    unsigned short lut_nbands = lookup->getOutputNBands();

    Xil_unsigned32 * squares_table = getSquaresTable();
    
    if(lut_nbands == 1) {
        NearestColor8Data* ncd =
            (NearestColor8Data*)op->getPreprocessData(this);

	returnValue = singleBand8(op, op_count, roi, bl, ncd);
    } else {
	//
	// TODO : call into the fast NC lut routine from 1.2
	//        if bands is < 32. oconnor 1-Feb-96
        //
	returnValue = default8(op, op_count, roi, bl, squares_table);
    }
    
    return(returnValue);
}


static XilStatus
singleBand8(XilOp*             op,
	    unsigned           op_count,
	    XilRoi*            roi,
	    XilBoxList*        bl,
            NearestColor8Data* ncd)
{
    ComputeInfoGENERAL  ci(op, op_count, roi, bl);

    if(ci.isOK() == FALSE) {
        return ci.returnValue;
    }

    while(ci.hasMoreInfo()) {
	int sR_offset = ci.src1Offset;
	// Create source data pointer tables
	Xil_unsigned8* src1_scanline = (Xil_unsigned8*)ci.src1Data;
	Xil_unsigned8* dest_scanline = (Xil_unsigned8*)ci.destData;
	for(int y=ci.ysize; y>0; y--) { // each scanline
	    Xil_unsigned8* src1 = src1_scanline;
	    Xil_unsigned8* dest = dest_scanline;

	    for (int x=0; x<ci.xsize; x++) { // each pixel
		*dest = ncd->values[(XIL_BMAP_TST(src1, sR_offset + x) ? 1 : 0)];
		dest += ci.destPixelStride;
	    }
	    
	    src1_scanline += ci.src1ScanlineStride;
	    dest_scanline += ci.destScanlineStride;
	}
    }

    return ci.returnValue;
}
static XilStatus
default8(XilOp*          op,
	 unsigned        op_count,
	 XilRoi*         roi,
	 XilBoxList*     bl,
	 Xil_unsigned32* squares_table)
{
    ComputeInfoGENERAL  ci(op, op_count, roi, bl);

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
	int sR_offset = ci.src1Offset;
	int band;
	// Create source data pointer tables
	Xil_unsigned8** srcPtrTable = new Xil_unsigned8*[ci.src1NumBands];
	Xil_unsigned8** srcStartLineTable = new Xil_unsigned8*[ci.src1NumBands];
	for (int i = 0; i < ci.src1NumBands; i++)
	    srcStartLineTable[i] = (Xil_unsigned8*)ci.getSrc1Data(i);
	
	Xil_unsigned8* dest_scanline = (Xil_unsigned8*)ci.destData;
	
	for(int y=ci.ysize; y>0; y--) { // each scanline
	    for (band = 0; band < ci.src1NumBands; band++)
		srcPtrTable[band] = srcStartLineTable[band];
	    
	    Xil_unsigned8* dest = dest_scanline;

	    for (int x=0; x<ci.xsize; x++) { // each pixel
		int closest_entry;
		int closest_dist = INT_MAX;
		
		for(int idx=0; idx < max_index; idx += entry_size) {
		    int distance = 0;
		    
		    for (band = 0; band < ci.src1NumBands; band++) {
			int pix= XIL_BMAP_TST(srcPtrTable[band], sR_offset + x) ? 1 : 0;
			distance += squares_table[pix -(lut[band + idx] & 0x1)];
		    }
		    
		    // compare if distance is shorter than previous
		    if (distance < closest_dist) {
			closest_dist = distance;
			closest_entry = idx / entry_size;
		    }
		}
		*dest = lut_offset + closest_entry;
		dest += ci.destPixelStride;
	    }
	    
	    for (band = 0; band < ci.src1NumBands; band++)
		srcStartLineTable[band] += ci.getSrc1ScanlineStride(band);
	    
	    dest_scanline += ci.destScanlineStride;
	}
	
	delete srcPtrTable;
	delete srcStartLineTable;
    }

    return ci.returnValue;
}

XilStatus
XilDeviceManagerComputeBIT::NearestColor16Preprocess(XilOp*        op,
                                                     unsigned      ,
                                                     XilRoi*       ,
                                                     void**        compute_data,
                                                     unsigned int* )
{
    XilImage* dst = op->getDstImage(1);

    //
    // Create a NearestColor structure no matter what
    //
    NearestColor16Data* ncd = new NearestColor16Data;
    if(ncd == NULL) {
        XIL_ERROR(dst->getSystemState(), XIL_ERROR_RESOURCE, "di-1", TRUE);
        return XIL_FAILURE;
    }
    ncd->values = NULL;

    XilLookupSingle* lookup;
    op->getParam(1, (XilObject**) &lookup);
    unsigned int num_output_bands = lookup->getOutputNBands();

    //
    // Only set these up if there is only one band
    //
    if (num_output_bands == 1) {
	Xil_signed16* values = new Xil_signed16[2];
	if(values == NULL) {
	    XIL_ERROR(dst->getSystemState(), XIL_ERROR_RESOURCE,
		      "di-1", TRUE);
	    return XIL_FAILURE;
	}

	int num_entries = lookup->getNumEntries();
	Xil_unsigned8* lut_data = (Xil_unsigned8*) lookup->getData();

	//
	// Look for the first low bit is zero entry
	//
	int found = -1;
	for (int j =0; j < num_entries; j++) {
	    if ((lut_data[j] & 0x1) == 0) {
		found = j;
		break;
	    }
	}
	values[0] = found;

	//
	// Look for the first low bit is one entry
	found = -1;
	for (j =0; j < num_entries; j++) {
	    if ((lut_data[j] & 0x1) == 1) {
		found = j;
		break;
	    }
	}

	//
	// if didn't find a one, then just use the value for the zero
	// entry which is of course 0. QED.
	if (found == -1)
	    values[1] = values[0];
	else {
	    //
	    // We have a valid value for finding a one, check the entry
	    // for zero, and set it to the entry for finding a one which
	    // would of course be 0.  QED.
	    values[1] = found;
	    if (values[0] == -1)
		values[0] = values[1];
	}

	//
	// Add the offset here, otherwise, it will be added every time
	// in the compute routine
	values[0] += lookup->getOffset();
	values[1] += lookup->getOffset();
	ncd->values = values;
    }

    *compute_data = ncd;
    
    return XIL_SUCCESS;
}

XilStatus
XilDeviceManagerComputeBIT::NearestColor16Postprocess(XilOp*     ,
						     void*      compute_data)
{
    NearestColor16Data* ncd = (NearestColor16Data*)compute_data;

    if (ncd->values != NULL)
	delete ncd->values;
    delete ncd;
    
    return XIL_SUCCESS;
}


XilStatus
XilDeviceManagerComputeBIT::NearestColor16(XilOp*       op,
					   unsigned     op_count,
					   XilRoi*      roi,
					   XilBoxList*  bl)
{
    XilStatus returnValue;
    
    XilLookupSingle* lookup;
    op->getParam(1, (XilObject**) &lookup);

    unsigned short lut_nbands = lookup->getOutputNBands();

    Xil_unsigned32 * squares_table = getSquaresTable();

    if(lut_nbands == 1) {
        NearestColor16Data* ncd =
            (NearestColor16Data*)op->getPreprocessData(this);

	returnValue = singleBand16(op, op_count, roi, bl, ncd);
    } else {
	//
	// TODO : call into the fast NC lut routine from 1.2
	//        if bands is < 32. oconnor 1-Feb-96
	returnValue = default16(op, op_count, roi, bl, squares_table);
    }
    
    return returnValue;
}


static XilStatus
singleBand16(XilOp*              op,
	     unsigned            op_count,
	     XilRoi*             roi,
	     XilBoxList*         bl,
             NearestColor16Data* ncd)
{
    ComputeInfoGENERAL  ci(op, op_count, roi, bl);

    if(ci.isOK() == FALSE) {
        return ci.returnValue;
    }

    while(ci.hasMoreInfo()) {
	int sR_offset = ci.src1Offset;
	// Create source data pointer tables
	Xil_unsigned8* src1_scanline = (Xil_unsigned8*)ci.src1Data;
	Xil_signed16* dest_scanline = (Xil_signed16*)ci.destData;
	for(int y=ci.ysize; y>0; y--) { // each scanline
	    Xil_unsigned8* src1 = src1_scanline;
	    Xil_signed16* dest = dest_scanline;

	    for (int x=0; x<ci.xsize; x++) { // each pixel
		*dest = ncd->values[(XIL_BMAP_TST(src1, sR_offset + x) ? 1 : 0)];
		dest += ci.destPixelStride;
	    }
	    
	    src1_scanline += ci.src1ScanlineStride;
	    dest_scanline += ci.destScanlineStride;
	}
    }

    return ci.returnValue;
}
static XilStatus
default16(XilOp*          op,
	  unsigned        op_count,
	  XilRoi*         roi,
	  XilBoxList*     bl,
	  Xil_unsigned32* squares_table)
{
    ComputeInfoGENERAL  ci(op, op_count, roi, bl);

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
	int sR_offset = ci.src1Offset;
	int band;
	// Create source data pointer tables
	Xil_unsigned8** srcPtrTable = new Xil_unsigned8*[ci.src1NumBands];
	Xil_unsigned8** srcStartLineTable = new Xil_unsigned8*[ci.src1NumBands];
	for (int i = 0; i < ci.src1NumBands; i++)
	    srcStartLineTable[i] = (Xil_unsigned8*)ci.getSrc1Data(i);
	
	Xil_signed16* dest_scanline = (Xil_signed16*)ci.destData;
	
	for(int y=ci.ysize; y>0; y--) { // each scanline
	    for (band = 0; band < ci.src1NumBands; band++)
		srcPtrTable[band] = srcStartLineTable[band];
	    
	    Xil_signed16* dest = dest_scanline;

	    for (int x=0; x<ci.xsize; x++) { // each pixel
		int closest_entry;
		int closest_dist = INT_MAX;
		
		for(int idx=0; idx < max_index; idx += entry_size) {
		    int distance = 0;
		    
		    for (band = 0; band < ci.src1NumBands; band++) {
			int pix= XIL_BMAP_TST(srcPtrTable[band], sR_offset + x) ? 1 : 0;
			distance += squares_table[pix -(lut[band + idx] & 0x1)];
		    }
		    
		    // compare if distance is shorter than previous
		    if (distance < closest_dist) {
			closest_dist = distance;
			closest_entry = idx / entry_size;
		    }
		}
		*dest = lut_offset + closest_entry;
		dest += ci.destPixelStride;
	    }
	    
	    for (band = 0; band < ci.src1NumBands; band++)
		srcStartLineTable[band] += ci.getSrc1ScanlineStride(band);
	    
	    dest_scanline += ci.destScanlineStride;
	}
	
	delete srcPtrTable;
	delete srcStartLineTable;
    }

    return ci.returnValue;
}


