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
//  Revision:   1.1
//  Last Mod:   14:34:05, 07/09/97
//
//  Description:
//
//    SetValue function. Needs to work on a band at a time
//    since Ipl doesn't allow a vector of fill values to be used.
//
//------------------------------------------------------------------------
//    COPYRIGHT
//------------------------------------------------------------------------
#pragma ident   "@(#)Lookup.cc	1.1\t97/07/09  "

#include "XilDeviceManagerComputeMMXBYTE.hh"
#include "ComputeInfoMMX.hh"


XilStatus
XilDeviceManagerComputeMMXBYTE::SetValue(XilOp*       op,
                                         unsigned     op_count,
                                         XilRoi*      roi,
                                         XilBoxList*  bl)
{
    ComputeInfoMMX  ci(op, op_count, roi, bl);

    //
    // Get the values to set from the op.
    // They have already been converted from floats to uchars
    //
    unsigned int   nbands = op->getDstImage(1)->getNumBands();
    Xil_unsigned8* val;
    op->getParam(1, (void**)&val);

    //
    // Test whether the values to be set are the same for all bands.
    // This will frequently be the case, e.g. when clearing to zero.
    //
    Xil_boolean   all_same   = TRUE;
    Xil_unsigned8 first_band = val[0];
    for(unsigned int band=1; band<nbands; band++) {
        if(val[band] != first_band) {
            all_same = FALSE;
            break;
        }
    }

    while(ci.hasMoreInfo()) {
        if(ci.isStorageType(XIL_PIXEL_SEQUENTIAL)) {

            if(ci.createIplImages() == XIL_FAILURE) {
                MMX_MARK_BOX;
            }

            if(all_same) {
                //
                // Set all bands in one call
                //
                iplSet(&ci.mmxDst, (int)val[0], NULL) ;

                if(IPL_ERRCHK("SetValue", "Error in IPL Library")) {
                    MMX_MARK_BOX;
                }
            } else {
                //
                // Set one band at a time.
                //
                for(band=1; band<=nbands; band++) {

                    //
                    // Create a channel of interest (COI) in the
                    // ROI for the destination image.
                    // Ipl counts bands from 1, not zero.
                    //
                    ci.mmxDst.roi->coi = band;
                    iplSet(&ci.mmxDst, (int)val[band-1], NULL) ;

                    if(IPL_ERRCHK("SetValue", "Error in IPL Library")) {
                        MMX_MARK_BOX;
                    }
                }
            }

        } else {
            MMX_MARK_BOX;
        }

    }

    return ci.returnValue;

}

