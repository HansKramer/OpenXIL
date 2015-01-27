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
//  File:       DecompressInfo.cc
//  Project:    XIL
//  Revision:   1.7
//  Last Mod:   10:16:23, 03/10/00
//
//  Description:
//
//    Utility object to gather information about this
//    frame decompression in a single container.
//
//
//------------------------------------------------------------------------
//    COPYRIGHT
//------------------------------------------------------------------------
#pragma ident   "@(#)DecompressInfo.cc	1.7\t00/03/10  "

#include "DecompressInfo.hh"
#include "XiliUtils.hh"

//
// Normal Constructor - Initialize all parameters
//
DecompressInfo::DecompressInfo(XilOp*       op,
                               unsigned int op_count,
                               XilRoi*      roi,
                               XilBoxList*  bl)
{
    isOKFlag = FALSE;

    image_roi = roi;
    image_bl  = bl;

    objectPtr1 = NULL;
    objectPtr2 = NULL;
    objectPtr3 = NULL;

    //
    // This isn't the constructor for temp storage, so
    // we won't need to delete a temporary buffer
    //
    mustDeleteTemporary = FALSE;

    //
    // This may be a molecule, so get the src and dst
    // from the first and last ops on the op list
    //
    XilOp** op_list = op->getOpList();

    // 
    // The image comes from the first op on the list (last in molecule)
    //
    image           = op_list[0]->getDstImage(1);

    //
    // The cis comes from the last op on the list (first in molecule)
    //
    cis             = op_list[op_count-1]->getSrcCis();

    //
    // Save the system state for error reporting
    //
    system_state    = cis->getSystemState();

    //
    // Get the output type of the CIS.
    //
    XilImageFormat* cis_output_type = cis->getDeviceCompression()->getOutputType();
    cis_output_type->getInfo(&cis_width, &cis_height, 
                            &cis_nbands, &cis_datatype);

    //
    // Get the frame number. Deferred execution may
    // result in out of order decompressions.
    //
    op_list[op_count-1]->getParam(1, &frame_number);

    image->getInfo(&image_width, &image_height, 
                   &image_nbands, &image_datatype);

    if(image_datatype == XIL_BIT) {
        image_storage_type = XIL_BAND_SEQUENTIAL;
    } else {
        image_storage_type = XIL_PIXEL_SEQUENTIAL;
    }

    //
    // Get the rectangle of the boxes of the cis and the image
    // The op creates only a single box for decompress operations.
    //
    bl->getNext(&cis_box, &image_box);
    cis_box->getAsRect(&cis_box_x, &cis_box_y, 
                       &cis_box_width, &cis_box_height);
    image_box->getAsRect(&image_box_x, &image_box_y, 
                         &image_box_width, &image_box_height);

    image_width  = cis_width;
    image_height = cis_height;

    //
    // Get the storage description for the dst image
    //
    // TODO: Extend this to tiled storage and XIL_GENERAL storage
    //
    image_storage = new XilStorage(image);
    if(image_storage == NULL) {
        XIL_ERROR(system_state, XIL_ERROR_RESOURCE, "di-1", TRUE);
        return;
    }

    if(image->getStorage(image_storage, op_list[0], image_box, "XilMemory",
                         XIL_WRITE_ONLY, image_storage_type) == XIL_FAILURE) {
        bl->markAsFailed();
        return;
    }

    image_storage->getStorageInfo(&image_ps, &image_ss, &image_bs, 
                                  &image_offset, &image_dataptr);
    image_box_offset  = image_offset;
    image_box_dataptr = image_dataptr;

    //
    // Default to not doing any of the molecule post-decompress functions
    //
    doColorConvert         = FALSE;
    doOrderedDither        = FALSE;
    doScale                = FALSE;

    isOKFlag = TRUE;
}

