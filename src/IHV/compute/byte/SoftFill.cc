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
//  File:	SoftFill.cc
//  Project:	XIL
//  Revision:	1.7
//  Last Mod:	10:10:48, 03/10/00
//
//  Description:
//	soft fill
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
//	COPYRIGHT
//------------------------------------------------------------------------
#pragma ident	"@(#)SoftFill.cc	1.7\t00/03/10  "

#include "XiliUtils.hh"
#include "XilDeviceManagerComputeBYTE.hh"
#include "xili_fill_utils.hh"
#include "xili_svd.hh"


//
// Data structure declarations
//
typedef struct {
    long  lx;				// curr leftmost x
    long  rx;				// curr rightmost x
    Xil_unsigned8	*src_pixel;	// curr pixel in image
    float		*weight_pixel;	// curr pixel in weight image
    Xil_unsigned8	*fill_scan;	// curr scanline in writemask
    long  plx;				// parent leftx
    long  prx;				// parent rightx
    long  y;				// curr y
    Xil_signed16	dir;		// current direction (+1,-1)
} sfill8_stack_frame_t; 

struct sfill8_stack_block {
    struct sfill8_stack_block	*prev;
    struct sfill8_stack_block	*next;
    sfill8_stack_frame_t	frame[XILI_FILL_MAX_STACK_FRAMES];
};
typedef struct sfill8_stack_block sfill8_stack_block_t;

struct sfill8_stack_ctrl {
    struct sfill8_stack_block	*curr_block;
    int				curr_idx;
};
typedef struct sfill8_stack_ctrl sfill8_stack_ctrl_t;

typedef struct {
    long 	xmax;
    long 	ymax;
    Xil_unsigned8*	bg;
    unsigned int	nbg;
    unsigned int	nbands;
    float	*mtx;
    unsigned int	src_next_pixel;
    long		src_next_scan;
    long		fill_next_scan;
    long		weight_next_scan;
    struct sfill8_stack_ctrl	*stack_ctrl;
} sfill8_data_t;


//
// Forward function declarations
//
static int
SoftFill8(long x, long y, Xil_unsigned8* src_pixel,
	  Xil_unsigned8* fill_scan, float* weight_pixel,
	  sfill8_data_t* fill_ptr, XilSystemState* err_state);

static int
Sfill8Push(sfill8_stack_ctrl_t *ctrl_ptr, 
	   sfill8_stack_frame_t *local_frame_ptr);

static int
Sfill8Pop(sfill8_stack_ctrl_t *ctrl_ptr, 
	  sfill8_stack_frame_t *local_frame_ptr);

static Xil_boolean
Sfill8PixelInside(Xil_unsigned8* pixel,
		  float* weight_pixel,
		  Xil_unsigned8* bg,
		  unsigned int nbands,
		  float* mtx,
		  unsigned int nbg,
		  Xil_unsigned8* scan,
		  long offset);

static void
Sfill8Shadow(sfill8_stack_frame_t *frame,
	     sfill8_data_t	 *fill_ptr,
	     long		 lx,
	     Xil_unsigned8	 *pixel,
	     float	 	 *weight_pixel,
	     long		 rx);

static Xil_boolean
Sfill8PixelInsideG(XilStorage* storage,
		   float* weight_pixel,
		   Xil_unsigned8* bg,
		   unsigned int nbands,
		   float* mtx,
		   unsigned int nbg,
		   Xil_unsigned8* scan,
		   unsigned int x,
		   unsigned int y,
		   int box_x,
		   int box_y);
static int
SoftFill8G(long x, long y, XilStorage* src_storage,
	   Xil_unsigned8* fill_scan, float* weight_pixel,
	   sfill8_data_t* fill_ptr, XilSystemState* err_state,
	   int box_x, int box_y);


