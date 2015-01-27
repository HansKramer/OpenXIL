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
//  File:	XilDeviceManagerComputeBIT.cc
//  Project:	XIL
//  Revision:	1.52
//  Last Mod:	10:09:16, 03/10/00
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
//	COPYRIGHT
//------------------------------------------------------------------------
#pragma ident	"@(#)XilDeviceManagerComputeBIT.cc	1.52\t00/03/10  "

#include <string.h>
#include "XilDeviceManagerComputeBIT.hh"
#include "XiliUtils.hh"

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

    XilDeviceManagerComputeBIT* device;

    device = new XilDeviceManagerComputeBIT;
    
    if(device == NULL) {
        XIL_ERROR(NULL,XIL_ERROR_RESOURCE,"di-1",TRUE);
        return NULL;
    }

    return device;
}

XilDeviceManagerComputeBIT::XilDeviceManagerComputeBIT()
{
    //
    //  Initialize Persistent Data Members
    //
    edgeDetectTable = NULL;
    byteCastArray = NULL;
    shortCastArray = NULL;
    floatCastArray = NULL;
    squaresTable = NULL;
    castByteTable = NULL;
    castByteUpperTable = NULL;
    castByteLowerTable = NULL;
    castShortTable = NULL;
    castFloat32Table = NULL;
}

XilDeviceManagerComputeBIT::~XilDeviceManagerComputeBIT()
{
    delete [] byteCastArray;
    delete [] shortCastArray;
    delete [] floatCastArray;
    delete [] squaresTable;
    delete [] castByteTable;
    delete [] castByteUpperTable;
    delete [] castByteLowerTable;
    delete [] castShortTable;
    delete [] castFloat32Table;

    if(edgeDetectTable != NULL) {
        for(int h=-4; h<=4; h++) {
            delete [] (edgeDetectTable[h] - 4);
        }
        delete [] (edgeDetectTable - 4);
    }
}

const char*
XilDeviceManagerComputeBIT::getDeviceName()
{
    return "SunSoft_XIL_BIT";
}

