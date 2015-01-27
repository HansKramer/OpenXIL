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
//  File:	ScaleNearest.cc
//  Project:	XIL
//  Revision:	1.11
//  Last Mod:	10:13:56, 03/10/00
//
//  Description:
//	
//	Molecule to do a scale_nearest;8 followed by optional copy;8 and
//	then a display_SUNWcg6.  Uses the GX to accellerate the scaling.
//	See the note on usage restrictions.
//	
//	
//	
//	
//  MT-level:  <??????>
//	
//------------------------------------------------------------------------
//	COPYRIGHT
//------------------------------------------------------------------------
#pragma ident	"@(#)ScaleNearest.cc	1.11\t00/03/10  "

#include <stdio.h>
#include "XilDeviceIOcg6.hh"
#include "XiliUtils.hh"

XilStatus
XilDeviceIOcg6::scaleNearestDisplay(XilOp*       op,
				    unsigned int ,
				    XilRoi*      roi,
				    XilBoxList*  bl)
{
    //
    //  Molecule doesn't support backing store.
    //
    if(bs_ptr) {
        return XIL_FAILURE;
    }

    //
    //  Split the list of XilBoxes to take tile boundaries into account.  This
    //  will work to ensure that no cobbling of the data is required because
    //  the boxes will not cross tile boundaries in the source images.
    //
    if(op->splitOnTileBoundaries(bl) == XIL_FAILURE) {
	return XIL_FAILURE;
    }
    
    //
    //  Get the list of ops in this molecule.
    //
    XilOp* scale_op = op->getOpList()[1];

    //
    //  Get our source image. The display op guarantees that if we have
    //  a parent image it will pass it in on the op.
    //
    XilImage* srcImage = scale_op->getSrcImage(1);

    //
    //  Get the X and Y scale factors
    //
    float xscale;
    float yscale;
    scale_op->getParam(1, &xscale);
    scale_op->getParam(2, &yscale);

    //
    //  Scale factor must be 2 in both directions
    //
    if(! XILI_FLT_EQ(xscale, 2.0F) ||
       ! XILI_FLT_EQ(yscale, 2.0F)) {
	return XIL_FAILURE;
    }

    //
    //  Loop over each of the boxes of storage we are to process.
    //
    XilBox*    src_box;
    XilBox*    dst_box;
    while(bl->getNext(&src_box, &dst_box)) {
	//
        //  Aquire our storage from the images.  The storage returned is valid
        //  for the box given.  Thus, any origins or child offsets have been
        //  taken into account.
        //
        XilStorage  src_storage(srcImage);
	if(srcImage->getStorage(&src_storage, scale_op, src_box, "XilMemory",
                                XIL_READ_ONLY) == XIL_FAILURE) {
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
	//  Get the co-ordinates of the box, we need to adjust to
	//  absolute image co-ordinates in the destination.
	//
	int          abs_x;
	int          abs_y;
	unsigned int abs_w;
	unsigned int abs_h;
	dst_box->getAsRect(&abs_x, &abs_y, &abs_w, &abs_h);

	//
	//  Lock the DGA window for our use.
	//
	lockDGA(FALSE);
	
	//
	//  Intersect the dgaClipList with the ROI, to give us a RectList
	//  for the entire image.
	//
	//  The dgaClipList is relative to the framebuffer and we need
	//  the window x, y to generate a good XilRectList
	//
	XilRectList    crl(roi, dgaClipList, winX, winY);

	//
	// The cg6 only supports single banded images which
	// default to pixel sequential so no check is made
	// here for storage type.
	// 
	unsigned int   src_pixel_stride;
	unsigned int   src_scanline_stride;
	Xil_unsigned8* src_data;
	src_storage.getStorageInfo(&src_pixel_stride,
				   &src_scanline_stride,
				   NULL, NULL,
				   (void**)&src_data);

        Xil_boolean    not_fast_storage =
            src_pixel_stride != 1 || src_scanline_stride & 3;
        
	//
	// Init GX for FONT and BLT operations
	//
	((int*)fb_fbc)[L_FBC_MISC] =(L_FBC_MISC_BLIT_NOSRC << 20) |
	    ((L_FBC_MISC_DATA_COLOR1&3) << 17) |
	    (L_FBC_MISC_DRAW_RENDER << 15);
	fb_fbc->l_fbc_rasteroffx = 0;
	fb_fbc->l_fbc_rasteroffy = 0;
	fb_fbc->l_fbc_clipcheck = 0;
	fb_fbc->l_fbc_pixelmask = 0xffffffff;
	fb_fbc->l_fbc_planemask = 0xff;
	((int*)fb_fbc)[L_FBC_RASTEROP] = 0xe980cccc;
	fb_fbc->l_fbc_autoincx = 4;
	fb_fbc->l_fbc_autoincy = 0;
	fb_fbc->l_fbc_misc.l_fbc_misc_data = L_FBC_MISC_DATA_COLOR8;
  
        //
        //  Don't worry about clipping in y...
        //
	fb_fbc->l_fbc_clipminy = 0;
	fb_fbc->l_fbc_clipmaxy = fb_height;

	//
	//  Note the rectlist is generated from the intersection
	//  of the clip rectlist and the box.
	//
	XilRectList  rl(&crl, dst_box);

        //
	//  Convert dst_box to absolute screen coordinates, (0,0) is top
	//  left of screen.
        //
	abs_x += winX;
	abs_y += winY;

        //
	//  src_line_word_stride is the scanline stride in 32-bit words
        //
	unsigned int src_line_word_stride = src_scanline_stride / 4;

	int          x;
	int          y;
	unsigned int x_size;
	unsigned int y_size;
	while(rl.getNext(&x, &y, &x_size, &y_size)) {
            //
            //  For scale, 0,0 in source may not correspond to the 0,0 in the
            //  destination backward mapped into the source but it does map
            //  within the source box.  Call the scale op to backward map x,y
            //  into the source box. 
            //
            double srcx;
            double srcy;
            scale_op->backwardMap(dst_box, (double)x, (double)y,
                                  src_box, &srcx, &srcy);

	    Xil_unsigned8* src_scanline = src_data +
                ((int)srcy)*src_scanline_stride +
                ((int)srcx)*src_pixel_stride;

            //
	    //  These are in absolute screen coordinates
            //
	    int xstart = abs_x + x;
	    int xend   = xstart + x_size - 1;
	    int ystart = abs_y + y;
	    int yend   = ystart + y_size - 1;

            Xil_boolean do_double = ! ((ystart - winY) & 1);

            //
            //  Setup X clipping
            //
            fb_fbc->l_fbc_clipminx = xstart;
            fb_fbc->l_fbc_clipmaxx = xend;

            if(not_fast_storage || ((long)src_scanline) & 0x3) {
                //
                //  We're not starting on word aligned boundary or our source
                //  storage is not packed properly.
                //
                for(int i_y = ystart; i_y <= yend;) {
                    //
                    //  Set up GX for FONT operation
                    //
                    fb_fbc->l_fbc_y0 = i_y;
                    fb_fbc->l_fbc_x0 = xstart;
                    fb_fbc->l_fbc_x1 = xstart + 4;

                    //
                    //  Scale 4 src pixels to 8 dest pixels
                    //
                    Xil_unsigned8* src_pixels = src_scanline;

                    for(int i_x = xstart; i_x < xend; i_x += 8) {
                        unsigned int v1 = *src_pixels;
                        src_pixels += src_pixel_stride;

                        unsigned int v2 = *src_pixels;
                        fb_fbc->l_fbc_font =
                            (v1<<24) | (v1<<16) | (v2<<8) | v2;

                        src_pixels += src_pixel_stride;

                        v1 = *src_pixels;
                        src_pixels += src_pixel_stride;

                        v2 = *src_pixels;
                        fb_fbc->l_fbc_font =
                            (v1<<24) | (v1<<16) | (v2<<8) | v2;

                        src_pixels += src_pixel_stride;
                    }

                    src_scanline += src_scanline_stride;

                    if(do_double) {
                        //
                        //  Tell GX to copy the row of pixels to the next line
                        //  using the GX BLT function
                        //
                        fb_fbc->l_fbc_x0 = xstart;
                        fb_fbc->l_fbc_x1 = xend - 1;
                        fb_fbc->l_fbc_y0 = i_y;
                        fb_fbc->l_fbc_y1 = i_y;
                        fb_fbc->l_fbc_x2 = xstart;
                        fb_fbc->l_fbc_x3 = xend - 1;
                        fb_fbc->l_fbc_y2 = i_y + 1;
                        fb_fbc->l_fbc_y3 = i_y + 1;
                        int do_blit = fb_fbc->l_fbc_blitstatus;

                        i_y += 2;

                        //
                        //  Wait until GX is done
                        //
                        while(fb_fbc->l_fbc_status & L_FBC_BUSY);
                    } else {
                        i_y += 1;
                        do_double = 1;
                    }
                }
            } else {
                for(int i_y = ystart; i_y <= yend;) {
                    //
                    //  Set up GX for FONT operation
                    //
                    fb_fbc->l_fbc_y0 = i_y;
                    fb_fbc->l_fbc_x0 = xstart;
                    fb_fbc->l_fbc_x1 = xstart + 4;

                    //
                    //  Scale 4 src pixels to 8 dest pixels
                    //
                    Xil_unsigned32* src_pixels = (Xil_unsigned32*)src_scanline;

                    for(int i_x = xstart; i_x < xend; i_x += 8) {
                        //
                        //  Get 4 src pixels "abcd"
                        //
                        Xil_unsigned32 abcd = (Xil_unsigned32)*src_pixels++;

                        //
                        //  tmp = "a000"
                        //
                        unsigned int tmp = abcd & 0xff000000;

                        //
                        //  tmp = "a000" | "00b0" = "a0b0"
                        //
                        tmp = tmp | (abcd >> 8) & 0x0000ff00;

                        //
                        //  tmp = "a0b0" | "0a0b" = "aabb"
                        //
                        tmp = tmp | (tmp >> 8);

                        //
                        //  Write "aabb"
                        //
                        fb_fbc->l_fbc_font = tmp;

                        
                        //
                        //  tmp = "000d"
                        //
                        tmp = abcd & 0xff;

                        //
                        //  tmp = "000d" | "0c00" = "0c0d"
                        //
                        tmp = tmp | ((abcd & 0xff00) << 8);

                        //
                        //  tmp = "0c0d" | "c0d0" = "ccdd"
                        //
                        tmp = tmp | (tmp << 8);

                        //
                        //  Write "ccdd"
                        //
                        fb_fbc->l_fbc_font = tmp;
                    }

                    src_scanline += src_scanline_stride;

                    if(do_double) {
                        //
                        //  Tell GX to copy the row of pixels to the next line
                        //  using the GX BLT function
                        //
                        fb_fbc->l_fbc_x0 = xstart;
                        fb_fbc->l_fbc_x1 = xend;
                        fb_fbc->l_fbc_y0 = i_y;
                        fb_fbc->l_fbc_y1 = i_y;
                        fb_fbc->l_fbc_x2 = xstart;
                        fb_fbc->l_fbc_x3 = xend;
                        fb_fbc->l_fbc_y2 = i_y + 1;
                        fb_fbc->l_fbc_y3 = i_y + 1;
                        int do_blit = fb_fbc->l_fbc_blitstatus;

                        i_y += 2;

                        //
                        //  Wait until GX is done
                        //
                        while(fb_fbc->l_fbc_status & L_FBC_BUSY);
                    } else {
                        i_y += 1;
                        do_double = 1;
                    }
                }
	    }
	}
	
	//
	// Unlock the DGA window
	//
	unlockDGA();
    }
	    
    return XIL_SUCCESS;
}
