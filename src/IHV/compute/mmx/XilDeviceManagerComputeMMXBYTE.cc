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
//  File:	XilDeviceManagerComputeMMXBYTE.cc
//  Project:	XIL
//  Revision:	1.96
//  Last Mod:	16:58:31, 05/08/97
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
#pragma ident	"@(#)XilDeviceManagerComputeMMXBYTE.cc	1.96\t97/05/08  "

#include "XilDeviceManagerComputeMMXBYTE.hh"
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

    XilDeviceManagerComputeMMXBYTE* device;

    device = new XilDeviceManagerComputeMMXBYTE;
    
    if(device == NULL) {
        XIL_ERROR(NULL, XIL_ERROR_RESOURCE, "di-1", TRUE);
        return NULL;
    }

    return device;
}

XilDeviceManagerComputeMMXBYTE::XilDeviceManagerComputeMMXBYTE()
{
    int i;
    
    for(i = 0; i < _XILI_NUM_RESCALE_TABLES; i++) {
        rescaleCache[i]   = NULL;
        rescaleRefCnts[i] = 0;
    }
}

XilDeviceManagerComputeMMXBYTE::~XilDeviceManagerComputeMMXBYTE()
{
    int i;

    for(i = 0; i < _XILI_NUM_RESCALE_TABLES; i++) {
        delete [] rescaleCache[i];
    }
}

const char*
XilDeviceManagerComputeMMXBYTE::getDeviceName()
{
    return "SunSoft_XIL_MMXBYTE";
}

XilStatus
XilDeviceManagerComputeMMXBYTE::describeMembers()
{
    XilFunctionInfo*  func_info;

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "add;8");
    func_info->setFunction((XilComputeFunctionPtr)
                           XilDeviceManagerComputeMMXBYTE::Add);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "add_const;8");
    func_info->setFunction((XilComputeFunctionPtr)
                           XilDeviceManagerComputeMMXBYTE::AddConst);
    addFunction(func_info);
    func_info->destroy();

    // TODO: 10/9/95 dtb
    // all of the geometrics point to the same function for testing
    // purposes, when the real versions are added we may want to call
    // different methods.

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "and;8");
    func_info->setFunction((XilComputeFunctionPtr)
                           XilDeviceManagerComputeMMXBYTE::And);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "and_const;8");
    func_info->setFunction((XilComputeFunctionPtr)
                           XilDeviceManagerComputeMMXBYTE::AndConst);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "blend;8,8,8->8");
    func_info->setFunction((XilComputeFunctionPtr)
                           XilDeviceManagerComputeMMXBYTE::Blenda8);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "convolve;8");
    func_info->setFunction((XilComputeFunctionPtr)
                           XilDeviceManagerComputeMMXBYTE::Convolve);
    func_info->setPreprocessFunction((XilComputePreprocessFunctionPtr)
                           XilDeviceManagerComputeMMXBYTE::ConvolvePreprocess);
    func_info->setPostprocessFunction((XilComputePostprocessFunctionPtr)
                           XilDeviceManagerComputeMMXBYTE::ConvolvePostprocess);
    addFunction(func_info);
    func_info->destroy();
    
    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "separable_convolve;8");
    func_info->setFunction((XilComputeFunctionPtr)
                           XilDeviceManagerComputeMMXBYTE::Convolve_Separable);
    func_info->setPreprocessFunction((XilComputePreprocessFunctionPtr)
                           XilDeviceManagerComputeMMXBYTE::Convolve_SeparablePreprocess);
    func_info->setPostprocessFunction((XilComputePostprocessFunctionPtr)
                           XilDeviceManagerComputeMMXBYTE::Convolve_SeparablePostprocess);
    addFunction(func_info);
    func_info->destroy();
    
    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "copy;8");
    func_info->setFunction((XilComputeFunctionPtr)
                           XilDeviceManagerComputeMMXBYTE::Copy);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "lookup;8->8");
    func_info->setFunction((XilComputeFunctionPtr)
                           XilDeviceManagerComputeMMXBYTE::Lookup8);
    func_info->setPreprocessFunction((XilComputePreprocessFunctionPtr)
                           XilDeviceManagerComputeMMXBYTE::Lookup8Preprocess);
    func_info->setPostprocessFunction((XilComputePostprocessFunctionPtr)
                           XilDeviceManagerComputeMMXBYTE::Lookup8Postprocess);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "multiply;8");
    func_info->setFunction((XilComputeFunctionPtr)
                           XilDeviceManagerComputeMMXBYTE::Multiply);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "multiply_const;8");
    func_info->setFunction((XilComputeFunctionPtr)
                           XilDeviceManagerComputeMMXBYTE::MultiplyConst);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "not;8");
    func_info->setFunction((XilComputeFunctionPtr)
                           XilDeviceManagerComputeMMXBYTE::Not);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "or;8");
    func_info->setFunction((XilComputeFunctionPtr)
                           XilDeviceManagerComputeMMXBYTE::Or);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "or_const;8");
    func_info->setFunction((XilComputeFunctionPtr)
                           XilDeviceManagerComputeMMXBYTE::OrConst);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "rescale;8");
    func_info->setFunction((XilComputeFunctionPtr)
                           XilDeviceManagerComputeMMXBYTE::Rescale);
    func_info->setPreprocessFunction((XilComputePreprocessFunctionPtr)
                           XilDeviceManagerComputeMMXBYTE::RescalePreprocess);
    func_info->setPostprocessFunction((XilComputePostprocessFunctionPtr)
                           XilDeviceManagerComputeMMXBYTE::RescalePostprocess);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "scale_nearest;8");
    func_info->setFunction((XilComputeFunctionPtr)
                           XilDeviceManagerComputeMMXBYTE::ScaleNearest);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "scale_bilinear;8");
    func_info->setFunction((XilComputeFunctionPtr)
                           XilDeviceManagerComputeMMXBYTE::ScaleBilinear);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "scale_bicubic;8");
    func_info->setFunction((XilComputeFunctionPtr)
                           XilDeviceManagerComputeMMXBYTE::ScaleBicubic);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "set_value;8");
    func_info->setFunction((XilComputeFunctionPtr)
                           XilDeviceManagerComputeMMXBYTE::SetValue);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "subtract;8");
    func_info->setFunction((XilComputeFunctionPtr)
                           XilDeviceManagerComputeMMXBYTE::Subtract);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "subtract_from_const;8");
    func_info->setFunction((XilComputeFunctionPtr)
                           XilDeviceManagerComputeMMXBYTE::SubtractFromConst);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "subtract_const;8");
    func_info->setFunction((XilComputeFunctionPtr)
                           XilDeviceManagerComputeMMXBYTE::SubtractConst);
    addFunction(func_info);
    func_info->destroy();

