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
//  File:	XiliDiffusionUtils.cc
//  Project:	XIL
//  Revision:	1.6
//  Last Mod:	10:13:35, 03/10/00
//
//  Description:
//	This file contains utility functions for ErrorDiffusion operations;
//  Particularly used in default (non-optimized) cases.
//	
//	
//	
//	
//	
//	
//  MT-level:  SAFE
//	
//------------------------------------------------------------------------
//	COPYRIGHT
//------------------------------------------------------------------------
#pragma ident	"@(#)XiliDiffusionUtils.cc	1.6\t00/03/10  "

#include <string.h>
#include "XiliDiffusionUtils.hh"
#include "XiliUtils.hh"			// For xili_memcpy()

//
//  This function will only be used by error-diffusion since it
//  assumes that the pixel passed in is a float. Its inputs are a float
//  pixel, a lookup, and a pointer to an integer. It sets the pointer to
//  the integer to be the lookup index (adjusted with offset) and returns
//  a (void *) to the actual entry data in the lookup.
//
void*
xili_find_nearest_color(float*           pixel,
			XilLookupSingle* cmap,
			int*             nc_idx)
{
    XilDataType in_dt  = cmap->getInputDataType();
    XilDataType out_dt = cmap->getOutputDataType();

    if(cmap->isColorcube() && in_dt != XIL_BIT) {
        XilLookupColorcube* ccube  = (XilLookupColorcube*)cmap;
        unsigned int        nbands = ccube->getOutputNBands();
        const unsigned int* dims   = ccube->getDimsMinus1();
        const int*          mults  = ccube->getMultipliers();
        unsigned int        offset = ccube->getAdjustedOffset();

        unsigned int index = 0;

        switch(out_dt) {
          case XIL_BIT:
          {
              for(unsigned int band = 0; band < nbands; band++) {
                  if(pixel[band] != 0.0F) {
                      float ftmp = pixel[band] * (float)dims[band];
                      int   itmp = (int)ftmp;

                      if((ftmp - (float)itmp) >= 0.5F) {
                          itmp += 2;
                      }

                      index += (itmp>>1) * mults[band];
                  }
              }

              *nc_idx = index + offset;

              return (Xil_unsigned8*)ccube->getData() + (index * nbands);
          }

          case XIL_BYTE:
          {
              for(unsigned int band = 0; band < nbands; band++) {
                  int tmp = (int)(pixel[band] * (float)dims[band]);

                  if((tmp & 0xFF) > 127) {
                      tmp += 0x100;
                  }

                  index += (tmp>>8) * mults[band];
              }

              *nc_idx = index + offset;
              return (Xil_unsigned8*)ccube->getData() + (index * nbands);
          }

          case XIL_SHORT:
          {
              for(unsigned int band = 0; band < nbands; band++) {
                  int tmp = (pixel[band] + 32768.0F) * (float)dims[band];

                  if((tmp & 0xFFFF) > 32767) {
                      tmp += 0x10000;
                  }

                  index += (tmp>>16) * mults[band];
              }

              *nc_idx = index + offset;

              return (Xil_signed16*)ccube->getData() + (index * nbands);
          }

          case XIL_FLOAT:
          {
              for(unsigned int band = 0; band < nbands; band++) {
                  float ftmp = pixel[band] * (float)dims[band];
                  int   itmp = (int)ftmp;

                  if((ftmp - itmp) >= 0.5F) {
                      itmp++;
                  }

                  index += itmp * mults[band];
              }

              *nc_idx = index + offset;

              return (Xil_float32*)ccube->getData() + (index * nbands);
          }
        }
    } else {
        unsigned int cmap_size   = cmap->getNumEntries();
        unsigned int nbands      = cmap->getOutputNBands();

        *nc_idx = cmap->getOffset();

        switch(out_dt) {
          case XIL_BIT: 
          case XIL_BYTE:
          {
              //
              //  If output datatype of lookup is XIL_BIT, each band can only
              //  have values 0 or 1.  If pixel_value < 0.5, it gets rounded
              //  down to 0, & distance only changes if cmap entry is 1; if
              //  pixel_value >= 0.5, it gets rounded up to 1, & distance
              //  only changes value if cmap entry if 0.
              //
              Xil_unsigned8* cmap_base = (Xil_unsigned8*)cmap->getData();
              Xil_unsigned8* cmap_data = cmap_base;

              float closest_distance = 0.0F;
              int   closest_index    = 0;
              for(unsigned int k = 0; k < nbands; k++) {
                  float diff = pixel[k] - _XILI_B2F(*cmap_data++);

                  closest_distance += (diff * diff);
              }

              for(unsigned int i = 1; i < cmap_size; i++) {
                  float distance = 0.0F;
                  for(k = 0; k < nbands; k++) {
                      float diff = pixel[k] - _XILI_B2F(*cmap_data++);
                      distance  += (diff * diff);
                  }

                  if(distance < closest_distance) {
                      closest_distance = distance;
                      closest_index    = i;
                  }
              }

              *nc_idx += closest_index;

              return cmap_base + (closest_index * nbands);
          }

          case XIL_SHORT:
          {
              //
              //  If output datatype of lookup is XIL_SHORT, greatest diff
              //  should only be 65535, but since float error added to
              //  pixels, find difference & its square as float values.
              //  If square will fit into unsigned integer after it
              //  is rounded, then use it, else set distance to max. value a
              //  unsigned integer can take (to assure that it will not be
              //  closer).
              //
              Xil_signed16* cmap_base = (Xil_signed16*)cmap->getData();
              Xil_signed16* cmap_data = cmap_base;

              float closest_distance = 0.0F;
              int   closest_index    = 0;
              for(unsigned int k = 0; k < nbands; k++) {
                  float diff = pixel[k] - (float)(*cmap_data++);

                  closest_distance += (diff * diff);
              }

              for(unsigned int i = 1; i < cmap_size; i++) {
                  float distance = 0.0F;
                  for(k = 0; k < nbands; k++) {
                      float diff = pixel[k] - (float)(*cmap_data++);
                      distance  += (diff * diff);
                  }

                  if(distance < closest_distance) {
                      closest_distance = distance;
                      closest_index    = i;
                  }
              }

              *nc_idx += closest_index;

              return cmap_base + (closest_index * nbands);
          }

          case XIL_FLOAT:
          {
              Xil_float32* cmap_base = (Xil_float32*)cmap->getData();
              Xil_float32* cmap_data = cmap_base;

              float closest_distance = 0.0F;
              int   closest_index    = 0;
              for(unsigned int k = 0; k < nbands; k++) {
                  float diff = pixel[k] - *cmap_data++;

                  closest_distance += (diff * diff);
              }

              for(unsigned int i = 1; i < cmap_size; i++) {
                  float distance = 0.0F;
                  for(k = 0; k < nbands; k++) {
                      float diff = pixel[k] - *cmap_data++;
                      distance  += (diff * diff);
                  }

                  if(distance < closest_distance) {
                      closest_distance = distance;
                      closest_index    = i;
                  }
              }

              *nc_idx += closest_index;

              return cmap_base + (closest_index * nbands);
          }

          default:
            *nc_idx = -1;
            return NULL;
        }
    }
    return NULL;
}

