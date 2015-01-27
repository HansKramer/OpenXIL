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
//  Revision:	1.7
//  Last Mod:	10:12:24, 03/10/00
//
//  Description:
//	Error Diffusion
//	
//	
//	
//	
//  TODO 5apr96: Some of the code could be factored out into separate
//  functions to reduce code size.  Add other optimized versions.  See
//  code that is currently ifdef-ed out and also refer to XIL 1.2 code.
//	
//  MT-level:  <??????>
//	
//------------------------------------------------------------------------
//	COPYRIGHT
//------------------------------------------------------------------------
#pragma ident	"@(#)ErrorDiffusion.cc	1.7\t00/03/10  "

#include "XiliUtils.hh"
#include "XilDeviceManagerComputeSHORT.hh"
#include "XiliDiffusionUtils.hh"


//
// Forward function declarations
//

static XilStatus
default_16to1_error_diffusion(XilOp* op,
			     XilRoi* roi,
			     XilBoxList* bl,
			     XilImage* src,
			     XilImage* dst,
			     XilLookupSingle* cmap,
			     XilKernel* dist,
			     XilSystemState* err_state);

static XilStatus
default_16to8_error_diffusion(XilOp* op,
			      XilRoi* roi,
			      XilBoxList* bl,
			      XilImage* src,
			      XilImage* dst,
			      XilLookupSingle* cmap,
			      XilKernel* dist,
			      XilSystemState* err_state);

static XilStatus
default_16to16_error_diffusion(XilOp* op,
			       XilRoi* roi,
			       XilBoxList* bl,
			       XilImage* src,
			       XilImage* dst,
			       XilLookupSingle* cmap,
			       XilKernel* dist,
			       XilSystemState* err_state);

static XilStatus
xili_floyd_steinburg_1band_16to1_error_diffusion(XilOp*           op,
                                                XilRoi*          roi,
                                                XilBoxList*      bl,
                                                XilImage*        src,
                                                XilImage*        dst,
                                                XilLookupSingle* cmap,
                                                XilSystemState*  err_state);



//
// Perform an short to 1-bit ErrorDiffusion
//
XilStatus
XilDeviceManagerComputeSHORT::ErrorDiffusion1(XilOp*       op,
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
    // See if we can use the code specialized for the
    // Floyd-Steinberg kernel. Its much faster.
    //
    XilStatus stat;
    if(src->getNumBands() == 1 && xili_is_floyd_steinberg_kernel(dist)) {
        stat = xili_floyd_steinburg_1band_16to1_error_diffusion(
                  op, roi, bl, src, dst, cmap, err_state);
        //
        //  Go ahead and try calling the default version if our special-case
        //  failed just in case its possible to continue.
        //
        if(stat == XIL_FAILURE) {
            stat = default_16to1_error_diffusion(
                    op, roi, bl, src, dst, cmap, dist, err_state);
        }
    } else {
        stat = default_16to1_error_diffusion(
                op, roi, bl, src, dst, cmap, dist, err_state);
    }

    return stat;
}

