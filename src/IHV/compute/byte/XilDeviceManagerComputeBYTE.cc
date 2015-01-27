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
//  File:	XilDeviceManagerComputeBYTE.cc
//  Project:	XIL
//  Revision:	1.98
//  Last Mod:	10:10:11, 03/10/00
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
#pragma ident	"@(#)XilDeviceManagerComputeBYTE.cc	1.98\t00/03/10  "

#include "XilDeviceManagerComputeBYTE.hh"
#include "XiliNearestIndexSelector.hh"
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

    XilDeviceManagerComputeBYTE* device;

    device = new XilDeviceManagerComputeBYTE;
    
    if(device == NULL) {
        XIL_ERROR(NULL, XIL_ERROR_RESOURCE, "di-1", TRUE);
        return NULL;
    }

    return device;
}

XilDeviceManagerComputeBYTE::XilDeviceManagerComputeBYTE()
    :
#ifdef BLEND_PRECISION
    _XIL_BLEND_FRAC_BITS(8),
#else
    _XIL_BLEND_FRAC_BITS(0),
#endif
    _XIL_BLEND_MULTIPLIER((float) (1 << _XIL_BLEND_FRAC_BITS)),
    _XIL_BLEND_SCALED_HALF(1 << (_XIL_BLEND_FRAC_BITS - 1)),
    _XIL_BLEND_NORM_FACTOR((float) _XIL_BLEND_MULTIPLIER / 255.0F)
{
    int i;
    
    //
    //  Initialize Persistent Data Members
    //
    addClampArray      = NULL;
    subtractClampArray = NULL;
    blendTable         = NULL;
    squaresTable       = NULL;
    squaresOver4Table  = NULL;

    photoYArray   = NULL;
    photoYCbArray = NULL;
    photoYCrArray = NULL;
    photoCbArray  = NULL;
    photoCrArray  = NULL;

    nearestIndexSelector = NULL;
    nearestIndexSelectorOwner = NULL;

    for(i = 0; i < _XILI_NUM_PAINT_BRUSHES; i++) {
        pcacheBrush[i]   = NULL;
        pcacheRefCnts[i] = 0;
    }

    for(i = 0; i < _XILI_NUM_ERROR_DIFFUSION_LUTS; i++) {
        edcacheTable[i]   = NULL;
        edcacheRefCnts[i] = 0;
    }

    for(i = 0; i < _XILI_NUM_ORDERED_DITHER_LUTS; i++) {
        odcacheDitherLut[i] = NULL;
        odcacheRefCnts[i] = 0;
    }

    for(i = 0; i < _XILI_NUM_RESCALE_TABLES; i++) {
        rescaleCache[i]   = NULL;
        rescaleRefCnts[i] = 0;
    }
}

XilDeviceManagerComputeBYTE::~XilDeviceManagerComputeBYTE()
{
    int i;
    
    delete [] addClampArray;

    //
    //  Delete the subtract clamp array only if it's been set because we're
    //  going to subtract the 256.  Otherwise, it would put the address to be
    //  deleted at (unsigned)-256.
    //
    if(subtractClampArray != NULL) {
        delete [] (subtractClampArray - 256);
    }

    delete [] blendTable;

    delete [] photoYArray;
    delete [] photoYCbArray;
    delete [] photoYCrArray;
    delete [] photoCbArray;
    delete [] photoCrArray;

    delete nearestIndexSelector;

    delete [] squaresTable;
    delete [] squaresOver4Table;

    for(i = 0; i < _XILI_NUM_PAINT_BRUSHES; i++) {
        delete [] pcacheBrush[i];
    }

    for(i = 0; i < _XILI_NUM_ERROR_DIFFUSION_LUTS; i++) {
        delete edcacheTable[i];
    }

    for(i = 0; i < _XILI_NUM_ORDERED_DITHER_LUTS; i++) {
        delete odcacheDitherLut[i];
    }

    for(i = 0; i < _XILI_NUM_RESCALE_TABLES; i++) {
        delete [] rescaleCache[i];
    }
}

const char*
XilDeviceManagerComputeBYTE::getDeviceName()
{
    return "SunSoft_XIL_BYTE";
}

