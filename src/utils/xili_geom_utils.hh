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
//  File:	xili_geom_utils.hh
//  Project:	XIL
//  Revision:	1.42
//  Last Mod:	10:24:00, 03/10/00
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

#ifndef _GEOM_UTIL_HH
#define _GEOM_UTIL_HH

#pragma ident	"@(#)xili_geom_utils.hh	1.42\t00/03/10  "

#include <math.h>

#ifdef _XIL_LIBXIL_PRIVATE
#include "_XilDefines.h"
#include "_XilClasses.hh"
#include "_XilBox.hh"
#else
#include <xil/xilGPI.hh>
#endif

// makes 1 fract be 65536 represented as integer

#define	XILI_GEOM_DOT16	((double)65536.0)

struct Vertex {
    double  x;
    double  y;
    Vertex* next;

    Vertex() { }
    Vertex(double init_x,
           double init_y) {
        x = init_x;
        y = init_y;
    }
};

struct AffineTr {
    double a;
    double b;
    double c;
    double d;
    double tx;
    double ty;
};

struct AffineData {
    XilOp*                 op;
    XilStorage*            src_storage;	// image storage ptrs
    XilStorage*            dst_storage;
    XilRoi*                roi;
    XilBox*                src_box;
    XilBox*                dst_box;
    float*                 matrix;
    float                  src_x_origin;
    float                  src_y_origin;
    unsigned int           nbands;
    XilInterpolationTable* htable; // used for general interpolation
    XilInterpolationTable* vtable;
};

struct SubsampleData {
    XilOp*                 op;
    XilStorage*            src_storage;	// image storage ptrs
    XilStorage*            dst_storage;
    XilRoi*                roi;
    XilBox*                src_box;
    XilBox*                dst_box;
    float                  xscale;
    float                  yscale;
    unsigned int           nbands;
};

struct TablewarpData {
    XilOp*                 op;
    XilRoi*                roi;
    XilRoi*                src_roi;
    XilStorage*            src_storage;	
    XilStorage*            warp_storage;
    XilStorage*            dst_storage;
    XilBox*                src_box;
    XilBox*                warp_box;
    XilBox*                dst_box;

    float                  src_x_origin;
    float                  src_y_origin;
    float                  dst_x_origin;
    float                  dst_y_origin;

    unsigned int           nbands;
    unsigned int           v_key;
    unsigned int           v_size;
    const float*           v_data;
    unsigned int           v_subsamples;

    unsigned int           h_key;
    unsigned int           h_size;
    const float*           h_data;
    unsigned int           h_subsamples;

    int*                   row;
    XilSystemState*        state;
};

//
// Used by geometric routines to get the integer coordinates of
// the sampling point. The fractional parts of the sampling
// point is separately calculated. Both positive and negative
// x values are rounded down.
//
inline
int
XILI_GEOM_ROUNDDOWN(double x)
{
    return (int)x;
}


//
//  Performs "linear part" of affine transform on v.
//
inline
void
xili_lmap(Vertex*   vertex,
	  AffineTr& t)
{
    double x = vertex->x;
    double y = vertex->y;

    vertex->x = t.a*x + t.b*y;
    vertex->y = t.c*x + t.d*y;
}

//
//  Performs affine transform on vertex.
//
inline
void
xili_affmap(Vertex*   vertex,
	    AffineTr& t)
{		
    xili_lmap(vertex, t);
    vertex->x += t.tx;
    vertex->y += t.ty;
}

//
//  Generate determinant of "linear" matrix part of affine transform
//
inline
double
xili_affdet(AffineTr& t)
{
    return ((t.a * t.d) - (t.b * t.c));
}

//
//  Provide the inverse of an affine transform
//
inline
void
xili_invert(AffineTr& t,
            AffineTr* inv)
{		
    double inv_det = 1.0 / xili_affdet(t);

    inv->a =  t.d*inv_det;
    inv->b = -t.b*inv_det;
    inv->c = -t.c*inv_det;
    inv->d =  t.a*inv_det;

    Vertex v(t.tx, t.ty);
    v.x = t.tx;
    v.y = t.ty;

    xili_lmap(&v, *inv);

    inv->tx = -v.x;
    inv->ty = -v.y;
}

//
//  Provide an affine transform which is a pure rotation of the given angle.
//
//  We use float because the XIL API only defines things to float precision.
//
inline
void
xili_rotate(float     theta,
            AffineTr* t)
{		
    t->a  = cos(theta);
    t->b  = sin(theta);
    t->c  = -t->b;
    t->d  = t->a;
    t->tx = 0.0;
    t->ty = 0.0;
}

