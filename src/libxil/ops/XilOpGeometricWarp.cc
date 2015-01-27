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
//  File:	XilOpGeometricWarp.cc
//  Project:	XIL
//  Revision:	1.19
//  Last Mod:	10:07:36, 03/10/00
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
#pragma ident	"@(#)XilOpGeometricWarp.cc	1.19\t00/03/10  "

#include <xil/xilGPI.hh>
#include "XilOpGeometricWarp.hh"

XilOp*
XilOpGeometricWarp::constructWarpOp(const char*                  function_name,
                                    void*                        args[],
                                    XilOpCache*                  op_caches[],
                                    unsigned int                 num_warp_bands,
                                    XiliTablewarpOpCreateFuncPtr op_create_func)
{
    //
    //  Args to the operation.
    //
    XilImage*           src_image     = (XilImage*)args[0];
    XilImage*           dst_image     = (XilImage*)args[1];
    const char*         interpolation = (const char*)args[2];
    XilImage*           warp_image    = (XilImage*)args[3];

    //
    //  Check for NULL dst_image
    //
    if(dst_image == NULL) {
        if(src_image) {
            XIL_ERROR(src_image->getSystemState(), XIL_ERROR_USER, "di-207", TRUE);
	} else {
            XIL_ERROR(NULL, XIL_ERROR_USER, "di-207", TRUE);
        }

	return NULL;
    }

    //
    //  Check for invalid image being used
    //
    if((src_image) && !src_image->isValid()) {
        XIL_ERROR(src_image->getSystemState(), XIL_ERROR_USER, "di-327", TRUE);
	return NULL;
    }

    //
    //  Convert the interpolation string into an internally-used constant.
    //
    XiliInterpolationType type =
        xili_get_interpolation_type(interpolation);

    if(type == XiliUnsupportedInterpolation) {
        XIL_ERROR(dst_image->getSystemState(), XIL_ERROR_USER, "di-416", TRUE);
        return NULL;
    }

    //
    //  Set the op cache.
    //
    XilOpCache* op_cache = op_caches[type];

    //
    //  Get the interpolation tables if it's general interpolation.
    //
    XilInterpolationTable* horiz    = NULL;
    XilInterpolationTable* vertical = NULL;
    if(type == XiliGeneral) {
	XilSystemState* state = dst_image->getSystemState();

	if((state->getInterpolationTables(&horiz, &vertical) == XIL_FAILURE) ||
	   (horiz == NULL) || (vertical == NULL)) {
            interpolation = "nearest";
	    type          = XiliNearest;
	}
    }

    //
    //  Build the real string name
    //
    char func_name[XILI_MAX_GEOMETRIC_NAME_LENGTH];
    if(xili_get_geometric_function_name(dst_image->getSystemState(),
                                        function_name,
                                        type, func_name) == FALSE) {
	XIL_ERROR(dst_image->getSystemState(), XIL_ERROR_USER, "di-146", TRUE);
	return NULL;
    }

    XilOpNumber opnum;
    if((opnum = xili_verify_op_tablewarp(func_name, 
                                         op_cache,
                                         src_image, dst_image,
                                         warp_image, num_warp_bands))== -1) {
        return NULL;
    }

    //
    //  Create the op.
    //
    XilOpGeometricWarp* op =
        (*op_create_func)(opnum, src_image, dst_image, type, horiz, vertical);
    if(op == NULL) {
        XIL_ERROR(dst_image->getSystemState(),XIL_ERROR_RESOURCE,"di-1",TRUE);
        return NULL;
    }
    op->setSrc(1, src_image);
    op->setSrc(2, warp_image);
    op->setDst(1, dst_image);

    //
    //  Set up the other op parameters, these are all derived
    //  from the source. The method returns the next op parameter
    //  position.
    //
    int next_posn = op->setWarpParams();

    //
    //  If the interpolation is general need to add the interpolation table
    //  parameters to the op. 
    //
    if(type == XiliGeneral) {
        //
        //  Reference the interpolation tables -- actually, setup a reference
        //  to the interpolation tables from the op to the interpolation
        //  tables and vice versa.  If the interpolation tables change before
        //  the op is executed, then a copy will be made.
        //
	XilInterpolationTable* op_horiz;
	XilInterpolationTable* op_vertical;

	op_horiz = (XilInterpolationTable*)(horiz->aquireDefRef(op));
	if(op_horiz == NULL) {
            op->destroy();
	    XIL_ERROR(dst_image->getSystemState(), XIL_ERROR_RESOURCE, "di-1", TRUE);
	    return NULL;
	}

	op_vertical = (XilInterpolationTable*)(vertical->aquireDefRef(op));
	if(op_vertical == NULL) {
            op->destroy();
	    XIL_ERROR(dst_image->getSystemState(), XIL_ERROR_RESOURCE, "di-1", TRUE);
	    return NULL;
	}

	op->setParam(next_posn, op_horiz, XIL_RELEASE_REF);
	op->setParam(next_posn+1, op_vertical, XIL_RELEASE_REF);
    }
    
    return op;
}


