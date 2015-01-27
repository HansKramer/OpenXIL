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
//  File:   flush.cc
//  Project:    XIL
//  Revision:   1.2
//  Last Mod:   10:15:49, 03/10/00
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
#pragma ident   "@(#)flush.cc	1.2\t00/03/10  "

#include "XilDeviceCompressionCell.hh"

void XilDeviceCompressionCell::flush()
{
    //
    //  If there arn't any frames, then just return.
    //
    if (compData.intraFrames[0] == NULL) 
      return;

    //
    //  If I've been called when the intraframe in compData.currentFrame is
    //  NULL, then I'll keep decrementing compData.currentFrame until I find a
    //  frame.  This case can occur when flush() has been called after
    //  compress() has updated currentFrame (i.e. when compress() isn't
    //  actually calling us).  Since we've made it this far, we know that
    //  compData.intraFrames[0] != NULL.  So, compData.currentFrame can never
    //  make it below 0 in this loop.
    //
    while (compData.intraFrames[compData.currentFrame] == NULL) {
      compData.currentFrame--;
    }
    
    //
    //  This mask is used to determine if there are any frames that should be
    //  dropped.
    //
    unsigned int dropMask  = 0;
    
    if (cellAttribs.bytesPerFrameGroup > 0) {
      dropMask = controlBitRate();
    }

    for (int i=0; i<=compData.currentFrame; i++) {
      // 
      //  CellOutput is the output class that manages the functions
      //  associated with outputting the different cellcodes into the
      //  XilCisBuffer.  Note that the CellOutput constructor calls
      //  cbm->nextBuffer() and the CellOutput destructor calls
      //  compressedFrame().  Since the lifetime of this object is only the
      //  compress function, constructing CellOutput causes it to get the
      //  next buffer and upon leaving this function I deleted which causes
      //  it to call compressedFrame()
      //
      XilImageFormat* otype = getOutputType();
      CellOutput cellout(cbm, otype->getWidth());

      //
      //  The first frame (intraFrames[0]) becomes our previous frame and
      //  on the next frame, we begin interframe encoding against it...
      //
      if (i==0) {
        cellout.outputKeyFrame(otype->getWidth(),
                               otype->getHeight(),
                               cellAttribs,
                               *compData.intraFrames[0]);
      } else { // non-keyframe
        if (compData.intraFrames[i]->getColormapChanged() == TRUE) {
          CmapTable cmap;
          compData.intraFrames[i]->getColormap(cmap);
          cellout.outputColorMap(cmap);
        }

        if (compData.intraFrames[i]->getUserData())
          cellout.outputUserData(compData.intraFrames[i]->getUserData());

        if (dropMask & (1<<i)) {  // drop frame?
          cellout.skipEntireFrame();
        } else {
          //
          //  Interframe compression.
          //
          //  If we're encoding with bit-rate control turned on, then
          //  we'll use the errorFrame to encode the frame.  Otherwise,
          //  we'll use the traditional Cell encoding scheme with a
          //  default bitson and color threshold.
          //
          if (cellAttribs.bytesPerFrameGroup > 0) {
            interEncodeFrameError(cellout,
                                  i,
                                  *compData.intraFrames[i]);
          } else {
            interEncodeFrameThres(cellout,
                                  *compData.intraFrames[0],
                                  *compData.intraFrames[i]);
          }
        }
      }
    }

    //
    //  Clear out the entryUsedIndex at this point because
    //  we now know exactly what's on the screen.
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
    CellFrame& hstFrame = *compData.intraFrames[0];
    for (int y=0; y < compData.cellHeight; y++) {
      for (int x=0; x < compData.cellWidth; x++) {
        compData.entryUsedIndex[hstFrame[y][x].C0()] = TRUE;
        compData.entryUsedIndex[hstFrame[y][x].C1()] = TRUE;
      }
    }
    
    for (j=0; j<compData.currentCmap.getNumEntries(); j++)
      if (compData.entryUsedIndex[j] == FALSE &&
          compData.nextCmap.used(j) == 0)
        compData.nextCmap.used(j) = -1;

    //
    //  Delete and clear out all of the intraFrames pointers that were used
    //  for this frame group.  Need to be sure that they're reset to NULL
    //  because this is used to search for the last encoded frame.
    //
    for (i=0; i<=compData.currentFrame; i++) {
      delete compData.intraFrames[i];
      compData.intraFrames[i] = NULL;
    }

    //
    //  Reset currentFrame
    //
    compData.currentFrame = 0;
}
