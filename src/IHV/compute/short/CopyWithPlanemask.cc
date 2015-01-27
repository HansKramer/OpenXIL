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
//  File:	CopyWithPlanemask.cc
//  Project:	XIL
//  Revision:	1.8
//  Last Mod:	10:11:45, 03/10/00
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
#pragma ident	"@(#)CopyWithPlanemask.cc	1.8\t00/03/10  "

#include "XilDeviceManagerComputeSHORT.hh"
#include "ComputeInfo.hh"

XilStatus
XilDeviceManagerComputeSHORT::CopyWithPlanemask(XilOp*       op,
                                                unsigned     op_count,
                                                XilRoi*      roi,
                                                XilBoxList*  bl)
{
    ComputeInfoSHORT  ci(op, op_count, roi, bl);

    if(ci.isOK() == FALSE) {
        return ci.returnValue;
    }

    unsigned int* mask;
    op->getParam(1, (void**)&mask);

    while(ci.hasMoreInfo()) {
        unsigned int* m = mask;

        COMPUTE_GENERAL_1S_1D_W_BAND(Xil_signed16, Xil_signed16,

                                     *dest = (*dest & ~*m) | (*src1 & *m),

                                     *(dest+1) =
                                         (*(dest+1) & ~*(m+1)) | 
                                         (*(src1+1) &  *(m+1));
                                     *(dest+2) =
                                         (*(dest+2) & ~*(m+2)) | 
                                         (*(src1+2) &  *(m+2)),

                                     *dest =
                                         (*dest & ~*(m+band))|
                                         (*src1 & *(m+band))
            );
    }

    return ci.returnValue;
}

