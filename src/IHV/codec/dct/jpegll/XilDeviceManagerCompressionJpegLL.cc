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
//  File:       XilDeviceManagerCompressionJpegLL.cc
//  Project:    XIL
//  Revision:   1.4
//  Last Mod:   10:15:04, 03/10/00
//
//  Description:
//
//    Manager class. Registers codec-specific attributes.
//    Constructs the DeviceCompression object.
//
//
//------------------------------------------------------------------------
//    COPYRIGHT
//------------------------------------------------------------------------
#pragma ident   "@(#)XilDeviceManagerCompressionJpegLL.cc	1.4\t00/03/10  "

#include "XilDeviceManagerCompressionJpegLL.hh"
#include "XilDeviceCompressionJpegLL.hh"

//
// Constructor - Register codec-specific attributes
//
XilDeviceManagerCompressionJpegLL::XilDeviceManagerCompressionJpegLL()
: XilDeviceManagerCompression("JpegLL", "JPEGLL") 
{

    registerAttr("COMPRESSED_DATA_FORMAT",
                 (setAttrFunc) &XilDeviceCompressionJpegLL::setAbbrFormat,
                 (getAttrFunc)NULL);

    registerAttr("HUFFMAN_TABLE",
                 (setAttrFunc) &XilDeviceCompressionJpegLL::setHTable,
                 (getAttrFunc)NULL);

    registerAttr("BAND_HUFFMAN_TABLE",
                 (setAttrFunc) &XilDeviceCompressionJpegLL::setBandHTable,
                 (getAttrFunc)NULL);

    registerAttr("OPTIMIZE_HUFFMAN_TABLES",
                 (setAttrFunc) &XilDeviceCompressionJpegLL::setOptHTables,
                 (getAttrFunc)NULL);

    registerAttr("ENCODE_INTERLEAVED",
                 (setAttrFunc) &XilDeviceCompressionJpegLL::setEncodeInterleaved,
                 (getAttrFunc)NULL);

    registerAttr("LOSSLESS_BAND_PT_TRANSFORM",
                 (setAttrFunc) &XilDeviceCompressionJpegLL::setBandPtTransform,
                 (getAttrFunc)NULL);

    registerAttr("LOSSLESS_BAND_SELECTOR",
                 (setAttrFunc) &XilDeviceCompressionJpegLL::setBandSelector,
                 (getAttrFunc)NULL);

};

//
// Destructor
//
XilDeviceManagerCompressionJpegLL::~XilDeviceManagerCompressionJpegLL()
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

    XilDeviceManagerCompressionJpegLL* mgr;

    mgr = new XilDeviceManagerCompressionJpegLL();
    if(mgr == NULL) {
        XIL_ERROR(NULL, XIL_ERROR_RESOURCE, "di-1", TRUE);
    }

    return mgr;
}

//
//  constructNewDevice() is used to create new instances of
//  the JpegLL device compression when new CISs are created by the
//  user.
//
XilDeviceCompression*
XilDeviceManagerCompressionJpegLL::constructNewDevice(XilCis* xcis)
{
    XilDeviceCompressionJpegLL* device;

    device = new XilDeviceCompressionJpegLL(this, xcis);
    if(device == NULL) {
        XIL_ERROR( NULL, XIL_ERROR_RESOURCE, "di-1", TRUE); 
        return NULL;
    }

    return device;
}
 
const char*
XilDeviceManagerCompressionJpegLL::getDeviceName()
{
    return "JpegLL";
}
 
XilStatus
XilDeviceManagerCompressionJpegLL::describeMembers()
{
    XilFunctionInfo*  func_info;

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "compress_JpegLL");
    func_info->setFunction((XilCodecFunctionPtr)
                            &XilDeviceCompressionJpegLL::compress);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "decompress_JpegLL");
    func_info->setFunction((XilCodecFunctionPtr)
                            &XilDeviceCompressionJpegLL::decompress);
    addFunction(func_info);
    func_info->destroy();

    return XIL_SUCCESS;
}
