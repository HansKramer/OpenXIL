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
//  File:   CellOutput.cc
//  Project:    XIL
//  Revision:   1.2
//  Last Mod:   10:15:44, 03/10/00
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
#pragma ident   "@(#)CellOutput.cc	1.2\t00/03/10  "

#include "CellOutput.hh"
#include "CellFrame.hh"
#include "CellCompressorData.hh"
#include "CellAttribs.hh"

#include "CmapTable.hh"

// OK_FUNC(CellOutput)

//------------------------------------------------------------------------
//
//  Function:    CellOutput::CellOutput()
//
//  Description:
//    CellOutput contructor/destructor.
//    
//------------------------------------------------------------------------

CellOutput::CellOutput(XilCisBufferManager* cis_mgr, unsigned int iw) 
{
    isok = FALSE;
    cismgr = cis_mgr;
    if (cismgr != NULL) {
      cisbuf = cismgr->nextBuffer();
      if (cisbuf == NULL) {
        //  Unable to perform the specified function
        XIL_ERROR(NULL, XIL_ERROR_SYSTEM, "di-23", FALSE);
        return;
      }
    } else {
      cisbuf = NULL;
    }
        
    cellWidth = iw;
    skipCount = 0;
    runLength = 0;
    runColor  = 0;
    currentX  = 0;
    frameType = XIL_CIS_CELL_NON_KEY_FRAME;
    isok = TRUE;
}
    
CellOutput::~CellOutput(void) 
{
    if (cismgr != NULL)
      cismgr->compressedFrame(frameType);
}


//------------------------------------------------------------------------
//
//  Function:    CellOutput::outputCellFrame
//
//  Description:
//    Output an entire CellFrame's worth of data excluding the colormap.
//    
//------------------------------------------------------------------------
void
CellOutput::outputCellFrame(CellFrame& cframe)
{
    for(int y=0; y<cframe.getFrameHeight(); y++) {
      for(int x=0; x<cframe.getFrameWidth(); x++) {
        outputCell(cframe[y][x]);
      }
      flushRun();
    }
}

//------------------------------------------------------------------------
//
//  Function:    CellOutput::outputCell
//
//  Description:
//    Output a single Cell's worth of data.
//    
//------------------------------------------------------------------------

Xil_boolean
CellOutput::outputCell(Cell cell) 
{
    Xil_boolean flip = FALSE;

    flushSkip();

    if (currentX == cellWidth) {
      flushRun();
      currentX = 0;
    } else if(currentX > cellWidth) {
      XIL_ERROR( NULL, XIL_ERROR_SYSTEM, "di-95", TRUE); 
    }

    currentX++;
    if (runLength > 0) {
      if (cell.C0() == runColor && cell.C1() == runColor) {
        runLength++;
        if (runLength == 0xff)
          flushRun();
        return flip;
      } else {
        flushRun();
      }
    }

    if (cell.C0() == cell.C1()) {
      runLength = 1;
      runColor  = cell.C0();
      return flip;
    } else if(cell.MASK() & 0x8000) {
      cell.flipCell();
      flip = TRUE;
    }

    if (cell.MASK() == 0) {
      cell.C1() = cell.C0();
      cell.C0() = 0;        // make it a run length of 1
    }

    outputShort(cell.MASK());
    outputByte(cell.C1());
    outputByte(cell.C0());

    return flip;
}

//------------------------------------------------------------------------
//
//  Function:    CellOutput::outputColorMap
//
//  Description:
//    Output the entire colormap.
//    
//  Deficiencies/ToDo:
//    A check is necessary here to ensure that eight 0xff codes are not put
//  out in a row which would be confused with a Cell frame header.
//    
//------------------------------------------------------------------------

inline Xil_unsigned8
checkFF(Xil_unsigned8 byte, unsigned int& ffcnt) 
{
    if (byte == 0xff) {
      if (ffcnt++==7) {
        byte--;
        ffcnt = 0;
      }            
    }
    return byte;
}

