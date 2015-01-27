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
//  File:       IdctSplatter.cc
//  Project:    XIL
//  Revision:   1.4
//  Last Mod:   10:16:16, 03/10/00
//
//  Description:
//
//    TODO: Enter some descriptive text here
//
//
//------------------------------------------------------------------------
//    COPYRIGHT
//------------------------------------------------------------------------
#pragma ident   "@(#)IdctSplatter.cc	1.4\t00/03/10  "

#include "xil/xilGPI.hh"
#include "IdctSplatter.hh"
#include <string.h>

#define CSCALE        ((float) 16384.0)
#define COS_11        ((float) 0.9619397663)
#define COS_12        ((float) 0.9061274464)
#define COS_13        ((float) 0.8154931568)
#define COS_14        ((float) 0.6935199227)
#define COS_15        ((float) 0.5448951068)
#define COS_16        ((float) 0.3753302775)
#define COS_17        ((float) 0.1913417162)
#define COS_22        ((float) 0.8535533906)
#define COS_23        ((float) 0.7681777567)
#define COS_24        ((float) 0.6532814824)
#define COS_25        ((float) 0.5132799672)
#define COS_26        ((float) 0.3535533906)
#define COS_27        ((float) 0.1802399555)
#define COS_33        ((float) 0.6913417162)
#define COS_34        ((float) 0.5879378012)
#define COS_35        ((float) 0.4619397663)
#define COS_36        ((float) 0.3181896451)
#define COS_37        ((float) 0.1622116744)
#define COS_44        ((float) 0.5000000000)
#define COS_45        ((float) 0.3928474792)
#define COS_46        ((float) 0.2705980501)
#define COS_47        ((float) 0.1379496896)
#define COS_55        ((float) 0.3086582838)
#define COS_56        ((float) 0.2126075237)
#define COS_57        ((float) 0.1083863757)
#define COS_66        ((float) 0.1464466094)
#define COS_67        ((float) 0.0746578341)
#define COS_77        ((float) 0.0380602337)

IdctSplatter::IdctSplatter()
{

    int *taddr;
    int *cptr;
    int level, i;
    float *Cosine;
    float cscale, tmp;
    static float CosTable[ ] = {
        COS_44,
        COS_24, COS_46,
        COS_14, COS_34, COS_45, COS_47,
        COS_12, COS_23, COS_25, COS_27, COS_16, COS_36, COS_56, COS_67,
        COS_22, COS_26, COS_66,
        COS_11, COS_13, COS_15, COS_17, COS_33,
        COS_35, COS_37, COS_55, COS_57, COS_77
    };

    splatterOk = 1;
    cache = (CacheEntry*) new double[(CACHE_ENTRIES * sizeof(CacheEntry) +
                                    sizeof(double) - 1) / sizeof(double)];
    if(!cache) {
        splatterOk = 0;
        return;
    }
    memset((char *) cache, 0, CACHE_ENTRIES * sizeof(CacheEntry));


    //
    //  Allocate memory for Cosine table
    //
    taddr = new int[4096 * 32 + 15];
    if(!taddr) {
        splatterOk = 0;
        return;
    }
    saveptr = taddr;

    //
    //  Align the table base address on a 64-byte (16-word) boundry
    //
    cptr = (int *) ((((int)taddr) + 60) & ~63);

    //
    //  Offset into middle of table for addressing with signed offets
    //
    cosine = cptr + (32 * 2048);

    //
    //  Initialize Cosine table
    //
    cscale = CSCALE;
    Cosine = CosTable;
    for(i = 0; i < 4096; i++) {
        level = i - 2048;
        tmp = level * cscale;
        cptr[0] = (int) (Cosine[0] * tmp);
        cptr[2] = (int) (Cosine[1] * tmp);
        cptr[3] = (int) (Cosine[2] * tmp);
        cptr[4] = (int) (Cosine[3] * tmp);
        cptr[5] = (int) (Cosine[4] * tmp);
        cptr[6] = (int) (Cosine[5] * tmp);
        cptr[7] = (int) (Cosine[6] * tmp);
        cptr[8] = (int) (Cosine[7] * tmp);
        cptr[9] = (int) (Cosine[8] * tmp);
        cptr[10] = (int) (Cosine[9] * tmp);
        cptr[11] = (int) (Cosine[10] * tmp);
        cptr[12] = (int) (Cosine[11] * tmp);
        cptr[13] = (int) (Cosine[12] * tmp);
        cptr[14] = (int) (Cosine[13] * tmp);
        cptr[15] = (int) (Cosine[14] * tmp);
        cptr[16] = (int) (Cosine[15] * tmp);
        cptr[18] = cptr[17] = (int) (Cosine[16] * tmp);
        cptr[19] = (int) (Cosine[17] * tmp);
        cptr[20] = (int) (Cosine[18] * tmp);
        cptr[21] = (int) (Cosine[19] * tmp);
        cptr[22] = (int) (Cosine[20] * tmp);
        cptr[23] = (int) (Cosine[21] * tmp);
        cptr[24] = (int) (Cosine[22] * tmp);
        cptr[25] = (int) (Cosine[23] * tmp);
        cptr[26] = (int) (Cosine[24] * tmp);
        cptr[27] = (int) (Cosine[25] * tmp);
        cptr[28] = (int) (Cosine[26] * tmp);
        cptr[29] = (int) (Cosine[27] * tmp);
        cptr += 32;
    }
}

IdctSplatter::~IdctSplatter()
{
    delete saveptr;
    delete cache;
}
