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
//  File:	xili_interp_utils.hh
//  Project:	XIL
//  Revision:	1.9
//  Last Mod:	10:24:02, 03/10/00
//
//  Description:
//	
//	 Utilities related to interpolation.
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

#ifndef _XILI_INTERP_UTILS_HH
#define _XILI_INTERP_UTILS_HH

#pragma ident  "@(#)xili_interp_utils.hh	1.9\t00/03/10  "

#ifdef DEBUG
//
//  For XIL datatype definitions
//
#ifdef _XIL_LIBXIL_PRIVATE
#include "_XilDefines.h"
#else
#include <xil/xilGPI.hh>
#endif

#include "xili_geom_utils.hh"
#include "XiliUtils.hh"
#endif

//
// Begin basic utility macro definitions
//

#define XILI_GEOM_BLEND(P, P0, P1, ALPHA) \
        P = P0 + ((P1-P0)*ALPHA);
#define XILI_GEOM_PERTURB(P, Q, ALPHA) \
        P += ((Q)*ALPHA);


#define DEBUG_BILINEAR 0 // TODO  bpb  03/21/1997  get rid of this stuff.
//
// Begin linear interpolation overloaded inline functions
//
inline
Xil_unsigned8
xili_interp_linear(Xil_unsigned8* src_pixel,
                   unsigned int stride,
                   float frac)
{
#if DEBUG_BILINEAR
if(frac < 0.0F || frac > 1.0F) printf("frac = %f\n", frac);
#endif
    float p0 = _XILI_B2F(*src_pixel);
    float p1 = _XILI_B2F(*(src_pixel + stride));
    return _XILI_ROUND_U8(p0 + (p1 - p0)*frac);
}

inline
Xil_signed16
xili_interp_linear(Xil_signed16* src_pixel,
                   unsigned int stride,
                   float frac)
{
#if DEBUG_BILINEAR
if(frac < 0.0F || frac > 1.0F) printf("frac = %f\n", frac);
#endif
    float p0 = *src_pixel;
    float p1 = *(src_pixel + stride);
    return _XILI_ROUND_S16(p0 + (p1 - p0)*frac);
}

inline
Xil_float32
xili_interp_linear(Xil_float32* src_pixel,
                   unsigned int stride,
                   float frac)
{
#if DEBUG_BILINEAR
if(frac < 0.0F || frac > 1.0F) printf("frac = %f\n", frac);
#endif
    float p0 = *src_pixel;
    float p1 = *(src_pixel + stride);
    return (Xil_float32)(p0 + (p1 - p0)*frac);
}


//
// Begin cubic interpolation inline functions for bit data
//
//
// Note that this cubic interpolation function expects that the bit buffer
// "src_pixel" points to the preceeding line in the source, i.e., an address
// one scanline stride less than the source position representing the backward
// mapped location of the destination bit the value of which will be set on
// the basis of the interpolation.
//
inline
void
xili_interp_cubic_bit_vertical(Xil_unsigned8* src_pixel,
                               unsigned int src_scanline_stride,
                               unsigned int src_bit,
                               float fracy,
                               Xil_unsigned8* dst_scanline,
                               unsigned int dst_bit)
{
    float p_ = (float)XIL_BMAP_TST(src_pixel, src_bit);
    float p0 = (float)XIL_BMAP_TST(src_pixel += src_scanline_stride, src_bit);
    float p1 = (float)XIL_BMAP_TST(src_pixel += src_scanline_stride, src_bit);
    float p2 = (float)XIL_BMAP_TST(src_pixel +  src_scanline_stride, src_bit);

    float p;
    XILI_GEOM_BLEND(p, p0, p1, fracy);
    float q;
    XILI_GEOM_BLEND(q, (p1 + p_), (p2 + p0), fracy);

    q = p - (q / 2.0F);
    XILI_GEOM_PERTURB(p, q, (fracy * (1.0F - fracy)));

    if(p >= 0.5F) {
        XIL_BMAP_SET(dst_scanline, dst_bit);
    } else {
        XIL_BMAP_CLR(dst_scanline, dst_bit);
    }
}