//
//  Provide an affine transform which is a pure scale.
//
//  We use float because the XIL API only defines things to float precision.
//
inline
AffineTr
xili_scale(float xscale,
           float yscale)
{
    AffineTr t;

    t.a  = xscale;
    t.b  = 0.0;
    t.c  = 0.0;
    t.d  = yscale;
    t.tx = 0.0;
    t.ty = 0.0;

    return t;
}

//
//  Returns an affine transform which is a translation.
//
//  We use float because the XIL API only defines things to float precision.
//
inline
void
xili_translate(float     xtrans,
               float     ytrans,
               AffineTr* t)
{		
    t->a  = 1.0;
    t->b  = 0.0;
    t->c  = 0.0;
    t->d  = 1.0;
    t->tx = xtrans;
    t->ty = ytrans;
}

//
// Copy an affine transform from the standard structure to an array
//
inline
void
xili_afftr_to_array(AffineTr t,    // TODO: affected by xil_affine coeff order
                    float* matrix)
{
    matrix[0] = (float) t.a;
    matrix[1] = (float) t.b;
    matrix[2] = (float) t.c;
    matrix[3] = (float) t.d;
    matrix[4] = (float) t.tx;
    matrix[5] = (float) t.ty;
}

//
// Copy an affine transform from an array to the standard structure
//
inline
AffineTr
xili_array_to_afftr(float* matrix) // TODO: affected by xil_affine coeff order
{
    AffineTr t;

    t.a  = matrix[0];
    t.b  = matrix[1];
    t.c  = matrix[2];
    t.d  = matrix[3];
    t.tx = matrix[4];
    t.ty = matrix[5];

    return t;
}

//
// Routines used in checking for special cases
//
Xil_boolean xili_is_multiple_of_2pi(float angle);
Xil_boolean xili_is_multiple_of_pi_2(float angle);

float       xili_affine_extract_rotation(AffineTr t);
void        xili_affine_extract_scale(AffineTr t, float* sx, float* sy);
void        xili_affine_extract_translation(AffineTr t, float* tx, float* ty);

Xil_boolean xili_affine_is_copy(AffineTr t);
Xil_boolean xili_affine_is_rotate(AffineTr t);
Xil_boolean xili_affine_is_scale(AffineTr t);
Xil_boolean xili_affine_is_translate(AffineTr t);
Xil_boolean xili_affine_is_transpose(AffineTr     t,
                                     float        src_width,
                                     float        src_height,
                                     float        src_origin_x,
                                     float        src_origin_y,
                                     float        dst_origin_x,
                                     float        dst_origin_y,
                                     XilFlipType* fliptype);

Xil_boolean xili_scale_is_subsample2x(float  sx, float  sy);
Xil_boolean xili_scale_is_zoom2x(float  sx, float  sy);

Xil_boolean xili_translation_is_integral(float  tx, float  ty);

//
// Definitions and routines for tablewarp displacement calculation.
//
int*        xili_build_opt_table(int, int);

//
// Number of rightmost bits of signed short used for fractional displacement.
//
#define WARP_TABLE_PRECISION 4

//
// Integer value representing 0.5 for short integer warp table.
//
#define WARP_TABLE_HALF 8

//
// Integer value which would represent 1.0 for short integer warp table.
//
#define WARP_TABLE_ONE 16

#ifndef _WINDOWS
//
// Table of fractional position in units of 1/16: xili_frac_table[i] = i/16.0F
// Definition is in xili_geom_utils.cc.
//
extern float xili_frac_table[16];
#endif // _WINDOWS

//
//  Inline functions for extraction of integer table displacement.
//
inline
int
xili_tablewarp_offset_int(Xil_signed16 delta)
{
    return (delta >> WARP_TABLE_PRECISION);
}

inline
int
xili_tablewarp_offset_int(Xil_float32 delta)
{
    return ((int)delta);
}

//
//  Convert the XIL_SHORT warp table value into a floating point value.
//
inline
float
xili_convert_tablewarp_offset(Xil_signed16 val)
{
#ifdef _WINDOWS
    return (float)((val >> WARP_TABLE_PRECISION) + ((val & 0xf)/16.0F));
#else
    //
    //  If it's a negative value, it's in 2s compliment.  It works out
    //  so no special casing is needed.  (-50.125 -> -51 + 0.875)
    //
    return (val >> WARP_TABLE_PRECISION) + xili_frac_table[val & 0xf];
#endif // _WINDOWS
}

