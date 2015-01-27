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
//  File:       Rescale.cc
//  Project:    XIL
//  Revision:   1.5
//  Last Mod:   10:13:47, 03/10/00
//
//  Description:
//
//    Implement the rescale operation. This is converted
//    into a lookup operation.
//
//
//------------------------------------------------------------------------
//    COPYRIGHT
//------------------------------------------------------------------------
#pragma ident   "@(#)Rescale.cc	1.5\t00/03/10  "


#include "XilDeviceManagerComputeMMXBYTE.hh"
#include "ComputeInfoMMX.hh"


//
// Use the preprocess routine to create the IplLUT objects
// for the iplContrastStretch (lookup) operation
// 
XilStatus
XilDeviceManagerComputeMMXBYTE::RescalePreprocess(XilOp*       op,
                                                  unsigned     ,
                                                  XilRoi*      ,
                                                  void**       compute_data,
                                                  unsigned int*  )
{

    //
    // Get the parameters of the rescale operation
    //
    unsigned int nbands = op->getDstImage(1)->getNumBands();

    float* multConst;
    op->getParam(1, (void **)&multConst);

    float* addConst;
    op->getParam(2, (void **)&addConst);

    //
    // Fill in IplLUT structs (one per band) with a complete
    // lookup table constructed from the rescale parameters.
    //
    IplLUT** lut_array = new IplLUT*[nbands];
    MMX_MEMCHK(lut_array);

    int*     key_array = new int[257];
    MMX_MEMCHK(key_array);
    for(int i=0; i<257; i++) {
        key_array[i] = i;
    }

    for(unsigned int band=0; band<nbands; band++) {

        //
        // Special case all channels having the same params.
        // Just reuse the first band's lut in that case
        //
        if((int) band > 0                 &&
           addConst[band]  == addConst[0] &&
           multConst[band] == multConst[0]) {

            lut_array[band] = lut_array[0];

        } else {

            IplLUT* lut          = new IplLUT;
            MMX_MEMCHK(lut);
            lut_array[band]      = lut;
            lut_array[band]->key = key_array;
            lut->value           = new int[256];
            MMX_MEMCHK(lut->value);
            lut->num             = 257;
            lut->interpolateType = IPL_LUT_LOOKUP;
            lut->factor          = NULL;

            //
            // Evaluate the effect of the rescale and load it into the lut
            //
            for(i=0; i<256; i++) {
                int val = (int) ((float)i * multConst[band] + 
                                        addConst[band] + 0.5F);
                if(val < 0) {
                    val = 0;
                } else if(val > 255) {
                    val = 255;
                }
                lut->value[i] = val;
            }
        }

    }

    *compute_data = (void*)lut_array;;

    return XIL_SUCCESS;
}

//
// Cleanup the IplLUT objects after the operation
//
XilStatus
XilDeviceManagerComputeMMXBYTE::RescalePostprocess(XilOp*        op,
                                                    void*        compute_data)
{
    
    unsigned int nbands = op->getDstImage(1)->getNumBands();

    IplLUT** lut_array = (IplLUT**) compute_data;

    //
    // NULL the ptrs after the deletion, since we often use the
    // same ptr for multiple bands. This will avoid duplicate deletions.
    //
    for(unsigned int band=0; band<nbands; band++) {
        delete [] lut_array[band]->key;
        lut_array[band]->key = NULL;

        delete [] lut_array[band]->value;
        lut_array[band]->value = NULL;

        delete lut_array[band];
        lut_array[band] = NULL;
    }

    delete [] lut_array;

    return XIL_SUCCESS;
}


XilStatus
XilDeviceManagerComputeMMXBYTE::Rescale(XilOp*       op,
                                        unsigned     op_count,
                                        XilRoi*      roi,
                                        XilBoxList*  bl)
{
    ComputeInfoMMX  ci(op, op_count, roi, bl);

    IplLUT** lut_array  = (IplLUT**)op->getPreprocessData(this);

    while(ci.hasMoreInfo()) {
        if(ci.isStorageType(XIL_PIXEL_SEQUENTIAL)) {

            if(ci.createIplImages() == XIL_FAILURE) {
                MMX_MARK_BOX;
            }

            iplContrastStretch(&ci.mmxSrc1, &ci.mmxDst, lut_array, NULL);

            if(IPL_ERRCHK("Rescale", "Error in IPL Library")) {
                MMX_MARK_BOX;
            }

        } else {
            MMX_MARK_BOX;
        }

    }

    return ci.returnValue;

}

