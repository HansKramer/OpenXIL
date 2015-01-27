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
//  File:	SubsampleBinaryToGray.cc
//  Project:	XIL
//  Revision:	1.23
//  Last Mod:	10:10:44, 03/10/00
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
//  MT-level:  SAFE
//	
//------------------------------------------------------------------------
//	COPYRIGHT
//------------------------------------------------------------------------
#pragma ident	"@(#)SubsampleBinaryToGray.cc	1.23\t00/03/10  "

#include "XilDeviceManagerComputeBYTE.hh"
#include "ComputeInfo.hh"
#include "XiliUtils.hh"
#include "xili_geom_utils.hh"

// TODO  bpb  03/12/1997  Get rid of DEBUG_SUBSAMPLE blocks
#define DEBUG_SUBSAMPLE 0
#define DEBUG_RECTS     0

//
// forward declarations
//

#if DEBUG_SUBSAMPLE
void print_bits(unsigned char* b, unsigned int line_stride, int w, int h);
void print_bytes(unsigned char* b, unsigned int line_stride, int w, int h);
#endif

static XilStatus subsample_binary_to_gray(SubsampleData subsample_data);

static void Subsample1to8_2x2(Xil_unsigned8* src_scanline0,
                              Xil_unsigned8* dst_scanline,
                              unsigned int   d_xsize,
                              unsigned int   d_ysize,
                              unsigned int   src_nx_scan,
                              unsigned int   dst_nx_scan);
static void Subsample1to8_3x3(Xil_unsigned8* src_scanline,
                              Xil_unsigned8* dst_scanline,
                              unsigned int   d_xsize,
                              unsigned int   d_ysize,
                              unsigned int   src_nx_scan,
                              unsigned int   dst_nx_scan);
static void Subsample1to8_4x4(Xil_unsigned8* src_scanline,
                              Xil_unsigned8* dst_scanline,
                              unsigned int   d_xsize,
                              unsigned int   d_ysize,
                              unsigned int   src_nx_scan,
                              unsigned int   dst_nx_scan);
#ifdef XIL_LITTLE_ENDIAN
inline unsigned int swap_bytes(unsigned int buf)
{
    _XILI_BSWAP(buf);
    return buf;
}
#endif

XilStatus
XilDeviceManagerComputeBYTE::SubsampleBinaryToGray(XilOp*       op,
						   unsigned     ,
						   XilRoi*      roi,
						   XilBoxList*  bl)
{
    XilStatus    status        = XIL_SUCCESS;
    SubsampleData sd;

    sd.op  = op;
    sd.roi = roi;

    op->getParam(1, &sd.xscale);    
    op->getParam(2, &sd.yscale);

   //
    // Get the basic data, assuming that the src image is a BIT image.
    // roi is the complete image ROI. This means that src and dst ROI's are
    // already taken into account, and the roi passed is the intersection
    // of these. 
    //
    if(op->splitOnTileBoundaries(bl) == XIL_FAILURE) {
        return XIL_FAILURE;
    }

    XilImage* src = op->getSrcImage(1);
    XilImage* dst = op->getDstImage(1);
    XilBox*   src_box;
    XilBox*   dst_box;

    sd.nbands = dst->getNumBands();

    
    while(bl->getNext(&src_box, &dst_box)) {
	//
        //  Aquire our storage from the images.  The storage returned is valid
        //  for the box given.  Thus, any origins or child offsets have been
        //  taken into account. Currently we only think in terms of "inner"
	//  tiles and this code works only for the case of entire image.
        //
        XilStorage  src_storage(src);
        XilStorage  dst_storage(dst);
        if((src->getStorage(&src_storage, op, src_box, "XilMemory",
			    XIL_READ_ONLY)  == XIL_FAILURE) ||
           (dst->getStorage(&dst_storage, op, dst_box, "XilMemory",
			    XIL_WRITE_ONLY) == XIL_FAILURE)) {
            //
            //  Mark this box as failed and if that succeeds, continue
            //  processing the next box.  Otherwise, return XIL_FAILURE now.
            //
            if(bl->markAsFailed() == XIL_FAILURE) {
                return XIL_FAILURE;
            } else {
                continue;
            }
        }
	sd.src_storage = &src_storage;
	sd.dst_storage = &dst_storage;
	sd.src_box     = src_box;
	sd.dst_box     = dst_box;

	status = subsample_binary_to_gray(sd);

    } // end of while(bl->getNext)

    return status;
    
}

