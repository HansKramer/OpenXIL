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
//  File:   Ycc601.cc
//  Project:    XIL
//  Revision:   1.4
//  Last Mod:   10:12:07, 03/10/00
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
//  COPYRIGHT
//------------------------------------------------------------------------
#pragma ident   "@(#)Ycc601.cc	1.4\t00/03/10  "

#include "color_convert.hh"

void
ycc601_to_rgblinear(Xil_signed16 *i_ptr,
                    Xil_signed16 *o_ptr,
                    unsigned int count,
                    unsigned int src_pixel_stride,
                    unsigned int dst_pixel_stride)
{
    //
    // Color-convert YCbCr601 to RGBlinear
    //

    for (unsigned int i = 0; i < count; i++) {
      float y6, cb6, cr6, r6, g6, b6, r7, g7, b7;                 
                                    
      _XILI_NORMALIZE_YCC601_S(*(i_ptr), *(i_ptr+1), *(i_ptr+2), 
                               &y6, &cb6, &cr6);
                                    
      _XILI_YCC601_TO_NL(y6, cb6, cr6, &r7, &g7, &b7);               
                                    
      _XILI_NL_TO_L(r7, g7, b7, &r6, &g6, &b6);

      *(o_ptr)     = _XILI_QUANTIZE_S(b6);
      *(o_ptr + 1) = _XILI_QUANTIZE_S(g6);
      *(o_ptr + 2) = _XILI_QUANTIZE_S(r6);

      i_ptr += src_pixel_stride;
      o_ptr += dst_pixel_stride;
    }
}

void
ycc601_to_photoycc(Xil_signed16 *i_ptr,
                   Xil_signed16 *o_ptr,
                   unsigned int count,
                   unsigned int src_pixel_stride,
                   unsigned int dst_pixel_stride)
{
    //
    // Color-convert YCbCr601 to PhotoYCC
    //

    for (unsigned int i = 0; i < count; i++) {
      float y, cb, cr;

      _XILI_NORMALIZE_YCC601_S(*(i_ptr), *(i_ptr+1), *(i_ptr+2), &y, &cb, &cr);

      _XILI_QUANTIZE_PHOTO_S(y, cb, cr, o_ptr, o_ptr+1, o_ptr+2);

      i_ptr += src_pixel_stride;
      o_ptr += dst_pixel_stride;
    }
}

void
ycc601_to_y709(Xil_signed16 *i_ptr,
                 Xil_signed16 *o_ptr,
                 unsigned int count,
                 unsigned int src_pixel_stride,
                 unsigned int dst_pixel_stride)
{
    //
    // Color-convert YCbCr601 to Y709
    //

    for (unsigned int i = 0; i < count; i++) {
      float y6, cb, cr, r, g, b, y7;

      _XILI_NORMALIZE_YCC601_S(*(i_ptr), *(i_ptr+1), *(i_ptr+2), 
                               &y6, &cb, &cr);

      _XILI_YCC601_TO_NL(y6, cb, cr, &r, &g, &b);

      y7 = _XILI_NL_TO_Y709(r, g, b); 

      *o_ptr = _XILI_QUANTIZE_Y709_S(y7);

      i_ptr += src_pixel_stride;
      o_ptr += dst_pixel_stride;
    }
}

void
ycc601_to_ycc709(Xil_signed16 *i_ptr,
                 Xil_signed16 *o_ptr,
                 unsigned int count,
                 unsigned int src_pixel_stride,
                 unsigned int dst_pixel_stride)
{
    //
    // Color-convert YCbCr601 to YCbCr709
    //

    for (unsigned int i = 0; i < count; i++) {
      float y6, cb6, cr6, red, green, blue, y7, cb7, cr7;

      _XILI_NORMALIZE_YCC601_S(*(i_ptr), *(i_ptr+1), *(i_ptr+2), 
                               &y6, &cb6, &cr6);

      _XILI_YCC601_TO_NL(y6, cb6, cr6, &red, &green, &blue);

      _XILI_NL_TO_YCC709(red, green, blue, &y7, &cb7, &cr7);

      _XILI_QUANTIZE_YCC709_S(y7, cb7, cr7, o_ptr, o_ptr+1, o_ptr+2);

      i_ptr += src_pixel_stride;
      o_ptr += dst_pixel_stride;
    }
}

