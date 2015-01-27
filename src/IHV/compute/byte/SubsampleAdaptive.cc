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
//  File:	SubsampleAdaptive.cc
//  Project:	XIL
//  Revision:	1.14
//  Last Mod:	10:10:37, 03/10/00
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
#pragma ident	"@(#)SubsampleAdaptive.cc	1.14\t00/03/10  "

#include "XilDeviceManagerComputeBYTE.hh"
#include "ComputeInfo.hh"
#include "XiliUtils.hh"
#include "xili_geom_utils.hh"

//
//  Forward declarations
//
static void subsample_pixel_sequential(SubsampleData subsample_data);
static void subsample_general_storage(SubsampleData subsample_data);

XilStatus
XilDeviceManagerComputeBYTE::SubsampleAdaptive(XilOp*       op,
					       unsigned     ,
					       XilRoi*      roi,
					       XilBoxList*  bl)
{
    SubsampleData sd;

    sd.roi = roi;

    op->getParam(1, &sd.xscale);    
    op->getParam(2, &sd.yscale);

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

	//
        //  Test to see if all of our storage is of type XIL_PIXEL_SEQUENTIAL.
        //  If so, optimize for pixel-sequential storage.
        //
        if((src_storage.isType(XIL_PIXEL_SEQUENTIAL)) &&
	   (dst_storage.isType(XIL_PIXEL_SEQUENTIAL))) {
            subsample_pixel_sequential(sd);
	} else {
	    subsample_general_storage(sd);
	}
    } // end of while(bl->getNext)

    return XIL_SUCCESS;
}

//
//  Performs subsample transform of dst values for n banded image
//  assuming pixel sequential image.
//
static
void
subsample_pixel_sequential(SubsampleData subsample_data)
{
    XilStorage*  src_storage   = subsample_data.src_storage;
    XilStorage*  dst_storage   = subsample_data.dst_storage;
    XilBox*      src_box       = subsample_data.src_box;
    XilBox*      dst_box       = subsample_data.dst_box;
    XilRoi*      roi           = subsample_data.roi;
    unsigned int nbands        = subsample_data.nbands;
    float        xscale        = subsample_data.xscale;
    float        yscale        = subsample_data.yscale;

    Xil_unsigned8* src_data;
    unsigned int   src_pstride;
    unsigned int   src_sstride;
    src_storage->getStorageInfo(&src_pstride,
			        &src_sstride,
			        NULL, NULL,
			        (void**)&src_data);
            
    Xil_unsigned8* dst_data;
    unsigned int   dst_pstride;
    unsigned int   dst_sstride;
    dst_storage->getStorageInfo(&dst_pstride,
			        &dst_sstride,
			        NULL, NULL,
			        (void**)&dst_data);
    
    //
    //  Calculate our source walking deltas.
    //
    float src_dw = 1.0/xscale;
    float src_dh = 1.0/yscale;

    //
    //  Calculate source box end-points for clipping.
    //
    int          src_x;
    int          src_y;
    unsigned int src_w;
    unsigned int src_h;
    src_box->getAsRect(&src_x, &src_y, &src_w, &src_h);

    //
    //  Create a list of rectangles to loop over.  The resulting list
    //  of rectangles is the area left by intersecting the ROI with
    //  the destination box.
    //
    XilRectList    rl(roi, dst_box);

    int          x;
    int          y;
    unsigned int xsize;
    unsigned int ysize;
    while(rl.getNext(&x, &y, &xsize, &ysize)) {
        Xil_unsigned8* src_scanline = src_data +
            ((int)(y*src_dh) * src_sstride) +
            ((int)(x*src_dw) * src_pstride);

        Xil_unsigned8* dst_scanline = dst_data +
            (y * dst_sstride) +
            (x * dst_pstride);

        //
        //  The algorithm keeps the floating point location of where we're
        //  located while stepping through the source.  For each move in x or
        //  y in the destination, we calculate where we are going to be next
        //  and the difference between where we are and where we're going to
        //  be for the next destination pixel gives us the block size to
        //  traverse in the source for the destination pixel.
        //  
        float        src_flt_y = y;
        unsigned int src_old_y = y;
        unsigned int src_new_y;
        unsigned int src_blk_y;
        for(unsigned int i=ysize; i!=0; i--) {
            src_flt_y += src_dh;
            src_new_y  = (unsigned int)ceil(src_flt_y);
            if(src_new_y > src_h) {
                src_new_y = src_h;
            }
            src_blk_y  = src_new_y - src_old_y;
            src_old_y  = src_new_y;

            if(src_blk_y == 0) {
                dst_scanline += dst_sstride;
                continue;
            }

            Xil_unsigned8* src_pixel = src_scanline;
            Xil_unsigned8* dst_pixel = dst_scanline;

            float        src_flt_x = x;
            unsigned int src_old_x = x;
            unsigned int src_new_x;
            unsigned int src_blk_x;
            for(unsigned int j=xsize; j!=0; j--) {
                src_flt_x += src_dw;
                src_new_x  = (unsigned int)ceil(src_flt_x);
                if(src_new_x > src_w) {
                    src_new_x = src_w;
                }
                src_blk_x  = src_new_x - src_old_x;
                src_old_x  = src_new_x;

                if(src_blk_x == 0) {
                    dst_pixel += dst_pstride;
                    continue;
                }

                Xil_unsigned8* src_band = src_pixel;
                Xil_unsigned8* dst_band = dst_pixel;
                for(unsigned int b=nbands; b!=0; b--) {
                    Xil_unsigned8* blk_scanline = src_band;
                    float          ftmp         = 0.0F;
                    float          blk_size     = 0.0F;

                    for(unsigned int k=src_blk_y; k!=0; k--) {
                        Xil_unsigned8* blk_pixel = blk_scanline;
                        for(unsigned int l=src_blk_x; l!=0; l--) {
                            ftmp += _XILI_B2F(*blk_pixel);

                            //
                            //  Ok, ok, it looks weird, but by computing the
                            //  block size by counting here, it's much faster
                            //  than doing a multiplication since we need to
                            //  go through the loops anyway.  Werid.
                            //
                            blk_size++;

                            blk_pixel += src_pstride;

                        }

                        blk_scanline += src_sstride;
                    }

                    *dst_band = _XILI_ROUND_U8(ftmp/blk_size);

                    src_band++;
                    dst_band++;
                }

                src_pixel += src_blk_x * src_pstride;
                dst_pixel += dst_pstride;
            }

            src_scanline += src_blk_y * src_sstride;
            dst_scanline += dst_sstride;
        }
    }
}

