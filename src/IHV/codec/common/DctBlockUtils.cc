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
//  File:       DctBlockUtils.cc
//  Project:    XIL
//  Revision:   1.5
//  Last Mod:   10:16:26, 03/10/00
//
//  Description:
//
//    Miscellaneous functions to move blocks of data between 
//    rasters and block buffers. Useful for DCT-based codecs.
//    These are particularly well optimized for aligned storage.
//
//
//------------------------------------------------------------------------
//    COPYRIGHT
//------------------------------------------------------------------------
#pragma ident   "@(#)DctBlockUtils.cc	1.5\t00/03/10  "

#include <xil/xilGPI.hh>
#include "DctBlockUtils.hh"

//
// Constructor - Init flags and pointers
//
BlockMan::BlockMan()
{
    //
    // Init pointers to each block in the macroblock
    //
    Xil_signed16*  ptr16 = (Xil_signed16*)macroblock16;
    Xil_unsigned8* ptr8  = (Xil_unsigned8*)macroblock8;
    for(int i=0; i<6; i++) {
        blold[i] = ptr16;
        blnew[i] = ptr8;
        ptr16 += 64;
        ptr8  += 64;
    }

    //
    // Init all support object pointers and flags
    //
    colorCvt  = NULL;
    ditherCvt = NULL;

    doOrderedDither        = FALSE;
    doColorConvert         = FALSE;
}

//
// Destructor
//
BlockMan::~BlockMan()
{
}


//
// Copy an 8x8 BYTE DCT block to a one band raster
// with no upsampling.
//
void
BlockMan::copyBlockToRaster(Xil_unsigned8* src, 
                            Xil_unsigned8* dst,
                            unsigned int   dst_ss)
{
    if(((int)src & 0x3) == 0 && ((int)dst & 0x3) == 0) {
        //
        // 32 bit aligned case (most common)
        // Unroll the transfer of each word
        //
        int*         src32 = (int*)src;
        int*         dst32 = (int*)dst;
        unsigned int dst32_ss = dst_ss / 4;

        dst32[0] = src32[0];
        dst32[1] = src32[1];
        dst32 += dst32_ss;

        dst32[0] = src32[2];
        dst32[1] = src32[3];
        dst32 += dst32_ss;

        dst32[0] = src32[4];
        dst32[1] = src32[5];
        dst32 += dst32_ss;

        dst32[0] = src32[6];
        dst32[1] = src32[7];
        dst32 += dst32_ss;

        dst32[0] = src32[8];
        dst32[1] = src32[9];
        dst32 += dst32_ss;

        dst32[0] = src32[10];
        dst32[1] = src32[11];
        dst32 += dst32_ss;

        dst32[0] = src32[12];
        dst32[1] = src32[13];
        dst32 += dst32_ss;

        dst32[0] = src32[14];
        dst32[1] = src32[15];
        dst32 += dst32_ss;
    } else {
        //
        // Arbitrary alignment case
        // Move one byte at a time
        //
        for(int line=8; line !=0; line--) {
            dst[0] = src[0];
            dst[1] = src[1];
            dst[2] = src[2];
            dst[3] = src[3];
            dst[4] = src[4];
            dst[5] = src[5];
            dst[6] = src[6];
            dst[7] = src[7];
            src += 8;
            dst += dst_ss;
        }
    }

}

//
// Copy an 8x8 SHORT block to a one band raster
// with no upsampling.
//
void
BlockMan::copyBlockToRaster(Xil_signed16* src, 
                            Xil_signed16* dst,
                            unsigned int  dst_ss)
{
    if(((int)src & 0x3) == 0 &&
       ((int)dst & 0x3) == 0) {
        //
        // 32 bit aligned case (most common)
        //

        int*         src32 = (int*)src;
        int*         dst32 = (int*)dst;
        unsigned int dst32_ss = dst_ss >> 1;

        dst32[0] = src32[0];
        dst32[1] = src32[1];
        dst32[2] = src32[2];
        dst32[3] = src32[3];
        dst32 += dst32_ss;

        dst32[0] = src32[4];
        dst32[1] = src32[5];
        dst32[2] = src32[6];
        dst32[3] = src32[7];
        dst32 += dst32_ss;

        dst32[0] = src32[8];
        dst32[1] = src32[9];
        dst32[2] = src32[10];
        dst32[3] = src32[11];
        dst32 += dst32_ss;

        dst32[0] = src32[12];
        dst32[1] = src32[13];
        dst32[2] = src32[14];
        dst32[3] = src32[15];
        dst32 += dst32_ss;

        dst32[0] = src32[16];
        dst32[1] = src32[17];
        dst32[2] = src32[18];
        dst32[3] = src32[19];
        dst32 += dst32_ss;

        dst32[0] = src32[20];
        dst32[1] = src32[21];
        dst32[2] = src32[22];
        dst32[3] = src32[23];
        dst32 += dst32_ss;

        dst32[0] = src32[24];
        dst32[1] = src32[25];
        dst32[2] = src32[26];
        dst32[3] = src32[27];
        dst32 += dst32_ss;

        dst32[0] = src32[28];
        dst32[1] = src32[29];
        dst32[2] = src32[30];
        dst32[3] = src32[31];
        dst32 += dst32_ss;
    } else {
        //
        // Arbitrary alignment case
        // Move one short at a time
        //
        for(int line=8; line !=0; line--) {
            dst[0] = src[0];
            dst[1] = src[1];
            dst[2] = src[2];
            dst[3] = src[3];
            dst[4] = src[4];
            dst[5] = src[5];
            dst[6] = src[6];
            dst[7] = src[7];
            src += 8;
            dst += dst_ss;
        }
    }

}

