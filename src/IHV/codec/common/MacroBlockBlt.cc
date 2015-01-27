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
//  File:       MacroBlockBlt.cc
//  Project:    XIL
//  Revision:   1.3
//  Last Mod:   10:16:13, 03/10/00
//
//  Description:
//
//    TODO: Enter some descriptive text here
//
//
//------------------------------------------------------------------------
//    COPYRIGHT
//------------------------------------------------------------------------
#pragma ident   "@(#)MacroBlockBlt.cc	1.3\t00/03/10  "


#include "MacroBlockBlt.hh"
#include "Idct.hh"

void 
getblocks(Xil_unsigned8* yp, 
          Xil_unsigned8* up, 
          Xil_unsigned8* vp, 
          short*         blold[6], 
          Xil_unsigned8 *blnew[6], 
          int            yyad, 
          int            uvad,
          int            mvy, 
          int            mvx, 
          int            width, 
          int cbp)
{
    int        i,j,k;
    int uvy,uvx,uv_width;

    /*
    * Compute stride and motion vectors for U and V
    */
    uv_width = width>>1;
    if(mvx < 0)
        uvx = -((-mvx)>>1);
    else
        uvx = mvx>>1;
    if(mvy < 0)
        uvy = -((-mvy)>>1);
    else
        uvy = mvy>>1;

    /*
    * Load src_start array with pointers to src for each block in the
    * macroblock.
    */
    Xil_unsigned8* mvaddr = yp + yyad + mvy * width + mvx;
    Xil_unsigned8* src_start[6];
    src_start[0] = mvaddr;
    src_start[1] = mvaddr + 8;
    src_start[2] = mvaddr + (width << 3);
    src_start[3] = mvaddr + (width << 3) + 8;
    int uvoffset = uvad + uvy * uv_width + uvx;
    src_start[4] = up + uvoffset;
    src_start[5] = vp + uvoffset;
    int stride = width;

    for(k=0; k < 6; k++) {
        Xil_unsigned32* pblock_new = (Xil_unsigned32*)blnew[k];
        Xil_unsigned8* psrc = src_start[k];
        if(k == 4)
            stride = uv_width;
        if((cbp >> (5-k)) & 1) {
            /*
            * There are DCT coefficients for this block, so the destination
            * block already has valid data in it.  We need to add in the
            * predictor and clamp the result
            */
            short* pblock_old = blold[k];
            int value;
            Xil_unsigned32 saved_values;
            for(i=0;i<8;i++) {
                for(j=0;j<2;j++) {
                    value = (int) (pblock_old[0]) + (int) psrc[0];
                    if(value & ~255) {
                        if(value < 0)
                        saved_values = 0;
                        else
                        ASSIGN_FIRST_BYTE(saved_values,255);
                    } else
                    ASSIGN_FIRST_BYTE(saved_values,value);
                    value = (int) (pblock_old[1]) + (int) psrc[1];
                    if(value & ~255) {
                        if(value > 255)
                        ASSIGN_SECOND_BYTE(saved_values,255);
                    } else
                    ASSIGN_SECOND_BYTE(saved_values,value);
                    value = (int) (pblock_old[2]) + (int) psrc[2];
                    if(value & ~255) {
                        if(value > 255)
                        ASSIGN_THIRD_BYTE(saved_values,255);
                    } else
                    ASSIGN_THIRD_BYTE(saved_values,value);
                    value = (int) (pblock_old[3]) + (int) psrc[3];
                    if(value & ~255) {
                        if(value > 255)
                        ASSIGN_FOURTH_BYTE(saved_values,255);
                    } else
                    ASSIGN_FOURTH_BYTE(saved_values,value);
                    *pblock_new = saved_values;
                    pblock_old += 4;
                    psrc += 4;
                    pblock_new++;
                }
                psrc += stride - 8;
            }
        } else {
            /*
            * Use best copy for given src pointer alignment
            * (block pointers are double word aligned)
            */
            switch (((int)psrc) & 7) {
#ifdef XIL_LITTLE_ENDIAN
                case 1:
                case 3:
                case 5:
                case 7:
                /*
                * byte copies
                */
                {
                    Xil_unsigned8* pblock_new_bytes = 
                    (Xil_unsigned8*)pblock_new;
                    for(i=0;i<8;i++)
                    {
                        pblock_new_bytes[0] = psrc[0];
                        pblock_new_bytes[1] = psrc[1];
                        pblock_new_bytes[2] = psrc[2];
                        pblock_new_bytes[3] = psrc[3];
                        pblock_new_bytes[4] = psrc[4];
                        pblock_new_bytes[5] = psrc[5];
                        pblock_new_bytes[6] = psrc[6];
                        pblock_new_bytes[7] = psrc[7];
                        pblock_new_bytes += 8;
                        psrc += stride;
                    }
                }
                break;
                case 2:
                case 6:
                /*
                * short copies
                */
                {
                    Xil_unsigned16* pblock_new_shorts = 
                    (Xil_unsigned16*)pblock_new;
                    for(i=0;i<8;i++)
                    {
                        pblock_new_shorts[0] = ((Xil_unsigned16*)psrc)[0];
                        pblock_new_shorts[1] = ((Xil_unsigned16*)psrc)[1];
                        pblock_new_shorts[2] = ((Xil_unsigned16*)psrc)[2];
                        pblock_new_shorts[3] = ((Xil_unsigned16*)psrc)[3];
                        pblock_new_shorts += 4;
                        psrc += stride;
                    }
                }
                break;
#else
                case 1:
                case 5:
                for(i=0;i<8;i++)
                {
                    Xil_unsigned32 word1;
                    Xil_unsigned32 word2;

                    word1 = ((Xil_unsigned32*)(psrc-1))[0];
                    word2 = ((Xil_unsigned32*)(psrc+3))[0];
                    pblock_new[0] = (word1<<8) | (word2>>24);
                    word1 = ((Xil_unsigned32*)(psrc+7))[0];
                    pblock_new[1] = (word2 << 8) | (word1>>24);
                    pblock_new += 2;
                    psrc += stride;
                }
                break;
                case 3:
                case 7:
                for(i=0;i<8;i++)
                {
                    Xil_unsigned32 word1;
                    Xil_unsigned32 word2;

                    word1 = ((Xil_unsigned32*)(psrc-3))[0];
                    word2 = ((Xil_unsigned32*)(psrc+1))[0];
                    pblock_new[0] = (word1<<24) | (word2>>8);
                    word1 = ((Xil_unsigned32*)(psrc+5))[0];
                    pblock_new[1] = (word2 << 24) | (word1>>8);
                    pblock_new += 2;
                    psrc += stride;
                }
                break;
                case 2:
                case 6:
                for(i=0;i<8;i++)
                {
                    pblock_new[0] = (((Xil_unsigned16*)psrc)[0] << 16) |
                    ((Xil_unsigned16*)psrc)[1];
                    pblock_new[1] = (((Xil_unsigned16*)psrc)[2] << 16) |
                    ((Xil_unsigned16*)psrc)[3];
                    pblock_new += 2;
                    psrc += stride;
                }
                break;
#endif
                case 4:
                for(i=0;i<4;i++)
                {
                    pblock_new[0] = ((int*)psrc)[0];
                    pblock_new[1] = ((int*)psrc)[1];
                    psrc += stride;
                    pblock_new[2] = ((int*)psrc)[0];
                    pblock_new[3] = ((int*)psrc)[1];
                    psrc += stride;
                    pblock_new += 4;
                }
                break;
                case 0:
                for(i=0;i<2;i++)
                {
                    ((double*)pblock_new)[0] = ((double*)psrc)[0];
                    psrc += stride;
                    ((double*)pblock_new)[1] = ((double*)psrc)[0];
                    psrc += stride;
                    ((double*)pblock_new)[2] = ((double*)psrc)[0];
                    psrc += stride;
                    ((double*)pblock_new)[3] = ((double*)psrc)[0];
                    psrc += stride;
                    pblock_new += 8;
                }
                break;
            }
        }
    }

}



