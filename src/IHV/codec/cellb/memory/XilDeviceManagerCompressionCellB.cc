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
//  File:   XilDeviceManagerCompressionCellB.cc
//  Project:    XIL
//  Revision:   1.6
//  Last Mod:   10:15:34, 03/10/00
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
#pragma ident   "@(#)XilDeviceManagerCompressionCellB.cc	1.6\t00/03/10  "

#include "XilDeviceManagerCompressionCellB.hh"
#include "XilDeviceCompressionCellB.hh"

//
// Constructor/Destructor
//

XilDeviceManagerCompressionCellB::XilDeviceManagerCompressionCellB() :
   XilDeviceManagerCompression("CellB", "CELLB")
{
    //
    // Register the codec-specific attributes
    //
    registerAttr("WIDTH",
        (setAttrFunc) &XilDeviceCompressionCellB::setWidth, 
        (getAttrFunc)NULL);
 
    registerAttr("HEIGHT",
        (setAttrFunc) &XilDeviceCompressionCellB::setHeight,
        (getAttrFunc)NULL);
 
    registerAttr("IGNORE_HISTORY",
        (setAttrFunc) &XilDeviceCompressionCellB::setIgnoreHistoryFlag,
        (getAttrFunc) &XilDeviceCompressionCellB::getIgnoreHistoryFlag);

    colorConverter = NULL;
}
 
XilDeviceManagerCompressionCellB::~XilDeviceManagerCompressionCellB()
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

    XilDeviceManagerCompressionCellB* mgr;
 
    mgr = new XilDeviceManagerCompressionCellB();
    if(mgr == NULL) {
        XIL_ERROR(NULL, XIL_ERROR_RESOURCE, "di-1", TRUE);
    }
 
    return mgr;
}
 
//
//  constructNewDevice() is used to create new instances of
//  the CellB device compression when new CISs are created by the
//  user.
//
XilDeviceCompression*
XilDeviceManagerCompressionCellB::constructNewDevice(XilCis* xcis)
{
    XilDeviceCompressionCellB* device;

    device = new XilDeviceCompressionCellB(this, xcis);
    if (device == NULL) {
        XIL_ERROR( NULL, XIL_ERROR_RESOURCE, "di-1", TRUE); 
        return NULL;
    }

    return device;
}
 
const char*
XilDeviceManagerCompressionCellB::getDeviceName()
{
    return "CellB";
}
 
XilStatus
XilDeviceManagerCompressionCellB::describeMembers()
{
    XilFunctionInfo*  func_info;
 
    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "compress_CellB");
    func_info->setFunction((XilCodecFunctionPtr)
                            &XilDeviceCompressionCellB::compress);
    addFunction(func_info);
    func_info->destroy();
 
    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "decompress_CellB");
    func_info->setFunction((XilCodecFunctionPtr)
                            &XilDeviceCompressionCellB::decompress);
    addFunction(func_info);
    func_info->destroy();
 
    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "color_convert;8");
    func_info->describeOp(XIL_STEP, 1, "decompress_CellB");
    func_info->setFunction((XilCodecFunctionPtr)
                            &XilDeviceCompressionCellB::decompressColorConvert,
                            "color_convert;8(decompress_CellB())");
    addFunction(func_info);
    func_info->destroy();
 
    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "ordered_dither;8->8");
    func_info->describeOp(XIL_STEP, 1, "decompress_CellB");
    func_info->setFunction((XilCodecFunctionPtr)
                            &XilDeviceCompressionCellB::decompressOrderedDither,
                            "ordered_dither;8->8(decompress_CellB())");
    addFunction(func_info);
    func_info->destroy();
 
    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "ordered_dither;8->8");
    func_info->describeOp(XIL_STEP, 1, "rescale;8");
    func_info->describeOp(XIL_STEP, 1, "decompress_CellB");
    func_info->setFunction((XilCodecFunctionPtr)
                            &XilDeviceCompressionCellB::decompressOrderedDither,
                            "ordered_dither;8->8(rescale;8(decompress_CellB()))");
    addFunction(func_info);
    func_info->destroy();
 
    return XIL_SUCCESS;
}

Ycc2RgbConverter*
XilDeviceManagerCompressionCellB::getColorConverter()
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