//
// Routines for most XIL_FLOAT warp tables.
//
inline
float
xili_convert_tablewarp_offset(Xil_float32 val)
{
    return val;
}

//
// Overloaded inline function "xili_geom_blend()" was found to provide faster
// execution than macro XILI_GEOM_BLEND in affine mappings using bilinear
// interpolation. This function performs linear interpolation of two values
// p0 and p1 using a fractional offset alpha from p0 where alpha is in [0,1].
//
inline
float
xili_geom_blend(float p0,
                float p1,
                float alpha)
{
    return p0 + (p1 - p0)*alpha;
}

inline
double
xili_geom_blend(double p0,
                double p1,
                double alpha)
{
    return p0 + (p1 - p0)*alpha;
}

inline
int
xili_geom_blend(int    p0,
                int    p1,
                double alpha)
{
    return p0 + (int)(((double)(p1 - p0))*alpha); // should round 2nd addend?
}

inline
int
xili_geom_blend(int   p0,
                int   p1,
                float alpha)
{
    return p0 + (int)(((float)(p1 - p0))*alpha); // should round 2nd addend?
}


//
// Overloaded inline function "xili_geom_perturb()" was found to provide
// faster execution than macro XILI_GEOM_PERTURB in affine mappings using
// bicubic interpolation.
//
inline
int
xili_geom_perturb(int   p,
                  int   q,
                  float alpha)
{
    return p + (int)(((float)q)*alpha);
}

inline
int
xili_geom_perturb(int    p,
                  int    q,
                  double alpha)
{
    return p + (int)(((double)q)*alpha);
}

inline
float
xili_geom_perturb(float p,
                  float q,
                  float alpha)
{
    return p + q*alpha;
}

inline
double
xili_geom_perturb(double p,
                  double q,
                  double alpha)
{
    return p + q*alpha;
}


//
// Begining of section related to scanline walk affine acceleration
//

//
// Template function for source data buffer clipping -- if needed.
//
// This should NOT be needed so we just return what we're given and expect the
// optimizer to remove any overhead.
//
template <class Type>
inline
Type
xili_srcbox_clip(Type src_pixel,
#ifdef DEBUG
                 Type src_pixel_min,
                 Type src_pixel_max)
#else
                 Type ,
                 Type )
#endif
{
#ifdef DEBUG
    if(src_pixel < src_pixel_min) {
        XIL_ERROR(NULL, XIL_ERROR_INTERNAL,
                  "Geometric source box CLIP on src_pixel_min!", TRUE);
        return src_pixel_min;
    } else if(src_pixel > src_pixel_max) {
        XIL_ERROR(NULL, XIL_ERROR_INTERNAL,
                  "Geometric source box CLIP on src_pixel_max!", TRUE);
        return src_pixel_max;
    } else {
        return src_pixel;
    }
#else
    return src_pixel;
#endif
}

//
// Symbolic names for quantities related to int tracking of fractional position
//

//
// XILI_GEOM_FRAC_MAX      Integer value representing unity
// XILI_GEOM_FRAC_MAX_HALF Integer value representing one-half
// XILI_GEOM_FRAC_SHIFT    Amount to shift integer to obtain left-most 8 bits
// XILI_GEOM_FRAC_MASK     Mask to obtain 8 right-most bits
//
#define XILI_GEOM_FRAC_MAX      0x3ffffffe
#define XILI_GEOM_FRAC_MAX_HALF 0x1fffffff
#define XILI_GEOM_FRAC_SHIFT    22
#define XILI_GEOM_FRAC_MASK     0xff

//
// Definitions of inline functions
//

//
// Calculate one dimensional incremental value (delta) for scanline walking.
//
inline
void
xili_scanline_delta(double       dsrc_ddstx,
                    unsigned int src_stride,
                    double*      fracd,
                    double*      fracd1,
                    int*         inc,
                    int*         inc1)
{
    //
    // Calculate incremental values by which the source image position is
    // changed in one dimension for a positive unit change in the destination
    // abscissa direction.
    //

    //
    // Calculate integral src change per +1 abscissa dst change.
    //
    double id = floor(dsrc_ddstx);

    //
    // Calculate fractional src change per +1 abscissa dst change
    //
    *fracd = dsrc_ddstx - id;

    //
    // Store the complement of the fractional position vs. unity
    //
    *fracd1 = 1.0F - *fracd;

    //
    // Calculate the number of address bins to skip for the case where the
    // sum of the current fractional image coordinate and the fractional
    // increment is less than unity.
    //
    *inc = ((int)id) * src_stride;

    //
    // Calculate the number of address bins to skip for the case where the
    // sum of the current fractional image coordinate and the fractional
    // increment is greater than or equal to unity.
    //
    *inc1 = *inc + src_stride;
}