/*
 * Copy pixel data (shorts) from blocks to a history raster.
 */
void putblocks(Xil_unsigned8 *yp, Xil_unsigned8 *up, Xil_unsigned8 *vp,
                Xil_unsigned8 *blold[6],
                int yyad, int uvad, int width)
{
    int        i,j;

    /*
    * We know that we can do double loads and stores in both src and dest.
    */

    int width_in_dbls = width >> 3;
    double* pdest = (double*) (yp + yyad);
    double* pblock0 = (double*) blold[0];
    double* pblock1 = (double*) blold[1];

    j = 1;
    do {
        /*
        * Write out 2  Y blocks at a time (the blocks are adjacent in the X
         * direction.)
        * Each time through the loop writes 16 pixels from each block.
        */
        for(i=0;i<4;i++) {
            pdest[0] = pblock0[0];
            pdest[1] = pblock1[0];
            pdest += width_in_dbls;

            pdest[0] = pblock0[1];
            pdest[1] = pblock1[1];
            pdest += width_in_dbls;

            pblock0 += 2;
            pblock1 += 2;
        }

        if(j == 0)
            break;
        /*
        * Set up for next 2 Y blocks block
        */
        j = 0;
        pblock0 = (double*) blold[2];
        pblock1 = (double*) blold[3];
    } while(1);


    /*
    * Write U and V blocks
    */
    double* pblockuv = (double*) blold[4];
    width_in_dbls >>= 1;
    pdest = (double*) (up + uvad);
    j = 1;

    do {
        pdest[0] = pblockuv[0];
        pdest += width_in_dbls;
        pdest[0] = pblockuv[1];
        pdest += width_in_dbls;
        pdest[0] = pblockuv[2];
        pdest += width_in_dbls;
        pdest[0] = pblockuv[3];
        pdest += width_in_dbls;
        pdest[0] = pblockuv[4];
        pdest += width_in_dbls;
        pdest[0] = pblockuv[5];
        pdest += width_in_dbls;
        pdest[0] = pblockuv[6];
        pdest += width_in_dbls;
        pdest[0] = pblockuv[7];

        if(j == 0)
            break;
        /*
        * Set up for U block
        */
        j = 0;
        pblockuv = (double*) blold[5];
        pdest = (double*) (vp + uvad);
    } while(1);
}

