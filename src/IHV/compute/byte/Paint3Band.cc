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
//  File:	Paint3Band.cc
//  Project:	XIL
//  Revision:	1.7
//  Last Mod:	10:10:56, 03/10/00
//
//  Description:
//	
//	
//	
//	
//  MT-level:  Safe
//	
//------------------------------------------------------------------------
//	COPYRIGHT
//------------------------------------------------------------------------
#pragma ident	"@(#)Paint3Band.cc	1.7\t00/03/10  "

#include "XiliUtils.hh"
#include "XilDeviceManagerComputeBYTE.hh"

XilStatus
XilDeviceManagerComputeBYTE::Paint3BandPreprocess(XilOp*        op,
                                                  unsigned      ,
                                                  XilRoi*       ,
                                                  void**        compute_data,
                                                  unsigned int* )
{
    //
    //  Check nbands == 3...
    //
    if(op->getDstImage(1)->getNumBands() != 3) {
        return XIL_FAILURE;
    }

    //
    //  Get color and the kernel
    //
    float*      color;
    op->getParam(1, (void**)&color);

    XilKernel*  kernel;
    op->getParam(2, (XilObject**)&kernel);

    //
    //  Check each of our caches to see if one of them contains the same
    //  inforamtion as our operation.
    //
    pcacheMutex.lock();

    int empty_table = -1;
    for(int i=0; i<_XILI_NUM_PAINT_BRUSHES; i++) {
        if(pcacheBrush[i] == NULL) {
            if(empty_table == -1 || pcacheBrush[empty_table] != NULL) {
                empty_table = i;
            }

            continue;
        }

        if(kernel->isSameAs(&pcacheKernelVersion[i]) &&
           color[0] == pcacheBrushColor[i][0] &&
           color[1] == pcacheBrushColor[i][1] &&
           color[2] == pcacheBrushColor[i][2]) {
            //
            //  Found one-- indicate which cache to use and bump the ref cnt.
            //
            *compute_data = (void*)i;
            pcacheRefCnts[i]++;

            pcacheMutex.unlock();
            return XIL_SUCCESS;
        }

        //
        //  Need to rebuild cache -- store whether this is an unused cache.
        //
        if(pcacheRefCnts[i] == 0 &&
           empty_table      == -1) {
            empty_table = i;
        }
    }

    if(empty_table != -1) {
        int et = empty_table;

        kernel->getVersion(&pcacheKernelVersion[et]);

        pcacheBrushColor[et][0] = color[0];
        pcacheBrushColor[et][1] = color[1];
        pcacheBrushColor[et][2] = color[2];

        delete pcacheBrush[et];

        unsigned int  bwidth  = kernel->getWidth();
        unsigned int  bheight = kernel->getHeight();
        const float*  bvals   = kernel->getData();

        pcacheBrush[et] = new float [bwidth * bheight * 3];
        if(pcacheBrush[et] == NULL) {
            XIL_ERROR(op->getDstImage(1)->getSystemState(),
                      XIL_ERROR_RESOURCE, "di-1", TRUE);
            pcacheMutex.unlock();
            return XIL_FAILURE;
        }

        float* bptr = pcacheBrush[et];
        for(unsigned int j=0; j<bwidth*bheight; j++) {
            *bptr     = *bvals * *color     + 0.5F;
            *(bptr+1) = *bvals * *(color+1) + 0.5F;
            *(bptr+2) = *bvals * *(color+2) + 0.5F;

            bvals++;
            bptr += 3;
        }

        //
        //  Indicate which cache to use and bump the ref cnt.
        //
        *compute_data = (void*)et;
        pcacheRefCnts[et]++;

        pcacheMutex.unlock();
        return XIL_SUCCESS;
    }

    //
    //  Couldn't get a cache so let the regular version handle it...
    //
    pcacheMutex.unlock();
    return XIL_FAILURE;
}

XilStatus
XilDeviceManagerComputeBYTE::Paint3BandPostprocess(XilOp*       ,
                                                   void*        compute_data)
{
    pcacheMutex.lock();

    pcacheRefCnts[(int)compute_data]--;

    pcacheMutex.unlock();

    return XIL_SUCCESS;
}

