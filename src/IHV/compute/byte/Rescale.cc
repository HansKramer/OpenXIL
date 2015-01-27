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
//  File:	Rescale.cc
//  Project:	XIL
//  Revision:	1.18
//  Last Mod:	10:10:21, 03/10/00
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
#pragma ident	"@(#)Rescale.cc	1.18\t00/03/10  "

#include <string.h>

#include "XilDeviceManagerComputeBYTE.hh"
#include "ComputeInfo.hh"

XilStatus
XilDeviceManagerComputeBYTE::RescalePreprocess(XilOp*        op,
                                               unsigned      ,
                                               XilRoi*       ,
                                               void**        compute_data,
                                               unsigned int* )
{
    //
    //  Get the multConst and addConst for this operation.
    //
    float* multConst;
    op->getParam(1, (void **)&multConst);

    float* addConst;
    op->getParam(2, (void **)&addConst);

    unsigned int    nbands = op->getDstImage(1)->getNumBands();
    XilSystemState* state  = op->getDstImage(1)->getSystemState();
    
    //
    //  Get the cache number we can use...
    //
    *compute_data = (void*)getRescaleTables(state, multConst, addConst, nbands);

    return XIL_SUCCESS;
}

XilStatus
XilDeviceManagerComputeBYTE::RescalePostprocess(XilOp*       op,
                                                void*        compute_data)
{
    releaseRescaleTables((int*)compute_data,
                         op->getDstImage(1)->getNumBands());

    return XIL_SUCCESS;
}

XilStatus
XilDeviceManagerComputeBYTE::Rescale(XilOp*       op,
				     unsigned     op_count,
				     XilRoi*      roi,
				     XilBoxList*  bl)
{
    ComputeInfoBYTE  ci(op, op_count, roi, bl);

    if(ci.isOK() == FALSE) {
        return ci.returnValue;
    }

    int* tables = (int*)op->getPreprocessData(this);

    if(tables == NULL) {
        //
        //  Do operation old-style...
        //
        float* multConst;
        op->getParam(1, (void**)&multConst);

        float* addConst;
        op->getParam(2, (void**)&addConst);

        while(ci.hasMoreInfo()) {
            float result;

            COMPUTE_GENERAL_1S_1D_W_BAND(Xil_unsigned8, Xil_unsigned8,

                                         result = (_XILI_B2F(*src1) *
                                                   multConst[0]) + addConst[0];
                                         *dest = _XILI_ROUND_U8(result),

                                         result = (_XILI_B2F(*(src1+1)) *
                                                   multConst[1]) + addConst[1];
                                         *(dest+1) = _XILI_ROUND_U8(result);
                                         result = (_XILI_B2F(*(src1+2)) *
                                                   multConst[2]) + addConst[2];
                                         *(dest+2) = _XILI_ROUND_U8(result),

                                         result = (_XILI_B2F(*src1) *
                                                   multConst[band]) + addConst[band];
                                         *dest  = _XILI_ROUND_U8(result)
                    );
        }
    } else {
        tableRescale(ci, tables);
    }

    return ci.returnValue;
}

