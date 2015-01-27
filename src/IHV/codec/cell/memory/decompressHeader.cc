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
//  File:   decompressHeader.cc
//  Project:    XIL
//  Revision:   1.4
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
#pragma ident   "@(#)decompressHeader.cc	1.4\t00/03/10  "

#include "XilDeviceCompressionCell.hh"
#include "string.h"

inline void
updateUserColormapIfPending(CellDecompressorData&     decompData,
                            XilDeviceCompressionCell& xdc)
{
    //
    // When burning more than one frame, we postpone updates to the user's
    // colormap until reading the header for the last frame.  The
    // updateUserColormapEnabled flag is controlled in burnFrames().
    //
    if (decompData.updateUserColormapPending &&
        decompData.updateUserColormapEnabled) {
      xdc.updateUserColormap();
      decompData.updateUserColormapPending = 0;
    }
}

//------------------------------------------------------------------------
//
//  Function:    XilDeviceCompressionCell::decompressHeader
//
//  Description:
//    This reads any header information before decompressing the 
//    next frame.
//
//    We are currently assuming that keyframes and only keyframes
//    will have a frame header.
//
//------------------------------------------------------------------------

XilStatus
XilDeviceCompressionCell::decompressHeader(void)
{
    unsigned int count;
    
    //
    // We need to clear any outstanding compressions because we must read the
    // header information here.
    //
    // In XIL 1.3, we don't.
    //
    //getCis()->sync();

    // if we've already read the header we want, we're done
    if (decompData.bp != NULL &&
        decompData.headerFrameNumber == cbm->getRFrameId()) {

      //
      //  It's possible that the user set the colormap since we read this
      //  hearder.  If the user did change the colormap, then it needs to be
      //  updated.
      //
      updateUserColormapIfPending(decompData, *this);
    
      return XIL_SUCCESS;
    }

    //
    //  We should delete any outstanding userdata before every frame because
    //  the data is only guarenteed to last between two successive decompress
    //  calls.
    //
    if (cellAttribs.decompressorUserData) {
      delete cellAttribs.decompressorUserData->data;
      delete cellAttribs.decompressorUserData;
      cellAttribs.decompressorUserData = NULL;
    }
    
    // bp is the byte pointer used to access the Cell bytestream; get its
    // initial value from the CIS buffer mgr
    Xil_unsigned8* bp = (Xil_unsigned8 *)cbm->nextFrame();
    
    if (bp == NULL) {
      // XilCis: No data to decompress
      XIL_ERROR( NULL, XIL_ERROR_SYSTEM, "di-100", TRUE);
      decompData.bp = bp;
      return XIL_FAILURE;
    }
    
    decompData.frameType = XIL_CIS_CELL_NON_KEY_FRAME;
    
    // If this is the very first frame, check for a VFF header (for backward
    // compatibility with old Cell movies).  I'm intentionally testing bp[0]
    // first for speed.
    if (bp[0] == 'n' && cbm->getRFrameId() == 0) {

      // This must be a VFF header from an old Cell movie file
      if (strncmp((const char *)bp, "ncaa", 4) != 0) {
        // found 'n' but not 'ncaa'
        XIL_ERROR( NULL, XIL_ERROR_SYSTEM, "di-97", TRUE);
        return byteStreamError(bp + 1);
      }
      bp += 4;
    
      // get the image dimensions from the "size=" keyword
      do {
        bp++;
        if (*bp == '\f') {
          // VFF: missing 'size='
          XIL_ERROR( NULL, XIL_ERROR_SYSTEM, "di-97", TRUE);
          return byteStreamError(bp + 1);
        }
      } while (strncmp((const char *)bp, "size=", 5) != 0);

      bp += 5;

      // imageWidth & imageHeight are unsigned int, note 
      // the 'h' modifier
      int assert_fail=FALSE;
      if ( assert_fail ||  
         (sscanf((const char *)bp, "%hd %hd", &imageWidth, &imageHeight)
          != 2) ) {
        // bad width & height in VFF header
        XIL_ERROR( NULL, XIL_ERROR_SYSTEM, "di-97", TRUE);
        return byteStreamError(bp);
      }
    
      // look for "frames="
      do {
        bp++;
        if (*bp == '\f') {
          // VFF: missing 'frames='
          XIL_ERROR( NULL, XIL_ERROR_SYSTEM, "di-97", TRUE); 
          return byteStreamError(bp + 1);
        }
      } while (strncmp((const char *)bp, "frames=", 7) != 0);

      bp += 7;

      int frames;

      if ( assert_fail || 
         (sscanf((const char *)bp, "%d", &frames) != 1) ) {
        // bad frame count in VFF header
        XIL_ERROR( NULL, XIL_ERROR_SYSTEM, "di-97", TRUE);  
        return byteStreamError(bp);
      }
    
      // look for "frame_rate="
      do {
        bp++;
        if (*bp == '\f') {
          // VFF: missing 'frame_rate='
          XIL_ERROR( NULL, XIL_ERROR_SYSTEM, "di-97", TRUE);
          return byteStreamError(bp + 1);
        }
      } while (strncmp((const char *)bp, "frame_rate=", 11) != 0);

      bp += 11;

      int frame_rate;

      if ( assert_fail || 
         (sscanf((const char *)bp, "%d", &frame_rate) != 1) ) {
        // bad frame_rate in VFF header
        XIL_ERROR( NULL, XIL_ERROR_SYSTEM, "di-97", TRUE);  
        return byteStreamError(bp);
      }

      // we store the frame rate in microseconds per frame
      cellAttribs.decompressorFrameRate = 1000000 / frame_rate;
    
      // look for "colormapsize="
      do {
        bp++;
        if (*bp == '\f') {
          // VFF: missing 'colormapsize='
          XIL_ERROR( NULL, XIL_ERROR_SYSTEM, "di-97", TRUE);   
          return byteStreamError(bp + 1);
        }
      } while (strncmp((const char *)bp, "colormapsize=", 13) != 0);

      bp += 13;

      if ( assert_fail ||  
         (sscanf((const char *)bp, "%d",
           &cellAttribs.maxDecompressorCmapSize) != 1) ) {
        // bad colormapsize in VFF header
        XIL_ERROR( NULL, XIL_ERROR_SYSTEM, "di-97", TRUE);    
        return byteStreamError(bp);
      }
    
      // scan to the end of the VFF header, which is a form-feed followed
      // by a new-line
      do {
        bp++;
      } while (*bp != '\f');

      if ( assert_fail ||   
         (*++bp != '\n') ) {
        // VFF: missing newline at end
        return byteStreamError(bp);
      }
      bp++; // now pointing to byte after the '\n'
    
      // advance past the frame index
      bp += frames * 8;      // 8 = sizeof(MBlock)
    
      // Since this frame is the first frame, it must be a key frame
      decompData.frameType = XIL_CIS_CELL_KEY_FRAME;
    }
    
    while(bp[0] == CELL_NEW_COLOR_MAP ||  // it's a new colormap
          bp[0] == CELL_USER_DATA     ||  // it's a block of user data
          bp[0] == 0xff) {                // it's a Cell frame header
    
      switch(bp[0])  {
        case CELL_NEW_COLOR_MAP:
          //
          //  Read the values directly from the stream.  The count is
          //  encoded in the byte-stream as "count-1" so we add 1 to it in
          //  order to arrive at the proper count.
          //
          count = bp[1] + 1;
        
          // advance the byte pointer past the count to the RGB data
          bp += 2;
        
          //
          // The colormap is always included for a keyframe, even if it
          // is the same as the last colormap in the bytesream, so for
          // efficiency, check to see if this is just a repeated colormap.
          // '(count<<1) + count' is a shortcut for 'count*3'
          //
          if (count != (unsigned int) decompData.colormapEntries ||
              memcmp(bp,
               decompData.colormap->getData(),
               (count<<1) + count) != 0) {
            decompData.colormap->setValues(0, count, bp);
            //
            // The setValues() function does not affect the XilLookup's
            // idea of the number of entries, so we keep track of that
            // value separately here:
            //
            decompData.colormapEntries = count;
            //
            //  If the user gave me a colormap to manage, set a flag
            //  which indicates an update of the user's colormap is
            //  pending.
            if (cellAttribs.decompressorCmap != NULL) {
              decompData.updateUserColormapPending = 1;
            }
          }
        
          // advance the byte pointer past the RGB data
          bp += (count<<1) + count;      // count*3
        
          break;  // CELL_NEW_COLOR_MAP

        case CELL_USER_DATA:
          //
          //  Read the values directly from the stream.  The count is
          //  encoded in the byte-stream as "count-1" so we add 1 to it in
          //  order to arrive at the proper count.
          //
          count = ((bp[1]<<16)|(bp[2]<<8)|bp[3]);
            
          // advance the byte pointer past the count to the USER DATA
          bp += 4;

          {
            unsigned int    ffcnt;
                
            if ((count&0xffffff)==0xffffff) ffcnt = 3;
            else if((count&0x00ffff)==0x00ffff) ffcnt = 2;
            else if((count&0x0000ff)==0x0000ff) ffcnt = 1;
            else ffcnt = 0;

            //  increment count because its encoded (count-1) in the
            //  byte-stream
            count++;

            Xil_unsigned8*  databuf = new unsigned char[count];
            Xil_unsigned8*  tmpbuf = databuf;
            unsigned int    cnt = count;
                
            while (cnt--) {
              *tmpbuf++ = *bp;
              if (*bp == 0xff) {
                if (++ffcnt==7) {
                  bp++;
                  if (*bp != 0x0) {
                    XIL_ERROR(getCis()->getSystemState(),
                                          XIL_ERROR_RESOURCE, "di-97", TRUE);
                    return XIL_FAILURE;
                  }
                            
                  ffcnt=0;
                }
              } else {
                ffcnt = 0;
              }
              bp++;
            }

            cellAttribs.decompressorUserData = new CellUserData;

            if (cellAttribs.decompressorUserData == NULL) {
              XIL_ERROR(getCis()->getSystemState(),
                              XIL_ERROR_RESOURCE, "di-1", TRUE);
              return XIL_FAILURE;
            }
            cellAttribs.decompressorUserData->data   = databuf;
            cellAttribs.decompressorUserData->length = count;
          }
            
          break;
            
        case 0xFF:
          // A Cell frame header starts with CELL_FHEADER_FF_COUNT FF's
          {
          for (int i = 1; i < (int)CELL_FHEADER_FF_COUNT; i++) {

            if (bp[i] != 0xFF) {
              XIL_ERROR( NULL, XIL_ERROR_SYSTEM, "di-97", TRUE);   
              return byteStreamError(bp+i);
            }
          }
          }
          bp += CELL_FHEADER_FF_COUNT;
          while (bp[0] != CELL_FHEADER_END) {
            switch(bp[0]) {
              case CELL_FHEADER_WIDTH_HEIGHT:
                imageWidth  = bp[1]<<8 | bp[2];
                imageHeight = bp[3]<<8 | bp[4];
                bp += 5;
                break;
            
              case CELL_FHEADER_FRAME_RATE:
                // 32-bit unsigned frame rate in microseconds/frame
                cellAttribs.decompressorFrameRate = 
                        bp[1]<<24 | bp[2]<<16 | bp[3]<<8 | bp[4];
                bp += 5;
                break;
            
              case CELL_FHEADER_MAX_CMAP_SIZE:
                // 32-bit unsigned frame rate in microseconds/frame
                cellAttribs.maxDecompressorCmapSize = bp[1] + 1;
                bp += 2;
                break;
            
              default:
                XIL_ERROR( NULL, XIL_ERROR_SYSTEM, "di-97", TRUE);   
                return byteStreamError(bp+1);
            }
          }
          // Since this frame has a header, it must be a key frame
          decompData.frameType = XIL_CIS_CELL_KEY_FRAME;

          bp++;
          break;  // case 0xFF
      }
    }

    updateUserColormapIfPending(decompData, *this);
    
    decompData.headerFrameNumber = cbm->getRFrameId();
    decompData.bp = bp;

    return XIL_SUCCESS;
}
