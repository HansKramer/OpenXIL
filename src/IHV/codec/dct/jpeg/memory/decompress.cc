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
//  File:       decompress.cc
//  Project:    XIL
//  Revision:   1.24
//  Last Mod:   10:14:34, 03/10/00
//
//  Description:
//
//    Memory-to-memory JPEG decompression path
//
//
//------------------------------------------------------------------------
//    COPYRIGHT
//------------------------------------------------------------------------
#pragma ident   "@(#)decompress.cc	1.24\t00/03/10  "

#include <string.h>
#include <xil/xilGPI.hh>
#include "XilDeviceManagerCompressionJpeg.hh"
#include "XilDeviceCompressionJpeg.hh"
#include "ColorValue.hh"
#include "Idct.hh"
#include "XiliUtils.hh"

//
//  Band destination info that can be computed at the start of scan
//  processing.  There are a max of 4 bands in a scan
//
class BandInfo {
public:
    unsigned int  xrepeat;// times to replicate pixel in X for scaling
    unsigned int  yrepeat;// times to replicate pixel in Y for scaling
    int           dc_prediction;
    Xil_unsigned8 hsamp;
    Xil_unsigned8 vsamp;
    Xil_unsigned8 qindex;
    int*          dctable;
    int*          actable;
};

//
// The decompress driver function 
//
// Decompress a frame to memory.  Scale subsampled bands to produce
// a uniformly sampled image.
//
XilStatus
XilDeviceCompressionJpeg::decompress(XilOp*       op,
                                     unsigned int op_count,
                                     XilRoi*      roi,
                                     XilBoxList*  bl)
{
    DecompressInfo di(op, op_count, roi, bl);
    if(!di.isOK()) {
        XIL_ERROR(getSystemState(), XIL_ERROR_RESOURCE, "di-1", FALSE);
        return XIL_FAILURE;
    }

    setInMolecule(op_count > 1);

    //
    // Get the cis, destination image and the frame number
    // The dst image will always be on op_list[0].
    // The cis will alwyas be on op_list[op_count-1]
    //
    XilOp** op_list = op->getOpList();

    //
    // Seek to the proper frame. Deferred execution may have
    // caused other frames to be decompressed before this one.
    //
    seek(di.frame_number);

    JpegDecompressorData* decoder = getJpegDecompressorData();

    //
    // Read the bitstream header
    //
    if(!decoder->setByteStreamPtr()) {
        //
        // decompress: no frame from buffer mgr
        // Internal error.
        //
        XIL_ERROR(system_state, XIL_ERROR_SYSTEM,"di-95",FALSE);
        return XIL_FAILURE;
    }

    if(!decoder->parseByteStreamHeader()) {
        return XIL_FAILURE;
    }

    //
    // Test whether the cis image and the dst image are the same
    // size and that the dst image has no ROI. This allows simpler 
    // decompression, without the need for a temporary image.
    //
    if(cisFitsInDst(roi)) {
        //
        // We can decompress directly into the dst image.
        //
        if(decompressFrame(&di) != XIL_SUCCESS) {
            return XIL_FAILURE;
        }

    } else {
        //
        // We need to decompress into a temporary buffer first.
        // Use the specual DecompressInfo constructor to create one.
        //
        DecompressInfo tmp_di(&di);

        if(decompressFrame(&tmp_di) != XIL_SUCCESS) {
            return XIL_FAILURE;
        }

        //
        // Copy from the temporary buffer to the destination image,
        // dealing with all the rectangles in the box.
        //
        di.copyRects(&tmp_di);
    }

    if(decoder->notSeekable) {
        setRandomAccess(FALSE);
    }

    return XIL_SUCCESS; 
}

