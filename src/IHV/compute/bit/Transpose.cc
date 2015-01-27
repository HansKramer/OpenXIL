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
//  File:	Transpose.cc
//  Project:	XIL
//  Revision:	1.13
//  Last Mod:	10:09:41, 03/10/00
//
//
//  Description: 
//
//  Transpose() retrieves bit src and dst images, a fliptype and
//  ROIs.  It then "flips" the src image, around image center,
//  into the dst image in the following manner:
//
//  If "fliptype" = XIL_FLIP_X_AXIS - horizontal, across the X axis
//                  XIL_FLIP_Y_AXIS - vertical, across the Y axis
//                  XIL_FLIP_MAIN_DIAGONAL - across the main diagonal
//                  XIL_FLIP_ANTIDIAGONAL -  across the anti diagonal
//                  XIL_FLIP_90  - rotate counterclockwise 90 degress
//                  XIL_FLIP_180 - rotate counterclockwise 180 degress
//                  XIL_FLIP_270 - rotate counterclockwise 270 degress
//
//	
//  MT-level:  SAFE
//	
//------------------------------------------------------------------------
//	COPYRIGHT
//------------------------------------------------------------------------
#pragma ident	"@(#)Transpose.cc	1.13\t00/03/10  "

#include "XilDeviceManagerComputeBIT.hh"
#include "Transpose.hh"


XilStatus
XilDeviceManagerComputeBIT::Transpose(XilOp* op,
				      unsigned ,
				      XilRoi*      roi,
				      XilBoxList*  bl)
{
    //
    // Get the basic data, assuming that the src image is a BIT image.
    // roi is the complete image ROI. This means that src and dst ROI's are
    // already taken into account, and the roi passed is the intersection
    // of these. 
    //
    if(op->splitOnTileBoundaries(bl) == XIL_FAILURE) {
        return XIL_FAILURE;
    }

    //
    //  Get the images for our operation.
    //
    XilImage* src = op->getSrcImage(1);
    XilImage* dst = op->getDstImage(1);

    //
    //  Get the fliptype for the transpose operation
    //
    XilFlipType fliptype;
    op->getParam(1, (int *)&fliptype);
    
    unsigned int nbands = src->getNumBands();

    // 
    //  Loop over each of the boxes of storage we are to process.
    //
    XilBox* src_box;
    XilBox* dst_box;

    XilStorage src_storage(src);
    XilStorage dst_storage(dst);

    while(bl->getNext(&src_box, &dst_box)) {
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

	//
	//  Create a list of rectangles to loop over.  The resulting list
	//  of rectangles is the area created by intersecting the ROI with
	//  the destination box.
	//
	XilRectList rl(roi, dst_box);

	//
	//  Bit images have band sequential layout or general type of
	//  storage (bands are at random locations, not consecutive)
	//
	transpose_bit_region(op, src_storage, dst_storage, src_box, dst_box, rl,
			     fliptype, nbands);
        
    }

    return XIL_SUCCESS;
}

static void
transpose_bit_region(XilOp*        op,
		     XilStorage&   src_storage,
		     XilStorage&   dst_storage,
		     XilBox*       src_box,
		     XilBox*       dst_box,
		     XilRectList&  rl,
		     XilFlipType   fliptype,
		     unsigned int  nbands)
{
    //
    // src data
    //
    unsigned int    src_offset;
    unsigned int    src_scanline_stride;
    Xil_unsigned8*  src_data;
    int             srcR_x;
    int             srcR_y;	        // src region location 

    //
    // dst data
    //
    unsigned int    dst_offset;
    unsigned int    dst_scanline_stride;
    Xil_unsigned8*  dst_data;
    int             dstR_x;
    int             dstR_y;	        // dst region location 
    unsigned int    dstR_xsize;
    unsigned int    dstR_ysize;		// dst region size 


    //
    // loop over the list of rectangles
    //
    while(rl.getNext(&dstR_x, &dstR_y, &dstR_xsize, &dstR_ysize)) {
	    
	for(unsigned int band = 0; band < nbands; band++) {
	    //
	    // for each band, get image data
	    //
	    src_storage.getStorageInfo(band,
				       NULL,
				       &src_scanline_stride,
				       &src_offset,
				       (void**)&src_data);
	    dst_storage.getStorageInfo(band,
				       NULL,
				       &dst_scanline_stride,
				       &dst_offset,
				       (void**)&dst_data);

	    //
	    // The rectangle in the list applies to the dst, so we have
	    // to find appropriate rectangle in the src according to
	    // the fliptype. The op does the backward map for us.
	    //
	    // The op does the backward map for us.
	    //
            {
                double srcx;
                double srcy;
                op->backwardMap(dst_box, dstR_x, dstR_y,
                                src_box, &srcx, &srcy);

                srcR_x = (int)srcx;
                srcR_y = (int)srcy;
            }

	    //
	    // setup data pointers
	    // src and dst data are values with respect to the
	    // src and dst box. src_scanline and dst_scanline 
	    // point to the beginning pixel, where the 
	    // transposition should start taking place.
	    //
	    //
	    src_data += (srcR_y * src_scanline_stride);

	    dst_data += (dstR_y * dst_scanline_stride);

	    //
	    // since the core returns only the index
	    // of the starting pixel, we do rest of
	    // the work in the compute routine
	    // srcR_x is the INDEX of the starting pixel,
	    // in the src_box (which is therefore
	    // either 0 or srcR_xsize-1). src_offset
	    // is ptr to the first bit of the src_box.
	    // i.e. src_data + src_offset point to the
	    // pixel in the upper left corner of the src box.


	    switch (fliptype) {
	      case XIL_FLIP_X_AXIS:
		xil_flip_x_axis(src_data, dst_data,
				src_scanline_stride,
                                dst_scanline_stride,
				dstR_xsize,
                                dstR_ysize,
				src_offset,
                                dst_offset);
		break;

	      case XIL_FLIP_Y_AXIS:
		xil_flip_y_axis(src_data, dst_data,
				src_scanline_stride,
                                dst_scanline_stride,
				dstR_xsize,
                                dstR_ysize,
				src_offset + srcR_x + 1,
                                dst_offset);
		break;

	      case XIL_FLIP_MAIN_DIAGONAL:
	        xil_flip_main_diagonal(src_data, dst_data,
                                       src_scanline_stride,
                                       dst_scanline_stride,
                                       dstR_xsize,
                                       dstR_ysize,
                                       src_offset,
                                       dst_offset);
		break;

	      case XIL_FLIP_ANTIDIAGONAL:
		xil_flip_anti_diagonal(src_data, dst_data,
                                       src_scanline_stride,
                                       dst_scanline_stride,
                                       dstR_xsize,
                                       dstR_ysize,
                                       src_offset + srcR_x + 1,
                                       dst_offset);
		break;

	      case XIL_FLIP_90:
		xil_flip_90(src_data, dst_data,
			    src_scanline_stride,
                            dst_scanline_stride,
			    dstR_xsize,
                            dstR_ysize,
			    src_offset + srcR_x + 1,
                            dst_offset);		
		break;

	      case XIL_FLIP_180:
		xil_flip_180(src_data, dst_data,
                             src_scanline_stride,
                             dst_scanline_stride,
                             dstR_xsize,
                             dstR_ysize,
                             src_offset + srcR_x + 1,
                             dst_offset);
		break;
		    
	      case XIL_FLIP_270:
		xil_flip_270(src_data, dst_data,
                             src_scanline_stride,
                             dst_scanline_stride,
                             dstR_xsize,
                             dstR_ysize,
                             src_offset,
                             dst_offset);		
		break;
	    }
	}
    }
}

