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
//  File:       Ycc2RgbConverter.cc
//  Project:    XIL
//  Revision:   1.7
//  Last Mod:   10:16:25, 03/10/00
//
//  Description:
//
//    YCbCr to RGB709 Color conversion object for use in DCT-based codecs
//    Each call to cvtBlock converts a single 8x8 block.
//    The table is allocated only once.
//
//------------------------------------------------------------------------
//    COPYRIGHT
//------------------------------------------------------------------------
#pragma ident   "@(#)Ycc2RgbConverter.cc	1.7\t00/03/10  "

#include "Ycc2RgbConverter.hh"
#include "XiliUtils.hh"

#define GUARD_BAND      128
#define INPUT_RANGE     256

//
// Constructor - build the tables for the converter
//
//   Compute the floating point arithmetic for converting YUV->RGB and
//   store in tables.  Include Y_OFFSET here to save some add operations 
//   during YUV->RGB.
//
//   Red and green terms are combined into a single entry (2 shorts).
//   Blue and green terms are combined into a single entry (2 shorts).
//
//   The conversion being used here is as follows:
//
//     R = 1.164384 * (Y-16)                         + 1.596432 * (Cr - 128) 
//     G = 1.164384 * (Y-16) - 0.392973 * (Cb - 128) - 0.812948 * (Cr - 128) 
//     B = 1.164384 * (Y-16) + 2.017406 * (Cb - 128) 
// 
//   All of the constant terms are multiplied through and added to 
//   one of the lookups used to create  the conversion. The table values
//   are also multiplied by 16 to provide an extra 4 bits of fractional
//   precision in the integer table values.
//
//   So we actually have 5 lookups, implemented as three tables.
//   The Cb and Cr input terms use the upper and lower 16 bits of
//   a 32 bit word to contain two lookup results.
//   So one lookup gets us both Cb contributions (or Cr).
//
//     Table       Output                            Output
//     Input   (upper 16 bits)                   (lower 16 bits)
//     -----   ---------------                   ---------------
//       Y     1.164384 * (Y -16)    {no split word used for Y term}
//       Cb   -0.392973 * (Cb - 128) + K    2.016406 * (Cb - 128) + K
//       Cr   -0.812948 * (Cr - 128)        1.596432 * (Cr - 128) + K
//
//
Ycc2RgbConverter::Ycc2RgbConverter(int tbl_offset)
{
    isOKFlag = FALSE;

    //
    // Allocate the memory for all tables as a single buffer 
    // Each of the color multiplier tables gets a size twice as
    // large as needed, to allow for out-of-range input values.
    //
    const int color_table_size = INPUT_RANGE + 2*GUARD_BAND; // As ints
    const int clamp_table_size = 3 * INPUT_RANGE;            // As bytes

    int table_size = 3 * color_table_size + clamp_table_size/sizeof(int);
    table_buffer = new int[table_size];
    if(table_buffer == NULL) {
        XIL_ERROR(NULL, XIL_ERROR_RESOURCE, "di-1", TRUE);
        return;
    }

    //
    // Set ptrs for each table within the single table buffer
    //
    redTable = table_buffer;
    bluTable = redTable + color_table_size;
    yTable   = bluTable + color_table_size;

    //
    // Point to the zero index point of the clamp table so that
    // positive and negative input values will index correctly.
    //
    clamp    = (Xil_unsigned8*)(yTable + color_table_size) + 256;

    //
    // Populate the tables.
    // Pack green value into high 16 bits of table entry.
    // Multiply by 16.0 so that resulting integer has 4 bits of fraction.
    // put 0.5 offset into Y band contribution so that final calculation
    // will be rounded.
    //
    //

    //
    // The values below MIN_CCIR_YUV are all set to the same value.
    //
    float y  = MIN_CCIR_YUV;
    float uv = MIN_CCIR_YUV;
    int y_value = (int) (8.0F + 18.630144F * y);

    int rv        = (int) (25.542912F * uv - 3567.57504F);
    int gv        = (int) (-13.007168F * uv);
    int red_value = (gv << 16) | (rv & 0xffff);

    int gu        = (int) (-6.287568F * uv + 2171.644638F);
    int bu        = (int) (32.262496F * uv - 4427.681008F);
    int blu_value = (gu << 16) | (bu & 0xffff);

    int i=0;
    for(int j=-GUARD_BAND; j<MIN_CCIR_YUV; j++) {
        yTable[i]   = y_value;
        redTable[i] = red_value;
        bluTable[i] = blu_value;
        i++;
    }

    //
    // Calculate the non-clamped values for the Y part and
    // the red/blue contributions.
    //
    int iy  = i;
    int iuv = i;
    for(j=MIN_CCIR_YUV; j<MAX_CCIR_Y; j++) {
        y = (float)j;
        yTable[iy] = (int) (8.0F + 18.630144F * y);
        iy++;
    }

    for(j=MIN_CCIR_YUV; j<MAX_CCIR_UV; j++) {
        uv = (float)j;
        rv = (int) (25.542912F * uv - 3567.57504F);
        gv = (int) (-13.007168F * uv);
        redTable[iuv] = (gv << 16) | (rv & 0xffff);

        gu = (int) (-6.287568F * uv + 2171.644638F);
        bu = (int) (32.262496F * uv - 4427.681008F);
        bluTable[iuv] = (gu << 16) | (bu & 0xffff);
        iuv++;
    }

    //
    // The Y values above MAX_CCIR_Y are all set to the same value.
    //
    y = MAX_CCIR_Y;
    y_value = (int) (8.0F + 18.630144F * y);
    for(j=MAX_CCIR_Y; j<INPUT_RANGE+GUARD_BAND; j++) {
        yTable[iy] = y_value;
        iy++;
    }

    //
    // The red/blue values above MAX_CCIR_UV are all set to the same value.
    //
    uv = MAX_CCIR_UV;
    rv = (int) (25.542912F * uv - 3567.57504F);
    gv = (int) (-13.007168F * uv);
    red_value = (gv << 16) | (rv & 0xffff);

    gu = (int) (-6.287568F * uv + 2171.644638F);
    bu = (int) (32.262496F * uv - 4427.681008F);
    blu_value = (gu << 16) | (bu & 0xffff);
    for(j=MAX_CCIR_UV; j<INPUT_RANGE+GUARD_BAND; j++) {
        redTable[iuv] = red_value;
        bluTable[iuv] = blu_value;
        iuv++;
    }

    //
    // Set the zero point, i.e. where a value of zero will
    // index the table. The tbl_offset term is used to allow
    // this routine to handle Jpeg's bias of 128. Mpeg and
    // H.261 codecs do not use this offset, so their tbl_offset=0.
    //
    yTable   += GUARD_BAND + tbl_offset;
    redTable += GUARD_BAND + tbl_offset;
    bluTable += GUARD_BAND + tbl_offset;

    //
    // Populate a range limiting table to do the clamping to [0, 255].
    // The maximum range of input values is -256 to +511.
    //
    xili_memset(clamp-INPUT_RANGE, 0, INPUT_RANGE);
    for(i=0; i<INPUT_RANGE; i++) {
        clamp[i] = i;
    }
    xili_memset(clamp+INPUT_RANGE, 255, INPUT_RANGE);

    isOKFlag = TRUE;

    return;
}

