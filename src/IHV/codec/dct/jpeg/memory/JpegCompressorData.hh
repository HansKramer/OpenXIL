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
//  File:       JpegCompressorData.hh
//  Project:    XIL
//  Revision:   1.4
//  Last Mod:   10:22:49, 03/10/00
//
//  Description:
//
//    Jpeg Compressor Related Data
//
//
//------------------------------------------------------------------------
//    COPYRIGHT
//------------------------------------------------------------------------
#pragma ident   "@(#)JpegCompressorData.hh	1.4\t00/03/10  "


#ifndef JPEGCOMPRESSORDATA_HH
#define JPEGCOMPRESSORDATA_HH

#include <xil/xilGPI.hh>

#include "SingleBuffer.hh"
#include "JpegBandInfo.hh"
#include "Jpeg_Quantizer.hh"
#include "Jpeg_Huffman_Encoder.hh"
#include "JpegOptHuffmanEncoder.hh"
#include "GobGetter.hh"
#include "JpegMacros.hh"


#define Y0         0
#define Y1         1
#define Y2         2
#define Y3         3
#define U0         4
#define V0         5

#define YBAND 0
#define UBAND 1
#define VBAND 2


class JpegCompressorData {
public:

    //
    // Constructor / destructor
    //
    JpegCompressorData(void);
    ~JpegCompressorData(void);

    //
    // Public methods
    //
    void                   reset(void);
    void                   useDefaultQTables();
    void                   useDefaultHTables();
    void                   useDefaultBandQTables();
    void                   useDefaultBandHTables();
    int                    createDefaultHTables();
    int allocOk()        {return allocSuccess;}

    //
    //  Attributes
    //
    Xil_boolean            abbr_format;
    unsigned int           qvalue;
    Xil_boolean            encode_video;
    Xil_boolean            encode_interleaved;
    Xil_boolean            use_optimal_htables;    
    Xil_boolean            filter_image;
    int                    bytes_per_frame;

    //
    //  Flags
    //
    Xil_boolean            qflag;
    Xil_boolean            bandinfo_changed;
    Xil_boolean            using_411_bandinfo;
    Xil_boolean            allocSuccess;

    //
    //  Encoder Objects
    //
    int                    version;
    JpegBandInfo*          banddata;
    Jpeg_Quantizer*        quantizer;
    JpegOptHuffmanEncoder* huffman_encoder;
    XilCisBufferManager*   buf_manager;
    SingleBuffer*          buffer;
    GobGetter*             gob_getter;
    int*                   gob;

    //
    //  Default Tables
    //
    HTable*                default_htables[4];

    //
    //  State Info
    //
    unsigned               nbands;

    //
    //  Temporal filtering requirments
    //
    Xil_unsigned8*         previous_image;
    Xil_unsigned8*         filteredBuf;

};

#endif // JPEGCOMPRESSORDATA_HH