//
// Clip integral coordinates to the image box
//
inline
void
xili_scanline_clip(double  coord_in,
                   int     box_max,
                   int*    coord_int_out,
                   double* coord_out)
{
    int coord_int_in = (int)coord_in;

    if(coord_int_in < 0) {
        *coord_int_out = 0;
        *coord_out     = 0.0;
    }
    else if(coord_int_in > box_max) {
        *coord_int_out = box_max;
        *coord_out     = box_max;
    } else {
        *coord_out     = coord_in;
        *coord_int_out = coord_int_in;
    }
}

//
// Definitions of macros
//

// Scanline walk initialization: box level
//
// Declare and calculate incremental (delta) variables.
// These variables are used again only in XILI_SCANLINE_SRC_INCREMENT, below.
// Due to the nature of affine transforms it is only necessary to calculate
// the incremental values at the box level (actually only at the image level).
//
// "Exact" version of XILI_SCANLINE_INIT_BOX: floats instead of ints are
// used to track fractions which is necessary for accuracy in some cases.
//
// TODO: 12/20/96 bpb  Retrieve inverse matrix from op instead of calculating
//
#define XILI_SCANLINE_INIT_BOX_EXACT \
double fracdx;  \
double fracdx1; \
int incx;      \
int incx1;     \
\
double fracdy;  \
double fracdy1; \
int incy;      \
int incy1;     \
\
{ \
    AffineTr fwd; \
    fwd.a  = affine_data.matrix[0]; \
    fwd.b  = affine_data.matrix[2]; \
    fwd.c  = affine_data.matrix[1]; \
    fwd.d  = affine_data.matrix[3]; \
    fwd.tx = affine_data.matrix[4]; \
    fwd.ty = affine_data.matrix[5]; \
    \
    AffineTr inv; \
    xili_invert(fwd, &inv); \
    \
    xili_scanline_delta(inv.a,       \
                        src_pstride, \
                        &fracdx,     \
                        &fracdx1,    \
                        &incx,       \
                        &incx1);     \
    \
    xili_scanline_delta(inv.c,       \
                        src_sstride, \
                        &fracdy,     \
                        &fracdy1,    \
                        &incy,       \
                        &incy1);     \
}

//
// Convert double deltas to ints to pick up a little speed; used for nearest only
//
#define XILI_SCANLINE_INIT_BOX \
XILI_SCANLINE_INIT_BOX_EXACT; \
int ifracdx  = _XILI_ROUND(fracdx * XILI_GEOM_FRAC_MAX); \
int ifracdx1 = XILI_GEOM_FRAC_MAX - ifracdx; \
int ifracdy  = _XILI_ROUND(fracdy * XILI_GEOM_FRAC_MAX); \
int ifracdy1 = XILI_GEOM_FRAC_MAX - ifracdy;

//
// "Bit" version of XILI_SCANLINE_INIT_BOX: identical to the "exact"
// version except for variable "src_pstride".
//
#define XILI_SCANLINE_INIT_BOX_BIT \
unsigned int src_pstride = 1; \
XILI_SCANLINE_INIT_BOX_EXACT;

//
// Scanline walk initialization: scanline level
//
// Reset scanline walk for the beginning of the current scanline
// clipping the coordinates if necessary. Variables declared here which
// persist outside the scope of this macro are used again only in
// XILI_SCANLINE_GET_FRACS and XILI_SCANLINE_SRC_INCREMENT, below.
//
// NB: Within the code using it this macro must be preceded by a declaration
// of the variable "src_pixel" which will vary as a function of the data type
// of the source image.
//
//
// "Exact" version of XILI_SCANLINE_INIT_LINE: doubles instead of ints are
// used to track fractions which is necessary for accuracy in some cases.
//
#define XILI_SCANLINE_INIT_LINE_EXACT \
double fracx; \
double fracy; \
int isx;     \
int isy;     \
{ \
    double src_x; \
    double src_y; \
    op->backwardMap(dst_box,        \
                    dst_scan_start, \
                    y,              \
                    src_box,        \
                    &src_x,         \
                    &src_y);        \
    \
    xili_scanline_clip(src_x,      \
                       src_box_x2, \
                       &isx,       \
                       &src_x);    \
    \
    xili_scanline_clip(src_y,      \
                       src_box_y2, \
                       &isy,       \
                       &src_y);    \
    \
    fracx = (src_x - isx); \
    fracy = (src_y - isy); \
    \
    src_pixel = src_data +          \
                isy * src_sstride + \
                isx * src_pstride;  \
}