//
// Clamp an 8x8 block of SHORT values
// into a one band destination raster.
// Values stay as shorts.
//
void
BlockMan::clampBlockToRaster(Xil_signed16* src_blk, 
                             Xil_signed16* dst_blk,
                             unsigned int  dst_ss)
{
    int* src      = (int*)src_blk;
    int* dst_line = (int*)dst_blk;

    unsigned int dst32_ss = dst_ss >> 1;

    for(int line=8; line!=0; line--) {
        int* dst = dst_line;

        //
        // Since we're doing two values per int, the loop count
        // is only 4 here for one line of the 8x8 block.
        //
        for(int samp=4; samp!=0; samp--) {
            int pair = *src++;

            //
            // Test if either element has underflowed or overflowed 8 bits, as
            // indicated by any bits being set in the upper half of the shorts
            // If not, we're done. This is the major speed gain here.
            //
            if((pair & 0xff00ff00) != 0) {
                //
                // Test the most significant word
                //
                if(pair & 0xff000000) {
                    if(pair & 0x80000000) {
                        // Clamp negative to zero
                        pair &= 0x0000ffff;
                    } else {
                        // Clamp overflow to 255
                        pair &= 0x0000ffff;
                        pair |= 0x00ff0000;
                    }
                }

                //
                // Test the least significant word.
                //
                if(pair & 0x0000ff00) {
                    if(pair & 0x00008000) {
                        // Clamp negative to zero
                        pair &= 0xffff0000;
                    } else {
                        // Clamp overflow to 255
                        pair &= 0xffff0000;
                        pair |= 0x000000ff;
                    }
                }

            }

            *dst++ = pair;
        }

        dst_line += dst32_ss;
    }

}

//
// Clamp an 8x8 block of SHORT values
// into a one band destination raster.
// Values get downgraded to bytes
//
void
BlockMan::clampBlockToRaster(Xil_signed16*  src_blk, 
                             Xil_unsigned8* dst_blk,
                             unsigned int   dst_ss)
{
    int* src      = (int*)src_blk;
    int* dst_line = (int*)dst_blk;

    unsigned int dst32_ss = dst_ss >> 2;

    for(int line=8; line!=0; line--) {
        short* dst = (short*)dst_line;

        //
        // Since we're doing two values per int, the loop count
        // is only 4 here for one line of the 8x8 block.
        //
        for(int samp=4; samp!=0; samp--) {
            int pair = *src++;

            //
            // Test if either element has underflowed or overflowed 8 bits, as
            // indicated by any bits being set in the upper half of the shorts
            // If not, we're done. This is the major speed gain here, since
            // most of the time clamping is not required.
            //
            if((pair & 0xff00ff00) != 0) {
                //
                // Test the most significant word
                //
                if(pair & 0xff000000) {
                    if(pair & 0x80000000) {
                        // Clamp negative to zero
                        pair &= 0x0000ffff;
                    } else {
                        // Clamp overflow to 255
                        pair &= 0x0000ffff;
                        pair |= 0x00ff0000;
                    }
                }

                //
                // Test the least significant word.
                //
                if(pair & 0x0000ff00) {
                    if(pair & 0x00008000) {
                        // Clamp negative to zero
                        pair &= 0xffff0000;
                    } else {
                        // Clamp overflow to 255
                        pair &= 0xffff0000;
                        pair |= 0x000000ff;
                    }
                }

            }

            *dst++ = (pair & 0xff) | (pair >> 8);
        }

        dst_line += dst32_ss;
    }

}

void
BlockMan::processBlock(Xil_unsigned8* y,
                       Xil_unsigned8* cb,
                       Xil_unsigned8* cr,
                       unsigned int   y_ss,
                       unsigned int   cb_ss,
                       unsigned int   cr_ss,
                       Xil_unsigned8* dst,
                       unsigned int   dst_ps,
                       unsigned int   dst_ss)
{
    if(doColorConvert) {
        colorCvt->cvtMacroBlock(y, cb, cr, y_ss, cb_ss, cr_ss,
                                dst, dst_ps, dst_ss);
    } else if(doOrderedDither) {
        ditherCvt->cvtMacroBlock(y, cb, cr, y_ss, cb_ss, cr_ss,
                             dst, dst_ps, dst_ss);
    } else {
        upsample411BlockFromRaster(y, cb, cr, y_ss, cb_ss, cr_ss,
                                   dst, dst_ps, dst_ss);
    }
}


void
BlockMan::processBlock(Xil_signed16*  y,
                       Xil_signed16*  cb,
                       Xil_signed16*  cr,
                       unsigned int   y_ss,
                       unsigned int   cb_ss,
                       unsigned int   cr_ss,
                       Xil_unsigned8* dst,
                       unsigned int   dst_ps,
                       unsigned int   dst_ss)
{
    if(doColorConvert) {
        colorCvt->cvtMacroBlock(y, cb, cr, y_ss, cb_ss, cr_ss,
                                dst, dst_ps, dst_ss);
    } else if(doOrderedDither) {
        ditherCvt->cvtMacroBlock(y, cb, cr, y_ss, cb_ss, cr_ss,
                             dst, dst_ps, dst_ss);
    } else {
        upsample411BlockFromRaster(y, cb, cr, y_ss, cb_ss, cr_ss,
                                   dst, dst_ps, dst_ss);
    }
}


