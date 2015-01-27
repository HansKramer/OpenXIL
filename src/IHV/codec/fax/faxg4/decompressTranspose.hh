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
//  File:       decompressTranspose.hh
//  Project:    XIL
//  Revision:   1.2
//  Last Mod:   10:22:44, 03/10/00
//
//  Description:
//
//    Horrible macros for the faxG4 decompressTranspose molecule
//
//
//------------------------------------------------------------------------
//    COPYRIGHT
//------------------------------------------------------------------------
#pragma ident   "@(#)decompressTranspose.hh	1.2\t00/03/10  "

#define VSTRIP(INC, B0, B1, B2, B3, B4, B5, B6, B7)        \
       (int                *runs[8],                        \
        int                 height,                        \
        unsigned char        *dst,                                \
        unsigned int          dststride)                        \
    {                                                        \
        unsigned int        byte=0;                                \
        int        *p0, *p1, *p2, *p3, *p4, *p5, *p6, *p7;        \
        int         min, r0, r1, r2, r3, r4, r5, r6, r7;        \
                                        \
        p0 = runs[0];                        \
        p1 = runs[1];                        \
        p2 = runs[2];                        \
        p3 = runs[3];                        \
        p4 = runs[4];                        \
        p5 = runs[5];                        \
        p6 = runs[6];                        \
        p7 = runs[7];                        \
                                        \
        min = r0 = *p0++;                \
        if ((r1 = *p1++) < min)                \
            min = r1;                        \
        if ((r2 = *p2++) < min)                \
            min = r2;                        \
        if ((r3 = *p3++) < min)                \
            min = r3;                        \
        if ((r4 = *p4++) < min)                \
            min = r4;                        \
        if ((r5 = *p5++) < min)                \
            min = r5;                        \
        if ((r6 = *p6++) < min)                \
            min = r6;                        \
        if ((r7 = *p7++) < min)                \
            min = r7;                        \
        dst INC min*dststride;                \
                                        \
        r0 -= min;                        \
        r1 -= min;                        \
        r2 -= min;                        \
        r3 -= min;                        \
        r4 -= min;                        \
        r5 -= min;                        \
        r6 -= min;                        \
        r7 -= min;                        \
        height -= min;                        \
                                        \
        while (height > 0) {                \
                                        \
            if (r0)                        \
                min = r0;                \
            else {                        \
                min = r0 = *p0++;        \
                byte ^= B0;                \
                }                        \
                                        \
            if (!r1) {                        \
                r1 = *p1++;                \
                byte ^= B1;                \
                }                        \
            if (r1 < min)                \
                min = r1;                \
                                        \
            if (!r2) {                        \
                r2 = *p2++;                \
                byte ^= B2;                \
                }                        \
            if (r2 < min)                \
                min = r2;                \
                                        \
            if (!r3) {                        \
                r3 = *p3++;                \
                byte ^= B3;                \
                }                        \
            if (r3 < min)                \
                min = r3;                \
                                        \
            if (!r4) {                        \
                r4 = *p4++;                \
                byte ^= B4;                \
                }                        \
            if (r4 < min)                \
                min = r4;                \
                                        \
            if (!r5) {                        \
                r5 = *p5++;                \
                byte ^= B5;                \
                }                        \
            if (r5 < min)                \
                min = r5;                \
                                        \
            if (!r6) {                        \
                r6 = *p6++;                \
                byte ^= B6;                \
                }                        \
            if (r6 < min)                \
                min = r6;                \
                                        \
            if (!r7) {                        \
                r7 = *p7++;                \
                byte ^= B7;                \
                }                        \
            if (r7 < min)                \
                min = r7;                \
                                        \
            r0 -= min;                        \
            r1 -= min;                        \
            r2 -= min;                        \
            r3 -= min;                        \
            r4 -= min;                        \
            r5 -= min;                        \
            r6 -= min;                        \
            r7 -= min;                        \
            height -= min;                \
                                        \
            if (byte)                        \
                while (min-- > 0) {        \
                    *dst = byte;        \
                    dst INC dststride;        \
                    }                        \
            else                        \
                dst INC min*dststride;        \
            }                                \
    }

void strip_d
        VSTRIP(+=, 0x80, 0x40, 0x20, 0x10, 0x8, 0x4, 0x2, 0x1)

void strip_90
        VSTRIP(-=, 0x80, 0x40, 0x20, 0x10, 0x8, 0x4, 0x2, 0x1)

void strip_270
        VSTRIP(+=, 0x1, 0x2, 0x4, 0x8, 0x10, 0x20, 0x40, 0x80)

void strip_a
        VSTRIP(-=, 0x1, 0x2, 0x4, 0x8, 0x10, 0x20, 0x40, 0x80)


#define HSTRIP(WSHIFT, BSHIFT, BYTEBUMP, SKIP, LINEBUMP)        \
       (int                *runs[8],                                \
        unsigned char        *dst,                                        \
        unsigned int          dststride,                                \
        unsigned int          n)                                        \
    {                                                                \
        unsigned char  *byte;                                        \
        unsigned int    outbits;                                \
        int                        out_ndx;                                \
        unsigned int    allblack = 0xff;                        \
        int        *runptr;                                        \
        int         runlen, i;                                        \
                                                                \
        for (i=0; i<n; i++) {                                        \
            out_ndx = 0;                                        \
            byte = dst;                                                \
            runptr = runs[i];                                        \
                                                                \
            runlen = *runptr++;                                        \
            do {                                                \
                outbits &= allblack WSHIFT (8-out_ndx);                \
                out_ndx += runlen;                                \
                if (out_ndx > 8) {                                \
                    BYTEBUMP = outbits;                                \
                    byte SKIP (out_ndx-8) >> 3;                        \
                    out_ndx &= 0x7;                                \
                    outbits = 0;                                \
                    }                                                \
                                                                \
                if (!(runlen = *runptr++))                        \
                    break;                                        \
                                                                \
                outbits |= allblack BSHIFT out_ndx;                \
                out_ndx += runlen;                                \
                while (out_ndx > 8) {                                \
                    BYTEBUMP = outbits;                                \
                    outbits = allblack;                                \
                    out_ndx -= 8;                                \
                    }                                                \
                outbits ^= allblack BSHIFT out_ndx;                \
                }                                                \
            while (runlen = *runptr++);                                \
            *byte = outbits;                /* flush last byte */        \
                                                                \
            dst LINEBUMP dststride;                                \
            }                                                        \
    }



void strip_x 
        HSTRIP(<<, >>, *byte++, +=, -=)

void strip_y 
        HSTRIP(>>, <<, *byte--, -=, +=)

void strip_180 
        HSTRIP(>>, <<, *byte--, -=, -=)


#define GETINDEX(LENGTH)                                        \
    if ((shift = 32-(LENGTH+cis_ndx)) < 0) /* bits span both */        \
        tblindex = (inbits<<-shift)|(inbits2>>(32+shift));        \
    else                                   /* all bits in 1st */\
        tblindex = (inbits >> shift);

#define GETMOREBITS                                                \
    if (cis_ndx > 31) {  /* get next word from bitstream */        \
        inbits = inbits2;                                        \
        GETWORD(inbits2, base, end)                                \
        cis_ndx -= 32;                                                \
        }

#define BUMPREF                                                        \
        extra -= runlen;                                        \
        while (extra < 0)                                        \
            extra += *refruns++;                                \
        color ^= 1;

#define EOFB4        3
