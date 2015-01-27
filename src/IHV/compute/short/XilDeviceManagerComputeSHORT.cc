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
//  File:	XilDeviceManagerComputeSHORT.cc
//  Project:	XIL
//  Revision:	1.63
//  Last Mod:	10:11:29, 03/10/00
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
//	COPYRIGHT
//------------------------------------------------------------------------
#pragma ident	"@(#)XilDeviceManagerComputeSHORT.cc	1.63\t00/03/10  "

#include "XilDeviceManagerComputeSHORT.hh"
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

    XilDeviceManagerComputeSHORT* device;

    device = new XilDeviceManagerComputeSHORT;
    
    if(device == NULL) {
        XIL_ERROR(NULL, XIL_ERROR_RESOURCE, "di-1", TRUE);
        return NULL;
    }

    return device;
}

XilDeviceManagerComputeSHORT::XilDeviceManagerComputeSHORT()
{
    //
    //  Initialize Persistent Data Members
    //
#ifdef _XIL_USE_TABLE_FLT_CNV
    shortToFloatArray  = NULL;
#endif

    int i;
    for(i = 0; i < _XILI_NUM_RESCALE_TABLES; i++) {
        rescaleCache[i]   = NULL;
        rescaleRefCnts[i] = 0;
    }
}

XilDeviceManagerComputeSHORT::~XilDeviceManagerComputeSHORT()
{
    //
    //  Delete only if it's been set.  Otherwise, we get a negative (very
    //  high) address given to delete which is invalid.
    //
#ifdef _XIL_USE_TABLE_FLT_CNV
    if(shortToFloatArray != NULL) {
        delete [] (shortToFloatArray - 32768);
    }
#endif

    int i;
    for(i = 0; i < _XILI_NUM_RESCALE_TABLES; i++) {
        if(rescaleCache[i] != NULL) {
            delete [] (rescaleCache[i] - 32768);
        }
    }
}

const char*
XilDeviceManagerComputeSHORT::getDeviceName()
{
    return "SunSoft_XIL_SHORT";
}

XilStatus
XilDeviceManagerComputeSHORT::describeMembers()
{
    XilFunctionInfo*  func_info;

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "absolute;16");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeSHORT::Absolute);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "add;16");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeSHORT::Add);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "add_const;16");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeSHORT::AddConst);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "affine_nearest;16");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeSHORT::AffineNearest);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "affine_bilinear;16");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeSHORT::AffineBilinear);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "affine_bicubic;16");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeSHORT::AffineBicubic);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "affine_general;16");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeSHORT::AffineGeneral);
    addFunction(func_info);
    func_info->destroy();
    
    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "and;16");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeSHORT::And);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "and_const;16");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeSHORT::AndConst);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "band_combine;16");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeSHORT::BandCombine);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "black_generation;16");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeSHORT::BlackGeneration);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "blend;16,16,1->16");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeSHORT::Blenda1);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "blend;16,16,8->16");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeSHORT::Blenda8);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "blend;16,16,16->16");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeSHORT::Blenda16);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "blend;16,16,f32->16");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeSHORT::Blendaf32);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "cast;16->1");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeSHORT::CastTo1);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "cast;16->8");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeSHORT::CastTo8);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "cast;16->f32");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeSHORT::CastTof32);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "convolve;16");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeSHORT::Convolve);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "separable_convolve;16");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeSHORT::Convolve_Separable);
    func_info->setPreprocessFunction((XilComputePreprocessFunctionPtr)
                           &XilDeviceManagerComputeSHORT::Convolve_SeparablePreprocess);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "color_convert;16");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeSHORT::ColorConvert);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "copy;16");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeSHORT::Copy);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "copy_pattern;16");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeSHORT::Copy);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "copy_with_planemask;16");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeSHORT::CopyWithPlanemask);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "dilate;16");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeSHORT::Dilate);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "divide;16");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeSHORT::Divide);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "divide_by_const;16");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeSHORT::DivideByConst);
    func_info->setPreprocessFunction((XilComputePreprocessFunctionPtr)
                           &XilDeviceManagerComputeSHORT::DivideByConstPreprocess);
    func_info->setPostprocessFunction((XilComputePostprocessFunctionPtr)
                           &XilDeviceManagerComputeSHORT::DivideByConstPostprocess);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "divide_into_const;16");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeSHORT::DivideIntoConst);