Ycc2RgbConverter::~Ycc2RgbConverter()
{
    delete table_buffer;
}


//
// Wrapper to convert a Colorconvert a 16x16 macroblock.
// This will make 4 calls to the cvtBlock routines,
// one for each Y block. SHORT src version.
//
void
Ycc2RgbConverter::cvtMacroBlock(Xil_signed16*  yBlk,
                                Xil_signed16*  cbBlk,
                                Xil_signed16*  crBlk,
                                unsigned int   y_ss,
                                unsigned int   cb_ss,
                                unsigned int   cr_ss,
                                Xil_unsigned8* dst,
                                unsigned int   dst_ps,
                                unsigned int   dst_ss)
{
    //
    // Upper left block
    //
    cvtBlock(yBlk, cbBlk, crBlk, 
             y_ss, cb_ss, cr_ss,
             dst, dst_ps, dst_ss);

    //
    // Upper right block
    //
    cvtBlock(yBlk+8, cbBlk+4, crBlk+4, 
             y_ss, cb_ss, cr_ss,
             dst+8*dst_ps, dst_ps, dst_ss);

    //
    // Lower left block
    //
    cvtBlock(yBlk+8*y_ss, cbBlk+4*cb_ss, crBlk+4*cr_ss, 
             y_ss, cb_ss, cr_ss,
             dst+8*dst_ss, dst_ps, dst_ss);

    //
    // Lower right block
    //
    cvtBlock(yBlk+8*y_ss+8, cbBlk+4*cb_ss+4, crBlk+4*cr_ss+4, 
             y_ss, cb_ss, cr_ss,
             dst+8*(dst_ps+dst_ss), dst_ps, dst_ss);

}