//
// Given pointer to the src data and pointer to dst data
// writes down  region of 16x16 bits to dst in inverted
// order (upside down). There are 8 integers used as
// registers to save the intermediate data. Other functions
// blk_flip_xxx are similar in nature to this function,
// but perform transformation of the block around other axis.
//

static void
blk_flip_x(Xil_unsigned8* src_blk,
	   Xil_unsigned8* dst,
	   unsigned int   offset,
	   unsigned int   src_stride,
	   unsigned int   dst_stride)
{
    unsigned int i1, i2, i3, i4,
	         i5, i6, i7, i8;
    unsigned int up, down;

    if (offset) {
	LOAD_REG_UNALIGNED(i1);
	LOAD_REG_UNALIGNED(i2);
	LOAD_REG_UNALIGNED(i3);
	LOAD_REG_UNALIGNED(i4);
	LOAD_REG_UNALIGNED(i5);
	LOAD_REG_UNALIGNED(i6);
	LOAD_REG_UNALIGNED(i7);
	LOAD_REG_UNALIGNED(i8);
    } else {
	LOAD_REG(i1);
	LOAD_REG(i2);
	LOAD_REG(i3);
	LOAD_REG(i4);
	LOAD_REG(i5);
	LOAD_REG(i6);
	LOAD_REG(i7);
	LOAD_REG(i8);
    }

    up = 0x0000ffff;
    down = up << 16;
    //
    // transpose two by two src lines
    //
    i1 = ((i1 & down) >> 16) | ((i1 & up) << 16);
    i2 = ((i2 & down) >> 16) | ((i2 & up) << 16);
    i3 = ((i3 & down) >> 16) | ((i3 & up) << 16);
    i4 = ((i4 & down) >> 16) | ((i4 & up) << 16);
    i5 = ((i5 & down) >> 16) | ((i5 & up) << 16);
    i6 = ((i6 & down) >> 16) | ((i6 & up) << 16);
    i7 = ((i7 & down) >> 16) | ((i7 & up) << 16);
    i8 = ((i8 & down) >> 16) | ((i8 & up) << 16);
    //
    // order in which you store these registers
    // reflects type of transformation
    //
    STORE_REG(i8);
    STORE_REG(i7);
    STORE_REG(i6);
    STORE_REG(i5);
    STORE_REG(i4);
    STORE_REG(i3);
    STORE_REG(i2);
    STORE_REG(i1);
}
//
// This one is more tricky, since we have to transpose
// block vertically. The idea behind this one is that
// if you keep dividing the block on 1/2 (vertical
// stripes) and than transposing them (fliping) you
// finish with transposed block.
//
static void
blk_flip_y(Xil_unsigned8* src_blk,
	   Xil_unsigned8* dst,
	   unsigned int   offset,
	   unsigned int   src_stride,
	   unsigned int   dst_stride)
{
    unsigned int i1, i2, i3, i4,
	         i5, i6, i7, i8;
    unsigned int up, down;

    if (offset) {
	LOAD_REG_UNALIGNED(i1);
	LOAD_REG_UNALIGNED(i2);
	LOAD_REG_UNALIGNED(i3);
	LOAD_REG_UNALIGNED(i4);
	LOAD_REG_UNALIGNED(i5);
	LOAD_REG_UNALIGNED(i6);
	LOAD_REG_UNALIGNED(i7);
	LOAD_REG_UNALIGNED(i8);
    }
    else {
	LOAD_REG(i1);
	LOAD_REG(i2);
	LOAD_REG(i3);
	LOAD_REG(i4);
	LOAD_REG(i5);
	LOAD_REG(i6);
	LOAD_REG(i7);
	LOAD_REG(i8);
    }

    up = 0x55555555;
    down = up << 1;
    i1 = ((i1 & down) >> 1) | ((i1 & up) << 1);
    i2 = ((i2 & down) >> 1) | ((i2 & up) << 1);
    i3 = ((i3 & down) >> 1) | ((i3 & up) << 1);
    i4 = ((i4 & down) >> 1) | ((i4 & up) << 1);
    i5 = ((i5 & down) >> 1) | ((i5 & up) << 1);
    i6 = ((i6 & down) >> 1) | ((i6 & up) << 1);
    i7 = ((i7 & down) >> 1) | ((i7 & up) << 1);
    i8 = ((i8 & down) >> 1) | ((i8 & up) << 1);

    up = 0x33333333;
    down = up << 2;
    i1 = ((i1 & down) >> 2) | ((i1 & up) << 2);
    i2 = ((i2 & down) >> 2) | ((i2 & up) << 2);
    i3 = ((i3 & down) >> 2) | ((i3 & up) << 2);
    i4 = ((i4 & down) >> 2) | ((i4 & up) << 2);
    i5 = ((i5 & down) >> 2) | ((i5 & up) << 2);
    i6 = ((i6 & down) >> 2) | ((i6 & up) << 2);
    i7 = ((i7 & down) >> 2) | ((i7 & up) << 2);
    i8 = ((i8 & down) >> 2) | ((i8 & up) << 2);

    up = 0x0f0f0f0f;
    down = up << 4;
    i1 = ((i1 & down) >> 4) | ((i1 & up) << 4);
    i2 = ((i2 & down) >> 4) | ((i2 & up) << 4);
    i3 = ((i3 & down) >> 4) | ((i3 & up) << 4);
    i4 = ((i4 & down) >> 4) | ((i4 & up) << 4);
    i5 = ((i5 & down) >> 4) | ((i5 & up) << 4);
    i6 = ((i6 & down) >> 4) | ((i6 & up) << 4);
    i7 = ((i7 & down) >> 4) | ((i7 & up) << 4);
    i8 = ((i8 & down) >> 4) | ((i8 & up) << 4);

    up = 0x00ff00ff;
    down = up << 8;
    i1 = ((i1 & down) >> 8) | ((i1 & up) << 8);
    i2 = ((i2 & down) >> 8) | ((i2 & up) << 8);
    i3 = ((i3 & down) >> 8) | ((i3 & up) << 8);
    i4 = ((i4 & down) >> 8) | ((i4 & up) << 8);
    i5 = ((i5 & down) >> 8) | ((i5 & up) << 8);
    i6 = ((i6 & down) >> 8) | ((i6 & up) << 8);
    i7 = ((i7 & down) >> 8) | ((i7 & up) << 8);
    i8 = ((i8 & down) >> 8) | ((i8 & up) << 8);

    STORE_REG(i1);
    STORE_REG(i2);
    STORE_REG(i3);
    STORE_REG(i4);
    STORE_REG(i5);
    STORE_REG(i6);
    STORE_REG(i7);
    STORE_REG(i8);
}

