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
//  MT-level:  <??????>
//	
//------------------------------------------------------------------------
//	COPYRIGHT
//------------------------------------------------------------------------
#pragma ident	"@(#)Histogram.cc	1.21\t00/03/10  "

//
//  C++ Includes
//
#include "XilDeviceManagerComputeBYTE.hh"
#include "ComputeInfo.hh"


XilStatus
XilDeviceManagerComputeBYTE::Histogram(XilOp*       op,
                                      unsigned     op_count,
                                      XilRoi*      roi,
                                      XilBoxList*  bl)
{
    ComputeInfoBYTE  ci(op, op_count, roi, bl);

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

    //
    // Build a table for each band which takes a gray level
    // from 0 to 255 and returns the bin number into which that
    // gray level falls
    //
    int* bin_num = new int[nbands*256];
    if(bin_num == NULL) {
        delete [] data;
        XIL_ERROR(ci.getSystemState(), XIL_ERROR_RESOURCE, "di-1", TRUE);
        return XIL_FAILURE;
    }

    int bin;
    int step = 1;
    int* bin_num_ptr = &bin_num[(nbands-1)*256];
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

        for(int gray=0; gray<256; gray++) {
            bin = (int)(((float)gray - low[band]) / scale + 0.5);
            //
            // Indicate OutOfGamut with negative value
            //
            if(bin < 0 || bin >= nbins[band]) {
                // This value insures OutOfGamut 3 band case stays negative.
                *(bin_num_ptr + gray) = - (1<<26);
            } else {
                *(bin_num_ptr + gray) = bin * step;
            }
        }

        step *= nbins[band];
        bin_num_ptr -= 256;
    }

    int index;
    while(ci.hasMoreInfo()) {
        if(ci.isStorageType(XIL_PIXEL_SEQUENTIAL)) {
            // Pre-compute x and y increments
            unsigned int realScanlineStride = skip_y * ci.src1ScanlineStride;
            unsigned int realPixelStride = skip_x * ci.src1PixelStride;

            Xil_unsigned8* src1_scanline = ci.src1Data;

            unsigned int ycount = ci.ysize / skip_y;
            unsigned int xcount = ci.xsize / skip_x;

            if(nbands == 1) {
                for(int y=ycount; y!=0; y--) {
                    Xil_unsigned8* src1_pixel = src1_scanline;
                    
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

                int* bin0 = bin_num;  
                int* bin1 = bin_num + 256;  
                int* bin2 = bin_num + 512;  
                for(int y=ycount; y!=0; y--) {
                    Xil_unsigned8* src1_pixel = src1_scanline;
                    
                    for(int x=xcount; x!=0; x--) {
                        
                        bin = *(bin0 + src1_pixel[0]);
                        bin += *(bin1 + src1_pixel[1]);
                        bin += *(bin2 + src1_pixel[2]);

                        // Increment element
                        if(bin >= 0) {
                            data[bin]++;
                        }

                        src1_pixel += realPixelStride;
                    }
                    
                    src1_scanline += realScanlineStride;
                }
            } else {
                //
                // N band case
                //
                for(int y=ycount; y!=0; y--) {
                    Xil_unsigned8* src1_pixel = src1_scanline;
                    
                    for(int x=xcount; x!=0; x--) {
                        Xil_unsigned8* src1_band = src1_pixel;
                        
                        bin_num_ptr = bin_num;
                        index = *(bin_num_ptr + *src1_band++);

                        if(index >= 0) {
                            Xil_boolean ingamut = TRUE;
                            for(unsigned int b=1; b<nbands; b++) {
                                //
                                // Find out what bin current pixel value maps to.
                                // Negative indicates an OutOfGamut value
                                //
                                bin_num_ptr += 256;
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
            }

        } else { // XIL_BAND_SEQUENTIAL and XIL_GENERAL storage
            for(unsigned int y=0; y < ci.ysize; y += skip_y) { // each scanline
                for(unsigned int x=0; x < ci.xsize; x += skip_x) { // each pixel
                    Xil_boolean ingamut = TRUE;

                    Xil_unsigned8* src1_band = ci.getSrc1Data(0) +
                        y * ci.getSrc1ScanlineStride(0) +
                        x * ci.getSrc1PixelStride(0);

                    bin_num_ptr = bin_num;
                    index = *(bin_num_ptr + *src1_band);

                    if(index >= 0) {
                        for(unsigned int b=1; b<nbands; b++) {
                            // locate the band of the current pixel
                            src1_band = ci.getSrc1Data(b) +
                                y * ci.getSrc1ScanlineStride(b) +
                                x * ci.getSrc1PixelStride(b);

                            // find out what bin it maps to
                            bin_num_ptr += 256;
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
    delete [] bin_num;

    if(status != XIL_SUCCESS) {
        return status;
    }

    return ci.returnValue;
}
