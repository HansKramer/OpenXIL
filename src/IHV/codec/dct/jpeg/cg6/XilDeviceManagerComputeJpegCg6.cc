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
//  File:       XilDeviceManagerComputeJpegCg6.cc
//  Project:    XIL
//  Revision:   1.4
//  Last Mod:   10:14:22, 03/10/00
//
//  Description:
//
//    TODO: Enter some descriptive text here
//
//
//------------------------------------------------------------------------
//    COPYRIGHT
//------------------------------------------------------------------------
#pragma ident   "@(#)XilDeviceManagerComputeJpegCg6.cc	1.4\t00/03/10  "

#include "XilDeviceManagerComputeJpegCg6.hh"

//
// Constructor - Register codec-specific attributes
//
XilDeviceManagerComputeJpegCg6::XilDeviceManagerComputeJpegCg6()
{
};

//
// Destructor
//
XilDeviceManagerComputeJpegCg6::~XilDeviceManagerComputeJpegCg6()
{
}

//
// Create a new device manager
// (One per compression type)
//
XilDeviceManagerCompute*
XilDeviceManagerCompute::create(unsigned int  libxil_gpi_major,
                                unsigned int  libxil_gpi_minor,
                                unsigned int* devhandler_gpi_major,
                                unsigned int* devhandler_gpi_minor)
{
    XIL_BASIC_GPI_VERSION_TEST(libxil_gpi_major,
                               libxil_gpi_minor,
                               devhandler_gpi_major,
                               devhandler_gpi_minor);

    return new XilDeviceManagerComputeJpegCg6();
}
 
const char*
XilDeviceManagerComputeJpegCg6::getDeviceName()
{
    return "JpegCg6";
}
 
XilStatus
XilDeviceManagerComputeJpegCg6::describeMembers()
{
    XilFunctionInfo*  func_info;
 
    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "display_SUNWcg6");
    func_info->describeOp(XIL_STEP, 1, "color_convert;8");
    func_info->describeOp(XIL_STEP, 1, "decompress_Jpeg");
    func_info->setFunction((XilComputeFunctionPtr)
        &XilDeviceManagerComputeJpegCg6::decompressDither8Cg6,
        "display_SUNWcg6(color_convert;8(decompress_Jpeg()))" );
    addFunction(func_info);
    func_info->destroy();
 
    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "display_SUNWcg6");
    func_info->describeOp(XIL_STEP, 1, "copy;8");
    func_info->describeOp(XIL_STEP, 1, "color_convert;8");
    func_info->describeOp(XIL_STEP, 1, "decompress_Jpeg");
    func_info->setFunction((XilComputeFunctionPtr)
        &XilDeviceManagerComputeJpegCg6::decompressDither8Cg6,
        "display_SUNWcg6(copy;8(color_convert;8(decompress_Jpeg())))" );
    addFunction(func_info);
    func_info->destroy();
 

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "display_SUNWcg6");
    func_info->describeOp(XIL_STEP, 1, "scale;8");
    func_info->describeOp(XIL_STEP, 1, "color_convert;8");
    func_info->describeOp(XIL_STEP, 1, "decompress_Jpeg");
    func_info->setFunction((XilComputeFunctionPtr)
        &XilDeviceManagerComputeJpegCg6::decompressDither8Cg6,
        "display_SUNWcg6(scale;8(color_convert;8(decompress_Jpeg())))" );
    addFunction(func_info);
    func_info->destroy();
 

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "display_SUNWcg6");
    func_info->describeOp(XIL_STEP, 1, "copy;8");
    func_info->describeOp(XIL_STEP, 1, "scale;8");
    func_info->describeOp(XIL_STEP, 1, "color_convert;8");
    func_info->describeOp(XIL_STEP, 1, "decompress_Jpeg");
    func_info->setFunction((XilComputeFunctionPtr)
        &XilDeviceManagerComputeJpegCg6::decompressDither8Cg6,
        "display_SUNWcg6(copy;8(scale;8((color_convert;8(decompress_Jpeg()))))" );
    addFunction(func_info);
    func_info->destroy();
 

 
    return XIL_SUCCESS;
}