XilStatus
XilDeviceCompressionJpeg::decompressColorConvert(XilOp*       op,
                                                 unsigned int op_count,
                                                 XilRoi*      roi,
                                                 XilBoxList*  bl)
{
    DecompressInfo di(op, op_count, roi, bl);
    if(!di.isOK()) {
        XIL_ERROR(getSystemState(), XIL_ERROR_RESOURCE, "di-1", FALSE);
        return XIL_FAILURE;
    }

    setInMolecule(TRUE);

    //
    // Get the cis, destination image and the frame number
    // The dst image will always be on op_list[0].
    // The cis will alwyas be on op_list[op_count-1]
    //
    XilOp** op_list = op->getOpList();

    //
    // Seek to the proper frame. Deferred execution may have
    // caused other frames to be decompressed before this one.
    //
    seek(di.frame_number);

    JpegDecompressorData* decoder = getJpegDecompressorData();

    //
    // Read the bitstream header
    //
    if(!decoder->setByteStreamPtr()) {
        //
        // decompress: no frame from buffer mgr
        // Internal error.
        //
        XIL_ERROR(system_state, XIL_ERROR_SYSTEM,"di-95",FALSE);
        return XIL_FAILURE;
    }

    if(!decoder->parseByteStreamHeader()) {
        return XIL_FAILURE;
    }

    //
    // Verify that the parameters are acceptable. If not,
    // fail back to atomic mode.
    //
    if(! validDecompressColorConvert(op_list[0]->getSrcImage(),
                                     op_list[0]->getDstImage())) {
        return XIL_FAILURE;
    }
    di.doColorConvert = TRUE;

    //
    // Get the JpegYcc2Rgb object (creates it on first call)
    // This will initialize the tables which speed the color conversion.
    //
    XilDeviceManager* dmc = getDeviceManager();
    Ycc2RgbConverter* converter = ((XilDeviceManagerCompressionJpeg*)
                                  dmc)->getColorConverter();
    if(converter == NULL) {
        return XIL_FAILURE;
    }
    di.objectPtr1 = converter;

    //
    // Test whether the cis image and the dst image are the same
    // size and that the dst image has no ROI. This allows simpler 
    // decompression, without the need for a temporary image.
    //
    if(cisFitsInDst(roi)) {
        //
        // We can decompress directly into the dst image.
        //
        if(decompData.is411Frame()) {
            if(decompress411Frame(&di) != XIL_SUCCESS) {
                return XIL_FAILURE;
            }
        } else {
            if(decompress422Frame(&di) != XIL_SUCCESS) {
                return XIL_FAILURE;
            }
        }

    } else {
        //
        // We need to decompress into a temporary buffer first.
        // Use the specual DecompressInfo constructor to create one.
        //
        DecompressInfo tmp_di(&di);

        if(decompData.is411Frame()) {
            if(decompress411Frame(&tmp_di) != XIL_SUCCESS) {
                return XIL_FAILURE;
            }
        } else {
            if(decompress422Frame(&tmp_di) != XIL_SUCCESS) {
                return XIL_FAILURE;
            }
        }

        //
        // Copy from the temporary buffer to the destination image,
        // dealing with all the rectangles in the box.
        //
        di.copyRects(&tmp_di);
    }

    if(decoder->notSeekable) {
        setRandomAccess(FALSE);
    }

    return XIL_SUCCESS; 
}