//
// Wrapper to convert a Colorconvert a 16x16 macroblock.
// This will make 4 calls to the cvtBlock routines,
// one for each Y block. SHORT src version.
//
void
Ycc2RgbConverter::cvtMacroBlock422(Xil_signed16*  yBlk,
                                   Xil_signed16*  cbBlk,
                                   Xil_signed16*  crBlk,
                                   unsigned int   y_ss,
                                   unsigned int   cb_ss,
                                   unsigned int   cr_ss,
                                   Xil_unsigned8* dst,
                                   unsigned int   dst_ps,
                                   unsigned int   dst_ss)
{
    //
    //  Left block
    //
    cvtBlock(yBlk, cbBlk, crBlk, 
             y_ss, cb_ss, cr_ss,
             dst, dst_ps, dst_ss);

    //
    //  Right block
    //
    cvtBlock(yBlk+8, cbBlk+4, crBlk+4, 
             y_ss, cb_ss, cr_ss,
             dst+8*dst_ps, dst_ps, dst_ss);

}



//
// Wrapper to convert a Colorconvert a 16x16 macroblock.
// This will make 4 calls to the cvtBlock routines,
// one for each Y block. BYTE src version.
//
void
Ycc2RgbConverter::cvtMacroBlock(Xil_unsigned8* yBlk,
                                Xil_unsigned8* cbBlk,
                                Xil_unsigned8* crBlk,
                                unsigned int   y_ss,
                                unsigned int   cb_ss,
                                unsigned int   cr_ss,
                                Xil_unsigned8* dst,
                                unsigned int   dst_ps,
                                unsigned int   dst_ss)
{
    //
    // Upper left block
    //
    cvtBlock(yBlk, cbBlk, crBlk, 
             y_ss, cb_ss, cr_ss,
             dst, dst_ps, dst_ss);

    //
    // Upper right block
    //
    cvtBlock(yBlk+8, cbBlk+4, crBlk+4, 
             y_ss, cb_ss, cr_ss,
             dst+8*dst_ps, dst_ps, dst_ss);

    //
    // Lower left block
    //
    cvtBlock(yBlk+8*y_ss, cbBlk+4*cb_ss, crBlk+4*cr_ss, 
             y_ss, cb_ss, cr_ss,
             dst+8*dst_ss, dst_ps, dst_ss);

    //
    // Lower right block
    //
    cvtBlock(yBlk+8*y_ss+8, cbBlk+4*cb_ss+4, crBlk+4*cr_ss+4, 
             y_ss, cb_ss, cr_ss,
             dst+8*(dst_ps+dst_ss), dst_ps, dst_ss);

}



