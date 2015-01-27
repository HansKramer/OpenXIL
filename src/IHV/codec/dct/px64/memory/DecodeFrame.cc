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
//  File:       DecodeFrame.cc
//  Project:    XIL
//  Revision:   1.9
//  Last Mod:   10:15:13, 03/10/00
//
//  Description:
//
//    Implementation of the decoding loop for H.261 video.
//
//    This function also provides the entry points for 
//    performing color conversion and dithering for molecules.
//    All decompression and color conversion/dithering is done to an 
//    internal buffer. Because H.261 does not transmit some blocks
//    in "inter" frames, when there is negligible change, the 
//    dither process does not need to be re-applied for those blocks.
//    Instead the internal copy of the same block is used from the previous
//    frame. This can be a significant win for the common type of
//    "talking-head" scene in a video conference where many blocks seldom
//    change.
//
//    When the cis is forward-seeked, the logic, needs to continue to
//    update the internal frame buffers, but not the dither buffers.
//    This writing to the internal buffers is done even if there is no
//    color conversion or dithering requested. This will incur the 
//    penalty of an extra copy in this case, but that will be a very
//    small percentage of the total work performed here. It is also
//    necessary anyway to keep the current state up to date.
//
//------------------------------------------------------------------------
//    COPYRIGHT
//------------------------------------------------------------------------
#pragma ident   "@(#)DecodeFrame.cc	1.9\t00/03/10  "



#include <xil/xilGPI.hh>
#include "H261DecompressorData.hh"
#include "XilDeviceCompressionH261.hh"
#include "MacroBlockBlt.hh"
#include "Blocks.hh"

#define H261_GOB_WIDTH 11*16
#define H261_GOB_HEIGHT 3*16


