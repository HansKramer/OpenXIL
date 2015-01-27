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
//  File:   CellBParser.cc
//  Project:    XIL
//  Revision:   1.2
//  Last Mod:   10:15:30, 03/10/00
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
#pragma ident   "@(#)CellBParser.cc	1.2\t00/03/10  "

#include "CellBParser.hh"

CellBParser::CellBParser(XilDeviceCompressionCellB* dc,
                         void*                      dst,
                         Xil_boolean                history_buffer_valid,
                         unsigned long              dst_cellb_stride,
                         unsigned long              stride)
{
    isOKFlag = FALSE;

    CellBDecompressorData *decompData = dc->getCellBDecompressorData();

    XilImageFormat* cis_outtype = dc->getOutputType();

    int cis_width  = cis_outtype->getWidth();
    int cis_height = cis_outtype->getHeight();
    width  = cis_width  >> 2;
    height = cis_height >> 2;

    cellb_frame = decompData->getCellBFrame(cis_width, cis_height);

    cur_cellb = (*cellb_frame)[0];

    cbm = dc->getCisBufferManager();
    bp = cbm->nextFrame();
    if (bp == NULL) {
      // No data to decompress: let atomic report it
      return;
    }

    if (cellb_frame == NULL) {
      //getCellBFrame has already reported an error
      return;
    }

    this->dest = (Xil_unsigned8*)dst;
    bp = (Xil_unsigned8*)cbm->nextFrame();
    
    // We trust what is in the history buffer if it is valid or if we
    // have been instructed to by setting ignoreHistory == TRUE
    history_buffer = history_buffer_valid || decompData->ignoreHistory;
    cellb_stride = dst_cellb_stride;
    this->stride3 = stride;
    done = FALSE;
    row = 0;
    col = 0;
    count = 0; // signal that we are not in a skip code

    isOKFlag = TRUE;
}

