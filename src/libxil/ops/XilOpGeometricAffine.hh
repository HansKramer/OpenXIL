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
//  File:	XilOpGeometricAffine.hh
//  Project:	XIL
//  Revision:	1.44
//  Last Mod:	10:20:36, 03/10/00
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
#pragma ident	"@(#)XilOpGeometricAffine.hh	1.44\t00/03/10  "

#ifndef _XIL_OP_GEOMETRIC_AFFINE_HH
#define _XIL_OP_GEOMETRIC_AFFINE_HH

#include <xil/xilGPI.hh>

#include "XilOpGeometric.hh"
#include "XiliRect.hh"
#include "XiliList.hh"
#include "XiliConvexRegion.hh"
#include "xili_geom_utils.hh"


class XilOpGeometricAffine : public XilOpGeometric {
protected:
                         XilOpGeometricAffine(XilOpNumber            op_num,
                                              XilImage*              src_image,
                                              XilImage*              dst_image,
                                              XiliInterpolationType  type,
                                              AffineTr               affine_tr,
                                              XilInterpolationTable* h_table,
                                              XilInterpolationTable* v_table);

    virtual              ~XilOpGeometricAffine();


    //
    //  Produces an intersected roi that includes a mapping of
    //  source roi onto the destination.
    //
    virtual XilStatus    generateIntersectedRoi();


    //
    //  Special version of divideBoxList which takes into account backward and
    //  forward mapping.
    //
    //  And the fact that this called from the GPI side so boxes are in object
    //  space. 
    //
    virtual Xil_boolean  divideBoxList(XilBoxList*   boxlist,
				       unsigned int  box_number,
				       unsigned int  tile_xdelta,
				       unsigned int  tile_ydelta);

    //    
    //  Over-riding setBoxStorage map to set the source storage area and clip
    //  properly against the source image.
    //
    virtual XilStatus    setBoxStorage(XiliRect*            rect,
                                       XilDeferrableObject* object,
                                       XilBox*              box);
    
    //
    //  These calls are used by the op to backward map rects in global space.
    //
    virtual XilStatus    gsBackwardMap(XiliRect*    dst_rect,
				       XiliRect*    src_rect,
				       unsigned int src_number);

    virtual XilStatus    gsForwardMap(XiliRect*    src_rect,
				      unsigned int src_number,
                                      XiliRect*    dst_box);

    //
    // Backward map a single point
    //
    virtual XilStatus     vBackwardMap(XilBox*       dst_box,
				       double        dx,
				       double        dy,
				       XilBox*       src_box,
				       double*       sx,
				       double*       sy,
				       unsigned int  src_number);

    //
    //  For Affine these need to implemented to aquire the global space ROIs
    //  with floating point precision.
    //
    virtual XilRoi*        getSrcGlobalSpaceRoi(unsigned int src_num);
    virtual XilRoi*        getDstGlobalSpaceRoi(unsigned int dst_num = 0);
    
    //
    //  These are used by the sub-classes so they are protected.
    //

    //
    //  Store a pointer to the convex region associated with a box
    //  on the box.
    //
    XilStatus              updateBoxConvexRegion(XilBox*           box,
                                                 XiliConvexRegion* region);

    //
    //  Reorder a convex region in the clockwise direction
    //
    void                   reorder(XiliConvexRegion* cr);

private:
    //
    //  Keep a list of convex regions that are referenced by boxes we
    //  create.  We delete them when the op is destructed.
    //
    XiliSLList<XiliConvexRegion*> regionList;
    XilMutex                      regionListMutex;

    //
    //  Store the transform for the operation.
    //
    AffineTr             affineTr;
    AffineTr             inverseTr;

    //
    //  Forward map or backward map the given convex region leaving it
    //  properly ordered (i.e. clockwise).
    //
    XilStatus            forwardMapCR(XiliConvexRegion* cr);
    XilStatus            backwardMapCR(XiliConvexRegion* cr);

    double               xPixelAdj;
    double               yPixelAdj;

    //
    //  Pointer to hold the src ROI when it's non-nearest.
    //
    XilRoi*              clippedSrcRoi;

    //
    //  Pointer to hold the extended src ROI when it's a molecule.
    //
    XilRoi*              extentMoleculeSrcROI;
};

#endif // _XIL_OP_GEOMETRIC_AFFINE_HH

