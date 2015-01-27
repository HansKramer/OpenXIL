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
//  File:	XilOpRotate.cc
//  Project:	XIL
//  Revision:	1.22
//  Last Mod:	10:07:28, 03/10/00
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
#pragma ident	"@(#)XilOpRotate.cc	1.22\t00/03/10  "

#include <stdlib.h>
#include <string.h>

#include <xil/xilGPI.hh>
#include "XilOpCopy.hh"
#include "XilOpGeometricAffine.hh"
#include "XilOpRotate.hh"
#include "XilOpTranspose.hh"
#include "XiliOpUtils.hh"
#include "xili_geom_utils.hh"

XilOp*
XilOpRotate::create(char  function_name[],
		    void* args[],
		    int)
{
    static XilOpCache	rotate_op_caches[XILI_NUM_SUPPORTED_INTERPOLATIONS];
    XilOpCache*	        rotate_op_cache;
    char		func_name[XILI_MAX_GEOMETRIC_NAME_LENGTH];

    //
    //  Args to the operation.
    //
    XilImage*           src            = (XilImage*)args[0];
    XilImage*           dst            = (XilImage*)args[1];
    const char*         interpolation  = (const char*)args[2];
    float               rotation_angle = *((float*)args[3]);

    //
    //  Check for NULL dst
    //
    if(dst == NULL) {
        if(src) {
            XIL_ERROR(src->getSystemState(), XIL_ERROR_USER, "di-207", TRUE);
	} else {
            XIL_ERROR(NULL, XIL_ERROR_USER, "di-207", TRUE);
        }

	return NULL;
    }

    //
    //  Check for invalid image being used
    //
    if((src) && !src->isValid()) {
        XIL_ERROR(src->getSystemState(), XIL_ERROR_USER, "di-327", TRUE);
	return NULL;
    }

    //
    //  Convert the interpolation string into an internally-used constant.
    //
    XiliInterpolationType type =
        xili_get_interpolation_type(interpolation);

    if(type == XiliUnsupportedInterpolation) {
        XIL_ERROR(dst->getSystemState(), XIL_ERROR_USER, "di-416", TRUE);
        return NULL;
    }

    //
    //  Set the op cache.
    //
    rotate_op_cache = &rotate_op_caches[type];

    //
    //  Get the interpolation tables if it's general interpolation.
    //
    XilInterpolationTable* horiz    = NULL;
    XilInterpolationTable* vertical = NULL;
    if(type == XiliGeneral) {
	XilSystemState* state = dst->getSystemState();

	if((state->getInterpolationTables(&horiz, &vertical) == XIL_FAILURE) ||
	   (horiz == NULL) || (vertical == NULL)) {
            interpolation = "nearest";
	    type          = XiliNearest;
	}
    }

    //
    //  Initialize the affine transform structure from the angle.
    //
    AffineTr affine_tr;
    xili_rotate(rotation_angle, &affine_tr);
    
    if(xili_is_multiple_of_2pi(rotation_angle)) {
	//
        //  It's a rotate by an integral multiple of 2*PI make it a copy.
	//
	return XilOpCopy::create("copy", args, 2);
    } else if(xili_is_multiple_of_pi_2(rotation_angle)) {
        //
        // If execution branches in here we know that the angle in degrees is
        // an integral multiple of 90 but not of 360.
        //

        //
        // Get the source image dimensions
        //
        unsigned int src_width  = src->getWidth();
        unsigned int src_height = src->getHeight();

        //
        // Get the image origins
        //
        float src_origin_x;
        float src_origin_y;
        src->getOrigin(&src_origin_x, &src_origin_y);
        float dst_origin_x;
        float dst_origin_y;
        dst->getOrigin(&dst_origin_x, &dst_origin_y);

        //
        // Check for transposition using the equivalent affine transform
        //
        XilFlipType fliptype;
        if(xili_affine_is_transpose(affine_tr,
                                    (float)src_width,
                                    (float)src_height,
                                    src_origin_x,
                                    src_origin_y,
                                    dst_origin_x,
                                    dst_origin_y,
                                    &fliptype)) {
            //
            // It's a transposition so create that op instead.
            //
            void* args_transpose[4] =
                { (void *)src, (void *)dst, (void*)fliptype, NULL };

            return XilOpTranspose::create("transpose", args_transpose, 3);
        }
    }

    //
    //  Build the real string name
    //
    if(xili_get_geometric_function_name(dst->getSystemState(),
                                        function_name,
                                        type, func_name) == FALSE) {
	XIL_ERROR(dst->getSystemState(), XIL_ERROR_USER, "di-146", TRUE);
	return NULL;
    }

    //
    //  Check args and get op number.
    //
    XilOpNumber opnum;
    if((opnum = xili_verify_op_args(func_name, rotate_op_cache,
				    dst, src))== -1) {
        return NULL;
    }

    //
    //  Create the op
    //
    XilOpRotate* op =
        new XilOpRotate(opnum, src, dst, type, affine_tr, horiz, vertical);
    if(op == NULL) {
	XIL_ERROR(dst->getSystemState(), XIL_ERROR_RESOURCE, "di-1", TRUE);
	return NULL;
    }

    op->setSrc(1, src);
    op->setDst(1, dst);
    op->setParam(1, rotation_angle);

    //
    //  Pass in the source origins which form the point which the rotation is
    //  performed.
    //
    op->setParam(2, src->getOriginX());
    op->setParam(3, src->getOriginY());

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
            XIL_ERROR(dst->getSystemState(), XIL_ERROR_RESOURCE, "di-1", TRUE);
            return NULL;
        }

        op_vertical = (XilInterpolationTable*)(vertical->aquireDefRef(op));
        if(op_vertical == NULL) {
            op->destroy();
            XIL_ERROR(dst->getSystemState(), XIL_ERROR_RESOURCE, "di-1", TRUE);
            return NULL;
        }

        op->setParam(4, op_horiz, XIL_RELEASE_REF);
        op->setParam(5, op_vertical, XIL_RELEASE_REF);
    }

    return op;
}