XilStatus
XilDeviceManagerComputeBYTE::describeMembers()
{
    XilFunctionInfo*  func_info;

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "add;8");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeBYTE::Add);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "add_const;8");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeBYTE::AddConst);
    func_info->setPreprocessFunction((XilComputePreprocessFunctionPtr)
                           &XilDeviceManagerComputeBYTE::AddConstPreprocess);
    func_info->setPostprocessFunction((XilComputePostprocessFunctionPtr)
                           &XilDeviceManagerComputeBYTE::AddConstPostprocess);
    addFunction(func_info);
    func_info->destroy();

    // TODO: 10/9/95 dtb
    // all of the geometrics point to the same function for testing
    // purposes, when the real versions are added we may want to call
    // different methods.

    // Affine
    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "affine_nearest;8");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeBYTE::AffineNearest);
    addFunction(func_info);
    func_info->destroy();
    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "affine_bilinear;8");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeBYTE::AffineBilinear);
    addFunction(func_info);
    func_info->destroy();
    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "affine_bicubic;8");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeBYTE::AffineBicubic);
    addFunction(func_info);
    func_info->destroy();
    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "affine_general;8");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeBYTE::AffineGeneral);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "and;8");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeBYTE::And);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "and_const;8");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeBYTE::AndConst);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "black_generation;8");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeBYTE::BlackGeneration);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "blend;8,8,1->8");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeBYTE::Blenda1);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "blend;8,8,8->8");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeBYTE::Blenda8);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "blend;8,8,16->8");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeBYTE::Blenda16);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "blend;8,8,f32->8");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeBYTE::Blendaf32);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "cast;8->1");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeBYTE::CastTo1);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "cast;8->16");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeBYTE::CastTo16);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "cast;8->f32");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeBYTE::CastTof32);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "choose_colormap;8");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeBYTE::ChooseColormap);
    func_info->setPreprocessFunction((XilComputePreprocessFunctionPtr)
                           &XilDeviceManagerComputeBYTE::ChooseColormapPreprocess);
    func_info->setPostprocessFunction((XilComputePostprocessFunctionPtr)
                           &XilDeviceManagerComputeBYTE::ChooseColormapPostprocess);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "color_convert;8");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeBYTE::ColorConvert);
    addFunction(func_info);
    func_info->destroy();
    
    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "convolve;8");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeBYTE::Convolve);
    addFunction(func_info);
    func_info->destroy();
    
    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "separable_convolve;8");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeBYTE::Convolve_Separable);
    func_info->setPreprocessFunction((XilComputePreprocessFunctionPtr)
                           &XilDeviceManagerComputeBYTE::Convolve_SeparablePreprocess);
    addFunction(func_info);
    func_info->destroy();
    
    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "copy;8");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeBYTE::Copy);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "copy_pattern;8");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeBYTE::Copy);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "copy_with_planemask;8");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeBYTE::CopyWithPlanemask);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "dilate;8");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeBYTE::Dilate);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "divide;8");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeBYTE::Divide);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "divide_by_const;8");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeBYTE::DivideByConst);
    func_info->setPreprocessFunction((XilComputePreprocessFunctionPtr)
                           &XilDeviceManagerComputeBYTE::DivideByConstPreprocess);
    func_info->setPostprocessFunction((XilComputePostprocessFunctionPtr)
                           &XilDeviceManagerComputeBYTE::DivideByConstPostprocess);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "divide_into_const;8");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeBYTE::DivideIntoConst);
#ifndef _XIL_USE_INTMULDIV
    func_info->setPreprocessFunction((XilComputePreprocessFunctionPtr)
                           &XilDeviceManagerComputeBYTE::DivideIntoConstPreprocess);
    func_info->setPostprocessFunction((XilComputePostprocessFunctionPtr)
                           &XilDeviceManagerComputeBYTE::DivideIntoConstPostprocess);
