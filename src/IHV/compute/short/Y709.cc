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
//  File:   Y709.cc
//  Project:    XIL
//  Revision:   1.4
//  Last Mod:   10:12:08, 03/10/00
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
#pragma ident   "@(#)Y709.cc	1.4\t00/03/10  "

#include "color_convert.hh"

void
y709_to_rgblinear(Xil_signed16 *i_ptr,
                  Xil_signed16 *o_ptr,
                  unsigned int count,
                  unsigned int src_pixel_stride,
                  unsigned int dst_pixel_stride)
{
    //
    // Color-convert Y709 to RGBlinear
    //

    for (unsigned int i = 0; i < count; i++) {
      float y, r6;                                
                                    
      y = _XILI_NORMALIZE_Y709_S(*(i_ptr));                     

      r6 = _XILI_NL_TO_L_1(y);                     

      *(o_ptr + 2) = _XILI_QUANTIZE_S(r6);

      *(o_ptr + 1) = *(o_ptr + 2);                        
      *(o_ptr)     = *(o_ptr + 2);                        

      i_ptr += src_pixel_stride;
      o_ptr += dst_pixel_stride;
    }
}

void
y709_to_photoycc(Xil_signed16 *i_ptr,
                 Xil_signed16 *o_ptr,
                 unsigned int count,
                 unsigned int src_pixel_stride,
                 unsigned int dst_pixel_stride)
{
    //
    // Color-convert Y709 to PhotoYCC
    //

    for (unsigned int i = 0; i < count; i++) {
      float y;

      y = _XILI_NORMALIZE_Y709_S(*(i_ptr));

      _XILI_QUANTIZE_PHOTO_S(y, 0.0, 0.0, o_ptr, o_ptr+1, o_ptr+2);

      i_ptr += src_pixel_stride;
      o_ptr += dst_pixel_stride;
    }
}

void
y709_to_ycc601(Xil_signed16 *i_ptr,
               Xil_signed16 *o_ptr,
               unsigned int count,
               unsigned int src_pixel_stride,
               unsigned int dst_pixel_stride)
{
    //
    // Color-convert Y709 to YCbCr601
    //

    for (unsigned int i = 0; i < count; i++) {
      float y;

      y = _XILI_NORMALIZE_Y709_S(*(i_ptr));

      _XILI_QUANTIZE_YCC601_S(y, 0.0, 0.0, o_ptr, o_ptr+1, o_ptr+2);

      i_ptr += src_pixel_stride;
      o_ptr += dst_pixel_stride;
    }
}

void
y709_to_ycc709(Xil_signed16 *i_ptr,
               Xil_signed16 *o_ptr,
               unsigned int count,
               unsigned int src_pixel_stride,
               unsigned int dst_pixel_stride)
{
    //
    // Color-convert Y709 to YCbCr709
    //

    for (unsigned int i = 0; i < count; i++) {
      float y;

      y = _XILI_NORMALIZE_Y709_S(*(i_ptr));
      
      _XILI_QUANTIZE_YCC709_S(y, 0.0, 0.0, o_ptr, o_ptr+1, o_ptr+2);

      i_ptr += src_pixel_stride;
      o_ptr += dst_pixel_stride;
    }
}

void
y709_to_cmy(Xil_signed16 *i_ptr,
            Xil_signed16 *o_ptr,
            unsigned int count,
            unsigned int src_pixel_stride,
            unsigned int dst_pixel_stride)
{
    //
    // Color-convert Y709 to CMY
    //

    for (unsigned int i = 0; i < count; i++) {
      float y7, r6, g6, b6, c, m, y;

      y7 = _XILI_NORMALIZE_Y709_S(*(i_ptr));
      
      _XILI_NL_TO_L(y7, y7, y7, &r6, &g6, &b6);

      _XILI_L_TO_CMY(r6, g6, b6, &c, &m, &y);

      *o_ptr       = _XILI_QUANTIZE_S(c);
      *(o_ptr + 1) = _XILI_QUANTIZE_S(m);
      *(o_ptr + 2) = _XILI_QUANTIZE_S(y);
      
      i_ptr += src_pixel_stride;
      o_ptr += dst_pixel_stride;
    }
}

void
y709_to_cmyk(Xil_signed16 *i_ptr,
             Xil_signed16 *o_ptr,
             unsigned int count,
             unsigned int src_pixel_stride,
             unsigned int dst_pixel_stride)
{
    //
    // Color-convert Y709 to CMYK
    //

    for (unsigned int i = 0; i < count; i++) {
      float y7, r6, g6, b6, c, m, y;

      y7 = _XILI_NORMALIZE_Y709_S(*(i_ptr));
      
      _XILI_NL_TO_L(y7, y7, y7, &r6, &g6, &b6);

      _XILI_L_TO_CMY(r6, g6, b6, &c, &m, &y);

      *o_ptr       = _XILI_QUANTIZE_S(c);
      *(o_ptr + 1) = _XILI_QUANTIZE_S(m);
      *(o_ptr + 2) = _XILI_QUANTIZE_S(y);
      *(o_ptr + 3) = XIL_MINSHORT;
      
      i_ptr += src_pixel_stride;
      o_ptr += dst_pixel_stride;
    }
}

void
y709_to_ylinear(Xil_signed16 *i_ptr,
                Xil_signed16 *o_ptr,
                unsigned int count,
                unsigned int src_pixel_stride,
                unsigned int dst_pixel_stride)
{
    //
    // Color-convert Y709 to ylinear
    //

    for (unsigned int i = 0; i < count; i++) {
      float y7, r6, g6, b6, r7, g7, b7, ylinear;

      y7 = _XILI_NORMALIZE_Y709_S(*(i_ptr));
      
      _XILI_Y709_TO_NL(y7, &r7, &g7, &b7);

      _XILI_NL_TO_L(r7, g7, b7, &r6, &g6, &b6);

      ylinear = _XILI_L_TO_Ylinear(r6, g6, b6);

      *o_ptr = _XILI_QUANTIZE_S(ylinear);

      i_ptr += src_pixel_stride;
      o_ptr += dst_pixel_stride;
    }
}

void
y709_to_y601(Xil_signed16 *i_ptr,
             Xil_signed16 *o_ptr,
             unsigned int count,
             unsigned int src_pixel_stride,
             unsigned int dst_pixel_stride)
{
    //
    // Color-convert Y709 to y601
    //

    for (unsigned int i = 0; i < count; i++) {
      *o_ptr = *(i_ptr);

      i_ptr += src_pixel_stride;
      o_ptr += dst_pixel_stride;
    }
}

void
y709_to_rgb709(Xil_signed16 *i_ptr,
               Xil_signed16 *o_ptr,
               unsigned int count,
               unsigned int src_pixel_stride,
               unsigned int dst_pixel_stride)
{
    //
    // Color-convert Y709 to RGB709
    //

    for (unsigned int i = 0; i < count; i++) {
      float y;

      y = _XILI_NORMALIZE_Y709_S(*(i_ptr));
      
      *o_ptr = _XILI_QUANTIZE_S(y);

      *(o_ptr + 1) = *(o_ptr);
      *(o_ptr + 2) = *(o_ptr);

      i_ptr += src_pixel_stride;
      o_ptr += dst_pixel_stride;
    }
}

