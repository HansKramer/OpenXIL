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
//  File:   XilDeviceCompressionCell.hh
//  Project:    XIL
//  Revision:   1.7
//  Last Mod:   10:23:30, 03/10/00
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
#pragma ident   "@(#)XilDeviceCompressionCell.hh	1.7\t00/03/10  "

#ifndef XIL_DEVICE_COMPRESSION_CELL_HH
#define XIL_DEVICE_COMPRESSION_CELL_HH

#include <xil/xilGPI.hh>

#include "CellDefines.hh"
#include "CellCompressorData.hh"
#include "CellDecompressorData.hh"
#include "CellAttribs.hh"
#include "CellFrame.hh"
#include "ColorValue.hh"
#include "XiliUtils.hh"
#include "CompressInfo.hh"
#include "DecompressInfo.hh"

class    XilDeviceCompressionCell : public XilDeviceCompression
{
public:

    //
    // Constructor/Destructor
    //
    XilDeviceCompressionCell(XilDeviceManagerCompression* xdct,
                             XilCis*                      xcis);

    ~XilDeviceCompressionCell(void);

    XilStatus compress(XilOp*       op,
                       unsigned int op_count,
                       XilRoi*      roi,
                       XilBoxList*  bl);

    XilStatus decompress(XilOp*       op,
                         unsigned int op_count,
                         XilRoi*      roi,
                         XilBoxList*  bl);

    XilStatus decompressNearestColor8(XilOp*       op,
                                      unsigned int op_count,
                                      XilRoi*      roi,
                                      XilBoxList*  bl);

    XilStatus decompressOrderedDither8(XilOp*       op,
                                       unsigned int op_count,
                                       XilRoi*      roi,
                                       XilBoxList*  bl);


    //
    //  virtual member functions of XilDeviceCompression
    //
    void        seek(int framenumber, Xil_boolean history_update = TRUE);
    int         findNextFrameBoundary();

    int         adjustStart(int new_start_frame);
    void        flush();         

    XilStatus   decompressHeader();

    //
    //  Decompressor Supporting Member Functions
    //
    void           burnFrames(int);
    void           burnLines(int);
    int            getMaxFrameSize(void);
    void           rebuildRemap(void);
    void           updateUserColormap(void);
    void           reset();
    
    //
    //  Used by compress to get a CellFrame into the byte-stream when not
    //  interencoding.
    //
    int            outputCellFrame(CellFrame* hstCellFrame,
                                   CellFrame* curCellFrame);
    //
    //  Report a byte-stream error
    //
    XilStatus               byteStreamError(Xil_unsigned8* bp);
    
    //
    //  Function to read header and fill in the header information --
    //  specifically width and height
    //
    XilStatus         deriveOutputType(void);

    //
    //  Set/Get Member Functions....
    //
    //
    //  ColorMapAdaptionMode
    //
    void         setColorMapAdaptionMode(Xil_boolean val);
    Xil_boolean  getColorMapAdaptionMode(void);
    
    //
    //  CompressorColorMap
    //
    void           setCompressorColorMap(XilLookupSingle* tmp_lookup);

    //
    //  DecompressorColorMap
    //
    void           setDecompressorColorMap(XilLookupSingle* tmp_lookup);
    XilLookup*     getDecompressorColorMap(void);

    //
    //  Decompressor Read-Write Indices
    //
    void           setReadWriteIndices(XilIndexList* ilist);
    
    //
    //  InterframeEncodingMode
    //
    void           setEncodingType(XilCellEncodingType newval);

    XilCellEncodingType getEncodingType(void);
    
    //
    //  KeyFrameInterval
    //
    void           setKeyFrameInterval(int interval);
    int            getKeyFrameInterval(void);
    
    //
    //  CompressorMaxCmapSize
    //
    void           setCompressorMaxCmapSize(int newval);
    int            getCompressorMaxCmapSize(void);
    
    //
    //  DecompressorMaxCmapSize
    //
    int            getDecompressorMaxCmapSize(void);
    
    //
    //  compressorFrameRate
    //
    void           setCompressorFrameRate(Xil_unsigned32);
    Xil_unsigned32 getCompressorFrameRate(void);

    //
    //  decompressorFrameRate
    //
    Xil_unsigned32 getDecompressorFrameRate(void);