static void
blk_flip_d(Xil_unsigned8* src_blk,
           Xil_unsigned8* dst,
	   unsigned int   offset,
	   unsigned int   src_stride,
	   unsigned int   dst_stride)
{
    unsigned int i1, i2, i3, i4, i5, i6, i7, i8;
    unsigned int t1, t2, t3, t4, t5, t6, t7, t8;
    unsigned int fix;
    unsigned int up, down;

    if (offset) {
	LOAD_REG_UNALIGNED(i1);
	LOAD_REG_UNALIGNED(i2);
	LOAD_REG_UNALIGNED(i3);
	LOAD_REG_UNALIGNED(i4);
	LOAD_REG_UNALIGNED(i5);
	LOAD_REG_UNALIGNED(i6);
	LOAD_REG_UNALIGNED(i7);
	LOAD_REG_UNALIGNED(i8);
    }
    else {
	LOAD_REG(i1);
	LOAD_REG(i2);
	LOAD_REG(i3);
	LOAD_REG(i4);
	LOAD_REG(i5);
	LOAD_REG(i6);
	LOAD_REG(i7);
	LOAD_REG(i8);
    }

    up = 0x55550000;
    down = up >> 15;
    fix = ~(up | down);
    i1 = ((i1 & down) << 15) | ((i1 & up) >> 15) | (i1 & fix);
    i2 = ((i2 & down) << 15) | ((i2 & up) >> 15) | (i2 & fix);
    i3 = ((i3 & down) << 15) | ((i3 & up) >> 15) | (i3 & fix);
    i4 = ((i4 & down) << 15) | ((i4 & up) >> 15) | (i4 & fix);
    i5 = ((i5 & down) << 15) | ((i5 & up) >> 15) | (i5 & fix);
    i6 = ((i6 & down) << 15) | ((i6 & up) >> 15) | (i6 & fix);
    i7 = ((i7 & down) << 15) | ((i7 & up) >> 15) | (i7 & fix);
    i8 = ((i8 & down) << 15) | ((i8 & up) >> 15) | (i8 & fix);

    up = 0x33333333;
    down = up << 2;
    t1 = ((i2 & down) >> 2) | (i1 & down);
    t2 = ((i1 & up) << 2) | (i2 & up);
    t3 = ((i4 & down) >> 2) | (i3 & down);
    t4 = ((i3 & up) << 2) | (i4 & up);
    t5 = ((i6 & down) >> 2) | (i5 & down);
    t6 = ((i5 & up) << 2) | (i6 & up);
    t7 = ((i8 & down) >> 2) | (i7 & down);
    t8 = ((i7 & up) << 2) | (i8 & up);

    up = 0x0f0f0f0f;
    down = up << 4;
    i1 = ((t3 & down) >> 4) | (t1 & down);
    i2 = ((t4 & down) >> 4) | (t2 & down);
    i3 = ((t1 & up) << 4) | (t3 & up);
    i4 = ((t2 & up) << 4) | (t4 & up);
    i5 = ((t7 & down) >> 4) | (t5 & down);
    i6 = ((t8 & down) >> 4) | (t6 & down);
    i7 = ((t5 & up) << 4) | (t7 & up);
    i8 = ((t6 & up) << 4) | (t8 & up);

    up = 0x00ff00ff;
    down = up << 8;
    t1 = ((i5 & down) >> 8) | (i1 & down);
    t2 = ((i6 & down) >> 8) | (i2 & down);
    t3 = ((i7 & down) >> 8) | (i3 & down);
    t4 = ((i8 & down) >> 8) | (i4 & down);
    t5 = ((i1 & up) << 8) | (i5 & up);
    t6 = ((i2 & up) << 8) | (i6 & up);
    t7 = ((i3 & up) << 8) | (i7 & up);
    t8 = ((i4 & up) << 8) | (i8 & up);

    STORE_REG(t1);
    STORE_REG(t2);
    STORE_REG(t3);
    STORE_REG(t4);
    STORE_REG(t5);
    STORE_REG(t6);
    STORE_REG(t7);
    STORE_REG(t8);
}

static void
blk_flip_a(Xil_unsigned8* src_blk,
	   Xil_unsigned8* dst,
	   unsigned int   offset,
	   unsigned int   src_stride,
	   unsigned int   dst_stride)
{
    unsigned int i1, i2, i3, i4, i5, i6, i7, i8;
    unsigned int t1, t2, t3, t4, t5, t6, t7, t8;
    unsigned int fix;
    unsigned int up, down;

    if (offset) {
	LOAD_REG_UNALIGNED(i1);
	LOAD_REG_UNALIGNED(i2);
	LOAD_REG_UNALIGNED(i3);
	LOAD_REG_UNALIGNED(i4);
	LOAD_REG_UNALIGNED(i5);
	LOAD_REG_UNALIGNED(i6);
	LOAD_REG_UNALIGNED(i7);
	LOAD_REG_UNALIGNED(i8);
    }
    else {
	LOAD_REG(i1);
	LOAD_REG(i2);
	LOAD_REG(i3);
	LOAD_REG(i4);
	LOAD_REG(i5);
	LOAD_REG(i6);
	LOAD_REG(i7);
	LOAD_REG(i8);
    }

    down = 0x00005555;
    up = down << 17;
    fix = ~(up | down);
    i1 = ((i1 & down) << 17) | ((i1 & up) >> 17) | (i1 & fix);
    i2 = ((i2 & down) << 17) | ((i2 & up) >> 17) | (i2 & fix);
    i3 = ((i3 & down) << 17) | ((i3 & up) >> 17) | (i3 & fix);
    i4 = ((i4 & down) << 17) | ((i4 & up) >> 17) | (i4 & fix);
    i5 = ((i5 & down) << 17) | ((i5 & up) >> 17) | (i5 & fix);
    i6 = ((i6 & down) << 17) | ((i6 & up) >> 17) | (i6 & fix);
    i7 = ((i7 & down) << 17) | ((i7 & up) >> 17) | (i7 & fix);
    i8 = ((i8 & down) << 17) | ((i8 & up) >> 17) | (i8 & fix); 

    down = 0x33333333;
    up = down << 2;
    t1 = ((i2 & down) << 2) | (i1 & down);
    t2 = ((i1 & up) >> 2) | (i2 & up);
    t3 = ((i4 & down) << 2) | (i3 & down);
    t4 = ((i3 & up) >> 2) | (i4 & up);
    t5 = ((i6 & down) << 2) | (i5 & down);
    t6 = ((i5 & up) >> 2) | (i6 & up);
    t7 = ((i8 & down) << 2) | (i7 & down);
    t8 = ((i7 & up) >> 2) | (i8 & up);

    down = 0x0f0f0f0f;
    up = down << 4;
    i1 = ((t3 & down) << 4) | (t1 & down);
    i2 = ((t4 & down) << 4) | (t2 & down);
    i3 = ((t1 & up) >> 4) | (t3 & up);
    i4 = ((t2 & up) >> 4) | (t4 & up);
    i5 = ((t7 & down) << 4) | (t5 & down);
    i6 = ((t8 & down) << 4) | (t6 & down);
    i7 = ((t5 & up) >> 4) | (t7 & up);
    i8 = ((t6 & up) >> 4) | (t8 & up);

    down = 0x00ff00ff;
    up = down << 8;
    t1 = ((i5 & down) << 8) | (i1 & down);
    t2 = ((i6 & down) << 8) | (i2 & down);
    t3 = ((i7 & down) << 8) | (i3 & down);
    t4 = ((i8 & down) << 8) | (i4 & down);
    t5 = ((i1 & up) >> 8) | (i5 & up);
    t6 = ((i2 & up) >> 8) | (i6 & up);
    t7 = ((i3 & up) >> 8) | (i7 & up);
    t8 = ((i4 & up) >> 8) | (i8 & up);

    STORE_REG(t1);
    STORE_REG(t2);
    STORE_REG(t3);
    STORE_REG(t4);
    STORE_REG(t5);
    STORE_REG(t6);
    STORE_REG(t7);
    STORE_REG(t8);
}

