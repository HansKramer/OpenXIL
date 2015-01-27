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
//  File:	SetValue.cc
//  Project:	XIL
//  Revision:	1.6
//  Last Mod:	10:13:50, 03/10/00
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
#pragma ident	"@(#)SetValue.cc	1.6\t00/03/10  "

#include <stdio.h>
#include "XilDeviceIOcg6.hh"
#include "XiliUtils.hh"

XilStatus
XilDeviceIOcg6::setValueDisplay(XilOp*       op,
                                unsigned int ,
                                XilRoi*      roi,
                                XilBoxList*  bl)
{
    //
    //  Get the list of ops in this molecule.
    //
    XilOp**      op_list = op->getOpList();

    //
    //  Get the values to set...
    //
    Xil_unsigned8* value;
    op_list[1]->getParam(1, (void**)&value);

    //
    //  Loop over each of the boxes of storage we are to process.
    //
    XilBox*    dst_box;
    int        first = 1;
    while(bl->getNext(&dst_box)) {
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
        lockDGA();

	//
	//  Intersect the dgaClipList with the ROI, to give us a RectList
	//  for the entire image.
	//
	//  The dgaClipList is relative to the framebuffer and we need
	//  the window x, y to generate a good XilRectList
	//
	XilRectList    crl(roi, dgaClipList, winX, winY);

	//
	//  Update the retained image if needed, we only check the
	//  bs_ptr as only when this is valid do we actually need
	//  to do anything.
	//
	unsigned char* dst;
	if(bs_ptr) {
            //
            //  Note the rectlist is generated from the intersection
            //  of the ROI and the box.
            //
            XilRectList  rl(roi, dst_box);

            int          x;
            int          y;
            unsigned int x_size;
            unsigned int y_size;
            while(rl.getNext(&x, &y, &x_size, &y_size)) {
                dst = bs_ptr + y*rtn_linebytes + x;

                unsigned int height = y_size;
                do {
                    Xil_unsigned8* dst_pixel = dst;
                    unsigned int   width     = x_size;

                    do {
                        *dst_pixel++ = *value;
                    } while(--width);

                    dst += rtn_linebytes;
                } while(--height);
            }
	}

        //
        //  Get the clipping out of the way?
        //
        fb_fbc->l_fbc_clipminx = 0;
        fb_fbc->l_fbc_clipminy = 0;
        fb_fbc->l_fbc_clipmaxx = fb_width;
        fb_fbc->l_fbc_clipmaxy = fb_height;

        fb_fbc->l_fbc_rasteroffx= 0;
        fb_fbc->l_fbc_rasteroffy= 0;

        //
        //  Set the foreground color on the GX
        //
        fb_fbc->l_fbc_fcolor = *value;

        //
        //  Specify the RASTEROP
        //
        ((unsigned int*)fb_fbc)[L_FBC_RASTEROP]=0xed80ff00;

	//
	//  Note the rectlist is generated from the intersection
	//  of the clip rectlist and the box.
	//
	XilRectList  rl(&crl, dst_box);

	int          x;
	int          y;
	unsigned int x_size;
	unsigned int y_size;

	abs_x += winX;
	abs_y += winY;
	while(rl.getNext(&x, &y, &x_size, &y_size)) {
            x += abs_x;
            y += abs_y;

            fb_fbc->l_fbc_irectabsy = y;
            fb_fbc->l_fbc_irectabsx = x;
            fb_fbc->l_fbc_irectabsy = (y+y_size-1);
            fb_fbc->l_fbc_irectabsx = (x+x_size-1);

            int draw_status;
            if((draw_status = fb_fbc->l_fbc_drawstatus) < 0) {
                while(draw_status & L_FBC_FULL)
                    draw_status = fb_fbc->l_fbc_drawstatus;
            }
	}
	
	//
	// Unlock the DGA window
	//
        unlockDGA();
    }
	    
    return XIL_SUCCESS;
}
