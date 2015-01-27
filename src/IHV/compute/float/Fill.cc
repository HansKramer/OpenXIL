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
//  Revision:	1.5
//  Last Mod:	10:13:07, 03/10/00
//
//  Description:
//	This routine performs a 4-connected fill starting at the seed
//  position (x,y).  Each pixel which is not a boundary pixel  
//  is forced to the input fill_color in the destination image.
//	
//  MT-level:  SAFE
//	
//------------------------------------------------------------------------
//	COPYRIGHT
//------------------------------------------------------------------------
#pragma ident	"@(#)Fill.cc	1.5\t00/03/10  "

#include "XiliUtils.hh"
#include "XilDeviceManagerComputeFLOAT.hh"
#include "xili_fill_utils.hh"

//
//  Data structure declarations
//
struct bfillF_stack_frame {
    int            lx;                   // curr leftmost x
    int            rx;                   // curr rightmost x
    Xil_float32*   src_pixel;            // curr pixel in image
    Xil_unsigned8* fill_scan;            // curr scanline in writemask
    int            plx;                  // parent leftx
    int            prx;                  // parent rightx
    int            y;                    // curr y
    Xil_signed16   dir;                  // current direction (+1,-1)
}; 

struct bfillF_stack_block {
    bfillF_stack_block* prev;
    bfillF_stack_block* next;
    bfillF_stack_frame  frame[XILI_FILL_MAX_STACK_FRAMES];
};

struct bfillF_stack_ctrl {
    bfillF_stack_block* curr_block;
    int                 curr_idx;
};

struct bfillF_data {
    int                xmax;             // max x value
    int                ymax;             // max y value
    Xil_float32*       bound;            // boundary pixel array
    unsigned int       nbands;
    unsigned int       src_next_pixel;   // src pixel stride
    int                src_next_scan;    // src scanline stride
    int                fill_next_scan;   // bitmap scanline stride
    bfillF_stack_ctrl* stack_ctrl;       // points to stack ctrl block
};


//
// Forward function declarations
//
static int BoundaryFillF(int            x,
                         int            y,
                         Xil_float32*   src_pixel,
                         Xil_unsigned8* fill_scan,
                         bfillF_data*  fill_ptr);

static int BFillFPush(bfillF_stack_ctrl*  ctrl_ptr,
                      bfillF_stack_frame* local_frame_ptr);

static int BFillFPop(bfillF_stack_ctrl*  ctrl_ptr,
                     bfillF_stack_frame* local_frame_ptr);

inline Xil_boolean BFillFPixelBoundary(Xil_float32*   pixel,
                                       Xil_float32*   bound,
                                       unsigned int   nbands);

static void BFillFShadow(bfillF_stack_frame* frame,
                         bfillF_data*        fill_ptr,
                         int                  lx,
                         Xil_float32*         pixel,
                         int                  rx);

static int  BoundaryFillFG(int            x,
                           int            y,
                           XilStorage*    src_storage,
                           Xil_unsigned8* fill_scan,
                           bfillF_data*  fill_ptr,
                           int            box_x,
                           int            box_y);

static Xil_boolean BFillFPixelBoundaryG(XilStorage*    storage,
                                        unsigned int   x,
                                        unsigned int   y,
                                        Xil_float32*   bound,
                                        unsigned int   nbands,
                                        int            box_x,
                                        int            box_y);

