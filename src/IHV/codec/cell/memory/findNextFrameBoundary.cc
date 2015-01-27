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
//  File:   findNextFrameBoundary.cc
//  Project:    XIL
//  Revision:   1.2
//  Last Mod:   10:15:57, 03/10/00
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
#pragma ident   "@(#)findNextFrameBoundary.cc	1.2\t00/03/10  "

#include "XilDeviceCompressionCell.hh"
#include "string.h"

//------------------------------------------------------------------------
//
//  Function:    XilDeviceCompressionCell::findNextFrameBoundary
//
//  Description:
//    Fast scan of the Cell bytestream to find the next frame boundary.
//    
//  Returns:
//    XIL_SUCCESS if a frame boundary found.
//    XIL_FAILURE if getNextByte returned a NULL before we reached a frame
//    boundary, if a bytestream error was detected, or if
//    foundNextFrameBoundary returned XIL_FAILURE.
//    
//  Notes:
//    We may have to traverse multiple "chunks" of data to get to the next
//    frame boundary.
//
//    Since a Cell code might straddle two "chunks" of data, we only get a
//    byte of data at a time by calling getNextByte.  If this were re-coded
//    to use getNextBytes, the code would be very complicated, and probably
//    slower anyway.
//    
//  Deficiencies/ToDo:
//    This first implementation is very slow, it needs to be sped
//    up before FCS.  Here's an idea about how to do this:
//
//    Have a fast version of the main loop which is used when nbytes is
//    >= 4 and a slow version (essentially the current loop) when it is
//    is 3 or less that uses a local get next byte function
//    which would be a layer on top of getNextBytes.
//------------------------------------------------------------------------

#define GET_NEXT_BYTE_OR_FAIL \
    if ((bp = cbm->getNextByte()) == NULL) return XIL_FAILURE;

#define VFF_BUF_SIZE 70 // see code for explanation

