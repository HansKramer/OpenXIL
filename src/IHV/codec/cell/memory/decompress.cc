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
//  Revision:   1.8
//  Last Mod:   10:15:53, 03/10/00
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
#pragma ident   "@(#)decompress.cc	1.8\t00/03/10  "

#include <xil/xilGPI.hh>

#include "XilDeviceCompressionCell.hh"

XilStatus
XilDeviceCompressionCell::decompress(XilOp*       op,
                                     unsigned int op_count,
                                     XilRoi*      roi,
                                     XilBoxList*  bl)
{
    DecompressInfo di(op, op_count, roi, bl);
    if(! di.isOK()) {
        return XIL_FAILURE;
    }

    seek(di.frame_number);

    setInMolecule(op_count > 1);

    //
    // Test whether the cis image and the dst image are the same
    // size and that the dst image has no ROI. This allows simpler
    // decompression, without the need for a temporary image.
    //
    if(cisFitsInDst(roi)) {

        //
        // We can decompress directly into the dst image.
        //
        if(decompressFrame(&di) != XIL_SUCCESS) {
            return XIL_FAILURE;
        }

    } else {


        DecompressInfo tmp_di(&di);
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

//------------------------------------------------------------------------
//
//  Function:    decompressFrame
//
//  Description:
//    
//    Memory-to-memory Cell decompressor.
//    
//  Parameters:
//    
//    Destination must be a 3-banded, XIL_BYTE image.
//
//  Returns:
//
//    XIL_SUCCESS or XIL_FAILURE
//    
//  Side Effects:
//    
//  Notes:
//    
//  Deficiencies/ToDo:
//    
//    - Deficiency: the Cell foreground & background color indices are
//      not checked against the size of the current bytestream colormap.
//      Technically speaking, it is a bytestream error for either of these
//      indices to be out of range, but the worst thing that will happen
//      are some wrongly colored pixels, so checking for this would not
//      be worth the performance hit.
//    
//------------------------------------------------------------------------

XilStatus
XilDeviceCompressionCell::decompressFrame(DecompressInfo* di)
{
    Xil_unsigned8* buffer      = (Xil_unsigned8*)di->image_dataptr;
    unsigned int   pix_stride  = di->image_ps;
    unsigned int   scan_stride = di->image_ss;
    unsigned int   src_x_size  = di->image_width;
    unsigned int   src_y_size  = di->image_height;

    //
    // Set a flag for those molecules which rely on the frame buffer contents,
    // such that when processing skip codes they redraw from the history
    // buffer.
    //
    decompData.redrawNeeded  = 1;

    // ensure the most recent header that we've read is really the 
    // one we want

    if (decompData.headerFrameNumber != cbm->getRFrameId() ||
        decompData.bp == NULL) {
      if (decompressHeader() == XIL_FAILURE) {
        XIL_OBJ_ERROR(system_state, 
                      XIL_ERROR_SYSTEM, 
                      "di-311", 
                      FALSE, 
                      getCis());
        return XIL_FAILURE;
      }
    }
    
    // burnFrames is called to process the bytestream and update the 
    // contents of a CellFrame object, which is a two-dimensional array 
    // of Cell objects in which history is maintained for interframe 
    // compression.
    burnFrames(1);
    
    // Now the CellFrame object is used to produce the destination image.
    // The CellFrame object pointed to by decompData.cellFrame is constructed
    // in deriveOutputType().
    CellFrame& cell_frame = (*decompData.cellFrame);
    
    // since we built the colormap object, we know that...
    //   - the colormap values are in RGB order
    //   - bytePerEntry is 3
    //   - the offset is 0
    Xil_unsigned8* cmap = (Xil_unsigned8 *)((*(decompData.colormap)).getData());
    
    // compute the number of rows in columns of 4x4 cells in the image

    unsigned int max_row = src_y_size >> 2;
    unsigned int max_col = src_x_size >> 2;

    for (int row = 0; row < max_row; row++) {
      // set up four pixel pointers for four adjacent rows

      Xil_unsigned8* y0 = buffer + scan_stride * row * 4;
      Xil_unsigned8* y1 = y0 + scan_stride;
      Xil_unsigned8* y2 = y1 + scan_stride;
      Xil_unsigned8* y3 = y2 + scan_stride;

      for (int col = 0; col < max_col; col++) {
        Cell& cell = cell_frame[row][col];

        // lookup the RGB values for both colors in this cell
        Xil_unsigned8* cmap_entry = cmap + cell.C0() * 3;
        Xil_unsigned8 r0 = cmap_entry[0];
        Xil_unsigned8 g0 = cmap_entry[1];
        Xil_unsigned8 b0 = cmap_entry[2];

        cmap_entry = cmap + cell.C1() * 3;

        Xil_unsigned8 r1 = cmap_entry[0];
        Xil_unsigned8 g1 = cmap_entry[1];
        Xil_unsigned8 b1 = cmap_entry[2];

        Xil_unsigned16 mask = cell.MASK();
        
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
          // pixels are in BGR byte order in a 24-bit XIL image
          if (mask & 0x8000) {
            y0[0] = b1; y0[1] = g1; y0[2] = r1;
          } else {
            y0[0] = b0; y0[1] = g0; y0[2] = r0;
          }

          if (mask & 0x0800) {
            y1[0] = b1; y1[1] = g1; y1[2] = r1;
          } else {
            y1[0] = b0; y1[1] = g0; y1[2] = r0;
          }

          if (mask & 0x0080) {
            y2[0] = b1; y2[1] = g1; y2[2] = r1;
          } else {
            y2[0] = b0; y2[1] = g0; y2[2] = r0;
          }

          if (mask & 0x0008) {
            y3[0] = b1; y3[1] = g1; y3[2] = r1;
          } else {
            y3[0] = b0; y3[1] = g0; y3[2] = r0;
          }

          y0 += pix_stride;
          y1 += pix_stride;
          y2 += pix_stride;
          y3 += pix_stride;

          mask <<= 1;
        }
      }
    }
    
    return XIL_SUCCESS;
}

