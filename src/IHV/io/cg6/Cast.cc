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
//  File:	Cast.cc
//  Project:	XIL
//  Revision:	1.3
//  Last Mod:	10:13:55, 03/10/00
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
#pragma ident	"@(#)Cast.cc	1.3\t00/03/10  "

#include <stdio.h>
#include "XilDeviceIOcg6.hh"
#include "XiliUtils.hh"

XilStatus
XilDeviceIOcg6::cast1to8Display(XilOp*       op,
                                unsigned int ,
                                XilRoi*      roi,
                                XilBoxList*  bl)
{
    return generic1to8(op, roi, bl, 1, 0);
}

XilStatus
XilDeviceIOcg6::lookup1to8Display(XilOp*       op,
                                  unsigned int ,
                                  XilRoi*      roi,
                                  XilBoxList*  bl)
{
    XilLookupSingle* lookup;

    op->getOpList()[1]->getParam(1, (XilObject**)&lookup); 

    const Xil_unsigned8* data = (const Xil_unsigned8*)lookup->getData();

    if((lookup->getOffset() == 0) && (lookup->getNumEntries() == 2)) {
        return generic1to8(op, roi, bl, data[1], data[0]);
    } else {
        return generic1to8(op, roi, bl, data[0], data[0]);
    }
}

XilStatus
XilDeviceIOcg6::generic1to8(XilOp*       op,
                            XilRoi*      roi,
                            XilBoxList*  bl,
                            unsigned int fcolor,
                            unsigned int bcolor)
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
    XilOp**      op_list = op->getOpList();

    //
    //  Get our source image. The display op guarantees that if we have
    //  a parent image it will pass it in on the op.
    //
    XilImage*    srcImage = op_list[1]->getSrcImage(1);

    //
    //  Loop over each of the boxes of storage we are to process.
    //
    XilBox*    src_box;
    XilBox*    dst_box;
    int        first = 1;
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
	//  Lock the DGA window for our use.
	//
	lockDGA(FALSE);
	
        //
        //  Setup the raster offsets...
        //
        fb_fbc->l_fbc_rasteroffx= 0;
        fb_fbc->l_fbc_rasteroffy= 0;

        //
        //  Set the foreground and background.
        //
        fb_fbc->l_fbc_bcolor=bcolor;
        fb_fbc->l_fbc_fcolor=fcolor;

        //
        //  Set the RASTEROP.
        //
        ((unsigned int *)fb_fbc)[L_FBC_RASTEROP]=0xe980fc30;

        //
        //  Set up autoincrement for the fonting
        //
        fb_fbc->l_fbc_autoincx= 32;
        fb_fbc->l_fbc_autoincy= 0;

        //
        //  Set 1 bit mode
        //
        fb_fbc->l_fbc_misc.l_fbc_misc_data = L_FBC_MISC_DATA_COLOR1;

        //
        //  Don't worry about y clipping
        //
        fb_fbc->l_fbc_clipminy = 0;
        fb_fbc->l_fbc_clipmaxy = fb_height;
        fb_fbc->l_fbc_clipminx = 0;
        fb_fbc->l_fbc_clipmaxx = 1000;

	//
	//  Intersect the dgaClipList with the ROI, to give us a RectList
	//  for the entire image.
	//
	//  The dgaClipList is relative to the framebuffer and we need
	//  the window x, y to generate a good XilRectList
	//
	XilRectList    crl(roi, dgaClipList, winX, winY);

	//
	//  The cg6 only supports single banded images which
	//  default to pixel sequential so no check is made
	//  here for storage type.
	//
	unsigned int   src_pixel_stride;
	unsigned int   src_scanline_stride;
	unsigned int   src_offset;
	Xil_unsigned8* src_data;
	src_storage.getStorageInfo(&src_pixel_stride,
				   &src_scanline_stride,
				   NULL,
                                   &src_offset,
				   (void**)&src_data);

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
            fb_fbc->l_fbc_clipminx = x + abs_x;
            fb_fbc->l_fbc_clipmaxx = x + abs_x + x_size-1;

            Xil_unsigned8* src = src_data + y*src_scanline_stride + x;

            for(int pass=0; pass<4; pass++) {
                src = src_data + (y + pass)*src_scanline_stride + (x>>3);
            
                int offset = src_offset + (x & 7);
                if(offset>7) {
                    offset-=8;
                    src++;
                }

                int          xcount = x_size;
                unsigned int tmp_x  = x;
                unsigned int tmp_y  = y;

                if(offset || ((int)src & 3)) {
                    int pixel = offset;

                    if((int)src & 3) {
                        pixel += (((int)src & 3)<<3);
                    }

                    tmp_x  -= (pixel & 0x1f);
                    xcount += (pixel & 0x1f);
                
                    src = (Xil_unsigned8*)((int)src & 0xFFFFFFFC);
                }
                xcount = (xcount+31)&0xFFFFFFE0;

                for(int k=pass; k<y_size; k+=4) {
                    fb_fbc->l_fbc_x0 =
                        (unsigned int)tmp_x + abs_x;
                    fb_fbc->l_fbc_x1 =
                        (unsigned int)tmp_x + abs_x + 31;
                    fb_fbc->l_fbc_y0 =
                        (unsigned int)tmp_y + abs_y + k;
                
                    for(int i=0; i<xcount; i+=32) {
                        fb_fbc->l_fbc_font= *(int*)src;
                        src += 4;
                    }

                    src += src_scanline_stride - (xcount>>3);
                    src += src_scanline_stride;
                    src += src_scanline_stride;
                    src += src_scanline_stride;
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