static void
blk_flip_270(Xil_unsigned8* src_blk,
	     Xil_unsigned8* dst,
	     unsigned int   offset,
	     unsigned int   src_stride,
	     unsigned int   dst_stride)
{
    unsigned int i1, i2, i3, i4, i5, i6, i7, i8;
    unsigned int t1, t2, t3, t4, t5, t6, t7, t8;
    unsigned int up, down, left, right;

    if (offset) {
	LOAD_REG_UNALIGNED(i1);
	LOAD_REG_UNALIGNED(i2);
	LOAD_REG_UNALIGNED(i3);
	LOAD_REG_UNALIGNED(i4);
	LOAD_REG_UNALIGNED(i5);
	LOAD_REG_UNALIGNED(i6);
	LOAD_REG_UNALIGNED(i7);
	LOAD_REG_UNALIGNED(i8);
    }
    else {
	LOAD_REG(i1);
	LOAD_REG(i2);
	LOAD_REG(i3);
	LOAD_REG(i4);
	LOAD_REG(i5);
	LOAD_REG(i6);
	LOAD_REG(i7);
	LOAD_REG(i8);
    }

    left = 0x00005555;
    up = left << 1;
    down = left << 16;
    right = left << 17;
    i1 = ((i1 & left) << 1)  | ((i1 & up) << 16) |
	((i1 & right) >> 1) | ((i1 & down) >> 16);
    i2 = ((i2 & left) << 1)  | ((i2 & up) << 16) |
	((i2 & right) >> 1) | ((i2 & down) >> 16);
    i3 = ((i3 & left) << 1)  | ((i3 & up) << 16) |
	((i3 & right) >> 1) | ((i3 & down) >> 16);
    i4 = ((i4 & left) << 1)  | ((i4 & up) << 16) |
	((i4 & right) >> 1) | ((i4 & down) >> 16);
    i5 = ((i5 & left) << 1)  | ((i5 & up) << 16) |
	((i5 & right) >> 1) | ((i5 & down) >> 16);
    i6 = ((i6 & left) << 1)  | ((i6 & up) << 16) |
	((i6 & right) >> 1) | ((i6 & down) >> 16);
    i7 = ((i7 & left) << 1)  | ((i7 & up) << 16) |
	((i7 & right) >> 1) | ((i7 & down) >> 16);
    i8 = ((i8 & left) << 1)  | ((i8 & up) << 16) |
	((i8 & right) >> 1) | ((i8 & down) >> 16);

    left = 0x33333333;
    right = left << 2;
    t1 = ((i1 & right) >> 2) | (i2 & right);
    t2 = ((i2 & left) << 2)  | (i1 & left);
    t3 = ((i3 & right) >> 2) | (i4 & right);
    t4 = ((i4 & left) << 2)  | (i3 & left);
    t5 = ((i5 & right) >> 2) | (i6 & right);
    t6 = ((i6 & left) << 2)  | (i5 & left);
    t7 = ((i7 & right) >> 2) | (i8 & right);
    t8 = ((i8 & left) << 2)  | (i7 & left);

    left = 0x0f0f0f0f;
    right = left << 4;
    i1 = (t3 & right) | ((t1 & right) >> 4);
    i2 = (t4 & right) | ((t2 & right) >> 4);
    i3 = (t1 & left)  | ((t3 & left) << 4);
    i4 = (t2 & left)  | ((t4 & left) << 4);
    i5 = (t7 & right) | ((t5 & right) >> 4);
    i6 = (t8 & right) | ((t6 & right) >> 4);
    i7 = (t5 & left)  | ((t7 & left) << 4);
    i8 = (t6 & left)  | ((t8 & left) << 4);

    left = 0x00ff00ff;
    right = left << 8;
    t1 = (i5 & right) | ((i1 & right) >> 8);
    t2 = (i6 & right) | ((i2 & right) >> 8);
    t3 = (i7 & right) | ((i3 & right) >> 8);
    t4 = (i8 & right) | ((i4 & right) >> 8);
    t5 = (i1 & left)  | ((i5 & left) << 8);
    t6 = (i2 & left)  | ((i6 & left) << 8);
    t7 = (i3 & left)  | ((i7 & left) << 8);
    t8 = (i4 & left)  | ((i8 & left) << 8);

    STORE_REG(t1);
    STORE_REG(t2);
    STORE_REG(t3);
    STORE_REG(t4);
    STORE_REG(t5);
    STORE_REG(t6);
    STORE_REG(t7);
    STORE_REG(t8);
}


