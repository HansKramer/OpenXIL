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
//  File:   bitRateControl.cc
//  Project:    XIL
//  Revision:   1.2
//  Last Mod:   10:15:50, 03/10/00
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
#pragma ident   "@(#)bitRateControl.cc	1.2\t00/03/10  "

#include "XilDeviceCompressionCell.hh"

inline int
MaxKeyFrameBytes(CellFrame& cframe) 
{
    // Estimate largest size...
    return  ((cframe.getFrameWidth()*cframe.getFrameHeight())<<2) + 3*256;
}

inline int
RealKeyFrameBytes(CellFrame& cframe, CellAttribs& cellAttribs) 
{
    CellOutputCounter cellout(cframe.getFrameWidth()<<2);
    
    if (cframe.getColormapChanged() == TRUE) {
      CmapTable cmap;
      cframe.getColormap(cmap);
      cellout.outputColorMap(cmap);
    }
    cellout.outputKeyFrame(cframe.getFrameWidth()<<2,
                           cframe.getFrameHeight()<<2,
                           cellAttribs, cframe);

    return cellout.numBytesOutput();
}    
    
unsigned int
XilDeviceCompressionCell::controlBitRate(void)
{

    unsigned int max_metric = 256;
    unsigned int metric;        // metric is a number that maps to the
                                // luminance and chromanance error and is used
                                // to control the size of the byte stream 
    unsigned int bottom     = 0;
    unsigned int top        = max_metric;
    long nbytes             = 0;
    unsigned int dropMask   = 0;

    //
    //  Fill the error frame
    //
    fillErrorFrame();

    do {
      if (nbytes == 0) {
        metric = top;
      } else if (nbytes > cellAttribs.bytesPerFrameGroup) {
        bottom = metric;
        metric = (top+bottom)>>1;
      } else if (nbytes < cellAttribs.bytesPerFrameGroup) {
        top    = metric;
        metric = (bottom+top)>>1;
      }                    

      if ((top - bottom) < 16) {
        //
        //  Recompute the smallest number of bytes that we think
        //  will work -- the bottom
        //
        metric = bottom;
        nbytes = computeBytesInFrameGroup(metric);

        //
        //  Actually do a trial compress to determine exactly how
        //  many bytes will be required
        //
        nbytes = figureBytesInFrameGroup(dropMask);
                        
        while (nbytes > cellAttribs.bytesPerFrameGroup &&
               metric < max_metric) {
           metric += 8;
           computeBytesInFrameGroup(metric);

           nbytes = figureBytesInFrameGroup(dropMask);
        }

        //
        //  If we're still over our required goal, then we'll just
        //  keep as many full frames as we can.
        //  
        if (nbytes > cellAttribs.bytesPerFrameGroup) {
          int keybytes  = MaxKeyFrameBytes(*compData.intraFrames[0]);
          int bytesleft = cellAttribs.bytesPerFrameGroup - keybytes;

          //
          //  The maximum number of key frames that can fit
          //  within the number of bytes left.
          //
          int max_frames_on = (bytesleft/keybytes);
                        
          if (max_frames_on > 0) {
            int interval = cellAttribs.keyFrameInterval/
                        (max_frames_on+1);

            if (interval <= 0) interval = 1;

            unsigned int on_frames=0;
            for (int i=interval; i<compData.currentFrame; i+=interval) {
               on_frames |= (1<<i);
            }
            dropMask = ~on_frames;

            //
            //  Go through the error frame and set all of the
            //  partitions to just the frames that will be
            //  encoded.
            //
            for (int y=0; y<compData.cellHeight; y++) {
               for (int x=0; x<compData.cellWidth; x++) {
                  (*compData.errorFrame)[y][x].setPartition(on_frames>>1);
               }
            }
          } else {
            //
            //  Drop everything but the keyframe
            //
            dropMask = ~0;
          }
        }
        break;
      } else {
        nbytes = computeBytesInFrameGroup(metric);
      }
    } while(1);

    return dropMask;
}