#ifndef _XIL_USE_INTMULDIV
    func_info->setPreprocessFunction((XilComputePreprocessFunctionPtr)
                           &XilDeviceManagerComputeSHORT::DivideIntoConstPreprocess);
    func_info->setPostprocessFunction((XilComputePostprocessFunctionPtr)
                           &XilDeviceManagerComputeSHORT::DivideIntoConstPostprocess);
#endif //_XIL_USE_INTMULDIV
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "edge_detection;16");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeSHORT::EdgeDetection);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "erode;16");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeSHORT::Erode);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "error_diffusion;16->1");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeSHORT::ErrorDiffusion1);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "error_diffusion;16->8");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeSHORT::ErrorDiffusion8);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "error_diffusion;16->16");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeSHORT::ErrorDiffusion16);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "extrema;16");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeSHORT::Extrema);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "fill;16");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeSHORT::Fill);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "histogram;16");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeSHORT::Histogram);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "lookup;16->1");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeSHORT::Lookup1);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "lookup;16->8");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeSHORT::Lookup8);
    func_info->setPreprocessFunction((XilComputePreprocessFunctionPtr)
                           &XilDeviceManagerComputeSHORT::Lookup8Preprocess);
    func_info->setPostprocessFunction((XilComputePostprocessFunctionPtr)
                           &XilDeviceManagerComputeSHORT::Lookup8Postprocess);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "lookup;16->16");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeSHORT::Lookup16);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "lookup;16->f32");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeSHORT::Lookupf32);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "lookup_n_to_n;16->1");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeSHORT::LookupCombined1);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "lookup_n_to_n;16->8");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeSHORT::LookupCombined8);
    func_info->setPreprocessFunction((XilComputePreprocessFunctionPtr)
                    &XilDeviceManagerComputeSHORT::LookupCombined8Preprocess);
    func_info->setPostprocessFunction((XilComputePostprocessFunctionPtr)
                    &XilDeviceManagerComputeSHORT::LookupCombined8Postprocess);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "lookup_n_to_n;16->16");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeSHORT::LookupCombined16);
    func_info->setPreprocessFunction((XilComputePreprocessFunctionPtr)
                    &XilDeviceManagerComputeSHORT::LookupCombined16Preprocess);
    func_info->setPostprocessFunction((XilComputePostprocessFunctionPtr)
                    &XilDeviceManagerComputeSHORT::LookupCombined16Postprocess);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "lookup_n_to_n;16->f32");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeSHORT::LookupCombinedf32);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "max;16");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeSHORT::Max);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "min;16");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeSHORT::Min);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "multiply;16");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeSHORT::Multiply);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "multiply_const;16");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeSHORT::MultiplyConst);
    func_info->setPreprocessFunction((XilComputePreprocessFunctionPtr)
                    &XilDeviceManagerComputeSHORT::MultiplyConstPreprocess);
    func_info->setPostprocessFunction((XilComputePostprocessFunctionPtr)
                    &XilDeviceManagerComputeSHORT::MultiplyConstPostprocess);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "nearest_color;16->1");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeSHORT::NearestColor1);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "nearest_color;16->8");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeSHORT::NearestColor8);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "nearest_color;16->16");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeSHORT::NearestColor16);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "not;16");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeSHORT::Not);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "or;16");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeSHORT::Or);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "or_const;16");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeSHORT::OrConst);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "ordered_dither;16->1");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeSHORT::OrderedDither1);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "ordered_dither;16->8");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeSHORT::OrderedDither8);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "ordered_dither;16->16");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeSHORT::OrderedDither16);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "paint;16");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeSHORT::Paint);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "rescale;16");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeSHORT::Rescale);
    func_info->setPreprocessFunction((XilComputePreprocessFunctionPtr)
                           &XilDeviceManagerComputeSHORT::RescalePreprocess);
    func_info->setPostprocessFunction((XilComputePostprocessFunctionPtr)
                           &XilDeviceManagerComputeSHORT::RescalePostprocess);
    addFunction(func_info);
    func_info->destroy();


    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "rotate_nearest;16");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeSHORT::RotateNearest);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "rotate_bilinear;16");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeSHORT::RotateBilinear);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "rotate_bicubic;16");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeSHORT::RotateBicubic);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "rotate_general;16");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeSHORT::RotateGeneral);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "scale_nearest;16");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeSHORT::ScaleNearest);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "scale_bilinear;16");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeSHORT::ScaleBilinear);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "scale_bicubic;16");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeSHORT::ScaleBicubic);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "scale_general;16");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeSHORT::ScaleGeneral);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "set_value;16");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeSHORT::SetValue);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "soft_fill;16");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeSHORT::SoftFill);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "squeeze_range;16");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeSHORT::SqueezeRange);
    addFunction(func_info);
    func_info->destroy();

    // Subsample Adaptive
    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "subsample_adaptive;16");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeSHORT::SubsampleAdaptive);
    addFunction(func_info);
    func_info->destroy();


    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "subtract;16");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeSHORT::Subtract);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "subtract_const;16");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeSHORT::SubtractConst);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "subtract_from_const;16");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeSHORT::SubtractFromConst);
    addFunction(func_info);
    func_info->destroy();

    // Tablewarp, geometric
    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "tablewarp_nearest;16,16->16");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeSHORT::TablewarpNearest);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "tablewarp_nearest;16,f32->16");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeSHORT::TablewarpNearest);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "tablewarp_bilinear;16,16->16");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeSHORT::TablewarpBilinear);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "tablewarp_bilinear;16,f32->16");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeSHORT::TablewarpBilinear);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "tablewarp_bicubic;16,16->16");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeSHORT::TablewarpBicubic);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "tablewarp_bicubic;16,f32->16");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeSHORT::TablewarpBicubic);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "tablewarp_general;16,16->16");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeSHORT::TablewarpGeneral);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "tablewarp_general;16,f32->16");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeSHORT::TablewarpGeneral);
    addFunction(func_info);
    func_info->destroy();

    // Tablewarp horizontal, geometric
    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "tablewarp_horizontal_nearest;16,16->16");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeSHORT::TablewarpHorizontalNearest);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "tablewarp_horizontal_nearest;16,f32->16");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeSHORT::TablewarpHorizontalNearest);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "tablewarp_horizontal_bilinear;16,16->16");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeSHORT::TablewarpHorizontalBilinear);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "tablewarp_horizontal_bilinear;16,f32->16");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeSHORT::TablewarpHorizontalBilinear);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "tablewarp_horizontal_bicubic;16,16->16");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeSHORT::TablewarpHorizontalBicubic);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "tablewarp_horizontal_bicubic;16,f32->16");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeSHORT::TablewarpHorizontalBicubic);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "tablewarp_horizontal_general;16,16->16");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeSHORT::TablewarpHorizontalGeneral);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "tablewarp_horizontal_general;16,f32->16");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeSHORT::TablewarpHorizontalGeneral);
    addFunction(func_info);
    func_info->destroy();

    // Tablewarp vertical, geometric
    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "tablewarp_vertical_nearest;16,16->16");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeSHORT::TablewarpVerticalNearest);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "tablewarp_vertical_nearest;16,f32->16");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeSHORT::TablewarpVerticalNearest);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "tablewarp_vertical_bilinear;16,16->16");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeSHORT::TablewarpVerticalBilinear);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "tablewarp_vertical_bilinear;16,f32->16");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeSHORT::TablewarpVerticalBilinear);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "tablewarp_vertical_bicubic;16,16->16");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeSHORT::TablewarpVerticalBicubic);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "tablewarp_vertical_bicubic;16,f32->16");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeSHORT::TablewarpVerticalBicubic);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "tablewarp_vertical_general;16,16->16");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeSHORT::TablewarpVerticalGeneral);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "tablewarp_vertical_general;16,f32->16");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeSHORT::TablewarpVerticalGeneral);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "threshold;16");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeSHORT::Threshold);
    addFunction(func_info);
    func_info->destroy();

    //
    //  The Threshold_1BAND function must be listed after the generic
    //  Threshold so it will be called first.
    //
    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "threshold;16");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeSHORT::Threshold_1BAND);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "threshold;16");
    func_info->describeOp(XIL_STEP, 1, "threshold;16");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeSHORT::ThresholdThreshold,
                           "threshold;16(threshold;16())");
    addFunction(func_info);
    func_info->destroy();

    //
    //  Translate...
    //
    //  Nearest neighbor is the same as a copy.  The translate op takes care
    //  of the offset in determining the source and destination boxes.
    //
    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "translate_nearest;16");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeSHORT::Copy);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "translate_bilinear;16");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeSHORT::TranslateBilinear);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "translate_bicubic;16");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeSHORT::TranslateBicubic);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "translate_general;16");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeSHORT::TranslateGeneral);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "transpose;16");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeSHORT::Transpose);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "unsharp;16");
    func_info->setFunction((XilComputeFunctionPtr) &XilDeviceManagerComputeSHORT::Unsharp);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "unsharp_ic;16");
    func_info->setFunction((XilComputeFunctionPtr) &XilDeviceManagerComputeSHORT::UnsharpIC);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "xor;16");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeSHORT::Xor);
    addFunction(func_info);
    func_info->destroy();
    
    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "xor_const;16");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeSHORT::XorConst);
    addFunction(func_info);
    func_info->destroy();

    return XIL_SUCCESS;
}