#endif // _XIL_USE_INTMULDIV
    addFunction(func_info);
    func_info->destroy();
    
    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "edge_detection;8");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeBYTE::EdgeDetection);
    addFunction(func_info);
    func_info->destroy();
    
    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "erode;8");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeBYTE::Erode);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "error_diffusion;8->1");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeBYTE::ErrorDiffusion1);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "error_diffusion;8->8");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeBYTE::ErrorDiffusion8);
    func_info->setPreprocessFunction((XilComputePreprocessFunctionPtr)
                           &XilDeviceManagerComputeBYTE::ErrorDiffusion8Preprocess);
    func_info->setPostprocessFunction((XilComputePostprocessFunctionPtr)
                           &XilDeviceManagerComputeBYTE::ErrorDiffusion8Postprocess);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "error_diffusion;8->16");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeBYTE::ErrorDiffusion16);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "extrema;8");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeBYTE::Extrema);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "histogram;8");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeBYTE::Histogram);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "lookup;8->1");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeBYTE::Lookup1);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "lookup;8->8");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeBYTE::Lookup8);
    func_info->setPreprocessFunction((XilComputePreprocessFunctionPtr)
                           &XilDeviceManagerComputeBYTE::Lookup8Preprocess);
    func_info->setPostprocessFunction((XilComputePostprocessFunctionPtr)
                           &XilDeviceManagerComputeBYTE::Lookup8Postprocess);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "lookup;8->16");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeBYTE::Lookup16);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "lookup;8->f32");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeBYTE::Lookupf32);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "lookup_n_to_n;8->1");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeBYTE::LookupCombined1);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "lookup_n_to_n;8->8");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeBYTE::LookupCombined8);
    func_info->setPreprocessFunction((XilComputePreprocessFunctionPtr)
                    &XilDeviceManagerComputeBYTE::LookupCombined8Preprocess);
    func_info->setPostprocessFunction((XilComputePostprocessFunctionPtr)
                    &XilDeviceManagerComputeBYTE::LookupCombined8Postprocess);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "lookup_n_to_n;8->16");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeBYTE::LookupCombined16);
    func_info->setPreprocessFunction((XilComputePreprocessFunctionPtr)
                    &XilDeviceManagerComputeBYTE::LookupCombined16Preprocess);
    func_info->setPostprocessFunction((XilComputePostprocessFunctionPtr)
                    &XilDeviceManagerComputeBYTE::LookupCombined16Postprocess);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "lookup_n_to_n;8->f32");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeBYTE::LookupCombinedf32);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "max;8");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeBYTE::Max);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "min;8");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeBYTE::Min);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "multiply;8");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeBYTE::Multiply);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "multiply_const;8");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeBYTE::MultiplyConst);
    func_info->setPreprocessFunction((XilComputePreprocessFunctionPtr)
                           &XilDeviceManagerComputeBYTE::MultiplyConstPreprocess);
    func_info->setPostprocessFunction((XilComputePostprocessFunctionPtr)
                           &XilDeviceManagerComputeBYTE::MultiplyConstPostprocess);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "nearest_color;8->1");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeBYTE::NearestColor1);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "nearest_color;8->8");
    func_info->setPreprocessFunction((XilComputePreprocessFunctionPtr)
                    &XilDeviceManagerComputeBYTE::NearestColor8Preprocess);
    func_info->setPostprocessFunction((XilComputePostprocessFunctionPtr)
                    &XilDeviceManagerComputeBYTE::NearestColor8Postprocess);
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeBYTE::NearestColor8);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "nearest_color;8->16");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeBYTE::NearestColor16);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "not;8");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeBYTE::Not);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "or;8");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeBYTE::Or);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "or_const;8");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeBYTE::OrConst);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "ordered_dither;8->1");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeBYTE::OrderedDither1);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "ordered_dither;8->8");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeBYTE::OrderedDither8);
    func_info->setPreprocessFunction((XilComputePreprocessFunctionPtr)
                           &XilDeviceManagerComputeBYTE::OrderedDither8Preprocess);
    func_info->setPostprocessFunction((XilComputePostprocessFunctionPtr)
                           &XilDeviceManagerComputeBYTE::OrderedDither8Postprocess);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "ordered_dither;8->16");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeBYTE::OrderedDither16);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "paint;8");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeBYTE::Paint);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "paint;8");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeBYTE::Paint3Band);
    func_info->setPreprocessFunction((XilComputePreprocessFunctionPtr)
                           &XilDeviceManagerComputeBYTE::Paint3BandPreprocess);
    func_info->setPostprocessFunction((XilComputePostprocessFunctionPtr)
                           &XilDeviceManagerComputeBYTE::Paint3BandPostprocess);
    addFunction(func_info);
    func_info->destroy();


    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "rescale;8");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeBYTE::Rescale);
    func_info->setPreprocessFunction((XilComputePreprocessFunctionPtr)
                           &XilDeviceManagerComputeBYTE::RescalePreprocess);
    func_info->setPostprocessFunction((XilComputePostprocessFunctionPtr)
                           &XilDeviceManagerComputeBYTE::RescalePostprocess);
    addFunction(func_info);
    func_info->destroy();

    // Rotate, geometric
    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "rotate_nearest;8");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeBYTE::RotateNearest);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "rotate_bilinear;8");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeBYTE::RotateBilinear);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "rotate_bicubic;8");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeBYTE::RotateBicubic);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "rotate_general;8");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeBYTE::RotateGeneral);
    addFunction(func_info);
    func_info->destroy();
    
    // Scale, geometric
    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "scale_nearest;8");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeBYTE::ScaleNearest);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "scale_bilinear;8");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeBYTE::ScaleBilinear);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "scale_bicubic;8");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeBYTE::ScaleBicubic);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "scale_general;8");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeBYTE::ScaleGeneral);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "set_value;8");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeBYTE::SetValue);
    func_info->setPreprocessFunction((XilComputePreprocessFunctionPtr)
                                     &XilDeviceManagerComputeBYTE::SetValuePreprocess);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "squeeze_range;8");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeBYTE::SqueezeRange);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "subtract;8");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeBYTE::Subtract);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "subtract_from_const;8");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeBYTE::SubtractFromConst);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "subtract_const;8");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeBYTE::SubtractConst);
    func_info->setPreprocessFunction((XilComputePreprocessFunctionPtr)
                           &XilDeviceManagerComputeBYTE::SubtractConstPreprocess);
    func_info->setPostprocessFunction((XilComputePostprocessFunctionPtr)
                           &XilDeviceManagerComputeBYTE::SubtractConstPostprocess);
    addFunction(func_info);
    func_info->destroy();

    // Subsample Adaptive
    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "subsample_adaptive;8");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeBYTE::SubsampleAdaptive);
    addFunction(func_info);
    func_info->destroy();

    // Subsample Binary to Gray
    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "subsample_binary_to_gray;8");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeBYTE::SubsampleBinaryToGray);
    addFunction(func_info);
    func_info->destroy();

    // Tablewarp, geometric
    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "tablewarp_nearest;8,16->8");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeBYTE::TablewarpNearest);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "tablewarp_nearest;8,f32->8");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeBYTE::TablewarpNearest);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "tablewarp_bilinear;8,16->8");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeBYTE::TablewarpBilinear);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "tablewarp_bilinear;8,f32->8");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeBYTE::TablewarpBilinear);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "tablewarp_bicubic;8,16->8");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeBYTE::TablewarpBicubic);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "tablewarp_bicubic;8,f32->8");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeBYTE::TablewarpBicubic);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "tablewarp_general;8,16->8");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeBYTE::TablewarpGeneral);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "tablewarp_general;8,f32->8");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeBYTE::TablewarpGeneral);
    addFunction(func_info);
    func_info->destroy();

    // Tablewarp horizontal, geometric
    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "tablewarp_horizontal_nearest;8,16->8");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeBYTE::TablewarpHorizontalNearest);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "tablewarp_horizontal_nearest;8,f32->8");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeBYTE::TablewarpHorizontalNearest);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "tablewarp_horizontal_bilinear;8,16->8");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeBYTE::TablewarpHorizontalBilinear);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "tablewarp_horizontal_bilinear;8,f32->8");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeBYTE::TablewarpHorizontalBilinear);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "tablewarp_horizontal_bicubic;8,16->8");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeBYTE::TablewarpHorizontalBicubic);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "tablewarp_horizontal_bicubic;8,f32->8");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeBYTE::TablewarpHorizontalBicubic);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "tablewarp_horizontal_general;8,16->8");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeBYTE::TablewarpHorizontalGeneral);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "tablewarp_horizontal_general;8,f32->8");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeBYTE::TablewarpHorizontalGeneral);
    addFunction(func_info);
    func_info->destroy();

    // Tablewarp vertical, geometric
    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "tablewarp_vertical_nearest;8,16->8");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeBYTE::TablewarpVerticalNearest);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "tablewarp_vertical_nearest;8,f32->8");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeBYTE::TablewarpVerticalNearest);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "tablewarp_vertical_bilinear;8,16->8");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeBYTE::TablewarpVerticalBilinear);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "tablewarp_vertical_bilinear;8,f32->8");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeBYTE::TablewarpVerticalBilinear);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "tablewarp_vertical_bicubic;8,16->8");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeBYTE::TablewarpVerticalBicubic);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "tablewarp_vertical_bicubic;8,f32->8");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeBYTE::TablewarpVerticalBicubic);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "tablewarp_vertical_general;8,16->8");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeBYTE::TablewarpVerticalGeneral);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "tablewarp_vertical_general;8,f32->8");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeBYTE::TablewarpVerticalGeneral);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "threshold;8");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeBYTE::Threshold);
    addFunction(func_info);
    func_info->destroy();

    //
    //  The Threshold_1BAND function must be listed after the generic
    //  Threshold so it will be called first.
    //
    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "threshold;8");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeBYTE::Threshold_1BAND);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "threshold;8");
    func_info->describeOp(XIL_STEP, 1, "threshold;8");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeBYTE::ThresholdThreshold,
                           "threshold;8(threshold;8())");
    addFunction(func_info);
    func_info->destroy();

    //
    //  Translate...
    //
    //  Nearest neighbor is the same as a copy.  The translate op takes care
    //  of the offset in determining the source and destination boxes.
    //
    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "translate_nearest;8");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeBYTE::Copy);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "translate_bilinear;8");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeBYTE::TranslateBilinear);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "translate_bicubic;8");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeBYTE::TranslateBicubic);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "translate_general;8");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeBYTE::TranslateGeneral);
    addFunction(func_info);
    func_info->destroy();

    // Transpose, geometric
    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "transpose;8");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeBYTE::Transpose);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "xor;8");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeBYTE::Xor);
    addFunction(func_info);
    func_info->destroy();
    
    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "xor_const;8");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeBYTE::XorConst);
    addFunction(func_info);
    func_info->destroy();

    // BandCombine
    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "band_combine;8");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeBYTE::BandCombine);
    addFunction(func_info);
    func_info->destroy();

    // Fill and SoftFill
    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "fill;8");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeBYTE::Fill);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "soft_fill;8");
    func_info->setFunction((XilComputeFunctionPtr)
                           &XilDeviceManagerComputeBYTE::SoftFill);
    addFunction(func_info);
    func_info->destroy();
    