static XilStatus
subsample_binary_to_gray(SubsampleData subsample_data)
{
    XilStatus    status        = XIL_SUCCESS;
    XilStorage*  src_storage   = subsample_data.src_storage;
    XilStorage*  dst_storage   = subsample_data.dst_storage;
    XilBox*      src_box       = subsample_data.src_box;
    XilBox*      dst_box       = subsample_data.dst_box;
    XilRoi*      roi           = subsample_data.roi;
    unsigned int nbands        = subsample_data.nbands;
    float        xscale        = subsample_data.xscale;
    float        yscale        = subsample_data.yscale;

    int          i, j;         // loop index variables

    //
    // Floating point block sizes used in subsample adaptive are
    // rounded up, to obtain blocks that contain whole number of
    // pixels.
    // Depending of how many pixels are there in the block the
    // destination pixel is assigned one of  the values in the
    // range 0..block_area. For example, if the scaling factor would
    // require a fractional block of size 2.2 x 2.2, the block
    // would be rounded up to 3x3 block resulting in 10 possible
    // intensity values. Therefor the range of the dst pixel values
    // depends on the scaling factors.
    //
    int         	big_block_xsize;
    int                 big_block_ysize;
    int			reg_block_xsize;
    int                 reg_block_ysize;

    //
    // we construct lookup tables to normalize the output
    //
    Xil_unsigned8 *     big_table      = NULL; 
    Xil_unsigned8 *     medium_table_1 = NULL;
    Xil_unsigned8 *     medium_table_2 = NULL;
    Xil_unsigned8 *     small_table    = NULL;

    Xil_boolean    tables_initialized = FALSE; // general case init flag

    Xil_boolean    check_for_optimized_case;
    unsigned int   inverse_scale;

    if(XILI_FLT_EQ(xscale, (1.0F/2.0F)) &&
       XILI_FLT_EQ(yscale, (1.0F/2.0F))) {
        check_for_optimized_case = TRUE;
        inverse_scale            = 2;
    } else if(XILI_FLT_EQ(xscale, (1.0F/3.0F), 0.0033F) &&
              XILI_FLT_EQ(yscale, (1.0F/3.0F), 0.0033F)) {
        check_for_optimized_case = TRUE;
        inverse_scale            = 3;
    } else if(XILI_FLT_EQ(xscale, (1.0F/4.0F)) &&
              XILI_FLT_EQ(yscale, (1.0F/4.0F))) {
        check_for_optimized_case = TRUE;
        inverse_scale            = 4;
    } else {
        check_for_optimized_case = FALSE;
        inverse_scale            = 1;
    }

    //
    // Get a reference to the destination image.
    //
    XilImage* dst = dst_storage->getImage();

    //
    // Calculate the inverse scale in both dimensions and adjust the
    // original scale values if necessary.
    //
    float inv_xscale;
    float inv_yscale;
    if(check_for_optimized_case == TRUE) {
        //
        // If it's possible that any of the boxes could be
        // processed by one of the optimized routines, then force
        // the inverse scale to be equal to that which applies in
        // that case. This avoids pixel value discontinuities at
        // boundaries between boxes processed by the two different
        // methods.
        //
        inv_xscale = inv_yscale = (float) inverse_scale;
        xscale = yscale = 1.0F/inverse_scale;
    } else {
        //
        // we allow scale to 0.0, however this is a dangerous
        // assumption here.
        //
        if(xscale != 0.0F) // TODO bpb 03/27/1997 ...if xscale == 0?
            inv_xscale = 1.0F / xscale;
        if(yscale != 0.0F) // TODO bpb 03/27/1997 ...if yscale == 0?
            inv_yscale = 1.0F / yscale;

#if 0
        // TODO  bpb  04/28/1997  OK to adjust scale factors in compute routine?
        // This might be causing some problems as it would cause scale factors
        // to differ from those used in box calculations.
        //
        // If rounding the inverse scale to the nearest integer
        // would cause an error of less than half a pixel then
        // round it and adjust the original scale to match.
        //
        if(XILI_FLT_EQ(inv_xscale,
                       (float)_XILI_ROUND(inv_xscale),
                       0.5F / dst->getWidth())) {
            inv_xscale = (float)_XILI_ROUND(inv_xscale);
            xscale = 1.0F / inv_xscale;
        }
        if(XILI_FLT_EQ(inv_yscale,
                       (float)_XILI_ROUND(inv_yscale),
                       0.5F / dst->getHeight())) {
            inv_yscale = (float)_XILI_ROUND(inv_yscale);
            yscale = 1.0F / inv_yscale;
        }
#endif
    }

    //
    // Get the source box location and size in image space.
    //
    int          srcB_x;
    int          srcB_y;
    unsigned int srcB_xsize;
    unsigned int srcB_ysize;
    src_box->getAsRect(&srcB_x, &srcB_y, &srcB_xsize, &srcB_ysize);

    //
    //  Create a list of rectangles to loop over.  The resulting list
    //  of rectangles is the area left by intersecting the ROI with
    //  the destination box.
    //
    XilRectList    rl(roi, dst_box);

    int            dstR_x;
    int            dstR_y;
    unsigned int   dstR_xsize;
    unsigned int   dstR_ysize;
    while(rl.getNext(&dstR_x, &dstR_y, &dstR_xsize, &dstR_ysize)) {
        //
        // Calculate the dimensions of the source rectangle and
        // its position in box space.
        //
        int srcR_x = (int)(dstR_x*inv_xscale);
        int srcR_y = (int)(dstR_y*inv_yscale);

        //
        // Src rect size is ceil() per man page.
        //
        unsigned int srcR_xsize = (int)ceil((double)(dstR_xsize*inv_xscale));
        unsigned int srcR_ysize = (int)ceil((double)(dstR_ysize*inv_yscale));

        //
        // Clip the src rect size to be within the src box
        //
        srcR_xsize = _XILI_MIN(srcR_xsize, srcB_xsize - srcR_x);
        srcR_ysize = _XILI_MIN(srcR_ysize, srcB_ysize - srcR_y);

#if DEBUG_RECTS
        fprintf(stderr,
                "dst rect (%d, %d), %d x %d\n",
                 dstR_x, dstR_y, dstR_xsize, dstR_ysize);
        fprintf(stderr,
                "src rect (%d, %d), %d x %d\n",
                 srcR_x, srcR_y, srcR_xsize, srcR_ysize);
#endif
	//
	//  Each Band...
	//
	for(unsigned int band = 0; band < nbands; band++) {
	    //
	    // src image is a multi-banded bit image
	    //
            Xil_unsigned8* src_data;
            unsigned int   src_offset;
            unsigned int   src_scanline_stride;
	    src_storage->getStorageInfo(band,
					NULL, 
					&src_scanline_stride,
					&src_offset,
					(void**)&src_data);
	    //
	    // dst image is a multi-banded byte image
	    //
            Xil_unsigned8* dst_data;
            unsigned int   dst_pixel_stride;
            unsigned int   dst_scanline_stride;
	    dst_storage->getStorageInfo(band,
					&dst_pixel_stride,
					&dst_scanline_stride,
					NULL,
					(void**)&dst_data);

            Xil_unsigned8* srcR_data = src_data +
                               srcR_y * src_scanline_stride +
                               (src_offset + srcR_x) / 8;
            unsigned int srcR_offset = (src_offset + srcR_x) % 8;
            Xil_unsigned8* dstR_data = dst_data +
                               dstR_y * dst_scanline_stride +
                               dstR_x * dst_pixel_stride;

            //
            //  Check for optimized cases and process as such if possible.
            //
            if((check_for_optimized_case == TRUE) &&
                (((unsigned long)dst_data & 0x3) == 0) &&
                (srcR_offset == 0) &&
                (dst_pixel_stride == 1) &&
                (((inverse_scale == 3) && (dst_scanline_stride % 8 == 0))
                 || (dst_scanline_stride % 4) == 0) &&
//
// TODO  bpb  03/14/1997  Determine whether x block size restriction is needed.
//
// In the original XIL 1.2 subsample 1 to 8 optimizations a restriction was
// placed on the x size of the destination block. The ported optimized routines 
// have been extended to handle arbitrary x sizes. It may be that this
// extension does not work correctly in some cases. If so the restriction might
// need to be reinstated.
//
#if 0
                (((inverse_scale == 3) && (dstR_xsize % 8 == 0)) ||
                (dstR_xsize % 4 == 0)) &&
#endif
               (srcR_xsize >= inverse_scale * dstR_xsize) &&
               (srcR_ysize >= inverse_scale * dstR_ysize)) {

#if DEBUG_RECTS
        fprintf(stderr, "optimized case\n\n");
#endif

                if(inverse_scale == 2) {
                    Subsample1to8_2x2(srcR_data,
                                      dstR_data,
                                      dstR_xsize,
                                      dstR_ysize,
                                      src_scanline_stride,
                                      dst_scanline_stride);
                } else if(inverse_scale == 4) {
                    Subsample1to8_4x4(srcR_data,
                                      dstR_data,
                                      dstR_xsize,
                                      dstR_ysize,
                                      src_scanline_stride,
                                      dst_scanline_stride);
                } else {
                    Subsample1to8_3x3(srcR_data,
                                      dstR_data,
                                      dstR_xsize,
                                      dstR_ysize,
                                      src_scanline_stride,
                                      dst_scanline_stride);
                }
                continue;
            }   // end optimized cases.

            //
            // The remainder of the loop over bands is for the general case.
            // The values and arrays specific to this case are initialized
            // the first time that they are required.
            //
            if(tables_initialized == FALSE) {
                //
                // Set the initialization flag so this block is not re-entered.
                //
                tables_initialized = TRUE;

                //
                // TODO: igor Thu Oct  3 10:37:28 1996
                // There is a problem here: big_block_xsize can be smaller
                // than the block size used further down compute routine. I am
                // sure this is now what it was meant by big_block_xsize that
                // is encompassing the averaging area. For example, for 1/3
                // scale big_block_xsize is still 3, while the block_x_size
                // becomes 4
                //
                big_block_xsize = (int)ceil((double)inv_xscale);
                big_block_ysize = (int)ceil((double)inv_yscale);
                reg_block_xsize = (int)(inv_xscale); // use floor here
                reg_block_ysize = (int)(inv_yscale);    

                //
                // The area of sampling determines nuber of gray
                // levels (area+1).
                //
                int big_area      =
                    big_block_xsize * big_block_ysize; // ceil in  x and y
                int medium_area_1 =
                    big_block_xsize * reg_block_ysize; // ceil in x, floor in y
                int medium_area_2 = reg_block_xsize * big_block_ysize;
                int small_area    = reg_block_xsize * reg_block_ysize;

                //
                //  Get system state for reporting errors
                //
                XilSystemState* err_state = dst->getSystemState();

                //
                // construct lookup tables to normalize output
                //
                big_table = new Xil_unsigned8 [big_area + 1];
                if(big_table == NULL) {
                    XIL_ERROR(err_state, XIL_ERROR_RESOURCE, "di-1", TRUE);
                    return XIL_FAILURE;
                }
                medium_table_1 = new Xil_unsigned8 [medium_area_1 + 1];
                if(medium_table_1 == NULL) {
                    delete [] big_table;
                    XIL_ERROR(err_state, XIL_ERROR_RESOURCE, "di-1", TRUE);
                    return XIL_FAILURE;
                }
                medium_table_2 = new Xil_unsigned8 [medium_area_2 + 1];
                if(medium_table_2 == NULL) {
                    delete [] big_table;
                    delete [] medium_table_1;
                    XIL_ERROR(err_state, XIL_ERROR_RESOURCE, "di-1", TRUE);
                    return XIL_FAILURE;
                }
                small_table = new Xil_unsigned8 [small_area + 1];
                if(small_table == NULL) {
                    delete [] big_table;
                    delete [] medium_table_1;
                    delete [] medium_table_2;
                    XIL_ERROR(err_state, XIL_ERROR_RESOURCE, "di-1", TRUE);
                    return XIL_FAILURE;
                }

                //
                // clip on the XIL_MAXBYTE.
                // This clipping will take affect for very small
                // scales. All the tables have normalized values
                // to the same range 0..MIN(XIL_MAXBYTE, big_area).
                //
                for (i = 0; i <= big_area; i++)
            	    big_table[i] = (i > XIL_MAXBYTE ? XIL_MAXBYTE : i);
    
                float ttmp;
                for (i = 0; i <= medium_area_1; i++)
                {
            	    ttmp = (((float) i)/ reg_block_ysize) * big_block_ysize;
	            medium_table_1[i] = (Xil_unsigned8)
	                (ttmp > (float)XIL_MAXBYTE ? XIL_MAXBYTE : ttmp);
                }
                for (i = 0; i <= medium_area_2; i++)
                {
    	            ttmp = (((float) i)/reg_block_xsize) * big_block_xsize;
	            medium_table_2[i] = (Xil_unsigned8)
	                (ttmp > (float)XIL_MAXBYTE ? XIL_MAXBYTE : ttmp);
                }
                for (i = 0; i <= small_area; i++)
                {
    	            ttmp = (((float) i)/small_area) * big_area;
	            small_table[i] = (Xil_unsigned8)
	                (ttmp > (float)XIL_MAXBYTE ? XIL_MAXBYTE : ttmp);
                }
            } // end initialization block

            //
	    // Src box is divided into changing size blocks. Each block
	    // is defined as the rectangle that will contain all
	    // the src pixels mapped into a single destination pixel.
	    // We compute the block size from the values of the scale
	    // and the size of the src_box. src and dst box origins are
	    // already taken care of so we do computations relative to
	    // their spaces. Block coordinates are integers.
	    //
	    unsigned int block_x, block_y; // block top left corner
	    int   dst_x, dst_y; // dst pixel coordinates into which block maps

	    float dx;		// exact increment due to the x scale
	    float dy;		// exact increment due to the y scale


	    //
	    // computation is relative the src box. src_data
	    // points to the upper left corner of the src_box
	    //
	    block_x = 0;
	    block_y = 0;

	    dy = 0.0;

            Xil_unsigned8* src_scanline = srcR_data;
            Xil_unsigned8* dst_scanline = dstR_data;

#if DEBUG_RECTS
        fprintf(stderr, "UNoptimized case:  %d %d %d %d %d %d %d\n",
                (check_for_optimized_case == TRUE),
                (((unsigned int)dst_data & 0x3) == 0),
                (srcR_offset == 0),
                (dst_pixel_stride == 1),
                (((inverse_scale == 3) && (dst_scanline_stride % 8 == 0))
                 || (dst_scanline_stride % 4) == 0),
               (srcR_xsize >= inverse_scale * dstR_xsize),
               (srcR_ysize >= inverse_scale * dstR_ysize));
#endif
	    //
	    // compute all the blocks in the src box
	    //
	    while ( block_y < srcR_ysize){
		//
		// find the y block size
		//
		dst_y = (int) dy;
		//
		// keep increasing block size while it will still map
		// into one dst pixel
		//
		int block_y_size = 1;

		while ( dst_y == (int) (dy = dy + yscale))
		    block_y_size += 1;
		//
		// if we have gone over the src border, clip down
		// the last rectangle.
		//
		if ( block_y + block_y_size >= srcR_ysize)
		    block_y_size = srcR_ysize - block_y;

		dx = block_x * xscale;
		
		Xil_unsigned8* dst_pixel = dst_scanline +
                    ((int) dx) * dst_pixel_stride;

		while ((int) block_x < (int) srcR_xsize) {
		    //
		    // find the x block size
		    //
		    dst_x = (int) dx;

		    int block_x_size = 1;
		    
		    while ( dst_x == (int) (dx = dx + xscale))
			block_x_size += 1;
		    //
		    // if we have gone over the src border, clip down
		    // the last rectangle.
		    //
		    if(block_x + block_x_size >= srcR_xsize)
			block_x_size = srcR_xsize - block_x;

#if DEBUG_RECTS
    fprintf(stderr, "src: %d %d %d %d\n",
                     block_x, block_y, block_x_size, block_y_size);
    fprintf(stderr, "dst: %d %d %f %f\n",
                     dst_x, dst_y, dx, dy);
#endif
		    if(((dst_x >= 0) && (dst_x < dstR_xsize)) &&
			 ((dst_y >= 0) && (dst_y < dstR_ysize))) {
			//
			// in each band find the number of on pixels in the 
			// block defined by dst_x, dst_y, block_x_size,
			// block_y_size. Remember that the bit images have
			// block storage.
			//
                        Xil_unsigned8* src_pixel = src_scanline +
                            (block_y * src_scanline_stride) +
                            (block_x + srcR_offset) / 8;

			unsigned int src_bit = (((int) block_x) +
                                                    srcR_offset) % 8;

			Xil_unsigned8* src_band = src_pixel;
			
		        Xil_unsigned8* dst_band = dst_pixel;
          
 			unsigned int bits_on = 0;
			for (i = 0; i < block_y_size; i++) {
			    for (j = 0; j < block_x_size; j++) {
			        if(XIL_BMAP_TST(src_band, src_bit + j))
				    bits_on += 1;
			    }
			    src_band += src_scanline_stride;
			}

			if ((block_x_size == big_block_xsize) &&
			    (block_y_size == big_block_ysize))
			    *dst_band = big_table[bits_on];
			else if ((block_x_size == big_block_xsize) &&
				 (block_y_size == reg_block_ysize))
			    *dst_band = medium_table_1[bits_on];
			else if ((block_x_size == reg_block_xsize) &&
				 (block_y_size == big_block_ysize))
			    *dst_band = medium_table_2[bits_on];
			else if ((block_x_size == reg_block_xsize) &&
				 (block_y_size == reg_block_ysize))
			    *dst_band = small_table[bits_on];
			else
                            *dst_band = (Xil_unsigned8)
                        (((float) bits_on / (block_x_size * block_y_size)) *
                        (big_block_xsize * big_block_ysize));
		    } // end of dst band value computation (if ...)
		    
		block_x += block_x_size;
		dst_pixel += dst_pixel_stride;
		
		} // end of while in x direction
		
		block_x = 0;
		block_y += block_y_size;
		
		dst_scanline += dst_scanline_stride;
		
	    } // end of while in y direction
	} // end of computation for each band
    } // end of loop over destination rectangles
    
    if (big_table != NULL)
	delete big_table;
    if (medium_table_1 != NULL)
	delete medium_table_1;
    if (medium_table_2 != NULL)
	delete medium_table_2;
    if (small_table != NULL)
	delete small_table;
    return status;
}
 
