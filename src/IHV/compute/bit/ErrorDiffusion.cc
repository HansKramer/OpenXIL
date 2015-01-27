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
//  Revision:	1.5
//  Last Mod:	10:09:51, 03/10/00
//
//  Description:
//	Error Diffusion
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
#pragma ident	"@(#)ErrorDiffusion.cc	1.5\t00/03/10  "

#include "XiliUtils.hh"
#include "XilDeviceManagerComputeBIT.hh"
#include "XiliDiffusionUtils.hh"


//
// Forward function declarations
//


//
// Read pixel value in image space coordinates and return as a float
// Inputs:
//  src_storage = 1-bit image storage
//  b = band number
//  x, y = x, y coordinates in src image space
//  box_x, box_y = top left coordinates of box associated with src_storage
// Returns: float value of pixel
//
static inline float
get_src_pixel_as_float(XilStorage* src_storage,
		       int b,
		       int x, int y,
		       int box_x, int box_y)
{
    unsigned int src_scanline_stride;
    unsigned int src_offset;
    Xil_unsigned8* src_data;
    src_storage->getStorageInfo(b,
				NULL,
				&src_scanline_stride,
				&src_offset,
				(void**)&src_data);

    // Convert src_data and src_offset from box space to image space.
    // Image space results are in "scanline" and "offset".
    Xil_unsigned8* scanline;
    int offset = src_offset - box_x % 8;
    if (offset >= 0) {
	scanline = src_data +
	    (y - box_y) * src_scanline_stride -
	    box_x / 8;
    } else {
	// Borrow 8 bits from the scanline
	scanline = src_data +
	    (y - box_y) * src_scanline_stride -
	    box_x / 8 - 1;
	offset += 8;
    }

    if (XIL_BMAP_TST(scanline, x + offset)) {
	return 1.0;
    } else {
	return 0.0;
    }
}