void
ycc601_to_cmy(Xil_signed16 *i_ptr,
              Xil_signed16 *o_ptr,
              unsigned int count,
              unsigned int src_pixel_stride,
              unsigned int dst_pixel_stride)
{
    //
    // Color-convert YCbCr601 to CMY
    //

    for (unsigned int i = 0; i < count; i++) {
      float y6, cb6, cr6, r6, g6, b6, r7, g7, b7, c, m, y;
                                    
      _XILI_NORMALIZE_YCC601_S(*(i_ptr), *(i_ptr+1), *(i_ptr+2), 
                               &y6, &cb6, &cr6);
                                    
      _XILI_YCC601_TO_NL(y6, cb6, cr6, &r7, &g7, &b7);               
                                    
      _XILI_NL_TO_L(r7, g7, b7, &r6, &g6, &b6);

      _XILI_L_TO_CMY(r6, g6, b6, &c, &m, &y);

      *(o_ptr)     = _XILI_QUANTIZE_S(c);
      *(o_ptr + 1) = _XILI_QUANTIZE_S(m);
      *(o_ptr + 2) = _XILI_QUANTIZE_S(y);

      i_ptr += src_pixel_stride;
      o_ptr += dst_pixel_stride;
    }
}

void
ycc601_to_cmyk(Xil_signed16 *i_ptr,
               Xil_signed16 *o_ptr,
               unsigned int count,
               unsigned int src_pixel_stride,
               unsigned int dst_pixel_stride)
{
    //
    // Color-convert YCbCr601 to CMYK
    //

    for (unsigned int i = 0; i < count; i++) {
      float y6, cb6, cr6, r6, g6, b6, r7, g7, b7, c, m, y;
                                    
      _XILI_NORMALIZE_YCC601_S(*(i_ptr), *(i_ptr+1), *(i_ptr+2), 
                               &y6, &cb6, &cr6);
                                    
      _XILI_YCC601_TO_NL(y6, cb6, cr6, &r7, &g7, &b7);               
                                    
      _XILI_NL_TO_L(r7, g7, b7, &r6, &g6, &b6);

      _XILI_L_TO_CMY(r6, g6, b6, &c, &m, &y);

      *(o_ptr)     = _XILI_QUANTIZE_S(c);
      *(o_ptr + 1) = _XILI_QUANTIZE_S(m);
      *(o_ptr + 2) = _XILI_QUANTIZE_S(y);
      *(o_ptr + 3) = XIL_MINSHORT;

      i_ptr += src_pixel_stride;
      o_ptr += dst_pixel_stride;
    }
}

void
ycc601_to_ylinear(Xil_signed16 *i_ptr,
                  Xil_signed16 *o_ptr,
                  unsigned int count,
                  unsigned int src_pixel_stride,
                  unsigned int dst_pixel_stride)
{
    //
    // Color-convert YCbCr601 to ylinear
    //

    for (unsigned int i = 0; i < count; i++) {
      float y6, cb6, cr6, r6, g6, b6, r7, g7, b7, ylinear;
                                    
      _XILI_NORMALIZE_YCC601_S(*(i_ptr), *(i_ptr+1), *(i_ptr+2), 
                               &y6, &cb6, &cr6);
                                    
      _XILI_YCC601_TO_NL(y6, cb6, cr6, &r7, &g7, &b7);               
                                    
      _XILI_NL_TO_L(r7, g7, b7, &r6, &g6, &b6);

      ylinear = _XILI_L_TO_Ylinear(r6, g6, b6);

      *o_ptr = _XILI_QUANTIZE_S(ylinear);

      i_ptr += src_pixel_stride;
      o_ptr += dst_pixel_stride;
    }
}

void
ycc601_to_y601(Xil_signed16 *i_ptr,
               Xil_signed16 *o_ptr,
               unsigned int count,
               unsigned int src_pixel_stride,
               unsigned int dst_pixel_stride)
{
    //
    // Color-convert YCbCr601 to y601
    //

    for (unsigned int i = 0; i < count; i++) {
      *(o_ptr) = *(i_ptr);

      i_ptr += src_pixel_stride;
      o_ptr += dst_pixel_stride;
    }
}

void
ycc601_to_rgb709(Xil_signed16 *i_ptr,
               Xil_signed16 *o_ptr,
               unsigned int count,
               unsigned int src_pixel_stride,
               unsigned int dst_pixel_stride)
{
    //
    // Color-convert YCbCr601 to RGB709
    //

    for (unsigned int i = 0; i < count; i++) {
      float y6, cb6, cr6, r7, g7, b7;

      _XILI_NORMALIZE_YCC601_S(*(i_ptr), *(i_ptr + 1), *(i_ptr + 2), 
                               &y6, &cb6, &cr6);

      _XILI_YCC601_TO_NL(y6, cb6, cr6, &r7, &g7, &b7); 

      *o_ptr = _XILI_QUANTIZE_S(b7);
      *(o_ptr + 1) = _XILI_QUANTIZE_S(g7);
      *(o_ptr + 2) = _XILI_QUANTIZE_S(r7);

      i_ptr += src_pixel_stride;
      o_ptr += dst_pixel_stride;
    }
}

