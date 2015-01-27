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
//  File:	XilDeviceManagerComputeBYTE.hh
//  Project:	XIL
//  Revision:	1.72
//  Last Mod:	10:22:21, 03/10/00
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
#pragma ident	"@(#)XilDeviceManagerComputeBYTE.hh	1.72\t00/03/10  "

#ifndef _XIL_DEVICE_MANAGER_COMPUTE_BYTE_HH
#define _XIL_DEVICE_MANAGER_COMPUTE_BYTE_HH

#include <xil/xilGPI.hh>
#include "xili_geom_utils.hh"
#include "XiliOrderedDitherLut.hh"
#include "XiliColormapGenerator.hh"

//
// Forward Declarations
//
class ComputeInfoBYTE;
class XiliNearestIndexSelector;

class XilDeviceManagerComputeBYTE : public XilDeviceManagerCompute
{
public:
    //
    //  Constructor/Destructor
    //
                   XilDeviceManagerComputeBYTE();
                   ~XilDeviceManagerComputeBYTE();

    //
    //  Required Virtual Functions
    //
    const char*    getDeviceName();
    XilStatus      describeMembers();
    
    //
    //  Compute Routines
    //
    XilStatus       Add(XilOp*       op,
                        unsigned     op_count,
                        XilRoi*      roi,
                        XilBoxList*  bl);
    
    //
    //  AddConst
    //
    XilStatus       AddConst(XilOp*       op,
                             unsigned     op_count,
                             XilRoi*      roi,
                             XilBoxList*  bl);
    XilStatus       AddConstPreprocess(XilOp*        op,
                                       unsigned int  op_count,
                                       XilRoi*       roi,
                                       void**        compute_data,
                                       unsigned int* func_ident);
    XilStatus       AddConstPostprocess(XilOp*       op,
                                        void*        compute_data);

    XilStatus       AffineNearest(XilOp*       op,
                                  unsigned     op_count,
                                  XilRoi*      roi,
                                  XilBoxList*  bl);

    XilStatus       AffineBilinear(XilOp*       op,
                                   unsigned     op_count,
                                   XilRoi*      roi,
                                   XilBoxList*  bl);

    XilStatus       AffineBicubic(XilOp*       op,
                                  unsigned     op_count,
                                  XilRoi*      roi,
                                  XilBoxList*  bl);
    
    XilStatus       AffineGeneral(XilOp*       op,
                                  unsigned     op_count,
                                  XilRoi*      roi,
                                  XilBoxList*  bl);
    
    
    XilStatus       And(XilOp*       op,
                        unsigned     op_count,
                        XilRoi*      roi,
                        XilBoxList*  bl);
    
    XilStatus       AndConst(XilOp*       op,
                             unsigned     op_count,
                             XilRoi*      roi,
                             XilBoxList*  bl);
    
    XilStatus       BandCombine(XilOp*       op,
                                unsigned     op_count,
                                XilRoi*      roi,
                                XilBoxList*  bl);
    
    XilStatus       BlackGeneration(XilOp*       op,
                                    unsigned     op_count,
                                    XilRoi*      roi,
                                    XilBoxList*  bl);
  
    XilStatus       Blenda1(XilOp*       op,
                            unsigned     op_count,
                            XilRoi*      roi,
                            XilBoxList*  bl);
    
    XilStatus       Blenda8(XilOp*       op,
                            unsigned     op_count,
                            XilRoi*      roi,
                            XilBoxList*  bl);
    
    XilStatus       Blenda16(XilOp*       op,
                             unsigned     op_count,
                             XilRoi*      roi,
                             XilBoxList*  bl);
    
    XilStatus       Blendaf32(XilOp*       op,
                              unsigned     op_count,
                              XilRoi*      roi,
                              XilBoxList*  bl);
    
    XilStatus       CastTo1(XilOp*       op,
                            unsigned     op_count,
                            XilRoi*      roi,
                            XilBoxList*  bl);
    
    XilStatus       CastTo16(XilOp*       op,
                             unsigned     op_count,
                             XilRoi*      roi,
                             XilBoxList*  bl);
    
    XilStatus       CastTof32(XilOp*       op,
			     unsigned     op_count,
			     XilRoi*      roi,
			     XilBoxList*  bl);

    XilStatus       ChooseColormapPreprocess(XilOp*        op,
                                             unsigned      op_count,
                                             XilRoi*       roi,
                                             void**        compute_data,
                                             unsigned int* func_ident);

