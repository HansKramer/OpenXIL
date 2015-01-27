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
//  File:	LookupCombined.cc
//  Project:	XIL
//  Revision:	1.17
//  Last Mod:	10:10:33, 03/10/00
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
#pragma ident	"@(#)LookupCombined.cc	1.17\t00/03/10  "

#include "XilDeviceManagerComputeBYTE.hh"
#include "ComputeInfo.hh"

struct LookupCombined8Data {
    Xil_unsigned8** values;
    Xil_boolean*    allocated;
    int             num_bands;
};
    
struct LookupCombined16Data {
    Xil_signed16** values;
    Xil_boolean*   allocated;
    int            num_bands;
};
    
XilStatus
XilDeviceManagerComputeBYTE::LookupCombined8Preprocess(XilOp*        op,
                                                       unsigned      ,
                                                       XilRoi*       ,
                                                       void**        compute_data,
                                                       unsigned int* )
{
    XilImage* dst = op->getDstImage(1);

    //
    // Create a lookup structure no matter what
    //
    LookupCombined8Data* lcd = new LookupCombined8Data;
    if(lcd == NULL) {
        XIL_ERROR(dst->getSystemState(), XIL_ERROR_RESOURCE, "di-1", TRUE);
        return XIL_FAILURE;
    }
    
    XilLookupCombined* lookup;
    op->getParam(1, (XilObject**) &lookup);

    lcd->num_bands = lookup->getInputNBands();
    lcd->allocated = new Xil_boolean[lcd->num_bands];
    if(lcd->allocated == NULL) {
        XIL_ERROR(dst->getSystemState(), XIL_ERROR_RESOURCE, "di-1", TRUE);
        return XIL_FAILURE;
    }
    lcd->values = new Xil_unsigned8 * [lcd->num_bands];
    if(lcd->values == NULL) {
        XIL_ERROR(dst->getSystemState(), XIL_ERROR_RESOURCE, "di-1", TRUE);
        return XIL_FAILURE;
    }

    //
    // If the table does not cover the dynamic space (256 entries)
    // fill in the empty spots.
    //
    const unsigned int* entriesList = lookup->getEntriesList();
    const int*          offsetsList = lookup->getOffsetsList();
    Xil_unsigned8**     dataList    = (Xil_unsigned8**)lookup->getDataList();
    for (int j = 0; j < lcd->num_bands; j++) {
        if (entriesList[j] == 256) {
            //
            // The core does not allow negative offsets nor does it allow
            // the offset + num_entries to exceed the maximum possible.
            // Therefore, if entries is 256, then everything is set to go.
            lcd->allocated[j] = FALSE;
            lcd->values[j]   = dataList[j];
        } else {
#define XIL_LOOKUP_COMBINED_8_8_COPY_THRESHOLD 256
            if (lcd->num_bands > XIL_LOOKUP_COMBINED_8_8_COPY_THRESHOLD) {
                lcd->allocated[j] = FALSE;
                lcd->values[j] = 0;
            } else {
                
                Xil_unsigned8* values = new Xil_unsigned8[256];
                if(values == NULL) {
                XIL_ERROR(dst->getSystemState(),
                          XIL_ERROR_RESOURCE, "di-1", TRUE);
                return XIL_FAILURE;
                }
                
                int i;
                for (i = 0; i < offsetsList[j]; i++)
                    values[i] = dataList[j][0];
                
                xili_memcpy(values+offsetsList[j],
                            dataList[j],
                            entriesList[j]);
                
                for (i = entriesList[j] + offsetsList[j]; i < 256; i++)
                    values[i] = dataList[j][entriesList[j] - 1];
                
                lcd->values[j] = values;
                lcd->allocated[j] = TRUE;
            }
        }
    }
    *compute_data = lcd;
    
    return XIL_SUCCESS;
}

XilStatus
XilDeviceManagerComputeBYTE::LookupCombined8Postprocess(XilOp*    ,
                                                        void*     compute_data)
{
    LookupCombined8Data* lcd = (LookupCombined8Data*)compute_data;

    for (int i = 0; i < lcd->num_bands; i++)
        if (lcd->allocated[i] == TRUE)
            delete lcd->values[i];
    
    delete lcd;
    
    return XIL_SUCCESS;
}