static void
blk_flip_180(Xil_unsigned8* src_blk,
	     Xil_unsigned8* dst,
	     unsigned int   offset,
	     unsigned int   src_stride,
	     unsigned int   dst_stride)
{
    unsigned int i1, i2, i3, i4, i5, i6, i7, i8;
    unsigned int up, down;

    if (offset) {
	LOAD_REG_UNALIGNED(i1);
	LOAD_REG_UNALIGNED(i2);
	LOAD_REG_UNALIGNED(i3);
	LOAD_REG_UNALIGNED(i4);
	LOAD_REG_UNALIGNED(i5);
	LOAD_REG_UNALIGNED(i6);
	LOAD_REG_UNALIGNED(i7);
	LOAD_REG_UNALIGNED(i8);
    }
    else {
	LOAD_REG(i1);
	LOAD_REG(i2);
	LOAD_REG(i3);
	LOAD_REG(i4);
	LOAD_REG(i5);
	LOAD_REG(i6);
	LOAD_REG(i7);
	LOAD_REG(i8);
    }

    up = 0x55555555;
    down = up << 1;
    i1 = ((i1 & down) >> 1) | ((i1 & up) << 1);
    i2 = ((i2 & down) >> 1) | ((i2 & up) << 1);
    i3 = ((i3 & down) >> 1) | ((i3 & up) << 1);
    i4 = ((i4 & down) >> 1) | ((i4 & up) << 1);
    i5 = ((i5 & down) >> 1) | ((i5 & up) << 1);
    i6 = ((i6 & down) >> 1) | ((i6 & up) << 1);
    i7 = ((i7 & down) >> 1) | ((i7 & up) << 1);
    i8 = ((i8 & down) >> 1) | ((i8 & up) << 1);

    up = 0x33333333;
    down = up << 2;
    i1 = ((i1 & down) >> 2) | ((i1 & up) << 2);
    i2 = ((i2 & down) >> 2) | ((i2 & up) << 2);
    i3 = ((i3 & down) >> 2) | ((i3 & up) << 2);
    i4 = ((i4 & down) >> 2) | ((i4 & up) << 2);
    i5 = ((i5 & down) >> 2) | ((i5 & up) << 2);
    i6 = ((i6 & down) >> 2) | ((i6 & up) << 2);
    i7 = ((i7 & down) >> 2) | ((i7 & up) << 2);
    i8 = ((i8 & down) >> 2) | ((i8 & up) << 2);

    up = 0x0f0f0f0f;
    down = up << 4;
    i1 = ((i1 & down) >> 4) | ((i1 & up) << 4);
    i2 = ((i2 & down) >> 4) | ((i2 & up) << 4);
    i3 = ((i3 & down) >> 4) | ((i3 & up) << 4);
    i4 = ((i4 & down) >> 4) | ((i4 & up) << 4);
    i5 = ((i5 & down) >> 4) | ((i5 & up) << 4);
    i6 = ((i6 & down) >> 4) | ((i6 & up) << 4);
    i7 = ((i7 & down) >> 4) | ((i7 & up) << 4);
    i8 = ((i8 & down) >> 4) | ((i8 & up) << 4);

    up = 0x00ff00ff;
    down = up << 8;
    i1 = ((i1 & down) >> 8) | ((i1 & up) << 8);
    i2 = ((i2 & down) >> 8) | ((i2 & up) << 8);
    i3 = ((i3 & down) >> 8) | ((i3 & up) << 8);
    i4 = ((i4 & down) >> 8) | ((i4 & up) << 8);
    i5 = ((i5 & down) >> 8) | ((i5 & up) << 8);
    i6 = ((i6 & down) >> 8) | ((i6 & up) << 8);
    i7 = ((i7 & down) >> 8) | ((i7 & up) << 8);
    i8 = ((i8 & down) >> 8) | ((i8 & up) << 8);

    up = 0x0000ffff;
    down = up << 16;
    i1 = ((i1 & down) >> 16) | ((i1 & up) << 16);
    i2 = ((i2 & down) >> 16) | ((i2 & up) << 16);
    i3 = ((i3 & down) >> 16) | ((i3 & up) << 16);
    i4 = ((i4 & down) >> 16) | ((i4 & up) << 16);
    i5 = ((i5 & down) >> 16) | ((i5 & up) << 16);
    i6 = ((i6 & down) >> 16) | ((i6 & up) << 16);
    i7 = ((i7 & down) >> 16) | ((i7 & up) << 16);
    i8 = ((i8 & down) >> 16) | ((i8 & up) << 16);

    STORE_REG(i8);
    STORE_REG(i7);
    STORE_REG(i6);
    STORE_REG(i5);
    STORE_REG(i4);
    STORE_REG(i3);
    STORE_REG(i2);
    STORE_REG(i1);
}
//
// yet another trick from graphics:
// use recursive relations on 16x16 image, and divide image
// on half in both directions. Rotate each of the subimages
// as parts for 90 degrees counterclockwise. Repeat procedure
// on subimages until you finish with 1x1 image.The resulting
// image is rotated image.
//
static void
blk_flip_90(Xil_unsigned8* src_blk,
	    Xil_unsigned8* dst,
	    unsigned int   offset,
	    unsigned int   src_stride,
	    unsigned int   dst_stride)
{
    unsigned int i1, i2, i3, i4, i5, i6, i7, i8;
    unsigned int t1, t2, t3, t4, t5, t6, t7, t8;
    unsigned int up, down, left, right;

    if (offset) {
	LOAD_REG_UNALIGNED(i1);
	LOAD_REG_UNALIGNED(i2);
	LOAD_REG_UNALIGNED(i3);
	LOAD_REG_UNALIGNED(i4);
	LOAD_REG_UNALIGNED(i5);
	LOAD_REG_UNALIGNED(i6);
	LOAD_REG_UNALIGNED(i7);
	LOAD_REG_UNALIGNED(i8);
    }
    else {
	LOAD_REG(i1);
	LOAD_REG(i2);
	LOAD_REG(i3);
	LOAD_REG(i4);
	LOAD_REG(i5);
	LOAD_REG(i6);
	LOAD_REG(i7);
	LOAD_REG(i8);
    }

    up = 0x00005555;
    right = up << 1;
    left = up << 16;
    down = up << 17;
    i1 = ((i1 & left) << 1)  | ((i1 & up) << 16) |
	((i1 & right) >> 1) | ((i1 & down) >> 16);
    i2 = ((i2 & left) << 1)  | ((i2 & up) << 16) |
	((i2 & right) >> 1) | ((i2 & down) >> 16);
    i3 = ((i3 & left) << 1)  | ((i3 & up) << 16) |
	((i3 & right) >> 1) | ((i3 & down) >> 16);
    i4 = ((i4 & left) << 1)  | ((i4 & up) << 16) |
	((i4 & right) >> 1) | ((i4 & down) >> 16);
    i5 = ((i5 & left) << 1)  | ((i5 & up) << 16) |
	((i5 & right) >> 1) | ((i5 & down) >> 16);
    i6 = ((i6 & left) << 1)  | ((i6 & up) << 16) |
	((i6 & right) >> 1) | ((i6 & down) >> 16);
    i7 = ((i7 & left) << 1)  | ((i7 & up) << 16) |
	((i7 & right) >> 1) | ((i7 & down) >> 16);
    i8 = ((i8 & left) << 1)  | ((i8 & up) << 16) |
	((i8 & right) >> 1) | ((i8 & down) >> 16); 

    left = 0x33333333;
    right = left << 2;
    t1 = ((i1 & left) << 2)  | (i2 & left);
    t2 = ((i2 & right) >> 2) | (i1 & right);
    t3 = ((i3 & left) << 2)  | (i4 & left);
    t4 = ((i4 & right) >> 2) | (i3 & right);
    t5 = ((i5 & left) << 2)  | (i6 & left);
    t6 = ((i6 & right) >> 2) | (i5 & right);
    t7 = ((i7 & left) << 2)  | (i8 & left);
    t8 = ((i8 & right) >> 2) | (i7 & right);

    left = 0x0f0f0f0f;
    right = left << 4;
    i1 = (t3 & left) | ((t1 & left) << 4);
    i2 = (t4 & left) | ((t2 & left) << 4);
    i3 = (t1 & right)  | ((t3 & right) >> 4);
    i4 = (t2 & right)  | ((t4 & right) >> 4);
    i5 = (t7 & left) | ((t5 & left) << 4);
    i6 = (t8 & left) | ((t6 & left) << 4);
    i7 = (t5 & right)  | ((t7 & right) >> 4);
    i8 = (t6 & right)  | ((t8 & right) >> 4);

    left = 0x00ff00ff;
    right = left << 8;
    t1 = (i5 & left) | ((i1 & left) << 8);
    t2 = (i6 & left) | ((i2 & left) << 8);
    t3 = (i7 & left) | ((i3 & left) << 8);
    t4 = (i8 & left) | ((i4 & left) << 8);
    t5 = (i1 & right)  | ((i5 & right) >> 8);
    t6 = (i2 & right)  | ((i6 & right) >> 8);
    t7 = (i3 & right)  | ((i7 & right) >> 8);
    t8 = (i4 & right)  | ((i8 & right) >> 8);

    STORE_REG(t1);
    STORE_REG(t2);
    STORE_REG(t3);
    STORE_REG(t4);
    STORE_REG(t5);
    STORE_REG(t6);
    STORE_REG(t7);
    STORE_REG(t8);
}
//
// The hart of the procedure is blk_flip_x that rotates a 16x16
// block. The rest supports locating the blocks of that size in
// the image and dealing with the edge conditions.
//

