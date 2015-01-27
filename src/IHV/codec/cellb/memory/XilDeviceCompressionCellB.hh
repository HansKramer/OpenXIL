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
//  File:   XilDeviceCompressionCellB.hh
//  Project:    XIL
//  Revision:   1.5
//  Last Mod:   10:23:21, 03/10/00
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
#pragma ident   "@(#)XilDeviceCompressionCellB.hh	1.5\t00/03/10  "

#ifndef _XILDEVICECOMPRESSION_CELLB_HH
#define _XILDEVICECOMPRESSION_CELLB_HH

#include <xil/xilGPI.hh>

#include "CellBCompressorData.hh"
#include "CellBDecompressorData.hh"
#include "CellBManagerCompressorData.hh"

#include "XilDeviceManagerCompressionCellB.hh"
#include "CompressInfo.hh"
#include "DecompressInfo.hh"
#include "Ycc2RgbConverter.hh"
#include "XiliOrderedDitherLut.hh"

//
// Description:
//  This class is the definition of CellB compression and
//  decompression within XIL.
//


class XilDeviceCompressionCellB : public XilDeviceCompression
{
public:

    //
    // Constructor/Destructor
    //
    XilDeviceCompressionCellB(XilDeviceManagerCompressionCellB* xdct,
                              XilCis*                           xcis);

    ~XilDeviceCompressionCellB();

    XilStatus compress(XilOp*       op,
                       unsigned int op_count,
                       XilRoi*      roi,
                       XilBoxList*  bl);

    XilStatus decompress(XilOp*       op,
                         unsigned int op_count,
                         XilRoi*      roi,
                         XilBoxList*  bl);

    XilStatus decompressColorConvert(XilOp*       op,
                                     unsigned int op_count,
                                     XilRoi*      roi,
                                     XilBoxList*  bl);

    XilStatus decompressOrderedDither(XilOp*       op,
                                      unsigned int op_count,
                                      XilRoi*      roi,
                                      XilBoxList*  bl);

    XilStatus decompressFrame(DecompressInfo* di);
    XilStatus decompressCCFrame(DecompressInfo* di);
    XilStatus decompressDitherFrame(DecompressInfo* di);

    //
    // virtual member functions of XilDeviceCompression
    //

    void          seek(int framenumber, Xil_boolean history_update = TRUE);
    void          reset();
    int           findNextFrameBoundary();

    void          burnFrames(int nframes);
    int           getMaxFrameSize();

    XilStatus     deriveOutputType();

    //  routine used INSTEAD of seek in actual deferred op.  does
    //  the seeking operation but also flushes any other decompresses
    //  that need to happen first due to deferred operations.

    void          seekFlush(int framenumber);

    //
    //  Routines to get references to the Compressor, Decompressor and
    //  Attribute specific classes.
    //
    CellBCompressorData*    getCellBCompressorData(void);
    CellBDecompressorData*  getCellBDecompressorData(void);

    //
    //  Set/Get Member Functions....
    //
    void          setWidth(int);
    void          setHeight(int);

    void          setIgnoreHistoryFlag(Xil_boolean value);
    Xil_boolean   getIgnoreHistoryFlag();

    //
    // Routines used in decompression
    //
    void setRandomAccess(Xil_boolean value) { random_access = value; }

    //
    // Routines used in compression
    //

    // the q values are the y values for each pixel in the cell, packed
    // into 4 words.
    int encodeCell(Xil_unsigned32 ymean,
                   Xil_unsigned32 uvmean,
                   Xil_unsigned32 q0,
                   Xil_unsigned32 q1,
                   Xil_unsigned32 q2,
                   Xil_unsigned32 q3);

    int inner_loop(Xil_unsigned8* base_addr,
                   unsigned int p_stride,
                   unsigned int s_stride);

    Xil_boolean    skipCell(Xil_unsigned32 cell, int index);

private:

    Xil_boolean         isOKFlag;

    int      imageHeight;
    int      imageWidth;

    //
    //  Data Storage Classes
    //
    CellBCompressorData    compData;    // compressor specific data
    CellBDecompressorData  decompData;  // decompressor specific data

    //
    // Support conversion objects and flags
    //
    Ycc2RgbConverter*     colorCvt;
    XiliOrderedDitherLut* ditherCvt;
    XilMutex              mutex;
    XilVersion            current_cmap_version;
    XilVersion            current_dmask_version;
    float                 current_scale[3];
    float                 current_offset[3];


    //
    // Verify that the constraints for decompressColorConvert molecules
    // are satisfied (ycc601->rgb709 color cvt, 16X dimensions)
    //
    Xil_boolean validDecompressColorConvert(XilImage* src,
                                            XilImage* dst);

    Xil_boolean validDecompressOrderedDither(XilLookupColorcube* cube,
                                             XilDitherMask*      dmask);

    //
    //  Function used by reset and the constructor to set values
    //
    int   initValues();

    XiliOrderedDitherLut* getDitherTable(XilLookupColorcube* cmap,
                                         XilDitherMask*      dmask, 
                                         float*              scale,
                                         float*              offset);


};

#endif // _XILDEVICECOMPRESSION_CELLB_HH
