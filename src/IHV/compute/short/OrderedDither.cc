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
//  File:	OrderedDither.cc
//  Project:	XIL
//  Revision:	1.4
//  Last Mod:	10:11:55, 03/10/00
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
#pragma ident	"@(#)OrderedDither.cc	1.4\t00/03/10  "

#include "XiliUtils.hh"
#include "XilDeviceManagerComputeSHORT.hh"

XilStatus
XilDeviceManagerComputeSHORT::OrderedDither1(XilOp*       op,
                                            unsigned,
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
    // Get params from the op
    //
    XilLookupColorcube* cmap;
    op->getParam(1, (XilObject**) &cmap);
    XilDitherMask* mask;
    op->getParam(2, (XilObject**) &mask);

    //
    // Get XilSystemState used to report errors
    //
    XilSystemState* err_state = dest->getSystemState();

    //
    // get multipliers & dimensions of colorcube
    //
    unsigned int        nbands           = cmap->getOutputNBands();
    const int*          mults            = cmap->getMultipliers();
    const unsigned int* dims             = cmap->getDimsMinus1();
    int                 ccube_adj_offset = cmap->getAdjustedOffset();

    //
    // get dither matrices out of Dither Mask. 
    //
    //
    //  TODO:  9/12/96 jlf   The dmat array should be allocated and filled in
    //                       a preprocessor, not here in the compute routine!
    //
    const float* mat_values = mask->getData();
    unsigned int dmat_w = mask->getWidth();
    unsigned int dmat_h = mask->getHeight();
    unsigned int dmat_sz = dmat_w * dmat_h;
    int* dmat = new int[dmat_sz * nbands];
    if (dmat==NULL) {
        XIL_ERROR(err_state, XIL_ERROR_RESOURCE, "di-1", TRUE);
        return XIL_FAILURE;
    }

    //
    // scale dither mask from 0.0-1.0 range to 0-65535 range.
    //
    int        dmat_entries = nbands * dmat_sz;
    for(int i=0; i<dmat_entries; i++) {
        dmat[i] = (int)(mat_values[i] * 65535.0);
    }

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
                delete [] dmat;
                return XIL_FAILURE;
            } else {
                continue;
            }
        }
        
        //
        // Get the image space coordinates of the box
        //
        int          box_x;
        int          box_y;
        unsigned int box_w;
        unsigned int box_h;
        src1_box->getAsRect(&box_x, &box_y, &box_w, &box_h);

        //
        // Test to see if our source storage is XIL_PIXEL_SEQUENTIAL.
        // If so, implement a loop optimized for pixel-sequential
        // storage.  The destination storage must be a 1 banded bit
        // image so it is equivalent to a pixel sequential image.
        //
        if((src1_storage.isType(XIL_PIXEL_SEQUENTIAL))) {
            unsigned int   src1_pixel_stride;
            unsigned int   src1_scanline_stride;
            Xil_signed16*  src1_data;
            src1_storage.getStorageInfo(&src1_pixel_stride,
                                        &src1_scanline_stride,
                                        NULL, NULL,
                                        (void**)&src1_data);
            
            unsigned int   dest_scanline_stride;
            unsigned int   dest_offset;
            Xil_unsigned8* dest_data;
            dest_storage.getStorageInfo((unsigned int*)NULL,
                                        &dest_scanline_stride,
                                        NULL,
                                        &dest_offset,
                                        (void**)&dest_data);

            //
            //  Create a list of rectangles.  The resulting list
            //  of rectangles is the area left by intersecting the ROI with
            //  the destination box.
            //
            XilRectList    rl(roi, dest_box);

            int             x1;
            int             y1;
            unsigned int    xsize;
            unsigned int    ysize;
            while(rl.getNext(&x1, &y1, &xsize, &ysize)) {
                Xil_signed16* src_scanline = src1_data +
                    y1 * src1_scanline_stride + x1 * src1_pixel_stride;
                Xil_unsigned8* dest_scanline = dest_data +
                    y1 * dest_scanline_stride;
                //
                // For each scanline in image
                //
                for (int y = y1; y < y1 + ysize; y++) {
                    //
                    // point to the first pixel of the scanline
                    //
                    Xil_signed16* src_pixel = src_scanline;
                    //
                    // For each pixel in scanline
                    //
                    for (int x = x1; x < x1 + xsize; x++) {

                        // point to the first band of the pixel
                        Xil_signed16* src_band = src_pixel;

                        // for each band element in pixel
                        int cmap_index = (int)ccube_adj_offset;
                        for (int k = 0; k < nbands; k++) {
                            int tmp0 = (int)(((*src_band) + 32768) * dims[k]);
                            int frac = (int)(tmp0 & 0xFFFF);
                            int dmat_index = (k * dmat_sz) +
                                (int)((((y + box_y) % dmat_h) * dmat_w)        +
                                      ((x + box_x) % dmat_w));
                            cmap_index += (((tmp0>>16) * mults[k]) + 
                                ((frac>dmat[dmat_index]) ? (mults[k]) : (0)));

                            // move to next data element
                            src_band++;
                        }
                        if (cmap_index) {
                            XIL_BMAP_SET(dest_scanline, dest_offset + x);
                        } else {
                            XIL_BMAP_CLR(dest_scanline, dest_offset + x);
                        }

                        // move to the next pixel
                        src_pixel += src1_pixel_stride;
                    }

                    // move to next scanline
                    src_scanline += src1_scanline_stride;
                    dest_scanline += dest_scanline_stride;
                }
            }
        } else {
            //
            // General Storage Implementation.
            //

            unsigned int   dest_scanline_stride;
            unsigned int   dest_offset;
            Xil_unsigned8* dest_data;
            dest_storage.getStorageInfo((unsigned int) 0,
                                        NULL,
                                        &dest_scanline_stride,
                                        &dest_offset,
                                        (void**)&dest_data);

            XilRectList  rl(roi, dest_box);

            int             x1;
            int             y1;
            unsigned int    xsize;
            unsigned int    ysize;
            while(rl.getNext(&x1, &y1, &xsize, &ysize)) {
                //
                // For each scanline in image
                //
                for (int y = y1; y < y1 + ysize; y++) {
                    //
                    // For each pixel in scanline
                    //
                    for (int x = x1; x < x1 + xsize; x++) {

                        // for each band element in pixel
                        int cmap_index = ccube_adj_offset;
                        for (unsigned int k = 0; k < nbands; k++) {
                            //
                            // Locate the band of the current pixel
                            //
                            unsigned int   src1_pixel_stride;
                            unsigned int   src1_scanline_stride;
                            Xil_signed16*  src1_data;
                            src1_storage.getStorageInfo(k,
                                                        &src1_pixel_stride,
                                                        &src1_scanline_stride,
                                                        NULL,
                                                        (void**)&src1_data);
                            Xil_signed16* src_band = src1_data +
                                y * src1_scanline_stride +
                                x * src1_pixel_stride;

                            int tmp0 = (int)(((*src_band) + 32768) * dims[k]);
                            int frac = (int)(tmp0 & 0xFFFF);
                            int dmat_index = (k * dmat_sz) +
                                (int)((((y + box_y) % dmat_h) * dmat_w) +
                                      ((x + box_x) % dmat_w));
                            cmap_index += (((tmp0>>16) * mults[k]) + 
                                ((frac>dmat[dmat_index]) ? (mults[k]) : (0)));
                        }
                        //
                        // Store result in destination pixel
                        //
                        Xil_unsigned8* dest_scanline = dest_data +
                            y * dest_scanline_stride;
                        if (cmap_index) {
                            XIL_BMAP_SET(dest_scanline, dest_offset + x);
                        } else {
                            XIL_BMAP_CLR(dest_scanline, dest_offset + x);
                        }
                    }
                }
            }
        }
    }
    delete [] dmat;

    return XIL_SUCCESS;
}