#ifdef _XIL_USE_TABLE_FLT_CNV
Xil_float32*
XilDeviceManagerComputeSHORT::getShortToFloat(XilSystemState* state)
{
    shortToFloatArrayMutex.lock();
    if(shortToFloatArray == NULL) {
        shortToFloatArray = new Xil_float32[65536];

        if(shortToFloatArray == NULL) {
            XIL_ERROR(state, XIL_ERROR_RESOURCE, "di-1", TRUE);
            shortToFloatArrayMutex.unlock();
            return NULL;
        }

        shortToFloatArray += 32768;

        int i;
        Xil_float32 f;
        for(i=-32768, f=-32768; i<0; i++, f++) {
            shortToFloatArray[i] = f;
        }
        for(i=32767,  f=32767;  i>=0; i--, f--) {
            shortToFloatArray[i] = f;
        }
    }
    shortToFloatArrayMutex.unlock();

    return shortToFloatArray;
}
#endif

int*
XilDeviceManagerComputeSHORT::getRescaleTables(XilSystemState* state,
                                               float*          mult_const,
                                               float*          add_const,
                                               unsigned int    nbands)
{
    if(nbands > _XILI_NUM_RESCALE_TABLES) {
        return NULL;
    }

    //
    //  The table information for each band.
    //
    int tables[_XILI_NUM_RESCALE_TABLES];
    for(int t=0; t<nbands; t++) {
        tables[t] = -1;
    }
    
    //
    //  Check each of our caches to see if one of them contains the same
    //  inforamtion as our operation.
    //
    rescaleCacheMutex.lock();

    for(int b=0; b<nbands; b++) {
        int         empty_table = -1;
        Xil_boolean table_match = FALSE;

        for(int i=0; i<_XILI_NUM_RESCALE_TABLES; i++) {
            //
            //  Is there data in this table?  If not, consider it empty and keep
            //  looking...  
            //
            if(rescaleCache[i] == NULL) {
                if(empty_table == -1 || rescaleCache[empty_table] != NULL) {
                    empty_table = i;
                }

                continue;
            }

            //
            //  Assume this is a table we cannot use...
            //
            table_match = FALSE;
            if(mult_const == NULL) {
                if((rescaleCacheInfo[i].multConst == 1.0) &&
                   (rescaleCacheInfo[i].addConst  == add_const[b])) {
                    table_match = TRUE;
                }
            } else if(add_const == NULL) {
                if((rescaleCacheInfo[i].multConst == mult_const[b]) &&
                   (rescaleCacheInfo[i].addConst  == 0.0)) {
                    table_match = TRUE;
                }
            } else {
                if((rescaleCacheInfo[i].multConst == mult_const[b]) &&
                   (rescaleCacheInfo[i].addConst  == add_const[b])) {
                    table_match = TRUE;
                }
            }

            //
            //  Any luck?
            //
            if(table_match == TRUE) {
                //
                //  Ok, we've found a match for this band.
                //
                rescaleRefCnts[i]++;
                tables[b] = i;
                break;
            }

            //
            //  Is nobody else using it (i.e. can we replace it later?)
            //
            if(rescaleRefCnts[i] == 0 && empty_table == -1) {
                empty_table = i;
            }
        }

        //
        //  Did we find an available table for this band at all?
        //
        if(empty_table == -1 && table_match == FALSE) {
            for(t=0; t<nbands; t++) {
                if(tables[t] != -1) {
                    rescaleRefCnts[tables[t]]--;
                }
            }

            rescaleCacheMutex.unlock();
            return NULL;
        } else if(table_match == TRUE) {
            //
            //  Found one -- move onto the next band.
            //
            continue;
        }

        //
        //  Fill in the table for this band...
        //
        int et = empty_table;

        if(rescaleCache[et] == NULL) {
            rescaleCache[et] = new Xil_signed16[65536];

            if(rescaleCache[et] == NULL) {
                for(t=0; t<nbands; t++) {
                    if(tables[t] != -1) {
                        rescaleRefCnts[tables[t]]--;
                    }
                }

                XIL_ERROR(state, XIL_ERROR_RESOURCE, "di-1", TRUE);
                rescaleCacheMutex.unlock();
                return NULL;
            }

            rescaleCache[et] += 32768;
        }

        Xil_signed16* cdata = rescaleCache[et];
        if(mult_const == NULL) {
            for(int j=-32768; j<32767; j++) {
                cdata[j] = _XILI_ROUND_S16(((float)j) + add_const[b]);
            }

            rescaleCacheInfo[et].multConst = 1.0;
            rescaleCacheInfo[et].addConst  = add_const[b];
        } else if(add_const == NULL) {
            for(int j=-32768; j<32767; j++) {
                cdata[j] = _XILI_ROUND_S16(((float)j) * mult_const[b]);
            }

            rescaleCacheInfo[et].multConst = mult_const[b];
            rescaleCacheInfo[et].addConst  = 0.0;
        } else {
            for(int j=-32768; j<32767; j++) {
                cdata[j] = _XILI_ROUND_S16((((float)j) * mult_const[b]) + add_const[b]);
            }

            rescaleCacheInfo[et].multConst = mult_const[b];
            rescaleCacheInfo[et].addConst  = add_const[b];
        }

        //
        //  Bump the reference count for the table, indicate we found a table
        //  for this band and then move onto the next band.
        //
        rescaleRefCnts[et]++;
        tables[b] = et;
    }

    //
    //  Now that we know we've found all of the tables we'll need, create a
    //  new array of length nbands storing all of the offsets and return it.
    //
    int* table_info = new int[nbands];

    if(table_info == NULL) {
        for(t=0; t<nbands; t++) {
            if(tables[t] != -1) {
                rescaleRefCnts[tables[t]]--;
            }
        }

        XIL_ERROR(state, XIL_ERROR_RESOURCE, "di-1", TRUE);
        rescaleCacheMutex.unlock();
        return NULL;
    }

    xili_memcpy(table_info, tables, nbands*sizeof(int));

    rescaleCacheMutex.unlock();

    return table_info;
}

void
XilDeviceManagerComputeSHORT::releaseRescaleTables(int*         tables,
                                                   unsigned int nbands)
{
    if(tables == NULL) {
        return;
    }

    rescaleCacheMutex.lock();

    for(int i=0; i<nbands; i++) {
        rescaleRefCnts[tables[i]]--;
    }

    rescaleCacheMutex.unlock();

    delete [] tables;
}