//
//  Check to see if a kernel is a floyd-steinberg kernel.
//
Xil_boolean
xili_is_floyd_steinberg_kernel(XilKernel* kernel)
{
    //
    //  First, check if it is the standard Floyd-Steinberg kernel object.
    //
    char* kname = kernel->getName();
    if(kname != NULL && strcmp(kname, "floyd-steinberg") == 0) {
        return TRUE;
    }

    //
    //  If it's not standard Floyd-Steinberg kernel object, check to see that it
    //  is functionally equivalent.  We know it's a valid distribution kernel.
    //
    unsigned short dist_w     = kernel->getWidth();
    unsigned short dist_h     = kernel->getHeight();
    unsigned short dist_key_x = kernel->getKeyX();
    unsigned short dist_key_y = kernel->getKeyY();
    const float*   dist_value = kernel->getData();
    const float*   key_idx    = dist_value + (dist_key_y * dist_w) + dist_key_x;

    if(! XILI_FLT_EQ(key_idx[1], 7.0F/16.0F)) {
        return FALSE;
    }

    if(! XILI_FLT_EQ(key_idx[dist_w - 1], 3.0F/16.0F)) {
        return FALSE;
    }

    if(! XILI_FLT_EQ(key_idx[dist_w], 5.0F/16.0F)) {
        return FALSE;
    }

    if(! XILI_FLT_EQ(key_idx[dist_w + 1], 1.0F/16.0F)) {
        return FALSE;
    }

    return TRUE;
}

