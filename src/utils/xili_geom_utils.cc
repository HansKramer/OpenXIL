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
//  File:	xili_geom_utils.cc
//  Project:	XIL
//  Revision:	1.22
//  Last Mod:	10:16:31, 03/10/00
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
#pragma ident	"@(#)xili_geom_utils.cc	1.22\t00/03/10  "

#include <math.h>
#include <stdlib.h>
#include "XiliUtils.hh"
#include "xili_geom_utils.hh"

#ifndef _WINDOWS
//
// Table of fractional position in units of 1/16: xili_frac_table[i] = i/16.0F
// Used for conversion of rightmost 4 bits of short int warp table values.
//
float xili_frac_table[16] = {
    0.0,  0.0625, 0.125, 0.1875,
    0.25, 0.3125, 0.375, 0.4375,
    0.5,  0.5625, 0.625, 0.6875,
    0.75, 0.8125, 0.875, 0.9375
};
#endif // _WINDOWS

//
//  precomputes values i*stride into a table of size 'elements'
//
int*
xili_build_opt_table(int elements,
		     int stride)
{
    int*  opt_table = (int*) new int[elements];
    if(opt_table == NULL) {
        return NULL;
    }

    opt_table[0] = 0;
    for (int i = 1; i < elements; i++) {
	opt_table[i] = opt_table[i - 1] + stride;
    }
    return opt_table;
}

//
// --- Begin affine op special case checking routines ---
//

//
// Utility routines
//

//
// Just in case M_PI is in a block of <math.h> disabled by the preprocessor;
// these symbolic names are #undef'ed below.
//
#ifdef M_PI
#define XIL_PI          M_PI
#else
#define XIL_PI          3.14159265358979323846
#endif
#define XIL_PI_2        (XIL_PI/2.0)
#define XIL_2_PI        (2.0*XIL_PI)

Xil_boolean
xili_is_multiple_of_2pi(float angle)
{
    return _XILI_FLT_EQ_INT(angle/XIL_2_PI);
}

Xil_boolean
xili_is_multiple_of_pi_2(float angle)
{
    return _XILI_FLT_EQ_INT(angle/XIL_PI_2);
}

#undef XIL_PI
#undef XIL_PI_2
#undef XIL_2_PI

//
// Routines to extract information from an affine transform matrix
//
//
// This routine should be called only after it is verified that t.a and t.b
// are in the range [-1,1], i.e., that the "linear" part of the affine
// transform is indeed a pure rotation; otherwise a matherr might result.
//
float
xili_affine_extract_rotation(AffineTr t)
{
    return (float)atan2(t.b, t.a);
}

void
xili_affine_extract_scale(AffineTr t,
                          float*   sx,
                          float*   sy)
{
    *sx = (float) t.a;
    *sy = (float) t.d;
}

void
xili_affine_extract_translation(AffineTr t,
                                float*   tx,
                                float*   ty)
{
    *tx = (float) t.tx;
    *ty = (float) t.ty;
}

//
// Affine special case routines
//
Xil_boolean
xili_affine_is_copy(AffineTr t)
{
    return (XILI_FLT_EQ((float) t.a,  1.0) &&
            XILI_FLT_EQ((float) t.b,  0.0) &&
            XILI_FLT_EQ((float) t.c,  0.0) &&
            XILI_FLT_EQ((float) t.d,  1.0) &&
            XILI_FLT_EQ((float) t.tx, 0.0) &&
            XILI_FLT_EQ((float) t.ty, 0.0));
}

//
// The following 3 routines assume that the affine transform is not a copy.
//
Xil_boolean
xili_affine_is_rotate(AffineTr t)
{
    return (XILI_FLT_EQ((float) t.a,   (float) t.d)  &&
            XILI_FLT_EQ((float) t.b, (float)  -t.c)  &&
            XILI_FLT_EQ((float) t.tx,  0.0) &&
            XILI_FLT_EQ((float) t.ty,  0.0) &&
            XILI_FLT_EQ((float) xili_affdet(t), 1.0)); // no scale chg. allowed
}

Xil_boolean
xili_affine_is_scale(AffineTr t)
{
    return (t.a > 0.0 &&
            XILI_FLT_EQ((float) t.b,  0.0) &&
            XILI_FLT_EQ((float) t.c,  0.0) &&
            t.d > 0.0 &&
            XILI_FLT_EQ((float) t.tx, 0.0) &&
            XILI_FLT_EQ((float) t.ty, 0.0));
}

Xil_boolean
xili_affine_is_translate(AffineTr t)
{
    return (XILI_FLT_EQ((float) t.a, 1.0) &&
            XILI_FLT_EQ((float) t.b, 0.0) &&
            XILI_FLT_EQ((float) t.c, 0.0) &&
            XILI_FLT_EQ((float) t.d, 1.0));
}