//
// Note that this cubic interpolation function expects that the bit index
// "src_bit" indicates the preceeding bit in the source, i.e., the bit which
// is a single bit less than the source bit representing the backward
// mapped location of the destination bit the value of which will be set on
// the basis of the interpolation.
//
inline
void
xili_interp_cubic_bit_horizontal(Xil_unsigned8* src_pixel,
                                 unsigned int src_bit,
                                 float fracx,
                                 Xil_unsigned8* dst_scanline,
                                 unsigned int dst_bit)
{
    float p_ = (float)XIL_BMAP_TST(src_pixel, src_bit++);
    float p0 = (float)XIL_BMAP_TST(src_pixel, src_bit++);
    float p1 = (float)XIL_BMAP_TST(src_pixel, src_bit++);
    float p2 = (float)XIL_BMAP_TST(src_pixel, src_bit);

    float p;
    XILI_GEOM_BLEND(p, p0, p1, fracx);
    float q;
    XILI_GEOM_BLEND(q, (p1 + p_), (p2 + p0), fracx);

    q = p - (q / 2.0F);
    XILI_GEOM_PERTURB(p, q, (fracx * (1.0F - fracx)));

    if(p >= 0.5F) {
        XIL_BMAP_SET(dst_scanline, dst_bit);
    } else {
        XIL_BMAP_CLR(dst_scanline, dst_bit);
    }
}


//
// Begin cubic interpolation overloaded inline functions
//
inline
Xil_unsigned8
xili_interp_cubic(Xil_unsigned8* src_pixel,
                  unsigned int stride,
                  float frac)
{
    float p_ = _XILI_B2F(*(src_pixel -  stride));
    float p0 = _XILI_B2F(*src_pixel);
    float p1 = _XILI_B2F(*(src_pixel += stride));
    float p2 = _XILI_B2F(*(src_pixel +  stride));

    float p;
    XILI_GEOM_BLEND(p, p0, p1, frac);
    float q;
    XILI_GEOM_BLEND(q, (p1 + p_), (p2 + p0), frac);

    q = p - (q / 2.0F);
    XILI_GEOM_PERTURB(p, q, (frac * (1.0F - frac)));

    return _XILI_ROUND_U8(p);
}

inline
Xil_signed16
xili_interp_cubic(Xil_signed16* src_pixel,
                  unsigned int stride,
                  float frac)
{
    float p_ = (float)(*(src_pixel -  stride));
    float p0 = (float)(*src_pixel);
    float p1 = (float)(*(src_pixel += stride));
    float p2 = (float)(*(src_pixel +  stride));

    float p;
    XILI_GEOM_BLEND(p, p0, p1, frac);
    float q;
    XILI_GEOM_BLEND(q, (p1 + p_), (p2 + p0), frac);

    q = p - (q / 2.0F);
    XILI_GEOM_PERTURB(p, q, (frac * (1.0F - frac)));

    return _XILI_ROUND_S16(p);
}

inline
Xil_float32
xili_interp_cubic(Xil_float32* src_pixel,
                  unsigned int stride,
                  float frac)
{
    float p_ = *(src_pixel -  stride);
    float p0 = *src_pixel;
    float p1 = *(src_pixel += stride);
    float p2 = *(src_pixel +  stride);

    float p;
    XILI_GEOM_BLEND(p, p0, p1, frac);
    float q;
    XILI_GEOM_BLEND(q, (p1 + p_), (p2 + p0), frac);

    q = p - (q / 2.0F);
    XILI_GEOM_PERTURB(p, q, (frac * (1.0F - frac)));

    return (Xil_float32)p;
}


//
// Begin overloaded inline bilinear interpolation functions.
//
inline
void
xili_interp_bilinear(Xil_unsigned8* src_pixel,
                     unsigned int src_scanline_stride,
                     unsigned int pixel_offset,
                     float fracx,
                     float fracy,
                     Xil_unsigned8* dst_scanline,
                     unsigned int dst_bit)
{
    unsigned int pixel_offset_1 = pixel_offset + 1;
    float p00 = (float)XIL_BMAP_TST(src_pixel, pixel_offset);
    float p10 = (float)XIL_BMAP_TST(src_pixel, pixel_offset_1);
    float p01 = (float)XIL_BMAP_TST(src_pixel += src_scanline_stride,
                                    pixel_offset);
    float p11 = (float)XIL_BMAP_TST(src_pixel, pixel_offset_1);

    float p0;
    XILI_GEOM_BLEND(p0, p00, p10, fracx);
    float p1;
    XILI_GEOM_BLEND(p1, p01, p11, fracx);
    float p;
    XILI_GEOM_BLEND(p, p0, p1, fracy);

    if(p < 0.5F) {
        XIL_BMAP_CLR(dst_scanline, dst_bit);
    } else {
        XIL_BMAP_SET(dst_scanline, dst_bit);
    }
}