XilStatus
XilDeviceManagerComputeBIT::describeMembers()
{
    XilFunctionInfo*  func_info;
    
    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "add;1");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeBIT::OrMaxAdd);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "add_const;1");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeBIT::AddConst);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "affine_nearest;1");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeBIT::AffineNearest);
    addFunction(func_info);
    func_info->destroy();
    
    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "affine_bilinear;1");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeBIT::AffineBilinear);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "affine_bicubic;1");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeBIT::AffineBicubic);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "affine_general;1");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeBIT::AffineGeneral);
    addFunction(func_info);
    func_info->destroy();
    
    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "and;1");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeBIT::AndMinMultiply);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "and_const;1");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeBIT::AndConst);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "band_combine;1");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeBIT::BandCombine);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "blend;1,1,1->1");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeBIT::Blenda1);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "blend;1,1,8->1");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeBIT::Blenda8);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "blend;1,1,16->1");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeBIT::Blenda16);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "blend;1,1,f32->1");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeBIT::Blendaf32);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "cast;1->8");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeBIT::CastTo8);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "cast;1->16");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeBIT::CastTo16);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "cast;1->f32");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeBIT::CastTof32);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "convolve;1");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeBIT::Convolve);
    addFunction(func_info);
    func_info->destroy();
        
    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "copy;1");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeBIT::Copy);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "copy_pattern;1");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeBIT::Copy);
    addFunction(func_info);
    func_info->destroy();
        
    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "copy_with_planemask;1");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeBIT::CopyWithPlanemask);
    addFunction(func_info);
    func_info->destroy();
        
    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "dilate;1");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeBIT::Dilate);
    addFunction(func_info);
    func_info->destroy();
        
    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "divide;1");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeBIT::Divide);
    addFunction(func_info);
    func_info->destroy();
        
    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "divide_by_const;1");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeBIT::DivideByConst);
    addFunction(func_info);
    func_info->destroy();
        
    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "divide_into_const;1");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeBIT::DivideIntoConst);
    addFunction(func_info);
    func_info->destroy();
        
    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "edge_detection;1");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeBIT::EdgeDetection);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "erode;1");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeBIT::Erode);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "error_diffusion;1->1");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeBIT::ErrorDiffusion1);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "error_diffusion;1->8");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeBIT::ErrorDiffusion8);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "error_diffusion;1->16");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeBIT::ErrorDiffusion16);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "extrema;1");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeBIT::Extrema);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "fill;1");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeBIT::Fill);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "histogram;1");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeBIT::Histogram);
    addFunction(func_info);
    func_info->destroy();
        
    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "lookup;1->1");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeBIT::LookupTo1);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "lookup;1->8");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeBIT::LookupTo8);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "lookup;1->16");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeBIT::LookupTo16);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "lookup;1->f32");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeBIT::LookupTof32);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "lookup_n_to_n;1->1");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeBIT::LookupCombined1);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "lookup_n_to_n;1->8");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeBIT::LookupCombined8);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "lookup_n_to_n;1->16");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeBIT::LookupCombined16);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "lookup_n_to_n;1->f32");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeBIT::LookupCombinedf32);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "max;1");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeBIT::OrMaxAdd);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "min;1");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeBIT::AndMinMultiply);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "multiply;1");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeBIT::AndMinMultiply);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "multiply_const;1");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeBIT::MultiplyConst);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "nearest_color;1->1");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeBIT::NearestColor1);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "nearest_color;1->8");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeBIT::NearestColor8);
    func_info->setPreprocessFunction((XilComputePreprocessFunctionPtr)
                    &XilDeviceManagerComputeBIT::NearestColor8Preprocess);
    func_info->setPostprocessFunction((XilComputePostprocessFunctionPtr)
                    &XilDeviceManagerComputeBIT::NearestColor8Postprocess);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "nearest_color;1->16");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeBIT::NearestColor16);
    func_info->setPreprocessFunction((XilComputePreprocessFunctionPtr)
                    &XilDeviceManagerComputeBIT::NearestColor16Preprocess);
    func_info->setPostprocessFunction((XilComputePostprocessFunctionPtr)
                    &XilDeviceManagerComputeBIT::NearestColor16Postprocess);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "not;1");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeBIT::Not);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "or;1");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeBIT::OrMaxAdd);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "or_const;1");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeBIT::OrConst);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "ordered_dither;1->1");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeBIT::OrderedDither1);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "ordered_dither;1->8");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeBIT::OrderedDither8);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "ordered_dither;1->16");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeBIT::OrderedDither16);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "paint;1");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeBIT::Paint);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "rescale;1");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeBIT::Rescale);
    addFunction(func_info);
    func_info->destroy();

    
    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "rotate_nearest;1");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeBIT::RotateNearest);
    addFunction(func_info);
    func_info->destroy();
    
    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "rotate_bilinear;1");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeBIT::RotateBilinear);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "rotate_bicubic;1");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeBIT::RotateBicubic);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "rotate_general;1");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeBIT::RotateGeneral);
    addFunction(func_info);
    func_info->destroy();
      
    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "scale_nearest;1");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeBIT::ScaleNearest);
    addFunction(func_info);
    func_info->destroy();
    
    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "scale_bilinear;1");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeBIT::ScaleBilinear);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "scale_bicubic;1");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeBIT::ScaleBicubic);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "scale_general;1");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeBIT::ScaleGeneral);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "set_value;1");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeBIT::SetValue);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "soft_fill;1");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeBIT::SoftFill);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "squeeze_range;1");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeBIT::SqueezeRange);
    addFunction(func_info);
    func_info->destroy();

    // Subsample Adaptive
    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "subsample_adaptive;1");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeBIT::SubsampleAdaptive);
    addFunction(func_info);
    func_info->destroy();
    

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "subtract;1");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeBIT::Subtract);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "subtract_const;1");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeBIT::SubtractConst);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "subtract_from_const;1");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeBIT::SubtractFromConst);
    addFunction(func_info);
    func_info->destroy();

    // Tablewarp, geometric
    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "tablewarp_nearest;1,16->1");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeBIT::TablewarpNearest);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "tablewarp_nearest;1,f32->1");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeBIT::TablewarpNearest);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "tablewarp_bilinear;1,16->1");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeBIT::TablewarpBilinear);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "tablewarp_bilinear;1,f32->1");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeBIT::TablewarpBilinear);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "tablewarp_bicubic;1,16->1");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeBIT::TablewarpBicubic);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "tablewarp_bicubic;1,f32->1");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeBIT::TablewarpBicubic);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "tablewarp_general;1,16->1");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeBIT::TablewarpGeneral);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "tablewarp_general;1,f32->1");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeBIT::TablewarpGeneral);
    addFunction(func_info);
    func_info->destroy();

    // Tablewarp horizontal, geometric
    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "tablewarp_horizontal_nearest;1,16->1");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeBIT::TablewarpHorizontalNearest);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "tablewarp_horizontal_nearest;1,f32->1");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeBIT::TablewarpHorizontalNearest);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "tablewarp_horizontal_bilinear;1,16->1");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeBIT::TablewarpHorizontalBilinear);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "tablewarp_horizontal_bilinear;1,f32->1");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeBIT::TablewarpHorizontalBilinear);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "tablewarp_horizontal_bicubic;1,16->1");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeBIT::TablewarpHorizontalBicubic);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "tablewarp_horizontal_bicubic;1,f32->1");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeBIT::TablewarpHorizontalBicubic);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "tablewarp_horizontal_general;1,16->1");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeBIT::TablewarpHorizontalGeneral);
    addFunction(func_info);
    func_info->destroy();    

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "tablewarp_horizontal_general;1,f32->1");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeBIT::TablewarpHorizontalGeneral);
    addFunction(func_info);
    func_info->destroy();    

    // Tablewarp vertical, geometric
    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "tablewarp_vertical_nearest;1,16->1");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeBIT::TablewarpVerticalNearest);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "tablewarp_vertical_nearest;1,f32->1");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeBIT::TablewarpVerticalNearest);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "tablewarp_vertical_bilinear;1,16->1");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeBIT::TablewarpVerticalBilinear);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "tablewarp_vertical_bilinear;1,f32->1");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeBIT::TablewarpVerticalBilinear);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "tablewarp_vertical_bicubic;1,16->1");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeBIT::TablewarpVerticalBicubic);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "tablewarp_vertical_bicubic;1,f32->1");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeBIT::TablewarpVerticalBicubic);
    addFunction(func_info);
    func_info->destroy();


    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "tablewarp_vertical_general;1,16->1");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeBIT::TablewarpVerticalGeneral);
    addFunction(func_info);
    func_info->destroy();    

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "tablewarp_vertical_general;1,f32->1");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeBIT::TablewarpVerticalGeneral);
    addFunction(func_info);
    func_info->destroy();    

    
    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "threshold;1");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeBIT::Threshold);
    addFunction(func_info);
    func_info->destroy();

    //
    //  Translate...
    //
    //  Nearest neighbor is the same as a copy.  The translate op takes care
    //  of the offset in determining the source and destination boxes.
    //
    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "translate_nearest;1");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeBIT::Copy);
    addFunction(func_info);
    func_info->destroy();
    
        func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "translate_bilinear;1");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeBIT::TranslateBilinear);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "translate_bicubic;1");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeBIT::TranslateBicubic);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "translate_general;1");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeBIT::TranslateGeneral);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "transpose;1");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeBIT::Transpose);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "xor;1");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeBIT::Xor);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "xor_const;1");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeBIT::XorConst);
    addFunction(func_info);
    func_info->destroy();

    return XIL_SUCCESS;
}

