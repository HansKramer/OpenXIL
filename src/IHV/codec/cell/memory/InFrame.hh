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
//  File:   InFrame.hh
//  Project:    XIL
//  Revision:   1.4
//  Last Mod:   10:23:27, 03/10/00
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
#pragma ident   "@(#)InFrame.hh	1.4\t00/03/10  "

#ifndef INFRAME_HH
#define INFRAME_HH

#include <xil/xilGPI.hh>
#include "ColorValue.hh"

class InFrame {
public:
  InFrame(void);
  
  int  useNewImage(XilSystemState* state,
                   unsigned int   imageWidth,
                   unsigned int   imageHeight,
                   unsigned int   num_bands,
                   unsigned int   pixel_stride,
                   unsigned int   scanline_stride,
                   Xil_unsigned8* buffer,
                   XilColorspace* colorspace);

  int  getYUVBlock(ColorValue* block);
  int  getRGBBlock(ColorValue* block);
  
protected:
    XilSystemState* systemState;

    Xil_unsigned8*  data;

    unsigned int    cur_y, cur_x, scan_stride;
    unsigned int    line_length;
    unsigned int    bands, height, width;
    unsigned int    pixel_stride;
    Xil_boolean     yuvimage;

private:
    int   Yr[256];
    int   Yg[256];
    int   Yb[256];

    int   Ur[256];
    int   Ug[256];
    int   Ub[256];

    int   Vr[256];
    int   Vg[256];
    int   Vb[256];

    Xil_unsigned8  rgb2yTable(Xil_unsigned8 r,
                              Xil_unsigned8 g,
                              Xil_unsigned8 b);
    Xil_unsigned8  rgb2uTable(Xil_unsigned8 r,
                              Xil_unsigned8 g,
                              Xil_unsigned8 b);
    Xil_unsigned8  rgb2vTable(Xil_unsigned8 r,
                              Xil_unsigned8 g,
                              Xil_unsigned8 b);
};

#endif  // INFRAME_HH