static XilStatus
default_16to1_error_diffusion(XilOp* op,
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

	//
        //  Test to see if our src storage is of type
        //  XIL_PIXEL_SEQUENTIAL.  If so, implement an loop optimized
        //  for pixel-sequential storage.
        //
        if(src_storage.isType(XIL_PIXEL_SEQUENTIAL)) {
            unsigned int src_pixel_stride;
            unsigned int src_scanline_stride;
            Xil_signed16* src_data;
            src_storage.getStorageInfo(&src_pixel_stride,
				       &src_scanline_stride,
				       NULL, NULL,
				       (void**)&src_data);

	    // Prepare to perform error diffusion into the tmp
	    // image.  Set pointers to (0, 0) in image space.
	    Xil_signed16* src_scanline = src_data -
		box_y * src_scanline_stride -
		box_x * src_pixel_stride;
	    Xil_unsigned8* tmp_scanline = tmp_base_addr;

	    //
	    // First pass thru, so need to fill rolling buffers with
	    // first set of scanline data.  While filling buffer,
	    // convert data to float values.
	    //
	    Xil_signed16* src_p;
	    float* buf_p;
	    for (int i = 0; i < buf_lines; i++) {
		src_p = src_scanline;
		buf[i] = buf_mem + (i * line_size);
		buf_p = buf[i];
		for (int j = 0; j < src_width; j++) {
		    for (int k = 0; k < src_nbands; k++) {
			buf_p[k] = (float)src_p[k];
		    }
		    src_p += src_pixel_stride;
		    buf_p += src_nbands;
		}
		src_scanline += src_scanline_stride;
	    }

	    int nearest_color_idx;
	    Xil_signed16* nearest_color;
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
		    nearest_color = (Xil_signed16 *)
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
		    src_p = src_scanline;
		    for (x = 0; x < src_width; x++) {
			for (int b = 0; b < src_nbands; b++) {
			    buf_p[b] = (float)src_p[b];
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
		for (int src_x = 0; src_x < src_width; src_x++) {
		    for (unsigned int b = 0; b < src_nbands; b++) {
			unsigned int src_pixel_stride;
			unsigned int src_scanline_stride;
			Xil_signed16* src_data;
			src_storage.getStorageInfo(b,
						   &src_pixel_stride,
						   &src_scanline_stride,
						   NULL,
						   (void**)&src_data);
			// Translate from image space to box space
			Xil_signed16* src_band = src_data +
			    (src_y - box_y) * src_scanline_stride +
			    (src_x - box_x) * src_pixel_stride;

			buf_p[b] = (float)*src_band;
		    }
		    buf_p += src_nbands;
		}
		// Next scanline in src
		src_y++;
	    }

	    int nearest_color_idx;
	    Xil_signed16* nearest_color;
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
		    nearest_color = (Xil_signed16 *)
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
			    unsigned int src_pixel_stride;
			    unsigned int src_scanline_stride;
			    Xil_signed16* src_data;
			    src_storage.getStorageInfo(b,
						       &src_pixel_stride,
						       &src_scanline_stride,
						       NULL,
						       (void**)&src_data);
			    // Translate from image space to box space
			    Xil_signed16* src_band = src_data +
				(src_y - box_y) * src_scanline_stride +
				(src_x - box_x) * src_pixel_stride;

			    buf_p[b] = (float)*src_band;
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

XilStatus
XilDeviceManagerComputeSHORT::ErrorDiffusion8(XilOp*       op,
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

    // Figure which case we are in and choose appropriate code
    XilStatus stat;
#if 0
    switch (src->getBands()) {
	case 1:
    	    if (IsFloydSteinbergKernel(dist))
    		stat = FS_band1_16to8(src, dst, cmap);

    	    else
    		stat = default_16to8_error_diffusion(src, dst, cmap, dist);
	    break;

	default:
	    stat = default_16to8_error_diffusion(src, dst, cmap, dist);
	    break;
    }
#else
    stat = default_16to8_error_diffusion(op, roi, bl, src, dst, cmap, dist,
					 err_state);
#endif    
    return stat;
}

static XilStatus
default_16to8_error_diffusion(XilOp* op,
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

	//
        //  Test to see if all of our storage is of type XIL_PIXEL_SEQUENTIAL.
        //  If so, implement an loop optimized for pixel-sequential storage.
        //
        if((src_storage.isType(XIL_PIXEL_SEQUENTIAL)) &&
	   (dst_storage.isType(XIL_PIXEL_SEQUENTIAL))) {
            unsigned int src_pixel_stride;
            unsigned int src_scanline_stride;
            Xil_signed16* src_data;
            src_storage.getStorageInfo(&src_pixel_stride,
				       &src_scanline_stride,
				       NULL, NULL,
				       (void**)&src_data);

	    // Prepare to perform error diffusion into the tmp
	    // image.  Set pointers to (0, 0) in image space.
	    Xil_signed16* src_scanline = src_data -
		box_y * src_scanline_stride -
		box_x * src_pixel_stride;
	    Xil_unsigned8* tmp_scanline = tmp_base_addr;

	    //
	    // First pass thru, so need to fill rolling buffers with
	    // first set of scanline data.  While filling buffer,
	    // convert data to float values.
	    //
	    Xil_signed16* src_p;
	    float* buf_p;
	    for (int i = 0; i < buf_lines; i++) {
		src_p = src_scanline;
		buf[i] = buf_mem + (i * line_size);
		buf_p = buf[i];
		for (int j = 0; j < src_width; j++) {
		    for (int k = 0; k < src_nbands; k++) {
			buf_p[k] = (float)src_p[k];
		    }
		    src_p += src_pixel_stride;
		    buf_p += src_nbands;
		}
		src_scanline += src_scanline_stride;
	    }

	    int nearest_color_idx;
	    Xil_signed16* nearest_color;
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
		    nearest_color = (Xil_signed16 *)
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
		    src_p = src_scanline;
		    for (x = 0; x < src_width; x++) {
			for (int b = 0; b < src_nbands; b++) {
			    buf_p[b] = (float)src_p[b];
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
		for (int src_x = 0; src_x < src_width; src_x++) {
		    for (unsigned int b = 0; b < src_nbands; b++) {
			unsigned int src_pixel_stride;
			unsigned int src_scanline_stride;
			Xil_signed16* src_data;
			src_storage.getStorageInfo(b,
						   &src_pixel_stride,
						   &src_scanline_stride,
						   NULL,
						   (void**)&src_data);
			// Translate from image space to box space
			Xil_signed16* src_band = src_data +
			    (src_y - box_y) * src_scanline_stride +
			    (src_x - box_x) * src_pixel_stride;

			buf_p[b] = (float)*src_band;
		    }
		    buf_p += src_nbands;
		}
		// Next scanline in src
		src_y++;
	    }

	    int nearest_color_idx;
	    Xil_signed16* nearest_color;
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
		    nearest_color = (Xil_signed16 *)
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
			    unsigned int src_pixel_stride;
			    unsigned int src_scanline_stride;
			    Xil_signed16* src_data;
			    src_storage.getStorageInfo(b,
						       &src_pixel_stride,
						       &src_scanline_stride,
						       NULL,
						       (void**)&src_data);
			    // Translate from image space to box space
			    Xil_signed16* src_band = src_data +
				(src_y - box_y) * src_scanline_stride +
				(src_x - box_x) * src_pixel_stride;

			    buf_p[b] = (float)*src_band;
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
XilDeviceManagerComputeSHORT::ErrorDiffusion16(XilOp*       op,
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

    XilStatus stat;
#if 0
    switch (src->getNumBands()) {
        case 1:
            if (IsFloydSteinbergKernel(dist))
                stat = FS_band1_16to16(src, dst, cmap);

            else
    	    	stat  = default_16to16_error_diffusion(src, dst, cmap, dist);
            break;

        default:
    	    stat  = default_16to16_error_diffusion(src, dst, cmap, dist);
            break;
    }
#else
    stat = default_16to16_error_diffusion(op, roi, bl, src, dst, cmap,
					  dist, err_state);
#endif
    return stat;
}


static XilStatus
default_16to16_error_diffusion(XilOp* op,
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

	//
        //  Test to see if all of our storage is of type XIL_PIXEL_SEQUENTIAL.
        //  If so, implement an loop optimized for pixel-sequential storage.
        //
        if((src_storage.isType(XIL_PIXEL_SEQUENTIAL)) &&
	   (dst_storage.isType(XIL_PIXEL_SEQUENTIAL))) {
            unsigned int src_pixel_stride;
            unsigned int src_scanline_stride;
            Xil_signed16* src_data;
            src_storage.getStorageInfo(&src_pixel_stride,
				       &src_scanline_stride,
				       NULL, NULL,
				       (void**)&src_data);

	    // Prepare to perform error diffusion into the tmp
	    // image.  Set pointers to (0, 0) in image space.
	    Xil_signed16* src_scanline = src_data -
		box_y * src_scanline_stride -
		box_x * src_pixel_stride;
	    Xil_signed16* tmp_scanline = tmp_base_addr;

	    //
	    // First pass thru, so need to fill rolling buffers with
	    // first set of scanline data.  While filling buffer,
	    // convert data to float values.
	    //
	    Xil_signed16* src_p;
	    float* buf_p;
	    for (int i = 0; i < buf_lines; i++) {
		src_p = src_scanline;
		buf[i] = buf_mem + (i * line_size);
		buf_p = buf[i];
		for (int j = 0; j < src_width; j++) {
		    for (int k = 0; k < src_nbands; k++) {
			buf_p[k] = (float)src_p[k];
		    }
		    src_p += src_pixel_stride;
		    buf_p += src_nbands;
		}
		src_scanline += src_scanline_stride;
	    }

	    int nearest_color_idx;
	    Xil_signed16* nearest_color;
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
		    nearest_color = (Xil_signed16 *)
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
		    src_p = src_scanline;
		    for (x = 0; x < src_width; x++) {
			for (int b = 0; b < src_nbands; b++) {
			    buf_p[b] = (float)src_p[b];
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
		for (int src_x = 0; src_x < src_width; src_x++) {
		    for (unsigned int b = 0; b < src_nbands; b++) {
			unsigned int src_pixel_stride;
			unsigned int src_scanline_stride;
			Xil_signed16* src_data;
			src_storage.getStorageInfo(b,
						   &src_pixel_stride,
						   &src_scanline_stride,
						   NULL,
						   (void**)&src_data);
			// Translate from image space to box space
			Xil_signed16* src_band = src_data +
			    (src_y - box_y) * src_scanline_stride +
			    (src_x - box_x) * src_pixel_stride;

			buf_p[b] = (float)*src_band;
		    }
		    buf_p += src_nbands;
		}
		// Next scanline in src
		src_y++;
	    }

	    int nearest_color_idx;
	    Xil_signed16* nearest_color;
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
		    nearest_color = (Xil_signed16 *)
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
			    unsigned int src_pixel_stride;
			    unsigned int src_scanline_stride;
			    Xil_signed16* src_data;
			    src_storage.getStorageInfo(b,
						       &src_pixel_stride,
						       &src_scanline_stride,
						       NULL,
						       (void**)&src_data);
			    // Translate from image space to box space
			    Xil_signed16* src_band = src_data +
				(src_y - box_y) * src_scanline_stride +
				(src_x - box_x) * src_pixel_stride;

			    buf_p[b] = (float)*src_band;
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

#define FS_SHIFT 12

#define FS_16TO1_1BAND_ERR_DIST(err)          \
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
// Optimized case for single-band 16 to 1 bit error diffusion,
// specific to the Floyd-Steinberg kernel
//
//                     | |X|A|
//                     -------
//                     |B|C|D|
//
// The value of pixel X is tested against the midpoint of the two cmap values. 
// If less, the dst becomes lo_idx, else hi_idx(0 or 1). The error 
// (actual value minus the either lo_val or hi_val) is distributed to 
// pixels A, B, C, D.
//
// All distribution coefficients are in 16ths, so we use 
// scaled integer arithmetic. All values are left shifted by FS_SHIFT
// (currently 12) to preserve precision. The multiplies by x/16 are 
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
xili_floyd_steinburg_1band_16to1_error_diffusion(XilOp*           op,
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
        Xil_signed16* cmap_data = (Xil_signed16*)cmap->getData();

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
        Xil_signed16* src_data;
        src_storage.getStorageInfo(&src_pixel_stride,
                                   &src_scanline_stride,
                                   NULL, NULL,
                                   (void**)&src_data);

        //
        // Prepare to perform error diffusion into the tmp
        // image.  Set pointers to (0, 0) in image space.
        //
        Xil_signed16* src_scanline = src_data -
            box_y * src_scanline_stride -
            box_x * src_pixel_stride;
        Xil_unsigned8* tmp_scanline = tmp_base_addr;

        //
        // On the first line, fill the error accumulation buffer
        // with the first line of source data. Convert to floats.
        //
        Xil_signed16* pSrc = src_scanline;
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
            //  Each execution of the FS_16TO1_1BAND_ERR_DIST macro
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
                    FS_16TO1_1BAND_ERR_DIST(err);
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

                FS_16TO1_1BAND_ERR_DIST(err);

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