inline
Xil_unsigned8
xili_interp_bilinear(Xil_unsigned8* src_pixel,
                     unsigned int src_pixel_stride,
                     unsigned int src_scanline_stride,
                     float fracx,
                     float fracy)
{
#if DEBUG_BILINEAR
if(fracx < 0.0F || fracx > 1.0F) printf("fracx = %f\n", fracx);
if(fracy < 0.0F || fracy > 1.0F) printf("fracy = %f\n", fracy);
#endif
    float p00 = _XILI_B2F(*(src_pixel));
    float p10 = _XILI_B2F(*(src_pixel + src_pixel_stride));
    float p01 = _XILI_B2F(*(src_pixel += src_scanline_stride));
    float p11 = _XILI_B2F(*(src_pixel + src_pixel_stride));

    float p0;
    XILI_GEOM_BLEND(p0, p00, p10, fracx);
    float p1;
    XILI_GEOM_BLEND(p1, p01, p11, fracx);
    float p;
    XILI_GEOM_BLEND(p, p0, p1, fracy);

    return _XILI_ROUND_U8(p);
}

inline
Xil_signed16
xili_interp_bilinear(Xil_signed16* src_pixel,
                     unsigned int src_pixel_stride,
                     unsigned int src_scanline_stride,
                     float fracx,
                     float fracy)
{
#if DEBUG_BILINEAR
if(fracx < 0.0F || fracx > 1.0F) printf("fracx = %f\n", fracx);
if(fracy < 0.0F || fracy > 1.0F) printf("fracy = %f\n", fracy);
#endif
    float p00 = (float)(*(src_pixel));
    float p10 = (float)(*(src_pixel + src_pixel_stride));
    float p01 = (float)(*(src_pixel += src_scanline_stride));
    float p11 = (float)(*(src_pixel + src_pixel_stride));

    float p0;
    XILI_GEOM_BLEND(p0, p00, p10, fracx);
    float p1;
    XILI_GEOM_BLEND(p1, p01, p11, fracx);
    float p;
    XILI_GEOM_BLEND(p, p0, p1, fracy);

    return _XILI_ROUND_S16(p);
}

inline
Xil_float32
xili_interp_bilinear(Xil_float32* src_pixel,
                     unsigned int src_pixel_stride,
                     unsigned int src_scanline_stride,
                     float fracx,
                     float fracy)
{
#if DEBUG_BILINEAR
if(fracx < 0.0F || fracx > 1.0F) printf("fracx = %f\n", fracx);
if(fracy < 0.0F || fracy > 1.0F) printf("fracy = %f\n", fracy);
#endif
    float p00 = (*(src_pixel));
    float p10 = (*(src_pixel + src_pixel_stride));
    float p01 = (*(src_pixel += src_scanline_stride));
    float p11 = (*(src_pixel + src_pixel_stride));

    float p0;
    XILI_GEOM_BLEND(p0, p00, p10, fracx);
    float p1;
    XILI_GEOM_BLEND(p1, p01, p11, fracx);
    float p;
    XILI_GEOM_BLEND(p, p0, p1, fracy);

    return (Xil_float32)(p);
}


//
// Begin bicubic interpolation macros.
//