XilStatus
XilDeviceCompressionJpeg::decompressOrderedDither(XilOp*       op,
                                                  unsigned int op_count,
                                                  XilRoi*      roi,
                                                  XilBoxList*  bl)
{
    float scale[3]  = {1.0F, 1.0F, 1.0F };
    float offset[3] = {0.0F, 0.0F, 0.0F };


    DecompressInfo di(op, op_count, roi, bl);
    if(!di.isOK()) {
        XIL_ERROR(getSystemState(), XIL_ERROR_RESOURCE, "di-1", FALSE);
        return XIL_FAILURE;
    }


    setInMolecule(TRUE);

    //
    // Get the op_list for determining molecule components.
    // The cis will always be on op_list[op_count-1].
    //
    XilOp**   op_list = op->getOpList();

    //
    // Seek to the proper frame. Deferred execution may have
    // caused other frames to be decompressed before this one.
    //
    seek(di.frame_number);

    JpegDecompressorData* decoder = getJpegDecompressorData();

    //
    // Read the bitstream header
    //
    if(!decoder->setByteStreamPtr()) {
        //
        // decompress: no frame from buffer mgr
        // Internal error.
        //
        XIL_ERROR(system_state, XIL_ERROR_SYSTEM,"di-95",FALSE);
        return XIL_FAILURE;
    }

    if(!decoder->parseByteStreamHeader()) {
        return XIL_FAILURE;
    }


    //
    // Verify that the parameters are acceptable. If not,
    // fail back to atomic mode.
    //
    XilLookupColorcube* cube;
    XilDitherMask*      dmask;
    op_list[0]->getParam(1, (XilObject**)&cube);
    op_list[0]->getParam(2, (XilObject**)&dmask);

    if(! validDecompressOrderedDither(cube, dmask)) {
        return XIL_FAILURE;
    }
    di.doOrderedDither = TRUE;

    //
    // Check if the op_count is 3. If it is, then this is the
    // decompressRescaleDither molecule.
    // So load the rescale params into the scale/offset arrays.
    //
    if(op_count == 3) {
        float*              xscale;
        float*              xoffset;
        op_list[1]->getParam(1, (void**)&xscale);
        op_list[1]->getParam(2, (void**)&xoffset);
        memcpy(scale, xscale, 3*sizeof(float));
        memcpy(offset, xoffset, 3*sizeof(float));
    }

    //
    // Get the XiliOrderedDitherLut object (creates it on first call)
    // This will initialize the tables which speed ordered dither
    //
    ditherTable = getDitherTable(cube, dmask, scale, offset);
    if(ditherTable == NULL) {
        return XIL_FAILURE;
    }
    di.objectPtr1 = ditherTable;
    
    //
    // Test whether the cis image and the dst image are the same
    // size and that the dst image has no ROI. This allows simpler 
    // decompression, without the need for a temporary image.
    //
    if(cisFitsInDst(roi)) {
        //
        // We can decompress directly into the dst image.
        //
        if(decompress411Frame(&di) != XIL_SUCCESS) {
            return XIL_FAILURE;
        }

    } else {
        //
        // We need to decompress into a temporary buffer first.
        // Use the specual DecompressInfo constructor to create one.
        //
        DecompressInfo tmp_di(&di);

        if(decompress411Frame(&tmp_di) != XIL_SUCCESS) {
            return XIL_FAILURE;
        }

        //
        // Copy from the temporary buffer to the destination image,
        // dealing with all the rectangles in the box.
        //
        di.copyRects(&tmp_di);
    }

    if(decoder->notSeekable) {
        setRandomAccess(FALSE);
    }

    return XIL_SUCCESS; 
}

