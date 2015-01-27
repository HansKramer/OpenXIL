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
//  File:	XilOpSubsample.hh
//  Project:	XIL
//  Revision:	1.3
//  Last Mod:	10:20:42, 03/10/00
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
#pragma ident	"@(#)XilOpSubsample.hh	1.3\t00/03/10  "

#include <xil/xilGPI.hh>
#include "XiliOpUtils.hh"

class XilOpSubsample : public XilOp {
public:
    //
    //  Produces an intersected roi that includes a mapping of
    //  source roi onto the destination.
    //
    virtual XilStatus    generateIntersectedRoi();

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
    // Backward map a single point
    //
    virtual XilStatus    vBackwardMap(XilBox*       dst_box,
                                      double        dx,
                                      double        dy,
                                      XilBox*       src_box,
                                      double*       sx,
                                      double*       sy,
                                      unsigned int  src_number);

protected:
                         XilOpSubsample(XilOpNumber            op_num,
                                        XilImage*              src_image,
                                        XilImage*              dst_image,
                                        float                  xscale,
                                        float                  yscale) :
                             XilOp(op_num)
    {
        xScale    = xscale;
        yScale    = yscale;
        xInvScale = 1.0/xscale;
        yInvScale = 1.0/yscale;

        srcImage  = src_image;
        dstImage  = dst_image;

        //
        //  Store their origins.
        //
        src_ox = _XILI_ROUND(srcImage->getOriginX());
        src_oy = _XILI_ROUND(srcImage->getOriginY());
        dst_ox = _XILI_ROUND(dstImage->getOriginX());
        dst_oy = _XILI_ROUND(dstImage->getOriginY());

        //
        //  Source in global and object space for simple reference later. 
        //
        unsigned int src_width  = src_image->getWidth();
        unsigned int src_height = src_image->getHeight();
        
        srcGSRect.setX1(-src_ox);
        srcGSRect.setY1(-src_oy);
        srcGSRect.setX2(srcGSRect.getX1() + src_width - 1);
        srcGSRect.setY2(srcGSRect.getY1() + src_height - 1);

        srcOSRect.set(0, 0, src_width - 1, src_height - 1);
    }

    virtual              ~XilOpSubsample()
    {
    }

    float                xScale;
    float                yScale;

    double               xInvScale;
    double               yInvScale;

    XiliRectInt          srcGSRect;
    XiliRectInt          srcOSRect;

    //
    //  Often-utilized image information.
    //
    //  Source and destination image of the operation.
    //
    XilImage*            srcImage;
    XilImage*            dstImage;

    //
    //  Image origins.
    //
    int                  src_ox;
    int                  src_oy;
    int                  dst_ox;
    int                  dst_oy;

};
