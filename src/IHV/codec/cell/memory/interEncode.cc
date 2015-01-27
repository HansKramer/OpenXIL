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
//  File:   interEncode.cc
//  Project:    XIL
//  Revision:   1.3
//  Last Mod:   10:15:51, 03/10/00
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
#pragma ident   "@(#)interEncode.cc	1.3\t00/03/10  "

#include <stdlib.h>
#include "XilDeviceCompressionCell.hh"
#include "CellOutput.hh"

static int
computeLuma(CmapTable& cmap, Cell cell0, Cell cell1) 
{
    int mae = 0, bits, diff;

    int l00 = cmap[cell0.C0()].band0();
    int l01 = cmap[cell0.C1()].band0();
    int l10 = cmap[cell1.C0()].band0();
    int l11 = cmap[cell1.C1()].band0();

    for (int i=0; i<16; i++) {
      bits = (((cell0.MASK() & (1<<i))&&1)<<1) | ((cell1.MASK() & (1<<i))&&1);

      switch(bits) 
      {
        case 0:    diff = l00-l10; break;
        case 1:    diff = l00-l11; break;
        case 2:    diff = l01-l10; break;
        case 3:    diff = l01-l11; break;
      }
      mae += ((diff>0) ? diff : -diff);
    }
    
    return mae;
}

static int
computeChrom(CmapTable& cmap, Cell cell0, Cell cell1) 
{
    int dist = 0, bits, udiff, vdiff;
    
    int u00 = cmap[cell0.C0()].band1();
    int u01 = cmap[cell0.C1()].band1();
    int u10 = cmap[cell1.C0()].band1();
    int u11 = cmap[cell1.C1()].band1();
    
    int v00 = cmap[cell0.C0()].band2();
    int v01 = cmap[cell0.C1()].band2();
    int v10 = cmap[cell1.C0()].band2();
    int v11 = cmap[cell1.C1()].band2();

    for (int i=0; i<16; i++) {
      bits = (((cell0.MASK() & (1<<i))&&1)<<1) | ((cell1.MASK() & (1<<i))&&1);

      switch(bits) 
      {
         case 0:
            udiff = u00-u10;
            vdiff = v00-v10;
            break;
            
         case 1:
            udiff = u00-u11;
            vdiff = v00-v11;
            break;
            
         case 2:
            udiff = u01-u10;
            vdiff = v01-v10;
            break;
            
         case 3:
            udiff = u01-u11;
            vdiff = v01-v11;
            break;
      }
#ifdef TODO
      // Replace after sqrs_table is in
      dist += sqrs_table[udiff] + sqrs_table[vdiff];
#endif
      dist += ((udiff*udiff) + (vdiff*vdiff));
    }
    
    return dist;
}

void
XilDeviceCompressionCell::fillErrorFrame(void)
{
    //
    //  Go through all of the frame and fill in the error frame
    //
    unsigned int perms  = 1<<compData.currentFrame;  // 2^nframes-1
    unsigned int topbit = 1<<(compData.currentFrame-1);
    
    for (int y=0; y < compData.cellHeight; y++) {
      for (int x=0; x < compData.cellWidth; x++) {
        ErrorInfo& errinfo = (*compData.errorFrame)[y][x];

        CmapTable tmpcmap;
    
        //
        //  Go through every permutation and determine its error.
        //
        for (unsigned int part=0; part<perms; part++) {
          //
          // keyCell always starts out at the 0th intraFrame
          //
          Cell*          keyCell  = &(compData.intraFrames[0]->cell(y,x));
          unsigned int   chroma_err = 0;
          unsigned int   luma_err = 0;
          unsigned int   maxchroma_err = 0;
          unsigned int   maxluma_err = 0;

          for (int j=0; j<compData.currentFrame; j++) {
            if ((part<<j) & topbit) {
              keyCell = &(compData.intraFrames[j+1]->cell(y,x));
              continue;  // this keyCell does not increase the error
            }

            CellFrame& curFrame = *(compData.intraFrames[j+1]);
            curFrame.getColormap(tmpcmap);

            luma_err   = computeLuma(tmpcmap, curFrame[y][x], *keyCell);

            if (luma_err > maxluma_err) maxluma_err = luma_err;
              chroma_err = computeChrom(tmpcmap,
                                        curFrame[y][x],
                                        *keyCell);

            if (chroma_err > maxchroma_err)
              maxchroma_err = chroma_err;
          }
          errinfo.setErr(part, maxluma_err, maxchroma_err);
        }
      }
    }
}