/*
 * Copy pixel data (shorts) from one history raster to another.
 */
void copyblocks(Xil_unsigned8 *ysrc, Xil_unsigned8 *usrc, Xil_unsigned8 *vsrc,
                Xil_unsigned8 *ydst, Xil_unsigned8 *udst, Xil_unsigned8 *vdst,
                int yyad, int uvad, int width)
{
    int        j;
    int index;

    /*
    * We know that we can do double loads and stores in both src and dest.
    */

    int width_in_dbls = width >> 3;
    int increment = width_in_dbls - 1;
    double* psrc = (double*) (ysrc + yyad);
    double* pdest = (double*) (ydst + yyad);

    /*
    * Write out 2  Y blocks at a time (the blocks are adjacent in the X
     * direction.)
    */
#define WRITE16_PIXELS_IN_X        \
    pdest[index] = psrc[index]; \
    index += 1;                        \
    pdest[index] = psrc[index]; \
    index += increment;

    j = 1;
    index = 0;
    do {
        WRITE16_PIXELS_IN_X
        WRITE16_PIXELS_IN_X
        WRITE16_PIXELS_IN_X
        WRITE16_PIXELS_IN_X

        WRITE16_PIXELS_IN_X
        WRITE16_PIXELS_IN_X
        WRITE16_PIXELS_IN_X
        WRITE16_PIXELS_IN_X
    } while(j-- > 0);


    /*
    * Write U and V blocks
    */
    width_in_dbls >>= 1;
    psrc = (double*) (usrc + uvad);
    pdest = (double*) (udst + uvad);

#define WRITE8_PIXELS_IN_X        \
    pdest[index] = psrc[index]; \
    index += width_in_dbls;

    j = 1;
    index = 0;
    do {
        WRITE8_PIXELS_IN_X
        WRITE8_PIXELS_IN_X
        WRITE8_PIXELS_IN_X
        WRITE8_PIXELS_IN_X
        WRITE8_PIXELS_IN_X
        WRITE8_PIXELS_IN_X
        WRITE8_PIXELS_IN_X
        WRITE8_PIXELS_IN_X

        if(j == 0)
            break;
        /*
        * Set up for U block
        */
        j = 0;
        psrc = (double*) (vsrc + uvad);
        pdest = (double*) (vdst + uvad);
        index = 0;
    } while(1);
}
