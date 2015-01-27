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
//This line lets emacs recognize this as -*- C++ -*- Code
//------------------------------------------------------------------------
//
//  File:       IdctRefFrame.cc
//  Project:    XIL
//  Revision:   1.6
//  Last Mod:   10:14:46, 03/10/00
//
//  Descbiption:
//    Functions to cbeate and access Reference Frames,
//    used to hold previously decoded frames to be used
//    in reconstructing B and P frames.
//
//    Reference macroblocks can be cbeated from any 
//    arbitrarily-aligned 16x16 pixel section of the
//    reference frame, depending on the motion vector
//    stored for a Macroblock. Motion vectors can have 
//    half-pixel addresses, requiring interpolation.
//
//    This implementation uses some trickery to process
//    two 16 bit quantities at once using 32 bit arithmetic.
//
//
//------------------------------------------------------------------------
//    COPYRIGHT
//------------------------------------------------------------------------
#pragma ident   "@(#)IdctRefFrame.cc	1.6\t00/03/10  "


#include "IdctRefFrame.hh"
#include "XiliUtils.hh"

#ifdef XIL_LITTLE_ENDIAN
//
// Shift pixel '1' to pixel '0' position
//
#define SD        >>
//
// Shift pixel '0' to pixel '1' position
//
#define SU        <<

#else

//
// Shift pixel '1' to pixel '0' position
//
#define SD        <<
//
// Shift pixel '0' to pixel '1' position
//
#define SU        >>

#endif

const int  GUARDBAND = 32;

Mpeg1ReferenceFrame::Mpeg1ReferenceFrame()
{
    filled    = FALSE;
    mb        = NULL;
    theBuffer = NULL;
}

Mpeg1ReferenceFrame::~Mpeg1ReferenceFrame()
{
    delete [] mb;
    mb = NULL;
    delete [] theBuffer;
    theBuffer = NULL;
}

void
Mpeg1ReferenceFrame::reset()
{
    delete [] mb;
    mb = NULL;
    delete [] theBuffer;
    theBuffer = NULL;
}

XilStatus
Mpeg1ReferenceFrame::createReference(unsigned int w,
                                     unsigned int h)
{

    //
    // If we've already allocated the buffer, just return
    //
    if(theBuffer != NULL) {
        return XIL_SUCCESS;
    }
    //
    // Force size to be a multiple of 16
    //
    w16 = (w + 15) & ~15;
    h16 = (h + 15)  & ~15;

    width  = w16;
    height = h16;

    nxmb = w16 / 16;
    nymb = h16 / 16;

    //
    // Allocate the memory for the reference frame
    // including the surrounding guardband area
    //
    unsigned int padded_height = h16 + 2*GUARDBAND;
    unsigned int padded_width  = w16 + 2*GUARDBAND;
    unsigned int ybufsize       = padded_width * padded_height;
    unsigned int cbufsize       = padded_width * padded_height / 4;
    unsigned int theBufferSize  = ybufsize + 2*cbufsize;

    //
    // Allocate the data buffer
    //
    theBuffer = new Xil_signed16[theBufferSize];
    if(theBuffer == NULL) {
        return XIL_FAILURE;
    }
    xili_memset(theBuffer, 0, theBufferSize*sizeof(Xil_signed16));

    //
    // Now set the pointers and the line stride values
    // all within the theBuffer. These are all for shorts.
    //
    ystride     = padded_width;
    cstride     = ystride / 2;

    //
    // Set block and macroblock stride values for the buffer.
    // These are all for ints.
    //
    yblkstride  = ystride * 8 / 2;
    ymbstride   = ystride * 16 / 2;
    cmbstride   = cstride * 8 / 2;

    ydata = theBuffer + GUARDBAND*ystride + GUARDBAND; 
    cbdata = theBuffer + padded_height*ystride +
                         (GUARDBAND/2)*cstride + GUARDBAND/2;
    crdata = cbdata + (padded_height/2)*cstride;

    return XIL_SUCCESS;
}

//
// Populate a reference frame with a list of Macroblocks.
// Convert from a Macroblock structure to a linear structure.
//
void 
Mpeg1ReferenceFrame::populateReference()
{
    // Skip it if already filled
//    if(filled) {
 //       return;
  //  }

    MacroBlock* mb_ptr = mb;
    int* y_mb_row  = (int*)ydata;
    int* cb_mb_row = (int*)cbdata;
    int* cr_mb_row = (int*)crdata;
    for(int y=0; y<nymb; y++) {
        int* y_mb_col  = y_mb_row;
        int* cb_mb_col = cb_mb_row;
        int* cr_mb_col = cr_mb_row;
        for(int x=0; x<nxmb; x++) {
            insertMacroBlock(mb_ptr++, y_mb_col, cb_mb_col, cr_mb_col);
            y_mb_col  += 8;
            cb_mb_col += 4;
            cr_mb_col += 4;
        }
        y_mb_row  += ymbstride;
        cb_mb_row += cmbstride;
        cr_mb_row += cmbstride;
    }

    // Mark as already filled
   // filled = TRUE;
}

Xil_boolean
Mpeg1ReferenceFrame::getFilledFlag()
{
    return filled;
}

void
Mpeg1ReferenceFrame::setFilledFlag(Xil_boolean on_off)
{
    filled = on_off;
}


XilStatus
Mpeg1ReferenceFrame::allocMacroBlocks(int nblocks)
{
    // Skip it if allocation already done
    if(mb != NULL) {
        return XIL_SUCCESS;
    }

    mb = new MacroBlock[nblocks];
    if(mb == NULL) {
        return XIL_FAILURE;
    } else {
        return XIL_SUCCESS;
    }
}

