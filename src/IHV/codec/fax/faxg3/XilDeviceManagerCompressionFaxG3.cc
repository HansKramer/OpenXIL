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
//  File:       XilDeviceManagerCompressionFaxG3.cc
//  Project:    XIL
//  Revision:   1.6
//  Last Mod:   10:14:13, 03/10/00
//
//  Description:
//
//    DeviceManager for Group 3 Fax
//
//
//------------------------------------------------------------------------
//    COPYRIGHT
//------------------------------------------------------------------------
#pragma ident   "@(#)XilDeviceManagerCompressionFaxG3.cc	1.6\t00/03/10  "


#include "XilDeviceCompressionFaxG3.hh"
#include "XilDeviceManagerCompressionFaxG3.hh"

//
// Constructor/Destructor
//
XilDeviceManagerCompressionFaxG3::XilDeviceManagerCompressionFaxG3()
 : XilDeviceManagerCompression("faxG3", "FAXG3")
{
    //
    // Register the codec-specific attributes
    //
    registerAttr("WIDTH",
        (setAttrFunc) &XilDeviceCompressionFax::setWidth, 
        (getAttrFunc)NULL);
 
    registerAttr("HEIGHT",
        (setAttrFunc) &XilDeviceCompressionFax::setHeight,
        (getAttrFunc)NULL);
 
    registerAttr("BANDS",
        (setAttrFunc) &XilDeviceCompressionFax::setBands,
        (getAttrFunc)NULL);
}
 
XilDeviceManagerCompressionFaxG3::~XilDeviceManagerCompressionFaxG3(void)
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

    XilDeviceManagerCompressionFaxG3* mgr;
 
    mgr = new XilDeviceManagerCompressionFaxG3();
    if(mgr == NULL) {
        XIL_ERROR(NULL, XIL_ERROR_RESOURCE, "di-1", TRUE);
    }
 
    return mgr;
}
 
//
//  constructNewDevice() is used to create new instances of
//  the FaxG3 device compression when new CISs are created by the
//  user.
//
XilDeviceCompression*
XilDeviceManagerCompressionFaxG3::constructNewDevice(XilCis* xcis)
{
    XilDeviceCompressionFaxG3* device;

    device = new XilDeviceCompressionFaxG3(this, xcis);
    if (device == NULL) {
        XIL_ERROR( NULL, XIL_ERROR_RESOURCE, "di-1", TRUE); 
        return NULL;
    }

    return device;
}
 
const char*
XilDeviceManagerCompressionFaxG3::getDeviceName()
{
    return "faxG3";
}
 
XilStatus
XilDeviceManagerCompressionFaxG3::describeMembers()
{
    XilFunctionInfo*  func_info;
 
    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "compress_faxG3");
    func_info->setFunction((XilCodecFunctionPtr)
                            &XilDeviceCompressionFaxG3::compress);
    addFunction(func_info);
    func_info->destroy();
 
    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "decompress_faxG3");
    func_info->setFunction((XilCodecFunctionPtr)
                            &XilDeviceCompressionFaxG3::decompress);
    addFunction(func_info);
    func_info->destroy();
 
    return XIL_SUCCESS;
}
