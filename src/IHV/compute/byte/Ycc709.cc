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
//  File:   Ycc709.cc
//  Project:    XIL
//  Revision:   1.4
//  Last Mod:   10:11:10, 03/10/00
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
#pragma ident   "@(#)Ycc709.cc	1.4\t00/03/10  "

#include "color_convert.hh"

void
ycc709_to_rgb709(Xil_unsigned8 *i_ptr,
                 Xil_unsigned8 *o_ptr,
                 unsigned int count,
                 unsigned int src_pixel_stride,
                 unsigned int dst_pixel_stride)
{
    //
    // Color-convert YCbCr709 to RGB709
    //

    for (unsigned int i = 0; i < count; i++) {
      float y, cb, cr, red, green, blue;                 
                                    
      _XILI_NORMALIZE_YCC709_B(*(i_ptr), *(i_ptr+1), *(i_ptr+2), &y, &cb, &cr);
                                    
      _XILI_YCC709_TO_NL(y, cb, cr, &red, &green, &blue);               
                                      
      *(o_ptr)   = _XILI_QUANTIZE_B(blue);       
      *(o_ptr+1) = _XILI_QUANTIZE_B(green);  
      *(o_ptr+2) = _XILI_QUANTIZE_B(red); 

      i_ptr += src_pixel_stride;
      o_ptr += dst_pixel_stride;
    }
}

void
ycc709_to_photoycc(Xil_unsigned8 *i_ptr,
                   Xil_unsigned8 *o_ptr,
                   unsigned int count,
                   unsigned int src_pixel_stride,
                   unsigned int dst_pixel_stride)
{
    //
    // Color-convert YCbCr709 to PhotoYCC
    //

    for (unsigned int i = 0; i < count; i++) {
      float y, cb, cr, red, green, blue, photoy, photocb, photocr;
                                      
      _XILI_NORMALIZE_YCC709_B(*(i_ptr), *(i_ptr+1), *(i_ptr+2), &y, &cb, &cr);
                                    
      _XILI_YCC709_TO_NL(y, cb, cr, &red, &green, &blue);               
                                    
      _XILI_NL_TO_YCC601(red, green, blue, &photoy, &photocb, &photocr);
                                    
      _XILI_QUANTIZE_PHOTO_B(photoy, photocb, photocr,
                             o_ptr, o_ptr+1, o_ptr+2); 

      i_ptr += src_pixel_stride;
      o_ptr += dst_pixel_stride;
    }
}

void
ycc709_to_ycc601(Xil_unsigned8 *i_ptr,
                 Xil_unsigned8 *o_ptr,
                 unsigned int count,
                 unsigned int src_pixel_stride,
                 unsigned int dst_pixel_stride)
{
    //
    // Color-convert YCbCr709 to YCbCr601
    //

    for (unsigned int i = 0; i < count; i++) {
      float y709, cb709, cr709, red, green, blue, y601, cb601, cr601;           
                                    
      _XILI_NORMALIZE_YCC709_B(*(i_ptr), *(i_ptr+1), *(i_ptr+2), 
                               &y709, &cb709, &cr709);  
                                    
      _XILI_YCC709_TO_NL(y709, cb709, cr709, &red, &green, &blue);
                                    
      _XILI_NL_TO_YCC601(red, green, blue, &y601, &cb601, &cr601);
                                    
      _XILI_QUANTIZE_YCC601_B(y601, cb601, cr601, o_ptr, o_ptr+1, o_ptr+2);

      i_ptr += src_pixel_stride;
      o_ptr += dst_pixel_stride;
    }
}

void
ycc709_to_y709(Xil_unsigned8 *i_ptr,
              Xil_unsigned8 *o_ptr,
              unsigned int count,
              unsigned int src_pixel_stride,
              unsigned int dst_pixel_stride)
{
    //
    // Color-convert YCbCr709 to CMY
    //

    for (unsigned int i = 0; i < count; i++) {
        *(o_ptr) = *(i_ptr);

        i_ptr += src_pixel_stride;
        o_ptr += dst_pixel_stride;
    }
}

void
ycc709_to_y601(Xil_unsigned8 *i_ptr,
               Xil_unsigned8 *o_ptr,
               unsigned int count,
               unsigned int src_pixel_stride,
               unsigned int dst_pixel_stride)
{
    //
    // Color-convert YCbCr709 to y601
    //

    for (unsigned int i = 0; i < count; i++) {
      float y7, cb, cr, red, green, blue, y6;                 
                                    
      _XILI_NORMALIZE_YCC709_B(*(i_ptr), *(i_ptr+1), *(i_ptr+2), &y7, &cb, &cr);
                                    
      _XILI_YCC709_TO_NL(y7, cb, cr, &red, &green, &blue);               
                                    
      y6 = _XILI_NL_TO_Y601(red, green, blue);                       
                                    
      *o_ptr = _XILI_QUANTIZE_Y601_B(y6);                       

      i_ptr += src_pixel_stride;
      o_ptr += dst_pixel_stride;
    }
}

