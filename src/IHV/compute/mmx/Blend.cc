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
//  File:       Blend.cc
//  Project:    XIL
//  Revision:   1.6
//  Last Mod:   10:13:43, 03/10/00
//
//  Description:
//
//    MMX Blend function 
//
//
//------------------------------------------------------------------------
//    COPYRIGHT
//------------------------------------------------------------------------
#pragma ident   "@(#)Blend.cc	1.6\t00/03/10  "

#include "XilDeviceManagerComputeMMXBYTE.hh"
#include "ComputeInfoMMX.hh"

XilStatus
XilDeviceManagerComputeMMXBYTE::Blenda8(XilOp*       op,
                                        unsigned     op_count,
                                        XilRoi*      roi,
                                        XilBoxList*  bl)
{
    ComputeInfoMMX  ci(op, op_count, roi, bl);

    while(ci.hasMoreInfo()) {
        if(ci.isStorageType(XIL_PIXEL_SEQUENTIAL)) {

            if(ci.createIplImages() == XIL_FAILURE) {
                MMX_MARK_BOX;
            }

            iplAlphaComposite(&ci.mmxSrc1, &ci.mmxSrc2, &ci.mmxDst, 
                              IPL_COMPOSITE_PLUS, &ci.mmxSrc3, 
                              NULL, NULL, FALSE, FALSE, NULL);

            if(IPL_ERRCHK("Blend", "Error in IPL Library")) {
                MMX_MARK_BOX;
            }

        } else {
            MMX_MARK_BOX;
        }

    }

    return ci.returnValue;

}