XilStatus
XilDeviceManagerComputeBYTE::Paint3Band(XilOp*       op,
                                        unsigned int   ,
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
    //  Get the images for our operation.
    //
    XilImage* src1 = op->getSrcImage(1);
    XilImage* dest = op->getDstImage(1);

    //
    // Get kernel information
    //
    XilKernel* brush;
    op->getParam(2, (XilObject**)&brush);

    int          key_x         = brush->getKeyX();
    int          key_y         = brush->getKeyY();
    unsigned int kernel_width  = brush->getWidth();
    unsigned int kernel_height = brush->getHeight();
    const float* brush_data    = brush->getData();

    //
    //  Get coordinate list information.
    //
    unsigned int coordinate_count;
    op->getParam(3, &coordinate_count);
    
    Xil_signed32* coordinate_list;
    op->getParam(4, (void**)&coordinate_list);
    
    //
    //  Get cache information
    //
    unsigned int cache_num   = (unsigned int)op->getPreprocessData(this);
    float*       cache_brush = pcacheBrush[cache_num];
    
    //
    //  Loop over each of the boxes of storage we are to process.
    //
    XilBox* src1_box;
    XilBox* dest_box;
    while(bl->getNext(&src1_box, &dest_box)) {
        //
        //  Aquire our storage from the images.  The storage returned is valid
        //  for the box given.  Thus, any origins or child offsets have been
        //  taken into account.
        //
        XilStorage  src1_storage(src1);
        XilStorage  dest_storage(dest);
        if((src1->getStorage(&src1_storage, op, src1_box, "XilMemory",
                             XIL_READ_ONLY)  == XIL_FAILURE) ||
           (dest->getStorage(&dest_storage, op, dest_box, "XilMemory",
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
        //  Test to see if all of our storage is of type XIL_PIXEL_SEQUENTIAL.
        //  If so, implement an loop optimized fro pixel-sequential storage.
        //
        if(! (src1_storage.isType(XIL_PIXEL_SEQUENTIAL)) ||
	   ! (dest_storage.isType(XIL_PIXEL_SEQUENTIAL))) {
            //
            //  We only handle pixel-sequential.  If we encounter a box that
            //  is not pixel sequential, we mark it as failed for the regular
            //  routine to take over.
            // 
            if(bl->markAsFailed() == XIL_FAILURE) {
                return XIL_FAILURE;
            } else {
                continue;
            }
        } else {
            unsigned int   src1_pixel_stride;
            unsigned int   src1_scanline_stride;
            Xil_unsigned8* src1_data;
            src1_storage.getStorageInfo(&src1_pixel_stride,
                                        &src1_scanline_stride,
                                        NULL, NULL,
                                        (void**)&src1_data);
            
            unsigned int   dest_pixel_stride;
            unsigned int   dest_scanline_stride;
            Xil_unsigned8* dest_data;
            dest_storage.getStorageInfo(&dest_pixel_stride,
                                        &dest_scanline_stride,
                                        NULL, NULL,
                                        (void**)&dest_data);

            //
            //  Create a list of rectangles.  The resulting list
            //  of rectangles is the area left by intersecting the ROI with
            //  the destination box.
            //
            XilRectList    rl(roi, dest_box);

            //
            // Get the absolute image co-ordinates of the box
            //
            int abs_x1;
            int abs_y1;
            int abs_x2;
            int abs_y2;
            src1_box->getAsCorners(&abs_x1, &abs_y1, &abs_x2, &abs_y2);

            //
            //  But the coordinant point might not be in this box, so loop
            //  over each coordinant point, see if it's in this box.
            //
            for(unsigned int idx = 0; idx < coordinate_count; idx++) {
                int cx = coordinate_list[idx*2];
                int cy = coordinate_list[idx*2+1];
                int kernel_corner_x = cx - key_x - abs_x1;
                int kernel_corner_y = cy - key_y - abs_y1;
		
                //
                // Now create a rectlist to loop over using the previous
                // rectlist and the kernel corners moved into box space
                //
                XilRectList paint_rl(&rl,
                                     kernel_corner_x,
                                     kernel_corner_y,
                                     kernel_corner_x + kernel_width - 1,
                                     kernel_corner_y + kernel_height - 1);

                int            x;
                int            y;
                unsigned int   xsize;
                unsigned int   ysize;
                while(paint_rl.getNext(&x, &y, &xsize, &ysize)) {
                    Xil_unsigned8* src1_scanline = src1_data +
                        (y*src1_scanline_stride) + (x*src1_pixel_stride);

                    Xil_unsigned8* dest_scanline = dest_data +
                        (y*dest_scanline_stride) + (x*dest_pixel_stride);

                    const float*   brush_row = brush_data +
                        (y - kernel_corner_y) * kernel_width +
                        (x - kernel_corner_x);

                    const float*   cache_row = cache_brush + 3 * (
                        ((y - kernel_corner_y) * kernel_width +
                         (x - kernel_corner_x)));

                    //
                    //  Each Scanline...
                    //
                    do {
                        Xil_unsigned8* src1_pixel  = src1_scanline;
                        Xil_unsigned8* dest_pixel  = dest_scanline;
                        const float*   brush_pixel = brush_row;
                        const float*   cache_pixel = cache_row;
                        unsigned int   pixel_count = xsize;

                        //
                        //  Each Pixel...
                        //
                        do {
                            //
                            //  dst = ((1.0F - brush) * src) + (brush + color)
                            //
                            float brush_inv = (1.0F - *brush_pixel);

                            *dest_pixel     = \
                                (unsigned char) (*cache_pixel +
                                    (brush_inv * _XILI_B2F(*src1_pixel)));

                            *(dest_pixel+1) = \
                                (unsigned char) (*(cache_pixel+1) +
                                    (brush_inv * _XILI_B2F(*(src1_pixel+1))));

                            *(dest_pixel+2) = \
                                (unsigned char) (*(cache_pixel+2) +
                                    (brush_inv * _XILI_B2F(*(src1_pixel+2))));

                            src1_pixel += src1_pixel_stride;
                            dest_pixel += dest_pixel_stride;
                            brush_pixel++;
                            cache_pixel+= 3;
                        } while(--pixel_count);

                        src1_scanline += src1_scanline_stride;
                        dest_scanline += dest_scanline_stride;
                        brush_row     += kernel_width;
                        cache_row     += (kernel_width*3);
                    } while(--ysize);
                }
            }
        }
    }

    return XIL_SUCCESS;
}