void
BlockMan::upsample411FrameFromRaster(DecompressInfo* di,
                                     Xil_signed16*   src_Y_dataptr,
                                     unsigned int    src_Y_ss,
                                     Xil_signed16*   src_Cb_dataptr,
                                     unsigned int    src_Cb_ss,
                                     Xil_signed16*   src_Cr_dataptr,
                                     unsigned int    src_Cr_ss)
{
    Xil_signed16*  src_Y_block_row_ptr  = src_Y_dataptr;
    Xil_signed16*  src_Cb_block_row_ptr = src_Cb_dataptr;
    Xil_signed16*  src_Cr_block_row_ptr = src_Cr_dataptr;

    Xil_unsigned8* dst_block_row_ptr    = (Xil_unsigned8*)
                                          di->image_dataptr;
    unsigned int   dst_ps               = di->image_ps;
    unsigned int   dst_ss               = di->image_ss;

    unsigned int   dst_block_row_inc    = 16 * dst_ss;
    unsigned int   dst_block_col_inc    = 16 * dst_ps;

    //
    // Record the support object pointers and flags, if any
    //
    if(di->doColorConvert) {
        colorCvt  = (Ycc2RgbConverter*) (di->objectPtr1);
        doColorConvert = TRUE;
    } else if(di->doOrderedDither) {
        ditherCvt = (XiliOrderedDitherLut*) (di->objectPtr1);
        doOrderedDither = TRUE;
    } 

    //
    // Create a temporary block on the stack.
    // The upsampling of partial macroblocks will go into this block 
    // and then be copied to the dst image.
    //
    Xil_unsigned8 tmp_block[16*16*3];
    unsigned int  tmp_ss = 16 * 3;
    unsigned int  tmp_ps = 3;


    //
    // Do all complete macroblock rows (16 lines)
    //
    unsigned int width  = di->cis_width;
    unsigned int height = di->cis_height;
    for(int y=0; y<(height&(~15)); y+=16) {
        Xil_signed16*  src_Y_block_col_ptr = src_Y_block_row_ptr;
        Xil_signed16*  src_Cb_block_col_ptr = src_Cb_block_row_ptr;
        Xil_signed16*  src_Cr_block_col_ptr = src_Cr_block_row_ptr;
        Xil_unsigned8* dst_block_col_ptr = dst_block_row_ptr;
        for(int x=0; x<(width&(~15)); x+=16) {
            processBlock(src_Y_block_col_ptr,
                         src_Cb_block_col_ptr,
                         src_Cr_block_col_ptr,
                         src_Y_ss, src_Cb_ss, src_Cr_ss,
                         dst_block_col_ptr, dst_ps, dst_ss);

            src_Y_block_col_ptr  += 16;
            src_Cb_block_col_ptr += 8;
            src_Cr_block_col_ptr += 8;
            dst_block_col_ptr    += dst_block_col_inc;
        }

        //
        // Do any partial macroblock at a row end
        //
        if(x < width) {
            processBlock(src_Y_block_col_ptr,
                         src_Cb_block_col_ptr,
                         src_Cr_block_col_ptr,
                         src_Y_ss, src_Cb_ss, src_Cr_ss,
                         tmp_block, tmp_ps, tmp_ss);

            copyPartialBlock(tmp_block, dst_block_col_ptr, 
                             width%16, 16, dst_ps, dst_ss);
        }
        src_Y_block_row_ptr  += 16*src_Y_ss;
        src_Cb_block_row_ptr += 8*src_Cb_ss;
        src_Cr_block_row_ptr += 8*src_Cr_ss;
        dst_block_row_ptr    += dst_block_row_inc;
    }

    //
    // Do any partial macroblock row
    //
    if(y < height) {
        Xil_signed16*  src_Y_block_col_ptr = src_Y_block_row_ptr;
        Xil_signed16*  src_Cb_block_col_ptr = src_Cb_block_row_ptr;
        Xil_signed16*  src_Cr_block_col_ptr = src_Cr_block_row_ptr;
        Xil_unsigned8* dst_block_col_ptr = dst_block_row_ptr;
        for(int x=0; x<(width&(~15)); x+=16) {
            processBlock(src_Y_block_col_ptr,
                         src_Cb_block_col_ptr,
                         src_Cr_block_col_ptr,
                         src_Y_ss, src_Cb_ss, src_Cr_ss,
                         tmp_block, tmp_ps, tmp_ss);

            copyPartialBlock(tmp_block, dst_block_col_ptr, 
                             width%16, 16, dst_ps, dst_ss);

            src_Y_block_col_ptr  += 16;
            src_Cb_block_col_ptr += 8;
            src_Cr_block_col_ptr += 8;
            dst_block_col_ptr    += dst_block_col_inc;
        }

        //
        // Do any partial macroblock at the end of this row
        // (This will be the very last block at the lower-right).
        //
        if(x < width) {
            processBlock(src_Y_block_col_ptr,
                         src_Cb_block_col_ptr,
                         src_Cr_block_col_ptr,
                         src_Y_ss, src_Cb_ss, src_Cr_ss,
                         tmp_block, tmp_ps, tmp_ss);

            copyPartialBlock(tmp_block, dst_block_col_ptr, 
                             width%16, 16, dst_ps, dst_ss);
        }
    }

}


