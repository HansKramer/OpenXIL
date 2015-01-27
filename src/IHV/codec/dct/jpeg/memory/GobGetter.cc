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
//  File:       GobGetter.cc
//  Project:    XIL
//  Revision:   1.11
//  Last Mod:   10:14:25, 03/10/00
//
//  Description:
//
//    GobGetter returns Groups Of Blocks (GOBs) formatted for
//    easy access by other routines used in JPEG encoding.
//
//------------------------------------------------------------------------
//    COPYRIGHT
//------------------------------------------------------------------------
#pragma ident   "@(#)GobGetter.cc	1.11\t00/03/10  "

#include "GobGetter.hh"
#include "CompressInfo.hh"

//
// TODO: DO what this comment says
// Constructor for the Group of Blocks Getter
// Give the constructor the XilStorage Object to use
// to initialize its internal variables
//

GobGetter::GobGetter()
{
    in_base   = NULL;    // the in_base address
    in_pixel  = 0;       // the # of bytes to next pixel
    in_scan   = 0;       // the # of bytes to next scanline
    in_pixel2 = 0;       // the # of bytes to 2nd pixel
    in_scan2  = 0;       // the # of bytes to 2nd scanline
    in_bband  = 0;       // the # of bytes to next band
    in_nbands = 0;       // the # of bands in the input image
    UR        = 0;       // displacement to upper right block
    LL        = 0;       // displacement to lower left block
    LR        = 0;       // displacement to lower right block
}

void
GobGetter::useImage(CompressInfo* ci)
{
    in_width   = ci->image_width;
    in_height  = ci->image_height;
    
    in_base    = (Xil_unsigned8*)ci->image_dataptr;
    in_nbands  = ci->image_nbands;
    in_pixel   = ci->image_ps;
    in_scan    = ci->image_ss;
    in_pixel2  = in_pixel * 2;
    in_scan2   = in_scan * 2;
    in_bband   = ci->image_bs; // PIXEL_SEQUENTIAL only for now
    UR         = in_pixel;
    LL         = in_scan;
    LR         = UR + LL;
}

//
// Constructor version which takes a separate memory
// buffer for use as tha data src. This is used during the
// temporal filter operations (The data comes from a filtered
// tmp image).
//
void
GobGetter::useImage(CompressInfo* ci,
                    Xil_unsigned8* tmp_data_ptr)
{
    in_width   = ci->image_width;
    in_height  = ci->image_height;
    
    in_base    = tmp_data_ptr;
    in_nbands  = 3;
    in_pixel   = 3;
    in_scan    = ci->image_width * 3;
    in_pixel2  = in_pixel * 2;
    in_scan2   = in_scan * 2;
    in_bband   = 1;
    UR         = in_pixel;
    LL         = in_scan;
    LR         = UR + LL;
}

//
//  Get a single 8x8 block from one band of the source image.
//
//
void 
GobGetter::getNonInterleavedGob(int px, 
                                int py, 
                                int band,
                                int* gob)
{
    getBlock(px, py, in_base + px*in_pixel + py*in_scan + band, gob);
}


//
//  Get an interleaved group of blocks from a multiband image.
//  Each band goes into a separate 8x8 block.
//
void 
GobGetter::getInterleavedGob(int px, 
                             int py, 
                             int nbands,
                             int* gob)
{
    int* ptr = gob;
    for(int band=0; band<nbands; band++) {

        getBlock(px, py, in_base + px*in_pixel + py*in_scan + band, ptr);
        ptr += Msize;

    }

}


//
//  This routine populates 6 8x8 blocks from 3 pixel_interleaved
//  input bands. It is intended to be used on YUV (actually YCbCr) data.
//  The Y blocks (blocks 0...3) are not sampled. The U and V blocks
//  are subsampled by 2 in each axis by averaging each 2x2 pixel block
//  in the 16x16 area. This grouping is termed a macroblock.
//        

