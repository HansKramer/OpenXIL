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
//  File:       XilDeviceManagerComputeCellCg6.cc
//  Project:    XIL
//  Revision:   1.3
//  Last Mod:   10:15:40, 03/10/00
//
//  Description:
//
//    TODO: Enter some descriptive text here
//
//
//------------------------------------------------------------------------
//    COPYRIGHT
//------------------------------------------------------------------------
#pragma ident   "@(#)XilDeviceManagerComputeCellCg6.cc	1.3\t00/03/10  "

#include "XilDeviceManagerComputeCellCg6.hh"

//
// Constructor - Register codec-specific attributes
//
XilDeviceManagerComputeCellCg6::XilDeviceManagerComputeCellCg6()
{
};

//
// Destructor
//
XilDeviceManagerComputeCellCg6::~XilDeviceManagerComputeCellCg6()
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

    return new XilDeviceManagerComputeCellCg6;
}
 
const char*
XilDeviceManagerComputeCellCg6::getDeviceName()
{
    return "CellCg6";
}
 
XilStatus
XilDeviceManagerComputeCellCg6::describeMembers()
{
    XilFunctionInfo*  func_info;
 
    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "display_SUNWcg6");
    func_info->describeOp(XIL_STEP, 1, "color_convert;8");
    func_info->describeOp(XIL_STEP, 1, "decompress_Cell");
    func_info->setFunction((XilComputeFunctionPtr)
        &XilDeviceManagerComputeCellCg6::decompressDither8Cg6,
        "display_SUNWcg6(color_convert;8(decompress_Cell()))" );
    addFunction(func_info);
    func_info->destroy();
 
    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "display_SUNWcg6");
    func_info->describeOp(XIL_STEP, 1, "copy;8");
    func_info->describeOp(XIL_STEP, 1, "color_convert;8");
    func_info->describeOp(XIL_STEP, 1, "decompress_Cell");
    func_info->setFunction((XilComputeFunctionPtr)
        &XilDeviceManagerComputeCellCg6::decompressDither8Cg6,
        "display_SUNWcg6(copy;8(color_convert;8(decompress_Cell())))" );
    addFunction(func_info);
    func_info->destroy();
 

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "display_SUNWcg6");
    func_info->describeOp(XIL_STEP, 1, "scale;8");
    func_info->describeOp(XIL_STEP, 1, "color_convert;8");
    func_info->describeOp(XIL_STEP, 1, "decompress_Cell");
    func_info->setFunction((XilComputeFunctionPtr)
        &XilDeviceManagerComputeCellCg6::decompressDither8Cg6,
        "display_SUNWcg6(scale;8(color_convert;8(decompress_Cell())))" );
    addFunction(func_info);
    func_info->destroy();
 

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "display_SUNWcg6");
    func_info->describeOp(XIL_STEP, 1, "copy;8");
    func_info->describeOp(XIL_STEP, 1, "scale;8");
    func_info->describeOp(XIL_STEP, 1, "color_convert;8");
    func_info->describeOp(XIL_STEP, 1, "decompress_Cell");
    func_info->setFunction((XilComputeFunctionPtr)
        &XilDeviceManagerComputeCellCg6::decompressDither8Cg6,
        "display_SUNWcg6(copy;8(scale;8((color_convert;8(decompress_Cell()))))" );
    addFunction(func_info);
    func_info->destroy();
 

 
    return XIL_SUCCESS;
}