Xil_unsigned8*
XilDeviceManagerComputeBIT::getByteCastArray()
{
    byteCastArrayMutex.lock();
    if (byteCastArray == NULL) {
        byteCastArray = new Xil_unsigned8[256*8];

        if (byteCastArray == NULL) {
            XIL_ERROR(NULL, XIL_ERROR_RESOURCE, "di-1", TRUE); 
            byteCastArrayMutex.unlock();
            return NULL;
        }

        int i;

        memset(byteCastArray, 0, 256*8);

        for (i=0; i<256; i++) {
            if ((i & 0x80) != 0)  byteCastArray[i<<3] = 1;
            if ((i & 0x40) != 0)  byteCastArray[(i<<3) + 1] = 1;
            if ((i & 0x20) != 0)  byteCastArray[(i<<3) + 2] = 1;
            if ((i & 0x10) != 0)  byteCastArray[(i<<3) + 3] = 1;
            if ((i & 0x08) != 0)  byteCastArray[(i<<3) + 4] = 1;
            if ((i & 0x04) != 0)  byteCastArray[(i<<3) + 5] = 1;
            if ((i & 0x02) != 0)  byteCastArray[(i<<3) + 6] = 1;
            if ((i & 0x01) != 0)  byteCastArray[(i<<3) + 7] = 1;
        }
    }
    byteCastArrayMutex.unlock();
    return byteCastArray;
}