void
BlockMan::upsample411FrameFromRaster(DecompressInfo* di,
                                     Xil_unsigned8*   src_Y_dataptr,
                                     unsigned int    src_Y_ss,
                                     Xil_unsigned8*   src_Cb_dataptr,
                                     unsigned int    src_Cb_ss,
                                     Xil_unsigned8*   src_Cr_dataptr,
                                     unsigned int    src_Cr_ss)
{
    Xil_unsigned8*  src_Y_block_row_ptr  = src_Y_dataptr;
    Xil_unsigned8*  src_Cb_block_row_ptr = src_Cb_dataptr;
    Xil_unsigned8*  src_Cr_block_row_ptr = src_Cr_dataptr;

    Xil_unsigned8* dst_block_row_ptr    = (Xil_unsigned8*)
                                          di->image_dataptr;
    unsigned int   dst_ps               = di->image_ps;
    unsigned int   dst_ss               = di->image_ss;

    unsigned int   dst_block_row_inc    = 16 * dst_ss;
    unsigned int   dst_block_col_inc    = 16 * dst_ps;

    //
    // Record the support object pointers and flags, if any
    //
    if(di->doColorConvert) {
        colorCvt  = (Ycc2RgbConverter*) (di->objectPtr1);
        doColorConvert = TRUE;
    } else if(di->doOrderedDither) {
        ditherCvt = (XiliOrderedDitherLut*) (di->objectPtr1);
        doOrderedDither = TRUE;
    } 

    //
    // Create a temporary block on the stack.
    // The upsampling of partial macroblocks will go into this block 
    // and then be copied to the dst image.
    //
    Xil_unsigned8 tmp_block[16*16*3];
    unsigned int  tmp_ss = 16 * 3;
    unsigned int  tmp_ps = 3;

    //
    // Do all complete macroblock rows (16 lines)
    //
    unsigned int width  = di->cis_width;
    unsigned int height = di->cis_height;
    for(int y=0; y<(height&(~15)); y+=16) {
        Xil_unsigned8*  src_Y_block_col_ptr = src_Y_block_row_ptr;
        Xil_unsigned8*  src_Cb_block_col_ptr = src_Cb_block_row_ptr;
        Xil_unsigned8*  src_Cr_block_col_ptr = src_Cr_block_row_ptr;
        Xil_unsigned8* dst_block_col_ptr = dst_block_row_ptr;
        for(int x=0; x<(width&(~15)); x+=16) {
            processBlock(src_Y_block_col_ptr,
                         src_Cb_block_col_ptr,
                         src_Cr_block_col_ptr,
                         src_Y_ss, src_Cb_ss, src_Cr_ss,
                         dst_block_col_ptr, dst_ps, dst_ss);

            src_Y_block_col_ptr  += 16;
            src_Cb_block_col_ptr += 8;
            src_Cr_block_col_ptr += 8;
            dst_block_col_ptr    += dst_block_col_inc;
        }

        //
        // Do any partial macroblock at a row end
        //
        if(x < width) {
            processBlock(src_Y_block_col_ptr,
                         src_Cb_block_col_ptr,
                         src_Cr_block_col_ptr,
                         src_Y_ss, src_Cb_ss, src_Cr_ss,
                         tmp_block, tmp_ps, tmp_ss);

            copyPartialBlock(tmp_block, dst_block_col_ptr, 
                             width%16, 16, dst_ps, dst_ss);
        }
        src_Y_block_row_ptr  += 16*src_Y_ss;
        src_Cb_block_row_ptr += 8*src_Cb_ss;
        src_Cr_block_row_ptr += 8*src_Cr_ss;
        dst_block_row_ptr    += dst_block_row_inc;
    }

    //
    // Do any partial macroblock row
    //
    if(y < height) {
        Xil_unsigned8*  src_Y_block_col_ptr = src_Y_block_row_ptr;
        Xil_unsigned8*  src_Cb_block_col_ptr = src_Cb_block_row_ptr;
        Xil_unsigned8*  src_Cr_block_col_ptr = src_Cr_block_row_ptr;
        Xil_unsigned8* dst_block_col_ptr = dst_block_row_ptr;
        for(int x=0; x<(width&(~15)); x+=16) {
            processBlock(src_Y_block_col_ptr,
                         src_Cb_block_col_ptr,
                         src_Cr_block_col_ptr,
                         src_Y_ss, src_Cb_ss, src_Cr_ss,
                         tmp_block, tmp_ps, tmp_ss);

            copyPartialBlock(tmp_block, dst_block_col_ptr, 
                             width%16, 16, dst_ps, dst_ss);

            src_Y_block_col_ptr  += 16;
            src_Cb_block_col_ptr += 8;
            src_Cr_block_col_ptr += 8;
            dst_block_col_ptr    += dst_block_col_inc;
        }

        //
        // Do any partial macroblock at the end of this row
        // (This will be the very last block at the lower-right).
        //
        if(x < width) {
            processBlock(src_Y_block_col_ptr,
                         src_Cb_block_col_ptr,
                         src_Cr_block_col_ptr,
                         src_Y_ss, src_Cb_ss, src_Cr_ss,
                         tmp_block, tmp_ps, tmp_ss);

            copyPartialBlock(tmp_block, dst_block_col_ptr, 
                             width%16, 16, dst_ps, dst_ss);
        }
    }

}