    XilStatus       ChooseColormapPostprocess(XilOp*       op,
                                              void*        compute_data);

    XilStatus       ChooseColormap(XilOp*       op,
                                   unsigned     op_count,
                                   XilRoi*      roi,
                                   XilBoxList*  bl);
    
    XilStatus       ColorConvert(XilOp*       op,
				 unsigned     op_count,
				 XilRoi*      roi,
				 XilBoxList*  bl);
    
    XilStatus       Convolve(XilOp*       op,
			     unsigned     op_count,
			     XilRoi*      roi,
			     XilBoxList*  bl);
    
    XilStatus       Convolve_SeparablePreprocess(XilOp*        op,
                                                 unsigned int  op_count,
                                                 XilRoi*       roi,
                                                 void**        compute_data,
                                                 unsigned int* func_ident);
    XilStatus       Convolve_Separable(XilOp*       op,
                                       unsigned     op_count,
                                       XilRoi*      roi,
                                       XilBoxList*  bl);
    
    XilStatus       Copy(XilOp*       op,
                         unsigned     op_count,
                         XilRoi*      roi,
                         XilBoxList*  bl);
    
    XilStatus       CopyWithPlanemask(XilOp*       op,
                                      unsigned     op_count,
                                      XilRoi*      roi,
                                      XilBoxList*  bl);
  
    XilStatus       Dilate(XilOp*       op,
                           unsigned     op_count,
                           XilRoi*      roi,
                           XilBoxList*  bl);
    
    XilStatus       Divide(XilOp*       op,
                           unsigned     op_count,
                           XilRoi*      roi,
                           XilBoxList*  bl);
    
    //
    //  DivideByConst
    //
    XilStatus       DivideByConst(XilOp*       op,
				  unsigned     op_count,
				  XilRoi*      roi,
				  XilBoxList*  bl);
    XilStatus       DivideByConstPreprocess(XilOp*        op,
                                            unsigned int  op_count,
                                            XilRoi*       roi,
                                            void**        compute_data,
                                            unsigned int* func_ident);
    XilStatus       DivideByConstPostprocess(XilOp*       op,
                                             void*        compute_data);

    //
    //  DivideIntoConst
    //
    XilStatus       DivideIntoConst(XilOp*       op,
                                    unsigned     op_count,
                                    XilRoi*      roi,
                                    XilBoxList*  bl);
#ifndef _XIL_USE_INTMULDIV
    XilStatus       DivideIntoConstPreprocess(XilOp*        op,
                                              unsigned int  op_count,
                                              XilRoi*       roi,
                                              void**        compute_data,
                                              unsigned int* func_ident);
    XilStatus       DivideIntoConstPostprocess(XilOp*       op,
                                               void*        compute_data);
#endif // _XIL_USE_INTMULDIV

    XilStatus       EdgeDetection(XilOp*       op,
				  unsigned     op_count,
				  XilRoi*      roi,
				  XilBoxList*  bl);
    
    XilStatus       Erode(XilOp*       op,
                          unsigned     op_count,
                          XilRoi*      roi,
                          XilBoxList*  bl);

    XilStatus       ErrorDiffusion1(XilOp*       op,
				    unsigned     op_count,
				    XilRoi*      roi,
				    XilBoxList*  bl);

    XilStatus       ErrorDiffusion8(XilOp*       op,
				    unsigned     op_count,
				    XilRoi*      roi,
				    XilBoxList*  bl);

    XilStatus       ErrorDiffusion8Preprocess(XilOp*        op,
                                              unsigned int  op_count,
                                              XilRoi*       roi,
                                              void**        compute_data,
                                              unsigned int* func_ident);

    XilStatus       ErrorDiffusion8Postprocess(XilOp*       op,
                                               void*        compute_data);

    XilStatus       ErrorDiffusion16(XilOp*       op,
				     unsigned     op_count,
				     XilRoi*      roi,
				     XilBoxList*  bl);

    XilStatus       Extrema(XilOp*       op,
                            unsigned     op_count,
                            XilRoi*      roi,
                            XilBoxList*  bl);

    XilStatus       Fill(XilOp*       op,
			    unsigned     op_count,
                            XilRoi*      roi,
                            XilBoxList*  bl);
    
    XilStatus       Histogram(XilOp*       op,
                              unsigned     op_count,
                              XilRoi*      roi,
                              XilBoxList*  bl);
    