static void
xil_flip_x_axis(Xil_unsigned8*  src_pixel,
                Xil_unsigned8*  dst_pixel,		// ptr to current pixel
                unsigned int    src_stride,
                unsigned int    dst_stride,	    
                unsigned int    dstR_xsize,
                unsigned int    dstR_ysize,		// dst region size 
                int		src_offset,
                int 	        dst_offset          // location of bit
	    )
{
    unsigned int i = 0;
    unsigned int j = 0;
    unsigned int    endblocks;
    Xil_unsigned8* src_blk;
    Xil_unsigned8* dst_blk;
    
    while((i>>4) < (dstR_ysize>>4)) {
	j = 0;
	while (((dst_offset + j) % 8) && (j < dstR_xsize)) {
	    if (XIL_BMAP_TST(src_pixel, src_offset + j)) {
		XIL_BMAP_SET(dst_pixel, dst_offset + j);
	    } else {
		XIL_BMAP_CLR(dst_pixel, dst_offset + j);
	    }
	    j++;
	}
	if (i%16 == 0) {
	    while ((j+15) < dstR_xsize) {
		src_blk = src_pixel + (src_offset + j)/8 - (src_stride << 4) + src_stride;
		dst_blk = dst_pixel + (dst_offset + j)/8;
		blk_flip_x(src_blk, dst_blk, (src_offset + j)%8, src_stride, dst_stride );
		j += 16;
	    }
	    endblocks = j;
	} else {
	    j = endblocks;
	}
	while (j < dstR_xsize) {
	    if (XIL_BMAP_TST(src_pixel, src_offset + j)) {
		XIL_BMAP_SET(dst_pixel, dst_offset + j);
	    }
	    else {
		XIL_BMAP_CLR(dst_pixel, dst_offset + j);
	    }
	    j++;
	}
	i++;
	src_pixel -= src_stride;
	dst_pixel += dst_stride;
    }
    while(i++ < dstR_ysize) {
	for (j = 0; j < dstR_xsize; j++) {
	    if (XIL_BMAP_TST(src_pixel, src_offset + j)) {
		XIL_BMAP_SET(dst_pixel, dst_offset + j);
	    } else {
		XIL_BMAP_CLR(dst_pixel, dst_offset + j);
	    }
	}
	src_pixel -= src_stride;
	dst_pixel += dst_stride;
    }

}

static void
xil_flip_y_axis(Xil_unsigned8*  src_pixel,
                Xil_unsigned8*  dst_pixel,		// ptr to current pixel
                unsigned int    src_stride,
                unsigned int    dst_stride,
                unsigned int    dstR_xsize,
                unsigned int    dstR_ysize,		// dst region size 
                int		src_offset,         // ptr to first bit in src
                int 	        dst_offset          // ptr to first bit in dst
	    )
{
    //
    // We start from the top right corner and walk backwards
    //
    unsigned int i = 0;		// pointer to the current row
    unsigned int j = 0;		// pointer to the current column
    unsigned int endblocks;
    //
    // Transposing 16 x 16 blocks except at the edges
    //
    while((i>>4) < (dstR_ysize>>4)) {
	//
	// Transpose the end area until you get to the first byte
        // boundary in destination. Walk from right to left.
	// After that start transposing 16x16 bit regions. Finally
	// transpose a smaller region.
	//
	
	j = 0;		              	// ptr to the dst bit
	//
	// While you do not reach byte boundary in destination,
	// transpose src bits.
	//
	while (((dst_offset + j) % 8) && (j < dstR_xsize)){
	    if (XIL_BMAP_TST(src_pixel, src_offset - j - 1)) {
		XIL_BMAP_SET(dst_pixel, dst_offset + j);
	    }
	    else {
		XIL_BMAP_CLR(dst_pixel, dst_offset + j);
	    }
	    j++;
	}
	//
	// do only every so often the whole 16x16  block transpose
	// 
	if (i%16 == 0) {
	    while ((j+15) < dstR_xsize) {
                unsigned int right_src_offset = src_offset - j;

		blk_flip_y(src_pixel + right_src_offset/8 - 2,
			   dst_pixel + (dst_offset + j)/8,
			   right_src_offset % 8,
                           src_stride, dst_stride);
		j += 16;
 	    }
	    endblocks = j;
	}
	else
	    j = endblocks;
	//
	// finish up the rest of all the other bits in a smaller than
	// 16x16 block
	//
	while (j < dstR_xsize) {
	    if (XIL_BMAP_TST(src_pixel, src_offset - j - 1)) {
		XIL_BMAP_SET(dst_pixel, dst_offset + j);
	    }
	    else {
		XIL_BMAP_CLR(dst_pixel, dst_offset + j);
	    }
	    j++;
	}
	i++;
	src_pixel += src_stride;
	dst_pixel += dst_stride;
    }
    //
    // finish up transposing for the rest of the image in y direction
    //
    while(i++ < dstR_ysize) {
	for (j = 0; j < dstR_xsize; j++) {
	    if (XIL_BMAP_TST(src_pixel, src_offset - j - 1)) {
		XIL_BMAP_SET(dst_pixel, dst_offset + j);
	    } else {
		XIL_BMAP_CLR(dst_pixel, dst_offset + j);
	    }
	}
	src_pixel += src_stride;
	dst_pixel += dst_stride;
    }
}

