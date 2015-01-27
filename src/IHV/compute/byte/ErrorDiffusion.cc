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
//  File:	ErrorDiffusion.cc
//  Project:	XIL
//  Revision:	1.17
//  Last Mod:	10:10:49, 03/10/00
//
//  Description:
//	Error Diffusion
//	
//	
//	
//	
//  MT-level:  SAFE
//	
//------------------------------------------------------------------------
//	COPYRIGHT
//------------------------------------------------------------------------
#pragma ident	"@(#)ErrorDiffusion.cc	1.17\t00/03/10  "

#include "XiliUtils.hh"
#include "XilDeviceManagerComputeBYTE.hh"
#include "XiliDiffusionUtils.hh"


//
// Forward function declarations
//

static XilStatus
default_8to1_error_diffusion(XilOp*           op,
			     XilRoi*          roi,
			     XilBoxList*      bl,
			     XilImage*        src,
			     XilImage*        dst,
			     XilLookupSingle* cmap,
			     XilKernel*       dist,
			     XilSystemState*  err_state);

static XilStatus
xili_floyd_steinburg_1band_8to1_error_diffusion(XilOp*           op,
                                                XilRoi*          roi,
                                                XilBoxList*      bl,
                                                XilImage*        src,
                                                XilImage*        dst,
                                                XilLookupSingle* cmap,
                                                XilSystemState*  err_state);

static XilStatus
default_8to8_error_diffusion(XilOp* op,
			     XilRoi* roi,
			     XilBoxList* bl,
			     XilImage* src,
			     XilImage* dst,
			     XilLookupSingle* cmap,
			     XilKernel* dist,
			     XilSystemState* err_state);

XilStatus
xili_floyd_steinburg_3band_8to8_error_diffusion(XilOp*           op,
                                                XilRoi*          roi,
                                                XilBoxList*      bl,
                                                XilImage*        src,
                                                XilImage*        dst,
                                                XilLookupColorcube* cmap,
                                                XilSystemState*  err_state,
                                                int*             dither_table);

//
//  Perform an byte to 1-bit ErrorDiffusion
//
XilStatus
XilDeviceManagerComputeBYTE::ErrorDiffusion1(XilOp*       op,
					     unsigned,
					     XilRoi*      roi,
					     XilBoxList*  bl)
{
    //
    // Get the images for our operation.
    //
    XilImage* src = op->getSrcImage(1);
    XilImage* dst = op->getDstImage(1);

    //
    // Get params from the op
    //
    XilLookupSingle* cmap;
    op->getParam(1, (XilObject**) &cmap);
    XilKernel* dist;
    op->getParam(2, (XilObject**) &dist);

    //
    // Get XilSystemState used to report errors
    //
    XilSystemState* err_state = dst->getSystemState();

    //
    // Figure which case we are in and choose appropriate code
    //
    XilStatus status;

    if(src->getNumBands() == 1 && xili_is_floyd_steinberg_kernel(dist)) {
        status =
            xili_floyd_steinburg_1band_8to1_error_diffusion(op, roi, bl,
                                                            src, dst, cmap,
                                                            err_state);
        //
        //  Go ahead and try calling the default version if our special-case
        //  failed just in case its possible to continue.
        //
        if(status == XIL_FAILURE) {
            status =
                default_8to1_error_diffusion(op, roi, bl, src, dst, cmap, dist,
                                             err_state);
        }
    } else {
        status =
            default_8to1_error_diffusion(op, roi, bl, src, dst, cmap, dist,
                                         err_state);
    }

    return status;
}

