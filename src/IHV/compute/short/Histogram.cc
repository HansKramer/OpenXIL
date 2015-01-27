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
//  File:	Histogram.cc
//  Project:	XIL
//  Revision:	1.13
//  Last Mod:	10:11:57, 03/10/00
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
#pragma ident	"@(#)Histogram.cc	1.13\t00/03/10  "

//
//  C++ Includes
//
#include "XilDeviceManagerComputeSHORT.hh"
#include "ComputeInfo.hh"


#define _XILI_MAX_HISTO_BANDS 8

XilStatus
XilDeviceManagerComputeSHORT::Histogram(XilOp*       op,
                                      unsigned     op_count,
                                      XilRoi*      roi,
                                      XilBoxList*  bl)
{
    ComputeInfoSHORT  ci(op, op_count, roi, bl);

    if(ci.isOK() == FALSE) {
        return ci.returnValue;
    }

    unsigned int nbands = ci.src1NumBands;

    //
    //  Get params from the op
    //
    XilHistogram* histogram;
    unsigned int skip_x;
    unsigned int skip_y;
    op->getParam(1, (XilObject**) &histogram);
    op->getParam(2, &skip_x);
    op->getParam(3, &skip_y);

    //
    //  Allocate and init histogram data array
    //
    unsigned int nElements = histogram->getNumElements();
    unsigned int* data = new unsigned int[nElements];
    if(data == NULL) {
        XIL_ERROR(ci.getSystemState(), XIL_ERROR_RESOURCE, "di-1", TRUE);
        return XIL_FAILURE;
    }
    xili_memset(data, 0, sizeof(unsigned int) * nElements);

    //
    //  Get parameters from histogram object
    //
    const float* low          = histogram->getLowValues();
    const float* high         = histogram->getHighValues();
    const unsigned int* nbins = histogram->getNumBins();

    int           bin;
    unsigned int* bin_num;
    unsigned int* bin_num_ptr;
    float*        scalef = NULL;

    if(nbands <= _XILI_MAX_HISTO_BANDS) {
        //
        // Build a table for each band which takes a gray level
        // from -32768 to +32767 and returns the bin number 
        // into which that gray level falls
        //
        bin_num = new unsigned int[nbands*65536];
        if(bin_num == NULL) {
            delete [] data;
            XIL_ERROR(ci.getSystemState(), XIL_ERROR_RESOURCE, "di-1", TRUE);
            return XIL_FAILURE;
        }

        //
        // Point at zeroth element of the array
        //
        bin_num += 32768;

        int step = 1;
        bin_num_ptr = &bin_num[(nbands-1)*65536];
        for(int band=nbands-1; band>=0; band--) {
            float scale;

            if(nbins[band] != 1) {
                //
                //  Zero already taken care of in constructor
                //
                scale = (high[band] - low[band])/(nbins[band] - 1);
            } else {
                scale = high[band] - low[band];
            }

            if(scale == 0.0) {
                scale = 1.0;
            }

            for(int gray=-32768; gray<32768; gray++) {
                bin = (int)(((float)gray - low[band]) / scale + 0.5);
                //
                // Indicate OutOfGamut with -1
                //
                if(bin < 0 || bin >= nbins[band]) {
                    *(bin_num_ptr + gray) = -1;
                } else {
                    *(bin_num_ptr + gray) = bin * step;
                }
            }

            step *= nbins[band];
            bin_num_ptr -= 65536;
        }

    } else {
        scalef = new float[nbands];
        if(scalef == NULL) {
            XIL_ERROR(ci.getSystemState(), XIL_ERROR_RESOURCE, "di-1", TRUE);
            delete [] data;
            return XIL_FAILURE;
        }

        for(int i = 0; i < nbands; i++) {
            //
            //  Zero already taken care of in constructor
            //
            if(nbins[i] != 1) {
                scalef[i] = (high[i] - low[i])/(nbins[i] - 1);
            } else {
                scalef[i] = high[i] - low[i];
            }

            if(scalef[i] == 0.0) {
                scalef[i] = 1.0; 
            }
        }

    }

    int index;
    while(ci.hasMoreInfo()) {
        if(ci.isStorageType(XIL_PIXEL_SEQUENTIAL)) {
            // Pre-compute x and y increments
            unsigned int realScanlineStride = skip_y * ci.src1ScanlineStride;
            unsigned int realPixelStride = skip_x * ci.src1PixelStride;

            Xil_signed16* src1_scanline = ci.src1Data;

            unsigned int ycount = ci.ysize / skip_y;
            unsigned int xcount = ci.xsize / skip_x;

            if(nbands == 1) {
                for(int y=ycount; y!=0; y--) {
                    Xil_signed16* src1_pixel = src1_scanline;
                    
                    for(int x=xcount; x!=0; x--) {
                        index = *(bin_num + *src1_pixel);

                        if(index >= 0) {
                            data[index]++;
                        }
                        src1_pixel += realPixelStride;
                    }
                    src1_scanline += realScanlineStride;
                }

            } else if (nbands == 3) {

                for(int y=ycount; y!=0; y--) {
                    Xil_signed16* src1_pixel = src1_scanline;
                    
                    for(int x=xcount; x!=0; x--) {
                        
                        bin = *(bin_num + src1_pixel[0]);
                        if(bin >= 0) {
                            index = bin;
                        } else {
                            continue;
                        }

                        bin = *(bin_num + 65536 + src1_pixel[1]);
                        if(bin >= 0) {
                            index += bin;
                        } else {
                            continue;
                        }

                        bin = *(bin_num + 65536*2 + src1_pixel[2]);
                        if(bin >= 0) {
                            index += bin;
                        } else {
                            continue;
                        }

                        // Increment element
                        data[index]++;

                        src1_pixel += realPixelStride;
                    }
                    
                    src1_scanline += realScanlineStride;
                }
            } else if (nbands < _XILI_MAX_HISTO_BANDS) {
                //
                // N band case
                //
                for(int y = 0; y < ci.ysize; y += skip_y) {
                    Xil_signed16* src1_pixel = src1_scanline;
                    
                    for(int x = 0; x < ci.xsize; x += skip_x) {
                        Xil_signed16* src1_band = src1_pixel;
                        Xil_boolean ingamut = TRUE;
                        
                        bin_num_ptr = bin_num;
                        index = *(bin_num_ptr + *src1_band++);

                        if(index >= 0) {
                            for(int b=1; b<nbands; b++) {
                                //
                                // Find out what bin current pixel value maps to.
                                // Negative indicates an OutOfGamut value
                                //
                                bin_num_ptr += 65536;
                                bin = *(bin_num_ptr + *src1_band++);
                                if(bin < 0) {
                                    //
                                    // This one won't count - break out
                                    //
                                    ingamut = FALSE;
                                    break;
                                }
                                index += bin;
                            }

                            if(ingamut) {
                                // Increment element
                                data[index]++;
                            }

                        }
                        src1_pixel += realPixelStride;
                    }
                    
                    src1_scanline += realScanlineStride;
                }
            } else {
                //
                // N band case
                //
                 for(int y = 0; y < ci.ysize; y += skip_y) {
                    Xil_signed16* src1_pixel = src1_scanline;

                    for(int x = 0; x < ci.xsize; x += skip_x) {
                        Xil_signed16* src1_band = src1_pixel;
                        Xil_boolean ingamut = TRUE;

                        bin = (int)(((float)*src1_band++ - low[0]) /
                                       scalef[0] + 0.5);
                        index = bin;
                        if((bin >= 0) && (bin < nbins[0])) {
                            for(int b=1; b<nbands; b++) {
                                //
                                //  Find out what bin current value maps to
                                //
                                bin = (int)(((float)*src1_band++ - low[b]) /
                                               scalef[b] + 0.5);

                                //
                                //  Set flag if out of range
                                //
                                if((bin < 0) || (bin >= nbins[b])) {
                                    ingamut = FALSE;
                                    break;
                                }
                                index *= nbins[b];
                                index += bin;
                            }

                            if(ingamut) {
                                data[index]++;
                            }

                        }
                        src1_pixel += realPixelStride;
                    }

                    src1_scanline += realScanlineStride;
                }

            }


        } else { // XIL_BAND_SEQUENTIAL and XIL_GENERAL storage
            if(nbands <= _XILI_MAX_HISTO_BANDS) {
                for(int y = 0; y < ci.ysize; y += skip_y) { // each scanline
                    for(int x = 0; x < ci.xsize; x += skip_x) { // each pixel
                        Xil_boolean ingamut = TRUE;

                        Xil_signed16* src1_band = ci.getSrc1Data(0) +
                            y * ci.getSrc1ScanlineStride(0) +
                            x * ci.getSrc1PixelStride(0);

                        bin_num_ptr = bin_num;
                        index = *(bin_num_ptr + *src1_band);

                        if(index >= 0) {
                            for(int b=1; b<nbands; b++) {
                                // locate the band of the current pixel
                                src1_band = ci.getSrc1Data(b) +
                                    y * ci.getSrc1ScanlineStride(b) +
                                    x * ci.getSrc1PixelStride(b);

                                // find out what bin it maps to
                                bin_num_ptr += 65536;
                                bin = *(bin_num_ptr + *src1_band);
                                if(bin < 0) {
                                    ingamut = FALSE;
                                    break;
                                }
                                index += bin;
                            }

                            if(ingamut) {
                                // increment element
                                data[index]++;
                            }
                        }
                    }
                }
            } else {
                for(int y = 0; y < ci.ysize; y += skip_y) { // each scanline
                    for(int x = 0; x < ci.xsize; x += skip_x) { // each pixel
                        Xil_boolean ingamut = TRUE;

                        Xil_signed16* src1_band = ci.getSrc1Data(0) +
                            y * ci.getSrc1ScanlineStride(0) +
                            x * ci.getSrc1PixelStride(0);

                        bin = (int)(((float)*src1_band - low[0]) /
                                       scalef[0] + 0.5);
                        index = bin;

                        if((bin >= 0) && (bin < nbins[0])) {
                            for(int b = 0; b < nbands; b++) {
                                // Locate the band of the current pixel
                                src1_band = ci.getSrc1Data(b) +
                                    y * ci.getSrc1ScanlineStride(b) +
                                    x * ci.getSrc1PixelStride(b);

                                // Find out what bin it maps to
                                bin = (int)(((float)*src1_band - low[b]) /
                                               scalef[b] + 0.5);

                                // Set flag if out of range
                                if((bin < 0) || (bin >= nbins[b])) {
                                    ingamut = FALSE;
                                    break;
                                }
                                index *= nbins[b];
                                index += bin;
                            }

                            if(ingamut) {
                                data[index]++;
                            }
                        }
                    }
                }

            }
        }
    }

    //
    // Report the results for this strip
    //
    XilStatus status = op->reportResults((void**)&data);

    //
    // Delete the allocated arrays
    //
    delete [] data;
    delete [] (bin_num - 32768);

    if(status != XIL_SUCCESS) {
        return status;
    }

    return ci.returnValue;
}
