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
//  File:       xili_codec_utils.cc
//  Project:    XIL
//  Revision:   1.2
//  Last Mod:   10:16:24, 03/10/00
//
//  Description:
//
//    Utility functions for codecs
//
//
//------------------------------------------------------------------------
//    COPYRIGHT
//------------------------------------------------------------------------
#pragma ident   "@(#)xili_codec_utils.cc	1.2\t00/03/10  "

#include "XiliUtils.hh"
#include "xili_codec_utils.hh"

//
// Utility function to copy rectangles of the src image to
// a destination image that has an ROI.
//
void
xili_copy_rects(void*          src_buf, 
                unsigned int   src_nbands,
                unsigned int   src_ps,
                unsigned int   src_ss,
                unsigned int   src_bs,
                unsigned int   src_offset,
                XilStorage*    dst_storage,
                XilRoi*        roi,
                XilBox*        box)
{
    //
    // Get the parameters of the dst storage from
    // the storage object
    //
    unsigned int dst_ps;
    unsigned int dst_ss;
    unsigned int dst_bs;
    unsigned int dst_offset;
    void*        dst_buf;
    dst_storage->getStorageInfo(&dst_ps, &dst_ss, &dst_bs,
                                &dst_offset, &dst_buf);

    unsigned int dst_nbands   = dst_storage->getImage()->getNumBands();
    XilDataType  datatype = dst_storage->getImage()->getDataType();

    //
    // Create a rectlist using the box and the roi
    //
    XilRectList rl(roi, box);

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
            // Src storage was created zero aligned, so we only
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
