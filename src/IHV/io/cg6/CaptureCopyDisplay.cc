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
//  File:	CaptureCopyDisplay.cc
//  Project:	XIL
//  Revision:	1.6
//  Last Mod:	10:13:56, 03/10/00
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
#pragma ident	"@(#)CaptureCopyDisplay.cc	1.6\t00/03/10  "

#include <stdio.h>
#include "XilDeviceIOcg6.hh"
#include "XiliUtils.hh"

XilStatus
XilDeviceIOcg6::captureCopyDisplay(XilOp*       op,
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
    //  Verify that the source image and the destination image is the same
    //  device image.
    //
    XilOp**      op_list = op->getOpList();

    if(op_list[2]->getSrcImage(1) != op_list[0]->getDstImage(1)) {
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
    //  Loop over each of the boxes of storage we are to process.
    //
    XilBox*    src_box;
    XilBox*    dst_box;
    while(bl->getNext(&src_box, &dst_box)) {
	//
	//  Get the co-ordinates of the box, we need to adjust to
	//  absolute image co-ordinates in the destination.
	//
	int          dst_x;
	int          dst_y;
	unsigned int dst_w;
	unsigned int dst_h;
	dst_box->getAsRect(&dst_x, &dst_y, &dst_w, &dst_h);
	
	int          src_x;
	int          src_y;
	unsigned int src_w;
	unsigned int src_h;
	src_box->getAsRect(&src_x, &src_y, &src_w, &src_h);

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
	XilRectList crl(roi, dgaClipList, winX, winY);

	//
	//  Note the rectlist is generated from the intersection
	//  of the clip rectlist and the box.
	//
	XilRectList rl(&crl, dst_box);

        //
        //  Get the clipping out of the way...
        //
        fb_fbc->l_fbc_clipminx = 0;
        fb_fbc->l_fbc_clipminy = 0;
        fb_fbc->l_fbc_clipmaxx = fb_width;
        fb_fbc->l_fbc_clipmaxy = fb_height;

        fb_fbc->l_fbc_rasteroffx= 0;
        fb_fbc->l_fbc_rasteroffy= 0;

        //
        //  This sets the raster operation to be done on each pixel as a copy.
        //
        ((unsigned int*)fb_fbc)[L_FBC_RASTEROP] = 0xed80cccc;
        fb_fbc->l_fbc_fcolor                    = 0xff;
        fb_fbc->l_fbc_bcolor                    = 0x00;

        //
        //  Move our boxes into device-space.
        //
	dst_x += winX;
	dst_y += winY;

	src_x += winX;
	src_y += winY;

	int          x;
	int          y;
	unsigned int x_size;
	unsigned int y_size;
        unsigned int sx;
        unsigned int sy;
        unsigned int dx;
        unsigned int dy;
	while(rl.getNext(&x, &y, &x_size, &y_size)) {
            sx = src_x + x;
            sy = src_y + y;
            //
            //  Set the source and destination blit areas.
            //
            fb_fbc->l_fbc_x0 = sx;
            fb_fbc->l_fbc_y0 = sy;
            fb_fbc->l_fbc_x1 = sx + x_size;
            fb_fbc->l_fbc_y1 = sy + y_size;

            dx = dst_x + x;
            dy = dst_y + y;
            fb_fbc->l_fbc_x2 = dx;
            fb_fbc->l_fbc_y2 = dy;
            fb_fbc->l_fbc_x3 = dx + x_size;
            fb_fbc->l_fbc_y3 = dy + y_size;

            //
            //  Check to make sure it was done in hardware -- if not fail.
            //
            int draw_status;
            draw_status = fb_fbc->l_fbc_blitstatus;
            if((draw_status & L_FBC_BLIT_HARDWARE) == 0) {
                if(bl->markAsFailed() == XIL_FAILURE) {
                    //
                    // Unlock the DGA window
                    //
                    unlockDGA();

                    return XIL_FAILURE;
                } else {
                    continue;
                }
            }

            if((draw_status = fb_fbc->l_fbc_blitstatus) < 0) {
                while(draw_status & L_FBC_FULL) {
                    draw_status = fb_fbc->l_fbc_blitstatus;
                }
            }
	}
    }

    //
    // Unlock the DGA window
    //
    unlockDGA();

    return XIL_SUCCESS;
}