void 
H261DecompressorData::color_video_frame(DecompressInfo* di)
{

    int               mvx, mvy;
    int               gn;


    Xil_unsigned8*    dest;
    int               xblkstride;
    int               yblkstride;
    int               xmbstride;
    int               ymbstride;
    Xil_unsigned8*    pdest;
    Xil_boolean       skip_blocks;

    //
    // If a NULL is passed in for the decompressInfo object,
    // then we do not require any updating of the dithered
    // or color-converted buffers.
    //
    Xil_boolean update_output = (di != NULL);

    if(update_output) {
        if(di->doOrderedDither) {
            //
            // One band output to CIF internal buffer
            //
            dest         = buf8;
            xblkstride   = 8;
            yblkstride   = 8 * 352;
        } else {
            //
            // Three band output to CIF internal buffer
            // This applies to both color converted and Ycc output
            //
            dest         = buf24;
            xblkstride   = 8 * 3;
            yblkstride   = 8 * 3 * 352;
        }

        xmbstride    = xblkstride * 2;
        ymbstride    = yblkstride * 2;
    }

    // 
    // Swap object pointers for current and previous images
    // 
    swapCurrPrevBlockPtrs();  

    // 
    // Look for Picture Start Code (PSC).
    // sync up with the first "1" followed by 4 zeros,
    // Some of the leading "0" PSC bits may be part of the previous frame!
    // Then return unused bits to savedBits
    // 
    if(syncPSC() == XIL_FAILURE) {
        XIL_ERROR(di->system_state, XIL_ERROR_CIS_DATA,"di-314", TRUE);
        return;
    }

    // 
    // Get temporal reference (5 bits) and PTYPE (6 bits)
    // 
    int tempword = getbits(11);
    Xil_boolean cif = ((tempword & 0x04) != 0);
    if(update_output) {
        skip_blocks = validDitherState;
    } else {
        skip_blocks = TRUE;
    }
    if(tempword == BAD_BIT_VALUE) {
        XIL_ERROR(di->system_state, XIL_ERROR_USER,"di-312", TRUE);
        return;
    }

    int maxgob;
    int width, height;
    if(cif) {
        //
        // CIF resolution
        //
        width  = 352;
        height = 288;
        maxgob = 12;

        if(getCisSize() != CIS_CIF_SIZE) {
            if(getCisSize() == -1) {
                setCisSize(CIS_CIF_SIZE);
            } else {
                XIL_ERROR(di->system_state, XIL_ERROR_USER,"di-312", TRUE);
                return;
            }
        }
    } else {
        //
        // QCIF resolution
        //
        width  = 176;
        height = 144;
        maxgob = 5;

        if(getCisSize() != CIS_QCIF_SIZE) {
            if(getCisSize() == -1) {
                setCisSize(CIS_QCIF_SIZE);
            } else {
                XIL_ERROR(di->system_state, XIL_ERROR_USER,"di-312", TRUE);
                return;
            }
        }
    }

    //
    // Relative offsets of the four Y blocks
    //
    int ur_off = 8;
    int ll_off = 8 * width;
    int lr_off = ll_off + 8;

    //
    // Discard extra insertion information
    // Get the single PEI bit indicating PSPARE info follows
    //
    GETBITS(1, tempword);

    //
    // Get PSPARE bits
    //
    while(tempword == 1) {
        GETBITS(9, tempword);
        tempword &= 1;
    }

    // 
    // We know that there will be at least 20 bits here.
    // Get GOB Start Code (GBSC)  (16 bits)
    // Get GOB number             (4 bits)
    // However, some encoders put a PSC at the end of a bitstream, so we will
    // accept this bad bitstream and just finish parsing
    // 
    tempword = getbits(20);
    if( (tempword == BAD_BIT_VALUE) || 
        ((tempword>>4) != 0x0001) ) {
        goto EndOfFrame;
    }

    //
    // Get GOB number (4 bits)
    //
    gn = tempword & 0xf;

    //
    // Process each GOB
    //
    while(gn <= maxgob)
    {
        // 
        // We know that there will be at least 6 bits here.
        // Get GOB quantizer        (5)
        // Get Extra Insertion bit(1)
        // 
        if((tempword = getbits(6)) == BAD_BIT_VALUE) {
            XIL_ERROR(di->system_state, XIL_ERROR_USER,"di-312", TRUE);
            return;
        }

        // 
        // Set GOB quantizer
        // 
        int quant = (tempword>>1) & 0x1f;

        // 
        // Calculate xgob, ygob: image coords of the current GOB
        //
        // GOB numbering system for H.261
        // Each GOB contains 33 macroblocks 
        // (11 wide x 3 high, i.e. 176x48 pixels)
        //
        //     CIF            QCIF                              
        //   -------         -----
        //   | 1| 2|         | 1 |                              
        //   -------         -----
        //   | 3| 4|         | 3 |                              
        //   -------         -----
        //   | 5| 6|         | 5 |                              
        //   -------         -----
        //   | 7| 8|                                            
        //   -------
        //   | 9|10|                                            
        //   -------
        //   |11|12|                                            
        //   -------
        //
        int xgob, ygob;
        int xgobno, ygobno;
        if(cif) {
            if(gn > 12) {
                XIL_ERROR(di->system_state, XIL_ERROR_USER,"di-312", TRUE);
                return;
            }
            xgobno = (gn-1)&0x01;
            ygobno = (gn-1) >> 1;
        } else {
            if(gn > 5) {
                XIL_ERROR(di->system_state, XIL_ERROR_USER,"di-312", TRUE);
                return;
            }
            xgobno = 0;
            ygobno = gn>>1;
        }
        xgob = xgobno * H261_GOB_WIDTH;
        ygob = ygobno * H261_GOB_HEIGHT;

        //
        // Get rid of Extra insertion information
        // If GEI is set, we know that 9 bits will follow
        //
        while(tempword & 1) {
            GETBITS(9, tempword);      
        }

        int mb_address = 0;
        int mvf1 = 1;
        int mvf2 = 1;
        int mvf3 = 1;

        //
        // Macroblock Layer
        //
        do
        {
            int mba;
            int mb_type;
            int mb_cbp;

            // 
            // There might not be any bits left in this picture.
            // There can be 0 or more macroblock address stuffing codes here.
            // If we run out of bits during the search for MBA_STUFFING,
            // then we have reached the end of the frame and are skipping the
            // last macroblocks -- so we need to catchup.
            // There can be a GBSC or PSC here
            // 
            do {
                mba = decode(useMBATable());
            } while (mba == MBA_STUFFING);

            if(mba == MBA_START_CODE || mba == EOB_CODE) {
                // 
                // Catchup all skipped macroblocks. Here we use
                // the decoded information from the previous frame.
                // 
                catchup(di, skip_blocks, mb_address+1, 34, 
                        ygob, xgob, width);
                if(mba == EOB_CODE) {
                    goto EndOfFrame;
                }

                // 
                // If the next 4 bits are 0, then we have found a PSC and we
                // have read into the next frame.  Otherwise, we have found
                // a GBSC.  Either way, we have prematurely ended the 
                // macroblock loop, so jump to end of GOB loop to process
                // 
                if((tempword = getbits(4)) == BAD_BIT_VALUE) {
                    XIL_ERROR(di->system_state, XIL_ERROR_USER,"di-312", TRUE);
                    return;
                }
                goto HandleStartCode;
            } // if bad bitstream, just continue

            //
            // Save the expected mb_address (the very next one) and
            // then get the actual one using the transmitted increment.
            //
            int mb_expected  = mb_address + 1;
            mb_address += mba;
            if((mb_address > 33) || mb_address < 1) {
                XIL_ERROR(di->system_state, XIL_ERROR_USER,"di-312", TRUE);
                return;
            }

            // 
            // If its not the expected macroblock, then we skipped one or 
            // more macroblocks. So call 'catchup' to update current blocks 
            // and dither dest, using the block data from the previous frame.
            // 
            if(mb_address != mb_expected) {
                catchup(di, skip_blocks, mb_expected, mb_address, 
                        ygob, xgob, width);

                mvx = 0;
                mvy = 0;
            }

            // 
            // blockx and block y are block offsets of the current macroblock
            // from the start of the current gob  (# blocks into the gob)
            // 
            int blockx = (mb_address-1) % 11;
            int blocky = (mb_address-1) / 11;

            // 
            // base*ad is the pixel offset of the current macroblock from
            // the start of the image.
            // This would be equivalent to the byte offset of an image if
            // the image stride were == width.
            // 
            int originy = (ygob + (blocky<<4)) * width;
            int originx = xgob + (blockx<<4);
            int ymbaddr = originy + originx;
            int uvmbaddr = (originy >> 2) + (originx >> 1);

            if(update_output) {
                pdest = dest + (((ygob>>4) + blocky) * (ymbstride)) + 
                               ((blockx + (xgob>>4)) * (xmbstride));
            }

            if(mb_address <= 11) {
                // First macroblock row 
                if(mvf1) {
                    mvx = mvy = mvf1 = 0;
                }
            } else if(mb_address <= 22) {
                // Second macroblock row
                if(mvf2) {
                    mvx = mvy = mvf2 = 0;
                }
            } else {
                // Third macroblock row
                if(mvf3) {
                    mvx = mvy = mvf3 = 0;
                }
            }

            // 
            // We know there is a type field
            // 
            mb_type = decode(useMTYTable());


            if(mb_type & MTYPE_MQUANT) {
                GETBITS(5, quant);
            }

            if(mb_type & MTYPE_MC) {
                //  
                // table includes subtraction of 16
                // 
                short tempa = decode(useMVDTable());
                short tempb = decode(useMVDTable());

                // 
                // The MVD VLC encodes a pair of numbers which differ by
                // 32.  Only one of the pair can produce a legal value
                // (which is +-15).  If the motion vector is illegal, we must
                // have guessed wrong, so use the other delta value instead
                // 
                mvx += tempa;
                mvy += tempb;

                if(mvx < -16) {
                    mvx += 32;
                } else if(mvx >  16) {
                    mvx -= 32;
                }

                if(mvy < -16) {
                    mvy += 32;
                } else if(mvy >  16) {
                    mvy -= 32;
                }

            } else {
                mvx = 0;
                mvy = 0;
            }

            if(mb_type & MTYPE_CBP) {
                mb_cbp = decode(useCBPTable());
            } else {
                mb_cbp = 0x3f;        // Set cbp to all 1's */
            }

            // 
            // This section will set the (local) blold array.
            // For Inter blocks, the value is the predictor + difference,
            // For Intra blocks, the value is the decoded coefficients for 
            // each block
            // In all cases, the value is rounded and clamped.
            // 
            if(mb_type & MTYPE_INTER) {
                int block_control;
                // 
                // Intra macroblocks never have a CBP
                // Inter blocks which have no CBP have no coefficients
                // 
                if(mb_type & MTYPE_CBP) {
                    doInter(mb_cbp, blok.blold, quant);
                    block_control = mb_cbp;
                } else {
                    block_control = 0;
                }

                //
                // Check for out of bounds mvd checking and
                // get the predictors for all 6 blocks 
                //
                if(ygob == 0) {
                    if(blocky == 0) {
                        if(mvy < 0) {
                            XIL_ERROR(di->system_state, 
                                      XIL_ERROR_USER,"di-312", TRUE);
                            return;
                        }
                        if(blockx == 0 && xgob == 0) {
                            if(mvx < 0) {
                                XIL_ERROR(di->system_state, 
                                          XIL_ERROR_USER,"di-312", TRUE);
                                return;
                            }
                        }
                    }
                } else if(ygob == (height - 48)) {
                    if(blocky == 2) {
                        if(mvy > 0) {
                            XIL_ERROR(di->system_state, 
                                      XIL_ERROR_USER,"di-312", TRUE);
                            return;
                        }
                        if(blockx == 10 && 
                           ((cif && xgob) || (!cif && !xgob))) {
                            if(mvx > 0) {
                                XIL_ERROR(di->system_state, 
                                          XIL_ERROR_USER,"di-312", TRUE);
                                return;
                            }
                        }
                    }
                }

                if(mb_type & MTYPE_FIL) {
                    getfilterblocks(ypptr, upptr, vpptr, blok.blold,
                        blok.blnew, ymbaddr, uvmbaddr,
                        mvy, mvx, width, block_control);

                    //
                    // Set the CBP to all ones. Since we filtered the
                    // data, we need to update all blocks
                    //
                    mb_cbp = 0x3f;
                } else {
                    getblocks(ypptr, upptr, vpptr, blok.blold, blok.blnew, 
                              ymbaddr, uvmbaddr, mvy, mvx,
                              width, block_control);
                }
            } else {
                // 
                // Intra macroblock
                // 
                doIntra(blok.blold, quant);
                for(int blk=0; blk<6; blk++) {
                    blok.clampBlockToRaster(blok.blold[blk], 
                                            blok.blnew[blk], 8U);
                }
            }

            //
            // Update the history raster with the new macroblock
            // This copies the 4 luma and 2 chroma blocks.
            //
            Xil_unsigned8* ybase = ycptr + ymbaddr;
            blok.copyBlockToRaster(blok.blnew[0], ybase, width);
            blok.copyBlockToRaster(blok.blnew[1], ybase+ur_off, width);
            blok.copyBlockToRaster(blok.blnew[2], ybase+ll_off, width);
            blok.copyBlockToRaster(blok.blnew[3], ybase+lr_off, width);
            blok.copyBlockToRaster(blok.blnew[4], ucptr+uvmbaddr, width/2);
            blok.copyBlockToRaster(blok.blnew[5], vcptr+uvmbaddr, width/2);

            //
            // If dithering or color_conversion is required, process only
            // the blocks indicated in the Coded Block Pattern (CBP).
            //
            if(update_output) {
                if((mb_type & MTYPE_MC) || !skip_blocks) {
                    mb_cbp = 0x3f;   // Set cbp to all 1's */
                }

                if(di->doColorConvert) {
                    //
                    // Get the color converter object
                    //
                    Ycc2RgbConverter* converter = 
                        (Ycc2RgbConverter*)di->objectPtr1;

                    //
                    // Do upper left block
                    //
                    if(mb_cbp & 0x23) {
                        converter->cvtBlock(blok.blnew[0], blok.blnew[4], blok.blnew[5],
                                  8, 8, 8, pdest, 3, 352*3);
                    }
                    //
                    // Do upper right block
                    //
                    if(mb_cbp & 0x13) {
                        converter->cvtBlock(blok.blnew[1], blok.blnew[4]+4, blok.blnew[5]+4,
                                  8, 8, 8, pdest+xblkstride, 3, 352*3);
                    }
                    pdest += yblkstride;

                    //
                    // Do lower left block
                    //
                    if(mb_cbp & 0x0b) {
                        converter->cvtBlock(blok.blnew[2], blok.blnew[4]+32, blok.blnew[5]+32,
                                  8, 8, 8, pdest, 3, 352*3);
                    }
                    //
                    // Do lower right block
                    //
                    if(mb_cbp & 0x07) {
                        converter->cvtBlock(blok.blnew[3], blok.blnew[4]+36, blok.blnew[5]+36,
                                  8, 8, 8, pdest+xblkstride, 3, 352*3);
                    }

                } else if(di->doOrderedDither) {
                    //
                    // Get the ordered dither object
                    //
                    XiliOrderedDitherLut* ditherTable = 
                        (XiliOrderedDitherLut*)di->objectPtr1;

                    //
                    // Do upper left block
                    //
                    if(mb_cbp & 0x23) {
                        ditherTable->dither411(blok.blnew[0], 
                                               blok.blnew[4], 
                                               blok.blnew[5],
                                               8, 8, 8, pdest, 3, 352);
                    }
                    //
                    // Do upper right block
                    //
                    if(mb_cbp & 0x13) {
                        ditherTable->dither411(blok.blnew[1], 
                                               blok.blnew[4]+4, 
                                               blok.blnew[5]+4,
                                               8, 8, 8, pdest+xblkstride, 3, 352);
                    }
                    pdest += yblkstride;

                    //
                    // Do lower left block
                    //
                    if(mb_cbp & 0x0b) {
                        ditherTable->dither411(blok.blnew[2], 
                                               blok.blnew[4]+32, 
                                               blok.blnew[5]+32,
                                               8, 8, 8, pdest, 3, 352);
                    }
                    //
                    // Do lower right block
                    //
                    if(mb_cbp & 0x07) {
                        ditherTable->dither411(blok.blnew[3], 
                                               blok.blnew[4]+36, 
                                               blok.blnew[5]+36,
                                               8, 8, 8, pdest+xblkstride, 3, 352);
                    }

                } else {

                    //
                    // This is the atomic decompress case.
                    // Do a straight upsample to the dst image.
                    //

                    //
                    // Do upper left block
                    //
                    if(mb_cbp & 0x23) {
                        blok.upsample411Block8FromRaster(blok.blnew[0], 
                                                       blok.blnew[4], 
                                                       blok.blnew[5],
                                                       8, 8, 8, pdest, 3, 352*3);
                    }
                    //
                    // Do upper right block
                    //
                    if(mb_cbp & 0x13) {
                        blok.upsample411Block8FromRaster(blok.blnew[1], 
                                                       blok.blnew[4]+4, 
                                                       blok.blnew[5]+4,
                                                       8, 8, 8, pdest+xblkstride, 3, 352*3);
                    }
                    pdest += yblkstride;

                    //
                    // Do lower left block
                    //
                    if(mb_cbp & 0x0b) {
                        blok.upsample411Block8FromRaster(blok.blnew[2], 
                                                   blok.blnew[4]+32, 
                                                   blok.blnew[5]+32,
                                                   8, 8, 8, pdest, 3, 352*3);
                    }
                    //
                    // Do lower right block
                    //
                    if(mb_cbp & 0x07) {
                        blok.upsample411Block8FromRaster(blok.blnew[3], 
                                                   blok.blnew[4]+36, 
                                                   blok.blnew[5]+36,
                                                   8, 8, 8, pdest+xblkstride, 3, 352*3);
                    }
                }
            }

        } while(mb_address < 33);

        // 
        // Get rid of macroblock stuffing before next GBSC or PSC
        // There are 0 or more stuffing words followed by:
        // GBSC, PSC or end-of-buffer
        // 
        do {
            tempword = getbits(11);
        } while (tempword == 0x0f);


        //
        // End of file, presumably
        //
        if(tempword != 0) {
            break;
        }

        //
        // Test for start code. If not found, assume EOF.
        //
        tempword = getbits(9);
        if((tempword >> 4) != 1) {
            break;
        }
        tempword &= 0xf;

HandleStartCode:
        // 
        // tempword is either 0 (PSC) or is the gob number for a GOB header
        // 
        if(tempword == 0) {                // PSC
            adjustRdptr();
            break;
        } else {
            gn = tempword;
        }

    } // END GOB LAYER

EndOfFrame:
    //
    // Create another DecompressInfo object with the storage
    // description of the internal image buffer. 
    // then call copyRects to move it to the dst image.
    // Since we have to do the copy anyway, copyrects
    // will take care of things whether there is an ROI or not.
    //
    if(update_output) {
        if (di->doOrderedDither) {
            DecompressInfo h261_di(di, 352, 288, 1, XIL_BYTE, 
                                   1, 352, buf8);
            di->copyRects(&h261_di);
        } else {
            DecompressInfo h261_di(di, 352, 288, 3, XIL_BYTE, 
                                   3, 3*352, buf24);
            di->copyRects(&h261_di);
        }
    }

    initParser();                   // reset nbits and savedBits

    validDitherState = update_output;

    return;

ErrorReturn:
    XIL_ERROR(di->system_state, XIL_ERROR_USER,"di-312", TRUE);
    return;

}

