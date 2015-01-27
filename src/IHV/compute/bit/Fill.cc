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
//  File:	Fill.cc
//  Project:	XIL
//  Revision:	1.6
//  Last Mod:	10:09:44, 03/10/00
//
//  Description:
//	This routine performs a 4-connected fill starting at the seed
//  position (x,y).  Each pixel which is not a boundary pixel  
//  is forced to the input fill_color in the destination image.
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
#pragma ident	"@(#)Fill.cc	1.6\t00/03/10  "

#include "XiliUtils.hh"
#include "XilDeviceManagerComputeBIT.hh"
#include "xili_fill_utils.hh"


//
// Data structure declarations
//
typedef struct {
    long  x;				// curr x
    long  y;				// curr y
    Xil_unsigned8	*src_scan;	// curr pixel in image
    Xil_unsigned8	*fill_scan;	// curr scanline in writemask
    long  plx;				// parent leftx
    long  prx;				// parent rightx
    long  py;				// parent y
} bfill1_stack_frame_t; 

struct bfill1_stack_block {
    struct bfill1_stack_block	*prev;
    struct bfill1_stack_block	*next;
    bfill1_stack_frame_t	frame[XILI_FILL_MAX_STACK_FRAMES];
};
typedef struct bfill1_stack_block bfill1_stack_block_t;

struct bfill1_stack_ctrl {
    struct bfill1_stack_block	*curr_block;
    int				curr_idx;
};
typedef struct bfill1_stack_ctrl bfill1_stack_ctrl_t;

typedef struct {
    long 	xmax;
    long 	ymax;
    Xil_unsigned8	*bound;
    unsigned int	nbands;
    long	        src_next_band;
    long		src_next_scan;
    long		src_offset;
    long		fill_next_scan;
    struct bfill1_stack_ctrl	*stack_ctrl;
} bfill1_data_t;


//
// Forward function declarations
//
static int BoundaryFill1(long x,
			 long y,
			 Xil_unsigned8 *src_scan,
			 Xil_unsigned8 *fill_scan,
			 bfill1_data_t *fill_ptr);

static int BFill1Push(bfill1_stack_ctrl_t *stack_ptr,
		      bfill1_stack_frame_t *local_unit_ptr);

static int BFill1Pop(bfill1_stack_ctrl_t *stack_ptr,
		     bfill1_stack_frame_t *local_unit_ptr);

static long BFill1Left(bfill1_stack_frame_t  *frame,
		       bfill1_data_t  *fill_ptr);

static long BFill1Right(bfill1_stack_frame_t  *frame,
			bfill1_data_t  *fill_ptr);

static int BFill1ScanHigh(bfill1_stack_frame_t  *frame,
			  long lx,
			  long rx, 
			  bfill1_data_t  *fill_ptr);

static int BFill1ScanLow(bfill1_stack_frame_t  *frame,
			 long lx,
			 long rx, 
			 bfill1_data_t  *fill_ptr);

static Xil_boolean BFill1PixelBoundary(Xil_unsigned8 *scan,
				       long offset,
				       Xil_unsigned8 *bound,
				       unsigned int nbands,
				       unsigned long next_band);

static int
BoundaryFill1G(long x, long y, XilStorage* storage,
	       Xil_unsigned8* fill_scan, bfill1_data_t* fill_ptr,
	       int box_x, int box_y);

static long
BFill1LeftG(bfill1_stack_frame_t* frame,
	    bfill1_data_t* fill_ptr,
	    XilStorage* storage,
	    int box_x,
	    int box_y);

static long
BFill1RightG(bfill1_stack_frame_t* frame,
	     bfill1_data_t* fill_ptr,
	     XilStorage* storage,
	     int box_x,
	     int box_y);

static int
BFill1ScanHighG(bfill1_stack_frame_t* frame,
		long lx,
		long rx, 
		bfill1_data_t* fill_ptr,
		XilStorage* storage,
		int box_x,
		int box_y);
