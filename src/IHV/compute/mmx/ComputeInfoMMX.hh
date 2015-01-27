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
//  File:       ComputeInfoMMX.hh
//  Project:    XIL
//  Revision:   1.9
//  Last Mod:   10:22:30, 03/10/00
//
//  Description:
//
//    Prototype for the ComputeInfo calls for the MMX pipeline
//
//
//------------------------------------------------------------------------
//    COPYRIGHT
//------------------------------------------------------------------------
#pragma ident   "@(#)ComputeInfoMMX.hh	1.9\t00/03/10  "

//
//  C++ Includes
//
#include <xil/xilGPI.hh>
#include "ComputeInfo.hh"


//
// MMX includes
//
#ifndef _WINDOWS
#define IPL_WINDOWS
#define __cdecl
#endif

#include <string.h>
#include "ipl.h"

#define MMX_MEMCHK(ptr_arg) \
    if(ptr_arg == NULL) {       \
        XilImage* src_memchk = op->getSrcImage(); \
        XilImage* dst_memchk = op->getDstImage(); \
        XilSystemState* state = (src_memchk) ? \
                src_memchk->getSystemState() :  \
                ((dst_memchk) ? dst_memchk->getSystemState() : NULL); \
        XIL_ERROR(state, XIL_ERROR_RESOURCE, "di-1", TRUE); \
        return XIL_FAILURE;      \
    }

#define MMX_MARK_BOX \
   if(bl->markAsFailed() == XIL_FAILURE) { \
       return XIL_FAILURE; \
   } else { \
       continue; \
   }

class ComputeInfoMMX : public ComputeInfoGENERAL {
public:

    //
    //  Constructor/Destructor
    //
    ComputeInfoMMX(XilOp*       op,
                   unsigned int op_count,
                   XilRoi*      roi,
                   XilBoxList*  bl);

    ~ComputeInfoMMX();

    XilStatus createIplImages(Xil_boolean failOnMisaligned=TRUE);

    //
    // Get access to boxes
    //
    XilBox* getSrc1Box();
    XilBox* getDestBox();

    //
    // Get dest box tag (for convolve)
    //
    XilBoxAreaType getBoxTag();

    //
    // Make the MMX images and ROIs data members (they're small).
    // Since the ComputeInfo object is stack-allocated, this
    // should speed up launching the compute routine.
    //
    IplImage mmxSrc1;
    IplImage mmxSrc2;
    IplImage mmxSrc3;
    IplImage mmxDst;

    IplROI   mmxSrc1Roi;
    IplROI   mmxSrc2Roi;
    IplROI   mmxSrc3Roi;
    IplROI   mmxDstRoi;

};

