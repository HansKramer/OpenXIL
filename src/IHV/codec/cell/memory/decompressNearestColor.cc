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
//  File:       decompressNearestColor.cc
//  Project:    XIL
//  Revision:   1.6
//  Last Mod:   10:16:00, 03/10/00
//
//  Description:
//
//    Molecule for Cell decompressNearestColor
//
//
//------------------------------------------------------------------------
//    COPYRIGHT
//------------------------------------------------------------------------
#pragma ident   "@(#)decompressNearestColor.cc	1.6\t00/03/10  "

#include <xil/xilGPI.hh>

#include "XilDeviceCompressionCell.hh"

#ifdef XIL_LITTLE_ENDIAN

static Xil_unsigned32 expandMask[16] = {
    0,
    0xff000000,
    0x00ff0000,
    0xffff0000,
    0x0000ff00,
    0xff00ff00,
    0x00ffff00,
    0xffffff00,
    0x000000ff,
    0xff0000ff,
    0x00ff00ff,
    0xffff00ff,
    0x0000ffff,
    0xff00ffff,
    0x00ffffff,
    0xffffffff
};

#define  BYTE0(x)  (((x)>>24)&0xff)
#define  BYTE1(x)  (((x)>>16)&0xff)
#define  BYTE2(x)  (((x)>>8)&0xff)
#define  BYTE3(x)  ((x)&0xff)

#else

static Xil_unsigned32 expandMask[16] = {
    0,
    0xff,
    0xff00,
    0xffff,
    0xff0000,
    0xff00ff,
    0xffff00,
    0xffffff,
    0xff000000,
    0xff0000ff,
    0xff00ff00,
    0xff00ffff,
    0xffff0000,
    0xffff00ff,
    0xffffff00,
    0xffffffff
};

#define  BYTE3(x)  (((x)>>24)&0xff)
#define  BYTE2(x)  (((x)>>16)&0xff)
#define  BYTE1(x)  (((x)>>8)&0xff)
#define  BYTE0(x)  ((x)&0xff)

#endif

XilStatus
XilDeviceCompressionCell::decompressNearestColor8(XilOp*       op,
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
    // Get the op_list for determining molecule components.
    // The cis will always be on op_list[op_count-1].
    //
    XilOp**   op_list = op->getOpList();

    //
    // Get the lookup table from the bottom op (xil_nearest_color)
    //
    XilLookup* lookup;
    op_list[0]->getParam(1, (XilObject**)&lookup);

    //
    // Make sure that the supplied lookup is identical to 
    // the one in the bytestream. If it isn't, we need to
    // let the atomic nearest_color function handle it.
    //
    if( !(lookup->isSameAs(&decompData.currentLookupVersion))) {
        return XIL_FAILURE;
    }

    //
    // Test whether the cis image and the dst image are the same
    // size and that the dst image has no ROI. This allows simpler
    // decompression, without the need for a temporary image.
    //
    if(cisFitsInDst(roi)) {

        //
        // We can decompress directly into the dst image.
        //
        if(decompressFrameNC8(&di) != XIL_SUCCESS) {
            return XIL_FAILURE;
        }

    } else {


        DecompressInfo tmp_di(&di);
        if(decompressFrameNC8(&tmp_di) != XIL_SUCCESS) {
          return XIL_FAILURE;
        }

        //
        // Copy the temp image to the dst, handling rois
        //
        di.copyRects(&tmp_di);
    }
    
    return XIL_SUCCESS;
}

