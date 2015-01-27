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
//  File:       Blocks.cc
//  Project:    XIL
//  Revision:   1.4
//  Last Mod:   10:15:09, 03/10/00
//
//  Description:
//
//    Extract motion-compensated blocks from a history buffer
//
//
//------------------------------------------------------------------------
//    COPYRIGHT
//------------------------------------------------------------------------
#pragma ident   "@(#)Blocks.cc	1.4\t00/03/10  "


#include "Blocks.hh"
#include "Idct.hh"


#ifdef XIL_LITTLE_ENDIAN

#define SHIFT2        8
#define SHIFT3        16

#else

#define SHIFT2        16
#define SHIFT3        8

#endif

#define CLAMP1(value,saved_values) \
    if(value >> 8) { \
    if(value < 0) \
    saved_values = 0; \
    else \
    ASSIGN_FIRST_BYTE(saved_values,255); \
} \
    else \
        ASSIGN_FIRST_BYTE(saved_values,value); \

#define CLAMP4(value,saved_values) \
    if(value >> 8) { \
    if(value > 255) \
    ASSIGN_FOURTH_BYTE(saved_values,255); \
} \
    else \
        ASSIGN_FOURTH_BYTE(saved_values,value);



#define CLAMP2_3(value,saved_values,shift) \
    if(value >> 8) { \
    if(value > 255) \
    saved_values |= 255<<shift; \
} \
    else \
        saved_values |= value<<shift;