int
XilDeviceCompressionCell::findNextFrameBoundary()
{
  Xil_unsigned8* bp;
  int tmp;

  // fprintf(stderr, "in findNextFrameBoundary\n");
  // get the first byte

  GET_NEXT_BYTE_OR_FAIL;

  // fprintf(stderr, "bp = %08x, *bp = %x\n", bp, *bp);

  // If this is the very first frame, check for a VFF header (for backward
  // compatibility with old Cell movies).  I'm intentionally testing *bp
  // first for speed.
  if (*bp == 'n' && cbm->getRFrameId() == 0) {
    // This must be a VFF header from an old Cell movie file
    Xil_unsigned8 buf[VFF_BUF_SIZE + 1];
    Xil_unsigned8* bufp = buf;

    // Get enough bytes to be sure that we've got the "size=" and "frames="
    // keywords but not too many so that we're beyond the VFF header.
    // Inspection of a few old Cell movies shows that 70 bytes is about
    // right.
    for (int i = VFF_BUF_SIZE; i > 0; i--) {
      GET_NEXT_BYTE_OR_FAIL;
      *bufp++ = *bp;
    }

    // Simulate the end of the VFF header to guard against infinite loop below.
    *bufp = '\f';

    bufp = buf;
    if (strncmp((const char *)bufp, "ncaa", 4) != 0) {
      // The address passed here to byteStreamError is actually
      // a ways beyond where the error was detected, but oh well.
      // Same problem for other byteStreamError calls while we're
      // in the VFF header.

      // found 'n' but not 'ncaa'
      XIL_ERROR( NULL, XIL_ERROR_SYSTEM, "di-97", TRUE);
      byteStreamError(bp);
      return XIL_FAILURE;
    }
    bufp += 4;
    
    // get the image dimensions from the "size=" keyword
    do {
      bufp++;

      if (*bufp == '\f') {
        // VFF: missing 'size='
        XIL_ERROR( NULL, XIL_ERROR_SYSTEM, "di-97", TRUE); 

        byteStreamError(bp);
        return XIL_FAILURE;
      }
    } while (strncmp((const char *)bufp, "size=", 5) != 0);
    bufp += 5;

    // imageWidth & imageHeight are unsigned int, note the 'h' modifier
    int assert_fail=FALSE;
    if ( assert_fail ||
         (sscanf((const char *)bufp, "%hd %hd", &imageWidth, &imageHeight) 
         != 2) ) {

      // bad width & height in VFF header"
      XIL_ERROR( NULL, XIL_ERROR_SYSTEM, "di-97", TRUE);  

      byteStreamError(bp);
      return XIL_FAILURE;
    }
    
    // look for "frames="
    do {
      bufp++;
      if (*bufp == '\f') {

        // VFF: missing 'frames='
        XIL_ERROR( NULL, XIL_ERROR_SYSTEM, "di-97", TRUE);   

        byteStreamError(bp);
        return XIL_FAILURE;
      }
    } while (strncmp((const char *)bufp, "frames=", 7) != 0);
    bufp += 7;

    int frames;
    if ( assert_fail ||
         (sscanf((const char *)bufp, "%d", &frames) != 1) ) {

      // bad frame count in VFF header
      XIL_ERROR( NULL, XIL_ERROR_SYSTEM, "di-97", TRUE);

      byteStreamError(bp);
      return XIL_FAILURE;
    }
    
    // Scan to the end of the VFF header, which is a form-feed followed
    // by a new-line.  Note that we just discard the rest of 'buf' and
    // start gettings bytes from the CIS buffer manager again.
    do {
      GET_NEXT_BYTE_OR_FAIL;
    } while (*bp != '\f');
    GET_NEXT_BYTE_OR_FAIL;

    if (*bp != '\n') {
      // VFF: missing newline at end"
      XIL_ERROR( NULL, XIL_ERROR_SYSTEM, "di-97", TRUE);

      byteStreamError(bp);
      return XIL_FAILURE;
    }
    GET_NEXT_BYTE_OR_FAIL;
    // bp now points to the first byte of the frame index

    for (i = frames * 8; i > 0; i--) {      // 8 = sizeof(MBlock)
      GET_NEXT_BYTE_OR_FAIL;
    }
    
  }

  // check for frame header
  if (*bp == 0xFF) {
    for (int count = CELL_FHEADER_FF_COUNT; count > 0; count--) {
      // We could be checking these bytes to see if they're 0xFF, but
      // this would slow us down, and if there is a bytesream error, we'll
      // certainly catch it later anyway.
      GET_NEXT_BYTE_OR_FAIL;
    }
    // bp now points to the first byte after the 0xFF's

    while (*bp != CELL_FHEADER_END) {
      switch (*bp) {
        case CELL_FHEADER_WIDTH_HEIGHT:
          if (imageHeight == 0) {
            // The image dimensions are unknown at this point.  This can result
            // from, for example, the sequence [putBitsPtr, hasFrame].  So we
            // must derive the dimensions from the first frame header.
            //
            // We cannot call deriveOutputType because that calls
            // decompressHeader which calls nextFrame, and that would
            // confuse the CIS buffer manager.
            GET_NEXT_BYTE_OR_FAIL;
            tmp = *bp;
            GET_NEXT_BYTE_OR_FAIL;
            imageWidth = (tmp << 8) + *bp;
            GET_NEXT_BYTE_OR_FAIL;
            tmp = *bp;
            GET_NEXT_BYTE_OR_FAIL;
            imageHeight = (tmp << 8) + *bp;
          } else {
            GET_NEXT_BYTE_OR_FAIL;
            GET_NEXT_BYTE_OR_FAIL;
            GET_NEXT_BYTE_OR_FAIL;
            GET_NEXT_BYTE_OR_FAIL;
          }
          break;

        case CELL_FHEADER_FRAME_RATE:
          GET_NEXT_BYTE_OR_FAIL;
          GET_NEXT_BYTE_OR_FAIL;
          GET_NEXT_BYTE_OR_FAIL;
          // no break;
        case CELL_FHEADER_MAX_CMAP_SIZE:
          GET_NEXT_BYTE_OR_FAIL;
          break;
    
        default:
          // bad Cell frame header code

          byteStreamError(bp + 1);
          return XIL_FAILURE;
      }
      GET_NEXT_BYTE_OR_FAIL;
      // bp now points to the next CELL_FHEADER code
    }
    GET_NEXT_BYTE_OR_FAIL;
    // bp now points to the byte after the Cell frame header
  }
  
  // check for colormap
  if (*bp == CELL_NEW_COLOR_MAP) {
    GET_NEXT_BYTE_OR_FAIL;
    // bp now points to the byte containing (length - 1)
    for (int length = *bp + 1; length > 0; length--) {
      // skip 3 bytes (RGB) for each colormap entry
      GET_NEXT_BYTE_OR_FAIL;
      GET_NEXT_BYTE_OR_FAIL;
      GET_NEXT_BYTE_OR_FAIL;
    }
    GET_NEXT_BYTE_OR_FAIL;
    // bp now points to the byte after the colormap
  }

  // check for userData
  if (*bp == CELL_USER_DATA) {
    //
    //  Read the values directly from the stream.  The count is
    //  encoded in the byte-stream as "count-1" so we add 1 to it in
    //  order to arrive at the proper count.
    //
    GET_NEXT_BYTE_OR_FAIL;
    int bp1 = *bp;
    GET_NEXT_BYTE_OR_FAIL;
    int bp2 = *bp;
    GET_NEXT_BYTE_OR_FAIL;
    int bp3 = *bp;
    GET_NEXT_BYTE_OR_FAIL;
      
    int count = ((bp1<<16)|(bp2<<8)|bp3);
            
    {
      unsigned int    ffcnt;
                
      if((count&0xffffff)==0xffffff) ffcnt = 3;
      else if((count&0x00ffff)==0x00ffff) ffcnt = 2;
      else if((count&0x0000ff)==0x0000ff) ffcnt = 1;
      else ffcnt = 0;

      //  increment count because its encoded (count-1) in the
      //  byte-stream
      count++;

      while(count--) {
        if (*bp == 0xff) {
          if (++ffcnt==7) {
            GET_NEXT_BYTE_OR_FAIL;
            if (*bp != 0x0) {
              XIL_OBJ_ERROR(getCis()->getSystemState(),
                            XIL_ERROR_SYSTEM, "di-97", TRUE, getCis());
              return XIL_FAILURE;
            }
            ffcnt=0;
          }
        } else {
          ffcnt = 0;
        }
        GET_NEXT_BYTE_OR_FAIL;
      }
    }
  }

  // Instead of counting by rows & columns separately, we'll just count
  // total cells.  Although it's not legal for a runlength or skip code to
  // wrap past the end of a row, we'll check for that error later when
  // we actually decompress.
  int cell_count = (int)(imageHeight * imageWidth) >> 4; // 16 pixels per cell

  if (cell_count <= 0) {
    // unknown Cell image dimensions
    XIL_ERROR( NULL, XIL_ERROR_SYSTEM, "di-97", TRUE);
    byteStreamError(bp + 1);
    return XIL_FAILURE;
  }

  unsigned int code;

  while (1) {
    code = *bp; // code = first byte of the code
    if ((code & 0x80) == 0) {
      // this is either a regular 4x4 Cell or a runlength code
      GET_NEXT_BYTE_OR_FAIL;
      code = (code << 8) | *bp; // code = first two bytes of the code
      GET_NEXT_BYTE_OR_FAIL;  // skip the third byte
      GET_NEXT_BYTE_OR_FAIL;  // bp now points to the fourth byte
      if ((code) != 0) {
        // this is a regular 4x4 Cell
        cell_count--;
      } else {
        // this is a runlength code, the fourth byte is (length - 1)
        cell_count -= *bp + 1;
      }
    } else {
      // this must be an escape code
      if ((code & 0x40) == 0) {
        // 10NNNNNN = interframe skip N+1 blocks
        cell_count -= (code & 0x3F) + 1;
      } else {
        switch (code) {
          case CELL_COLORS_ONLY:
          case CELL_MASK_ONLY:
            GET_NEXT_BYTE_OR_FAIL;  // second byte
            // no break;
          case CELL_COLOR0_ONLY:
          case CELL_COLOR1_ONLY:
            GET_NEXT_BYTE_OR_FAIL;  // second or third byte
            break;
          default:
            // bad Cell escape code
            XIL_ERROR( NULL, XIL_ERROR_SYSTEM, "di-97", TRUE); 
            byteStreamError(bp + 1);
            return XIL_FAILURE;
        }
        cell_count--;
      }
    }

    if (cell_count > 0) {
      GET_NEXT_BYTE_OR_FAIL;
    } else if (cell_count == 0) {
      return cbm->foundNextFrameBoundary(bp + 1);
    } else {
      // The cell_count is negative.  This could happen if a runlength
      // or interframe skip code ran past the end of the last row.
      // The error could have occurred earlier in this frame, or even
      // in an earlier frame; there is no way to be sure.

      // bad runlength or interframe skip code");
      XIL_ERROR( NULL, XIL_ERROR_SYSTEM, "di-97", TRUE);  

      byteStreamError(bp + 1);
      return XIL_FAILURE;
    }
  }
}
