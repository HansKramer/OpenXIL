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
//  File:	XilOpPaint.cc
//  Project:	XIL
//  Revision:	1.12
//  Last Mod:	10:07:30, 03/10/00
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
#pragma ident	"@(#)XilOpPaint.cc	1.12\t00/03/10  "

//
//  Standard Includes
//
#include <stdlib.h>
#include <string.h>

//
//  XIL C++ Includes
//
#include <xil/xilGPI.hh>

//
//  Local Includes
//
#include "XilOpPoint.hh"
#include "XiliUtils.hh"

class XilOpPaint : public XilOpPoint {
public:
    static XilOp*        create(char* function_name,
                                void* args[],
                                int   count);

    //
    //  Produces an intersected roi that includes a mapping of
    //  source roi onto the destination.  For paint, this is the bounding box
    //  of points in the destination.
    //
    virtual XilStatus    generateIntersectedRoi();

protected:
    XilOpPaint(XilOpNumber op_num);
    virtual ~XilOpPaint();
};

XilStatus
XilOpPaint::generateIntersectedRoi()
{
    XilRoi* intersected_roi = getIntersectedRoi();
    if(intersected_roi == NULL) {
        return XIL_FAILURE;
    }

    XilRoi* dst_roi = getDstGlobalSpaceRoi();
    if(dst_roi == NULL) {
        XIL_ERROR(getSystemState(), XIL_ERROR_INTERNAL, "di-402", FALSE);
        return XIL_FAILURE;
    }

    //
    //  Do we need to do a full intersection?
    //
    XilImage* source = (XilImage*)getSrc(1);
    if(source->getRoi() != NULL) {
        XilRoi* src_roi = getSrcGlobalSpaceRoi(0);
        if(src_roi == NULL) {
            XIL_ERROR(getSystemState(), XIL_ERROR_INTERNAL, "di-402", FALSE);
            return XIL_FAILURE;
        }

        //
        //  Intersect the dest and src[0] image and place results in
        //  intersected_roi. 
        //
        dst_roi->intersect(src_roi, intersected_roi);
    } else {
        //
        //  We can do a fast intersection with something smaller than the
        //  entire source -- based on the coordinate list.
        //
        //  Calculate the bounding box of the points we're writing in the
        //  destination (in global space) by moving the points we're reading
        //  from in the source into global space.  Also, takes care of moving
        //  the coordinate list into object space.  Start by setting the paint_gsbbox
        //  to an invalid rectangle where x1, y1 is width - 1 and height - 1
        //  respectively and x2, y2 equal to -originX and -originY respectively.
        //
        int srcox = _XILI_ROUND(source->getOriginX());
        int srcoy = _XILI_ROUND(source->getOriginY());

        XiliRectInt paint_gsbbox(source->getWidth() - 1 - srcox,
                                 source->getWidth() - 1 - srcoy,
                                 -srcox,
                                 -srcoy);

        unsigned int count;
        getParam(3, &count);
        int* tmp;
        getParam(4, (void**)&tmp);
        for(unsigned int i=0; i<count; i++) {
            int cx = *tmp++ - srcox;
            int cy = *tmp++ - srcoy;

            if(cx < paint_gsbbox.getX1()) {
                paint_gsbbox.setX1(cx);
            }
            if(cx > paint_gsbbox.getX2()) {
                paint_gsbbox.setX2(cx);
            }
            if(cy < paint_gsbbox.getY1()) {
                paint_gsbbox.setY1(cy);
            }
            if(cy > paint_gsbbox.getY2()) {
                paint_gsbbox.setY2(cy);
            }
        }

        //
        //  Grow paint_gsbbox by kernel width and kernel height.
        //
        XilKernel* brush;
        getParam(2, (XilObject**)&brush);
        paint_gsbbox.setX1(paint_gsbbox.getX1() - brush->getKeyX());
        paint_gsbbox.setY1(paint_gsbbox.getY1() - brush->getKeyY());
        paint_gsbbox.setX2(paint_gsbbox.getX2() +
                              (brush->getWidth() - brush->getKeyX()));
        paint_gsbbox.setY2(paint_gsbbox.getY2() +
                              (brush->getWidth() - brush->getKeyX()));


        //
        //  Clip paint_gsbbox to the source image.
        //
        paint_gsbbox.clip(source->getGlobalSpaceRect());

        //
        //  Construct a ROI representing the bounding box.
        //
        XilRoi src_roi(source->getSystemState());
        src_roi.addRect(paint_gsbbox.getX1(),
                        paint_gsbbox.getY1(),
                        paint_gsbbox.getX2() - paint_gsbbox.getX1() + 1,
                        paint_gsbbox.getY2() - paint_gsbbox.getY1() + 1);

        //
        //  Intersect the dest and src[0] image and place results in
        //  intersected_roi.
        //
        dst_roi->intersect(&src_roi, intersected_roi);
    }

    return XIL_SUCCESS;
}