static void Subsample1to8_2x2(Xil_unsigned8* src_scanline0,
                              Xil_unsigned8* dst_scanline,
                              unsigned int   d_xsize,
                              unsigned int   d_ysize,
                              unsigned int   src_nx_scan,
                              unsigned int   dst_nx_scan)
{
#if DEBUG_SUBSAMPLE
    fprintf(stderr, "In Subsample1to8_2x2().\n");
#endif
    unsigned int i;
    unsigned char bitson[4] = { 0, 1, 1, 2 };

    unsigned int  lut[256];
    for (i = 0; i < 256; i++) { // build lookup table
	lut[i] = (bitson[(i>>6) & 0x3] << 24) + (bitson[(i>>4) & 0x3] << 16) +
		 (bitson[(i>>2) & 0x3] <<  8) + (bitson[i & 0x3]);
    }

    unsigned int dst_chunks = d_xsize / 4;
    unsigned int dst_extra  = d_xsize % 4;

    Xil_unsigned8* src_scanline1 = src_scanline0 + src_nx_scan;

    for(i = 0; i < d_ysize; i++) { // scale to grey via lookup
        Xil_unsigned8* src_pixel0 = src_scanline0;
        Xil_unsigned8* src_pixel1 = src_scanline1;
	unsigned int*  dst_pixel = (unsigned int *) dst_scanline;

	for(unsigned int j = 0; j < dst_chunks; j++) {
#ifndef XIL_LITTLE_ENDIAN
	    *dst_pixel++ = lut[*src_pixel0++] + lut[*src_pixel1++];
#else
	    *dst_pixel++ = swap_bytes(lut[*src_pixel0++] + lut[*src_pixel1++]);
#endif
        }

        if(dst_extra != 0) {
#if DEBUG_SUBSAMPLE
    fprintf(stderr, "dst_extra = %d\n", dst_extra);
#endif
            unsigned int dbuf;
            Xil_unsigned8* dst_byte = (Xil_unsigned8*)dst_pixel;
            switch(dst_extra) {
                case 1:
                    dbuf = lut[0xc0 & (*src_pixel0)] +
                           lut[0xc0 & (*src_pixel1)];
                    *dst_byte = (Xil_unsigned8)((dbuf & 0xff000000) >> 24);
                    break;
                case 2:
                    dbuf = lut[0xf0 & (*src_pixel0)] +
                           lut[0xf0 & (*src_pixel1)];
                    *dst_byte++ = (Xil_unsigned8)((dbuf & 0xff000000) >> 24);
                    *dst_byte   = (Xil_unsigned8)((dbuf & 0x00ff0000) >> 16);
                    break;
                case 3:
                    dbuf = lut[0xfc & (*src_pixel0)] +
                           lut[0xfc & (*src_pixel1)];
                    *dst_byte++ = (Xil_unsigned8)((dbuf & 0xff000000) >> 24);
                    *dst_byte++ = (Xil_unsigned8)((dbuf & 0x00ff0000) >> 16);
                    *dst_byte   = (Xil_unsigned8)((dbuf & 0x0000ff00) >>  8);
                    break;
            }
        }

	src_scanline0 = src_scanline1 + src_nx_scan;
	src_scanline1 = src_scanline0 + src_nx_scan;
	dst_scanline += dst_nx_scan;
    }
}