static XilStatus
default_8to1_error_diffusion(XilOp* op,
			     XilRoi* roi,
			     XilBoxList* bl,
			     XilImage* src,
			     XilImage* dst,
			     XilLookupSingle* cmap,
			     XilKernel* dist,
			     XilSystemState* err_state)
{
    // Get info about src image
    unsigned int src_width;
    unsigned int src_height;
    src->getSize(&src_width, &src_height);
    unsigned int src_nbands = src->getNumBands();

    // Create a 1 banded tmp image of same datatype as dst.  Dimensions
    // will be same as src.  This is where the error diffusion results
    // will go before copying into the real dst.
    unsigned int tmp_next_scan = (src_width + 7) / 8;
    Xil_unsigned8* tmp_base_addr =
	new Xil_unsigned8[tmp_next_scan * src_height];
    if (tmp_base_addr == NULL) {
	XIL_ERROR(err_state, XIL_ERROR_RESOURCE, "di-1", TRUE);
	return XIL_FAILURE;
    }

    //
    // Get diffusion kernel info that will be needed later
    //
    unsigned int dist_w = dist->getWidth();
    unsigned int dist_h = dist->getHeight();
    unsigned int dist_key_x = dist->getKeyX();
    unsigned int dist_key_y = dist->getKeyY();
    // Get pointer to beginning of kernel data
    const float *dist_value = dist->getData();
    // Get pointer to key value of kernel
    const float *key_idx = dist_value + (dist_key_y * dist_w) + dist_key_x;
    // Get number of cells to the right of key (r == right)
    unsigned int kern_r = dist_w - dist_key_x - 1;
    // Get number of cells down from key (d == down)
    unsigned int kern_d = dist_h - dist_key_y - 1;

    // Allocate array to hold error
    float* error = new float[src_nbands];
    if (error == NULL) {
	delete [] tmp_base_addr;
	XIL_ERROR(err_state, XIL_ERROR_RESOURCE, "di-1", TRUE);
	return XIL_FAILURE;
    }

    //
    //  Loop over each of the boxes of storage we are to process.
    //
    XilBox* src_box;
    XilBox* dst_box;
    while(bl->getNext(&src_box, &dst_box)) {
        //
        //  Aquire our storage from the images.  The storage returned is
        //  valid for the box given.  Thus, any origins or child offsets
        //  have been taken into account.
        //
        XilStorage src_storage(src);
        XilStorage dst_storage(dst);
        if((src->getStorage(&src_storage, op, src_box, "XilMemory",
			    XIL_READ_ONLY)  == XIL_FAILURE) ||
           (dst->getStorage(&dst_storage, op, dst_box, "XilMemory",
			    XIL_WRITE_ONLY) == XIL_FAILURE)) {
            //
            //  Mark this box entry as having failed.  If marking the box
            //  returns XIL_FAILURE, then we return XIL_FAILURE.
            //
            if(bl->markAsFailed() == XIL_FAILURE) {
		delete [] error;
		delete [] tmp_base_addr;
                return XIL_FAILURE;
            } else {
                continue;
            }
        }

	//
	// Get the image space coordinates of the src box
	//
	int          box_x;
	int          box_y;
	unsigned int box_w;
	unsigned int box_h;
	src_box->getAsRect(&box_x, &box_y, &box_w, &box_h);

	// "end_scanline" is the max height of the src image that we
	// need to process.  This will always be <= src_height.
	unsigned int end_scanline = box_y + box_h;

	//
	// Allocate rolling buffers for converting src scanlines to
	// floats in order to distribute error.
	//
	int line_size = src_nbands * src_width;
	// "buf_lines" is the number of rolling buffer lines needed
	int buf_lines = _XILI_MIN(end_scanline, dist_h - dist_key_y);
	float* buf_mem = new float[buf_lines * line_size];
	if (buf_mem == NULL) {
	    delete [] error;
	    delete [] tmp_base_addr;
	    XIL_ERROR(err_state, XIL_ERROR_RESOURCE, "di-1", TRUE);
	    return XIL_FAILURE;
	}
	float** buf = new float*[buf_lines];
	if (buf == NULL) {
	    delete [] buf_mem;
	    delete [] error;
	    delete [] tmp_base_addr;
	    XIL_ERROR(err_state, XIL_ERROR_RESOURCE, "di-1", TRUE);
	    return XIL_FAILURE;
	}

	// "last_buf_line" is the index of last rolling buffer line
	int last_buf_line = buf_lines - 1;
	unsigned int src_kern_end = end_scanline - buf_lines;

	//
        //  Test to see if our src storage is of type
        //  XIL_PIXEL_SEQUENTIAL.  If so, implement an loop optimized
        //  for pixel-sequential storage.
        //
        if(src_storage.isType(XIL_PIXEL_SEQUENTIAL)) {
            unsigned int src_pixel_stride;
            unsigned int src_scanline_stride;
            Xil_unsigned8* src_data;
            src_storage.getStorageInfo(&src_pixel_stride,
				       &src_scanline_stride,
				       NULL, NULL,
				       (void**)&src_data);

	    // Prepare to perform error diffusion into the tmp
	    // image.  Set pointers to (0, 0) in image space.
	    Xil_unsigned8* src_scanline = src_data -
		box_y * src_scanline_stride -
		box_x * src_pixel_stride;
	    Xil_unsigned8* tmp_scanline = tmp_base_addr;

	    //
	    // First pass thru, so need to fill rolling buffers with
	    // first set of scanline data.  While filling buffer,
	    // convert data to float values.
	    //
	    Xil_unsigned8* src_p;
	    float* buf_p;
	    for (int i = 0; i < buf_lines; i++) {
		src_p = src_scanline;
		buf[i] = buf_mem + (i * line_size);
		buf_p = buf[i];
		for (unsigned int j = 0; j < src_width; j++) {
		    for (unsigned int k = 0; k < src_nbands; k++) {
			buf_p[k] = _XILI_B2F(src_p[k]);
		    }
		    src_p += src_pixel_stride;
		    buf_p += src_nbands;
		}
		src_scanline += src_scanline_stride;
	    }

	    int nearest_color_idx;
	    Xil_unsigned8* nearest_color;
	    float* curr_pix;

	    //
	    // Do error diffusion on src image for each scanline and
	    // place results in tmp image.
	    //
	    for (unsigned int y = 0; y < end_scanline; y++) {
		curr_pix = buf[0];
 
		// For each pixel
		for (unsigned int x = 0; x < src_width; x++) {

		    // Do nearest_color step of operation
		    nearest_color = (Xil_unsigned8 *)
			xili_find_nearest_color(curr_pix, cmap,
						&nearest_color_idx);

		    // Set tmp pixel to the displayable nearest color
		    if (nearest_color_idx) {
			XIL_BMAP_SET(tmp_scanline, x);
		    } else {
			XIL_BMAP_CLR(tmp_scanline, x);
		    }

		    // Calculate amount of error between nearest and
		    // actual color
		    int non_zero_error = 0;
		    for (unsigned int b = 0; b < src_nbands; b++) {
			// get error in intensity
			if (error[b] = curr_pix[b] -
			    _XILI_B2F(nearest_color[b])) {
			    non_zero_error = 1;
			}
		    }

		    // if (non_zero_error) then need to distribute error, else
		    // skip this step.
		    if (non_zero_error) {
			// First line of kernel only distributes error
			// to right of key.  "rght_cnt" = number of
			// cells to right of key.
			int rght_cnt = _XILI_MIN(kern_r, src_width - 1 - x);
			const float* kern_entry = key_idx;
			buf_p = curr_pix;
			// For each cell to right of key (if any)...
			for (int k = 1; k <= rght_cnt; k++) {
			    for (b = 0; b < src_nbands; b++) {
				*(buf_p + (k * src_nbands) + b) +=
				    error[b] * kern_entry[k];
			    }
			}

			// Distribute error below key
			int down_cnt = _XILI_MIN(kern_d, end_scanline - 1 - y);
			int left_cnt = _XILI_MIN(dist_key_x, x) * -1;
			kern_entry = key_idx;
			for (int l = 0; l < down_cnt; l++) {
			    kern_entry += dist_w;
			    buf_p = buf[l + 1];
			    buf_p += (x * src_nbands);
			    for (k = left_cnt; k <= rght_cnt; k++) {
				for (b = 0; b < src_nbands; b++) {
				    *(buf_p + (k * src_nbands) + b) +=
					error[b] * kern_entry[k];
				}
			    }
			}
		    } // if (non_zero_error)

		    // Advance to next pixel on current scanline
		    curr_pix  += src_nbands;
		} // For each pixel
 
		// Rotate lines in buffer everytime so that buf[0] always
		// points to line being processed.
		buf_p = buf[0];
		for (int k = 0; k < last_buf_line; k++)
		    buf[k] = buf[k+1];
 
		// Only need to add new line if 'src_line_num+buf_lines' <
		// 'src_height', because buffer will already contain last few
		// lines of src image.
		if (y < src_kern_end) {
		    buf[last_buf_line] = buf_p;
		    src_p = src_scanline;
		    for (x = 0; x < src_width; x++) {
			for (unsigned int b = 0; b < src_nbands; b++) {
			    buf_p[b] = _XILI_B2F(src_p[b]);
			}
			src_p += src_pixel_stride;
			buf_p += src_nbands;
		    }
		    src_scanline += src_scanline_stride;
		} else {
		    buf[last_buf_line] = NULL;
		}
		tmp_scanline += tmp_next_scan;
	    } // end for each scanline

	    //
	    // Copy data from tmp to dst taking the ROI into account
	    //
            unsigned int dst_scanline_stride;
            unsigned int dst_offset;
            Xil_unsigned8* dst_data;
            dst_storage.getStorageInfo((unsigned int*)NULL,
				       &dst_scanline_stride,
				       NULL,
				       &dst_offset,
				       (void**)&dst_data);
	    xili_diffusion_copy_1(tmp_base_addr, tmp_next_scan, dst_data,
				  dst_scanline_stride, dst_offset,
				  roi, dst_box, box_x, box_y);
	} else {
            //
            // General Storage Implementation.
            //

	    // Prepare to perform error diffusion into the tmp
	    // image.  Set pointers to (0, 0) in image space.
	    Xil_unsigned8* tmp_scanline = tmp_base_addr;

	    //
	    // First pass thru, so need to fill rolling buffers with
	    // first set of scanline data.  While filling buffer,
	    // convert data to float values.  (src_x, src_y) is in src
	    // image space.  "src_y" keeps track of where in src image
	    // data will next be read to fill rolling buffer.
	    //
	    int src_y = 0;
	    float* buf_p;
	    for (int i = 0; i < buf_lines; i++) {
		buf[i] = buf_mem + (i * line_size);
		buf_p = buf[i];
		for (unsigned int src_x = 0; src_x < src_width; src_x++) {
		    for (unsigned int b = 0; b < src_nbands; b++) {
			unsigned int src_pixel_stride;
			unsigned int src_scanline_stride;
			Xil_unsigned8* src_data;
			src_storage.getStorageInfo(b,
						   &src_pixel_stride,
						   &src_scanline_stride,
						   NULL,
						   (void**)&src_data);
			// Translate from image space to box space
			Xil_unsigned8* src_band = src_data +
			    (src_y - box_y) * src_scanline_stride +
			    (src_x - box_x) * src_pixel_stride;

			buf_p[b] = _XILI_B2F(*src_band);
		    }
		    buf_p += src_nbands;
		}
		// Next scanline in src
		src_y++;
	    }

	    int nearest_color_idx;
	    Xil_unsigned8* nearest_color;
	    float* curr_pix;

	    //
	    // Do error diffusion on src image for each scanline and
	    // place results in tmp image.
	    //
	    for (unsigned int y = 0; y < end_scanline; y++) {
		curr_pix = buf[0];

		// For each pixel
		for (unsigned int x = 0; x < src_width; x++) {

		    // Do nearest_color step of operation
		    nearest_color = (Xil_unsigned8 *)
			xili_find_nearest_color(curr_pix, cmap,
						&nearest_color_idx);

		    // Set tmp pixel to the displayable nearest color
		    if (nearest_color_idx) {
			XIL_BMAP_SET(tmp_scanline, x);
		    } else {
			XIL_BMAP_CLR(tmp_scanline, x);
		    }

		    // Calculate amount of error between nearest and
		    // actual color
		    int non_zero_error = 0;
		    for (unsigned int b = 0; b < src_nbands; b++) {
			// get error in intensity
			if (error[b] = curr_pix[b] -
			    _XILI_B2F(nearest_color[b])) {
			    non_zero_error = 1;
			}
		    }

		    // if (non_zero_error) then need to distribute error, else
		    // skip this step.
		    if (non_zero_error) {
			// First line of kernel only distributes error
			// to right of key.  "rght_cnt" = number of
			// cells to right of key.
			int rght_cnt = _XILI_MIN(kern_r, src_width - 1 - x);
			const float* kern_entry = key_idx;
			buf_p = curr_pix;
			// For each cell to right of key (if any)...
			for (int k = 1; k <= rght_cnt; k++) {
			    for (b = 0; b < src_nbands; b++) {
				*(buf_p + (k * src_nbands) + b) +=
				    error[b] * kern_entry[k];
			    }
			}

			// Distribute error below key
			int down_cnt = _XILI_MIN(kern_d, end_scanline - 1 - y);
			int left_cnt = _XILI_MIN(dist_key_x, x) * -1;
			kern_entry = key_idx;
			for (int l = 0; l < down_cnt; l++) {
			    kern_entry += dist_w;
			    buf_p = buf[l + 1];
			    buf_p += (x * src_nbands);
			    for (k = left_cnt; k <= rght_cnt; k++) {
				for (b = 0; b < src_nbands; b++) {
				    *(buf_p + (k * src_nbands) + b) +=
					error[b] * kern_entry[k];
				}
			    }
			}
		    } // if (non_zero_error)

		    // Advance to next pixel on current scanline
		    curr_pix  += src_nbands;
		} // For each pixel
 
		// Rotate lines in buffer everytime so that buf[0] always
		// points to line being processed.
		buf_p = buf[0];
		for (int k = 0; k < last_buf_line; k++)
		    buf[k] = buf[k+1];

		// Only need to add new line if 'src_line_num+buf_lines' <
		// 'src_height', because buffer will already contain last few
		// lines of src image.
		if (y < src_kern_end) {
		    buf[last_buf_line] = buf_p;
		    for (unsigned int src_x = 0; src_x < src_width; src_x++) {
			for (unsigned int b = 0; b < src_nbands; b++) {
			    unsigned int src_pixel_stride;
			    unsigned int src_scanline_stride;
			    Xil_unsigned8* src_data;
			    src_storage.getStorageInfo(b,
						       &src_pixel_stride,
						       &src_scanline_stride,
						       NULL,
						       (void**)&src_data);
			    // Translate from image space to box space
			    Xil_unsigned8* src_band = src_data +
				(src_y - box_y) * src_scanline_stride +
				(src_x - box_x) * src_pixel_stride;

			    buf_p[b] = _XILI_B2F(*src_band);
			}
			buf_p += src_nbands;
		    }
		    // Next scanline in src
		    src_y++;
		} else {
		    buf[last_buf_line] = NULL;
		}
		tmp_scanline += tmp_next_scan;
	    } // end for each scanline

	    //
	    // Copy data from tmp to dst taking the ROI into
	    // account.  dst will always be 1-banded so we can just get
	    // the storage info once and treat it as a pixel-sequential
	    // image.
	    //
            unsigned int dst_scanline_stride;
            unsigned int dst_offset;
            Xil_unsigned8* dst_data;
            dst_storage.getStorageInfo((unsigned int)0,
				       NULL,
				       &dst_scanline_stride,
				       &dst_offset,
				       (void**)&dst_data);
	    xili_diffusion_copy_1(tmp_base_addr, tmp_next_scan, dst_data,
				  dst_scanline_stride, dst_offset,
				  roi, dst_box, box_x, box_y);
	} // if pixel-sequential

	// Free memory used by rolling buffers
	delete [] buf;
	delete [] buf_mem;
    } // while

    // Free error array and tmp image
    delete [] error;
    delete [] tmp_base_addr;

    return XIL_SUCCESS;
}

const unsigned int FS_SHIFT = 16;

#define FS_8TO1_1BAND_ERR_DIST(err)          \
{                                            \
    int err1 = err >> 4;                     \
    int err4 = err1 * 4;                     \
    int err3 = err4 - err1;                  \
    src_val = curr_pix[1] + (err4 + err3);   \
    curr_pix[-1] += err3;                    \
    curr_pix[0] += (err4 + err1);            \
    pSrc += src_pixel_stride;                \
    curr_pix++;                              \
    curr_pix[0] = err1 + (((int)(*pSrc)) << FS_SHIFT);   \
}

//////////////////////////////////////////////////////////////////////////
//
//
// Optimized case for single-band 8 to 1 bit error diffusion,
// specific to the Floyd-Steinberg kernel
//
//                     | |X|A|
//                     -------
//                     |B|C|D|
//
// The value of pixel X is tested against the midpoint of the two 
// cmap values. (Usually the value will be 127.5).
// If less, the dst becomes lo_idx, else hi_idx(0 or 1). The error 
// (actual value minus the either lo_val or hi_val) is distributed to 
// pixels A, B, C, D.
//
// All distribution coefficients are in 16ths, so we use 
// scaled integer arithmetic. All values are left shifted by FS_SHIFT
// (currently 16) to preserve precision. The multiplies by x/16 are 
// accomplished by right shifting the error term by 4 bits to get
// the 1/16 portion.
// 
// Coefficients are:
//      A: 7/16
//      B: 3/16
//      C: 5/16
//      D: 1/16
//
// A single row error accumulation buffer is used.
// As an error is calculated, it is distributed to the right and below.
// The next source value on the current line becomes the error adjusted value
// of Pixel A (see disgarm above). Pixel positions B and C already will 
// already have values from the line below (from previous error distributions
// or from the line initialization). The only NEW pixel needed is that at 
// position D, which is gotten from the src line below. We only have to
// be careful that the value of error-adjusted pixel A is calculated
// BEFORE we get pixel D. With that precaution, the single line buffer works.
//
// ---------------------------------------------------------------------------
// |  Newly adjusted values (from below) | Sample Point | Old adjusted values |
// ---------------------------------------------------------------------------
//
//////////////////////////////////////////////////////////////////////////

