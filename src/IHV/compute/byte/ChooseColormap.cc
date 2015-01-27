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
//  File:       ChooseColormap.cc
//  Project:    XIL
//  Revision:   1.5
//  Last Mod:   10:11:23, 03/10/00
//
//  Description:
//
//    Create a best fit colormap for a 3 band byte image
//    using a median-cut type of algorithm.
//
//
//------------------------------------------------------------------------
//    COPYRIGHT
//------------------------------------------------------------------------
#pragma ident   "@(#)ChooseColormap.cc	1.5\t00/03/10  "

//
//  C++ Includes
//
#include <stdio.h>
#include <xil/xilGPI.hh>
#include "XilDeviceManagerComputeBYTE.hh"
#include "ComputeInfo.hh"
#include "XiliColormapGenerator.hh"

XilStatus
XilDeviceManagerComputeBYTE::ChooseColormapPreprocess(
    XilOp*        op,
    unsigned      ,
    XilRoi*       ,
    void**        compute_data,
    unsigned int* )
{
    //
    // Instantiate a Master Colormap generator object which will
    // be used to accumulate the statistics from the individual
    // tiles.
    //
    XiliColormapGenerator* totalCmapGen = new XiliColormapGenerator;
    if(totalCmapGen == NULL) {
        XIL_ERROR(op->getSrcImage(1)->getSystemState(), 
                  XIL_ERROR_RESOURCE, "di-1", TRUE);
        return XIL_FAILURE;
    }

    //
    // Record totalCmapGen in compute_data, so its can be
    // used in compute routine and then destroyed by the Postprocess routine
    //
    *compute_data = (void*)totalCmapGen;

    return XIL_SUCCESS;
}

XilStatus
XilDeviceManagerComputeBYTE::ChooseColormapPostprocess(
    XilOp*        op,
    void*         compute_data)
{

    //
    // Get the parameters from the Op
    //
    unsigned int cmap_size;
    op->getParam(1, &cmap_size);

    XilLookupSingle**  user_lut;
    op->getParam(2, (XilObject**) &user_lut);

    //
    // Create colormap table on the stack and clear it to zero
    //
    Xil_unsigned8 cmap_data[768];
    xili_memset(cmap_data, 0, 768);

    //
    // Retrieve totalCmapGen from the compute_data
    //
    XiliColormapGenerator* totalCmapGen = (XiliColormapGenerator*)compute_data;

    //
    //  Build the best colormap into the cmap_data array
    //
    totalCmapGen->generateColormap(cmap_data, cmap_size);

    //
    // Build an XilLookup object to be returned to the user.
    // This is built with an offset of zero.
    //
    *user_lut = op->getSrcImage(1)->getSystemState()->createXilLookupSingle(
                      XIL_BYTE, XIL_BYTE, 3, cmap_size, 0, cmap_data);

    if(*user_lut == NULL) {
        XIL_ERROR(op->getSrcImage(1)->getSystemState(), 
                      XIL_ERROR_RESOURCE, "di-1", TRUE);
        return XIL_FAILURE;
    }

    delete totalCmapGen;

    return XIL_SUCCESS;
}

XilStatus
XilDeviceManagerComputeBYTE::ChooseColormap(XilOp*       op,
                                            unsigned     op_count,
                                            XilRoi*      roi,
                                            XilBoxList*  bl)
{
    ComputeInfoBYTE  ci(op, op_count, roi, bl);

    if(ci.isOK() == FALSE) {
        return ci.returnValue;
    }

    XilImage* image = op->getSrcImage(1);

    //
    // Retrieve the grand total CmapGen object
    //
    XiliColormapGenerator* totalCmapGen = 
            (XiliColormapGenerator*)op->getPreprocessData(this);

    //
    // Instantiate a new colormap generator object.
    // This will be used to collect the stats for this tile/chunk.
    //
    XiliColormapGenerator* cmapGen = new XiliColormapGenerator;
    if(cmapGen == NULL) {
        XIL_ERROR(ci.getSystemState(), XIL_ERROR_RESOURCE, "di-1", TRUE);
        return XIL_FAILURE;
    }

    while(ci.hasMoreInfo()) {
        if(ci.isStorageType(XIL_PIXEL_SEQUENTIAL)) {

            cmapGen->hist3dPixSeq(ci.src1Data,
                                  ci.xsize, ci.ysize,
                                  ci.src1PixelStride,
                                  ci.src1ScanlineStride);
                                  

        } else { // XIL_BAND_SEQUENTIAL and XIL_GENERAL storage

            Xil_unsigned8* src_ptr[3];
            unsigned int   src_ps[3];
            unsigned int   src_ss[3];
            for(int band=0; band<3; band++) {
                src_ptr[band] = ci.getSrc1Data(band);
                src_ps[band] = ci.getSrc1PixelStride(band);
                src_ss[band] = ci.getSrc1ScanlineStride(band);
            }
            cmapGen->hist3dGeneral(src_ptr,
                                   ci.xsize, ci.ysize,
                                   src_ps, src_ss);

        }

    }

    //
    // Accumulate local stats into the master Cmap Generator object.
    // We must lock around this operation, since the data from
    // multiple tiles may be getting summed simultaneously.
    //
    cmapGenMutex.lock();
    totalCmapGen->accumulateHistogram(cmapGen);
    cmapGenMutex.unlock();

    //
    // Get rid of the local Cmap Generator object
    //
    delete cmapGen;

    return XIL_SUCCESS;
}

