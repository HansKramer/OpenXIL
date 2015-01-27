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
//  File:	ScaleNearest.cc
//  Project:	XIL
//  Revision:	1.15
//  Last Mod:	10:12:17, 03/10/00
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
#pragma ident	"@(#)ScaleNearest.cc	1.15\t00/03/10  "

#include "XilDeviceManagerComputeSHORT.hh"
#include "ComputeInfo.hh"
#include "xili_geom_utils.hh"

// TODO bpb 05/07/1997 Clean up bad indenting left over from 1.2

#define SCALE2X_DEBUG 0

static XilStatus scaleZoom2xNearest(XilBoxList* bl, AffineData ad);
static XilStatus scale_zoom2x_pixel_sequential_NN(AffineData affine_data);
static XilStatus scale_zoom2x_general_storage_NN(AffineData affine_data);

XilStatus
XilDeviceManagerComputeSHORT::ScaleNearest(XilOp*       op,
					  unsigned       ,
					  XilRoi*      roi,
					  XilBoxList*  bl)
{
    XilStatus status;
    AffineData ad;
    
    ad.op  = op;
    ad.roi = roi;
    
    //
    //  Get the transformation matrix. Translational part of
    //  the matrix contains destination origin, currently
    //  this is not supplied by the core.
    //
    float	xscale, yscale;
    
    op->getParam(1, &xscale);
    op->getParam(2, &yscale);
    //
    // Get the basic data, assuming that the src image is a BYTE image.
    // roi is the complete image ROI. This means that src and dst ROI's are
    // already taken into account, and the roi passed is the intersection
    // of these. 
    //
    if(op->splitOnTileBoundaries(bl) == XIL_FAILURE) {
        return XIL_FAILURE;
    }

    if(xili_scale_is_zoom2x(xscale, yscale)) {
        //
        // 2x replicate zoom
        //
        status = scaleZoom2xNearest(bl, ad);
    } else {
        //
        // construct affine matrix
        //
        float matrix[6];		// Equivalent affine matrix for scale

        matrix[0] = xscale;
        matrix[1] = 0.0;
        matrix[2] = 0.0;
        matrix[3] = yscale;
        matrix[4] = 0.0;	
        matrix[5] = 0.0;     

        ad.matrix = matrix;
        //
        // If nothing else works, call affine transform
        //
        status = affineNearest(bl, ad);
    }

    return status;  

}

static
XilStatus
scaleZoom2xNearest(XilBoxList* bl,
                   AffineData ad)
{
    XilOp*     op = ad.op;

    //
    //  Get the images for our operation.
    //
    XilImage* src = op->getSrcImage(1);
    XilImage* dst = op->getDstImage(1);

    ad.nbands = dst->getNumBands();

    //
    // Each dst box is covered with several convex regions. Each of
    // the convex regions is bound to lie in only one src box.
    //
    XilBox*   src_box;
    XilBox*   dst_box;
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

        ad.src_storage = &src_storage;
        ad.dst_storage = &dst_storage;
        ad.src_box     = src_box;
        ad.dst_box     = dst_box;

        //
        //  Test to see if all of our storage is of type XIL_PIXEL_SEQUENTIAL.
        //  If so, optimize for pixel-sequential storage.
        //
        if((src_storage.isType(XIL_PIXEL_SEQUENTIAL)) &&
           (dst_storage.isType(XIL_PIXEL_SEQUENTIAL))) {
            if(scale_zoom2x_pixel_sequential_NN(ad) == XIL_FAILURE) {
                //
                //  Mark this box as failed and if that succeeds, continue
                //  processing the next box.  Otherwise, return XIL_FAILURE
                //  now.
                //
                if(bl->markAsFailed() == XIL_FAILURE) {
                    return XIL_FAILURE;
                } else {
                    continue;
                }
            }
        } else {
            if(scale_zoom2x_general_storage_NN(ad) == XIL_FAILURE) {
                //
                //  Mark this box as failed and if that succeeds, continue
                //  processing the next box.  Otherwise, return XIL_FAILURE
                //  now.
                //
                if(bl->markAsFailed() == XIL_FAILURE) {
                    return XIL_FAILURE;
                } else {
                    continue;
                }
            }
        }
    }

    return XIL_SUCCESS;
}