//
// Routine to take a "4:1:1" macroblock and copy it to the destination,
// upsampling the chroma components by 2 in both axes.
// This version has a SHORT src and a BYTE dst.
//
void 
BlockMan::upsample411BlockFromRaster(Xil_signed16*  src_Y_dataptr,
                                     Xil_signed16*  src_Cb_dataptr,
                                     Xil_signed16*  src_Cr_dataptr,
                                     unsigned int   src_Y_ss,
                                     unsigned int   src_Cb_ss,
                                     unsigned int   src_Cr_ss,
                                     Xil_unsigned8* dst_dataptr,
                                     unsigned int   dst_ps,
                                     unsigned int   dst_ss)
{
    //
    // Copy the luminance (Y) part to the output frame.
    // Unroll the loops to copy 16 pixels per pass.
    // If the dst pixel stride is 3, explicitly use the location.
    //
    Xil_unsigned8* dst_scan    = dst_dataptr;
    Xil_signed16*  src_scan    = src_Y_dataptr;
    unsigned int   src_ss      = src_Y_ss;
    unsigned int   src_cb_ss   = src_Cb_ss;
    unsigned int   src_cr_ss   = src_Cr_ss;

    if(dst_ps == 3) { 
        for(int lines=16; lines!=0; lines--) {
            dst_scan[0]  = src_scan[0];  dst_scan[3]  = src_scan[1];
            dst_scan[6]  = src_scan[2];  dst_scan[9]  = src_scan[3];
            dst_scan[12] = src_scan[4];  dst_scan[15] = src_scan[5];
            dst_scan[18] = src_scan[6];  dst_scan[21] = src_scan[7];
            dst_scan[24] = src_scan[8];  dst_scan[27] = src_scan[9];
            dst_scan[30] = src_scan[10]; dst_scan[33] = src_scan[11];
            dst_scan[36] = src_scan[12]; dst_scan[39] = src_scan[13];
            dst_scan[42] = src_scan[14]; dst_scan[45] = src_scan[15];
            dst_scan += dst_ss;
            src_scan += src_ss;
        }

    } else {  // Non Optimized case

        Xil_unsigned8* dst_ptr; 
        for(int lines=16; lines!=0; lines--) {
            dst_ptr  = dst_scan;
            *dst_ptr = src_scan[0];  dst_ptr += dst_ps;
            *dst_ptr = src_scan[1];  dst_ptr += dst_ps;
            *dst_ptr = src_scan[2];  dst_ptr += dst_ps;
            *dst_ptr = src_scan[3];  dst_ptr += dst_ps;
            *dst_ptr = src_scan[4];  dst_ptr += dst_ps;
            *dst_ptr = src_scan[5];  dst_ptr += dst_ps;
            *dst_ptr = src_scan[6];  dst_ptr += dst_ps;
            *dst_ptr = src_scan[7];  dst_ptr += dst_ps;
            *dst_ptr = src_scan[8];  dst_ptr += dst_ps;
            *dst_ptr = src_scan[9];  dst_ptr += dst_ps;
            *dst_ptr = src_scan[10]; dst_ptr += dst_ps;
            *dst_ptr = src_scan[11]; dst_ptr += dst_ps;
            *dst_ptr = src_scan[12]; dst_ptr += dst_ps;
            *dst_ptr = src_scan[13]; dst_ptr += dst_ps;
            *dst_ptr = src_scan[14]; dst_ptr += dst_ps;
            *dst_ptr = src_scan[15]; dst_ptr += dst_ps;
            dst_scan += dst_ss;
            src_scan += src_ss;
        }
    }


    //
    // Upsample the chroma components (Cb and Cr)
    // Pixel stride and scanline stride in the destination
    // are doubled, since we upsample by 2 in both axes.
    //
    unsigned int chroma_ps = dst_ps * 2;
    unsigned int chroma_ss = dst_ss * 2;

    Xil_signed16*  src_cb_scan = src_Cb_dataptr;
    Xil_signed16*  src_cr_scan = src_Cr_dataptr;

    dst_scan = dst_dataptr + 1;

    Xil_unsigned8 cb;
    Xil_unsigned8 cr;
    if(dst_ps == 3) {

        for(int line=8; line!=0; line--) {
            //
            // Fill top (tptr) and bottom (bptr) rows in one loop
            //
            Xil_signed16*  src_cb_ptr = src_cb_scan;
            Xil_signed16*  src_cr_ptr = src_cr_scan;
            Xil_unsigned8* tptr = dst_scan;
            Xil_unsigned8* bptr = dst_scan + dst_ss;
            for(int samp=8; samp!=0; samp--) {
                cb = *src_cb_ptr++;
                cr = *src_cr_ptr++;
                tptr[0] = cb; tptr[1] = cr; tptr[3] = cb; tptr[4] = cr;
                bptr[0] = cb; bptr[1] = cr; bptr[3] = cb; bptr[4] = cr;
                tptr += 6;
                bptr += 6;
            }
            src_cb_scan += src_cb_ss;
            src_cr_scan += src_cr_ss;
            dst_scan    += chroma_ss;
        }

    } else {

        unsigned int cb_r = dst_ps;
        unsigned int cr_r = dst_ps + 1;
        for(int line=8; line!=0; line--) {
            Xil_signed16*  src_cb_ptr = src_cb_scan;
            Xil_signed16*  src_cr_ptr = src_cr_scan;
            Xil_unsigned8* tptr = dst_scan;
            Xil_unsigned8* bptr = dst_scan + dst_ss;
            for(int samp=8; samp!=0; samp--) {
                cb = *src_cb_ptr++;
                cr = *src_cr_ptr++;
                tptr[0] = cb; tptr[1] = cr; tptr[cb_r] = cb; tptr[cr_r] = cr;
                bptr[0] = cb; bptr[1] = cr; bptr[cb_r] = cb; bptr[cr_r] = cr;
                tptr += chroma_ps;
                bptr += chroma_ps;
            }
            src_cb_scan += src_cb_ss;
            src_cr_scan += src_cr_ss;
            dst_scan    += chroma_ss;
        }

    }

}

