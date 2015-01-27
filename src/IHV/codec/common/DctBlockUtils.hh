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

#ifndef _COPY_UTILS_HH_
#define _COPY_UTILS_HH_

#include <xil/xilGPI.hh>
#include "DecompressInfo.hh"
#include "Ycc2RgbConverter.hh"
#include "XiliOrderedDitherLut.hh"

class BlockMan {

public:

    BlockMan();

    ~BlockMan();


    //
    // Copy an 8x8 BYTE block to a one band raster
    // with no upsampling.
    //
    void copyBlockToRaster(Xil_unsigned8* src, 
                           Xil_unsigned8* dst,
                           unsigned int   dst_ss);

    //
    // Copy an 8x8 SHORT block to a one band raster
    // with no upsampling.
    //
    void copyBlockToRaster(Xil_signed16* src, 
                           Xil_signed16* dst,
                           unsigned int  dst_ss);

    //
    // Clamp an 8x8 block of SHORT values
    // into a one band destination raster.
    // Values stay as shorts.
    //
    void clampBlockToRaster(Xil_signed16* src_blk, 
                            Xil_signed16* dst_blk,
                            unsigned int  dst_ss);

    //
    // Clamp an 8x8 block of SHORT values
    // into a one band destination raster.
    // Values get downgraded to bytes
    //
    void clampBlockToRaster(Xil_signed16*  src_blk, 
                            Xil_unsigned8* dst_blk,
                            unsigned int   dst_ss);

    //
    // Routines to upsample a FRAME of 411 YCC data
    // SHORT src and BYTE src versions.
    //
    void
    upsample411FrameFromRaster(DecompressInfo* di,
                               Xil_signed16*  src_Y_dataptr,
                               unsigned int   src_Y_ss,
                               Xil_signed16*  src_Cb_dataptr,
                               unsigned int   src_Cb_ss,
                               Xil_signed16*  src_Cr_dataptr,
                               unsigned int   src_Cr_ss);

    void
    upsample411FrameFromRaster(DecompressInfo* di,
                               Xil_unsigned8* src_Y_dataptr,
                               unsigned int   src_Y_ss,
                               Xil_unsigned8* src_Cb_dataptr,
                               unsigned int   src_Cb_ss,
                               Xil_unsigned8* src_Cr_dataptr,
                               unsigned int   src_Cr_ss);


    //
    // Routine to take a "4:1:1" macroblock and copy it to the destination,
    // upsampling the chroma components by 2 in both axes.
    // SHORT src and BYTE src versions.
    //
    void upsample411BlockFromRaster(Xil_signed16*         src_Y_dataptr,
                                    Xil_signed16*         src_Cb_dataptr,
                                    Xil_signed16*         src_Cr_dataptr,
                                    unsigned int          src_Y_ss,
                                    unsigned int          src_Cb_ss,
                                    unsigned int          src_Cr_ss,
                                    Xil_unsigned8*        dst_dataptr,
                                    unsigned int          dst_ps,
                                    unsigned int          dst_ss);

    void upsample411BlockFromRaster(Xil_unsigned8*        src_Y_dataptr,
                                    Xil_unsigned8*        src_Cb_dataptr,
                                    Xil_unsigned8*        src_Cr_dataptr,
                                    unsigned int          src_Y_ss,
                                    unsigned int          src_Cb_ss,
                                    unsigned int          src_Cr_ss,
                                    Xil_unsigned8*        dst_dataptr,
                                    unsigned int          dst_ps,
                                    unsigned int          dst_ss);

    void upsample411Block8FromRaster(Xil_unsigned8*        src_Y_dataptr,
                                    Xil_unsigned8*        src_Cb_dataptr,
                                    Xil_unsigned8*        src_Cr_dataptr,
                                    unsigned int          src_Y_ss,
                                    unsigned int          src_Cb_ss,
                                    unsigned int          src_Cr_ss,
                                    Xil_unsigned8*        dst_dataptr,
                                    unsigned int          dst_ps,
                                    unsigned int          dst_ss);

    
    //
    // Wrapper functions to call any of the block conversion routines
    // depending on the settings of the "doXXX" flags.
    // Versions for HSORT and BYTE source data
    //
    void processBlock(Xil_signed16*  y,
                      Xil_signed16*  cb,
                      Xil_signed16*  cr,
                      unsigned int   y_ss,
                      unsigned int   cb_ss,
                      unsigned int   cr_ss,
                      Xil_unsigned8* dst,
                      unsigned int   dst_ps,
                      unsigned int   dst_ss);

    void processBlock(Xil_unsigned8* y,
                      Xil_unsigned8* cb,
                      Xil_unsigned8* cr,
                      unsigned int   y_ss,
                      unsigned int   cb_ss,
                      unsigned int   cr_ss,
                      Xil_unsigned8* dst,
                      unsigned int   dst_ps,
                      unsigned int   dst_ss);

    //
    // Utility routine to copy data from a upsampled temporary block
    // to the destination. Since edge blocks are a very small percentage
    // of the total data, this approach leads to considerable code
    // simplification. The upsampler can just always deal with 16x16
    // blocks and this routine will deal with the edges.
    // Note: This is only posssible because the source data always
    //       contains complete macroblocks.
    //
    void copyPartialBlock(Xil_unsigned8* src_block, 
                          Xil_unsigned8* dst_block,
                          unsigned int   nx,
                          unsigned int   ny,
                          unsigned int   dst_ps,
                          unsigned int   dst_ss);

    void copyBlockRasterToRaster(Xil_unsigned8* src,
                                 Xil_unsigned8* dst,
                                 unsigned int   width,
                                 unsigned int   height,
                                 unsigned int   src_ss,
                                 unsigned int   dst_ss);

    void copyBlockRasterToRaster(Xil_signed16* src,
                                 Xil_signed16* dst,
                                 unsigned int  width,
                                 unsigned int  height,
                                 unsigned int  src_ss,
                                 unsigned int  dst_ss);

    Xil_signed16*  blold[6];
    Xil_unsigned8* blnew[6];

private:
    //
    // Internal macroblock space - 64 bit aligned
    //
    double macroblock16[6 * 64 * 2 / 8];
    double macroblock8[6 * 64 / 8];

    //
    // Pointers to support objects to perform
    // color conversion and dithering
    //
    Ycc2RgbConverter*      colorCvt;
    XiliOrderedDitherLut*  ditherCvt;

    //
    // Booleans to indicate what post-processing functions
    // are to be performed on the blocks.
    //
    Xil_boolean doColorConvert;
    Xil_boolean doOrderedDither;

};

#endif // _COPY_UTILS_HH_

