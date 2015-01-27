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
//  File:       XilDeviceCompressionMpeg1.hh
//  Project:    XIL
//  Revision:   1.10
//  Last Mod:   10:23:01, 03/10/00
//
//  Description:
//
//    Header for XilDeviceCompressionMpeg1 Class
//
//
//------------------------------------------------------------------------
//    COPYRIGHT
//------------------------------------------------------------------------
#pragma ident   "@(#)XilDeviceCompressionMpeg1.hh	1.10\t00/03/10  "

#ifndef XILDEVICECOMPRESSIONMPEG1_H
#define XILDEVICECOMPRESSIONMPEG1_H

#include <xil/xilGPI.hh>
#include "Mpeg1DecompressorData.hh"
#include "Ycc2RgbConverter.hh"
#include "XiliOrderedDitherLut.hh"
#include "DecompressInfo.hh"
#include "DctBlockUtils.hh"


typedef void (XilDeviceManager::*PMFV_XilDeviceManager)(void);

class XilDeviceCompressionMpeg1 : public XilDeviceCompression {
public:

    XilDeviceCompressionMpeg1(XilDeviceManagerCompression* xdct, 
                              XilCis*                      cis);

    ~XilDeviceCompressionMpeg1();

    //
    // Required Pure Virtual Functions
    //
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

    //
    //  Glue Member Functions Call from Above
    //
    XilStatus   decompressHeader(void);
    void        seek(int framenumber, Xil_boolean history_update = TRUE);
    int         findNextFrameBoundary();
    void*       getBitsPtr(int* nbytes, int* nframes);
    int         numberOfFrames();
    int         adjustStart(int framenumber);

    //
    //  Decompressor Supporting Member Functions
    //
    void        burnFrames(int nframes);
    int         getMaxFrameSize();
    void        reset();

    //
    //  Routines to get references to the Compressor, Decompressor and
    //  Attribute specific classes.
    //
    Mpeg1DecompressorData*  getMpeg1DecompressorData(void);

    //
    //  Function to read header and fill in the header information
    //  specifically width and height
    //
    XilStatus  deriveOutputType();
    void       setWidth(short w)     {frameWidth  = w;}
    void       setHeight(short h)    {frameHeight = h;}

    //
    //  Set/Get Member (Attribute) Functions....
    //
    //

    //
    // Compression ....
    //

    int                    setBitsPerSecond(int value);
    int                    getBitsPerSecond();
    int                    getSlicesPerPicture();
    void                   setCAspectRatio(XilMpeg1PelAspectRatio value);
    void                   setCPictureRate(XilMpeg1PictureRate value);
    void                   setPattern(XilMpeg1Pattern *pattern);
    void                   setIntraQTable(Xil_unsigned8 *qtable);
    void                   setNonIntraQTable(Xil_unsigned8 *qtable);
    void                   setSlicesPerPicture(int value);
    void                   setInsertSequenceEnd(Xil_boolean value);
    Xil_boolean            getInsertSequenceEnd();
    Xil_unsigned8*         getIntraQTable();
    Xil_unsigned8*         getNonIntraQTable();
    Xil_unsigned32         setCTimeCode(XilMpeg1TimeCode *code);
    XilMpeg1TimeCode*      getCTimeCode();
    XilMpeg1Pattern*       getPattern();
    XilMpeg1PictureRate    getCPictureRate();
    XilMpeg1PelAspectRatio getCAspectRatio();


    //
    // Decompression ....
    //

    int                    getDecompressorQuality();
    int                    getDAspectRatio();
    int                    getDPictureRate();
    void                   setDecompressorQuality(int value);
    Xil_boolean            getClosedFlag();
    Xil_boolean            getLinkFlag();
    Xil_unsigned32         getTemporalReference();
    XilMpeg1TimeCode*      getDTimeCode();
    XilMpeg1FrameType      getFrameType();

    // compressor flush internal buffered data 
    void                   flush();  

    // called to check for avail frame at read pos 
    Xil_boolean            hasFrame(); 

    // called to check amount of data at read pos 
    int                    hasData(); 

    // register external flush to call 
    void                   registerExternalFlush(PMFV_XilDeviceManager pmfv,
                                                 XilDeviceManager*  dev); 

private:
    //
    // Private member functions
    //
    void      initValues();

    void upsampleFrame(Mpeg1ReferenceFrame*  mpeg_ref,
                       DecompressInfo*       di);

    void upsampleBlock(Xil_signed16*         src_Y_dataptr,
                       unsigned int          src_Y_ss,
                       Xil_signed16*         src_Cb_dataptr,
                       unsigned int          src_Cb_ss,
                       Xil_signed16*         src_Cr_dataptr,
                       unsigned int          src_Cr_ss,
                       Xil_unsigned8*        dst_dataptr,
                       unsigned int          dst_ps,
                       unsigned int          dst_ss);

    void copyPartialBlock(Xil_unsigned8* src_block,
                          Xil_unsigned8* dst_block,
                          unsigned int   nx,
                          unsigned int   ny,
                          unsigned int   dst_ps,
                          unsigned int   dst_ss);

    //
    // Private data
    //
    unsigned int           frameWidth;
    unsigned int           frameHeight;
    unsigned int           frameNBands;
    Xil_boolean            inputOutputType;


    XilMutex     mutex;
    XilVersion   current_cmap_version;
    XilVersion   current_dmask_version;
    float        current_scale[3];
    float        current_offset[3];

    // Decompressor-specific data
    Mpeg1DecompressorData  decompData;

    int eos_frame_id;  // class variable for tracking EOS state
    Xil_unsigned8          mpeg1_eos_marker[4]; // EOS marker

    // hook to register external flush routine
    XilDeviceManager*      external_device;
    PMFV_XilDeviceManager  external_flush; 

    //
    // Pointers to the molecule support objects
    //
    Ycc2RgbConverter*     converter;
    XiliOrderedDitherLut* ditherTable;

    //
    // Verify that the constraints for decompressColorConvert molecules
    // are satisfied (ycc601->rgb709 color cvt, 16X dimensions)
    //
    Xil_boolean validDecompressColorConvert(XilImage* src,
                                            XilImage* dst);

    //
    // Verify that the constraints for decompressOrderedDither
    // are satisfied (4x4 dither mask, 855 colorcube)
    //
    Xil_boolean validDecompressOrderedDither(DecompressInfo*     di,
                                             XilLookupColorcube* cube,
                                             XilDitherMask*      dmask);

    XiliOrderedDitherLut* getDitherTable(XilLookupColorcube* cmap,
                                         XilDitherMask*      dmask,
                                         float*              scale,
                                         float*              offset);

};

#endif