//
// Routine to take a "4:1:1" macroblock and copy it to the destination,
// upsampling the chroma components by 2 in both axes.
// This version has a BYTE src and a BYTE dst.
//
void 
BlockMan::upsample411BlockFromRaster(Xil_unsigned8* src_Y_dataptr,
                                     Xil_unsigned8* src_Cb_dataptr,
                                     Xil_unsigned8* src_Cr_dataptr,
                                     unsigned int   src_Y_ss,
                                     unsigned int   src_Cb_ss,
                                     unsigned int   src_Cr_ss,
                                     Xil_unsigned8* dst_dataptr,
                                     unsigned int   dst_ps,
                                     unsigned int   dst_ss)
{
    //
    // Copy the luminance (Y) part to the output frame.
    // Unroll the loops to copy 16 pixels per pass.
    // If the dst pixel stride is 3, explicitly use the location.
    //
    Xil_unsigned8* dst_scan    = dst_dataptr;
    Xil_unsigned8* src_scan    = src_Y_dataptr;
    unsigned int   src_ss      = src_Y_ss;
    unsigned int   src_cb_ss   = src_Cb_ss;
    unsigned int   src_cr_ss   = src_Cr_ss;

    if(dst_ps == 3) { 
        for(int lines=16; lines!=0; lines--) {
            dst_scan[0]  = src_scan[0];  dst_scan[3]  = src_scan[1];
            dst_scan[6]  = src_scan[2];  dst_scan[9]  = src_scan[3];
            dst_scan[12] = src_scan[4];  dst_scan[15] = src_scan[5];
            dst_scan[18] = src_scan[6];  dst_scan[21] = src_scan[7];
            dst_scan[24] = src_scan[8];  dst_scan[27] = src_scan[9];
            dst_scan[30] = src_scan[10]; dst_scan[33] = src_scan[11];
            dst_scan[36] = src_scan[12]; dst_scan[39] = src_scan[13];
            dst_scan[42] = src_scan[14]; dst_scan[45] = src_scan[15];
            dst_scan += dst_ss;
            src_scan += src_ss;
        }

    } else {  // Non Optimized case

        Xil_unsigned8* dst_ptr; 
        for(int lines=16; lines!=0; lines--) {
            dst_ptr  = dst_scan;
            *dst_ptr = src_scan[0];  dst_ptr += dst_ps;
            *dst_ptr = src_scan[1];  dst_ptr += dst_ps;
            *dst_ptr = src_scan[2];  dst_ptr += dst_ps;
            *dst_ptr = src_scan[3];  dst_ptr += dst_ps;
            *dst_ptr = src_scan[4];  dst_ptr += dst_ps;
            *dst_ptr = src_scan[5];  dst_ptr += dst_ps;
            *dst_ptr = src_scan[6];  dst_ptr += dst_ps;
            *dst_ptr = src_scan[7];  dst_ptr += dst_ps;
            *dst_ptr = src_scan[8];  dst_ptr += dst_ps;
            *dst_ptr = src_scan[9];  dst_ptr += dst_ps;
            *dst_ptr = src_scan[10]; dst_ptr += dst_ps;
            *dst_ptr = src_scan[11]; dst_ptr += dst_ps;
            *dst_ptr = src_scan[12]; dst_ptr += dst_ps;
            *dst_ptr = src_scan[13]; dst_ptr += dst_ps;
            *dst_ptr = src_scan[14]; dst_ptr += dst_ps;
            *dst_ptr = src_scan[15]; dst_ptr += dst_ps;
            dst_scan += dst_ss;
            src_scan += src_ss;
        }
    }


    //
    // Upsample the chroma components (Cb and Cr)
    // Pixel stride and scanline stride in the destination
    // are doubled, since we upsample by 2 in both axes.
    //
    unsigned int chroma_ps = dst_ps * 2;
    unsigned int chroma_ss = dst_ss * 2;

    Xil_unsigned8*  src_cb_scan = src_Cb_dataptr;
    Xil_unsigned8*  src_cr_scan = src_Cr_dataptr;

    dst_scan = dst_dataptr + 1;

    Xil_unsigned8 cb;
    Xil_unsigned8 cr;
    if(dst_ps == 3) {

        for(int line=8; line!=0; line--) {
            //
            // Fill top (tptr) and bottom (bptr) rows in one loop
            //
            Xil_unsigned8* src_cb_ptr = src_cb_scan;
            Xil_unsigned8* src_cr_ptr = src_cr_scan;
            Xil_unsigned8* tptr = dst_scan;
            Xil_unsigned8* bptr = dst_scan + dst_ss;
            for(int samp=8; samp!=0; samp--) {
                cb = *src_cb_ptr++;
                cr = *src_cr_ptr++;
                tptr[0] = cb; tptr[1] = cr; tptr[3] = cb; tptr[4] = cr;
                bptr[0] = cb; bptr[1] = cr; bptr[3] = cb; bptr[4] = cr;
                tptr += 6;
                bptr += 6;
            }
            src_cb_scan += src_cb_ss;
            src_cr_scan += src_cr_ss;
            dst_scan    += chroma_ss;
        }

    } else {

        unsigned int cb_r = dst_ps;
        unsigned int cr_r = dst_ps + 1;
        for(int line=8; line!=0; line--) {
            Xil_unsigned8* src_cb_ptr = src_cb_scan;
            Xil_unsigned8* src_cr_ptr = src_cr_scan;
            Xil_unsigned8* tptr = dst_scan;
            Xil_unsigned8* bptr = dst_scan + dst_ss;
            for(int samp=8; samp!=0; samp--) {
                cb = *src_cb_ptr++;
                cr = *src_cr_ptr++;
                tptr[0] = cb; tptr[1] = cr; tptr[cb_r] = cb; tptr[cr_r] = cr;
                bptr[0] = cb; bptr[1] = cr; bptr[cb_r] = cb; bptr[cr_r] = cr;
                tptr += chroma_ps;
                bptr += chroma_ps;
            }
            src_cb_scan += src_cb_ss;
            src_cr_scan += src_cr_ss;
            dst_scan    += chroma_ss;
        }

    }

}