    XilStatus       Lookup1(XilOp*       op,
			    unsigned     op_count,
			    XilRoi*      roi,
			    XilBoxList*  bl);
    
    XilStatus       Lookup8(XilOp*       op,
			    unsigned     op_count,
			    XilRoi*      roi,
			    XilBoxList*  bl);
    XilStatus       Lookup8Preprocess(XilOp*        op,
                                      unsigned int  op_count,
                                      XilRoi*       roi,
				      void**        compute_data,
                                      unsigned int* func_ident);
    XilStatus       Lookup8Postprocess(XilOp*       op,
                                       void*        compute_data);
    
    XilStatus       Lookupf32(XilOp*       op,
			      unsigned     op_count,
			      XilRoi*      roi,
			      XilBoxList*  bl);
    
    XilStatus       Lookup16(XilOp*       op,
			     unsigned     op_count,
			     XilRoi*      roi,
			     XilBoxList*  bl);
    
    XilStatus       LookupCombined1(XilOp*       op,
				    unsigned     op_count,
				    XilRoi*      roi,
				    XilBoxList*  bl);

    XilStatus       LookupCombined8(XilOp*       op,
				    unsigned     op_count,
				    XilRoi*      roi,
				    XilBoxList*  bl);
    XilStatus       LookupCombined8Preprocess(XilOp*        op,
                                              unsigned int  op_count,
                                              XilRoi*       roi,
                                              void**        compute_data,
                                              unsigned int* func_ident);
    XilStatus       LookupCombined8Postprocess(XilOp*       op,
					       void*        compute_data);
    
    XilStatus       LookupCombined16(XilOp*       op,
				     unsigned     op_count,
				     XilRoi*      roi,
				     XilBoxList*  bl);
    XilStatus       LookupCombined16Preprocess(XilOp*        op,
                                               unsigned int  op_count,
                                               XilRoi*       roi,
                                               void**        compute_data,
                                               unsigned int* func_ident);
    XilStatus       LookupCombined16Postprocess(XilOp*       op,
						void*        compute_data);

    XilStatus       LookupCombinedf32(XilOp*       op,
				      unsigned     op_count,
				      XilRoi*      roi,
				      XilBoxList*  bl);

    XilStatus       Max(XilOp*       op,
                        unsigned     op_count,
                        XilRoi*      roi,
                        XilBoxList*  bl);
    
    XilStatus       Min(XilOp*       op,
                        unsigned     op_count,
                        XilRoi*      roi,
                        XilBoxList*  bl);
    
    XilStatus       Multiply(XilOp*       op,
                             unsigned     op_count,
                             XilRoi*      roi,
                             XilBoxList*  bl);
    
    //
    //  MultiplyConst
    //
    XilStatus       MultiplyConst(XilOp*       op,
				  unsigned     op_count,
				  XilRoi*      roi,
				  XilBoxList*  bl);
    XilStatus       MultiplyConstPreprocess(XilOp*        op,
                                            unsigned int  op_count,
                                            XilRoi*       roi,
                                            void**        compute_data,
                                            unsigned int* func_ident);
    XilStatus       MultiplyConstPostprocess(XilOp* op,
                                             void*  compute_data);

    XilStatus       NearestColor1(XilOp*       op,
				  unsigned     op_count,
				  XilRoi*      roi,
				  XilBoxList*  bl);

    XilStatus       NearestColor8(XilOp*       op,
				  unsigned     op_count,
				  XilRoi*      roi,
				  XilBoxList*  bl);
    XilStatus       NearestColor8Preprocess(XilOp*        op,
                                            unsigned int  op_count,
                                            XilRoi*       roi,
                                            void**        compute_data,
                                            unsigned int* func_ident);
    XilStatus       NearestColor8Postprocess(XilOp*       op,
					     void*        compute_data);
    
    XilStatus       NearestColor16(XilOp*       op,
				   unsigned     op_count,
				   XilRoi*      roi,
				   XilBoxList*  bl);
    
    XilStatus       Not(XilOp*       op,
                        unsigned     op_count,
                        XilRoi*      roi,
                        XilBoxList*  bl);
    
    XilStatus       Or(XilOp*       op,
                       unsigned     op_count,
                       XilRoi*      roi,
                       XilBoxList*  bl);
    
    XilStatus       OrConst(XilOp*       op,
                            unsigned     op_count,
                            XilRoi*      roi,
                            XilBoxList*  bl);