//
// Convert a single DCT block (8x8) from YCbCr 601 colorspace
// to RGB709 colorspace. The Cb and Cr pointers must point to
// the proper quadrant of the chroma blocks, since this routine is
// only for use with so-called 4:1:1 data in which the chroma channels
// have been downsampled 2:1 relative to the luma channel.
//
// The main loop is unrolled to process two lines of an 8x8 block,
// i.e. 16 output pixels. This only requires 4 Cb and 4 Cr pixels.
//
void
Ycc2RgbConverter::cvtBlock(Xil_signed16*  yBlk,
                           Xil_signed16*  cbBlk,
                           Xil_signed16*  crBlk,
                           unsigned int   ysrc_ss,
                           unsigned int   cbsrc_ss,
                           unsigned int   crsrc_ss,
                           Xil_unsigned8* dst,
                           unsigned int   dst_ps,
                           unsigned int   dst_ss)
{
    Xil_signed16*  ysrc_line     = yBlk;
    Xil_signed16*  cb_src_line   = cbBlk;
    Xil_signed16*  cr_src_line   = crBlk;
    Xil_unsigned8* dst_line      = dst;

    unsigned int   ysrc_line_inc = 2 * ysrc_ss;
    unsigned int   dst_line_inc  = 2 * dst_ss;
    int            inc_right     = dst_ps;
    int            inc_diag      = dst_ps - dst_ss;
    int            yll           = ysrc_ss;
    int            ylr           = ysrc_ss+1;

    //
    // Do four 2 line x 8 pixel strips
    //
    for(int linecount=4; linecount!=0; linecount-- ) {
        //
        // Get ptr to the current set of two lines
        //
        Xil_signed16*  pY   = ysrc_line;
        Xil_signed16*  pCb  = cb_src_line;
        Xil_signed16*  pCr  = cr_src_line;
        Xil_unsigned8* pDst = dst_line;

        //
        // Do 4 sets of 2x2 pixel blocks within 2 line x 8 pixel strip
        //
        for(int count2x2=4; count2x2!=0; count2x2-- ) {
            //
            // Get the two chroma values
            //
            int cb = pCb[0]; 

            int cr = pCr[0];

            //
            // Index into the tables holding the color contributions
            //
            int regCb = bluTable[cb];
            int regCr = redTable[cr];

            //
            // Get the Cr contribution to the Red output 
            // and the Cb contribution to the Blue output.
            // (These are in the 16 LSBs of each table value).
            //
            int redCC = (short)(regCr & 0xffff);
            int bluCC = (short)(regCb & 0xffff);

            //
            // Get the Cb and Cr contributions to the Green output.
            // (These are in the 16 MSBs of each table value).
            //
            int grnCC = (regCr>>16) + (regCb>>16);

            //
            // Get the unique Y value, calculate output BGR values,
            // undo the 16X scaling and clamp the output
            // values to the [0, 255] range. Repeat for whole 2x2 block.
            //
            int y = yTable[pY[0]];
            pDst[0] = clamp[(y + bluCC) >> 4];
            pDst[1] = clamp[(y + grnCC) >> 4];
            pDst[2] = clamp[(y + redCC) >> 4];

            pDst += inc_right;

            y = yTable[pY[1]];
            pDst[0] = clamp[(y + bluCC) >> 4];
            pDst[1] = clamp[(y + grnCC) >> 4];
            pDst[2] = clamp[(y + redCC) >> 4];

            pDst -= inc_diag;

            y = yTable[pY[yll]];
            pDst[0] = clamp[(y + bluCC) >> 4];
            pDst[1] = clamp[(y + grnCC) >> 4];
            pDst[2] = clamp[(y + redCC) >> 4];

            pDst += inc_right;

            y = yTable[pY[ylr]];
            pDst[0] = clamp[(y + bluCC) >> 4];
            pDst[1] = clamp[(y + grnCC) >> 4];
            pDst[2] = clamp[(y + redCC) >> 4];

            pDst += inc_diag;

            pY += 2;
            pCb++;
            pCr++;
        }
        ysrc_line   += ysrc_line_inc;
        cb_src_line += cbsrc_ss;
        cr_src_line += crsrc_ss;
        dst_line    += dst_line_inc;
    }

}

