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
//  File:       XilDeviceManagerCompressionJpeg.cc
//  Project:    XIL
//  Revision:   1.10
//  Last Mod:   10:14:32, 03/10/00
//
//  Description:
//
//    TODO: Enter some descriptive text here
//
//
//------------------------------------------------------------------------
//    COPYRIGHT
//------------------------------------------------------------------------
#pragma ident   "@(#)XilDeviceManagerCompressionJpeg.cc	1.10\t00/03/10  "

#include "XilDeviceManagerCompressionJpeg.hh"
#include "XilDeviceCompressionJpeg.hh"

//
// Constructor - Register codec-specific attributes
//
XilDeviceManagerCompressionJpeg::XilDeviceManagerCompressionJpeg()
: XilDeviceManagerCompression("Jpeg", "JPEG") 
{
    //
    //   Compressor Attributes
    //
    registerAttr("COMPRESSED_DATA_FORMAT",
                 (setAttrFunc) &XilDeviceCompressionJpeg::setDataFormat,
                 (getAttrFunc)NULL);
    
    registerAttr("QUANTIZATION_TABLE",
                 (setAttrFunc) &XilDeviceCompressionJpeg::setQTable,
                 (getAttrFunc)NULL);
    
    registerAttr("BAND_QUANTIZER",
                 (setAttrFunc) &XilDeviceCompressionJpeg::setBandQTable,
                 (getAttrFunc)NULL);

    registerAttr("HUFFMAN_TABLE",
                 (setAttrFunc) &XilDeviceCompressionJpeg::setHTable,
                 (getAttrFunc)NULL);
    
    registerAttr("BAND_HUFFMAN_TABLE",
                 (setAttrFunc) &XilDeviceCompressionJpeg::setBandHTable,
                 (getAttrFunc)NULL);
    
    registerAttr("OPTIMIZE_HUFFMAN_TABLES",
                 (setAttrFunc) &XilDeviceCompressionJpeg::setOptHTables,
                 (getAttrFunc)NULL);
    
    registerAttr("COMPRESSION_QUALITY",
                 (setAttrFunc) &XilDeviceCompressionJpeg::setCompressionQuality,
                 (getAttrFunc)NULL);
    
    registerAttr("ENCODE_411_INTERLEAVED",
                 (setAttrFunc) &XilDeviceCompressionJpeg::setEncodeVideo,
                 (getAttrFunc)NULL);
    
    registerAttr("ENCODE_INTERLEAVED",
                 (setAttrFunc) &XilDeviceCompressionJpeg::setEncodeInterleaved,
                 (getAttrFunc)NULL);

    registerAttr("BYTES_PER_FRAME",
                 (setAttrFunc)NULL,
                 (getAttrFunc) &XilDeviceCompressionJpeg::getBytesPerFrame);
    
    registerAttr("TEMPORAL_FILTERING",
                 (setAttrFunc) &XilDeviceCompressionJpeg::setTemporalFilter,
                 (getAttrFunc) &XilDeviceCompressionJpeg::getTemporalFilter);
    
    //
    //   Decompressor Attributes
    //
    
    registerAttr("IGNORE_HISTORY",
                 (setAttrFunc) &XilDeviceCompressionJpeg::setIgnoreHistory,
                 (getAttrFunc)NULL);

    registerAttr("DECOMPRESSION_QUALITY",
                 (setAttrFunc) &XilDeviceCompressionJpeg::setDecompressionQuality,
                 (getAttrFunc)NULL);
    

    //
    // Init the color converter object pointer (for molecules)
    //
    colorConverter = NULL;

};

//
// Destructor
//
XilDeviceManagerCompressionJpeg::~XilDeviceManagerCompressionJpeg()
{
    delete colorConverter;
}

//
// Create a new device manager
// (One per compression type)
//
XilDeviceManagerCompression*
XilDeviceManagerCompression::create(unsigned int  libxil_gpi_major,
                                    unsigned int  libxil_gpi_minor,
                                    unsigned int* devhandler_gpi_major,
                                    unsigned int* devhandler_gpi_minor)
{
    XIL_BASIC_GPI_VERSION_TEST(libxil_gpi_major,
                               libxil_gpi_minor,
                               devhandler_gpi_major,
                               devhandler_gpi_minor);

    XilDeviceManagerCompressionJpeg* mgr;

    mgr = new XilDeviceManagerCompressionJpeg();
    if(mgr == NULL) {
        XIL_ERROR(NULL, XIL_ERROR_RESOURCE, "di-1", TRUE);
    }

    return mgr;
}

//
//  constructNewDevice() is used to create new instances of
//  the Jpeg device compression when new CISs are created by the
//  user.
//
XilDeviceCompression*
XilDeviceManagerCompressionJpeg::constructNewDevice(XilCis* xcis)
{
    XilDeviceCompressionJpeg* device;

    device = new XilDeviceCompressionJpeg(this, xcis);
    if (device == NULL) {
        XIL_ERROR( NULL, XIL_ERROR_RESOURCE, "di-1", TRUE); 
        return NULL;
    }

    return device;
}
 
const char*
XilDeviceManagerCompressionJpeg::getDeviceName()
{
    return "Jpeg";
}
 
XilStatus
XilDeviceManagerCompressionJpeg::describeMembers()
{
    XilFunctionInfo*  func_info;
 
    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "compress_Jpeg");
    func_info->setFunction((XilCodecFunctionPtr)
                            &XilDeviceCompressionJpeg::compress);
    addFunction(func_info);
    func_info->destroy();
 
    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "decompress_Jpeg");
    func_info->setFunction((XilCodecFunctionPtr)
                            &XilDeviceCompressionJpeg::decompress);
    addFunction(func_info);
    func_info->destroy();
 
    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "color_convert;8");
    func_info->describeOp(XIL_STEP, 1, "decompress_Jpeg");
    func_info->setFunction((XilCodecFunctionPtr)
                            &XilDeviceCompressionJpeg::decompressColorConvert,
                            "color_convert;8(decompress_Jpeg())");
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "ordered_dither;8->8");
    func_info->describeOp(XIL_STEP, 1, "decompress_Jpeg");
    func_info->setFunction((XilCodecFunctionPtr)
                            &XilDeviceCompressionJpeg::decompressOrderedDither,
                            "ordered_dither;8->8(decompress_Jpeg())");
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "ordered_dither;8->8");
    func_info->describeOp(XIL_STEP, 1, "rescale;8");
    func_info->describeOp(XIL_STEP, 1, "decompress_Jpeg");
    func_info->setFunction((XilCodecFunctionPtr)
                            &XilDeviceCompressionJpeg::decompressOrderedDither,
                            "ordered_dither;8->8(rescale;8(decompress_Jpeg()))");
    addFunction(func_info);
    func_info->destroy();


 
    return XIL_SUCCESS;
}

Ycc2RgbConverter*
XilDeviceManagerCompressionJpeg::getColorConverter()
{
    //
    // The first time through, create the color conversion
    // object (under the protection of a mutex lock
    //
    mutex.lock();
    if(colorConverter == NULL) {
        colorConverter = new Ycc2RgbConverter(128);
    }
    mutex.unlock();

    return colorConverter;
}