static void
xil_flip_main_diagonal(Xil_unsigned8*  src_pixel,
		       Xil_unsigned8*  dst_pixel,
		       unsigned int    src_scanline_stride,
		       unsigned int    dst_scanline_stride,	    
		       unsigned int    dstR_xsize,
		       unsigned int    dstR_ysize,
		       int	       src_offset,
		       int 	       dst_offset   
		       )
{
    Xil_unsigned8* src_scanline = src_pixel;
    Xil_unsigned8* endpixel;
    unsigned int   endblocks;
    
    unsigned int   i = 0;
    unsigned int   j = 0;

    while((i>>4) < (dstR_ysize>>4)) {
	j = 0;
	while (((dst_offset + j)%8) && (j < dstR_xsize)) {
	    if (XIL_BMAP_TST(src_pixel, src_offset)) {
		XIL_BMAP_SET(dst_pixel, dst_offset + j);
	    } else {
		XIL_BMAP_CLR(dst_pixel, dst_offset + j);
	    }
	    src_pixel += src_scanline_stride;
	    j++;
	}
	if (i%16 == 0) {
	    while ((j+15) < dstR_xsize) {
		blk_flip_d(src_pixel + src_offset/8,
			   dst_pixel + (dst_offset + j)/8,
			   (src_offset)%8,
			   src_scanline_stride,
			   dst_scanline_stride );
		j += 16;
		src_pixel += src_scanline_stride << 4;
	    }
	    endblocks = j;
	    endpixel = src_pixel;
	}
	else {
	    j = endblocks;
	    src_pixel = endpixel;
	}
	while (j < dstR_xsize) {
	    if (XIL_BMAP_TST(src_pixel, src_offset)) {
		XIL_BMAP_SET(dst_pixel, dst_offset + j);
	    } else {
		XIL_BMAP_CLR(dst_pixel, dst_offset + j);
	    }
	    src_pixel += src_scanline_stride;
	    j++;
	}
	i++;
	src_pixel = src_scanline;
	dst_pixel += dst_scanline_stride;
	src_offset++;
    }
    while(i++ < dstR_ysize) {
	for (j = 0; j < dstR_xsize; j++) {
	    if (XIL_BMAP_TST(src_pixel, src_offset)) {
		XIL_BMAP_SET(dst_pixel, dst_offset + j);
	    } else {
		XIL_BMAP_CLR(dst_pixel, dst_offset + j);
	    }
	    src_pixel += src_scanline_stride;
	}
	src_pixel = src_scanline;
	dst_pixel += dst_scanline_stride;
	src_offset++;
    }
}

static void
xil_flip_anti_diagonal(Xil_unsigned8*  src_pixel,
		       Xil_unsigned8*  dst_pixel,
		       unsigned int    src_scanline_stride,
		       unsigned int    dst_scanline_stride,	    
		       unsigned int    dstR_xsize,		       
		       unsigned int    dstR_ysize,
		       int	       src_offset,
		       int 	       dst_offset   
		       )
{
    Xil_unsigned8* src_scanline = src_pixel;
    Xil_unsigned8* endpixel;
    unsigned int   endblocks;
    
    unsigned int   i = 0;
    unsigned int   j = 0;

    while((i>>4) < (dstR_ysize>>4)) {
	j = 0;
	while (((dst_offset + j) % 8) && (j < dstR_xsize)) {
	    if (XIL_BMAP_TST(src_pixel, src_offset - 1)) {
		XIL_BMAP_SET(dst_pixel, dst_offset + j);
	    } else {
		XIL_BMAP_CLR(dst_pixel, dst_offset + j);
	    }
	    src_pixel -= src_scanline_stride;
	    j++;
	}
	if (i%16 == 0) {
	    while ((j+15) < dstR_xsize) {
		blk_flip_a(src_pixel + (src_offset)/8
			   - (src_scanline_stride<<4) + src_scanline_stride - 2,
			   dst_pixel + (dst_offset + j)/8,
			   (src_offset)%8,
			   src_scanline_stride,
			   dst_scanline_stride );
		j += 16;
		src_pixel -= src_scanline_stride << 4;
	    }
	    endblocks = j;
	    endpixel = src_pixel;
	} else {
	    j = endblocks;
	    src_pixel = endpixel;
	}
	while (j < dstR_xsize) {
	    if (XIL_BMAP_TST(src_pixel, src_offset - 1)) {
		XIL_BMAP_SET(dst_pixel, dst_offset + j);
	    } else {
		XIL_BMAP_CLR(dst_pixel, dst_offset + j);
	    }
	    src_pixel -= src_scanline_stride;
	    j++;
	}
	i++;
	src_pixel = src_scanline;
	dst_pixel += dst_scanline_stride;
	src_offset--;
    }
    while(i++ < dstR_ysize) {
	for (j = 0; j < dstR_xsize; j++) {
	    if (XIL_BMAP_TST(src_pixel, src_offset - 1)) {
		XIL_BMAP_SET(dst_pixel, dst_offset + j);
	    } else {
		XIL_BMAP_CLR(dst_pixel, dst_offset + j);
	    }
	    src_pixel -= src_scanline_stride;
	}
	src_pixel = src_scanline;
	dst_pixel += dst_scanline_stride;
	src_offset--;
    }
}