void 
GobGetter::getInterleaved411Gob(int px, 
                                int py,
                                int* gob)
{
    Xil_unsigned8* yBlk = in_base + py*in_scan + px*in_pixel;
    Xil_unsigned8* uBlk = yBlk + 1;
    Xil_unsigned8* vBlk = yBlk + 2;

    unsigned int xBlkStride = BLOCK_SIZE * in_pixel;
    unsigned int yBlkStride = BLOCK_SIZE * in_scan;

    //
    // Pre-test whether this is a border block.
    // If not, the logic is much simpler (and faster).
    //
    Xil_boolean overX  = (px + MACRO_BLOCK_SIZE_411) > in_width;
    Xil_boolean overY  = (py + MACRO_BLOCK_SIZE_411) > in_height;

    if(!overX && !overY) {
        ////////////////////////////////
        //
        // Interior case
        //
        //   We can use streamlined code here since the 
        //   image bounds can be ignored.
        //
        ////////////////////////////////

        //
        // Do the four Y blocks
        //
        Xil_unsigned8* pYblk = yBlk;
        Xil_unsigned8* pLine;
        int* pBlk = gob;
        int* pDst;
        for(int yb=0; yb<2; yb++) {
            Xil_unsigned8* pXblk = pYblk;
            for(int xb=0; xb<2; xb++) {
                pDst = pBlk;
                pLine = pXblk;
                for(int line=BLOCK_SIZE; line-- !=0; pLine+=in_scan) {
                    Xil_unsigned8* pSamp = pLine;
                    for(int samp=BLOCK_SIZE; samp-- !=0; pSamp+=in_pixel) {
                        *pDst++ = *pSamp - 128;
                    }
                }
                pXblk += xBlkStride;
                pBlk += Msize;
            }
            pYblk += yBlkStride;
        }
        
        //
        // Do the U block
        // Subsample both axes by 2 by averaging 2x2 pixel blocks
        //
        pDst = pBlk;
        pLine = uBlk;
        for(int line=BLOCK_SIZE; line-- !=0; pLine+=in_scan2) {
            Xil_unsigned8* pSamp = pLine;
            for(int samp=BLOCK_SIZE; samp-- !=0; pSamp+=in_pixel2) {
                *pDst++ = (pSamp[0]+pSamp[UR]+pSamp[LL]+pSamp[LR]) / 4 - 128;
            }
        }
        pBlk += Msize;

        //
        // Do the V block
        //
        pDst = pBlk;
        pLine = vBlk;
        for(line=BLOCK_SIZE; line-- !=0; pLine+=in_scan2) {
            Xil_unsigned8* pSamp = pLine;
            for(int samp=BLOCK_SIZE; samp-- !=0; pSamp+=in_pixel2) {
                *pDst++ = (pSamp[0]+pSamp[UR]+pSamp[LL]+pSamp[LR]) / 4 - 128;
            }
        }
        
    } else {

        ////////////////////////////////////////////////////////////////
        //
        // Border cases
        //
        //   First, fill all the blocks with only the valid pixels
        //   Then, go through each block and pad out the edges with the 
        //   last valid pixel in that line or column.
        //
        ////////////////////////////////////////////////////////////////

        int nValidY = (int)in_height - py;
        if(nValidY >= MACRO_BLOCK_SIZE_411) {
            nValidY = MACRO_BLOCK_SIZE_411;
        }

        int nValidX = (int)in_width - px;
        if(nValidX >= MACRO_BLOCK_SIZE_411) {
            nValidX = MACRO_BLOCK_SIZE_411;
        }

        //
        // Do the four Y blocks as a single 16x16
        //
        int tmpBlock[MACRO_BLOCK_SIZE_411*MACRO_BLOCK_SIZE_411];
        Xil_unsigned8* pLine = yBlk;
        int* pDstLine = tmpBlock;
        for(int line=nValidY; line-- !=0; pLine+=in_scan) {
            Xil_unsigned8* pSamp = pLine;
            int* pDst = pDstLine;
            for(int samp=nValidX; samp-- !=0; pSamp+=in_pixel) {
                *pDst++ = *pSamp - 128;
            }
            pDstLine += MACRO_BLOCK_SIZE_411;
        }

        //
        // Pad out the 16x16 Y block
        //
        padBlock(tmpBlock, nValidX, nValidY, 
                 MACRO_BLOCK_SIZE_411, MACRO_BLOCK_SIZE_411);

        //
        // Copy the 16x16 block to the 4 8x8 blocks
        //
        for(int yb=0; yb<2; yb++) {
            for(int xb=0; xb<2; xb++) {
                copy88(tmpBlock+yb*128+xb*8, gob+(yb*2 + xb)*Msize);
            }
        }


        //
        // Do the U and V blocks
        // If the last valid column is even, we can't average
        // the pixels, so we'll just take the single value.
        // Ditto in the row direction.
        //
        Xil_unsigned8* pSrcLineU = uBlk;
        Xil_unsigned8* pSrcLineV = vBlk;
        int*           pDstLineU = gob+4*Msize;
        int*           pDstLineV = gob+5*Msize;
        for(line=0; line<nValidY; line+=2) {
            Xil_unsigned8* pSrcU = pSrcLineU;
            Xil_unsigned8* pSrcV = pSrcLineV;
            int*           pDstU = pDstLineU;
            int*           pDstV = pDstLineV;
            for(int samp=0; samp<nValidX; samp+=2) {
                if((line+1)<nValidY && (samp+1)<nValidX) {
                    *pDstU++ = (pSrcU[0]+pSrcU[UR]+pSrcU[LL]+pSrcU[LR]) / 4 - 128;
                    *pDstV++ = (pSrcV[0]+pSrcV[UR]+pSrcV[LL]+pSrcV[LR]) / 4 - 128;
                } else {
                    *pDstU++ = pSrcU[0] - 128;
                    *pDstV++ = pSrcV[0] - 128;
                }
                pSrcU += in_pixel2;
                pSrcV += in_pixel2;
            }
            pSrcLineU += in_scan2;
            pSrcLineV += in_scan2;
            pDstLineU += BLOCK_SIZE;
            pDstLineV += BLOCK_SIZE;
        }

        padBlock(gob+4*Msize, (nValidX+1)/2, (nValidY+1)/2, 
                 BLOCK_SIZE, BLOCK_SIZE);
        padBlock(gob+5*Msize, (nValidX+1)/2, (nValidY+1)/2, 
                 BLOCK_SIZE, BLOCK_SIZE);

    }

}