Xil_signed16*
XilDeviceManagerComputeBIT::getShortCastArray()
{
    shortCastArrayMutex.lock();
    if (shortCastArray == NULL) {
        shortCastArray = new Xil_signed16[256*8];

        if (shortCastArray == NULL) {
            XIL_ERROR(NULL, XIL_ERROR_RESOURCE, "di-1", TRUE); 
            shortCastArrayMutex.unlock();
            return NULL;
        }

        int i;

        memset(shortCastArray, 0, 256*8*sizeof(Xil_signed16));

        for (i=0; i<256; i++) {
            if ((i & 0x80) != 0) shortCastArray[i<<3] = 1;
            if ((i & 0x40) != 0) shortCastArray[(i<<3) + 1] = 1;
            if ((i & 0x20) != 0) shortCastArray[(i<<3) + 2] = 1;
            if ((i & 0x10) != 0) shortCastArray[(i<<3) + 3] = 1;
            if ((i & 0x08) != 0) shortCastArray[(i<<3) + 4] = 1;
            if ((i & 0x04) != 0) shortCastArray[(i<<3) + 5] = 1;
            if ((i & 0x02) != 0) shortCastArray[(i<<3) + 6] = 1;
            if ((i & 0x01) != 0) shortCastArray[(i<<3) + 7] = 1;
        }
    }
    shortCastArrayMutex.unlock();
    return shortCastArray;
}

Xil_float32*
XilDeviceManagerComputeBIT::getFloatCastArray()
{
    floatCastArrayMutex.lock();
    if (floatCastArray == NULL) {
        floatCastArray = new Xil_float32[256*8];

        if (floatCastArray == NULL) {
            XIL_ERROR(NULL, XIL_ERROR_RESOURCE, "di-1", TRUE); 
            floatCastArrayMutex.unlock();
            return NULL;
        }

        int i;

        for (i=256*8; i>0; i--) {
            floatCastArray[i] = 0.0;
        }

        for (i=0; i<256; i++) {
            if ((i & 0x80) != 0) floatCastArray[i<<3] = 1.0;
            if ((i & 0x40) != 0) floatCastArray[(i<<3) + 1] = 1.0;
            if ((i & 0x20) != 0) floatCastArray[(i<<3) + 2] = 1.0;
            if ((i & 0x10) != 0) floatCastArray[(i<<3) + 3] = 1.0;
            if ((i & 0x08) != 0) floatCastArray[(i<<3) + 4] = 1.0;
            if ((i & 0x04) != 0) floatCastArray[(i<<3) + 5] = 1.0;
            if ((i & 0x02) != 0) floatCastArray[(i<<3) + 6] = 1.0;
            if ((i & 0x01) != 0) floatCastArray[(i<<3) + 7] = 1.0;
        }
    }
    floatCastArrayMutex.unlock();
    return floatCastArray;
}


