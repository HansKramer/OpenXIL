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
//  File:       OrderedDither.cc
//  Project:    XIL
//  Revision:   1.13
//  Last Mod:   10:10:38, 03/10/00
//
//  Description:
//
//    Implementation of Ordered Dither for BYTE sources.
//    Special cases are provided for 3 band - 32 bit aligned
//    and <= 4 band, Dither Size <= 16x16
//
//
//------------------------------------------------------------------------
//    COPYRIGHT
//------------------------------------------------------------------------
#pragma ident   "@(#)OrderedDither.cc	1.13\t00/03/10  "

#include "XiliUtils.hh"
#include "XiliOrderedDitherLut.hh"
#include "XilDeviceManagerComputeBYTE.hh"

XilStatus
XilDeviceManagerComputeBYTE::OrderedDither8Preprocess(
    XilOp*        op,
    unsigned      ,
    XilRoi*       ,
    void**        compute_data,
    unsigned int* )
{
    //
    // Get colorcube and dithermask parameters
    //
    XilLookupColorcube* cmap;
    op->getParam(1, (XilObject**) &cmap);

    XilSystemState* err_state = cmap->getSystemState();

    XilDitherMask* dmask;
    op->getParam(2, (XilObject**) &dmask);

    int first_unreferenced_slot = -1;
    int first_null_slot         = -1;

    //
    // Lock around the updating of the caching information
    //
    odcacheMutex.lock();

    //
    // Check all of the tables, looking for a match
    //
    for(int j=0; j<_XILI_NUM_ORDERED_DITHER_LUTS; j++) {
        //
        // Check object versions to see if this case ia a repeat
        //
        if(odcacheDitherLut[j] != NULL &&
           cmap->isSameAs(&odcacheCmapVersion[j]) &&
           dmask->isSameAs(&odcacheDmaskVersion[j])) {

            //
            // Found a match - return the index and bump ref count
            //
            *compute_data = (void*)j;
            odcacheRefCnts[j]++;

            odcacheMutex.unlock();

            return XIL_SUCCESS;
        }

        //
        // Record the first NULL (never allocated) and the first
        // unreferenced slot for possible later use if no table 
        // matches are found
        //
        if(first_null_slot == -1 && odcacheDitherLut[j] == NULL) {
            first_null_slot = j;
        }
        if(first_unreferenced_slot == -1 && odcacheRefCnts[j] == 0) {
            first_unreferenced_slot = j;
        }
    }

    //
    // No matching DitherLut found. If an unreferenced slot
    // was found, use it, else construct an uncached table.
    //
    int new_slot;
    if(first_null_slot >= 0) {
        new_slot = first_null_slot;
    } else if(first_unreferenced_slot >= 0) {
        new_slot = first_unreferenced_slot;
    }

    if(new_slot >= 0) {
        //
        // Get the object versions for use in future comparisons
        //
        cmap->getVersion(&odcacheCmapVersion[new_slot]);
        dmask->getVersion(&odcacheDmaskVersion[new_slot]);

        delete odcacheDitherLut[new_slot];
    } 

    //
    // Construct the new dither table
    //
    XiliOrderedDitherLut* lut = new XiliOrderedDitherLut(cmap, dmask);
    if(! lut->isOK()) {
        XIL_ERROR(err_state, XIL_ERROR_RESOURCE, "di-1", TRUE);
        odcacheMutex.unlock();
        return XIL_FAILURE;
    }

    if(new_slot >= 0) {
        //
        // Return the cached table index (0...3)
        //
        odcacheDitherLut[new_slot] = lut;
        odcacheRefCnts[new_slot]++;
        *compute_data = (void*)new_slot;
    } else {
        //
        // Return the ptr to the XiliOrderedDitherLut object
        // TODO: Is this too much of a hack? A valid ptr
        //       should never have a value between 0 and 3,
        //       so we should be able to distinguish the
        //       two cases. (This is all to avoid a NEW op).
        // 
        *compute_data = (void*)lut;
    }

    odcacheMutex.unlock();
    return XIL_SUCCESS;
}