//
//  Copy 1-bit images that have no offsets
//
static void
xili_copy1_no_offsets(Xil_unsigned8 *src_band, 
		      Xil_unsigned8 *dst_band,
		      unsigned short nbands,
		      unsigned int xs, 
		      unsigned int ys,
		      unsigned long src_next_scan,
		      unsigned long src_next_band,
		      unsigned long dst_next_scan,
		      unsigned long dst_next_band)
{
    // Masks to clear least significant bits without altering most 
    // significant bits (byte & mask[i]); index is number of bits to clear
    static Xil_unsigned8 clr_lsb_mask[8] = {
	0xFF, 0xFE, 0xFC, 0xF8, 0xF0, 0xE0, 0xC0, 0x80 
    };
    // Masks to clear most significant bits without altering least 
    // significant bits (byte & mask[i]); index is number of bits to clear
    static Xil_unsigned8 clr_msb_mask[8] = { 
	0xFF, 0x7F, 0x3F, 0x1F, 0x0F, 0x07, 0x03, 0x01
    };

    Xil_unsigned8 *src_pixel, *dst_pixel, *tmp_src, *tmp_dst;
    int src_tail = (int)(xs & 0x7),
    	cpy_span = (xs >> 3);

    do {
	// loop over all bands of image
	int y_size = (int) ys;	

	src_pixel = src_band;
	dst_pixel = dst_band;

	do {
	    // loop over all scanlines in ROI
	    if (cpy_span) {
		 xili_memcpy(dst_pixel, src_pixel, cpy_span);
	    }

	    if ( src_tail ) {
		tmp_src = src_pixel + cpy_span;
		tmp_dst = dst_pixel + cpy_span;
		*tmp_dst = ((*tmp_dst) & clr_msb_mask[src_tail]) |
			   ((*tmp_src) & clr_lsb_mask[8 - src_tail]);
	    }

	    // go to next scanline
	    src_pixel += src_next_scan;
	    dst_pixel  += dst_next_scan;
	} while (--y_size);

	src_band += src_next_band;
	dst_band += dst_next_band;
    } while (--nbands);
}

//
// Copy all bands of src and dst 8-bit images
//
static void
xili_copy8_all_bands(Xil_unsigned8 *src1_scanline,
		     Xil_unsigned8 *dest_scanline,
		     unsigned int xs,
		     unsigned int ys,
		     unsigned long src1_next_scan,
		     unsigned long dest_next_scan)
{
    do {
        xili_memcpy(dest_scanline, src1_scanline, xs);
        src1_scanline += src1_next_scan;
        dest_scanline += dest_next_scan;
    } while (--ys);
}
 
