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
//  File:   CellBDecompressorData.hh
//  Project:    XIL
//  Revision:   1.3
//  Last Mod:   10:23:18, 03/10/00
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
#pragma ident   "@(#)CellBDecompressorData.hh	1.3\t00/03/10  "

#ifndef CELLBDECOMPRESSORDATA_HH
#define CELLBDECOMPRESSORDATA_HH

#include <xil/xilGPI.hh>

#include "CellBDefines.hh"
#include "CellBFrame.hh"
#include "CellBManagerCompressorData.hh"
#include "CellBHistoryImage.hh"

enum histImageEnum {I8, IRGB, IZoom8, IAny, HISTIMAGEMAX};


class CellBDecompressorData {
public:
    //
    //  constructor, destructor, & their component functions
    //
    CellBDecompressorData(CellBManagerCompressorData* ct);

    ~CellBDecompressorData(void);

    void initValues();
    void deleteValues();
    void reset();

    Xil_boolean ignoreHistory;
    
    CellBDecompressorData* ok(Xil_boolean destroy = TRUE);

    // These are currently initialized to point to a constant table.
    Xil_unsigned16*   yytable;
    Xil_unsigned16*   uvtable;

    // These are used by the dither molecules.
    Xil_unsigned32    yydither[MAX_BYTE_VAL][4];
    Xil_unsigned32    uvdither[UVTABLE_SIZE][4];

    CellBFrame*       getCellBFrame(unsigned int, unsigned int);

    // pass in the width, height, and frame number. Return the buffer and
    // whether or not it is currently up-to-date. Note that we could use
    // just the single routine, getHistImage. These others are here for
    // backwards compatibility and to emphasize that these have their own
    // buffers.  Also note that separate buffers are unnecessary.  However,
    // they can speed things up if you are switching between two types of
    // decompression frequently.

    CellBHistoryImage* getHistImage8(unsigned int w,
                                     unsigned int h,
                                     unsigned int f)
    { 
       return getHistImage(I8, w, h, f, 1, 1, 0); 
    }

    CellBHistoryImage* getHistZoomImage8(unsigned int w,
                                         unsigned int h,
                                         unsigned int f)
    {
       return getHistImage(IZoom8, w << 1, h << 1, f, 1, 1, 0); 
    }

    CellBHistoryImage* getHistImageRGB(unsigned int w,
                                       unsigned int h,
                                       unsigned int f)
    {
       return getHistImage(IRGB, w, h, f, 3, 3, 0);
    }

    CellBHistoryImage* getHistImageBGRX(unsigned int w,
                                        unsigned int h,
                                        unsigned int f)
    {
       return getHistImage(IAny, w, h, f, 3 , 4, 0); 
    }

    CellBHistoryImage* getHistImageXBGR(unsigned int w,
                                        unsigned int h,
                                        unsigned int f)
    {
       return getHistImage(IAny, w, h, f, 3, 4, 1); 
    }

    int ditherInit(XilLookupColorcube*, XilDitherMask*, float*, float*);

    int checkRescaleAndOffset(float* r, float* o) 
    {
       return (r[0] == rescaleVals[0] &&
               r[1] == rescaleVals[1] &&
               r[2] == rescaleVals[2] &&
               o[0] == offsetVals[0]  &&
               o[1] == offsetVals[1]  &&
               o[2] == offsetVals[2]  );
    }

    void storeRescaleAndOffset(float* r, float* o) 
    {
       rescaleVals[0] = r[0];
       rescaleVals[1] = r[1];
       rescaleVals[2] = r[2];
       offsetVals[0]  = o[0];
       offsetVals[1]  = o[1];
       offsetVals[2]  = o[2];
    }

    
    CellBHistoryImage*        getHistImage(histImageEnum imagenum,
                                           unsigned int w,
                                           unsigned int h,
                                           unsigned int f,
                                           unsigned int nbands, 
                                           unsigned int parent_bands,
                                           unsigned int band_offset);

private:
    Xil_boolean     isOKFlag;
    CellBFrame*     cellBFrame;

    CellBHistoryImage*        histImages[HISTIMAGEMAX];

    float           rescaleVals[3]; // these don't need to be initialized...
    float           offsetVals[3];  // ...in the constructor
};

#endif  // CELLBDECOMPRESSORDATA_HH