XilStatus
XilDeviceManagerComputeBYTE::LookupCombined16Preprocess(XilOp*        op,
                                                        unsigned      ,
                                                        XilRoi*       ,
                                                        void**        compute_data,
                                                        unsigned int* )
{
    XilImage* dst = op->getDstImage(1);

    //
    // Create a lookup structure no matter what
    //
    LookupCombined16Data* lcd = new LookupCombined16Data;
    if(lcd == NULL) {
        XIL_ERROR(dst->getSystemState(), XIL_ERROR_RESOURCE, "di-1", TRUE);
        return XIL_FAILURE;
    }
    
    XilLookupCombined* lookup;
    op->getParam(1, (XilObject**) &lookup);

    lcd->num_bands = lookup->getInputNBands();
    lcd->allocated = new Xil_boolean[lcd->num_bands];
    if (lcd->allocated == NULL) {
        XIL_ERROR(dst->getSystemState(), XIL_ERROR_RESOURCE, "di-1", TRUE);
        return XIL_FAILURE;
    }
    lcd->values = new Xil_signed16 * [lcd->num_bands];
    if (lcd->values == NULL) {
        XIL_ERROR(dst->getSystemState(), XIL_ERROR_RESOURCE, "di-1", TRUE);
        return XIL_FAILURE;
    }

    //
    // If the table does not cover the dynamic space (256 entries)
    // fill in the empty spots.
    //
    const unsigned int* entriesList = lookup->getEntriesList();
    const int*          offsetsList = lookup->getOffsetsList();
    Xil_signed16**      dataList    = (Xil_signed16**)lookup->getDataList();
    for (int j = 0; j < lcd->num_bands; j++) {
        if (entriesList[j] == 256) {
            lcd->allocated[j] = FALSE;
            lcd->values[j] = dataList[j];
        } else {
#define XIL_LOOKUP_COMBINED_8_16_COPY_THRESHOLD 128
            if (lcd->num_bands > XIL_LOOKUP_COMBINED_8_16_COPY_THRESHOLD) {
                lcd->allocated[j] = FALSE;
                lcd->values[j] = 0;
            } else {
                Xil_signed16* values = new Xil_signed16[256];
                if(values == NULL) {
                    XIL_ERROR(dst->getSystemState(),
                              XIL_ERROR_RESOURCE, "di-1", TRUE);
                    return XIL_FAILURE;
                }
                
                int i;
                for (i = 0; i < offsetsList[j]; i++)
                    values[i] = dataList[j][0];
                
                xili_memcpy(values+offsetsList[j],
                            dataList[j],
                            2*entriesList[j]);
                
                for (i = entriesList[j] + offsetsList[j]; i < 256; i++)
                    values[i] = dataList[j][entriesList[j] - 1];
                
                lcd->values[j] = values;
                lcd->allocated[j] = TRUE;
            }
        }
    }
    *compute_data = lcd;
    
    return XIL_SUCCESS;
}

XilStatus
XilDeviceManagerComputeBYTE::LookupCombined16Postprocess(XilOp*   ,
                                                         void*     compute_data)
{
    LookupCombined16Data* lcd = (LookupCombined16Data*)compute_data;

    for (int i = 0; i < lcd->num_bands; i++)
        if (lcd->allocated[i] == TRUE)
            delete lcd->values[i];
    
    delete lcd;
    
    return XIL_SUCCESS;
}