    XilStatus       OrderedDither1(XilOp*       op,
				   unsigned     op_count,
				   XilRoi*      roi,
				   XilBoxList*  bl);

    XilStatus       OrderedDither8(XilOp*       op,
				   unsigned     op_count,
				   XilRoi*      roi,
				   XilBoxList*  bl);

    XilStatus       OrderedDither8Preprocess(XilOp*        op,
                                      unsigned int  op_count,
                                      XilRoi*       roi,
				      void**        compute_data,
                                      unsigned int* func_ident);

    XilStatus       OrderedDither8Postprocess(XilOp*       op,
                                       void*        compute_data);
    

    XilStatus       OrderedDither16(XilOp*       op,
				    unsigned     op_count,
				    XilRoi*      roi,
				    XilBoxList*  bl);
   
    XilStatus       Paint(XilOp*       op,
			  unsigned     op_count,
			  XilRoi*      roi,
			  XilBoxList*  bl);
    XilStatus       Paint3Band(XilOp*       op,
                               unsigned     op_count,
                               XilRoi*      roi,
                               XilBoxList*  bl);
    XilStatus       Paint3BandPreprocess(XilOp*        op,
                                         unsigned int  op_count,
                                         XilRoi*       roi,
                                         void**        compute_data,
                                         unsigned int* func_ident);
    XilStatus       Paint3BandPostprocess(XilOp*       op,
                                          void*        compute_data);
    
    
    XilStatus       Rescale(XilOp*       op,
                            unsigned     op_count,
                            XilRoi*      roi,
                            XilBoxList*  bl);
    XilStatus       RescalePreprocess(XilOp*        op,
                                      unsigned int  op_count,
                                      XilRoi*       roi,
                                      void**        compute_data,
                                      unsigned int* func_ident);
    XilStatus       RescalePostprocess(XilOp*       op,
                                       void*        compute_data);

    XilStatus       RotateNearest(XilOp*       op,
				  unsigned     op_count,
				  XilRoi*      roi,
				  XilBoxList*  bl);

    XilStatus       RotateBilinear(XilOp*       op,
				   unsigned     op_count,
				   XilRoi*      roi,
				   XilBoxList*  bl);

    XilStatus       RotateBicubic(XilOp*       op,
				  unsigned     op_count,
				  XilRoi*      roi,
				  XilBoxList*  bl);

    XilStatus       RotateGeneral(XilOp*       op,
				  unsigned     op_count,
				  XilRoi*      roi,
				  XilBoxList*  bl);

    XilStatus       ScaleNearest(XilOp*       op,
				 unsigned     op_count,
				 XilRoi*      roi,
				 XilBoxList*  bl);
    
    XilStatus       ScaleBilinear(XilOp*       op,
				  unsigned     op_count,
				  XilRoi*      roi,
				  XilBoxList*  bl);
    
    XilStatus       ScaleBicubic(XilOp*       op,
				 unsigned     op_count,
				 XilRoi*      roi,
				 XilBoxList*  bl);
    
    XilStatus       ScaleGeneral(XilOp*       op,
				 unsigned     op_count,
				 XilRoi*      roi,
				 XilBoxList*  bl);

    //
    //  SetValue
    //
    XilStatus       SetValue(XilOp*       op,
                             unsigned     op_count,
                             XilRoi*      roi,
                             XilBoxList*  bl);
    XilStatus       SetValuePreprocess(XilOp*        op,
                                       unsigned int  op_count,
                                       XilRoi*       roi,
                                       void**        compute_data,
                                       unsigned int* func_ident);

    XilStatus       SoftFill(XilOp*       op,
                             unsigned     op_count,
                             XilRoi*      roi,
                             XilBoxList*  bl);
    
    //
    //  Squeeze Range
    //
    XilStatus       SqueezeRange(XilOp*       op,
                                 unsigned     op_count,
                                 XilRoi*      roi,
                                 XilBoxList*  bl);

    XilStatus       SubsampleAdaptive(XilOp*       op,
				      unsigned     op_count,
				      XilRoi*      roi,
				      XilBoxList*  bl);	

    XilStatus       SubsampleBinaryToGray(XilOp*       op,
					  unsigned     op_count,
					  XilRoi*      roi,
					  XilBoxList*  bl);	
    
    XilStatus       Subtract(XilOp*       op,
                             unsigned     op_count,
                             XilRoi*      roi,
                             XilBoxList*  bl);

