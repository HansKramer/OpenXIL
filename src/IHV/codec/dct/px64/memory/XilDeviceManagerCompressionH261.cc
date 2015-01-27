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
//  File:       XilDeviceManagerCompressionH261.cc
//  Project:    XIL
//  Revision:   1.8
//  Last Mod:   10:15:19, 03/10/00
//
//  Description:
//
//    Device manager for H.261 codec
//
//
//------------------------------------------------------------------------
//    COPYRIGHT
//------------------------------------------------------------------------
#pragma ident   "@(#)XilDeviceManagerCompressionH261.cc	1.8\t00/03/10  "


#include "XilDeviceManagerCompressionH261.hh"
#include "XilDeviceCompressionH261.hh"

//
// Constructor - Register codec-specific attributes
//
XilDeviceManagerCompressionH261::XilDeviceManagerCompressionH261()
: XilDeviceManagerCompression("H261", "H261") 
{
    //
    //   Compressor Attributes
    //
    registerAttr("COMPRESSOR_BITS_PER_IMAGE",
            (setAttrFunc) &XilDeviceCompressionH261::setBitsPerImage,
            (getAttrFunc) &XilDeviceCompressionH261::getBitsPerImage);
    
    registerAttr("COMPRESSOR_IMAGE_SKIP",
            (setAttrFunc) &XilDeviceCompressionH261::setImageSkip,
            (getAttrFunc) &XilDeviceCompressionH261::getImageSkip);
    
    registerAttr("COMPRESSOR_MV_SEARCH_RANGE",
            (setAttrFunc) &XilDeviceCompressionH261::setSearchRange,
            (getAttrFunc) &XilDeviceCompressionH261::getSearchRange);
    
    registerAttr("COMPRESSOR_LOOP_FILTER",
            (setAttrFunc) &XilDeviceCompressionH261::setLoopFilter,
            (getAttrFunc) &XilDeviceCompressionH261::getLoopFilter);

    registerAttr("COMPRESSOR_ENCODE_INTRA",
            (setAttrFunc) &XilDeviceCompressionH261::setEncodeIntra,
            (getAttrFunc) &XilDeviceCompressionH261::getEncodeIntra);

    registerAttr("COMPRESSOR_FREEZE_RELEASE",
            (setAttrFunc) &XilDeviceCompressionH261::setCompFreezeRelease,
            (getAttrFunc) &XilDeviceCompressionH261::getCompFreezeRelease);

    registerAttr("COMPRESSOR_SPLIT_SCREEN",
            (setAttrFunc) &XilDeviceCompressionH261::setCompSplitScreen,
            (getAttrFunc) &XilDeviceCompressionH261::getCompSplitScreen);

    registerAttr("COMPRESSOR_DOC_CAMERA",
            (setAttrFunc) &XilDeviceCompressionH261::setCompDocCamera,
            (getAttrFunc) &XilDeviceCompressionH261::getCompDocCamera);

    //
    //   Decompressor Attributes
    //
    
    registerAttr("IGNORE_HISTORY",
            (setAttrFunc) &XilDeviceCompressionH261::setIgnoreHistoryFlag,
            (getAttrFunc) &XilDeviceCompressionH261::getIgnoreHistoryFlag);
    
    registerAttr("DECOMPRESSOR_SPLIT_SCREEN", (setAttrFunc)NULL,
            (getAttrFunc) &XilDeviceCompressionH261::getScreenFlag);
    
    registerAttr("DECOMPRESSOR_DOC_CAMERA", (setAttrFunc)NULL,
            (getAttrFunc) &XilDeviceCompressionH261::getCameraFlag);

    registerAttr("DECOMPRESSOR_FREEZE_RELEASE", (setAttrFunc)NULL,
            (getAttrFunc) &XilDeviceCompressionH261::getFreezeFlag);

    registerAttr("DECOMPRESSOR_SOURCE_FORMAT", (setAttrFunc)NULL,
            (getAttrFunc) &XilDeviceCompressionH261::getCifFlag);

    registerAttr("DECOMPRESSOR_TEMPORAL_REFERENCE", (setAttrFunc)NULL,
            (getAttrFunc) &XilDeviceCompressionH261::getTemporalRef);

    //
    // Initialize the color conversion object and dither objects
    // (used for molecules)
    //
    colorConverter = NULL;
    ditherTable    = NULL;

};

//
// Destructor
//
XilDeviceManagerCompressionH261::~XilDeviceManagerCompressionH261()
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

    XilDeviceManagerCompressionH261* mgr;

    mgr = new XilDeviceManagerCompressionH261();
    if(mgr == NULL) {
        XIL_ERROR(NULL, XIL_ERROR_RESOURCE, "di-1", TRUE);
    }

    return mgr;
}

//
//  constructNewDevice() is used to create new instances of
//  the H261 device compression when new CISs are created by the
//  user.
//
XilDeviceCompression*
XilDeviceManagerCompressionH261::constructNewDevice(XilCis* xcis)
{
    XilDeviceCompressionH261* device;

    device = new XilDeviceCompressionH261(this, xcis);
    if (device == NULL) {
        XIL_ERROR(xcis->getSystemState(), XIL_ERROR_RESOURCE, "di-1", TRUE); 
        return NULL;
    }

    return device;
}
 
const char*
XilDeviceManagerCompressionH261::getDeviceName()
{
    return "H261";
}
 
XilStatus
XilDeviceManagerCompressionH261::describeMembers()
{
    XilFunctionInfo*  func_info;
 
    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "compress_H261");
    func_info->setFunction((XilCodecFunctionPtr)
                            &XilDeviceCompressionH261::compress);
    addFunction(func_info);
    func_info->destroy();
 
    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "decompress_H261");
    func_info->setFunction((XilCodecFunctionPtr)
                            &XilDeviceCompressionH261::decompress);
    addFunction(func_info);
    func_info->destroy();
 
    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "color_convert;8");
    func_info->describeOp(XIL_STEP, 1, "decompress_H261");
    func_info->setFunction((XilCodecFunctionPtr)
                            &XilDeviceCompressionH261::decompressColorConvert,
                            "color_convert;8(decompress_H261())");
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "ordered_dither;8->8");
    func_info->describeOp(XIL_STEP, 1, "decompress_H261");
    func_info->setFunction((XilCodecFunctionPtr)
                            &XilDeviceCompressionH261::decompressOrderedDither,
                            "ordered_dither;8->8(decompress_H261())");
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "ordered_dither;8->8");
    func_info->describeOp(XIL_STEP, 1, "rescale;8");
    func_info->describeOp(XIL_STEP, 1, "decompress_H261");
    func_info->setFunction((XilCodecFunctionPtr)
                            &XilDeviceCompressionH261::decompressOrderedDither,
                            "ordered_dither;8->8(rescale;8(decompress_H261()))");
    addFunction(func_info);
    func_info->destroy();

    return XIL_SUCCESS;
}

Ycc2RgbConverter*
XilDeviceManagerCompressionH261::getColorConverter()
{
    //
    // The first time through, create the color conversion
    // object (under the protection of a mutex lock)
    // For H261, the table is created with an offset of zero
    //
    mutex.lock();
    if(colorConverter == NULL) {
        colorConverter = new Ycc2RgbConverter(0);
    }
    mutex.unlock();

    return colorConverter;
}