//
// Perform a Boundary Fill operation.
//
// The pixel sequential case has no suffix, the corresponding general
// storage case have functions that end in "G".
//
XilStatus
XilDeviceManagerComputeFLOAT::Fill(XilOp*       op,
                                   unsigned     ,
                                   XilRoi*      roi,
                                   XilBoxList*  bl)
{
    //
    //  For Fill, the entire source image is made available to the compute
    //  operation since we may need to access every pixel in the source in
    //  order to determine the boundaries.  The source box is the
    //  corresponding area in the source for a destination box.  We know we
    //  have the entire source, so we just subtract the source box offsets to
    //  point at the beginning of the source image.
    //
    //  We can't call split on tile boundaries for fill operations...it does
    //  nothing since there isn't a reasonable or generic algorithm for
    //  splitting on tiles in the source.
    //
    //  TODO:  9/22/96 jlf  We don't have a preprocess routine for fill
    //                      because the XIL core will not split this operation
    //                      in the destination (no multiple threads).  Is this
    //                      a reasonable assumption?  We build the area to be
    //                      filled based on the source so we could have
    //                      multiple threads writing into the destination.
    //

    //
    //  Get the images for our operation.
    //
    XilImage* src = op->getSrcImage(1);
    XilImage* dst = op->getDstImage(1);

    //
    //  Get params from the op.
    //
    //  NOTE:  The seeds are already in source image space.
    //
    unsigned int x_seed;
    op->getParam(1, &x_seed);

    unsigned int y_seed;
    op->getParam(2, &y_seed);

    Xil_float32*   boundary;
    op->getParam(3, (void**)&boundary);

    Xil_float32*   fill_color;
    op->getParam(4, (void**)&fill_color);

    //
    //  Get system state for reporting errors
    //
    XilSystemState* err_state = dst->getSystemState();

    //
    //  Number of bands for this operation.
    //
    unsigned int nbands = src->getNumBands();

    //
    //  Get some info about the source image
    //
    unsigned int src_xsize;
    unsigned int src_ysize;
    src->getSize(&src_xsize, &src_ysize);
    
    //
    //  Create and init a bitmap to store writemask for fill color
    //
    unsigned int   fill_next_scan = (src_xsize + 7) / 8;
    Xil_unsigned8* fill_base_addr =
        new Xil_unsigned8[fill_next_scan * src_ysize];
    if(fill_base_addr == NULL) {
        XIL_ERROR(err_state, XIL_ERROR_RESOURCE, "di-1", TRUE);
        return XIL_FAILURE;
    }
    xili_memset(fill_base_addr, 0, fill_next_scan * src_ysize);

    //
    //  Loop over each of the boxes of storage we are to process.
    //
    XilBox* src_box;
    XilBox* dst_box;
    while(bl->getNext(&src_box, &dst_box)) {
        //
        //  Aquire our storage from the images.  The storage returned is valid
        //  for the box given.  Thus, any origins or child offsets have been
        //  taken into account.
        //
        XilStorage  src_storage(src);
        XilStorage  dst_storage(dst);
        if((src->getStorage(&src_storage, op, src_box, "XilMemory",
                             XIL_READ_ONLY)  == XIL_FAILURE) ||
           (dst->getStorage(&dst_storage, op, dst_box, "XilMemory",
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
        //  Test to see if all of our storage is of type XIL_PIXEL_SEQUENTIAL.
        //  If so, implement an loop optimized for pixel-sequential storage.
        //
        if((src_storage.isType(XIL_PIXEL_SEQUENTIAL)) &&
           (dst_storage.isType(XIL_PIXEL_SEQUENTIAL))) {
            unsigned int   src_pixel_stride;
            unsigned int   src_scanline_stride;
            Xil_float32*   src_data;
            src_storage.getStorageInfo(&src_pixel_stride,
                                        &src_scanline_stride,
                                        NULL, NULL,
                                        (void**)&src_data);
            
            unsigned int   dst_pixel_stride;
            unsigned int   dst_scanline_stride;
            Xil_float32*   dst_data;
            dst_storage.getStorageInfo(&dst_pixel_stride,
                                        &dst_scanline_stride,
                                        NULL, NULL,
                                        (void**)&dst_data);

            //
            //  Get the source box offsets so we can compute the offset of our
            //  seed since its given in image space.
            //
            int          srcbox_x;
            int          srcbox_y;
            unsigned int srcbox_w;
            unsigned int srcbox_h;
            src_box->getAsRect(&srcbox_x, &srcbox_y, &srcbox_w, &srcbox_h);

            //
            //  Find seed pixel location in src and writemask.  Use
            //  appropriate image/box space seed coordinates.
            //
            Xil_float32*   src_pixel = src_data +
                (y_seed - srcbox_y) * src_scanline_stride +
                (x_seed - srcbox_x) * src_pixel_stride;
            Xil_unsigned8* fill_scanline = fill_base_addr +
                y_seed * fill_next_scan;

            //
            //  Create and init bfill data struct
            //
            bfillF_data fill_data;
            fill_data.xmax = src_xsize - 1;
            fill_data.ymax = src_ysize - 1;
            fill_data.bound = boundary;
            fill_data.nbands = nbands;
            fill_data.src_next_pixel = src_pixel_stride;
            fill_data.src_next_scan = src_scanline_stride;
            fill_data.fill_next_scan = fill_next_scan;

            //
            //  Allocate 1st block of stack and stack control
            //
            bfillF_stack_ctrl* ctrl = new bfillF_stack_ctrl;
            if(ctrl == NULL) {
                delete [] fill_base_addr;
                XIL_ERROR(err_state, XIL_ERROR_RESOURCE, "di-1", TRUE);
                return XIL_FAILURE;
            }
            bfillF_stack_block* block = new bfillF_stack_block;
            if(block == NULL) {
                delete ctrl;
                delete [] fill_base_addr;
                XIL_ERROR(err_state, XIL_ERROR_RESOURCE, "di-1", TRUE);
                return XIL_FAILURE;
            }
            block->prev = NULL;            // no previous block
            block->next = NULL;            // no next block
            ctrl->curr_block = block;      // current block
            ctrl->curr_idx = 0;            // current avail frame index
            fill_data.stack_ctrl = ctrl;   // store stack control

            //
            //  Create fill write mask bit image.  We use image space
            //  coordinates here.
            //
            if(BoundaryFillF(x_seed, y_seed, src_pixel, fill_scanline,
                              &fill_data) == XIL_FAILURE) {
                //
                // Failed during fill operation, attempt to clean up and
                // return.  Must delete chain of alloc blocks following
                // the next ptr
                //
                while (block != NULL) {
                    ctrl->curr_block = block->next;        // save &next block
                    delete block;                        // free curr block
                    block = ctrl->curr_block;                // curr = next
                }

                //
                //  Free other allocs
                //
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
            XilRectList    rl(roi, dst_box);

            int            x;
            int            y;
            unsigned int   xsize;
            unsigned int   ysize;
            while(rl.getNext(&x, &y, &xsize, &ysize)) {
                Xil_float32*   dst_scanline = dst_data +
                    (y*dst_scanline_stride) + (x*dst_pixel_stride);
                fill_scanline = fill_base_addr +
                    (y + srcbox_y) * fill_next_scan;

                for(int i = ysize; i != 0; i--) {
                    Xil_float32*   dst_pixel       = dst_scanline;
                    unsigned int   fill_bit_offset = x + srcbox_x;

                    for(int j = xsize; j != 0; j--) {
                        //
                        // If writemask is set, then transfer fill color
                        // to the destination image.
                        //
                        if(BFILL_MASK_ON(fill_scanline,
                                          fill_bit_offset)) {
                            Xil_float32*   dst_band = dst_pixel;

                            for(int k = 0; k < nbands; k++) {
                                *dst_band++ = fill_color[k];
                            }
                        }

                        fill_bit_offset++;
                        dst_pixel += dst_pixel_stride;
                    }

                    fill_scanline += fill_next_scan;
                    dst_scanline += dst_scanline_stride;
                }
            }

            //
            //  Delete chain of alloc blocks following the next ptr
            //
            while(block != NULL) {
                ctrl->curr_block = block->next;
                delete block;
                block = ctrl->curr_block;
            }

            //
            //  Free other allocs
            //
            delete ctrl;
        } else {
            //
            //  General storage case...
            //
            //  NOTE:  The src_pixel in bfillF_stack_frame and related slots
            //         in bfillF_data are not used for this case.
            //

            //
            //  Get the source box offsets so we can compute the offset of our
            //  seed since its given in image space.
            //
            //  Adjust the source pointer to point at the beginning of the
            //  source image.
            //
            int          srcbox_x;
            int          srcbox_y;
            unsigned int srcbox_w;
            unsigned int srcbox_h;
            src_box->getAsRect(&srcbox_x, &srcbox_y, &srcbox_w, &srcbox_h);

            //
            //  Convert seed to scanline location in writemask
            //
            Xil_unsigned8* fill_scanline = fill_base_addr +
                y_seed * fill_next_scan;

            //
            //  Create and init bfill data struct
            //
            bfillF_data fill_data;
            fill_data.xmax = src_xsize - 1;
            fill_data.ymax = src_ysize - 1;
            fill_data.bound = boundary;
            fill_data.nbands = nbands;
            fill_data.src_next_pixel = 0;   // not used
            fill_data.src_next_scan = 0;    // not used
            fill_data.fill_next_scan = fill_next_scan;

            //
            //  Allocate 1st block of stack and stack control
            //
            bfillF_stack_ctrl* ctrl = new bfillF_stack_ctrl;
            if(ctrl == NULL) {
                delete [] fill_base_addr;
                XIL_ERROR(err_state, XIL_ERROR_RESOURCE, "di-1", TRUE);
                return XIL_FAILURE;
            }
            bfillF_stack_block* block = new bfillF_stack_block;
            if(block == NULL) {
                delete ctrl;
                delete [] fill_base_addr;
                XIL_ERROR(err_state, XIL_ERROR_RESOURCE, "di-1", TRUE);
                return XIL_FAILURE;
            }
            block->prev = NULL;             // no previous block
            block->next = NULL;             // no next block
            ctrl->curr_block = block;       // current block
            ctrl->curr_idx = 0;             // current avail frame index
            fill_data.stack_ctrl = ctrl;    // store stack control

            //
            //  Create fill write mask bit image.
            //
            if(BoundaryFillFG(x_seed, y_seed, &src_storage, fill_scanline,
                               &fill_data,
                               srcbox_x, srcbox_y) == XIL_FAILURE) {
                //
                //  Failed during fill operation, attempt to clean up and
                //  return.  Must delete chain of alloc blocks following
                //  the next ptr
                //
                while(block != NULL) {
                    ctrl->curr_block = block->next;
                    delete block;
                    block = ctrl->curr_block;
                }
                //
                //  Free other allocs
                //
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
            XilRectList    rl(roi, dst_box);
            
            int            x;
            int            y;
            unsigned int   xsize;
            unsigned int   ysize;
            while(rl.getNext(&x, &y, &xsize, &ysize)) {
                //
                //  Each Band...
                //
                for(unsigned int band=0; band<nbands; band++) {
                    unsigned int   dst_pixel_stride;
                    unsigned int   dst_scanline_stride;
                    Xil_float32*   dst_data;
                    dst_storage.getStorageInfo(band,
                                               &dst_pixel_stride,
                                               &dst_scanline_stride,
                                               NULL,
                                               (void**)&dst_data);

                    Xil_float32*   dst_scanline = dst_data +
                        (y*dst_scanline_stride) + (x*dst_pixel_stride);

                    fill_scanline = fill_base_addr +
                        (y + srcbox_y) * fill_next_scan;

                    for(int i = ysize; i != 0; i--) {
                        Xil_float32*   dst_pixel = dst_scanline;
                        unsigned int   fill_bit_offset = x + srcbox_x;
                        
                        for(int j = xsize; j != 0; j--) {
                            //
                            // If writemask is set, then transfer fill color
                            // to the destination image.
                            //
                            if (BFILL_MASK_ON(fill_scanline,
                                              fill_bit_offset)) {
                                *dst_pixel = fill_color[band];
                            }

                            fill_bit_offset++;
                            dst_pixel += dst_pixel_stride;
                        }

                        fill_scanline += fill_next_scan;
                        dst_scanline += dst_scanline_stride;
                    }
                }
            }

            //
            //  Delete chain of alloc blocks following the next ptr
            //
            while (block != NULL) {
                ctrl->curr_block = block->next;
                delete block;
                block = ctrl->curr_block;
            }
            //
            //  Free other allocs
            //
            delete ctrl;
        }
    }

    delete [] fill_base_addr;
    return XIL_SUCCESS;
}

inline
Xil_boolean
BFillFPixelBoundary(Xil_float32*   pixel,
                     Xil_float32*   bound,
                     unsigned int   nbands)
{
    Xil_boolean ret_val = TRUE;

    //
    //  Check each band to see if the pixel matches the boundary color.
    //
    for(int i=0; i<nbands; i++) { 
        if(*(pixel+i) != *(bound+i)) {
            ret_val = FALSE;
            break;
        }
    }

    return ret_val;
}

//
//  Perform a boundary fill and place results in a bitmap
//
static
int
BoundaryFillF(int            x,
               int            y,
               Xil_float32*   src_pixel,
               Xil_unsigned8* fill_scan,
               bfillF_data*   fill_ptr)
{
    bfillF_stack_frame curr_frame;
    int                lx;
    int                rx;
    Xil_float32*       left_src_pixel;
    Xil_boolean        inside;

    //
    //  Check y within bounds
    //
    if((y <= fill_ptr->ymax) && (y >= 0)) {
        lx = x;
        left_src_pixel = src_pixel;

        //
        //  while INSIDE go left
        //
        while((lx >= 0) && !(BFillFPixelBoundary(left_src_pixel,
                                                 fill_ptr->bound,
                                                 fill_ptr->nbands)) ) {
            //
            //  Set pixel mask
            //
            BFILL_MASK_SET(fill_scan, lx);

            //
            //  Continue left
            //
            lx             = lx-1;
            left_src_pixel = left_src_pixel - fill_ptr->src_next_pixel;

        }

        //
        //  Here to check right -- save leftmost
        //
        lx             = lx + 1;
        left_src_pixel = left_src_pixel + fill_ptr->src_next_pixel;

        rx        = x + 1;
        src_pixel = src_pixel + fill_ptr->src_next_pixel;

        //
        //  while INSIDE go right
        //
        while((rx <= fill_ptr->xmax) &&
              !(BFillFPixelBoundary(src_pixel,
                                    fill_ptr->bound,
                                    fill_ptr->nbands))) {
            //
            //  Set pixel mask
            //
            BFILL_MASK_SET(fill_scan, rx);

            //
            //  Continue right
            //
            rx        = rx+1;
            src_pixel = src_pixel + fill_ptr->src_next_pixel;

        }

        //
        //  Save rightmost x
        //
        rx = rx - 1;
    } else {
        //
        //  Not within y range
        //
        return XIL_FAILURE;
    }        

    //
    //  Push initial stack frame above
    //
    curr_frame.lx        = lx;
    curr_frame.rx        = rx;
    curr_frame.src_pixel = left_src_pixel + fill_ptr->src_next_scan;
    curr_frame.fill_scan = fill_scan + fill_ptr->fill_next_scan;
    curr_frame.plx       = lx-1;
    curr_frame.prx       = rx+1;
    curr_frame.y         = y+1;
    curr_frame.dir       = 1;

    if(BFillFPush(fill_ptr->stack_ctrl, &curr_frame) == XIL_FAILURE) {
        return XIL_FAILURE;
    }

    //
    //  Push initial stack frame below
    //
    curr_frame.src_pixel = left_src_pixel - fill_ptr->src_next_scan;
    curr_frame.fill_scan = fill_scan - fill_ptr->fill_next_scan;
    curr_frame.y         = y-1;
    curr_frame.dir       = -1;

    if(BFillFPush(fill_ptr->stack_ctrl, &curr_frame) == XIL_FAILURE) {
        return XIL_FAILURE;
    }

    //
    //  Begin fill loop
    //
    while(!(BFILL_STACK_EMPTY(fill_ptr->stack_ctrl))) {
        //
        //  Pop stack frame to process
        //
        if(BFillFPop(fill_ptr->stack_ctrl, &curr_frame) == XIL_FAILURE) {
           return XIL_FAILURE;
        }

        y = curr_frame.y;

        //
        //  Check y within bounds
        //
        if((y <= fill_ptr->ymax) && (y >= 0)) {
            //
            //  Check for previously filled segment
            //
            x = curr_frame.lx;
            fill_scan = curr_frame.fill_scan;

            //
            //  Here to check left
            //
            src_pixel = curr_frame.src_pixel;
            inside =
                (!(BFillFPixelBoundary(src_pixel,
                                       fill_ptr->bound,
                                       fill_ptr->nbands))) &&
                (!(BFILL_MASK_ON(fill_scan, x)));

            if(inside) {
                //
                //  Set pixel mask
                //
                BFILL_MASK_SET(fill_scan, x);

                //
                //  Continue left
                //
                x         = x-1;
                src_pixel = src_pixel - fill_ptr->src_next_pixel;

                //
                //  while INSIDE go left
                //
                while((x >= 0) &&
                      !(BFillFPixelBoundary(src_pixel,
                                            fill_ptr->bound,
                                            fill_ptr->nbands)) &&
                      (!(BFILL_MASK_ON(fill_scan, x)))) {
                    //
                    //  Set pixel mask
                    //
                    BFILL_MASK_SET(fill_scan, x);

                    //
                    //  Continue left
                    //
                    x = x-1;
                    src_pixel = src_pixel - fill_ptr->src_next_pixel;
                }

                //
                //  Save leftmost x coord
                //
                lx             = x + 1;
                left_src_pixel = src_pixel + fill_ptr->src_next_pixel;
            }

            //
            //  Here to check right
            //
            x = curr_frame.lx + 1;
            src_pixel = curr_frame.src_pixel + fill_ptr->src_next_pixel;

            //
            //  while WITHIN right limit
            //
            while(x <= fill_ptr->xmax) {

                if(inside) {
                    if(!(BFillFPixelBoundary(src_pixel,
                                             fill_ptr->bound,
                                             fill_ptr->nbands)) &&
                       (!(BFILL_MASK_ON(fill_scan, x)))) {
                        //
                        //  Still inside
                        //
                        BFILL_MASK_SET(fill_scan, x);
                    } else {
                        //
                        //  No longer inside
                        //
                        BFillFShadow(&curr_frame, fill_ptr,
                                     lx, left_src_pixel, (x-1));
                        inside = FALSE;
                    }
                } else {
                    if(x > curr_frame.rx) {
                        //
                        //  Not inside, and past shadow right
                        //
                        break;
                    }

                    if(!(BFillFPixelBoundary(src_pixel,
                                             fill_ptr->bound,
                                             fill_ptr->nbands)) &&
                       (!(BFILL_MASK_ON(fill_scan, x)))) {
                        //
                        //  Now inside, new scan
                        //
                        BFILL_MASK_SET(fill_scan, x);
                        inside = TRUE;
                        lx     = x;        // new leftmost x
                        left_src_pixel = src_pixel;
                    } else {
                        //
                        //  Still not inside!, no action
                        //
                    }
                }

                x = x+1;
                src_pixel = src_pixel + fill_ptr->src_next_pixel;
            }

            //
            //  Here to check edge of region
            //
            if(inside) {
                //
                //  Hit the edge of region while still inside scan
                //
                BFillFShadow(&curr_frame, fill_ptr, lx, left_src_pixel, (x-1));
            }
        }
    }

    return XIL_SUCCESS;
}

static
int
BFillFPush(bfillF_stack_ctrl*  ctrl_ptr, 
            bfillF_stack_frame* local_frame_ptr)
{
    int        idx;

    //
    //  Is current block full?
    //
    if(BFILL_STACK_BLOCK_FULL(ctrl_ptr)) {
        //
        //  No next block?
        //
        if(BFILL_STACK_NO_NEXT_BLOCK(ctrl_ptr)) {
            //
            //  Must allocate new stack block, curr block full
            //
            bfillF_stack_block* block = new bfillF_stack_block;
            if(block == NULL) {
                XIL_ERROR(NULL, XIL_ERROR_RESOURCE, "di-1", TRUE);
                return XIL_FAILURE;
            }
            ctrl_ptr->curr_block->next = block;
            block->prev                = ctrl_ptr->curr_block;
            block->next                = NULL;

            ctrl_ptr->curr_block = block;
            ctrl_ptr->curr_idx = 0;
        } else {
            //
            //  Next block already exists.
            //
            ctrl_ptr->curr_block = ctrl_ptr->curr_block->next;
            ctrl_ptr->curr_idx   = 0;
        }
    }

    idx = ctrl_ptr->curr_idx;

    //
    //  Save frame
    //
    ctrl_ptr->curr_block->frame[idx] = *local_frame_ptr;

    //
    //  Incr curr=> next avail
    //
    ctrl_ptr->curr_idx += 1;

    return XIL_SUCCESS;
}

static
int
BFillFPop(bfillF_stack_ctrl*  ctrl_ptr, 
           bfillF_stack_frame* local_frame_ptr)
{
    int        idx;

    //
    //  Popped last frame of block?
    //
    if(BFILL_STACK_BLOCK_EMPTY(ctrl_ptr)) {
        //
        //  No prev block?
        //
        if(BFILL_STACK_NO_PREV_BLOCK(ctrl_ptr)) {
            return XIL_FAILURE;
        } else {
            //
            //  Use prev block.
            //
            ctrl_ptr->curr_block = ctrl_ptr->curr_block->prev;
            ctrl_ptr->curr_idx = XILI_FILL_MAX_STACK_FRAMES;
        }
    }

    idx              = ctrl_ptr->curr_idx-1;

    //
    //  restore frame
    //
    *local_frame_ptr = ctrl_ptr->curr_block->frame[idx];

    //
    //  decr counter
    //
    ctrl_ptr->curr_idx = idx;

    return XIL_SUCCESS;
}

static
void
BFillFShadow(bfillF_stack_frame* frame,
              bfillF_data*        fill_ptr,
              int                 lx,
              Xil_float32*        pixel,
              int                 rx)
    
{
    bfillF_stack_frame new_frame;
    int                newy;
    Xil_float32*       new_pixel;
    Xil_unsigned8*     new_fill_scan;
    int                newlx;
    int                newrx;
    Xil_signed16       dir;

    //
    //  Create 3 shadow spans
    //
    //  1. span(curr.lx,curr.rx)     leftmost x <-> rightmost x
    //                               in the current direction
    //
    //  2. span(curr.prx+1,rx)       parent rightmost x + 1 <-> rightmost x
    //                               in the opposite direction (if needed)
    //
    //  3. span(curr.lx,curr.plx-1)  leftmost x <-> parent leftmost x - 1
    //                               in the opposite direction (if needed)

    //
    //  Always create (1).
    //
    dir = frame->dir;
    newy = frame->y + dir;
    new_pixel = pixel + (dir * fill_ptr->src_next_scan);
    new_fill_scan = frame->fill_scan + (dir * fill_ptr->fill_next_scan);

    new_frame.lx = lx;
    new_frame.rx = rx;
    new_frame.src_pixel = new_pixel;
    new_frame.fill_scan = new_fill_scan;
    new_frame.plx = lx - 1;
    new_frame.prx = rx + 1;
    new_frame.y = newy;
    new_frame.dir = dir;

    BFillFPush(fill_ptr->stack_ctrl, &new_frame);

    //
    //  Only create (2) if current span extends beyond parent rx
    //
    if(rx > frame->prx) {
        newlx = frame->prx+1;
        newy = frame->y - dir;
        new_pixel = pixel + (newlx - lx)*fill_ptr->src_next_pixel -
                        (dir * fill_ptr->src_next_scan);
        new_fill_scan = frame->fill_scan - (dir * fill_ptr->fill_next_scan);
        
        new_frame.lx = newlx;
            new_frame.src_pixel = new_pixel;
            new_frame.fill_scan = new_fill_scan;
            new_frame.y = newy;
        new_frame.dir = -dir;
            BFillFPush(fill_ptr->stack_ctrl, &new_frame);

    }

    //
    //  Only create (3) if current span begins before parent lx
    //
    if(lx < frame->plx) {
        newrx = frame->plx-1;
        newy = frame->y - dir;
        new_pixel = pixel - (dir * fill_ptr->src_next_scan);
        new_fill_scan = frame->fill_scan - (dir * fill_ptr->fill_next_scan);
        
        new_frame.lx = lx;
        new_frame.rx = newrx;
            new_frame.src_pixel = new_pixel;
            new_frame.fill_scan = new_fill_scan;
            new_frame.y = newy;
        new_frame.dir = -dir;

            BFillFPush(fill_ptr->stack_ctrl, &new_frame);
    }
}


//
//  Perform a boundary fill and place results in a bitmap.
//
//  This is for the General storage case.
//
static
int
BoundaryFillFG(int            x,
                int            y,
                XilStorage*    src_storage,
                Xil_unsigned8* fill_scan,
                bfillF_data*  fill_ptr,
                int            box_x,
                int            box_y)
{
    bfillF_stack_frame curr_frame;
    int                lx;
    int                rx;
    Xil_boolean        inside;

    //
    //  Check y within bounds
    //
    if((y <= fill_ptr->ymax) && (y >= 0)) {
        lx = x;

        //
        //  while INSIDE go left
        //
        while((lx >= 0) && !(BFillFPixelBoundaryG(src_storage, lx, y,
                                                  fill_ptr->bound,
                                                  fill_ptr->nbands,
                                                  box_x, box_y)) ) {
            //
            //  Set pixel mask
            //
            BFILL_MASK_SET(fill_scan, lx);

            //
            //  Continue left
            //
            lx = lx-1;
        }

        //
        //  Here to check right -- save leftmost
        //
        lx = lx + 1;
        rx = x + 1;

        //
        //  while INSIDE go right
        //
        while((rx <= fill_ptr->xmax) &&
              !(BFillFPixelBoundaryG(src_storage, rx, y,
                                     fill_ptr->bound,
                                     fill_ptr->nbands,
                                     box_x, box_y))) {
            //
            //  Set pixel mask
            //
            BFILL_MASK_SET(fill_scan, rx);

            //
            //  Continue right
            //
            rx = rx+1;

        }

        //
        //  Save rightmost x
        //
        rx = rx - 1;
    } else {
        //
        //  Not within y range
        //
        return XIL_FAILURE;
    }        

    //
    //  Push initial stack frame above
    //
    curr_frame.lx        = lx;
    curr_frame.rx        = rx;
    curr_frame.fill_scan = fill_scan + fill_ptr->fill_next_scan;
    curr_frame.plx       = lx-1;
    curr_frame.prx       = rx+1;
    curr_frame.y         = y+1;
    curr_frame.dir       = 1;

    if(BFillFPush(fill_ptr->stack_ctrl, &curr_frame) == XIL_FAILURE) {
        return XIL_FAILURE;
    }

    //
    //  Push initial stack frame below
    //
    curr_frame.fill_scan = fill_scan - fill_ptr->fill_next_scan;
    curr_frame.y         = y-1;
    curr_frame.dir       = -1;

    if(BFillFPush(fill_ptr->stack_ctrl, &curr_frame) == XIL_FAILURE) {
        return XIL_FAILURE;
    }

    //
    //  Begin fill loop
    //
    while(!(BFILL_STACK_EMPTY(fill_ptr->stack_ctrl))) {
        //
        //  Pop stack frame to process
        //
        if(BFillFPop(fill_ptr->stack_ctrl, &curr_frame) == XIL_FAILURE) {
           return XIL_FAILURE;
        }

        y = curr_frame.y;

        //
        //  Check y within bounds
        //
        if((y <= fill_ptr->ymax) && (y >= 0)) {
            //
            //  Check for previously filled segment
            //
            x = curr_frame.lx;
            fill_scan = curr_frame.fill_scan;

            //
            //  Here to check left
            //
            inside =
                (!(BFillFPixelBoundaryG(src_storage, x, y,
                                        fill_ptr->bound,
                                        fill_ptr->nbands,
                                        box_x, box_y))) &&
                (!(BFILL_MASK_ON(fill_scan, x)));

            if(inside) {
                //
                //  Set pixel mask
                //
                BFILL_MASK_SET(fill_scan, x);

                //
                //  Continue left
                //
                x = x-1;

                //
                //  while INSIDE go left
                //
                while((x >= 0) &&
                      !(BFillFPixelBoundaryG(src_storage, x, y,
                                             fill_ptr->bound,
                                             fill_ptr->nbands,
                                             box_x, box_y)) &&
                      (!(BFILL_MASK_ON(fill_scan, x)))) {
                    //
                    //  Set pixel mask
                    //
                    BFILL_MASK_SET(fill_scan, x);

                    //
                    //  Continue left
                    //
                    x = x-1;
                }

                //
                //  Save leftmost x coord
                //
                lx = x + 1;
            }

            //
            //  Here to check right
            //
            x = curr_frame.lx + 1;

            //
            //  while WITHIN right limit
            //
            while(x <= fill_ptr->xmax) {

                if(inside) {
                    if(!(BFillFPixelBoundaryG(src_storage, x, y,
                                              fill_ptr->bound,
                                              fill_ptr->nbands,
                                              box_x, box_y)) &&
                       (!(BFILL_MASK_ON(fill_scan, x)))) {
                        //
                        //  Still inside
                        //
                        BFILL_MASK_SET(fill_scan, x);
                    } else {
                        //
                        //  No longer inside
                        //
                        BFillFShadow(&curr_frame, fill_ptr, lx, NULL, (x-1));
                        inside = FALSE;
                    }
                } else {
                    if(x > curr_frame.rx) {
                        //
                        //  Not inside, and past shadow right
                        //
                        break;
                    }

                    if(!(BFillFPixelBoundaryG(src_storage, x, y,
                                              fill_ptr->bound,
                                              fill_ptr->nbands,
                                              box_x, box_y)) &&
                       (!(BFILL_MASK_ON(fill_scan, x)))) {
                        //
                        //  Now inside, new scan
                        //
                        BFILL_MASK_SET(fill_scan, x);
                        inside = TRUE;
                        lx     = x;        // new leftmost x
                    } else {
                        //
                        //  Still not inside!, no action
                        //
                    }
                }

                x = x+1;
            }

            //
            //  Here to check edge of region
            //
            if(inside) {
                //
                //  Hit the edge of region while still inside scan
                //
                BFillFShadow(&curr_frame, fill_ptr, lx, NULL, (x-1));
            }
        }
    }

    return XIL_SUCCESS;
}



//
//  Returns TRUE if pixel (x, y) of storage is a boundary pixel.
//
//  Boundary pixel passed in as "bound" is pixel sequential.
//
//  This is for the General storage case.
//
static
Xil_boolean
BFillFPixelBoundaryG(XilStorage*    storage,
                     unsigned int   x,
                     unsigned int   y,
                     Xil_float32*   bound,
                     unsigned int   nbands,
                     int            box_x,
                     int            box_y)
{
    for(unsigned int b = 0; b < nbands; b++) {
        //
        //  Compute pixel location.
        //
        //  Convert image space to box space coordinates.
        //
        unsigned int pixel_stride;
        unsigned int scanline_stride;
        Xil_float32*   data;
        storage->getStorageInfo(b,
                                &pixel_stride,
                                &scanline_stride,
                                NULL,
                                (void**) &data);
        Xil_float32* pixel = data + (y - box_y) * scanline_stride +
            (x - box_x) * pixel_stride;

        //
        //  Return FALSE at first mismatch
        //
        if(*pixel != *bound++) {
            return FALSE;
        }
    }

    return TRUE;
}
