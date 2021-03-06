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
//  File:   Rgblinear.cc
//  Project:    XIL
//  Revision:   1.4
//  Last Mod:   10:12:12, 03/10/00
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
#pragma ident   "@(#)Rgblinear.cc	1.4\t00/03/10  "

#include "color_convert.hh"

void
rgblinear_to_y709(Xil_signed16 *i_ptr,
                  Xil_signed16 *o_ptr,
                  unsigned int count,
                  unsigned int src_pixel_stride,
                  unsigned int dst_pixel_stride)
{
    //
    // Color-convert RGBlinear to Y709
    //

    for (unsigned int i = 0; i < count; i++) {
      float r6, g6, b6, r7, g7, b7, y;

      b6 = _XILI_NORMALIZE_S(*(i_ptr));
      g6 = _XILI_NORMALIZE_S(*(i_ptr + 1));
      r6 = _XILI_NORMALIZE_S(*(i_ptr + 2));

      _XILI_L_TO_NL(r6, g6, b6, &r7, &g7, &b7);

      y = _XILI_NL_TO_Y709(r7, g7, b7);

      *o_ptr = _XILI_QUANTIZE_Y709_S(y);
                                    
      i_ptr += src_pixel_stride;
      o_ptr += dst_pixel_stride;
    }
}

void
rgblinear_to_photoycc(Xil_signed16 *i_ptr,
                      Xil_signed16 *o_ptr,
                      unsigned int count,
                      unsigned int src_pixel_stride,
                      unsigned int dst_pixel_stride)
{
    //
    // Color-convert RGBlinear to PhotoYCC
    //

    for (unsigned int i = 0; i < count; i++) {
      float r6, g6, b6, r7, g7, b7, y, cb, cr;

      b6 = _XILI_NORMALIZE_S(*(i_ptr));
      g6 = _XILI_NORMALIZE_S(*(i_ptr + 1));
      r6 = _XILI_NORMALIZE_S(*(i_ptr + 2));

      _XILI_L_TO_NL(r6, g6, b6, &r7, &g7, &b7);

      _XILI_NL_TO_YCC601(r7, g7, b7, &y, &cb, &cr);

      _XILI_QUANTIZE_PHOTO_S(y, cb, cr, o_ptr, o_ptr+1, o_ptr+2);
                                    
      i_ptr += src_pixel_stride;
      o_ptr += dst_pixel_stride;
    }
}

void
rgblinear_to_ycc601(Xil_signed16 *i_ptr,
                    Xil_signed16 *o_ptr,
                    unsigned int count,
                    unsigned int src_pixel_stride,
                    unsigned int dst_pixel_stride)
{
    //
    // Color-convert RGBlinear to YCbCr601
    //

    for (unsigned int i = 0; i < count; i++) {
      float r6, g6, b6, r7, g7, b7, y, cb, cr;

      b6 = _XILI_NORMALIZE_S(*(i_ptr));
      g6 = _XILI_NORMALIZE_S(*(i_ptr + 1));
      r6 = _XILI_NORMALIZE_S(*(i_ptr + 2));

      _XILI_L_TO_NL(r6, g6, b6, &r7, &g7, &b7);

      _XILI_NL_TO_YCC601(r7, g7, b7, &y, &cb, &cr);

      _XILI_QUANTIZE_YCC601_S(y, cb, cr, o_ptr, o_ptr+1, o_ptr+2);
                                    
      i_ptr += src_pixel_stride;
      o_ptr += dst_pixel_stride;
    }
}

void
rgblinear_to_ycc709(Xil_signed16 *i_ptr,
                    Xil_signed16 *o_ptr,
                    unsigned int count,
                    unsigned int src_pixel_stride,
                    unsigned int dst_pixel_stride)
{
    //
    // Color-convert RGBlinear to YCbCr709
    //

    for (unsigned int i = 0; i < count; i++) {
      float r6, g6, b6, r7, g7, b7, y, cb, cr;

      b6 = _XILI_NORMALIZE_S(*(i_ptr));
      g6 = _XILI_NORMALIZE_S(*(i_ptr + 1));
      r6 = _XILI_NORMALIZE_S(*(i_ptr + 2));

      _XILI_L_TO_NL(r6, g6, b6, &r7, &g7, &b7);

      _XILI_NL_TO_YCC709(r7, g7, b7, &y, &cb, &cr);

      _XILI_QUANTIZE_YCC709_S(y, cb, cr, o_ptr, o_ptr+1, o_ptr+2);
                                    
      i_ptr += src_pixel_stride;
      o_ptr += dst_pixel_stride;
    }
}