void
CellOutput::outputColorMap(const CmapTable& cmap)
{
    CmapTable  tmpcmap;

    tmpcmap = cmap;
    tmpcmap.toRGB();

    outputByte(CELL_NEW_COLOR_MAP);
    Xil_unsigned8 entries = char(tmpcmap.getNumEntries() - 1);
    outputByte(entries);
    
    unsigned int   ffcnt = 0;
    if(entries==0xff) ffcnt = 1;

    for(unsigned int i=0; i<tmpcmap.getNumEntries(); i++) {
      outputByte(checkFF(tmpcmap[i].band0(), ffcnt));
      outputByte(checkFF(tmpcmap[i].band1(), ffcnt));
      outputByte(checkFF(tmpcmap[i].band2(), ffcnt));
    }
}

//------------------------------------------------------------------------
//
//  Function:    CellOutput::outputUserData
//
//  Description:
//    Output user data.
//    
//------------------------------------------------------------------------
void
CellOutput::outputUserData(CellUserData* udata)
{
    if (udata && udata->data) {
      outputByte(CELL_USER_DATA);

      Xil_unsigned32 outlen = (udata->length-1);
      outputBytes(((Xil_unsigned8*)&outlen)+1, 3);

      //
      //  Since we're pre-incrementing ffcnt, it should be initialized to -1
      //  when the bottom bytes of the number are not a squence of 0xffs.
      //
      unsigned int   ffcnt;

      if((outlen&0xffffff)==0xffffff) ffcnt = 3;
      else if((outlen&0x00ffff)==0x00ffff) ffcnt = 2;
      else if((outlen&0x0000ff)==0x0000ff) ffcnt = 1;
      else ffcnt = 0;
        
      Xil_unsigned8* d = udata->data;
      Xil_unsigned8* end = udata->data + udata->length;
      while(d<end) {
        outputByte(*d);
        if (*d == 0xff) {
          if (++ffcnt==7) {
            outputByte(0x00);
            ffcnt = 0;
          }
        } else {
          ffcnt = 0;
        }
        d++;
      }
    }
}

//------------------------------------------------------------------------
//
//  Function:    CellOutput::outputFrameHeader
//
//  Description:
//    Output the frame header based upon the different attributes and size
//  of the image.
//    
//------------------------------------------------------------------------
void
CellOutput::outputFrameHeader(unsigned int width,
                              unsigned int height,
                              const CellAttribs& cellAttribs)
{
    for(int i=0; i<(int)CELL_FHEADER_FF_COUNT; i++)
        outputByte(0xff);

    //
    //  Width & Height
    //
    outputByte(CELL_FHEADER_WIDTH_HEIGHT);
    outputShort(width);
    outputShort(height);

    //
    //  Frame Rate
    //
    outputByte(CELL_FHEADER_FRAME_RATE);
    outputShort((cellAttribs.compressorFrameRate)>>16);
    outputShort((cellAttribs.compressorFrameRate)&0x0000ffff);

    //
    //  Max Colormap Size
    //
    if (cellAttribs.maxCompressorCmapSize > 0) {
      outputByte(CELL_FHEADER_MAX_CMAP_SIZE);
     outputByte((cellAttribs.maxCompressorCmapSize - 1)&0x000000ff);
    }
    
    //
    //  Ending Byte
    //
    outputByte(CELL_FHEADER_END);
}


//------------------------------------------------------------------------
//
//  Function:    CellOutput::outputKeyFrame
//
//  Description:
//    Output everything necessary to create a true key frame.
//    
//------------------------------------------------------------------------

void
CellOutput::outputKeyFrame(unsigned int width,
                           unsigned int height,
                           const CellAttribs& cellAttribs,
                           CellFrame& cellFrame)
{
    outputFrameHeader(width, height, cellAttribs);

    CmapTable  cmap;
    cellFrame.getColormap(cmap);
    outputColorMap(cmap);

    if(cellFrame.getUserData())
        outputUserData(cellFrame.getUserData());
    
    outputCellFrame(cellFrame);
    markKeyFrame();
}
    