//
// Convert a single DCT block (8x8) from YCbCr 601 colorspace
// to RGB709 colorspace. The Cb and Cr pointers must point to
// the proper half of the chroma blocks, since this routine is
// only for use with so-called 4:2:2 data in which the chroma channels
// have been downsampled in X only 2:1 relative to the luma channel.
//
// The main loop is unrolled to process a full line of 8 output pixels
//
void
Ycc2RgbConverter::cvtBlock422(Xil_signed16*  yBlk,
                              Xil_signed16*  cbBlk,
                              Xil_signed16*  crBlk,
                              unsigned int   ysrc_ss,
                              unsigned int   cbsrc_ss,
                              unsigned int   crsrc_ss,
                              Xil_unsigned8* dst,
                              unsigned int   dst_ps,
                              unsigned int   dst_ss)
{
    Xil_signed16*  pY   = yBlk;
    Xil_signed16*  pCb  = cbBlk;
    Xil_signed16*  pCr  = crBlk;

    //
    // Do each line of the output block
    //
    for(int linecount=8; linecount!=0; linecount-- ) {

        Xil_unsigned8* pDst = dst;

        //
        // Get two chroma values
        //
        int cb = pCb[0]; 
        int cr = pCr[0];

        //
        // Index into the tables holding the color contributions
        //
        int regCb = bluTable[cb];
        int regCr = redTable[cr];

        //
        // Get the Cr contribution to the Red output 
        // and the Cb contribution to the Blue output.
        // (These are in the 16 LSBs of each table value).
        //
        int redCC = (short)(regCr & 0xffff);
        int bluCC = (short)(regCb & 0xffff);

        //
        // Get the Cb and Cr contributions to the Green output.
        // (These are in the 16 MSBs of each table value).
        //
        int grnCC = (regCr>>16) + (regCb>>16);

        //
        // Get the unique Y value, calculate output BGR values,
        // undo the 16X scaling and clamp the output
        // values to the [0, 255] range. Repeat for whole 2x2 block.
        //
        int y = yTable[pY[0]];
        pDst[0] = clamp[(y + bluCC) >> 4];
        pDst[1] = clamp[(y + grnCC) >> 4];
        pDst[2] = clamp[(y + redCC) >> 4];

        pDst += dst_ps;

        y = yTable[pY[1]];
        pDst[0] = clamp[(y + bluCC) >> 4];
        pDst[1] = clamp[(y + grnCC) >> 4];
        pDst[2] = clamp[(y + redCC) >> 4];

        pDst += dst_ps;

        //
        // Repeat same pattern for the remaining 6 pixels in the line
        //
        cb = pCb[1]; 
        cr = pCr[1];
        regCb = bluTable[cb];
        regCr = redTable[cr];
        redCC = (short)(regCr & 0xffff);
        bluCC = (short)(regCb & 0xffff);
        grnCC = (regCr>>16) + (regCb>>16);
        y = yTable[pY[2]];
        pDst[0] = clamp[(y + bluCC) >> 4];
        pDst[1] = clamp[(y + grnCC) >> 4];
        pDst[2] = clamp[(y + redCC) >> 4];
        pDst += dst_ps;
        y = yTable[pY[3]];
        pDst[0] = clamp[(y + bluCC) >> 4];
        pDst[1] = clamp[(y + grnCC) >> 4];
        pDst[2] = clamp[(y + redCC) >> 4];
        pDst += dst_ps;

        cb = pCb[2]; 
        cr = pCr[2];
        regCb = bluTable[cb];
        regCr = redTable[cr];
        redCC = (short)(regCr & 0xffff);
        bluCC = (short)(regCb & 0xffff);
        grnCC = (regCr>>16) + (regCb>>16);
        y = yTable[pY[4]];
        pDst[0] = clamp[(y + bluCC) >> 4];
        pDst[1] = clamp[(y + grnCC) >> 4];
        pDst[2] = clamp[(y + redCC) >> 4];
        pDst += dst_ps;
        y = yTable[pY[5]];
        pDst[0] = clamp[(y + bluCC) >> 4];
        pDst[1] = clamp[(y + grnCC) >> 4];
        pDst[2] = clamp[(y + redCC) >> 4];
        pDst += dst_ps;

        cb = pCb[3]; 
        cr = pCr[3];
        regCb = bluTable[cb];
        regCr = redTable[cr];
        redCC = (short)(regCr & 0xffff);
        bluCC = (short)(regCb & 0xffff);
        grnCC = (regCr>>16) + (regCb>>16);
        y = yTable[pY[6]];
        pDst[0] = clamp[(y + bluCC) >> 4];
        pDst[1] = clamp[(y + grnCC) >> 4];
        pDst[2] = clamp[(y + redCC) >> 4];
        pDst += dst_ps;
        y = yTable[pY[7]];
        pDst[0] = clamp[(y + bluCC) >> 4];
        pDst[1] = clamp[(y + grnCC) >> 4];
        pDst[2] = clamp[(y + redCC) >> 4];

        pY  += ysrc_ss;
        pCb += cbsrc_ss;
        pCr += crsrc_ss;
        dst += dst_ss;
    }

}

