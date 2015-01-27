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
//  File:	XilOpAreaKernel.hh
//  Project:	XIL
//  Revision:	1.21
//  Last Mod:	10:20:37, 03/10/00
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
#pragma ident	"@(#)XilOpAreaKernel.hh	1.21\t00/03/10  "

#ifndef _XIL_OP_AREA_KERNEL_HH
#define _XIL_OP_AREA_KERNEL_HH

#include "XilOpArea.hh"

class XilOpAreaKernel : public XilOpArea {
protected:
    //
    // Constructors and desctructors
    //
    XilOpAreaKernel(XilOpNumber op_number);
    virtual ~XilOpAreaKernel();
   
    //
    //  Over-riding generateIntersectedRoi to catch the case where the
    //  source is too small from the kernel and the edge condition is
    //  EDGE_NO_WRITE. In this case generateIntersectedRoi should return
    //  XIL_FAILURE so that the operation does not proceed. Otherwise,
    //  call the default generateIntersectdRoi().
    //
    XilStatus     generateIntersectedRoi();

    //    
    //  Over-riding setBoxStorage map to set the source storage area to
    //  be bigger than the pixel area.
    //
    virtual XilStatus setBoxStorage(XiliRect*            rect,
                                    XilDeferrableObject* object,
                                    XilBox*              box);

    //
    //  The region written in the destination is shrunk due to an absence of
    //  data in the source.  We need to overload gsFowardMap() for this
    //  reason.
    //
    virtual XilStatus gsForwardMap(XiliRect*     src_rect,
                                   unsigned int  src_number,
                                   XiliRect*     dst_rect);

    //
    // The area methods need to over-ride splitOnTileBoundaries
    // to add boxes for the edge of the tiles.
    //
    virtual XilStatus vSplitOnTileBoundaries(XilBoxList* bl);

    //
    // This method is used by the sub-classes to set up
    // information about the Kernel or Sel in use. This allows
    // us to use the same splitting code for erode, dilate and
    // convolve
    //
    void initializeAreaKernel(unsigned int     k_w,
			      unsigned int     k_h,
			      unsigned int     key_x,
			      unsigned int     key_y,
			      XilEdgeCondition condition,
                              Xil_boolean      small_source);

private:
    //
    // Given a box and image tells you what type of box
    // it is within that image.
    //
    Xil_boolean     getValidBoxAreaType(XilImage*         image,
                                        int               x1,
                                        int               y1,
                                        int               x2,
                                        int               y2,
                                        XilBoxAreaType*   type);
    
    //
    // Setup a new box
    //
    XilStatus       setupBoxes(XilBox*      src_box,
			       int          src_x1,
			       int          src_y1,
			       int          src_x2,
			       int          src_y2,
			       int          src_sx1,
			       int          src_sy1,
			       int          src_sx2,
			       int          src_sy2,
			       unsigned int src_band,
			       XilBox*      dst_box,
			       int          dst_x1,
			       int          dst_y1,
			       int          dst_x2,
			       int          dst_y2,
			       int          dst_sx1,
			       int          dst_sy1,
			       int          dst_sx2,
			       int          dst_sy2,
			       unsigned int dst_band);

    //
    // Extend the storage based on the type of box
    //
    void           extendSourceStorageBox(XilBoxAreaType   type,
					  int*              sx1,
					  int*              sy1,
					  int*              sx2,
					  int*              sy2);
    //
    // Clip the source box to make sure storage doesn't go
    // negative, after setting things up.
    //
    Xil_boolean    clipSourceStorageBox(int* x1,
					int* y1,
					int* x2,
					int* y2,
					int* dx1,
					int* dy1,
					int *dx2,
					int* dy2);

    //
    // Is the box passed an edge box that will be passed
    // on to the compute routines.
    //
    Xil_boolean    edgeBox(XilBoxAreaType type);

    //
    // Does the box list entry need to be split
    //
    Xil_boolean    needsSplitting(unsigned int src_x1,
				  unsigned int src_y1,
				  unsigned int src_x2,
				  unsigned int src_y2,
				  unsigned int dst_sx1,
				  unsigned int dst_sy1,
				  unsigned int dst_sx2,
				  unsigned int dst_sy2);


#ifdef BOX_DEBUG    
    void           printBoxType(XilBoxAreaType type);
#endif

    //
    // Source and destination images
    //
    XilImage*       source;
    XilImage*       dest;
    
    //
    // Kernel dimensions
    //
    unsigned int    k_width;
    unsigned int    k_height;
    unsigned int    key_x;
    unsigned int    key_y;
    unsigned int    r_border;    // Right border
    unsigned int    l_border;    // Left border
    unsigned int    t_border;    // Top border
    unsigned int    b_border;    // Bottom border

    XilEdgeCondition  edge_condition;

    //
    //  Mark the case where the source is too small to provide enough
    //  data for the kernel.
    //
    Xil_boolean notEnoughSourceToProcess;

    //
    //  Available source image area.
    //
    XiliRectInt     srcAreaRect;

    //
    // Base storage co-ordinates
    //
    int base_src_sx1;
    int base_src_sy1;
    int base_src_sx2;
    int base_src_sy2;
};

#endif // _XIL_OP_AREA_KERNEL_HH

