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
//  File:       XilDeviceManagerCompressionMpeg1.cc
//  Project:    XIL
//  Revision:   1.11
//  Last Mod:   10:14:45, 03/10/00
//
//  Description:
//
//    DeviceManager class for Mpeg1
//
//
//------------------------------------------------------------------------
//    COPYRIGHT
//------------------------------------------------------------------------
#pragma ident   "@(#)XilDeviceManagerCompressionMpeg1.cc	1.11\t00/03/10  "

#include <stdio.h>
#include "XilDeviceManagerCompressionMpeg1.hh"
#include "XilDeviceCompressionMpeg1.hh"

XilMutex XilDeviceManagerCompressionMpeg1::colorConvertTableMutex;

//
// Constructor - Register codec-specific attributes
//
XilDeviceManagerCompressionMpeg1::XilDeviceManagerCompressionMpeg1()
: XilDeviceManagerCompression("Mpeg1", "MPEG1") 
{

    //
    //   Compressor Attributes
    //

    registerAttr("COMPRESSOR_PEL_ASPECT_RATIO",
            (setAttrFunc) &XilDeviceCompressionMpeg1::setCAspectRatio,
            (getAttrFunc) &XilDeviceCompressionMpeg1::getCAspectRatio);

    registerAttr("COMPRESSOR_PICTURE_RATE",
            (setAttrFunc) &XilDeviceCompressionMpeg1::setCPictureRate,
            (getAttrFunc) &XilDeviceCompressionMpeg1::getCPictureRate);

    registerAttr("COMPRESSOR_TIME_CODE",
            (setAttrFunc) &XilDeviceCompressionMpeg1::setCTimeCode,
            (getAttrFunc) &XilDeviceCompressionMpeg1::getCTimeCode);

    registerAttr("COMPRESSOR_PATTERN",
            (setAttrFunc) &XilDeviceCompressionMpeg1::setPattern,
            (getAttrFunc) &XilDeviceCompressionMpeg1::getPattern);

    registerAttr("COMPRESSOR_BITS_PER_SECOND",
            (setAttrFunc) &XilDeviceCompressionMpeg1::setBitsPerSecond,
            (getAttrFunc) &XilDeviceCompressionMpeg1::getBitsPerSecond);

    registerAttr("COMPRESSOR_INTRA_QUANTIZATION_TABLE",
            (setAttrFunc) &XilDeviceCompressionMpeg1::setIntraQTable,
            (getAttrFunc) &XilDeviceCompressionMpeg1::getIntraQTable);

    registerAttr("COMPRESSOR_NON_INTRA_QUANTIZATION_TABLE",
            (setAttrFunc) &XilDeviceCompressionMpeg1::setNonIntraQTable,
            (getAttrFunc) &XilDeviceCompressionMpeg1::getNonIntraQTable);

    registerAttr("COMPRESSOR_SLICES_PER_PICTURE",
            (setAttrFunc) &XilDeviceCompressionMpeg1::setSlicesPerPicture,
            (getAttrFunc) &XilDeviceCompressionMpeg1::getSlicesPerPicture);

    registerAttr("COMPRESSOR_INSERT_VIDEO_SEQUENCE_END",
            (setAttrFunc) &XilDeviceCompressionMpeg1::setInsertSequenceEnd,
            (getAttrFunc) &XilDeviceCompressionMpeg1::getInsertSequenceEnd);


    //
    //   Decompressor Attributes
    //

    registerAttr("DECOMPRESSOR_QUALITY",
            (setAttrFunc) &XilDeviceCompressionMpeg1::setDecompressorQuality,
            (getAttrFunc) &XilDeviceCompressionMpeg1::getDecompressorQuality);

    registerAttr("DECOMPRESSOR_PEL_ASPECT_RATIO_VALUE", (setAttrFunc)NULL,
            (getAttrFunc) &XilDeviceCompressionMpeg1::getDAspectRatio);

    registerAttr("DECOMPRESSOR_PICTURE_RATE_VALUE", (setAttrFunc)NULL,
            (getAttrFunc) &XilDeviceCompressionMpeg1::getDPictureRate);

    registerAttr("DECOMPRESSOR_TIME_CODE", (setAttrFunc)NULL,
            (getAttrFunc) &XilDeviceCompressionMpeg1::getDTimeCode);

    registerAttr("DECOMPRESSOR_CLOSED_GOP", (setAttrFunc)NULL,
            (getAttrFunc) &XilDeviceCompressionMpeg1::getClosedFlag);

    registerAttr("DECOMPRESSOR_BROKEN_LINK", (setAttrFunc)NULL,
            (getAttrFunc) &XilDeviceCompressionMpeg1::getLinkFlag);

    registerAttr("DECOMPRESSOR_TEMPORAL_REFERENCE", (setAttrFunc)NULL,
            (getAttrFunc) &XilDeviceCompressionMpeg1::getTemporalReference);

    registerAttr("DECOMPRESSOR_FRAME_TYPE", (setAttrFunc)NULL,
            (getAttrFunc) &XilDeviceCompressionMpeg1::getFrameType);


    //
    // Initialize the color conversion object (used for molecules)
    //
    colorConverter = NULL;

};

