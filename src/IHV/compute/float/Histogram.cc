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
//  Revision:	1.10
//  Last Mod:	10:12:52, 03/10/00
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
#pragma ident	"@(#)Histogram.cc	1.10\t00/03/10  "

//
//  System Includes
//

//
//  C++ Includes
//
#include "XilDeviceManagerComputeFLOAT.hh"
#include "ComputeInfo.hh"


XilStatus
XilDeviceManagerComputeFLOAT::Histogram(XilOp*       op,
                                      unsigned     op_count,
                                      XilRoi*      roi,
                                      XilBoxList*  bl)
{

    ComputeInfoFLOAT ci(op, op_count, roi, bl);

    if(ci.isOK() == FALSE) {
        return ci.returnValue;
    }

    unsigned int nbands = ci.src1NumBands;

    //
    //  Get params from the op
    //
    XilImage* image = op->getSrcImage(1);

    XilHistogram* histogram;
    op->getParam(1, (XilObject**) &histogram);

    unsigned int  skip_x;
    op->getParam(2, &skip_x);

    unsigned int  skip_y;
    op->getParam(3, &skip_y);

    //
    //  Allocate and init histogram data array
    //
    unsigned int  nElements = histogram->getNumElements();
    unsigned int* data      = new unsigned int[nElements];
    if(data == NULL) {
        XIL_ERROR(image->getSystemState(), XIL_ERROR_RESOURCE, "di-1", TRUE);
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
    //  Allocate and init scale arrays -- needs to be double to properly hold
    //  the division results of single precision floating point.
    //
    double* scale = new double[nbands];
    if(scale == NULL) {
        delete [] data;

        XIL_ERROR(image->getSystemState(), XIL_ERROR_RESOURCE, "di-1", TRUE);
	return XIL_FAILURE;
    }

    for(int i = 0; i < nbands; i++) {
        //
        //  Zero already taken care of in constructor
        //
	if(nbins[i] != 1) {
	    scale[i] = ((double)high[i] - (double)low[i])/(double)(nbins[i] - 1);
        } else {
            scale[i] = (double)high[i] - (double)low[i];
        }

	if(scale[i] == 0.0) {
            scale[i] = 1.0;
        }
    }

    int bin;
    while(ci.hasMoreInfo()) {
        if(ci.isStorageType(XIL_PIXEL_SEQUENTIAL)) {
            //
	    // Pre-compute x and y increments
            //
	    unsigned int skip_scanline_stride = skip_y * ci.src1ScanlineStride;
	    unsigned int skip_pixel_stride    = skip_x * ci.src1PixelStride;
            unsigned int xsize                = ci.xsize;
            Xil_float32* src1_scanline        = ci.src1Data;

            for(int y = 0; y < ci.ysize; y += skip_y) {
                Xil_float32* src1_pixel = src1_scanline;
                
                for(int x = 0; x < xsize; x += skip_x) {
                    Xil_float32* src1_band = src1_pixel;
		    Xil_boolean ingamut = TRUE;

                    bin = (int)((*src1_band++ - low[0]) / scale[0] + 0.5);

                    if((bin >= 0) && (bin < nbins[0])) {
                        unsigned int index = bin;

                        for(int b=1; b<nbands; b++) {
                            //
                            //  Find out what bin current value maps to
                            //
                            bin =
                                (int)((*src1_band++ - low[b]) / scale[b] + 0.5);

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

                    src1_pixel += skip_pixel_stride;
                }
                
                src1_scanline += skip_scanline_stride;
            }
        } else {
	    for(int y = 0; y < ci.ysize; y += skip_y) { // each scanline
		for(int x = 0; x < ci.xsize; x += skip_x) { // each pixel
		    Xil_boolean  ingamut = TRUE;
                    Xil_float32* src1_band = ci.getSrc1Data(0) +
                                y * ci.getSrc1ScanlineStride(0) +
                                x * ci.getSrc1PixelStride(0);

                    //
                    //  Find out what bin it maps to
                    //
                    bin = (int)((*src1_band - low[0]) / scale[0] + 0.5);

                    if((bin >= 0) && (bin < nbins[0])) {
                        unsigned int index = bin;

                        for(int b = 1; b < nbands; b++) {
                            //
                            //  Locate the current pixel.
                            //
                            src1_band = ci.getSrc1Data(b) +
                                y * ci.getSrc1ScanlineStride(b) +
                                x * ci.getSrc1PixelStride(b);

                            //
                            //  Find out what bin it maps to
                            //
                            bin = (int)((*src1_band - low[b]) /
                                           scale[b] + 0.5);

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
		}
	    }
        }
    }

    //
    //  Clean up allocated memory
    //
    delete [] scale;

    if(ci.returnValue != XIL_SUCCESS) {
        delete [] data;
	return ci.returnValue;
    }

    XilStatus status = op->reportResults((void**)&data);

    delete [] data;

    return status;
}