XilStatus
xili_floyd_steinburg_1band_8to1_error_diffusion(XilOp*           op,
                                                XilRoi*          roi,
                                                XilBoxList*      bl,
                                                XilImage*        src,
                                                XilImage*        dst,
                                                XilLookupSingle* cmap,
                                                XilSystemState*  err_state)
{
    //
    //  Get info about src image
    //
    unsigned int src_width;
    unsigned int src_height;
    src->getSize(&src_width, &src_height);

    //
    //  Create a 1 banded tmp image.  Dimensions will be same as src.  This is
    //  where the error diffusion results will go before copying into the real
    //  dst.
    //
    //  We make tmp_next_scan so that it's 8-byte aligned.
    //
    unsigned int tmp_next_scan = (src_width + 7) / 8;

    tmp_next_scan += 8 - (tmp_next_scan & 0x7);

    Xil_unsigned8* tmp_base_addr =
        new Xil_unsigned8[tmp_next_scan * src_height];
    if(tmp_base_addr == NULL) {
        XIL_ERROR(err_state, XIL_ERROR_RESOURCE, "di-1", TRUE);
        return XIL_FAILURE;
    }

    //
    //  Loop over each of the boxes of storage we are to process.
    //
    XilBox* src_box;
    XilBox* dst_box;
    while(bl->getNext(&src_box, &dst_box)) {
        //
        //  Aquire our storage from the images.  The storage returned is
        //  valid for the box given.  Thus, any origins or child offsets
        //  have been taken into account.
        //
        XilStorage src_storage(src);
        XilStorage dst_storage(dst);
        if((src->getStorage(&src_storage, op, src_box, "XilMemory",
                            XIL_READ_ONLY)  == XIL_FAILURE) ||
           (dst->getStorage(&dst_storage, op, dst_box, "XilMemory",
                            XIL_WRITE_ONLY) == XIL_FAILURE)) {
            //
            //  Mark this box entry as having failed.  If marking the box
            //  returns XIL_FAILURE, then we return XIL_FAILURE.
            //
            if(bl->markAsFailed() == XIL_FAILURE) {
                delete [] tmp_base_addr;
                return XIL_FAILURE;
            } else {
                continue;
            }
        }

        //
        // Get the image space coordinates of the src box
        //
        int          box_x;
        int          box_y;
        unsigned int box_w;
        unsigned int box_h;
        src_box->getAsRect(&box_x, &box_y, &box_w, &box_h);

        //
        //  "end_scanline" is the max height of the src image that we
        //  need to process.  This will always be <= src_height.
        //
        int end_scanline = box_y + box_h;

        //
        // We only need to have a single line for the buffer, since
        // the pixels to which we distribute error can go in space
        // BEHIND where we're pointing to.
        //
        int    src_padded_width = src_width + 2;
        int* buf_mem          = new int[src_padded_width];
        if (buf_mem == NULL) {
            delete [] tmp_base_addr;
            XIL_ERROR(err_state, XIL_ERROR_RESOURCE, "di-1", TRUE);
            return XIL_FAILURE;
        }

        //
        // Point to the first non-pad pixel
        //
        int* buf = buf_mem + 1;

        //
        //  Get colormap information
        //
        Xil_unsigned8* cmap_data = (Xil_unsigned8*)cmap->getData();

        //
        //  Since only 2 possible entries in colormap and dealing with 
        //  1-band in colormap, create sorted list and find mid point 
        //  so all that is needed is quick comparison. Build lookup since there
        //  are only 256 possible input values.
        //
        int          mid_val = (cmap_data[0] + cmap_data[1]) << (FS_SHIFT-1);
        unsigned int lo_idx;
        unsigned int hi_idx;
        if(cmap_data[0] > cmap_data[1]) {
            //
            //  Colormap is a reverse ramp.
            //
            lo_idx = 1;
            hi_idx = 0;
        } else {
            lo_idx = 0;
            hi_idx = 1;
        }

        int lo_val = ((cmap_data[lo_idx]) << FS_SHIFT);
        int hi_val = ((cmap_data[hi_idx]) << FS_SHIFT);

        unsigned int src_pixel_stride;
        unsigned int src_scanline_stride;
        Xil_unsigned8* src_data;
        src_storage.getStorageInfo(&src_pixel_stride,
                                   &src_scanline_stride,
                                   NULL, NULL,
                                   (void**)&src_data);

        //
        // Prepare to perform error diffusion into the tmp
        // image.  Set pointers to (0, 0) in image space.
        //
        Xil_unsigned8* src_scanline = src_data -
            box_y * src_scanline_stride -
            box_x * src_pixel_stride;
        Xil_unsigned8* tmp_scanline = tmp_base_addr;

        //
        // On the first line, fill the error accumulation buffer
        // with the first line of source data. Convert to floats.
        //
        Xil_unsigned8* pSrc = src_scanline;
        int*         pBuf = buf;
        for (int j=src_width; j!=0; j--) {
            *pBuf++ = ((*pSrc) << FS_SHIFT);
            pSrc += src_pixel_stride;
        }

        //
        // Do error diffusion on src image for each scanline and
        // place results in tmp image.
        //
        int* curr_pix;
        int src_val;
        unsigned int  nlongs     = src_width >> 5;
        unsigned int  xstart     = nlongs * 32;
        unsigned int  nbits_R    = src_width - (nlongs << 5);
        for(int y=0; y<end_scanline; y++) {
            //
            // Point to first pixel and get its value
            //
            curr_pix  = buf;
            src_val = *curr_pix;

            //
            // Set a ptr to the scanline below.
            // This is where we'll be getting all of
            // our NEW pixel data, i.e that from Pixel D
            //
            src_scanline += src_scanline_stride;
            pSrc      = src_scanline;

            //
            // On the last line, we don't want to actually
            // read from the source data (could be outside image).
            // So we'll just set the pSrc ptr to read "new" data
            // from the last image line. This is harmless,
            // since the errors don't need to be propagated anyway,
            // and it keeps us from needing special tests in the inner loops.
            //
            if(y == end_scanline-1) {
                pSrc = src_scanline - src_scanline_stride;
            }

            //
            // Load the values of Pixels B and C from the src line below 
            // into the error accumulation buffer. B is off the edge,
            // so we just load a zero there.
            //
            buf[-1] = 0;
            buf[0] = ((*pSrc) << FS_SHIFT);

            unsigned int* pTmp = (unsigned int*)tmp_scanline;

            //
            //  Do all of the full 32 bit words in the loop below.
            //  Each execution of the FS_8TO1_1BAND_ERR_DIST macro
            //  distributes the error and updates the value of src_val.
            //
            int err;
            for(int count=nlongs; count !=0; count--) {
                unsigned int wrd1 = 0;
                for(int bitcount=32; bitcount !=0; bitcount--) {
                    if(src_val >= mid_val) {
                        err = src_val - hi_val;
                        wrd1 = (wrd1 << 1) | hi_idx;
                    } else {
                        err = src_val - lo_val;
                        wrd1 = (wrd1 << 1) | lo_idx;
                    }
                    FS_8TO1_1BAND_ERR_DIST(err);
                }

#ifdef XIL_LITTLE_ENDIAN
                //
                // Byte swap the 32 bit word if Intel or other 
                // little-endian architectures.
                //
                _XILI_BSWAP(wrd1);
#endif
                *pTmp++ = wrd1;
            }

            //
            //  Handle any remaining bits on the right which don't fill a word.
            //  This will also handle image widths less than 32.
            //
            unsigned int x = xstart;
            unsigned int idx;
            for(count=nbits_R; count !=0; count--) {
                if(src_val >= mid_val) {
                    err = src_val - hi_val;
                    idx = hi_idx;
                } else {
                    err = src_val - lo_val;
                    idx = lo_idx;
                }
                if(idx) {
                    XIL_BMAP_SET(tmp_scanline, x);
                } else {
                    XIL_BMAP_CLR(tmp_scanline, x);
                }

                FS_8TO1_1BAND_ERR_DIST(err);

                x++;
            }


            tmp_scanline += tmp_next_scan;
        }

        //
        //  Copy data from tmp to dst taking the ROI into account
        //
        unsigned int dst_scanline_stride;
        unsigned int dst_offset;
        Xil_unsigned8* dst_data;
        dst_storage.getStorageInfo((unsigned int*)NULL,
                                   &dst_scanline_stride,
                                   NULL,
                                   &dst_offset,
                                   (void**)&dst_data);
        xili_diffusion_copy_1(tmp_base_addr, tmp_next_scan, dst_data,
                              dst_scanline_stride, dst_offset,
                              roi, dst_box, box_x, box_y);

        //
        //  Free memory used by the error buffer
        //
        delete [] buf_mem;
    }

    //
    //  Free error array and tmp image
    //
    delete [] tmp_base_addr;

    return XIL_SUCCESS;
}

XilStatus
XilDeviceManagerComputeBYTE::ErrorDiffusion8(XilOp*       op,
					     unsigned,
					     XilRoi*      roi,
					     XilBoxList*  bl)
{
    //
    //  Get the images for our operation.
    //
    XilImage* src = op->getSrcImage(1);
    XilImage* dst = op->getDstImage(1);

    //
    // Get params from the op
    //
    XilLookupSingle* cmap;
    op->getParam(1, (XilObject**) &cmap);
    XilKernel* dist;
    op->getParam(2, (XilObject**) &dist);

    //
    // Get XilSystemState used to report errors
    //
    XilSystemState* err_state = dst->getSystemState();

    //
    //  Get the data set by our preprocess routine.
    //
    unsigned int data = (unsigned int)op->getPreprocessData(this);

    if(data != -1) {
        //
        //  Optimized 3-band case.
        //
        int* dither_table;
        if((int)data >= 0 && data < _XILI_NUM_ERROR_DIFFUSION_LUTS) {
            dither_table = edcacheTable[data];
        } else {
            dither_table = (int*)data;
        }

        XilStatus status =
            xili_floyd_steinburg_3band_8to8_error_diffusion(op,
                                                            roi,
                                                            bl,
                                                            src,
                                                            dst,
                                                            (XilLookupColorcube*)cmap,
                                                            err_state,
                                                            dither_table);

        //
        //  Go ahead and try calling the default version if our special-case
        //  failed just in case its possible to continue.
        //
        if(status == XIL_FAILURE) {
            return default_8to8_error_diffusion(op, roi, bl, src, dst, cmap, dist,
                                                err_state);
        }

        return XIL_SUCCESS;
    } else {
        return default_8to8_error_diffusion(op, roi, bl, src, dst, cmap, dist,
                                            err_state);
    }
}

