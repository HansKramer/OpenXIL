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
//  File:	XilDeviceManagerCompressionIdentity.cc
//  Project:	XIL
//  Revision:	1.7
//  Last Mod:	10:14:07, 03/10/00
//
//  Description:
//------------------------------------------------------------------------
#pragma ident	"@(#)XilDeviceManagerCompressionIdentity.cc	1.7\t00/03/10  "


#include "XilDeviceManagerCompressionIdentity.hh"
#include "XilDeviceCompressionIdentity.hh"

//------------------------------------------------------------------------
//
//  Function:	create
//
//  Description:
//    create() is called when the XIL core opens the xilIdentity.so 
//    library.  It is responsible for creating the 
//    XilDeviceManagerCompressionIdentity class.
//
//    Note that this function is a member of the PARENT class.
//
//------------------------------------------------------------------------
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

    XilDeviceManagerCompression* mgr;

    mgr = new XilDeviceManagerCompressionIdentity();
    if(mgr == NULL) {
        XIL_ERROR(NULL, XIL_ERROR_RESOURCE, "di-1", TRUE);
    }   

    return mgr;
}

//------------------------------------------------------------------------
//
//  Function:	XilDeviceManagerCompressionIdentity::constructNewDevice()
//
//  Description:
//	constructNewDevice() is used to create new instances of
//  the Identity device compression when new CISs are created by the
//  user.
//	
//------------------------------------------------------------------------
XilDeviceCompression*
XilDeviceManagerCompressionIdentity::constructNewDevice(XilCis* xcis)
{
    XilDeviceCompressionIdentity* device;

    device =  new XilDeviceCompressionIdentity(this, xcis);
    if(device == NULL) {
        XIL_ERROR(xcis->getSystemState(), XIL_ERROR_RESOURCE, "di-TODO", TRUE);
        return NULL;
    }

    return device;
}

const char*
XilDeviceManagerCompressionIdentity::getDeviceName()
{
    return "Identity";
}

XilStatus
XilDeviceManagerCompressionIdentity::describeMembers()
{
    XilFunctionInfo*  func_info;
 
    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "compress_Identity");
    func_info->setFunction((XilCodecFunctionPtr)
                            &XilDeviceCompressionIdentity::compress);
    addFunction(func_info);
    func_info->destroy();
 
    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "decompress_Identity");
    func_info->setFunction((XilCodecFunctionPtr)
                            &XilDeviceCompressionIdentity::decompress);
    addFunction(func_info);
    func_info->destroy();

    return XIL_SUCCESS;
}


//------------------------------------------------------------------------
//
//  Function:	XilDeviceManagerCompressionIdentity()
//
//  Description:
//	The device compression type constructor initializes any
//  Identity compression type specific data and registers all of the
//  Identity attributes with the XIL core.
//	
//------------------------------------------------------------------------

XilDeviceManagerCompressionIdentity::XilDeviceManagerCompressionIdentity()
 : XilDeviceManagerCompression("Identity","IDENTITY")
{


   // any attributes which the codec would like to provide access
   // to via the xil_cis_set_attribute() and/or xil_cis_get_attribute()
   // bindings must be registered here.
   // NOTE: These attribute routines are registered here as an 
   // example ONLY...the Identity codec does not make use of
   // "quality" ...it is just an example of the registerAttr mechanism.

    registerAttr("COMPRESSION_QUALITY",
        (setAttrFunc) &XilDeviceCompressionIdentity::setCompressionQuality,
        (getAttrFunc) &XilDeviceCompressionIdentity::getCompressionQuality);

    registerAttr("DECOMPRESSION_QUALITY",
        (setAttrFunc) &XilDeviceCompressionIdentity::setDecompressionQuality,
        (getAttrFunc) &XilDeviceCompressionIdentity::getDecompressionQuality);
}

XilDeviceManagerCompressionIdentity::~XilDeviceManagerCompressionIdentity(void) 
{ 
}