void
CellOutput::skipEntireFrame(void)
{
    outputByte(CELL_SKIP_ENTIRE_FRAME);
}
    
void
CellOutput::outputByte(Xil_unsigned8 byte)
{
    cisbuf->addByte(byte);
}
    
void
CellOutput::outputBytes(Xil_unsigned8* data, unsigned int len)
{
    cisbuf->addBytes(data, len);
}

void
CellOutput::outputShort(Xil_signed16 shrt)
{
    cisbuf->addShort(shrt);
}
    
int
CellOutput::numBytesOutput(void)
{
    return cisbuf->getNumBytesInWFrame();
}
    
void
CellOutput::markKeyFrame(void)
{
    frameType = XIL_CIS_CELL_KEY_FRAME;
}
    
void
CellOutput::outputSkip(void)
{
    skipCount++;
    if(skipCount == CELL_MAX_SKIP) {
        flushSkip();
    }
}
    
void
CellOutput::outputMaskOnly(unsigned int mask, Xil_boolean flipped)
{
    flushSkip();
    flushRun();
    outputByte(CELL_MASK_ONLY);
    if (flipped == TRUE)
      outputShort(~mask);
    else
      outputShort(mask);
    currentX++;
}
    
void
CellOutput::outputColorsOnly(Xil_unsigned8 color0,
                             Xil_unsigned8 color1,
                             Xil_boolean   flipped)
{
    flushSkip();
    flushRun();
    outputByte(CELL_COLORS_ONLY);
    if (flipped == TRUE) {
      outputByte(color1);
      outputByte(color0);
    } else {
      outputByte(color0);
      outputByte(color1);
    }
    currentX++;
}
    
void
CellOutput::outputColor0Only(Xil_unsigned8 color, Xil_boolean flipped)
{
    flushSkip();
    flushRun();
    if (flipped == TRUE)
      outputByte(CELL_COLOR1_ONLY);
    else
      outputByte(CELL_COLOR0_ONLY);
    outputByte(color);
    currentX++;
}
    
void
CellOutput::outputColor1Only(Xil_unsigned8 color, Xil_boolean flipped)
{
    flushSkip();
    flushRun();
    if (flipped == TRUE)
      outputByte(CELL_COLOR0_ONLY);
    else
      outputByte(CELL_COLOR1_ONLY);
    outputByte(color);
    currentX++;
}
    
void
CellOutput::flushSkip(void)
{
    if (skipCount > 0) {
      flushRun();

      if(currentX == cellWidth)  currentX = 0;

      if (skipCount <= 0 || skipCount > (unsigned int)CELL_MAX_SKIP) {
        // Internal error
        XIL_ERROR( NULL, XIL_ERROR_SYSTEM,"di-95",TRUE);
      }
        
      currentX += skipCount;
        
      if (currentX > cellWidth) {
        // Internal error   
        XIL_ERROR( NULL, XIL_ERROR_SYSTEM,"di-95",TRUE);
      }
        
      skipCount--;
      Xil_unsigned8 code = 0x80 | (skipCount & 0x7f);
      outputByte(code);
      skipCount = 0;
    }
}
    
void
CellOutput::flushRun(void)
{
    if (runLength > 0) {
      outputByte(0);
      outputByte(0);
      outputByte(runColor);
      outputByte(runLength - 1);
      runLength = 0;
    }
}

//------------------------------------------------------------------------
//
//  Function:    CellOutputCounter::CellOutputCounter
//
//  Description:
//    Routines for the output counter class.
//    
//------------------------------------------------------------------------

CellOutputCounter::CellOutputCounter(int iw) :
CellOutput(NULL, iw) 
{
    numbytes = 0;
}

void
CellOutputCounter::outputByte(Xil_unsigned8) 
{
    numbytes++;
}
    
void
CellOutputCounter::outputBytes(Xil_unsigned8*, unsigned int len) 
{
    numbytes += len;
}

void
CellOutputCounter::outputShort(Xil_signed16) 
{
    numbytes+=2;
}
    
int
CellOutputCounter::numBytesOutput(void) 
{
    return numbytes;
}

