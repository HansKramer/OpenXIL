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
//  File:	BandCombine.cc
//  Project:	XIL
//  Revision:	1.9
//  Last Mod:	10:10:48, 03/10/00
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
#pragma ident	"@(#)BandCombine.cc	1.9\t00/03/10  "

#include "XilDeviceManagerComputeBYTE.hh"
#include "ComputeInfo.hh"

XilStatus
XilDeviceManagerComputeBYTE::BandCombine(XilOp*       op,
                                         unsigned     op_count,
                                         XilRoi*      roi,
                                         XilBoxList*  bl)
{
    ComputeInfoBYTE  ci(op, op_count, roi, bl);

    if(ci.isOK() == FALSE) {
        return ci.returnValue;
    }
    XilKernel*    kernel;

    op->getParam(1, (void**)&kernel);

    unsigned int mwidth = kernel->getWidth();
    unsigned int mheight = kernel->getHeight();
    const float* mdata = kernel->getData();
    
    while(ci.hasMoreInfo()) {
        if(ci.isStorageType(XIL_PIXEL_SEQUENTIAL)) {
            Xil_unsigned8* src1_scanline = ci.src1Data;
            Xil_unsigned8* dest_scanline = ci.destData;

            unsigned int   src1_sstride  = ci.src1ScanlineStride;
            unsigned int   dest_sstride  = ci.destScanlineStride;

            unsigned int   src1_pstride  = ci.src1PixelStride;
            unsigned int   dest_pstride  = ci.destPixelStride;

            for(int y=ci.ysize; y>0; y--) {
                Xil_unsigned8* src1_pixel = src1_scanline;
                Xil_unsigned8* dest_pixel = dest_scanline;

                for(int x=ci.xsize; x>0; x--) {
                    Xil_unsigned8* src1 = src1_pixel;
                    Xil_unsigned8* dest = dest_pixel;

                    //
                    //  Do the per-pixel matrix multiply
                    //
                    const float* mptr = mdata;

                    //
                    //  Loop over dst bands
                    //
                    for(unsigned int m=0; m<mheight; m++) {
                        float sum = 0.0;

                        //
                        //  Loop over src bands
                        //
                        for(unsigned int k=0; k<mwidth - 1; k++) {
                            sum += *mptr * _XILI_B2F(*(src1 + k));
                            mptr++;
                        }

                        //
                        //  Add in the constant value
                        //
                        sum += *mptr++;

                        *(dest+m) = _XILI_ROUND_U8(sum);
                    }

                    src1_pixel += src1_pstride;
                    dest_pixel += dest_pstride;
                }

                src1_scanline += src1_sstride;
                dest_scanline += dest_sstride;
            }
        } else {
            if(ci.isStorageType(XIL_BAND_SEQUENTIAL)) {
                Xil_unsigned8* src1_scanline = ci.src1Data;
                Xil_unsigned8* dest_scanline = ci.destData;

                unsigned int   src1_bstride  = ci.src1BandStride;
                unsigned int   dest_bstride  = ci.destBandStride;

                unsigned int   src1_sstride  = ci.src1ScanlineStride;
                unsigned int   dest_sstride  = ci.destScanlineStride;

                for(int y=ci.ysize; y>0; y--) {
                    Xil_unsigned8* src1_pixel = src1_scanline;
                    Xil_unsigned8* dest_pixel = dest_scanline;

                    for(int x=ci.xsize; x>0; x--) {
                        //
                        //  Do the per-pixel matrix multiply
                        //
                        const float* mptr = mdata;

                        //
                        //  Loop over dst bands
                        //
                        for(unsigned int m=0; m<mheight; m++) {
                            float sum = 0.0;

                            //
                            //  Loop over src bands
                            //
                            for(unsigned int k=0; k<mwidth - 1; k++) {
                                sum += *mptr * _XILI_B2F(*(src1_pixel + k*src1_bstride));
                                mptr++;
                            }

                            //
                            //  Add in the constant value
                            //
                            sum += *mptr++;

                            *(dest_pixel+m*dest_bstride) = _XILI_ROUND_U8(sum);
                        }

                        //
                        //  Go to the next pixel
                        //
                        src1_pixel++;
                        dest_pixel++;
                    }

                    //
                    //  Go to next scanline
                    //
                    src1_scanline += src1_sstride;
                    dest_scanline += dest_sstride;
                }
            } else {
                //
                //  General Storage (ci.isStorageType(XIL_GENERAL) == TRUE)
                //
                for(int y=0; y<ci.ysize; y++) {

                    for(int x=0; x<ci.xsize; x++) {
                        //
                        // Reset the matrix data pointer.
                        //
                        const float* mptr = mdata;

                        //
                        //  Loop over dst bands
                        //
                        for(unsigned int m=0; m<mheight; m++) {
                            //
                            // Get strides and data ptr for current dst band
                            //
                            unsigned int dest_sstride =
                                ci.getDestScanlineStride(m);
                            unsigned int dest_pstride =
                                ci.getDestPixelStride(m);
                            Xil_unsigned8*  dest_data =
                                ci.getDestData(m);

                            //
                            // Set ptr to dst pixel value for current band.
                            //
                            Xil_unsigned8* dest_band = dest_data +
                                y * dest_sstride +
                                x * dest_pstride;

                            //
                            // Clear accumulator variable
                            //
                            float sum = 0.0F;

                            //
                            //  Loop over src bands
                            //
                            for(unsigned int k=0; k<mwidth - 1; k++) {
                                //
                                // Get strides and data ptr for current src band
                                //
                                unsigned int src1_sstride =
                                    ci.getSrc1ScanlineStride(k);
                                unsigned int src1_pstride =
                                    ci.getSrc1PixelStride(k);
                                Xil_unsigned8*  src1_data =
                                    ci.getSrc1Data(k);

                                //
                                // Set ptr to src pixel value for current band.
                                //
                                Xil_unsigned8* src1_band = src1_data +
                                    y * src1_sstride +
                                    x * src1_pstride;

                                //
                                // Multiply src pixel value for this band by
                                // associated weight value and accumulate.
                                //
                                sum += (*mptr++) * _XILI_B2F(*src1_band);
                            }

                            //
                            //  Add in the constant value
                            //
                            sum += *mptr++;

                            //
                            // Store result in current dst band address
                            //
                            *dest_band = _XILI_ROUND_U8(sum);
                        }
                    }
                }
            }
        }
    }

    return ci.returnValue;
}
