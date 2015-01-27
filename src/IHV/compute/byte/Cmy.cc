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
//  File:   Cmy.cc
//  Project:    XIL
//  Revision:   1.5
//  Last Mod:   10:11:21, 03/10/00
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
#pragma ident   "@(#)Cmy.cc	1.5\t00/03/10  "

#include "color_convert.hh"

void
cmy_to_rgblinear(Xil_unsigned8 *i_ptr,
                 Xil_unsigned8 *o_ptr,
                 unsigned int count,
                 unsigned int src_pixel_stride,
                 unsigned int dst_pixel_stride)
{
    //
    // Color-convert CMY to RGBlinear
    //

    for (unsigned int i = 0; i < count; i++) {
      _XILI_CMY_TO_L_B(*(i_ptr), *(i_ptr + 1), *(i_ptr + 2),
                       o_ptr + 2, o_ptr + 1, o_ptr);

      i_ptr += src_pixel_stride;
      o_ptr += dst_pixel_stride;
    }
}

void
cmy_to_y709(Xil_unsigned8 *i_ptr,
           Xil_unsigned8 *o_ptr,
           unsigned int count,
           unsigned int src_pixel_stride,
           unsigned int dst_pixel_stride)
{
    //
    // Color-convert CMY to Y709
    //

    for (unsigned int i = 0; i < count; i++) {
      float red, green, blue, r7, g7, b7, y709, c, m, y;            
                                    
      c = _XILI_NORMALIZE_B(*(i_ptr));                     
      m = _XILI_NORMALIZE_B(*(i_ptr + 1));                     
      y = _XILI_NORMALIZE_B(*(i_ptr + 2));                     
                                    
      _XILI_CMY_TO_L(c, m, y, &red, &green, &blue);                    
                                    
      _XILI_L_TO_NL(red, green, blue, &r7, &g7, &b7);                  
                                    
      y709 = _XILI_NL_TO_Y709(r7, g7, b7);                     
                                    
      *o_ptr = _XILI_QUANTIZE_Y709_B(y709);                     

      i_ptr += src_pixel_stride;
      o_ptr += dst_pixel_stride;
    }
}

void
cmy_to_cmyk(Xil_unsigned8 *i_ptr,
            Xil_unsigned8 *o_ptr,
            unsigned int count,
            unsigned int src_pixel_stride,
            unsigned int dst_pixel_stride)
{
    //
    // Color-convert CMY to CMYK
    //

    for (unsigned int i = 0; i < count; i++) {
      *o_ptr = *i_ptr; 
      *(o_ptr + 1) = *(i_ptr + 1);
      *(o_ptr + 2) = *(i_ptr + 2);
      *(o_ptr + 3) = XIL_MINBYTE;

      i_ptr += src_pixel_stride;
      o_ptr += dst_pixel_stride;
    }
}

void
cmy_to_ylinear(Xil_unsigned8 *i_ptr,
               Xil_unsigned8 *o_ptr,
               unsigned int count,
               unsigned int src_pixel_stride,
               unsigned int dst_pixel_stride)
{
    //
    // Color-convert CMY to ylinear
    //

    for (unsigned int i = 0; i < count; i++) {
      Xil_unsigned8 red, green, blue;                       
      double ylinear;                         
                                    
      _XILI_CMY_TO_L_B(*(i_ptr), *(i_ptr+1), *(i_ptr+2), &red, &green, &blue);

      ylinear = _XILI_L_TO_Ylinear(red, green, blue);                    
                                    
      *o_ptr = _XILI_ROUND_U8((float)ylinear);                       

      i_ptr += src_pixel_stride;
      o_ptr += dst_pixel_stride;
    }
}

void
cmy_to_y601(Xil_unsigned8 *i_ptr,
            Xil_unsigned8 *o_ptr,
            unsigned int count,
            unsigned int src_pixel_stride,
            unsigned int dst_pixel_stride)
{
    //
    // Color-convert CMY to y601
    //

    for (unsigned int i = 0; i < count; i++) {
      float red, green, blue, r7, g7, b7, y601, c, m, y;            
                                    
      c = _XILI_NORMALIZE_B(*(i_ptr));                     
      m = _XILI_NORMALIZE_B(*(i_ptr + 1));                     
      y = _XILI_NORMALIZE_B(*(i_ptr + 2));                     
                                    
      _XILI_CMY_TO_L(c, m, y, &red, &green, &blue);                    
                                    
      _XILI_L_TO_NL(red, green, blue, &r7, &g7, &b7);                  
                                    
      y601 = _XILI_NL_TO_Y601(r7, g7, b7);                     
                                    
      *o_ptr = _XILI_QUANTIZE_Y601_B(y601);                     

      i_ptr += src_pixel_stride;
      o_ptr += dst_pixel_stride;
    }
}