//
//  Set up warp parameters
//
int
XilOpGeometricWarp::setWarpParams()
{
    //
    //  The image returns a copy of the roi if one is present so it should be
    //  ok for the op to destroy it later.
    //
    XilRoi* roi = srcImage->getRoi();
    setParam(1, roi);

    //
    //  We also need to pass down the offsets for source and dest images so
    //  the compute routine can effectively align in global space.
    //    
    setParam(2, src_ox);
    setParam(3, src_oy);
    setParam(4, dst_ox);
    setParam(5, dst_oy);

    src_width  = srcImage->getWidth();
    src_height = srcImage->getHeight();
    if(roi == NULL) {
	//
	// No roi set the roi bounding box values
	// to the whole image.
	//
	sroi_x = 0;
	sroi_y = 0;
	sroi_w = src_width;
	sroi_h = src_height;
	whole_image = TRUE;
    } else {
	//
	// There is a roi get its bounding box
	//
	roi->getIntBoundingBox(&sroi_x, &sroi_y,
                               &sroi_w, &sroi_h);
	if((sroi_x == 0) && (sroi_y == 0) &&
	   (sroi_w == src_width) && (sroi_h == src_height)) {
	    whole_image = TRUE;
	} else {
	    whole_image = FALSE;
	}
    }

    //
    //  return the number corresponding to the next param to be set
    //  we have set 5 params here, so return 6.
    //
    return 6;
}

//    
//  Over-riding setBoxStorage map to set the source storage area to
//  be bigger than the pixel area.
//
XilStatus
XilOpGeometricWarp::setBoxStorage(XiliRect*            rect,
                                  XilDeferrableObject* object,
                                  XilBox*              box)
{
    XilImage* image = (XilImage*)object;

    //
    //  Default case for all images and then adjust the box to be slightly
    //  larger in the source based on interpolation type.
    //
    *box = *rect;

    if(image->setBoxStorage(box) == XIL_FAILURE) {
        return XIL_FAILURE;
    }

    //
    //  Source Image -- adjust if interpolation type implies growing the box.
    //
    if(image == getSrcImage(1) && interpolationType != XiliNearest) {
        int sx1, sy1, sx2, sy2, sband;

        box->getStorageAsCorners(&sx1, &sy1, &sx2, &sy2, &sband);

        //
        //  Compute new locations by expanding the region we shrunk before
        //  by the same edges we added.
        //
        unsigned int new_sx1 = (unsigned int)(sx1 - leftEdge);
        unsigned int new_sy1 = (unsigned int)(sy1 - topEdge);
        unsigned int new_sx2 = (unsigned int)(sx2 - rightEdge);
        unsigned int new_sy2 = (unsigned int)(sy2 - bottomEdge);

        box->setStorageLocation(new_sx1, new_sy1,
                                new_sx2 - new_sx1 + 1,
                                new_sy2 - new_sy1 + 1, sband);
    }

    return XIL_SUCCESS;
}