#if 0
    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "transpose;8");
    func_info->setFunction((XilComputeFunctionPtr)
                           XilDeviceManagerComputeMMXBYTE::Transpose);
    addFunction(func_info);
    func_info->destroy();
#endif

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "xor;8");
    func_info->setFunction((XilComputeFunctionPtr)
                           XilDeviceManagerComputeMMXBYTE::Xor);
    addFunction(func_info);
    func_info->destroy();
    
    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "xor_const;8");
    func_info->setFunction((XilComputeFunctionPtr)
                           XilDeviceManagerComputeMMXBYTE::XorConst);
    addFunction(func_info);
    func_info->destroy();

    return XIL_SUCCESS;
}

#if 0
    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "blend;8,8,1->8");
    func_info->setFunction((XilComputeFunctionPtr)
                           XilDeviceManagerComputeMMXBYTE::Blenda1);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "blend;8,8,16->8");
    func_info->setFunction((XilComputeFunctionPtr)
                           XilDeviceManagerComputeMMXBYTE::Blenda16);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "cast;8->1");
    func_info->setFunction((XilComputeFunctionPtr)
                           XilDeviceManagerComputeMMXBYTE::CastTo1);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "cast;8->16");
    func_info->setFunction((XilComputeFunctionPtr)
                           XilDeviceManagerComputeMMXBYTE::CastTo16);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "color_convert;8");
    func_info->setFunction((XilComputeFunctionPtr)
                           XilDeviceManagerComputeMMXBYTE::ColorConvert);
    addFunction(func_info);
    func_info->destroy();
    
    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "divide_by_const;8");
    func_info->setFunction((XilComputeFunctionPtr)
                           XilDeviceManagerComputeMMXBYTE::DivideByConst);
    func_info->setPreprocessFunction((XilComputePreprocessFunctionPtr)
                           XilDeviceManagerComputeMMXBYTE::DivideByConstPreprocess);
    func_info->setPostprocessFunction((XilComputePostprocessFunctionPtr)
                           XilDeviceManagerComputeMMXBYTE::DivideByConstPostprocess);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "divide_into_const;8");
    func_info->setFunction((XilComputeFunctionPtr)
                           XilDeviceManagerComputeMMXBYTE::DivideIntoConst);