#if 0
    //
    //  Temporary multi-branch molecules for testing the XIL Core code.
    //
    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_PUSH, 1, "add;8");
    func_info->describeOp(XIL_STEP, 1, "copy;8");
    func_info->describeOp(XIL_POP, TRUE);
    func_info->describeOp(XIL_STEP, 2, "copy;8");
    func_info->setFunction((XilComputeFunctionPtr)
                           XilDeviceManagerComputeBYTE::AddCopyCopy);
    addFunction(func_info);
    func_info->destroy();
#endif
    
    return XIL_SUCCESS;
}

Xil_unsigned8*
XilDeviceManagerComputeBYTE::getAddClampArray(XilSystemState* state)
{
    addClampArrayMutex.lock();
    if(addClampArray == NULL) {
        addClampArray = new Xil_unsigned8[512];

        if(addClampArray == NULL) {
            XIL_ERROR(state, XIL_ERROR_RESOURCE, "di-1", TRUE);
            addClampArrayMutex.unlock();
            return NULL;
        }
            
        int i;
        for(i=0; i<256; i++) {
            addClampArray[i] = i;
        }
        for(i=256; i<512; i++) {
            addClampArray[i] = 255;
        }
    }
    addClampArrayMutex.unlock();

    return addClampArray;
}


Xil_unsigned8*
XilDeviceManagerComputeBYTE::getSubtractClampArray(XilSystemState* state)
{
    subtractClampArrayMutex.lock();
    if(subtractClampArray == NULL) {
        subtractClampArray = new Xil_unsigned8[768];

        if(subtractClampArray == NULL) {
            XIL_ERROR(state, XIL_ERROR_RESOURCE, "di-1", TRUE);
            subtractClampArrayMutex.unlock();
            return NULL;
        }
            
        int i;
        for(i=0; i<256; i++) {
            subtractClampArray[i] = 0;
        }
        for(i=256; i<512; i++) {
            subtractClampArray[i] = i;
        }
        for(i=512; i<768; i++) {
            subtractClampArray[i] = 255;
        }

        subtractClampArray += 256;
    }
    subtractClampArrayMutex.unlock();

    return subtractClampArray;
}