XilStatus
XilDeviceManagerComputeSHORT::OrderedDither8(XilOp*       op,
                                            unsigned int,
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
    // Get params from the op
    //
    XilLookupColorcube* cmap;
    op->getParam(1, (XilObject**) &cmap);
    XilDitherMask* mask;
    op->getParam(2, (XilObject**) &mask);

    //
    // Get XilSystemState used to report errors
    //
    XilSystemState* err_state = dest->getSystemState();

    //
    // get multipliers & dimensions of colorcube
    //
    unsigned int        nbands           = cmap->getOutputNBands();
    const int*          mults            = cmap->getMultipliers();
    const unsigned int* dims             = cmap->getDimsMinus1();
    int                 ccube_adj_offset = cmap->getAdjustedOffset();

    //
    // get dither matrices out of Dither Mask. 
    //
    //
    //  TODO:  9/12/96 jlf   The dmat array should be allocated and filled in
    //                       a preprocessor, not here in the compute routine!
    //
    const float* mat_values = mask->getData();
    unsigned int dmat_w = mask->getWidth();
    unsigned int dmat_h = mask->getHeight();
    unsigned int dmat_sz = dmat_w * dmat_h;
    int* dmat = new int[dmat_sz * nbands];
    if (dmat==NULL) {
        XIL_ERROR(err_state, XIL_ERROR_RESOURCE, "di-1", TRUE);
        return XIL_FAILURE;
    }

    //
    // scale dither mask from 0.0-1.0 range to 0-65535 range.
    //
    int        dmat_entries = nbands * dmat_sz;
    for(int i=0; i<dmat_entries; i++) {
        dmat[i] = (int)(mat_values[i] * 65535.0);
    }

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
                delete [] dmat;
                return XIL_FAILURE;
            } else {
                continue;
            }
        }
        
        //
        // Get the image space coordinates of the box
        //
        int          box_x;
        int          box_y;
        unsigned int box_w;
        unsigned int box_h;
        src1_box->getAsRect(&box_x, &box_y, &box_w, &box_h);

        //
        //  Test to see if all of our storage is of type XIL_PIXEL_SEQUENTIAL.
        //  If so, implement a loop optimized for pixel-sequential storage.
        //
        if((src1_storage.isType(XIL_PIXEL_SEQUENTIAL)) &&
           (dest_storage.isType(XIL_PIXEL_SEQUENTIAL))) {
            unsigned int   src1_pixel_stride;
            unsigned int   src1_scanline_stride;
            Xil_signed16*  src1_data;
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

            int             x1;
            int             y1;
            unsigned int    xsize;
            unsigned int    ysize;
            while(rl.getNext(&x1, &y1, &xsize, &ysize)) {
                Xil_signed16* src_scanline = src1_data +
                    y1 * src1_scanline_stride + x1 * src1_pixel_stride;
                Xil_unsigned8* dest_scanline = dest_data +
                    y1 * dest_scanline_stride + x1 * dest_pixel_stride;
                //
                // For each scanline in image
                //
                for (int y = y1; y < y1 + ysize; y++) {
                    //
                    // point to the first pixel of the scanline
                    //
                    Xil_signed16* src_pixel = src_scanline;
                    Xil_unsigned8* dest_pixel = dest_scanline;
                    //
                    // For each pixel in scanline
                    //
                    for (int x = x1; x < x1 + xsize; x++) {

                        // point to the first band of the pixel
                        Xil_signed16* src_band = src_pixel;

                        // for each band element in pixel
                        int         tmp0,
                                dmat_index,
                                cmap_index = (int)ccube_adj_offset;
                        int frac;
                        for (int k = 0; k < nbands; k++) {
                            dmat_index = (dmat_sz * k) +
                                (int)((((y + box_y) % dmat_h) * dmat_w) +
                                      ((x + box_x) % dmat_w));
                            tmp0 = (int)((*src_band) + 32768) * dims[k];
                            frac = (tmp0 & 0xFFFF);
                            cmap_index += (int)(((tmp0 >> 16) * mults[k]) + 
                                ((frac > dmat[dmat_index]) ? (mults[k]) : (0)));

                            // move to next data element
                            src_band++;
                        }
                        *dest_pixel = (Xil_unsigned8)cmap_index;

                        // move to the next pixel
                        src_pixel += src1_pixel_stride;
                        dest_pixel += dest_pixel_stride;
                    }

                    // move to next scanline
                    src_scanline += src1_scanline_stride;
                    dest_scanline += dest_scanline_stride;
                }
            }
        } else {
            //
            // General Storage Implementation.
            //

            unsigned int   dest_pixel_stride;
            unsigned int   dest_scanline_stride;
            Xil_unsigned8* dest_data;
            dest_storage.getStorageInfo((unsigned int) 0,
                                        &dest_pixel_stride,
                                        &dest_scanline_stride,
                                        NULL,
                                        (void**)&dest_data);

            XilRectList  rl(roi, dest_box);

            int             x1;
            int             y1;
            unsigned int    xsize;
            unsigned int    ysize;
            while(rl.getNext(&x1, &y1, &xsize, &ysize)) {
                //
                // For each scanline in image
                //
                for (int y = y1; y < y1 + ysize; y++) {
                    //
                    // For each pixel in scanline
                    //
                    for (int x = x1; x < x1 + xsize; x++) {

                        // for each band element in pixel
                        int         tmp0,
                                dmat_index,
                                cmap_index = (int)ccube_adj_offset;
                        int frac;
                        for (unsigned int k = 0; k < nbands; k++) {
                            //
                            // Locate the band of the current pixel
                            //
                            unsigned int   src1_pixel_stride;
                            unsigned int   src1_scanline_stride;
                            Xil_signed16*  src1_data;
                            src1_storage.getStorageInfo(k,
                                                        &src1_pixel_stride,
                                                        &src1_scanline_stride,
                                                        NULL,
                                                        (void**)&src1_data);
                            Xil_signed16* src_band = src1_data +
                                y * src1_scanline_stride +
                                x * src1_pixel_stride;

                            dmat_index = (dmat_sz * k) +
                                (int)((((y + box_y) % dmat_h) * dmat_w) +
                                      ((x + box_x) % dmat_w));
                            tmp0 = (int)((*src_band) + 32768) * dims[k];
                            frac = (tmp0 & 0xFFFF);
                            cmap_index += (int)(((tmp0 >> 16) * mults[k]) + 
                                ((frac > dmat[dmat_index]) ? (mults[k]) : (0)));
                        }
                        //
                        // Store result in destination pixel
                        //
                        Xil_unsigned8* dest_pixel = dest_data +
                            y * dest_scanline_stride +
                            x * dest_pixel_stride;
                        *dest_pixel = (Xil_unsigned8)cmap_index;
                    }
                }
            }
        }
    }
    delete [] dmat;

    return XIL_SUCCESS;
}