void 
getfilterblocks(Xil_unsigned8* yp, 
                Xil_unsigned8* up, 
                Xil_unsigned8* vp, 
                Xil_signed16*  blold[6],
                Xil_unsigned8* blnew[6],
                int yyad, 
                int uvad, 
                int mvy, 
                int mvx, 
                int width, 
                int cbp)
{
    Xil_unsigned8 *row0, *row1, *row2;
    int t0, t1, t2;
    int value;

    /*
    * Compute stride and motion vectors for U and V
    */
    int uv_width = width>>1;
    int uvy, uvx;
    if(mvx < 0) {
        uvx = -((-mvx)>>1);
    } else {
        uvx = mvx>>1;
    }

    if(mvy < 0) {
        uvy = -((-mvy)>>1);
    } else {
        uvy = mvy>>1;
    }

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

    for(int k=0; k < 6; k++) {
        Xil_unsigned32* pblock_new = (Xil_unsigned32*)blnew[k];
        Xil_unsigned8* psrc = src_start[k];
        if(k == 4)
        stride = uv_width;
        if((cbp >> (5-k)) & 1) {
            /*
            * There are DCT coefficients for this block, so the destination
            * block already has valid data in it.  We need to add in the
            * filtered predictor and clamp the result
            */

            short* pblock_old = blold[k];
            row1 = psrc;
            Xil_unsigned32 saved_values;
            row2 = psrc + stride;
            t0 = row1[0];
            t1 = row1[1];
            value = t0 + pblock_old[0];
            CLAMP1(value,saved_values)
            t2 = row1[2];
            value = pblock_old[1] + ((t0 + t1 + t1 + t2 + 2) >> 2);
            CLAMP2_3(value,saved_values,SHIFT2)
            t0 = row1[3];
            value = pblock_old[2] + ((t1 + t2 + t2 + t0 + 2) >> 2);
            CLAMP2_3(value,saved_values,SHIFT3)
            t1 = row1[4];
            value = pblock_old[3] + ((t2 + t0 + t0 + t1 + 2) >> 2);
            CLAMP4(value,saved_values)
            pblock_new[0] = saved_values;
            t2 = row1[5];
            value = pblock_old[4] + ((t0 + t1 + t1 + t2 + 2) >> 2);
            CLAMP1(value,saved_values)
            t0 = row1[6];
            value = pblock_old[5] + ((t1 + t2 + t2 + t0 + 2) >> 2);
            CLAMP2_3(value,saved_values,SHIFT2)
            t1 = row1[7];
            value = pblock_old[6] + ((t2 + t0 + t0 + t1 + 2) >> 2);
            CLAMP2_3(value,saved_values,SHIFT3)
            value = pblock_old[7] + t1;
            CLAMP4(value,saved_values)
            pblock_new[1] = saved_values;
            pblock_new += 2;
            pblock_old += 8;

            for(int y = 2; y < 8; y++) {
                row0 = row1;
                row1 = row2;
                row2 = row1 + stride;
                t0 = row0[0] + (row1[0]<<1) + row2[0];
                t1 = row0[1] + (row1[1]<<1) + row2[1];
                value = pblock_old[0] + ((t0 + 2) >> 2);
                CLAMP1(value,saved_values)
                t2 = row0[2] + (row1[2]<<1) + row2[2];
                value = pblock_old[1] + ((t0 + t1 + t1 + t2 + 8) >> 4);
                CLAMP2_3(value,saved_values,SHIFT2)
                t0 = row0[3] + (row1[3]<<1) + row2[3];
                value = pblock_old[2] + ((t1 + t2 + t2 + t0 + 8) >> 4);
                CLAMP2_3(value,saved_values,SHIFT3)
                t1 = row0[4] + (row1[4]<<1) + row2[4];
                value = pblock_old[3] + ((t2 + t0 + t0 + t1 + 8) >> 4);
                CLAMP4(value,saved_values)
                pblock_new[0] = saved_values;
                t2 = row0[5] + (row1[5]<<1) + row2[5];
                value = pblock_old[4] + ((t0 + t1 + t1 + t2 + 8) >> 4);
                CLAMP1(value,saved_values)
                t0 = row0[6] + (row1[6]<<1) + row2[6];
                value = pblock_old[5] + ((t1 + t2 + t2 + t0 + 8) >> 4);
                CLAMP2_3(value,saved_values,SHIFT2)
                t1 = row0[7] + (row1[7]<<1) + row2[7];
                value = pblock_old[6] + ((t2 + t0 + t0 + t1 + 8) >> 4);
                CLAMP2_3(value,saved_values,SHIFT3)
                value = pblock_old[7] + ((t1 + 2) >> 2);
                CLAMP4(value,saved_values)
                pblock_new[1] = saved_values;
                pblock_old += 8;
                pblock_new += 2;
            }

            t0 = row2[0];
            t1 = row2[1];
            value = pblock_old[0] = t0;
            CLAMP1(value,saved_values)
            t2 = row2[2];
            value = pblock_old[1] + ((t0 + t1 + t1 + t2 + 2) >> 2);
            CLAMP2_3(value,saved_values,SHIFT2)
            t0 = row2[3];
            value = pblock_old[2] + ((t1 + t2 + t2 + t0 + 2) >> 2);
            CLAMP2_3(value,saved_values,SHIFT3)
            t1 = row2[4];
            value = pblock_old[3] + ((t2 + t0 + t0 + t1 + 2) >> 2);
            CLAMP4(value,saved_values)
            pblock_new[0] = saved_values;
            t2 = row2[5];
            value = pblock_old[4] + ((t0 + t1 + t1 + t2 + 2) >> 2);
            CLAMP1(value,saved_values)
            t0 = row2[6];
            value = pblock_old[5] + ((t1 + t2 + t2 + t0 + 2) >> 2);
            CLAMP2_3(value,saved_values,SHIFT2)
            t1 = row2[7];
            value = pblock_old[6] + ((t2 + t0 + t0 + t1 + 2) >> 2);
            CLAMP2_3(value,saved_values,SHIFT3)
            value = pblock_old[7] + t1;
            CLAMP4(value,saved_values)
            pblock_new[1] = saved_values;

        } else {
            Xil_unsigned32 saved_dst;
            /*
            * Use filter without a corrector
            */
            row1 = &psrc[0];
            row2 = &psrc[stride];
            t0 = row1[0];
            t1 = row1[1];
            ASSIGN_FIRST_BYTE(saved_dst,t0);
            t2 = row1[2];
            ASSIGN_SECOND_BYTE(saved_dst,((t0 + t1 + t1 + t2 + 2) >> 2));
            t0 = row1[3];
            ASSIGN_THIRD_BYTE(saved_dst,((t1 + t2 + t2 + t0 + 2) >> 2));
            t1 = row1[4];
            ASSIGN_FOURTH_BYTE(saved_dst,((t2 + t0 + t0 + t1 + 2) >> 2));
            pblock_new[0] = saved_dst;
            t2 = row1[5];
            ASSIGN_FIRST_BYTE(saved_dst,((t0 + t1 + t1 + t2 + 2) >> 2));
            t0 = row1[6];
            ASSIGN_SECOND_BYTE(saved_dst,((t1 + t2 + t2 + t0 + 2) >> 2));
            t1 = row1[7];
            ASSIGN_THIRD_BYTE(saved_dst,((t2 + t0 + t0 + t1 + 2) >> 2));
            ASSIGN_FOURTH_BYTE(saved_dst,t1);
            pblock_new[1] = saved_dst;
            pblock_new += 2;

            for(int y = 2; y < 8; y++) {
                row0 = row1;
                row1 = row2;
                row2 = row1 + stride;
                t0 = row0[0] + (row1[0]<<1) + row2[0];
                t1 = row0[1] + (row1[1]<<1) + row2[1];
                ASSIGN_FIRST_BYTE(saved_dst,((t0 + 2) >> 2));
                t2 = row0[2] + (row1[2]<<1) + row2[2];
                ASSIGN_SECOND_BYTE(saved_dst,((t0 + t1 + t1 + t2 + 8) >> 4));
                t0 = row0[3] + (row1[3]<<1) + row2[3];
                ASSIGN_THIRD_BYTE(saved_dst,((t1 + t2 + t2 + t0 + 8) >> 4));
                t1 = row0[4] + (row1[4]<<1) + row2[4];
                ASSIGN_FOURTH_BYTE(saved_dst,(t2 + t0 + t0 + t1 + 8) >> 4);
                pblock_new[0] = saved_dst;
                t2 = row0[5] + (row1[5]<<1) + row2[5];
                ASSIGN_FIRST_BYTE(saved_dst,((t0 + t1 + t1 + t2 + 8) >> 4));
                t0 = row0[6] + (row1[6]<<1) + row2[6];
                ASSIGN_SECOND_BYTE(saved_dst,((t1 + t2 + t2 + t0 + 8) >> 4));
                t1 = row0[7] + (row1[7]<<1) + row2[7];
                ASSIGN_THIRD_BYTE(saved_dst,((t2 + t0 + t0 + t1 + 8) >> 4));
                ASSIGN_FOURTH_BYTE(saved_dst,(t1 + 2) >> 2);
                pblock_new[1] = saved_dst;
                pblock_new += 2;
            }

            t0 = row2[0];
            t1 = row2[1];
            ASSIGN_FIRST_BYTE(saved_dst,t0);
            t2 = row2[2];
            ASSIGN_SECOND_BYTE(saved_dst,((t0 + t1 + t1 + t2 + 2) >> 2));
            t0 = row2[3];
            ASSIGN_THIRD_BYTE(saved_dst,((t1 + t2 + t2 + t0 + 2) >> 2));
            t1 = row2[4];
            ASSIGN_FOURTH_BYTE(saved_dst,(t2 + t0 + t0 + t1 + 2) >> 2);
            t2 = row2[5];
            pblock_new[0] = saved_dst;
            ASSIGN_FIRST_BYTE(saved_dst,((t0 + t1 + t1 + t2 + 2) >> 2));
            t0 = row2[6];
            ASSIGN_SECOND_BYTE(saved_dst,((t1 + t2 + t2 + t0 + 2) >> 2));
            t1 = row2[7];
            ASSIGN_THIRD_BYTE(saved_dst,((t2 + t0 + t0 + t1 + 2) >> 2));
            ASSIGN_FOURTH_BYTE(saved_dst,t1);
            pblock_new[1] = saved_dst;
        }
    }

}
