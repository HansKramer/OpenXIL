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
//  File:       XilDeviceCompressionH261.hh
//  Project:    XIL
//  Revision:   1.9
//  Last Mod:   10:23:09, 03/10/00
//
//  Description:
//
//    Class header for XilDeviceCompressionH261
//
//
//------------------------------------------------------------------------
//    COPYRIGHT
//------------------------------------------------------------------------
#pragma ident   "@(#)XilDeviceCompressionH261.hh	1.9\t00/03/10  "


#ifndef XILDEVICECOMPRESSIONH261_HH
#define XILDEVICECOMPRESSIONH261_HH

#include <xil/xilGPI.hh>
#include "H261DecompressorData.hh"
#include "H261CompressorData.hh"
#include "Ycc2RgbConverter.hh"
#include "XiliOrderedDitherLut.hh"
#include "DecompressInfo.hh"

class XilDeviceCompressionH261 : public XilDeviceCompression {
public:
    //
    // Constructor / destructor
    //
    XilDeviceCompressionH261(XilDeviceManagerCompression* xdct, 
                             XilCis*                      cis);

    ~XilDeviceCompressionH261();
  
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
    //  Set/Get Member (Attribute) Functions....
    //
    //  These need to be public so XilDeviceManagerCompressionH261 can
    //  access them in order to register them with the XIL library.
    //
    // Compression....

    void          setBitsPerImage(int value);
    int           getBitsPerImage();
    void          setImageSkip(int value);
    int           getImageSkip();
    void		setSearchRange(XilH261MVSearchRange* value);
    XilH261MVSearchRange* getSearchRange();
    void          setLoopFilter(Xil_boolean value);
    Xil_boolean   getLoopFilter();
    void          setEncodeIntra(Xil_boolean value);
    Xil_boolean   getEncodeIntra();
    void          setCompFreezeRelease(Xil_boolean value);
    Xil_boolean   getCompFreezeRelease();
    void          setCompSplitScreen(Xil_boolean value);
    Xil_boolean   getCompSplitScreen();
    void          setCompDocCamera(Xil_boolean value);
    Xil_boolean   getCompDocCamera();
    
    //
    // Decompression ....
    //
    void		setIgnoreHistoryFlag(Xil_boolean value);
    Xil_boolean	getIgnoreHistoryFlag();
    Xil_boolean	getScreenFlag();
    Xil_boolean	getCameraFlag();
    Xil_boolean	getFreezeFlag();
    XilH261SourceFormat	getCifFlag();
    int		getTemporalRef();

private: 
    XilMutex     mutex;
    XilVersion   current_cmap_version;
    XilVersion   current_dmask_version;
    float        current_scale[3];
    float        current_offset[3];

    XilImage*    srcImage;
    unsigned int imageWidth;
    unsigned int imageHeight;
    unsigned int imageNbands;
    XilDataType  imageDatatype;

    H261DecompressorData  decompData;   // decompressor specific
    H261CompressorData    compData;      // compressor specific

    //
    // Pointer to the color conversion object
    //
    Ycc2RgbConverter* converter;

    //
    // Pointer to ditherTable object
    //
    XiliOrderedDitherLut* ditherTable;

    //
    // Flags for molecules
    //
    Xil_boolean doColorConvert;
    Xil_boolean doOrderedDither;

    //
    // Buffer for upsampling prior to dither
    //
    Xil_unsigned8* ditherBuf;

    //
    // Table for rescaling (used in some molecules)
    // Its small enough to just make it part of the object
    //
    Xil_unsigned8 rescaleTable[3*256];

    //
    // Verify that the constraints for decompressColorConvert molecules
    // are satisfied (ycc601->rgb709 color cvt, 16X dimensions)
    //
    Xil_boolean validDecompressColorConvert(XilImage* src,
                                            XilImage* dst);

    Xil_boolean validDecompressOrderedDither(XilLookupColorcube* cube,
                                             XilDitherMask*      dmask);

    Xil_unsigned8* getRescaleTable(float* scale, 
                                   float* offset);

    //
    // The compressed frame bitmap address
    //
    Xil_unsigned8* cfbm_adress;

    Xil_boolean  inputOutputType;
    Xil_boolean  isOKFlag;

    int initValues();

    XilStatus upsampleFrame(H261DecompressorData* decoder,
                            DecompressInfo*      di);

    XilStatus decompressFrame(DecompressInfo* di);

    void       use411Sampling(Xil_boolean use_411);
    XilStatus  compressSetup(XilStorage* storage);
    XilStatus  compress411(unsigned w, unsigned h);

    void upsampleBlock(Xil_signed16*  src_scan,
                       Xil_unsigned8* dst_scan,
                       unsigned int nsamps,
                       unsigned int nlines,
                       unsigned int xmag,
                       unsigned int ymag,
                       unsigned int dst_ps,
                       unsigned int dst_ss);

    void copyImageToBuf(XilStorage*    storage,
                        Xil_unsigned8* prvBuf);

    XilStatus   decompressHeader(void);
    void        seek(int framenumber, Xil_boolean history_udpate= TRUE);
    int         findNextFrameBoundary();
    void setRandomAccess(Xil_boolean value)	{ random_access = value; }

    //
    //  Routine used INSTEAD of seek in actual deferred op.  does
    //  the seeking operation but also flushes any other decompresses
    //  that need to happen first due to deferred operations.
    //
    void 	seekFlush(int framenumber);

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
    H261DecompressorData*  getH261DecompressorData(void);
    H261CompressorData*    getH261CompressorData(void);
    
    //
    //  Function to read header and fill in the header information
    //  specifically width and height
    //
    XilStatus  deriveOutputType();
    void       setWidth(unsigned int w)		{imageWidth = w;}
    void       setHeight(unsigned int h)		{imageHeight = h;}

      Xil_boolean isOK();
    
    XiliOrderedDitherLut* getDitherTable(XilLookupColorcube* cmap,
                                         XilDitherMask*      dmask,
                                         float*              scale,
                                         float*              offset);
};

#endif // XILDEVICECOMPRESSIONH261_HH

