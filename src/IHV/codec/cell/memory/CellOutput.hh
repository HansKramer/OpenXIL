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
//  File:   CellOutput.hh
//  Project:    XIL
//  Revision:   1.2
//  Last Mod:   10:23:28, 03/10/00
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
#pragma ident   "@(#)CellOutput.hh	1.2\t00/03/10  "

#ifndef CELLOUTPUT_HH
#define CELLOUTPUT_HH

#include <xil/xilGPI.hh>
#include "CellDefines.hh"

class CellOutput {
    
public:
    CellOutput(XilCisBufferManager* cis_mgr, unsigned int iw);
    ~CellOutput(void);

    Xil_boolean  outputCell(Cell cell);
    void         outputCellFrame(CellFrame& cframe);
    void         outputKeyFrame(unsigned int width,
                                unsigned int height,
                                const CellAttribs& cellAttribs,
                                CellFrame& cellFrame);
    void         outputColorMap(const CmapTable& cmap);
    void         outputUserData(CellUserData* udata);
    void         outputFrameHeader(unsigned int width,
                                   unsigned int height,
                                   const CellAttribs& cellattribs);

    virtual void  outputByte(Xil_unsigned8 byte);
    virtual void  outputBytes(Xil_unsigned8* data, unsigned int len);
    virtual void  outputShort(Xil_signed16 shrt);
    virtual int   numBytesOutput(void);
    
    void  markKeyFrame(void);
    void  skipEntireFrame(void);
    
    void  outputSkip(void);
    void  outputMaskOnly(unsigned int mask, Xil_boolean flipped);

    void  outputColorsOnly(Xil_unsigned8 color0,
                           Xil_unsigned8 color1,
                           Xil_boolean   flipped);

    void  outputColor0Only(Xil_unsigned8 color, Xil_boolean flipped);
    void  outputColor1Only(Xil_unsigned8 color, Xil_boolean flipped);
    void  flushSkip(void);    
    void  flushRun(void);
    
private:
    XilCisBufferManager* cismgr;
    XilCisBuffer*        cisbuf;
    
    int   skipCount;
    int   runLength;
    int   runColor;
    int   currentX;
    int   cellWidth;
    int   frameType;
    Xil_boolean isok;
};

class CellOutputCounter : public CellOutput {

public:
    CellOutputCounter(int iw);
    
    void  outputByte(Xil_unsigned8);
    void  outputBytes(Xil_unsigned8*, unsigned int len);
    void  outputShort(Xil_signed16);
    int   numBytesOutput(void);
    
    CellOutputCounter*  ok(Xil_boolean destroy = TRUE);

private:
    int  numbytes;
    Xil_boolean isok;
};

#endif  // CELLOUTPUT_HH