void
GobGetter::padBlock(int* block, 
                    int  nx, 
                    int  ny, 
                    int  xsize, 
                    int  ysize)
{
    //
    // Pad right edge (if any)
    //
    if(nx < xsize) {
        int npad = xsize - nx;
        int* dst = block + nx;
        for(int line=0; line<ny; line++) {
            int value = dst[-1];
            padX(value, dst, npad);
            dst += xsize;
        }
    }

    //
    // Pad bottom edge (if any)
    //
    if(ny < ysize) {
        int* dst = block + xsize*ny;
        int* src = dst - xsize;
        for(int line=ny; line<ysize; line++) {
            padY(src, dst, xsize);
            dst += xsize;
        }
    }

}

//
// Replicate a right edge pixel
//
void
GobGetter::padX(int  xvalue, 
                int* dst, 
                int  npad)
{
    int* pDst = dst;
    for(int count=npad; count!=0; count--) {
        *pDst++ = xvalue;
    }
}

//
// Replicate a bottom line
//
void
GobGetter::padY(int* pSrc, 
                int* pDst, 
                int  xsize)
{
    for(int count=xsize; count!=0; count--) {
        *pDst++ = *pSrc++;
    }
}


//
// Copy a single 8x8 block from within a 16x16 block
// to its own 8x8 block. Used only for 4:1:1 block processing.
//
void
GobGetter::copy88(int* src, 
                  int* dst)
{
    int* srcLine = src;
    int* pDst = dst;
    for(int count=BLOCK_SIZE; count-- !=0; ) {
        int* pSrc = srcLine;
        for(int xcount=BLOCK_SIZE; xcount-- !=0; ) {
            *pDst++ = *pSrc++;
        }
        srcLine += MACRO_BLOCK_SIZE_411;
    }
}

//
// Get a single 8x8 block from the source image,
// padding it if necessary. No subsampling is done.
// X padding is done by replicating the rightmost pixel
// in each line. Y padding is done by replicating the last
// valid line, with its X pad, if applicable.
//
void
GobGetter::getBlock(int            px,
                    int            py,
                    Xil_unsigned8* blkBase,
                    int*           dst)
{
    Xil_unsigned8* pLine = blkBase;

    //
    // Pre-test whether this is a border block.
    // If not, the logic is much simpler (and faster).
    //
    Xil_boolean overX  = px > ((int)in_width  - BLOCK_SIZE);
    Xil_boolean overY  = py > ((int)in_height - BLOCK_SIZE);

    if(!overX && !overY) {
        ////////////////////////////////
        //
        // Interior case
        //
        //   We can use streamlined code here since the 
        //   image bounds can be ignored.
        //
        ////////////////////////////////

        //
        // Do the whole 8x8 block
        //
        int* pDst = dst;
        for(int line=BLOCK_SIZE; line-- !=0; pLine+=in_scan) {
            Xil_unsigned8* pSamp = pLine;
            for(int samp=BLOCK_SIZE; samp-- !=0; pSamp+=in_pixel) {
                *pDst++ = *pSamp - 128;
            }
        }
        
    } else {

        ////////////////////////////////////////////////////////////////
        //
        // Border cases
        //
        //   First, fill all the blocks with only the valid pixels
        //   Then, go through each block and pad out the edges with the 
        //   last valid pixel in that line or column.
        //
        ////////////////////////////////////////////////////////////////

        int nValidY = (int)in_height - py;
        if(nValidY >= BLOCK_SIZE) {
            nValidY = BLOCK_SIZE;
        }

        int nValidX = (int)in_width - px;
        if(nValidX >= BLOCK_SIZE) {
            nValidX = BLOCK_SIZE;
        }

        //
        // Do the valid part of the block
        //
        int* pDstLine = dst;
        for(int line=nValidY; line-- !=0; pLine+=in_scan) {
            Xil_unsigned8* pSamp = pLine;
            int* pDst = pDstLine;
            for(int samp=nValidX; samp-- !=0; pSamp+=in_pixel) {
                *pDst++ = *pSamp - 128;
            }
            pDstLine += BLOCK_SIZE;
        }

        //
        // Pad out the remainder of the block
        //
        padBlock(dst, nValidX, nValidY, BLOCK_SIZE, BLOCK_SIZE);

    }

}