Xil_unsigned32 *
XilDeviceManagerComputeBIT::getSquaresTable()
{
    squaresTableMutex.lock();
    if(squaresTable == NULL) {
        squaresTable = new Xil_unsigned32[514];
        if(squaresTable == NULL) {
            XIL_ERROR(NULL,XIL_ERROR_RESOURCE,"di-1",TRUE);
            squaresTableMutex.unlock();
            return NULL;
        }
            
        for(int i=0; i<257; i++)
            squaresTable[i] = (256-i)*(256-i);
        
        for(i=0; i<257; i++)
            squaresTable[256+i] = i*i;
    }
    squaresTableMutex.unlock();
    
    return &squaresTable[256];
}

XilStatus
XilDeviceManagerComputeBIT::getCastByteTable()
{
    castByteTableMutex.lock();
    if (castByteTable == NULL) {
        castByteTable = new struct XilCastTable8[256];
        if (castByteTable == NULL) {
            XIL_ERROR(NULL,XIL_ERROR_RESOURCE,"di-1",TRUE);
            castByteTableMutex.unlock();
            return XIL_FAILURE;
        }

        castByteUpperTable = new int[256];
        if (castByteUpperTable == NULL) {
            XIL_ERROR(NULL,XIL_ERROR_RESOURCE,"di-1",TRUE);
            delete [] castByteTable;
            castByteTableMutex.unlock();
            return XIL_FAILURE;
        }

        castByteLowerTable = new int[256];
        if (castByteLowerTable == NULL) {
            XIL_ERROR(NULL,XIL_ERROR_RESOURCE,"di-1",TRUE);
            delete [] castByteTable;
            delete [] castByteUpperTable;
            castByteTableMutex.unlock();
            return XIL_FAILURE;
        }

        for (int i = 0; i< 256; i++) {
            unsigned char index = i;

            castByteTable[i].b[0] = XIL_BMAP_TST(&index, 0);
            castByteTable[i].b[1] = XIL_BMAP_TST(&index, 1);
            castByteTable[i].b[2] = XIL_BMAP_TST(&index, 2);
            castByteTable[i].b[3] = XIL_BMAP_TST(&index, 3);
            castByteTable[i].b[4] = XIL_BMAP_TST(&index, 4);
            castByteTable[i].b[5] = XIL_BMAP_TST(&index, 5);
            castByteTable[i].b[6] = XIL_BMAP_TST(&index, 6);
            castByteTable[i].b[7] = XIL_BMAP_TST(&index, 7);

            ((unsigned char *) (&castByteUpperTable[i]))[0] = (index & 0x80) ? 1 : 0;
            ((unsigned char *) (&castByteUpperTable[i]))[1] = (index & 0x40) ? 1 : 0;
            ((unsigned char *) (&castByteUpperTable[i]))[2] = (index & 0x20) ? 1 : 0;
            ((unsigned char *) (&castByteUpperTable[i]))[3] = (index & 0x10) ? 1 : 0;

            ((unsigned char *) (&castByteLowerTable[i]))[0] = (index & 0x8) ? 1 : 0;
            ((unsigned char *) (&castByteLowerTable[i]))[1] = (index & 0x4) ? 1 : 0;
            ((unsigned char *) (&castByteLowerTable[i]))[2] = (index & 0x2) ? 1 : 0;
            ((unsigned char *) (&castByteLowerTable[i]))[3] = (index & 0x1) ? 1 : 0;

        }
    }
    
    castByteTableMutex.unlock();
    return XIL_SUCCESS;
}
       

