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
//  File:   CellDefines.hh
//  Project:    XIL
//  Revision:   1.2
//  Last Mod:   10:23:26, 03/10/00
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
#pragma ident   "@(#)CellDefines.hh	1.2\t00/03/10  "

#ifndef  CELLDEFINES_HH
#define  CELLDEFINES_HH

#include <xil/xilGPI.hh>

//
//  These are the special Cell byte-stream codes used for both
//  encoding and decoding the byte-stream
//
const Xil_unsigned8   CELL_MAX_SKIP          = 64;
const Xil_unsigned8   CELL_NEW_COLOR_MAP     = 0xc0;
const Xil_unsigned8   CELL_COLORS_ONLY       = 0xc2;
const Xil_unsigned8   CELL_COLOR0_ONLY       = 0xc3;
const Xil_unsigned8   CELL_COLOR1_ONLY       = 0xc4;
const Xil_unsigned8   CELL_MASK_ONLY         = 0xc5;
const Xil_unsigned8   CELL_USER_DATA         = 0xc6;
const Xil_unsigned8   CELL_SKIP_ENTIRE_FRAME = 0xc7;

// The constant FF_COUNT is the number of consecutive bytes of FF at the
// beginning of a frame header.  This value is 8 because the highest number
// of consecutive bytes of FF that could otherwise appear in the bytestream
// is 7 (the colormap sequence CYAN, WHITE, YELLOW).
const Xil_unsigned8   CELL_FHEADER_FF_COUNT       = 8;
const Xil_unsigned8   CELL_FHEADER_END            = 0x00;
const Xil_unsigned8   CELL_FHEADER_WIDTH_HEIGHT   = 0x01;
const Xil_unsigned8   CELL_FHEADER_FRAME_RATE     = 0x02;
const Xil_unsigned8   CELL_FHEADER_MAX_CMAP_SIZE  = 0x03;

// These are used to indicate frame type to the cbm
const Xil_unsigned8   XIL_CIS_CELL_NON_KEY_FRAME = 0;
const Xil_unsigned8   XIL_CIS_CELL_KEY_FRAME     = 1;

//
//  The Default size of a chosen colormap
//
const Xil_unsigned8   CELL_COMPRESSOR_DEFAULT_CMAP_SIZE = 216;

//
//  The Maximum size of a Cell colormap
//
const unsigned int    CELL_MAX_CMAP_SIZE = 256;

//
//  The size of some Cell dither tables
//
const unsigned int    CELL_DITHER_NBANDS  =   3;
const unsigned int    CELL_DITHER_MAXVALS = 256;

//
//  The Default FrameRate
//
const unsigned int    CELL_DEFAULT_FRAMERATE  = 33333;

//
//  The Default Color Similarity and Bits Similarity
//
const unsigned int    CELL_DEFAULT_COLOR_SIMILARITY = 32;
const unsigned int    CELL_DEFAULT_BITS_SIMILARITY  =  2;

//
//  The Keyframe Intervals
//
const unsigned int    CELL_DEFAULT_KEYFRAME_INTERVAL           = 6;
const unsigned int    CELL_MAX_KEYFRAME_INTERVAL               = 512;
const unsigned int    CELL_MAX_KEYFRAME_INTERVAL_WITH_BIT_RATE = 15;

//
//  The temporal filtering thresholds.
//
const unsigned int    CELL_LOW_FILTER_THRESHOLD   = 16;
const unsigned int    CELL_HIGH_FILTER_THRESHOLD  = 32;

//
//  Adaptive Colormap encoding thresholds.
//
const unsigned int    CELL_UPWARD_THRESHOLD    = 10;
const unsigned int    CELL_DOWNWARD_THRESHOLD  = 5;

//
//  The largest number of user data bytes accepted.
//
//  For XIL 1.0, this will be limited to 8k in order to keep the current
//  CisBufferManager working.  At some future time, this should be increased
//  to the full 16Mb.
//
const unsigned int    CELL_MAX_USER_DATA_SIZE  = 8192;

//
//  This defines the size of each buffer in the Cis Buffer Manager.
//  I have chosen 10 because the difference between the average size
//  per frame and the largest frame is very large with Cell.  Thus,
//  each buffer will hold 10 max frames which is a fairly large buffer
//  space since most frames will be much smaller than the maximum
//  size.
//
const int             FRAMES_PER_BUFFER   = 10;

//
//  Forward declaration of Cell Classes
//
class Cell;
class CellFrame;
class CellUserData;
class CellOutput;
class CellCompressorData;
class CellDecompressorData;
class CellAttribs;
class CmapTable;
class ColorValue;
class XilDeviceCompressionCell;
class XilDeviceManagerCompressionCell;

#endif  // CELLDEFINES_HH
