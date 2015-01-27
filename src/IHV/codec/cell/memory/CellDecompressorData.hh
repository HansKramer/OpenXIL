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
//  File:   CellDecompressorData.hh
//  Project:    XIL
//  Revision:   1.3
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
#pragma ident   "@(#)CellDecompressorData.hh	1.3\t00/03/10  "

#ifndef CELLDECOMPRESSORDATA_HH
#define CELLDECOMPRESSORDATA_HH

#include <xil/xilGPI.hh>
#include "CellFrame.hh"

#define INCL_DGA

class CellDecompressorData {
public:
    //  Constructor
    CellDecompressorData(XilSystemState*);
    
    //  Destructor
    ~CellDecompressorData(void);

    CellDecompressorData* ok(Xil_boolean destroy = TRUE);

    int  initialize(XilSystemState* systemState,
                    unsigned int    imageWidth,
                    unsigned int    imageHeight);

    void reset(void);

    // the current colormap as defined in the bytestream
    XilLookupSingle*  colormap;
    int               colormapEntries;
    
    // Flags controlling update of the user's colormap as specified with
    // the DECOMPRESSOR_COLORMAP attribute.
    int               updateUserColormapPending;
    int               updateUserColormapEnabled;

    //  Array of Cells[height/4][width/4]
    CellFrame*        cellFrame;
    
    Xil_unsigned8*    rdwrIndices;
    int               numRdwrIndices;

    //  Remap Array
    Xil_unsigned8*    remap;
    Xil_unsigned32*   remapExp;

    //  Ordered dither tables
    Xil_unsigned32*   dith_multi_table;

    //
    // Current object versions. These need to be
    // checked to determine if new tables need to be built.
    //
    XilVersion currentLookupVersion;
    XilVersion currentBytestreamCmapVersion;
    XilVersion currentCubeVersion;
    XilVersion currentDmaskVersion;
    
    //  Byte-stream pointer
    Xil_unsigned8*    bp;
    
    // Frame type for CIS buffer manager
    int               frameType;

    // Frame number of most recently read header
    int               headerFrameNumber;

    // Set after seek as a signal to the decompression routines
    int               redrawNeeded;

    //  y2bp array used for GX decoding
    Xil_unsigned8**   y2bpArray;

    //  dga_win number for knowing to redisplay the whole decompressed image
    //  in the GX molecules.
#ifdef HAS_LIBDGA 
    Dga_window        lastDGA_Win;
#else
    int*              sizePlaceHolder;
#endif
    
private:
    Xil_boolean isok;
};

#endif  // CELLDECOMPRESSORDATA_HH