static XilStatus
default_8to8_error_diffusion(XilOp* op,
			     XilRoi* roi,
			     XilBoxList* bl,
			     XilImage* src,
			     XilImage* dst,
			     XilLookupSingle* cmap,
			     XilKernel* dist,
			     XilSystemState* err_state)
{
    // Get info about src image
    unsigned int src_width;
    unsigned int src_height;
    src->getSize(&src_width, &src_height);
    unsigned int src_nbands = src->getNumBands();

    // Create a 1 banded tmp image of same datatype as dst.  Dimensions
    // will be same as src.  This is where the error diffusion results
    // will go before copying into the real dst.
    unsigned int tmp_next_scan = src_width;
    Xil_unsigned8* tmp_base_addr =
	new Xil_unsigned8[src_width * src_height];
    if (tmp_base_addr == NULL) {
	XIL_ERROR(err_state, XIL_ERROR_RESOURCE, "di-1", TRUE);
	return XIL_FAILURE;
    }

    //
    // Get diffusion kernel info that will be needed later
    //
    unsigned int dist_w = dist->getWidth();
    unsigned int dist_h = dist->getHeight();
    unsigned int dist_key_x = dist->getKeyX();
    int dist_key_y = dist->getKeyY();
    // Get pointer to beginning of kernel data
    const float *dist_value = dist->getData();
    // Get pointer to key value of kernel
    const float *key_idx = dist_value + (dist_key_y * dist_w) + dist_key_x;
    // Get number of cells to the right of key (r == right)
    unsigned int kern_r = dist_w - dist_key_x - 1;
    // Get number of cells down from key (d == down)
    unsigned int kern_d = dist_h - dist_key_y - 1;

    // Allocate array to hold error
    float* error = new float[src_nbands];
    if (error == NULL) {
	delete [] tmp_base_addr;
	XIL_ERROR(err_state, XIL_ERROR_RESOURCE, "di-1", TRUE);
	return XIL_FAILURE;
    }

    //
    //  Loop over each of the boxes of storage we are to process.
    //
    XilBox* src_box;
    XilBox* dst_box;
    while(bl->getNext(&src_box, &dst_box)) {
        //
        //  Aquire our storage from the images.  The storage returned is
        //  valid for the box given.  Thus, any origins or child offsets
        //  have been taken into account.
        //
        XilStorage src_storage(src);
        XilStorage dst_storage(dst);
        if((src->getStorage(&src_storage, op, src_box, "XilMemory",
			    XIL_READ_ONLY)  == XIL_FAILURE) ||
           (dst->getStorage(&dst_storage, op, dst_box, "XilMemory",
			    XIL_WRITE_ONLY) == XIL_FAILURE)) {
            //
            //  Mark this box entry as having failed.  If marking the box
            //  returns XIL_FAILURE, then we return XIL_FAILURE.
            //
            if(bl->markAsFailed() == XIL_FAILURE) {
		delete [] error;
		delete [] tmp_base_addr;
                return XIL_FAILURE;
            } else {
                continue;
            }
        }

	//
	// Get the image space coordinates of the src box
	//
	int          box_x;
	int          box_y;
	unsigned int box_w;
	unsigned int box_h;
	src_box->getAsRect(&box_x, &box_y, &box_w, &box_h);

        //
	// "end_scanline" is the max height of the src image that we
	// need to process.  This will always be <= src_height.
        //
	unsigned int end_scanline = box_y + box_h;

	//
	// Allocate rolling buffers for converting src scanlines to
	// floats in order to distribute error.
	//
	int line_size = src_nbands * src_width;

        //
	// "buf_lines" is the number of rolling buffer lines needed
        //
	int buf_lines = _XILI_MIN(end_scanline, dist_h - dist_key_y);

	float* buf_mem = new float[buf_lines * line_size];
	if (buf_mem == NULL) {
	    delete [] error;
	    delete [] tmp_base_addr;
	    XIL_ERROR(err_state, XIL_ERROR_RESOURCE, "di-1", TRUE);
	    return XIL_FAILURE;
	}
	float** buf = new float*[buf_lines];
	if (buf == NULL) {
	    delete [] buf_mem;
	    delete [] error;
	    delete [] tmp_base_addr;
	    XIL_ERROR(err_state, XIL_ERROR_RESOURCE, "di-1", TRUE);
	    return XIL_FAILURE;
	}

        //
	// "last_buf_line" is the index of last rolling buffer line
        //
	int last_buf_line = buf_lines - 1;
	unsigned int src_kern_end = end_scanline - buf_lines;

	//
        //  Test to see if all of our storage is of type XIL_PIXEL_SEQUENTIAL.
        //  If so, implement an loop optimized for pixel-sequential storage.
        //
        if((src_storage.isType(XIL_PIXEL_SEQUENTIAL)) &&
	   (dst_storage.isType(XIL_PIXEL_SEQUENTIAL))) {
            unsigned int src_pixel_stride;
            unsigned int src_scanline_stride;
            Xil_unsigned8* src_data;
            src_storage.getStorageInfo(&src_pixel_stride,
				       &src_scanline_stride,
				       NULL, NULL,
				       (void**)&src_data);

            //
	    // Prepare to perform error diffusion into the tmp
	    // image.  Set pointers to (0, 0) in image space.
            //
	    Xil_unsigned8* src_scanline = src_data -
		box_y * src_scanline_stride -
		box_x * src_pixel_stride;
	    Xil_unsigned8* tmp_scanline = tmp_base_addr;

	    //
	    // First pass thru, so need to fill rolling buffers with
	    // first set of scanline data.  While filling buffer,
	    // convert data to float values.
	    //
	    Xil_unsigned8* src_p;
	    float* buf_p;
	    for (int i = 0; i < buf_lines; i++) {
		src_p = src_scanline;
		buf[i] = buf_mem + (i * line_size);
		buf_p = buf[i];
		for (unsigned int j = 0; j < src_width; j++) {
		    for (unsigned int k = 0; k < src_nbands; k++) {
			buf_p[k] = _XILI_B2F(src_p[k]);
		    }
		    src_p += src_pixel_stride;
		    buf_p += src_nbands;
		}
		src_scanline += src_scanline_stride;
	    }

	    int nearest_color_idx;
	    Xil_unsigned8* nearest_color;
	    Xil_unsigned8* tmp_pixel;
	    float* curr_pix;

	    //
	    // Do error diffusion on src image for each scanline and
	    // place results in tmp image.
	    //
	    for (unsigned int y = 0; y < end_scanline; y++) {
		curr_pix = buf[0];
		tmp_pixel = tmp_scanline;

                //
		// For each pixel
                //
		for (unsigned int x = 0; x < src_width; x++) {
                    //
		    // Do nearest_color step of operation
                    //
		    nearest_color = (Xil_unsigned8*)
			xili_find_nearest_color(curr_pix, cmap,
						&nearest_color_idx);

                    //
		    // Set tmp pixel to the displayable nearest color
                    //
		    *tmp_pixel = (Xil_unsigned8)nearest_color_idx;

                    //
		    // Calculate amount of error between nearest and
		    // actual color
                    //
		    int non_zero_error = 0;
		    for (unsigned int b = 0; b < src_nbands; b++) {
                        //
			// Calculate error in intensity
                        //
                        error[b] = curr_pix[b] - _XILI_B2F(nearest_color[b]);
                        if (error[b] != 0.0F) {
			    non_zero_error = 1;
			}
		    }

                    //
		    // if (non_zero_error) then need to distribute error, else
		    // skip this step.
                    //
		    if (non_zero_error) {
			// First line of kernel only distributes error
			// to right of key.  "rght_cnt" = number of
			// cells to right of key.
			int rght_cnt = _XILI_MIN(kern_r, src_width - 1 - x);
			const float* kern_entry = key_idx;
			buf_p = curr_pix;
			// For each cell to right of key (if any)...
			for (int k = 1; k <= rght_cnt; k++) {
			    for (b = 0; b < src_nbands; b++) {
				*(buf_p + (k * src_nbands) + b) +=
				    error[b] * kern_entry[k];
			    }
			}

			// Distribute error below key
			int down_cnt = _XILI_MIN(kern_d, end_scanline - 1 - y);
			int left_cnt = _XILI_MIN(dist_key_x, x) * -1;
			kern_entry = key_idx;
			for (int l = 0; l < down_cnt; l++) {
			    kern_entry += dist_w;
			    buf_p = buf[l + 1];
			    buf_p += (x * src_nbands);
			    for (k = left_cnt; k <= rght_cnt; k++) {
				for (b = 0; b < src_nbands; b++) {
				    *(buf_p + (k * src_nbands) + b) +=
					error[b] * kern_entry[k];
				}
			    }
			}
		    } // if (non_zero_error)

		    // Advance to next src & tmp pixels on current scanline
		    curr_pix  += src_nbands;
		    tmp_pixel++;
		} // For each pixel
 
		// Rotate lines in buffer everytime so that buf[0] always
		// points to line being processed.
		buf_p = buf[0];
		for (int k = 0; k < last_buf_line; k++)
		    buf[k] = buf[k+1];
 
		// Only need to add new line if 'src_line_num+buf_lines' <
		// 'src_height', because buffer will already contain last few
		// lines of src image.
		if (y < src_kern_end) {
		    buf[last_buf_line] = buf_p;
		    src_p = src_scanline;
		    for (x = 0; x < src_width; x++) {
			for (unsigned int b = 0; b < src_nbands; b++) {
			    buf_p[b] = _XILI_B2F(src_p[b]);
			}
			src_p += src_pixel_stride;
			buf_p += src_nbands;
		    }
		    src_scanline += src_scanline_stride;
		} else {
		    buf[last_buf_line] = NULL;
		}
		tmp_scanline += tmp_next_scan;
	    } // end for each scanline

	    //
	    // Copy data from tmp to dst taking the ROI into account
	    //
            unsigned int dst_pixel_stride;
            unsigned int dst_scanline_stride;
            Xil_unsigned8* dst_data;
            dst_storage.getStorageInfo(&dst_pixel_stride,
				       &dst_scanline_stride,
				       NULL, NULL,
				       (void**)&dst_data);
	    xili_diffusion_copy_8(tmp_base_addr, tmp_next_scan, dst_data,
				  dst_scanline_stride, dst_pixel_stride,
				  roi, dst_box, box_x, box_y);
	} else {
            //
            // General Storage Implementation.
            //

	    // Prepare to perform error diffusion into the tmp
	    // image.  Set pointers to (0, 0) in image space.
	    Xil_unsigned8* tmp_scanline = tmp_base_addr;

	    //
	    // First pass thru, so need to fill rolling buffers with
	    // first set of scanline data.  While filling buffer,
	    // convert data to float values.  (src_x, src_y) is in src
	    // image space.  "src_y" keeps track of where in src image
	    // data will next be read to fill rolling buffer.
	    //
	    int src_y = 0;
	    float* buf_p;
	    for (int i = 0; i < buf_lines; i++) {
		buf[i] = buf_mem + (i * line_size);
		buf_p = buf[i];
		for (unsigned int src_x = 0; src_x < src_width; src_x++) {
		    for (unsigned int b = 0; b < src_nbands; b++) {
			unsigned int src_pixel_stride;
			unsigned int src_scanline_stride;
			Xil_unsigned8* src_data;
			src_storage.getStorageInfo(b,
						   &src_pixel_stride,
						   &src_scanline_stride,
						   NULL,
						   (void**)&src_data);
			// Translate from image space to box space
			Xil_unsigned8* src_band = src_data +
			    (src_y - box_y) * src_scanline_stride +
			    (src_x - box_x) * src_pixel_stride;

			buf_p[b] = _XILI_B2F(*src_band);
		    }
		    buf_p += src_nbands;
		}
		// Next scanline in src
		src_y++;
	    }

	    int nearest_color_idx;
	    Xil_unsigned8* nearest_color;
	    Xil_unsigned8* tmp_pixel;
	    float* curr_pix;

	    //
	    // Do error diffusion on src image for each scanline and
	    // place results in tmp image.
	    //
	    for (unsigned int y = 0; y < end_scanline; y++) {
		curr_pix = buf[0];
		tmp_pixel = tmp_scanline;
 
		// For each pixel
		for (unsigned int x = 0; x < src_width; x++) {

		    // Do nearest_color step of operation
		    nearest_color = (Xil_unsigned8 *)
			xili_find_nearest_color(curr_pix, cmap,
						&nearest_color_idx);

		    // Set tmp pixel to the displayable nearest color
		    *tmp_pixel = (Xil_unsigned8)nearest_color_idx;

		    // Calculate amount of error between nearest and
		    // actual color
		    int non_zero_error = 0;
		    for (unsigned int b = 0; b < src_nbands; b++) {
			// get error in intensity
			if (error[b] = curr_pix[b] -
			    _XILI_B2F(nearest_color[b])) {
			    non_zero_error = 1;
			}
		    }

		    // if (non_zero_error) then need to distribute error, else
		    // skip this step.
		    if (non_zero_error) {
			// First line of kernel only distributes error
			// to right of key.  "rght_cnt" = number of
			// cells to right of key.
			int rght_cnt = _XILI_MIN(kern_r, src_width - 1 - x);
			const float* kern_entry = key_idx;
			buf_p = curr_pix;
			// For each cell to right of key (if any)...
			for (int k = 1; k <= rght_cnt; k++) {
			    for (b = 0; b < src_nbands; b++) {
				*(buf_p + (k * src_nbands) + b) +=
				    error[b] * kern_entry[k];
			    }
			}

			// Distribute error below key
			int down_cnt = _XILI_MIN(kern_d, end_scanline - 1 - y);
			int left_cnt = _XILI_MIN(dist_key_x, x) * -1;
			kern_entry = key_idx;
			for (int l = 0; l < down_cnt; l++) {
			    kern_entry += dist_w;
			    buf_p = buf[l + 1];
			    buf_p += (x * src_nbands);
			    for (k = left_cnt; k <= rght_cnt; k++) {
				for (b = 0; b < src_nbands; b++) {
				    *(buf_p + (k * src_nbands) + b) +=
					error[b] * kern_entry[k];
				}
			    }
			}
		    } // if (non_zero_error)

		    // Advance to next src & tmp pixels on current scanline
		    curr_pix  += src_nbands;
		    tmp_pixel++;
		} // For each pixel

		// Rotate lines in buffer everytime so that buf[0] always
		// points to line being processed.
		buf_p = buf[0];
		for (int k = 0; k < last_buf_line; k++)
		    buf[k] = buf[k+1];
 
		// Only need to add new line if 'src_line_num+buf_lines' <
		// 'src_height', because buffer will already contain last few
		// lines of src image.
		if (y < src_kern_end) {
		    buf[last_buf_line] = buf_p;
		    for (unsigned int src_x = 0; src_x < src_width; src_x++) {
			for (unsigned int b = 0; b < src_nbands; b++) {
			    unsigned int src_pixel_stride;
			    unsigned int src_scanline_stride;
			    Xil_unsigned8* src_data;
			    src_storage.getStorageInfo(b,
						       &src_pixel_stride,
						       &src_scanline_stride,
						       NULL,
						       (void**)&src_data);
			    // Translate from image space to box space
			    Xil_unsigned8* src_band = src_data +
				(src_y - box_y) * src_scanline_stride +
				(src_x - box_x) * src_pixel_stride;

			    buf_p[b] = _XILI_B2F(*src_band);
			}
			buf_p += src_nbands;
		    }
		    // Next scanline in src
		    src_y++;
		} else {
		    buf[last_buf_line] = NULL;
		}
		tmp_scanline += tmp_next_scan;
	    } // end for each scanline

	    //
	    // Copy data from tmp to dst taking the ROI into
	    // account.  dst will always be 1-banded so we can just get
	    // the storage info once and treat it as a pixel-sequential
	    // image.
	    //
	    unsigned int dst_scanline_stride;
	    unsigned int dst_pixel_stride;
	    Xil_unsigned8* dst_data;
	    dst_storage.getStorageInfo((unsigned int)0,
				       &dst_pixel_stride,
				       &dst_scanline_stride,
				       NULL,
				       (void**)&dst_data);
	    xili_diffusion_copy_8(tmp_base_addr, tmp_next_scan, dst_data,
				  dst_scanline_stride, dst_pixel_stride,
				  roi, dst_box, box_x, box_y);
	}

	// Free memory used by rolling buffers
	delete [] buf;
	delete [] buf_mem;
    } // while

    // Free error array and tmp image
    delete [] error;
    delete [] tmp_base_addr;

    return XIL_SUCCESS;
}