XilOp*
XilOpPaint::create(char  function_name[],
		 void* args[],
		 int)
{
    static XilOpCache	paint_op_cache;

    float*        color      = (float*)args[2];
    XilKernel*    brush      = (XilKernel*)args[3];
    unsigned int  count      = (unsigned int) args[4];
    float*        coord_list = (float*)args[5];

    XilImage*   src   = (XilImage*)args[0];
    XilImage*   dst   = (XilImage*)args[1];
    XilOpNumber opnum;
    if((opnum = xili_verify_op_args(function_name, &paint_op_cache,
                                    dst, src)) == -1) {
	return NULL;
    }
    
    //
    //  Check for NULL parameter pointers.
    //
    if(color == NULL) {
        XIL_ERROR(dst->getSystemState(), XIL_ERROR_USER, "di-259", TRUE);
        return NULL;
    }

    if(brush == NULL) {
        XIL_ERROR(dst->getSystemState(), XIL_ERROR_USER, "di-182", TRUE);
        return NULL;
    }

    if(coord_list == NULL) {
        XIL_ERROR(dst->getSystemState(), XIL_ERROR_USER, "di-259", TRUE);
        return NULL;
    }

    //
    //  Make a copy of the parameters
    //
    //
    //  Copy and round the color array
    //
    unsigned int nBands = src->getNumBands();
    float* color_array = new float[nBands];
    if(color_array==NULL) {
	XIL_ERROR(src->getSystemState(), XIL_ERROR_RESOURCE,"di-1",TRUE);
	return NULL;
    }
    for(unsigned int i =0; i < nBands; i++) {
	color_array[i] = color[i];
    }

    //
    //  Copy and round the coordinate array
    //
    int* coord_array = (int*)xili_round_op_values(XIL_SIGNED_32,
                                                  (float*)args[5],
                                                  count * 2);
    if(coord_array == NULL) {
	XIL_ERROR(dst->getSystemState(), XIL_ERROR_RESOURCE, "di-1", TRUE);
	delete [] color_array;
	return NULL;
    }

    XilOpPaint* op = new XilOpPaint(opnum);
    if(op == NULL) {
	XIL_ERROR(dst->getSystemState(), XIL_ERROR_RESOURCE, "di-1", TRUE);
	delete [] color_array;
	delete [] coord_array;
	return NULL;
    }

    //
    //  Reference the Kernel -- actually, setup a reference to the kernel from
    //  the op to the kernel and vice versa.  If the kernel changes before the
    //  op is executed, only then will a copy be made.
    //
    XilKernel* brush_ref = (XilKernel*)(brush->aquireDefRef(op));
    if(brush_ref == NULL) {
        op->destroy();
	delete [] color_array;
	delete [] coord_array;
	XIL_ERROR(dst->getSystemState(),
                  XIL_ERROR_SYSTEM, "di-189", FALSE);
	return NULL;
    }

    //
    //  Move the coordinate list from global space into object space for the
    //  compute routine.
    //
    int srcox = _XILI_ROUND(src->getOriginX());
    int srcoy = _XILI_ROUND(src->getOriginY());

    for(i=0; i<(count<<1); i+=2) {
	coord_array[i]     += srcox;
	coord_array[i + 1] += srcoy;
    }

    op->setSrc(1, src);
    op->setDst(1, dst);
    op->setParam(1, color_array);
    op->setParam(2, brush_ref, XIL_RELEASE_REF);
    op->setParam(3, count);
    op->setParam(4, coord_array);

    return op;
}

XilOpPaint::XilOpPaint(XilOpNumber op_num) : XilOpPoint(op_num) { }
XilOpPaint::~XilOpPaint() { }
