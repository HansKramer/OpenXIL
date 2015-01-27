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
//  File:   CellCompressorData.cc
//  Project:    XIL
//  Revision:   1.4
//  Last Mod:   10:15:59, 03/10/00
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
#pragma ident   "@(#)CellCompressorData.cc	1.4\t00/03/10  "

#ifndef _WINDOWS
#include "values.h"
#endif
#include "CellCompressorData.hh"

// OK_FUNC(CellCompressorData);


//------------------------------------------------------------------------
//
//  Function(s):
//    CellCompressorData::CellCompressorData
//    CellCompressorData::~CellCompressorData
//    CellCompressorData::initialize
//      CellCompressorData::reset
//      CellCompressorData::initValues
//
//  Description:
//    The constructor and destructor for CellCompressorData.  These
//    setup the compressor values to their default.
//
//      The reset sets all values back to default.
//
//      initValues is the common code between the constructor and the
//      reset function
//    
//    The initialize function is used to appropriately initialize
//    the compressor data when the height/width and any other
//    necessary information is known in order to actually compress
//    images.
//    
//    This class in indended to help differentiate the compressor
//    state data from the decompressor state data.
//    
//  Parameters:
//    CellCompressorData::CellCompressorData
//    CellCompressorData::~CellCompressorData
//        void.
//
//    CellCompressorData::initialize
//        unsigned int width, unsigned int height,
//              Xil_boolean createErrorFrameFlag
//
//  Returns:
//    CellCompressorData::initialize
//        int    XIL_SUCCESS or XIL_FAILURE
//    
//  Side Effects:
//    CellCompressorData::initialize
//        Creates a new CellFrame and resets cellWidth and cellHeight
//    
//------------------------------------------------------------------------

void
CellCompressorData::initValues(void)
{
    isok = FALSE;
    
    cellHeight     = 0;
    cellWidth      = 0;

    hstCellFrame   = NULL;

    for (int i=0; i<CELL_MAX_KEYFRAME_INTERVAL; i++)
      intraFrames[i] = NULL;
    currentFrame   = 0;

    currentImage   = NULL;
    previousImage  = NULL;
    
    ditherMinError = 0.0;
    ditherMinXoX   = 0.0;
    
    upwardThreshold      =  CELL_UPWARD_THRESHOLD;
    downwardThreshold    =  CELL_DOWNWARD_THRESHOLD;

    currentCmapIsSet     = FALSE;

    imageError           = XIL_MAXDOUBLE;
    currentError         = 0.0;
    previousError        = 0.0;
    adaptIsChanging      = FALSE;
    colormapControlMode  = BigChanges;
    numConsecutiveDown   = 0;
    currentRunningTotal  = 0.0;
    previousRunningTotal = 0.0;
    for (i=0; i<CELL_RUNNINGTOTAL_LENGTH; i++) {
      runningTotals[i]  = 0.0;
    }
    currentTotal         = 0;

    currentFrameNumber   = 0;
    initialized          = FALSE;

    for (i=0; i<256; i++)
      entryUsedIndex[i] = FALSE;
    
    errorFrame = NULL;
    inFrame = new InFrame;
    
    if (inFrame == NULL) {
      // TODO : Get system_state here
      XIL_ERROR(NULL, XIL_ERROR_RESOURCE, "di-1", TRUE);
      return;
    }

    if (cmapSelection.ok() == NULL) {
      // TODO : Get system_state here
      XIL_ERROR(NULL, XIL_ERROR_SYSTEM, "di-275", FALSE);
      return;
    }
    
    isok = TRUE;
};

void
CellCompressorData::reset(void)
{
    deleteValues();
    initValues();
};

CellCompressorData::CellCompressorData()
{
    initValues();
};

void
CellCompressorData::deleteValues(void) {

    for (int i=0; i<CELL_MAX_KEYFRAME_INTERVAL; i++) {
       delete intraFrames[i];
       intraFrames[i] = NULL;
    }
    
    delete errorFrame;
    errorFrame = NULL;
    
    delete inFrame;
    inFrame = NULL;

    delete hstCellFrame;
    hstCellFrame = NULL;
    
    currentImage->destroy();
    currentImage = NULL;

    delete [] previousImage;
    previousImage = NULL;
}


CellCompressorData::~CellCompressorData(void) {
    deleteValues();
}

int
CellCompressorData::initialize(unsigned int width,
                               unsigned int height,
                               unsigned int createErrorFrameNumBits) 
{
    //
    //  The width and height are verified in compress
    //
    cellWidth     = width/4;
    cellHeight    = height/4;

    hstCellFrame = new CellFrame(cellWidth, cellHeight);
    if (hstCellFrame == NULL) {
      // out of memory error 
      // TODO : Get system_state here
      XIL_ERROR(NULL, XIL_ERROR_SYSTEM,"di-1",TRUE);
      return XIL_FAILURE;
    }

    if (hstCellFrame->ok() == NULL) {
      // Couldn't create internal Cell compressor object 
      // TODO : Get system_state here
      XIL_ERROR(NULL, XIL_ERROR_SYSTEM,"di-275",FALSE);
      return XIL_FAILURE;
    }

    //
    //  This can be VERY large so we'll only create it when bit-rate control
    //  is turned on.
    //
    if (createErrorFrameNumBits > 0) {
      errorFrame = new ErrorInfoFrame(cellWidth,
                                      cellHeight,
                                      createErrorFrameNumBits);
      if (errorFrame == NULL) {
        XIL_ERROR(NULL, XIL_ERROR_RESOURCE, "di-1", TRUE);
      }

      if (errorFrame->ok() == NULL) {
        // Couldn't create internal Cell compressor object 
        XIL_ERROR(NULL, XIL_ERROR_SYSTEM,"di-275",FALSE);
        return XIL_FAILURE;
      } 
    }
    
    initialized   = TRUE;
    
    return XIL_SUCCESS;
}