//
// Put a YCC MacroBlock into the Mpeg reference frame,
// converting to a linear structure in separate bands.
//
// This does clamping of the values before they go into
// the reference frame. Clamps are done two shorts at
// a time.
//
void 
Mpeg1ReferenceFrame::insertMacroBlock(MacroBlock* mblock,
                                      int*        y_dst,
                                      int*        cb_dst,
                                      int*        cr_dst)
{
    unsigned int dst_blk_row_stride = yblkstride;

    //
    // Do the four Y blocks
    //
    int  k           = 0;
    int* dst_blk_row = y_dst;
    for(int yblk=0; yblk<2; yblk++) {
        int* dst_blk_col = dst_blk_row; 
        for(int xblk=0; xblk<2; xblk++) {
            insertBlock((int*)(mblock->block88[k]), dst_blk_col, ystride/2);
            dst_blk_col += 4;
            k++;
        }
        dst_blk_row += dst_blk_row_stride;
    }

    //
    // Do the Cb And Cr blocks
    //
    insertBlock((int*)(mblock->block88[4]), (int*)cb_dst, cstride/2);
    insertBlock((int*)(mblock->block88[5]), (int*)cr_dst, cstride/2);

}

//
// Put a single 8x8 DCT block into the reference frame buffer
//
// This does clamping of the values before they go into
// the reference frame. Clamps are done two shorts at
// a time. This is all done as int's, which is possible
// because everything is guaranteed aligned.
//
//
void
Mpeg1ReferenceFrame::insertBlock(int*         src_blk,
                                 int*         dst_blk,
                                 unsigned int line_stride)
{
    int* src      = src_blk;
    int* dst_line = dst_blk;
    for(int line=8; line!=0; line--) {
        int* dst = dst_line;

        //
        // Since we're doing two values per int, the loop count
        // is only 4 here for one line of the 8x8 block.
        //
        int* dst_end = dst_line + 4;
        while(dst < dst_end) {
            int pair = *src++;

            //
            // Test if either element has underflowed or overflowed 8 bits, as
            // indicated by any bits being set in the upper half of the shorts.
            // If not, we're done. This is the major speed gain here.
            //
            if((pair & 0xff00ff00) == 0) {
                *dst++ = pair;
            } else {
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

                *dst++ = pair;
            }
        }

        dst_line += line_stride;

    }
}