XilStatus
XilDeviceManagerComputeBYTE::ErrorDiffusion16(XilOp*       op,
					      unsigned int,
					      XilRoi*      roi,
					      XilBoxList*  bl)
{
    //
    //  Get the images for our operation.
    //
    XilImage* src = op->getSrcImage(1);
    XilImage* dst = op->getDstImage(1);

    //
    // Get params from the op
    //
    XilLookupSingle* cmap;
    op->getParam(1, (XilObject**) &cmap);
    XilKernel* dist;
    op->getParam(2, (XilObject**) &dist);

    //
    // Get XilSystemState used to report errors
    //
    XilSystemState* err_state = dst->getSystemState();

    // Get info about src image
    unsigned int src_width;
    unsigned int src_height;
    src->getSize(&src_width, &src_height);
    unsigned int src_nbands = src->getNumBands();

    // Create a 1 banded tmp image of same datatype as dst.  Dimensions
    // will be same as src.  This is where the error diffusion results
    // will go before copying into the real dst.
    unsigned int tmp_next_scan = src_width;
    Xil_signed16* tmp_base_addr = new Xil_signed16[src_width * src_height];
    if (tmp_base_addr == NULL) {
	XIL_ERROR(err_state, XIL_ERROR_RESOURCE, "di-1", TRUE);
	return XIL_FAILURE;
    }

    //
    // Get diffusion kernel info that will be needed later
    //
    unsigned int dist_w = dist->getWidth();
    unsigned int dist_h = dist->getHeight();
    unsigned int dist_key_x = dist->getKeyX();
    unsigned int dist_key_y = dist->getKeyY();
    // Get pointer to beginning of kernel data
    const float *dist_value = dist->getData();
    // Get pointer to key value of kernel
    const float *key_idx = dist_value + (dist_key_y * dist_w) + dist_key_x;
    // Get number of cells to the right of key (r == right)
    unsigned int kern_r = dist_w - dist_key_x - 1;
    // Get number of cells down from key (d == down)
    unsigned int kern_d = dist_h - dist_key_y - 1;

    // Allocate array to hold error
    float* error = new float[src_nbands];
    if (error == NULL) {
	delete [] tmp_base_addr;
	XIL_ERROR(err_state, XIL_ERROR_RESOURCE, "di-1", TRUE);
	return XIL_FAILURE;
    }

    //
    //  Loop over each of the boxes of storage we are to process.
    //
    XilBox* src_box;
    XilBox* dst_box;
    while(bl->getNext(&src_box, &dst_box)) {
        //
        //  Aquire our storage from the images.  The storage returned is
        //  valid for the box given.  Thus, any origins or child offsets
        //  have been taken into account.
        //
        XilStorage src_storage(src);
        XilStorage dst_storage(dst);
        if((src->getStorage(&src_storage, op, src_box, "XilMemory",
			    XIL_READ_ONLY)  == XIL_FAILURE) ||
           (dst->getStorage(&dst_storage, op, dst_box, "XilMemory",
			    XIL_WRITE_ONLY) == XIL_FAILURE)) {
            //
            //  Mark this box entry as having failed.  If marking the box
            //  returns XIL_FAILURE, then we return XIL_FAILURE.
            //
            if(bl->markAsFailed() == XIL_FAILURE) {
		delete [] error;
		delete [] tmp_base_addr;
                return XIL_FAILURE;
            } else {
                continue;
            }
        }

	//
	// Get the image space coordinates of the src box
	//
	int          box_x;
	int          box_y;
	unsigned int box_w;
	unsigned int box_h;
	src_box->getAsRect(&box_x, &box_y, &box_w, &box_h);

	// "end_scanline" is the max height of the src image that we
	// need to process.  This will always be <= src_height.
	unsigned int end_scanline = box_y + box_h;

	//
	// Allocate rolling buffers for converting src scanlines to
	// floats in order to distribute error.
	//
	int line_size = src_nbands * src_width;
	// "buf_lines" is the number of rolling buffer lines needed
	int buf_lines = _XILI_MIN(end_scanline, dist_h - dist_key_y);
	float* buf_mem = new float[buf_lines * line_size];
	if (buf_mem == NULL) {
	    delete [] error;
	    delete [] tmp_base_addr;
	    XIL_ERROR(err_state, XIL_ERROR_RESOURCE, "di-1", TRUE);
	    return XIL_FAILURE;
	}
	float** buf = new float*[buf_lines];
	if (buf == NULL) {
	    delete [] buf_mem;
	    delete [] error;
	    delete [] tmp_base_addr;
	    XIL_ERROR(err_state, XIL_ERROR_RESOURCE, "di-1", TRUE);
	    return XIL_FAILURE;
	}

	// "last_buf_line" is the index of last rolling buffer line
	int last_buf_line = buf_lines - 1;
	unsigned int src_kern_end = end_scanline - buf_lines;

	//
        //  Test to see if all of our storage is of type XIL_PIXEL_SEQUENTIAL.
        //  If so, implement an loop optimized for pixel-sequential storage.
        //
        if((src_storage.isType(XIL_PIXEL_SEQUENTIAL)) &&
	   (dst_storage.isType(XIL_PIXEL_SEQUENTIAL))) {
            unsigned int src_pixel_stride;
            unsigned int src_scanline_stride;
            Xil_unsigned8* src_data;
            src_storage.getStorageInfo(&src_pixel_stride,
				       &src_scanline_stride,
				       NULL, NULL,
				       (void**)&src_data);

	    // Prepare to perform error diffusion into the tmp
	    // image.  Set pointers to (0, 0) in image space.
	    Xil_unsigned8* src_scanline = src_data -
		box_y * src_scanline_stride -
		box_x * src_pixel_stride;
	    Xil_signed16* tmp_scanline = tmp_base_addr;

	    //
	    // First pass thru, so need to fill rolling buffers with
	    // first set of scanline data.  While filling buffer,
	    // convert data to float values.
	    //
	    Xil_unsigned8* src_p;
	    float* buf_p;
	    for (int i = 0; i < buf_lines; i++) {
		src_p = src_scanline;
		buf[i] = buf_mem + (i * line_size);
		buf_p = buf[i];
		for (unsigned int j = 0; j < src_width; j++) {
		    for (unsigned int k = 0; k < src_nbands; k++) {
			buf_p[k] = _XILI_B2F(src_p[k]);
		    }
		    src_p += src_pixel_stride;
		    buf_p += src_nbands;
		}
		src_scanline += src_scanline_stride;
	    }

	    int nearest_color_idx;
	    Xil_unsigned8* nearest_color;
	    Xil_signed16* tmp_pixel;
	    float* curr_pix;

	    //
	    // Do error diffusion on src image for each scanline and
	    // place results in tmp image.
	    //
	    for (unsigned int y = 0; y < end_scanline; y++) {
		curr_pix = buf[0];
		tmp_pixel = tmp_scanline;
 
		// For each pixel
		for (unsigned int x = 0; x < src_width; x++) {

		    // Do nearest_color step of operation
		    nearest_color = (Xil_unsigned8 *)
			xili_find_nearest_color(curr_pix, cmap,
						&nearest_color_idx);

		    // Set tmp pixel to the displayable nearest color
		    *tmp_pixel = (Xil_signed16)nearest_color_idx;

		    // Calculate amount of error between nearest and
		    // actual color
		    int non_zero_error = 0;
		    for (unsigned int b = 0; b < src_nbands; b++) {
			// get error in intensity
			if (error[b] = curr_pix[b] -
			    _XILI_B2F(nearest_color[b])) {
			    non_zero_error = 1;
			}
		    }

		    // if (non_zero_error) then need to distribute error, else
		    // skip this step.
		    if (non_zero_error) {
			// First line of kernel only distributes error
			// to right of key.  "rght_cnt" = number of
			// cells to right of key.
			int rght_cnt = _XILI_MIN(kern_r, src_width - 1 - x);
			const float* kern_entry = key_idx;
			buf_p = curr_pix;
			// For each cell to right of key (if any)...
			for (int k = 1; k <= rght_cnt; k++) {
			    for (b = 0; b < src_nbands; b++) {
				*(buf_p + (k * src_nbands) + b) +=
				    error[b] * kern_entry[k];
			    }
			}

			// Distribute error below key
			int down_cnt = _XILI_MIN(kern_d, end_scanline - 1 - y);
			int left_cnt = _XILI_MIN(dist_key_x, x) * -1;
			kern_entry = key_idx;
			for (int l = 0; l < down_cnt; l++) {
			    kern_entry += dist_w;
			    buf_p = buf[l + 1];
			    buf_p += (x * src_nbands);
			    for (k = left_cnt; k <= rght_cnt; k++) {
				for (b = 0; b < src_nbands; b++) {
				    *(buf_p + (k * src_nbands) + b) +=
					error[b] * kern_entry[k];
				}
			    }
			}
		    } // if (non_zero_error)

		    // Advance to next src & dst pixels on current scanline
		    curr_pix  += src_nbands;
		    tmp_pixel++;
		} // For each pixel
 
		// Rotate lines in buffer everytime so that buf[0] always
		// points to line being processed.
		buf_p = buf[0];
		for (int k = 0; k < last_buf_line; k++)
		    buf[k] = buf[k+1];
 
		// Only need to add new line if 'src_line_num+buf_lines' <
		// 'src_height', because buffer will already contain last few
		// lines of src image.
		if (y < src_kern_end) {
		    buf[last_buf_line] = buf_p;
		    src_p = src_scanline;
		    for (x = 0; x < src_width; x++) {
			for (unsigned int b = 0; b < src_nbands; b++) {
			    buf_p[b] = _XILI_B2F(src_p[b]);
			}
			src_p += src_pixel_stride;
			buf_p += src_nbands;
		    }
		    src_scanline += src_scanline_stride;
		} else {
		    buf[last_buf_line] = NULL;
		}
		tmp_scanline += tmp_next_scan;
	    } // end for each scanline

	    //
	    // Copy data from tmp to dst taking the ROI into account
	    //
            unsigned int dst_pixel_stride;
            unsigned int dst_scanline_stride;
            Xil_signed16* dst_data;
            dst_storage.getStorageInfo(&dst_pixel_stride,
				       &dst_scanline_stride,
				       NULL, NULL,
				       (void**)&dst_data);
	    xili_diffusion_copy_16(tmp_base_addr, tmp_next_scan, dst_data,
				   dst_scanline_stride, dst_pixel_stride,
				   roi, dst_box, box_x, box_y);
	} else {
            //
            // General Storage Implementation.
            //

	    // Prepare to perform error diffusion into the tmp
	    // image.  Set pointers to (0, 0) in image space.
	    Xil_signed16* tmp_scanline = tmp_base_addr;

	    //
	    // First pass thru, so need to fill rolling buffers with
	    // first set of scanline data.  While filling buffer,
	    // convert data to float values.  (src_x, src_y) is in src
	    // image space.  "src_y" keeps track of where in src image
	    // data will next be read to fill rolling buffer.
	    //
	    int src_y = 0;
	    float* buf_p;
	    for (int i = 0; i < buf_lines; i++) {
		buf[i] = buf_mem + (i * line_size);
		buf_p = buf[i];
		for (unsigned int src_x = 0; src_x < src_width; src_x++) {
		    for (unsigned int b = 0; b < src_nbands; b++) {
			unsigned int src_pixel_stride;
			unsigned int src_scanline_stride;
			Xil_unsigned8* src_data;
			src_storage.getStorageInfo(b,
						   &src_pixel_stride,
						   &src_scanline_stride,
						   NULL,
						   (void**)&src_data);
			// Translate from image space to box space
			Xil_unsigned8* src_band = src_data +
			    (src_y - box_y) * src_scanline_stride +
			    (src_x - box_x) * src_pixel_stride;

			buf_p[b] = _XILI_B2F(*src_band);
		    }
		    buf_p += src_nbands;
		}
		// Next scanline in src
		src_y++;
	    }

	    int nearest_color_idx;
	    Xil_unsigned8* nearest_color;
	    Xil_signed16* tmp_pixel;
	    float* curr_pix;

	    //
	    // Do error diffusion on src image for each scanline and
	    // place results in tmp image.
	    //
	    for (unsigned int y = 0; y < end_scanline; y++) {
		curr_pix = buf[0];
		tmp_pixel = tmp_scanline;
 
		// For each pixel
		for (unsigned int x = 0; x < src_width; x++) {

		    // Do nearest_color step of operation
		    nearest_color = (Xil_unsigned8 *)
			xili_find_nearest_color(curr_pix, cmap,
						&nearest_color_idx);

		    // Set tmp pixel to the displayable nearest color
		    *tmp_pixel = (Xil_signed16)nearest_color_idx;

		    // Calculate amount of error between nearest and
		    // actual color
		    int non_zero_error = 0;
		    for (unsigned int b = 0; b < src_nbands; b++) {
			// get error in intensity
			if (error[b] = curr_pix[b] -
			    _XILI_B2F(nearest_color[b])) {
			    non_zero_error = 1;
			}
		    }

		    // if (non_zero_error) then need to distribute error, else
		    // skip this step.
		    if (non_zero_error) {
			// First line of kernel only distributes error
			// to right of key.  "rght_cnt" = number of
			// cells to right of key.
			int rght_cnt = _XILI_MIN(kern_r, src_width - 1 - x);
			const float* kern_entry = key_idx;
			buf_p = curr_pix;
			// For each cell to right of key (if any)...
			for (int k = 1; k <= rght_cnt; k++) {
			    for (b = 0; b < src_nbands; b++) {
				*(buf_p + (k * src_nbands) + b) +=
				    error[b] * kern_entry[k];
			    }
			}

			// Distribute error below key
			int down_cnt = _XILI_MIN(kern_d, end_scanline - 1 - y);
			int left_cnt = _XILI_MIN(dist_key_x, x) * -1;
			kern_entry = key_idx;
			for (int l = 0; l < down_cnt; l++) {
			    kern_entry += dist_w;
			    buf_p = buf[l + 1];
			    buf_p += (x * src_nbands);
			    for (k = left_cnt; k <= rght_cnt; k++) {
				for (b = 0; b < src_nbands; b++) {
				    *(buf_p + (k * src_nbands) + b) +=
					error[b] * kern_entry[k];
				}
			    }
			}
		    } // if (non_zero_error)

		    // Advance to next src & dst pixels on current scanline
		    curr_pix  += src_nbands;
		    tmp_pixel++;
		} // For each pixel

		// Rotate lines in buffer everytime so that buf[0] always
		// points to line being processed.
		buf_p = buf[0];
		for (int k = 0; k < last_buf_line; k++)
		    buf[k] = buf[k+1];
 
		// Only need to add new line if 'src_line_num+buf_lines' <
		// 'src_height', because buffer will already contain last few
		// lines of src image.
		if (y < src_kern_end) {
		    buf[last_buf_line] = buf_p;
		    for (unsigned int src_x = 0; src_x < src_width; src_x++) {
			for (unsigned int b = 0; b < src_nbands; b++) {
			    unsigned int src_pixel_stride;
			    unsigned int src_scanline_stride;
			    Xil_unsigned8* src_data;
			    src_storage.getStorageInfo(b,
						       &src_pixel_stride,
						       &src_scanline_stride,
						       NULL,
						       (void**)&src_data);
			    // Translate from image space to box space
			    Xil_unsigned8* src_band = src_data +
				(src_y - box_y) * src_scanline_stride +
				(src_x - box_x) * src_pixel_stride;

			    buf_p[b] = _XILI_B2F(*src_band);
			}
			buf_p += src_nbands;
		    }
		    // Next scanline in src
		    src_y++;
		} else {
		    buf[last_buf_line] = NULL;
		}
		tmp_scanline += tmp_next_scan;
	    } // end for each scanline

	    //
	    // Copy data from tmp to dst taking the ROI into
	    // account.  dst will always be 1-banded so we can just get
	    // the storage info once and treat it as a pixel-sequential
	    // image.
	    //
	    unsigned int dst_scanline_stride;
	    unsigned int dst_pixel_stride;
	    Xil_signed16* dst_data;
	    dst_storage.getStorageInfo((unsigned int) 0,
				       &dst_pixel_stride,
				       &dst_scanline_stride,
				       NULL,
				       (void**)&dst_data);
	    xili_diffusion_copy_16(tmp_base_addr, tmp_next_scan, dst_data,
				   dst_scanline_stride, dst_pixel_stride,
				   roi, dst_box, box_x, box_y);
	}

	// Free memory used by rolling buffers
	delete [] buf;
	delete [] buf_mem;
    } // while

    // Free error array and tmp image
    delete [] error;
    delete [] tmp_base_addr;

    return XIL_SUCCESS;
}