//
// Perform a bit to 1-bit ErrorDiffusion
//
XilStatus
XilDeviceManagerComputeBIT::ErrorDiffusion1(XilOp*       op,
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
    int dist_key_x = dist->getKeyX();
    int dist_key_y = dist->getKeyY();
    // Get pointer to beginning of kernel data
    const float *dist_value = dist->getData();
    // Get pointer to key value of kernel
    const float *key_idx = dist_value + (dist_key_y * dist_w) + dist_key_x;
    // Get number of cells to the right of key (r == right)
    int kern_r = dist_w - dist_key_x - 1;
    // Get number of cells down from key (d == down)
    int kern_d = dist_h - dist_key_y - 1;

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
	int end_scanline = box_y + box_h;

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
	int src_kern_end = end_scanline - buf_lines;

	// Prepare to perform error diffusion into the tmp
	// image.  Set pointers to (0, 0) in image space.
	Xil_unsigned8* tmp_scanline = tmp_base_addr;

	//
	// First pass thru, so need to fill rolling buffers with first
	// set of scanline data.  While filling buffer, convert data to
	// float values.  (src_x, src_y) is in src image space.  "src_y"
	// keeps track of where in src image data will next be read to
	// fill rolling buffer.
	//
	int src_y = 0;
	float* buf_p;
	for (int i = 0; i < buf_lines; i++) {
	    buf[i] = buf_mem + (i * line_size);
	    buf_p = buf[i];
	    for (int src_x = 0; src_x < src_width; src_x++) {
		for (unsigned int b = 0; b < src_nbands; b++) {
		    buf_p[b] = get_src_pixel_as_float(&src_storage, b,
						      src_x, src_y,
						      box_x, box_y);
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
	for (int y = 0; y < end_scanline; y++) {
	    curr_pix = buf[0];

	    // For each pixel
	    for (int x = 0; x < src_width; x++) {

		// Do nearest_color step of operation
		nearest_color = (Xil_unsigned8*)
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
		for (int b = 0; b < src_nbands; b++) {
		    // get error in intensity
		    if (error[b] = curr_pix[b] -
			(float)nearest_color[b]) {
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
		for (int src_x = 0; src_x < src_width; src_x++) {
		    for (unsigned int b = 0; b < src_nbands; b++) {
			buf_p[b] = get_src_pixel_as_float(&src_storage, b,
							  src_x, src_y,
							  box_x, box_y);
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
XilDeviceManagerComputeBIT::ErrorDiffusion8(XilOp*       op,
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
    int dist_key_x = dist->getKeyX();
    int dist_key_y = dist->getKeyY();
    // Get pointer to beginning of kernel data
    const float *dist_value = dist->getData();
    // Get pointer to key value of kernel
    const float *key_idx = dist_value + (dist_key_y * dist_w) + dist_key_x;
    // Get number of cells to the right of key (r == right)
    int kern_r = dist_w - dist_key_x - 1;
    // Get number of cells down from key (d == down)
    int kern_d = dist_h - dist_key_y - 1;

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
	int end_scanline = box_y + box_h;

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
	int src_kern_end = end_scanline - buf_lines;

	// Prepare to perform error diffusion into the tmp
	// image.  Set pointers to (0, 0) in image space.
	Xil_unsigned8* tmp_scanline = tmp_base_addr;

	//
	// First pass thru, so need to fill rolling buffers with first
	// set of scanline data.  While filling buffer, convert data to
	// float values.  (src_x, src_y) is in src image space.  "src_y"
	// keeps track of where in src image data will next be read to
	// fill rolling buffer.
	//
	int src_y = 0;
	float* buf_p;
	for (int i = 0; i < buf_lines; i++) {
	    buf[i] = buf_mem + (i * line_size);
	    buf_p = buf[i];
	    for (int src_x = 0; src_x < src_width; src_x++) {
		for (unsigned int b = 0; b < src_nbands; b++) {
		    buf_p[b] = get_src_pixel_as_float(&src_storage, b,
						      src_x, src_y,
						      box_x, box_y);
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
	for (int y = 0; y < end_scanline; y++) {
	    curr_pix = buf[0];
	    tmp_pixel = tmp_scanline;
 
	    // For each pixel
	    for (int x = 0; x < src_width; x++) {

		// Do nearest_color step of operation
		nearest_color = (Xil_unsigned8*)
		    xili_find_nearest_color(curr_pix, cmap,
					    &nearest_color_idx);

		// Set tmp pixel to the displayable nearest color
		*tmp_pixel = (Xil_unsigned8)nearest_color_idx;

		// Calculate amount of error between nearest and
		// actual color
		int non_zero_error = 0;
		for (int b = 0; b < src_nbands; b++) {
		    // get error in intensity
		    if (error[b] = curr_pix[b] -
			(float)nearest_color[b]) {
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
		for (int src_x = 0; src_x < src_width; src_x++) {
		    for (unsigned int b = 0; b < src_nbands; b++) {
			buf_p[b] = get_src_pixel_as_float(&src_storage, b,
							  src_x, src_y,
							  box_x, box_y);
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
XilDeviceManagerComputeBIT::ErrorDiffusion16(XilOp*       op,
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
    int dist_key_x = dist->getKeyX();
    int dist_key_y = dist->getKeyY();
    // Get pointer to beginning of kernel data
    const float *dist_value = dist->getData();
    // Get pointer to key value of kernel
    const float *key_idx = dist_value + (dist_key_y * dist_w) + dist_key_x;
    // Get number of cells to the right of key (r == right)
    int kern_r = dist_w - dist_key_x - 1;
    // Get number of cells down from key (d == down)
    int kern_d = dist_h - dist_key_y - 1;

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
	int end_scanline = box_y + box_h;

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
	int src_kern_end = end_scanline - buf_lines;

	// Prepare to perform error diffusion into the tmp
	// image.  Set pointers to (0, 0) in image space.
	Xil_signed16* tmp_scanline = tmp_base_addr;

	//
	// First pass thru, so need to fill rolling buffers with first
	// set of scanline data.  While filling buffer, convert data to
	// float values.  (src_x, src_y) is in src image space.  "src_y"
	// keeps track of where in src image data will next be read to
	// fill rolling buffer.
	//
	int src_y = 0;
	float* buf_p;
	for (int i = 0; i < buf_lines; i++) {
	    buf[i] = buf_mem + (i * line_size);
	    buf_p = buf[i];
	    for (int src_x = 0; src_x < src_width; src_x++) {
		for (unsigned int b = 0; b < src_nbands; b++) {
		    buf_p[b] = get_src_pixel_as_float(&src_storage, b,
						      src_x, src_y,
						      box_x, box_y);
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
	for (int y = 0; y < end_scanline; y++) {
	    curr_pix = buf[0];
	    tmp_pixel = tmp_scanline;
 
	    // For each pixel
	    for (int x = 0; x < src_width; x++) {

		// Do nearest_color step of operation
		nearest_color = (Xil_unsigned8*)
		    xili_find_nearest_color(curr_pix, cmap,
					    &nearest_color_idx);

		// Set tmp pixel to the displayable nearest color
		*tmp_pixel = (Xil_signed16)nearest_color_idx;

		// Calculate amount of error between nearest and
		// actual color
		int non_zero_error = 0;
		for (int b = 0; b < src_nbands; b++) {
		    // get error in intensity
		    if (error[b] = curr_pix[b] -
			(float)nearest_color[b]) {
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
		for (int src_x = 0; src_x < src_width; src_x++) {
		    for (unsigned int b = 0; b < src_nbands; b++) {
			buf_p[b] = get_src_pixel_as_float(&src_storage, b,
							  src_x, src_y,
							  box_x, box_y);
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

	// Free memory used by rolling buffers
	delete [] buf;
	delete [] buf_mem;
    } // while

    // Free error array and tmp image
    delete [] error;
    delete [] tmp_base_addr;

    return XIL_SUCCESS;
}
