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
//  File:	XiliDiffusionUtils.hh
//  Project:	XIL
//  Revision:	1.3
//  Last Mod:	10:22:27, 03/10/00
//
//  Description:
//
//  This file contains declarations for utility functions for
//  ErrorDiffusion operations; Particularly used in default
//  (non-optimized) cases.  See the corresponding *.cc file.
//	
//	
//------------------------------------------------------------------------
//	COPYRIGHT
//------------------------------------------------------------------------
#pragma ident	"@(#)XiliDiffusionUtils.hh	1.3\t00/03/10  "

#ifndef _XILI_DIFFUSION_UTILS_HH
#define _XILI_DIFFUSION_UTILS_HH

#ifdef _XIL_LIBXIL_PRIVATE
#include "_XilLookup.hh"
#else
#include <xil/xilGPI.hh>
#endif

//
//  This function will only be used by error-diffusion since it
//  assumes that the pixel passed in is a float. Its inputs are a float
//  pixel, a lookup, and a pointer to an integer. It sets the pointer to
//  the integer to be the lookup index (adjusted with offset) and returns
//  a (void *) to the actual entry data in the lookup.
//
void *
xili_find_nearest_color(float* pixel, 
			XilLookupSingle* cmap,
			int* nc_idx);

//
// The following xili_diffusion_copy_x() functions copy image data from a
// temporary image to a destination image taking into account the ROI.  The
// x part of the name specifies the type of data to copy.
//
void
xili_diffusion_copy_1(Xil_unsigned8* tmp_data,
		      unsigned int tmp_scanline_stride,
		      Xil_unsigned8* dst_data,
		      unsigned int dst_scanline_stride,
		      unsigned int dst_offset,
		      XilRoi* roi,
		      XilBox* dst_box,
		      int box_x, int box_y);

void
xili_diffusion_copy_8(Xil_unsigned8* tmp_data,
		      unsigned int tmp_scanline_stride,
		      Xil_unsigned8* dst_data,
		      unsigned int dst_scanline_stride,
		      unsigned int dst_pixel_stride,
		      XilRoi* roi,
		      XilBox* dst_box,
		      int box_x, int box_y);

void
xili_diffusion_copy_16(Xil_signed16* tmp_data,
		       unsigned int tmp_scanline_stride,
		       Xil_signed16* dst_data,
		       unsigned int dst_scanline_stride,
		       unsigned int dst_pixel_stride,
		       XilRoi* roi,
		       XilBox* dst_box,
		       int box_x, int box_y);

void
xili_diffusion_copy_f32(Xil_float32* tmp_data,
			unsigned int tmp_scanline_stride,
			Xil_float32* dst_data,
			unsigned int dst_scanline_stride,
			unsigned int dst_pixel_stride,
			XilRoi* roi,
			XilBox* dst_box,
			int box_x, int box_y);

//
//  Check to see if a kernel is a floyd-steinberg kernel.
//
Xil_boolean
xili_is_floyd_steinberg_kernel(XilKernel* kernel);

#endif	// _XILI_DIFFUSION_UTILS_HH