//
//	Error Diffusion optimization for 24 bit to 8 bit 
//      when a colorcube is available. Currently
//      restricted to XIL_PIXEL_SEQUENTIAL souce data.
//      The dither kernel MUST be the floyd-steinberg
//      variety.
//
//      This implementation borrows some ideas from
//      the Independent Jpeg Group's error diffusion code
//      and extends them with packed table building
//      and unrolled color index calculations.
//	
const int NBANDS = 3;
const int NGRAYS = 256;
const int OVERSHOOT = 256;
const int UNDERSHOOT = 256;
const int TOTALGRAYS = (NGRAYS + UNDERSHOOT + OVERSHOOT);
#ifdef NOCOMPILE
const int FS3_SCALE = 16;
const int FS3_SHIFT = 4;
#endif

// 
// Error table values are shifted FS3_SHIFT for scaling,
// then 8 bits left to move into the 24 MSBs.
//
const int ERR_SHIFT = 8;
#ifdef NOCOMPILE
const int          PIX_A = 1;
const int          PIX_B = -1;
const int          PIX_C = 0;
const int          PIX_D = 1;
#endif

//
// Create a pair of tables to be used to accelerate error diffusion
// of a 3 band image to an 1 band indexed color image via a colorcube.
// The presence of the colorcube allows the critical assumption that
// the bands are independent. Thus the index contribution from each
// band can be calculated separately. Most importantly, no nearest_color
// operation is needed.
//
// One table, three if you count each band, maps error-adjusted gray level
// to a contribution to the color index value for a band.
// The second table maps error-adjusted gray level for each band to the error 
// This table actually holds 1/16 of the error value, but at a scale of
// 2^16 to match that of the scaled pixel data.
//
// Since both tables are indexed by the same value, entries are packed into
// an int, with the error in the 24 MSBs and the index in the 8 LSBs.
//
int*
create_FS_3band_8to8_tables(XilLookupColorcube* cube,
                            int*                ditherTable)
{
    //
    // Use a stack array for the thresholds. Its temporaray and can 
    // never be larger that 256 for XIL_BYTE data, so just make it 256.
    // This gets reused for each band.
    //
    float thresh[NGRAYS];

    //
    // The ditherTable array holds the error for a given band
    // and error-adjusted graylevel. This array is conceptually
    // indexed as bandError[band][gray].
    //
    //
    // It also holds, packed in the same integer the contribution to the 
    // color index value for a given band and error-adjusted gray level.
    // This array is conceptually indexed as bandIndexContrib[band][gray].
    // This is only possible for a colorcube, where the three components
    // are independent. It lets us compute the color index as:
    //
    //   index = red_contrib + green_contrib + blue_contrib + cmap_offet
    //
    // To avoid adding it in the inner loops, the cmap_offset is rolled
    // into the table entries for the first band.
    //

    //
    // Get the colorcube parameters
    //
    const int* mult = cube->getMultipliers();
    const unsigned int* dimsMinus1 = cube->getDimsMinus1();
    unsigned int offset = cube->getAdjustedOffset();

    //
    //  Construct tables for each band
    //
    for(int band=0; band<NBANDS; band++) {
	int* pTab = ditherTable + band*TOTALGRAYS;
	
	//
	// Calculate the binwidth for this band, i.e. the gray level step 
	// from one quantization level to the next. Do this in scaled integer
	// to maintain precision.
	//
	float binwidth = 255.0F / dimsMinus1[band];

	//
	// Pre-calculate the thresholds, so we don't have to do
	// it in the inner loops. The threshold is always the 
	// midpoint of each bin, since, in error diffusion, the dithering
	// is done by the error distribution process, not by varying
	// the dither threshold as in ordered dither. 
	//
	for(int i=0; i<dimsMinus1[band]; i++) {
	    thresh[i] = (i + 0.5F) * binwidth;
	}
        thresh[dimsMinus1[band]] = 256.0;
	
	//
	// Populate the range below gray level zero with the same entry
	// as that for zero. The error distribution can cause undershoots 
	// of as much as 255.
	//
        int table_inc = 1 << ERR_SHIFT;
        int table_value = (-UNDERSHOOT) << ERR_SHIFT;
	for(int gray=-UNDERSHOOT; gray<0; gray++) {
            *pTab++ = table_value;
            table_value += table_inc;
	}
	
	//
	// Populate the main range of 0...255.
	//
	int index_contrib      = 0;
	float frep_value = 0.0;
        int rep_value;
	int bin_num   = 0;
        float threshold = thresh[0];
        gray          = 0;
        while(gray < 256) {
            //
            // Populate all the table values up to the next threshold.
            // Since the only thing which changes is the error,
            // and it changes by one scaled gray level, we can
            // just add the increment at each iteration.
            //
            int table_base = index_contrib;
            rep_value = frep_value + 0.5;
            while((float)gray < threshold) {
                *pTab++ = ((gray - rep_value) << ERR_SHIFT) + table_base;
                gray++;
            }

	    //
	    // Once the gray level crosses a threshold,
	    // move to the next bin threshold. Also update
	    // the color contribution index step and the 
	    // representative value, needed to compute the error.
	    //
            threshold = thresh[++bin_num];
            index_contrib += mult[band];
            frep_value += binwidth;
	}

	//
	// Populate the range above gray level 255 with the same entry
	// as that for 255. As in the under-range case, the error
	// distribution can cause overshoots as high as 255 over max.
	// 
        index_contrib       -= mult[band];
        rep_value   = 255;
        table_value = ((256 - rep_value) << ERR_SHIFT) | index_contrib;
        
	for(gray=256; gray<(256+OVERSHOOT); gray++) {
            *pTab++ = table_value;
            table_value += table_inc;
	}

    } // End band loop

    //
    // Add in the colormap offset value to the index contribution
    // for the first band. This eliminates the need to add it in 
    // when we do the error diffusion.
    //
    int* pTab = ditherTable;
    for(int count=TOTALGRAYS; count!=0; count--) {
        *pTab += offset;
        pTab++;
    }


    return ditherTable;
}