static void Subsample1to8_3x3(Xil_unsigned8* src_scanline,
                              Xil_unsigned8* dst_scanline,
                              unsigned int   d_xsize,
                              unsigned int   d_ysize,
                              unsigned int   src_nx_scan,
                              unsigned int   dst_nx_scan)
{
#if DEBUG_SUBSAMPLE
    fprintf(stderr, "In Subsample1to8_3x3().\n");
#endif
#if 0
    Xil_unsigned8* dst_addr = dst_scanline;
    print_bits(src_scanline, src_nx_scan, 3*d_xsize, 3*d_ysize);
#endif
    unsigned int i;
    unsigned char bitson[8] = { 0, 1, 1, 2, 1, 2, 2, 3 };

    unsigned int  lut0[1024];
    unsigned int* lut1 = lut0 + 256;
    unsigned int* lut2 = lut0 + 512;
    unsigned int* lut3 = lut0 + 768;

    /*
     * build lookup tables
     */
    for(i = 0; i < 256; i++) {
	    
	unsigned char* c0 = (unsigned char *) &(lut0[i]);
	unsigned char* c1 = (unsigned char *) &(lut1[i]);
	unsigned char* c2 = (unsigned char *) &(lut2[i]);
	unsigned char* c3 = (unsigned char *) &(lut3[i]);

	*c0 = bitson[(i >> 5) & 0x7];
	*(c0+1) = bitson[(i >> 2) & 0x7];
	*(c0+2) = bitson[i & 0x3];
	*(c0+3) = 0;

	*c1 = 0;
	*(c1+1) = 0;
	*(c1+2) = bitson[(i >> 7) & 0x1];
	*(c1+3) = bitson[(i >> 4) & 0x7];

	*c2 = bitson[(i >> 1) & 0x7];
	*(c2+1) = bitson[i & 0x1];
	*(c2+2) = 0;
	*(c2+3) = 0;

	*c3 = 0;
	*(c3+1) = bitson[(i >> 6) & 0x3];
	*(c3+2) = bitson[(i >> 3) & 0x7];
	*(c3+3) = bitson[i & 0x7];
    }

    unsigned int src_nx_3scan  = src_nx_scan * 3;

    unsigned int dst_chunks = d_xsize / 8;
    unsigned int dst_extra  = d_xsize % 8;

    for(i = 0; i < d_ysize; i++) {

	Xil_unsigned8* src_pixel0 = src_scanline;
	Xil_unsigned8* src_pixel1 = src_pixel0 + src_nx_scan;
	Xil_unsigned8* src_pixel2 = src_pixel1 + src_nx_scan;

	unsigned int* dst_pixel = (unsigned int *) dst_scanline;

	for(unsigned int j = 0; j < dst_chunks; j++) {

	    unsigned char b0 = (unsigned char)(*(src_pixel0));
	    unsigned char b1 = (unsigned char)(*(src_pixel1));
	    unsigned char b2 = (unsigned char)(*(src_pixel2));

	    unsigned int i0 = lut0[b0] + lut0[b1] + lut0[b2];

	    b0 = *(src_pixel0+1);
	    b1 = *(src_pixel1+1);
	    b2 = *(src_pixel2+1);

	    i0 += lut1[b0] + lut1[b1] + lut1[b2];

	    unsigned int i1 = lut2[b0] + lut2[b1] + lut2[b2];

	    b0 = *(src_pixel0+2);
	    b1 = *(src_pixel1+2);
	    b2 = *(src_pixel2+2);

	    i1 += lut3[b0] + lut3[b1] + lut3[b2];

	    src_pixel0 += 3;
	    src_pixel1 += 3;
	    src_pixel2 += 3;

            //
            // Note: There is no XIL_LITTLE_ENDIAN conditional compilation
            // block for swapping bytes here because the lookup table itself
            // is byte-swapped so that the results in i0 and i1 are already
            // byte-swapped.
            //
	    *dst_pixel++ = i0;
	    *dst_pixel++ = i1;
	}

        if(dst_extra != 0) {
#if DEBUG_SUBSAMPLE
    fprintf(stderr, "dst_extra = %d\n", dst_extra);
#endif
            Xil_unsigned8* dst_byte;
            unsigned int i0;

	    unsigned char b0 = (unsigned char)(*(src_pixel0));
    	    unsigned char b1 = (unsigned char)(*(src_pixel1));
	    unsigned char b2 = (unsigned char)(*(src_pixel2));

            if(dst_extra < 4) {
                dst_byte = (Xil_unsigned8*)dst_pixel;
                switch(dst_extra) {
                    case 1:
	                i0 = lut0[0xe0 & b0] + lut0[0xe0 & b1] + lut0[0xe0 & b2];
                        *dst_byte = (Xil_unsigned8)((i0 & 0xff000000) >> 24);
                        break;
                    case 2:
	                i0 = lut0[0xfc & b0] + lut0[0xfc & b1] + lut0[0xfc & b2];
                        *dst_byte++ = (Xil_unsigned8)((i0 & 0xff000000) >> 24);
                        *dst_byte   = (Xil_unsigned8)((i0 & 0x00ff0000) >> 16);
                        break;
                    case 3:
	                i0 = lut1[b0] + lut1[b1] + lut1[b2];

	                b0 = *(src_pixel0+1);
	                b1 = *(src_pixel1+1);
	                b2 = *(src_pixel2+1);

	                i0 += lut1[0x80 & b0] + lut1[0x80 & b1] + lut1[0x80 & b2];
                        *dst_byte++ = (Xil_unsigned8)((i0 & 0xff000000) >> 24);
                        *dst_byte++ = (Xil_unsigned8)((i0 & 0x00ff0000) >> 16);
                        *dst_byte   = (Xil_unsigned8)((i0 & 0x0000ff00) >>  8);
                        break;
                }
            } else {
	        i0 = lut0[b0] + lut0[b1] + lut0[b2];

	        b0 = *(src_pixel0+1);
	        b1 = *(src_pixel1+1);
	        b2 = *(src_pixel2+1);

                dst_byte = (Xil_unsigned8*)(dst_pixel + 1);

                switch(dst_extra) {
                    unsigned int i1;
                    case 4:
	                i0 += lut1[b0 & 0xf0] + lut1[b1 & 0xf0] + lut1[b2 & 0xf0];
                        *dst_pixel = i0;
                        break;
                    case 5:
	                i0 += lut1[b0 & 0xfe] + lut1[b1 & 0xfe] + lut1[b2 & 0xfe];
	                i1 = lut2[b0 & 0xfe] + lut2[b1 & 0xfe] + lut2[b2 & 0xfe];
                        *dst_pixel = i0;
                        *dst_byte  = (Xil_unsigned8)((i1 & 0xff000000) >> 24);
                        break;
                    case 6:
	                i0 += lut1[b0] + lut1[b1] + lut1[b2];
	                i1  = lut2[b0] + lut2[b1] + lut2[b2];

	                b0 = *(src_pixel0+2);
            	        b1 = *(src_pixel1+2);
            	        b2 = *(src_pixel2+2);

	                i1 += lut3[b0 & 0xc0] + lut3[b1 & 0xc0] + lut3[b2 & 0xc0];

                        *dst_pixel  = i0;
                        *dst_byte++ = (Xil_unsigned8)((i1 & 0xff000000) >> 24);
                        *dst_byte   = (Xil_unsigned8)((i1 & 0x00ff0000) >> 16);
                        break;
                    case 7:
	                i0 += lut1[b0] + lut1[b1] + lut1[b2];
	                i1  = lut2[b0] + lut2[b1] + lut2[b2];

	                b0 = *(src_pixel0+2);
            	        b1 = *(src_pixel1+2);
            	        b2 = *(src_pixel2+2);

	                i1 += lut3[b0 & 0xf8] + lut3[b1 & 0xf8] + lut3[b2 & 0xf8];

                        *dst_pixel  = i0;
                        *dst_byte++ = (Xil_unsigned8)((i1 & 0xff000000) >> 24);
                        *dst_byte++ = (Xil_unsigned8)((i1 & 0x00ff0000) >> 16);
                        *dst_byte   = (Xil_unsigned8)((i1 & 0x0000ff00) >>  8);
                        break;
                }
            }
        }

	src_scanline += src_nx_3scan;
	dst_scanline += dst_nx_scan;
    }
#if 0
    print_bytes(dst_addr, dst_nx_scan, d_xsize, d_ysize);
#endif
}

