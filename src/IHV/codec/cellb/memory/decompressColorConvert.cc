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
//  File:       decompressColorConvert.cc
//  Project:    XIL
//  Revision:   1.2
//  Last Mod:   10:15:38, 03/10/00
//
//  Description:
//
//    CellB molecules
//      DecompressDither
//      DecompressColorConvert
//
//
//------------------------------------------------------------------------
//    COPYRIGHT
//------------------------------------------------------------------------
#pragma ident   "@(#)decompressColorConvert.cc	1.2\t00/03/10  "

#include <xil/xilGPI.hh>

#include "XilDeviceCompressionCellB.hh"
#include "XiliUtils.hh"

XilStatus
XilDeviceCompressionCellB::decompressColorConvert(XilOp*       op,           
                                                  unsigned int op_count,
                                                  XilRoi*      roi,
                                                  XilBoxList*  bl)
{
    DecompressInfo di(op, op_count, roi, bl);
    if(! di.isOK()) {
        return XIL_FAILURE;
    }

    setInMolecule(TRUE);

    seekFlush(di.frame_number);

    di.doColorConvert = TRUE;

    //
    // Get the op_list for determining molecule components.
    // The cis will always be on op_list[op_count-1].
    //
    XilOp**   op_list = op->getOpList();

    //
    // Verify that the parameters are acceptable. If not,
    // fail back to atomic mode.
    //
    if(! validDecompressColorConvert(op_list[0]->getSrcImage(),
                                     op_list[0]->getDstImage())) {
        return XIL_FAILURE;
    }

    //
    // Get the Ycc2RgbConverter object (creates it on first call)
    // This will initialize the tables which speed the color conversion.
    //
    colorCvt =
    ((XilDeviceManagerCompressionCellB*)getDeviceManager())->getColorConverter(
);
    if(colorCvt == NULL) {
        return XIL_FAILURE;
    }

    //
    // Set the generic object pointer in DecompressInfo so
    // that this object can be passed around.
    // (Sort of a hack, but it works).
    //
    di.objectPtr1 = colorCvt;
    
    //
    // Test whether the cis image and the dst image are the same
    // size and that the dst image has no ROI. This allows simpler
    // decompression, without the need for a temporary image.
    //
    if(cisFitsInDst(roi)) {
      //
      // Can decompress into frame directly
      //
      if(decompressCCFrame(&di) == XIL_FAILURE) {
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

      if(decompressCCFrame(&tmp_di) != XIL_SUCCESS) {
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
XilDeviceCompressionCellB::decompressCCFrame(DecompressInfo* di)
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
        return XIL_FAILURE;
    }

    //
    // Compute the number of rows and columns of 4x4 cells in the image
    //
    unsigned int height = height_cis >> 2;
    unsigned int width  = width_cis  >> 2;

    Xil_unsigned8* dst_row = buffer;
    for (int row = 0; row < height; row++) {

        Xil_unsigned8* dst_col = dst_row;

        for (int col = 0; col < width; col++) {

            CellB& cellb = (*cellb_frame)[row][col];

            //
            // Lookup the Y, U, & V values for this cell
            //
            Xil_unsigned8 y0 = decompData.yytable[cellb.YY()] >> 8;
            Xil_unsigned8 y1 = decompData.yytable[cellb.YY()] & 0xFF;
            Xil_unsigned8 u  = decompData.uvtable[cellb.UV()] >> 8;
            Xil_unsigned8 v  = decompData.uvtable[cellb.UV()] & 0xFF;

            Xil_unsigned16 mask = cellb.MASK();
          
            //
            // Call the color conversion function
            // to convert this cell.
            //
            colorCvt->cvtCellB(y0, y1, u, v, mask, dst_col, dst_ps, dst_ss);

            dst_col += dst_ps * 4;
        }

        dst_row += dst_ss * 4;
    }
    
    return XIL_SUCCESS;
}

