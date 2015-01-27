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
//  File:	TablewarpHorizontalNearest.cc
//  Project:	XIL
//  Revision:	1.20
//  Last Mod:	10:10:36, 03/10/00
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
#pragma ident	"@(#)TablewarpHorizontalNearest.cc	1.20\t00/03/10  "

#include "XilDeviceManagerComputeBYTE.hh"
#include "XiliUtils.hh"
#include "xili_geom_utils.hh"

static XilStatus tablewarp_h_nearest(XilBoxList*   bl,
                                   TablewarpData td);
static XilStatus tablewarp_h_pixel_sequential_16(TablewarpData td);
static XilStatus tablewarp_h_general_storage_16(TablewarpData td);
static XilStatus tablewarp_h_pixel_sequential_f32(TablewarpData td);
static XilStatus tablewarp_h_general_storage_f32(TablewarpData td);

XilStatus
XilDeviceManagerComputeBYTE::TablewarpHorizontalNearest(XilOp*       op,
                                                        unsigned     ,
                                                        XilRoi*      roi,
                                                        XilBoxList*  bl)
{
    XilStatus status = XIL_SUCCESS;
    TablewarpData td;

    td.op = op;
    td.roi = roi;

    //
    //  Source ROI
    //
    op->getParam(1, (XilObject**)&td.src_roi);

    op->getParam(2, &td.src_x_origin);
    op->getParam(3, &td.src_y_origin);

    op->getParam(4, &td.dst_x_origin);
    op->getParam(5, &td.dst_y_origin);

    status = tablewarp_h_nearest(bl, td);

    return status;
}



