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
//  Revision:	1.5
//  Last Mod:	10:09:54, 03/10/00
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
#pragma ident	"@(#)LookupCombined.cc	1.5\t00/03/10  "

#include "XilDeviceManagerComputeBIT.hh"
#include "ComputeInfo.hh"
#include "XiliUtils.hh"

XilStatus
XilDeviceManagerComputeBIT::LookupCombined1(XilOp*       op,
                                              unsigned     op_count,
                                              XilRoi*      roi,
                                              XilBoxList*  bl)
{
    ComputeInfoBIT  ci(op, op_count, roi, bl);

    if(ci.isOK() == FALSE) {
        return ci.returnValue;
    }

    XilLookupCombined* lookup;
    op->getParam(1, (XilObject**) &lookup);
    
    const unsigned int* entriesList = lookup->getEntriesList();
    const int*          offsetsList = lookup->getOffsetsList();
    Xil_unsigned8**     dataList    = (Xil_unsigned8**) lookup->getDataList();

    int nbands = lookup->getInputNBands();

    while(ci.hasMoreInfo()) {
        for(int b=0; b<ci.destNumBands; b++) { // each band

            Xil_unsigned8 zero_val = dataList[b][0];
            if (zero_val > 1) zero_val = 1;
            
            Xil_unsigned8 one_val = dataList[b][1];
            if (one_val > 1) one_val = 1;
    
            Xil_unsigned8* src1_scanline = ci.getSrc1Data(b);
            Xil_unsigned8* dest_scanline = ci.getDestData(b);

            unsigned int src1_offset = ci.getSrc1Offset(b);
            unsigned int dest_offset = ci.getDestOffset(b);

            int src1_scanline_stride = ci.getSrc1ScanlineStride(b);
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
XilDeviceManagerComputeBIT::LookupCombined8(XilOp*       op,
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

    const unsigned int* entriesList = lookup->getEntriesList();
    const int*          offsetsList = lookup->getOffsetsList();
    Xil_unsigned8**     dataList    = (Xil_unsigned8**) lookup->getDataList();

    int nbands = lookup->getInputNBands();

    while (ci.hasMoreInfo()) {
        if (ci.src1Storage.isType(XIL_BAND_SEQUENTIAL) &&
            ci.destStorage.isType(XIL_PIXEL_SEQUENTIAL)) {
            Xil_unsigned8** src_ptr_table = new Xil_unsigned8*[nbands];
            for (int i = 0; i < nbands; i++) {
                src_ptr_table[i] = (Xil_unsigned8*) ci.getSrc1Data(i);
            }
                
            Xil_unsigned8* dest_scanline = (Xil_unsigned8*) ci.destData;

            unsigned int src1_offset = ci.src1Offset;
            unsigned int src1_scanline_stride = ci.src1ScanlineStride;
            unsigned int dest_pixel_stride = ci.destPixelStride;
            unsigned int dest_scanline_stride = ci.destScanlineStride;
            
            for (int y=0; y<ci.ysize; y++) {
                Xil_unsigned8* dest_pixel = dest_scanline;
                
                for (int x=0; x<ci.xsize; x++) {
                    Xil_unsigned8* dest = dest_pixel;
                    for (int band = 0; band < nbands; band++) {
                        if (XIL_BMAP_TST(src_ptr_table[band], src1_offset + x))
                            *dest++ = dataList[band][1];
                        else
                            *dest++ = dataList[band][0];
                    }
                    dest_pixel += dest_pixel_stride;
                }
                dest_scanline += dest_scanline_stride;
                for (i = 0; i < nbands; i++) {
                    src_ptr_table[i] += src1_scanline_stride;
                }
            }
            delete [] src_ptr_table;
        } else {
            for (int band = 0; band < nbands; band++) {
                Xil_unsigned8* src1_scanline = (Xil_unsigned8*) ci.getSrc1Data(band);
                Xil_unsigned8* dest_scanline = (Xil_unsigned8*) ci.getDestData(band);
                
                unsigned int src1_offset = ci.getSrc1Offset(band);
                unsigned int src1_scanline_stride = ci.getSrc1ScanlineStride(band);
                
                unsigned int dest_pixel_stride = ci.getDestPixelStride(band);
                unsigned int dest_scanline_stride = ci.getDestScanlineStride(band);
                
                for (int y=0; y<ci.ysize; y++) {
                    Xil_unsigned8* dest = dest_scanline;
                    
                    for (int x=0; x<ci.xsize; x++) {
                        if (XIL_BMAP_TST(src1_scanline, src1_offset + x))
                            *dest = dataList[band][1];
                        else
                            *dest = dataList[band][0];
                        
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
XilDeviceManagerComputeBIT::LookupCombined16(XilOp*       op,
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

    const unsigned int* entriesList = lookup->getEntriesList();
    const int*          offsetsList = lookup->getOffsetsList();
    Xil_signed16**      dataList    = (Xil_signed16**) lookup->getDataList();

    int nbands = lookup->getInputNBands();

    while (ci.hasMoreInfo()) {
        if (ci.src1Storage.isType(XIL_BAND_SEQUENTIAL) &&
            ci.destStorage.isType(XIL_PIXEL_SEQUENTIAL)) {
            Xil_unsigned8** src_ptr_table = new Xil_unsigned8*[nbands];
            for (int i = 0; i < nbands; i++) {
                src_ptr_table[i] = (Xil_unsigned8*) ci.getSrc1Data(i);
            }
                
            Xil_signed16* dest_scanline = (Xil_signed16*) ci.destData;

            unsigned int src1_offset = ci.src1Offset;
            unsigned int src1_scanline_stride = ci.src1ScanlineStride;
            unsigned int dest_pixel_stride = ci.destPixelStride;
            unsigned int dest_scanline_stride = ci.destScanlineStride;
            
            for (int y=0; y<ci.ysize; y++) {
                Xil_signed16* dest_pixel = dest_scanline;
                
                for (int x=0; x<ci.xsize; x++) {
                    Xil_signed16* dest = dest_pixel;
                    for (int band = 0; band < nbands; band++) {
                        if (XIL_BMAP_TST(src_ptr_table[band], src1_offset + x))
                            *dest++ = dataList[band][1];
                        else
                            *dest++ = dataList[band][0];
                    }
                    dest_pixel += dest_pixel_stride;
                }
                dest_scanline += dest_scanline_stride;
                for (i = 0; i < nbands; i++) {
                    src_ptr_table[i] += src1_scanline_stride;
                }
            }
            delete [] src_ptr_table;
        } else {
            for (int band = 0; band < nbands; band++) {
                Xil_unsigned8* src1_scanline = (Xil_unsigned8*) ci.getSrc1Data(band);
                Xil_signed16* dest_scanline = (Xil_signed16*) ci.getDestData(band);
                
                unsigned int src1_offset = ci.getSrc1Offset(band);
                unsigned int src1_scanline_stride = ci.getSrc1ScanlineStride(band);
                
                unsigned int dest_pixel_stride = ci.getDestPixelStride(band);
                unsigned int dest_scanline_stride = ci.getDestScanlineStride(band);
                
                for (int y=0; y<ci.ysize; y++) {
                    Xil_signed16* dest = dest_scanline;
                    
                    for (int x=0; x<ci.xsize; x++) {
                        if (XIL_BMAP_TST(src1_scanline, src1_offset + x))
                            *dest = dataList[band][1];
                        else
                            *dest = dataList[band][0];
                        
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
XilDeviceManagerComputeBIT::LookupCombinedf32(XilOp*       op,
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

    const unsigned int* entriesList = lookup->getEntriesList();
    const int*          offsetsList = lookup->getOffsetsList();
    Xil_float32**       dataList    = (Xil_float32**) lookup->getDataList();

    int nbands = lookup->getInputNBands();

    while (ci.hasMoreInfo()) {
        if (ci.src1Storage.isType(XIL_BAND_SEQUENTIAL) &&
            ci.destStorage.isType(XIL_PIXEL_SEQUENTIAL)) {
            Xil_unsigned8** src_ptr_table = new Xil_unsigned8*[nbands];
            for (int i = 0; i < nbands; i++) {
                src_ptr_table[i] = (Xil_unsigned8*) ci.getSrc1Data(i);
            }
                
            Xil_float32* dest_scanline = (Xil_float32*) ci.destData;

            unsigned int src1_offset = ci.src1Offset;
            unsigned int src1_scanline_stride = ci.src1ScanlineStride;
            unsigned int dest_pixel_stride = ci.destPixelStride;
            unsigned int dest_scanline_stride = ci.destScanlineStride;
            
            for (int y=0; y<ci.ysize; y++) {
                Xil_float32* dest_pixel = dest_scanline;
                
                for (int x=0; x<ci.xsize; x++) {
                    Xil_float32* dest = dest_pixel;
                    for (int band = 0; band < nbands; band++) {
                        if (XIL_BMAP_TST(src_ptr_table[band], src1_offset + x))
                            *dest++ = dataList[band][1];
                        else
                            *dest++ = dataList[band][0];
                    }
                    dest_pixel += dest_pixel_stride;
                }
                dest_scanline += dest_scanline_stride;
                for (i = 0; i < nbands; i++) {
                    src_ptr_table[i] += src1_scanline_stride;
                }
            }
            delete [] src_ptr_table;
        } else {
            for (int band = 0; band < nbands; band++) {
                Xil_unsigned8* src1_scanline = (Xil_unsigned8*) ci.getSrc1Data(band);
                Xil_float32* dest_scanline = (Xil_float32*) ci.getDestData(band);
                
                unsigned int src1_offset = ci.getSrc1Offset(band);
                unsigned int src1_scanline_stride = ci.getSrc1ScanlineStride(band);
                
                unsigned int dest_pixel_stride = ci.getDestPixelStride(band);
                unsigned int dest_scanline_stride = ci.getDestScanlineStride(band);
                
                for (int y=0; y<ci.ysize; y++) {
                    Xil_float32* dest = dest_scanline;
                    
                    for (int x=0; x<ci.xsize; x++) {
                        if (XIL_BMAP_TST(src1_scanline, src1_offset + x))
                            *dest = dataList[band][1];
                        else
                            *dest = dataList[band][0];
                        
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
