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
//  Revision:   1.8
//  Last Mod:   10:15:46, 03/10/00
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
#pragma ident   "@(#)compress.cc	1.8\t00/03/10  "

#include <xil/xilGPI.hh>

#include "XilDeviceCompressionCell.hh"
#include "CellOutput.hh"
#include "XiliColormapGenerator.hh"
#include "XiliUtils.hh"

//------------------------------------------------------------------------
//
//  Function:    XilDeviceCompressCell::compress
//
//  Description:
//    Takes an XilImage and encodes it into a Cell byte-stream.
//
//    The very first image that is received defines the size of all
//    the images to follow.  XilCis derives this information and
//    puts it into inputType.
//    
//    There are two types of encoding.  Block Truncation Coding
//    (BTC) and Dither encoding.  BTC is much faster that Dither yet
//    is still produces a fair result whereas Dither produces a
//    superior image but takes much longer to encode.
//    
//    There are four phases to the compress routine.
//
//    The first is the locating which colormap to use for the
//        encoding of the given image.
//    The second is filtering the image to reduce "noise" which
//        provides for a much greater compression ratio.
//    The third is IntraFrame encoding.  This is where the actual
//        image is encoded into a CellFrame.  Either BTC or
//        Dither encoding is used to create the CellFrame.
//    The fourth is InterFrame  encoding.  Based upon the keyframe
//        interval, full CellFrames will be sent and any future
//        history will be based upon the key frames.  Non-key
//        frames are encoded with an assortment of skip codes.
//
//    In addition, Adaptive Colormap Selection is used to choose
//    colormaps if the user has requested the algorithm to be turned
//    on.  Another alorithm determines whether or not the error
//    between two colormaps requires that a new colormap be encoded
//    into the byte-stream.
//    
//  Parameters:
//    
//  Returns:
//    XIL_SUCCESS or XIL_FAILURE
//    
//  Side Effects:
//    
//    
//  Notes:
//    
//    
//  Deficiencies/ToDo:
//    
//    
//------------------------------------------------------------------------

