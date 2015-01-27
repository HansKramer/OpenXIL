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
//  File:	Paint.cc
//  Project:	XIL
//  Revision:	1.3
//  Last Mod:	10:09:29, 03/10/00
//
//  Description:
//	
//
//	
//  MT-level:  Safe
//	
//------------------------------------------------------------------------
//	COPYRIGHT
//------------------------------------------------------------------------
#pragma ident	"@(#)Paint.cc	1.3\t00/03/10  "

#include "XiliUtils.hh"
#include "XilDeviceManagerComputeBIT.hh"

XilStatus
XilDeviceManagerComputeBIT::Paint(XilOp*       op,
				  unsigned int   ,
				  XilRoi*      roi,
				  XilBoxList*  bl)
{
    //
    //  Split the list of XilBoxes to take tile boundaries into account.  This
    //  will work to ensure that no cobbling of the data is required because
    //  the boxes will not cross tile boundaries in the source images.
    //
    if(op->splitOnTileBoundaries(bl) == XIL_FAILURE)
        return XIL_FAILURE;

    //
    //  Get the images for our operation.
    //
    XilImage* src1 = op->getSrcImage(1);
    XilImage* dest = op->getDstImage(1);

    //
    //  Get parameters for painting
    //
    float* color;
    op->getParam(1, (void **) &color);

    XilKernel* brush;
    op->getParam(2, (void**)&brush);
    //
    // Get kernel information
    //
    int key_x = brush->getKeyX();
    int key_y = brush->getKeyY();
    unsigned int kernel_width = brush->getWidth();
    unsigned int kernel_height = brush->getHeight();
    const float* bdata = brush->getData();

    unsigned int coordinate_count;
    op->getParam(3, &coordinate_count);
    
    Xil_signed32* coordinate_list;
    op->getParam(4, (void**)&coordinate_list);
    
    //
    //  The is a counter which keeps track of how many boxes we have
    //  processed.
    //
    unsigned int boxcount = 0;
    
    //
    //  Store away the number of bands for this operation.
    //    
    unsigned int nbands   = dest->getNumBands();

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
            //  Mark this box entry as having failed.  If marking the box
            //  returns XIL_FAILURE, then we return XIL_FAILURE.
            //
            if(bl->markAsFailed() == XIL_FAILURE) {
                return XIL_FAILURE;
            } else {
                continue;
            }
        }
        
	//
	// General Storage Implementation.
	//
	XilRectList  rl(roi, dest_box);
	
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
	for (int idx = 0; idx < coordinate_count; idx++) {
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
		//
		//  Each Band...
		//
		for(unsigned int band=0; band<nbands; band++) {
		    unsigned int   src1_pixel_stride;
		    unsigned int   src1_scanline_stride;
		    unsigned int   src1_storage_offset;
		    Xil_unsigned8* src1_data;
		    src1_storage.getStorageInfo(band,
						&src1_pixel_stride,
						&src1_scanline_stride,
						&src1_storage_offset,
						(void**)&src1_data);
		    
		    unsigned int   dest_pixel_stride;
		    unsigned int   dest_scanline_stride;
		    unsigned int   dest_storage_offset;
		    Xil_unsigned8* dest_data;
		    dest_storage.getStorageInfo(band,
						&dest_pixel_stride,
						&dest_scanline_stride,
						&dest_storage_offset,
						(void**)&dest_data);
		    
		    Xil_unsigned8* src1_scanline = src1_data +
			(y*src1_scanline_stride) +
			((src1_storage_offset + x) / XIL_BIT_ALIGNMENT);
		    
		    Xil_unsigned8* dest_scanline = dest_data +
			(y*dest_scanline_stride) +
			((dest_storage_offset + x) / XIL_BIT_ALIGNMENT);
		    
		    const float* brush_row = bdata + (x - kernel_corner_x)+
			(y - kernel_corner_y) * kernel_width;
		    
		    unsigned int scanline_count = ysize;
		    
		    //
		    //  Each Scanline...
		    //
		    do {
			Xil_unsigned8* src1_pixel = src1_scanline;
			Xil_unsigned8* dest_pixel = dest_scanline;
			const float*   brush_pixel = brush_row;
			
			unsigned int pixel_count = xsize;
			
			unsigned int src1_offset = ((src1_storage_offset + x) % XIL_BIT_ALIGNMENT);
			unsigned int dest_offset = ((dest_storage_offset + x) % XIL_BIT_ALIGNMENT);
			
			//
			//  Each Pixel...
			//
			do {
			    //
			    // paint is a take on blend
			    //   ((1 - brush) * source) + (bursh + color)
			    //
			    float painted =((1.0 - *brush_pixel) *
     XIL_BMAP_TST(src1_pixel,src1_offset)) + (*brush_pixel * color[band]);
			    if (painted < 0.5)
				XIL_BMAP_CLR(dest_pixel, dest_offset);
			    else
				XIL_BMAP_SET(dest_pixel, dest_offset);
			    
			    src1_offset++;
			    dest_offset++;
			    *brush_pixel++;
			} while(--pixel_count);
			
			src1_scanline += src1_scanline_stride;
			dest_scanline += dest_scanline_stride;
			brush_row += kernel_width;
		    } while(--scanline_count);
		}
	    }
	}
	boxcount++;
    }

    return XIL_SUCCESS;
}

