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
//  File:       XiliOrderedDitherLut.hh
//  Project:    XIL
//  Revision:   1.9
//  Last Mod:   10:22:26, 03/10/00
//
//  Description:
//
//    Class definition for table-based Ordered Dither for BYTE sources.
//    Special cases are provided for 3 band - 32 bit aligned
//    and <= 4 band, Dither Size <= 16x16
//
//
//------------------------------------------------------------------------
//    COPYRIGHT
//------------------------------------------------------------------------
#pragma ident   "@(#)XiliOrderedDitherLut.hh	1.9\t00/03/10  "

#ifndef _XILI_ORDERED_DITHER_LUT_HH
#define _XILI_ORDERED_DITHER_LUT_HH

#include <xil/xilGPI.hh>

//
// Limit dither lookup table size
// based on 4 bands, 16x16 dithermask
//
#define _XIL_OD_MAX_LUT_SIZE (4*16*16*256)

enum ODCase {
    OD_3_BAND_ALIGNED,    // 3 band, dmask width mult of 4, dst 32 bit aligned
    OD_3_BAND,            // 3 band, any table dmask, any alignment
    OD_N_BAND,            // N band, any table dmask, any alignment
    OD_GENERAL            // Anything else
};

class XiliOrderedDitherLut {
public:

   
    //
    // Standard constructor
    //
    XiliOrderedDitherLut(XilLookupColorcube* cmap,
                         XilDitherMask*      dmask);

    //
    // Constructor for 411 YUV data
    //
    XiliOrderedDitherLut(XilLookupColorcube* cmap,
                         XilDitherMask*      dmask,
                         float*              scale_arg,
                         float*              off_arg,
                         int                 tbl_offset);

    //
    // Constructor for CellB YUV data
    //
    XiliOrderedDitherLut(XilLookupColorcube*     cmap,
                         XilDitherMask*          dmask,
                         float*                  scale_arg,
                         float*                  off_arg);

    ~XiliOrderedDitherLut();

    Xil_boolean isOK();

    void dither8(Xil_unsigned8* src_pixel,
                 unsigned int   src_pixel_stride,
                 unsigned int   src_scanline_stride,
                 Xil_unsigned8* dst_pixel,
                 unsigned int   dst_pixel_stride,
                 unsigned int   dst_scanline_stride,
                 unsigned int   width,
                 unsigned int   height,
                 unsigned int   xpos,
                 unsigned int   ypos);


    //
    //  Implemented for any storage type. Note that the
    //  x1 and y1 are absolute in the image (used for position
    //  of the kernel) and box_x1 and box_y1 are used
    //  to pick up the correct x,y in the storage.
    //
    void dither8General(XilStorage* src_storage,
                        Xil_unsigned8* dst_data,
                        unsigned int   dst_pixel_stride,
                        unsigned int   dst_scanline_stride,
                        unsigned int   xsize,
                        unsigned int   ysize,
                        int            x1,
                        int            y1,
                        int            box_x1,
                        int            box_y1);

    void dither411(Xil_unsigned8* ysrc,
                   Xil_unsigned8* usrc,
                   Xil_unsigned8* vsrc,
                   unsigned int   y_ss,
                   unsigned int   u_ss,
                   unsigned int   v_ss,
                   Xil_unsigned8* dst,
                   unsigned int   dst_ps,
                   unsigned int   dst_ss);

    void dither411(Xil_signed16*  ysrc,
                   Xil_signed16*  usrc,
                   Xil_signed16*  vsrc,
                   unsigned int   y_ss,
                   unsigned int   u_ss,
                   unsigned int   v_ss,
                   Xil_unsigned8* dst,
                   unsigned int   dst_ps,
                   unsigned int   dst_ss);


    void cvtMacroBlock(Xil_unsigned8* ysrc,
                       Xil_unsigned8* usrc,
                       Xil_unsigned8* vsrc,
                       unsigned int   y_ss,
                       unsigned int   u_ss,
                       unsigned int   v_ss,
                       Xil_unsigned8* dst,
                       unsigned int   dst_ps,
                       unsigned int   dst_ss);

    void cvtMacroBlock(Xil_signed16*  ysrc,
                       Xil_signed16*  usrc,
                       Xil_signed16*  vsrc,
                       unsigned int   y_ss,
                       unsigned int   u_ss,
                       unsigned int   v_ss,
                       Xil_unsigned8* dst,
                       unsigned int   dst_ps,
                       unsigned int   dst_ss);

    void cvtCellB(Xil_unsigned8  y0,
                  Xil_unsigned8  y1,
                  Xil_unsigned8  u,
                  Xil_unsigned8  v,
                  Xil_unsigned16 mask,
                  Xil_unsigned8* dst,
                  unsigned int   dst_ss);

private:

    void Dither3BandAligned(Xil_unsigned8* src_pixel,
                            unsigned int   src_pixel_stride,
                            Xil_unsigned8* dst_pixel,
                            unsigned int   width,
                            unsigned int   xmod,
                            unsigned int   ymod);

    void Dither3Band(Xil_unsigned8* src_pixel,
                     unsigned int   src_pixel_stride,
                     Xil_unsigned8* dst_pixel,
                     unsigned int   dst_pixel_stride,
                     unsigned int   width,
                     unsigned int   xmod,
                     unsigned int   ymod);

    void DitherNBand(Xil_unsigned8* src_pixel,
                     unsigned int   src_pixel_stride,
                     Xil_unsigned8* dst_pixel,
                     unsigned int   dst_pixel_stride,
                     unsigned int   width,
                     unsigned int   xmod,
                     unsigned int   ymod);

    void DitherNonTable(Xil_unsigned8* src_pixel,
                        unsigned int   src_pixel_stride,
                        Xil_unsigned8* dst_pixel,
                        unsigned int   dst_pixel_stride,
                        unsigned int   width,
                        unsigned int   xmod,
                        unsigned int   ymod);

    Xil_unsigned8* getYDitherLut();
    unsigned int*  getUDitherLut();
    unsigned int*  getVDitherLut();

    unsigned int    nbands;
    Xil_unsigned8*  multi_table;
    int*            mults;
    unsigned int*   dims;
    int             cmap_offset;
    unsigned int    cmap_base32;
    unsigned int    dmat_w;
    unsigned int    dmat_h;
    unsigned int    dmat_sz;
    Xil_unsigned8*  dmat;
    Xil_unsigned8*  dlut;
    unsigned int    dlut_band_stride;
    unsigned int    dlut_row_stride;
    unsigned int    dlut_col_stride;
    ODCase          special_case;
    Xil_boolean     isOKFlag;

    Xil_unsigned8*  yDitherLut;
    unsigned int*   uvDitherLut;
};

#endif // _XILI_ORDERED_DITHER_LUT_HH