XilStatus
XilDeviceManagerComputeBIT::getCastShortTable()
{
    castShortTableMutex.lock();
    if (castShortTable == NULL) {
        castShortTable = new struct XilCastTable16[256];
        if (castShortTable == NULL) {
            XIL_ERROR(NULL,XIL_ERROR_RESOURCE,"di-1",TRUE);
            castShortTableMutex.unlock();
            return XIL_FAILURE;
        }
        
#define BIT_TST_16(x)  (Xil_signed16)((x) ? (1) : (0))

        for(int i = 0; i < 256; i++) {
            unsigned char index = i;

            castShortTable[i].b[0] = BIT_TST_16(XIL_BMAP_TST((&index), 0));
            castShortTable[i].b[1] = BIT_TST_16(XIL_BMAP_TST((&index), 1));
            castShortTable[i].b[2] = BIT_TST_16(XIL_BMAP_TST((&index), 2));
            castShortTable[i].b[3] = BIT_TST_16(XIL_BMAP_TST((&index), 3));
            castShortTable[i].b[4] = BIT_TST_16(XIL_BMAP_TST((&index), 4));
            castShortTable[i].b[5] = BIT_TST_16(XIL_BMAP_TST((&index), 5));
            castShortTable[i].b[6] = BIT_TST_16(XIL_BMAP_TST((&index), 6));
            castShortTable[i].b[7] = BIT_TST_16(XIL_BMAP_TST((&index), 7));
        }
    }

    castShortTableMutex.unlock();
    return XIL_SUCCESS;
}
       
XilStatus
XilDeviceManagerComputeBIT::getCastFloat32Table()
{
    castFloat32TableMutex.lock();
    if (castFloat32Table == NULL) {
        castFloat32Table = new struct XilCastTablef32[256];
        if (castFloat32Table == NULL) {
            XIL_ERROR(NULL,XIL_ERROR_RESOURCE,"di-1",TRUE);
            castFloat32TableMutex.unlock();
            return XIL_FAILURE;
        }
        
#define BIT_TST_F32(x)  (Xil_float32)((x) ? (1) : (0))

        for(int i = 0; i < 256; i++) {
            unsigned char index = i;

            castFloat32Table[i].b[0] = BIT_TST_F32(XIL_BMAP_TST((&index), 0));
            castFloat32Table[i].b[1] = BIT_TST_F32(XIL_BMAP_TST((&index), 1));
            castFloat32Table[i].b[2] = BIT_TST_F32(XIL_BMAP_TST((&index), 2));
            castFloat32Table[i].b[3] = BIT_TST_F32(XIL_BMAP_TST((&index), 3));
            castFloat32Table[i].b[4] = BIT_TST_F32(XIL_BMAP_TST((&index), 4));
            castFloat32Table[i].b[5] = BIT_TST_F32(XIL_BMAP_TST((&index), 5));
            castFloat32Table[i].b[6] = BIT_TST_F32(XIL_BMAP_TST((&index), 6));
            castFloat32Table[i].b[7] = BIT_TST_F32(XIL_BMAP_TST((&index), 7));
        }
    }

    castFloat32TableMutex.unlock();
    return XIL_SUCCESS;
}
       
Xil_unsigned8**
XilDeviceManagerComputeBIT::getEdgeDetectTable()
{
    edgeDetectTableMutex.lock();
    if(edgeDetectTable == NULL) {
        edgeDetectTable = new Xil_unsigned8*[9];
        if(edgeDetectTable == NULL) {
            XIL_ERROR(NULL, XIL_ERROR_RESOURCE, "di-1", TRUE);
            return NULL;
        }

        for(int i=0; i<9; i++) {
            edgeDetectTable[i] = new Xil_unsigned8[9];
            if(edgeDetectTable[i] == NULL) {
                XIL_ERROR(NULL, XIL_ERROR_RESOURCE, "di-1", TRUE);
                return NULL;
            }
            edgeDetectTable[i] += 4;
        }

        edgeDetectTable += 4;

        for(int h= -4; h<=4; h++) {
            for(int v= -4; v<=4; v++) {
                edgeDetectTable[h][v] = _XILI_ROUND_1((float)(sqrt(v*v + h*h)/2.0F));
            }
        }
    }
    edgeDetectTableMutex.unlock();

    return edgeDetectTable;
}