static void Subsample1to8_4x4(Xil_unsigned8* src_scanline,
                              Xil_unsigned8* dst_scanline,
                              unsigned int   d_xsize,
                              unsigned int   d_ysize,
                              unsigned int   src_nx_scan,
                              unsigned int   dst_nx_scan)
{
#if DEBUG_SUBSAMPLE
    fprintf(stderr, "In Subsample1to8_4x4().\n");
#endif
    unsigned int i;
    unsigned char  bitson[16] = { 0, 1, 1, 2, 1, 2, 2, 3,
                                  1, 2, 2, 3, 2, 3, 3, 4 };

    unsigned int   lut[256];
    for(i = 0; i < 256; i++)	// build lookup table
	lut[i] = (bitson[(i>>4) & 0xf] << 8) + bitson[i & 0xf];

    unsigned int dst_chunks = d_xsize / 4;
    unsigned int dst_extra  = d_xsize % 4;

    for(i = 0; i < d_ysize; i++) {	// scale to grey via lookup
	Xil_unsigned8* src_pixel0 = src_scanline;
	Xil_unsigned8* src_pixel1 = src_pixel0 + src_nx_scan;
	Xil_unsigned8* src_pixel2 = src_pixel1 + src_nx_scan;
	Xil_unsigned8* src_pixel3 = src_pixel2 + src_nx_scan;
	unsigned int*  dst_pixel = (unsigned int *) dst_scanline;

	for(unsigned int j = 0; j < dst_chunks; j++) {
#ifndef XIL_LITTLE_ENDIAN
	    *dst_pixel++ = (lut[src_pixel0[0]] << 16) + lut[src_pixel0[1]] +
		  	   (lut[src_pixel1[0]] << 16) + lut[src_pixel1[1]] +
			   (lut[src_pixel2[0]] << 16) + lut[src_pixel2[1]] +
			   (lut[src_pixel3[0]] << 16) + lut[src_pixel3[1]];
#else
	    *dst_pixel++ = swap_bytes(
                               (lut[src_pixel0[0]] << 16) + lut[src_pixel0[1]] +
		  	       (lut[src_pixel1[0]] << 16) + lut[src_pixel1[1]] +
			       (lut[src_pixel2[0]] << 16) + lut[src_pixel2[1]] +
			       (lut[src_pixel3[0]] << 16) + lut[src_pixel3[1]]);
#endif
	    src_pixel0 += 2;
	    src_pixel1 += 2;
	    src_pixel2 += 2;
	    src_pixel3 += 2;
        }

        if(dst_extra != 0) {
#if DEBUG_SUBSAMPLE
    fprintf(stderr, "dst_extra = %d\n", dst_extra);
#endif
            unsigned int dbuf;
            Xil_unsigned8* dst_byte = (Xil_unsigned8*)dst_pixel;
            switch(dst_extra) {
                case 1:
                    dbuf = (lut[0xf0 & src_pixel0[0]] << 16) +
                           (lut[0xf0 & src_pixel1[0]] << 16) +
                           (lut[0xf0 & src_pixel2[0]] << 16) +
                           (lut[0xf0 & src_pixel3[0]] << 16);
                    *dst_byte = (Xil_unsigned8)((dbuf & 0xff000000) >> 24);
                    break;
                case 2:
                    dbuf = (lut[src_pixel0[0]] << 16) +
                           (lut[src_pixel1[0]] << 16) +
                           (lut[src_pixel2[0]] << 16) +
                           (lut[src_pixel3[0]] << 16);
                    *dst_byte++ = (Xil_unsigned8)((dbuf & 0xff000000) >> 24);
                    *dst_byte   = (Xil_unsigned8)((dbuf & 0x00ff0000) >> 16);
                    break;
                case 3:
	            dbuf = (lut[src_pixel0[0]] << 16) + lut[0xf0 & src_pixel0[1]] +
			   (lut[src_pixel1[0]] << 16) + lut[0xf0 & src_pixel1[1]] +
			   (lut[src_pixel2[0]] << 16) + lut[0xf0 & src_pixel2[1]] +
			   (lut[src_pixel3[0]] << 16) + lut[0xf0 & src_pixel3[1]];
                    *dst_byte++ = (Xil_unsigned8)((dbuf & 0xff000000) >> 24);
                    *dst_byte++ = (Xil_unsigned8)((dbuf & 0x00ff0000) >> 16);
                    *dst_byte   = (Xil_unsigned8)((dbuf & 0x0000ff00) >>  8);
                    break;
            }
        }

	src_scanline += src_nx_scan << 2;
	dst_scanline += dst_nx_scan;
    }
}

