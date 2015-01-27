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
//  Revision:	1.11
//  Last Mod:	13:47:35, 01/29/96
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
#pragma ident	"@(#)NearestColor.cc	1.11\t96/01/29  "

#ifndef _WINDOWS
#include <values.h>
#endif
#include "XilDeviceManagerComputeSHORT.hh"
#include "ComputeInfo.hh"
    
XilStatus
XilDeviceManagerComputeSHORT::NearestColor1(XilOp*       op,
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

    unsigned int   lut_size = lookup->getNumEntries();
    Xil_signed16*  lut=(Xil_signed16*)lookup->getData();
    unsigned short lut_nbands = lookup->getOutputNBands();
    short          lut_offset = lookup->getOffset();
    int            max_index  = lut_size * lut_nbands;

    while(ci.hasMoreInfo()) {
        int dR_offset = ci.destOffset;
        //
        // destination is always single banded so don't check it's storage
        //
        double closest_dist, distance, diff;
        if ((ci.src1Storage.isType(XIL_PIXEL_SEQUENTIAL))) {
            Xil_signed16*  src1_scanline = (Xil_signed16*) ci.src1Data;
            Xil_unsigned8* dest_scanline = (Xil_unsigned8*) ci.destData;
            for(int y=ci.ysize; y>0; y--) {
                Xil_signed16* src1 = src1_scanline;
                Xil_unsigned8* dest = dest_scanline;
                int closest_index;
                for(int x = 0; x < ci.xsize; x++) {
                    closest_dist = XIL_MAXDOUBLE;
                    for(int b=0; b < max_index; b += lut_nbands) {
                        // go throught the whole LUT
                        distance = 0;
                        for (int i = lut_nbands - 1; i >= 0; i--) {
                            diff = (*(src1 + i) - lut[b + i]);
                            distance += diff * diff;
                        }
                        // compare if distance is shorter than previous
                        if (distance < closest_dist) {
                            closest_dist = distance;
                            closest_index = b;
                        }
                    } // for colormap size

                    if ((lut_offset + closest_index / lut_nbands))
                        XIL_BMAP_SET(dest, dR_offset + x);
                    else
                        XIL_BMAP_CLR(dest, dR_offset + x);

                    src1 += ci.src1PixelStride;
                }
                src1_scanline += ci.src1ScanlineStride;
                dest_scanline += ci.destScanlineStride;
            }
        } else {
            int band;
            // Create source data pointer tables
            Xil_signed16** srcPtrTable =
                new Xil_signed16*[ci.src1NumBands];
            Xil_signed16** srcStartLineTable =
                new Xil_signed16*[ci.src1NumBands];
            for (int i = 0; i < ci.src1NumBands; i++)
                srcStartLineTable[i] = (Xil_signed16*) ci.getSrc1Data(i);
            
            Xil_unsigned8* dest_scanline = (Xil_unsigned8*) ci.destData;

            for(int y=ci.ysize; y>0; y--) { // each scanline
                for (band = 0; band < ci.src1NumBands; band++)
                    srcPtrTable[band] = srcStartLineTable[band];
                
                Xil_unsigned8* dest = dest_scanline;
                
                for(int x=0; x < ci.xsize; x++) { // each pixel
                    int closest_index;
                    closest_dist = XIL_MAXDOUBLE;
                    
                    for(int idx=0; idx < max_index; idx += lut_nbands) {
                        distance = 0;
                        
                        for (band = 0; band < ci.src1NumBands; band++) {
                            diff = *(srcPtrTable[band]) - lut[band+idx];
                            distance += diff * diff;
                        }
                        
                        // compare if distance is shorter than previous
                        if (distance < closest_dist) {
                            closest_dist = distance;
                            closest_index  = idx;
                        }
                    }

                    if ((lut_offset + closest_index / lut_nbands))
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
XilDeviceManagerComputeSHORT::NearestColor8(XilOp*       op,
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
    
    unsigned int   lut_size = lookup->getNumEntries();
    Xil_signed16*  lut=(Xil_signed16*)lookup->getData();
    unsigned short lut_nbands = lookup->getOutputNBands();
    short          lut_offset = lookup->getOffset();
    int            max_index  = lut_size * lut_nbands;

    while(ci.hasMoreInfo()) {
        //
        // destination is always single banded so don't check it's storage
        //
        double closest_dist, distance, diff;
        if ((ci.src1Storage.isType(XIL_PIXEL_SEQUENTIAL))) {
            Xil_signed16* src1_scanline = (Xil_signed16*) ci.src1Data;
            Xil_unsigned8*  dest_scanline = (Xil_unsigned8*)ci.destData;
            for(int y=ci.ysize; y>0; y--) {
                Xil_signed16* src1 = src1_scanline;
                Xil_unsigned8* dest = dest_scanline;
                int closest_entry;
                for(int x=ci.xsize; x>0; x--) {
                    closest_dist = XIL_MAXDOUBLE;
                    for(int b=0; b < max_index; b += lut_nbands) {
                        // go throught the whole LUT
                        distance = 0;
                        for (int i = lut_nbands - 1; i >= 0; i--) {
                            diff = (*(src1 + i) - lut[b + i]);
                            distance += diff * diff;
                        }
                        // compare if distance is shorter than previous
                        if (distance < closest_dist) {
                            closest_dist = distance;
                            closest_entry = b;
                        }
                    } // for colormap size
                    *dest = lut_offset + closest_entry / lut_nbands;
                    src1 += ci.src1PixelStride;
                    dest += ci.destPixelStride;
                }
                src1_scanline += ci.src1ScanlineStride;
                dest_scanline += ci.destScanlineStride;
            }
        } else {
            int band;
            // Create source data pointer tables
            Xil_signed16** srcPtrTable =
                new Xil_signed16*[ci.src1NumBands];
            Xil_signed16** srcStartLineTable =
                new Xil_signed16*[ci.src1NumBands];
            for (int i = 0; i < ci.src1NumBands; i++)
                srcStartLineTable[i] = (Xil_signed16 *) ci.getSrc1Data(i);
            
            Xil_unsigned8* dest_scanline = (Xil_unsigned8*) ci.destData;

            for(int y=ci.ysize; y>0; y--) { // each scanline
                for (band = 0; band < ci.src1NumBands; band++)
                    srcPtrTable[band] = srcStartLineTable[band];
                
                Xil_unsigned8* dest = dest_scanline;
                
                for(int x=ci.xsize; x>0; x--) { // each pixel
                    int closest_entry;
                    closest_dist = XIL_MAXDOUBLE;
                    
                    for(int idx=0; idx < max_index; idx += lut_nbands) {
                        distance = 0;
                        
                        for (band = 0; band < ci.src1NumBands; band++) {
                            diff = *(srcPtrTable[band]) - lut[band+idx];
                            distance += diff * diff;
                        }
                        
                        // compare if distance is shorter than previous
                        if (distance < closest_dist) {
                            closest_dist = distance;
                            closest_entry = idx;
                        }
                    }

                    *dest = lut_offset + closest_entry / lut_nbands;
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
XilDeviceManagerComputeSHORT::NearestColor16(XilOp*       op,
                                            unsigned     op_count,
                                            XilRoi*      roi,
                                            XilBoxList*  bl)
{
    ComputeInfoSHORT  ci(op, op_count, roi, bl);

    if(ci.isOK() == FALSE) {
        return ci.returnValue;
    }

    //
    // Assuming number of entries in colormap is not greater than 65536
    // GPG (giant programming guy) says this is impossible.
    //
    XilLookupSingle* lookup;
    op->getParam(1, (XilObject**) &lookup);
    
    unsigned int   lut_size = lookup->getNumEntries();
    Xil_signed16*  lut=(Xil_signed16*)lookup->getData();
    unsigned short lut_nbands = lookup->getOutputNBands();
    short          lut_offset = lookup->getOffset();
    int            max_index  = lut_size * lut_nbands;

    while(ci.hasMoreInfo()) {
        //
        // destination is always single banded so don't check it's storage
        //
        double closest_dist;
        double distance;
        double diff;
        if((ci.src1Storage.isType(XIL_PIXEL_SEQUENTIAL))) {
            Xil_signed16* src1_scanline = ci.src1Data;
            Xil_signed16* dest_scanline = ci.destData;
            for(int y=ci.ysize; y>0; y--) {
                Xil_signed16* src1 = src1_scanline;
                Xil_signed16* dest = dest_scanline;
                int closest_entry;

                for(int x=ci.xsize; x>0; x--) {
                    closest_dist = XIL_MAXDOUBLE;

                    for(int b=0; b < max_index; b += lut_nbands) {
                        //
                        //  Go throught the whole LUT
                        //
                        distance = 0;
                        for(int i = lut_nbands - 1; i >= 0; i--) {
                            diff      = (*(src1 + i) - lut[b + i]);
                            distance += diff * diff;
                        }

                        //
                        //  Compare if distance is shorter than previous
                        //
                        if(distance < closest_dist) {
                            closest_dist = distance;
                            closest_entry = b;
                        }
                    }

                    *dest = lut_offset + closest_entry / lut_nbands;

                    src1 += ci.src1PixelStride;
                    dest += ci.destPixelStride;
                }
                src1_scanline += ci.src1ScanlineStride;
                dest_scanline += ci.destScanlineStride;
            }
        } else {
            int band;
            //
            //  Create source data pointer tables
            //
            Xil_signed16** srcPtrTable =
                new Xil_signed16*[ci.src1NumBands];
            Xil_signed16** srcStartLineTable =
                new Xil_signed16*[ci.src1NumBands];

            for(int i = 0; i < ci.src1NumBands; i++) {
                srcStartLineTable[i] = ci.getSrc1Data(i);
            }

            Xil_signed16* dest_scanline = ci.destData;

            for(int y=ci.ysize; y>0; y--) {
                for (band = 0; band < ci.src1NumBands; band++)
                    srcPtrTable[band] = srcStartLineTable[band];
                
                Xil_signed16* dest = dest_scanline;
                
                for(int x=ci.xsize; x>0; x--) {
                    int closest_entry;
                    closest_dist = XIL_MAXDOUBLE;
                    
                    for(int idx=0; idx < max_index; idx += lut_nbands) {
                        distance = 0;
                        
                        for(band = 0; band < ci.src1NumBands; band++) {
                            diff = *(srcPtrTable[band]) - lut[band+idx];
                            distance += diff * diff;
                        }

                        //
                        // compare if distance is shorter than previous
                        //
                        if(distance < closest_dist) {
                            closest_dist = distance;
                            closest_entry = idx;
                        }
                    }

                    *dest = lut_offset + closest_entry / lut_nbands;
                    for(band = 0; band < ci.src1NumBands; band++) {
                        srcPtrTable[band] += ci.getSrc1PixelStride(band);
                    }
                    dest += ci.destPixelStride;
                }
                        
                for(band = 0; band < ci.src1NumBands; band++) {
                    srcStartLineTable[band] += ci.getSrc1ScanlineStride(band);
                }
                        
                dest_scanline += ci.destScanlineStride;
            }
            
            delete srcPtrTable;
            delete srcStartLineTable;
        }
    }

    return ci.returnValue;
}