//
// Macro which defines the actual interpolation algorithm. The values to be
// interpolated are passed in row order.
//
#define XILI_BICUBIC_COMPUTE(dst_value) \
{ \
    float p_, p0, p1, p2; \
    float float_fracx = (float)fracx; \
    XILI_GEOM_BLEND(p0, p00, p10, float_fracx); \
    XILI_GEOM_BLEND(p1, p01, p11, float_fracx); \
    XILI_GEOM_BLEND(p_, p0_, p1_, float_fracx); \
    XILI_GEOM_BLEND(p2, p02, p12, float_fracx); \
    \
    float q_, q0, q1, q2; \
    XILI_GEOM_BLEND(q_, (p1_ + p__), (p2_ + p0_), float_fracx); \
    XILI_GEOM_BLEND(q0, (p10 + p_0), (p20 + p00), float_fracx); \
    XILI_GEOM_BLEND(q1, (p11 + p_1), (p21 + p01), float_fracx); \
    XILI_GEOM_BLEND(q2, (p12 + p_2), (p22 + p02), float_fracx); \
    \
    q_ = p_ - q_ / 2.0F; \
    q0 = p0 - q0 / 2.0F; \
    q1 = p1 - q1 / 2.0F; \
    q2 = p2 - q2 / 2.0F; \
    \
    float xx = float_fracx * (1.0F - float_fracx); \
    \
    XILI_GEOM_PERTURB(p_, q_, xx); \
    XILI_GEOM_PERTURB(p0, q0, xx); \
    XILI_GEOM_PERTURB(p1, q1, xx); \
    XILI_GEOM_PERTURB(p2, q2, xx); \
    \
    float p, q; \
    float float_fracy = (float)fracy; \
    XILI_GEOM_BLEND(p, p0, p1, float_fracy); \
    XILI_GEOM_BLEND(q, (p1 + p_), (p2 + p0), float_fracy); \
    \
    q = p - q/2.0F; \
    XILI_GEOM_PERTURB(p, q, (fracy * (1.0F - float_fracy))); \
    \
    (dst_value) = p; \
}

//
// Note that this bicubic interpolation macro, unlike the equivalents for
// byte, short, and float data, expects that the source position indicated
// by the source buffer address "src_pixel" and source bit index "pixel_offset"
// will be one stride in each dimension less than the source position to which
// the target destination pixel is backward mapped. This means that src_pixel
// should be one scanline stride less than the backward mapped position and
// that pixel_offset should be one less than the backward mapped position.
//
#define XILI_INTERP_BICUBIC_BIT\
(src_pixel,src_sstride,pixel_offset,fracx,fracy,dst_scanline,dst_bit) \
{ \
    Xil_unsigned8* src_row = src_pixel; \
    \
    float p__ = (float)XIL_BMAP_TST(src_row, pixel_offset); \
    float p0_ = (float)XIL_BMAP_TST(src_row, pixel_offset + 1); \
    float p1_ = (float)XIL_BMAP_TST(src_row, pixel_offset + 2); \
    float p2_ = (float)XIL_BMAP_TST(src_row, pixel_offset + 3); \
    float p_0 = (float)XIL_BMAP_TST(src_row += src_sstride, pixel_offset); \
    float p00 = (float)XIL_BMAP_TST(src_row, pixel_offset + 1); \
    float p10 = (float)XIL_BMAP_TST(src_row, pixel_offset + 2); \
    float p20 = (float)XIL_BMAP_TST(src_row, pixel_offset + 3); \
    float p_1 = (float)XIL_BMAP_TST(src_row += src_sstride, pixel_offset); \
    float p01 = (float)XIL_BMAP_TST(src_row, pixel_offset + 1); \
    float p11 = (float)XIL_BMAP_TST(src_row, pixel_offset + 2); \
    float p21 = (float)XIL_BMAP_TST(src_row, pixel_offset + 3); \
    float p_2 = (float)XIL_BMAP_TST(src_row += src_sstride, pixel_offset); \
    float p02 = (float)XIL_BMAP_TST(src_row, pixel_offset + 1); \
    float p12 = (float)XIL_BMAP_TST(src_row, pixel_offset + 2); \
    float p22 = (float)XIL_BMAP_TST(src_row, pixel_offset + 3); \
    \
    float dst_value; \
    XILI_BICUBIC_COMPUTE(dst_value); \
    \
    if(dst_value< 0.5F) { \
        XIL_BMAP_CLR((dst_scanline), (dst_bit)); \
    } else { \
        XIL_BMAP_SET((dst_scanline), (dst_bit)); \
    } \
}