#ifndef _XIL_USE_INTMULDIV
    func_info->setPreprocessFunction((XilComputePreprocessFunctionPtr)
                           XilDeviceManagerComputeMMXBYTE::DivideIntoConstPreprocess);
    func_info->setPostprocessFunction((XilComputePostprocessFunctionPtr)
                           XilDeviceManagerComputeMMXBYTE::DivideIntoConstPostprocess);
#endif // _XIL_USE_INTMULDIV
    addFunction(func_info);
    func_info->destroy();
    
    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "extrema;8");
    func_info->setFunction((XilComputeFunctionPtr)
                           XilDeviceManagerComputeMMXBYTE::Extrema);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "histogram;8");
    func_info->setFunction((XilComputeFunctionPtr)
                           XilDeviceManagerComputeMMXBYTE::Histogram);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "lookup;8->1");
    func_info->setFunction((XilComputeFunctionPtr)
                           XilDeviceManagerComputeMMXBYTE::Lookup1);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "lookup;8->16");
    func_info->setFunction((XilComputeFunctionPtr)
                           XilDeviceManagerComputeMMXBYTE::Lookup16);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "lookup_n_to_n;8->1");
    func_info->setFunction((XilComputeFunctionPtr)
                           XilDeviceManagerComputeMMXBYTE::LookupCombined1);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "lookup_n_to_n;8->8");
    func_info->setFunction((XilComputeFunctionPtr)
                           XilDeviceManagerComputeMMXBYTE::LookupCombined8);
    func_info->setPreprocessFunction((XilComputePreprocessFunctionPtr)
                    XilDeviceManagerComputeMMXBYTE::LookupCombined8Preprocess);
    func_info->setPostprocessFunction((XilComputePostprocessFunctionPtr)
                    XilDeviceManagerComputeMMXBYTE::LookupCombined8Postprocess);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "lookup_n_to_n;8->16");
    func_info->setFunction((XilComputeFunctionPtr)
                           XilDeviceManagerComputeMMXBYTE::LookupCombined16);
    func_info->setPreprocessFunction((XilComputePreprocessFunctionPtr)
                    XilDeviceManagerComputeMMXBYTE::LookupCombined16Preprocess);
    func_info->setPostprocessFunction((XilComputePostprocessFunctionPtr)
                    XilDeviceManagerComputeMMXBYTE::LookupCombined16Postprocess);
    addFunction(func_info);
    func_info->destroy();

    // Rotate, geometric
    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "rotate_nearest;8");
    func_info->setFunction((XilComputeFunctionPtr)
                           XilDeviceManagerComputeMMXBYTE::RotateNearest);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "rotate_bilinear;8");
    func_info->setFunction((XilComputeFunctionPtr)
                           XilDeviceManagerComputeMMXBYTE::RotateBilinear);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "rotate_bicubic;8");
    func_info->setFunction((XilComputeFunctionPtr)
                           XilDeviceManagerComputeMMXBYTE::RotateBicubic);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "threshold;8");
    func_info->setFunction((XilComputeFunctionPtr)
                           XilDeviceManagerComputeMMXBYTE::Threshold);
    addFunction(func_info);
    func_info->destroy();

    //
    //  The Threshold_1BAND function must be listed after the generic
    //  Threshold so it will be called first.
    //
    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "threshold;8");
    func_info->setFunction((XilComputeFunctionPtr)
                           XilDeviceManagerComputeMMXBYTE::Threshold_1BAND);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "threshold;8");
    func_info->describeOp(XIL_STEP, 1, "threshold;8");
    func_info->setFunction((XilComputeFunctionPtr)
                           XilDeviceManagerComputeMMXBYTE::ThresholdThreshold,
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
                           XilDeviceManagerComputeMMXBYTE::Copy);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "translate_bilinear;8");
    func_info->setFunction((XilComputeFunctionPtr)
                           XilDeviceManagerComputeMMXBYTE::TranslateBilinear);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "translate_bicubic;8");
    func_info->setFunction((XilComputeFunctionPtr)
                           XilDeviceManagerComputeMMXBYTE::TranslateBicubic);
    addFunction(func_info);
    func_info->destroy();

    // BandCombine
    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "band_combine;8");
    func_info->setFunction((XilComputeFunctionPtr)
                           XilDeviceManagerComputeMMXBYTE::BandCombine);
    addFunction(func_info);
    func_info->destroy();

#endif
