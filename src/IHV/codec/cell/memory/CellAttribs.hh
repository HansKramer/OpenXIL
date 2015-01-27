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
//  File:   CellAttribs.hh
//  Project:    XIL
//  Revision:   1.5
//  Last Mod:   10:23:31, 03/10/00
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
#pragma ident   "@(#)CellAttribs.hh	1.5\t00/03/10  "

#ifndef CELLATTRIBS_HH
#define CELLATTRIBS_HH

#include <xil/xilGPI.hh>
#include "CellFrame.hh"

class CellAttribs {
public:
    int                  version;
    Xil_boolean          colorMapAdaption;
    Xil_boolean          temporalFiltering;
    XilCellEncodingType  encodingType;
    UsedCmapTable*       compressorCmap;
    int                  maxCompressorCmapSize;
    int                  maxDecompressorCmapSize;
    int                  lowFilterThreshold;
    int                  highFilterThreshold;
    int                  keyFrameInterval;
    XilLookupSingle*     decompressorCmap;
    Xil_unsigned32       compressorFrameRate;    // in microseconds/frame
    Xil_unsigned32       decompressorFrameRate;  // in microseconds/frame
    int                  bitsPerSecond;
    int                  bytesPerFrameGroup;
    CellUserData*        compressorUserData;
    CellUserData*        decompressorUserData;

    CellAttribs(void);
    ~CellAttribs(void);
    
    void                 reset();
};

#endif // CELLATTRIBS_HH