#define XILI_INTERP_BICUBIC_BYTE\
(src_pixel,src_sstride,src_pstride,fracx,fracy,dst_pixel) \
{ \
    Xil_unsigned8* src_row = src_pixel - src_sstride - src_pstride; \
    Xil_unsigned8* src_col = src_row; \
    \
    float p__ = _XILI_B2F(*(src_col)); \
    float p0_ = _XILI_B2F(*(src_col += src_pstride)); \
    float p1_ = _XILI_B2F(*(src_col += src_pstride)); \
    float p2_ = _XILI_B2F(*(src_col += src_pstride)); \
    float p_0 = _XILI_B2F(*(src_col = (src_row += src_sstride))); \
    float p00 = _XILI_B2F(*(src_col += src_pstride)); \
    float p10 = _XILI_B2F(*(src_col += src_pstride)); \
    float p20 = _XILI_B2F(*(src_col += src_pstride)); \
    float p_1 = _XILI_B2F(*(src_col = (src_row += src_sstride))); \
    float p01 = _XILI_B2F(*(src_col += src_pstride)); \
    float p11 = _XILI_B2F(*(src_col += src_pstride)); \
    float p21 = _XILI_B2F(*(src_col += src_pstride)); \
    float p_2 = _XILI_B2F(*(src_col = (src_row += src_sstride))); \
    float p02 = _XILI_B2F(*(src_col += src_pstride)); \
    float p12 = _XILI_B2F(*(src_col += src_pstride)); \
    float p22 = _XILI_B2F(*(src_col += src_pstride)); \
    \
    float dst_value; \
    XILI_BICUBIC_COMPUTE(dst_value); \
    \
    *(dst_pixel) = _XILI_ROUND_U8(dst_value); \
}

#define XILI_INTERP_BICUBIC_SHORT\
(src_pixel,src_sstride,src_pstride,fracx,fracy,dst_pixel) \
{ \
    Xil_signed16* src_row = src_pixel - src_sstride - src_pstride; \
    Xil_signed16* src_col = src_row; \
    \
    float p__ = (float)(*(src_col)); \
    float p0_ = (float)(*(src_col += src_pstride)); \
    float p1_ = (float)(*(src_col += src_pstride)); \
    float p2_ = (float)(*(src_col += src_pstride)); \
    float p_0 = (float)(*(src_col = (src_row += src_sstride))); \
    float p00 = (float)(*(src_col += src_pstride)); \
    float p10 = (float)(*(src_col += src_pstride)); \
    float p20 = (float)(*(src_col += src_pstride)); \
    float p_1 = (float)(*(src_col = (src_row += src_sstride))); \
    float p01 = (float)(*(src_col += src_pstride)); \
    float p11 = (float)(*(src_col += src_pstride)); \
    float p21 = (float)(*(src_col += src_pstride)); \
    float p_2 = (float)(*(src_col = (src_row += src_sstride))); \
    float p02 = (float)(*(src_col += src_pstride)); \
    float p12 = (float)(*(src_col += src_pstride)); \
    float p22 = (float)(*(src_col += src_pstride)); \
    \
    float dst_value; \
    XILI_BICUBIC_COMPUTE(dst_value); \
    \
    *(dst_pixel) = _XILI_ROUND_S16(dst_value); \
}

#define XILI_INTERP_BICUBIC_FLOAT\
(src_pixel,src_sstride,src_pstride,fracx,fracy,dst_pixel) \
{ \
    Xil_float32* src_row = src_pixel - src_sstride - src_pstride; \
    Xil_float32* src_col = src_row; \
    \
    float p__ = (float)(*(src_col)); \
    float p0_ = (float)(*(src_col += src_pstride)); \
    float p1_ = (float)(*(src_col += src_pstride)); \
    float p2_ = (float)(*(src_col += src_pstride)); \
    float p_0 = (float)(*(src_col = (src_row += src_sstride))); \
    float p00 = (float)(*(src_col += src_pstride)); \
    float p10 = (float)(*(src_col += src_pstride)); \
    float p20 = (float)(*(src_col += src_pstride)); \
    float p_1 = (float)(*(src_col = (src_row += src_sstride))); \
    float p01 = (float)(*(src_col += src_pstride)); \
    float p11 = (float)(*(src_col += src_pstride)); \
    float p21 = (float)(*(src_col += src_pstride)); \
    float p_2 = (float)(*(src_col = (src_row += src_sstride))); \
    float p02 = (float)(*(src_col += src_pstride)); \
    float p12 = (float)(*(src_col += src_pstride)); \
    float p22 = (float)(*(src_col += src_pstride)); \
    \
    XILI_BICUBIC_COMPUTE(*dst_pixel); \
}