XilStatus
XilDeviceManagerComputeSHORT::OrderedDither16(XilOp*       op,
                                             unsigned int,
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
    // Get params from the op
    //
    XilLookupColorcube* cmap;
    op->getParam(1, (XilObject**) &cmap);
    XilDitherMask* mask;
    op->getParam(2, (XilObject**) &mask);

    //
    // Get XilSystemState used to report errors
    //
    XilSystemState* err_state = dest->getSystemState();

    //
    // get multipliers & dimensions of colorcube
    //
    unsigned int        nbands           = cmap->getOutputNBands();
    const int*          mults            = cmap->getMultipliers();
    const unsigned int* dims             = cmap->getDimsMinus1();
    int                 ccube_adj_offset = cmap->getAdjustedOffset();

    //
    // get dither matrices out of Dither Mask. 
    //
    //
    //  TODO:  9/12/96 jlf   The dmat array should be allocated and filled in
    //                       a preprocessor, not here in the compute routine!
    //
    const float* mat_values = mask->getData();
    unsigned int dmat_w = mask->getWidth();
    unsigned int dmat_h = mask->getHeight();
    unsigned int dmat_sz = dmat_w * dmat_h;
    int* dmat = new int[dmat_sz * nbands];
    if (dmat==NULL) {
        XIL_ERROR(err_state, XIL_ERROR_RESOURCE, "di-1", TRUE);
        return XIL_FAILURE;
    }

    //
    // scale dither mask from 0.0-1.0 range to 0-65535 range.
    //
    int        dmat_entries = nbands * dmat_sz;
    for(int i=0; i<dmat_entries; i++) {
        dmat[i] = (int)(mat_values[i] * 65535.0);
    }

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
                delete [] dmat;
                return XIL_FAILURE;
            } else {
                continue;
            }
        }
        
        //
        // Get the image space coordinates of the box
        //
        int          box_x;
        int          box_y;
        unsigned int box_w;
        unsigned int box_h;
        src1_box->getAsRect(&box_x, &box_y, &box_w, &box_h);

        //
        //  Test to see if all of our storage is of type XIL_PIXEL_SEQUENTIAL.
        //  If so, implement a loop optimized for pixel-sequential storage.
        //
        if((src1_storage.isType(XIL_PIXEL_SEQUENTIAL)) &&
           (dest_storage.isType(XIL_PIXEL_SEQUENTIAL))) {
            unsigned int   src1_pixel_stride;
            unsigned int   src1_scanline_stride;
            Xil_signed16*  src1_data;
            src1_storage.getStorageInfo(&src1_pixel_stride,
                                        &src1_scanline_stride,
                                        NULL, NULL,
                                        (void**)&src1_data);
            
            unsigned int   dest_pixel_stride;
            unsigned int   dest_scanline_stride;
            Xil_signed16*  dest_data;
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

            int             x1;
            int             y1;
            unsigned int    xsize;
            unsigned int    ysize;
            while(rl.getNext(&x1, &y1, &xsize, &ysize)) {
                Xil_signed16* src_scanline = src1_data +
                    y1 * src1_scanline_stride + x1 * src1_pixel_stride;
                Xil_signed16* dest_scanline = dest_data +
                    y1 * dest_scanline_stride + x1 * dest_pixel_stride;
                //
                // For each scanline in image
                //
                for (int y = y1; y < y1 + ysize; y++) {
                    //
                    // point to the first pixel of the scanline
                    //
                    Xil_signed16* src_pixel = src_scanline;
                    Xil_signed16* dest_pixel = dest_scanline;
                    //
                    // For each pixel in scanline
                    //
                    for (int x = x1; x < x1 + xsize; x++) {

                        // point to the first band of the pixel
                        Xil_signed16* src_band = src_pixel;

                        // for each band element in pixel
                        int         tmp0,
                                dmat_index,
                                cmap_index = (int)ccube_adj_offset;
                        int frac;
                        for (int k = 0; k < nbands; k++) {
                            tmp0 = (int)((*src_band) + 32768) * dims[k];
                            frac = (int)(tmp0 & 0xFFFF);
                            dmat_index = (dmat_sz * k) +
                                (int)((((y + box_y) % dmat_h) * dmat_w) +
                                      ((x + box_x) % dmat_w));
                            cmap_index += (int)(((tmp0 >> 16) * mults[k]) + 
                                ((frac>dmat[dmat_index]) ? (mults[k]) : (0)));

                            // move to next data element
                            src_band++;
                        }
                        *dest_pixel = (Xil_signed16)cmap_index;

                        // move to the next pixel
                        src_pixel += src1_pixel_stride;
                        dest_pixel += dest_pixel_stride;
                    }

                    // move to next scanline
                    src_scanline += src1_scanline_stride;
                    dest_scanline += dest_scanline_stride;
                }
            }
        } else {
            //
            // General Storage Implementation.
            //

            unsigned int   dest_pixel_stride;
            unsigned int   dest_scanline_stride;
            Xil_signed16*  dest_data;
            dest_storage.getStorageInfo((unsigned int) 0,
                                        &dest_pixel_stride,
                                        &dest_scanline_stride,
                                        NULL,
                                        (void**)&dest_data);

            XilRectList  rl(roi, dest_box);

            int             x1;
            int             y1;
            unsigned int    xsize;
            unsigned int    ysize;
            while(rl.getNext(&x1, &y1, &xsize, &ysize)) {
                //
                // For each scanline in image
                //
                for (int y = y1; y < y1 + ysize; y++) {
                    //
                    // For each pixel in scanline
                    //
                    for (int x = x1; x < x1 + xsize; x++) {

                        // for each band element in pixel
                        int         tmp0,
                                dmat_index,
                                cmap_index = (int)ccube_adj_offset;
                        int frac;
                        for (int k = 0; k < nbands; k++) {
                            //
                            // Locate the band of the current pixel
                            //
                            unsigned int   src1_pixel_stride;
                            unsigned int   src1_scanline_stride;
                            Xil_signed16*  src1_data;
                            src1_storage.getStorageInfo(k,
                                                        &src1_pixel_stride,
                                                        &src1_scanline_stride,
                                                        NULL,
                                                        (void**)&src1_data);
                            Xil_signed16* src_band = src1_data +
                                y * src1_scanline_stride +
                                x * src1_pixel_stride;

                            tmp0 = (int)((*src_band) + 32768) * dims[k];
                            frac = (int)(tmp0 & 0xFFFF);
                            dmat_index = (dmat_sz * k) +
                                (int)((((y + box_y) % dmat_h) * dmat_w) +
                                      ((x + box_x) % dmat_w));
                            cmap_index += (int)(((tmp0 >> 16) * mults[k]) + 
                                ((frac>dmat[dmat_index]) ? (mults[k]) : (0)));
                        }
                        //
                        // Store result in destination pixel
                        //
                        Xil_signed16* dest_pixel = dest_data +
                            y * dest_scanline_stride +
                            x * dest_pixel_stride;
                        *dest_pixel = (Xil_signed16)cmap_index;
                    }
                }
            }
        }
    }
    delete [] dmat;

    return XIL_SUCCESS;
}