//
// Same as cvtBlock() above, but for byte data.
// This is needed for the H261 codec which differs
// from the Jpeg and Mpeg1 versions (which use short data).
//
void
Ycc2RgbConverter::cvtBlock(Xil_unsigned8*  yBlk,
                           Xil_unsigned8*  cbBlk,
                           Xil_unsigned8*  crBlk,
                           unsigned int   ysrc_ss,
                           unsigned int   cbsrc_ss,
                           unsigned int   crsrc_ss,
                           Xil_unsigned8* dst,
                           unsigned int   dst_ps,
                           unsigned int   dst_ss)
{
    Xil_unsigned8*  ysrc_line     = yBlk;
    Xil_unsigned8*  cb_src_line   = cbBlk;
    Xil_unsigned8*  cr_src_line   = crBlk;
    Xil_unsigned8* dst_line      = dst;

    unsigned int   ysrc_line_inc = 2 * ysrc_ss;
    unsigned int   dst_line_inc  = 2 * dst_ss;
    int            inc_right     = dst_ps;
    int            inc_diag      = dst_ps - dst_ss;
    int            yll           = ysrc_ss;
    int            ylr           = ysrc_ss+1;

    //
    // Do four 2 line x 8 pixel strips
    //
    for(int linecount=4; linecount!=0; linecount-- ) {
        //
        // Get ptr to the current set of two lines
        //
        Xil_unsigned8*  pY   = ysrc_line;
        Xil_unsigned8*  pCb  = cb_src_line;
        Xil_unsigned8*  pCr  = cr_src_line;
        Xil_unsigned8*  pDst = dst_line;

        //
        // Do 4 sets of 2x2 pixel blocks within 2 line x 8 pixel strip
        //
        for(int count2x2=4; count2x2!=0; count2x2-- ) {
            //
            // Get the two chroma values
            //
            int cb = pCb[0]; 
            int cr = pCr[0];

            //
            // Index into the tables holding the color contributions
            //
            int regCb = bluTable[cb];
            int regCr = redTable[cr];

            //
            // Get the Cr contribution to the Red output 
            // and the Cb contribution to the Blue output.
            // (These are in the 16 LSBs of each table value).
            //
            int redCC = (short)(regCr & 0xffff);
            int bluCC = (short)(regCb & 0xffff);

            //
            // Get the Cb and Cr contributions to the Green output.
            // (These are in the 16 MSBs of each table value).
            //
            int grnCC = (regCr>>16) + (regCb>>16);

            //
            // Get the unique Y value, calculate output BGR values,
            // undo the 16X scaling and clamp the output
            // values to the [0, 255] range. Repeat for whole 2x2 block.
            //
            int y = yTable[pY[0]];
            pDst[0] = clamp[(y + bluCC) >> 4];
            pDst[1] = clamp[(y + grnCC) >> 4];
            pDst[2] = clamp[(y + redCC) >> 4];

            pDst += inc_right;

            y = yTable[pY[1]];
            pDst[0] = clamp[(y + bluCC) >> 4];
            pDst[1] = clamp[(y + grnCC) >> 4];
            pDst[2] = clamp[(y + redCC) >> 4];

            pDst -= inc_diag;

            y = yTable[pY[yll]];
            pDst[0] = clamp[(y + bluCC) >> 4];
            pDst[1] = clamp[(y + grnCC) >> 4];
            pDst[2] = clamp[(y + redCC) >> 4];

            pDst += inc_right;

            y = yTable[pY[ylr]];
            pDst[0] = clamp[(y + bluCC) >> 4];
            pDst[1] = clamp[(y + grnCC) >> 4];
            pDst[2] = clamp[(y + redCC) >> 4];

            pDst += inc_diag;

            pY += 2;
            pCb++;
            pCr++;
        }
        ysrc_line   += ysrc_line_inc;
        cb_src_line += cbsrc_ss;
        cr_src_line += crsrc_ss;
        dst_line    += dst_line_inc;
    }

}