//
// Constructor for temporary storage buffer.
// Used for decompressing the whole frame to a temporary buffer,
// prior to using the copyRects function to deal with Dst ROIs.
// This constructor takes the original DecompressInfo object as its argument.
//
DecompressInfo::DecompressInfo(DecompressInfo* di)
{
    isOKFlag = FALSE;

    //
    // Copy the contents of the supplied object into this one
    //
    *this = *di;

    //
    // Set the image storage ptr to NULL.
    // We don't want the dst storage getting deleted after
    // this temporary object is deleted.
    //
    image_storage = NULL;

    //
    // We will create a temp buffer, so flag it to be deleted
    //
    mustDeleteTemporary = TRUE;

    image_width      = di->cis_width;
    image_height     = di->cis_height;
    image_box_x      = di->cis_box_x;
    image_box_y      = di->cis_box_y;
    image_box_width  = di->cis_box_width;
    image_box_height = di->cis_box_height;
    image_nbands     = di->cis_nbands;
    image_datatype   = di->cis_datatype;
    image_ss         = image_width * image_nbands;
    image_ps         = image_nbands;
    image_bs         = 1;
    image_offset     = 0;

    //
    // Here we calculate the data ptr to tmp box that will be
    // involved in the copy from the tmp image to the dst image.
    //
    // Make the scanline stride a multiple of 8 bytes
    // so we have a better chance of hitting some optimized
    // cases in xili_memcpy() and some of the decompressors
    //
    image_storage_type = XIL_PIXEL_SEQUENTIAL;
    unsigned int n64;
    switch(image_datatype) {
      case XIL_BIT:

        //
        // # of 64 bit words
        //
        n64 = (image_width + 63) / 64;
        image_ss = n64 * (8/sizeof(Xil_unsigned8));
        image_bs = image_ss * image_height;
        image_dataptr = new Xil_float64[n64 * image_height * image_nbands];
        image_data_quantity = image_ss * image_height * image_nbands;
        image_storage_type = XIL_BAND_SEQUENTIAL;

        //
        // XIL_BIT is special, so calculate the box start here
        //
        image_box_dataptr = (Xil_unsigned8*)image_dataptr +
                            image_box_y * image_ss +
                            image_box_x / XIL_BIT_ALIGNMENT;
        image_box_offset = image_box_x % XIL_BIT_ALIGNMENT;
        break;

      case XIL_BYTE:
        n64 = (image_width*image_nbands*sizeof(Xil_unsigned8) + 7) / 8;
        image_ss = n64 * (8/sizeof(Xil_unsigned8));
        image_bs = image_ss * image_height;
        image_dataptr = new Xil_float64[n64 * image_height];
        image_data_quantity = image_ss * image_height;

        image_box_dataptr = (Xil_unsigned8*)image_dataptr +
                            image_box_y * image_ss +
                            image_box_x * image_ps;
        break;

      case XIL_SHORT:
        n64 = (image_width*image_nbands*sizeof(Xil_signed16) + 7) / 8;
        image_ss = n64 * (8/sizeof(Xil_signed16));
        image_bs = image_ss * image_height;
        image_dataptr = new Xil_float64[n64 * image_height];
        image_data_quantity = image_ss * image_height;

        image_box_dataptr = (Xil_signed16*)image_dataptr +
                            image_box_y * image_ss +
                            image_box_x * image_ps;
        break;

      case XIL_FLOAT:
        n64 = (image_width*image_nbands*sizeof(Xil_float32) + 7) / 8;
        image_ss = n64 * (8/sizeof(Xil_float32));
        image_bs = image_ss * image_height;
        image_dataptr = new Xil_float64[n64 * image_height];
        image_data_quantity = image_ss * image_height;

        image_dataptr = new Xil_float32[image_data_quantity];
        image_box_dataptr = (Xil_float32*)image_dataptr +
                            image_box_y * image_ss +
                            image_box_x * image_ps;
        break;

      default:
        // Invalid destination data type for decompression
        XIL_ERROR(di->system_state, XIL_ERROR_USER, "di-123", TRUE);
    }

    if(image_dataptr == NULL) {
        XIL_ERROR(di->system_state, XIL_ERROR_RESOURCE, "di-1", TRUE);
        return;
    }

    isOKFlag = TRUE;
}

