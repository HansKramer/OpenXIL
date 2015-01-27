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
//  File:       JpegMolecules.cc
//  Project:    XIL
//  Revision:   1.5
//  Last Mod:   10:14:38, 03/10/00
//
//  Description:
//
//    Routines to support Jpeg Molecules
//
//
//------------------------------------------------------------------------
//    COPYRIGHT
//------------------------------------------------------------------------
#pragma ident   "@(#)JpegMolecules.cc	1.5\t00/03/10  "

#include "JpegMolecules.hh"
#include "XiliUtils.hh"

//
// Define and initialize static variables
//
    int*           JpegYcc2Rgb::table_buffer = NULL;
    int*           JpegYcc2Rgb::redTable     = NULL;
    int*           JpegYcc2Rgb::bluTable     = NULL;
    int*           JpegYcc2Rgb::yTable       = NULL;
    Xil_unsigned8* JpegYcc2Rgb::clamp        = NULL;
    unsigned int   JpegYcc2Rgb::ref_count    = 0;

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
JpegYcc2Rgb::JpegYcc2Rgb()
{
    //
    // Lock while we check if the table has already been allocated
    //
    mutex.lock();
    if(table_buffer != NULL) {
        ref_count++;
        isOKFlag = TRUE;
        mutex.unlock();
        return;
    }

    isOKFlag = FALSE;

    //
    // Allocate the memory for all tables as a single buffer 
    // Each of the color multiplier tables gets a size twice as
    // large as needed, to allow for out-of-range input values.
    //
    const int color_table_size = 2 * INPUT_MAGNITUDE;  // Measured as ints
    const int clamp_table_size = 3 * 256;              // Measured as bytes

    int table_size = 3 * color_table_size + clamp_table_size/sizeof(int);
    table_buffer = new int[table_size];
    if(table_buffer == NULL) {
        XIL_ERROR(NULL, XIL_ERROR_RESOURCE, "di-1", TRUE);
        mutex.unlock();
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
    for(int j=-INPUT_MAGNITUDE + CODING_OFFSET; j< MIN_CCIR_YUV; j++) {
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
    for(j=MAX_CCIR_Y; j<INPUT_MAGNITUDE+CODING_OFFSET; j++) {
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
    for(j=MAX_CCIR_UV; j<INPUT_MAGNITUDE+CODING_OFFSET; j++) {
        redTable[iuv] = red_value;
        bluTable[iuv] = blu_value;
        iuv++;
    }

    yTable   += INPUT_MAGNITUDE;
    redTable += INPUT_MAGNITUDE;
    bluTable += INPUT_MAGNITUDE;

    //
    // Populate a range limiting table to do the clamping to [0, 255].
    // The maximum range of input values is -256 to +511.
    //
    xili_memset(clamp-256, 0, 256);
    for(i=0; i<256; i++) {
        clamp[i] = i;
    }
    xili_memset(clamp+256, 255, 256);

    isOKFlag = TRUE;

    ref_count++;
    mutex.unlock();
    return;
}

JpegYcc2Rgb::~JpegYcc2Rgb()
{
    mutex.lock();
    if(--ref_count == 0) {
        delete table_buffer;
        table_buffer = NULL;
    }
    mutex.unlock();
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
JpegYcc2Rgb::cvtBlock(Xil_signed16*  yBlk,
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

            y = yTable[pY[8]];
            pDst[0] = clamp[(y + bluCC) >> 4];
            pDst[1] = clamp[(y + grnCC) >> 4];
            pDst[2] = clamp[(y + redCC) >> 4];

            pDst += inc_right;

            y = yTable[pY[9]];
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

Xil_boolean
JpegYcc2Rgb::isOK()
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