static XilStatus scale_zoom2x_pixel_sequential_NN(AffineData affine_data)
{
    XilOp*       op            = affine_data.op;
    XilStorage*  src_storage   = affine_data.src_storage;
    XilStorage*  dst_storage   = affine_data.dst_storage;
    XilBox*      src_box       = affine_data.src_box;
    XilBox*      dst_box       = affine_data.dst_box;
    XilRoi*      roi           = affine_data.roi;
    unsigned int nbands        = affine_data.nbands;
    
    //
    // Get the storage information
    //
    Xil_signed16 * src_base_addr;
    unsigned int   src_next_pixel;
    unsigned int   src_next_scan;
    src_storage->getStorageInfo(&src_next_pixel,
                                &src_next_scan,
                                NULL, NULL,
                                (void**)&src_base_addr);
            
    Xil_signed16 * dst_base_addr;
    unsigned int   dst_next_pixel;
    unsigned int   dst_next_scan;
    dst_storage->getStorageInfo(&dst_next_pixel,
                                &dst_next_scan,
                                NULL, NULL,
                                (void**)&dst_base_addr);
    unsigned int dst_next_2pixel = dst_next_pixel << 1;

    //
    // Create a rectangle list that is an intersection of
    // the intersected roi and dst_box. The list is
    // returned in the dst_box coordinate space (not
    // in the dst image coordinate space).
    //
    XilRectList rl(roi, dst_box);

    //
    //  Loop over rectangles in the destination.
    //
    int x;
    int y;
    unsigned int x_size;
    unsigned int y_size;
    while(rl.getNext(&x, &y, &x_size, &y_size)) {
            //
            //  For scale, 0,0 in source may not correspond to the 0,0 in the
            //  destination backward mapped into the source but it does map
            //  within the source box.  Call op->backwardMap() to get the
            //  source point within the box corresponding to x,y.
            //
            double srcx;
            double srcy;
            op->backwardMap(dst_box, (double)x, (double)y,
                            src_box, &srcx, &srcy);

            Xil_signed16* src_scanline = src_base_addr +
                ((int)srcy) * src_next_scan +
                ((int)srcx) * src_next_pixel;

            //
            // Determine the processing to apply to the first row and column
            // of the destination rect. Each source pixel might be forward
            // mapped into up to four destination pixels. Due to non-zero
            // origins it is possible that a given source pixel could be
            // forward mapped into _different_ destination rects. The optimized
            // algorithm used below generates destination lines as pairs of
            // identical lines so that if the first source line is not to be
            // replicated then it must be processed separately. Also, for
            // each band-version of the optimized algorithm there is a variant
            // for the cases in which the first pixel in the source line is
            // and is not replicated.
            //
            // The following tests determine whether the first line should
            // be processed separately and whether the first source pixel in
            // each line should be replicated. These tests are equivalent to
            // asking, respectively, does the dst pixel at (rect coordinates)
            // (x,y+1) backward map into the same src pixel as does the dst
            // pixel at (x,y), and does the dst pixel at (x+1,y) map into the
            // same src pixel as the one at (x,y)? The difference is that we
            // step +0.5 in a source image dimension rather than +1.0 in the
            // corresponding destination dimension and then backward mapping.
            //
            Xil_boolean   first_scanline_is_odd =
                ((int)srcy) != ((int)(srcy + 0.5)) ? TRUE : FALSE;
            Xil_boolean   first_pixel_is_odd    =
                ((int)srcx) != ((int)(srcx + 0.5)) ? TRUE : FALSE;

	    Xil_signed16* dst_scanline1 = dst_base_addr +
		y * dst_next_scan +
		x * dst_next_pixel;
	    Xil_signed16* dst_scanline2 = dst_scanline1 + dst_next_scan;

	    if ((first_scanline_is_odd == TRUE) && y_size) {
		Xil_signed16    *src_pixel = src_scanline,
				*dst_pixel1 = dst_scanline1;
		int		 xcount = x_size;

		if ((first_pixel_is_odd == TRUE) && xcount ) {
		    Xil_signed16  *s  = src_pixel,
				  *db11 = dst_pixel1;
		    int	bandcount = nbands;
		    while(bandcount--) {
			*db11++ = *s++;
			}
		    xcount --;

		    src_pixel += src_next_pixel;
		    dst_pixel1 += dst_next_pixel;
		    }

		while (xcount > 1) {	// write pixels in pairs
		    Xil_signed16  *s  = src_pixel,
				  *db11 = dst_pixel1,
				  *db12 = dst_pixel1 + dst_next_pixel;
		    int	bandcount = nbands;
		    while(bandcount--) {
			*db11++ = *s;
			*db12++ = *s++;
			}
		    xcount -= 2;

		    src_pixel += src_next_pixel;
		    dst_pixel1 += dst_next_2pixel;
		    }

		if (xcount) {		// last pixel is odd
		    int	bandcount = nbands;
		    while(bandcount--)
			*dst_pixel1++ = *src_pixel++;
		    }

		src_scanline += src_next_scan;
		dst_scanline1 += dst_next_scan;
		dst_scanline2 = dst_scanline1 + dst_next_scan;
		y_size --;
		} // end 1st dst scan is odd

	    if (((((long)(src_scanline) & 0x3) + ((long)(dst_scanline1) & 0x3) +
		  ((long)(dst_scanline2) & 0x3) + (x_size % 4) +
		  (dst_next_scan % 4) + (src_next_scan % 4)) == 0) &&
		(src_next_pixel == nbands) &&
		(dst_next_pixel == nbands) &&
#ifdef XIL_LITTLE_ENDIAN
// TODO bpb 05/08/1997 Not all x86 optimizations are complete.
		((nbands == 4))) {
#else
		((nbands == 1) || (nbands == 3) || (nbands == 4))) {
#endif

#if SCALE2X_DEBUG
fprintf(stderr, "short BIP %d-band optimized case; ", nbands);
fprintf(stderr, "first_pixel_is_odd == %s\n",
        first_pixel_is_odd ? "TRUE":"FALSE");
#endif
		switch (nbands) {	// we have 1, 3, or 4 band contiguous memory image
		    case 1:
// TODO bpb 05/08/1997 1-band pixel-sequential x86 optimization (both cases)
                      if(first_pixel_is_odd == TRUE) {
			while (y_size > 1) {
			    unsigned int *s  = (unsigned int *)src_scanline;
			    unsigned int *d1 = (unsigned int *)dst_scanline1;
			    unsigned int *d2 = (unsigned int *)dst_scanline2;
			    int xcount = x_size;

			    while (xcount) {
				unsigned int sbuf = *s++;
				unsigned int dbuf1 = sbuf;
				unsigned int dbuf2 =
                                    ((sbuf << 16) & 0xffff0000) |
                                    (((*s) >> 16) & 0x0000ffff);
				*d2++ = *d1++ = dbuf1;
				*d2++ = *d1++ = dbuf2;
				xcount -= 4;
				}

			    src_scanline += src_next_scan;
			    dst_scanline1 = dst_scanline2 + dst_next_scan;
			    dst_scanline2 = dst_scanline1 + dst_next_scan;
			    y_size -= 2;
			    }
                          } else {
                        while (y_size > 1) {
                            unsigned int *s  = (unsigned int *)src_scanline;
                            unsigned int *d1 = (unsigned int *)dst_scanline1;
                            unsigned int *d2 = (unsigned int *)dst_scanline2;
                            int xcount = x_size;

                            while (xcount) {
                                unsigned int sbuf = *s++;
                                unsigned int dbuf1 = sbuf & 0xffff0000;
                                dbuf1 |= dbuf1 >> 16;
                                unsigned int dbuf2 = sbuf & 0x0000ffff;
                                dbuf2 |= dbuf2 << 16;
                                *d2++ = *d1++ = dbuf1;
                                *d2++ = *d1++ = dbuf2;
                                xcount -= 4;
                                }

                            src_scanline += src_next_scan;
                            dst_scanline1 = dst_scanline2 + dst_next_scan;
                            dst_scanline2 = dst_scanline1 + dst_next_scan;
                            y_size -= 2;
                            }
                          }
			break;
		    case 3:
// TODO bpb 05/08/1997 3-band pixel-sequential x86 optimization (both cases)
                      if(first_pixel_is_odd == TRUE) {
			while (y_size > 1) {
			    unsigned int *s  = (unsigned int *)src_scanline;
			    unsigned int *d1 = (unsigned int *)dst_scanline1;
			    unsigned int *d2 = (unsigned int *)dst_scanline2;
			    int xcount = x_size;
			    while (xcount) {
				unsigned int sbuf1 = *s++;
				unsigned int sbuf2 = *s++;

#if 1
                                *d2++ = *d1++ = sbuf1;
                                *d2++ = *d1++ = sbuf2;
                                sbuf1 = *s++;
                                *d2++ = *d1++ = sbuf1;
                                *d2++ = *d1++ =
                                    ((sbuf2 << 16) & 0xffff0000) |
                                    ((sbuf1 >> 16) & 0x0000ffff);
                                sbuf2 = *s;
                                *d2++ = *d1++ =
                                    ((sbuf1 << 16) & 0xffff0000) |
                                    ((sbuf2 >> 16) & 0x0000ffff);
                                *d2++ = *d1++ =
                                    ((sbuf2 << 16) & 0xffff0000) |
                                    (((*(s+1)) >> 16) & 0x0000ffff);
#else
                                int bandcount = 3;
                                while(bandcount--) {
                                    *dst_pixel1++ = *dst_pixel2++ = *src_pixel++;
                                }
                                bandcount = 3;
                                while(bandcount--) {
                                    *dst_pixel1++ = *dst_pixel2++ = *src_pixel++;
                                }
                                src_pixel -= 3;
                                bandcount = 3;
                                while(bandcount--) {
                                    *dst_pixel1++ = *dst_pixel2++ = *src_pixel++;
                                }
                                bandcount = 3;
                                while(bandcount--) {
                                    *dst_pixel1++ = *dst_pixel2++ = *src_pixel++;
                                }
                                src_pixel -= 3;
#endif
				xcount -= 4;
				}

			    src_scanline += src_next_scan;
			    dst_scanline1 = dst_scanline2 + dst_next_scan;
			    dst_scanline2 = dst_scanline1 + dst_next_scan;
			    y_size -= 2;
			    }
                      } else {
			while (y_size > 1) {
			    unsigned int *s  = (unsigned int *)src_scanline;
			    unsigned int *d1 = (unsigned int *)dst_scanline1;
			    unsigned int *d2 = (unsigned int *)dst_scanline2;
			    int xcount = x_size;
			    while (xcount) {
				unsigned int sbuf1 = *s++;
				unsigned int sbuf2 = *s++;

                                *d2++ = *d1++ = sbuf1;
                                *d2++ = *d1++ = 
                                    (sbuf2 & 0xffff0000) |
                                    ((sbuf1 >> 16) & 0x0000ffff);
                                *d2++ = *d1++ = 
                                    ((sbuf1 << 16) & 0xffff0000) |
                                    ((sbuf2 >> 16) & 0x0000ffff);
                                sbuf1 = *s++;
                                *d2++ = *d1++ = 
                                    ((sbuf2 << 16) & 0xffff0000) |
                                    ((sbuf1 >> 16) & 0x0000ffff);
                                *d2++ = *d1++ = 
                                    ((sbuf1 << 16) & 0xffff0000) |
                                    (sbuf2 & 0x0000ffff);
                                *d2++ = *d1++ = sbuf1;
				xcount -= 4;
				}

			    src_scanline += src_next_scan;
			    dst_scanline1 = dst_scanline2 + dst_next_scan;
			    dst_scanline2 = dst_scanline1 + dst_next_scan;
			    y_size -= 2;
			    }
                          }
			break;
		    case 4:
                      if(first_pixel_is_odd == TRUE) {
			while (y_size > 1) {
			    unsigned int *s1 = (unsigned int *)src_scanline;
			    unsigned int *s2 = s1 + 1;
			    unsigned int *d1 = (unsigned int *)dst_scanline1;
			    unsigned int *d2 = (unsigned int *)dst_scanline2;
			    int xcount = x_size;
			    *d2++ = *d1++ = *s1;
			    *d2++ = *d1++ = *s2++;
                            s1 = s2++;
			    xcount--;
			    while (xcount > 1) {
				*d2++ = *d1++ = *s1;
				*d2++ = *d1++ = *s2;
				*d2++ = *d1++ = *s1;
				*d2++ = *d1++ = *s2++;

                                s1 = s2++;
				xcount -= 2;
				}
                            if(xcount) {
			        *d2++ = *d1++ = *s1;
			        *d2++ = *d1++ = *s2;
                            }

			    src_scanline += src_next_scan;
			    dst_scanline1 = dst_scanline2 + dst_next_scan;
			    dst_scanline2 = dst_scanline1 + dst_next_scan;
			    y_size -= 2;
			    }
                      } else {
			while (y_size > 1) {
			    unsigned int *s1 = (unsigned int *)src_scanline;
			    unsigned int *s2 = s1 + 1;
			    unsigned int *d1 = (unsigned int *)dst_scanline1;
			    unsigned int *d2 = (unsigned int *)dst_scanline2;
			    int xcount = x_size;
			    while (xcount) {
				*d2++ = *d1++ = *s1;
				*d2++ = *d1++ = *s2;
				*d2++ = *d1++ = *s1;
				*d2++ = *d1++ = *s2++;

                                s1 = s2++;
				xcount -= 2;
				}

			    src_scanline += src_next_scan;
			    dst_scanline1 = dst_scanline2 + dst_next_scan;
			    dst_scanline2 = dst_scanline1 + dst_next_scan;
			    y_size -= 2;
			    }
                          }
			break;
		    }
		}
	    else {	// read/write shorts

		while (y_size > 1) {	// write dst lines in pairs
		    Xil_signed16    *src_pixel = src_scanline,
				    *dst_pixel1 = dst_scanline1,
				    *dst_pixel2 = dst_scanline2;
		    int		     xcount = x_size;

		    if ((first_pixel_is_odd == TRUE) && xcount ) {
			Xil_signed16  *s  = src_pixel,
				      *db11 = dst_pixel1,
				      *db21 = dst_pixel2;
			int	bandcount = nbands;
			while(bandcount--)
			    *db21++ = *db11++ = *s++;
			xcount --;

			src_pixel += src_next_pixel;
			dst_pixel1 += dst_next_pixel;
			dst_pixel2 += dst_next_pixel;
			}

		    while (xcount > 1) {	//write dst pixels in pairs
			Xil_signed16  *s  = src_pixel,
				  *db11 = dst_pixel1,
				  *db21 = dst_pixel2,
				  *db12 = dst_pixel1 + dst_next_pixel,
				  *db22 = dst_pixel2 + dst_next_pixel;
			int	bandcount = nbands;
			while(bandcount--) {
			    *db12++ = *db11++ = *s;
			    *db22++ = *db21++ = *s++;
			    }
			xcount -= 2;

			src_pixel += src_next_pixel;
			dst_pixel1 += dst_next_2pixel;
			dst_pixel2 += dst_next_2pixel;
			}

		    if (xcount) {		// last pixels are odd
			int	bandcount = nbands;
			while(bandcount--)
			    *dst_pixel2++ = *dst_pixel1++ = *src_pixel++;
			}

		    src_scanline += src_next_scan;
		    dst_scanline1 = dst_scanline2 + dst_next_scan;
		    dst_scanline2 = dst_scanline1 + dst_next_scan;
		    y_size -= 2;
		    }
		} // end generic case ("read/write shorts")

		if (y_size) {		// last scan line is odd
		    Xil_signed16    *src_pixel = src_scanline,
				*dst_pixel1 = dst_scanline1;
		    int		 xcount = x_size;

		    if ((first_pixel_is_odd == TRUE) && xcount ) {
			Xil_signed16  *s  = src_pixel,
				      *db11 = dst_pixel1;
			int	bandcount = nbands;
			while(bandcount--)
			    *db11++ = *s++;
			xcount --;

			src_pixel += src_next_pixel;
			dst_pixel1 += dst_next_pixel;
			}

		    while (xcount > 1) {	// write pixels in pairs
			Xil_signed16  *s  = src_pixel,
				  *db11 = dst_pixel1,
				  *db12 = dst_pixel1 + dst_next_pixel;
			int	bandcount = nbands;
			while(bandcount--) {
			    *db11++ = *s;
			    *db12++ = *s++;
			    }
			xcount -= 2;

			src_pixel += src_next_pixel;
			dst_pixel1 += dst_next_2pixel;
			}

		    if (xcount) {		// last pixel is odd
			int	bandcount = nbands;
			while(bandcount--)
			    *dst_pixel1++ = *src_pixel++;
			}
		    } // end last scan line is odd
    } // end rect list while loop
    
    return XIL_SUCCESS;
}