XilStatus       
XilDeviceManagerComputeBYTE::OrderedDither8Postprocess(
    XilOp*       ,
    void*        compute_data)
{

    odcacheMutex.lock();

    //
    // See if the index is in the 0 .. 3 range.
    // If so, decrement the reference count.
    // Otherwise its a single use table, so delete it.
    //
    unsigned int data = (unsigned int)compute_data;
    if((int)data >= 0 && data < _XILI_NUM_ORDERED_DITHER_LUTS) {
        odcacheRefCnts[data]--;
    } else {
        delete (XiliOrderedDitherLut*)compute_data;
    }

    odcacheMutex.unlock();

    return XIL_SUCCESS;
}

XilStatus
XilDeviceManagerComputeBYTE::OrderedDither1(XilOp*       op,
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
    XilImage* src = op->getSrcImage(1);
    XilImage* dst = op->getDstImage(1);

    //
    // Get params from the op
    //
    XilLookupColorcube* cmap;
    op->getParam(1, (XilObject**) &cmap);
    XilDitherMask* dmask;
    op->getParam(2, (XilObject**) &dmask);

    //
    // Get XilSystemState used to report errors
    //
    XilSystemState* err_state = dst->getSystemState();

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
    const float* mat_values = dmask->getData();
    unsigned int dmat_w = dmask->getWidth();
    unsigned int dmat_h = dmask->getHeight();
    unsigned int dmat_sz = dmat_w * dmat_h;
    Xil_unsigned8* dmat = new Xil_unsigned8[dmat_sz * nbands];
    if(dmat==NULL) {
        XIL_ERROR(err_state, XIL_ERROR_RESOURCE, "di-1", TRUE);
        return XIL_FAILURE;
    }

    //
    // scale dither mask from 0.0-1.0 range to 0-255 range.
    //
    int        dmat_entries = nbands * dmat_sz;
    for(int i=0; i<dmat_entries; i++) {
        dmat[i] = (Xil_unsigned8)(mat_values[i] * 255.0);
    }

    //
    //  Loop over each of the boxes of storage we are to process.
    //
    XilBox* src_box;
    XilBox* dst_box;
    while(bl->getNext(&src_box, &dst_box)) {
        //
        //  Aquire our storage from the images.  The storage returned is valid
        //  for the box given.  Thus, any origins or child offsets have been
        //  taken into account.
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
        src_box->getAsRect(&box_x, &box_y, &box_w, &box_h);

        //
        // Test to see if our source storage is XIL_PIXEL_SEQUENTIAL.
        // If so, implement a loop optimized for pixel-sequential
        // storage.  The destination storage must be a 1 banded bit
        // image so it is equivalent to a pixel sequential image.
        //
        if((src_storage.isType(XIL_PIXEL_SEQUENTIAL))) {
            unsigned int   src_pixel_stride;
            unsigned int   src_scanline_stride;
            Xil_unsigned8* src_data;
            src_storage.getStorageInfo(&src_pixel_stride,
                                        &src_scanline_stride,
                                        NULL, NULL,
                                        (void**)&src_data);
            
            unsigned int   dst_scanline_stride;
            unsigned int   dst_offset;
            Xil_unsigned8* dst_data;
            dst_storage.getStorageInfo((unsigned int*)NULL,
                                        &dst_scanline_stride,
                                        NULL,
                                        &dst_offset,
                                        (void**)&dst_data);

            //
            //  Create a list of rectangles.  The resulting list
            //  of rectangles is the area left by intersecting the ROI with
            //  the destination box.
            //
            XilRectList    rl(roi, dst_box);

            int             x1;
            int             y1;
            unsigned int    xsize;
            unsigned int    ysize;
            while(rl.getNext(&x1, &y1, &xsize, &ysize)) {
                Xil_unsigned8* src_scanline = src_data +
                    y1 * src_scanline_stride + x1 * src_pixel_stride;
                Xil_unsigned8* dst_scanline = dst_data +
                    y1 * dst_scanline_stride;
                //
                // For each scanline in image
                //
                for (unsigned int y = y1; y < y1 + ysize; y++) {
                    //
                    // point to the first pixel of the scanline
                    //
                    Xil_unsigned8* src_pixel = src_scanline;
                    //
                    // For each pixel in scanline
                    //
                    for (unsigned int x = x1; x < x1 + xsize; x++) {

                        // point to the first band of the pixel
                        Xil_unsigned8* src_band = src_pixel;

                        // for each band element in pixel
                        int cmap_index = ccube_adj_offset;
                        for (unsigned int k = 0; k < nbands; k++) {
                            int tmp0 = (int)(*src_band) * dims[k];
                            int dmat_index = (k * dmat_sz) +
                                (int)((((y + box_y) % (long)dmat_h) * dmat_w) +
                                      ((x + box_x) % (long)dmat_w));
                            cmap_index += (((tmp0>>8) * mults[k]) +
                                           (((tmp0 & 0xFF)>dmat[dmat_index])
                                            ? (mults[k]) : (0)));

                            // move to next data element
                            src_band++;
                        }
                        if (cmap_index) {
                            XIL_BMAP_SET(dst_scanline, dst_offset + x);
                        } else {
                            XIL_BMAP_CLR(dst_scanline, dst_offset + x);
                        }

                        // move to the next pixel
                        src_pixel += src_pixel_stride;
                    }

                    // move to next scanline
                    src_scanline += src_scanline_stride;
                    dst_scanline += dst_scanline_stride;
                }
            }
        } else {
            //
            // General Storage Implementation.
            //

            unsigned int   dst_scanline_stride;
            unsigned int   dst_offset;
            Xil_unsigned8* dst_data;
            dst_storage.getStorageInfo((unsigned int) 0,
                                        NULL,
                                        &dst_scanline_stride,
                                        &dst_offset,
                                        (void**)&dst_data);

            XilRectList  rl(roi, dst_box);

            int             x1;
            int             y1;
            unsigned int    xsize;
            unsigned int    ysize;
            while(rl.getNext(&x1, &y1, &xsize, &ysize)) {
                //
                // For each scanline in image
                //
                for (unsigned int y = y1; y < y1 + ysize; y++) {
                    //
                    // For each pixel in scanline
                    //
                    for (unsigned int x = x1; x < x1 + xsize; x++) {

                        // for each band element in pixel
                        int cmap_index = ccube_adj_offset;
                        for (unsigned int k = 0; k < nbands; k++) {
                            //
                            // Locate the band of the current pixel
                            //
                            unsigned int   src_pixel_stride;
                            unsigned int   src_scanline_stride;
                            Xil_unsigned8* src_data;
                            src_storage.getStorageInfo(k,
                                                        &src_pixel_stride,
                                                        &src_scanline_stride,
                                                        NULL,
                                                        (void**)&src_data);
                            Xil_unsigned8* src_band = src_data +
                                y * src_scanline_stride +
                                x * src_pixel_stride;

                            int tmp0 = (int)(*src_band) * dims[k];
                            int dmat_index = (k * dmat_sz) +
                                (int)((((y + box_y) % (long)dmat_h) * dmat_w) +
                                      ((x + box_x) % (long)dmat_w));
                            cmap_index += (((tmp0>>8) * mults[k]) +
                                           (((tmp0 & 0xFF)>dmat[dmat_index])
                                            ? (mults[k]) : (0)));
                        }
                        //
                        // Store result in destination pixel
                        //
                        Xil_unsigned8* dst_scanline = dst_data +
                            y * dst_scanline_stride;
                        if (cmap_index) {
                            XIL_BMAP_SET(dst_scanline, dst_offset + x);
                        } else {
                            XIL_BMAP_CLR(dst_scanline, dst_offset + x);
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
XilDeviceManagerComputeBYTE::OrderedDither8(XilOp*       op,
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

    XiliOrderedDitherLut* ditherLut;

    unsigned int data = (unsigned int)op->getPreprocessData(this);

    if((int)data >= 0 && data < _XILI_NUM_ORDERED_DITHER_LUTS) {
        ditherLut = odcacheDitherLut[data];
    } else {
        ditherLut = (XiliOrderedDitherLut*)data;
    }

    //
    //  Get the images for our operation.
    //
    XilImage* src = op->getSrcImage(1);
    XilImage* dst = op->getDstImage(1);

    unsigned int nbands = src->getNumBands();

    //
    // Get XilSystemState used to report errors
    //
    XilSystemState* err_state = dst->getSystemState();

    //
    //  Loop over each of the boxes of storage we are to process.
    //
    XilBox* src_box;
    XilBox* dst_box;
    while(bl->getNext(&src_box, &dst_box)) {
        //
        //  Aquire our storage from the images.  The storage returned is valid
        //  for the box given.  Thus, any origins or child offsets have been
        //  taken into account.
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
        
        //
        // Get the image space coordinates of the box
        //
        int          box_x;
        int          box_y;
        unsigned int box_w;
        unsigned int box_h;
        src_box->getAsRect(&box_x, &box_y, &box_w, &box_h);

        //
        //  Test to see if all of our storage is of type XIL_PIXEL_SEQUENTIAL.
        //  If so, implement a loop optimized for pixel-sequential storage.
        //
        if((src_storage.isType(XIL_PIXEL_SEQUENTIAL)) &&
           (dst_storage.isType(XIL_PIXEL_SEQUENTIAL))) {
            unsigned int   src_pixel_stride;
            unsigned int   src_scanline_stride;
            Xil_unsigned8* src_data;
            src_storage.getStorageInfo(&src_pixel_stride,
                                        &src_scanline_stride,
                                        NULL, NULL,
                                        (void**)&src_data);
            
            unsigned int   dst_pixel_stride;
            unsigned int   dst_scanline_stride;
            Xil_unsigned8* dst_data;
            dst_storage.getStorageInfo(&dst_pixel_stride,
                                        &dst_scanline_stride,
                                        NULL, NULL,
                                        (void**)&dst_data);

            //
            //  Create a list of rectangles.  The resulting list
            //  of rectangles is the area left by intersecting the ROI with
            //  the destination box.
            //
            XilRectList    rl(roi, dst_box);

            int             x1;
            int             y1;
            unsigned int    xsize;
            unsigned int    ysize;
            while(rl.getNext(&x1, &y1, &xsize, &ysize)) {
                Xil_unsigned8* src_scanline = src_data +
                    y1 * src_scanline_stride + x1 * src_pixel_stride;
                Xil_unsigned8* dst_scanline = dst_data +
                    y1 * dst_scanline_stride + x1 * dst_pixel_stride;
                
                ditherLut->dither8(src_scanline, src_pixel_stride,
                                   src_scanline_stride,
                                   dst_scanline, dst_pixel_stride,
                                   dst_scanline_stride,
                                   xsize, ysize, box_x+x1, box_y+y1);
            }
        } else {

            //
            // General Storage Implementation.
            //

            unsigned int   dst_pixel_stride;
            unsigned int   dst_scanline_stride;
            Xil_unsigned8* dst_data;
            dst_storage.getStorageInfo((unsigned int) 0,
                                        &dst_pixel_stride,
                                        &dst_scanline_stride,
                                        NULL,
                                        (void**)&dst_data);

            XilRectList  rl(roi, dst_box);

            int             x1;
            int             y1;
            unsigned int    xsize;
            unsigned int    ysize;
            while(rl.getNext(&x1, &y1, &xsize, &ysize)) {
                Xil_unsigned8* dst_ptr = dst_data +
                                         y1 * dst_scanline_stride +
                                         x1 * dst_pixel_stride;
                ditherLut->dither8General(&src_storage,
                                          dst_ptr, dst_pixel_stride,
                                          dst_scanline_stride,
                                          xsize, ysize, box_x+x1, box_y+y1,
                                          box_x, box_y);
            }
        }
    }

    return XIL_SUCCESS;
}

XilStatus
XilDeviceManagerComputeBYTE::OrderedDither16(XilOp*       op,
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
    XilImage* src = op->getSrcImage(1);
    XilImage* dst = op->getDstImage(1);

    //
    // Get params from the op
    //
    XilLookupColorcube* cmap;
    op->getParam(1, (XilObject**) &cmap);
    XilDitherMask* dmask;
    op->getParam(2, (XilObject**) &dmask);

    //
    // Get XilSystemState used to report errors
    //
    XilSystemState* err_state = dst->getSystemState();

    //
    // get multipliers & dimensions of colorcube
    //
    unsigned int        nbands           = cmap->getOutputNBands();
    const int*          mults            = cmap->getMultipliers();
    const unsigned int* dims             = cmap->getDimsMinus1();
    int                 ccube_adj_offset = cmap->getAdjustedOffset();

    //
    //  get dither matrices out of Dither Mask. 
    //
    //
    //  TODO:  9/12/96 jlf   The dmat array should be allocated and filled in
    //                       a preprocessor, not here in the compute routine!
    //
    const float* mat_values = dmask->getData();
    unsigned int dmat_w     = dmask->getWidth();
    unsigned int dmat_h     = dmask->getHeight();
    unsigned int dmat_sz    = dmat_w * dmat_h;
    Xil_unsigned8* dmat     = new Xil_unsigned8[dmat_sz * nbands];
    if(dmat==NULL) {
        XIL_ERROR(err_state, XIL_ERROR_RESOURCE, "di-1", TRUE);
        return XIL_FAILURE;
    }

    //
    // scale dither mask from 0.0-1.0 range to 0-255 range.
    //
    int        dmat_entries = nbands * dmat_sz;
    for(int i=0; i<dmat_entries; i++) {
        dmat[i] = (Xil_unsigned8)(mat_values[i] * 255.0);
    }

    //
    //  Loop over each of the boxes of storage we are to process.
    //
    XilBox* src_box;
    XilBox* dst_box;
    while(bl->getNext(&src_box, &dst_box)) {
        //
        //  Aquire our storage from the images.  The storage returned is valid
        //  for the box given.  Thus, any origins or child offsets have been
        //  taken into account.
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
        src_box->getAsRect(&box_x, &box_y, &box_w, &box_h);

        //
        //  Test to see if all of our storage is of type XIL_PIXEL_SEQUENTIAL.
        //  If so, implement a loop optimized for pixel-sequential storage.
        //
        if((src_storage.isType(XIL_PIXEL_SEQUENTIAL)) &&
           (dst_storage.isType(XIL_PIXEL_SEQUENTIAL))) {
            unsigned int   src_pixel_stride;
            unsigned int   src_scanline_stride;
            Xil_unsigned8* src_data;
            src_storage.getStorageInfo(&src_pixel_stride,
                                        &src_scanline_stride,
                                        NULL, NULL,
                                        (void**)&src_data);
            
            unsigned int   dst_pixel_stride;
            unsigned int   dst_scanline_stride;
            Xil_signed16*  dst_data;
            dst_storage.getStorageInfo(&dst_pixel_stride,
                                        &dst_scanline_stride,
                                        NULL, NULL,
                                        (void**)&dst_data);

            //
            //  Create a list of rectangles.  The resulting list
            //  of rectangles is the area left by intersecting the ROI with
            //  the destination box.
            //
            XilRectList    rl(roi, dst_box);

            int             x1;
            int             y1;
            unsigned int    xsize;
            unsigned int    ysize;
            while(rl.getNext(&x1, &y1, &xsize, &ysize)) {
                Xil_unsigned8* src_scanline = src_data +
                    y1 * src_scanline_stride + x1 * src_pixel_stride;
                Xil_signed16* dst_scanline = dst_data +
                    y1 * dst_scanline_stride + x1 * dst_pixel_stride;
                //
                // For each scanline in image
                //
                for (unsigned int y = y1; y < y1 + ysize; y++) {
                    //
                    // point to the first pixel of the scanline
                    //
                    Xil_unsigned8* src_pixel = src_scanline;
                    Xil_signed16*  dst_pixel = dst_scanline;
                    //
                    // For each pixel in scanline
                    //
                    for (unsigned int x = x1; x < x1 + xsize; x++) {

                        // point to the first band of the pixel
                        Xil_unsigned8* src_band = src_pixel;

                        // for each band element in pixel
                        int     tmp0,
                                dmat_index,
                                frac,
                                cmap_index = (int)ccube_adj_offset;
                        for (unsigned int k = 0; k < nbands; k++) {
                            tmp0 = (int)(*src_band) * dims[k];
                            frac = (int)(tmp0 & 0xFF);
                            dmat_index = (dmat_sz * k) +
                                (int)((((y + box_y) % dmat_h) * dmat_w) +
                                      ((x + box_x) % dmat_w));
                            cmap_index += (((tmp0 >> 8) * mults[k]) + 
                                ((frac>dmat[dmat_index]) ? (mults[k]) : (0)));

                            // move to next data element
                            src_band++;
                        }
                        *dst_pixel = (Xil_signed16)cmap_index;

                        // move to the next pixel
                        src_pixel += src_pixel_stride;
                        dst_pixel += dst_pixel_stride;
                    }

                    // move to next scanline
                    src_scanline += src_scanline_stride;
                    dst_scanline += dst_scanline_stride;
                }
            }
        } else {
            //
            // General Storage Implementation.
            //

            unsigned int   dst_pixel_stride;
            unsigned int   dst_scanline_stride;
            Xil_signed16*  dst_data;
            dst_storage.getStorageInfo((unsigned int) 0,
                                        &dst_pixel_stride,
                                        &dst_scanline_stride,
                                        NULL,
                                        (void**)&dst_data);

            XilRectList  rl(roi, dst_box);

            int                    x1;
            int                    y1;
            unsigned int    xsize;
            unsigned int    ysize;
            while(rl.getNext(&x1, &y1, &xsize, &ysize)) {
                //
                // For each scanline in image
                //
                for (unsigned int y = y1; y < y1 + ysize; y++) {
                    //
                    // For each pixel in scanline
                    //
                    for (unsigned int x = x1; x < x1 + xsize; x++) {

                        // for each band element in pixel
                        int     tmp0,
                                dmat_index,
                                frac,
                                cmap_index = (int)ccube_adj_offset;
                        for (unsigned int k = 0; k < nbands; k++) {
                            //
                            // Locate the band of the current pixel
                            //
                            unsigned int   src_pixel_stride;
                            unsigned int   src_scanline_stride;
                            Xil_unsigned8* src_data;
                            src_storage.getStorageInfo(k,
                                                        &src_pixel_stride,
                                                        &src_scanline_stride,
                                                        NULL,
                                                        (void**)&src_data);
                            Xil_unsigned8* src_band = src_data +
                                y * src_scanline_stride +
                                x * src_pixel_stride;

                            tmp0 = (int)(*src_band) * dims[k];
                            frac = (int)(tmp0 & 0xFF);
                            dmat_index = (dmat_sz * k) +
                                (int)((((y + box_y) % dmat_h) * dmat_w) +
                                      ((x + box_x) % dmat_w));
                            cmap_index += (((tmp0 >> 8) * mults[k]) + 
                                ((frac>dmat[dmat_index]) ? (mults[k]) : (0)));
                        }
                        //
                        // Store result in destination pixel
                        //
                        Xil_signed16* dst_pixel = dst_data +
                            y * dst_scanline_stride +
                            x * dst_pixel_stride;
                        *dst_pixel = (Xil_signed16)cmap_index;
                    }
                }
            }
        }
    }
    delete [] dmat;

    return XIL_SUCCESS;
}
