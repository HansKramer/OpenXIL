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
//  File:   Ylinear.cc
//  Project:    XIL
//  Revision:   1.4
//  Last Mod:   10:12:03, 03/10/00
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
#pragma ident   "@(#)Ylinear.cc	1.4\t00/03/10  "

#include "color_convert.hh"

void
ylinear_to_rgblinear(Xil_signed16 *i_ptr,
                     Xil_signed16 *o_ptr,
                     unsigned int count,
                     unsigned int src_pixel_stride,
                     unsigned int dst_pixel_stride)
{
    //
    // Color-convert Ylinear to RGBlinear
    //

    for (unsigned int i = 0; i < count; i++) {

      *(o_ptr) = *(o_ptr + 1) = *(o_ptr + 2) = *(i_ptr);

      i_ptr += src_pixel_stride;
      o_ptr += dst_pixel_stride;
    }
}

void
ylinear_to_photoycc(Xil_signed16 *i_ptr,
                    Xil_signed16 *o_ptr,
                    unsigned int count,
                    unsigned int src_pixel_stride,
                    unsigned int dst_pixel_stride)
{
    //
    // Color-convert Ylinear to PhotoYCC
    //

    for (unsigned int i = 0; i < count; i++) {
      float ylinear, r;                          
                                    
      ylinear = _XILI_NORMALIZE_S(*(i_ptr));                   
                                    
      r = _XILI_L_TO_NL_1(ylinear);                       
                                    
      _XILI_QUANTIZE_PHOTO_S(r, 0.0, 0.0, o_ptr, o_ptr+1, o_ptr+2);

      i_ptr += src_pixel_stride;
      o_ptr += dst_pixel_stride;
    }
}

void
ylinear_to_ycc601(Xil_signed16 *i_ptr,
                  Xil_signed16 *o_ptr,
                  unsigned int count,
                  unsigned int src_pixel_stride,
                  unsigned int dst_pixel_stride)
{
    //
    // Color-convert Ylinear to YCbCr601
    //

    for (unsigned int i = 0; i < count; i++) {
      float ylinear, r, y, cb, cr;                   
                                    
      ylinear = _XILI_NORMALIZE_S(*(i_ptr));                   
                                    
      r = _XILI_L_TO_NL_1(ylinear);                       
                                    
      _XILI_NL_TO_YCC601(r, r, r, &y, &cb, &cr);                  
                                    
      _XILI_QUANTIZE_YCC601_S(y, cb, cr, o_ptr, o_ptr+1, o_ptr+2);       

      i_ptr += src_pixel_stride;
      o_ptr += dst_pixel_stride;
    }
}

void
ylinear_to_ycc709(Xil_signed16 *i_ptr,
                  Xil_signed16 *o_ptr,
                  unsigned int count,
                  unsigned int src_pixel_stride,
                  unsigned int dst_pixel_stride)
{
    //
    // Color-convert Ylinear to YCbCr709
    //

    for (unsigned int i = 0; i < count; i++) {
      float ylinear, r, y, cb, cr;                   
                                    
      ylinear = _XILI_NORMALIZE_S(*(i_ptr));                   
                                    
      r = _XILI_L_TO_NL_1(ylinear);                       
                                    
      _XILI_NL_TO_YCC709(r, r, r, &y, &cb, &cr);                  
                                    
      _XILI_QUANTIZE_YCC709_S(y, cb, cr, o_ptr, o_ptr+1, o_ptr+2);        

      i_ptr += src_pixel_stride;
      o_ptr += dst_pixel_stride;
    }
}

void
ylinear_to_cmy(Xil_signed16 *i_ptr,
               Xil_signed16 *o_ptr,
               unsigned int count,
               unsigned int src_pixel_stride,
               unsigned int dst_pixel_stride)
{
    //
    // Color-convert Ylinear to CMY
    //

    for (unsigned int i = 0; i < count; i++) {
      *o_ptr = XIL_MAXSHORT - *(i_ptr) + XIL_MINSHORT;
      *(o_ptr + 1) = *o_ptr;               
      *(o_ptr + 2) = *o_ptr;              

      i_ptr += src_pixel_stride;
      o_ptr += dst_pixel_stride;
    }
}

void
ylinear_to_cmyk(Xil_signed16 *i_ptr,
                Xil_signed16 *o_ptr,
                unsigned int count,
                unsigned int src_pixel_stride,
                unsigned int dst_pixel_stride)
{
    //
    // Color-convert Ylinear to CMYK
    //

    for (unsigned int i = 0; i < count; i++) {
      *o_ptr = XIL_MAXSHORT - *(i_ptr) + XIL_MINSHORT;
      *(o_ptr + 1) = *o_ptr;               
      *(o_ptr + 2) = *o_ptr;              
      *(o_ptr + 3) = XIL_MINSHORT;            

      i_ptr += src_pixel_stride;
      o_ptr += dst_pixel_stride;
    }
}

void
ylinear_to_y709(Xil_signed16 *i_ptr,
                   Xil_signed16 *o_ptr,
                   unsigned int count,
                   unsigned int src_pixel_stride,
                   unsigned int dst_pixel_stride)
{
    //
    // Color-convert Ylinear to y709
    //

    for (unsigned int i = 0; i < count; i++) {
      float ylinear, r7; 
                            
      ylinear = _XILI_NORMALIZE_S(*(i_ptr));                   
                                    
      r7 = _XILI_L_TO_NL_1(ylinear);                            
                                    
      *o_ptr = _XILI_QUANTIZE_Y709_S(r7);                       

      i_ptr += src_pixel_stride;
      o_ptr += dst_pixel_stride;
    }
}

void
ylinear_to_y601(Xil_signed16 *i_ptr,
                Xil_signed16 *o_ptr,
                unsigned int count,
                unsigned int src_pixel_stride,
                unsigned int dst_pixel_stride)
{
    //
    // Color-convert Ylinear to y601
    //

    for (unsigned int i = 0; i < count; i++) {
      float ylinear, r7; 
                            
      ylinear = _XILI_NORMALIZE_S(*(i_ptr));                   
                                    
      r7 = _XILI_L_TO_NL_1(ylinear);                            
                                    
      *o_ptr = _XILI_QUANTIZE_Y601_S(r7);                       

      i_ptr += src_pixel_stride;
      o_ptr += dst_pixel_stride;
    }
}

void
ylinear_to_rgb709(Xil_signed16 *i_ptr,
                 Xil_signed16 *o_ptr,
                 unsigned int count,
                 unsigned int src_pixel_stride,
                 unsigned int dst_pixel_stride)
{
    //
    // Color-convert Ylinear to RGB709
    //

    for (unsigned int i = 0; i < count; i++) {
      float ylinear, r7;

      ylinear = _XILI_NORMALIZE_S(*(i_ptr));

      r7 = _XILI_L_TO_NL_1(ylinear);

      *o_ptr = _XILI_QUANTIZE_S(r7);

      *(o_ptr + 1) = *o_ptr;
      *(o_ptr + 2) = *o_ptr;


      i_ptr += src_pixel_stride;
      o_ptr += dst_pixel_stride;
    }
}