//
// Copy some bands of src and dst 8-bit images.  This works for copying
// all bands but is faster than xili_copy8_all_bands().
//
static void
xili_copy8_child(Xil_unsigned8 *src1_scanline,
		 Xil_unsigned8 *dest_scanline,
		 unsigned int x_size,
		 unsigned int y_size,
		 unsigned short nbands,
		 unsigned long src1_next_scan,
		 unsigned long src1_next_pixel,
		 unsigned long dest_next_scan,
		 unsigned long dest_next_pixel)
{
    Xil_unsigned8       *src1_pixel, *dest_pixel;
    Xil_unsigned8       *src1_band, *dest_band;
    unsigned int i, j, k;
    for (i=0; i<y_size; i++) {
        src1_pixel = src1_scanline;
        dest_pixel = dest_scanline;
        for (j=0; j<x_size; j++) {
            src1_band = src1_pixel;
            dest_band = dest_pixel;
            for (k=0; k<(int)nbands; k++) {
                *dest_band = (*src1_band);
                src1_band++;
                dest_band++;
            }

            // move to next pixel
            src1_pixel += src1_next_pixel;
            dest_pixel += dest_next_pixel;
        }

        // move to next scanline
        src1_scanline += src1_next_scan;
        dest_scanline += dest_next_scan;
    }
}

//
// Copy all bands of src and dst 16-bit images
//
static void
xili_copy16_all_bands(Xil_signed16 *src1_scanline,
		      Xil_signed16 *dest_scanline,
		      unsigned int xs,
		      unsigned int ys,
		      unsigned long src1_next_scan,
		      unsigned long dest_next_scan)
{
    do {
        xili_memcpy(dest_scanline, src1_scanline, xs);
        src1_scanline += src1_next_scan;
        dest_scanline += dest_next_scan;
    } while (--ys);
}

//
// Copy some bands of src and dst 16-bit images.  This works for copying
// all bands but is faster than xili_copy16_all_bands().
//
static void
xili_copy16_child(Xil_signed16 *src1_scanline,
		  Xil_signed16 *dest_scanline,
		  unsigned int xs,
		  unsigned int ys,
		  unsigned short nbands,
		  unsigned long src1_next_scan,
		  unsigned long src1_next_pixel,
		  unsigned long dest_next_scan,
		  unsigned long dest_next_pixel)
{
    // want to copy only some of bands in storage.
    Xil_signed16       *src1_pixel, *dest_pixel;
    Xil_signed16       *src1_band, *dest_band;
    unsigned int i, j, k;
    for (i=0; i<ys; i++) {
        src1_pixel = src1_scanline;
        dest_pixel = dest_scanline;
        for (j=0; j<xs; j++) {
            src1_band = src1_pixel;
            dest_band = dest_pixel;
            for (k=0; k<(int)nbands; k++) {
                *dest_band = (*src1_band);
 
                // move to next data element
                src1_band++;
                dest_band++;
            }
 
            // move to next pixel
            src1_pixel += src1_next_pixel;
            dest_pixel += dest_next_pixel;
        }
 
        // move to next scanline
        src1_scanline += src1_next_scan;
        dest_scanline += dest_next_scan;
    }
}

//
// Copy all bands of src and dst float images
//
static void
xili_copyF_all_bands(Xil_float32 *src1_scanline,
		     Xil_float32 *dest_scanline,
		     unsigned int xs,
		     unsigned int ys,
		     unsigned long src1_next_scan,
		     unsigned long dest_next_scan)
{
    do {
        xili_memcpy(dest_scanline, src1_scanline, xs);
        src1_scanline += src1_next_scan;
        dest_scanline += dest_next_scan;
    } while (--ys);
}

//
// Copy some bands of src and dst float images.  This works for copying
// all bands but is faster than xili_copyF_all_bands().
//
static void
xili_copyF_child(Xil_float32 *src1_scanline,
		 Xil_float32 *dest_scanline,
		 unsigned int xs,
		 unsigned int ys,
		 unsigned short nbands,
		 unsigned long src1_next_scan,
		 unsigned long src1_next_pixel,
		 unsigned long dest_next_scan,
		 unsigned long dest_next_pixel)
{
    // want to copy only some of bands in storage.
    Xil_float32       *src1_pixel, *dest_pixel;
    Xil_float32       *src1_band, *dest_band;
    unsigned int i, j, k;
    for (i=0; i<ys; i++) {
        src1_pixel = src1_scanline;
        dest_pixel = dest_scanline;
        for (j=0; j<xs; j++) {
            src1_band = src1_pixel;
            dest_band = dest_pixel;
            for (k=0; k<(int)nbands; k++) {
                *dest_band = (*src1_band);
 
                // move to next data element
                src1_band++;
                dest_band++;
            }
 
            // move to next pixel
            src1_pixel += src1_next_pixel;
            dest_pixel += dest_next_pixel;
        }
 
        // move to next scanline
        src1_scanline += src1_next_scan;
        dest_scanline += dest_next_scan;
    }
}

