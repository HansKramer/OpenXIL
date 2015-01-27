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
//  File:   encodeBTC.cc
//  Project:    XIL
//  Revision:   1.3
//  Last Mod:   10:15:56, 03/10/00
//
//  Description:
//
//
//
//
//
//
//
//
//  MT-level:  <??????>
//
//------------------------------------------------------------------------
//  COPYRIGHT
//------------------------------------------------------------------------
#pragma ident   "@(#)encodeBTC.cc	1.3\t00/03/10  "

#include <stdio.h>
#include <math.h>
#include "XilDeviceCompressionCell.hh"

//
// TODO : SquaresTables later
//
#if 0
#include "SquaresTable.h"
#endif


#define SCALE 1024
#define SCALESHIFT 10

static short table[17][4] = {
    {        0, SCALE/16,            0,            0 },
    {  SCALE/1, SCALE/15, (SCALE*15)/1, (SCALE*1)/15 },
    {  SCALE/2, SCALE/14, (SCALE*14)/2, (SCALE*2)/14 },
    {  SCALE/3, SCALE/13, (SCALE*13)/3, (SCALE*3)/13 },
    {  SCALE/4, SCALE/12, (SCALE*12)/4, (SCALE*4)/12 },
    {  SCALE/5, SCALE/11, (SCALE*11)/5, (SCALE*5)/11 },
    {  SCALE/6, SCALE/10, (SCALE*10)/6, (SCALE*6)/10 },
    {  SCALE/7,  SCALE/9,  (SCALE*9)/7,  (SCALE*7)/9 },
    {  SCALE/8,  SCALE/8,  (SCALE*8)/8,  (SCALE*8)/8 },
    {  SCALE/9,  SCALE/7,  (SCALE*7)/9,  (SCALE*9)/7 },
    { SCALE/10,  SCALE/6, (SCALE*6)/10, (SCALE*10)/6 },
    { SCALE/11,  SCALE/5, (SCALE*5)/11, (SCALE*11)/5 },
    { SCALE/12,  SCALE/4, (SCALE*4)/12, (SCALE*12)/4 },
    { SCALE/13,  SCALE/3, (SCALE*3)/13, (SCALE*13)/3 },
    { SCALE/14,  SCALE/2, (SCALE*2)/14, (SCALE*14)/2 },
    { SCALE/15,  SCALE/1, (SCALE*1)/15, (SCALE*15)/1 },
    { SCALE/16,        0,            0,            0 }
};

static int
bs_isqrt(register int value)
{
    //
    //  ensure value is in range [0,65535]
    //
    int shift = 0;
    while(value & ~0xffff) {
        value >>= 2;
        shift += 1;
    }

    int upper = 255;
    int lower = 0;
    int middle;
    while (lower < upper) {
        middle = (lower + upper + 1) >> 1;
#ifdef TODO
        // Replace after sqrs_table is in
        if (value < ((int)sqrs_table[middle]))
#endif
        if (value < ((int)(middle*middle)))
            upper = middle - 1;
        else
            lower = middle;
    }
    return (lower << shift);
}

inline int isqrt(int value) {
    //
    //  I'll use a table lookup if the square-root value is < SQRT_TABLESIZE.
    //  The table lookup is here and will in most cases handle all of the
    //  square root requests.  In the event that it doesn't, do a binary
    //  search for the number in the table containing the squares.
    //
#ifdef TODO
    // Replace when sqrt_table is in place
    return (value<SQRT_TABLESIZE)? sqrt_table[value] : bs_isqrt(value);
#endif
    return (value<8192)? sqrt(value) : bs_isqrt(value);
}    

Cell
XilDeviceCompressionCell::encodeBTC(ColorValue* block)
{
    int          mean = 0, vari = 8;
    int          mask, nlo;
    int          locolory, locoloru, locolorv;
    int          hicolory, hicoloru, hicolorv;
    Cell         out;
    short*       tptr;

    /*
        .... Collect statistics ....
    */
    int i;
    ColorValue* pCV = block;
    for(i=0; i<16; i++) {
        mean += pCV[i].band0();
#ifdef TODO
      // Replace after sqrs_table is in
        vari += sqrs_table[pCV[i].band0()];
#endif

        vari += (pCV[i].band0() * pCV[i].band0());
    }
    vari = ((int)(16.0*(float)vari - (float)mean*(float)mean)) >> 8;
    mean = (mean + 8) >> 4;

    /*
    .... generate mask and average chroma values ....
    */
    nlo = mask = 0;
    locolory = hicolory = mean;
    locoloru = hicoloru = 0;
    locolorv = hicolorv = 0;
    
    for(i=0; i<16; i+=4) {
        mask<<=4;
        if (((int)pCV[i].band0()) >= mean) {
            hicoloru += pCV[i].band1();
            hicolorv += pCV[i].band2();
            mask |= 8;
        } else {
            locoloru += pCV[i].band1();;
            locolorv += pCV[i].band2();;
            nlo++;
        }

        if (((int)pCV[i+1].band0()) >= mean) {
            hicoloru += pCV[i+1].band1();
            hicolorv += pCV[i+1].band2();
            mask |= 4;
        } else {
            locoloru += pCV[i+1].band1();
            locolorv += pCV[i+1].band2();
            nlo++;
        }
        
        if (((int)pCV[i+2].band0()) >= mean) {
            hicoloru += pCV[i+2].band1();
            hicolorv += pCV[i+2].band2();
            mask |= 2;
        } else {
            locoloru += pCV[i+2].band1();
            locolorv += pCV[i+2].band2();
            nlo++;
        }
        
        if (((int)pCV[i+3].band0()) >= mean) {
            hicoloru += pCV[i+3].band1();
            hicolorv += pCV[i+3].band2();
            mask |= 1;
        } else {
            locoloru += pCV[i+3].band1();
            locolorv += pCV[i+3].band2();
            nlo++;
        }
    }

    tptr = &table[nlo][0];
    if (nlo != 0) {
      locoloru = (locoloru * tptr[0]) >> SCALESHIFT;
      locolorv = (locolorv * tptr[0]) >> SCALESHIFT;
      locolory -= isqrt((vari * tptr[2]) >> SCALESHIFT);
      if (locolory < 16) locolory = 16;

      out.C0() =
            compData.cmapSelection.selectIndexAdaptive(locolory,
                                                       locoloru,
                                                       locolorv);
      if(mask == 0x0000) out.C1() = out.C0();
    }


    if(nlo != 16) {
      hicoloru = (hicoloru * tptr[1]) >> SCALESHIFT;
      hicolorv = (hicolorv * tptr[1]) >> SCALESHIFT;
      hicolory += isqrt((vari * tptr[3]) >> SCALESHIFT);
      if (hicolory > 235) hicolory = 235;

      out.C1() =
            compData.cmapSelection.selectIndexAdaptive(hicolory,
                                                       hicoloru,
                                                       hicolorv);
      if(mask == 0xffff) out.C0() = out.C1();
    }

    out.MASK() = mask;

    return (out);
}
