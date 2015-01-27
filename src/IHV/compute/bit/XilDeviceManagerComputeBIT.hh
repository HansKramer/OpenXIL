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
//  File:	XilDeviceManagerComputeBIT.hh
//  Project:	XIL
//  Revision:	1.43
//  Last Mod:	10:22:19, 03/10/00
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
#pragma ident	"@(#)XilDeviceManagerComputeBIT.hh	1.43\t00/03/10  "

#ifndef _XIL_DEVICE_MANAGER_COMPUTE_BIT_HH
#define _XIL_DEVICE_MANAGER_COMPUTE_BIT_HH

#include <xil/xilGPI.hh>
#include "xili_geom_utils.hh"

//
// Forward Declarations
//
class ComputeInfoBIT;


class XilDeviceManagerComputeBIT : public XilDeviceManagerCompute
{
public:
    //
    //  Constructor/Destructor
    //
                    XilDeviceManagerComputeBIT();
                    ~XilDeviceManagerComputeBIT();

    //
    //  Required Virtual Functions
    //
    const char*     getDeviceName();
    XilStatus       describeMembers();
    
    //
    //  Compute Routines
    //
    XilStatus       AddConst(XilOp*       op,
                             unsigned     op_count,
                             XilRoi*      roi,
                             XilBoxList*  bl);
    
    XilStatus       AndConst(XilOp*       op,
                             unsigned     op_count,
                             XilRoi*      roi,
                             XilBoxList*  bl);

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

    XilStatus       AndMinMultiply(XilOp*       op,
                                   unsigned     op_count,
                                   XilRoi*      roi,
                                   XilBoxList*  bl);
    