static XilStatus scale_zoom2x_general_storage_NN(AffineData affine_data)
{
    XilOp*       op            = affine_data.op;
    XilStorage*  src_storage   = affine_data.src_storage;
    XilStorage*  dst_storage   = affine_data.dst_storage;
    XilBox*      src_box       = affine_data.src_box;
    XilBox*      dst_box       = affine_data.dst_box;
    XilRoi*      roi           = affine_data.roi;
    unsigned int nbands        = affine_data.nbands;

    //
    // Create a rectangle list that is an intersection of
    // the intersected roi and dst_box. The list is
    // returned in the dst_box coordinate space (not
    // in the dst image coordinate space).
    //
    XilRectList rl(roi, dst_box);

    //
    //  Loop over rectangles in the destination.
    //
    int x;
    int y;
    unsigned int x_size;
    unsigned int y_size;
    while(rl.getNext(&x, &y, &x_size, &y_size)) {
        //
        //  For scale, 0,0 in source may not correspond to the 0,0 in the
        //  destination backward mapped into the source but it does map
        //  within the source box.  Call op->backwardMap() to get the
        //  source point within the box corresponding to x,y.
        //
        double srcx;
        double srcy;
        op->backwardMap(dst_box, (double)x, (double)y,
                        src_box, &srcx, &srcy);

        //
        // Determine the processing to apply to the first row and column
        // of the destination rect. Each source pixel might be forward
        // mapped into up to four destination pixels. Due to non-zero
        // origins it is possible that a given source pixel could be
        // forward mapped into _different_ destination rects. The optimized
        // algorithm used below generates destination lines as pairs of
        // identical lines so that if the first source line is not to be
        // replicated then it must be processed separately. Also, for
        // each band-version of the optimized algorithm there is a variant
        // for the cases in which the first pixel in the source line is
        // and is not replicated.
        //
        // The following tests determine whether the first line should
        // be processed separately and whether the first source pixel in
        // each line should be replicated. These tests are equivalent to
        // asking, respectively, does the dst pixel at (rect coordinates)
        // (x,y+1) backward map into the same src pixel as does the dst
        // pixel at (x,y), and does the dst pixel at (x+1,y) map into the
        // same src pixel as the one at (x,y)? The difference is that we
        // step +0.5 in a source image dimension rather than +1.0 in the
        // corresponding destination dimension and then backward mapping.
        //
        Xil_boolean   first_scanline_is_odd =
            ((int)srcy) != ((int)(srcy + 0.5)) ? TRUE : FALSE;
        Xil_boolean   first_pixel_is_odd    =
            ((int)srcx) != ((int)(srcx + 0.5)) ? TRUE : FALSE;

        for(int bandn = 0; bandn < nbands; bandn++) {
            //
            // Get the storage information
            //
            Xil_signed16* src_base_addr =
                (Xil_signed16 *) src_storage->getDataPtr(bandn);
            unsigned int  src_next_pixel = src_storage->getPixelStride(bandn);
            unsigned int  src_next_scan = src_storage->getScanlineStride(bandn);

            Xil_signed16* dst_base_addr =
                (Xil_signed16 *) dst_storage->getDataPtr(bandn);
            unsigned int  dst_next_pixel = dst_storage->getPixelStride(bandn);
            unsigned int  dst_next_scan = dst_storage->getScanlineStride(bandn);

            Xil_signed16* src_scanline = src_base_addr +
                ((int)srcy) * src_next_scan +
                ((int)srcx) * src_next_pixel;

	    Xil_signed16* dst_scanline1 = dst_base_addr +
		y * dst_next_scan +
		x * dst_next_pixel;
	    Xil_signed16* dst_scanline2 = dst_scanline1 + dst_next_scan;

            //
            // If the first line is odd, process it.
            //
            int ycount = y_size;
	    if ((first_scanline_is_odd == TRUE) && ycount) {
		Xil_signed16    *src_pixel = src_scanline,
				*dst_pixel1 = dst_scanline1;
		int		 xcount = x_size;

		if ((first_pixel_is_odd == TRUE) && xcount ) {
                    *dst_pixel1 = *src_pixel;
		    xcount --;

		    src_pixel += src_next_pixel;
		    dst_pixel1 += dst_next_pixel;
		    }

		while (xcount > 1) {	// write pixels in pairs
                    *dst_pixel1 = *src_pixel;
		    dst_pixel1 += dst_next_pixel;
                    *dst_pixel1 = *src_pixel;
		    xcount -= 2;

		    src_pixel += src_next_pixel;
		    dst_pixel1 += dst_next_pixel;
		    }

		if (xcount) {		// last pixel is odd
                    *dst_pixel1 = *src_pixel;
		    }

		src_scanline += src_next_scan;
		dst_scanline1 += dst_next_scan;
		dst_scanline2 = dst_scanline1 + dst_next_scan;
		ycount --;
		}

                //
                // Process remaining lines: do as pairs
                //
		while (ycount > 1) {	// write dst lines in pairs
		    Xil_signed16    *src_pixel = src_scanline,
				    *dst_pixel1 = dst_scanline1,
				    *dst_pixel2 = dst_scanline2;
		    int		     xcount = x_size;

		    if ((first_pixel_is_odd == TRUE) && xcount ) {
                        *dst_pixel1 = *dst_pixel2 = *src_pixel;
			xcount --;

			src_pixel += src_next_pixel;
			dst_pixel1 += dst_next_pixel;
			dst_pixel2 += dst_next_pixel;
			}

		    while (xcount > 1) {	//write dst pixels in pairs
                        *dst_pixel1 = *dst_pixel2 = *src_pixel;
			dst_pixel1 += dst_next_pixel;
			dst_pixel2 += dst_next_pixel;
                        *dst_pixel1 = *dst_pixel2 = *src_pixel;
			xcount -= 2;

			src_pixel += src_next_pixel;
			dst_pixel1 += dst_next_pixel;
			dst_pixel2 += dst_next_pixel;
			}

		    if (xcount) {		// last pixels are odd
                        *dst_pixel1 = *dst_pixel2 = *src_pixel;
			}

		    src_scanline += src_next_scan;
		    dst_scanline1 = dst_scanline2 + dst_next_scan;
		    dst_scanline2 = dst_scanline1 + dst_next_scan;
		    ycount -= 2;
		    }

		if (ycount) {		// last scan line is odd
		    Xil_signed16    *src_pixel = src_scanline,
				*dst_pixel1 = dst_scanline1;
		    int		 xcount = x_size;

		    if ((first_pixel_is_odd == TRUE) && xcount ) {
                        *dst_pixel1 = *src_pixel;
			xcount --;

			src_pixel += src_next_pixel;
			dst_pixel1 += dst_next_pixel;
			}

		    while (xcount > 1) {	// write pixels in pairs
                        *dst_pixel1 = *src_pixel;
			dst_pixel1 += dst_next_pixel;
                        *dst_pixel1 = *src_pixel;
			xcount -= 2;

			src_pixel += src_next_pixel;
			dst_pixel1 += dst_next_pixel;
			}

		    if (xcount) {		// last pixel is odd
                        *dst_pixel1 = *src_pixel;
		    }
            }
        } // band for loop
    } // rect list while loop

    return XIL_SUCCESS;
}