//
// Passing ci by reference because I want to use the macros
//
void
XilDeviceManagerComputeBYTE::tableRescale(ComputeInfoBYTE& ci,
                                          int*             tables)
{
    unsigned int nbands = ci.destNumBands;

    while(ci.hasMoreInfo()) {
        if(ci.isStorageType(XIL_PIXEL_SEQUENTIAL)) {
            Xil_unsigned8* src1_scanline = ci.src1Data; 
            Xil_unsigned8* dest_scanline = ci.destData; 

            if(nbands == 1) {
                if(ci.src1PixelStride == 1 && ci.destPixelStride == 1) {
                    unsigned int src1_ss = ci.src1ScanlineStride;
                    unsigned int dest_ss = ci.destScanlineStride;

                    Xil_unsigned8* r_lut = rescaleCache[tables[0]];

                    unsigned int xsize   = ci.xsize;

                    for(int y=ci.ysize; y>0; y--) {
                        fastLookupWrite(src1_scanline, dest_scanline,
                                        xsize, r_lut);

                        src1_scanline += src1_ss;
                        dest_scanline += dest_ss;
                    }
                } else {
                    unsigned int src1_ps = ci.src1PixelStride;
                    unsigned int dest_ps = ci.destPixelStride;

                    unsigned int src1_ss = ci.src1ScanlineStride;
                    unsigned int dest_ss = ci.destScanlineStride;

                    Xil_unsigned8* r_lut = rescaleCache[tables[0]];
                    unsigned int xsize   = ci.xsize;

                    for(int y=ci.ysize; y>0; y--) {
                        Xil_unsigned8* src1 = src1_scanline;
                        Xil_unsigned8* dest = dest_scanline;

                        for(int x=xsize; x>0; x--) {
                            *dest = r_lut[*src1];

                            src1 += src1_ps;
                            dest += dest_ps;
                        }

                        src1_scanline += src1_ss;
                        dest_scanline += dest_ss;
                    }
                }
            } else if(nbands == 3) {
                unsigned int src1_ps = ci.src1PixelStride;
                unsigned int dest_ps = ci.destPixelStride;

                unsigned int src1_ss = ci.src1ScanlineStride;
                unsigned int dest_ss = ci.destScanlineStride;

                Xil_unsigned8* r_lut_0 = rescaleCache[tables[0]];
                Xil_unsigned8* r_lut_1 = rescaleCache[tables[1]];
                Xil_unsigned8* r_lut_2 = rescaleCache[tables[2]];

                Xil_unsigned8* src1;
                Xil_unsigned8* dest;

                for(int y=ci.ysize; y>0; y--) {
                    src1 = src1_scanline;
                    dest = dest_scanline;

                    for(int x=ci.xsize; x>0; x--) {
                        *dest     = r_lut_0[*src1];
                        *(dest+1) = r_lut_1[*(src1+1)];
                        *(dest+2) = r_lut_2[*(src1+2)];

                        src1 += src1_ps;
                        dest += dest_ps;
                    }

                    src1_scanline += src1_ss;
                    dest_scanline += dest_ss;
                }
            } else {
                for(int y=ci.ysize; y>0; y--) {
                    Xil_unsigned8* src1_pixel = src1_scanline;
                    Xil_unsigned8* dest_pixel = dest_scanline;

                    for(int x=ci.xsize; x>0; x--) {
                        Xil_unsigned8* src1 = src1_pixel;
                        Xil_unsigned8* dest = dest_pixel;

                        for(unsigned int band=0; band<nbands; band++) {
                            *dest = (rescaleCache[tables[band]])[*src1];

                            src1++;
                            dest++;
                        }

                        src1_pixel += ci.src1PixelStride;
                        dest_pixel += ci.destPixelStride;
                    }

                    src1_scanline += ci.src1ScanlineStride;
                    dest_scanline += ci.destScanlineStride;
                }
            }
        } else {
            for(unsigned int band=0; band<nbands; band++) {
                int src1PixelStride          = ci.getSrc1PixelStride(band);
                int destPixelStride          = ci.getDestPixelStride(band);

                int src1ScanlineStride       = ci.getSrc1ScanlineStride(band);
                int destScanlineStride       = ci.getDestScanlineStride(band);

                Xil_unsigned8* src1_scanline = ci.getSrc1Data(band);
                Xil_unsigned8* dest_scanline = ci.getDestData(band);

                Xil_unsigned8* rlut          = rescaleCache[tables[band]];

                for(int y=ci.ysize; y>0; y--) { 
                    Xil_unsigned8* src1 = src1_scanline;
                    Xil_unsigned8* dest = dest_scanline;

                    for(int x=ci.xsize; x>0; x--) {
                        *dest = rlut[*src1];

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