//
// This version converts a 4x4 cell of pixels as used in Cell and CellB
//
void
Ycc2RgbConverter::cvtCellB(Xil_unsigned8  y0,
                           Xil_unsigned8  y1,
                           Xil_unsigned8  u,
                           Xil_unsigned8  v,
                           Xil_unsigned16 mask,
                           Xil_unsigned8* dst,
                           unsigned int   dst_ps,
                           unsigned int   dst_ss)
{
    Xil_unsigned8* dst_line = dst;

    //
    // Shift the 16 bit mask to the MSB of a 32 bit word.
    // This lets us just test the MSB and then shift to get the next value.
    //
    unsigned int   mask32   = mask << 16;

    //
    // Index into the tables holding the color contributions for U and V
    //
    int regCb = bluTable[u];
    int regCr = redTable[v];

    //
    // Get the Cr contribution to the Red output 
    // and the Cb contribution to the Blue output.
    // (These are in the 16 LSBs of each table value).
    //
    int redCC = (short)(regCr & 0xffff);
    int bluCC = (short)(regCb & 0xffff);

    //
    // Get the Cb and Cr contributions to the Green output.
    // (These are in the 16 MSBs of each table value).
    //
    int grnCC = (regCr>>16) + (regCb>>16);

    //
    // Get the color contributions of each Y value
    //
    int yCC0 = yTable[y0];
    int yCC1 = yTable[y1];

    //
    // This stuff is all constant for a cell, so precalculate
    // the two possible sets of RGB values for this cell.
    // Get the Y0 or Y1 value, calculate output BGR values,
    // undo the 16X scaling and clamp the output
    // values to the [0, 255] range.
    //
    Xil_unsigned8 b0 = clamp[(yCC0 + bluCC) >> 4];
    Xil_unsigned8 g0 = clamp[(yCC0 + grnCC) >> 4];
    Xil_unsigned8 r0 = clamp[(yCC0 + redCC) >> 4];

    Xil_unsigned8 b1 = clamp[(yCC1 + bluCC) >> 4];
    Xil_unsigned8 g1 = clamp[(yCC1 + grnCC) >> 4];
    Xil_unsigned8 r1 = clamp[(yCC1 + redCC) >> 4];

    //
    // Do one row (4 pixels) per loop pass
    //
    for(int linecount=4; linecount!=0; linecount-- ) {

        Xil_unsigned8* pDst = dst_line;

        //
        // Test the MSB of the mask to decide which set of 
        // RGB values to use.
        //
        if(mask32 & 0x80000000) {
            pDst[0] = b1;
            pDst[1] = g1;
            pDst[2] = r1;
        } else {
            pDst[0] = b0;
            pDst[1] = g0;
            pDst[2] = r0;
        }
        mask32 <<= 1;
        pDst += dst_ps;

        if(mask32 & 0x80000000) {
            pDst[0] = b1;
            pDst[1] = g1;
            pDst[2] = r1;
        } else {
            pDst[0] = b0;
            pDst[1] = g0;
            pDst[2] = r0;
        }
        mask32 <<= 1;
        pDst += dst_ps;

        if(mask32 & 0x80000000) {
            pDst[0] = b1;
            pDst[1] = g1;
            pDst[2] = r1;
        } else {
            pDst[0] = b0;
            pDst[1] = g0;
            pDst[2] = r0;
        }
        mask32 <<= 1;
        pDst += dst_ps;

        if(mask32 & 0x80000000) {
            pDst[0] = b1;
            pDst[1] = g1;
            pDst[2] = r1;
        } else {
            pDst[0] = b0;
            pDst[1] = g0;
            pDst[2] = r0;
        }
        mask32 <<= 1;

        dst_line += dst_ss;
    }

}

Xil_boolean
Ycc2RgbConverter::isOK()
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

