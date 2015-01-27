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
//  File:   compress.cc
//  Project:    XIL
//  Revision:   1.6
//  Last Mod:   10:15:32, 03/10/00
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
//  COPYRIGHT
//------------------------------------------------------------------------
#pragma ident   "@(#)compress.cc	1.6\t00/03/10  "

#include <xil/xilGPI.hh>

#include "XilDeviceCompressionCellB.hh"
#include "CellBDefines.hh"

XilStatus
XilDeviceCompressionCellB::compress(XilOp*        op,
                                    unsigned int  op_count,
                                    XilRoi*       roi,
                                    XilBoxList*   bl)
{
    CompressInfo ci(op, op_count, roi, bl);
    if(! ci.isOK()) {
        return XIL_FAILURE;
    }

    setInMolecule(op_count > 1);

    //
    // Test whether the cis and the src image are the same size.
    // Roi test done in the core.
    //
    if(ci.image_box_width  != ci.cis_width ||
       ci.image_box_height != ci.cis_height) {
        XIL_ERROR (ci.system_state, XIL_ERROR_USER, "di-22", TRUE);
        return XIL_FAILURE;
    }

    //
    //  Verify the Image width and height is divisible by 4.
    //
    if((ci.cis_width % 4) != 0) {
        XIL_ERROR(system_state, XIL_ERROR_USER, "di-253", TRUE);
        return XIL_FAILURE;
    }

    if((ci.cis_height % 4) != 0) {
        XIL_ERROR(system_state, XIL_ERROR_USER, "di-254", TRUE);
        return XIL_FAILURE;
    }        

    //
    //  XilImage type verification is done above and in XilCis.
    //  But, I have to check and set up compData if it hasn't been
    //  setup properly on a previous call.
    //
    if(compData.initialized == FALSE) {
        if(compData.initialize(ci.cis_width, ci.cis_height) == XIL_FAILURE) {
            return XIL_FAILURE;
        }
    }

    if(!compData.atomicHistoryValid()) {
        compData.initializeAtomicHistory();
    }
  
    //    
    // get space to compress into
    //
    Xil_unsigned8* buffer          = cbm->nextBufferSpace();
    Xil_unsigned8* starting_buffer = buffer;

    unsigned int   skip_count      = 0;
    int            index           = 0;
    unsigned int   cell_xstride   = ci.image_ps * 4;
    unsigned int   cell_ystride   = ci.image_ss * 4;

    Xil_unsigned8* ycell = (Xil_unsigned8*)ci.image_box_dataptr;
    for(int y=0; y<ci.cis_height; y+=4) {

        Xil_unsigned8* xcell = ycell;

        for(int x=0; x<ci.cis_width; x+=4) {
            int cell = inner_loop(xcell, ci.image_ps, ci.image_ss);

            if(skipCell(cell, index)) {
                skip_count++;
            } else {
                //
                // Flush skips
                // 
                while (skip_count > CELLB_MAX_SKIP) {
                  *buffer++ = SKIPCODE + CELLB_MAX_SKIP - 1;
                  skip_count -= CELLB_MAX_SKIP;
                }

                if (skip_count > 0) {
                  *buffer++ = SKIPCODE + skip_count - 1;
                  skip_count = 0;
                }

                //
                // Add cell
                //
                buffer[0] = cell >> 24;
                buffer[1] = cell >> 16;
                buffer[2] = cell >> 8;
                buffer[3] = cell;
                buffer += 4;

            }

            xcell += cell_xstride;
            index++;
        }
        ycell += cell_ystride;
    }

    //
    // Flush skips
    // 
    while (skip_count > CELLB_MAX_SKIP) {
      *buffer++ = SKIPCODE + CELLB_MAX_SKIP - 1;
      skip_count -= CELLB_MAX_SKIP;
    }

    if (skip_count > 0) {
      *buffer++ = SKIPCODE + skip_count - 1;
      skip_count = 0;
    }

    //
    // Tell the cis buffer manager how much space we used
    //
    cbm->doneBufferSpace(buffer - starting_buffer);

    return XIL_SUCCESS;
}

