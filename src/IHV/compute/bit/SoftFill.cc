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
//  Revision:	1.3
//  Last Mod:	10:09:42, 03/10/00
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
#pragma ident	"@(#)SoftFill.cc	1.3\t00/03/10  "

#include "XiliUtils.hh"
#include "XilDeviceManagerComputeBIT.hh"
#include "xili_fill_utils.hh"
#include "xili_svd.hh"


//
// Data structure declarations
//
typedef struct {
    long  lx;				// curr leftmost x
    long  rx;				// curr rightmost x
    Xil_unsigned8	*src_scan;	// curr scanline in image
    float		*weight_pixel;	// curr pixel in weight image
    Xil_unsigned8	*fill_scan;	// curr scanline in writemask
    long  plx;				// parent leftx
    long  prx;				// parent rightx
    long  y;				// curr y
    Xil_signed16	dir;		// current direction (+1,-1)
} sfill1_stack_frame_t; 

struct sfill1_stack_block {
    struct sfill1_stack_block	*prev;
    struct sfill1_stack_block	*next;
    sfill1_stack_frame_t	frame[XILI_FILL_MAX_STACK_FRAMES];
};
typedef struct sfill1_stack_block sfill1_stack_block_t;

struct sfill1_stack_ctrl {
    struct sfill1_stack_block	*curr_block;
    int				curr_idx;
};
typedef struct sfill1_stack_ctrl sfill1_stack_ctrl_t;

typedef struct {
    long 	xmax;
    long 	ymax;
    Xil_unsigned8	*bg;
    unsigned int	nbg;
    unsigned int	nbands;
    float	*mtx;
    unsigned long	src_next_band;
    long		src_next_scan;
    long		src_offset;
    long		fill_next_scan;
    long		weight_next_scan;
    struct sfill1_stack_ctrl	*stack_ctrl;
} sfill1_data_t;


//
// Forward function declarations
//
static int
SoftFill1(long x, long y, Xil_unsigned8 *src_scan,
	  Xil_unsigned8 *fill_scan, float *weight_pixel,
	  sfill1_data_t *fill_ptr, XilSystemState* err_state);

static int
Sfill1Push(sfill1_stack_ctrl_t *ctrl_ptr, 
	   sfill1_stack_frame_t *local_frame_ptr);

static int
Sfill1Pop(sfill1_stack_ctrl_t *ctrl_ptr, 
	  sfill1_stack_frame_t *local_frame_ptr);

static Xil_boolean
Sfill1PixelInside(Xil_unsigned8 *src_scan,
		  float	*weight_pixel,
		  long offset,
		  Xil_unsigned8 *fill_scan,
		  sfill1_data_t *fill_ptr);

static void
Sfill1Shadow(sfill1_stack_frame_t *frame,
	     sfill1_data_t	 *fill_ptr,
	     long		 lx,
	     float	 	 *weight_pixel,
	     long		 rx);

static Xil_boolean
Sfill1PixelInsideG(XilStorage* storage,
		   unsigned int x,
		   unsigned int y,
		   float* weight_pixel,
		   Xil_unsigned8* fill_scan,
		   sfill1_data_t* fill_ptr,
		   int box_x,
		   int box_y);

static int
SoftFill1G(long x, long y, XilStorage* storage,
	   Xil_unsigned8 *fill_scan, float *weight_pixel,
	   sfill1_data_t *fill_ptr, XilSystemState* err_state,
	   int box_x, int box_y);