#if DEBUG_SUBSAMPLE
//
// Print a block of bits in 3x3 blocks.
//
void print_bits(unsigned char* b, unsigned int line_stride, int w, int h)
{
    int i, j;
    unsigned char* line = b;
    unsigned char* pixel;

    for(i = 0; i < h; i++) {
        fprintf(stderr, "line %d: ", i);
        pixel = line;
        for(j = 0; j < w; j++) {
            switch(j % 8) {
                case 0:
                    fprintf(stderr, "%d", (*pixel) & 0x80 ? 1 : 0);
                    break;
                case 1:
                    fprintf(stderr, "%d", (*pixel) & 0x40 ? 1 : 0);
                    break;
                case 2:
                    fprintf(stderr, "%d", (*pixel) & 0x20 ? 1 : 0);
                    break;
                case 3:
                    fprintf(stderr, "%d", (*pixel) & 0x10 ? 1 : 0);
                    break;
                case 4:
                    fprintf(stderr, "%d", (*pixel) & 0x08 ? 1 : 0);
                    break;
                case 5:
                    fprintf(stderr, "%d", (*pixel) & 0x04 ? 1 : 0);
                    break;
                case 6:
                    fprintf(stderr, "%d", (*pixel) & 0x02 ? 1 : 0);
                    break;
                case 7:
                    fprintf(stderr, "%d", (*pixel) & 0x01 ? 1 : 0);
                    pixel++;
                    break;
            }
            if((j + 1) % 3 == 0) fprintf(stderr, " ");
        }
        fprintf(stderr, "\n");
        if((i + 1) % 3 == 0) fprintf(stderr, "\n");
        line += line_stride;
    }
}

//
// Print a block of bytes
//
void print_bytes(unsigned char* b, unsigned int line_stride, int w, int h)
{
    int i, j;
    unsigned char* line = b;
    unsigned char* pixel;

    for(i = 0; i < h; i++) {
        fprintf(stderr, "line %d: ", i);
        pixel = line;
        for(j = 0; j < w; j++) {
            fprintf(stderr, "%d ", (unsigned int)(*pixel));
            pixel++;
        }
        fprintf(stderr, "\n");
        line += line_stride;
    }
}
#endif
