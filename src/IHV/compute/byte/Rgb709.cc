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
//  File:   Rgb709.cc
//  Project:    XIL
//  Revision:   1.5
//  Last Mod:   10:11:20, 03/10/00
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
#pragma ident   "@(#)Rgb709.cc	1.5\t00/03/10  "

#include "color_convert.hh"

void
rgb709_to_photoycc(Xil_unsigned8 *i_ptr,
                   Xil_unsigned8 *o_ptr,
                   unsigned int count,
                   unsigned int src_pixel_stride,
                   unsigned int dst_pixel_stride)
{
    //
    // Color-convert RGB709 to PhotoYCC
    //

    for (unsigned int i = 0; i < count; i++)
    {
      float red, green, blue, photoy, photocb, photocr;

      blue  = _XILI_NORMALIZE_B(*(i_ptr));
      green = _XILI_NORMALIZE_B(*(i_ptr + 1));
      red   = _XILI_NORMALIZE_B(*(i_ptr + 2));
                                 
      _XILI_NL_TO_YCC601(red, green, blue, &photoy, &photocb, &photocr);
                                    
      _XILI_QUANTIZE_PHOTO_B(photoy, photocb, photocr,
                             o_ptr, o_ptr+1, o_ptr+2);

      i_ptr += src_pixel_stride;
      o_ptr += dst_pixel_stride;
    }
}

void
rgb709_to_ycc601(Xil_unsigned8 *i_ptr,
                 Xil_unsigned8 *o_ptr,
                 unsigned int count,
                 unsigned int src_pixel_stride,
                 unsigned int dst_pixel_stride)
{
    //
    // Color-convert RGB709 to YCbCr601
    //

    for (unsigned int i = 0; i < count; i++) {
      float red, green, blue, y, cb, cr;

      blue  = _XILI_NORMALIZE_B(*(i_ptr));  
      green = _XILI_NORMALIZE_B(*(i_ptr + 1));
      red   = _XILI_NORMALIZE_B(*(i_ptr + 2));
                                
      _XILI_NL_TO_YCC601(red, green, blue, &y, &cb, &cr);
                                    
      _XILI_QUANTIZE_YCC601_B(y, cb, cr, o_ptr, o_ptr+1, o_ptr+2);

      i_ptr += src_pixel_stride;
      o_ptr += dst_pixel_stride;
    }
}

void
rgb709_to_ycc709(Xil_unsigned8 *i_ptr,
                 Xil_unsigned8 *o_ptr,
                 unsigned int count,
                 unsigned int src_pixel_stride,
                 unsigned int dst_pixel_stride)
{
    //
    // Color-convert RGB709 to YCbCr709
    //

    for (unsigned int i = 0; i < count; i++) {
      float red, green, blue, y, cb, cr; 
                                    
      blue  = _XILI_NORMALIZE_B(*(i_ptr));     
      green = _XILI_NORMALIZE_B(*(i_ptr + 1));
      red   = _XILI_NORMALIZE_B(*(i_ptr + 2));
                                 
      _XILI_NL_TO_YCC709(red, green, blue, &y, &cb, &cr);
                                    
      _XILI_QUANTIZE_YCC709_B(y, cb, cr, o_ptr, o_ptr+1, o_ptr+2);

      i_ptr += src_pixel_stride;
      o_ptr += dst_pixel_stride;
    }
}

void
rgb709_to_y601(Xil_unsigned8 *i_ptr,
               Xil_unsigned8 *o_ptr,
               unsigned int count,
               unsigned int src_pixel_stride,
               unsigned int dst_pixel_stride)
{
    //
    // Color-convert RGB709 to y601
    //

    for (unsigned int i = 0; i < count; i++) {
      double y;

      y = _XILI_NL_TO_Y601(*(i_ptr + 2), *(i_ptr + 1), *(i_ptr));
                
      *o_ptr = _XILI_QUANTIZE_Y601((float)y);                        

      i_ptr += src_pixel_stride;
      o_ptr += dst_pixel_stride;
    }
}

void
rgb709_to_y709(Xil_unsigned8 *i_ptr,
               Xil_unsigned8 *o_ptr,
               unsigned int count,
               unsigned int src_pixel_stride,
               unsigned int dst_pixel_stride)
{
    //
    // Color-convert RGB709 to y709
    //

    for (unsigned int i = 0; i < count; i++) {
      double y;

      y = _XILI_NL_TO_Y709(*(i_ptr + 2), *(i_ptr + 1), *(i_ptr));
    
      *o_ptr = _XILI_QUANTIZE_Y709((float)y);                        

      i_ptr += src_pixel_stride;
      o_ptr += dst_pixel_stride;
    }
}