XilStatus 
XilDeviceCompressionJpeg::decompressFrame(DecompressInfo* di)
{
    // 
    // Get the header information from the JpegDecompressorData object
    //
    JpegDecompressorData* decoder = &decompData;
    Header* hdr = &(decoder->header);
    
    //
    // Jpeg permits up to 4 bands in an interleaved scan
    //
    BandInfo  bandInfo[4]; 

    banddata* bptr;

    //
    // Storage for one 8x8 block of decoded coefficients.
    // These are 16 bit signed quantities, but the IDCT
    // routine uses double-word loads and stores, so
    // we need to insure 8 byte alignment.
    // Should be small enough to allocate on stack (128 bytes)
    //
    double dctTile[16];
    Xil_signed16* tile = (Xil_signed16*)dctTile;

    unsigned int width      = hdr->width;
    unsigned int height     = hdr->height;
    unsigned int bands_in_frame = hdr->bands;

    // 
    // TODO: lperry.  Bunch of code removed at this point
    //
    // Tests to see if an optimized path can be taken.
    // Conditions are:
    //     Single band
    //     PixelStride = 1
    //     Width and height are multiples of 8
    //     Scanline stride is a multiple of 4 bytes
    //     The band number is a multiple  of 4
    //     No RST markers.
    //
    //   All this guarantees that pixels are contiguous in
    //   memory and align on 32 bit boundaries
    //      
    // Note: This code has been removed for now in XIL 1.3
    //       It applies only to grayscale, and used a lot
    //       of trickery. 
    //       TODO: lperry. Try testing speed against 1.2 and see if
    //             its really justified.
    //

    unsigned int maxh = hdr->maxh;
    unsigned int maxv = hdr->maxv;
    unsigned int xinc = 8 * maxh;
    unsigned int yinc = 8 * maxv;

    unsigned int hMCU_max = (width + xinc-1) / xinc;
    unsigned int vMCU_max = (height + yinc-1) / yinc;

    //
    // The FRAME loop: Each frame can have a number of SCANS
    // Typically, there will be only a single scan with
    // multiple components (interleaved mode). But there can
    // be multiple scans, each with a single component 
    // (non-interleaved mode). There can also be a mix of
    // interleaved and non-interleaved scans in a single frame.
    //
    unsigned int frameband = 0;
    while(frameband < bands_in_frame) {
        decoder->huffmandecoder.initdecode();
        int restartInterval = hdr->restartInterval;
        unsigned int MCUcount = 0;
        unsigned int bands_in_scan = hdr->scanbands;

        //
        // Pre-calculate parameters for all components in this scan
        //
        for(int scan=0; scan<bands_in_scan; scan++) {
            unsigned int component = hdr->scancomponents[scan];

            bptr = &hdr->band_data[component];
            bandInfo[scan].xrepeat = maxh / (int) bptr->h;
            bandInfo[scan].yrepeat = maxv / (int) bptr->v;
            bandInfo[scan].dc_prediction = 0;
            bandInfo[scan].hsamp = bptr->h;
            bandInfo[scan].vsamp = bptr->v;

            //
            // Test for non-integral sampling ratios in either axis.
            // We going to reject these Jpeg files. Though legal
            // Jpeg, they are extremely rare and probably not
            // intended by the Jpeg specification. Was unable
            // to find any way to produce such a file. So we
            // wouldn't be able to test the code anyway.
            //
            if( ((maxh % bptr->h) != 0) ||
                ((maxv % bptr->v) != 0) ) {
                XIL_ERROR(getSystemState(), XIL_ERROR_USER, "di-450", TRUE);
                return XIL_FAILURE;
            }

            bandInfo[scan].qindex = bptr->qindex;
            bandInfo[scan].dctable = bptr->dctable;
            bandInfo[scan].actable = bptr->actable;

        } // End scanloop for params

        unsigned int dstMCURowInc   = di->image_ss * maxv * 8;
        unsigned int dstMCUColInc   = di->image_ps * maxh * 8;

        //
        // Process MCU rows
        //
        Xil_unsigned8* dstMCURow = (Xil_unsigned8*)di->image_dataptr + 
                                   frameband;
        for(unsigned int vMCU=0; vMCU<vMCU_max; vMCU++) {

            //
            // Process MCU columns
            //
            Xil_unsigned8* dstMCUCol = dstMCURow;
            for(unsigned int hMCU = 0; hMCU < hMCU_max; hMCU++) {

                //
                // Process the DCT blocks for each component of the scan
                // (1 if non-interleaved, nbands if interleaved)
                //
                for(scan=0; scan<bands_in_scan; scan++) {

                    //
                    // Parameters for this scan only
                    //
                    int  xmag    = bandInfo[scan].xrepeat;
                    int  ymag    = bandInfo[scan].yrepeat;
                    int  qindex  = bandInfo[scan].qindex;
                    int  hsamp   = bandInfo[scan].hsamp;
                    int  vsamp   = bandInfo[scan].vsamp;
                    int* dctable = bandInfo[scan].dctable;
                    int* actable = bandInfo[scan].actable;

                    //
                    // Upsampled dst block size for this component
                    //
                    int xblksiz = xmag*8;
                    int yblksiz = ymag*8;

                    unsigned int dstBlkRowInc = yblksiz * di->image_ss;
                    unsigned int dstBlkColInc = xblksiz * di->image_ps;

                    //
                    // Process DCT block rows for this component
                    //
                    Xil_unsigned8* dstBlkRow = dstMCUCol + scan;
                    for(int vblk=0; vblk<(int)vsamp; vblk++) { 

                        Xil_unsigned8* dstBlkCol = dstBlkRow;

                        int dstY = vMCU*yinc + vblk*yblksiz;
                        int nscanlines = yblksiz;
                        if(dstY + yblksiz > height) {
                            int extra = height - dstY;
                            nscanlines = (extra > 0) ? extra : 0;
                        }

                        //
                        // Process DCT block columns for this component
                        //
                        for(int hblk=0; hblk<(int)hsamp; hblk++) { 

                            int dstX = hMCU*xinc + hblk*xblksiz;
                            int npix = xblksiz;
                            if(dstX + xblksiz > width) {
                                int extra = width - dstX;
                                npix = (extra > 0) ? extra : 0;
                            }

                            // 
                            // Decode the next 8x8 block of pixels,
                            // saving the DC predictor value for use
                            // in the next block of this component
                            //
                            bandInfo[scan].dc_prediction = 
                                decoder->decode8x8(tile, 
                                                   bandInfo[scan].dc_prediction,
                                                   qindex,
                                                   dctable,
                                                   actable);

                            // 
                            // Upsample 8x8 block, scaling up the bands
                            // which were subsampled before compression.
                            // (Also handles no upsampling case).
                            // TODO: (lperry) Test the performance on the
                            //       no-upsampling case, since this had used
                            //       some groddy special case code in 1.2
                            //
                            if(npix>0 || nscanlines>0) {
                                upsampleBlock(tile, dstBlkCol, 
                                              npix, nscanlines,
                                              xmag, ymag, 
                                              di->image_ps, di->image_ss);
                            }

                            dstBlkCol += dstBlkColInc;

                        } // End DCT block column loop within MCU component

                        dstBlkRow += dstBlkRowInc;

                    } // End DCT block row loop within MCU component

                } // End component loop within MCU

                //
                // Check for end of restart interval
                //
                if(restartInterval && (++MCUcount == restartInterval)) {
                    //
                    // Reset DC predictors to zero
                    //
                    for(scan=0; scan<bands_in_scan; scan++) {
                        bandInfo[scan].dc_prediction = 0;
                    }
                    MCUcount = 0;
                    decoder->doRestart();
                }

                dstMCUCol += dstMCUColInc;

            } // End MCU column loop

            dstMCURow += dstMCURowInc;

        } // End MCU row loop

        //
        // Increment count of components (bands) seen so far.
        // Note that there can be a mix of interleaved and 
        // non-interleaved scans, so we need to count them all.
        //
        frameband += bands_in_scan;

        //
        // Parse to the next scan
        //
        if(frameband < bands_in_frame) {
            if(!decoder->readtoscan()) {
                return XIL_FAILURE;
            }
        }

    } // End scan loop

    decoder->finishDecode();

    return XIL_SUCCESS;
}

