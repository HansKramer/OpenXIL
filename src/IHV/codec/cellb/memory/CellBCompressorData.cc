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
//  File:   CellBCompressorData.cc
//  Project:    XIL
//  Revision:   1.2
//  Last Mod:   10:15:27, 03/10/00
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
#pragma ident   "@(#)CellBCompressorData.cc	1.2\t00/03/10  "

#include "CellBCompressorData.hh"
#include "XiliUtils.hh"

// OK_FUNC(CellBCompressorData);

//------------------------------------------------------------------------
//
//  Function(s):
//    CellBCompressorData::CellBCompressorData
//    CellBCompressorData::~CellBCompressorData
//    CellBCompressorData::initialize
//      CellBCompressorData::reset
//      CellBCompressorData::initValues
//  Created:    92/11/11
//
//  Description:
//    The constructor and destructor for CellBCompressorData.  These
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
//    initializeAtomicHistory sets the updateHistory such that it forces
//    every cell to be updated on the next SkipCell call.
//
//    This class is intended to help differentiate the compressor
//    state data from the decompressor state data.
//    
//  Parameters:
//    CellBCompressorData::CellBCompressorData
//    CellBCompressorData::~CellBCompressorData
//    CellBCompressorData::initializeAtomicHistory
//        void.
//
//    CellBCompressorData::initialize
//        unsigned short width, unsigned short height,
//              Xil_boolean createErrorFrameFlag
//
//  Returns:
//    CellBCompressorData::initialize
//        int    XIL_SUCCESS or XIL_FAILURE
//    
//  Side Effects:
//    CellBCompressorData::initialize
//        resets cellWidth and cellHeight
//
//    CellBCompressorData::initializeAtomicHistory
//        increments validHistory
//        sets atomicHistory to validHistory
//    
//------------------------------------------------------------------------

void
CellBCompressorData::initValues(void)
{
    isOKFlag = FALSE;
    
    cellHeight     = 0;
    cellWidth      = 0;

    currentImage   = NULL;
    //    previousImage  = NULL;

    updateHistory = NULL;
    cellHistory = NULL;

    initialized          = FALSE;

    isOKFlag = TRUE;
};

void
CellBCompressorData::reset(void)
{
    deleteValues();
    initValues();
};

CellBCompressorData::CellBCompressorData()
{
    initValues();
};

void
CellBCompressorData::deleteValues(void) 
{

    currentImage->destroy();
    currentImage = NULL;

    //    previousImage->destroy();
    //    previousImage = NULL;

    delete cellHistory;
    delete updateHistory;
}

void 
CellBCompressorData::initializeAtomicHistory() 
{
    // initialize to 0
    int size = cellWidth * cellHeight;
    xili_memset(updateHistory, 0, sizeof(int) * size);
    atomicHistory = initializeOtherHistory();
}


CellBCompressorData::~CellBCompressorData(void) {
    deleteValues();
}

XilStatus
CellBCompressorData::initialize(unsigned int width,
                                unsigned int height) 
{
    int size;

    //
    //  The width and height are verified in compress_CellB
    //
    cellWidth     = width/4;
    cellHeight    = height/4;
    size = cellWidth * cellHeight;

    cellHistory   = new Xil_unsigned32[size];
    if (cellHistory== NULL) {
      XIL_ERROR(NULL, XIL_ERROR_RESOURCE, "di-1", TRUE);
      return XIL_FAILURE;
    }
    // initialize to an illegal value
    xili_memset(cellHistory, 0xff, sizeof(int) * size);

    updateHistory = new int[size];
    if (updateHistory== NULL) {
      XIL_ERROR(NULL, XIL_ERROR_RESOURCE, "di-1", TRUE);
      return XIL_FAILURE;
    }
    validHistory = 0;
    atomicHistory = -1;
    initialized   = TRUE;
    
    return XIL_SUCCESS;
}


