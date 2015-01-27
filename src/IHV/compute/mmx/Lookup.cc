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
//  File:       Lookup.cc
//  Project:    XIL
//  Revision:   1.7
//  Last Mod:   10:13:42, 03/10/00
//
//  Description:
//
//    Lookup function. This version is the 1 to N band lookup.
//
//------------------------------------------------------------------------
//    COPYRIGHT
//------------------------------------------------------------------------
#pragma ident   "@(#)Lookup.cc	1.7\t00/03/10  "

#include "XilDeviceManagerComputeMMXBYTE.hh"
#include "ComputeInfoMMX.hh"


//
// Use the preprocess routine to create the IplLUT objects
// for the iplContrastStretch (lookup) operation
// 
XilStatus
XilDeviceManagerComputeMMXBYTE::Lookup8Preprocess(XilOp*       op,
                                                  unsigned     ,
                                                  XilRoi*      ,
                                                  void**       compute_data,
                                                  unsigned int*  )
{
    XilImage*        dst = op->getDstImage(1);

    //
    // Get the XilLookup object and its parameters
    //
    XilLookupSingle* lookup;
    op->getParam(1, (XilObject**) &lookup);
    unsigned int   num_output_bands = lookup->getOutputNBands();
    int            offset           = lookup->getOffset();
    unsigned int   num_entries      = lookup->getNumEntries();
    Xil_unsigned8* lut_data         = (Xil_unsigned8 *) lookup->getData();

    //
    // Fill in IplLUT structs (one per band) with the values 
    // from the XilLookup object. Make a complete table, so
    // that simple indexing can be used.
    //
    IplLUT** lut_array = new IplLUT*[num_output_bands];
    MMX_MEMCHK(lut_array);

    //
    // Create a single key array to be used for all bands
    //
    int*     key_array = new int[257];
    MMX_MEMCHK(key_array);
    for(unsigned int i=0; i<257; i++) {
        key_array[i] = i;
    }

    for(unsigned int band=0; band<num_output_bands; band++) {
        IplLUT* lut          = new IplLUT;
        MMX_MEMCHK(lut);
        lut_array[band]      = lut;
        lut->key             = key_array;
        lut->value           = new int[257];
        MMX_MEMCHK(lut->value);
        lut->num             = 257;
        lut->interpolateType = IPL_LUT_LOOKUP;
        lut->factor          = NULL;

        //
        // Fill any part below the offset with the first entry
        //
        for(i=0; i<offset; i++) {
            lut->value[i] = lut_data[band];
        }

        //
        // The main part of the table
        //
        int elem = band;
        for(i=offset; i<offset+num_entries; i++) {
            lut->value[i] = lut_data[elem];
            elem         += num_output_bands;
        }

        //
        // Fill remainder with last entry
        //
        elem -= num_output_bands; 
        for(i=offset+num_entries; i<=256; i++) {
            lut->value[i] = lut_data[elem];
        }
    }


    *compute_data = (void*)lut_array;;

    return XIL_SUCCESS;
}

//
// Cleanup the IplLUT objects after the operation
//
XilStatus
XilDeviceManagerComputeMMXBYTE::Lookup8Postprocess(XilOp*        op,
                                                    void*        compute_data)
{
    
    XilLookupSingle* lookup;
    op->getParam(1, (XilObject**) &lookup);
    unsigned int   num_output_bands = lookup->getOutputNBands();

    IplLUT** lut_array = (IplLUT**) compute_data;

    //
    // Since the same key array is used for all bands,
    // make sure it only gets deleted once
    //
    delete [] lut_array[0]->key;

    for(unsigned int band=0; band<num_output_bands; band++) {
        delete [] lut_array[band]->value;
        delete lut_array[band];
    }
    
    delete [] lut_array;

    return XIL_SUCCESS;
}


XilStatus
XilDeviceManagerComputeMMXBYTE::Lookup8(XilOp*       op,
                                         unsigned     op_count,
                                         XilRoi*      roi,
                                         XilBoxList*  bl)
{
    ComputeInfoMMX  ci(op, op_count, roi, bl);

    IplLUT** lut_array = (IplLUT**)op->getPreprocessData(this);

    XilLookupSingle* lookup;
    op->getParam(1, (XilObject**) &lookup);
    unsigned int     num_output_bands = lookup->getOutputNBands();

    while(ci.hasMoreInfo()) {
        if(ci.isStorageType(XIL_PIXEL_SEQUENTIAL)) {

            if(ci.createIplImages() == XIL_FAILURE) {
                MMX_MARK_BOX;
            }

            for(unsigned int band=1; band<=num_output_bands; band++) {

                //
                // Create a channel of interest (COI) in the
                // ROI for the destination image.
                // Ipl counts bands from 1, not zero.
                //
                ci.mmxDst.roi->coi = band;
                iplContrastStretch(&ci.mmxSrc1, &ci.mmxDst, 
                                   lut_array + band - 1,
                                   NULL);

                if(IPL_ERRCHK("Lookup8", "Error in IPL Library")) {
                    MMX_MARK_BOX;
                }
            }

        } else {
            MMX_MARK_BOX;
        }

    }

    return ci.returnValue;

}