//
// Routine to take a "4:1:1" DCT block and copy it to the destination,
// upsampling the chroma components by 2 in both axes.
// This version has a BYTE src and a BYTE dst.
//
void 
BlockMan::upsample411Block8FromRaster(Xil_unsigned8* src_Y_dataptr,
                                     Xil_unsigned8* src_Cb_dataptr,
                                     Xil_unsigned8* src_Cr_dataptr,
                                     unsigned int   src_Y_ss,
                                     unsigned int   src_Cb_ss,
                                     unsigned int   src_Cr_ss,
                                     Xil_unsigned8* dst_dataptr,
                                     unsigned int   dst_ps,
                                     unsigned int   dst_ss)
{
    //
    // Copy the luminance (Y) part to the output frame.
    // Unroll the loops to copy 16 pixels per pass.
    // If the dst pixel stride is 3, explicitly use the location.
    //
    Xil_unsigned8* dst_scan    = dst_dataptr;
    Xil_unsigned8* src_scan    = src_Y_dataptr;
    unsigned int   src_ss      = src_Y_ss;
    unsigned int   src_cb_ss   = src_Cb_ss;
    unsigned int   src_cr_ss   = src_Cr_ss;

    if(dst_ps == 3) { 
        for(int lines=8; lines!=0; lines--) {
            dst_scan[0]  = src_scan[0];  dst_scan[3]  = src_scan[1];
            dst_scan[6]  = src_scan[2];  dst_scan[9]  = src_scan[3];
            dst_scan[12] = src_scan[4];  dst_scan[15] = src_scan[5];
            dst_scan[18] = src_scan[6];  dst_scan[21] = src_scan[7];
            dst_scan += dst_ss;
            src_scan += src_ss;
        }

    } else {  // Non Optimized case

        Xil_unsigned8* dst_ptr; 
        for(int lines=8; lines!=0; lines--) {
            dst_ptr  = dst_scan;
            *dst_ptr = src_scan[0];  dst_ptr += dst_ps;
            *dst_ptr = src_scan[1];  dst_ptr += dst_ps;
            *dst_ptr = src_scan[2];  dst_ptr += dst_ps;
            *dst_ptr = src_scan[3];  dst_ptr += dst_ps;
            *dst_ptr = src_scan[4];  dst_ptr += dst_ps;
            *dst_ptr = src_scan[5];  dst_ptr += dst_ps;
            *dst_ptr = src_scan[6];  dst_ptr += dst_ps;
            *dst_ptr = src_scan[7];  dst_ptr += dst_ps;
            dst_scan += dst_ss;
            src_scan += src_ss;
        }
    }


    //
    // Upsample the chroma components (Cb and Cr)
    // Pixel stride and scanline stride in the destination
    // are doubled, since we upsample by 2 in both axes.
    //
    unsigned int chroma_ps = dst_ps * 2;
    unsigned int chroma_ss = dst_ss * 2;

    Xil_unsigned8*  src_cb_scan = src_Cb_dataptr;
    Xil_unsigned8*  src_cr_scan = src_Cr_dataptr;

    dst_scan = dst_dataptr + 1;

    Xil_unsigned8 cb;
    Xil_unsigned8 cr;
    if(dst_ps == 3) {

        for(int line=4; line!=0; line--) {
            //
            // Fill top (tptr) and bottom (bptr) rows in one loop
            //
            Xil_unsigned8* src_cb_ptr = src_cb_scan;
            Xil_unsigned8* src_cr_ptr = src_cr_scan;
            Xil_unsigned8* tptr = dst_scan;
            Xil_unsigned8* bptr = dst_scan + dst_ss;
            for(int samp=4; samp!=0; samp--) {
                cb = *src_cb_ptr++;
                cr = *src_cr_ptr++;
                tptr[0] = cb; tptr[1] = cr; tptr[3] = cb; tptr[4] = cr;
                bptr[0] = cb; bptr[1] = cr; bptr[3] = cb; bptr[4] = cr;
                tptr += 6;
                bptr += 6;
            }
            src_cb_scan += src_cb_ss;
            src_cr_scan += src_cr_ss;
            dst_scan    += chroma_ss;
        }

    } else {

        unsigned int cb_r = dst_ps;
        unsigned int cr_r = dst_ps + 1;
        for(int line=4; line!=0; line--) {
            Xil_unsigned8* src_cb_ptr = src_cb_scan;
            Xil_unsigned8* src_cr_ptr = src_cr_scan;
            Xil_unsigned8* tptr = dst_scan;
            Xil_unsigned8* bptr = dst_scan + dst_ss;
            for(int samp=4; samp!=0; samp--) {
                cb = *src_cb_ptr++;
                cr = *src_cr_ptr++;
                tptr[0] = cb; tptr[1] = cr; tptr[cb_r] = cb; tptr[cr_r] = cr;
                bptr[0] = cb; bptr[1] = cr; bptr[cb_r] = cb; bptr[cr_r] = cr;
                tptr += chroma_ps;
                bptr += chroma_ps;
            }
            src_cb_scan += src_cb_ss;
            src_cr_scan += src_cr_ss;
            dst_scan    += chroma_ss;
        }

    }

}