//
// Check whether an affine transform equals a specified set of values
//
static
Xil_boolean
xili_affine_eq(AffineTr t,
               float a,
               float b,
               float c,
               float d,
               float tx,
               float ty)
{
    if(XILI_FLT_EQ((float) t.a,  a)  &&
       XILI_FLT_EQ((float) t.b,  b)  &&
       XILI_FLT_EQ((float) t.c,  c)  &&
       XILI_FLT_EQ((float) t.d,  d)  &&
       XILI_FLT_EQ((float) t.tx, tx) &&
       XILI_FLT_EQ((float) t.ty, ty)) {
        return (Xil_boolean)TRUE;
    }

    return (Xil_boolean)FALSE;
}

Xil_boolean
xili_affine_is_transpose(AffineTr t,
                         float src_width,
                         float src_height,
                         float src_origin_x,
                         float src_origin_y,
                         float dst_origin_x,
                         float dst_origin_y,
                         XilFlipType* fliptype = (XilFlipType*)NULL)
{
    Xil_boolean is_transpose = (Xil_boolean)FALSE;
    XilFlipType flip_type;

    //
    //  We consider the center of the image to be w/2, h/2.  Here we're
    //  checking to make sure the origins cancel themselves out, they're at
    //  the center and the transform matches a transposition.
    //
    if(xili_affine_eq(t, -1.0F, 0.0F, 0.0F, 1.0F,
                      src_width - dst_origin_x - src_origin_x,
                      src_origin_y - dst_origin_y)) {
        is_transpose = (Xil_boolean)TRUE;
        flip_type = XIL_FLIP_Y_AXIS;
    } else if(xili_affine_eq(t, 1.0F, 0.0F, 0.0F, -1.0F,
                             src_origin_x - dst_origin_x,
                             src_height - dst_origin_y - src_origin_y)) {
        is_transpose = (Xil_boolean)TRUE;
        flip_type = XIL_FLIP_X_AXIS;
    } else if(xili_affine_eq(t, 0.0F, 1.0F, 1.0F, 0.0F,
                             src_origin_y - dst_origin_x,
                             src_origin_x - dst_origin_y)) {
        is_transpose = (Xil_boolean)TRUE;
        flip_type = XIL_FLIP_MAIN_DIAGONAL;
    } else if(xili_affine_eq(t, 0.0F, -1.0F, -1.0F, 0.0F,
                             src_height - dst_origin_x - src_origin_y,
                             src_width  - dst_origin_y - src_origin_x)) {
        is_transpose = (Xil_boolean)TRUE;
        flip_type = XIL_FLIP_ANTIDIAGONAL;
    } else if(xili_affine_eq(t, 0.0F, 1.0F, -1.0F, 0.0F,
                             src_origin_y - dst_origin_x,
                             src_width - dst_origin_y - src_origin_x)) {
        is_transpose = (Xil_boolean)TRUE;
        flip_type = XIL_FLIP_90;
    } else if(xili_affine_eq(t, -1.0F, 0.0F, 0.0F, -1.0F,
                             src_width  - dst_origin_x - src_origin_x,
                             src_height - dst_origin_y - src_origin_y)) {
        is_transpose = (Xil_boolean)TRUE;
        flip_type = XIL_FLIP_180;
    } else if(xili_affine_eq(t, 0.0F, -1.0F, 1.0F, 0.0F,
                             src_height - dst_origin_x - src_origin_y,
                             src_origin_x - dst_origin_y)) {
        is_transpose = (Xil_boolean)TRUE;
        flip_type = XIL_FLIP_270;
    }

    if(is_transpose == (Xil_boolean)TRUE &&
       fliptype != (XilFlipType*)NULL) {
        *fliptype = flip_type;
    }

    return is_transpose;
}

//
// Scale special case routines
//
Xil_boolean
xili_scale_is_subsample2x(float sx, float sy)
{
    return (Xil_boolean) (XILI_FLT_EQ(sx, 0.5F) && XILI_FLT_EQ(sy, 0.5F));
}

Xil_boolean
xili_scale_is_zoom2x(float sx, float sy)
{
    return (Xil_boolean) (XILI_FLT_EQ(sx, 2.0F) && XILI_FLT_EQ(sy, 2.0F));
}

//
// Translate special case routines
//
Xil_boolean
xili_translation_is_integral(float tx, float ty)
{
    return (_XILI_FLT_EQ_INT(tx) && _XILI_FLT_EQ_INT(ty));
}

//
// --- End affine op special case checking routines ---
//