//
// Deal with skipped macroblocks
// number of macroblocks = current - last  (k - s)
// Update Current blocks using data from the previous frame.
// Also, optionally ;dither destination
///

void 
H261DecompressorData::catchup(DecompressInfo* di,
                              int             skip_blocks,
                              int             start_block, 
                              int             end_block, 
                              int             ygob, 
                              int             xgob, 
                              int             width)
{
    Xil_unsigned8* dest;
    int            xblkstride;
    int            yblkstride;
    int            xmbstride;
    int            ymbstride;

    if(!skip_blocks) {
        if(di->doOrderedDither) {
            //
            // One band output to CIF internal buffer
            //
            dest         = buf8;
            xblkstride   = 8;
            yblkstride   = 8 * 352;
        } else {
            //
            // Three band output to CIF internal buffer
            // This applies to both color converted and Ycc output
            //
            dest         = buf24;
            xblkstride   = 8 * 3;
            yblkstride   = 8 * 3 * 352;
        }

        xmbstride = xblkstride * 2;
        ymbstride = yblkstride * 2;
    }

    //
    // Relative offsets of the four Y blocks
    //

    for(int t=start_block; t<end_block; t++) {
        int blockx = (t-1) % 11;
        int blocky = (t-1) / 11;
        int originy = (ygob + (blocky<<4)) * width;
        int originx = xgob + (blockx<<4);
        int ymbaddr = originy + originx;
        int uvmbaddr = (originy >> 2) + (originx >> 1);

        if(!skip_blocks) {
            Xil_unsigned8    *pdest;
            pdest = dest + (((ygob>>4) + blocky) * ymbstride) + 
            ((blockx + (xgob>>4)) * xmbstride);

            //
            // Update the history raster with the new macroblock
            // (16x16 Y, 8x8 U and V)
            //
            blok.copyBlockRasterToRaster(ypptr+ymbaddr, ycptr+ymbaddr, 
                                         16, 16, width, width);

            blok.copyBlockRasterToRaster(upptr+uvmbaddr, ucptr+uvmbaddr, 
                                         8, 8, width/2, width/2);

            blok.copyBlockRasterToRaster(vpptr+uvmbaddr, vcptr+uvmbaddr, 
                                         8, 8, width/2, width/2);

            //
            // Also update the blnew block, so the converters can
            // access it.
            //
            blok.copyBlockRasterToRaster(ypptr+ymbaddr, blok.blnew[0],
                                         8, 8, width, 8);
            blok.copyBlockRasterToRaster(ypptr+ymbaddr+8, blok.blnew[1],
                                         8, 8, width, 8);
            blok.copyBlockRasterToRaster(ypptr+ymbaddr+8*width, blok.blnew[2],
                                         8, 8, width, 8);
            blok.copyBlockRasterToRaster(ypptr+ymbaddr+8*width+8, blok.blnew[3],
                                         8, 8, width, 8);
            blok.copyBlockRasterToRaster(upptr+uvmbaddr, blok.blnew[4],
                                         8, 8, width/2, 8);
            blok.copyBlockRasterToRaster(vpptr+uvmbaddr, blok.blnew[5],
                                         8, 8, width/2, 8);

            // 
            // Also update the internal dithered image
            //
            if(di->doColorConvert) {
                Ycc2RgbConverter* converter = 
                    (Ycc2RgbConverter*)di->objectPtr1;

                converter->cvtBlock(blok.blnew[0], blok.blnew[4], blok.blnew[5],
                          8, 8, 8, pdest, 3, 352*3); 

                converter->cvtBlock(blok.blnew[1], blok.blnew[4]+4, blok.blnew[5]+4,
                          8, 8, 8, pdest+xblkstride, 3, 352*3);

                pdest += yblkstride;

                converter->cvtBlock(blok.blnew[2], blok.blnew[4]+32, blok.blnew[5]+32,
                          8, 8, 8, pdest, 3, 352*3);

                converter->cvtBlock(blok.blnew[3], blok.blnew[4]+36, blok.blnew[5]+36,
                          8, 8, 8, pdest+xblkstride, 3, 352*3);

            } else if(di->doOrderedDither) {
                    //
                    // Get the ordered dither object
                    //
                    XiliOrderedDitherLut* ditherTable = 
                        (XiliOrderedDitherLut*)di->objectPtr1;

                    ditherTable->dither411(blok.blnew[0], 
                                           blok.blnew[4], 
                                           blok.blnew[5],
                                           8, 8, 8, pdest, 3, 352);
                    ditherTable->dither411(blok.blnew[1], 
                                           blok.blnew[4]+4, 
                                           blok.blnew[5]+4,
                                           8, 8, 8, pdest+xblkstride, 3, 352);

                    pdest += yblkstride;

                    ditherTable->dither411(blok.blnew[2], 
                                           blok.blnew[4]+32, 
                                           blok.blnew[5]+32,
                                           8, 8, 8, pdest, 3, 352);
                    ditherTable->dither411(blok.blnew[3], 
                                           blok.blnew[4]+36, 
                                           blok.blnew[5]+36,
                                           8, 8, 8, pdest+xblkstride, 3, 352);
            } else {
                //
                // The atomic decompress case - simple upsample.
                //

                //
                // Do upper left block
                //
                blok.upsample411Block8FromRaster(blok.blnew[0], 
                                       blok.blnew[4], 
                                       blok.blnew[5],
                                       8, 8, 8, pdest, 3, 352*3);
                //
                // Do upper right block
                //
                blok.upsample411Block8FromRaster(blok.blnew[1], 
                                       blok.blnew[4]+4, 
                                       blok.blnew[5]+4,
                                       8, 8, 8, pdest+xblkstride, 3, 352*3);
                pdest += yblkstride;

                //
                // Do lower left block
                //
                blok.upsample411Block8FromRaster(blok.blnew[2], 
                                       blok.blnew[4]+32, 
                                       blok.blnew[5]+32,
                                       8, 8, 8, pdest, 3, 352*3);
                //
                // Do lower right block
                //
                blok.upsample411Block8FromRaster(blok.blnew[3], 
                                       blok.blnew[4]+36, 
                                       blok.blnew[5]+36,
                                       8, 8, 8, pdest+xblkstride, 3, 352*3);
            }

        } else {

            //
            // Only update the history raster
            // (16x16 Y, 8x8 U and V)
            //
            blok.copyBlockRasterToRaster(ypptr+ymbaddr, ycptr+ymbaddr, 
                                         16, 16, width, width);

            blok.copyBlockRasterToRaster(upptr+uvmbaddr, ucptr+uvmbaddr, 
                                         8, 8, width/2, width/2);

            blok.copyBlockRasterToRaster(vpptr+uvmbaddr, vcptr+uvmbaddr, 
                                         8, 8, width/2, width/2);


        }

    }
}