unsigned int
XilDeviceCompressionCell::computeBytesInFrameGroup(unsigned int metric) 
{
    int numbytes=0;
    for (int y=0; y<compData.cellHeight; y++) {
      for (int x=0; x<compData.cellWidth; x++) {
#ifdef TODO
      // Replace after sqrs_table is in
         numbytes +=computeBytes((*compData.errorFrame)[y][x],
                                  metric,
                                  sqrs_table[(metric>>3)]);
#endif
         numbytes +=computeBytes((*compData.errorFrame)[y][x],
                                  metric,
                                  ((metric>>3)*(metric>>3)) );
      }
    }
    
    return numbytes;
}

int
XilDeviceCompressionCell::computeBytes(ErrorInfo&   errinfo,
                                       unsigned int max_lum,
                                       unsigned int max_chrom)
{
    static int cnt = 0;
    //
    //  Sort the entries based on the number of bits in the partition mask.
    //
    unsigned int min_bitson = 32;
    unsigned int totbitson  = 0;
    unsigned int perms      = 1<<compData.currentFrame;  // 2^nframes
    
    //
    //  Initialize mask as all 1s in region of interest
    //
    unsigned int min_perm   = (1<<compData.currentFrame)-1;

    for (unsigned int i=0; i<perms; i++) {
      int l = errinfo.getLumaErr(i);
      int c = errinfo.getChroErr(i);
        
      if (l < max_lum && c < max_chrom){
        for (unsigned int j=0; j<32; j+=8) {
          totbitson += bitson_table[((i>>j)&(0xff))];
        }

        if (totbitson < min_bitson) {
          min_bitson  = totbitson;
          min_perm    = i;
        }
      }
    }
    errinfo.setPartition(min_perm);

    //
    //  For each byte, find out how many bits are on...
    //
    //  Count 1 bit for the keyframe...
    //
    unsigned int bitson = 1;

    //
    // set any frames that are 1 to 0 if they're to be dropped
    //
    // min_perm = (min_perm^compData.dropMask)&min_perm;
    
    for (i=0; i<32; i+=8) {
      bitson += bitson_table[((min_perm>>i)&(0xff))];
    }
    int byteson = bitson<<2;

    for (i=0; i<((compData.currentFrame+1)-bitson); i++) {
      //      if(drand48() < 0.125) byteson++;
      if (cnt == 8) {
        cnt = 0;
        byteson++;
      } else {
        cnt++;
      }
    }
    
    //
    //  A bit that is on corresponds to 4 bytes.  A bit that is off should
    //  correspond to 0 or 1 byte.
    //
    return byteson;
}        

int
XilDeviceCompressionCell::outputCellFrame(CellFrame*  hstCellFrame,
                                          CellFrame*  curCellFrame)
{
    // 
    //  CellOutput is the output class that manages the functions
    //  associated with outputting the different cellcodes into the
    //  XilCisBuffer.  Note that the CellOutput constructor calls
    //  cbm.nextBuffer() and the CellOutput destructor calls
    //  compressedFrame().  Since the lifetime of this object is only the
    //  compress function, constructing CellOutput causes it to get the
    //  next buffer and upon leaving this function I deleted which causes
    //  it to call compressedFrame()
    //
    XilImageFormat* otype = getOutputType();
    CellOutput cellout(cbm, otype->getWidth());

    if (hstCellFrame == NULL) {
      cellout.outputKeyFrame(otype->getWidth(),
                             otype->getHeight(),
                             cellAttribs,
                             *curCellFrame);
    } else { // non-keyframe
      if (curCellFrame->getColormapChanged() == TRUE) {
        CmapTable cmap;
        curCellFrame->getColormap(cmap);
        cellout.outputColorMap(cmap);
      }

      if (curCellFrame->getUserData())
        cellout.outputUserData(curCellFrame->getUserData());

      interEncodeFrameThres(cellout, *hstCellFrame, *curCellFrame);
    }
    
    return XIL_SUCCESS;
}

void
XilDeviceCompressionCell::interEncodeFrameError(CellOutput&  cellout,
                                                unsigned int frameNumber,
                                                CellFrame&   curCellFrame)
{
    for (int y=0; y<compData.cellHeight; y++) {
      for (int x=0; x<compData.cellWidth; x++) {
        interEncodeCellError(frameNumber,
                             (*(compData.errorFrame))[y][x].getPartition(),
                             curCellFrame[y][x],
                             cellout);
      }
      cellout.flushSkip();
      cellout.flushRun();
    }
}

