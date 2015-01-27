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
//  File:	Copy.cc
//  Project:	XIL
//  Revision:	1.7
//  Last Mod:	10:13:58, 03/10/00
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
#pragma ident	"@(#)Copy.cc	1.7\t00/03/10  "

#include <stdio.h>
#include "XilDeviceIOcg6.hh"
#include "XiliUtils.hh"

XilStatus
XilDeviceIOcg6::copyDisplay(XilOp*       op,
                            unsigned int ,
                            XilRoi*      roi,
                            XilBoxList*  bl)
{
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
    XilOp** op_list = op->getOpList();

    //
    //  Get our source image. The display op guarantees that if we have
    //  a parent image it will pass it in on the op.
    //
    XilImage* srcImage = op_list[1]->getSrcImage(1);

    //
    //  Loop over each of the boxes of storage we are to process.
    //
    XilBox* src_box;
    XilBox* dst_box;
    while(bl->getNext(&src_box, &dst_box)) {
	//
        //  Aquire our storage from the source image.  The storage returned is valid
        //  for the box given.  Thus, any origins or child offsets have been
        //  taken into account.
        //
        XilStorage  src_storage(srcImage);
	if(srcImage->getStorage(&src_storage, op_list[1], src_box, "XilMemory",
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
	//  Lock the DGA window for our use -- but not the registers.
	//
	lockDGA(TRUE, FALSE);

	//
	//  Intersect the dgaClipList with the ROI, to give us a RectList
	//  for the entire image.
	//
	//  The dgaClipList is relative to the framebuffer and we need
	//  the window x, y to generate a good XilRectList
	//
	XilRectList crl(roi, dgaClipList, winX, winY);

	//
	//  The cg6 only supports single banded images which
	//  default to pixel sequential so no check is made
	//  here for storage type.
	//
	unsigned int   src_pixel_stride;
	unsigned int   src_scanline_stride;
	Xil_unsigned8* src_data;
	src_storage.getStorageInfo(&src_pixel_stride,
				   &src_scanline_stride,
				   NULL, NULL,
				   (void**)&src_data);

        if(src_pixel_stride == 1) {
            //
            //  Update the retained image if needed, we only check the
            //  bs_ptr as only when this is valid do we actually need
            //  to do anything.
            //
            unsigned char* src;
            unsigned char* dst;
            if(bs_ptr != NULL) {
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
                    src = src_data + y*rtn_linebytes + x;
                    dst = bs_ptr + y*rtn_linebytes + x;

                    do {
                        xili_memcpy(dst, src, x_size);

                        dst += rtn_linebytes;
                        src += src_scanline_stride;
                    } while(--y_size);
                }
            }

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
                dst = fb_mem + (y+abs_y)*fb_width + (x+abs_x);
                src = src_data + y*src_scanline_stride + x;

                do {
                    xili_memcpy(dst, src, x_size);

                    dst += fb_width;
                    src += src_scanline_stride;
                } while(--y_size);
            }
        } else {
            //
            //  Update the retained image if needed, we only check the
            //  bs_ptr as only when this is valid do we actually need
            //  to do anything.
            //
            unsigned char* src;
            unsigned char* dst;
            if(bs_ptr != NULL) {
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
                    src = src_data + y*rtn_linebytes + x;
                    dst = bs_ptr + y*rtn_linebytes + x;

                    do {
                        Xil_unsigned8* src_pixel = src;
                        Xil_unsigned8* dst_pixel = dst;
                        unsigned int   width     = x_size;

                        do {
                            *dst_pixel++ = *src_pixel;

                            src_pixel += src_pixel_stride;
                        } while(--width);

                        dst += rtn_linebytes;
                        src += src_scanline_stride;
                    } while(--y_size);
                }
            }

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
                dst = fb_mem + (y+abs_y)*fb_width + (x+abs_x);

                src = src_data + y*src_scanline_stride + x*src_pixel_stride;

                do {
                    Xil_unsigned8* src_pixel = src;
                    Xil_unsigned8* dst_pixel = dst;
                    unsigned int   width     = x_size;

                    do {
                        *dst_pixel++ = *src_pixel;

                        src_pixel += src_pixel_stride;
                    } while(--width);

                    dst += fb_width;
                    src += src_scanline_stride;
                } while(--y_size);
            }
        }
	
	//
	//  Unlock the DGA window -- but not the registers.
	//
	unlockDGA(FALSE);
    }


	    
    return XIL_SUCCESS;
}
