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
//This line lets emacs recognize this as -*- C++ -*- Code
//------------------------------------------------------------------------
//
//  File:	SubsampleAdaptive.cc
//  Project:	XIL
//  Revision:	1.6
//  Last Mod:	10:10:04, 03/10/00
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
//  MT-level:  <??????>
//	
//------------------------------------------------------------------------
//	COPYRIGHT
//------------------------------------------------------------------------
#pragma ident	"@(#)SubsampleAdaptive.cc	1.6\t00/03/10  "

#include "XilDeviceManagerComputeBIT.hh"
#include "ComputeInfo.hh"
#include "XiliUtils.hh"
#include "xili_geom_utils.hh"

//
// forward declarations
//

static XilStatus subsample_general_storage(SubsampleData subsample_data);


XilStatus
XilDeviceManagerComputeBIT::SubsampleAdaptive(XilOp*       op,
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

	status = subsample_general_storage(sd);

    } // end of while(bl->getNext)

    return status;
    
}

static XilStatus
subsample_general_storage(SubsampleData subsample_data)
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
    
    Xil_unsigned8* src_data;
    Xil_unsigned8* src_scanline;    
    unsigned int   src_scanline_stride;

    Xil_unsigned8* dst_data;
    unsigned int   dst_scanline_stride;

    Xil_unsigned8* src_band;
    Xil_unsigned8* dst_band;	// ptr to the current band

    unsigned int   src_offset; // user supplied bit offset
    unsigned int   dst_offset; 

    unsigned int   src_bit;	// index (ptr) to the current src bit
    unsigned int   dst_bit;	// index (ptr) to the current dst bit  

    int            	srcR_x, srcR_y;	// src region location 
    unsigned int    	srcR_xsize,
	srcR_ysize;	// src region size 
    int            	dstR_x, dstR_y;	// dst region location 
    unsigned int    	dstR_xsize,
	dstR_ysize;	// dst region size

    src_box->getAsRect(&srcR_x, &srcR_y, &srcR_xsize, &srcR_ysize);    
    //
    //
    //  Create a list of rectangles to loop over.  The resulting list
    //  of rectangles is the area left by intersecting the ROI with
    //  the destination box.
    //
    XilRectList    rl(roi, dst_box);
    
    while(rl.getNext(&dstR_x, &dstR_y, &dstR_xsize, &dstR_ysize)) {
        //
	// Src box is divided into changing size blocks. Each block
	// is defined as the rectangle that will contain all
	// the src pixels mapped into a single destination pixel.
	// We compute the block size from the values of the scale and
	// and the size of the src_box. src and dst box origins are
	// already taken care of so we do computations relative to
	// their spaces.
	//
	float block_x, block_y;	// block top left corner
	float block_x_size;
	float block_y_size;

	int   dst_x, dst_y;	// dst pixel coordinates into which block maps
	//
	//  Each Band...
	//
	for(unsigned int band = 0; band < nbands; band++) {
	    
	    src_storage->getStorageInfo(band,
				    NULL, 
					&src_scanline_stride,
					&src_offset,
					(void**)&src_data);
	    dst_storage->getStorageInfo(band,
					NULL,
					&dst_scanline_stride,
					&dst_offset,
					(void**)&dst_data);
	
	    block_x = 0.0;
	    block_y = 0.0;
	    //
	    // compute all the blocks in the src box
	    //
	    while ((int) block_y < (int) srcR_ysize){
		//
		// find the y block size
		//
		dst_y = (int) (block_y * yscale);
		//
		// keep increasing block size while it will still map
		// into one dst pixel
		//
		float block_y_tmp = block_y;

		while ( ((int) block_y_tmp < (int) srcR_ysize) && 
			(dst_y == (int) (block_y_tmp * yscale)))
		    block_y_tmp += 1.0;
	    
		block_y_size = (int) (block_y_tmp - block_y);

		if (block_y_size == 0)
		    break;

		while ((int) block_x < (int) srcR_xsize) {
		    //
		    // find the x block size
		    //
		    dst_x = (int) (block_x * xscale);

		    float block_x_tmp = block_x;
		    while ( ((int) block_x_tmp < (int) srcR_xsize) && 
			    (dst_x == (int) (block_x_tmp * xscale)) )
			block_x_tmp += 1.0;
		
		    block_x_size = (int) (block_x_tmp - block_x);

		    if (block_x_size == 0)
			break;

		    if ( ((dst_x >= 0) && (dst_x < dstR_xsize)) &&
			 ((dst_y >= 0) && (dst_y < dstR_ysize))){

		   
			src_band = src_data + (((int) block_y) * src_scanline_stride) +
			    (((int) block_x)  + src_offset) / 8;

			src_bit = (((int) block_x)  + src_offset) % 8;

		        dst_band = dst_data + (dst_y * dst_scanline_stride) +
			    (dst_x + dst_offset) / 8;

			dst_bit = (dst_x + dst_offset) % 8;
          
			src_scanline = src_band;

 			float ftmp = 0.0;
			for (int i = 0; i < block_y_size; i++) {
			    for (int j = 0; j < block_x_size; j++) {
			        if(XIL_BMAP_TST(src_scanline, src_bit + j))
				    ftmp += 1.0;
			    }
			    src_scanline += src_scanline_stride;
			}

			ftmp /= block_y_size*block_x_size;

			
			if (ftmp > 0.5)
			    XIL_BMAP_SET(dst_band, dst_bit);
			else
			    XIL_BMAP_CLR(dst_band, dst_bit);

		    }
		block_x += block_x_size;
		}
		block_x = 0.0;
		block_y += block_y_size;
	    }
	}
    }
    return status;
}
 