void
XilDeviceCompressionCell::interEncodeFrameThres(CellOutput&  cellout,
                                                CellFrame&   hstCellFrame,
                                                CellFrame&   curCellFrame)
{
    CmapTable  tmpcmap;
    curCellFrame.getColormap(tmpcmap);
    
    for (int y=0; y<compData.cellHeight; y++) {
      for (int x=0; x<compData.cellWidth; x++) {
        interEncodeCellThres(x,
                             y,
                             tmpcmap,
                             hstCellFrame,
                             curCellFrame,
                             cellout);
      }
      cellout.flushSkip();
      cellout.flushRun();
    }
}

            
void
XilDeviceCompressionCell::interEncodeCellThres(unsigned int x,
                                               unsigned int y,
                                               CmapTable&   cmap,
                                               CellFrame&   hstFrame,
                                               CellFrame&   curFrame,
                                               CellOutput&  cellout)
{
    Cell& hstCell = hstFrame[y][x];
    Cell& curCell = curFrame[y][x];
    
    //
    //  Set similarity flags based on the colors and masks
    //
    Xil_boolean  sim_bits   = (BitsOn(curCell, hstCell) <=
                               CELL_DEFAULT_BITS_SIMILARITY);
    
    Xil_boolean  sim_color0 = cmap.isSimilar(curCell.C0(),
                                             hstCell.C0(),
                                             CELL_DEFAULT_COLOR_SIMILARITY);
    
    Xil_boolean  sim_color1 = cmap.isSimilar(curCell.C1(),
                                             hstCell.C1(),
                                             CELL_DEFAULT_COLOR_SIMILARITY);

    
    if (sim_bits == TRUE && sim_color0 == TRUE && sim_color1 == TRUE) {
      cellout.outputSkip();
    } else if (x != 0 && sim_color0 == TRUE && sim_color1 == TRUE) {
      cellout.outputMaskOnly(curCell.MASK(), hstFrame.isFlipped(y,x));
      hstCell.MASK() = curCell.MASK();
    } else if (x != 0 && sim_bits == TRUE && hstCell.C0() != hstCell.C1()) {
      if (sim_color0) {
        cellout.outputColor1Only(curCell.C1(), hstFrame.isFlipped(y, x));
        hstCell.C1() = curCell.C1();
      } else if(sim_color1) {
        cellout.outputColor0Only(curCell.C0(), hstFrame.isFlipped(y, x));
        hstCell.C0() = curCell.C0();
      } else {
        cellout.outputColorsOnly(curCell.C0(),
                                 curCell.C1(),
                                 hstFrame.isFlipped(y, x));
        hstCell.C0() = curCell.C0();
        hstCell.C1() = curCell.C1();
      }
    } else {
      cellout.outputCell(curCell);
    
      hstCell = curCell;

      hstFrame.setFlipped(y, x, curFrame.isFlipped(y, x));
    }
}

void
XilDeviceCompressionCell::interEncodeCellError(unsigned int frameNumber,
                                               unsigned int partition,
                                               const Cell&  curCell,
                                               CellOutput&  cellout)
{
    if ((partition>>(compData.currentFrame - frameNumber)) & 1) {
      cellout.outputCell(curCell);
    } else {
      cellout.outputSkip();
    }
}

unsigned int
XilDeviceCompressionCell::figureBytesInFrameGroup(unsigned int dropMask)
{
    //
    //  To determine how many frames need to be flushed, we skip
    //  through the array until we come across a NULL pointer which
    //  indicates we're done.
    //
    //  When deleting frames from this array, we must be very careful
    //  to set the pointer back to NULL.
    //
    unsigned int bytes=0;
    for (int i=0; i<=compData.currentFrame; i++) {
      XilImageFormat* otype = getOutputType();
      CellOutputCounter cellout(otype->getWidth());
    
      CellFrame& curFrame = *compData.intraFrames[i];
      ErrorInfoFrame& errFrame = *compData.errorFrame;
    
      //
      //  If this is our first frame and we don't have a previous
      //  reference, then this frame becomes our previous frame and
      //  on the next frame, we begin interframe encoding against
      //  it...
      //
      //  This will also be the case for key frames.
      //
      if (i==0) {
        cellout.outputKeyFrame(otype->getWidth(),
                               otype->getHeight(),
                               cellAttribs,
                               *compData.intraFrames[i]);
      } else { // non-key frame
        if (dropMask & (1<<i)) {
          if (compData.intraFrames[i]->getColormapChanged() == TRUE) {
            CmapTable cmap;
            compData.intraFrames[i]->getColormap(cmap);
            cellout.outputColorMap(cmap);
          }
          cellout.skipEntireFrame();
        } else {
          if (compData.intraFrames[i]->getColormapChanged() == TRUE) {
            CmapTable cmap;
            compData.intraFrames[i]->getColormap(cmap);
            cellout.outputColorMap(cmap);
          }
          //
          //  Interframe compression
          //
          for (int y=0; y<compData.cellHeight; y++) {
            for (int x=0; x<compData.cellWidth; x++) {
              interEncodeCellError(i,
                                   errFrame[y][x].getPartition(),
                                   curFrame[y][x],
                                   cellout);
            }
            cellout.flushSkip();
            cellout.flushRun();
          }
        }
      }
    
      //
      //  Update our byte count
      //
      bytes += cellout.numBytesOutput();
    }

    return bytes;
}