    //
    //  SubtractFromConst
    //
    XilStatus       SubtractFromConst(XilOp*       op,
                                      unsigned     op_count,
                                      XilRoi*      roi,
                                      XilBoxList*  bl);
    
    //
    //  SubtractConst
    //
    XilStatus       SubtractConst(XilOp*       op,
                                      unsigned     op_count,
                                      XilRoi*      roi,
                                      XilBoxList*  bl);
    XilStatus       SubtractConstPreprocess(XilOp*        op,
                                            unsigned int  op_count,
                                            XilRoi*       roi,
                                            void**        compute_data,
                                            unsigned int* func_ident);
    XilStatus       SubtractConstPostprocess(XilOp*       op,
                                             void*        compute_data);
     
    XilStatus       TablewarpNearest(XilOp*       op,
				     unsigned     op_count,
				     XilRoi*      roi,
				     XilBoxList*  bl);

    XilStatus       TablewarpBilinear(XilOp*       op,
				      unsigned     op_count,
				      XilRoi*      roi,
				      XilBoxList*  bl);
    
    XilStatus       TablewarpBicubic(XilOp*       op,
				     unsigned     op_count,
				     XilRoi*      roi,
				     XilBoxList*  bl);
    
    XilStatus       TablewarpGeneral(XilOp*       op,
				     unsigned     op_count,
				     XilRoi*      roi,
				     XilBoxList*  bl);

    XilStatus       TablewarpHorizontalNearest(XilOp*       op,
					       unsigned     op_count,
					       XilRoi*      roi,
					       XilBoxList*  bl);

    XilStatus       TablewarpHorizontalBilinear(XilOp*       op,
						unsigned     op_count,
						XilRoi*      roi,
						XilBoxList*  bl);
    
    XilStatus       TablewarpHorizontalBicubic(XilOp*       op,
					       unsigned     op_count,
					       XilRoi*      roi,
					       XilBoxList*  bl);
    
    XilStatus       TablewarpHorizontalGeneral(XilOp*       op,
					       unsigned     op_count,
					       XilRoi*      roi,
					       XilBoxList*  bl);
    
    XilStatus       TablewarpVerticalNearest(XilOp*       op,
					     unsigned     op_count,
					     XilRoi*      roi,
					     XilBoxList*  bl);

    XilStatus       TablewarpVerticalBilinear(XilOp*       op,
					      unsigned     op_count,
					      XilRoi*      roi,
					      XilBoxList*  bl);
    
    XilStatus       TablewarpVerticalBicubic(XilOp*       op,
					     unsigned     op_count,
					     XilRoi*      roi,
					     XilBoxList*  bl);
    
    XilStatus       TablewarpVerticalGeneral(XilOp*       op,
					     unsigned     op_count,
					     XilRoi*      roi,
					     XilBoxList*  bl);
   
    XilStatus       Threshold(XilOp*       op,
                              unsigned     op_count,
                              XilRoi*      roi,
                              XilBoxList*  bl);
    XilStatus       Threshold_1BAND(XilOp*       op,
                                    unsigned     op_count,
                                    XilRoi*      roi,
                                    XilBoxList*  bl);
    XilStatus       ThresholdThreshold(XilOp*       op,
                                       unsigned     op_count,
                                       XilRoi*      roi,
                                       XilBoxList*  bl);


    XilStatus       TranslateNearest(XilOp*       op,
                              unsigned     op_count,
                              XilRoi*      roi,
                              XilBoxList*  bl);

    XilStatus       TranslateBilinear(XilOp*       op,
				      unsigned     op_count,
				      XilRoi*      roi,
				      XilBoxList*  bl);
    
    XilStatus       TranslateBicubic(XilOp*       op,
				     unsigned     op_count,
				     XilRoi*      roi,
				     XilBoxList*  bl);
    
    XilStatus       TranslateGeneral(XilOp*       op,
				     unsigned     op_count,
				     XilRoi*      roi,
				     XilBoxList*  bl);

    XilStatus       Transpose(XilOp*       op,
                              unsigned     op_count,
                              XilRoi*      roi,
                              XilBoxList*  bl);
    
    XilStatus       Xor(XilOp*       op,
                        unsigned     op_count,
                        XilRoi*      roi,
                        XilBoxList*  bl);
    
    XilStatus       XorConst(XilOp*       op,
                             unsigned     op_count,
                             XilRoi*      roi,
                             XilBoxList*  bl);

