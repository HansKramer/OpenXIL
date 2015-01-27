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
//  File:       decompressColorConvert.cc
//  Project:    XIL
//  Revision:   1.12
//  Last Mod:   10:14:39, 03/10/00
//
//  Description:
//
//    Accelerated path for decompress->colorConvert molecules.
//    Conditions:
//      Dst image width and height mulst be multiples of 16.
//      Colorspace is YCC601 to RGB709.
//      
//
//
//------------------------------------------------------------------------
//    COPYRIGHT
//------------------------------------------------------------------------
#pragma ident   "@(#)decompressColorConvert.cc	1.12\t00/03/10  "

#include <xil/xilGPI.hh>
#include "XilDeviceManagerCompressionJpeg.hh"
#include "XilDeviceCompressionJpeg.hh"
#include "Ycc2RgbConverter.hh"

XilStatus 
XilDeviceCompressionJpeg::decompress411Frame(DecompressInfo* di)
{

    Ycc2RgbConverter*     colorCvt;
    XiliOrderedDitherLut* ditherCvt;
    if(di->doColorConvert) {
        colorCvt = (Ycc2RgbConverter*)di->objectPtr1;
    } else if (di->doOrderedDither) {
        ditherCvt = (XiliOrderedDitherLut*)di->objectPtr1;
    }
    // 
    // Get the header information from the JpegDecompressorData object
    //
    JpegDecompressorData* decoder = &decompData;
    Header& hdr = decoder->header;
    int restartInterval = hdr.restartInterval;
    
    // 
    // Initailze the Huffman decoder
    //
    decoder->huffmandecoder.initdecode();

    //
    // Storage for six 8x8 block of decoded coefficients (one MCU for 411).
    // These are 16 bit signed quantities, but the IDCT
    // routine uses double-word loads and stores, so
    // we need to insure 8 byte alignment. Declare as doubles to do this.
    //
    double alltiles[6 * 16];
    Xil_signed16* Y1Tile = (Xil_signed16*)(alltiles); 
    Xil_signed16* Y2Tile = (Xil_signed16*)(alltiles + 16); 
    Xil_signed16* Y3Tile = (Xil_signed16*)(alltiles + 2*16); 
    Xil_signed16* Y4Tile = (Xil_signed16*)(alltiles + 3*16); 
    Xil_signed16* CbTile = (Xil_signed16*)(alltiles + 4*16); 
    Xil_signed16* CrTile = (Xil_signed16*)(alltiles + 5*16); 

    //
    // Get the huffman table and quantization table indices.
    // Also start the DC predictors out at zero.
    //
    banddata* bptr = hdr.band_data;

    int* luma_actable = bptr[0].actable;
    int* luma_dctable = bptr[0].dctable;
    int  luma_qindex  = bptr[0].qindex;
    int  luma_dcpred  = 0;

    int* cb_actable   = bptr[1].actable;
    int* cb_dctable   = bptr[1].dctable;
    int  cb_qindex    = bptr[1].qindex;
    int  cb_dcpred    = 0;

    int* cr_actable   = bptr[2].actable;
    int* cr_dctable   = bptr[2].dctable;
    int  cr_qindex    = bptr[2].qindex;
    int  cr_dcpred    = 0;

    int MCUcount = 0;

    //
    // Process all of the 16x16 (output) pixel macroblocks
    //
    Xil_unsigned8* dst_yblock = (Xil_unsigned8*)di->image_dataptr;
    for(unsigned int vMB=0; vMB<(hdr.height)/16; vMB++) {

        Xil_unsigned8* dst = dst_yblock;
        for(unsigned int hMB=0; hMB<(hdr.width)/16; hMB++) {
            //
            // Decode the 4 Y (luma) blocks
            //
            luma_dcpred = decoder->decode8x8(Y1Tile, luma_dcpred, luma_qindex, 
                                             luma_dctable, luma_actable);

            luma_dcpred = decoder->decode8x8(Y2Tile, luma_dcpred, luma_qindex, 
                                             luma_dctable, luma_actable);

            luma_dcpred = decoder->decode8x8(Y3Tile, luma_dcpred, luma_qindex, 
                                             luma_dctable, luma_actable);

            luma_dcpred = decoder->decode8x8(Y4Tile, luma_dcpred, luma_qindex, 
                                             luma_dctable, luma_actable);

            //
            // Decode the U (Cb) block.
            //
            cb_dcpred = decoder->decode8x8(CbTile, cb_dcpred, cb_qindex, 
                                           cb_dctable, cb_actable);

            //
            // Decode the V (Cr) block.
            //
            cr_dcpred = decoder->decode8x8(CrTile, cr_dcpred, cr_qindex, 
                                           cr_dctable, cr_actable);

            //
            // Check for end of restart interval, if active (non-zero).
            // If restart, reset DC predictors to zero and restart decoder.
            //
            if((restartInterval != 0) && (++MCUcount == restartInterval)) {
                // Zero out the DC coefficient predictors
                luma_dcpred = 0;
                cb_dcpred   = 0;
                cr_dcpred   = 0;
                MCUcount    = 0;
                decoder->doRestart();
            }

            //
            // Perform the combined upsample:color_convert 
            // from YCC601 to RGB709
            //
            if(di->doColorConvert) {
                colorCvt->cvtBlock(Y1Tile, CbTile, CrTile, 8, 8, 8,
                                   dst, di->image_ps, di->image_ss);
                colorCvt->cvtBlock(Y2Tile, CbTile+4, CrTile+4, 8, 8, 8,
                                   dst+8*di->image_ps, di->image_ps, di->image_ss);
                colorCvt->cvtBlock(Y3Tile, CbTile+32, CrTile+32, 8, 8, 8,
                                   dst+8*di->image_ss, di->image_ps, di->image_ss);
                colorCvt->cvtBlock(Y4Tile, CbTile+36, CrTile+36, 8, 8, 8,
                                   dst+8*(di->image_ss+di->image_ps), 
                                   di->image_ps, di->image_ss);
            } else if (di->doOrderedDither) {
                ditherCvt->dither411(Y1Tile, CbTile, CrTile, 8, 8, 8,
                                   dst, di->image_ps, di->image_ss);
                ditherCvt->dither411(Y2Tile, CbTile+4, CrTile+4, 8, 8, 8,
                                   dst+8*di->image_ps, di->image_ps, di->image_ss);
                ditherCvt->dither411(Y3Tile, CbTile+32, CrTile+32, 8, 8, 8,
                                   dst+8*di->image_ss, di->image_ps, di->image_ss);
                ditherCvt->dither411(Y4Tile, CbTile+36, CrTile+36, 8, 8, 8,
                                   dst+8*(di->image_ss+di->image_ps), 
                                   di->image_ps, di->image_ss);

            }

            dst += 16 * di->image_ps;

        } // End MCU column loop

        dst_yblock += 16 * di->image_ss;

    } // End MCU row loop

    decoder->finishDecode();

    return XIL_SUCCESS;
}

