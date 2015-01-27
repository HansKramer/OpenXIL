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
//  File:   skipCell.cc
//  Project:    XIL
//  Revision:   1.3
//  Last Mod:   10:15:35, 03/10/00
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
#pragma ident   "@(#)skipCell.cc	1.3\t00/03/10  "

#include "XilDeviceCompressionCellB.hh"
#include "XilDeviceManagerCompressionCellB.hh"

#define STATES 23

static int nextindex = 0;

static Xil_unsigned8 rtable[STATES] = {
    146, 75,  3,   95,  189, 165, 106, 229,
    239, 14,  208, 90,  8,   222, 122, 236,
    200, 171, 225, 131, 94,  12,  74
};


//
//   .... randbyte( ) does not repeat after 100,000,000 calls ....
//   Note that it returns an int which must be masked with & 255 to
//   really get a random byte.  The int returned is NOT random without
//   the masking. The & 255 is commented out because it is not necessary
//   in the particular places that it is used in this routine.
//
//   The randbyte routine comes from Knuth.

inline static int
randbyte(void)
{
    int i, rval;
    int *nptr = &nextindex;
    int next = *nptr;

    if ((i = next - 1) < 0)
      i = STATES - 1;

    rtable[next] = rval = (rtable[next] + rtable[i]); // & 255

    if ((next += 1) > (STATES-1)) {
      *nptr = 0;
    } else {
      *nptr = next;
    }

    return rval;
}

inline static void
CountBits(int &count, Xil_unsigned32 pattern)
{
    const register unsigned int mask5555 = 0x5555;
    const register unsigned int mask3333 = 0x3333;

    count = pattern & mask5555;
    pattern >>= 1;
    pattern &= mask5555;
    count += pattern;
    pattern = count & mask3333;
    count >>= 2;
    count &= mask3333;
    count += pattern;
    pattern = count & 0x0f0f;
    count >>= 4;
    count &= 0x0f0f;
    count += pattern;
    pattern = count & 0xff;
    count >>= 8;
    count += pattern;
}

Xil_boolean
XilDeviceCompressionCellB::skipCell(Xil_unsigned32 cell, int index)
{
    XilDeviceManagerCompressionCellB*  ct =
            (XilDeviceManagerCompressionCellB*)mgr;

    CellBManagerCompressorData* tables = &ct->compmgrData;
    error_array            &error = tables->error;

    if (compData.updateHistory[index] != 0) {
      // was unsigned int. Changed it because it overflows in
      // certain situations
      int rmask, rtmp, ctmp;

      //  .... grab the UV fields of the reference and
      //  current cells, then check if the two fields are
      //  "close". this is accomplished with a table called
      //  uvlookup[]. this table is actually a bitvector
      //  where each bit-position is set or cleared according
      //  to whether, UV(old) is close to UV(new), The
      //  heuristic for closeness is embodied by the
      //  uvclose[ ] vector which in the first version
      //  was based on euclidean distances ....
    
      rmask = compData.cellHistory[index];
      rtmp = (rmask << 16) >> 24;
      ctmp = (cell << 16) >> 24;
      rtmp = (rtmp << 5) + (ctmp >> 3);
      ctmp = ctmp & 7;
    
      if ((tables->uvlookup[rtmp] & (1 << ctmp)) != 0) {
        unsigned int cmask, pattern;
        int cy0, cy1, ry0, ry1;
        int bits, diff, count;
        //   .... if the colors were close then the
        //  total absolute luminance difference between
        //  the reference tile and the current tile is
        //  calculated ... The Y/Y vectors are first
        //  dequantized, via yytable lookups, and the
        //  difference corresponding to each of the
        //  possible bitmask differences between the
        //  reference mask and the current mask are
        //  found and multiplied by the number of
        //  occurances of each particular bit patterns.
        //
        //        Reference    Current
        //        Tile Bit    Tile Bit
        //        0        0      |ry0 - cy0|
        //        0        1      |ry0 - cy1|
        //        1        0      |ry1 - cy0|
        //        1        1      |ry1 - cy1|
        //
        //  The multiplys and absolute value of the
        //  luminance differences are performed by a
        //  table lookup. the number of bits corresponding
        //  to a given condition is computed by the
        //  CountBits( ) macro, which returns the value
        //  in "count". Note: that the 0/0 condition is
        //  not actually computed but deduced from the
        //  number of bits remaining after the 1/1,
        //  0/1, and 1/0 conditions are counted....
        //  
        //  .... dequantize Y/Y values and get the
        //  luminance values ....
        
        cy0 = cell & 255;
        ry0 = rmask & 255;
        cmask = cell >> 16;
        rmask = rmask >> 16;
        cy0 = tables->yytable[cy0];
        ry0 = tables->yytable[ry0];
        cy1 = cy0 & 255;
        ry1 = 256 - (ry0 & 255);
        cy0 = cy0 >> 8;
        ry0 = 256 - (ry0 >> 8);
        
        bits = 16;
        
        //  .... find the 1/1 bits ....
        pattern = cmask & rmask;
        CountBits(count, pattern);
        bits -= count;
        diff = error[count][cy1 + ry1];
        
        //  .... find the 1/0 bits ....
        pattern = cmask & ~rmask;
        CountBits(count, pattern);
        bits -= count;
        diff += error[count][cy1 + ry0];
        
        //  .... find the 0/1 bits ....
        pattern = ~cmask & rmask;
        CountBits(count, pattern);
        bits -= count;
        diff += error[count][cy0 + ry1];
        
        //  .... add in error from the 0/0 bits ....
        diff += error[bits][cy0 + ry0];
        
        //  .... if the total absolute luminance difference
        //  between the reference and the current cell are
        //  found to be less than 144 then skip the cell.
        //  this value of 144 was determined subjectively.
        //  144 corresponds to an average luminace error of
        //  9 per pixel which in most cases was barely
        //  visually noticable, and in typical cases, where
        //  the camera postion is fixed, the subject is agianist
        //  a static background and ocuppys approximately
        //  50% of the displayed pixels, this threshold
        //  allowed ~80% of all cells to be skipped, with
        //  very little observable difference verses sending
        //  all cells ....
        
        if (diff < 144) {
          compData.updateHistory[index] -= 1;
          return TRUE;
        }
      }
    }

    compData.cellHistory[index] = cell;
    compData.updateHistory[index] = (randbyte() & 7) + 8; // maximum skip of 15

    return FALSE;
}



