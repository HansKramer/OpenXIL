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
#pragma ident   "@(#)Rgblinear.cc	1.5\t00/03/10  "

#include "color_convert.hh"

void
rgblinear_to_cmy(Xil_unsigned8 *i_ptr,
                 Xil_unsigned8 *o_ptr,
                 unsigned int count,
                 unsigned int src_pixel_stride,
                 unsigned int dst_pixel_stride)
{
    //
    // Color-convert RGBlinear to CMY
    //

    for (unsigned int i = 0; i < count; i++) {
      *o_ptr       = XIL_MAXBYTE - *(i_ptr + 2) + XIL_MINBYTE;              
      *(o_ptr + 1) = XIL_MAXBYTE - *(i_ptr + 1) + XIL_MINBYTE;            
      *(o_ptr + 2) = XIL_MAXBYTE - *(i_ptr) + XIL_MINBYTE;            

      i_ptr += src_pixel_stride;
      o_ptr += dst_pixel_stride;
    }
}

void
rgblinear_to_cmyk(Xil_unsigned8 *i_ptr,
                  Xil_unsigned8 *o_ptr,
                  unsigned int count,
                  unsigned int src_pixel_stride,
                  unsigned int dst_pixel_stride)
{
    //
    // Color-convert RGBlinear to CMYK
    //

    for (unsigned int i = 0; i < count; i++) {
      *o_ptr       = XIL_MAXBYTE - *(i_ptr + 2) + XIL_MINBYTE;              
      *(o_ptr + 1) = XIL_MAXBYTE - *(i_ptr + 1) + XIL_MINBYTE;            
      *(o_ptr + 2) = XIL_MAXBYTE - *(i_ptr) + XIL_MINBYTE;            
      *(o_ptr + 3) = XIL_MINBYTE;                     

      i_ptr += src_pixel_stride;
      o_ptr += dst_pixel_stride;
    }
}

void
rgblinear_to_ylinear(Xil_unsigned8 *i_ptr,
                     Xil_unsigned8 *o_ptr,
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
                                    
      *o_ptr = _XILI_ROUND_U8((float)y);                           

      i_ptr += src_pixel_stride;
      o_ptr += dst_pixel_stride;
    }
}

