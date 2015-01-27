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
//  File:	XilDeviceManagerComputeSHORT.hh
//  Project:	XIL
//  Revision:	1.43
//  Last Mod:	10:22:22, 03/10/00
//
//  Description:
//	
//	
//	
//	
//	COPYRIGHT
//------------------------------------------------------------------------
#pragma ident	"@(#)XilDeviceManagerComputeSHORT.hh	1.43\t00/03/10  "

#ifndef _XIL_DEVICE_MANAGER_COMPUTE_SHORT_HH
#define _XIL_DEVICE_MANAGER_COMPUTE_SHORT_HH

#include <xil/xilGPI.hh>
#include "xili_geom_utils.hh"
//
// Forward Declarations
//
class ComputeInfoSHORT;


class XilDeviceManagerComputeSHORT : public XilDeviceManagerCompute
{
public:
    //
    //  Constructor/Destructor
    //
                   XilDeviceManagerComputeSHORT();
                   ~XilDeviceManagerComputeSHORT();

    //
    //  Required Virtual Functions
    //
    const char*    getDeviceName();
    XilStatus      describeMembers();
    
    //
    //  Compute Routines
    //
    XilStatus      Absolute(XilOp*       op,
                            unsigned     op_count,
                            XilRoi*      roi,
                            XilBoxList*  bl);

    XilStatus      Add(XilOp*       op,
                       unsigned     op_count,
                       XilRoi*      roi,
                       XilBoxList*  bl);

    XilStatus      AddConst(XilOp*       op,
                            unsigned     op_count,
                            XilRoi*      roi,
                            XilBoxList*  bl);

    XilStatus      AffineNearest(XilOp*       op,
				 unsigned     op_count,
				 XilRoi*      roi,
				 XilBoxList*  bl);

    XilStatus      AffineBilinear(XilOp*       op,
				  unsigned     op_count,
				  XilRoi*      roi,
				  XilBoxList*  bl);
    
    XilStatus      AffineBicubic(XilOp*       op,
				 unsigned     op_count,
				 XilRoi*      roi,
				 XilBoxList*  bl);

    XilStatus      AffineGeneral(XilOp*       op,
				 unsigned     op_count,
				 XilRoi*      roi,
				 XilBoxList*  bl);
            
    XilStatus      And(XilOp*       op,
                       unsigned     op_count,
                       XilRoi*      roi,
                       XilBoxList*  bl);

    XilStatus      AndConst(XilOp*       op,
                            unsigned     op_count,
                            XilRoi*      roi,
                            XilBoxList*  bl);

    XilStatus      BandCombine(XilOp*       op,
                               unsigned     op_count,
                               XilRoi*      roi,
                               XilBoxList*  bl);