    //
    //  bitsPerSecond
    //
    void           setBitsPerSecond(int Bps);
    int            getBitsPerSecond(void);
    int            computeBytesPerFrameGroup(void);

    //
    //  temporalFiltering
    //
    void           setTemporalFiltering(Xil_boolean);
    Xil_boolean    getTemporalFiltering(void);

    //
    //  temporalFilteringLow
    //
    void           setTemporalFilterLow(unsigned int);
    Xil_boolean    getTemporalFilterLow(void);

    //
    //  temporalFilteringHigh
    //
    void           setTemporalFilterHigh(unsigned int);
    Xil_boolean    getTemporalFilterHigh(void);
    
    //
    //  Cell User Data
    //
    void             setUserData(XilCellUserData*);
    XilCellUserData* getUserData(void);
    
private:
    //
    //  Private Data
    //

    unsigned int   imageHeight, imageWidth;

    Xil_boolean    isOKFlag;

    //
    //  Data Storage Classes
    //
    CellCompressorData    compData;     // compressor specific data
    CellDecompressorData  decompData;   // decompressor specific data
    CellAttribs           cellAttribs;  // changeable attributes


    XilStatus decompressFrame(DecompressInfo* di);

    XilStatus decompressFrameNC8(DecompressInfo* di);

    XilStatus decompressFrameOD8(DecompressInfo*     di,
                                 XilLookupColorcube* cube,
                                 XilDitherMask*      dmask,
                                 Xil_boolean         doZoom);

    Xil_unsigned32* createDitherTable(XilLookupColorcube* cube,
                                      XilDitherMask*      dmask);

    //
    //  Private Member Functions used by flush()
    //
    void   interEncodeCellThres(unsigned int x,
                                unsigned int y,
                                CmapTable&   cmap,
                                CellFrame&   hstFrame,
                                CellFrame&   curFrame,
                                CellOutput&  cellout);
    
    void   interEncodeCellError(unsigned int frameNumber,
                                unsigned int partition,
                                const Cell&  curCell,
                                CellOutput&  cellout);

    void   interEncodeFrameThres(CellOutput&   cellout,
                                 CellFrame&    hstCellFrame,
                                 CellFrame&    curCellFrame);
    
    void   interEncodeFrameError(CellOutput&   cellout,
                                 unsigned int  frameNumber,
                                 CellFrame&    curCellFrame);
 
    unsigned int   figureBytesInFrameGroup(unsigned int dropMask);
    
    unsigned int   controlBitRate(void);
    void   fillErrorFrame(void);
    void   computeError(int x, int y);
    int    computeBytes(ErrorInfo& errinfo,
                        unsigned int max_lum,
                        unsigned int max_chrom);

    unsigned int   computeBytesInFrameGroup(unsigned int metric);

    //
    //  Function used by reset and the constructor to set values
    //
    int    initValues();

    //
    // Compressor Supporting Member Functions...
    //

    int    intraEncodeCurrentImage(CellFrame&   curFrame);

    Xil_boolean  controlColormapOutput();

    double  temporalFilterImages(Xil_unsigned8*  previousImage,
                                 CompressInfo*   ci,
                                 Xil_unsigned8*  resultImage,
                                 int             lowFilterThreshold,
                                 int             highFilterThreshold);

    int    rcode[256], gcode[256], bcode[256];

    int    prvHist[4096], curHist[4096];

    void copyImageToBuf(XilStorage*    storage,
                        Xil_unsigned8* prvBuf);

    int    tryColorSegment(const ColorValue& color0,
                           const ColorValue& color1,
                           const ColorValue* block,
                           int maxnoise);

    Cell   encodeBTC(ColorValue* block);

    Cell   encodeDither(ColorValue* block);

};

const unsigned int  RED_CODE_SHIFT   = 8;
const unsigned int  GRN_CODE_SHIFT   = 4;
const unsigned int  BLU_CODE_SHIFT   = 0;
const unsigned int  CODE_MASK        = 0xf;

inline unsigned int
CODE(unsigned int x)  { return (((x)>>4) & CODE_MASK); }

#define MAKE_KEY(r,g,b)  (rcode[(r)]|gcode[(g)]|bcode[b])

#endif  // XILDEVICEOMPRESSIONCELL_HH
