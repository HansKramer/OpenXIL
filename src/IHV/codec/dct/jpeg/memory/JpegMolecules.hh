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
//  File:       JpegMolecules.hh
//  Project:    XIL
//  Revision:   1.3
//  Last Mod:   10:22:56, 03/10/00
//
//  Description:
//
//    Routines to support Jpeg Molecules
//
//
//------------------------------------------------------------------------
//    COPYRIGHT
//------------------------------------------------------------------------
#pragma ident   "@(#)JpegMolecules.hh	1.3\t00/03/10  "

#include <xil/xilGPI.hh>

#define INPUT_MAGNITUDE 255
#define IDCT_FRAC_BITS  6

//
// (clamped) YUV values are in the range -128 to 127 after inverse DCT.
// Offset to 0 to 255
//
#define CODING_OFFSET   128

#define MIN_CCIR_YUV        16
#define MAX_CCIR_Y        235
#define MAX_CCIR_UV        240

class JpegYcc2Rgb {
public:
    void        cvtBlock(Xil_signed16*  yBlk,
                         Xil_signed16*  cbBlk,
                         Xil_signed16*  crBlk,
                         unsigned int   ysrc_ss,
                         unsigned int   cbsrc_ss,
                         unsigned int   crsrc_ss,
                         Xil_unsigned8* dst,
                         unsigned int   dst_ps,
                         unsigned int   dst_ss);

    Xil_boolean isOK();

    JpegYcc2Rgb();
    ~JpegYcc2Rgb();

private:
    static int*           table_buffer;
    static int*           yTable;
    static int*           bluTable;
    static int*           redTable;
    static Xil_unsigned8* clamp;
    static unsigned int   ref_count;

    Xil_boolean    isOKFlag;
    XilMutex       mutex;
};