void
xili_diffusion_copy_1(Xil_unsigned8* tmp_data,
		      unsigned int tmp_scanline_stride,
		      Xil_unsigned8* dst_data,
		      unsigned int dst_scanline_stride,
		      unsigned int dst_offset,
		      XilRoi* roi,
		      XilBox* dst_box,
		      int box_x, int box_y)
{
    //
    //  Create a list of rectangles to loop over.  The resulting list
    //  of rectangles is the area left by intersecting the ROI with
    //  the destination box.
    //
    XilRectList    rl(roi, dst_box);
	    
    int            x;
    int            y;
    unsigned int   xsize;
    unsigned int   ysize;
    while(rl.getNext(&x, &y, &xsize, &ysize)) {
	Xil_unsigned8* dst_scanline = dst_data +
	    y * dst_scanline_stride +
	    (x + dst_offset) / 8;
	int dst_rect_offset = (x + dst_offset) % 8;
	// Since tmp is in src image space, translate (x, y)
	// which is in box space to image space
	Xil_unsigned8* tmp_scanline = tmp_data +
	    (y + box_y) * tmp_scanline_stride +
	    (x + box_x) / 8;
	int tmp_rect_offset = (x + box_x) % 8;

	if (dst_rect_offset == 0 && tmp_rect_offset == 0) {
	    xili_copy1_no_offsets(tmp_scanline, dst_scanline, 1, 
				  xsize, ysize,
				  tmp_scanline_stride, 0,
				  dst_scanline_stride, 0);
	} else {
	    //
	    // Handle rect offsets during copy
	    //
	    // TODO: Add code to optimize BIGENDIAN case.  See
	    // ErrorDiffusion16to1Memory.cc in XIL 1.2 code.
	    //
	    for (; ysize > 0; ysize--) {
		for (unsigned int rect_x = 0; rect_x < xsize; rect_x++) {
		    if (XIL_BMAP_TST(tmp_scanline,
				     tmp_rect_offset + rect_x)) {
			XIL_BMAP_SET(dst_scanline,
				     dst_rect_offset + rect_x);
		    } else {
			XIL_BMAP_CLR(dst_scanline,
				     dst_rect_offset + rect_x);
		    }
		}
		//
		// go to next scanline
		//
		tmp_scanline += tmp_scanline_stride;
		dst_scanline += dst_scanline_stride;
	    }
	}
    }
}

void
xili_diffusion_copy_8(Xil_unsigned8* tmp_data,
		      unsigned int tmp_scanline_stride,
		      Xil_unsigned8* dst_data,
		      unsigned int dst_scanline_stride,
		      unsigned int dst_pixel_stride,
		      XilRoi* roi,
		      XilBox* dst_box,
		      int box_x, int box_y)
{
    //
    //  Create a list of rectangles to loop over.  The resulting list
    //  of rectangles is the area left by intersecting the ROI with
    //  the destination box.
    //
    XilRectList    rl(roi, dst_box);
	    
    int            x;
    int            y;
    unsigned int   xsize;
    unsigned int   ysize;
    while(rl.getNext(&x, &y, &xsize, &ysize)) {
	Xil_unsigned8* dst_scanline = dst_data +
	    y * dst_scanline_stride +
	    x * dst_pixel_stride;
	// Since tmp is in src image space, translate (x, y)
	// which is in box space to image space
	Xil_unsigned8* tmp_scanline = tmp_data +
	    (y + box_y) * tmp_scanline_stride +
	    (x + box_x);

	if (dst_pixel_stride == 1) {
	    // Want to copy all the bands in storage
	    xili_copy8_all_bands(tmp_scanline, dst_scanline,
				 xsize * sizeof(Xil_unsigned8),
				 ysize, tmp_scanline_stride,
				 dst_scanline_stride);
	} else {
	    // Want to copy only some of bands in storage
	    xili_copy8_child(tmp_scanline, dst_scanline, xsize,
			     ysize, 1, tmp_scanline_stride, 1,
			     dst_scanline_stride, dst_pixel_stride);
	}
    }
}