XilStatus 
XilDeviceCompressionJpeg::decompress422Frame(DecompressInfo* di)
{

    Ycc2RgbConverter* colorCvt = (Ycc2RgbConverter*)di->objectPtr1;

    // 
    // Get the header information from the JpegDecompressorData object
    //
    JpegDecompressorData* decoder = &decompData;
    Header& hdr = decoder->header;
    int restartInterval = hdr.restartInterval;
    
    // 
    // Initailze the Huffman decoder
    //
    decoder->huffmandecoder.initdecode();

    //
    // Storage for four 8x8 block of decoded coefficients (one MCU for 422).
    // These are 16 bit signed quantities, but the IDCT
    // routine uses double-word loads and stores, so
    // we need to insure 8 byte alignment. Declare as doubles to do this.
    //
    double alltiles[4 * 16];
    Xil_signed16* Y1Tile = (Xil_signed16*)(alltiles); 
    Xil_signed16* Y2Tile = (Xil_signed16*)(alltiles + 16); 
    Xil_signed16* CbTile = (Xil_signed16*)(alltiles + 2*16); 
    Xil_signed16* CrTile = (Xil_signed16*)(alltiles + 3*16); 

    //
    // Get the huffman table and quantization table indices.
    // Also start the DC predictors out at zero.
    //
    banddata* bptr = hdr.band_data;

    int* luma_actable = bptr[0].actable;
    int* luma_dctable = bptr[0].dctable;
    int  luma_qindex  = bptr[0].qindex;
    int  luma_dcpred  = 0;

    int* cb_actable   = bptr[1].actable;
    int* cb_dctable   = bptr[1].dctable;
    int  cb_qindex    = bptr[1].qindex;
    int  cb_dcpred    = 0;

    int* cr_actable   = bptr[2].actable;
    int* cr_dctable   = bptr[2].dctable;
    int  cr_qindex    = bptr[2].qindex;
    int  cr_dcpred    = 0;

    int MCUcount = 0;

    //
    // Process all of the 16x8 (output) pixel macroblocks
    //
    Xil_unsigned8* dst_yblock = (Xil_unsigned8*)di->image_dataptr;
    for(unsigned int vMB=0; vMB<(hdr.height)/8; vMB++) {

        Xil_unsigned8* dst = dst_yblock;
        for(unsigned int hMB=0; hMB<(hdr.width)/16; hMB++) {
            //
            // Decode the 2 Y (luma) blocks
            //
            luma_dcpred = decoder->decode8x8(Y1Tile, luma_dcpred, luma_qindex, 
                                             luma_dctable, luma_actable);

            luma_dcpred = decoder->decode8x8(Y2Tile, luma_dcpred, luma_qindex, 
                                             luma_dctable, luma_actable);

            //
            // Decode the U (Cb) block.
            //
            cb_dcpred = decoder->decode8x8(CbTile, cb_dcpred, cb_qindex, 
                                           cb_dctable, cb_actable);

            //
            // Decode the V (Cr) block.
            //
            cr_dcpred = decoder->decode8x8(CrTile, cr_dcpred, cr_qindex, 
                                           cr_dctable, cr_actable);

            //
            // Check for end of restart interval, if active (non-zero).
            // If restart, reset DC predictors to zero and restart decoder.
            //
            if((restartInterval != 0) && (++MCUcount == restartInterval)) {
                // Zero out the DC coefficient predictors
                luma_dcpred = 0;
                cb_dcpred   = 0;
                cr_dcpred   = 0;
                MCUcount    = 0;
                decoder->doRestart();
            }

            //
            // Perform the combined upsample:color_convert 
            // from YCC601 to RGB709
            //
            colorCvt->cvtBlock(Y1Tile, CbTile, CrTile, 8, 8, 8,
                               dst, di->image_ps, di->image_ss);
            colorCvt->cvtBlock(Y2Tile, CbTile+4, CrTile+4, 8, 8, 8,
                               dst+8*di->image_ps, di->image_ps, di->image_ss);
            dst += 16 * di->image_ps;

        } // End MCU column loop

        dst_yblock += 8 * di->image_ss;

    } // End MCU row loop

    decoder->finishDecode();

    return XIL_SUCCESS;
}



