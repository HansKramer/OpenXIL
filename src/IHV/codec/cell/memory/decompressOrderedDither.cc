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
//  File:       decompressOrderedDither.cc
//  Project:    XIL
//  Revision:   1.2
//  Last Mod:   10:16:03, 03/10/00
//
//  Description:
//
//    Molecule for Cell decompressOrderedDither with optional zoom
//
//
//------------------------------------------------------------------------
//    COPYRIGHT
//------------------------------------------------------------------------
#pragma ident   "@(#)decompressOrderedDither.cc	1.2\t00/03/10  "

#include <xil/xilGPI.hh>
#include "XilDeviceCompressionCell.hh"

static Xil_unsigned32 expandMask[16] = {
  0x00000000,
  0x000000ff,
  0x0000ff00,
  0x0000ffff,
  0x00ff0000,
  0x00ff00ff,
  0x00ffff00,
  0x00ffffff,
  0xff000000,
  0xff0000ff,
  0xff00ff00,
  0xff00ffff,
  0xffff0000,
  0xffff00ff,
  0xffffff00,
  0xffffffff
};

static Xil_unsigned32 expandMaskZoom[4] = {
  0x00000000,
  0x0000ffff,
  0xffff0000,
  0xffffffff
};

XilStatus
XilDeviceCompressionCell::decompressOrderedDither8(XilOp*       op,
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
    // If there are three atoms in the molecule then
    // we must need to do the zoom
    //
    Xil_boolean doZoom = (op_count == 3);

    //
    // Get the colorcube and dithermask from the bottom op (xil_nearest_color)
    //
    XilLookupColorcube*     cube;
    XilDitherMask*          dmask;
    op_list[0]->getParam(1, (XilObject**)&cube);
    op_list[0]->getParam(2, (XilObject**)&dmask);

    //
    // Validate that colorcube and dmask meet reqts
    //
    if(cube->getOutputNBands() != 3 ||
       dmask->getWidth()       != 4 ||
       dmask->getHeight()      != 4 ||
       dmask->getData()        == NULL) {
      return XIL_FAILURE;
    }

    //
    // Get the scale factors for the optional scale8nearest op
    //
    float xscale, yscale;
    if(doZoom) {
        op_list[1]->getParam(1, &xscale);
        op_list[1]->getParam(2, &yscale);
        //
        // Bail out and do atomically if not an exact 2X zoom
        // or the dst image is not exactly 2X sized
        //
        if((xscale != 2.0)                          || 
           (yscale != 2.0)                          ||
           (di.cis_width*2  != di.image_box_width)  ||
           (di.cis_height*2 != di.image_box_height) ) {
            return XIL_FAILURE;
        }
    }

    if(((int)di.image_box_dataptr & 0x3) || di.image_ps != 1) {
        //
        // If dst image storage is not 4-byte aligned or the pixel_stride is
        // something other than 1, then bail out and do atomic operations.
        //
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
        if(decompressFrameOD8(&di, cube, dmask, doZoom) != XIL_SUCCESS) {
            return XIL_FAILURE;
        }

    } else {


        DecompressInfo tmp_di(&di);
        if(decompressFrameOD8(&tmp_di, cube, dmask, doZoom) != XIL_SUCCESS) {
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
// Process the frame to dither it to the supplied colorcube.
//
XilStatus
XilDeviceCompressionCell::decompressFrameOD8(DecompressInfo*     di,
                                             XilLookupColorcube* cube,
                                             XilDitherMask*      dmask,
                                             Xil_boolean         doZoom)
{
    //
    // Retrieve (or create) the dither table
    //
    Xil_unsigned32* dith_tbl = createDitherTable(cube, dmask);
    if(dith_tbl == NULL) {
        return XIL_FAILURE;
    }

    //
    // Set a flag for those molecules which rely on the frame buffer contents,
    // such that when processing skip codes they redraw from the history
    // buffer.
    //
    decompData.redrawNeeded  = 1;

    //
    // Ensure the most recent header that we've read is really the 
    // one we want.
    //
    if(decompData.headerFrameNumber != cbm->getRFrameId() ||
        decompData.bp == NULL) {
      if(decompressHeader() == XIL_FAILURE) {
        XIL_OBJ_ERROR(system_state, XIL_ERROR_SYSTEM, "di-311", FALSE, cis);
        return XIL_FAILURE;
      }
    }
    
    //
    // burnFrames is called to process the bytestream and update the 
    // contents of a CellFrame object, which is a two-dimensional array 
    // of Cell objects in which history is maintained for interframe 
    // compression.
    //
    burnFrames(1);
    
    //
    // Now the CellFrame object is used to produce the destination image.
    // The CellFrame object pointed to by decompData.cellFrame is constructed
    // in deriveOutputType().
    //
    CellFrame& cell_frame = (*decompData.cellFrame);
    
    int  max_row = cell_frame.getFrameHeight();
    int  max_col = cell_frame.getFrameWidth();
      
    //
    // The image storage is 4-byte aligned & the pixel_stride is 1.
    // We also know that all subsequent scanlines will be 4-byte aligned
    // since Cell image dimensions must always be a multiple of 4.
    //
    // Stride in ints
    //
    unsigned int ss = di->image_ss / 4;

    if(!doZoom) {
        //
        // Non-Zoom case
        //
        unsigned int* pCellRow = (unsigned int*)di->image_box_dataptr;
        for(int row=0; row<max_row; row++) {
            unsigned int* dest = pCellRow;
            for(int col=0; col<max_col; col++) {
                Cell& cell = cell_frame[row][col];
                Xil_unsigned16 mask = cell.MASK();
                
                //
                // The foreground color is C1, the background color is C0.
                // Recall that dith_tbl is conceptually an array of 
                // 256 x 16 bytes. Init two pointers to access the 
                // appropriate entries in the table
                // for the foreground and background colors.
                //
                Xil_unsigned32* dith_fg = dith_tbl + (cell.C1() << 2);
                Xil_unsigned32* dith_bg = dith_tbl + (cell.C0() << 2);
                
                // +----+----+----+----+
                // | 15 | 14 | 13 | 12 |
                // +----+----+----+----+
                // | 11 | 10 |  9 |  8 |        
                // +----+----+----+----+  Mapping of mask bits to 4x4 Cell
                // |  7 |  6 |  5 |  4 |
                // +----+----+----+----+
                // |  3 |  2 |  1 |  0 |
                // +----+----+----+----+
                
                Xil_unsigned32 maskExp;

                maskExp    = expandMask[(mask >> 12) & 0xf];
                dest[0]    = (dith_fg[0] & maskExp) | (dith_bg[0] & ~maskExp);
                
                maskExp    = expandMask[(mask >> 8) & 0xf];
                dest[ss]   = (dith_fg[1] & maskExp) | (dith_bg[1] & ~maskExp);
                
                maskExp    = expandMask[(mask >> 4) & 0xf];
                dest[2*ss] = (dith_fg[2] & maskExp) | (dith_bg[2] & ~maskExp);
                
                maskExp    = expandMask[mask & 0xf];
                dest[3*ss] = (dith_fg[3] & maskExp) | (dith_bg[3] & ~maskExp);
                dest++;
            }
            pCellRow += 4*ss;
        }
    } else {
        //
        // Zoom by 2 case
        //
        unsigned int* pCellRow = (unsigned int*)di->image_box_dataptr;
        for(int row=0; row<max_row; row++) {
            unsigned int* dest = pCellRow;
            for(int col=0; col<max_col; col++) {
                Cell& cell = cell_frame[row][col];
                Xil_unsigned16 mask = cell.MASK();
                  
                //
                // See comments above in the non-zoom loop; they apply here, too.
                //
                Xil_unsigned32* dith_fg = dith_tbl + (cell.C1() << 2);
                Xil_unsigned32* dith_bg = dith_tbl + (cell.C0() << 2);
                  
                Xil_unsigned32 maskExp;

                maskExp      = expandMaskZoom[(mask >> 14) & 0x3];
                dest[0]      = (dith_fg[0] & maskExp) | (dith_bg[0] & ~maskExp);
                dest[ss]     = (dith_fg[1] & maskExp) | (dith_bg[1] & ~maskExp);

                maskExp      = expandMaskZoom[(mask >> 12) & 0x3];
                dest[1]      = (dith_fg[0] & maskExp) | (dith_bg[0] & ~maskExp);
                dest[ss+1]   = (dith_fg[1] & maskExp) | (dith_bg[1] & ~maskExp);
                
                maskExp      = expandMaskZoom[(mask >> 10) & 0x3];
                dest[ss*2]   = (dith_fg[2] & maskExp) | (dith_bg[2] & ~maskExp);
                dest[ss*3]   = (dith_fg[3] & maskExp) | (dith_bg[3] & ~maskExp);

                maskExp      = expandMaskZoom[(mask >>  8) & 0x3];
                dest[ss*2+1] = (dith_fg[2] & maskExp) | (dith_bg[2] & ~maskExp);
                dest[ss*3+1] = (dith_fg[3] & maskExp) | (dith_bg[3] & ~maskExp);
                
                maskExp      = expandMaskZoom[(mask >>  6) & 0x3];
                dest[ss*4]   = (dith_fg[0] & maskExp) | (dith_bg[0] & ~maskExp);
                dest[ss*5]   = (dith_fg[1] & maskExp) | (dith_bg[1] & ~maskExp);
                
                maskExp      = expandMaskZoom[(mask >>  4) & 0x3];
                dest[ss*4+1] = (dith_fg[0] & maskExp) | (dith_bg[0] & ~maskExp);
                dest[ss*5+1] = (dith_fg[1] & maskExp) | (dith_bg[1] & ~maskExp);
                
                maskExp      = expandMaskZoom[(mask >>  2) & 0x3];
                dest[ss*6]   = (dith_fg[2] & maskExp) | (dith_bg[2] & ~maskExp);
                dest[ss*7]   = (dith_fg[3] & maskExp) | (dith_bg[3] & ~maskExp);

                maskExp      = expandMaskZoom[mask & 0x3];
                dest[ss*6+1] = (dith_fg[2] & maskExp) | (dith_bg[2] & ~maskExp);
                dest[ss*7+1] = (dith_fg[3] & maskExp) | (dith_bg[3] & ~maskExp);

                dest += 2;
            }
            pCellRow += ss*8;
        }
    }
    
    return XIL_SUCCESS;
}

//
// BUILDING THE DITHER TABLE
//
// Since the colorcube & dithermask are likely to change infrequently
// if at all, but the bytestream colormap may change as often as
// every frame, we want to optimize for the latter.  For a given
// colorcube & dithermask, we'll build three tables dith_tbl_{r,g,b}
// containing the red, green, & blue contributions to the index in the
// colorcube.  Then the colorcube index for a new color is simply
//
//  dith_tbl[i] = offset + dith_tbl_r[r] + dith_tbl_g[g] + dith_tbl_b[b]
//
// where r, g, & b are the 0..255 color values for color i.
//
// As a further optimization, we'll build the offset into dith_tbl_r
// to avoid an extra addition.  Note that each element of the dither
// tables as depicted in the above pseudocode is actually 16 bytes, one
// byte for each of the values in the 4x4 dithermask.
//
Xil_unsigned32*
XilDeviceCompressionCell::createDitherTable(XilLookupColorcube* cube,
                                            XilDitherMask*      dmask)
{
    Xil_unsigned32* dith_tbl_r;
    Xil_unsigned32* dith_tbl_g;
    Xil_unsigned32* dith_tbl_b;
    Xil_unsigned32* dith_tbl;

    //
    // If the colorcube and dithermask are not the same ones we used to
    // compute the dither tables, then we need to recompute them.
    // Otherwise, we'll just use the same values we had before.
    //
    Xil_boolean new_ccube_or_dmask = 
        ! ( cube->isSameAs(&decompData.currentCubeVersion) &&
            dmask->isSameAs(&decompData.currentDmaskVersion) );

    if(new_ccube_or_dmask) {
        //
        // Update the object version info
        //
        cube->getVersion(&decompData.currentCubeVersion);
        dmask->getVersion(&decompData.currentDmaskVersion);

        //
        // On the first time through, we'll need to allocate table space.
        // To minimize the number of NEW calls (and return value tests),
        // a single allocation is done, with pointers being set to the
        // three tables. This also means only one  deletion is required.
        //
        if(decompData.dith_multi_table == NULL) {
            unsigned int nwords = 4 * CELL_DITHER_MAXVALS * 4;
            decompData.dith_multi_table = new Xil_unsigned32[nwords];
            if(decompData.dith_multi_table == NULL) {
                XIL_ERROR(system_state, XIL_ERROR_RESOURCE, "di-1", TRUE);
                return NULL;
            }
        }

        //
        // Set the pointers
        //
        dith_tbl_r = decompData.dith_multi_table;
        dith_tbl_g = dith_tbl_r + CELL_DITHER_MAXVALS * 4;
        dith_tbl_b = dith_tbl_g + CELL_DITHER_MAXVALS * 4;
        dith_tbl   = dith_tbl_b + CELL_DITHER_MAXVALS * 4;
            
        //
        // Get the data from the colorcube and dithermask
        //
        const int*          mults      = cube->getMultipliers();
        const unsigned int* dims       = cube->getDimensions();
        const float*        dmask_vals = dmask->getData();
            
        for(int band=0; band<3; band++) {
            Xil_unsigned8* ptr;
            int offset;
                
            //
            // The dithermask band order is BGR
            //
            switch (band) {
              case 0:
                ptr = (Xil_unsigned8*)dith_tbl_b;
                offset = 0;
                break;
              case 1:
                ptr = (Xil_unsigned8*)dith_tbl_g;
                offset = 0;
                break;
              case 2:
                ptr = (Xil_unsigned8*)dith_tbl_r;
                // colorcube offset is built into the red table
                offset = cube->getAdjustedOffset();
                break;
            }
                
            if(dims[band] > 1) {
                int mult = mults[band];

                //
                // A negative multiplier indicates a decreasing color ramp 
                // rather than an increasing ramp.
                //
                int ramp_bottom;
                if(mult < 0) {
                    //ramp_bottom = offset + (dims[band] - 1) * abs(mult);
                    ramp_bottom = offset - (dims[band] - 1) * mult;
                } else {
                    ramp_bottom = offset;
                }
                float divisor = 256.0 / (float)((int)dims[band] - 1);
                for(int i=0; i<256; i++) {
                    // Temporarily store the entire quotient in 'fraction'
                    float fraction = (i / divisor);

                    // Now separate the quotient into whole and fractional parts
                    int whole = (int)fraction;
                    fraction -= whole;
              
                    // The dithermask values are in [3][4][4] order
                    const float* dmask_ptr = dmask_vals + (band << 4);
                
                    //
                    // Test fraction against each value in the dithermask for
                    // this band
                    //
                    for(int j=0; j<16; j++) {
                        if(fraction > dmask_ptr[j]) {
                            ptr[j] = ramp_bottom + mult * (whole + 1);
                        } else {
                            ptr[j] = ramp_bottom + mult * whole;
                        }
                    }
                    ptr += 16;
                }
            } else if(dims[band] == 1) {
                //
                // A dimension of 1 means a colorsquare (or a colorline) 
                // rather than a colorcube, and that this band is being 
                // discarded by the user.
                //
                for(int i=0; i<256*16; i++) {
                    ptr[i] = offset;
                }
            } else {
                // Unreachable code, the core should prevent this from happening.
                return NULL;
            }
        }
            
    } 

    //
    // Set the pointers. Have to do it here as well as in the loop above,
    // since we may be reusing the table, never passing thru loop above.
    //
    dith_tbl_r = decompData.dith_multi_table;
    dith_tbl_g = dith_tbl_r + CELL_DITHER_MAXVALS * 4;
    dith_tbl_b = dith_tbl_g + CELL_DITHER_MAXVALS * 4;
    dith_tbl   = dith_tbl_b + CELL_DITHER_MAXVALS * 4;
            

    //
    // If there was a new colormap in the bytestream, or if the colorcube
    // or dithermask changed, rebuild the dither table.
    //
    Xil_boolean new_cmap = 
        ! decompData.colormap->isSameAs(&decompData.currentBytestreamCmapVersion);
    if(new_cmap) {
        //
        // Update the current value
        //
        decompData.colormap->getVersion(&decompData.currentBytestreamCmapVersion);
    }

    Xil_unsigned32* pDith = dith_tbl;
    if(new_ccube_or_dmask || new_cmap) {

        Xil_unsigned8* cmapdata = (Xil_unsigned8*)decompData.colormap->getData();
        for(int i=decompData.colormapEntries; i>0; i--) {
            //
            // TODO: 
            // For a small performance gain, we could check if the colormap
            // entry has changed before recalculating.  This would require
            // keeping a copy of the dither table's idea of the colormap.
            //
            // Get a pointer into each of the r,g,b dither tables by adding the
            // colormap value times 4; remember that the tables are conceptually
            // 256 x 16 bytes
            //
            Xil_unsigned32* ptr_r = dith_tbl_r + (cmapdata[0] << 2);
            Xil_unsigned32* ptr_g = dith_tbl_g + (cmapdata[1] << 2);
            Xil_unsigned32* ptr_b = dith_tbl_b + (cmapdata[2] << 2);
            cmapdata += 3;

            //
            // We know that there won't be any overflow between bytes, so we do
            // the addition as 4 bytes in parallel.
            //
            pDith[0] = ptr_r[0] + ptr_g[0] + ptr_b[0];
            pDith[1] = ptr_r[1] + ptr_g[1] + ptr_b[1];
            pDith[2] = ptr_r[2] + ptr_g[2] + ptr_b[2];
            pDith[3] = ptr_r[3] + ptr_g[3] + ptr_b[3];
            pDith += 4;
        }
    }

    return dith_tbl;
}