//
// Begin general interpolation macros.
//

#define XILI_INTERP_GENERAL_BIT(\
src_pixel,src_row_stride,src_offset,\
kernel_v,kernel_h,kernel_size_v,kernel_size_h,\
dst_scanline,dst_bit) \
{ \
    Xil_unsigned8* src_row = (src_pixel); \
    \
    float sum = 0.0F; \
    for(unsigned int row_num = 0; row_num < kernel_size_v; row_num++) { \
        float sum_row = 0.0F; \
        for(unsigned int col_num = 0; col_num < kernel_size_h; col_num++) { \
            if(XIL_BMAP_TST(src_row, src_offset + col_num) == TRUE) \
                sum_row += kernel_h[col_num]; \
        } \
        sum += (kernel_v[row_num]*sum_row); \
        src_row += src_row_stride; \
    } \
    \
    if(sum < 0.5F) { \
        XIL_BMAP_CLR((dst_scanline), (dst_bit)); \
    } else { \
        XIL_BMAP_SET((dst_scanline), (dst_bit)); \
    } \
}

#define XILI_INTERP_GENERAL_BYTE(\
src_pixel,src_row_stride,src_col_stride,\
kernel_v,kernel_h,kernel_size_v,kernel_size_h,\
dst_pixel) \
{ \
    Xil_unsigned8* src_row = (src_pixel); \
    \
    float sum = 0.0F; \
    for(unsigned int row_num = 0; row_num < kernel_size_v; row_num++) { \
        Xil_unsigned8* src_col = src_row; \
        float sum_row = 0.0F; \
        for(unsigned int col_num = 0; col_num < kernel_size_h; col_num++) { \
            sum_row += (kernel_h[col_num] * _XILI_B2F(*src_col)); \
            src_col += src_col_stride; \
        } \
        sum += (kernel_v[row_num]*sum_row); \
        src_row += src_row_stride; \
    } \
    \
    *(dst_pixel) = _XILI_ROUND_U8(sum); \
}

#define XILI_INTERP_GENERAL_SHORT(\
src_pixel,src_row_stride,src_col_stride,\
kernel_v,kernel_h,kernel_size_v,kernel_size_h,\
dst_pixel) \
{ \
    Xil_signed16* src_row = (src_pixel); \
    \
    float sum = 0.0F; \
    for(unsigned int row_num = 0; row_num < kernel_size_v; row_num++) { \
        Xil_signed16* src_col = src_row; \
        float sum_row = 0.0F; \
        for(unsigned int col_num = 0; col_num < kernel_size_h; col_num++) { \
            sum_row += (kernel_h[col_num] * ((float)(*src_col))); \
            src_col += src_col_stride; \
        } \
        sum += (kernel_v[row_num]*sum_row); \
        src_row += src_row_stride; \
    } \
    \
    *(dst_pixel) = _XILI_ROUND_S16(sum); \
}

#define XILI_INTERP_GENERAL_FLOAT(\
src_pixel,src_row_stride,src_col_stride,\
kernel_v,kernel_h,kernel_size_v,kernel_size_h,\
dst_pixel) \
{ \
    Xil_float32* src_row = (src_pixel); \
    \
    float sum = 0.0F; \
    for(unsigned int row_num = 0; row_num < kernel_size_v; row_num++) { \
        Xil_float32* src_col = src_row; \
        float sum_row = 0.0F; \
        for(unsigned int col_num = 0; col_num < kernel_size_h; col_num++) { \
            sum_row += (kernel_h[col_num] * ((float)(*src_col))); \
            src_col += src_col_stride; \
        } \
        sum += (kernel_v[row_num]*sum_row); \
        src_row += src_row_stride; \
    } \
    \
    *(dst_pixel) = (Xil_float32)sum; \
}
#endif
