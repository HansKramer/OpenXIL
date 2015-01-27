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
//  File:	XilDeviceManagerComputeFLOAT.cc
//  Project:	XIL
//  Revision:	1.42
//  Last Mod:	10:12:40, 03/10/00
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
//------------------------------------------------------------------------
//        COPYRIGHT
//------------------------------------------------------------------------
#pragma ident        "@(#)XilDeviceManagerComputeFLOAT.cc	1.42\t00/03/10  "

#include "XilDeviceManagerComputeFLOAT.hh"

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

    XilDeviceManagerComputeFLOAT* device;

    device = new XilDeviceManagerComputeFLOAT;
    
    if(device == NULL) {
        XIL_ERROR(NULL,XIL_ERROR_RESOURCE,"di-1",TRUE);
        return NULL;
    }

    return device;
}

XilDeviceManagerComputeFLOAT::XilDeviceManagerComputeFLOAT()
{
    //
    //  Initialize Persistent Data Members
    //
}

XilDeviceManagerComputeFLOAT::~XilDeviceManagerComputeFLOAT()
{
}

const char*
XilDeviceManagerComputeFLOAT::getDeviceName()
{
    return "SunSoft_XIL_FLOAT";
}

XilStatus
XilDeviceManagerComputeFLOAT::describeMembers()
{
    XilFunctionInfo*  func_info;

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "absolute;f32");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeFLOAT::Absolute);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "add;f32");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeFLOAT::Add);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "add_const;f32");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeFLOAT::AddConst);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "affine_nearest;f32");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeFLOAT::AffineNearest);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "affine_bilinear;f32");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeFLOAT::AffineBilinear);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "affine_bicubic;f32");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeFLOAT::AffineBicubic);
    addFunction(func_info);
    func_info->destroy();
        func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "affine_general;f32");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeFLOAT::AffineGeneral);
    addFunction(func_info);
    func_info->destroy();
    

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "band_combine;f32");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeFLOAT::BandCombine);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "black_generation;f32");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeFLOAT::BlackGeneration);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "blend;f32,f32,1->f32");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeFLOAT::Blenda1);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "blend;f32,f32,8->f32");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeFLOAT::Blenda8);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "blend;f32,f32,16->f32");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeFLOAT::Blenda16);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "blend;f32,f32,f32->f32");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeFLOAT::Blendaf32);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "cast;f32->1");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeFLOAT::CastTo1);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "cast;f32->8");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeFLOAT::CastTo8);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "cast;f32->16");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeFLOAT::CastTo16);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "convolve;f32");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeFLOAT::Convolve);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "separable_convolve;f32");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeFLOAT::Convolve_Separable);
    func_info->setPreprocessFunction((XilComputePreprocessFunctionPtr)
                           &XilDeviceManagerComputeFLOAT::Convolve_SeparablePreprocess);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "copy;f32");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeFLOAT::Copy);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "copy_pattern;f32");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeFLOAT::Copy);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "dilate;f32");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeFLOAT::Dilate);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "divide;f32");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeFLOAT::Divide);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "divide_by_const;f32");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeFLOAT::DivideByConst);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "divide_into_const;f32");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeFLOAT::DivideIntoConst);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "edge_detection;f32");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeFLOAT::EdgeDetection);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "erode;f32");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeFLOAT::Erode);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "error_diffusion;f32->1");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeFLOAT::ErrorDiffusion1);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "error_diffusion;f32->8");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeFLOAT::ErrorDiffusion8);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "error_diffusion;f32->16");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeFLOAT::ErrorDiffusion16);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "extrema;f32");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeFLOAT::Extrema);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "fill;f32");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeFLOAT::Fill);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "histogram;f32");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeFLOAT::Histogram);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "max;f32");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeFLOAT::Max);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "min;f32");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeFLOAT::Min);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "multiply;f32");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeFLOAT::Multiply);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "multiply_const;f32");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeFLOAT::MultiplyConst);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "nearest_color;f32->1");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeFLOAT::NearestColor1);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "nearest_color;f32->8");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeFLOAT::NearestColor8);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "nearest_color;f32->16");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeFLOAT::NearestColor16);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "ordered_dither;f32->1");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeFLOAT::OrderedDither1);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "ordered_dither;f32->8");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeFLOAT::OrderedDither8);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "ordered_dither;f32->16");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeFLOAT::OrderedDither16);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "paint;f32");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeFLOAT::Paint);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "rescale;f32");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeFLOAT::Rescale);
    addFunction(func_info);
    func_info->destroy();

    
    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "rotate_nearest;f32");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeFLOAT::RotateNearest);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "rotate_bilinear;f32");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeFLOAT::RotateBilinear);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "rotate_bicubic;f32");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeFLOAT::RotateBicubic);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "rotate_general;f32");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeFLOAT::RotateGeneral);
    addFunction(func_info);
    func_info->destroy();
    
    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "scale_nearest;f32");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeFLOAT::ScaleNearest);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "scale_bilinear;f32");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeFLOAT::ScaleBilinear);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "scale_bicubic;f32");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeFLOAT::ScaleBicubic);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "scale_general;f32");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeFLOAT::ScaleGeneral);
    addFunction(func_info);
    func_info->destroy();
    

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "set_value;f32");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeFLOAT::SetValue);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "soft_fill;f32");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeFLOAT::SoftFill);
    addFunction(func_info);
    func_info->destroy();

    // Subsample Adaptive

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "subsample_adaptive;f32");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeFLOAT::SubsampleAdaptive);
    addFunction(func_info);
    func_info->destroy();


    
    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "subtract;f32");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeFLOAT::Subtract);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "subtract_const;f32");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeFLOAT::SubtractConst);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "subtract_from_const;f32");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeFLOAT::SubtractFromConst);
    addFunction(func_info);
    func_info->destroy();

    // Tablewarp, geometric
    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "tablewarp_nearest;f32,16->f32");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeFLOAT::TablewarpNearest);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "tablewarp_nearest;f32,f32->f32");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeFLOAT::TablewarpNearest);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "tablewarp_bilinear;f32,16->f32");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeFLOAT::TablewarpBilinear);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "tablewarp_bilinear;f32,f32->f32");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeFLOAT::TablewarpBilinear);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "tablewarp_bicubic;f32,16->f32");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeFLOAT::TablewarpBicubic);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "tablewarp_bicubic;f32,f32->f32");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeFLOAT::TablewarpBicubic);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "tablewarp_general;f32,16->f32");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeFLOAT::TablewarpGeneral);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "tablewarp_general;f32,f32->f32");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeFLOAT::TablewarpGeneral);
    addFunction(func_info);
    func_info->destroy();

    // Tablewarp horizontal, geometric
    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "tablewarp_horizontal_nearest;f32,16->f32");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeFLOAT::TablewarpHorizontalNearest);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "tablewarp_horizontal_nearest;f32,f32->f32");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeFLOAT::TablewarpHorizontalNearest);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "tablewarp_horizontal_bilinear;f32,16->f32");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeFLOAT::TablewarpHorizontalBilinear);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "tablewarp_horizontal_bilinear;f32,f32->f32");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeFLOAT::TablewarpHorizontalBilinear);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "tablewarp_horizontal_bicubic;f32,16->f32");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeFLOAT::TablewarpHorizontalBicubic);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "tablewarp_horizontal_bicubic;f32,f32->f32");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeFLOAT::TablewarpHorizontalBicubic);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "tablewarp_horizontal_general;f32,16->f32");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeFLOAT::TablewarpHorizontalGeneral);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "tablewarp_horizontal_general;f32,f32->f32");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeFLOAT::TablewarpHorizontalGeneral);
    addFunction(func_info);
    func_info->destroy();

    // Tablewarp vertical, geometric
    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "tablewarp_vertical_nearest;f32,16->f32");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeFLOAT::TablewarpVerticalNearest);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "tablewarp_vertical_nearest;f32,f32->f32");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeFLOAT::TablewarpVerticalNearest);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "tablewarp_vertical_bilinear;f32,16->f32");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeFLOAT::TablewarpVerticalBilinear);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "tablewarp_vertical_bilinear;f32,f32->f32");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeFLOAT::TablewarpVerticalBilinear);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "tablewarp_vertical_bicubic;f32,16->f32");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeFLOAT::TablewarpVerticalBicubic);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "tablewarp_vertical_bicubic;f32,f32->f32");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeFLOAT::TablewarpVerticalBicubic);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "tablewarp_vertical_general;f32,16->f32");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeFLOAT::TablewarpVerticalGeneral);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "tablewarp_vertical_general;f32,f32->f32");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeFLOAT::TablewarpVerticalGeneral);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "threshold;f32");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeFLOAT::Threshold);
    addFunction(func_info);
    func_info->destroy();

    //
    //  The Threshold_1BAND function must be listed after the generic
    //  Threshold so it will be called first.
    //
    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "threshold;f32");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeFLOAT::Threshold_1BAND);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "threshold;f32");
    func_info->describeOp(XIL_STEP, 1, "threshold;f32");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeFLOAT::ThresholdThreshold,
                           "threshold;f32(threshold;f32())");
    addFunction(func_info);
    func_info->destroy();

    //
    //  Translate...
    //
    //  Nearest neighbor is the same as a copy.  The translate op takes care
    //  of the offset in determining the source and destination boxes.
    //
    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "translate_nearest;f32");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeFLOAT::Copy);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "translate_bilinear;f32");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeFLOAT::TranslateBilinear);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "translate_bicubic;f32");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeFLOAT::TranslateBicubic);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "translate_general;f32");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeFLOAT::TranslateGeneral);
    addFunction(func_info);
    func_info->destroy();
    

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "transpose;f32");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeFLOAT::Transpose);
    addFunction(func_info);
    func_info->destroy();

    return XIL_SUCCESS;
}