//
// Utility function to handle DCT block upsampling.
// Also handles the case of no upsampling.
// Special cases 2X upsampling in both axes for 411.
//
void
XilDeviceCompressionJpeg::upsampleBlock(Xil_signed16*  src,
                                        Xil_unsigned8* dst,
                                        unsigned int nsamps,
                                        unsigned int nlines,
                                        unsigned int xmag,
                                        unsigned int ymag,
                                        unsigned int dst_ps,
                                        unsigned int dst_ss)
{
    // 
    // Symbols for elements of 2x2 block (upper-right, etc)
    //
    const int UR = dst_ps;
    const int LL = dst_ss;
    const int LR = dst_ss + dst_ps;

    //
    // Process (at most) 8 scanlines in Y
    //
    Xil_unsigned8 pixel_value;
    Xil_boolean   unityMagnification = (xmag == 1 && ymag == 1); 
    Xil_boolean   twoX               = (xmag == 2 && ymag == 2);

    Xil_signed16*  src_scan = src;
    Xil_unsigned8* dst_scan = dst;
    if(unityMagnification) {

        for(int yline=nlines; yline!=0; yline--) {
            Xil_signed16*  src_pixel = src_scan;
            Xil_unsigned8* dst_pixel = dst_scan;
            for(int xpixel=nsamps; xpixel!=0; xpixel--) {
                int decom_value = *src_pixel++ + 128;
                if((decom_value & (~0xFF)) != 0) {
                    if(decom_value < 0) {
                        *dst_pixel = 0;
                    } else {
                        *dst_pixel = 255;
                    }
                } else {
                    *dst_pixel = (Xil_unsigned8)(decom_value & 0xFF);
                }

                dst_pixel += dst_ps;
            }

            src_scan += 8;
            dst_scan += dst_ss;
        }

    } else if (twoX) {

        unsigned int xstep = dst_ps * xmag;
        unsigned int ystep = dst_ss * ymag;

        //
        // A full block. Write it directly to the dst.
        //
        if(nsamps==16 && nlines==16) {

            Xil_signed16*  src_pixel = src;
            for(int yline=8; yline!=0; yline--) {
                Xil_unsigned8* dst_pixel = dst_scan;
                for(int xpixel=8; xpixel!=0; xpixel--) {
                    int decom_value = *src_pixel++ + 128;
                    if((decom_value & (~0xFF)) != 0) {
                        if(decom_value < 0) {
                            pixel_value = 0;
                        } else {
                            pixel_value = 255;
                        }
                    } else {
                        pixel_value = (Xil_unsigned8)(decom_value & 0xFF);
                    }
                    dst_pixel[0]  = pixel_value;
                    dst_pixel[UR] = pixel_value;
                    dst_pixel[LL] = pixel_value;
                    dst_pixel[LR] = pixel_value;

                    dst_pixel += xstep;
                }
                dst_scan += ystep;
            }

        } else {
            //
            // A partial block.
            // Upsample to a temporary and then copy the desired area.
            //
            Xil_unsigned8 tmp[16*16];
            dst_scan = tmp;
            Xil_signed16*  src_pixel = src;
            for(int yline=8; yline!=0; yline--) {
                Xil_unsigned8* dst_pixel = dst_scan;
                for(int xpixel=8; xpixel!=0; xpixel--) {
                    int decom_value = *src_pixel++ + 128;
                    if((decom_value & (~0xFF)) != 0) {
                        if(decom_value < 0) {
                            pixel_value = 0;
                        } else {
                            pixel_value = 255;
                        }
                    } else {
                        pixel_value = (Xil_unsigned8)(decom_value & 0xFF);
                    }
                    dst_pixel[0]  = pixel_value;
                    dst_pixel[1]  = pixel_value;
                    dst_pixel[16] = pixel_value;
                    dst_pixel[17] = pixel_value;

                    dst_pixel += 2;
                }
                dst_scan += 32;
            }

            //
            // Copy the required section to the destination
            //
            Xil_unsigned8* tmp_scan = tmp;
            dst_scan = dst;
            for(yline=nlines; yline!=0; yline--) {
                Xil_unsigned8* tmp_pixel = tmp_scan;
                Xil_unsigned8* dst_pixel = dst_scan;
                for(int xpixel=nsamps; xpixel!=0; xpixel--) {
                    *dst_pixel = *tmp_pixel++;
                    dst_pixel += dst_ps;
                }

                tmp_scan += 16;
                dst_scan += dst_ss;
            }
        } // end 2X partial block case

    } else {
        //
        // Replicate the pixel
        // according to the xmag, ymag factors
        // TODO: lperry
        //   Need to add code to handle non-integral
        //   magnification factors.
        //
        if(nsamps==(8*xmag) && nlines==(8*ymag)) {
            unsigned int xstep = dst_ps * xmag;
            unsigned int ystep = dst_ss * ymag;

            //
            // A full block. Write it directly to the dst.
            //
            Xil_signed16*  src_pixel = src;
            for(int yline=8; yline!=0; yline--) {
                Xil_unsigned8* dst_pixel = dst_scan;
                for(int xpixel=8; xpixel!=0; xpixel--) {
                    int decom_value = *src_pixel++ + 128;
                    if((decom_value & (~0xFF)) != 0) {
                        if(decom_value < 0) {
                            pixel_value = 0;
                        } else {
                            pixel_value = 255;
                        }
                    } else {
                        pixel_value = (Xil_unsigned8)(decom_value & 0xFF);
                    }

                    for(int y=0; y<ymag; y++) {
                        for(int x=0; x<xmag; x++) {
                            dst_pixel[y*dst_ss+x*dst_ps] = pixel_value;
                        }
                    }

                    dst_pixel += xstep;
                }
                dst_scan += ystep;
            }
        } else {
            //
            // A partial block.
            // Upsample to a temporary.
            //
            Xil_unsigned8 tmp[64*64];
            dst_scan = tmp;
            Xil_signed16*  src_pixel = src;
            for(int yline=8; yline!=0; yline--) {
                Xil_unsigned8* dst_pixel = dst_scan;
                for(int xpixel=8; xpixel!=0; xpixel--) {
                    int decom_value = *src_pixel++ + 128;
                    if((decom_value & (~0xFF)) != 0) {
                        if(decom_value < 0) {
                            pixel_value = 0;
                        } else {
                            pixel_value = 255;
                        }
                    } else {
                        pixel_value = (Xil_unsigned8)(decom_value & 0xFF);
                    }
                    for(int y=0; y<ymag; y++) {
                        for(int x=0; x<xmag; x++) {
                            dst_pixel[y*64+x] = pixel_value;
                        }
                    }

                    dst_pixel += xmag;
                }
                dst_scan += ymag * 64;
            }

            //
            // Copy the required section to the destination
            //
            Xil_unsigned8* tmp_scan = tmp;
            dst_scan = dst;
            for(yline=nlines; yline!=0; yline--) {
                Xil_unsigned8* tmp_pixel = tmp_scan;
                Xil_unsigned8* dst_pixel = dst_scan;
                for(int xpixel=nsamps; xpixel!=0; xpixel--) {
                    *dst_pixel = *tmp_pixel++;
                    dst_pixel += dst_ps;
                }

                tmp_scan += 64;
                dst_scan += dst_ss;
            }

        }

    }

}

//
// Verify that the constraints for decompressOrderedDither
// are satisfied (4x4 dither mask, 855 colorcube)
//
Xil_boolean
XilDeviceCompressionJpeg::validDecompressOrderedDither(
          XilLookupColorcube* cube,
          XilDitherMask*      dmask)
{
    //
    // Dithermask must be 4x4 3-band, but contents don't matter
    //
    if(dmask->getWidth()    != 4 ||
       dmask->getHeight()   != 4 ||
       dmask->getNumBands() != 3) {
        return FALSE;
    }

    //
    // Colorcube must be the 855 variety, suitable for yuv2rgb conversion
    //

    const int*          mults = cube->getMultipliers();
    const unsigned int* dims  = cube->getDimensions();
    if(dims[0] != 8 || dims[1] != 5 || dims[2] != 5 ||
       mults[0] != 1 || mults[1] != 8 || mults[2] != 40) {
        return FALSE;
    }

    //
    // Both dimensions must be multiple of 16
    //
    if( ((getOutputTypeHoldTheDerivation()->getWidth() & 0xf)  != 0) ||
          ((getOutputTypeHoldTheDerivation()->getHeight() & 0xf) != 0) ) {
        return FALSE;
    }

    return TRUE;
}