    XilStatus       AddCopyCopy(XilOp*       op,
                                unsigned     op_count,
                                XilRoi*      roi,
                                XilBoxList*  bl);
    
private:

	static Xil_unsigned8 _L2NLbtable[256];
	static Xil_unsigned8 _NL2Lbtable[256];
	static Xil_unsigned8 _L2NLtable[6000];
	static Xil_unsigned8 _NL2Ltable[3808];

    XilStatus         photoycc_to_rgb709(XilOp*      op,
                                         XilRoi*     roi,
                                         XilBoxList* bl);

    XilStatus         cmy_to_photoycc(XilOp*      op,
                                        XilRoi*     roi,
                                        XilBoxList* bl);

    XilStatus         cmy_to_ycc601(XilOp*      op,
                                        XilRoi*     roi,
                                        XilBoxList* bl);

    XilStatus         cmy_to_ycc709(XilOp*      op,
                                        XilRoi*     roi,
                                        XilBoxList* bl);

    XilStatus         cmy_to_rgb709(XilOp*      op,
                                        XilRoi*     roi,
                                        XilBoxList* bl);

    XilStatus         cmyk_to_photoycc(XilOp*      op,
                                        XilRoi*     roi,
                                        XilBoxList* bl);

    XilStatus         cmyk_to_ycc601(XilOp*      op,
                                        XilRoi*     roi,
                                        XilBoxList* bl);

    XilStatus         cmyk_to_ycc709(XilOp*      op,
                                        XilRoi*     roi,
                                        XilBoxList* bl);

    XilStatus         cmyk_to_rgb709(XilOp*      op,
                                        XilRoi*     roi,
                                        XilBoxList* bl);

    XilStatus         rgb709_to_rgblinear(XilOp*      op,
                                        XilRoi*     roi,
                                        XilBoxList* bl);

    XilStatus         rgb709_to_cmy(XilOp*      op,
                                        XilRoi*     roi,
                                        XilBoxList* bl);

    XilStatus         rgb709_to_cmyk(XilOp*      op,
                                        XilRoi*     roi,
                                        XilBoxList* bl);

    XilStatus         rgb709_to_ylinear(XilOp*      op,
                                        XilRoi*     roi,
                                        XilBoxList* bl);

    XilStatus         rgblinear_to_y709(XilOp*      op,
                                        XilRoi*     roi,
                                        XilBoxList* bl);

    XilStatus         rgblinear_to_photoycc(XilOp*      op,
                                        XilRoi*     roi,
                                        XilBoxList* bl);

    XilStatus         rgblinear_to_ycc601(XilOp*      op,
                                        XilRoi*     roi,
                                        XilBoxList* bl);

    XilStatus         rgblinear_to_ycc709(XilOp*      op,
                                        XilRoi*     roi,
                                        XilBoxList* bl);

    XilStatus         rgblinear_to_y601(XilOp*      op,
                                        XilRoi*     roi,
                                        XilBoxList* bl);

    XilStatus         rgblinear_to_rgb709(XilOp*      op,
                                        XilRoi*     roi,
                                        XilBoxList* bl);

    XilStatus         ylinear_to_rgb709(XilOp*      op,
                                        XilRoi*     roi,
                                        XilBoxList* bl);

    XilStatus         photoycc_to_rgblinear(XilOp*      op,
                                        XilRoi*     roi,
                                        XilBoxList* bl);

    XilStatus         photoycc_to_cmy(XilOp*      op,
                                        XilRoi*     roi,
                                        XilBoxList* bl);

    XilStatus         photoycc_to_cmyk(XilOp*      op,
                                        XilRoi*     roi,
                                        XilBoxList* bl);

    XilStatus         photoycc_to_ylinear(XilOp*      op,
                                        XilRoi*     roi,
                                        XilBoxList* bl);

    XilStatus         y601_to_rgblinear(XilOp*      op,
                                        XilRoi*     roi,
                                        XilBoxList* bl);

    XilStatus         y601_to_cmy(XilOp*      op,
                                        XilRoi*     roi,
                                        XilBoxList* bl);

    XilStatus         y601_to_cmyk(XilOp*      op,
                                        XilRoi*     roi,
                                        XilBoxList* bl);

    XilStatus         y601_to_ylinear(XilOp*      op,
                                        XilRoi*     roi,
                                        XilBoxList* bl);

    XilStatus         y709_to_rgblinear(XilOp*      op,
                                        XilRoi*     roi,
                                        XilBoxList* bl);