//
// Constructor where the caller provides the storage description.
// This is used whenwe want to have copyRects access an existing buffer.
// The specific cases are the Mpeg1 and H261 codecs which have an
// internal image buffer. This way we can avoid needless double copies.
//
DecompressInfo::DecompressInfo(DecompressInfo* di,
                               unsigned int width,
                               unsigned int height,
                               unsigned int nbands,
                               XilDataType  datatype,
                               unsigned int ps,
                               unsigned int ss,
                               void*        dataptr)
{
    isOKFlag = FALSE;

    //
    // Copy the contents of the supplied object into this one
    //
    *this = *di;

    //
    // Set the image storage ptr to NULL.
    // We don't want the dst storage getting deleted after
    // this temporary object is deleted.
    //
    image_storage = NULL;

    //
    // We don't create a temp buffer, so flag it not to be deleted
    //
    mustDeleteTemporary = FALSE;


    image_width      = width;
    image_height     = height;
    image_dataptr    = dataptr;
    image_box_x      = di->cis_box_x;
    image_box_y      = di->cis_box_y;
    image_box_width  = di->cis_box_width;
    image_box_height = di->cis_box_height;
    image_nbands     = nbands;
    image_datatype   = datatype;
    image_ss         = ss;
    image_ps         = ps;
    image_bs         = 1;
    image_offset     = 0;

    //
    // Here we calculate the data ptr to tmp box that will be
    // involved in the copy from the tmp image to the dst image.
    //
    // Make the scanline stride a multiple of 8 bytes
    // so we have a better chance of hitting some optimized
    // cases in xili_memcpy() and some of the decompressors
    //
    image_storage_type = XIL_PIXEL_SEQUENTIAL;
    switch(image_datatype) {
      case XIL_BIT:

        image_bs = image_ss * image_height;
        image_storage_type = XIL_BAND_SEQUENTIAL;

        //
        // XIL_BIT is special, so calculate the box start here
        //
        image_box_dataptr = (Xil_unsigned8*)image_dataptr +
                            image_box_y * image_ss +
                            image_box_x / XIL_BIT_ALIGNMENT;
        image_box_offset = image_box_x % XIL_BIT_ALIGNMENT;
        break;

      case XIL_BYTE:
        image_bs = image_ss * image_height;

        image_box_dataptr = (Xil_unsigned8*)image_dataptr +
                            image_box_y * image_ss +
                            image_box_x * image_ps;
        break;

      case XIL_SHORT:
        image_bs = image_ss * image_height;

        image_box_dataptr = (Xil_signed16*)image_dataptr +
                            image_box_y * image_ss +
                            image_box_x * image_ps;
        break;

      case XIL_FLOAT:
        image_bs = image_ss * image_height;
        image_box_dataptr = (Xil_float32*)image_dataptr +
                            image_box_y * image_ss +
                            image_box_x * image_ps;
        break;

      default:
        // Invalid destination data type for decompression
        XIL_ERROR(di->system_state, XIL_ERROR_USER, "di-123", TRUE);
    }

    isOKFlag = TRUE;
}

DecompressInfo::~DecompressInfo()
{
    if(mustDeleteTemporary) {
        delete []image_dataptr;
    } else {
        delete image_storage;
    }
}

Xil_boolean
DecompressInfo::isOK()
{
    if(this == NULL) {
        return FALSE;
    } else {
        if(isOKFlag == TRUE) {
            return TRUE;
        } else {
            delete this;
            return FALSE;
        }
    }
}