static
void
subsample_general_storage(SubsampleData subsample_data)
{
    XilStorage*  src_storage   = subsample_data.src_storage;
    XilStorage*  dst_storage   = subsample_data.dst_storage;
    XilBox*      src_box       = subsample_data.src_box;
    XilBox*      dst_box       = subsample_data.dst_box;
    XilRoi*      roi           = subsample_data.roi;
    unsigned int nbands        = subsample_data.nbands;
    float        xscale        = subsample_data.xscale;
    float        yscale        = subsample_data.yscale;

    //
    //  Calculate our source walking deltas.
    //
    float src_dw = 1.0/xscale;
    float src_dh = 1.0/yscale;

    //
    //  Calculate source box end-points for clipping.
    //
    int          src_x;
    int          src_y;
    unsigned int src_w;
    unsigned int src_h;
    src_box->getAsRect(&src_x, &src_y, &src_w, &src_h);

    //
    //  Create a list of rectangles to loop over.  The resulting list
    //  of rectangles is the area left by intersecting the ROI with
    //  the destination box.
    //
    XilRectList    rl(roi, dst_box);

    int          x;
    int          y;
    unsigned int xsize;
    unsigned int ysize;
    while(rl.getNext(&x, &y, &xsize, &ysize)) {
        for(unsigned int b=0; b<nbands; b++) {
            Xil_unsigned8* src_data;
            unsigned int   src_pstride;
            unsigned int   src_sstride;
            src_storage->getStorageInfo(b,
                                        &src_pstride,
                                        &src_sstride,
                                        NULL,
                                        (void**)&src_data);

            Xil_unsigned8* dst_data;
            unsigned int   dst_pstride;
            unsigned int   dst_sstride;
            dst_storage->getStorageInfo(b,
                                        &dst_pstride,
                                        &dst_sstride,
                                        NULL,
                                        (void**)&dst_data);

            Xil_unsigned8* src_scanline = src_data +
                ((int)(y*src_dh) * src_sstride) +
                ((int)(x*src_dw) * src_pstride);

            Xil_unsigned8* dst_scanline = dst_data +
                (y * dst_sstride) +
                (x * dst_pstride);

            //
            //  See above for description of algorithm.
            //
            float        src_flt_y = y;
            unsigned int src_old_y = y;
            unsigned int src_new_y;
            unsigned int src_blk_y;
            for(unsigned int i=ysize; i!=0; i--) {
                src_flt_y += src_dh;
                src_new_y  = (unsigned int)ceil(src_flt_y);
                if(src_new_y > src_h) {
                    src_new_y = src_h;
                }
                src_blk_y  = src_new_y - src_old_y;
                src_old_y  = src_new_y;

                if(src_blk_y == 0) {
                    dst_scanline += dst_sstride;
                    continue;
                }

                Xil_unsigned8* src_pixel = src_scanline;
                Xil_unsigned8* dst_pixel = dst_scanline;

                float        src_flt_x = x;
                unsigned int src_old_x = x;
                unsigned int src_new_x;
                unsigned int src_blk_x;
                for(unsigned int j=xsize; j!=0; j--) {
                    src_flt_x += src_dw;
                    src_new_x  = (unsigned int)ceil(src_flt_x);
                    if(src_new_x > src_w) {
                        src_new_x = src_w;
                    }
                    src_blk_x  = src_new_x - src_old_x;
                    src_old_x  = src_new_x;

                    if(src_blk_x == 0) {
                        dst_pixel += dst_pstride;
                        continue;
                    }

                    Xil_unsigned8* blk_scanline = src_pixel;
                    float          ftmp         = 0.0F;
                    float          blk_size     = 0.0F;

                    for(unsigned int k=src_blk_y; k!=0; k--) {
                        Xil_unsigned8* blk_pixel = blk_scanline;
                        for(unsigned int l=src_blk_x; l!=0; l--) {
                            ftmp += _XILI_B2F(*blk_pixel);

                            //
                            //  Ok, ok, it looks weird, but by computing the
                            //  block size by counting here, it's much faster
                            //  than doing a multiplication since we need to
                            //  go through the loops anyway.  Werid.
                            //
                            blk_size++;

                            blk_pixel += src_pstride;

                        }

                        blk_scanline += src_sstride;
                    }

                    *dst_pixel = _XILI_ROUND_U8(ftmp/blk_size);

                    src_pixel += src_blk_x * src_pstride;
                    dst_pixel += dst_pstride;
                }

                src_scanline += src_blk_y * src_sstride;
                dst_scanline += dst_sstride;
            }
        }
    }
}
