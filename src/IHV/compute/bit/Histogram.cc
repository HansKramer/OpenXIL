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
//  Last Mod:	10:09:34, 03/10/00
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
#pragma ident	"@(#)Histogram.cc	1.10\t00/03/10  "

//
//  C++ Includes
//
#include "XilDeviceManagerComputeBIT.hh"
#include "ComputeInfo.hh"


XilStatus
XilDeviceManagerComputeBIT::Histogram(XilOp*       op,
                                      unsigned     op_count,
                                      XilRoi*      roi,
                                      XilBoxList*  bl)
{
    ComputeInfoBIT ci(op, op_count, roi, bl);

    if(ci.isOK() == FALSE) {
        return ci.returnValue;
    }

    unsigned int nbands = ci.src1NumBands;

    //
    //  Get params from the op
    //
    XilHistogram* histogram;
    op->getParam(1, (XilObject**)&histogram);

    unsigned int skip_x;
    op->getParam(2, &skip_x);

    unsigned int skip_y;
    op->getParam(3, &skip_y);

    //
    //  Allocate and init histogram data array
    //
    unsigned int  nElements = histogram->getNumElements();
    unsigned int* data      = new unsigned int[nElements];
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
    //  Allocate and init scale arrays
    //
    float* scale = new float[nbands];
    if(scale == NULL) {
        delete [] data;
        XIL_ERROR(ci.getSystemState(), XIL_ERROR_RESOURCE, "di-1", TRUE);
	return XIL_FAILURE;
    }
    
    for(int i = 0; i < nbands; i++) {
        //
        //  Zero already taken care of in constructor.
        //
	if(nbins[i] != 1) {
	    scale[i] = (high[i] - low[i])/(nbins[i] - 1);
        } else {
            scale[i] = high[i] - low[i];
        }
        
	if(scale[i] == 0.0) {
            scale[i] = 1.0;
        }
    }

    int* bin = new int[nbands];
    if(bin == NULL) {
        delete [] scale;
        delete [] data;
        XIL_ERROR(ci.getSystemState(), XIL_ERROR_RESOURCE, "di-1", TRUE);
	return XIL_FAILURE;
    }

    while(ci.hasMoreInfo()) {
        //
        //  Do each scanline
        //
	for(int y = 0; y < ci.ysize; y += skip_y) {
            //
            // Do each pixel
            //
	    for(int x = 0; x < ci.xsize; x += skip_x) {
		Xil_boolean ingamut = TRUE;

		for(int b = 0; b < nbands; b++) {
                    //
		    //  Locate beginning of scanline
                    //
		    Xil_unsigned8* src1_scanline =
                        ci.getSrc1Data(b) + y * ci.getSrc1ScanlineStride(b);

                    //
		    //  Get the value of the pixel
                    //
		    Xil_unsigned8 bmap_val =
                        XIL_BMAP_TST(src1_scanline,
                                     ci.getSrc1Offset(b) + x) ? 1 : 0;

                    //
		    //  Find out what bin it maps to
                    //
		    bin[b] = (int)(((float)bmap_val - low[b])/
                                   scale[b] + 0.5);

                    //
		    //  Set flag if out of range
                    //
		    if((bin[b] < 0) || (bin[b] >= nbins[b])) {
			ingamut = FALSE;
		    }
		}

		if(ingamut) {
                    //
		    //  Compute the array index
                    //
		    unsigned int index = bin[0];

		    for(b = 1; b < nbands; b++) {
			index *= nbins[b];
			index += bin[b];
		    }

                    //
		    //  Increment element
                    //
		    data[index]++;
		}
	    }
	}
    }

    //
    //  Clean up allocated memory
    //
    delete [] bin;
    delete [] scale;

    XilStatus status = op->reportResults((void**)&data);

    delete [] data;

    if(status != XIL_SUCCESS) {
	return status;
    }

    return ci.returnValue;
}