//
// Utility routine to copy data from a upsampled temporary block
// to the destination. Since edge blocks are a very small percentage
// of the total data, this approach leads to considerable code
// simplification. The upsampler can just always deal with 16x16
// blocks and this routine will deal with the edges.
// Note: This is only posssible because the source data always
//       contains complete macroblocks.
//
void
BlockMan::copyPartialBlock(Xil_unsigned8* src_block, 
                           Xil_unsigned8* dst_block,
                           unsigned int   nx,
                           unsigned int   ny,
                           unsigned int   dst_ps,
                           unsigned int   dst_ss)
{
    Xil_unsigned8* src_scan = src_block;
    Xil_unsigned8* dst_scan = dst_block;

    if(doOrderedDither) {
        //
        // Single band destination
        //
        for(int y=ny; y!=0; y--) {
            Xil_unsigned8* src_pixel = src_scan;
            Xil_unsigned8* dst_pixel = dst_scan;
            for(int x=nx; x!=0; x--) {
                *dst_pixel = *src_pixel++;
                dst_pixel += dst_ps;
            }
            src_scan += 48;    // Tmp macroblock is sized as 16 triplets
            dst_scan += dst_ss;
        }
    } else {
        //
        // All other cases are 3-band destinations
        //
        for(int y=ny; y!=0; y--) {
            Xil_unsigned8* src_pixel = src_scan;
            Xil_unsigned8* dst_pixel = dst_scan;
            for(int x=nx; x!=0; x--) {
                dst_pixel[0] = src_pixel[0];
                dst_pixel[1] = src_pixel[1];
                dst_pixel[2] = src_pixel[2];
                src_pixel += 3;
                dst_pixel += dst_ps;
            }
            src_scan += 48;    // Tmp macroblock is sized as 16 triplets
            dst_scan += dst_ss;
        }
    }
}

//
// Copy a BYTE block from one history raster to another
//
void
BlockMan::copyBlockRasterToRaster(Xil_unsigned8* src,
                                  Xil_unsigned8* dst,
                                  unsigned int   width,
                                  unsigned int   height,
                                  unsigned int   src_ss,
                                  unsigned int   dst_ss)
{
    //
    // Since we know the raster was set up aligned, we are free
    // to cast the pointer to an int* and use int transfers.
    // We know that the pixel stride is 1 in each raster.
    //
    int* pSrc = (int*) src;
    int* pDst = (int*) dst;

    //
    // Convert the strides from bytes to ints
    //
    unsigned int src32_ss = src_ss / 4;
    unsigned int dst32_ss = dst_ss / 4;

    //
    // Its either 8x8 blocks or 16x16 blocks
    //
    if(width == 8) {
        for(int line=height; line!=0; line--) {
            pDst[0] = pSrc[0];
            pDst[1] = pSrc[1];
            pSrc += src32_ss;
            pDst += dst32_ss;
        }
    } else {
        //
        // width == 16
        //
        for(int line=height; line!=0; line--) {
            pDst[0] = pSrc[0];
            pDst[1] = pSrc[1];
            pDst[2] = pSrc[2];
            pDst[3] = pSrc[3];
            pSrc += src32_ss;
            pDst += dst32_ss;
        }
    }
}

//
// Copy a SHORT block from one history raster to another
//
void
BlockMan::copyBlockRasterToRaster(Xil_signed16* src,
                                  Xil_signed16* dst,
                                  unsigned int  width,
                                  unsigned int  height,
                                  unsigned int  src_ss,
                                  unsigned int  dst_ss)
{
    //
    // Since we know the raster was set up aligned, we are free
    // to cast the pointer to an int* and use int transfers.
    // We know that the pixel stride is 1 in each raster.
    //
    int* pSrc = (int*) src;
    int* pDst = (int*) dst;

    //
    // Convert the strides from shorts to ints
    //
    unsigned int src32_ss = src_ss / 2;
    unsigned int dst32_ss = dst_ss / 2;

    //
    // Its either 8x8 blocks or 16x16 blocks
    //
    if(width == 8) {
        for(int line=height; line!=0; line--) {
            pDst[0] = pSrc[0];
            pDst[1] = pSrc[1];
            pDst[2] = pSrc[2];
            pDst[3] = pSrc[3];
            pSrc += src32_ss;
            pDst += dst32_ss;
        }
    } else {
        //
        // width == 16
        //
        for(int line=height; line!=0; line--) {
            pDst[0] = pSrc[0];
            pDst[1] = pSrc[1];
            pDst[2] = pSrc[2];
            pDst[3] = pSrc[3];
            pDst[4] = pSrc[4];
            pDst[5] = pSrc[5];
            pDst[6] = pSrc[6];
            pDst[7] = pSrc[7];
            pSrc += src32_ss;
            pDst += dst32_ss;
        }
    }
}