    XilStatus         y709_to_cmy(XilOp*      op,
                                        XilRoi*     roi,
                                        XilBoxList* bl);

    XilStatus         y709_to_cmyk(XilOp*      op,
                                        XilRoi*     roi,
                                        XilBoxList* bl);

    XilStatus         y709_to_ylinear(XilOp*      op,
                                        XilRoi*     roi,
                                        XilBoxList* bl);

    XilStatus         ycc601_to_rgblinear(XilOp*      op,
                                        XilRoi*     roi,
                                        XilBoxList* bl);

    XilStatus         ycc601_to_cmy(XilOp*      op,
                                        XilRoi*     roi,
                                        XilBoxList* bl);

    XilStatus         ycc601_to_cmyk(XilOp*      op,
                                        XilRoi*     roi,
                                        XilBoxList* bl);

    XilStatus         ycc601_to_ylinear(XilOp*      op,
                                        XilRoi*     roi,
                                        XilBoxList* bl);

    XilStatus         ycc709_to_rgblinear(XilOp*      op,
                                        XilRoi*     roi,
                                        XilBoxList* bl);

    XilStatus         ycc709_to_cmy(XilOp*      op,
                                        XilRoi*     roi,
                                        XilBoxList* bl);

    XilStatus         ycc709_to_cmyk(XilOp*      op,
                                        XilRoi*     roi,
                                        XilBoxList* bl);

    XilStatus         ycc709_to_ylinear(XilOp*      op,
                                        XilRoi*     roi,
                                        XilBoxList* bl);

    //
    //  Passing ci by reference because it's easier to read, etc.
    //
    void            tableRescale(ComputeInfoBYTE& ci,
                                 int*             tables);

    XilStatus        affineNearest(XilBoxList* bl,
				   AffineData  ad);

    XilStatus        affineBilinear(XilBoxList* bl,
				    AffineData  ad);

    XilStatus        affineBicubic(XilBoxList* bl,
				   AffineData  ad);

    XilStatus        affineGeneral(XilBoxList* bl,
				   AffineData  ad);

    //
    //  Very fast single-band lookups
    //
    void             fastLookupWrite(Xil_unsigned8* src,
                                     Xil_unsigned8* dst,
                                     unsigned int   xsize,
                                     Xil_unsigned8* lut);

    //
    //  Persistent Data Retrieval
    //
    //
    //  Add/Subtract Clamping Arrays
    //
    Xil_unsigned8*   getAddClampArray(XilSystemState* state);
    Xil_unsigned8*   getSubtractClampArray(XilSystemState* state);

    //
    //  Blending table.
    //
    XilStatus        getBlendTable(XilSystemState* state);

    //
    //  Squares tables.
    //
    Xil_unsigned32*  getSquaresTable(XilSystemState* state);
    Xil_float32*     getSquaresOver4Table(XilSystemState* state);

    //
    //  Nearest Color special 3-band optimized index selector class.
    //
    void                aquireNearestIndexSelector(XilSystemState*  state,
                                                   XilOp*           owner,
                                                   XilLookupSingle* lut);
    void                releaseNearestIndexSelector(XilOp* owner);

    //
    //  Color convert table aquisition.
    //
    Xil_signed32*       getPhotoYArray(XilSystemState* state);
    Xil_signed32*       getPhotoYCbArray(XilSystemState* state);
    Xil_signed32*       getPhotoYCrArray(XilSystemState* state);
    Xil_signed32*       getPhotoCbArray(XilSystemState* state);
    Xil_signed32*       getPhotoCrArray(XilSystemState* state);
    
    //
    //  Data for the tables.
    //
    Xil_unsigned8*      addClampArray;
    XilMutex            addClampArrayMutex;
    
    Xil_unsigned8*      subtractClampArray;
    XilMutex            subtractClampArrayMutex;

    Xil_float32*        byteToFloatArray;
    XilMutex            byteToFloatArrayMutex;

    Xil_unsigned32*     squaresTable;
    XilMutex            squaresTableMutex;
    
    Xil_float32*        squaresOver4Table;
    XilMutex            squaresOver4TableMutex;

    XiliNearestIndexSelector* nearestIndexSelector;
    XilOp*                    nearestIndexSelectorOwner;
    XilMutex                  nearestIndexSelectorMutex;

    Xil_signed32*       photoYArray;
    XilMutex            photoYArrayMutex;

    Xil_signed32*       photoYCbArray;
    XilMutex            photoYCbArrayMutex;