    XilStatus       BandCombine(XilOp*       op,
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
    
    XilStatus       CastTo8(XilOp*       op,
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
    
    XilStatus       Convolve(XilOp*       op,
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
    
    XilStatus       DivideIntoConst(XilOp*       op,
                                    unsigned     op_count,
                                    XilRoi*      roi,
                                    XilBoxList*  bl);
    
    XilStatus       DivideByConst(XilOp*       op,
                                  unsigned     op_count,
                                  XilRoi*      roi,
                                  XilBoxList*  bl);
    
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
    
    XilStatus       MultiplyConst(XilOp*       op,
                                  unsigned     op_count,
                                  XilRoi*      roi,
                                  XilBoxList*  bl);
    
    XilStatus       LookupTo1(XilOp*       op,
                              unsigned     op_count,
                              XilRoi*      roi,
                              XilBoxList*  bl);
    
    XilStatus       LookupTo8(XilOp*       op,
                              unsigned     op_count,
                              XilRoi*      roi,
                              XilBoxList*  bl);
    
    XilStatus       LookupTo16(XilOp*       op,
			       unsigned     op_count,
			       XilRoi*      roi,
			       XilBoxList*  bl);
    
    XilStatus       LookupTof32(XilOp*       op,
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
    
    XilStatus       LookupCombined16(XilOp*       op,
				     unsigned     op_count,
				     XilRoi*      roi,
				     XilBoxList*  bl);
    
    XilStatus       LookupCombinedf32(XilOp*       op,
				      unsigned     op_count,
				      XilRoi*      roi,
				      XilBoxList*  bl);
    
    XilStatus      NearestColor1(XilOp*       op,
				 unsigned     op_count,
				 XilRoi*      roi,
				 XilBoxList*  bl);
    
    XilStatus      NearestColor8(XilOp*       op,
				 unsigned     op_count,
				 XilRoi*      roi,
				 XilBoxList*  bl);
    XilStatus      NearestColor8Preprocess(XilOp*        op,
					   unsigned int  op_count,
					   XilRoi*       roi,
					   void**        compute_data,
                                           unsigned int* func_ident);
    XilStatus      NearestColor8Postprocess(XilOp*       op,
					    void*        compute_data);

    XilStatus      NearestColor16(XilOp*       op,
				  unsigned     op_count,
				  XilRoi*      roi,
				  XilBoxList*  bl);
    XilStatus      NearestColor16Preprocess(XilOp*        op,
                                            unsigned int  op_count,
                                            XilRoi*       roi,
                                            void**        compute_data,
                                            unsigned int* func_ident);
    XilStatus      NearestColor16Postprocess(XilOp*       op,
					     void*        compute_data);

    XilStatus       Not(XilOp*       op,
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

    XilStatus       OrderedDither16(XilOp*       op,
				    unsigned     op_count,
				    XilRoi*      roi,
				    XilBoxList*  bl);
    
    XilStatus       OrMaxAdd(XilOp*       op,
                             unsigned     op_count,
                             XilRoi*      roi,
                             XilBoxList*  bl);
    
    XilStatus       Paint(XilOp*       op,
                          unsigned     op_count,
                          XilRoi*      roi,
                          XilBoxList*  bl);
    
    XilStatus       Rescale(XilOp*       op,
                            unsigned     op_count,
                            XilRoi*      roi,
                            XilBoxList*  bl);
    
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

    XilStatus       SetValue(XilOp*       op,
                             unsigned     op_count,
                             XilRoi*      roi,
                             XilBoxList*  bl);

    XilStatus       SoftFill(XilOp*       op,
                             unsigned     op_count,
                             XilRoi*      roi,
                             XilBoxList*  bl);

    XilStatus       SqueezeRange(XilOp*       op,
                                 unsigned     op_count,
                                 XilRoi*      roi,
                                 XilBoxList*  bl);

    XilStatus       SubsampleAdaptive(XilOp*       op,
				      unsigned     op_count,
				      XilRoi*      roi,
				      XilBoxList*  bl);	
    
    XilStatus       Subtract(XilOp*       op,
                             unsigned     op_count,
                             XilRoi*      roi,
                             XilBoxList*  bl);
    
    XilStatus       SubtractConst(XilOp*       op,
                                  unsigned     op_count,
                                  XilRoi*      roi,
                                  XilBoxList*  bl);
    
    XilStatus       SubtractFromConst(XilOp*       op,
                                      unsigned     op_count,
                                      XilRoi*      roi,
                                      XilBoxList*  bl);
    
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
    
    
private:
    XilStatus        affineNearest(XilBoxList* bl,
				   AffineData  ad);

    XilStatus        affineBilinear(XilBoxList* bl,
				    AffineData  ad);

    XilStatus        affineBicubic(XilBoxList* bl,
				   AffineData  ad);

    XilStatus        affineGeneral(XilBoxList* bl,
				   AffineData  ad);

    XilStatus        scale_2x_BL(XilBoxList* bl,
				 AffineData  ad);
    
    XilStatus        scale_05x_BL(XilBoxList* bl,
				  AffineData  ad);

    int *            xili_make_BL_table(void);
    int *            xili_make_BC_table(void);

    //
    //  Persistent Data Retrieval
    //
    Xil_unsigned8*	getByteCastArray();
    Xil_unsigned8*	byteCastArray;
    XilMutex		byteCastArrayMutex;

    Xil_signed16*	getShortCastArray();
    Xil_signed16*	shortCastArray;
    XilMutex		shortCastArrayMutex;

    Xil_float32*	getFloatCastArray();
    Xil_float32*	floatCastArray;
    XilMutex		floatCastArrayMutex;

    Xil_unsigned8**     getEdgeDetectTable();
    Xil_unsigned8**	edgeDetectTable;
    XilMutex		edgeDetectTableMutex;

    // Called by MultiplyConst and DivideByConst
    XilStatus           multiply_const(ComputeInfoBIT* ci, float* values);

    Xil_unsigned32 * getSquaresTable();
    Xil_unsigned32 * squaresTable;
    XilMutex         squaresTableMutex;

    XilStatus              getCastByteTable();
    struct XilCastTable8 {
	Xil_unsigned8 b[8];
    };
    struct XilCastTable8 * castByteTable;
    int *                  castByteUpperTable;
    int *                  castByteLowerTable;
    XilMutex               castByteTableMutex;

    XilStatus               getCastShortTable();
    struct XilCastTable16 {
	Xil_signed16 b[8];
    };
    struct XilCastTable16 * castShortTable;
    XilMutex                castShortTableMutex;

    XilStatus                getCastFloat32Table();
    struct XilCastTablef32 {
	Xil_float32 b[8];
    };
    struct XilCastTablef32 * castFloat32Table;
    XilMutex                 castFloat32TableMutex;

    //
    // Logical utilities
    //
    void                     xili_bit_and(Xil_unsigned8* src1,
					  Xil_unsigned8* src2,
					  Xil_unsigned8* dest,
					  int	         width,
					  int            src1_offset,
					  int            src2_offset,
					  int            dest_offset);

    void                     xili_bit_or(Xil_unsigned8* src1,
					 Xil_unsigned8* src2,
					 Xil_unsigned8* dest,
					 int	        width,
					 int            src1_offset,
					 int            src2_offset,
					 int            dest_offset);

    void                     xili_bit_xor(Xil_unsigned8* src1,
					  Xil_unsigned8* src2,
					  Xil_unsigned8* dest,
					  int	         width,
					  int            src1_offset,
					  int            src2_offset,
					  int            dest_offset);

    void                     xili_bit_subtract(Xil_unsigned8* src1,
                                               Xil_unsigned8* src2,
                                               Xil_unsigned8* dest,
                                               int	          width,
                                               int            src1_offset,
                                               int            src2_offset,
                                               int            dest_offset);

#ifdef XIL_LITTLE_ENDIAN    

    //
    // inline used by the little endian logical code
    //
    inline
    Xil_unsigned32           unscramble(Xil_unsigned32 word) {
	                         Xil_unsigned32 rval;

				 rval = (word << 24) | ((word & 0xff00) << 8) | 
				     ((word >> 8) & 0xff00) | (word >> 24);
				 return rval;;
                             }
#endif // XIL_LITTLE_ENDIAN    
};

#endif // _XIL_DEVICE_MANAGER_COMPUTE_BIT_HH
