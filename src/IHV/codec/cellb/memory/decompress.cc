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
//  File:   decompress.cc
//  Project:    XIL
//  Revision:   1.10
//  Last Mod:   10:15:35, 03/10/00
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
#pragma ident   "@(#)decompress.cc	1.10\t00/03/10  "


#include <xil/xilGPI.hh>

#include "XilDeviceCompressionCellB.hh"
#include "XiliUtils.hh"

XilStatus
XilDeviceCompressionCellB::decompress(XilOp*       op,           
                                      unsigned int op_count,
                                      XilRoi*      roi,
                                      XilBoxList*  bl)
{
    DecompressInfo di(op, op_count, roi, bl);
    if(! di.isOK()) {
        return XIL_FAILURE;
    }

    setInMolecule(op_count > 1);

    seekFlush(di.frame_number);

    //
    // Test whether the cis image and the dst image are the same
    // size and that the dst image has no ROI. This allows simpler
    // decompression, without the need for a temporary image.
    //
    if(cisFitsInDst(roi)) {
      //
      // Can decompress into frame directly
      //
      if(decompressFrame(&di) == XIL_FAILURE) {
          return XIL_FAILURE;
      }
   
    } else {

      //
      // Decompress into a temporary
      //
      DecompressInfo tmp_di(&di);
      if(! tmp_di.isOK()) {
          return XIL_FAILURE;
      }

      if(decompressFrame(&tmp_di) != XIL_SUCCESS) {
          return XIL_FAILURE;
      }

      //
      // Copy the temp image to the dst, handling rois
      //
      di.copyRects(&tmp_di);

    }

    return XIL_SUCCESS;
}

XilStatus
XilDeviceCompressionCellB::decompressFrame(DecompressInfo* di)
{

    Xil_unsigned8* buffer     = (Xil_unsigned8*)di->image_dataptr;
    unsigned int   dst_ss     = di->image_ss;
    unsigned int   dst_ps     = di->image_ps;
    int            width_cis  = di->image_width;
    int            height_cis = di->image_height;

    //
    // burnFrames is called to process the bytestream and update the contents
    // of a CellBFrame object, which is a two-dimensional array of CellB
    // objects in which history is maintained for interframe compression.
    //
    burnFrames(1);
    
    //
    // Get a pointer to the CellBFrame object.  We know the CellBFrame object
    // has been allocated since we just called burnFrames, but we pass the
    // width and height anyway.
    //
    CellBFrame* cellb_frame = decompData.getCellBFrame(width_cis, height_cis);
    if (cellb_frame == NULL) {
      // getCellBFrame has already reported an error
      return XIL_FAILURE;
    }

    // compute the number of rows in columns of 4x4 cells in the image
    unsigned int height = height_cis >> 2;
    unsigned int width  = width_cis  >> 2;

    for (int row = 0; row < height; row++) {

      // set up four pixel pointers for four adjacent rows
      Xil_unsigned8* row0 = buffer + dst_ss * row * 4;
      Xil_unsigned8* row1 = row0 + dst_ss;
      Xil_unsigned8* row2 = row1 + dst_ss;
      Xil_unsigned8* row3 = row2 + dst_ss;

      for (int col = 0; col < width; col++) {

        CellB& cellb = (*cellb_frame)[row][col];

        //
        // Lookup the Y, U, & V values for this cell
        //
        Xil_unsigned8 y0, y1, u, v;

        y0 = decompData.yytable[cellb.YY()] >> 8;
        y1 = decompData.yytable[cellb.YY()] & 0xFF;
        u = decompData.uvtable[cellb.UV()] >> 8;
        v = decompData.uvtable[cellb.UV()] & 0xFF;

        Xil_unsigned16 mask = cellb.MASK();
        
        // +----+----+----+----+
        // | 15 | 14 | 13 | 12 |
        // +----+----+----+----+
        // | 11 | 10 |  9 |  8 |    
        // +----+----+----+----+  Mapping of mask bits to 4x4 Cell
        // |  7 |  6 |  5 |  4 |
        // +----+----+----+----+
        // |  3 |  2 |  1 |  0 |
        // +----+----+----+----+
        
        for (int i = 0; i < 4; i++) {
          //
          // Pixels are in YUV byte order in a 24-bit XIL image.
          // On the first pass thru this loop, we do pixels 15,11,7,3.
          // On the next pass, we do 14,10,6,2.  etc.
          //
          row0[0] = (mask & 0x8000) ? y1 : y0;
          row1[0] = (mask & 0x0800) ? y1 : y0;
          row2[0] = (mask & 0x0080) ? y1 : y0;
          row3[0] = (mask & 0x0008) ? y1 : y0;

          mask <<= 1;

          row0[1] = row1[1] = row2[1] = row3[1]= u;
          row0[2] = row1[2] = row2[2] = row3[2]= v;

          row0 += dst_ps;
          row1 += dst_ps;
          row2 += dst_ps;
          row3 += dst_ps;
        }
      }
    }
    
    return XIL_SUCCESS;
}