int*
XilDeviceManagerComputeBYTE::getRescaleTables(XilSystemState* state,
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
            rescaleCache[et] = new Xil_unsigned8[256];

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
        }

        Xil_unsigned8* cdata = rescaleCache[et];
        if(mult_const == NULL) {
            float tv;
            for(int j=0; j<256; j++) {
                tv       = (float)j + add_const[b];
                cdata[j] = _XILI_ROUND_U8(tv);
            }

            rescaleCacheInfo[et].multConst = 1.0;
            rescaleCacheInfo[et].addConst  = add_const[b];
        } else if(add_const == NULL) {
            float tv;
            for(int j=0; j<256; j++) {
                tv       = (mult_const[b] * (float)j);
                cdata[j] = _XILI_ROUND_U8(tv);
            }

            rescaleCacheInfo[et].multConst = mult_const[b];
            rescaleCacheInfo[et].addConst  = 0.0;
        } else {
            float tv;
            for(int j=0; j<256; j++) {
                tv       = (mult_const[b] * (float)j) + add_const[b];
                cdata[j] = _XILI_ROUND_U8(tv);
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
XilDeviceManagerComputeBYTE::releaseRescaleTables(int*         tables,
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

XilStatus
XilDeviceManagerComputeBYTE::getBlendTable(XilSystemState* state)
{
    blendTableMutex.lock();
    if(blendTable != NULL) {
        blendTableMutex.unlock();
        return XIL_SUCCESS;
    }

    blendTable = new BLEND_TYPE[256][256];
    if(blendTable == NULL) {
        XIL_ERROR(state, XIL_ERROR_RESOURCE, "di-1", TRUE);
        blendTableMutex.unlock();
        return XIL_FAILURE;
    }
            
    int alpha, data;
    float value, increment;
    BLEND_TYPE* table_ptr;
    /*
     * Do the easy ones (alpha = 0, and alpha = 255)
     */
    table_ptr = blendTable[255];
    for (data=0; data < 256; data++) {
        blendTable[0][data] = 0;
        table_ptr[data] = (data << _XIL_BLEND_FRAC_BITS);
    }

    for (alpha=1; alpha < 255; alpha++) {
        table_ptr = blendTable[alpha];
        /*
         * increment = (alpha / 255) * SHIFT_UP_TO_FIXED_PT
         *
         * value[n] = n * ((alpha / 255) * SHIFT_UP_TO_FIXED_PT)
         */
        increment = ((float)alpha) * _XIL_BLEND_NORM_FACTOR;
        value = increment;
        if (alpha > 127) {
            /*
             * Add 1/2 to values computed with an alpha > 127.
             * This will get us: src1*(1-alpha) + src2*(alpha) + 0.5
             * because alpha and 1-alpha are always on opposite sides of 127
             */
            value += 0.5F*_XIL_BLEND_MULTIPLIER;
            table_ptr[255] =
                (alpha << _XIL_BLEND_FRAC_BITS)+_XIL_BLEND_SCALED_HALF;
            table_ptr[0] = _XIL_BLEND_SCALED_HALF;
        } else {
            table_ptr[255] = alpha << _XIL_BLEND_FRAC_BITS;
            table_ptr[0] = 0;
        }
        for (data=1; data < 255; data++) {
            table_ptr[data] = (BLEND_TYPE) value;
//#define BLEND_INIT_TEST            
#ifdef BLEND_INIT_TEST
{ /* XXX Only works if BLEND_PRECISION is defined*/
float answer;
answer = ((((float)alpha)/255.0F)*((float)data)) * 256.0F;
if (alpha > 127) answer += 0.5F*256.0F;
if ((((BLEND_TYPE)answer)>>1) != (((BLEND_TYPE) value)>>1))
    printf("alpha %d, data %d, real %d, table %d\n", alpha, data,
    (BLEND_TYPE)answer, (BLEND_TYPE) value);
} /* XXX */
#endif

            value += increment;
        }
    }

    blendTableMutex.unlock();
    
    return XIL_SUCCESS;
}


Xil_unsigned32*
XilDeviceManagerComputeBYTE::getSquaresTable(XilSystemState* state)
{
    squaresTableMutex.lock();
    if(squaresTable == NULL) {
        squaresTable = new Xil_unsigned32[514];
        if(squaresTable == NULL) {
            XIL_ERROR(state, XIL_ERROR_RESOURCE, "di-1", TRUE);
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

Xil_float32*
XilDeviceManagerComputeBYTE::getSquaresOver4Table(XilSystemState* state)
{
    squaresOver4TableMutex.lock();
    if(squaresOver4Table == NULL) {
        squaresOver4Table = new Xil_float32[2050];
        if(squaresOver4Table == NULL) {
            XIL_ERROR(state, XIL_ERROR_RESOURCE, "di-1", TRUE);
            squaresOver4TableMutex.unlock();
            return NULL;
        }

        int i;
        for(i=0; i<1025; i++) {
            squaresOver4Table[i] = ((1024-i)*(1024-i))*0.25F;
        }

        for(i=0; i<1025; i++) {
            squaresOver4Table[1024+i] = (i*i)*0.25F;
        }
    }

    squaresOver4TableMutex.unlock();

    return &squaresOver4Table[1024];
}

#if 0
//
// TODO:
// This routine is not currently in use, but should be put in use by
// 2.6 FCS - oconnor 17-Jan-96
//
//
//  TODO:  2/29/96 jlf  Update routine to use new object versioning.
//
Xil_unsigned32
XilDeviceManagerComputeBYTE::getNearestColorGrid(XilSystemState*  state,
                                                 XilLookupSingle* cmap,
                                                 Xil_unsigned8*   )
{
    int j, k;
    
#define NEAREST_COLOR_DEBUG 1
    nearestColorGridMutex.lock();

    //
    // Look through the tables and try to find one that matches.
    // If we hit an empty table, go no further because there will be
    // not used tables later in the list.
    //
    int i = -1;
    int match = -1;
#ifdef NEAREST_COLOR_DEBUG
    printf("Looking for a table to reuse - stopping if I find an empty\n");
#endif        
    while (++i < _XIL_MAX_NEAREST_COLOR_GRIDS) {
        if(nearestColorGridVersion[i] == 0) {
#ifdef NEAREST_COLOR_DEBUG
            printf("Found empty slot, stoping check for matches");
#endif                
            break;
        }
        //
        //  TODO: 7/8/96 jlf  Looks like something else is expected to happen here!
        //
        if(nearestColorGridVersion[i] == 0 /*cmap->getVersionNumber()*/) {
#ifdef NEAREST_COLOR_DEBUG
            printf("Found a match at %d - Trying to read lock\n",i);
#endif                
            if(nearestColorGridReadWriteLocks[i].tryReadLock() == XIL_SUCCESS){
#ifdef NEAREST_COLOR_DEBUG
                printf("Read lock successful\n");
#endif
                match = i;
                break;
            }
        }
    }

    if (match == -1) {
        //
        // OK, so we won't be using an existing table.
        // Either fill an empty table or reuse a table.
        // Both require filling in the table's data.
        //
        if ((i < _XIL_MAX_NEAREST_COLOR_GRIDS) && (nearestColorGrid[i] == NULL)){
                //
                // Fill an empty grid
#if NEAREST_COLOR_DEBUG
                printf("Filling an empty grid\n");
#endif
#if NEAREST_COLOR_DEBUG
                printf("Getting write lock on %d\n",i);
#endif
                nearestColorGridReadWriteLocks[i].writeLock();
                
                match = i;
            } else {
                k = -1;
                while (++k < _XIL_MAX_NEAREST_COLOR_GRIDS) {
#if NEAREST_COLOR_DEBUG    
                    printf("Trying write lock on %d\n",k);
#endif    
                    if (nearestColorGridReadWriteLocks[k].tryWriteLock() == XIL_SUCCESS) {
#ifdef NEAREST_COLOR_DEBUG
                        printf("Write lock successful\n");
#endif                    
                        match = k;
                        break;
                    }
                }
            }
        
            if (match == -1) {
                //
                // Could not get a rescale table to use
                // Cleanup and fail
                //
#if NEAREST_COLOR_DEBUG    
                printf("Could not get a rescale table to use\n");
                printf("Unlocking rescale array mutex\n");
#endif    
                nearestColorGridMutex.unlock();
                return XIL_FAILURE;
            }

            if (nearestColorGrid[i] == NULL) {
                nearestColorGrid[i] = new Xil_unsigned8[256];
                if (nearestColorGrid[i] == NULL) {
                    XIL_ERROR(state, XIL_ERROR_RESOURCE, "di-1", TRUE);
                    nearestColorGridMutex.unlock();
                    return XIL_FAILURE;
                }
            }
            
                
            //
            // Fill the table
            //
#if NEAREST_COLOR_DEBUG
            printf("Filling the table\n");
#endif
            Xil_unsigned8* tmpPtr = nearestColorGrid[i];
            Xil_unsigned8* cmap_data = (Xil_unsigned8*)cmap->getData();
            int cmap_size = cmap->getNumEntries();
            int cmap_offset = cmap->getOffset();
            int closest_byte_entry;
            for(j=0; j<256; j++) {
                int dist,
                    closest_dist = INT_MAX;
                for (int byte_index = 0; byte_index < cmap_size; byte_index++){
                    // go through the whole lookup table
                    if ((dist = abs(j - (int)cmap_data[byte_index])) <
                        closest_dist) {
                        closest_dist = dist;
                        closest_byte_entry = byte_index;
                    }
                }
                tmpPtr[j] = byte_index + cmap_offset;
            }


#if NEAREST_COLOR_DEBUG
            printf("Put Grid in slot %d\n",i);
#endif
            nearestColorGridVersion[i] = 0 /*cmap->getVersionNumber()*/;
#if NEAREST_COLOR_DEBUG    
            printf("Unlock %d\n",i);
#endif    
            nearestColorGridReadWriteLocks[i].unlock();
#if NEAREST_COLOR_DEBUG    
            printf("Now put a read lock on %d\n",i);
#endif    
            nearestColorGridReadWriteLocks[i].readLock();
    }

#if NEAREST_COLOR_DEBUG    
    printf("Unlocking nearest color grids\n");
#endif    
    nearestColorGridMutex.unlock();

    return XIL_SUCCESS;
}


void
XilDeviceManagerComputeBYTE::freeNearestColorGrid(int i)
{
    nearestColorGridReadWriteLocks[i].unlock();
}
#endif

//
// Tables for ColorConvert
//
Xil_signed32*
XilDeviceManagerComputeBYTE::getPhotoYArray(XilSystemState* state)
{
    photoYArrayMutex.lock();
    if(photoYArray == NULL) {
        photoYArray = new Xil_signed32[256];

        if(photoYArray == NULL) {
            XIL_ERROR(state, XIL_ERROR_RESOURCE, "di-1", TRUE);
            photoYArrayMutex.unlock();
            return NULL;
        }
            
        int i;
        for(i=0; i<256; i++) {
            photoYArray[i] = i * 5743;
        }
    }
    photoYArrayMutex.unlock();

    return photoYArray;
}


Xil_signed32*
XilDeviceManagerComputeBYTE::getPhotoYCbArray(XilSystemState* state)
{
    photoYCbArrayMutex.lock();
    if(photoYCbArray == NULL) {
        photoYCbArray = new Xil_signed32[256];

        if(photoYCbArray == NULL) {
            XIL_ERROR(state, XIL_ERROR_RESOURCE, "di-1", TRUE);
            photoYCbArrayMutex.unlock();
            return NULL;
        }
            
        int i;
        for(i=0; i<256; i++) {
            photoYCbArray[i] = 283754 - i * 1819;
        }
    }
    photoYCbArrayMutex.unlock();

    return photoYCbArray;
}


Xil_signed32*
XilDeviceManagerComputeBYTE::getPhotoYCrArray(XilSystemState* state)
{
    photoYCrArrayMutex.lock();
    if(photoYCrArray == NULL) {
        photoYCrArray = new Xil_signed32[256];

        if(photoYCrArray == NULL) {
            XIL_ERROR(state, XIL_ERROR_RESOURCE, "di-1", TRUE);
            photoYCrArrayMutex.unlock();
            return NULL;
        }
            
        int i;
        for(i=0; i<256; i++) {
            photoYCrArray[i] = 536971 - i * 3919;
        }
    }
    photoYCrArrayMutex.unlock();

    return photoYCrArray;
}


Xil_signed32*
XilDeviceManagerComputeBYTE::getPhotoCbArray(XilSystemState* state)
{
    photoCbArrayMutex.lock();
    if(photoCbArray == NULL) {
        photoCbArray = new Xil_signed32[256];

        if(photoCbArray == NULL) {
            XIL_ERROR(state, XIL_ERROR_RESOURCE, "di-1", TRUE);
            photoCbArrayMutex.unlock();
            return NULL;
        }
            
        int i;
        for(i=0; i<256; i++) {
            photoCbArray[i] = i * 9376 - 1462647;
        }
    }
    photoCbArrayMutex.unlock();

    return photoCbArray;
}


Xil_signed32*
XilDeviceManagerComputeBYTE::getPhotoCrArray(XilSystemState* state)
{
    photoCrArrayMutex.lock();
    if(photoCrArray == NULL) {
        photoCrArray = new Xil_signed32[256];

        if(photoCrArray == NULL) {
            XIL_ERROR(state, XIL_ERROR_RESOURCE, "di-1", TRUE);
            photoCrArrayMutex.unlock();
            return NULL;
        }
            
        int i;
        for(i=0; i<256; i++) {
            photoCrArray[i] = i * 7700 - 1054953;
        }
    }
    photoCrArrayMutex.unlock();

    return photoCrArray;
}

void
XilDeviceManagerComputeBYTE::aquireNearestIndexSelector(XilSystemState*  state,
                                                        XilOp*           owner,
                                                        XilLookupSingle* lut)
{
    nearestIndexSelectorMutex.lock();

    if(nearestIndexSelectorOwner == owner ||
       nearestIndexSelectorOwner == NULL) {
        //
        //  Can aquire...
        //
        if(nearestIndexSelector == NULL) {
            nearestIndexSelector =
                new XiliNearestIndexSelector(state, lut);

            if(nearestIndexSelector == NULL) {
                nearestIndexSelectorMutex.unlock();
                return;
            }
        } else {
            nearestIndexSelector->setNewSystemState(state);

            nearestIndexSelector->newColormap(lut);
        }

        nearestIndexSelectorOwner = owner;
    } else {
        //
        //  Can't aquire...we indicate this by returning NULL which causes
        //  the routine to use the slower algorithm.
        //
        nearestIndexSelectorMutex.unlock();
        return;
    }

    nearestIndexSelectorMutex.unlock();
}

void
XilDeviceManagerComputeBYTE::releaseNearestIndexSelector(XilOp* owner)
{
    if(nearestIndexSelectorOwner == owner ||
       nearestIndexSelectorOwner == NULL) {
        nearestIndexSelectorOwner = NULL;
    }
}
