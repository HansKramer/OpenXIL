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
//  File:   CellAttribs.cc
//  Project:    XIL
//  Revision:   1.2
//  Last Mod:   10:16:02, 03/10/00
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
#pragma ident   "@(#)CellAttribs.cc	1.2\t00/03/10  "

#include "CellAttribs.hh"

void
CellAttribs::reset()
{
    version                  = 0;
    colorMapAdaption         = TRUE;
    temporalFiltering        = TRUE;
    encodingType             = BTC;
        
    delete compressorCmap;
    compressorCmap           = NULL;
    maxCompressorCmapSize    = -1; // No MaxCmapSize will be put out unless set
    maxDecompressorCmapSize  = CELL_MAX_CMAP_SIZE;
    lowFilterThreshold       = CELL_LOW_FILTER_THRESHOLD;
    highFilterThreshold      = CELL_HIGH_FILTER_THRESHOLD;
    keyFrameInterval         = CELL_DEFAULT_KEYFRAME_INTERVAL;
    decompressorCmap         = NULL;
    compressorFrameRate      = CELL_DEFAULT_FRAMERATE;
    decompressorFrameRate    = CELL_DEFAULT_FRAMERATE;
    bitsPerSecond            = 0; // No bit-rate control
    bytesPerFrameGroup       = 0;
    compressorUserData       = NULL;
    decompressorUserData     = NULL;
}

CellAttribs::CellAttribs(void) {
    compressorCmap = NULL; reset();
}

CellAttribs::~CellAttribs(void)
{
    delete compressorCmap;
    compressorCmap = NULL;
}