static XilStatus
tablewarp_h_nearest(XilBoxList*   bl,
		  TablewarpData td)
{
    XilOp*    op   = td.op;
    XilImage* src  = op->getSrcImage(1);
    XilImage* warp = op->getSrcImage(2);
    XilImage* dst  = op->getDstImage(1);

    td.nbands = dst->getNumBands();
    td.state  = dst->getSystemState();

    XilBox* src_box;
    XilBox* warp_box;
    XilBox* dst_box;
    while(bl->getNext(&src_box, &warp_box,  &dst_box)) {
        XilStorage  src_storage(src);
        XilStorage  warp_storage(warp);        
        XilStorage  dst_storage(dst);
        if((src->getStorage(&src_storage, op, src_box, "XilMemory",
                            XIL_READ_ONLY)  == XIL_FAILURE) ||
           (warp->getStorage(&warp_storage, op, warp_box, "XilMemory",
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
        td.src_storage  = &src_storage;
        td.warp_storage = &warp_storage;        
        td.dst_storage  = &dst_storage;
        td.src_box      = src_box;
        td.warp_box     = warp_box;
        td.dst_box      = dst_box;

	//
        //  Test to see if all of our storage is of type XIL_PIXEL_SEQUENTIAL.
        //  If so, optimize for pixel-sequential storage.
        //
        if(src_storage.isType(XIL_PIXEL_SEQUENTIAL) &&
           dst_storage.isType(XIL_PIXEL_SEQUENTIAL) &&
           warp_storage.isType(XIL_PIXEL_SEQUENTIAL)) {
            if(warp->getDataType() == XIL_SHORT) {
                if(tablewarp_h_pixel_sequential_16(td) == XIL_FAILURE) {
                    //
                    //  Mark this box as failed and if that succeeds, continue
                    //  processing the next box.  Otherwise, return XIL_FAILURE
                    //  now. 
                    //
                    if(bl->markAsFailed() == XIL_FAILURE) {
                        return XIL_FAILURE;
                    }
                }
            } else {
                if(tablewarp_h_pixel_sequential_f32(td) == XIL_FAILURE) {
                    //
                    //  Mark this box as failed and if that succeeds, continue
                    //  processing the next box.  Otherwise, return XIL_FAILURE
                    //  now. 
                    //
                    if(bl->markAsFailed() == XIL_FAILURE) {
                        return XIL_FAILURE;
                    }
                }
            }
        } else {
            if(warp->getDataType() == XIL_SHORT) {
                if(tablewarp_h_general_storage_16(td) == XIL_FAILURE) {
                    //
                    //  Mark this box as failed and if that succeeds, continue
                    //  processing the next box.  Otherwise, return XIL_FAILURE
                    //  now. 
                    //
                    if(bl->markAsFailed() == XIL_FAILURE) {
                        return XIL_FAILURE;
                    }
                }
            } else {
                if(tablewarp_h_general_storage_f32(td) == XIL_FAILURE) {
                    //
                    //  Mark this box as failed and if that succeeds, continue
                    //  processing the next box.  Otherwise, return XIL_FAILURE
                    //  now. 
                    //
                    if(bl->markAsFailed() == XIL_FAILURE) {
                        return XIL_FAILURE;
                    }
                }
            }
        }
    }

    return XIL_SUCCESS;
}

static
XilStatus
tablewarp_h_pixel_sequential_16(TablewarpData td)
{
    XilStorage*  src_storage   = td.src_storage;
    XilStorage*  warp_storage  = td.warp_storage;
    XilStorage*  dst_storage   = td.dst_storage;
    XilBox*      src_box       = td.src_box;
    XilBox*      dst_box       = td.dst_box;
    unsigned int nbands        = td.nbands;
    
    Xil_signed16*  warp_data;
    unsigned int   warp_pixel_stride;
    unsigned int   warp_scanline_stride;
    warp_storage->getStorageInfo(&warp_pixel_stride,
                                 &warp_scanline_stride,
                                 NULL, NULL,
                                 (void**)&warp_data);

    //
    //  We need the image coordinates of the area we're processing in order
    //  to clip in the source appropriately as well as for developing the
    //  warp conversion table.
    //
    int            src_box_x;        
    int            src_box_y;
    unsigned int   src_box_xsize;
    unsigned int   src_box_ysize;
    src_box->getAsRect(&src_box_x, &src_box_y,
                       &src_box_xsize, &src_box_ysize);

    int            dst_box_x;        
    int            dst_box_y;
    unsigned int   dst_box_xsize;
    unsigned int   dst_box_ysize;
    dst_box->getAsRect(&dst_box_x, &dst_box_y,
                       &dst_box_xsize, &dst_box_ysize);

    Xil_unsigned8* src_data;
    unsigned int   src_scanline_stride;
    unsigned int   src_pixel_stride;
    src_storage->getStorageInfo(&src_pixel_stride, 
                                &src_scanline_stride,
                                NULL,
                                NULL,
                                (void**)&src_data);

    Xil_unsigned8* dst_data;
    unsigned int   dst_scanline_stride;
    unsigned int   dst_pixel_stride;
    dst_storage->getStorageInfo(&dst_pixel_stride,
                                &dst_scanline_stride,
                                NULL,
                                NULL,
                                (void**)&dst_data);

    //
    //  These are the offsets that move the global space coordinates derived
    //  from the displacement of the warp table and puts them into box space
    //  by account for the origin and where the source box lives within the
    //  source. 
    //
    int src_to_box_x = _XILI_ROUND(td.src_x_origin) - src_box_x;
    int src_to_box_y = _XILI_ROUND(td.src_y_origin) - src_box_y;

    //
    //  Build the pixel and line tables for quickly calculating our location
    //  in the source image.
    //
    int* sxtable = xili_build_opt_table(src_box_xsize, src_pixel_stride);
    int* sytable = xili_build_opt_table(src_box_ysize, src_scanline_stride);
    if(sxtable == NULL || sytable == NULL) {
        delete [] sxtable;
        delete [] sytable;
        XIL_ERROR(td.state, XIL_ERROR_RESOURCE, "di-1", TRUE);
        return XIL_FAILURE;
    }

    //
    //  Convert the source ROI into a rect list we can loop over to see if the
    //  point we backward mapped to is inside of the ROI.
    //
    XilRectList src_rl(td.src_roi, NULL);

    //
    //  The ROI in the destination we loop over.  Coordinates are in box
    //  space.
    //
    XilRectList rl(td.roi, dst_box);

    int             x;
    int             y;
    unsigned int    xsize;
    unsigned int    ysize;
    while(rl.getNext(&x, &y, &xsize, &ysize)) {
        //
        //  Calculate the starting data positions in the destination and
        //  warp table.
        //
        Xil_signed16*  warp_scanline =
            warp_data + y*warp_scanline_stride + x*warp_pixel_stride;

        Xil_unsigned8* dst_scanline  =
            dst_data  + y*dst_scanline_stride + x*dst_pixel_stride;

        unsigned int   dst_xstart =
            dst_box_x + x - _XILI_ROUND(td.dst_x_origin);
        unsigned int   dst_ystart =
            dst_box_y + y - _XILI_ROUND(td.dst_y_origin);
        unsigned int   dst_xend   = dst_xstart + xsize;
        unsigned int   dst_yend   = dst_ystart + ysize;

        //
        //  Scanlines in the destination.
        //
        for(unsigned j=dst_ystart; j<dst_yend; j++) {
            //
            //  Point to the first pixel of the scanline 
            //
            Xil_signed16*  warp_pixel = warp_scanline;
            Xil_unsigned8* dst_pixel  = dst_scanline;

            for(unsigned i=dst_xstart; i<dst_xend;  i++) {
                //
                //  Calculate the source coordinates by reading the
                //  displacement from the warp table and then moving the
                //  values into source box space.
                //
                int isx = i + xili_tablewarp_offset_int(*warp_pixel) +
                    src_to_box_x;
                int isy = j + src_to_box_y;

                //
                //  Increment to the next warp_pixel.
                //
                warp_pixel += warp_pixel_stride;

                //
                //  Verify we've not mapped to outside the src box.
                //
                if((isx < 0) ||
                   (isy < 0) ||
                   (isx >= src_box_xsize) ||
                   (isy >= src_box_ysize)) {
                    dst_pixel += dst_pixel_stride;
                    continue;
                }

                //
                //  If we have a src ROI, verify we land within the ROI.
                //
                if(td.src_roi != NULL) {
                    Xil_boolean  inside_ROI = FALSE;

                    int          sx;
                    int          sy;
                    unsigned int sxsize;
                    unsigned int sysize;
                    while(src_rl.getNext(&sx, &sy, &sxsize, &sysize)) {
                        if((isx > sx) && (isx < (sx + sxsize - 1)) &&
                           (isy > sy) && (isy < (sy + sysize - 1))) {
                            inside_ROI = TRUE;
                            break;
                        }
                    }

                    src_rl.reset();

                    if(! inside_ROI) {
                        dst_pixel += dst_pixel_stride;
                        continue;
                    }
                }

                //
                //  Actual computation of the dst pixel is relatively simple
                //  for NN interpolation.
                //
                Xil_unsigned8* src_band   = src_data + sytable[isy] + sxtable[isx];
                Xil_unsigned8* dst_band   = dst_pixel;
                unsigned int   band_count = nbands;

                //
                //  Each band
                //
                do {      
                    *dst_band++ = *src_band++;
                } while(--band_count);

                //
                //  Move to the next destination pixel
                //
                dst_pixel += dst_pixel_stride;
            }

            //
            // Move to the next scanline
            //
            warp_scanline += warp_scanline_stride;
            dst_scanline += dst_scanline_stride;
        }
    }

    delete [] sytable;
    delete [] sxtable;

    return XIL_SUCCESS;
}

static
XilStatus
tablewarp_h_pixel_sequential_f32(TablewarpData td)
{
    XilStorage*  src_storage   = td.src_storage;
    XilStorage*  warp_storage  = td.warp_storage;
    XilStorage*  dst_storage   = td.dst_storage;
    XilBox*      src_box       = td.src_box;
    XilBox*      dst_box       = td.dst_box;
    unsigned int nbands        = td.nbands;
    
    Xil_float32*   warp_data;
    unsigned int   warp_pixel_stride;
    unsigned int   warp_scanline_stride;
    warp_storage->getStorageInfo(&warp_pixel_stride,
                                 &warp_scanline_stride,
                                 NULL, NULL,
                                 (void**)&warp_data);

    //
    //  We need the image coordinates of the area we're processing in order
    //  to clip in the source appropriately as well as for developing the
    //  warp conversion table.
    //
    int            src_box_x;        
    int            src_box_y;
    unsigned int   src_box_xsize;
    unsigned int   src_box_ysize;
    src_box->getAsRect(&src_box_x, &src_box_y,
                       &src_box_xsize, &src_box_ysize);

    int            dst_box_x;        
    int            dst_box_y;
    unsigned int   dst_box_xsize;
    unsigned int   dst_box_ysize;
    dst_box->getAsRect(&dst_box_x, &dst_box_y,
                       &dst_box_xsize, &dst_box_ysize);

    Xil_unsigned8* src_data;
    unsigned int   src_scanline_stride;
    unsigned int   src_pixel_stride;
    src_storage->getStorageInfo(&src_pixel_stride, 
                                &src_scanline_stride,
                                NULL,
                                NULL,
                                (void**)&src_data);

    Xil_unsigned8* dst_data;
    unsigned int   dst_scanline_stride;
    unsigned int   dst_pixel_stride;
    dst_storage->getStorageInfo(&dst_pixel_stride,
                                &dst_scanline_stride,
                                NULL,
                                NULL,
                                (void**)&dst_data);

    //
    //  These are the offsets that move the global space coordinates derived
    //  from the displacement of the warp table and puts them into box space
    //  by account for the origin and where the source box lives within the
    //  source. 
    //
    int src_to_box_x = _XILI_ROUND(td.src_x_origin) - src_box_x;
    int src_to_box_y = _XILI_ROUND(td.src_y_origin) - src_box_y;

    //
    //  Build the pixel and line tables for quickly calculating our location
    //  in the source image.
    //
    int* sxtable = xili_build_opt_table(src_box_xsize, src_pixel_stride);
    int* sytable = xili_build_opt_table(src_box_ysize, src_scanline_stride);
    if(sxtable == NULL || sytable == NULL) {
        delete [] sxtable;
        delete [] sytable;
        XIL_ERROR(td.state, XIL_ERROR_RESOURCE, "di-1", TRUE);
        return XIL_FAILURE;
    }

    //
    //  Convert the source ROI into a rect list we can loop over to see if the
    //  point we backward mapped to is inside of the ROI.
    //
    XilRectList src_rl(td.src_roi, NULL);

    //
    //  The ROI in the destination we loop over.  Coordinates are in box
    //  space.
    //
    XilRectList rl(td.roi, dst_box);

    int             x;
    int             y;
    unsigned int    xsize;
    unsigned int    ysize;
    while(rl.getNext(&x, &y, &xsize, &ysize)) {
        //
        //  Calculate the starting data positions in the destination and
        //  warp table.
        //
        Xil_float32*   warp_scanline =
            warp_data + y*warp_scanline_stride + x*warp_pixel_stride;

        Xil_unsigned8* dst_scanline  =
            dst_data  + y*dst_scanline_stride + x*dst_pixel_stride;

        unsigned int   dst_xstart = dst_box_x + x -
            _XILI_ROUND(td.dst_x_origin);
        unsigned int   dst_ystart = dst_box_y + y -
            _XILI_ROUND(td.dst_y_origin);
        unsigned int   dst_xend   = dst_xstart + xsize;
        unsigned int   dst_yend   = dst_ystart + ysize;

        //
        //  Scanlines in the destination.
        //
        for(unsigned j=dst_ystart; j<dst_yend; j++) {
            //
            //  Point to the first pixel of the scanline 
            //
            Xil_float32*   warp_pixel = warp_scanline;
            Xil_unsigned8* dst_pixel  = dst_scanline;

            for(unsigned i=dst_xstart; i<dst_xend;  i++) {
                //
                //  Calculate the source coordinates by reading the
                //  displacement from the warp table and then moving the
                //  values into source box space.
                //
                int isx = i + xili_tablewarp_offset_int(*warp_pixel) +
                    src_to_box_x;
                int isy = j + src_to_box_y;

                //
                //  Increment to the next warp_pixel.
                //
                warp_pixel += warp_pixel_stride;

                //
                //  Verify we've not mapped to outside the src box.
                //
                if((isx < 0) ||
                   (isy < 0) ||
                   (isx >= src_box_xsize) ||
                   (isy >= src_box_ysize)) {
                    dst_pixel += dst_pixel_stride;
                    continue;
                }

                //
                //  If we have a src ROI, verify we land within the ROI.
                //
                if(td.src_roi != NULL) {
                    Xil_boolean  inside_ROI = FALSE;

                    int          sx;
                    int          sy;
                    unsigned int sxsize;
                    unsigned int sysize;
                    while(src_rl.getNext(&sx, &sy, &sxsize, &sysize)) {
                        if((isx > sx) && (isx < (sx + sxsize - 1)) &&
                           (isy > sy) && (isy < (sy + sysize - 1))) {
                            inside_ROI = TRUE;
                            break;
                        }
                    }

                    src_rl.reset();

                    if(! inside_ROI) {
                        dst_pixel += dst_pixel_stride;
                        continue;
                    }
                }

                //
                //  Actual computation of the dst pixel is relatively simple
                //  for NN interpolation.
                //
                Xil_unsigned8* src_band   =
                    src_data + sytable[isy] + sxtable[isx];
                Xil_unsigned8* dst_band   = dst_pixel;
                unsigned int   band_count = nbands;

                do {      
                    *dst_band++ = *src_band++;
                } while(--band_count);

                //
                //  Move to the next destination pixel
                //
                dst_pixel += dst_pixel_stride;
            }

            //
            // Move to the next scanline
            //
            warp_scanline += warp_scanline_stride;
            dst_scanline += dst_scanline_stride;
        }
    }

    delete [] sytable;
    delete [] sxtable;

    return XIL_SUCCESS;
}

static
XilStatus
tablewarp_h_general_storage_16(TablewarpData td)
{
    XilStorage*  src_storage   = td.src_storage;
    XilStorage*  warp_storage  = td.warp_storage;
    XilStorage*  dst_storage   = td.dst_storage;
    XilBox*      src_box       = td.src_box;
    XilBox*      dst_box       = td.dst_box;
    unsigned int nbands        = td.nbands;
    
    //
    //  We need the image coordinates of the area we're processing in order
    //  to clip in the source appropriately as well as for developing the
    //  warp conversion table.
    //
    int            src_box_x;        
    int            src_box_y;
    unsigned int   src_box_xsize;
    unsigned int   src_box_ysize;
    src_box->getAsRect(&src_box_x, &src_box_y,
                       &src_box_xsize, &src_box_ysize);

    int            dst_box_x;        
    int            dst_box_y;
    unsigned int   dst_box_xsize;
    unsigned int   dst_box_ysize;
    dst_box->getAsRect(&dst_box_x, &dst_box_y,
                       &dst_box_xsize, &dst_box_ysize);

    //
    //  Get the warp table pointers.
    //
    Xil_signed16*  warp_data_b0;
    unsigned int   warp_pixel_stride_b0;
    unsigned int   warp_scanline_stride_b0;
    warp_storage->getStorageInfo((unsigned int)0,
                                 &warp_pixel_stride_b0,
                                 &warp_scanline_stride_b0,
                                 NULL,
                                 (void**)&warp_data_b0);

    //
    //  These are the offsets that move the global space coordinates derived
    //  from the displacement of the warp table and puts them into box space
    //  by account for the origin and where the source box lives within the
    //  source. 
    //
    int src_to_box_x = _XILI_ROUND(td.src_x_origin) - src_box_x;
    int src_to_box_y = _XILI_ROUND(td.src_y_origin) - src_box_y;

    //
    //  Convert the source ROI into a rect list we can loop over to see if the
    //  point we backward mapped to is inside of the ROI.
    //
    XilRectList src_rl(td.src_roi, NULL);

    //
    //  Loop over bands.
    //
    for(unsigned int b=0; b<nbands; b++) {
        Xil_unsigned8* src_data;
        unsigned int   src_scanline_stride;
        unsigned int   src_pixel_stride;
        src_storage->getStorageInfo(b,
                                    &src_pixel_stride, 
                                    &src_scanline_stride,
                                    NULL,
                                    (void**)&src_data);

        Xil_unsigned8* dst_data;
        unsigned int   dst_scanline_stride;
        unsigned int   dst_pixel_stride;
        dst_storage->getStorageInfo(b,
                                    &dst_pixel_stride,
                                    &dst_scanline_stride,
                                    NULL,
                                    (void**)&dst_data);

        //
        //  Build the pixel and line tables for quickly calculating our
        //  location in the source image.
        //
        int* sxtable = xili_build_opt_table(src_box_xsize,
                                            src_pixel_stride);
        int* sytable = xili_build_opt_table(src_box_ysize,
                                            src_scanline_stride);
        if(sxtable == NULL || sytable == NULL) {
            delete [] sxtable;
            delete [] sytable;
            XIL_ERROR(td.state, XIL_ERROR_RESOURCE, "di-1", TRUE);
            return XIL_FAILURE;
        }

        //
        //  The ROI in the destination we loop over.  Coordinates are in box
        //  space.
        //
        XilRectList rl(td.roi, dst_box);

        int             x;
        int             y;
        unsigned int    xsize;
        unsigned int    ysize;
        while(rl.getNext(&x, &y, &xsize, &ysize)) {
            //
            //  Calculate the starting data positions in the destination and
            //  warp table.
            //
            Xil_signed16*  warp_scanline_b0 = warp_data_b0 +
                y*warp_scanline_stride_b0 + x*warp_pixel_stride_b0;

            Xil_unsigned8* dst_scanline  =
                dst_data  + y*dst_scanline_stride + x*dst_pixel_stride;

            unsigned int   dst_xstart =
                dst_box_x + x - _XILI_ROUND(td.dst_x_origin);
            unsigned int   dst_ystart =
                dst_box_y + y - _XILI_ROUND(td.dst_y_origin);
            unsigned int   dst_xend   = dst_xstart + xsize;
            unsigned int   dst_yend   = dst_ystart + ysize;

            //
            //  Scanlines in the destination.
            //
            for(unsigned j=dst_ystart; j<dst_yend; j++) {
                //
                //  Point to the first pixel of the scanline 
                //
                Xil_signed16*  warp_pixel_b0 = warp_scanline_b0;
                Xil_unsigned8* dst_pixel     = dst_scanline;

                for(unsigned i=dst_xstart; i<dst_xend;  i++) {
                    //
                    //  Calculate the source coordinates by reading the
                    //  displacement from the warp table and then moving the 
                    //  values into source box space.
                    //
                    int isx = 
                        i + xili_tablewarp_offset_int(*warp_pixel_b0) +
                        src_to_box_x;
                    int isy = j + src_to_box_y;

                    //
                    //  Increment to the next warp_pixel.
                    //
                    warp_pixel_b0 += warp_pixel_stride_b0;

                    //
                    //  Verify we've not mapped to outside the src box.
                    //
                    if((isx < 0) ||
                       (isy < 0) ||
                       (isx >= src_box_xsize) ||
                       (isy >= src_box_ysize)) {
                        dst_pixel += dst_pixel_stride;
                        continue;
                    }

                    //
                    //  If we have a src ROI, verify we land within the ROI.
                    //
                    if(td.src_roi != NULL) {
                        Xil_boolean  inside_ROI = FALSE;

                        int          sx;
                        int          sy;
                        unsigned int sxsize;
                        unsigned int sysize;
                        while(src_rl.getNext(&sx, &sy, &sxsize, &sysize)) {
                            if((isx > sx) && (isx < (sx + sxsize - 1)) &&
                               (isy > sy) && (isy < (sy + sysize - 1))) {
                                inside_ROI = TRUE;
                                break;
                            }
                        }

                        src_rl.reset();

                        if(! inside_ROI) {
                            dst_pixel += dst_pixel_stride;
                            continue;
                        }
                    }

                    //
                    //  Actual computation of the dst pixel is relatively
                    //  simple for NN interpolation.
                    //
                    *dst_pixel = *(src_data + sytable[isy] + sxtable[isx]);

                    //
                    //  Move to the next destination pixel
                    //
                    dst_pixel += dst_pixel_stride;
                }

                //
                // Move to the next scanline
                //
                warp_scanline_b0 += warp_scanline_stride_b0;
                dst_scanline += dst_scanline_stride;
            }
        }

        delete [] sytable;
        delete [] sxtable;
    }

    return XIL_SUCCESS;
}

static
XilStatus
tablewarp_h_general_storage_f32(TablewarpData td)
{
    XilStorage*  src_storage   = td.src_storage;
    XilStorage*  warp_storage  = td.warp_storage;
    XilStorage*  dst_storage   = td.dst_storage;
    XilBox*      src_box       = td.src_box;
    XilBox*      dst_box       = td.dst_box;
    unsigned int nbands        = td.nbands;
    
    //
    //  We need the image coordinates of the area we're processing in order
    //  to clip in the source appropriately as well as for developing the
    //  warp conversion table.
    //
    int            src_box_x;        
    int            src_box_y;
    unsigned int   src_box_xsize;
    unsigned int   src_box_ysize;
    src_box->getAsRect(&src_box_x, &src_box_y,
                       &src_box_xsize, &src_box_ysize);

    int            dst_box_x;        
    int            dst_box_y;
    unsigned int   dst_box_xsize;
    unsigned int   dst_box_ysize;
    dst_box->getAsRect(&dst_box_x, &dst_box_y,
                       &dst_box_xsize, &dst_box_ysize);

    //
    //  Get the warp table pointers.
    //
    Xil_float32*   warp_data_b0;
    unsigned int   warp_pixel_stride_b0;
    unsigned int   warp_scanline_stride_b0;
    warp_storage->getStorageInfo((unsigned int)0,
                                 &warp_pixel_stride_b0,
                                 &warp_scanline_stride_b0,
                                 NULL,
                                 (void**)&warp_data_b0);

    //
    //  These are the offsets that move the global space coordinates derived
    //  from the displacement of the warp table and puts them into box space
    //  by account for the origin and where the source box lives within the
    //  source. 
    //
    int src_to_box_x = _XILI_ROUND(td.src_x_origin) - src_box_x;
    int src_to_box_y = _XILI_ROUND(td.src_y_origin) - src_box_y;

    //
    //  Convert the source ROI into a rect list we can loop over to see if the
    //  point we backward mapped to is inside of the ROI.
    //
    XilRectList src_rl(td.src_roi, NULL);

    //
    //  Loop over bands.
    //
    for(unsigned int b=0; b<nbands; b++) {
        Xil_unsigned8* src_data;
        unsigned int   src_scanline_stride;
        unsigned int   src_pixel_stride;
        src_storage->getStorageInfo(b,
                                    &src_pixel_stride, 
                                    &src_scanline_stride,
                                    NULL,
                                    (void**)&src_data);

        Xil_unsigned8* dst_data;
        unsigned int   dst_scanline_stride;
        unsigned int   dst_pixel_stride;
        dst_storage->getStorageInfo(b,
                                    &dst_pixel_stride,
                                    &dst_scanline_stride,
                                    NULL,
                                    (void**)&dst_data);

        //
        //  Build the pixel and line tables for quickly calculating our
        //  location in the source image.
        //
        int* sxtable = xili_build_opt_table(src_box_xsize,
                                            src_pixel_stride);
        int* sytable = xili_build_opt_table(src_box_ysize,
                                            src_scanline_stride);
        if(sxtable == NULL || sytable == NULL) {
            delete [] sxtable;
            delete [] sytable;
            XIL_ERROR(td.state, XIL_ERROR_RESOURCE, "di-1", TRUE);
            return XIL_FAILURE;
        }

        //
        //  The ROI in the destination we loop over.  Coordinates are in box
        //  space.
        //
        XilRectList rl(td.roi, dst_box);

        int             x;
        int             y;
        unsigned int    xsize;
        unsigned int    ysize;
        while(rl.getNext(&x, &y, &xsize, &ysize)) {
            //
            //  Calculate the starting data positions in the destination and
            //  warp table.
            //
            Xil_float32*   warp_scanline_b0 = warp_data_b0 +
                y*warp_scanline_stride_b0 + x*warp_pixel_stride_b0;

            Xil_unsigned8* dst_scanline  =
                dst_data  + y*dst_scanline_stride + x*dst_pixel_stride;

            unsigned int   dst_xstart =
                dst_box_x + x - _XILI_ROUND(td.dst_x_origin);
            unsigned int   dst_ystart =
                dst_box_y + y - _XILI_ROUND(td.dst_y_origin);
            unsigned int   dst_xend   = dst_xstart + xsize;
            unsigned int   dst_yend   = dst_ystart + ysize;

            //
            //  Scanlines in the destination.
            //
            for(unsigned j=dst_ystart; j<dst_yend; j++) {
                //
                //  Point to the first pixel of the scanline 
                //
                Xil_float32*   warp_pixel_b0 = warp_scanline_b0;
                Xil_unsigned8* dst_pixel     = dst_scanline;

                for(unsigned i=dst_xstart; i<dst_xend;  i++) {
                    //
                    //  Calculate the source coordinates by reading the
                    //  displacement from the warp table and then moving the 
                    //  values into source box space.
                    //
                    int isx = 
                        i + xili_tablewarp_offset_int(*warp_pixel_b0) +
                        src_to_box_x;
                    int isy = j + src_to_box_y;

                    //
                    //  Increment to the next warp_pixel.
                    //
                    warp_pixel_b0 += warp_pixel_stride_b0;

                    //
                    //  Verify we've not mapped to outside the src box.
                    //
                    if((isx < 0) ||
                       (isy < 0) ||
                       (isx >= src_box_xsize) ||
                       (isy >= src_box_ysize)) {
                        dst_pixel += dst_pixel_stride;
                        continue;
                    }

                    //
                    //  If we have a src ROI, verify we land within the ROI.
                    //
                    if(td.src_roi != NULL) {
                        Xil_boolean  inside_ROI = FALSE;

                        int          sx;
                        int          sy;
                        unsigned int sxsize;
                        unsigned int sysize;
                        while(src_rl.getNext(&sx, &sy, &sxsize, &sysize)) {
                            if((isx > sx) && (isx < (sx + sxsize - 1)) &&
                               (isy > sy) && (isy < (sy + sysize - 1))) {
                                inside_ROI = TRUE;
                                break;
                            }
                        }

                        src_rl.reset();

                        if(! inside_ROI) {
                            dst_pixel += dst_pixel_stride;
                            continue;
                        }
                    }

                    //
                    //  Actual computation of the dst pixel is relatively
                    //  simple for NN interpolation.
                    //
                    *dst_pixel = *(src_data + sytable[isy] + sxtable[isx]);

                    //
                    //  Move to the next destination pixel
                    //
                    dst_pixel += dst_pixel_stride;
                }

                //
                // Move to the next scanline
                //
                warp_scanline_b0 += warp_scanline_stride_b0;
                dst_scanline += dst_scanline_stride;
            }
        }

        delete [] sytable;
        delete [] sxtable;
    }

    return XIL_SUCCESS;
}