    XilStatus      BlackGeneration(XilOp*       op,
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
    
    XilStatus       CastTo8(XilOp*       op,
			     unsigned     op_count,
			     XilRoi*      roi,
			     XilBoxList*  bl);
    
    XilStatus       CastTof32(XilOp*       op,
			     unsigned     op_count,
			     XilRoi*      roi,
			     XilBoxList*  bl);
    
    XilStatus      Convolve(XilOp*       op,
			    unsigned     op_count,
			    XilRoi*      roi,
			    XilBoxList*  bl);
    
    XilStatus      Convolve_SeparablePreprocess(XilOp*        op,
                                                unsigned int  op_count,
                                                XilRoi*       roi,
                                                void**        compute_data,
                                                unsigned int* func_ident);
    XilStatus      Convolve_Separable(XilOp*       op,
			              unsigned     op_count,
			              XilRoi*      roi,
			              XilBoxList*  bl);

    XilStatus      ColorConvert(XilOp*       op,
                                unsigned     op_count,
                                XilRoi*      roi,
                                XilBoxList*  bl);

    XilStatus      Copy(XilOp*       op,
                        unsigned     op_count,
                        XilRoi*      roi,
                        XilBoxList*  bl);

    XilStatus      CopyWithPlanemask(XilOp*       op,
                                     unsigned     op_count,
                                     XilRoi*      roi,
                                     XilBoxList*  bl);

    XilStatus      Dilate(XilOp*       op,
                          unsigned     op_count,
                          XilRoi*      roi,
                          XilBoxList*  bl);

    XilStatus      Divide(XilOp*       op,
                          unsigned     op_count,
                          XilRoi*      roi,
                          XilBoxList*  bl);

    //
    //  DivideByConst
    //
    XilStatus      DivideByConst(XilOp*       op,
                                 unsigned     op_count,
                                 XilRoi*      roi,
                                 XilBoxList*  bl);
    XilStatus      DivideByConstPreprocess(XilOp*        op,
                                           unsigned int  op_count,
                                           XilRoi*       roi,
                                           void**        compute_data,
                                           unsigned int* func_ident);
    XilStatus      DivideByConstPostprocess(XilOp*       op,
                                            void*        compute_data);

    //
    //  DivideIntoConst
    //
    XilStatus      DivideIntoConst(XilOp*       op,
                                   unsigned     op_count,
                                   XilRoi*      roi,
                                   XilBoxList*  bl);
#ifndef _XIL_USE_INTMULDIV
    XilStatus      DivideIntoConstPreprocess(XilOp*        op,
                                             unsigned int  op_count,
                                             XilRoi*       roi,
                                             void**        compute_data,
                                             unsigned int* func_ident);
    
    XilStatus      DivideIntoConstPostprocess(XilOp*       op,
                                              void*        compute_data);
#endif // _XIL_USE_INTMULDIV

    XilStatus      EdgeDetection(XilOp*       op,
				 unsigned     op_count,
				 XilRoi*      roi,
				 XilBoxList*  bl);
    
    XilStatus      Erode(XilOp*       op,
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

    XilStatus       ErrorDiffusion16(XilOp*       op,
				     unsigned     op_count,
				     XilRoi*      roi,
				     XilBoxList*  bl);

    XilStatus	    Extrema(XilOp*       op,
			    unsigned     op_count,
			    XilRoi*      roi,
			    XilBoxList*  bl);

    XilStatus	    Fill(XilOp*       op,
			 unsigned     op_count,
			 XilRoi*      roi,
			 XilBoxList*  bl);
    
    XilStatus	    Histogram(XilOp*       op,
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
    
    XilStatus       Lookup16(XilOp*       op,
			     unsigned     op_count,
			     XilRoi*      roi,
			     XilBoxList*  bl);
    
    XilStatus       Lookupf32(XilOp*       op,
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

    XilStatus      Max(XilOp*       op,
                       unsigned     op_count,
                       XilRoi*      roi,
                       XilBoxList*  bl);

    XilStatus      Min(XilOp*       op,
                       unsigned     op_count,
                       XilRoi*      roi,
                       XilBoxList*  bl);

    XilStatus      Multiply(XilOp*       op,
                            unsigned     op_count,
                            XilRoi*      roi,
                            XilBoxList*  bl);

    XilStatus      MultiplyConst(XilOp*       op,
                                 unsigned     op_count,
                                 XilRoi*      roi,
                                 XilBoxList*  bl);
    XilStatus      MultiplyConstPreprocess(XilOp*        op,
                                           unsigned int  op_count,
                                           XilRoi*       roi,
                                           void**        compute_data,
                                           unsigned int* func_ident);
    XilStatus      MultiplyConstPostprocess(XilOp* op,
                                            void*  compute_data);

    XilStatus      NearestColor1(XilOp*       op,
				 unsigned     op_count,
				 XilRoi*      roi,
				 XilBoxList*  bl);
    
    XilStatus      NearestColor8(XilOp*       op,
				 unsigned     op_count,
				 XilRoi*      roi,
				 XilBoxList*  bl);

    XilStatus      NearestColor16(XilOp*       op,
				  unsigned     op_count,
				  XilRoi*      roi,
				  XilBoxList*  bl);

    XilStatus      Not(XilOp*       op,
                       unsigned     op_count,
                       XilRoi*      roi,
                       XilBoxList*  bl);

    XilStatus      Or(XilOp*       op,
                      unsigned     op_count,
                      XilRoi*      roi,
                      XilBoxList*  bl);

    XilStatus      OrConst(XilOp*       op,
                           unsigned     op_count,
                           XilRoi*      roi,
                           XilBoxList*  bl);
    
    XilStatus      OrderedDither1(XilOp*       op,
				  unsigned     op_count,
				  XilRoi*      roi,
				  XilBoxList*  bl);

    XilStatus      OrderedDither8(XilOp*       op,
				  unsigned     op_count,
				  XilRoi*      roi,
				  XilBoxList*  bl);

    XilStatus      OrderedDither16(XilOp*       op,
				   unsigned     op_count,
				   XilRoi*      roi,
				   XilBoxList*  bl);
   
    XilStatus      Paint(XilOp*       op,
                         unsigned     op_count,
                         XilRoi*      roi,
                         XilBoxList*  bl);
    
    XilStatus      Rescale(XilOp*       op,
                           unsigned     op_count,
                           XilRoi*      roi,
                           XilBoxList*  bl);
    XilStatus      RescalePreprocess(XilOp*        op,
                                     unsigned int  op_count,
                                     XilRoi*       roi,
                                     void**        compute_data,
                                     unsigned int* func_ident);
    XilStatus      RescalePostprocess(XilOp*       op,
                                      void*        compute_data);


    XilStatus      RotateNearest(XilOp*       op,
				 unsigned     op_count,
				 XilRoi*      roi,
				 XilBoxList*  bl);

    XilStatus      RotateBilinear(XilOp*       op,
				  unsigned     op_count,
				  XilRoi*      roi,
				  XilBoxList*  bl);

    XilStatus      RotateBicubic(XilOp*  op,
				 unsigned     op_count,
				 XilRoi*      roi,
				 XilBoxList*  bl);

    XilStatus      RotateGeneral(XilOp*  op,
				 unsigned     op_count,
				 XilRoi*      roi,
				 XilBoxList*  bl);

    XilStatus      ScaleNearest(XilOp*       op,
				unsigned     op_count,
				XilRoi*      roi,
				XilBoxList*  bl);

    XilStatus      ScaleBilinear(XilOp*       op,
				 unsigned     op_count,
				 XilRoi*      roi,
				 XilBoxList*  bl);

    XilStatus      ScaleBicubic(XilOp*  op,
				unsigned     op_count,
				XilRoi*      roi,
				XilBoxList*  bl);

    XilStatus      ScaleGeneral(XilOp*  op,
				unsigned     op_count,
				XilRoi*      roi,
				XilBoxList*  bl);

    XilStatus       SetValue(XilOp*       op,
                             unsigned     op_count,
                             XilRoi*      roi,
                             XilBoxList*  bl);

    XilStatus	    SoftFill(XilOp*       op,
			     unsigned     op_count,
			     XilRoi*      roi,
			     XilBoxList*  bl);

    XilStatus	    SqueezeRange(XilOp*       op,
				 unsigned     op_count,
				 XilRoi*      roi,
				 XilBoxList*  bl);

    XilStatus       SubsampleAdaptive(XilOp*       op,
				      unsigned     op_count,
				      XilRoi*      roi,
				      XilBoxList*  bl);	

    XilStatus      Subtract(XilOp*       op,
                            unsigned     op_count,
                            XilRoi*      roi,
                            XilBoxList*  bl);

    XilStatus      SubtractConst(XilOp*       op,
                                 unsigned     op_count,
                                 XilRoi*      roi,
                                 XilBoxList*  bl);

    XilStatus      SubtractFromConst(XilOp*       op,
                                     unsigned     op_count,
                                     XilRoi*      roi,
                                     XilBoxList*  bl);

    XilStatus      TablewarpNearest(XilOp*       op,
				    unsigned     op_count,
				    XilRoi*      roi,
				    XilBoxList*  bl);

    XilStatus      TablewarpBilinear(XilOp*       op,
				     unsigned     op_count,
				     XilRoi*      roi,
				     XilBoxList*  bl);
    
    XilStatus      TablewarpBicubic(XilOp*       op,
				    unsigned     op_count,
				    XilRoi*      roi,
				    XilBoxList*  bl);
    
    XilStatus      TablewarpGeneral(XilOp*       op,
				    unsigned     op_count,
				    XilRoi*      roi,
				    XilBoxList*  bl);

    XilStatus      TablewarpHorizontalNearest(XilOp*       op,
					      unsigned     op_count,
					      XilRoi*      roi,
					      XilBoxList*  bl);

    XilStatus      TablewarpHorizontalBilinear(XilOp*       op,
					       unsigned     op_count,
					       XilRoi*      roi,
					       XilBoxList*  bl);
    
    XilStatus      TablewarpHorizontalBicubic(XilOp*       op,
					      unsigned     op_count,
					      XilRoi*      roi,
					      XilBoxList*  bl);
    
    XilStatus      TablewarpHorizontalGeneral(XilOp*       op,
					      unsigned     op_count,
					      XilRoi*      roi,
					      XilBoxList*  bl);
    
    XilStatus      TablewarpVerticalNearest(XilOp*       op,
					    unsigned     op_count,
					    XilRoi*      roi,
					    XilBoxList*  bl);

    XilStatus      TablewarpVerticalBilinear(XilOp*       op,
					     unsigned     op_count,
					     XilRoi*      roi,
					     XilBoxList*  bl);
    
    XilStatus      TablewarpVerticalBicubic(XilOp*       op,
					    unsigned     op_count,
					    XilRoi*      roi,
					    XilBoxList*  bl);
    
    XilStatus      TablewarpVerticalGeneral(XilOp*       op,
					    unsigned     op_count,
					    XilRoi*      roi,
					    XilBoxList*  bl);
    
    XilStatus      Threshold(XilOp*       op,
			     unsigned     op_count,
			     XilRoi*      roi,
			     XilBoxList*  bl);
    XilStatus      Threshold_1BAND(XilOp*       op,
                                   unsigned     op_count,
                                   XilRoi*      roi,
                                   XilBoxList*  bl);
    XilStatus      ThresholdThreshold(XilOp*       op,
                                      unsigned     op_count,
                                      XilRoi*      roi,
                                      XilBoxList*  bl);
    
    XilStatus      TranslateNearest(XilOp*       op,
				    unsigned     op_count,
				    XilRoi*      roi,
				    XilBoxList*  bl);

    XilStatus      TranslateBilinear(XilOp*       op,
				     unsigned     op_count,
				     XilRoi*      roi,
				     XilBoxList*  bl);
    
    XilStatus      TranslateBicubic(XilOp*       op,
				    unsigned     op_count,
				    XilRoi*      roi,
				    XilBoxList*  bl);
    
    XilStatus      TranslateGeneral(XilOp*       op,
				    unsigned     op_count,
				    XilRoi*      roi,
				    XilBoxList*  bl);

    XilStatus      Transpose(XilOp*       op,
                             unsigned     op_count,
                             XilRoi*      roi,
                             XilBoxList*  bl);

    XilStatus      Unsharp(XilOp*       op,
                           unsigned     op_count,
                           XilRoi*      roi,
                           XilBoxList*  bl);

    XilStatus      UnsharpIC(XilOp*       op,
                           unsigned     op_count,
                           XilRoi*      roi,
                           XilBoxList*  bl);

    XilStatus      Xor(XilOp*       op,
                       unsigned     op_count,
                       XilRoi*      roi,
                       XilBoxList*  bl);

    XilStatus      XorConst(XilOp*       op,
                            unsigned     op_count,
                            XilRoi*      roi,
                            XilBoxList*  bl);

private:
    //
    //  Passing ci by reference because it's easier to read, etc.
    //
    void            tableRescale(ComputeInfoSHORT& ci,
                                 int*              tables);
    
    XilStatus       affineNearest(XilBoxList* bl,
                                  AffineData  ad);

    XilStatus       affineBilinear(XilBoxList* bl,
                                   AffineData  ad);

    XilStatus       affineBicubic(XilBoxList* bl,
                                  AffineData  ad);

    XilStatus       affineGeneral(XilBoxList* bl,
                                  AffineData  ad);
    
    XilStatus       scale_2x_BL(XilBoxList* bl,
                                AffineData  ad);
    
    XilStatus       scale_05x_BL(XilBoxList* bl,
                                 AffineData  ad);

    //
    //  Rescale table caches...each table caches a single band of a rescale
    //  operation.
    //
#define _XILI_NUM_RESCALE_TABLES      32
    struct XilRescaleArrayDesc {
        Xil_float32 multConst;
        Xil_float32 addConst;
    };

    Xil_signed16*       rescaleCache[_XILI_NUM_RESCALE_TABLES];
    XilRescaleArrayDesc rescaleCacheInfo[_XILI_NUM_RESCALE_TABLES];
    unsigned int        rescaleRefCnts[_XILI_NUM_RESCALE_TABLES];
    XilMutex            rescaleCacheMutex;

    int*                getRescaleTables(XilSystemState* state,
                                         float*          mult_const,
                                         float*          add_const,
                                         unsigned int    nbands);

    void                releaseRescaleTables(int*         tables,
                                             unsigned int nbands);

    //
    //  Short to Float conversion array.  Only really need if
    //  _XIL_USE_TABLE_FLT_CNV is defined.
    //
#ifdef _XIL_USE_TABLE_FLT_CNV
    Xil_float32*   getShortToFloat(XilSystemState* state);

    Xil_float32*   shortToFloatArray;
    XilMutex       shortToFloatArrayMutex;
#endif
};
#endif