//
// Create a reference macroblock using sub-pixel interpolation
// based on half-pixel precision motion vectors. The process
// converts from the saved raster-organized image to a Macroblock form.
//
//     fdx, fdy are x,y positions in half-pixel units
//     mbx, mby are the Macroblock indices
//
//
//     refqual is a flag to control quality/speed tradeoff.  
//     It will be used as a mask applied on: ((fdy & 1) << 2) + (fdx & 3);
//     The effect is to avoid sub-pixel positioning if refqual has a 0 
//     corresponding to a sub_pixel_position control bit:
//         y_sub_pixel_position  x_odd_delta  x_sub_pixel_position
//     e.g. refqual of 7 gives full sub-pixel positioning, but
//      refqual of 2 (010) gives no sub-pixel positioning
//     (You need to always set at least 010)
//
//
void 
Mpeg1ReferenceFrame::getMVReference(MacroBlock* mblock,
                                    int         mbx, 
                                    int         mby, 
                                    int         fdx, 
                                    int         fdy, 
                                    int         refqual)
{
    int* s0ptr;
    int* s1ptr;

    int  round;
    int  mask;
    int  pair0;
    int  pair1;

    //
    // Calculate position of reference Macroblock 
    // using the Macroblock index plus the motion vectors.
    //
    int x = (mbx * 16) + (fdx >> 1);
    int y = (mby * 16) + (fdy >> 1);

    //
    // If x/y parameters are illegal, make them legal
    //
    if(x < 0) {
        x   = 0;
        fdx = 0;
    }  
    if(y < 0) {
        y   = 0;
        fdy = 0;
    }  
    if(x+16 > width) {
        x   = width-16;
        fdx = 0;
    }  
    if(y+16 > height) {
        y   = height-16;
        fdy = 0;
    }

    mask = ((fdy & 1) << 2) + (fdx & 3);
    mask &= refqual;

    //
    // Do Y band
    //
    int  src_stride = ystride/2; // Convert stride from shorts to ints

    int* src_upper = (int*)(ydata + y*ystride + x);
    int* src_lower = src_upper + 8*src_stride;

    int* dptr      = (int*)(mblock->block88);
    switch (mask) {
        // 
        // No sub_pixel addressing.  X delta even
        // 
      case 0:

        for(y = 0; y < 8; y++) {
            dptr[ 0] = src_upper[0];
            dptr[ 1] = src_upper[1];
            dptr[ 2] = src_upper[2];
            dptr[ 3] = src_upper[3];
            dptr[32] = src_upper[4];
            dptr[33] = src_upper[5];
            dptr[34] = src_upper[6];
            dptr[35] = src_upper[7];

            dptr[64] = src_lower[0];
            dptr[65] = src_lower[1];  
            dptr[66] = src_lower[2];
            dptr[67] = src_lower[3];
            dptr[96] = src_lower[4];
            dptr[97] = src_lower[5];
            dptr[98] = src_lower[6];
            dptr[99] = src_lower[7];

            src_upper += src_stride;
            src_lower += src_stride;
            dptr      += 4;
        }
        break;

        // 
        // X sub_pixel addressing.  X delta even
        // Perform 2-at-a-time arithmetic for averaging src pixels:, 
        // e.g. (Sparc)
        // First pixel:  (| 0 | 1 |  +  | 1 | Zero |  +  | Zero | 2 |) / 2
        // [0, 1, 2 inside of '||' are pixel numbers, showing 2 pixels per lword]
        // 
      case 1:
        round = 0x00010001;
        mask = 0x00ff00ff;
        for(y = 0; y < 8; y++) {
            s0ptr = src_upper;
            pair0 = s0ptr[0];
            pair1 = s0ptr[1];

            dptr[ 0] = ((pair0 + (pair0 SD 16) + (pair1 SU 16) + round) >> 1) & mask;
            pair0 = s0ptr[2];
            dptr[ 1] = ((pair1 + (pair1 SD 16) + (pair0 SU 16) + round) >> 1) & mask;
            pair1 = s0ptr[3];
            dptr[ 2] = ((pair0 + (pair0 SD 16) + (pair1 SU 16) + round) >> 1) & mask;
            pair0 = s0ptr[4];
            dptr[ 3] = ((pair1 + (pair1 SD 16) + (pair0 SU 16) + round) >> 1) & mask;
            pair1 = s0ptr[5];
            dptr[32] = ((pair0 + (pair0 SD 16) + (pair1 SU 16) + round) >> 1) & mask;
            pair0 = s0ptr[6];
            dptr[33] = ((pair1 + (pair1 SD 16) + (pair0 SU 16) + round) >> 1) & mask;
            pair1 = s0ptr[7];
            dptr[34] = ((pair0 + (pair0 SD 16) + (pair1 SU 16) + round) >> 1) & mask;
            pair0 = s0ptr[8];
            dptr[35] = ((pair1 + (pair1 SD 16) + (pair0 SU 16) + round) >> 1) & mask;

            s0ptr = src_lower;
            pair0 = s0ptr[0];
            pair1 = s0ptr[1];

            dptr[64] = ((pair0 + (pair0 SD 16) + (pair1 SU 16) + round) >> 1) & mask;
            pair0 = s0ptr[2];
            dptr[65] = ((pair1 + (pair1 SD 16) + (pair0 SU 16) + round) >> 1) & mask;
            pair1 = s0ptr[3];
            dptr[66] = ((pair0 + (pair0 SD 16) + (pair1 SU 16) + round) >> 1) & mask;
            pair0 = s0ptr[4];
            dptr[67] = ((pair1 + (pair1 SD 16) + (pair0 SU 16) + round) >> 1) & mask;
            pair1 = s0ptr[5];
            dptr[96] = ((pair0 + (pair0 SD 16) + (pair1 SU 16) + round) >> 1) & mask;
            pair0 = s0ptr[6];
            dptr[97] = ((pair1 + (pair1 SD 16) + (pair0 SU 16) + round) >> 1) & mask;
            pair1 = s0ptr[7];
            dptr[98] = ((pair0 + (pair0 SD 16) + (pair1 SU 16) + round) >> 1) & mask;
            pair0 = s0ptr[8];
            dptr[99] = ((pair1 + (pair1 SD 16) + (pair0 SU 16) + round) >> 1) & mask;

            src_upper += src_stride;
            src_lower += src_stride;
            dptr      += 4;
        }
        break;

        // 
        // No sub_pixel addressing.  X delta odd
        // Combine src pixels from adjacent lwords:, e.g. (Sparc)
        // First pixel:  (| 1 | Zero |  +  | Zero | 2 |) == | 1 | 2 |
        // [1, 2 inside of '||' are pixel numbers, showing 2 pixels per lword]
        // 
      case 2:
        src_upper = (int*)((short*)src_upper-1);
        src_lower = (int*)((short*)src_lower-1);
        for(y = 0; y < 8; y++) {
            s0ptr = src_upper;

            pair0 = s0ptr[0];
            pair1 = s0ptr[1];
            dptr[ 0] = (pair0 SD 16) + (pair1 SU 16);
            pair0 = s0ptr[2];
            dptr[ 1] = (pair1 SD 16) + (pair0 SU 16);
            pair1 = s0ptr[3];
            dptr[ 2] = (pair0 SD 16) + (pair1 SU 16);
            pair0 = s0ptr[4];
            dptr[ 3] = (pair1 SD 16) + (pair0 SU 16);
            pair1 = s0ptr[5];
            dptr[32] = (pair0 SD 16) + (pair1 SU 16);
            pair0 = s0ptr[6];
            dptr[33] = (pair1 SD 16) + (pair0 SU 16);
            pair1 = s0ptr[7];
            dptr[34] = (pair0 SD 16) + (pair1 SU 16);
            pair0 = s0ptr[8];
            dptr[35] = (pair1 SD 16) + (pair0 SU 16);

            s0ptr = src_lower;

            pair0 = s0ptr[0];
            pair1 = s0ptr[1];
            dptr[64] = (pair0 SD 16) + (pair1 SU 16);
            pair0 = s0ptr[2];
            dptr[65] = (pair1 SD 16) + (pair0 SU 16);
            pair1 = s0ptr[3];
            dptr[66] = (pair0 SD 16) + (pair1 SU 16);
            pair0 = s0ptr[4];
            dptr[67] = (pair1 SD 16) + (pair0 SU 16);
            pair1 = s0ptr[5];
            dptr[96] = (pair0 SD 16) + (pair1 SU 16);
            pair0 = s0ptr[6];
            dptr[97] = (pair1 SD 16) + (pair0 SU 16);
            pair1 = s0ptr[7];
            dptr[98] = (pair0 SD 16) + (pair1 SU 16);
            pair0 = s0ptr[8];
            dptr[99] = (pair1 SD 16) + (pair0 SU 16);

            src_upper += src_stride;
            src_lower += src_stride;
            dptr      += 4;
        }
        break;

        // 
        // X sub_pixel addressing.  X delta odd
        // 
      case 3:
        round = 0x00010001;
        mask = 0x00ff00ff;
        src_upper = (int*)((short*)src_upper-1);
        src_lower = (int*)((short*)src_lower-1);
        for(y = 0; y < 8; y++) {
            s0ptr = src_upper;

            pair0 = s0ptr[0];
            pair1 = s0ptr[1];
            dptr[0] = (((pair0 SD 16) + (pair1 SU 16) + pair1 + round) >> 1) & mask;
            pair0 = s0ptr[2];
            dptr[1] = (((pair1 SD 16) + (pair0 SU 16) + pair0 + round) >> 1) & mask;
            pair1 = s0ptr[3];
            dptr[2] = (((pair0 SD 16) + (pair1 SU 16) + pair1 + round) >> 1) & mask;
            pair0 = s0ptr[4];
            dptr[3] = (((pair1 SD 16) + (pair0 SU 16) + pair0 + round) >> 1) & mask;
            pair1 = s0ptr[5];
            dptr[32] = (((pair0 SD 16) + (pair1 SU 16) + pair1 + round) >> 1) & mask;
            pair0 = s0ptr[6];
            dptr[33] = (((pair1 SD 16) + (pair0 SU 16) + pair0 + round) >> 1) & mask;
            pair1 = s0ptr[7];
            dptr[34] = (((pair0 SD 16) + (pair1 SU 16) + pair1 + round) >> 1) & mask;
            pair0 = s0ptr[8];
            dptr[35] = (((pair1 SD 16) + (pair0 SU 16) + pair0 + round) >> 1) & mask;

            s0ptr = src_lower;

            pair0 = s0ptr[0];
            pair1 = s0ptr[1];
            dptr[64] = (((pair0 SD 16) + (pair1 SU 16) + pair1 + round) >> 1) & mask;
            pair0 = s0ptr[2];
            dptr[65] = (((pair1 SD 16) + (pair0 SU 16) + pair0 + round) >> 1) & mask;
            pair1 = s0ptr[3];
            dptr[66] = (((pair0 SD 16) + (pair1 SU 16) + pair1 + round) >> 1) & mask;
            pair0 = s0ptr[4];
            dptr[67] = (((pair1 SD 16) + (pair0 SU 16) + pair0 + round) >> 1) & mask;
            pair1 = s0ptr[5];
            dptr[96] = (((pair0 SD 16) + (pair1 SU 16) + pair1 + round) >> 1) & mask;
            pair0 = s0ptr[6];
            dptr[97] = (((pair1 SD 16) + (pair0 SU 16) + pair0 + round) >> 1) & mask;
            pair1 = s0ptr[7];
            dptr[98] = (((pair0 SD 16) + (pair1 SU 16) + pair1 + round) >> 1) & mask;
            pair0 = s0ptr[8];
            dptr[99] = (((pair1 SD 16) + (pair0 SU 16) + pair0 + round) >> 1) & mask;
            src_upper += src_stride;
            src_lower += src_stride;
            dptr      += 4;
        }
        break;

        // 
        // Y sub_pixel addressing.  X delta even
        // 
      case 4:
        round = 0x00010001;
        mask = 0x00ff00ff;
        for(y = 0; y < 8; y++) {
            s0ptr = src_upper;
            s1ptr = src_upper + src_stride;

            dptr[ 0] = ((s0ptr[0] + s1ptr[0] + round) >> 1) & mask;
            dptr[ 1] = ((s0ptr[1] + s1ptr[1] + round) >> 1) & mask;
            dptr[ 2] = ((s0ptr[2] + s1ptr[2] + round) >> 1) & mask;
            dptr[ 3] = ((s0ptr[3] + s1ptr[3] + round) >> 1) & mask;
            dptr[32] = ((s0ptr[4] + s1ptr[4] + round) >> 1) & mask;
            dptr[33] = ((s0ptr[5] + s1ptr[5] + round) >> 1) & mask;
            dptr[34] = ((s0ptr[6] + s1ptr[6] + round) >> 1) & mask;
            dptr[35] = ((s0ptr[7] + s1ptr[7] + round) >> 1) & mask;

            s0ptr = src_lower;
            s1ptr = src_lower + src_stride;

            dptr[64] = ((s0ptr[0] + s1ptr[0] + round) >> 1) & mask;
            dptr[65] = ((s0ptr[1] + s1ptr[1] + round) >> 1) & mask;
            dptr[66] = ((s0ptr[2] + s1ptr[2] + round) >> 1) & mask;
            dptr[67] = ((s0ptr[3] + s1ptr[3] + round) >> 1) & mask;
            dptr[96] = ((s0ptr[4] + s1ptr[4] + round) >> 1) & mask;
            dptr[97] = ((s0ptr[5] + s1ptr[5] + round) >> 1) & mask;
            dptr[98] = ((s0ptr[6] + s1ptr[6] + round) >> 1) & mask;
            dptr[99] = ((s0ptr[7] + s1ptr[7] + round) >> 1) & mask;

            src_upper += src_stride;
            src_lower += src_stride;
            dptr      += 4;
        }
        break;

        // 
        // Y and X sub_pixel addressing.  X delta even
        // 
      case 5:
        round = 0x00020002;
        mask = 0x00ff00ff;
        for(y = 0; y < 8; y++) {
            s0ptr = src_upper;
            s1ptr = src_upper + src_stride;

            pair0 = s0ptr[0] + s1ptr[0];
            pair1 = s0ptr[1] + s1ptr[1];
            dptr[ 0] = ((pair0 + (pair0 SD 16) + (pair1 SU 16) + round) >> 2) & mask;
            pair0 = s0ptr[2] + s1ptr[2];
            dptr[ 1] = ((pair1 + (pair1 SD 16) + (pair0 SU 16) + round) >> 2) & mask;
            pair1 = s0ptr[3] + s1ptr[3];
            dptr[ 2] = ((pair0 + (pair0 SD 16) + (pair1 SU 16) + round) >> 2) & mask;
            pair0 = s0ptr[4] + s1ptr[4];
            dptr[ 3] = ((pair1 + (pair1 SD 16) + (pair0 SU 16) + round) >> 2) & mask;
            pair1 = s0ptr[5] + s1ptr[5];
            dptr[32] = ((pair0 + (pair0 SD 16) + (pair1 SU 16) + round) >> 2) & mask;
            pair0 = s0ptr[6] + s1ptr[6];
            dptr[33] = ((pair1 + (pair1 SD 16) + (pair0 SU 16) + round) >> 2) & mask;
            pair1 = s0ptr[7] + s1ptr[7];
            dptr[34] = ((pair0 + (pair0 SD 16) + (pair1 SU 16) + round) >> 2) & mask;
            pair0 = s0ptr[8] + s1ptr[8];
            dptr[35] = ((pair1 + (pair1 SD 16) + (pair0 SU 16) + round) >> 2) & mask;

            s0ptr = src_lower;
            s1ptr = src_lower + src_stride;

            pair0 = s0ptr[0] + s1ptr[0];
            pair1 = s0ptr[1] + s1ptr[1];
            dptr[64] = ((pair0 + (pair0 SD 16) + (pair1 SU 16) + round) >> 2) & mask;
            pair0 = s0ptr[2] + s1ptr[2];
            dptr[65] = ((pair1 + (pair1 SD 16) + (pair0 SU 16) + round) >> 2) & mask;
            pair1 = s0ptr[3] + s1ptr[3];
            dptr[66] = ((pair0 + (pair0 SD 16) + (pair1 SU 16) + round) >> 2) & mask;
            pair0 = s0ptr[4] + s1ptr[4];
            dptr[67] = ((pair1 + (pair1 SD 16) + (pair0 SU 16) + round) >> 2) & mask;
            pair1 = s0ptr[5] + s1ptr[5];
            dptr[96] = ((pair0 + (pair0 SD 16) + (pair1 SU 16) + round) >> 2) & mask;
            pair0 = s0ptr[6] + s1ptr[6];
            dptr[97] = ((pair1 + (pair1 SD 16) + (pair0 SU 16) + round) >> 2) & mask;
            pair1 = s0ptr[7] + s1ptr[7];
            dptr[98] = ((pair0 + (pair0 SD 16) + (pair1 SU 16) + round) >> 2) & mask;
            pair0 = s0ptr[8] + s1ptr[8];
            dptr[99] = ((pair1 + (pair1 SD 16) + (pair0 SU 16) + round) >> 2) & mask;

            src_upper += src_stride;
            src_lower += src_stride;
            dptr      += 4;
        }
        break;

        // 
        // Y sub_pixel addressing.  X delta odd
        // 
      case 6:
        round = 0x00010001;
        mask = 0x00ff00ff;
        src_upper = (int*)((short*)src_upper-1);
        src_lower = (int*)((short*)src_lower-1);
        for(y = 0; y < 8; y++) {
            s0ptr = src_upper;
            s1ptr = src_upper + src_stride;

            pair0 = ((s0ptr[0] + s1ptr[0] + round) >> 1) & mask;
            pair1 = ((s0ptr[1] + s1ptr[1] + round) >> 1) & mask;
            dptr[ 0] = (pair0 SD 16) + (pair1 SU 16);
            pair0 = ((s0ptr[2] + s1ptr[2] + round) >> 1) & mask;
            dptr[ 1] = (pair1 SD 16) + (pair0 SU 16);
            pair1 = ((s0ptr[3] + s1ptr[3] + round) >> 1) & mask;
            dptr[ 2] = (pair0 SD 16) + (pair1 SU 16);
            pair0 = ((s0ptr[4] + s1ptr[4] + round) >> 1) & mask;
            dptr[ 3] = (pair1 SD 16) + (pair0 SU 16);
            pair1 = ((s0ptr[5] + s1ptr[5] + round) >> 1) & mask;
            dptr[32] = (pair0 SD 16) + (pair1 SU 16);
            pair0 = ((s0ptr[6] + s1ptr[6] + round) >> 1) & mask;
            dptr[33] = (pair1 SD 16) + (pair0 SU 16);
            pair1 = ((s0ptr[7] + s1ptr[7] + round) >> 1) & mask;
            dptr[34] = (pair0 SD 16) + (pair1 SU 16);
            pair0 = ((s0ptr[8] + s1ptr[8] + round) >> 1) & mask;
            dptr[35] = (pair1 SD 16) + (pair0 SU 16);

            s0ptr = src_lower;
            s1ptr = src_lower + src_stride;

            pair0 = ((s0ptr[0] + s1ptr[0] + round) >> 1) & mask;
            pair1 = ((s0ptr[1] + s1ptr[1] + round) >> 1) & mask;
            dptr[64] = (pair0 SD 16) + (pair1 SU 16);
            pair0 = ((s0ptr[2] + s1ptr[2] + round) >> 1) & mask;
            dptr[65] = (pair1 SD 16) + (pair0 SU 16);
            pair1 = ((s0ptr[3] + s1ptr[3] + round) >> 1) & mask;
            dptr[66] = (pair0 SD 16) + (pair1 SU 16);
            pair0 = ((s0ptr[4] + s1ptr[4] + round) >> 1) & mask;
            dptr[67] = (pair1 SD 16) + (pair0 SU 16);
            pair1 = ((s0ptr[5] + s1ptr[5] + round) >> 1) & mask;
            dptr[96] = (pair0 SD 16) + (pair1 SU 16);
            pair0 = ((s0ptr[6] + s1ptr[6] + round) >> 1) & mask;
            dptr[97] = (pair1 SD 16) + (pair0 SU 16);
            pair1 = ((s0ptr[7] + s1ptr[7] + round) >> 1) & mask;
            dptr[98] = (pair0 SD 16) + (pair1 SU 16);
            pair0 = ((s0ptr[8] + s1ptr[8] + round) >> 1) & mask;
            dptr[99] = (pair1 SD 16) + (pair0 SU 16);

            src_upper += src_stride;
            src_lower += src_stride;
            dptr      += 4;
        }
        break;

        // 
        // Y and X sub_pixel addressing.  X delta odd
        // 
      case 7:
        round = 0x00020002;
        mask = 0x00ff00ff;
        src_upper = (int*)((short*)src_upper-1);
        src_lower = (int*)((short*)src_lower-1);
        for(y = 0; y < 8; y++) {
            s0ptr = src_upper;
            s1ptr = src_upper + src_stride;

            pair0 = s0ptr[0] + s1ptr[0];
            pair1 = s0ptr[1] + s1ptr[1];
            dptr[0] = (((pair0 SD 16) + (pair1 SU 16) + pair1 + round) >> 2) & mask;
            pair0 = s0ptr[2] + s1ptr[2];
            dptr[1] = (((pair1 SD 16) + (pair0 SU 16) + pair0 + round) >> 2) & mask;
            pair1 = s0ptr[3] + s1ptr[3];
            dptr[2] = (((pair0 SD 16) + (pair1 SU 16) + pair1 + round) >> 2) & mask;
            pair0 = s0ptr[4] + s1ptr[4];
            dptr[3] = (((pair1 SD 16) + (pair0 SU 16) + pair0 + round) >> 2) & mask;
            pair1 = s0ptr[5] + s1ptr[5];
            dptr[32] = (((pair0 SD 16) + (pair1 SU 16) + pair1 + round) >> 2) & mask;
            pair0 = s0ptr[6] + s1ptr[6];
            dptr[33] = (((pair1 SD 16) + (pair0 SU 16) + pair0 + round) >> 2) & mask;
            pair1 = s0ptr[7] + s1ptr[7];
            dptr[34] = (((pair0 SD 16) + (pair1 SU 16) + pair1 + round) >> 2) & mask;
            pair0 = s0ptr[8] + s1ptr[8];
            dptr[35] = (((pair1 SD 16) + (pair0 SU 16) + pair0 + round) >> 2) & mask;
            s0ptr = src_lower;
            s1ptr = src_lower + src_stride;

            pair0 = s0ptr[0] + s1ptr[0];
            pair1 = s0ptr[1] + s1ptr[1];
            dptr[64] = (((pair0 SD 16) + (pair1 SU 16) + pair1 + round) >> 2) & mask;
            pair0 = s0ptr[2] + s1ptr[2];
            dptr[65] = (((pair1 SD 16) + (pair0 SU 16) + pair0 + round) >> 2) & mask;
            pair1 = s0ptr[3] + s1ptr[3];
            dptr[66] = (((pair0 SD 16) + (pair1 SU 16) + pair1 + round) >> 2) & mask;
            pair0 = s0ptr[4] + s1ptr[4];
            dptr[67] = (((pair1 SD 16) + (pair0 SU 16) + pair0 + round) >> 2) & mask;
            pair1 = s0ptr[5] + s1ptr[5];
            dptr[96] = (((pair0 SD 16) + (pair1 SU 16) + pair1 + round) >> 2) & mask;
            pair0 = s0ptr[6] + s1ptr[6];
            dptr[97] = (((pair1 SD 16) + (pair0 SU 16) + pair0 + round) >> 2) & mask;
            pair1 = s0ptr[7] + s1ptr[7];
            dptr[98] = (((pair0 SD 16) + (pair1 SU 16) + pair1 + round) >> 2) & mask;
            pair0 = s0ptr[8] + s1ptr[8];
            dptr[99] = (((pair1 SD 16) + (pair0 SU 16) + pair0 + round) >> 2) & mask;

            src_upper += src_stride;
            src_lower += src_stride;
            dptr      += 4;
        }
        break;
    }


    //
    // Do Cr and Cb (UV) bands
    //
    int* cb_dptr = (int*)(mblock->block88[4]);
    int* cr_dptr = (int*)(mblock->block88[5]);
    fdx = fdx / 2;
    fdy = fdy / 2;
    x = (mbx << 3) + (fdx >> 1);
    y = (mby << 3) + (fdy >> 1);

    mask = ((fdy & 1) << 2) + (fdx & 3);
    mask &= refqual;
    int* cb_sptr = (int*)(cbdata + y*cstride + x);
    int* cr_sptr = (int*)(crdata + y*cstride + x);
    src_stride   = cstride/2;
    switch (mask) {
      case 0:
        for(y=0; y<8; y++) {
            cb_dptr[0] = cb_sptr[0];
            cb_dptr[1] = cb_sptr[1];
            cb_dptr[2] = cb_sptr[2];
            cb_dptr[3] = cb_sptr[3];

            cr_dptr[0] = cr_sptr[0];
            cr_dptr[1] = cr_sptr[1];
            cr_dptr[2] = cr_sptr[2];
            cr_dptr[3] = cr_sptr[3];

            cb_dptr += 4;
            cr_dptr += 4;
            cb_sptr += src_stride;
            cr_sptr += src_stride;
        }
        break;

      case 1:
        round = 0x00010001;
        mask = 0x00ff00ff;
        for(y=0; y<8; y++) {
            s0ptr = cb_sptr;
            pair0 = s0ptr[0];
            pair1 = s0ptr[1];

            cb_dptr[0] = ((pair0 + (pair0 SD 16) + (pair1 SU 16) + round) >> 1) & mask;
            pair0 = s0ptr[2];
            cb_dptr[1] = ((pair1 + (pair1 SD 16) + (pair0 SU 16) + round) >> 1) & mask;
            pair1 = s0ptr[3];
            cb_dptr[2] = ((pair0 + (pair0 SD 16) + (pair1 SU 16) + round) >> 1) & mask;
            pair0 = s0ptr[4];
            cb_dptr[3] = ((pair1 + (pair1 SD 16) + (pair0 SU 16) + round) >> 1) & mask;

            s0ptr = cr_sptr;
            pair0 = s0ptr[0];
            pair1 = s0ptr[1];

            cr_dptr[0] = ((pair0 + (pair0 SD 16) + (pair1 SU 16) + round) >> 1) & mask;
            pair0 = s0ptr[2];
            cr_dptr[1] = ((pair1 + (pair1 SD 16) + (pair0 SU 16) + round) >> 1) & mask;
            pair1 = s0ptr[3];
            cr_dptr[2] = ((pair0 + (pair0 SD 16) + (pair1 SU 16) + round) >> 1) & mask;
            pair0 = s0ptr[4];
            cr_dptr[3] = ((pair1 + (pair1 SD 16) + (pair0 SU 16) + round) >> 1) & mask;

            cb_dptr += 4;
            cr_dptr += 4;
            cb_sptr += src_stride;
            cr_sptr += src_stride;
        }
        break;

      case 2:
        cb_sptr = (int*)((short*)cb_sptr-1);
        cr_sptr = (int*)((short*)cr_sptr-1);
        for(y = 0; y < 8; y++) {
            s0ptr = cb_sptr;
            pair0 = s0ptr[0];
            pair1 = s0ptr[1];

            cb_dptr[0] = (pair0 SD 16) + (pair1 SU 16);
            pair0 = s0ptr[2];
            cb_dptr[1] = (pair1 SD 16) + (pair0 SU 16);
            pair1 = s0ptr[3];
            cb_dptr[2] = (pair0 SD 16) + (pair1 SU 16);
            pair0 = s0ptr[4];
            cb_dptr[3] = (pair1 SD 16) + (pair0 SU 16);

            s0ptr = cr_sptr;
            pair0 = s0ptr[0];
            pair1 = s0ptr[1];

            cr_dptr[0] = (pair0 SD 16) + (pair1 SU 16);
            pair0 = s0ptr[2];
            cr_dptr[1] = (pair1 SD 16) + (pair0 SU 16);
            pair1 = s0ptr[3];
            cr_dptr[2] = (pair0 SD 16) + (pair1 SU 16);
            pair0 = s0ptr[4];
            cr_dptr[3] = (pair1 SD 16) + (pair0 SU 16);

            cb_dptr += 4;
            cr_dptr += 4;
            cb_sptr += src_stride;
            cr_sptr += src_stride;
        }
        break;

      case 3:
        round = 0x00010001;
        mask = 0x00ff00ff;
        cb_sptr = (int*)((short*)cb_sptr-1);
        cr_sptr = (int*)((short*)cr_sptr-1);
        for(y = 0; y < 8; y++) {
            s0ptr = cb_sptr;
            pair0 = s0ptr[0];
            pair1 = s0ptr[1];

            cb_dptr[0] = (((pair0 SD 16) + (pair1 SU 16) + pair1 + round) >> 1) & mask;
            pair0 = s0ptr[2];
            cb_dptr[1] = (((pair1 SD 16) + (pair0 SU 16) + pair0 + round) >> 1) & mask;
            pair1 = s0ptr[3];
            cb_dptr[2] = (((pair0 SD 16) + (pair1 SU 16) + pair1 + round) >> 1) & mask;
            pair0 = s0ptr[4];
            cb_dptr[3] = (((pair1 SD 16) + (pair0 SU 16) + pair0 + round) >> 1) & mask;

            s0ptr = cr_sptr;
            pair0 = s0ptr[0];
            pair1 = s0ptr[1];

            cr_dptr[0] = (((pair0 SD 16) + (pair1 SU 16) + pair1 + round) >> 1) & mask;
            pair0 = s0ptr[2];
            cr_dptr[1] = (((pair1 SD 16) + (pair0 SU 16) + pair0 + round) >> 1) & mask;
            pair1 = s0ptr[3];
            cr_dptr[2] = (((pair0 SD 16) + (pair1 SU 16) + pair1 + round) >> 1) & mask;
            pair0 = s0ptr[4];
            cr_dptr[3] = (((pair1 SD 16) + (pair0 SU 16) + pair0 + round) >> 1) & mask;

            cb_dptr += 4;
            cr_dptr += 4;
            cb_sptr += src_stride;
            cr_sptr += src_stride;
        }
        break;

      case 4:
        round = 0x00010001;
        mask = 0x00ff00ff;
        for(y = 0; y < 8; y++) {
            s0ptr = cb_sptr;
            s1ptr = cb_sptr + src_stride;

            cb_dptr[0] = ((s0ptr[0] + s1ptr[0] + round) >> 1) & mask;
            cb_dptr[1] = ((s0ptr[1] + s1ptr[1] + round) >> 1) & mask;
            cb_dptr[2] = ((s0ptr[2] + s1ptr[2] + round) >> 1) & mask;
            cb_dptr[3] = ((s0ptr[3] + s1ptr[3] + round) >> 1) & mask;

            s0ptr = cr_sptr;
            s1ptr = cr_sptr + src_stride;

            cr_dptr[0] = ((s0ptr[0] + s1ptr[0] + round) >> 1) & mask;
            cr_dptr[1] = ((s0ptr[1] + s1ptr[1] + round) >> 1) & mask;
            cr_dptr[2] = ((s0ptr[2] + s1ptr[2] + round) >> 1) & mask;
            cr_dptr[3] = ((s0ptr[3] + s1ptr[3] + round) >> 1) & mask;

            cb_dptr += 4;
            cr_dptr += 4;
            cb_sptr += src_stride;
            cr_sptr += src_stride;
        }
        break;

      case 5:
        round = 0x00020002;
        mask = 0x00ff00ff;
        for(y = 0; y < 8; y++) {
            s0ptr = cb_sptr;
            s1ptr = cb_sptr + src_stride;

            pair0 = s0ptr[0] + s1ptr[0];
            pair1 = s0ptr[1] + s1ptr[1];
            cb_dptr[0] = ((pair0 + (pair0 SD 16) + (pair1 SU 16) + round) >> 2) & mask;
            pair0 = s0ptr[2] + s1ptr[2];
            cb_dptr[1] = ((pair1 + (pair1 SD 16) + (pair0 SU 16) + round) >> 2) & mask;
            pair1 = s0ptr[3] + s1ptr[3];
            cb_dptr[2] = ((pair0 + (pair0 SD 16) + (pair1 SU 16) + round) >> 2) & mask;
            pair0 = s0ptr[4] + s1ptr[4];
            cb_dptr[3] = ((pair1 + (pair1 SD 16) + (pair0 SU 16) + round) >> 2) & mask;

            s0ptr = cr_sptr;
            s1ptr = cr_sptr + src_stride;

            pair0 = s0ptr[0] + s1ptr[0];
            pair1 = s0ptr[1] + s1ptr[1];
            cr_dptr[0] = ((pair0 + (pair0 SD 16) + (pair1 SU 16) + round) >> 2) & mask;
            pair0 = s0ptr[2] + s1ptr[2];
            cr_dptr[1] = ((pair1 + (pair1 SD 16) + (pair0 SU 16) + round) >> 2) & mask;
            pair1 = s0ptr[3] + s1ptr[3];
            cr_dptr[2] = ((pair0 + (pair0 SD 16) + (pair1 SU 16) + round) >> 2) & mask;
            pair0 = s0ptr[4] + s1ptr[4];
            cr_dptr[3] = ((pair1 + (pair1 SD 16) + (pair0 SU 16) + round) >> 2) & mask;

            cb_dptr += 4;
            cr_dptr += 4;
            cb_sptr += src_stride;
            cr_sptr += src_stride;
        }
        break;

      case 6:
        round = 0x00010001;
        mask = 0x00ff00ff;
        cb_sptr = (int*)((short*)cb_sptr-1);
        cr_sptr = (int*)((short*)cr_sptr-1);
        for(y = 0; y < 8; y++) {
            s0ptr = cb_sptr;
            s1ptr = cb_sptr + src_stride;

            pair0 = ((s0ptr[0] + s1ptr[0] + round) >> 1) & mask;
            pair1 = ((s0ptr[1] + s1ptr[1] + round) >> 1) & mask;
            cb_dptr[0] = (pair0 SD 16) + (pair1 SU 16);
            pair0 = ((s0ptr[2] + s1ptr[2] + round) >> 1) & mask;
            cb_dptr[1] = (pair1 SD 16) + (pair0 SU 16);
            pair1 = ((s0ptr[3] + s1ptr[3] + round) >> 1) & mask;
            cb_dptr[2] = (pair0 SD 16) + (pair1 SU 16);
            pair0 = ((s0ptr[4] + s1ptr[4] + round) >> 1) & mask;
            cb_dptr[3] = (pair1 SD 16) + (pair0 SU 16);

            s0ptr = cr_sptr;
            s1ptr = cr_sptr + src_stride;

            pair0 = ((s0ptr[0] + s1ptr[0] + round) >> 1) & mask;
            pair1 = ((s0ptr[1] + s1ptr[1] + round) >> 1) & mask;
            cr_dptr[0] = (pair0 SD 16) + (pair1 SU 16);
            pair0 = ((s0ptr[2] + s1ptr[2] + round) >> 1) & mask;
            cr_dptr[1] = (pair1 SD 16) + (pair0 SU 16);
            pair1 = ((s0ptr[3] + s1ptr[3] + round) >> 1) & mask;
            cr_dptr[2] = (pair0 SD 16) + (pair1 SU 16);
            pair0 = ((s0ptr[4] + s1ptr[4] + round) >> 1) & mask;
            cr_dptr[3] = (pair1 SD 16) + (pair0 SU 16);

            cb_dptr += 4;
            cr_dptr += 4;
            cb_sptr += src_stride;
            cr_sptr += src_stride;
        }
        break;

      case 7:
        round = 0x00020002;
        mask = 0x00ff00ff;
        cb_sptr = (int*)((short*)cb_sptr-1);
        cr_sptr = (int*)((short*)cr_sptr-1);
        for(y = 0; y < 8; y++) {
            s0ptr = cb_sptr;
            s1ptr = cb_sptr + src_stride;

            pair0 = s0ptr[0] + s1ptr[0];
            pair1 = s0ptr[1] + s1ptr[1];
            cb_dptr[0] = (((pair0 SD 16) + (pair1 SU 16) + pair1 + round) >> 2) & mask;
            pair0 = s0ptr[2] + s1ptr[2];
            cb_dptr[1] = (((pair1 SD 16) + (pair0 SU 16) + pair0 + round) >> 2) & mask;
            pair1 = s0ptr[3] + s1ptr[3];
            cb_dptr[2] = (((pair0 SD 16) + (pair1 SU 16) + pair1 + round) >> 2) & mask;
            pair0 = s0ptr[4] + s1ptr[4];
            cb_dptr[3] = (((pair1 SD 16) + (pair0 SU 16) + pair0 + round) >> 2) & mask;

            s0ptr = cr_sptr;
            s1ptr = cr_sptr + src_stride;

            pair0 = s0ptr[0] + s1ptr[0];
            pair1 = s0ptr[1] + s1ptr[1];
            cr_dptr[0] = (((pair0 SD 16) + (pair1 SU 16) + pair1 + round) >> 2) & mask;
            pair0 = s0ptr[2] + s1ptr[2];
            cr_dptr[1] = (((pair1 SD 16) + (pair0 SU 16) + pair0 + round) >> 2) & mask;
            pair1 = s0ptr[3] + s1ptr[3];
            cr_dptr[2] = (((pair0 SD 16) + (pair1 SU 16) + pair1 + round) >> 2) & mask;
            pair0 = s0ptr[4] + s1ptr[4];
            cr_dptr[3] = (((pair1 SD 16) + (pair0 SU 16) + pair0 + round) >> 2) & mask;

            cb_dptr += 4;
            cr_dptr += 4;
            cb_sptr += src_stride;
            cr_sptr += src_stride;
        }
        break;
    }
}