//
//  Backward map for tablewarp always map back to
//  the bounding box of the source roi.
//
XilStatus
XilOpGeometricWarp::gsBackwardMap(XiliRect*    dst_rect,
				  XiliRect*    src_rect,
				  unsigned int src_number)
{
    double x1;
    double y1;
    double x2;
    double y2;
    
    if(src_number == 1) {
	//
	//  Its the warp image -- just make the box correspond
	//  destination rect as that's what is being used
	//        
	src_rect->set(dst_rect);

	return XIL_SUCCESS;
    }
    
    //
    //  It doesn't matter what the dest box is we
    //  always map to the bounding box of src_roi
    //
    if(whole_image == TRUE) {
	//
	//  Don't need to clip just set the values
	//
	x1 = srcgs_X1 + leftEdge;
	y1 = srcgs_Y1 + topEdge;
	x2 = srcgs_X2 + rightEdge;
	y2 = srcgs_Y2 + bottomEdge;
    } else {
	//
	//  If the roi isn't the whole image we clip
	//  to make sure that we can correctly expand
	//  the storage, from the borders. eg x1 = 1
	//  and leftEdge = 2, requires us to map to 2.
	//
	x1 = _XILI_MAX(sroi_x, leftEdge);
	y1 = _XILI_MAX(sroi_y, topEdge);
	x2 = _XILI_MIN((sroi_x+sroi_w-1), (src_width + rightEdge));
	y2 = _XILI_MIN((sroi_y+sroi_h-1), (src_height + bottomEdge));

	x1 -= src_ox;
	y1 -= src_oy;
	x2 -= src_ox;
	y2 -= src_oy;
    }

    //
    //  Now we have the values set up the boxes note
    //  that storage is expanded for the interpolation
    //  edges
    //
    src_rect->set(x1, y1, x2, y2);

    return XIL_SUCCESS;
}

XilStatus
XilOpGeometricWarp::moveIntoObjectSpace(XiliRect* rect,
                                        XilDeferrableObject* object)
{
    if(object == getSrcImage(2)) {
        //
        // It's the warp image. Since origins are not considered
        // for warp image, i.e., they are always at (0,0) do nothing.
        //
        return XIL_SUCCESS;
    } else {
        //
        // It's the source or destination image. So move into Object space
        //
        return object->convertToObjectSpace(rect);
    }
}

XilStatus
XilOpGeometricWarp::moveIntoGlobalSpace(XiliRect* rect,
                                        XilDeferrableObject* object)
{
    if(object == getSrcImage(2)) {
        //
        // It's the warp image. Since origins are not considered
        // for warp image, i.e, they are always at (0,0) do nothing.
        //
        return XIL_SUCCESS;
    } else {
        //
        // It's the source image. So move into Object space
        //
        return object->convertToGlobalSpace(rect);
    }
}

//
// Call the parents method
//
XilStatus
XilOpGeometricWarp::moveIntoObjectSpace(XilRoi*              roi,
                                        XilDeferrableObject* object)
{
    if(object == getSrcImage(2)) {
        //
        // It's the warp image. Since origins are not considered
        // for warp image, i.e, they are always at (0,0) do nothing.
        //
        return XIL_SUCCESS;
    } else {
        return XilOp::moveIntoObjectSpace(roi, object);
    }
}



XilStatus
XilOpGeometricWarp::moveIntoGlobalSpace(XilRoi*              roi,
                                        XilDeferrableObject* object)
{
    if(object == getSrcImage(2)) {
        //
        // It's the warp image. Since origins are not considered
        // for warp image, i.e, they are always at (0,0) do nothing.
        //
        return XIL_SUCCESS;
    } else {
        return XilOp::moveIntoGlobalSpace(roi, object);
    }
}
