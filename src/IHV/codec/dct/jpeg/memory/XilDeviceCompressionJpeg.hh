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
//  File:       XilDeviceCompressionJpeg.hh
//  Project:    XIL
//  Revision:   1.13
//  Last Mod:   10:22:50, 03/10/00
//
//  Description:
//
//    Class definition for the derived Jpeg Compression device
//
//    This class is the definition of Jpeg compression and
//    decompression within XIL.  There is one of these objects per
//    Baseline Jpeg cis.
//
//
//------------------------------------------------------------------------
//    COPYRIGHT
//------------------------------------------------------------------------
#pragma ident   "@(#)XilDeviceCompressionJpeg.hh	1.13\t00/03/10  "

#ifndef XILDEVICECOMPRESSIONJPEG_HH
#define XILDEVICECOMPRESSIONJPEG_HH

#include <xil/xilGPI.hh>
#include "JpegCompressorData.hh"
#include "JpegDecompressorData.hh"
#include "CompressInfo.hh"
#include "DecompressInfo.hh"
#include "Ycc2RgbConverter.hh"
#include "XiliOrderedDitherLut.hh"

class XilDeviceCompressionJpeg : public XilDeviceCompression {
public:
    //
    // Constructor / destructor
    //
    XilDeviceCompressionJpeg(XilDeviceManagerCompression* xdct, 
                             XilCis*                      cis);

    ~XilDeviceCompressionJpeg();
  
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


    void setRandomAccess(Xil_boolean value);

    Xil_boolean isOK();

    //
    // Molecules
    //

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
    void        seek(int framenumber, Xil_boolean history_update = TRUE);
    int         findNextFrameBoundary();
    int         adjustStart(int framenumber);
    void        attemptRecovery(unsigned int nframes,
                                unsigned int nbytes,
                                Xil_boolean& read_invalid,
                                Xil_boolean& write_invalid);


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
    JpegCompressorData*    getJpegCompressorData(void);
    JpegDecompressorData*  getJpegDecompressorData(void);
    
    //
    //  Function to read header and fill in the header information
    //  specifically width and height
    //
    XilStatus  deriveOutputType();
    void       setWidth(int w);
    void       setHeight(int h);

    //
    // Compression Attribute Get/Set Methods
    //
    void       setDataFormat(XilJpegCompressedDataFormat format);
    void       setEncodeVideo(Xil_boolean video);
    void       setEncodeInterleaved(Xil_boolean inter);
    void       setQTable(XilJpegQTable& qt);
    void       setHTable(XilJpegHTable& ht);
    void       setBandQTable(XilJpegBandQTable& bqt);
    void       setBandHTable(XilJpegBandHTable& bht);
    void       setOptHTables(Xil_boolean opt_ht);
    void       setTemporalFilter(Xil_boolean flag);
    int        getTemporalFilter();
    void       setCompressionQuality(int value);
    int        getBytesPerFrame();
    
    //
    // Decompression Attribute Get/Set Methods
    //
    void       setIgnoreHistory(Xil_boolean flag);
    void       setDecompressionQuality(int value);  

    //
    // Jpeg-specific output functions
    //
    void       output_header(SingleBuffer*, int, int, int, int ,JpegBandInfo*);
    void       output_scan_header(SingleBuffer*, int, JpegBandInfo*);
    void       output_scan_header_for_band(SingleBuffer*, int, JpegBandInfo*);
    void       output_trailer(SingleBuffer*);
 

    //
    // Classes which hold much of the compression-specific
    // data and code.
    // TODO:  Should these be friends instead
    //
    JpegCompressorData    compData;     // compressor specific
    JpegDecompressorData  decompData;   // decompressor specific

    //
    // Default Quantization and Huffman Tables
    //
    static int          qtable0[8][8];
    static int          qtable1[8][8];
    static Huffman_Code dc_table0[16];
    static Huffman_Code dc_table1[16];
    static Huffman_Code ac_table0[256];
    static Huffman_Code ac_table1[256];

private: 
    XilImage*    srcImage;
    unsigned int imageWidth;
    unsigned int imageHeight;
    unsigned int imageNbands;
    XilDataType  imageDatatype;

    XilMutex     mutex;
    XilVersion   current_cmap_version;
    XilVersion   current_dmask_version;
    float        current_scale[3];
    float        current_offset[3];

    Ycc2RgbConverter*     colorConverter;
    XiliOrderedDitherLut* ditherTable;

    //
    // Define symbolic constant using anonymous enum
    // Local to class
    //
    enum {
        FRAMES_PER_BUFFER = 2
    };
    


    //
    // The compressed frame bitmap address
    //
    Xil_unsigned8* cfbm_adress;

    Xil_boolean  inputOutputType;
    Xil_boolean  isOKFlag;

    int initValues();

    XilStatus decompressFrame(DecompressInfo* di);

    XilStatus decompress411Frame(DecompressInfo* di);
    XilStatus decompress422Frame(DecompressInfo* di);

    void       use411Sampling(Xil_boolean use_411);
    XilStatus  validateBandTableUsage();
    XilStatus  compressFrame(CompressInfo* ci);
    XilStatus  compressInterleaved(unsigned w, unsigned h);
    XilStatus  compressNonInterleaved(unsigned w, unsigned h);
    XilStatus  compress411(unsigned w, unsigned h);

    Xil_boolean OkCisType(XilDeviceCompressionJpeg* dc, 
                          unsigned int              width,
                          unsigned int              height,
                          unsigned int              nbands);

    void upsampleBlock(Xil_signed16*  src_scan,
                       Xil_unsigned8* dst_scan,
                       unsigned int nsamps,
                       unsigned int nlines,
                       unsigned int xmag,
                       unsigned int ymag,
                       unsigned int dst_ps,
                       unsigned int dst_ss);

    void copyImageToBuf(CompressInfo*  ci,
                        Xil_unsigned8* prvBuf);

    XilStatus temporalFilterImages(Xil_unsigned8*  prvBuf,
                                   CompressInfo*   ci,
                                   Xil_unsigned8*  outBuf,
                                   int             lowFilterThreshold,
                                   int             highFilterThreshold);

    XilStatus filterSetup(CompressInfo* ci);

    Xil_boolean validDecompressColorConvert(XilImage* src,
                                            XilImage* dst);

    XiliOrderedDitherLut* getDitherTable(XilLookupColorcube* cmap,
                                         XilDitherMask*      dmask,
                                         float*              scale,
                                         float*              offset);


    Xil_boolean validDecompressOrderedDither(XilLookupColorcube* cube,
                                             XilDitherMask*      dmask);
};

#endif // XILDEVICECOMPRESSIONJPEG_HH