    Xil_signed32*       photoYCrArray;
    XilMutex            photoYCrArrayMutex;

    Xil_signed32*       photoCbArray;
    XilMutex            photoCbArrayMutex;

    Xil_signed32*       photoCrArray;
    XilMutex            photoCrArrayMutex;

    XilMutex            cmapGenMutex;

#define BLEND_PRECISION

#ifdef BLEND_PRECISION
#define BLEND_TYPE      Xil_unsigned16
#else
#define BLEND_TYPE      Xil_unsigned8
#endif
    
    typedef BLEND_TYPE (*BlendArray)[256];

    BlendArray      blendTable;
    XilMutex        blendTableMutex;

    const unsigned int _XIL_BLEND_FRAC_BITS;
    const float        _XIL_BLEND_MULTIPLIER;
    const float        _XIL_BLEND_NORM_FACTOR;
    const unsigned int _XIL_BLEND_SCALED_HALF;

    //
    //  3-Band Paint Cache -- n brushes
    //
#define _XILI_NUM_PAINT_BRUSHES       4

    XilVersion          pcacheKernelVersion[_XILI_NUM_PAINT_BRUSHES];
    float               pcacheBrushColor[_XILI_NUM_PAINT_BRUSHES][3];
    float*              pcacheBrush[_XILI_NUM_PAINT_BRUSHES];
    unsigned int        pcacheRefCnts[_XILI_NUM_PAINT_BRUSHES];
    XilMutex            pcacheMutex;

    //
    //  ErrorDiffusion 3band->1band BYTE Lookup Table Caches
    //
#define _XILI_NUM_ERROR_DIFFUSION_LUTS 4
    XilVersion          edcacheCmapVersion[_XILI_NUM_ERROR_DIFFUSION_LUTS];
    int*                edcacheTable[_XILI_NUM_ERROR_DIFFUSION_LUTS];
    unsigned int        edcacheRefCnts[_XILI_NUM_ERROR_DIFFUSION_LUTS];
    XilMutex            edcacheMutex;

    //
    //  OrderedDither8 Dither Lookup Table Caches
    //
#define _XILI_NUM_ORDERED_DITHER_LUTS 4

    XilVersion          odcacheCmapVersion[_XILI_NUM_ORDERED_DITHER_LUTS];
    XilVersion          odcacheDmaskVersion[_XILI_NUM_ORDERED_DITHER_LUTS];
    XiliOrderedDitherLut* odcacheDitherLut[_XILI_NUM_ORDERED_DITHER_LUTS];
    unsigned int        odcacheRefCnts[_XILI_NUM_ORDERED_DITHER_LUTS];
    XilMutex            odcacheMutex;

    //
    //  Rescale table caches...each table caches a single band of a rescale
    //  operation.
    //
#define _XILI_NUM_RESCALE_TABLES      32
    struct XilRescaleArrayDesc {
	Xil_float32 multConst;
	Xil_float32 addConst;
    };

    Xil_unsigned8*      rescaleCache[_XILI_NUM_RESCALE_TABLES];
    XilRescaleArrayDesc rescaleCacheInfo[_XILI_NUM_RESCALE_TABLES];
    unsigned int        rescaleRefCnts[_XILI_NUM_RESCALE_TABLES];
    XilMutex            rescaleCacheMutex;

    int*                getRescaleTables(XilSystemState* state,
                                         float*          mult_const,
                                         float*          add_const,
                                         unsigned int    nbands);

    void                releaseRescaleTables(int*         tables,
                                             unsigned int nbands);
#if 0
    //
    //  Nearest color table routines.
    //
    //  TODO:  9/11/96 jlf  Implement these prior to FCS per oconnor's TODO.
    //
    Xil_unsigned32   getNearestColorGrid(XilSystemState*  state,
                                         XilLookupSingle* cmap,
					 Xil_unsigned8*   color_grid);
    void             freeNearestColorGrid(int index);

#define _XIL_MAX_NEAREST_COLOR_GRIDS 10
    XilMutex            nearestColorGridMutex;
    Xil_unsigned8*      nearestColorGrid[_XIL_MAX_NEAREST_COLOR_GRIDS];
    XilVersionNumber    nearestColorGridVersion[_XIL_MAX_NEAREST_COLOR_GRIDS];
    XilReadWrite        nearestColorGridReadWriteLocks[_XIL_MAX_NEAREST_COLOR_GRIDS];
#endif

};
#endif
