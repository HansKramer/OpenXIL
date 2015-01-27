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
//  File:	XilOpAffine.cc
//  Project:	XIL
//  Revision:	1.24
//  Last Mod:	10:07:29, 03/10/00
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
#pragma ident	"@(#)XilOpAffine.cc	1.24\t00/03/10  "

#include <stdlib.h>
#include <string.h>

#include <xil/xilGPI.hh>
#include "XilOpGeometricAffine.hh"
#include "XilOpCopy.hh"
#include "XilOpRotate.hh"
#include "XilOpScale.hh"
#include "XilOpTranslate.hh"
#include "XilOpTranspose.hh"
#include "xili_geom_utils.hh"

class XilOpAffine : public XilOpGeometricAffine {
public:
    static XilOp* create(char* function_name,
                         void* args[],
                         int count);
    
    //
    //  A test to see if the pixels written by the current op covers the area
    //  written by the previous op.  See routine in XilOpPrivate.cc for more
    //  details.
    //
    //  TODO: 10/8/96 jlf  Implement this routine so it will catch the cases
    //                     when is op covers previous op instead of always
    //                     assuming this op will not cover the previous op.
    //
    Xil_boolean   thisOpCoversPreviousOp()
    {
        return FALSE;
    }

protected:
                  XilOpAffine(XilOpNumber            op_num,
                              XilImage*              src_image,
                              XilImage*              dst_image,
                              XiliInterpolationType  type,
                              AffineTr               affine_tr,
                              XilInterpolationTable* h_table,
                              XilInterpolationTable* v_table) :
        XilOpGeometricAffine(op_num, src_image, dst_image,
                             type, affine_tr, h_table, v_table)
    {
    }


    virtual       ~XilOpAffine()
    {
    }
};


