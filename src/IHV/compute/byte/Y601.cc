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
//  File:   Y601.cc
//  Project:    XIL
//  Revision:   1.7
//  Last Mod:   10:11:01, 03/10/00
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
#pragma ident   "@(#)Y601.cc	1.7\t00/03/10  "

#include "color_convert.hh"

void
y601_to_photoycc(Xil_unsigned8 *i_ptr,
                 Xil_unsigned8 *o_ptr,
                 unsigned int count,
                 unsigned int src_pixel_stride,
                 unsigned int dst_pixel_stride)
{
    //
    // Color-convert Y601 to PhotoYCC
    //
    for (unsigned int i = 0; i < count; i++) {

      float y, photoy, photocb, photocr;                 
                                    
      y = _XILI_NORMALIZE_Y601_B(*(i_ptr));                        
                                    
      _XILI_NL_TO_YCC601(y, y, y, &photoy, &photocb, &photocr);           
                                    
      _XILI_QUANTIZE_PHOTO_B(photoy, photocb, photocr,
                             o_ptr, o_ptr+1, o_ptr+2); 

      i_ptr += src_pixel_stride;
      o_ptr += dst_pixel_stride;
    }
}

void
y601_to_ycc601(Xil_unsigned8 *i_ptr,
               Xil_unsigned8 *o_ptr,
               unsigned int count,
               unsigned int src_pixel_stride,
               unsigned int dst_pixel_stride)
{
    //
    // Color-convert Y601 to YCbCr601
    //

    for (unsigned int i = 0; i < count; i++) {
      float y;                               
                                    
      y = _XILI_NORMALIZE_Y601_B(*(i_ptr));                        
                                    
      _XILI_QUANTIZE_YCC601_B(y, 0.0, 0.0, o_ptr, o_ptr+1, o_ptr+2);     

      i_ptr += src_pixel_stride;
      o_ptr += dst_pixel_stride;
    }
}

void
y601_to_ycc709(Xil_unsigned8 *i_ptr,
               Xil_unsigned8 *o_ptr,
               unsigned int count,
               unsigned int src_pixel_stride,
               unsigned int dst_pixel_stride)
{
    //
    // Color-convert Y601 to YCbCr709
    //

    for (unsigned int i = 0; i < count; i++) {
      float y6, y7, cb7, cr7;                     
                                    
      y6 = _XILI_NORMALIZE_Y601_B(*(i_ptr));                        
                                    
      _XILI_NL_TO_YCC709(y6, y6, y6, &y7, &cb7, &cr7);               
                                    
      _XILI_QUANTIZE_YCC709_B(y7, cb7, cr7, o_ptr, o_ptr+1, o_ptr+2);

      i_ptr += src_pixel_stride;
      o_ptr += dst_pixel_stride;
    }
}

void
y601_to_y709(Xil_unsigned8 *i_ptr,
             Xil_unsigned8 *o_ptr,
             unsigned int count,
             unsigned int src_pixel_stride,
             unsigned int dst_pixel_stride)
{
    //
    // Color-convert Y601 to y709
    //

    for (unsigned int i = 0; i < count; i++) {
      *o_ptr = *(i_ptr);

      i_ptr += src_pixel_stride;
      o_ptr += dst_pixel_stride;
    }
}

void
y601_to_rgb709(Xil_unsigned8 *i_ptr,
               Xil_unsigned8 *o_ptr,
               unsigned int count,
               unsigned int src_pixel_stride,
               unsigned int dst_pixel_stride)
{
    //
    // Color-convert Y601 to RGB709
    //

    for (unsigned int i = 0; i < count; i++) {
      float y;                               
                                    
      y = _XILI_NORMALIZE_Y601_B(*(i_ptr));                        

      *o_ptr = _XILI_QUANTIZE_B(y);                     

      *(o_ptr + 1) = *(o_ptr);                        
      *(o_ptr + 2) = *(o_ptr);                        

      i_ptr += src_pixel_stride;
      o_ptr += dst_pixel_stride;
    }
}