static void
xil_flip_90(Xil_unsigned8*  src_pixel,
	    Xil_unsigned8*  dst_pixel,
	    unsigned int    src_scanline_stride,
	    unsigned int    dst_scanline_stride,	    
	    unsigned int    dstR_xsize,		       
	    unsigned int    dstR_ysize,
	    int	            src_offset,
	    int             dst_offset   
	    )
{
    Xil_unsigned8* src_scanline = src_pixel;
    Xil_unsigned8* endpixel;
    unsigned int   endblocks;
    
    unsigned int   i = 0;
    unsigned int   j = 0;

    while((i>>4) < (dstR_ysize>>4)) {
	j = 0;
	while (((dst_offset + j) % 8) && (j < dstR_xsize)) {
	    if (XIL_BMAP_TST(src_pixel, src_offset - 1)) {
		XIL_BMAP_SET(dst_pixel, dst_offset + j);
	    } else {
		XIL_BMAP_CLR(dst_pixel, dst_offset + j);
	    }
	    src_pixel += src_scanline_stride;
	    j++;
	}
	if (i%16 == 0) {
	    while ((j+15) < dstR_xsize) {
		blk_flip_90(src_pixel + (src_offset)/8 - 2,
			    dst_pixel + (dst_offset + j)/8,
			    (src_offset)%8, src_scanline_stride,
			    dst_scanline_stride );
		j += 16;
		src_pixel += src_scanline_stride << 4;
	    }
	    endblocks = j;
	    endpixel = src_pixel;
	}
	else {
	    j = endblocks;
	    src_pixel = endpixel;
	}
	while (j < dstR_xsize) {
	    if (XIL_BMAP_TST(src_pixel, src_offset - 1)) {
		XIL_BMAP_SET(dst_pixel, dst_offset + j);
	    } else {
		XIL_BMAP_CLR(dst_pixel, dst_offset + j);
	    }
	    src_pixel += src_scanline_stride;
	    j++;
	}
	i++;
	src_pixel = src_scanline;
	dst_pixel += dst_scanline_stride;
	src_offset--;
    }
    while(i++ < dstR_ysize) {
	for (j = 0; j < dstR_xsize; j++) {
	    if (XIL_BMAP_TST(src_pixel, src_offset - 1)) {
		XIL_BMAP_SET(dst_pixel, dst_offset + j);
	    } else {
		XIL_BMAP_CLR(dst_pixel, dst_offset + j);
	    }
	    src_pixel += src_scanline_stride;
	}
	src_pixel = src_scanline;
	dst_pixel += dst_scanline_stride;
	src_offset--;
    }
}

static void
xil_flip_180(Xil_unsigned8*  src_pixel,
	     Xil_unsigned8*  dst_pixel,
	     unsigned int    src_scanline_stride,
	     unsigned int    dst_scanline_stride,	    
	     unsigned int    dstR_xsize,		       
	     unsigned int    dstR_ysize,
	     int             src_offset,
	     int             dst_offset   
	     )
{
    Xil_unsigned8* src_scanline = src_pixel;
    unsigned int   endblocks;
    
    unsigned int   i = 0;
    unsigned int   j = 0;
    while((i>>4) < (dstR_ysize>>4)) {
	j = 0;
	while (((dst_offset + j) % 8) && (j < dstR_xsize)) {
	    if (XIL_BMAP_TST(src_pixel, src_offset - j - 1)) {
		XIL_BMAP_SET(dst_pixel, dst_offset + j);
	    } else {
		XIL_BMAP_CLR(dst_pixel, dst_offset + j);
	    }
	    j++;
	}
	if (i%16 == 0) {
	    while ((j+15) < dstR_xsize) {
                unsigned int right_src_offset = src_offset - j;

		blk_flip_180(src_pixel + right_src_offset/8
			     - (src_scanline_stride<<4) +
                             src_scanline_stride - 2,
			     dst_pixel + (dst_offset + j)/8,
			     right_src_offset % 8,
                             src_scanline_stride,
			     dst_scanline_stride);
		j += 16;
	    }
	    endblocks = j;
	} else {
	    j = endblocks;
	}
	while (j < dstR_xsize) {
	    if (XIL_BMAP_TST(src_pixel, src_offset - j - 1)) {
		XIL_BMAP_SET(dst_pixel, dst_offset + j);
	    } else {
		XIL_BMAP_CLR(dst_pixel, dst_offset + j);
	    }
	    j++;
	}
	i++;
	src_pixel = src_scanline -= src_scanline_stride;
	dst_pixel += dst_scanline_stride;
    }
    while(i++ < dstR_ysize) {
	for (j = 0; j < dstR_xsize; j++) {
	    if (XIL_BMAP_TST(src_pixel, src_offset - j - 1)) {
		XIL_BMAP_SET(dst_pixel, dst_offset + j);
	    } else {
		XIL_BMAP_CLR(dst_pixel, dst_offset + j);
	    }
	}
	src_pixel = src_scanline -= src_scanline_stride;
	dst_pixel += dst_scanline_stride;
    }
}

static void
xil_flip_270(Xil_unsigned8*  src_pixel,
	     Xil_unsigned8*  dst_pixel,
	     unsigned int    src_scanline_stride,
	     unsigned int    dst_scanline_stride,	    
	     unsigned int    dstR_xsize,		       
	     unsigned int    dstR_ysize,
	     int             src_offset,
	     int             dst_offset   
	     )
{
    Xil_unsigned8* src_scanline = src_pixel;
    Xil_unsigned8* endpixel;
    unsigned int   endblocks;
    
    unsigned int   i = 0;
    unsigned int   j = 0;

    while((i>>4) < (dstR_ysize>>4)) {
	j = 0;
	while (((dst_offset + j) % 8) && (j < dstR_xsize)) {
	    if (XIL_BMAP_TST(src_pixel, src_offset)) {
		XIL_BMAP_SET(dst_pixel, dst_offset + j);
	    }
	    else {
		XIL_BMAP_CLR(dst_pixel, dst_offset + j);
	    }
	    src_pixel -= src_scanline_stride;
	    j++;
	}
	if (i%16 == 0) {
	    while ((j+15) < dstR_xsize) {
		blk_flip_270(src_pixel + src_offset/8
			     - (src_scanline_stride << 4) + src_scanline_stride,
			     dst_pixel + (dst_offset + j)/8,
			     src_offset%8, src_scanline_stride,
			     dst_scanline_stride );
		j += 16;
		src_pixel -= src_scanline_stride << 4;
	    }
	    endblocks = j;
	    endpixel = src_pixel;
	} else {
	    j = endblocks;
	    src_pixel = endpixel;
	}
	while (j < dstR_xsize) {
	    if (XIL_BMAP_TST(src_pixel, src_offset)) {
		XIL_BMAP_SET(dst_pixel, dst_offset + j);
	    } else {
		XIL_BMAP_CLR(dst_pixel, dst_offset + j);
	    }
	    src_pixel -= src_scanline_stride;
	    j++;
	}
	i++;
	src_pixel = src_scanline;
	dst_pixel += dst_scanline_stride;
	src_offset++;
    }
    while(i++ < dstR_ysize) {
	for (j = 0; j < dstR_xsize; j++) {
	    if (XIL_BMAP_TST(src_pixel, src_offset)) {
		XIL_BMAP_SET(dst_pixel, dst_offset + j);
	    } else {
		XIL_BMAP_CLR(dst_pixel, dst_offset + j);
	    }
	    src_pixel -= src_scanline_stride;
	}
	src_pixel = src_scanline;
	dst_pixel += dst_scanline_stride;
	src_offset++;
    }
}