//
// Utility function to copy rectangles of the src image,
// as defined by the passed-in DecompressInfo object,
// to a destination image that has a ROI.
//
void
DecompressInfo::copyRects(DecompressInfo* tmp_di)
{
    //
    // Put dst  parameters in local variables
    //
    unsigned int dst_ps     = image_ps;
    unsigned int dst_ss     = image_ss;
    unsigned int dst_bs     = image_bs;
    unsigned int dst_nbands = image_nbands;
    unsigned int dst_offset = image_box_offset;
    void*        dst_buf    = image_box_dataptr;
    XilDataType  datatype   = image_datatype;

    //
    // Put src parameters in local variables
    //
    unsigned int src_ps     = tmp_di->image_ps;
    unsigned int src_ss     = tmp_di->image_ss;
    unsigned int src_bs     = tmp_di->image_bs;
    unsigned int src_nbands = tmp_di->image_nbands;
    unsigned int src_offset = tmp_di->image_box_offset;
    void*        src_buf    = tmp_di->image_box_dataptr;

    //
    // Create a rectlist using the box and the roi
    //
    XilRectList rl(this->image_roi, this->image_box);

    //
    // Copy each rectangle from the src tmp buffer to the dst image
    //
    int            rect_x, rect_y;
    unsigned int   rect_w, rect_h;

    switch (datatype) {
      case XIL_BIT:
        //
        // XIL_BIT images are always band sequential
        //
        while(rl.getNext(&rect_x, &rect_y, &rect_w, &rect_h)) {

            Xil_unsigned8* src_band = (Xil_unsigned8*)src_buf + 
                                      rect_y*src_ss + 
                                      (src_offset + rect_x) / XIL_BIT_ALIGNMENT;
            Xil_unsigned8* dst_band = (Xil_unsigned8*)dst_buf + 
                                      rect_y*dst_ss + 
                                      (dst_offset + rect_x) / XIL_BIT_ALIGNMENT;

            //
            // Temporary Src storage was created zero aligned, so we only
            // need to account for the rect starting coord.
            //
            unsigned int src_off = (src_offset + rect_x) % XIL_BIT_ALIGNMENT;

            //
            // Dst storage can be arbitrarily aligned, so need to
            // account for its offset plus that of the rect.
            //
            unsigned int dst_off = (dst_offset + rect_x) % XIL_BIT_ALIGNMENT;

            //
            // Copy each line in the rectangle
            //
            for(int band=0; band<dst_nbands; band++) {
                Xil_unsigned8* src_scan = src_band;
                Xil_unsigned8* dst_scan = dst_band;
                for(int line=0; line<rect_h; line++) {
                    xili_bit_memcpy(src_scan, dst_scan, rect_w, src_off, dst_off);
                    src_scan += src_ss;
                    dst_scan += dst_ss;
                }
                src_band += src_bs;
                dst_band += dst_bs;
            }

        }
        break;

      case XIL_BYTE:

        while(rl.getNext(&rect_x, &rect_y, &rect_w, &rect_h)) {

            Xil_unsigned8* src_scan = (Xil_unsigned8*)src_buf + 
                                      rect_y*src_ss + rect_x*src_ps;
            Xil_unsigned8* dst_scan = (Xil_unsigned8*)dst_buf + 
                                      rect_y*dst_ss + rect_x*dst_ps;

            //
            // Copy each line in the rectangle
            //
            if(src_ps     == dst_ps && 
               src_nbands == dst_nbands &&
               src_ps     == src_nbands &&
               dst_ps     == dst_nbands) {
                //
                // Optimized case.
                // A memcpy is possible if the following conditions are met:
                //     Pixel strides are identical.
                //     Number of bands are identical.
                //     All bands of the source are used.
                //     (Basically, no child band images involved).
                //
                for(int line=0; line<rect_h; line++) {
                    xili_memcpy(dst_scan, src_scan,
                                rect_w*dst_ps*sizeof(Xil_unsigned8));
                    src_scan += src_ss;
                    dst_scan += dst_ss;
                }
            } else {
                //
                // The generic loop
                //
                for(int line=0; line<rect_h; line++) {
                    Xil_unsigned8* src_pixel = src_scan;
                    Xil_unsigned8* dst_pixel = dst_scan;
                    for(int samp=0; samp<rect_w; samp++) {
                        Xil_unsigned8* src_band = src_pixel;
                        Xil_unsigned8* dst_band = dst_pixel;
                        for(int band=0; band<dst_nbands; band++) {
                            *dst_band++ = *src_band++;
                        }
                        src_pixel += src_ps;
                        dst_pixel += dst_ps;
                    }
                    src_scan += src_ss;
                    dst_scan += dst_ss;
                }
            }

        }
        break;

      case XIL_SHORT:

        while(rl.getNext(&rect_x, &rect_y, &rect_w, &rect_h)) {

            Xil_signed16* src_scan = (Xil_signed16*)src_buf + 
                                     rect_y*src_ss + rect_x*src_ps;
            Xil_signed16* dst_scan = (Xil_signed16*)dst_buf + 
                                     rect_y*dst_ss + rect_x*dst_ps;

            //
            // Copy each line in the rectangle
            //
            if(src_ps == dst_ps && src_nbands == dst_nbands) {
                //
                // Optimized loop where src/dst pixel strides are identical
                //
                for(int line=0; line<rect_h; line++) {
                    xili_memcpy(dst_scan, src_scan,
                                rect_w*dst_ps*sizeof(Xil_signed16));
                    src_scan += src_ss;
                    dst_scan += dst_ss;
                }
            } else {
                //
                // The generic loop
                //
                for(int line=0; line<rect_h; line++) {
                    Xil_signed16* src_pixel = src_scan;
                    Xil_signed16* dst_pixel = dst_scan;
                    for(int samp=0; samp<rect_w; samp++) {
                        Xil_signed16* src_band = src_pixel;
                        Xil_signed16* dst_band = dst_pixel;
                        for(int band=0; band<dst_nbands; band++) {
                            *dst_band++ = *src_band++;
                        }
                        src_pixel += src_ps;
                        dst_pixel += dst_ps;
                    }
                    src_scan += src_ss;
                    dst_scan += dst_ss;
                }
            }

        }
        break;

      default:
        return;
    }

    return;

}
