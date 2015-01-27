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
//  File:	XilOpEntry.cc
//  Project:	XIL
//  Revision:	1.22
//  Last Mod:	10:07:01, 03/10/00
//
//  Description:
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
#pragma ident	"@(#)XilOpEntry.cc	1.22\t00/03/10  "

#include <xil/xilGPI.hh>

//
//  Common Routines for all basic XIL imaging operations
//
XilStatus
XilOpEntry(void) {
    XilGlobalState* gs = XilGlobalState::getXilGlobalState();
  

    //
    // XilOp operaters 
    //

    // Copy 
    gs->describeOpFunction("copy", "XilOpCopy");

    // Copy with planemask
    gs->describeOpFunction("copy_with_planemask", "XilOpCopyWithPlanemask");
   
    // Arithmetic 
    gs->describeOpFunction("add", "XilOpAdd");
    gs->describeOpFunction("subtract", "XilOpSubtract");
    gs->describeOpFunction("multiply", "XilOpMultiply");
    gs->describeOpFunction("divide", "XilOpDivide");
   
    // Arithmetic const 
    gs->describeOpFunction("add_const", "XilOpAddConst");
    gs->describeOpFunction("subtract_const", "XilOpSubtractConst");
    gs->describeOpFunction("subtract_from_const", "XilOpSubtractFromConst");
    gs->describeOpFunction("multiply_const", "XilOpMultiplyConst");
    gs->describeOpFunction("divide_by_const", "XilOpDivideByConst");
    gs->describeOpFunction("divide_into_const", "XilOpDivideIntoConst");

    // Logical
    gs->describeOpFunction("and", "XilOpAnd");
    gs->describeOpFunction("not", "XilOpNot");
    gs->describeOpFunction("or", "XilOpOr");
    gs->describeOpFunction("xor", "XilOpXor");

    // Logical with const
    gs->describeOpFunction("and_const", "XilOpAndConst");
    gs->describeOpFunction("or_const", "XilOpOrConst");
    gs->describeOpFunction("xor_const", "XilOpXorConst");

    // Absolute, Max, Min
    gs->describeOpFunction("absolute", "XilOpAbsolute");
    gs->describeOpFunction("max", "XilOpMax");
    gs->describeOpFunction("min", "XilOpMin");

    // Black Generation
    gs->describeOpFunction("black_generation", "XilOpBlackGeneration");

    // Blend
    gs->describeOpFunction("blend", "XilOpBlend");

    // Cast
    gs->describeOpFunction("cast", "XilOpCast");

    // Rescale
    gs->describeOpFunction("rescale", "XilOpRescale");

    // Set Value
    gs->describeOpFunction("set_value", "XilOpSetValue");

    // Threshold
    gs->describeOpFunction("threshold", "XilOpThreshold");

    // Lookup
    gs->describeOpFunction("lookup", "XilOpLookup");

    // Choose colormap
    gs->describeOpFunction("choose_colormap", "XilOpChooseColormap");

    // Nearest color
    gs->describeOpFunction("nearest_color", "XilOpNearestColor");

    // Histogram
    gs->describeOpFunction("histogram", "XilOpHistogram");

    // Blur
    gs->describeOpFunction("unsharp", "XilOpUnsharp");
    gs->describeOpFunction("unsharp_ic", "XilOpUnsharpIC");

    // Area kernel operations
    gs->describeOpFunction("dilate", "XilOpDilate");
    gs->describeOpFunction("erode", "XilOpErode");
    gs->describeOpFunction("convolve", "XilOpConvolve");
    gs->describeOpFunction("edge_detection", "XilOpEdgeDetection");

    // Extrema
    gs->describeOpFunction("extrema", "XilOpExtrema");

    // SqueezeRange
    gs->describeOpFunction("squeeze_range", "XilOpSqueezeRange");

    // Geometrics
    gs->describeOpFunction("affine", "XilOpAffine");
    gs->describeOpFunction("scale", "XilOpScale");
    gs->describeOpFunction("rotate", "XilOpRotate");
    gs->describeOpFunction("translate", "XilOpTranslate");
    gs->describeOpFunction("transpose", "XilOpTranspose");

    // Subsample
    gs->describeOpFunction("subsample_adaptive", "XilOpSubsampleAdaptive");
    gs->describeOpFunction("subsample_binary_to_gray", "XilOpSubsampleBinaryToGray");
    
    // Tablewarp
    gs->describeOpFunction("tablewarp", "XilOpTablewarp");
    gs->describeOpFunction("tablewarp_horizontal", "XilOpTablewarpHorizontal");
    gs->describeOpFunction("tablewarp_vertical", "XilOpTablewarpVertical");

    // BandCombine
    gs->describeOpFunction("band_combine", "XilOpBandCombine");

    // CopyPattern
    gs->describeOpFunction("copy_pattern", "XilOpCopyPattern");

    // Capture and display
    gs->describeOpFunction("display", "XilOpDisplay");
    gs->describeOpFunction("capture", "XilOpCapture");

    // Fill and SoftFill
    gs->describeOpFunction("paint", "XilOpPaint");
    gs->describeOpFunction("fill", "XilOpFill");
    gs->describeOpFunction("soft_fill", "XilOpSoftFill");

    // Compress and Decompress
    gs->describeOpFunction("compress", "XilOpCompress");
    gs->describeOpFunction("decompress", "XilOpDecompress");

    // Ordered dither
    gs->describeOpFunction("ordered_dither", "XilOpOrderedDither");

    // Error diffusion
    gs->describeOpFunction("error_diffusion", "XilOpErrorDiffusion");

    // ColorConvert & ColorCorrect
    gs->describeOpFunction("color_convert", "XilOpColorConvert");
    gs->describeOpFunction("color_correct", "XilOpColorCorrect");

    return XIL_SUCCESS;
}
