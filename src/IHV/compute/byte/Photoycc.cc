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
//  File:   Photoycc.cc
//  Project:    XIL
//  Revision:   1.5
//  Last Mod:   10:11:14, 03/10/00
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
#pragma ident   "@(#)Photoycc.cc	1.5\t00/03/10  "

#include "color_convert.hh"

void
photoycc_to_y709(Xil_unsigned8 *i_ptr,
                   Xil_unsigned8 *o_ptr,
                   unsigned int count,
                   unsigned int src_pixel_stride,
                   unsigned int dst_pixel_stride)
{
    //
    // Color-convert PhotoYCC to Y709
    //

    for (unsigned int i = 0; i < count; i++) {
      float y6, cb6, cr6, red, green, blue, y7;                 
                                    
      _XILI_NORMALIZE_PHOTO_B(*(i_ptr), *(i_ptr+1), *(i_ptr+2), 
                              &y6, &cb6, &cr6);  
                                    
      _XILI_YCC601_TO_NL(y6, cb6, cr6, &red, &green, &blue);               
                                    
      y7 = _XILI_NL_TO_Y709(red, green, blue);                       
                                    
      *o_ptr = _XILI_QUANTIZE_Y709_B(y7);                       

      i_ptr += src_pixel_stride;
      o_ptr += dst_pixel_stride;
    }
}

void
photoycc_to_ycc601(Xil_unsigned8 *i_ptr,
                   Xil_unsigned8 *o_ptr,
                   unsigned int count,
                   unsigned int src_pixel_stride,
                   unsigned int dst_pixel_stride)
{
    //
    // Color-convert PhotoYCC to YCbCr601
    //

    for (unsigned int i = 0; i < count; i++) {
      float y6, cb6, cr6;                         
                                    
      _XILI_NORMALIZE_PHOTO_B(*(i_ptr), *(i_ptr+1), *(i_ptr+2), 
                              &y6, &cb6, &cr6);  
                                    
      _XILI_QUANTIZE_YCC601_B(y6, cb6, cr6, o_ptr, o_ptr+1, o_ptr+2); 

      i_ptr += src_pixel_stride;
      o_ptr += dst_pixel_stride;
    }
}

void
photoycc_to_ycc709(Xil_unsigned8 *i_ptr,
                 Xil_unsigned8 *o_ptr,
                 unsigned int count,
                 unsigned int src_pixel_stride,
                 unsigned int dst_pixel_stride)
{
    //
    // Color-convert PhotoYCC to YCbCr709
    //

    for (unsigned int i = 0; i < count; i++) {
      float y6, cb6, cr6, red, green, blue, y7, cb7, cr7;           
                                    
      _XILI_NORMALIZE_PHOTO_B(*(i_ptr), *(i_ptr+1), *(i_ptr+2), 
                              &y6, &cb6, &cr6);  
                                    
      _XILI_YCC601_TO_NL(y6, cb6, cr6, &red, &green, &blue);               
                                    
      _XILI_NL_TO_YCC709(red, green, blue, &y7, &cb7, &cr7);               
                                    
      _XILI_QUANTIZE_YCC709_B(y7, cb7, cr7, o_ptr, o_ptr+1, o_ptr+2);

      i_ptr += src_pixel_stride;
      o_ptr += dst_pixel_stride;
    }
}

void
photoycc_to_y601(Xil_unsigned8 *i_ptr,
                 Xil_unsigned8 *o_ptr,
                 unsigned int count,
                 unsigned int src_pixel_stride,
                 unsigned int dst_pixel_stride)
{
    //
    // Color-convert PhotoYCC to y601
    //

    for (unsigned int i = 0; i < count; i++) {
      float y6, cb6, cr6;                         
                                    
      _XILI_NORMALIZE_PHOTO_B(*(i_ptr), *(i_ptr+1), *(i_ptr+2), 
                              &y6, &cb6, &cr6);  
                                    
      *o_ptr = _XILI_QUANTIZE_Y601_B(y6);                       

      i_ptr += src_pixel_stride;
      o_ptr += dst_pixel_stride;
    }
}