//
// Perform a Soft Fill operation.
//
// The band sequential case has no suffix, the corresponding general
// storage case have functions that end in "G".
//
XilStatus
XilDeviceManagerComputeBIT::SoftFill(XilOp*       op,
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
    // Get params from the op.
    //
    // For color values, each band in color occupies an Xil_unsigned8
    // and its value is either 0 or 1.
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

    float *mtx = new float[nbands*num_bg];
    if(mtx == NULL) {
	XIL_ERROR(err_state, XIL_ERROR_RESOURCE, "di-1", TRUE);
        return XIL_FAILURE;
    }

    // Last basis color, Basis[n]
    Xil_unsigned8* last_bgcolor = &bgcolor[nbands*(num_bg-1)];

    for (int i = 0; i < nbands; i++) {
        mtx[i] = fgcolor[i] - last_bgcolor[i];	// first row Basis[0]-Basis[n]
    }

    // Compute remaining Basis[i] - Basis[n] if any
    for (int j = 0; j < num_bg - 1; j++) {
        for (i = 0; i < nbands; i++) {
	    mtx[((j+1)*nbands)+i] = bgcolor[(j*nbands)+i] - last_bgcolor[i];
        }
    }

    // Invert the matrix
    float *mtx_invert = new float[nbands*num_bg];
    if(mtx_invert == NULL) {
	delete [] mtx;
	XIL_ERROR(err_state, XIL_ERROR_RESOURCE, "di-1", TRUE);
        return XIL_FAILURE;
    }
    svinvrt(nbands, num_bg, mtx, mtx_invert);
    delete [] mtx;

    //
    // Create and init a bitmap to store writemask for fill color
    //
    unsigned int fill_next_scan = (x_size + 7) / 8;
    Xil_unsigned8* fill_base_addr = new Xil_unsigned8[fill_next_scan*y_size];
    if(fill_base_addr == NULL) {
	delete [] mtx_invert;
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
        //  Test to see if all of our storage is of type XIL_BAND_SEQUENTIAL.
        //  If so, implement an loop optimized for band-sequential storage.
        //
        if((src1_storage.isType(XIL_BAND_SEQUENTIAL)) &&
	   (dest_storage.isType(XIL_BAND_SEQUENTIAL))) {
            unsigned int src1_scanline_stride;
            unsigned int src1_band_stride;
            unsigned int src1_offset;
            Xil_unsigned8* src1_data;
            src1_storage.getStorageInfo((unsigned int*)NULL,
                                        &src1_scanline_stride,
                                        &src1_band_stride,
					&src1_offset,
                                        (void**)&src1_data);
            
            unsigned int dest_scanline_stride;
            unsigned int dest_band_stride;
            unsigned int dest_offset;
            Xil_unsigned8* dest_data;
            dest_storage.getStorageInfo((unsigned int*)NULL,
                                        &dest_scanline_stride,
                                        &dest_band_stride,
					&dest_offset,
                                        (void**)&dest_data);

	    //
	    // Find y_seed scanline in src.  "src_scanline" will be at
	    // the beginning of the scanline in image space not box
	    // space, and "src_offset" will be its offset value.
	    //
	    Xil_unsigned8* src_scanline;
	    int src_offset = src1_offset - box_x % 8;
	    if (src_offset >= 0) {
		src_scanline = src1_data +
		    (y_seed - box_y) * src1_scanline_stride -
		    box_x / 8;
	    } else {
		// Borrow 8 bits from the scanline
		src_scanline = src1_data +
		    (y_seed - box_y) * src1_scanline_stride -
		    box_x / 8 - 1;
		src_offset += 8;
	    }

	    //
	    // Find y_seed scanline in writemask
	    //
	    Xil_unsigned8* fill_scanline = fill_base_addr +
		y_seed * fill_next_scan;

	    //
	    // Find seed location in weight image
	    //
	    float* weight_pixel = weight_base_addr +
		y_seed * weight_next_scan +
		x_seed;

	    //
	    // Create and init sfill data struct
	    //
	    sfill1_data_t fill_data;
	    fill_data.xmax = x_size - 1;
	    fill_data.ymax = y_size - 1;
	    fill_data.bg = last_bgcolor;
	    fill_data.nbg = num_bg;
	    fill_data.nbands = nbands;
	    fill_data.mtx = mtx_invert;
	    // The src band and scanline stride is the same in image
	    // space as in box space.
	    fill_data.src_next_band = src1_band_stride;
	    fill_data.src_next_scan = src1_scanline_stride;
	    // Use the new computed image space src offset here
	    fill_data.src_offset = src_offset;
	    fill_data.fill_next_scan = fill_next_scan;
	    fill_data.weight_next_scan = weight_next_scan;

	    //
	    // Allocate 1st block of stack and stack control
	    //
	    sfill1_stack_ctrl_t *ctrl = new sfill1_stack_ctrl_t;
	    if(ctrl == NULL) {
		delete [] weight_base_addr;
		delete [] fill_base_addr;
		delete [] mtx_invert;
		XIL_ERROR(err_state, XIL_ERROR_RESOURCE, "di-1", TRUE);
		return XIL_FAILURE;
	    }
	    sfill1_stack_block_t *block = new sfill1_stack_block_t;
	    if(block == NULL) {
		delete ctrl;
		delete [] weight_base_addr;
		delete [] fill_base_addr;
		delete [] mtx_invert;
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
	    if (SoftFill1(x_seed, y_seed, src_scanline, fill_scanline,
			  weight_pixel, &fill_data, err_state)
		== XIL_FAILURE) {
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
		// Here both src_scanline and dst_scanline are in box space
		//
                Xil_unsigned8* dst_scanline = dest_data +
                    y * dest_scanline_stride;
                src_scanline = src1_data +
                    y * src1_scanline_stride;
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
		// For each scanline...
		//
		for (; ysize > 0; ysize--) {
		    int dst_offset = x + dest_offset;
		    src_offset = x + src1_offset;
		    int fill_offset = x + box_x;
		    weight_pixel = weight_scanline;

		    //
		    // For each pixel in a scanline...
		    //
		    for (i = xsize; i > 0; i--) {
			//
			// If writemask is set, then transfer fill color
			// to the destination image.
			//
			if (BFILL_MASK_ON(fill_scanline, fill_offset)) {
			    // Inside region, replace fg portion of
			    // color with new fill color.  New pixel =
			    // pixel-(weight*fgcolor)+(weight*fill_color)
			    Xil_unsigned8* dst_band = dst_scanline;
			    Xil_unsigned8* src_band = src_scanline;
			    for (int k = 0; k < nbands; k++) {
				float tmp_band = (float)
				    XIL_BMAP_TST(src_band, src_offset) -
				    (*weight_pixel * fgcolor[k]) + 
				    (*weight_pixel * fill_color[k]); 
				if (_XILI_ROUND_1(tmp_band) == 0) {
				    XIL_BMAP_CLR(dst_band, dst_offset);
				} else {
				    XIL_BMAP_SET(dst_band, dst_offset);
				}

				// Move to next band
				dst_band += dest_band_stride;
				src_band += src1_band_stride;
			    }
			}

			// Move to next pixel
			dst_offset++;
			src_offset++;
			fill_offset++;
			weight_pixel++;
		    }

		    // Move to next scanline
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
	    // Note: the src_scan in sfill1_stack_frame_t and
	    // src_next_scan, src_next_band, and src_offset slots in
	    // sfill1_data_t are not used for this case.

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
	    sfill1_data_t fill_data;
	    fill_data.xmax = x_size - 1;
	    fill_data.ymax = y_size - 1;
	    fill_data.bg = last_bgcolor;
	    fill_data.nbg = num_bg;
	    fill_data.nbands = nbands;
	    fill_data.mtx = mtx_invert;
	    fill_data.src_next_band = 0;    // not used
	    fill_data.src_next_scan = 0;    // not used
	    fill_data.src_offset = 0;	    // not used
	    fill_data.fill_next_scan = fill_next_scan;
	    fill_data.weight_next_scan = weight_next_scan;

	    //
	    // Allocate 1st block of stack and stack control
	    //
	    sfill1_stack_ctrl_t *ctrl = new sfill1_stack_ctrl_t;
	    if(ctrl == NULL) {
		delete [] weight_base_addr;
		delete [] fill_base_addr;
		delete [] mtx_invert;
		XIL_ERROR(err_state, XIL_ERROR_RESOURCE, "di-1", TRUE);
		return XIL_FAILURE;
	    }
	    sfill1_stack_block_t *block = new sfill1_stack_block_t;
	    if(block == NULL) {
		delete ctrl;
		delete [] weight_base_addr;
		delete [] fill_base_addr;
		delete [] mtx_invert;
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
	    if (SoftFill1G(x_seed, y_seed, &src1_storage, fill_scanline,
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
                    unsigned int dest_scanline_stride;
		    unsigned int dest_offset;
                    Xil_unsigned8* dest_data;
                    dest_storage.getStorageInfo(band,
                                                NULL,
                                                &dest_scanline_stride,
                                                &dest_offset,
                                                (void**)&dest_data);
                    unsigned int src1_scanline_stride;
                    unsigned int src1_offset;
                    Xil_unsigned8* src1_data;
                    src1_storage.getStorageInfo(band,
                                                NULL,
                                                &src1_scanline_stride,
                                                &src1_offset,
                                                (void**)&src1_data);

		    //
		    // Here both src_scanline and dst_scanline are in
		    // box space
		    //
		    Xil_unsigned8* dst_scanline = dest_data +
			y * dest_scanline_stride;
		    Xil_unsigned8* src_scanline = src1_data +
			y * src1_scanline_stride;

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
			int dst_offset = x + dest_offset;
			int src_offset = x + src1_offset;
			int fill_offset = x + box_x;
			weight_pixel = weight_scanline;

			//
			// For each pixel in a scanline...
			//
			for (i = xsize; i > 0; i--) {
			    //
			    // If writemask is set, then transfer fill color
			    // to the destination image.
			    //
			    if (BFILL_MASK_ON(fill_scanline, fill_offset)) {
				// Inside region, replace fg portion of
				// color with new fill color.  New pixel =
				// pixel-(weight*fgcolor)+(weight*fill_color)
				float tmp_band = (float)
				    XIL_BMAP_TST(src_scanline, src_offset) -
				    (*weight_pixel * fgcolor[band]) + 
				    (*weight_pixel * fill_color[band]); 
				if (_XILI_ROUND_1(tmp_band) == 0) {
				    XIL_BMAP_CLR(dst_scanline, dst_offset);
				} else {
				    XIL_BMAP_SET(dst_scanline, dst_offset);
				}
			    }

			    // Move to next pixel
			    dst_offset++;
			    src_offset++;
			    fill_offset++;
			    weight_pixel++;
			}

			// Move to next scanline
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

    return XIL_SUCCESS;
}

static int
SoftFill1(long x, long y, Xil_unsigned8 *src_scan,
	  Xil_unsigned8 *fill_scan, float *weight_pixel,
	  sfill1_data_t *fill_ptr, XilSystemState* err_state)
{
    sfill1_stack_frame_t  curr_frame;
    long lx;
    long rx;
    float		*left_weight_pixel;
    Xil_boolean		inside;

    // check y within bounds
    if ((y <= fill_ptr->ymax) && (y >= 0)) {

	lx = x;
	left_weight_pixel = weight_pixel;

        // check seed pixel within region
	while ( (lx >= 0) && Sfill1PixelInside(src_scan,left_weight_pixel,lx,fill_scan,fill_ptr) ) {
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

	while ((rx <= fill_ptr->xmax) && Sfill1PixelInside(src_scan,weight_pixel,rx,fill_scan,fill_ptr) ) {
	    // set pixel mask
	    BFILL_MASK_SET(fill_scan, rx);

	    // continue right
	    rx = rx+1;
	    weight_pixel = weight_pixel + 1;

	}    // end while INSIDE go right

	rx = rx -1;				// save rightmost x

    } else {
 	XIL_ERROR(err_state, XIL_ERROR_USER, "di-258", TRUE);
    	return XIL_FAILURE;		// not within y range
    }	


    // push initial stack frame above
    curr_frame.lx = lx;
    curr_frame.rx = rx;
    curr_frame.src_scan = src_scan + fill_ptr->src_next_scan;
    curr_frame.weight_pixel = left_weight_pixel + fill_ptr->weight_next_scan;
    curr_frame.fill_scan = fill_scan + fill_ptr->fill_next_scan;
    curr_frame.plx = lx-1;
    curr_frame.prx = rx+1;
    curr_frame.y = y+1;
    curr_frame.dir = 1;

    if (Sfill1Push(fill_ptr->stack_ctrl,&curr_frame)==XIL_FAILURE) {
	return(XIL_FAILURE);
    }

    // push initial stack frame below
    curr_frame.src_scan = src_scan - fill_ptr->src_next_scan;
    curr_frame.weight_pixel = left_weight_pixel - fill_ptr->weight_next_scan;
    curr_frame.fill_scan = fill_scan - fill_ptr->fill_next_scan;
    curr_frame.y = y-1;
    curr_frame.dir = -1;

    if (Sfill1Push(fill_ptr->stack_ctrl,&curr_frame)==XIL_FAILURE) {
	return(XIL_FAILURE);
    }

    // begin fill loop
    while (!(BFILL_STACK_EMPTY(fill_ptr->stack_ctrl))) {

	// pop stack frame to process
	if (Sfill1Pop(fill_ptr->stack_ctrl,&curr_frame)==XIL_FAILURE) {
	   return(XIL_FAILURE);
	}

	y = curr_frame.y;

        // check y within bounds
	if ((y <= fill_ptr->ymax) && (y >= 0)) {

	    x = curr_frame.lx;
	    fill_scan = curr_frame.fill_scan;
		
		// here to check left
		src_scan = curr_frame.src_scan;
		weight_pixel = curr_frame.weight_pixel;
		inside = Sfill1PixelInside(src_scan,weight_pixel, x,fill_scan,fill_ptr);
		if (inside) {
		    // set pixel mask
	    	    BFILL_MASK_SET(fill_scan, x);
		    // continue left
		    x = x-1;
		    weight_pixel = weight_pixel - 1;

		    while ((x >= 0) && Sfill1PixelInside(src_scan, weight_pixel,x,fill_scan,fill_ptr) ) {
		        // set pixel mask
	    	        BFILL_MASK_SET(fill_scan, x);

		        // continue left
		        x = x-1;
		        weight_pixel = weight_pixel - 1;

		    }    // end while INSIDE go left

		    lx = x + 1;			// save leftmost x coord
		    left_weight_pixel = weight_pixel + 1;    // save leftmost x pixel
		}    // end if (inside)

		// here to check right

		x = curr_frame.lx + 1;
		weight_pixel = curr_frame.weight_pixel + 1;
		while (x <= fill_ptr->xmax) {

		    if (inside) {
			if (Sfill1PixelInside(src_scan,weight_pixel,x,fill_scan,fill_ptr)) {
  			    // still inside
			    BFILL_MASK_SET(fill_scan, x);
			}
			else {
			    // no longer inside
			    Sfill1Shadow(&curr_frame,fill_ptr,lx,left_weight_pixel,(x-1));
			    inside = FALSE;
			}
		    }
		    else {
			if (x > curr_frame.rx) {
			    // not inside, and past shadow right
			    break;
			}
			if (Sfill1PixelInside(src_scan,weight_pixel,x,fill_scan,fill_ptr)) {
			    // now inside, new scan
			    BFILL_MASK_SET(fill_scan, x);
			    inside = TRUE;
			    lx = x;	// new leftmost x
			    left_weight_pixel = weight_pixel;
			}
			else {
			    // still not inside!, no action
			}
		    }
		    x = x+1;
		    weight_pixel = weight_pixel + 1;

	  	}    // end while within right limit

		// here to check edge of region
		if (inside) {
		    // hit the edge of region while still inside scan
		    Sfill1Shadow(&curr_frame,fill_ptr,lx,left_weight_pixel,(x-1));
		}


	}    // end if within y bounds

    }    // end while, done with fill loop, stack empty

    return(XIL_SUCCESS);
}

static int
Sfill1Push(sfill1_stack_ctrl_t *ctrl_ptr, 
	   sfill1_stack_frame_t *local_frame_ptr)
{
    int	idx;

    if (BFILL_STACK_BLOCK_FULL(ctrl_ptr)) {    // curr block full
	if (BFILL_STACK_NO_NEXT_BLOCK(ctrl_ptr)) {   // no next block
	    // must allocate new stack block, curr block full
	    sfill1_stack_block_t *block = new sfill1_stack_block_t;
	    if (block == NULL) {
		return(XIL_FAILURE);
	    }
	    ctrl_ptr->curr_block->next = block;   // connect curr to new
	    block->prev = ctrl_ptr->curr_block;    // connect new to prev
	    block->next = NULL;          // mark new as EOL
            ctrl_ptr->curr_block = block;      // update ctrl
	    ctrl_ptr->curr_idx = 0;	       // reset idx
	}
	else {
	    // next block already exists
	    ctrl_ptr->curr_block = ctrl_ptr->curr_block->next;  // update ctrl
	    ctrl_ptr->curr_idx = 0;	       		        // reset idx   
	}
    }
    idx = ctrl_ptr->curr_idx;
    ctrl_ptr->curr_block->frame[idx] = *local_frame_ptr;    // save frame
    ctrl_ptr->curr_idx += 1;		// incr curr=> next avail

    return(XIL_SUCCESS);
}

static int
Sfill1Pop(sfill1_stack_ctrl_t *ctrl_ptr, 
	  sfill1_stack_frame_t *local_frame_ptr)
{
    int	idx;

    if (BFILL_STACK_BLOCK_EMPTY(ctrl_ptr)) {	// popped last frame of block
        if (BFILL_STACK_NO_PREV_BLOCK(ctrl_ptr)) {    // no prev block
	    return(XIL_FAILURE);
	}
	else {
	   // use prev block
	   ctrl_ptr->curr_block = ctrl_ptr->curr_block->prev;	// update ctrl
	   ctrl_ptr->curr_idx = XILI_FILL_MAX_STACK_FRAMES;	// reset idx
	}
    }
    idx = ctrl_ptr->curr_idx-1;
    *local_frame_ptr = ctrl_ptr->curr_block->frame[idx];    // restore frame 
    ctrl_ptr->curr_idx = idx;			// decr counter
    return(XIL_SUCCESS);
}

static Xil_boolean
Sfill1PixelInside(Xil_unsigned8 *src_scan,
		  float	*weight_pixel,
		  long offset,
		  Xil_unsigned8 *fill_scan,
		  sfill1_data_t *fill_ptr)
{
    Xil_unsigned8 *bg;
    unsigned int nbands;
    float  *mtx;
    unsigned int nbg;
    int i;
    int j;
    float fraction[256];
    float differ[256];
    float *frac;
    float *diff;

    frac = &fraction[0];
    diff = &differ[0];

    bg = fill_ptr->bg;
    nbands = fill_ptr->nbands;
    mtx = fill_ptr->mtx;
    nbg = fill_ptr->nbg;

    if ( !(BFILL_MASK_ON(fill_scan, offset)) ) {

        for (i=0;i<nbands;i++) {		// create pixel - basis_n
	    *diff++ = (float)(XIL_BMAP_TST(src_scan,(offset+fill_ptr->src_offset))) - 
			*bg++;	// pixel[band_i] - basis_n[band_i]
	    src_scan += fill_ptr->src_next_band;
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
	    return(TRUE);			// fgcolor present, inside region
        }
        else {
	    return(FALSE);			// fgcolor not present, outside region
        }
    }
    else   {
	return(FALSE);	// prev filled pixel
    }
}

static void
Sfill1Shadow(sfill1_stack_frame_t *frame,
	     sfill1_data_t	 *fill_ptr,
	     long		 lx,
	     float	 	 *weight_pixel,
	     long		 rx)
{
    sfill1_stack_frame_t	new_frame;
    long			newy;
    Xil_unsigned8		*new_src_scan;
    float			*new_weight_pixel;
    Xil_unsigned8		*new_fill_scan;
    long			newlx;
    long			newrx;
    Xil_signed16		dir;

    // create 3 shadow spans
    //	  1. span(curr.lx,curr.rx)	leftmost x <-> rightmost x
    // 	  2. span(curr.prx+1,rx)	parent rightmost x + 1 <-> rightmost x
    // 	  3. span(curr.lx,curr.plx-1)	leftmost x <-> parent leftmost x - 1

    // Always create (1).

    dir = frame->dir;
    newy = frame->y + dir;
    new_src_scan = frame->src_scan + (dir * fill_ptr->src_next_scan);
    new_weight_pixel = weight_pixel + (dir * fill_ptr->weight_next_scan);
    new_fill_scan = frame->fill_scan + (dir * fill_ptr->fill_next_scan);

    new_frame.lx = lx;
    new_frame.rx = rx;
    new_frame.src_scan = new_src_scan;
    new_frame.weight_pixel = new_weight_pixel;
    new_frame.fill_scan = new_fill_scan;
    new_frame.plx = lx - 1;
    new_frame.prx = rx + 1;
    new_frame.y = newy;
    new_frame.dir = dir;

    Sfill1Push(fill_ptr->stack_ctrl, &new_frame);

    // Only create (2) if current span extends beyond parent rx 
    if (rx > frame->prx) {
	newlx = frame->prx+1;
	newy = frame->y - dir;
	new_src_scan = frame->src_scan - (dir * fill_ptr->src_next_scan);
	new_weight_pixel = weight_pixel + (newlx - lx) -
			(dir * fill_ptr->weight_next_scan);
        new_fill_scan = frame->fill_scan - (dir * fill_ptr->fill_next_scan);
	
        new_frame.lx = newlx;
    	new_frame.src_scan = new_src_scan;
    	new_frame.weight_pixel = new_weight_pixel;
    	new_frame.fill_scan = new_fill_scan;
    	new_frame.y = newy;
	new_frame.dir = -dir;
    	Sfill1Push(fill_ptr->stack_ctrl, &new_frame);

    }
    
    // Only create (3) if current span begins before parent lx 
    if (lx < frame->plx) {
	newrx = frame->plx-1;
	newy = frame->y - dir;
	new_src_scan = frame->src_scan - (dir * fill_ptr->src_next_scan);
	new_weight_pixel = weight_pixel - (dir * fill_ptr->weight_next_scan);
        new_fill_scan = frame->fill_scan - (dir * fill_ptr->fill_next_scan);
	
        new_frame.lx = lx;
        new_frame.rx = newrx;
    	new_frame.src_scan = new_src_scan;
    	new_frame.weight_pixel = new_weight_pixel;
    	new_frame.fill_scan = new_fill_scan;
    	new_frame.y = newy;
	new_frame.dir = -dir;

    	Sfill1Push(fill_ptr->stack_ctrl, &new_frame);

    }
    
}


static int
SoftFill1G(long x, long y, XilStorage* storage,
	   Xil_unsigned8 *fill_scan, float *weight_pixel,
	   sfill1_data_t *fill_ptr, XilSystemState* err_state,
	   int box_x, int box_y)
{
    sfill1_stack_frame_t  curr_frame;
    long lx;
    long rx;
    float		*left_weight_pixel;
    Xil_boolean		inside;

    // check y within bounds
    if ((y <= fill_ptr->ymax) && (y >= 0)) {

	lx = x;
	left_weight_pixel = weight_pixel;

        // check seed pixel within region
	while ( (lx >= 0) && Sfill1PixelInsideG(storage, lx, y,
						left_weight_pixel,
						fill_scan, fill_ptr,
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

	while ((rx <= fill_ptr->xmax) && Sfill1PixelInsideG(storage, rx, y,
							    weight_pixel,
							    fill_scan,
							    fill_ptr,
							    box_x, box_y) ) {
	    // set pixel mask
	    BFILL_MASK_SET(fill_scan, rx);

	    // continue right
	    rx = rx+1;
	    weight_pixel = weight_pixel + 1;

	}    // end while INSIDE go right

	rx = rx -1;				// save rightmost x

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

    if (Sfill1Push(fill_ptr->stack_ctrl,&curr_frame)==XIL_FAILURE) {
	return(XIL_FAILURE);
    }

    // push initial stack frame below
    curr_frame.weight_pixel = left_weight_pixel - fill_ptr->weight_next_scan;
    curr_frame.fill_scan = fill_scan - fill_ptr->fill_next_scan;
    curr_frame.y = y-1;
    curr_frame.dir = -1;

    if (Sfill1Push(fill_ptr->stack_ctrl,&curr_frame)==XIL_FAILURE) {
	return(XIL_FAILURE);
    }

    // begin fill loop
    while (!(BFILL_STACK_EMPTY(fill_ptr->stack_ctrl))) {

	// pop stack frame to process
	if (Sfill1Pop(fill_ptr->stack_ctrl,&curr_frame)==XIL_FAILURE) {
	   return(XIL_FAILURE);
	}

	y = curr_frame.y;

        // check y within bounds
	if ((y <= fill_ptr->ymax) && (y >= 0)) {

	    x = curr_frame.lx;
	    fill_scan = curr_frame.fill_scan;
		
		// here to check left
		weight_pixel = curr_frame.weight_pixel;
		inside = Sfill1PixelInsideG(storage, x, y, weight_pixel,
					    fill_scan, fill_ptr,
					    box_x, box_y);
		if (inside) {
		    // set pixel mask
	    	    BFILL_MASK_SET(fill_scan, x);
		    // continue left
		    x = x-1;
		    weight_pixel = weight_pixel - 1;

		    while ((x >= 0) && Sfill1PixelInsideG(storage, x, y,
							  weight_pixel,
							  fill_scan,
							  fill_ptr,
							  box_x, box_y) ) {
		        // set pixel mask
	    	        BFILL_MASK_SET(fill_scan, x);

		        // continue left
		        x = x-1;
		        weight_pixel = weight_pixel - 1;

		    }    // end while INSIDE go left

		    lx = x + 1;			// save leftmost x coord
		    left_weight_pixel = weight_pixel + 1;    // save leftmost x pixel
		}    // end if (inside)

		// here to check right

		x = curr_frame.lx + 1;
		weight_pixel = curr_frame.weight_pixel + 1;
		while (x <= fill_ptr->xmax) {

		    if (inside) {
			if (Sfill1PixelInsideG(storage, x, y,
					       weight_pixel,
					       fill_scan, fill_ptr,
					       box_x, box_y)) {
  			    // still inside
			    BFILL_MASK_SET(fill_scan, x);
			}
			else {
			    // no longer inside
			    Sfill1Shadow(&curr_frame,fill_ptr,lx,left_weight_pixel,(x-1));
			    inside = FALSE;
			}
		    }
		    else {
			if (x > curr_frame.rx) {
			    // not inside, and past shadow right
			    break;
			}
			if (Sfill1PixelInsideG(storage, x, y,
					      weight_pixel, fill_scan,
					      fill_ptr, box_x, box_y)) {
			    // now inside, new scan
			    BFILL_MASK_SET(fill_scan, x);
			    inside = TRUE;
			    lx = x;	// new leftmost x
			    left_weight_pixel = weight_pixel;
			}
			else {
			    // still not inside!, no action
			}
		    }
		    x = x+1;
		    weight_pixel = weight_pixel + 1;

	  	}    // end while within right limit

		// here to check edge of region
		if (inside) {
		    // hit the edge of region while still inside scan
		    Sfill1Shadow(&curr_frame,fill_ptr,lx,left_weight_pixel,(x-1));
		}


	}    // end if within y bounds

    }    // end while, done with fill loop, stack empty

    return(XIL_SUCCESS);
}

static Xil_boolean
Sfill1PixelInsideG(XilStorage* src_storage,
		   unsigned int x,
		   unsigned int y,
		   float* weight_pixel,
		   Xil_unsigned8* fill_scan,
		   sfill1_data_t* fill_ptr,
		   int box_x,
		   int box_y)
{
    Xil_unsigned8 *bg;
    unsigned int nbands;
    float  *mtx;
    unsigned int nbg;
    int i;
    int j;
    float fraction[256];
    float differ[256];
    float *frac;
    float *diff;

    frac = &fraction[0];
    diff = &differ[0];

    bg = fill_ptr->bg;
    nbands = fill_ptr->nbands;
    mtx = fill_ptr->mtx;
    nbg = fill_ptr->nbg;

    if ( !(BFILL_MASK_ON(fill_scan, x)) ) {

	// Create pixel - basis_n
        for (unsigned int b = 0;b < nbands; b++) {
	    unsigned int src_scanline_stride;
	    unsigned int src_offset;
	    Xil_unsigned8* src_data;
	    src_storage->getStorageInfo(b,
					NULL,
					&src_scanline_stride,
					&src_offset,
					(void**)&src_data);

	    //
	    // Convert src_data and src_offset from box space to image
	    // space.  Image space results are in "scanline" and "offset".
	    //
	    Xil_unsigned8* scanline;
	    int offset = src_offset - box_x % 8;
	    if (offset >= 0) {
		scanline = src_data + (y - box_y) * src_scanline_stride
		    - box_x / 8;
	    } else {
		// Borrow 8 bits from the scanline
		scanline = src_data + (y - box_y) * src_scanline_stride
		    - box_x / 8 - 1;
		offset += 8;
	    }

	    // pixel[band_i] - basis_n[band_i]
	    *diff++ = (float)XIL_BMAP_TST(scanline, offset + x) - *bg++;
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
	    return(TRUE);		// fgcolor present, inside region
        } else {
	    return(FALSE);		// fgcolor not present, outside region
        }
    } else {
	return(FALSE);			// prev filled pixel
    }
}