void
xili_diffusion_copy_16(Xil_signed16* tmp_data,
		       unsigned int tmp_scanline_stride,
		       Xil_signed16* dst_data,
		       unsigned int dst_scanline_stride,
		       unsigned int dst_pixel_stride,
		       XilRoi* roi,
		       XilBox* dst_box,
		       int box_x, int box_y)
{
    //
    //  Create a list of rectangles to loop over.  The resulting list
    //  of rectangles is the area left by intersecting the ROI with
    //  the destination box.
    //
    XilRectList    rl(roi, dst_box);
	    
    int            x;
    int		   y;
    unsigned int   xsize;
    unsigned int   ysize;
    while(rl.getNext(&x, &y, &xsize, &ysize)) {
	Xil_signed16* dst_scanline = dst_data +
	    y * dst_scanline_stride +
	    x * dst_pixel_stride;
	// Since tmp is in src image space, translate (x, y)
	// which is in box space to image space
	Xil_signed16* tmp_scanline = tmp_data +
	    (y + box_y) * tmp_scanline_stride +
	    (x + box_x);

	if (dst_pixel_stride == 1) {
	    // Want to copy all the bands in storage
	    xili_copy16_all_bands(tmp_scanline, dst_scanline,
				  xsize * sizeof(Xil_signed16),
				  ysize, tmp_scanline_stride,
				  dst_scanline_stride);
	} else {
	    // Want to copy only some of bands in storage
	    xili_copy16_child(tmp_scanline, dst_scanline,
			      xsize, ysize, 1,
			      tmp_scanline_stride, 1,
			      dst_scanline_stride, dst_pixel_stride);
	}
    }
}

//
// Copies image data from a temporary image to a destination image taking
// into account the ROI.
//
void
xili_diffusion_copy_f32(Xil_float32* tmp_data,
			unsigned int tmp_scanline_stride,
			Xil_float32* dst_data,
			unsigned int dst_scanline_stride,
			unsigned int dst_pixel_stride,
			XilRoi* roi,
			XilBox* dst_box,
			int box_x, int box_y)
{
    //
    //  Create a list of rectangles to loop over.  The resulting list
    //  of rectangles is the area left by intersecting the ROI with
    //  the destination box.
    //
    XilRectList    rl(roi, dst_box);
	    
    int            x;
    int            y;
    unsigned int   xsize;
    unsigned int   ysize;
    while(rl.getNext(&x, &y, &xsize, &ysize)) {
	Xil_float32* dst_scanline = dst_data +
	    y * dst_scanline_stride +
	    x * dst_pixel_stride;
	// Since tmp is in src image space, translate (x, y)
	// which is in box space to image space
	Xil_float32* tmp_scanline = tmp_data +
	    (y + box_y) * tmp_scanline_stride +
	    (x + box_x);

	if (dst_pixel_stride == 1) {
	    // Want to copy all the bands in storage
	    xili_copyF_all_bands(tmp_scanline, dst_scanline,
				 xsize * sizeof(Xil_float32),
				 ysize, tmp_scanline_stride,
				 dst_scanline_stride);
	} else {
	    // Want to copy only some of bands in storage
	    xili_copyF_child(tmp_scanline, dst_scanline,
			     xsize, ysize, 1,
			     tmp_scanline_stride, 1,
			     dst_scanline_stride, dst_pixel_stride);
	}
    }
}