XilStatus
XilDeviceCompressionCell::compress(XilOp*        op,
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
    //  Verify the Image width and height is divisible by 4.
    //
    if ((ci.cis_width % 4) != 0) {
      XIL_ERROR(system_state, XIL_ERROR_USER, "di-253", TRUE);
      return XIL_FAILURE;
    }        

    if ((ci.cis_height % 4) != 0) {
      XIL_ERROR(system_state, XIL_ERROR_USER, "di-254", TRUE);
      return XIL_FAILURE;
    }        

    //
    //  XilImage type verification is done above and in XilCis.
    //  But, I have to check and set up compData if it hasn't been
    //    setup properly on a previous call.
    //
    //  Ensure bytesPerFrameGroup is up-to-date by recomputing it if
    //  bitsPerSecond is on.  This also catches the case when the values have
    //  been set before the maximum frame size is known.
    //
    if (cellAttribs.bitsPerSecond > 0.0) {
      if (computeBytesPerFrameGroup() == FALSE) {
        return XIL_FAILURE;
      }
    }

    if (compData.initialized == FALSE) {
      //
      //  Calculate how many bits the error frame needs.
      //     It's 2^(kfi-1).
      //
      unsigned int bits = (1 << (cellAttribs.keyFrameInterval-1));
      unsigned int ebits = (cellAttribs.bytesPerFrameGroup > 0 ? bits : 0);
      if(compData.initialize(ci.cis_width, ci.cis_height, ebits) == XIL_FAILURE)
        return XIL_FAILURE;
    }
    
    //
    //  Create curCellFrame for this Image...
    //
    CellFrame*  curCellFrame =
        new CellFrame(compData.cellWidth, compData.cellHeight);
    
    if (curCellFrame == NULL) {
      // out of memory error 
      XIL_OBJ_ERROR(system_state, XIL_ERROR_SYSTEM,"di-1", TRUE, getCis());
      return XIL_FAILURE;
    }

    if (curCellFrame->ok() == NULL) {
      // Couldn't create internal Cell compressor object 
      XIL_OBJ_ERROR(system_state, XIL_ERROR_SYSTEM,"di-275", FALSE, getCis());
      return XIL_FAILURE;
    }

    //
    //  When bit-rate control is on, this is a buffered frame.
    //
    if(cellAttribs.bytesPerFrameGroup > 0) {
        compData.intraFrames[compData.currentFrame] = curCellFrame;
    }
    
    //  The compressor itself is checked as to whether it
    //  has a colormap associated with it.
    //  Once currentCmap is set to the colormap associated with
    //  the compressor, the attribute is returned to NULL in order to
    //  detect changes in the future.  This might be done better with
    //  a flag.
    //
    //  If the compressor doesn't have a colormap, and we're in
    //  adaptive mode, then we'll use the colormap in nextCmap.
    //  This needs to be fixed.  Currently, if we're in adaptive mode,
    //  and it's the first frame and there was no colormap associated
    //  with the compressor, a NULL colormap is used (i.e. whatever is
    //  in nextCmap)
    //
    //  If there doesn't appear to be a colormap to be found, then
    //  I'll generate one.
    //
    if (cellAttribs.compressorCmap != NULL) {
      compData.currentCmap = *(cellAttribs.compressorCmap);

      delete cellAttribs.compressorCmap;
      cellAttribs.compressorCmap = NULL;

      compData.currentCmapIsSet = TRUE;

      curCellFrame->setColormapChanged(TRUE);

      if (cellAttribs.encodingType == BTC)
        compData.currentCmap.toYUV();
      else
        compData.currentCmap.toRGB();

      compData.cmapSelection.useNewColormap(compData.currentCmap);
    } else if (cellAttribs.colorMapAdaption == TRUE &&
               compData.adaptIsChanging == TRUE &&
               cellAttribs.encodingType == BTC) {
      compData.currentCmap = compData.nextCmap;
      compData.currentCmap.setColorSpace(YUV);
      compData.currentCmapIsSet = TRUE;

      curCellFrame->setColormapChanged(TRUE);

      compData.cmapSelection.useNewColormap(compData.currentCmap);
    } else if (! compData.currentCmapIsSet) {
      // Generate Cmap...
      unsigned int cmapsize;
      if (cellAttribs.maxCompressorCmapSize > 1) {
        cmapsize = cellAttribs.maxCompressorCmapSize;
      } else {
        cmapsize = CELL_COMPRESSOR_DEFAULT_CMAP_SIZE;
      }
    
      //
      // Instantiate a new colormap generator object
      //
      XiliColormapGenerator* cmapGen = new XiliColormapGenerator;

      //
      // Generate the histogram
      //
      cmapGen->hist3dPixSeq((Xil_unsigned8*)ci.image_box_dataptr, 
                            ci.image_box_width, ci.image_box_height, 
                            ci.image_ps, ci.image_ss);

      //
      // Generate the colormap
      //
      Xil_unsigned8 lut[768];
      xili_memset(lut, 0, 768);
      cmapGen->generateColormap(lut, cmapsize);
      delete cmapGen;

      XilLookupSingle* tmpcmap = system_state->createXilLookupSingle(
                                     XIL_BYTE, XIL_BYTE, 3, 
                                     cmapsize, 0, lut);
      if (tmpcmap == NULL) {
          return XIL_FAILURE;
      }

      XilColorspace* colorspace = ci.image->refColorspace();

      if (colorspace != NULL) {
        XilColorspace* ycc601 = (XilColorspace*)
                system_state->getObjectByName("ycc601", XIL_COLORSPACE);
        XilColorspace* ycc709 = (XilColorspace*)
                system_state->getObjectByName("ycc709", XIL_COLORSPACE);
        XilColorspace* photoycc = (XilColorspace*)
                system_state->getObjectByName("photoycc", XIL_COLORSPACE);

        if (colorspace->getOpcode() == ycc601->getOpcode() ||
            colorspace->getOpcode() == ycc709->getOpcode() ||
            colorspace->getOpcode() == photoycc->getOpcode()) {
          compData.currentCmap.setAsColormapTable(
                             (const Xil_unsigned8*)tmpcmap->getData(),
                             YUV,
                             tmpcmap->getNumEntries());
        } else {
          compData.currentCmap.setAsColormapTable(
                             (const Xil_unsigned8*)tmpcmap->getData(),
                             BGR,
                             tmpcmap->getNumEntries());
        }
      } else {
        compData.currentCmap.setAsColormapTable(
                             (const Xil_unsigned8*)tmpcmap->getData(),
                             BGR,
                             tmpcmap->getNumEntries());
      }
      tmpcmap->destroy();

      compData.currentCmapIsSet = TRUE;
      curCellFrame->setColormapChanged(TRUE);

      if (cellAttribs.encodingType == BTC)
        compData.currentCmap.toYUV();
      else
        compData.currentCmap.toRGB();

      compData.cmapSelection.useNewColormap(compData.currentCmap);
    } else {
      compData.cmapSelection.clearStatTables();
    }
    
    //
    //  Store the colormap associated with this frame.
    //
    curCellFrame->setColormap(compData.currentCmap);

    //
    //  Store any User Data for this frame.  We make our own copy when the
    //  user sets the attribute.  So, I just set it now.
    //
    curCellFrame->setUserData(cellAttribs.compressorUserData);
    cellAttribs.compressorUserData = NULL;

    //
    //  Temporal filtering for the current image
    //
    Xil_unsigned8* filteredImage = NULL;
    Xil_unsigned8* compressImage = NULL;

    unsigned int ps, ss;

    if (cellAttribs.temporalFiltering == TRUE) {
      if (compData.previousImage == NULL) {

        compData.previousImage =
            new Xil_unsigned8[ci.cis_width*ci.cis_height*3];

        if (compData.previousImage == NULL) {
          XIL_ERROR( NULL, XIL_ERROR_RESOURCE, "di-1", FALSE);
          return XIL_FAILURE;
        }
  
        //
        // Copy the image data to the temporary buffer
        //
        Xil_unsigned8* src_line = (Xil_unsigned8*)ci.image_box_dataptr;
        Xil_unsigned8* dst_line = compData.previousImage;
        unsigned int src_ps = ci.image_ps;
        unsigned int dst_ss = ci.image_box_width * 3;
        for(int line=ci.image_box_height; line!=0; line--) {
            Xil_unsigned8* psrc = src_line;
            Xil_unsigned8* pdst = dst_line;
            for(int samp=ci.image_box_width; samp!=0; samp--) {
                pdst[0] = psrc[0];
                pdst[1] = psrc[1];
                pdst[2] = psrc[2];
                psrc += src_ps;
                pdst += 3;
            }
            src_line += ci.image_ss;
            dst_line += dst_ss;
        }

        compressImage = compData.previousImage;

        // Set Pixel & scanline stride */
        ps = 3;
        ss = ci.cis_width*3;

        for (int i=0, c;i<256;i++) {
          c = CODE(i);
          rcode[i] = c<<RED_CODE_SHIFT;
          gcode[i] = c<<GRN_CODE_SHIFT;
          bcode[i] = c<<BLU_CODE_SHIFT;
        }

        xili_memset(curHist, 0, 8192);
        xili_memset(prvHist, 0, 8192);
      } else {
        //
        //  Create a temporary image to store the final filtered
        //  image.
        //
        filteredImage =  new Xil_unsigned8[ci.image_data_quantity];
  
        if (filteredImage == NULL) {
            XIL_ERROR( NULL, XIL_ERROR_RESOURCE, "di-1", FALSE);
            return XIL_FAILURE;
        }
  
        if ((compData.imageError =
          temporalFilterImages(compData.previousImage,
                               &ci,
                               filteredImage,
                               cellAttribs.lowFilterThreshold,
                               cellAttribs.highFilterThreshold))==-1.0) {
          return XIL_FAILURE;
        }

        compressImage = filteredImage;

        // Set Pixel & scanline stride */
        ps = 3;
        ss = ci.cis_width*3;
      }
    } else {
      compressImage = (Xil_unsigned8*) ci.image_box_dataptr;

      // Set Pixel & scanline stride */
      ps = ci.image_ps;
      ss = ci.image_ss;
    }

    XilColorspace* cspace = ci.image->refColorspace();

    //
    //  Initialize the InFrame class which simply provides
    //  a mechanism to retrieve the image in 4x4 blocks of pixes.
    //
    if (compData.inFrame->useNewImage(system_state,
                                      ci.cis_width,
                                      ci.cis_height,
                                      3,
                                      ps,    // Pixel_stride
                                      ss, // Scanline_stride
                                      compressImage,
                                      cspace) == XIL_FAILURE) {
      delete [] filteredImage;
      return XIL_FAILURE;
    }

    //
    //  Raw CELL Encoding of the current image
    //
    if (intraEncodeCurrentImage(*curCellFrame) == XIL_FAILURE) {
      delete [] filteredImage;
      return XIL_FAILURE;
    }

    //
    //  We're done encoding so the filteredImage (if it was set) can be
    //  destroyed now.
    //
    delete [] filteredImage;
    
    //
    //  Always reset the colormap to gather statistics for the next
    //  colormap -- unless it's not initialized because we're using
    //  dithered encoding.
    //
    if (cellAttribs.encodingType != DITHER)
      compData.cmapSelection.getNextColorMap(compData.nextCmap);

    //
    //  Determine whether the colormap should change based on
    //  %difference in error between frames.
    //
    if (cellAttribs.colorMapAdaption == TRUE &&
        cellAttribs.encodingType     == BTC) {
      if (((CmapTable&)compData.currentCmap) ==
           ((CmapTable&)compData.nextCmap) ||
          !controlColormapOutput()) {
        curCellFrame->setColormapChanged(FALSE);
      }
    }
    
    //
    //  If we're doing bit rate control, then we'll buffer up all of the
    //  frames upto the keyFrameInterval.  Once we've got a full frame group's
    //  worth, then flush() will take care out sending out the byte-stream.
    //
    //  Otherwise, every frame will be directly encoded into the CIS.
    //
    if (cellAttribs.bytesPerFrameGroup > 0) {
      //
      //  Stop here and check if we have filled up a frame group and
      //  it's time to do the interframe encoding step.
      //
      if (compData.currentFrame == (cellAttribs.keyFrameInterval - 1)) {
        //
        //  Causes all of the temporal encoding to occur and actually
        //  write out the bits into the CIS.  It will also update
        //  currentFrame, and entryUsedIndex. 
        //
        flush();
        return XIL_SUCCESS;
      }
    
      if (compData.currentFrame == 0) { // the key frame
        //
        //  Clear out the entryUsedIndex at this point because
        //  we now know exactly what's on the screen.
        //
        for (unsigned int j=0; j<compData.currentCmap.getNumEntries(); j++) {
          compData.entryUsedIndex[j] = FALSE;
        }
      }

      //
      //  Keep track of whether indexes can be completely reused.
      //  UsedCmapTable used flag == -1 indicates to the
      //  Adaptive Colormap Selection algorithm that a color can
      //  be completely reused.
      //
      CellFrame& intraFrame = *(curCellFrame);
      for (int l_y = 0; l_y < compData.cellHeight; l_y++) {
        for (int l_x = 0; l_x < compData.cellWidth; l_x++) {
          compData.entryUsedIndex[intraFrame[l_y][l_x].C0()] = TRUE;
          compData.entryUsedIndex[intraFrame[l_y][l_x].C1()] = TRUE;
        }
      }
    
    } else {
      // bit-rate control is not on so we'll just output
      // the current frame

      if (compData.currentFrame == 0) {
        outputCellFrame(NULL, curCellFrame);

        *compData.hstCellFrame = *curCellFrame;
      } else { // non-keyframe
        outputCellFrame(compData.hstCellFrame, curCellFrame);
      }

      delete curCellFrame;

      //
      //  Since we're actively sending frames out, we can clear the
      //  array since the status of the screen is known.
      //
      for (unsigned int j=0; j<compData.currentCmap.getNumEntries(); j++) {
        compData.entryUsedIndex[j] = FALSE;
      }

      //
      //  Keep track of whether indexes can be completely reused.
      //  UsedCmapTable used flag == -1 indicates to the
      //  Adaptive Colormap Selection algorithm that a color can
      //  be completely reused.
      //
      CellFrame& intraFrame = *(compData.hstCellFrame);
      for (int l_y = 0; l_y < compData.cellHeight; l_y++) {
        for (int l_x = 0; l_x < compData.cellWidth; l_x++) {
          compData.entryUsedIndex[intraFrame[l_y][l_x].C0()] = TRUE;
          compData.entryUsedIndex[intraFrame[l_y][l_x].C1()] = TRUE;
        }
      }
    }
    
    for (unsigned int j=0; j < compData.currentCmap.getNumEntries(); j++) {
      if (compData.entryUsedIndex[j] == FALSE &&
          compData.nextCmap.used(j) == 0) {
        compData.nextCmap.used(j) = -1;
      }    
    }
    
    //
    //  Increment currentFrame counter.
    //
    if (compData.currentFrame == (cellAttribs.keyFrameInterval - 1)) {
      compData.currentFrame = 0;
    } else {
      compData.currentFrame++;
    }
    
    return XIL_SUCCESS;
}