XilStatus
XilDeviceManagerComputeBYTE::LookupCombined1(XilOp*       op,
                                             unsigned     op_count,
                                             XilRoi*      roi,
                                             XilBoxList*  bl)
{
    ComputeInfoGENERAL  ci(op, op_count, roi, bl);
    
    if(ci.isOK() == FALSE) {
        return ci.returnValue;
    }
    
    XilLookupCombined* lookup;
    op->getParam(1, (XilObject**) &lookup);

    const int*          offsetsList = lookup->getOffsetsList();
    const unsigned int* entriesList = lookup->getEntriesList();
    Xil_unsigned8**     dataList    = (Xil_unsigned8**)lookup->getDataList();
                    
    //
    // Assuming a combined lookup table
    //
    while(ci.hasMoreInfo()) {
        //
        // Destination is bit which is bit sequential only
        // Source could be pixel or band sequential.
        for (unsigned int band=0; band < ci.destNumBands; band++) {
            Xil_unsigned8* src1_scanline=(Xil_unsigned8*)ci.getSrc1Data(band);
            Xil_unsigned8* dest_scanline=(Xil_unsigned8*)ci.getDestData(band);
            unsigned int dest_offset = ci.getDestOffset(band);
            int top = entriesList[band] + offsetsList[band];
            int top_val = *(dataList[band] + (entriesList[band] - 1));
            int bottom = offsetsList[band];
            int bottom_val = *(dataList[band]);
            for(int y=ci.ysize; y>0; y--) {
                Xil_unsigned8* src1 = src1_scanline;
                for(unsigned int x=0; x<ci.xsize; x++) {
                    Xil_unsigned8* dest = dest_scanline;
                    Xil_unsigned8* lut_band = dataList[band] + *src1 - bottom;
                    if (*src1 > top){
                        if (top_val & 0x1)
                            XIL_BMAP_SET(dest, dest_offset + x);
                        else
                            XIL_BMAP_CLR(dest, dest_offset + x);
                    } else if (*src1 < bottom) {
                        if (bottom_val & 0x1) 
                            XIL_BMAP_SET(dest, dest_offset + x);
                        else
                            XIL_BMAP_CLR(dest, dest_offset + x);
                    } else {
                        if (*lut_band & 0x1)
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

    return XIL_SUCCESS;
}


XilStatus
XilDeviceManagerComputeBYTE::LookupCombined8(XilOp*       op,
                                             unsigned     op_count,
                                             XilRoi*      roi,
                                             XilBoxList*  bl)
{
    ComputeInfoBYTE  ci(op, op_count, roi, bl);

    if(ci.isOK() == FALSE) {
        return ci.returnValue;
    }
    
    XilLookupCombined* lookup;
    op->getParam(1, (XilObject**) &lookup);
    const int*          offsetsList = lookup->getOffsetsList();
    const unsigned int* entriesList = lookup->getEntriesList();
    Xil_unsigned8**     dataList    = (Xil_unsigned8**)lookup->getDataList();

    LookupCombined8Data*  lcd =
        (LookupCombined8Data*)op->getPreprocessData(this);
    
    //
    // Assuming a combined lookup table
    //
    while(ci.hasMoreInfo()) {
        if (ci.src1Storage.isType(XIL_PIXEL_SEQUENTIAL) &&
            ci.destStorage.isType(XIL_PIXEL_SEQUENTIAL)) {
            Xil_unsigned8* src1_scanline = ci.src1Data;
            Xil_unsigned8* dest_scanline = ci.destData;
            if ((ci.src1NumBands == 3) &&
                (ci.destNumBands == 3) &&
                (ci.src1PixelStride == 3) &&
                (ci.destPixelStride == 3)) {
                Xil_unsigned8* band0 = lcd->values[0];
                Xil_unsigned8* band1 = lcd->values[1];
                Xil_unsigned8* band2 = lcd->values[2];

                unsigned int xcount = ci.xsize >> 2;
                unsigned int edge   = ci.xsize & 0x3;

                unsigned int  src1_incr_scanline =
                    ci.src1Storage.getScanlineStride() -
                    ((ci.xsize << 1) + ci.xsize);
                unsigned int dest_incr_scanline =
                    ci.destStorage.getScanlineStride() -
                    ((ci.xsize << 1) + ci.xsize);

#define LOOKUP3(n) dest_scanline[n]   = band0[src1_scanline[n]];   \
                   dest_scanline[n+1] = band1[src1_scanline[n+1]]; \
                   dest_scanline[n+2] = band2[src1_scanline[n+2]]

                for(int y=ci.ysize; y>0; y--) {
                    for(int x=xcount; x>0; x--) {
                        LOOKUP3(0);
                        LOOKUP3(3);
                        LOOKUP3(6);
                        LOOKUP3(9);

                        dest_scanline += 12;
                        src1_scanline += 12;
                    }
                    switch (edge) {
                      case 3:
                        LOOKUP3(0);
                        LOOKUP3(3);
                        LOOKUP3(6);
                        dest_scanline += 9;
                        src1_scanline += 9;
                        break;
                      case 2:
                        LOOKUP3(0);
                        LOOKUP3(3);
                        dest_scanline += 6;
                        src1_scanline += 6;
                        break;
                      case 1:
                        LOOKUP3(0);
                        dest_scanline += 3;
                        src1_scanline += 3;
                        break;
                    }
                    src1_scanline += src1_incr_scanline;
                    dest_scanline += dest_incr_scanline;
                }
            } else {
                /*
                ** Pixel sequential but not accelerateable
                */
                if (lcd->values[0] == NULL) {
                    //
                    // and couldn't copy the data tables
                    //
                    for(int y=ci.ysize; y>0; y--) {
                        Xil_unsigned8* src1_pixel = src1_scanline;
                        Xil_unsigned8* dest_pixel = dest_scanline;
                        for(int x=ci.xsize; x>0; x--) {
                            Xil_unsigned8* src1 = src1_pixel;
                            Xil_unsigned8* dest = dest_pixel;
                            for (unsigned int band = 0; band < ci.src1NumBands; band++){
                                if (*src1 < offsetsList[band]) {
                                    *dest = *(dataList[band]);
                                } else if (*src1 >
                                   (offsetsList[band] + entriesList[band])){
                                    *dest = *(dataList[band] +
                                              (entriesList[band] - 1));
                                } else {
                                    *dest = *(dataList[band] + *src1 -
                                              offsetsList[band]);
                                }
                                src1++;
                                dest++;
                            }
                            src1_pixel += ci.src1PixelStride;
                            dest_pixel += ci.destPixelStride;
                        }
                        src1_scanline += ci.src1ScanlineStride;
                        dest_scanline += ci.destScanlineStride;
                    }
                } else {
                    //
                    // at least I was able to copy the lookup table
                    // 
                    for(int y=ci.ysize; y>0; y--) {
                        Xil_unsigned8* src1_pixel = src1_scanline;
                        Xil_unsigned8* dest_pixel = dest_scanline;
                        for(int x=ci.xsize; x>0; x--) {
                            Xil_unsigned8* src1 = src1_pixel;
                            Xil_unsigned8* dest = dest_pixel;
                            for (unsigned int band=0; band < ci.src1NumBands; band++)
                                *dest++ = *(lcd->values[band] + *src1++);
                            
                            src1_pixel += ci.src1PixelStride;
                            dest_pixel += ci.destPixelStride;
                        }
                        src1_scanline += ci.src1ScanlineStride;
                        dest_scanline += ci.destScanlineStride;
                    }
                }
            }
        } else {
            //
            // Band sequential case
            // check if we copied the lookup table
            //
            if (lcd->values[0] == NULL) {
                //
                // couldn't copy data
                // number of bands must have exceeded copy threshold
                //
                for(unsigned int band=0; band<ci.destNumBands; band++) { 
                    int src1PixelStride = ci.getSrc1PixelStride(band);
                    int destPixelStride = ci.getDestPixelStride(band);
                    int src1ScanlineStride = ci.getSrc1ScanlineStride(band);
                    int destScanlineStride = ci.getDestScanlineStride(band);
                    Xil_unsigned8* src1_scanline = ci.getSrc1Data(band);
                    Xil_unsigned8* dest_scanline = ci.getDestData(band);
                    for(int y=ci.ysize; y>0; y--) { 
                        Xil_unsigned8* src1 = src1_scanline;
                        Xil_unsigned8* dest = dest_scanline;
                        for(int x=ci.xsize; x>0; x--) { 
                            if (*src1 < offsetsList[band]) {
                                *dest = *(dataList[band]);
                            } else if (*src1 >
                                      (offsetsList[band] + entriesList[band])){
                                *dest = *(dataList[band] +
                                          (entriesList[band] - 1));
                            } else {
                                *dest = *(dataList[band] + *src1 -
                                          offsetsList[band]);
                            }
                            src1 += src1PixelStride;
                            dest += destPixelStride;
                        }
                        src1_scanline += src1ScanlineStride;
                        dest_scanline += destScanlineStride;
                    }
                }
            } else {
                //
                // at least the Lookup table data was copied
                //
                for(unsigned int band=0; band<ci.destNumBands; band++) { 
                    int src1PixelStride = ci.getSrc1PixelStride(band);
                    int destPixelStride = ci.getDestPixelStride(band);
                    int src1ScanlineStride = ci.getSrc1ScanlineStride(band);
                    int destScanlineStride = ci.getDestScanlineStride(band);
                    Xil_unsigned8* src1_scanline = ci.getSrc1Data(band);
                    Xil_unsigned8* dest_scanline = ci.getDestData(band);
                    for(int y=ci.ysize; y>0; y--) { 
                        Xil_unsigned8* src1 = src1_scanline;
                        Xil_unsigned8* dest = dest_scanline;
                        for(int x=ci.xsize; x>0; x--) { 
                            *dest = *(lcd->values[band]+ *src1);
                            
                            src1 += src1PixelStride;
                            dest += destPixelStride;
                        }
                        src1_scanline += src1ScanlineStride;
                        dest_scanline += destScanlineStride;
                    }
                }
            }
        }
    }

    return ci.returnValue;
}

XilStatus
XilDeviceManagerComputeBYTE::LookupCombined16(XilOp*       op,
                                              unsigned     op_count,
                                              XilRoi*      roi,
                                              XilBoxList*  bl)
{
    ComputeInfoGENERAL  ci(op, op_count, roi, bl);
    
    if(ci.isOK() == FALSE) {
        return ci.returnValue;
    }
    
    XilLookupCombined* lookup;
    op->getParam(1, (XilObject**) &lookup);
    const int*          offsetsList = lookup->getOffsetsList();
    const unsigned int* entriesList = lookup->getEntriesList();
    Xil_unsigned8**     dataList    = (Xil_unsigned8**)lookup->getDataList();

    LookupCombined16Data*  lcd =
        (LookupCombined16Data*)op->getPreprocessData(this);

    //
    // Assuming a combined lookup table
    //
    while(ci.hasMoreInfo()) {
        if (ci.src1Storage.isType(XIL_PIXEL_SEQUENTIAL) &&
            ci.destStorage.isType(XIL_PIXEL_SEQUENTIAL)) {
            Xil_unsigned8* src1_scanline = (Xil_unsigned8*) ci.src1Data;
            Xil_signed16*  dest_scanline = (Xil_signed16*)  ci.destData;
            if ((ci.src1NumBands == 3) &&
                (ci.destNumBands == 3) &&
                (ci.src1PixelStride == 3) &&
                (ci.destPixelStride == 3)) {
                Xil_signed16* band0 = lcd->values[0];
                Xil_signed16* band1 = lcd->values[1];
                Xil_signed16* band2 = lcd->values[2];

                unsigned int xcount = ci.xsize >> 2;
                unsigned int edge   = ci.xsize & 0x3;

                unsigned int src1_incr_scanline =
                    ci.src1Storage.getScanlineStride() -
                    ((ci.xsize << 1) + ci.xsize);
                unsigned int dest_incr_scanline =
                    ci.destStorage.getScanlineStride() -
                    ((ci.xsize << 1) + ci.xsize);

#define LOOKUP3(n) dest_scanline[n]   = band0[src1_scanline[n]];   \
                   dest_scanline[n+1] = band1[src1_scanline[n+1]]; \
                   dest_scanline[n+2] = band2[src1_scanline[n+2]]

                for(int y=ci.ysize; y>0; y--) {
                    for(int x=xcount; x>0; x--) {
                        LOOKUP3(0);
                        LOOKUP3(3);
                        LOOKUP3(6);
                        LOOKUP3(9);

                        dest_scanline += 12;
                        src1_scanline += 12;
                    }
                    switch (edge) {
                      case 3:
                        LOOKUP3(0);
                        LOOKUP3(3);
                        LOOKUP3(6);
                        dest_scanline += 9;
                        src1_scanline += 9;
                        break;
                      case 2:
                        LOOKUP3(0);
                        LOOKUP3(3);
                        dest_scanline += 6;
                        src1_scanline += 6;
                        break;
                      case 1:
                        LOOKUP3(0);
                        dest_scanline += 3;
                        src1_scanline += 3;
                        break;
                    }
                    src1_scanline += src1_incr_scanline;
                    dest_scanline += dest_incr_scanline;
                }
            } else {
                /*
                ** Pixel sequential but not accelerateable
                */
                if (lcd->values[0] == NULL) {
                    //
                    // couldn't copy lookup data
                    // number of bands must have exceeded copy threshold
                    //
                    for(int y=ci.ysize; y>0; y--) {
                        Xil_unsigned8* src1_pixel = src1_scanline;
                        Xil_signed16*  dest_pixel = dest_scanline;
                        for(int x=ci.xsize; x>0; x--) {
                            Xil_unsigned8* src1 = src1_pixel;
                            Xil_signed16*  dest = dest_pixel;
                            for (unsigned int band = 0; band < ci.src1NumBands; band++){
                                if (*src1 < offsetsList[band]) {
                                    *dest = *(dataList[band]);
                                } else if (*src1 >
                                   (offsetsList[band] + entriesList[band])){
                                    *dest = *(dataList[band] +
                                              (entriesList[band] - 1));
                                } else {
                                    *dest = *(dataList[band] + *src1 -
                                              offsetsList[band]);
                                }
                                src1++;
                                dest++;
                            }
                            src1_pixel += ci.src1PixelStride;
                            dest_pixel += ci.destPixelStride;
                        }
                        src1_scanline += ci.src1ScanlineStride;
                        dest_scanline += ci.destScanlineStride;
                    }
                } else {
                    //
                    // at least we were able to copy the data
                    //
                    for(int y=ci.ysize; y>0; y--) {
                        Xil_unsigned8* src1_pixel = src1_scanline;
                        Xil_signed16*  dest_pixel = dest_scanline;
                        for(int x=ci.xsize; x>0; x--) {
                            Xil_unsigned8* src1 = src1_pixel;
                            Xil_signed16*  dest = dest_pixel;
                            for(unsigned int band=0; band < ci.src1NumBands; band++) {
                                *dest++ = *(lcd->values[band]+ *src1++);
                            }
                            src1_pixel += ci.src1PixelStride;
                            dest_pixel += ci.destPixelStride;
                        }
                        src1_scanline += ci.src1ScanlineStride;
                        dest_scanline += ci.destScanlineStride;
                    }
                }
            }
        } else {
            //
            // Band sequential
            //
            if (lcd->values == NULL) {
                //
                // couldn't copy lookup table data
                // number of bands must have exceeded copy threshold
                //
                for(unsigned int band=0; band<ci.destNumBands; band++) { 
                    int src1PixelStride = ci.getSrc1PixelStride(band);
                    int destPixelStride = ci.getDestPixelStride(band);
                    int src1ScanlineStride = ci.getSrc1ScanlineStride(band);
                    int destScanlineStride = ci.getDestScanlineStride(band);
                    Xil_unsigned8* src1_scanline =
                        (Xil_unsigned8*) ci.getSrc1Data(band);
                    Xil_signed16*  dest_scanline =
                        (Xil_signed16*)ci.getDestData(band);
                    for(int y=ci.ysize; y>0; y--) { 
                        Xil_unsigned8* src1 = src1_scanline;
                        Xil_signed16*  dest = dest_scanline;
                        for(int x=ci.xsize; x>0; x--) { 
                            if (*src1 < offsetsList[band]) {
                                *dest = *(dataList[band]);
                            } else if (*src1 >
                                       (offsetsList[band]+entriesList[band])){
                                *dest = *(dataList[band] +
                                          (entriesList[band] - 1));
                            } else {
                                *dest = *(dataList[band] + *src1 -
                                          offsetsList[band]);
                            }
                            src1 += src1PixelStride;
                            dest += destPixelStride;
                        }
                        src1_scanline += src1ScanlineStride;
                        dest_scanline += destScanlineStride;
                    }
                }
            } else {
                for(unsigned int band=0; band<ci.destNumBands; band++) { 
                    int src1PixelStride = ci.getSrc1PixelStride(band);
                    int destPixelStride = ci.getDestPixelStride(band);
                    int src1ScanlineStride = ci.getSrc1ScanlineStride(band);
                    int destScanlineStride = ci.getDestScanlineStride(band);
                    Xil_unsigned8* src1_scanline =
                        (Xil_unsigned8*) ci.getSrc1Data(band);
                    Xil_signed16*  dest_scanline =
                        (Xil_signed16*)ci.getDestData(band);
                    for(int y=ci.ysize; y>0; y--) { 
                        Xil_unsigned8* src1 = src1_scanline;
                        Xil_signed16*  dest = dest_scanline;
                        for(int x=ci.xsize; x>0; x--) { 
                            *dest = *(lcd->values[band]+ *src1);
                            
                            src1 += src1PixelStride;
                            dest += destPixelStride;
                        }
                        src1_scanline += src1ScanlineStride;
                        dest_scanline += destScanlineStride;
                    }
                }
            }
        }
    }

    return XIL_SUCCESS;
}

XilStatus
XilDeviceManagerComputeBYTE::LookupCombinedf32(XilOp*       op,
                                               unsigned     op_count,
                                               XilRoi*      roi,
                                               XilBoxList*  bl)
{
    ComputeInfoGENERAL  ci(op, op_count, roi, bl);
    
    if(ci.isOK() == FALSE) {
        return ci.returnValue;
    }
    
    XilLookupCombined* lookup;
    op->getParam(1, (XilObject**) &lookup);

    const int*          offsetsList = lookup->getOffsetsList();
    const unsigned int* entriesList = lookup->getEntriesList();
    Xil_float32**       dataList    = (Xil_float32**)lookup->getDataList();
                    
    //
    // Assuming a combined lookup table
    //
    while(ci.hasMoreInfo()) {
        if (ci.src1Storage.isType(XIL_PIXEL_SEQUENTIAL) &&
            ci.destStorage.isType(XIL_PIXEL_SEQUENTIAL)) {
            Xil_unsigned8* src1_scanline = (Xil_unsigned8*) ci.src1Data;
            Xil_float32*   dest_scanline = (Xil_float32*) ci.destData;
            for(int y=ci.ysize; y>0; y--) {
                Xil_unsigned8* src1_pixel = src1_scanline;
                Xil_float32*   dest_pixel = dest_scanline;
                for(int x=ci.xsize; x>0; x--) {
                    Xil_unsigned8* src1 = src1_pixel;
                    Xil_float32*   dest = dest_pixel;
                    for (unsigned int band=0; band < ci.src1NumBands; band++) {
                        if (*src1 < offsetsList[band]) {
                            *dest = *(dataList[band]);
                        } else if (*src1 >
                                   (offsetsList[band] + entriesList[band])){
                            *dest = *(dataList[band] + (entriesList[band] -1));
                        } else {
                            *dest = *(dataList[band] + *src1 -
                                      offsetsList[band]);
                        }
                        src1++;
                        dest++;
                    }
                    src1_pixel += ci.src1PixelStride;
                    dest_pixel += ci.destPixelStride;
                }
                src1_scanline += ci.src1ScanlineStride;
                dest_scanline += ci.destScanlineStride;
            }
        } else {
            for(unsigned int band=0; band<ci.destNumBands; band++) { 
              int src1PixelStride = ci.getSrc1PixelStride(band);
              int destPixelStride = ci.getDestPixelStride(band);
              int src1ScanlineStride = ci.getSrc1ScanlineStride(band);
              int destScanlineStride = ci.getDestScanlineStride(band);
              Xil_unsigned8* src1_scanline = (Xil_unsigned8*) ci.getSrc1Data(band);
              Xil_float32*   dest_scanline =(Xil_float32*)ci.getDestData(band);
              
              int bottom = offsetsList[band];
              int top = entriesList[band] + bottom;
              for(int y=ci.ysize; y>0; y--) {
                  Xil_unsigned8* src1 = src1_scanline;
                  Xil_float32*   dest = dest_scanline;
                  for(int x=ci.xsize; x>0; x--) {
                      if (*src1 > top){
                          *dest = *(dataList[band] + (entriesList[band] -1));
                      } else if (*src1 < bottom) {
                          *dest = *(dataList[band]);
                      } else {
                          *dest = *(dataList[band] + *src1 - bottom);
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

    return XIL_SUCCESS;
}