//
// Destructor
//
XilDeviceManagerCompressionMpeg1::~XilDeviceManagerCompressionMpeg1()
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

    XilDeviceManagerCompressionMpeg1* mgr;

    mgr = new XilDeviceManagerCompressionMpeg1();
    if(mgr == NULL) {
        XIL_ERROR(NULL, XIL_ERROR_RESOURCE, "di-1", TRUE);
    }

    return mgr;
}

//
//  constructNewDevice() is used to create new instances of
//  the Mpeg1 device compression when new CISs are created by the
//  user.
//
XilDeviceCompression*
XilDeviceManagerCompressionMpeg1::constructNewDevice(XilCis* xcis)
{
    XilDeviceCompressionMpeg1* device;

    device = new XilDeviceCompressionMpeg1(this, xcis);
    if(device == NULL) {
        XIL_ERROR( NULL, XIL_ERROR_RESOURCE, "di-1", TRUE); 
        return NULL;
    }

    return device;
}
 
const char*
XilDeviceManagerCompressionMpeg1::getDeviceName()
{
    return "Mpeg1";
}
 
XilStatus
XilDeviceManagerCompressionMpeg1::describeMembers()
{
    XilFunctionInfo*  func_info;

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "compress_Mpeg1");
    func_info->setFunction((XilCodecFunctionPtr)
                            &XilDeviceCompressionMpeg1::compress);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "decompress_Mpeg1");
    func_info->setFunction((XilCodecFunctionPtr)
                            &XilDeviceCompressionMpeg1::decompress);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "color_convert;8");
    func_info->describeOp(XIL_STEP, 1, "decompress_Mpeg1");
    func_info->setFunction((XilCodecFunctionPtr)
                            &XilDeviceCompressionMpeg1::decompressColorConvert,
                            "color_convert;8(decompress_Mpeg1())");
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "ordered_dither;8->8");
    func_info->describeOp(XIL_STEP, 1, "decompress_Mpeg1");
    func_info->setFunction((XilCodecFunctionPtr)
                            &XilDeviceCompressionMpeg1::decompressOrderedDither,
                            "ordered_dither;8->8(decompress_Mpeg1())");
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "ordered_dither;8->8");
    func_info->describeOp(XIL_STEP, 1, "rescale;8");
    func_info->describeOp(XIL_STEP, 1, "decompress_Mpeg1");
    func_info->setFunction((XilCodecFunctionPtr)
                            &XilDeviceCompressionMpeg1::decompressOrderedDither,
                            "ordered_dither;8->8(rescale;8(decompress_Mpeg1()))");
    addFunction(func_info);
    func_info->destroy();

    return XIL_SUCCESS;
}

Ycc2RgbConverter*
XilDeviceManagerCompressionMpeg1::getColorConverter()
{
    //
    // The first time through, create the color conversion
    // object (under the protection of a mutex lock)
    // Foe Mpeg1, the table is created with an offset of zero
    //
    colorConvertTableMutex.lock();
    if(colorConverter == NULL) {
        colorConverter = new Ycc2RgbConverter(0);
    }
    colorConvertTableMutex.unlock();

    return colorConverter;
}

