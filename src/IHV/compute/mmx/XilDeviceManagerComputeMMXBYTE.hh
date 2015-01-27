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
//  File:	XilDeviceManagerComputeMMXBYTE.hh
//  Project:	XIL
//  Revision:	1.70
//  Last Mod:	16:58:15, 05/08/97
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
#pragma ident	"@(#)XilDeviceManagerComputeMMXBYTE.hh	1.70\t97/05/08  "

#ifndef _XIL_DEVICE_MANAGER_COMPUTE_MMXBYTE_HH
#define _XIL_DEVICE_MANAGER_COMPUTE_MMXBYTE_HH

#include <xil/xilGPI.hh>

//
// Forward Declarations
//
class ComputeInfoMMXBYTE;

class XilDeviceManagerComputeMMXBYTE : public XilDeviceManagerCompute
{
public:
    //
    //  Constructor/Destructor
    //
                   XilDeviceManagerComputeMMXBYTE();
                   ~XilDeviceManagerComputeMMXBYTE();

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

    XilStatus       ColorConvert(XilOp*       op,
				 unsigned     op_count,
				 XilRoi*      roi,
				 XilBoxList*  bl);
    
    XilStatus       ConvolvePreprocess(XilOp*        op,
                                       unsigned int  op_count,
                                       XilRoi*       roi,
                                       void**        compute_data,
                                       unsigned int* func_ident);

    XilStatus       ConvolvePostprocess(XilOp* op,
                                        void*  comoute_data);

    XilStatus       Convolve(XilOp*       op,
			     unsigned     op_count,
			     XilRoi*      roi,
			     XilBoxList*  bl);
    
    XilStatus       Convolve_SeparablePreprocess(XilOp*        op,
                                                 unsigned int  op_count,
                                                 XilRoi*       roi,
                                                 void**        compute_data,
                                                 unsigned int* func_ident);

    XilStatus       Convolve_SeparablePostprocess(XilOp*        op,
                                                  void*         compute_data);

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

    XilStatus       Extrema(XilOp*       op,
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

    //
    //  Squeeze Range
    //
    XilStatus       SqueezeRange(XilOp*       op,
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
    //
    //  Passing ci by reference because it's easier to read, etc.
    //
    void            tableRescale(ComputeInfoMMXBYTE& ci,
                                 int*             tables);

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
};
#endif