XilStatus
XilDeviceManagerComputeBYTE::ErrorDiffusion8Preprocess(
    XilOp*        op,
    unsigned      ,
    XilRoi*       ,
    void**        compute_data,
    unsigned int* )
{
    //
    // Get params from the op
    //
    XilLookupSingle* cmap;
    op->getParam(1, (XilObject**) &cmap);
    XilKernel* dist;
    op->getParam(2, (XilObject**) &dist);

    //
    //  Figure if it's potentially the 3 band BYTE to 1 band BYTE optimized
    //  case -- if not just set compute_data to -1 indicating not to call
    //  the optimized routine.
    //
    if(op->getSrcImage(1)->getNumBands() != 3 ||
       ! cmap->isColorcube() ||
       ! xili_is_floyd_steinberg_kernel(dist)) {
        *compute_data = (void*)-1;
        return XIL_SUCCESS;
    }

    XilSystemState* err_state = op->getDstImage(1)->getSystemState();

    int first_unreferenced_slot = -1;
    int first_null_slot         = -1;

    //
    // Lock around the updating of the caching information
    //
    edcacheMutex.lock();

    //
    // Check all of the tables, looking for a match
    //
    for(int j=0; j<_XILI_NUM_ERROR_DIFFUSION_LUTS; j++) {
        //
        // Check object versions to see if this case ia a repeat
        //
        if(edcacheTable[j] != NULL &&
           cmap->isSameAs(&edcacheCmapVersion[j])) {
            //
            //  Found a match - return the index and bump ref count
            //
            *compute_data = (void*)j;
            edcacheRefCnts[j]++;

            edcacheMutex.unlock();

            return XIL_SUCCESS;
        }

        //
        // Record the first NULL (never allocated) and the first
        // unreferenced slot for possible later use if no table 
        // matches are found
        //
        if(first_null_slot == -1 && edcacheTable[j] == NULL) {
            first_null_slot = j;
        }
        if(first_unreferenced_slot == -1 && edcacheRefCnts[j] == 0) {
            first_unreferenced_slot = j;
        }
    }

    //
    // No matching DitherLut found. If an unreferenced slot
    // was found, use it, else construct an uncached table.
    //
    int new_slot;
    if(first_null_slot >= 0) {
        new_slot = first_null_slot;
    } else if(first_unreferenced_slot >= 0) {
        new_slot = first_unreferenced_slot;
    }

    if(new_slot >= 0) {
        //
        // Get the object versions for use in future comparisons
        //
        cmap->getVersion(&edcacheCmapVersion[new_slot]);

        delete edcacheTable[new_slot];
    }

    //
    //  Construct the new table
    //
    int* table = new int[NBANDS * TOTALGRAYS];
    if(table == NULL) {
        XIL_ERROR(err_state, XIL_ERROR_RESOURCE, "di-1", TRUE);
        edcacheMutex.unlock();
        return XIL_FAILURE;
    }

    //
    //  Calculate the two packed error diffusion tables
    //     Cmap index contribution.
    //     Error to be distributed.
    //
    create_FS_3band_8to8_tables((XilLookupColorcube*)cmap, table);

    if(new_slot >= 0) {
        //
        // Return the cached table index (0...3)
        //
        edcacheTable[new_slot] = table;
        edcacheRefCnts[new_slot]++;
        *compute_data = (void*)new_slot;
    } else {
        //
        // Return the ptr to the XiliOrderedDitherLut object
        // TODO: Is this too much of a hack? A valid ptr
        //       should never have a value between 0 and 3,
        //       so we should be able to distinguish the
        //       two cases. (This is all to avoid a NEW op).
        // 
        *compute_data = (void*)table;
    }

    edcacheMutex.unlock();

    return XIL_SUCCESS;
}

XilStatus       
XilDeviceManagerComputeBYTE::ErrorDiffusion8Postprocess(
    XilOp*       ,
    void*        compute_data)
{

    edcacheMutex.lock();

    //
    // See if the index is in the 0 .. 3 range.
    // If so, decrement the reference count.
    // Otherwise its a single use table, so delete it.
    //
    int data = (int)compute_data;
    if(data >= 0 && data < _XILI_NUM_ERROR_DIFFUSION_LUTS) {
        edcacheRefCnts[data]--;
    } else if(data != -1) {
        delete (int*)compute_data;
    }

    edcacheMutex.unlock();

    return XIL_SUCCESS;
}