//
// Verify that this CIS meets the constraints for
// decompressColorConvert Molecules
//
Xil_boolean
XilDeviceCompressionJpeg::validDecompressColorConvert(XilImage* src,
                                                      XilImage* dst)
{
                                                      
    //
    // Must be 4:1:1 sampling, i.e 
    //   Luma = 2h, 2v (no subsamplng)
    //   Cb   = 1h, 1v (2X downsampled in both dimensions)
    //   Cr   = 1h, 1v (2X downsampled in both dimensions)
    //
    if(! (decompData.is411Frame() || decompData.is422Frame())) {
        return FALSE;
    }

    //
    // Color spaces must be Ycc601 src, Rgb709 dst
    //
    XilColorspace* src_cspace = src->refColorspace();
    XilColorspace* dst_cspace = dst->refColorspace();

    XilSystemState* state = getSystemState();
    XilColorspace*  desired_src_cspace = 
        (XilColorspace*)state->getObjectByName("ycc601", XIL_COLORSPACE);
    XilColorspace* desired_dst_cspace = 
        (XilColorspace*)state->getObjectByName("rgb709", XIL_COLORSPACE);

    //
    // Check that cspaces are those required.
    //
    if(src_cspace->getOpcode() != desired_src_cspace->getOpcode() ||
       dst_cspace->getOpcode() != desired_dst_cspace->getOpcode()) {
         return FALSE;
     }

     //
     // Both dimensions must be multiple of 16 for 4:1:1
     // Width must be multiple of 16 and height a multiple of 8 for 4:2:2
     //
     if(decompData.is411Frame()) {
         if( ((getOutputTypeHoldTheDerivation()->getWidth() & 0xf)  != 0) ||
             ((getOutputTypeHoldTheDerivation()->getHeight() & 0xf) != 0) ) {
             return FALSE;
         }
     } else { // 4:2:2
         if( ((getOutputTypeHoldTheDerivation()->getWidth() & 0xf)  != 0) ||
             ((getOutputTypeHoldTheDerivation()->getHeight() & 0x7) != 0) ) {
             return FALSE;
         }
     }

    return TRUE;
}