//
// Convert double fracs to ints to pick up a little speed; used for nearest only
//
#define XILI_SCANLINE_INIT_LINE \
XILI_SCANLINE_INIT_LINE_EXACT; \
int ifracx = _XILI_ROUND(fracx * XILI_GEOM_FRAC_MAX); \
int ifracy = _XILI_ROUND(fracy * XILI_GEOM_FRAC_MAX);

//
// Bit version of XILI_SCANLINE_INIT_LINE: fractions are calculated as
// doubles and the intial pixel address is NOT calculated.
//
#define XILI_SCANLINE_INIT_LINE_BIT \
int isx;     \
int isy;     \
double fracx; \
double fracy; \
{ \
    double src_x; \
    double src_y; \
    op->backwardMap(dst_box,                \
                    (double)dst_scan_start, \
                    (double)y,              \
                    src_box,                \
                    &src_x,                 \
                    &src_y);                \
    \
    xili_scanline_clip(src_x,      \
                       src_box_x2, \
                       &isx,       \
                       &src_x);    \
    fracx = src_x - (int)src_x; \
    \
    xili_scanline_clip(src_y,      \
                       src_box_y2, \
                       &isy,       \
                       &src_y);    \
    fracy = src_y - (int)src_y; \
    \
} \
Xil_unsigned8* src_scanline = src_data + \
    isy * src_sstride; \
int src_bit = src_offset + isx;

//
// Scanline walk: pixel level, nearest neighbor only
//
// Adjust the integral pixel address to that of the nearest neighbor based
// on the fractional image coordinates.
//
#define XILI_SCANLINE_GET_NEAREST(src_pixel, nearest_neighbor) \
    nearest_neighbor = src_pixel;

//
// Scanline walk: pixel level
//
// Increment the current source image position using delta values. The
// following may be understood by noting for example that ifracx < ifracdx1
// is TRUE if ifracx + ifracdx < XILI_GEOM_FRAC_MAX which is equivalent to
// the sum of the current fractional image x coordinate and the incremental
// change in the x coordinate being less than unity.
//
#define XILI_SCANLINE_SRC_INCREMENT \
    if(ifracx < ifracdx1) { \
        src_pixel += incx; \
        ifracx += ifracdx; \
    } else { \
        src_pixel += incx1; \
        ifracx -= ifracdx1; \
    } \
    \
    if(ifracy < ifracdy1) { \
        src_pixel += incy; \
        ifracy += ifracdy; \
    } else { \
        src_pixel += incy1; \
        ifracy -= ifracdy1; \
    }

//
// "Exact" version of XILI_SCANLINE_SRC_INCREMENT: doubles instead of ints
// are used to tack fractions which is necessary for accuracy in some cases.
// In this particular macro the difference is only in the variable names, e.g.,
// "fracx" instead of "ifracx", etc., where the initial 'i' is dropped.
//
#define XILI_SCANLINE_SRC_INCREMENT_EXACT \
    if(fracx < fracdx1) { \
        src_pixel += incx; \
        fracx += fracdx; \
    } else { \
        src_pixel += incx1; \
        fracx -= fracdx1; \
    } \
    \
    if(fracy < fracdy1) { \
        src_pixel += incy; \
        fracy += fracdy; \
    } else { \
        src_pixel += incy1; \
        fracy -= fracdy1; \
    }

//
// Bit version of XILI_SCANLINE_SRC_INCREMENT: uses doubles instead of ints
// to track fractional positions and tracks x and y positions separately as
// src_bit and src_scanline, respectively.
//
#define XILI_SCANLINE_SRC_INCREMENT_BIT \
    if (fracx < fracdx1) { \
        src_bit += incx; \
        fracx += fracdx; \
    } \
    else { \
        src_bit += incx1; \
        fracx -= fracdx1; \
    } \
    \
    if (fracy < fracdy1) { \
        src_scanline += incy; \
        fracy += fracdy; \
    } \
    else { \
        src_scanline += incy1; \
        fracy -= fracdy1; \
    }

//
// End of section related to scanline walk affine acceleration
//
#endif
