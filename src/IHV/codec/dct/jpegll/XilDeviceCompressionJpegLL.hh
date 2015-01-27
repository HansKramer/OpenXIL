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

//This line lets emacs recognize this as -*- C++ -*- Code
//------------------------------------------------------------------------
//
//  File:       XilDeviceCompressionJpegLL.hh
//  Project:    XIL
//  Revision:   1.7
//  Last Mod:   10:23:04, 03/10/00
//
//  Description:
//
//    TODO: Enter some descriptive text here
//
//
//------------------------------------------------------------------------
//    COPYRIGHT
//------------------------------------------------------------------------
#pragma ident   "@(#)XilDeviceCompressionJpegLL.hh	1.7\t00/03/10  "

#ifndef XILDEVICECOMPRESSIONJPEGLL_HH
#define XILDEVICECOMPRESSIONJPEGLL_HH

#include <xil/xilGPI.hh>
#include "SingleBuffer.hh"
#include "JpegParser.hh"
#include "JpegBandInfo.hh"
#include "Jpeg_Markers.hh"
#include "JpegLL_Huffman_Encoder.hh"
#include "JpegLLHuffmanDecoder.hh"
#include "CompressInfo.hh"
#include "DecompressInfo.hh"
 
#define FRAMES_PER_BUFFER   3
    
#define HUFFMAN_TABLE_SIZE  1
#define MAX_HUF_CODE_LENGTH 2

#define MAX_DECODER_TABLES  32
#define MAX_WIDTH           1024

class JpegLLCompressFrameInfo {
public:
    unsigned int   width;
    unsigned int   height;
    unsigned int   nbands;
    unsigned int   band_number;
    unsigned int   ps;
    unsigned int   ss;
    unsigned int   bs;
    XilDataType    datatype;
    void*          dataptr;
    SingleBuffer*  sbuffer;
};

struct JpegLLScanInfo {
    unsigned int   width;
    unsigned int   height;
    unsigned int   nbands;
    unsigned int   band_number;
    unsigned int   ps;
    unsigned int   ss;
    unsigned int   bs;
    int            precision;
    void*          dataptr;
    int            predictor;
    int            transform;
    int            component[4];
    int            restartInterval;
};

class XilDeviceCompressionJpegLL : public XilDeviceCompression {
public:
    //
    // Constructor / destructor
    //
    XilDeviceCompressionJpegLL(XilDeviceManagerCompression* xdct, 
                             XilCis*                      cis);

    ~XilDeviceCompressionJpegLL();
  
    //
    // Required Pure Virtual Functions
    //
    XilStatus   compress(XilOp*       op, 
                         unsigned int op_count, 
                         XilRoi*      roi, 
                         XilBoxList*  bl);

    XilStatus   decompress(XilOp*       op, 
                           unsigned int op_count, 
                           XilRoi*      roi, 
                           XilBoxList*  bl);

    int         findNextFrameBoundary();
    int         getMaxFrameSize();

    Xil_boolean isOK();

    void        burnFrames(int nframes);
    XilStatus   deriveOutputType();
   
    void        initDecompress();
    void        resetDecompress();
   
    XilStatus   decompressFrame(DecompressInfo* di);

    XilStatus   decompressInterleavedScan(JpegLLScanInfo* si);
    XilStatus   decompressNonInterleavedScan(JpegLLScanInfo* si);
    //
    // Attribute Setting Methods
    //
    void       setAbbrFormat(int abbr);
    void       setEncodeInterleaved(int inter);
    void       setBandPtTransform(XilJpegLLBandPtTransform& trans);
    void       setBandSelector(XilJpegLLBandSelector& sel);
    void       setHTable(XilJpegHTable& ht);
    void       setBandHTable(XilJpegBandHTable& bht);
    void       setOptHTables(int opt_ht);

    //
    // Encoder table / buffer methods
    //
    void        useDefaultHTables();
    int         createDefaultHTables();
    void        useDefaultBandHTables();
 
    //
    // Other methods
    //
    void       reset();

    //
    // Marker output functions
    //
    void output_frame_header(CompressInfo* ci);
    void output_scan_header(unsigned int start_band, unsigned int nbands);
    void output_trailer();

private: 
    //
    // Private Methods
    //
    XilStatus  compressInterleaved(CompressInfo* ci);
    XilStatus  compressNonInterleaved(CompressInfo* ci);
    int  initValues();
    int  compressInit();
    int  decompressInit();
    void compressReset();
    void decompressReset();
    int  validateBandTableUsage(unsigned int nbands);

    JpegLLCompressFrameInfo* getCompressInfo(XilOp*       op,
                                             unsigned int op_count,
                                             XilRoi* ,
                                             XilBoxList*  bl);
    //
    // Private data Members
    //
    XilImage*           srcImage;
    unsigned int        frameWidth;
    unsigned int        frameHeight;
    unsigned int        frameNbands;
    XilDataType         frameDataType;

    //
    // Pointers to other objects used by this class
    //
    JpegLL_Huffman_Encoder* huffman_encoder;
    JpegLLHuffmanDecoder*   decoder;
    JpegParser*             parser;
    JpegBandInfo*           banddata;
    SingleBuffer*           sbuffer;
    char*                   jpeg_name;
    XilImageFormat*         inputOutputType;

    //
    // Flags
    //
    Xil_boolean         bandinfo_changed;
    Xil_boolean         already_readtoscan;
    Xil_boolean         isOKFlag;

    //
    // Attributes
    //
    unsigned int      abbr_format;
    Xil_unsigned8     encode_interleaved;
    Xil_unsigned8     use_optimal_htables;
    Xil_unsigned8     precision;
    Xil_unsigned8     pt_transform[JPEG_MAX_NUM_BANDS];
    Xil_unsigned8     selector[JPEG_MAX_NUM_BANDS];
    int               version;

    HTable*           default_htables[4];
     
    //
    // Static tables - one copy for all instances
    //
    static Huffman_Code jpegll_table0[17];
    static Huffman_Code jpegll_table1[17];
    static Huffman_Code jpegll_table2[17];
    static Huffman_Code jpegll_table3[17];

    
};

#endif // XILDEVICECOMPRESSIONJPEGLL_HH
