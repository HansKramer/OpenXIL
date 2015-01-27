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
//  File:   encodeCell.cc
//  Project:    XIL
//  Revision:   1.3
//  Last Mod:   10:15:36, 03/10/00
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
#pragma ident   "@(#)encodeCell.cc	1.3\t00/03/10  "

#include "XilDeviceCompressionCellB.hh"
#include "XilDeviceManagerCompressionCellB.hh"
#include "CellBManagerCompressorData.hh"


// Note that we are using YVAL <= ymean instead of YVAL < mean as lm 
// originally did.  This gives better results since ymean is truncated
// and thus is likely to be ymean + a fraction.
#define SetMaskBit(YVAL, BIT)   \
    if ((YVAL) <= ymean) {    \
    ylo += (YVAL);        \
    tmp += 1;        \
    } else {            \
    yhi += (YVAL);        \
    mask += (1 << (BIT));    \
    }

#ifdef XIL_LITTLE_ENDIAN
#define BYTE3(QVAL) (QVAL) & 0xff
#define BYTE2(QVAL) ((QVAL) >> 8) & 0xff
#define BYTE1(QVAL) ((QVAL) >> 16) & 0xff
#define BYTE0(QVAL) ((QVAL) >> 24)
#else
#define BYTE0(QVAL) (QVAL) & 0xff
#define BYTE1(QVAL) ((QVAL) >> 8) & 0xff
#define BYTE2(QVAL) ((QVAL) >> 16) & 0xff
#define BYTE3(QVAL) ((QVAL) >> 24)
#endif

// If you really need speed, you can include this file and then
// define ENCODECELL_INLINE and let the optimizer do it\'s work.

#ifdef ENCODECELL_INLINE
inline
#endif
int
XilDeviceCompressionCellB::encodeCell(register Xil_unsigned32 ymean,
                                      register Xil_unsigned32 uvmean,
                                      register Xil_unsigned32 q0,
                                      register Xil_unsigned32 q1,
                                      register Xil_unsigned32 q2,
                                      register Xil_unsigned32 q3)
{
  //
  //    Note that this code is stolen almost directly from Leonard\'s
  //      original broadcast program.
  // 
  //     Also, I must apologize for the apparent
  //     terseness of the code, the reason it is written in this
  //     one-op-per-line-of-code style is to control the code
  //     generation of the compiler and too allow for easy conversion
  //     to assembly code if needed in the future. So each line
  //     of C-code roughly corresponds to a single SPARC instruction
  //
  //     ..... Here is the beginning of the second part of the
  //     cell encoding loop. first the value of the u and v means
  //     for the 4 x 4 tile are quantized using the uvremap[ ]
  //     lookup table. This table is indexed by the most significant
  //     6 bits of Umean and the most significant 6 bits of Vmean.
  //     The value returns is a single byte codeword which represents
  //     the quantized U/V vector.
  // 
  //     .... generate chroma part ....
    
    register unsigned int ylo, yhi, tmp, mask, yval;
    XilDeviceManagerCompressionCellB*  ct =
        (XilDeviceManagerCompressionCellB*)mgr;

    CellBManagerCompressorData* tables = &ct->compmgrData;
    Xil_unsigned8** divtable       = (Xil_unsigned8**)tables->divtable;

    tmp = uvmean >> 22;
    tmp <<= 6;
    uvmean <<= 20;
    uvmean >>= 26;
    tmp += uvmean;
    uvmean = tables->uvremap[tmp];

  //    .... next the bit mask for the tile is generated. for
  //    the first fifteen bits this is done by comparing each
  //    luminance value to the mean. if that value is greater
  //    than the mean the correnponding bit in the mask is set,
  //    otherwise the counter for the number of zeros in the mask,
  //    tmp, is incremented. This is handled by the SetMaskBit
  //    macro. also, the values of those pixels greater than the
  //    mean are accumulated in yhi, and the value of those less
  //    than the mean are accumulted in ylo ....

    mask = 0;
    ylo = 0;

    yhi = 0;
    tmp = 0;
    
    yval = BYTE0(q3);
    SetMaskBit(yval, 0);
    yval = BYTE1(q3);
    SetMaskBit(yval, 1);
    yval = BYTE2(q3);
    SetMaskBit(yval, 2);
    yval = BYTE3(q3);
    SetMaskBit(yval, 3);
    
    yval = BYTE0(q2);
    SetMaskBit(yval, 4);
    yval = BYTE1(q2);
    SetMaskBit(yval, 5);
    yval = BYTE2(q2);
    SetMaskBit(yval, 6);
    yval = BYTE3(q2);
    SetMaskBit(yval, 7);
    
    yval = BYTE0(q1);
    SetMaskBit(yval, 8);
    yval = BYTE1(q1);
    SetMaskBit(yval, 9);
    yval = BYTE2(q1);
    SetMaskBit(yval, 10);
    yval = BYTE3(q1);
    SetMaskBit(yval, 11);
    
    yval = BYTE0(q0);
    SetMaskBit(yval, 12);
    yval = BYTE1(q0);
    SetMaskBit(yval, 13);
    yval = BYTE2(q0);
    SetMaskBit(yval, 14);
    yval = BYTE3(q0);

    //     .... the last bit of the bitmask is calculated differently.
    //     if this pixel, the one in the upper-left corner is less than
    //     the mean, the last bit is calculated more-or-less normally.
    //     however, if this last pixel is greater or equal to the mean
    //     the mask is complemented and the meaning of ylo and yhi are
    //     reversed.... at this point the mean value of the hi and lo
    //     pixels are calulated this is accomplished by using the division
    //     lookup table. this table is index by the value of the accumulated
    //     pixels and the number of pixel contributing to that sum ....

    if (yval <= ymean) {
      ylo += yval;
      tmp += 1;
      ylo = divtable[tmp][ylo];
      // If all values are equal to the mean, then we get a better
      // color by using Ylo, Ylo, then by using 0, Ylo for our YYremap
      if (mask == 0)
        yhi = ylo;
      else {
        tmp = 16 - tmp;
        yhi = divtable[tmp][yhi];
        // If they are really close, we want to just munge them together
        if (yhi - ylo < 3){
          yhi = (yhi + ylo) >> 1;
          ylo = yhi;
          mask = 0;
        }
      }
      ylo <<= 8;
    } else {
      yhi += yval;
      ylo = divtable[tmp][ylo];
      tmp = 16 - tmp;
      yhi = divtable[tmp][yhi];
      mask ^= 0x7fff;
      // If they are really close, we want to just munge them together
      if (yhi - ylo < 3){
        yhi = (yhi + ylo) >> 1;
        ylo = yhi;
        mask = 0;
      }
      yhi <<= 8;
    }

    //     .... the high mean and low mean values are finally quantized
    //     by the yyremap[ ] table, the Y/Y vector codeword byte, the
    //     U/V codeword byte and the bitmask are then assembled into
    //     a Normal Cell bytecode and returned form the routine ....

    yhi += ylo;
    yval = tables->yyremap[yhi];

#if 0
    yval << 24;
    uvmean << 16;
    mask = ((mask & 0xff) << 8) | (mask >> 8);
    mask = yval | uvmean | mask;
#endif

    mask <<= 16;
    uvmean <<= 8;
    mask += uvmean;
    mask += yval;

    return (mask);
}