//
// Process the frame to find the index of the color in the colormap
// which most closely matches the color in the bitstream.
//
XilStatus
XilDeviceCompressionCell::decompressFrameNC8(DecompressInfo* di)
{
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
    

    // compute the number of rows in columns of 4x4 cells in the image

    unsigned int max_row = src_y_size >> 2;
    unsigned int max_col = src_x_size >> 2;

    Xil_unsigned32* remapExp = decompData.remapExp;

    if( (((int)di->image_box_dataptr & 0x3) == 0) && 
        (di->image_ps == 1)) {
        //
        // If the storage for the first scanline is 4-byte aligned, then
        // all subsequent scanlines must be as well, since Cell image
        // dimensions must always be a multiple of 4.
        //
        unsigned int ss = di->image_ss;
        unsigned int int_ss = di->image_ss / 4;
        Xil_unsigned8* pDstCellRow = (Xil_unsigned8*)di->image_box_dataptr;
        for(int row=0; row<max_row; row++) {
            unsigned int* pDst = (unsigned int*)pDstCellRow;
            for (int col=0; col<max_col; col++) {
                Cell& cell = cell_frame[row][col];
                Xil_unsigned16 mask = cell.MASK();
                Xil_unsigned32 fgExp = remapExp[cell.C1()];
                Xil_unsigned32 bgExp = remapExp[cell.C0()];

                // +----+----+----+----+
                // | 15 | 14 | 13 | 12 |
                // +----+----+----+----+
                // | 11 | 10 |  9 |  8 |
                // +----+----+----+----+  Mapping of mask bits to 4x4 Cell
                // |  7 |  6 |  5 |  4 |
                // +----+----+----+----+
                // |  3 |  2 |  1 |  0 |
                // +----+----+----+----+

                unsigned int maskExp;
                maskExp = expandMask[(mask>>12) & 0xf];
                pDst[0] = (fgExp & maskExp) | (bgExp & ~maskExp);

                maskExp = expandMask[(mask>>8) & 0xf];
                pDst[int_ss] = (fgExp & maskExp) | (bgExp & ~maskExp);

                maskExp = expandMask[(mask>>4) & 0xf];
                pDst[2*int_ss] = (fgExp & maskExp) | (bgExp & ~maskExp);

                maskExp = expandMask[mask & 0xf];
                pDst[3*int_ss] = (fgExp & maskExp) | (bgExp & ~maskExp);
                pDst++;
            }
            pDstCellRow += 4 * ss;
        }
 
    } else {
        //
        // The case when the storage is not 4 byte aligned.
        // We have to write out a byte at a time.
        //
        unsigned int ps = di->image_ps;
        unsigned int ss = di->image_ss;
        Xil_unsigned8* pDstCellRow = (Xil_unsigned8*)di->image_box_dataptr;
        for(int row=0; row<max_row; row++) {
            Xil_unsigned8* pDstCellCol = pDstCellRow;
            for (int col=0; col<max_col; col++) {
                Xil_unsigned8* pDst = pDstCellCol;

                Cell& cell = cell_frame[row][col];
                Xil_unsigned16 mask = cell.MASK();
                Xil_unsigned32 fgExp = remapExp[cell.C1()];
                Xil_unsigned32 bgExp = remapExp[cell.C0()];

                unsigned int maskExp = expandMask[(mask>>12) & 0xf];
                unsigned int tmpdst = (fgExp & maskExp) | (bgExp & ~maskExp);
                pDst[0]    = BYTE3(tmpdst);
                pDst[ps]   = BYTE2(tmpdst);
                pDst[2*ps] = BYTE1(tmpdst);
                pDst[3*ps] = BYTE0(tmpdst);
                pDst += ss;

                maskExp = expandMask[(mask>>8) & 0xf];
                tmpdst = (fgExp & maskExp) | (bgExp & ~maskExp);
                pDst[0]    = BYTE3(tmpdst);
                pDst[ps]   = BYTE2(tmpdst);
                pDst[2*ps] = BYTE1(tmpdst);
                pDst[3*ps] = BYTE0(tmpdst);
                pDst += ss;

                maskExp = expandMask[(mask>>4) & 0xf];
                tmpdst = (fgExp & maskExp) | (bgExp & ~maskExp);
                pDst[0]    = BYTE3(tmpdst);
                pDst[ps]   = BYTE2(tmpdst);
                pDst[2*ps] = BYTE1(tmpdst);
                pDst[3*ps] = BYTE0(tmpdst);
                pDst += ss;

                maskExp = expandMask[mask & 0xf];
                tmpdst = (fgExp & maskExp) | (bgExp & ~maskExp);
                pDst[0]    = BYTE3(tmpdst);
                pDst[ps]   = BYTE2(tmpdst);
                pDst[2*ps] = BYTE1(tmpdst);
                pDst[3*ps] = BYTE0(tmpdst);

                pDstCellCol += ps*4;
            }
            pDstCellRow += ss*4;
        }
    }

    return XIL_SUCCESS;
}

