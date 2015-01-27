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
//  File:	WindowLevel.cc
//  Project:	XIL
//  Revision:	1.3
//  Last Mod:	10:13:51, 03/10/00
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
#pragma ident	"@(#)WindowLevel.cc	1.3\t00/03/10  "

#include <stdio.h>
#include "XilDeviceIOcg6.hh"
#include "XiliUtils.hh"

XilStatus
XilDeviceIOcg6::winlevDisplayPre(XilOp*        op,
                                 unsigned      ,
                                 XilRoi*       ,
                                 void**        compute_data,
                                 unsigned int* )
{
    XilOp** oplist = op->getOpList();

    Xil_signed16* low1;
    Xil_signed16* high1;
    Xil_signed16* map1;
    oplist[2]->getParam(1, (void**)&low1);
    oplist[2]->getParam(2, (void**)&high1);
    oplist[2]->getParam(3, (void**)&map1);

    Xil_signed16* low2;
    Xil_signed16* high2;
    Xil_signed16* map2;
    oplist[3]->getParam(1, (void**)&low2);
    oplist[3]->getParam(2, (void**)&high2);
    oplist[3]->getParam(3, (void**)&map2);

    float* mul_const;
    float* add_const;
    oplist[4]->getParam(1, (void**)&mul_const);
    oplist[4]->getParam(2, (void**)&add_const);

    XilSystemState* state = op->getDstImage(1)->getSystemState();

    //
    //  Get the cache number we can use...
    //
    *compute_data =
        (void*)deviceManager->getWinlevTable(state, *mul_const, *add_const,
                                             *low1, *high1, *map1,
                                             *low2, *high2, *map2);

    return XIL_SUCCESS;
}

XilStatus
XilDeviceIOcg6::winlevDisplayPost(XilOp* ,
                                   void*  compute_data)
{
    deviceManager->releaseWinlevTable((int)compute_data);

    return XIL_SUCCESS;
}

XilStatus
XilDeviceIOcg6::winlevDisplay(XilOp*       op,
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
    //  Get the table we're expected to use.
    //
    int table = (int)op->getPreprocessData(this);
    if(table == -1) {
        return XIL_FAILURE;
    }

    //
    //  Get the actual data for the table...
    //
    Xil_unsigned8* lut = deviceManager->refWinlevCache(table);

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
    XilImage*    srcImage = op_list[4]->getSrcImage(1);

    //
    //  Loop over each of the boxes of storage we are to process.
    //
    XilBox*    src_box;
    XilBox*    dst_box;
    while(bl->getNext(&src_box, &dst_box)) {
	//
        //  Aquire our storage from the source image.  The storage returned is valid
        //  for the box given.  Thus, any origins or child offsets have been
        //  taken into account.
        //
        XilStorage  src_storage(srcImage);
	if(srcImage->getStorage(&src_storage, op_list[4], src_box, "XilMemory",
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
        //  Set the RASTEROP.
        //
        ((unsigned int *)fb_fbc)[L_FBC_RASTEROP]=0xed80cccc;

        //
        //  Set up autoincrement for the fonting
        //
        fb_fbc->l_fbc_autoincx= 4;
        fb_fbc->l_fbc_autoincy= 0;

        //
        //  Set 8 bit mode
        //
        fb_fbc->l_fbc_misc.l_fbc_misc_data = L_FBC_MISC_DATA_COLOR8;

        //
        //  Don't worry about y clipping
        //
        fb_fbc->l_fbc_clipminy = 0;
        fb_fbc->l_fbc_clipmaxy = fb_height;

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
	unsigned int  src_pixel_stride;
	unsigned int  src_scanline_stride;
	Xil_signed16* src_data;
	src_storage.getStorageInfo(&src_pixel_stride,
				   &src_scanline_stride,
				   NULL, NULL,
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
            int xcount = x_size>>2;
            int edge   = x_size & 3;
        
            Xil_signed16* src  = src_data +
                y*src_scanline_stride +
                x*src_pixel_stride;

            x += abs_x;
            y += abs_y;
        
            fb_fbc->l_fbc_clipminx = (unsigned int)x;
            fb_fbc->l_fbc_clipmaxx = (unsigned int)(x+x_size-1);
        
            for(int i=0; i<y_size; i++) {
                fb_fbc->l_fbc_x0 = (unsigned int)x;
                fb_fbc->l_fbc_x1 = (unsigned int)(x + 3);
                fb_fbc->l_fbc_y0 = (unsigned int)(y + i);

                if(src_pixel_stride==1) {
                    unsigned int value;

                    for(int j=0; j<xcount; j++) {
                        value  = lut[src[0]]<<24;
                        value |= lut[src[1]]<<16;
                        value |= lut[src[2]]<<8;
                        value |= lut[src[3]];
                        fb_fbc->l_fbc_font = value;
                        src += 4;
                    }
                    value = 0;

                    switch(edge) {
                      case 3:
                        value  = lut[src[0]] << 24;
                        value |= lut[src[1]] << 16;
                        value |= lut[src[2]] << 8;
                        fb_fbc->l_fbc_font = value;
                        src += 3;
                        break;
                    
                      case 2:
                        value  = lut[src[0]] << 24;
                        value |= lut[src[1]] << 16;
                        fb_fbc->l_fbc_font = value;
                        src += 2;
                        break;

                      case 1:
                        value = lut[src[0]] << 24;
                        fb_fbc->l_fbc_font = value;
                        src++;
                        break;
                    }
                
                    src += src_scanline_stride - x_size;
                } else {
                    unsigned int value;

                    for(int j=0; j<xcount; j++) {
                        value  = lut[*src] << 24;
                        src+=src_pixel_stride;

                        value |= (lut[*src]<<16);
                        src+=src_pixel_stride;

                        value |= (lut[*src]<<8);
                        src+=src_pixel_stride;

                        value |= lut[*src];
                        src+=src_pixel_stride;

                        fb_fbc->l_fbc_font= value;
                    }
                    value = 0;
                
                    switch(edge) {
                      case 3:
                        value  = lut[*src] << 24;
                        src+=src_pixel_stride;

                        value |= (lut[*src]<<16);
                        src+=src_pixel_stride;

                        value |= (lut[*src]<<8);
                        src+=src_pixel_stride;

                        fb_fbc->l_fbc_font= value;
                        break;

                      case 2:
                        value  = lut[*src] << 24;
                        src += src_pixel_stride;

                        value |= (lut[*src]<<16);
                        src+=src_pixel_stride;

                        fb_fbc->l_fbc_font= value;
                        break;

                      case 1:
                        value  = lut[*src] << 24;
                        src += src_pixel_stride;

                        fb_fbc->l_fbc_font= value;
                        break;
                    }

                    src += src_scanline_stride -
                        x_size*src_pixel_stride;
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