//
// Perform a Soft Fill operation.
//
// The pixel sequential case has no suffix, the corresponding general
// storage case have functions that end in "G".
//
XilStatus
XilDeviceManagerComputeBYTE::SoftFill(XilOp*       op,
				      unsigned,
				      XilRoi*      roi,
				      XilBoxList*  bl)
{
    //
    //  Get the images for our operation.
    //
    XilImage* src1 = op->getSrcImage(1);
    XilImage* dest = op->getDstImage(1);

    //
    //  Get params from the op
    //
    //  Get seed
    unsigned int x_seed;
    unsigned int y_seed;
    op->getParam(1, &x_seed);		// Value is already in image space
    op->getParam(2, &y_seed);		// Value is already in image space

    // Get fg
    Xil_unsigned8* fgcolor;
    op->getParam(3, (void**)&fgcolor);

    // Get number of bg colors
    unsigned int num_bg;
    op->getParam(4, &num_bg);

    // Get bg colors
    Xil_unsigned8* bgcolor;
    op->getParam(5, (void**)&bgcolor);

    // Get fill color
    Xil_unsigned8* fill_color;
    op->getParam(6, (void**)&fill_color);

    //
    // Get system state for reporting errors
    //
    XilSystemState* err_state = dest->getSystemState();

    //
    // Get some info about the source image
    //
    unsigned int x_size;
    unsigned int y_size;
    src1->getSize(&x_size, &y_size);
    unsigned int nbands = src1->getNumBands();

    Xil_unsigned8* color_diff = new Xil_unsigned8[nbands];
    if(color_diff == NULL) {
	XIL_ERROR(err_state, XIL_ERROR_RESOURCE, "di-1", TRUE);
        return XIL_FAILURE;
    }
    float *mtx = new float[nbands*num_bg];
    if(mtx == NULL) {
	delete [] color_diff;
	XIL_ERROR(err_state, XIL_ERROR_RESOURCE, "di-1", TRUE);
        return XIL_FAILURE;
    }

    // Last basis color, Basis[n]
    Xil_unsigned8* last_bgcolor = &bgcolor[nbands*(num_bg-1)];

    for (unsigned int i = 0; i < nbands; i++) {
	color_diff[i] = fgcolor[i] - fill_color[i];

        // first row Basis[0]-Basis[n]
        mtx[i] = (float)(fgcolor[i] - last_bgcolor[i]);
    }

    // Compute remaining Basis[i] - Basis[n] if any
    for (unsigned int j = 0; j < num_bg - 1; j++) {
        for (i = 0; i < nbands; i++) {
	    mtx[((j+1)*nbands)+i] = \
                    (float) (bgcolor[(j*nbands)+i] - last_bgcolor[i]);
        }
    }

    // Invert the matrix
    float *mtx_invert = new float[nbands*num_bg];
    if(mtx_invert == NULL) {
	delete [] mtx;
	delete [] color_diff;
	XIL_ERROR(err_state, XIL_ERROR_RESOURCE, "di-1", TRUE);
        return XIL_FAILURE;
    }
    svinvrt(nbands, num_bg, mtx, mtx_invert);
    delete [] mtx;

    //
    // Create and init a bitmap to store writemask for fill color
    //
    unsigned int fill_next_scan = (x_size + 7) / 8;
    Xil_unsigned8* fill_base_addr = new Xil_unsigned8[fill_next_scan * y_size];
    if(fill_base_addr == NULL) {
	delete [] mtx_invert;
	delete [] color_diff;
	XIL_ERROR(err_state, XIL_ERROR_RESOURCE, "di-1", TRUE);
        return XIL_FAILURE;
    }
    xili_memset(fill_base_addr, 0, fill_next_scan * y_size);

    //
    // Create single band float image to store "weight" (percentage) for
    // fill color
    //
    unsigned int weight_next_scan = x_size;
    float* weight_base_addr = new float[weight_next_scan * y_size];
    if(weight_base_addr == NULL) {
	delete [] mtx_invert;
	delete [] color_diff;
	delete [] fill_base_addr;
	XIL_ERROR(err_state, XIL_ERROR_RESOURCE, "di-1", TRUE);
        return XIL_FAILURE;
    }

    //
    //  Loop over each of the boxes of storage we are to process.
    //
    XilBox* src1_box;
    XilBox* dest_box;
    while(bl->getNext(&src1_box, &dest_box)) {
        //
        //  Aquire our storage from the images.  The storage returned is valid
        //  for the box given.  Thus, any origins or child offsets have been
        //  taken into account.
        //
        XilStorage  src1_storage(src1);
        XilStorage  dest_storage(dest);
        if((src1->getStorage(&src1_storage, op, src1_box, "XilMemory",
                             XIL_READ_ONLY)  == XIL_FAILURE) ||
           (dest->getStorage(&dest_storage, op, dest_box, "XilMemory",
                             XIL_WRITE_ONLY) == XIL_FAILURE)) {
            //
            //  Mark this box entry as having failed.  If marking the box
            //  returns XIL_FAILURE, then we return XIL_FAILURE.
            //
            if(bl->markAsFailed() == XIL_FAILURE) {
		delete [] weight_base_addr;
		delete [] fill_base_addr;
		delete [] mtx_invert;
		delete [] color_diff;
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
	src1_box->getAsRect(&box_x, &box_y, &box_w, &box_h);

	//
        //  Test to see if all of our storage is of type XIL_PIXEL_SEQUENTIAL.
        //  If so, implement an loop optimized for pixel-sequential storage.
        //
        if((src1_storage.isType(XIL_PIXEL_SEQUENTIAL)) &&
	   (dest_storage.isType(XIL_PIXEL_SEQUENTIAL))) {
            unsigned int   src1_pixel_stride;
            unsigned int   src1_scanline_stride;
            Xil_unsigned8* src1_data;
            src1_storage.getStorageInfo(&src1_pixel_stride,
                                        &src1_scanline_stride,
                                        NULL, NULL,
                                        (void**)&src1_data);
            
            unsigned int   dest_pixel_stride;
            unsigned int   dest_scanline_stride;
            Xil_unsigned8* dest_data;
            dest_storage.getStorageInfo(&dest_pixel_stride,
                                        &dest_scanline_stride,
                                        NULL, NULL,
                                        (void**)&dest_data);

	    //
	    // Convert seed coordinates from image space to box space
	    //
	    int xs = x_seed - box_x;
	    int ys = y_seed - box_y;

	    //
	    // Find seed pixel location in src, writemask, and weight
	    // image.  Use appropriate image/box space seed coordinates.
	    //
	    Xil_unsigned8* src_pixel = src1_data +
		ys * src1_scanline_stride +
		xs * src1_pixel_stride;
	    float* weight_pixel = weight_base_addr +
		y_seed * weight_next_scan +
		x_seed;
	    Xil_unsigned8* fill_scanline = fill_base_addr +
		y_seed * fill_next_scan;

	    //
	    // Create and init sfill data struct
	    //
	    sfill8_data_t fill_data;
	    fill_data.xmax = x_size - 1;
	    fill_data.ymax = y_size - 1;
	    fill_data.bg = last_bgcolor;
	    fill_data.nbg = num_bg;
	    fill_data.nbands = nbands;
	    fill_data.mtx = mtx_invert;
	    fill_data.src_next_pixel = src1_pixel_stride;
	    fill_data.src_next_scan = src1_scanline_stride;
	    fill_data.fill_next_scan = fill_next_scan;
	    fill_data.weight_next_scan = weight_next_scan;

	    //
	    // Allocate 1st block of stack and stack control
	    //
	    sfill8_stack_ctrl_t *ctrl = new sfill8_stack_ctrl_t;
	    if(ctrl == NULL) {
		delete [] weight_base_addr;
		delete [] fill_base_addr;
		delete [] mtx_invert;
		delete [] color_diff;
		XIL_ERROR(err_state, XIL_ERROR_RESOURCE, "di-1", TRUE);
		return XIL_FAILURE;
	    }
	    sfill8_stack_block_t *block = new sfill8_stack_block_t;
	    if(block == NULL) {
		delete ctrl;
		delete [] weight_base_addr;
		delete [] fill_base_addr;
		delete [] mtx_invert;
		delete [] color_diff;
		XIL_ERROR(err_state, XIL_ERROR_RESOURCE, "di-1", TRUE);
		return XIL_FAILURE;
	    }
	    block->prev = NULL;		    // no previous block
	    block->next = NULL;		    // no next block
	    ctrl->curr_block = block;	    // current block
	    ctrl->curr_idx = 0;		    // current avail frame index
	    fill_data.stack_ctrl = ctrl;    // store stack control

	    //
	    // Create fill write mask bit image.  We use image space
	    // coordinates here.
	    //
	    if (SoftFill8(x_seed, y_seed, src_pixel, fill_scanline,
			  weight_pixel, &fill_data, err_state) == XIL_FAILURE) {
		//
		// Failed during fill operation, attempt to clean up and
		// return.  Must delete chain of alloc blocks following
		// the next ptr
		//
		while (block != NULL) {
		    ctrl->curr_block = block->next;	// save &next block
		    delete block;			// free curr block
		    block = ctrl->curr_block;		// curr = next
		}
		// Free other allocs
		delete ctrl;
		delete [] weight_base_addr;
		delete [] fill_base_addr;
		delete [] mtx_invert;
		delete [] color_diff;
		// TODO: is XIL_ERROR() call need here like in Fill?
		return XIL_FAILURE;
	    }

            //
            //  Create a list of rectangles to loop over.  The resulting list
            //  of rectangles is the area left by intersecting the ROI with
            //  the destination box.
            //
            XilRectList    rl(roi, dest_box);
	    
            int            x;
            int            y;
            unsigned int   xsize;
            unsigned int   ysize;
            while(rl.getNext(&x, &y, &xsize, &ysize)) {
                Xil_unsigned8* dst_scanline = dest_data +
                    y * dest_scanline_stride +
		    x * dest_pixel_stride;
                Xil_unsigned8* src_scanline = src1_data +
                    y * src1_scanline_stride +
		    x * src1_pixel_stride;
		//
		// Translate (x, y) to src image space for writemask and
		// weight image
		//
		fill_scanline = fill_base_addr +
		    (y + box_y) * fill_next_scan;
		float* weight_scanline = weight_base_addr +
		    (y + box_y) * weight_next_scan +
		    (x + box_x);

		//
		//  Each Scanline...
		//
		for (; ysize > 0; ysize--) {
		    Xil_unsigned8* dst_pixel = dst_scanline;
		    src_pixel = src_scanline;
		    weight_pixel = weight_scanline;
		    //
		    // Translate to image space
		    //
		    unsigned int fill_bit_offset = x + box_x;

		    //
		    //  Each Pixel...
		    //
		    for (unsigned int xx = xsize; xx > 0; xx--) {
			//
			// If writemask is set, then transfer fill color
			// to the destination image.
			//
			if (BFILL_MASK_ON(fill_scanline,
					  fill_bit_offset)) {
			    // Inside region, replace fg portion of
			    // color with new fill color.  New pixel =
			    // pixel-(weight*fgcolor)+(weight*fill_color)
			    Xil_unsigned8* dst_band = dst_pixel;
			    Xil_unsigned8* src_band = src_pixel;

			    for (unsigned int k = 0; k < nbands; k++) {	
				float tmp_band = _XILI_B2F(*src_band) -
				    (*weight_pixel * color_diff[k]);
				*dst_band = _XILI_ROUND_U8(tmp_band);
				src_band++;
				dst_band++;
			    }
			}

			//
			// Move to next pixel
			//
			dst_pixel += dest_pixel_stride;
			src_pixel += src1_pixel_stride;
			fill_bit_offset++;
			weight_pixel++;
		    }

		    //
		    // Move to next scanline
		    //
		    dst_scanline += dest_scanline_stride;
		    src_scanline += src1_scanline_stride;
		    fill_scanline += fill_next_scan;
		    weight_scanline += weight_next_scan;
                }
            }

	    //
	    // Must delete chain of alloc blocks following the next ptr
	    //
	    while (block != NULL) {
		ctrl->curr_block = block->next;		// save &next block
		delete block;				// free curr block
		block = ctrl->curr_block;		// curr = next
	    }
	    // Free other allocs
	    delete ctrl;
	} else {
	    //
	    // General storage case...
	    //

	    //
	    // Convert seed to location in writemask and weight image
	    //
	    Xil_unsigned8* fill_scanline = fill_base_addr +
		y_seed * fill_next_scan;
	    float* weight_pixel = weight_base_addr +
		y_seed * weight_next_scan +
		x_seed;

	    //
	    // Create and init sfill data struct
	    //
	    sfill8_data_t fill_data;
	    fill_data.xmax = x_size - 1;
	    fill_data.ymax = y_size - 1;
	    fill_data.bg = last_bgcolor;
	    fill_data.nbg = num_bg;
	    fill_data.nbands = nbands;
	    fill_data.mtx = mtx_invert;
	    fill_data.src_next_pixel = 0;   // not used
	    fill_data.src_next_scan = 0;    // not used
	    fill_data.fill_next_scan = fill_next_scan;
	    fill_data.weight_next_scan = weight_next_scan;

	    //
	    // Allocate 1st block of stack and stack control
	    //
	    sfill8_stack_ctrl_t *ctrl = new sfill8_stack_ctrl_t;
	    if(ctrl == NULL) {
		delete [] weight_base_addr;
		delete [] fill_base_addr;
		delete [] mtx_invert;
		delete [] color_diff;
		XIL_ERROR(err_state, XIL_ERROR_RESOURCE, "di-1", TRUE);
		return XIL_FAILURE;
	    }
	    sfill8_stack_block_t *block = new sfill8_stack_block_t;
	    if(block == NULL) {
		delete ctrl;
		delete [] weight_base_addr;
		delete [] fill_base_addr;
		delete [] mtx_invert;
		delete [] color_diff;
		XIL_ERROR(err_state, XIL_ERROR_RESOURCE, "di-1", TRUE);
		return XIL_FAILURE;
	    }
	    block->prev = NULL;		    // no previous block
	    block->next = NULL;		    // no next block
	    ctrl->curr_block = block;	    // current block
	    ctrl->curr_idx = 0;		    // current avail frame index
	    fill_data.stack_ctrl = ctrl;    // store stack control

	    //
	    // Create fill write mask bit image.  We use image space
	    // coordinates here.
	    //
	    if (SoftFill8G(x_seed, y_seed, &src1_storage, fill_scanline,
			   weight_pixel, &fill_data, err_state,
			   box_x, box_y) == XIL_FAILURE) {
		//
		// Failed during fill operation, attempt to clean up and
		// return.  Must delete chain of alloc blocks following
		// the next ptr
		//
		while (block != NULL) {
		    ctrl->curr_block = block->next;	// save &next block
		    delete block;			// free curr block
		    block = ctrl->curr_block;		// curr = next
		}
		// Free other allocs
		delete ctrl;
		delete [] weight_base_addr;
		delete [] fill_base_addr;
		delete [] mtx_invert;
		delete [] color_diff;
		// TODO: is XIL_ERROR() call need here like in Fill?
		return XIL_FAILURE;
	    }

            //
            //  Create a list of rectangles to loop over.  The resulting list
            //  of rectangles is the area left by intersecting the ROI with
            //  the destination box.
            //
            XilRectList    rl(roi, dest_box);
	    
            int            x;
            int            y;
            unsigned int   xsize;
            unsigned int   ysize;
            while(rl.getNext(&x, &y, &xsize, &ysize)) {
                //
                //  Each Band...
                //
                for(unsigned int band=0; band<nbands; band++) {
                    unsigned int   dest_pixel_stride;
                    unsigned int   dest_scanline_stride;
                    Xil_unsigned8* dest_data;
                    dest_storage.getStorageInfo(band,
                                                &dest_pixel_stride,
                                                &dest_scanline_stride,
                                                NULL,
                                                (void**)&dest_data);
                    unsigned int   src1_pixel_stride;
                    unsigned int   src1_scanline_stride;
                    Xil_unsigned8* src1_data;
                    src1_storage.getStorageInfo(band,
                                                &src1_pixel_stride,
                                                &src1_scanline_stride,
                                                NULL,
                                                (void**)&src1_data);
		    Xil_unsigned8* dst_scanline = dest_data +
			y * dest_scanline_stride +
			x * dest_pixel_stride;
		    Xil_unsigned8* src_scanline = src1_data +
			y * src1_scanline_stride +
			x * src1_pixel_stride;
		    //
		    // Translate (x, y) to src image space for writemask and
		    // weight image
		    //
		    fill_scanline = fill_base_addr +
			(y + box_y) * fill_next_scan;
		    float* weight_scanline = weight_base_addr +
			(y + box_y) * weight_next_scan +
			(x + box_x);

                    //
                    //  Each Scanline...
                    //
		    for (unsigned int yy = ysize; yy > 0; yy--) {
			Xil_unsigned8* dst_pixel = dst_scanline;
			Xil_unsigned8* src_pixel = src_scanline;
			weight_pixel = weight_scanline;
			//
			// Translate to image space
			//
			unsigned int fill_bit_offset = x + box_x;
			
                        //
                        //  Each Pixel...
                        //
			for(unsigned int xx = xsize; xx > 0; xx--) {
			    //
			    // If writemask is set, then transfer fill color
			    // to the destination image.
			    //
			    if (BFILL_MASK_ON(fill_scanline,
					      fill_bit_offset)) {
				// Inside region, replace fg portion of
				// color with new fill color.  New pixel =
				// pixel-(weight*fgcolor)+(weight*fill_color)
				float tmp_band = _XILI_B2F(*src_pixel) -
				    (*weight_pixel * color_diff[band]);
				*dst_pixel = _XILI_ROUND_U8(tmp_band);
			    }

			    //
			    // Move to next pixel
			    //
			    dst_pixel += dest_pixel_stride;
			    src_pixel += src1_pixel_stride;
			    fill_bit_offset++;
			    weight_pixel++;
			}

			//
			// Move to next scanline
			//
			dst_scanline += dest_scanline_stride;
			src_scanline += src1_scanline_stride;
			fill_scanline += fill_next_scan;
			weight_scanline += weight_next_scan;
		    }
		}
	    }

	    //
	    // Must delete chain of alloc blocks following the next ptr
	    //
	    while (block != NULL) {
		ctrl->curr_block = block->next;		// save &next block
		delete block;				// free curr block
		block = ctrl->curr_block;		// curr = next
	    }
	    // Free other allocs
	    delete ctrl;
        }
    }

    // Free remaining allocs
    delete [] weight_base_addr;
    delete [] fill_base_addr;
    delete [] mtx_invert;
    delete [] color_diff;

    return XIL_SUCCESS;
}


static int
SoftFill8(long x, long y, Xil_unsigned8* src_pixel,
	  Xil_unsigned8* fill_scan, float* weight_pixel,
	  sfill8_data_t* fill_ptr, XilSystemState* err_state)
{
    sfill8_stack_frame_t  curr_frame;
    long  lx;
    long rx;
    Xil_unsigned8	*left_src_pixel;
    float		*left_weight_pixel;
    Xil_boolean		inside;

    // check y within bounds
    if ((y <= fill_ptr->ymax) && (y >= 0)) {

	lx = x;
	left_src_pixel = src_pixel;
	left_weight_pixel = weight_pixel;

        // check seed pixel within region
	while ( (lx >= 0) && Sfill8PixelInside(left_src_pixel,
					       left_weight_pixel,
					       fill_ptr->bg,
					       fill_ptr->nbands,
					       fill_ptr->mtx,
					       fill_ptr->nbg,
					       fill_scan, lx) ) {
	    // set pixel mask
	    BFILL_MASK_SET(fill_scan, lx);

	    // continue left
	    lx = lx-1;
	    left_src_pixel = left_src_pixel - fill_ptr->src_next_pixel;
	    left_weight_pixel = left_weight_pixel - 1;

	}    // end while INSIDE go left
		
	// here to check right

        if (lx == x) {
 	    XIL_ERROR(err_state, XIL_ERROR_USER, "di-258", TRUE);
	    return XIL_FAILURE;		// seed value not within region
	}

	// Save leftmost x
	lx = lx + 1;
	left_src_pixel = left_src_pixel + fill_ptr->src_next_pixel;
	left_weight_pixel = left_weight_pixel + 1;
	    
	rx = x + 1;
	src_pixel = src_pixel + fill_ptr->src_next_pixel;
	weight_pixel = weight_pixel + 1;

	while ((rx <= fill_ptr->xmax) && Sfill8PixelInside(src_pixel,
							   weight_pixel,
							   fill_ptr->bg,
							   fill_ptr->nbands,
							   fill_ptr->mtx,
							   fill_ptr->nbg,
							   fill_scan, rx) ) {
	    // set pixel mask
	    BFILL_MASK_SET(fill_scan, rx);

	    // continue right
	    rx = rx+1;
	    src_pixel = src_pixel + fill_ptr->src_next_pixel;
	    weight_pixel = weight_pixel + 1;

	}    // end while INSIDE go right

	rx = rx -1;			// save rightmost x

    } else {
 	XIL_ERROR(err_state, XIL_ERROR_USER, "di-258", TRUE);
    	return XIL_FAILURE;		// not within y range
    }	


    // push initial stack frame above
    curr_frame.lx = lx;
    curr_frame.rx = rx;
    curr_frame.src_pixel = left_src_pixel + fill_ptr->src_next_scan;
    curr_frame.weight_pixel = left_weight_pixel + fill_ptr->weight_next_scan;
    curr_frame.fill_scan = fill_scan + fill_ptr->fill_next_scan;
    curr_frame.plx = lx-1;
    curr_frame.prx = rx+1;
    curr_frame.y = y+1;
    curr_frame.dir = 1;

    if (Sfill8Push(fill_ptr->stack_ctrl,&curr_frame)==XIL_FAILURE) {
	return(XIL_FAILURE);
    }

    // push initial stack frame below
    curr_frame.src_pixel = left_src_pixel - fill_ptr->src_next_scan;
    curr_frame.weight_pixel = left_weight_pixel - fill_ptr->weight_next_scan;
    curr_frame.fill_scan = fill_scan - fill_ptr->fill_next_scan;
    curr_frame.y = y-1;
    curr_frame.dir = -1;

    if (Sfill8Push(fill_ptr->stack_ctrl,&curr_frame)==XIL_FAILURE) {
	return(XIL_FAILURE);
    }

    // begin fill loop
    while (!(BFILL_STACK_EMPTY(fill_ptr->stack_ctrl))) {

	// pop stack frame to process
	if (Sfill8Pop(fill_ptr->stack_ctrl,&curr_frame)==XIL_FAILURE) {
	   return(XIL_FAILURE);
	}

	y = curr_frame.y;

        // check y within bounds
	if ((y <= fill_ptr->ymax) && (y >= 0)) {

	    x = curr_frame.lx;
	    fill_scan = curr_frame.fill_scan;
		
	    // here to check left
	    src_pixel = curr_frame.src_pixel;
	    weight_pixel = curr_frame.weight_pixel;
	    inside = Sfill8PixelInside(src_pixel, weight_pixel,
				       fill_ptr->bg, fill_ptr->nbands,
				       fill_ptr->mtx, fill_ptr->nbg,
				       fill_scan, x);
	    if (inside) {
		// set pixel mask
		BFILL_MASK_SET(fill_scan, x);
		// continue left
		x = x-1;
		src_pixel = src_pixel - fill_ptr->src_next_pixel;
		weight_pixel = weight_pixel - 1;

		while ((x >= 0) && Sfill8PixelInside(src_pixel,
						     weight_pixel,
						     fill_ptr->bg,
						     fill_ptr->nbands,
						     fill_ptr->mtx,
						     fill_ptr->nbg,
						     fill_scan, x) ) {
		    // set pixel mask
		    BFILL_MASK_SET(fill_scan, x);

		    // continue left
		    x = x-1;
		    src_pixel = src_pixel - fill_ptr->src_next_pixel;
		    weight_pixel = weight_pixel - 1;

		}    // end while INSIDE go left

		lx = x + 1;		// save leftmost x coord
		// Save leftmost x pixel
		left_src_pixel = src_pixel + fill_ptr->src_next_pixel;
		left_weight_pixel = weight_pixel + 1;
	    }    // end if (inside)

	    // here to check right

	    x = curr_frame.lx + 1;
	    src_pixel = curr_frame.src_pixel + fill_ptr->src_next_pixel;
	    weight_pixel = curr_frame.weight_pixel + 1;
	    while (x <= fill_ptr->xmax) {

		if (inside) {
		    if (Sfill8PixelInside(src_pixel, weight_pixel,
					  fill_ptr->bg,
					  fill_ptr->nbands,
					  fill_ptr->mtx, fill_ptr->nbg,
					  fill_scan, x)) {
			// still inside
			BFILL_MASK_SET(fill_scan, x);
		    } else {
			// no longer inside
			Sfill8Shadow(&curr_frame, fill_ptr, lx,
				     left_src_pixel, left_weight_pixel,
				     (x-1));
			inside = FALSE;
		    }
		} else {
		    if (x > curr_frame.rx) {
			// not inside, and past shadow right
			break;
		    }
		    if (Sfill8PixelInside(src_pixel, weight_pixel,
					  fill_ptr->bg,
					  fill_ptr->nbands,
					  fill_ptr->mtx, fill_ptr->nbg,
					  fill_scan, x)) {
			// now inside, new scan
			BFILL_MASK_SET(fill_scan, x);
			inside = TRUE;
			lx = x;		// new leftmost x
			left_src_pixel = src_pixel;
			left_weight_pixel = weight_pixel;
		    } else {
			// still not inside!, no action
		    }
		}
		x = x+1;
		src_pixel = src_pixel + fill_ptr->src_next_pixel;
		weight_pixel = weight_pixel + 1;

	    }    // end while within right limit

	    // here to check edge of region
	    if (inside) {
		// hit the edge of region while still inside scan
		Sfill8Shadow(&curr_frame, fill_ptr, lx, left_src_pixel,
			     left_weight_pixel, (x-1));
	    }
	}    // end if within y bounds
    }    // end while, done with fill loop, stack empty

    return XIL_SUCCESS;
}

static int
Sfill8Push(sfill8_stack_ctrl_t *ctrl_ptr, 
	   sfill8_stack_frame_t *local_frame_ptr)
{
    int	idx;

    if (BFILL_STACK_BLOCK_FULL(ctrl_ptr)) {    // curr block full
	if (BFILL_STACK_NO_NEXT_BLOCK(ctrl_ptr)) {   // no next block
	    // must allocate new stack block, curr block full
	    sfill8_stack_block_t *block = new sfill8_stack_block_t;
	    if (block == NULL) {
		return XIL_FAILURE;
	    }
	    ctrl_ptr->curr_block->next = block;	    // connect curr to new
	    block->prev = ctrl_ptr->curr_block;	    // connect new to prev
	    block->next = NULL;			    // mark new as EOL
            ctrl_ptr->curr_block = block;	    // update ctrl
	    ctrl_ptr->curr_idx = 0;		    // reset idx
	} else {
	    // next block already exists
	    ctrl_ptr->curr_block = ctrl_ptr->curr_block->next;  // update ctrl
	    ctrl_ptr->curr_idx = 0;	       		        // reset idx   
	}
    }
    idx = ctrl_ptr->curr_idx;
    ctrl_ptr->curr_block->frame[idx] = *local_frame_ptr;    // save frame
    ctrl_ptr->curr_idx += 1;		// incr curr=> next avail

    return XIL_SUCCESS;
}

static int
Sfill8Pop(sfill8_stack_ctrl_t *ctrl_ptr, 
	  sfill8_stack_frame_t *local_frame_ptr)
{
    int	idx;

    if (BFILL_STACK_BLOCK_EMPTY(ctrl_ptr)) {	// popped last frame of block
        if (BFILL_STACK_NO_PREV_BLOCK(ctrl_ptr)) {    // no prev block
	    return XIL_FAILURE;
	} else {
	   // use prev block
	   ctrl_ptr->curr_block = ctrl_ptr->curr_block->prev;	// update ctrl
	   ctrl_ptr->curr_idx = XILI_FILL_MAX_STACK_FRAMES;	// reset idx
	}
    }
    idx = ctrl_ptr->curr_idx-1;
    *local_frame_ptr = ctrl_ptr->curr_block->frame[idx];    // restore frame 
    ctrl_ptr->curr_idx = idx;			// decr counter
    return XIL_SUCCESS;
}

static Xil_boolean
Sfill8PixelInside(Xil_unsigned8* pixel,
		  float* weight_pixel,
		  Xil_unsigned8* bg,
		  unsigned int nbands,
		  float* mtx,
		  unsigned int nbg,
		  Xil_unsigned8* scan,
		  long offset)
{
    unsigned int i;
    unsigned int j;
    float fraction[256];
    Xil_unsigned8 differ[256];
    float* frac;
    Xil_unsigned8* diff;

    frac = &fraction[0];
    diff = &differ[0];
    if ( !(BFILL_MASK_ON(scan, offset)) ) {

	// create pixel - basis_n
        for (i=0;i<nbands;i++) {
	    // pixel[band_i] - basis_n[band_i]
	    *diff++ = *pixel++ - *bg++;
	}
        diff -= nbands;

        for (i=0;i<nbg;i++) {
             *frac++ = *diff * *mtx++;	// init vector row0 product
        }

        for (j=0;j<(nbands-1);j++) {
            frac -= nbg;			// backup to fraction_00
            diff++;				// incr to diff_0j

            for (i=0;i<nbg;i++) {
                *frac++ += *diff * *mtx++;	// accum vector row1--n products
            }
        }
    
        // fraction_00 = percentage of foreground color in pixel
        frac -= nbg;			// backup to fraction_00
        *weight_pixel = *frac;		// save fgcolor "weight"in case inside
        if (*frac > XILI_SOFTFILL_THRESHOLD) {
	    return TRUE;		// fgcolor present, inside region
        }
        else {
	    return FALSE;		// fgcolor not present, outside region
        }
    } else {
	return FALSE;	// prev filled pixel
    }
}

static void
Sfill8Shadow(sfill8_stack_frame_t *frame,
	     sfill8_data_t	 *fill_ptr,
	     long		 lx,
	     Xil_unsigned8	 *pixel,
	     float	 	 *weight_pixel,
	     long		 rx)
				
{
    sfill8_stack_frame_t	new_frame;
    long			newy;
    Xil_unsigned8		*new_pixel;
    float			*new_weight_pixel;
    Xil_unsigned8		*new_fill_scan;
    long			newlx;
    long			newrx;
    Xil_signed16		dir;

    // create 3 shadow spans
    //	  1. span(curr.lx,curr.rx)	leftmost x <-> rightmost x
    // 	  2. span(curr.prx+1,rx)       	parent rightmost x + 1 <-> rightmost x
    // 	  3. span(curr.lx,curr.plx-1)	leftmost x <-> parent leftmost x - 1

    // Always create (1).

    dir = frame->dir;
    newy = frame->y + dir;
    new_pixel = pixel + (dir * fill_ptr->src_next_scan);
    new_weight_pixel = weight_pixel + (dir * fill_ptr->weight_next_scan);
    new_fill_scan = frame->fill_scan + (dir * fill_ptr->fill_next_scan);

    new_frame.lx = lx;
    new_frame.rx = rx;
    new_frame.src_pixel = new_pixel;
    new_frame.weight_pixel = new_weight_pixel;
    new_frame.fill_scan = new_fill_scan;
    new_frame.plx = lx - 1;
    new_frame.prx = rx + 1;
    new_frame.y = newy;
    new_frame.dir = dir;

    Sfill8Push(fill_ptr->stack_ctrl, &new_frame);

    // Only create (2) if current span extends beyond parent rx 
    if (rx > frame->prx) {
	newlx = frame->prx+1;
	newy = frame->y - dir;
	new_pixel = pixel + (newlx - lx)*fill_ptr->src_next_pixel -
	    (dir * fill_ptr->src_next_scan);
	new_weight_pixel = weight_pixel +
	    (newlx - lx) - (dir * fill_ptr->weight_next_scan);
        new_fill_scan = frame->fill_scan - (dir * fill_ptr->fill_next_scan);
	
        new_frame.lx = newlx;
    	new_frame.src_pixel = new_pixel;
    	new_frame.weight_pixel = new_weight_pixel;
    	new_frame.fill_scan = new_fill_scan;
    	new_frame.y = newy;
	new_frame.dir = -dir;
    	Sfill8Push(fill_ptr->stack_ctrl, &new_frame);
    }
    
    // Only create (3) if current span begins before parent lx 
    if (lx < frame->plx) {
	newrx = frame->plx-1;
	newy = frame->y - dir;
	new_pixel = pixel - (dir * fill_ptr->src_next_scan);
	new_weight_pixel = weight_pixel - (dir * fill_ptr->weight_next_scan);
        new_fill_scan = frame->fill_scan - (dir * fill_ptr->fill_next_scan);
	
        new_frame.lx = lx;
        new_frame.rx = newrx;
    	new_frame.src_pixel = new_pixel;
    	new_frame.weight_pixel = new_weight_pixel;
    	new_frame.fill_scan = new_fill_scan;
    	new_frame.y = newy;
	new_frame.dir = -dir;

    	Sfill8Push(fill_ptr->stack_ctrl, &new_frame);
    }
}


//
// Perform a soft fill and place results in a bitmap and a weight image.
// This is for the General storage case.
//
static int
SoftFill8G(long x, long y, XilStorage* src_storage,
	   Xil_unsigned8* fill_scan, float* weight_pixel,
	   sfill8_data_t* fill_ptr, XilSystemState* err_state,
	   int box_x, int box_y)
{
    sfill8_stack_frame_t  curr_frame;
    long  lx;
    long rx;
    float		*left_weight_pixel;
    Xil_boolean		inside;

    // check y within bounds
    if ((y <= fill_ptr->ymax) && (y >= 0)) {

	lx = x;
	left_weight_pixel = weight_pixel;

        // check seed pixel within region
	while ( (lx >= 0) && Sfill8PixelInsideG(src_storage,
						left_weight_pixel,
						fill_ptr->bg,
						fill_ptr->nbands,
						fill_ptr->mtx,
						fill_ptr->nbg,
						fill_scan, lx, y,
						box_x, box_y) ) {
	    // set pixel mask
	    BFILL_MASK_SET(fill_scan, lx);

	    // continue left
	    lx = lx-1;
	    left_weight_pixel = left_weight_pixel - 1;

	}    // end while INSIDE go left
		
	// here to check right

        if (lx == x) {
 	    XIL_ERROR(err_state, XIL_ERROR_USER, "di-258", TRUE);
	    return XIL_FAILURE;		// seed value not within region
	}

	// Save leftmost x
	lx = lx + 1;
	left_weight_pixel = left_weight_pixel + 1;
	    
	rx = x + 1;
	weight_pixel = weight_pixel + 1;

	while ((rx <= fill_ptr->xmax) && Sfill8PixelInsideG(src_storage,
							    weight_pixel,
							    fill_ptr->bg,
							    fill_ptr->nbands,
							    fill_ptr->mtx,
							    fill_ptr->nbg,
							    fill_scan,
							    rx, y,
							    box_x, box_y) ) {
	    // set pixel mask
	    BFILL_MASK_SET(fill_scan, rx);

	    // continue right
	    rx = rx+1;
	    weight_pixel = weight_pixel + 1;

	}    // end while INSIDE go right

	rx = rx -1;			// save rightmost x

    } else {
 	XIL_ERROR(err_state, XIL_ERROR_USER, "di-258", TRUE);
    	return XIL_FAILURE;		// not within y range
    }	


    // push initial stack frame above
    curr_frame.lx = lx;
    curr_frame.rx = rx;
    curr_frame.weight_pixel = left_weight_pixel + fill_ptr->weight_next_scan;
    curr_frame.fill_scan = fill_scan + fill_ptr->fill_next_scan;
    curr_frame.plx = lx-1;
    curr_frame.prx = rx+1;
    curr_frame.y = y+1;
    curr_frame.dir = 1;

    if (Sfill8Push(fill_ptr->stack_ctrl,&curr_frame)==XIL_FAILURE) {
	return(XIL_FAILURE);
    }

    // push initial stack frame below
    curr_frame.weight_pixel = left_weight_pixel - fill_ptr->weight_next_scan;
    curr_frame.fill_scan = fill_scan - fill_ptr->fill_next_scan;
    curr_frame.y = y-1;
    curr_frame.dir = -1;

    if (Sfill8Push(fill_ptr->stack_ctrl,&curr_frame)==XIL_FAILURE) {
	return(XIL_FAILURE);
    }

    // begin fill loop
    while (!(BFILL_STACK_EMPTY(fill_ptr->stack_ctrl))) {

	// pop stack frame to process
	if (Sfill8Pop(fill_ptr->stack_ctrl,&curr_frame)==XIL_FAILURE) {
	   return(XIL_FAILURE);
	}

	y = curr_frame.y;

        // check y within bounds
	if ((y <= fill_ptr->ymax) && (y >= 0)) {

	    x = curr_frame.lx;
	    fill_scan = curr_frame.fill_scan;
		
	    // here to check left
	    weight_pixel = curr_frame.weight_pixel;
	    inside = Sfill8PixelInsideG(src_storage, weight_pixel,
					fill_ptr->bg, fill_ptr->nbands,
					fill_ptr->mtx, fill_ptr->nbg,
					fill_scan, x, y, box_x, box_y);
	    if (inside) {
		// set pixel mask
		BFILL_MASK_SET(fill_scan, x);
		// continue left
		x = x-1;
		weight_pixel = weight_pixel - 1;

		while ((x >= 0) && Sfill8PixelInsideG(src_storage,
						      weight_pixel,
						      fill_ptr->bg,
						      fill_ptr->nbands,
						      fill_ptr->mtx,
						      fill_ptr->nbg,
						      fill_scan, x, y,
						      box_x, box_y) ) {
		    // set pixel mask
		    BFILL_MASK_SET(fill_scan, x);

		    // continue left
		    x = x-1;
		    weight_pixel = weight_pixel - 1;

		}    // end while INSIDE go left

		lx = x + 1;		// save leftmost x coord
		// Save leftmost x pixel
		left_weight_pixel = weight_pixel + 1;
	    }    // end if (inside)

	    // here to check right

	    x = curr_frame.lx + 1;
	    weight_pixel = curr_frame.weight_pixel + 1;
	    while (x <= fill_ptr->xmax) {

		if (inside) {
		    if (Sfill8PixelInsideG(src_storage, weight_pixel,
					   fill_ptr->bg,
					   fill_ptr->nbands,
					   fill_ptr->mtx, fill_ptr->nbg,
					   fill_scan, x, y, box_x, box_y)) {
			// still inside
			BFILL_MASK_SET(fill_scan, x);
		    } else {
			// no longer inside
			Sfill8Shadow(&curr_frame, fill_ptr, lx,
				     NULL, left_weight_pixel,
				     (x-1));
			inside = FALSE;
		    }
		} else {
		    if (x > curr_frame.rx) {
			// not inside, and past shadow right
			break;
		    }
		    if (Sfill8PixelInsideG(src_storage, weight_pixel,
					   fill_ptr->bg,
					   fill_ptr->nbands,
					   fill_ptr->mtx, fill_ptr->nbg,
					   fill_scan, x, y, box_x, box_y)) {
			// now inside, new scan
			BFILL_MASK_SET(fill_scan, x);
			inside = TRUE;
			lx = x;		// new leftmost x
			left_weight_pixel = weight_pixel;
		    } else {
			// still not inside!, no action
		    }
		}
		x = x+1;
		weight_pixel = weight_pixel + 1;

	    }    // end while within right limit

	    // here to check edge of region
	    if (inside) {
		// hit the edge of region while still inside scan
		Sfill8Shadow(&curr_frame, fill_ptr, lx, NULL,
			     left_weight_pixel, (x-1));
	    }
	}    // end if within y bounds
    }    // end while, done with fill loop, stack empty

    return XIL_SUCCESS;
}

static Xil_boolean
Sfill8PixelInsideG(XilStorage* storage,
		   float* weight_pixel,
		   Xil_unsigned8* bg,
		   unsigned int nbands,
		   float* mtx,
		   unsigned int nbg,
		   Xil_unsigned8* scan,
		   unsigned int x,
		   unsigned int y,
		   int box_x,
		   int box_y)
{
    unsigned int i;
    unsigned int j;
    float fraction[256];
    Xil_unsigned8 differ[256];
    float* frac;
    Xil_unsigned8* diff;

    frac = &fraction[0];
    diff = &differ[0];
    if ( !(BFILL_MASK_ON(scan, x)) ) {
	// create pixel - basis_n
	for (unsigned int b = 0; b < nbands; b++) {
	    //
	    // Compute pixel location.  Convert image space to box space
	    // coordinates.
	    //
	    unsigned int pixel_stride;
	    unsigned int scanline_stride;
	    Xil_unsigned8* data;
	    storage->getStorageInfo(b,
				    &pixel_stride,
				    &scanline_stride,
				    NULL,
				    (void**) &data);
	    Xil_unsigned8* pixel = data + (y - box_y) * scanline_stride +
		(x - box_x) * pixel_stride;

	    // pixel[band_i] - basis_n[band_i]
	    *diff = *pixel - *bg;

	    // Next band
	    diff++;
	    bg++;
	}
        diff -= nbands;

        for (i=0;i<nbg;i++) {
             *frac++ = *diff * *mtx++;	// init vector row0 product
        }

        for (j=0;j<(nbands-1);j++) {
            frac -= nbg;			// backup to fraction_00
            diff++;				// incr to diff_0j

            for (i=0;i<nbg;i++) {
                *frac++ += *diff * *mtx++;	// accum vector row1--n products
            }
        }
    
        // fraction_00 = percentage of foreground color in pixel
        frac -= nbg;			// backup to fraction_00
        *weight_pixel = *frac;		// save fgcolor "weight"in case inside
        if (*frac > XILI_SOFTFILL_THRESHOLD) {
	    return TRUE;		// fgcolor present, inside region
        }
        else {
	    return FALSE;		// fgcolor not present, outside region
        }
    } else {
	return FALSE;	// prev filled pixel
    }
}