static int
BFill1ScanLowG(bfill1_stack_frame_t* frame,
	       long lx,
	       long rx, 
	       bfill1_data_t* fill_ptr,
	       XilStorage* storage,
	       int box_x,
	       int box_y);

static Xil_boolean
BFill1PixelBoundaryG(XilStorage* storage,
		     unsigned int x,
		     unsigned int y,
		     Xil_unsigned8* bound,
		     unsigned int nbands,
		     int box_x,
		     int box_y);

//
// Perform a Boundary Fill operation.
//
// The pixel sequential case has no suffix, the corresponding general
// storage case have functions that end in "G".
//
XilStatus
XilDeviceManagerComputeBIT::Fill(XilOp*       op,
				  unsigned     ,
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
    unsigned int x_seed;
    unsigned int y_seed;
    op->getParam(1, &x_seed);		// Value is already in image space
    op->getParam(2, &y_seed);		// Value is already in image space
    //
    // For boundary and fill_color, each band occupies an
    // Xil_unsigned8 and its value is either 0 or 1.
    //
    Xil_unsigned8* boundary;
    Xil_unsigned8* fill_color;
    op->getParam(3, (void**)&boundary);
    op->getParam(4, (void**)&fill_color);

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
    
    //
    // Create and init a bitmap to store writemask for fill color
    //
    unsigned int fill_next_scan = (x_size + 7) / 8;
    Xil_unsigned8* fill_base_addr = new Xil_unsigned8[fill_next_scan * y_size];
    if(fill_base_addr == NULL) {
	XIL_ERROR(err_state, XIL_ERROR_RESOURCE, "di-1", TRUE);
        return XIL_FAILURE;
    }
    xili_memset(fill_base_addr, 0, fill_next_scan * y_size);

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
	    // Create and init bfill data struct
	    //
	    bfill1_data_t fill_data;
	    fill_data.xmax = x_size - 1;
	    fill_data.ymax = y_size - 1;
	    fill_data.bound = boundary;
	    fill_data.nbands = nbands;
	    fill_data.src_next_band = src1_band_stride;
	    fill_data.src_next_scan = src1_scanline_stride;
	    fill_data.src_offset = src_offset;
	    fill_data.fill_next_scan = fill_next_scan;

	    //
	    // Allocate 1st block of stack and stack control
	    //
	    bfill1_stack_ctrl_t *ctrl = new bfill1_stack_ctrl_t;
	    if(ctrl == NULL) {
		delete [] fill_base_addr;
		XIL_ERROR(err_state, XIL_ERROR_RESOURCE, "di-1", TRUE);
		return XIL_FAILURE;
	    }
	    bfill1_stack_block_t *block = new bfill1_stack_block_t;
	    if(block == NULL) {
		delete ctrl;
		delete [] fill_base_addr;
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
	    if (BoundaryFill1(x_seed, y_seed, src_scanline,
			      fill_scanline, &fill_data) == XIL_FAILURE) {
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
		// free other allocs
		delete ctrl;
		delete [] fill_base_addr;
		XIL_ERROR(err_state, XIL_ERROR_RESOURCE, "di-1", TRUE);
		return XIL_FAILURE;
	    }

	    //
	    //  Create a list of rectangles to loop over.  The resulting
	    //  list of rectangles is the area left by intersecting the
	    //  ROI with the destination box.
	    //
	    XilRectList    rl(roi, dest_box);

	    int            x;
	    int            y;
	    unsigned int   xsize;
	    unsigned int   ysize;
	    while(rl.getNext(&x, &y, &xsize, &ysize)) {
		Xil_unsigned8* dest_scanline = dest_data +
		    y * dest_scanline_stride;
		//
		// Translate to image space
		//
		fill_scanline = fill_base_addr + (y + box_y) * fill_next_scan;

		//
		// For each scanline...
		//
		for (; ysize > 0; ysize--) {
		    //
		    // For each pixel in a scanline...
		    //
		    for (int offset = x; offset < x + xsize; offset++) {
			//
			// Check writemask, if set, transfer fill color
			// to dest.  Translate x offset to image space
			// for writemask.
			//
			if (BFILL_MASK_ON(fill_scanline, offset + box_x)) {
			    Xil_unsigned8* dest_band = dest_scanline;
			    for (int k = 0; k < nbands; k++) {	// each band
				if (fill_color[k] == 0) {
				    XIL_BMAP_CLR(dest_band,
						 offset + dest_offset);  
				} else {
				    XIL_BMAP_SET(dest_band,
						 offset + dest_offset);
				}
				dest_band += dest_band_stride;
			    }
			}
		    }

		    // Move to next scanline
		    fill_scanline += fill_next_scan;
		    dest_scanline += dest_scanline_stride;
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
	    // Note: the src_scan in bfill1_stack_frame_t and
	    // src_next_scan, src_next_band, and src_offset slots in
	    // bfill1_data_t are not used for this case.

	    //
	    // Convert seed to scanline location in writemask
	    //
	    Xil_unsigned8* fill_scanline = fill_base_addr +
		y_seed * fill_next_scan;

	    //
	    // Create and init bfill data struct
	    //
	    bfill1_data_t fill_data;
	    fill_data.xmax = x_size - 1;
	    fill_data.ymax = y_size - 1;
	    fill_data.bound = boundary;
	    fill_data.nbands = nbands;
	    fill_data.src_next_band = 0;    // not used
	    fill_data.src_next_scan = 0;    // not used
	    fill_data.src_offset = 0;	    // not used
	    fill_data.fill_next_scan = fill_next_scan;

	    //
	    // Allocate 1st block of stack and stack control
	    //
	    bfill1_stack_ctrl_t *ctrl = new bfill1_stack_ctrl_t;
	    if(ctrl == NULL) {
		delete [] fill_base_addr;
		XIL_ERROR(err_state, XIL_ERROR_RESOURCE, "di-1", TRUE);
		return XIL_FAILURE;
	    }
	    bfill1_stack_block_t *block = new bfill1_stack_block_t;
	    if(block == NULL) {
		delete ctrl;
		delete [] fill_base_addr;
		XIL_ERROR(err_state, XIL_ERROR_RESOURCE, "di-1", TRUE);
		return XIL_FAILURE;
	    }
	    block->prev = NULL;		    // no previous block
	    block->next = NULL;		    // no next block
	    ctrl->curr_block = block;	    // current block
	    ctrl->curr_idx = 0;		    // current avail frame index
	    fill_data.stack_ctrl = ctrl;    // store stack control

	    //
	    // Create fill write mask bit image
	    //
	    if (BoundaryFill1G(x_seed, y_seed, &src1_storage,
			       fill_scanline, &fill_data,
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
		// free other allocs
		delete ctrl;
		delete [] fill_base_addr;
		XIL_ERROR(err_state, XIL_ERROR_RESOURCE, "di-1", TRUE);
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
                    unsigned int   dest_scanline_stride;
		    unsigned int   dest_offset;
                    Xil_unsigned8* dest_data;
                    dest_storage.getStorageInfo(band,
                                                NULL,
                                                &dest_scanline_stride,
                                                &dest_offset,
                                                (void**)&dest_data);

                    Xil_unsigned8* dest_scanline = dest_data +
                        y * dest_scanline_stride;
		    //
		    // Translate to image space
		    //
		    fill_scanline = fill_base_addr +
			(y + box_y) * fill_next_scan;

                    //
                    //  Each Scanline...
                    //
		    for (unsigned int yy = ysize; yy > 0; yy--) {
			//
			// For each pixel in a scanline...
			//
			for (int offset = x; offset < x + xsize; offset++) {
			    //
			    // Check writemask, if set, transfer fill color
			    // to dest
			    //
			    if (BFILL_MASK_ON(fill_scanline, offset + box_x)) {
				if (fill_color[band] == 0) {
				    XIL_BMAP_CLR(dest_scanline,
						 offset + dest_offset);
				} else {
				    XIL_BMAP_SET(dest_scanline,
						 offset + dest_offset);
				}
			    }
			}

			// Move to next scanline
			fill_scanline += fill_next_scan;
			dest_scanline += dest_scanline_stride;
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

    delete [] fill_base_addr;
    return XIL_SUCCESS;
}

static int BoundaryFill1(long x, long y, Xil_unsigned8 *src_scan,
			 Xil_unsigned8 *fill_scan, bfill1_data_t *fill_ptr)
{
    bfill1_stack_frame_t  curr_frame;
    long lx;
    long rx;

    // push initial stack frame
    curr_frame.x = x;
    curr_frame.y = y;
    curr_frame.src_scan = src_scan;
    curr_frame.fill_scan = fill_scan;
    curr_frame.plx = x;
    curr_frame.prx = x;
    curr_frame.py = y;
    if(BFill1Push(fill_ptr->stack_ctrl,&curr_frame)==XIL_FAILURE){
	return(XIL_FAILURE);
    }

    // begin fill loop
    while (!(BFILL_STACK_EMPTY(fill_ptr->stack_ctrl))) {

	// pop stack frame to process
	if (BFill1Pop(fill_ptr->stack_ctrl,&curr_frame)==XIL_FAILURE) {
	    return(XIL_FAILURE);
	}

	// check for previously filled segment
	if (!(BFILL_MASK_ON(curr_frame.fill_scan, curr_frame.x)) ) {

	    // fill this scanline connected left pixels until reach boundary
	    // return leftmost x
	    lx = BFill1Left(&curr_frame,fill_ptr);

	    // fill this scanline connected right pixels until reach boundary
	    // return rightmost x
	    rx = BFill1Right(&curr_frame,fill_ptr);

	    // check scanline above current scanline for connected pixels
	    // Push shadow seedpoint(s)
	    if (BFill1ScanHigh(&curr_frame,lx,rx,fill_ptr)==XIL_FAILURE) {
		return(XIL_FAILURE);
	    }

	    // check scanline below current scanline for connected pixels
	    // Push shadow seedpoint(s)
	    if (BFill1ScanLow(&curr_frame,lx,rx,fill_ptr)==XIL_FAILURE){
	   	return(XIL_FAILURE);
	    }
	}

    }    // done with fill loop, stack empty

    return(XIL_SUCCESS);
}

static int BFill1Push(bfill1_stack_ctrl_t *ctrl_ptr, 
	              bfill1_stack_frame_t *local_frame_ptr)
{
    int	idx;

    if (BFILL_STACK_BLOCK_FULL(ctrl_ptr)) {    // curr block full
	if (BFILL_STACK_NO_NEXT_BLOCK(ctrl_ptr)) {   // no next block
	    // must allocate new stack block, curr block full
	    bfill1_stack_block_t *block = new bfill1_stack_block_t;
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

static int BFill1Pop(bfill1_stack_ctrl_t *ctrl_ptr, 
	             bfill1_stack_frame_t *local_frame_ptr)
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

static long BFill1Left(bfill1_stack_frame_t  *frame,
		       bfill1_data_t	  *fill_ptr)
{
    long newx;
    Xil_unsigned8 *new_src_scan;

    newx = frame->x;
    new_src_scan = frame->src_scan;
    while (newx >= 0) {
	if (BFill1PixelBoundary(new_src_scan,(newx+fill_ptr->src_offset),
				fill_ptr->bound,fill_ptr->nbands,
				fill_ptr->src_next_band)) {
	    break;		// found leftmost x, break
	}
	else {
	    // not boundary, fill writemask
	    BFILL_MASK_SET(frame->fill_scan, newx);
	    newx = newx-1;		// continue search left
	}
    }
    return(newx+1);		// return leftmost x
}

static long BFill1Right(bfill1_stack_frame_t  *frame,
			bfill1_data_t  *fill_ptr)
{
    long newx;
    Xil_unsigned8 *new_src_scan;

    newx = frame->x + 1;
    new_src_scan = frame->src_scan;
    while (newx <= fill_ptr->xmax) {
	if (BFill1PixelBoundary(new_src_scan,(newx+fill_ptr->src_offset),fill_ptr->bound,fill_ptr->nbands,fill_ptr->src_next_band)) {
	    break;		// found rightmost x, break
	}
	else {
	    // not boundary, perform fill
	    BFILL_MASK_SET(frame->fill_scan, newx);
	    newx = newx+1;		// continue search right
	}
    }
    return(newx-1);		// return rightmost x
}

static int BFill1ScanHigh(bfill1_stack_frame_t  *frame,
			  long lx,
			  long rx, 
			  bfill1_data_t  *fill_ptr)
{
    long scany;
    long scanx;
    long scanlx;
    long scanrx;
    Xil_unsigned8 *scan_src_scan, *scan_fill_scan;
    Xil_boolean  inside_segment;
    bfill1_stack_frame_t  new_frame;

    scany = frame->y + 1;
    if (scany <= fill_ptr->ymax) {

	// check high scanline for shadow segments
        scan_src_scan = frame->src_scan + fill_ptr->src_next_scan;
        scan_fill_scan = frame->fill_scan + fill_ptr->fill_next_scan;
	scanlx = lx;
	scanrx = rx;

	// check for scany == parenty, may have already processed this shadow
	if (scany == frame->py) {
	    if (frame->plx <= lx) {	// parent already checked plx<x<prx
		scanlx = frame->prx+1;	// adjust scanlx
	    }
	    if (frame->prx >= rx) {	// parent already checked plx<x<prx
		scanrx = frame->plx-1;	// adjust scanrx
	    }
	    
	}

	// scanline still have valid shadow segment ??
	if (scanrx < scanlx) {
	    return(XIL_SUCCESS);	// no shadow, return
	}
	else {
	    // check for connecting pixels between scanlx < scanx < scanrx
	    inside_segment = FALSE;
   
	    for (scanx = scanlx; scanx <= scanrx; scanx++) {
		if (!(BFill1PixelBoundary(scan_src_scan,(scanx+fill_ptr->src_offset), fill_ptr->bound, fill_ptr->nbands,fill_ptr->src_next_band)) ) {
		    // found non-boundary pixel in shadow segment.
		    // check for already visited
		    if (!inside_segment) {
		        if (!(BFILL_MASK_ON(scan_fill_scan, scanx)) ) {
		            // push new stack frame for scanline high 
		            new_frame.x = scanx;
		            new_frame.y = scany;
		            new_frame.src_scan = scan_src_scan;
		            new_frame.fill_scan = scan_fill_scan;
		            new_frame.plx = lx;
		            new_frame.prx = rx;
		            new_frame.py = frame->y;
		            if (BFill1Push(fill_ptr->stack_ctrl, &new_frame) == XIL_FAILURE) {
				return(XIL_FAILURE);
			    }
			    inside_segment = TRUE;
			}
		    }
		}
		else {
		    // found boundary pixel in shadow segment, reset flag
		    inside_segment = FALSE;
		}

	    }    // end for (scanx <= scanrx)

        }    // end if/else (scanrx < scanlx)

    }    // end if (scany <=fill_ptr->ymax)

    return(XIL_SUCCESS);
}

static int BFill1ScanLow(bfill1_stack_frame_t  *frame,
			 long lx,
			 long rx, 
			 bfill1_data_t  *fill_ptr)
{
    long scany;
    long scanx;
    long scanlx;
    long scanrx;
    Xil_unsigned8 *scan_src_scan, *scan_fill_scan;
    Xil_boolean  inside_segment;
    bfill1_stack_frame_t  new_frame;

    scany = frame->y - 1;
    if (scany >= 0) {

	// check high scanline for shadow segments
        scan_src_scan = frame->src_scan - fill_ptr->src_next_scan;
        scan_fill_scan = frame->fill_scan - fill_ptr->fill_next_scan;
	scanlx = lx;
	scanrx = rx;

	// check for scany == parenty, may have already processed this shadow
	if (scany == frame->py) {
	    if (frame->plx <= lx) {	// parent already checked plx<x<prx
		scanlx = frame->prx+1;	// adjust scanlx
	    }
	    if (frame->prx >= rx) {	// parent already checked plx<x<prx
		scanrx = frame->plx-1;	// adjust scanrx
	    }
	}

	// scanline still have valid shadow segment ??
	if (scanrx < scanlx) {
	    return(XIL_SUCCESS);	// no shadow, return
	}
	else {
	    // check for connecting pixels between scanlx < scanx < scanrx
	    
	    inside_segment = FALSE;
	    for (scanx = scanlx; scanx <= scanrx; scanx++ ) {
		if (!(BFill1PixelBoundary(scan_src_scan,(scanx+fill_ptr->src_offset),fill_ptr->bound,fill_ptr->nbands,fill_ptr->src_next_band))) {
		    // found non-boundary pixel in shadow segment.
		    if (!inside_segment) {
		        // check for already visited writemask
		        if (!(BFILL_MASK_ON(scan_fill_scan, scanx)) ) {
		            // push new stack frame for scanline high 
		            new_frame.x = scanx;
		            new_frame.y = scany;
		            new_frame.src_scan = scan_src_scan;
		            new_frame.fill_scan = scan_fill_scan;
		            new_frame.plx = lx;
		            new_frame.prx = rx;
		            new_frame.py = frame->y;
		            if (BFill1Push(fill_ptr->stack_ctrl, &new_frame) == XIL_FAILURE) {
				return(XIL_FAILURE);
			    }
			    inside_segment = TRUE;
			}
		    }
		}
		else {
		    // found boundary pixel in shadow segment, reset flag
		    inside_segment = FALSE;
		}

	    }    // end for (scanx <= scanrx)

        }    // end if/else (scanrx < scanlx)

    }    // end if (scany >=fill_ptr->ymin)

    return(XIL_SUCCESS);
}

static Xil_boolean BFill1PixelBoundary(Xil_unsigned8 *scan,
				       long offset,
				       Xil_unsigned8 *bound,
				       unsigned int nbands,
				       unsigned long next_band)
{
    unsigned int i;
    Xil_unsigned8 *band;

    band = scan;
    for (i=0;i<nbands;i++) {		    // check each band
	if (XIL_BMAP_TST(band,offset)) {    // if src bit set,
            if(bound[i] == 0) {             // if boundary is '0'
	        return(FALSE);		    // stop, return false
	    }
	}
	else {				    // if src bit clear,
            if(bound[i] != 0) {             // if boundary is '1'
	        return(FALSE);		    // stop, return false
	    }
	}
	band += next_band;		    // move to next band
    }
    return(TRUE);	// processed all bands, matches boundary, return true
}


static int
BoundaryFill1G(long x, long y, XilStorage* storage,
	       Xil_unsigned8* fill_scan, bfill1_data_t* fill_ptr,
	       int box_x, int box_y)
{
    bfill1_stack_frame_t  curr_frame;
    long lx;
    long rx;

    // push initial stack frame
    curr_frame.x = x;
    curr_frame.y = y;
    curr_frame.fill_scan = fill_scan;
    curr_frame.plx = x;
    curr_frame.prx = x;
    curr_frame.py = y;
    if (BFill1Push(fill_ptr->stack_ctrl, &curr_frame) == XIL_FAILURE) {
	return XIL_FAILURE;
    }

    // begin fill loop
    while (!(BFILL_STACK_EMPTY(fill_ptr->stack_ctrl))) {

	// pop stack frame to process
	if (BFill1Pop(fill_ptr->stack_ctrl, &curr_frame) == XIL_FAILURE) {
	    return XIL_FAILURE;
	}

	// check for previously filled segment
	if (!(BFILL_MASK_ON(curr_frame.fill_scan, curr_frame.x)) ) {

	    // fill this scanline connected left pixels until reach boundary
	    // return leftmost x
	    lx = BFill1LeftG(&curr_frame, fill_ptr, storage, box_x, box_y);

	    // fill this scanline connected right pixels until reach boundary
	    // return rightmost x
	    rx = BFill1RightG(&curr_frame, fill_ptr, storage, box_x, box_y);

	    // check scanline above current scanline for connected pixels
	    // Push shadow seedpoint(s)
	    if (BFill1ScanHighG(&curr_frame, lx, rx, fill_ptr, storage,
				box_x, box_y) == XIL_FAILURE) {
		return XIL_FAILURE;
	    }

	    // check scanline below current scanline for connected pixels
	    // Push shadow seedpoint(s)
	    if (BFill1ScanLowG(&curr_frame, lx, rx, fill_ptr, storage,
			       box_x, box_y) == XIL_FAILURE) {
	   	return XIL_FAILURE;
	    }
	}

    }    // done with fill loop, stack empty

    return XIL_SUCCESS;
}

static long
BFill1LeftG(bfill1_stack_frame_t* frame,
	    bfill1_data_t* fill_ptr,
	    XilStorage* storage,
	    int box_x,
	    int box_y)
{
    long newx = frame->x;
    while (newx >= 0) {
	if (BFill1PixelBoundaryG(storage, newx, frame->y, fill_ptr->bound,
				 fill_ptr->nbands, box_x, box_y)) {
	    break;		// found leftmost x, break
	} else {
	    // not boundary, fill writemask
	    BFILL_MASK_SET(frame->fill_scan, newx);
	    newx--;		// continue search left
	}
    }
    return newx+1;		// return leftmost x
}

static long
BFill1RightG(bfill1_stack_frame_t* frame,
	     bfill1_data_t* fill_ptr,
	     XilStorage* storage,
	     int box_x,
	     int box_y)
{
    long newx = frame->x + 1;
    while (newx <= fill_ptr->xmax) {
	if (BFill1PixelBoundaryG(storage, newx, frame->y, fill_ptr->bound,
				 fill_ptr->nbands, box_x, box_y)) {
	    break;		// found rightmost x, break
	} else {
	    // not boundary, perform fill
	    BFILL_MASK_SET(frame->fill_scan, newx);
	    newx++;		// continue search right
	}
    }
    return newx-1;		// return rightmost x
}

static int
BFill1ScanHighG(bfill1_stack_frame_t* frame,
		long lx,
		long rx, 
		bfill1_data_t* fill_ptr,
		XilStorage* storage,
		int box_x,
		int box_y)
{
    long scany = frame->y + 1;
    if (scany <= fill_ptr->ymax) {

	// check high scanline for shadow segments
        Xil_unsigned8* scan_fill_scan = frame->fill_scan +
	    fill_ptr->fill_next_scan;
	long scanlx = lx;
	long scanrx = rx;

	// check for scany == parenty, may have already processed this shadow
	if (scany == frame->py) {
	    if (frame->plx <= lx) {	// parent already checked plx<x<prx
		scanlx = frame->prx+1;	// adjust scanlx
	    }
	    if (frame->prx >= rx) {	// parent already checked plx<x<prx
		scanrx = frame->plx-1;	// adjust scanrx
	    }
	    
	}

	// scanline still have valid shadow segment ??
	if (scanrx < scanlx) {
	    return XIL_SUCCESS;		    // no shadow, return
	}

	// check for connecting pixels between scanlx < scanx < scanrx
	Xil_boolean inside_segment = FALSE;

	for (long scanx = scanlx; scanx <= scanrx; scanx++) {
	    if (!BFill1PixelBoundaryG(storage, scanx, scany,
				      fill_ptr->bound,
				      fill_ptr->nbands, box_x, box_y)) {
		// found non-boundary pixel in shadow segment.
		if (!inside_segment) {
		    // check for already visited writemask
		    if (!BFILL_MASK_ON(scan_fill_scan, scanx)) {
			// push new stack frame for scanline high 
			bfill1_stack_frame_t  new_frame;
			new_frame.x = scanx;
			new_frame.y = scany;
			new_frame.fill_scan = scan_fill_scan;
			new_frame.plx = lx;
			new_frame.prx = rx;
			new_frame.py = frame->y;
			if (BFill1Push(fill_ptr->stack_ctrl, &new_frame)
			    == XIL_FAILURE) {
			    return XIL_FAILURE;
			}
			inside_segment = TRUE;
		    }
		}
	    } else {
		// found boundary pixel in shadow segment, reset flag
		inside_segment = FALSE;
	    }
	}    // end for (scanx <= scanrx)
    }    // end if (scany <=fill_ptr->ymax)

    return XIL_SUCCESS;
}

static int
BFill1ScanLowG(bfill1_stack_frame_t* frame,
	       long lx,
	       long rx, 
	       bfill1_data_t* fill_ptr,
	       XilStorage* storage,
	       int box_x,
	       int box_y)
{
    long scany = frame->y - 1;
    if (scany >= 0) {

	// check high scanline for shadow segments
        Xil_unsigned8* scan_fill_scan = frame->fill_scan -
	    fill_ptr->fill_next_scan;
	long scanlx = lx;
	long scanrx = rx;

	// check for scany == parenty, may have already processed this shadow
	if (scany == frame->py) {
	    if (frame->plx <= lx) {	// parent already checked plx<x<prx
		scanlx = frame->prx+1;	// adjust scanlx
	    }
	    if (frame->prx >= rx) {	// parent already checked plx<x<prx
		scanrx = frame->plx-1;	// adjust scanrx
	    }
	}

	// scanline still have valid shadow segment ??
	if (scanrx < scanlx) {
	    return XIL_SUCCESS;		    // no shadow, return
	}

	// check for connecting pixels between scanlx < scanx < scanrx
	Xil_boolean inside_segment = FALSE;
	    
	for (long scanx = scanlx; scanx <= scanrx; scanx++ ) {
	    if (!BFill1PixelBoundaryG(storage, scanx, scany,
				      fill_ptr->bound,
				      fill_ptr->nbands, box_x, box_y)) {
		// found non-boundary pixel in shadow segment.
		if (!inside_segment) {
		    // check for already visited writemask
		    if (!BFILL_MASK_ON(scan_fill_scan, scanx)) {
			// push new stack frame for scanline low
			bfill1_stack_frame_t  new_frame;
			new_frame.x = scanx;
			new_frame.y = scany;
			new_frame.fill_scan = scan_fill_scan;
			new_frame.plx = lx;
			new_frame.prx = rx;
			new_frame.py = frame->y;
			if (BFill1Push(fill_ptr->stack_ctrl, &new_frame)
			    == XIL_FAILURE) {
			    return XIL_FAILURE;
			}
			inside_segment = TRUE;
		    }
		}
	    } else {
		// found boundary pixel in shadow segment, reset flag
		inside_segment = FALSE;
	    }
	}    // end for (scanx <= scanrx)
    }    // end if (scany >=fill_ptr->ymin)

    return(XIL_SUCCESS);
}

//
// Returns TRUE if pixel (x, y) of image associated with "storage" is a
// boundary pixel.  Boundary pixel passed in as "bound" is array of Xil_Unsigned8
// whose values are 0 or 1, 1 entry for each band.
// This is for the General storage case.
//
static Xil_boolean
BFill1PixelBoundaryG(XilStorage* src_storage,
		     unsigned int x,
		     unsigned int y,
		     Xil_unsigned8* bound,
		     unsigned int nbands,
		     int box_x,
		     int box_y)
{
    for (unsigned int b = 0; b < nbands; b++) {
	unsigned int src_scanline_stride;
	unsigned int src_offset;
	Xil_unsigned8* src_data;
	src_storage->getStorageInfo(b,
				    NULL,
				    &src_scanline_stride,
				    &src_offset,
				    (void**) &src_data);

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

	//
	// Test pixel
	//
	Xil_unsigned8 pixel = XIL_BMAP_TST(scanline, offset + x)
	    ? 1 : 0;

	//
	//  Return FALSE at first mismatch
	//
	if (pixel != *bound) {
	    return FALSE;
	}

	//
	// Next boundary pixel band
	//
	bound++;
    }
    return TRUE;
}