//
// Create the affine op class
//
XilOp*
XilOpAffine::create(char  function_name[],
                    void* args[],
                    int   )
{
    static XilOpCache	affine_op_caches[XILI_NUM_SUPPORTED_INTERPOLATIONS];
    XilOpCache*         affine_op_cache;
    char		func_name[XILI_MAX_GEOMETRIC_NAME_LENGTH];

    //
    //  Args to the operation.
    //
    XilImage*           src           = (XilImage*)args[0];
    XilImage*           dst           = (XilImage*)args[1];
    const char*         interpolation = (const char*)args[2];
    float*              user_matrix   = (float*)args[3];

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
    //  Check for NULL affine matrix.
    //
    if(user_matrix == NULL) {
	XIL_ERROR(dst->getSystemState(), XIL_ERROR_USER, "di-446", TRUE);
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
    affine_op_cache = &affine_op_caches[type];

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
    //  Copy the transition matrix for use by the op and compute routine.
    //
    //  We copy it into our structure and only use the structure to guarentee
    //  that if the user is doing something stupid (like changing the
    //  user_matrix values in another thread), we get one consistent view and
    //  don't SEGV later due to mismatches.
    //
    //  NOTE: THE SECOND AND THIRD VALUES ARE PERMUTED FROM THOSE PASSED IN
    //
    AffineTr affine_tr;
    affine_tr.a  = user_matrix[0];
    affine_tr.b  = user_matrix[2];
    affine_tr.c  = user_matrix[1];
    affine_tr.d  = user_matrix[3];
    affine_tr.tx = user_matrix[4];
    affine_tr.ty = user_matrix[5];

#ifdef AFFINE_DEBUG
    fprintf(stderr, "-----------------------------------------------------------------------------\n");
    fprintf(stderr, "%20.15f %20.15f %20.15f\n\t%20.15f %20.15f %20.15f\n",
            affine_tr.a, affine_tr.c, affine_tr.b,
            affine_tr.d, affine_tr.tx, affine_tr.ty);
    fprintf(stderr, "-----------------------------------------------------------------------------\n");
#endif

    //
    //  Check for special cases and use different ops where appropriate.
    //
    if(xili_affine_is_copy(affine_tr)) {
        //
        //  It's an identity matrix so just call xil_copy.
        //
        return XilOpCopy::create("copy", args, 2);
    } else if(xili_affine_is_rotate(affine_tr)) {
        float angle = xili_affine_extract_rotation(affine_tr);

        void* args_rotate[5] = {(void*)src, (void*)dst, (void*)interpolation,
                                (void*)&angle, NULL};

        return XilOpRotate::create("rotate", args_rotate, 4);
    } else if(xili_affine_is_scale(affine_tr)) {
        float scale_x;
        float scale_y;
        xili_affine_extract_scale(affine_tr, &scale_x, &scale_y);

        void* args_scale[6] = {(void*)src, (void*)dst, (void*)interpolation,
                               (void*)&scale_x, (void*)&scale_y, NULL};

        return XilOpScale::create("scale", args_scale, 5);
    } else if(xili_affine_is_translate(affine_tr)) {
        float trans_x;
        float trans_y;
        xili_affine_extract_translation(affine_tr, &trans_x, &trans_y);

        void* args_translate[6] = {(void*)src, (void*)dst, (void*)interpolation,
                                   (void*)&trans_x, (void*)&trans_y, NULL};

        return XilOpTranslate::create("translate", args_translate, 5);
    } else {
        //
        // Transform could be a transposition; need to get some info to check.
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
        // Check for transposition
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

    XilOpNumber opnum;						   
    if((opnum = xili_verify_op_args(func_name, affine_op_cache,
                                    dst, src)) == -1) {
        return NULL;
    }

    //
    //  Copy the user's affine transformation matrix for the GPI routine.
    //
    float* matrix = new float[6];
    if(matrix == NULL) {
	XIL_ERROR(dst->getSystemState(), XIL_ERROR_RESOURCE, "di-1", TRUE);
	return NULL;
    }

    matrix[0] = affine_tr.a;
    matrix[1] = affine_tr.c;
    matrix[2] = affine_tr.b;
    matrix[3] = affine_tr.d;

    //
    //  Add in the origins to the tx, ty values since the compue routine does
    //  not have access to image origin information, we take care of it by
    //  modifying the matrix values.
    //
    matrix[4] = affine_tr.tx + dst->getOriginX();
    matrix[5] = affine_tr.ty + dst->getOriginY();

    //
    //  Furthermore, we take care of the 0.5 pixel shift from coordinates to
    //  the center of the pixel by modifying the affine transform.  See the
    //  XilOpGeometricAffine constructor for more details.
    //
    if(type != XiliNearest) {
        matrix[4] -= (1 - affine_tr.a - affine_tr.b)/2;
        matrix[5] -= (1 - affine_tr.c - affine_tr.d)/2;
    } else {
        matrix[4] -= 0.5;
        matrix[5] -= 0.5;
    }

    XilOpAffine* op =
        new XilOpAffine(opnum, src, dst, type, affine_tr, horiz, vertical);
    if(op == NULL) {
	XIL_ERROR(dst->getSystemState(), XIL_ERROR_RESOURCE, "di-1", TRUE);
	delete matrix;
	return NULL;
    }

    op->setSrc(1, src);
    op->setDst(1, dst);
    op->setParam(1, matrix);

    //
    //  Pass in the source origins which specifies the point in the source
    //  about which the affine transformation is performed.
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
            delete matrix;
            XIL_ERROR(dst->getSystemState(), XIL_ERROR_RESOURCE, "di-1", TRUE);
            return NULL;
        }

        op_vertical = (XilInterpolationTable*)(vertical->aquireDefRef(op));
        if(op_vertical == NULL) {
            op->destroy();
            delete matrix;
            XIL_ERROR(dst->getSystemState(), XIL_ERROR_RESOURCE, "di-1", TRUE);
            return NULL;
        }

        op->setParam(4, op_horiz, XIL_RELEASE_REF);
        op->setParam(5, op_vertical, XIL_RELEASE_REF);
    }

    return op;
}