void
rgblinear_to_cmy(Xil_signed16 *i_ptr,
                 Xil_signed16 *o_ptr,
                 unsigned int count,
                 unsigned int src_pixel_stride,
                 unsigned int dst_pixel_stride)
{
    //
    // Color-convert RGBlinear to CMY
    //

    for (unsigned int i = 0; i < count; i++) {
      *o_ptr       = XIL_MAXSHORT - *(i_ptr + 2) + XIL_MINSHORT;              
      *(o_ptr + 1) = XIL_MAXSHORT - *(i_ptr + 1) + XIL_MINSHORT;            
      *(o_ptr + 2) = XIL_MAXSHORT - *(i_ptr) + XIL_MINSHORT;            

      i_ptr += src_pixel_stride;
      o_ptr += dst_pixel_stride;
    }
}

void
rgblinear_to_cmyk(Xil_signed16 *i_ptr,
                  Xil_signed16 *o_ptr,
                  unsigned int count,
                  unsigned int src_pixel_stride,
                  unsigned int dst_pixel_stride)
{
    //
    // Color-convert RGBlinear to CMYK
    //

    for (unsigned int i = 0; i < count; i++) {
      *o_ptr       = XIL_MAXSHORT - *(i_ptr + 2) + XIL_MINSHORT;              
      *(o_ptr + 1) = XIL_MAXSHORT - *(i_ptr + 1) + XIL_MINSHORT;            
      *(o_ptr + 2) = XIL_MAXSHORT - *(i_ptr) + XIL_MINSHORT;            
      *(o_ptr + 3) = XIL_MINSHORT;                     

      i_ptr += src_pixel_stride;
      o_ptr += dst_pixel_stride;
    }
}

void
rgblinear_to_ylinear(Xil_signed16 *i_ptr,
                     Xil_signed16 *o_ptr,
                     unsigned int count,
                     unsigned int src_pixel_stride,
                     unsigned int dst_pixel_stride)
{
    //
    // Color-convert RGBlinear to ylinear
    //

    for (unsigned int i = 0; i < count; i++) {
      double  y;                          
                                    
      y = _XILI_L_TO_Ylinear(*(i_ptr+2), *(i_ptr+1), *i_ptr);          
                                    
      *o_ptr = _XILI_ROUND_S16(y);                           

      i_ptr += src_pixel_stride;
      o_ptr += dst_pixel_stride;
    }
}

void
rgblinear_to_y601(Xil_signed16 *i_ptr,
                  Xil_signed16 *o_ptr,
                  unsigned int count,
                  unsigned int src_pixel_stride,
                  unsigned int dst_pixel_stride)
{
    //
    // Color-convert RGBlinear to y601
    //

    for (unsigned int i = 0; i < count; i++) {
      float r6, g6, b6, r7, g7, b7, y;

      b6 = _XILI_NORMALIZE_S(*(i_ptr));
      g6 = _XILI_NORMALIZE_S(*(i_ptr + 1));
      r6 = _XILI_NORMALIZE_S(*(i_ptr + 2));

      _XILI_L_TO_NL(r6, g6, b6, &r7, &g7, &b7);

      y = _XILI_NL_TO_Y601(r7, g7, b7);

      *o_ptr = _XILI_QUANTIZE_Y601_S(y);
                                    
      i_ptr += src_pixel_stride;
      o_ptr += dst_pixel_stride;
    }
}

void
rgblinear_to_rgb709(Xil_signed16 *i_ptr,
                    Xil_signed16 *o_ptr,
                    unsigned int count,
                    unsigned int src_pixel_stride,
                    unsigned int dst_pixel_stride)
{
    //
    // Color-convert RGBlinear to RGB709
    //

    for (unsigned int i = 0; i < count; i++) {
      float r6, g6, b6, r7, g7, b7;

      b6 = _XILI_NORMALIZE_S(*(i_ptr));
      g6 = _XILI_NORMALIZE_S(*(i_ptr + 1));
      r6 = _XILI_NORMALIZE_S(*(i_ptr + 2));

      _XILI_L_TO_NL(r6, g6, b6, &r7, &g7, &b7);

      *(o_ptr)     = _XILI_QUANTIZE_S(b7);
      *(o_ptr + 1) = _XILI_QUANTIZE_S(g7);
      *(o_ptr + 2) = _XILI_QUANTIZE_S(r7);

      i_ptr += src_pixel_stride;
      o_ptr += dst_pixel_stride;
    }
}
