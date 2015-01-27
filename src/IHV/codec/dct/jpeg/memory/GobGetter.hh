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
//  File:       GobGetter.hh
//  Project:    XIL
//  Revision:   1.6
//  Last Mod:   10:22:48, 03/10/00
//
//  Description:
//
//        GobGetter returns Groups Of Blocks (GOBs) formatted for
//      easy access by other routines used in JPEG encoding.
//
//
//------------------------------------------------------------------------
//    COPYRIGHT
//------------------------------------------------------------------------
#pragma ident   "@(#)GobGetter.hh	1.6\t00/03/10  "

#ifndef GOBGETTER_H
#define GOBGETTER_H

#include <xil/xilGPI.hh>
#include "JpegMacros.hh"
#include "CompressInfo.hh"

#define Mheight    8
#define Mwidth     8
#define Msize      64
#define Y4UVsize   6
#define Y4UVheight 8
#define Y4UVwidth  8

#define Y0         0
#define Y1         1
#define Y2         2
#define Y3         3
#define U0         4
#define V0         5

class GobGetter {

    Xil_unsigned8* in_base;      // the in_base address
    unsigned int   in_pixel;     // the # of bytes to next pixel
    unsigned int   in_scan;      // the # of bytes to next scanline
    unsigned int   in_pixel2;    // the # of bytes to 2nd pixel
    unsigned int   in_scan2;     // the # of bytes to 2nd scanline
    unsigned int   in_bband;     // the # of bytes to next band
    unsigned int   in_nbands;    // the # of bands in the input image
    unsigned int   in_height;    // the height of input image
    unsigned int   in_width;     // the width of input image
    unsigned int   UR;           // displacement to upper right of 2x2 
    unsigned int   LL;           // displacement to lower left of 2x2 
    unsigned int   LR;           // displacement to lower right of 2x2 

public:

    GobGetter();


    //
    // Describe the storage with a storage object
    //
    void useImage(CompressInfo* ci);


    //
    // Describe the storage with explicit parameters.
    // (For use with private buffers).
    //
    void useImage(CompressInfo*  ci,
                  Xil_unsigned8* tmp_data_ptr);

    void getInterleavedGob(int px, int py, int nbands, int* gob);
    void getNonInterleavedGob(int px, int py, int band, int* gob);
    void getInterleaved411Gob(int px, int py, int* gob);

    void getBlock(int px, int py, Xil_unsigned8* srcbase, int* dst);

    void padBlock(int* block, int nx, int ny, int xsize, int ysize);

    void padX(int xvalue, int* pDst, int npad);

    void padY(int* pSrc, int* pDst, int xsize);

    void copy88(int* src, int* dst);
};

#endif
