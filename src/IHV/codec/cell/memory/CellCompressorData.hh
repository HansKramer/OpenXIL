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
//  File:   CellCompressorData.hh
//  Project:    XIL
//  Revision:   1.2
//  Last Mod:   10:23:33, 03/10/00
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
#pragma ident   "@(#)CellCompressorData.hh	1.2\t00/03/10  "

#ifndef CELLCOMPRESSORDATA_HH
#define CELLCOMPRESSORDATA_HH

#include <xil/xilGPI.hh>

#include "CellFrame.hh"
#include "CellOutput.hh"
#include "AdaptiveColormapSelection.hh"
#include "InFrame.hh"

const unsigned int  CELL_RUNNINGTOTAL_LENGTH = 4;

enum XilCmapModes {
    BigChanges, WaitingToSettle, Settled
};

class CellCompressorData {
public:
    //
    //  Constructor
    //
    CellCompressorData();
    
    //
    //  Destructor
    //
    ~CellCompressorData(void);
    
    //
    //  Initializer
    //
    int initialize(unsigned int width,
                   unsigned int height,
                   unsigned int createErrorFrameNumBits);
    
    void initValues();
    void deleteValues();
    void reset();

    CellCompressorData* ok(Xil_boolean destroy = TRUE);
    
    //
    //  The current image to filter and compress
    //    and the previous image for filtering.
    //
    XilImage*     currentImage;
    Xil_unsigned8*     previousImage;
    
    //
    //  ColorMap with used flags for adaptive colormap information and
    //        luminance -- for ordering Cells
    //
    AdaptiveColormapSelection   cmapSelection;
    LumCmapTable     currentCmap;
    LumCmapTable     nextCmap;
    Xil_boolean      currentCmapIsSet;
    Xil_boolean      entryUsedIndex[256];
    
    //
    //  encodeDither state
    //
    float            ditherMinError;
    float            ditherMinXoX;
    
    //
    //  Encoding colormap decision attributes
    //
    Xil_boolean      adaptIsChanging;
    XilCmapModes     colormapControlMode;
    unsigned int     numConsecutiveDown;
    double           imageError;
    double           currentError;
    double           previousError;
    double           currentRunningTotal;
    double           previousRunningTotal;
    double           runningTotals[CELL_RUNNINGTOTAL_LENGTH];
    unsigned int     currentTotal;

    //
    //  Cell Frame Size
    //
    unsigned int     cellHeight, cellWidth;

    //
    //  CellFrame for non-bitrate controlled inter-frame compression.
    //
    CellFrame*       hstCellFrame;
    
    //
    //  An array of CellFrame pointers for buffered bit-rate controlled
    //  inter-frame compression.
    //
    CellFrame*       intraFrames[CELL_MAX_KEYFRAME_INTERVAL];
    int              currentFrame;
    
    //
    //  Count every frame
    //
    int      currentFrameNumber;   //  count

    //
    //  Error Table
    //
    ErrorInfoFrame*  errorFrame;
    
    //
    //  More attributes
    //
    double           upwardThreshold, downwardThreshold;
    
    //
    //  Flag for if I've been initialized
    //
    Xil_boolean      initialized;

    //
    //  Input Management Class for Images
    //
    InFrame*         inFrame;

private:
    Xil_boolean      isok;
};

#endif  // CELLCOMPRESSORDATA_HH
