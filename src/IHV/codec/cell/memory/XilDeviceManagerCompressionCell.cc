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
//  File:   XilDeviceManagerCompressionCell.cc
//  Project:    XIL
//  Revision:   1.6
//  Last Mod:   10:15:45, 03/10/00
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
#pragma ident   "@(#)XilDeviceManagerCompressionCell.cc	1.6\t00/03/10  "

#include "XilDeviceCompressionCell.hh"
#include "XilDeviceManagerCompressionCell.hh"

//
// Constructor/Destructor
//
XilDeviceManagerCompressionCell::XilDeviceManagerCompressionCell()
 : XilDeviceManagerCompression("Cell", "CELL")
{
    //
    // Register the Compressor attributes
    //
    registerAttr("ENCODING_TYPE",
        (setAttrFunc) &XilDeviceCompressionCell::setEncodingType,
        (getAttrFunc) &XilDeviceCompressionCell::getEncodingType);

    registerAttr("COLORMAP_ADAPTION",
        (setAttrFunc) &XilDeviceCompressionCell::setColorMapAdaptionMode,
        (getAttrFunc) &XilDeviceCompressionCell::getColorMapAdaptionMode);

    registerAttr("COMPRESSOR_COLORMAP",
        (setAttrFunc) &XilDeviceCompressionCell::setCompressorColorMap,
        (getAttrFunc)NULL);

    registerAttr("KEYFRAME_INTERVAL",
        (setAttrFunc) &XilDeviceCompressionCell::setKeyFrameInterval,
        (getAttrFunc) &XilDeviceCompressionCell::getKeyFrameInterval);

    registerAttr("COMPRESSOR_MAX_CMAP_SIZE",
        (setAttrFunc) &XilDeviceCompressionCell::setCompressorMaxCmapSize,
        (getAttrFunc) &XilDeviceCompressionCell::getCompressorMaxCmapSize);

    registerAttr("COMPRESSOR_FRAME_RATE",
        (setAttrFunc) &XilDeviceCompressionCell::setCompressorFrameRate,
        (getAttrFunc) &XilDeviceCompressionCell::getCompressorFrameRate);

    registerAttr("BITS_PER_SECOND",
        (setAttrFunc) &XilDeviceCompressionCell::setBitsPerSecond,
        (getAttrFunc) &XilDeviceCompressionCell::getBitsPerSecond);

    registerAttr("TEMPORAL_FILTERING",
        (setAttrFunc) &XilDeviceCompressionCell::setTemporalFiltering,
        (getAttrFunc) &XilDeviceCompressionCell::getTemporalFiltering);

    registerAttr("TEMPORAL_FILTER_LOW",
        (setAttrFunc) &XilDeviceCompressionCell::setTemporalFilterLow,
        (getAttrFunc) &XilDeviceCompressionCell::getTemporalFilterLow);

    registerAttr("TEMPORAL_FILTER_HIGH",
        (setAttrFunc) &XilDeviceCompressionCell::setTemporalFilterHigh,
        (getAttrFunc) &XilDeviceCompressionCell::getTemporalFilterHigh);

    registerAttr("COMPRESSOR_USER_DATA",
        (setAttrFunc) &XilDeviceCompressionCell::setUserData,
        (getAttrFunc)NULL);

    //
    // Register the Decompressor attributes
    //
    registerAttr("DECOMPRESSOR_COLORMAP",
        (setAttrFunc) &XilDeviceCompressionCell::setDecompressorColorMap,
        (getAttrFunc) &XilDeviceCompressionCell::getDecompressorColorMap);

    registerAttr("DECOMPRESSOR_MAX_CMAP_SIZE",
        (setAttrFunc)NULL,
        (getAttrFunc) &XilDeviceCompressionCell::getDecompressorMaxCmapSize);

    registerAttr("RDWR_INDICES",
        (setAttrFunc) &XilDeviceCompressionCell::setReadWriteIndices,
        (getAttrFunc)NULL);

    registerAttr("DECOMPRESSOR_FRAME_RATE",
        (setAttrFunc)NULL,
        (getAttrFunc) &XilDeviceCompressionCell::getDecompressorFrameRate);

    registerAttr("DECOMPRESSOR_USER_DATA",
        (setAttrFunc)NULL,
        (getAttrFunc) &XilDeviceCompressionCell::getUserData);

}

XilDeviceManagerCompressionCell::~XilDeviceManagerCompressionCell(void)
{
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

    XilDeviceManagerCompressionCell* mgr;

    mgr = new XilDeviceManagerCompressionCell();
    if(mgr == NULL) {
        XIL_ERROR(NULL, XIL_ERROR_RESOURCE, "di-1", TRUE);
    }

    return mgr;
}

//
//  constructNewDevice() is used to create new instances of
//  the Cell device compression when new CISs are created by the
//  user.
//
XilDeviceCompression*
XilDeviceManagerCompressionCell::constructNewDevice(XilCis* xcis)
{
    XilDeviceCompressionCell* device;

    device = new XilDeviceCompressionCell(this, xcis);
    if (device == NULL) {
      // out of memory
      XIL_ERROR(NULL, XIL_ERROR_RESOURCE, "di-1", TRUE);
      return NULL;
    }

    return device;
}

const char*
XilDeviceManagerCompressionCell::getDeviceName()
{
    return "Cell";
}

XilStatus
XilDeviceManagerCompressionCell::describeMembers()
{
    XilFunctionInfo*  func_info;

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "compress_Cell");
    func_info->setFunction((XilCodecFunctionPtr)
                            &XilDeviceCompressionCell::compress);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "decompress_Cell");
    func_info->setFunction((XilCodecFunctionPtr)
                            &XilDeviceCompressionCell::decompress);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "nearest_color;8->8");
    func_info->describeOp(XIL_STEP, 1, "decompress_Cell");
    func_info->setFunction((XilCodecFunctionPtr)
                            &XilDeviceCompressionCell::decompressNearestColor8,
                            "nearest_color;8->8(decompress_Cell())");
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "ordered_dither;8->8");
    func_info->describeOp(XIL_STEP, 1, "decompress_Cell");
    func_info->setFunction((XilCodecFunctionPtr)
                            &XilDeviceCompressionCell::decompressOrderedDither8,
                            "ordered_dither;8->8(decompress_Cell())");
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "ordered_dither;8->8");
    func_info->describeOp(XIL_STEP, 1, "scale_nearest;8");
    func_info->describeOp(XIL_STEP, 1, "decompress_Cell");
    func_info->setFunction((XilCodecFunctionPtr)
                            &XilDeviceCompressionCell::decompressOrderedDither8,
                            "ordered_dither;8->8(scale_nearest;8(decompress_Cell()))");
    addFunction(func_info);
    func_info->destroy();

    return XIL_SUCCESS;
}