XilStatus
xili_floyd_steinburg_3band_8to8_error_diffusion(XilOp*           op,
                                                XilRoi*          roi,
                                                XilBoxList*      bl,
                                                XilImage*        src,
                                                XilImage*        dst,
                                                XilLookupColorcube* , // cmap
                                                XilSystemState*  , // err_state
                                                int*             dither_table)
{
    //
    //  Get info about src image
    //
    unsigned int src_width;
    unsigned int src_height;
    src->getSize(&src_width, &src_height);

    //
    //  Create a 1 banded tmp image.  Dimensions will be same as src.  
    //  This is where the error diffusion results will go before copying 
    //  into the real dst.
    //
    //  We make tmp_ss so that it's 8-byte aligned.
    //  This should speed most copies.
    //
    unsigned int tmp_ss = (src_width + 7) & ~0x7;

    Xil_unsigned8* tmp_base_addr = new Xil_unsigned8[tmp_ss * src_height];
    if(tmp_base_addr == NULL) {
        return XIL_FAILURE;
    }

    //
    //  Loop over each of the boxes of storage we are to process.
    //
    XilBox* src_box;
    XilBox* dst_box;
    while(bl->getNext(&src_box, &dst_box)) {
        //
        //  Aquire our storage from the images.  The storage returned is
        //  valid for the box given.  Thus, any origins or child offsets
        //  have been taken into account.
        //
        XilStorage src_storage(src);
        XilStorage dst_storage(dst);
        if((src->getStorage(&src_storage, op, src_box, "XilMemory",
                            XIL_READ_ONLY)  == XIL_FAILURE) ||
           (dst->getStorage(&dst_storage, op, dst_box, "XilMemory",
                            XIL_WRITE_ONLY) == XIL_FAILURE)) {
            //
            //  Mark this box entry as having failed.  If marking the box
            //  returns XIL_FAILURE, then we return XIL_FAILURE.
            //
            if(bl->markAsFailed() == XIL_FAILURE) {
                delete [] tmp_base_addr;
                return XIL_FAILURE;
            } else {
                continue;
            }
        }

        //
        //  Get the image space coordinates of the src box
        //
        int          box_x;
        int          box_y;
        unsigned int box_w;
        unsigned int box_h;
        src_box->getAsRect(&box_x, &box_y, &box_w, &box_h);

        //
        //  "end_scanline" is the max height of the src image that we
        //  need to process.  This will always be <= src_height.
        //
        int end_scanline = box_y + box_h;

        //
        //  We only need to have a single line for the error buffer, since
        //  the pixels to which we distribute error can go in space
        //  BEHIND where we're pointing to.
        //
        int  src_padded_width = src_width + 2;
        int* errbuf           = new int[src_padded_width*NBANDS];
        if (errbuf == NULL) {
            delete [] tmp_base_addr;
            return XIL_FAILURE;
        }

	//
	// IMPORTANT: Init the errors to zeros. 
	// This is the error propagated into the first image line.
	//
        xili_memset(errbuf, 0, src_padded_width*NBANDS*sizeof(int));

        if(src_storage.isType(XIL_PIXEL_SEQUENTIAL)) {
            unsigned int src_ps;
            unsigned int src_ss;
            Xil_unsigned8* src_data;
            src_storage.getStorageInfo(&src_ps,
                                       &src_ss,
                                       NULL, NULL,
                                       (void**)&src_data);

            //
            //  Prepare to perform error diffusion into the tmp
            //  image.  Set pointers to (0, 0) in image space.
            //
            Xil_unsigned8* src_scanline = src_data -
				          box_y * src_ss -
				          box_x * src_ps;

            //
            //  For each line, calculate and distribute the error into
            //  a 3 line error buffer (one line for each band).
            //  Also accumulate the contributions of the 3 bands
            //  into the same line of the temporary output buffer. 
            //
            //  The error buffer starts out with all zeroes as the
            //  amount of error to propagate forward.
            //
            Xil_unsigned8* pTmpLine = tmp_base_addr;
            Xil_unsigned8* pSrcLine = src_scanline;
            for(int y=0; y<end_scanline; y++) {
                Xil_unsigned8* pSrc    = pSrcLine;
                Xil_unsigned8* pTmp    = pTmpLine;

                //
                // Determine the error and index contribution for 
                // the each band. Keep the transitory errors
                // (errA, errC and errD) in local variables
                // (hopefully registers). The calculated value
                // of errB gets put into the error buffer, to be used
                // on the next line.
                //
                // This is the logic here. Floyd-Steinberg dithering 
                // distributes errors to four neighboring pixels,
                // as shown below. X is the pixel being operated on.
                //
                //    7/16 of the error goes to pixel A
                //    3/16 of the error goes to pixel B
                //    5/16 of the error goes to pixel C
                //    1/16 of the error goes to pixel D
                //
                //         X A
                //       B C D
                // 
                // The error distributed to pixel A is reused immediately
                // in the calculation of the next pixel on the same line.
                // The errors distributed to B, C and D will be used on the 
                // following line. As we move from left to right, the 
                // new error distributed to B gets added to the error
                // at the previous C. Likewise, the new C error gets added
                // to the previous D error. So only the errors propagating
                // to position B survive in the saved error buffer. The
                // only exception is at the line end, where error C must be
                // saved. The scheme is shown below.
                //
                //      XA
                //     BCD
                //      BCD
                //       BCD
                //        BCD
                //
                // Treat the error buffer as pixel sequential.
                // This lets us use a single pointer with offsets
                // for the entries for all three bands.
                //

                //
                // Zero the error holders for all bands
                // The bands are called Red, Grn and Blu, but are
                // really just the first, second and third bands.
                //
                int errRedA = 0;
                int errRedC = 0;
                int errRedD = 0;
                int errGrnA = 0;
                int errGrnC = 0;
                int errGrnD = 0;
                int errBluA = 0;
                int errBluC = 0;
                int errBluD = 0;

                int* pErr = errbuf;
                int* dither_table_zero = dither_table + UNDERSHOOT;
                for(int count=src_width; count!=0; count--) {
                    //
                    // First band (Red)
                    // The color index is initialized here.
                    // Set the table pointer to the "Red" band
                    //
                    int* pTab = dither_table_zero;

                    int adjVal = ((errRedA + pErr[3] + 8) >> 4) + pSrc[0];
                    int tabval = pTab[adjVal];
                    int err = tabval >> 8;
                    int err1 = err;
                    int index = (tabval & 0xff);
                    int err2 = err + err;
                    pErr[0] = errRedC + (err += err2); // 3/16 (B)
                    errRedC = errRedD + (err += err2); // 5/16 (C)
                    errRedD = err1;                    // 1/16 (D)
                    errRedA = (err += err2);           // 7/16 (A)

                    //
                    // Second band (Green)
                    // Set the table pointer to the "Green" band
                    // The color index is incremented here.
                    //
                    pTab += TOTALGRAYS;

                    adjVal = ((errGrnA + pErr[4] + 8) >> 4) + pSrc[1];
                    tabval = pTab[adjVal];
                    err = tabval >> 8;
                    err1 = err;
                    index += (tabval & 0xff);
                    err2 = err + err;
                    pErr[1] = errGrnC + (err += err2);
                    errGrnC = errGrnD + (err += err2);
                    errGrnD = err1;
                    errGrnA = (err += err2);

                    pTab += TOTALGRAYS;

                    //
                    // Third band (Blue)
                    // Set the table pointer to the "Blue" band
                    // The color index is incremented here.
                    //
                    adjVal = ((errBluA + pErr[5] + 8) >> 4) + pSrc[2];
                    tabval = pTab[adjVal];
                    err = tabval >> 8;
                    err1 = err;
                    index += (tabval & 0xff);
                    err2 = err + err;
                    pErr[2] = errBluC + (err += err2);
                    errBluC = errBluD + (err += err2);
                    errBluD = err1;
                    errBluA = (err += err2);

                    pErr += 3;
                    pSrc += src_ps;
                    *pTmp++ = index;

                } // End pixel loop

                //
                // Save last error in line
                //
                int last     = 3 * (src_padded_width - 2);
                errbuf[last]   = errRedC;
                errbuf[last+1] = errGrnC;
                errbuf[last+2] = errBluC;

                pTmpLine += tmp_ss;
                pSrcLine += src_ss;

	    } // End scanline loop
        } else { // End if XIL_PIXEL_SEQUENTIAL storage
            //
            // General storage implementation
            //

            unsigned int src_ps[3];
            unsigned int src_ss[3];
            Xil_unsigned8* src_scanline[3];
            for(unsigned int b = 0; b < 3; b++) {
                Xil_unsigned8* src_data;
                src_storage.getStorageInfo(b,
                                           src_ps + b,
                                           src_ss + b,
                                           NULL,
                                           (void**)&src_data);
                //
                //  Prepare to perform error diffusion into the tmp
                //  image.  Set pointers to (0, 0) in image space.
                //
                src_scanline[b] = src_data -
			              box_y * src_ss[b] -
			              box_x * src_ps[b];
            }

            //
            //  For each line, calculate and distribute the error into
            //  a 3 line error buffer (one line for each band).
            //  Also accumulate the contributions of the 3 bands
            //  into the same line of the temporary output buffer. 
            //
            //  The error buffer starts out with all zeroes as the
            //  amount of error to propagate forward.
            //
            Xil_unsigned8* pTmpLine = tmp_base_addr;
            Xil_unsigned8* pSrcLine[3];
            for(b = 0; b < 3; b++) {
                pSrcLine[b] = src_scanline[b];
            }

            for(int y=0; y<end_scanline; y++) {
                Xil_unsigned8* pSrc[3];
                for(b = 0; b < 3; b++) {
                    pSrc[b] = pSrcLine[b];
                }
                Xil_unsigned8* pTmp    = pTmpLine;

                //
                // Determine the error and index contribution for 
                // the each band. Keep the transitory errors
                // (errA, errC and errD) in local variables
                // (hopefully registers). The calculated value
                // of errB gets put into the error buffer, to be used
                // on the next line.
                //
                // This is the logic here. Floyd-Steinberg dithering 
                // distributes errors to four neighboring pixels,
                // as shown below. X is the pixel being operated on.
                //
                //    7/16 of the error goes to pixel A
                //    3/16 of the error goes to pixel B
                //    5/16 of the error goes to pixel C
                //    1/16 of the error goes to pixel D
                //
                //         X A
                //       B C D
                // 
                // The error distributed to pixel A is reused immediately
                // in the calculation of the next pixel on the same line.
                // The errors distributed to B, C and D will be used on the 
                // following line. As we move from left to right, the 
                // new error distributed to B gets added to the error
                // at the previous C. Likewise, the new C error gets added
                // to the previous D error. So only the errors propagating
                // to position B survive in the saved error buffer. The
                // only exception is at the line end, where error C must be
                // saved. The scheme is shown below.
                //
                //      XA
                //     BCD
                //      BCD
                //       BCD
                //        BCD
                //
                // Treat the error buffer as pixel sequential.
                // This lets us use a single pointer with offsets
                // for the entries for all three bands.
                //

                //
                // Zero the error holders for all bands
                // The bands are called Red, Grn and Blu, but are
                // really just the first, second and third bands.
                //
                int errRedA = 0;
                int errRedC = 0;
                int errRedD = 0;
                int errGrnA = 0;
                int errGrnC = 0;
                int errGrnD = 0;
                int errBluA = 0;
                int errBluC = 0;
                int errBluD = 0;

                int* pErr = errbuf;
                int* dither_table_zero = dither_table + UNDERSHOOT;
                for(int count=src_width; count!=0; count--) {
                    //
                    // First band (Red)
                    // The color index is initialized here.
                    // Set the table pointer to the "Red" band
                    //
                    int* pTab = dither_table_zero;

                    int adjVal = ((errRedA + pErr[3] + 8) >> 4) + *pSrc[0];
                    int tabval = pTab[adjVal];
                    int err = tabval >> 8;
                    int err1 = err;
                    int index = (tabval & 0xff);
                    int err2 = err + err;
                    pErr[0] = errRedC + (err += err2); // 3/16 (B)
                    errRedC = errRedD + (err += err2); // 5/16 (C)
                    errRedD = err1;                    // 1/16 (D)
                    errRedA = (err += err2);           // 7/16 (A)

                    //
                    // Second band (Green)
                    // Set the table pointer to the "Green" band
                    // The color index is incremented here.
                    //
                    pTab += TOTALGRAYS;

                    adjVal = ((errGrnA + pErr[4] + 8) >> 4) + *pSrc[1];
                    tabval = pTab[adjVal];
                    err = tabval >> 8;
                    err1 = err;
                    index += (tabval & 0xff);
                    err2 = err + err;
                    pErr[1] = errGrnC + (err += err2);
                    errGrnC = errGrnD + (err += err2);
                    errGrnD = err1;
                    errGrnA = (err += err2);

                    pTab += TOTALGRAYS;

                    //
                    // Third band (Blue)
                    // Set the table pointer to the "Blue" band
                    // The color index is incremented here.
                    //
                    adjVal = ((errBluA + pErr[5] + 8) >> 4) + *pSrc[2];
                    tabval = pTab[adjVal];
                    err = tabval >> 8;
                    err1 = err;
                    index += (tabval & 0xff);
                    err2 = err + err;
                    pErr[2] = errBluC + (err += err2);
                    errBluC = errBluD + (err += err2);
                    errBluD = err1;
                    errBluA = (err += err2);

                    pErr += 3;
                    for(b = 0; b < 3; b++) {
                        pSrc[b] += src_ps[b];
                    }
                    *pTmp++ = index;

                } // End pixel loop

                //
                // Save last error in line
                //
                int last     = 3 * (src_padded_width - 2);
                errbuf[last]   = errRedC;
                errbuf[last+1] = errGrnC;
                errbuf[last+2] = errBluC;

                pTmpLine += tmp_ss;
                for(b = 0; b < 3; b++) {
                    pSrcLine[b] += src_ss[b];
                }

	    } // End scanline loop
        } // End general storage implementation

        //
        //  Copy data from tmp to dst taking the ROI into account
        //
        //  TODO: If the destination has no rois, i.e. its a simple
        //        rectangle the same size as the image, then we
        //        can skip this two step process and write directly
        //        to the destination image instead of the temporary,
        //        eliminating the copy.
        //
        unsigned int   dst_ps;
        unsigned int   dst_ss;
        Xil_unsigned8* dst_data;
        dst_storage.getStorageInfo(&dst_ps,
                                   &dst_ss,
                                   NULL, NULL,
                                   (void**)&dst_data);

        xili_diffusion_copy_8(tmp_base_addr, tmp_ss, dst_data,
                              dst_ss, dst_ps,
                              roi, dst_box, box_x, box_y);

        //
        //  Free memory used by the error buffer
        //
        delete [] errbuf;
    }

    //
    //  Free error array and tmp image
    //
    delete [] tmp_base_addr;

    return XIL_SUCCESS;
}

