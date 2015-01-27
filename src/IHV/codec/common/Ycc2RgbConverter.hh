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
//  File:       Ycc2RgbConverter.hh
//  Project:    XIL
//  Revision:   1.7
//  Last Mod:   10:23:52, 03/10/00
//
//  Description:
//
//    Class definition for the Ycc2RgbConverter object
//
//
//------------------------------------------------------------------------
//    COPYRIGHT
//------------------------------------------------------------------------
#pragma ident   "@(#)Ycc2RgbConverter.hh	1.7\t00/03/10  "

#ifndef _YCC2RGBCONVERTER_HH_
#define _YCC2RGBCONVERTER_HH_

#include <xil/xilGPI.hh>

#define IDCT_FRAC_BITS  6

#define MIN_CCIR_YUV    16
#define MAX_CCIR_Y      235
#define MAX_CCIR_UV     240

class Ycc2RgbConverter {
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

    void        cvtBlock422(Xil_signed16*  yBlk,
                            Xil_signed16*  cbBlk,
                            Xil_signed16*  crBlk,
                            unsigned int   ysrc_ss,
                            unsigned int   cbsrc_ss,
                            unsigned int   crsrc_ss,
                            Xil_unsigned8* dst,
                            unsigned int   dst_ps,
                            unsigned int   dst_ss);

    void        cvtBlock(Xil_unsigned8* yBlk,
                         Xil_unsigned8* cbBlk,
                         Xil_unsigned8* crBlk,
                         unsigned int   ysrc_ss,
                         unsigned int   cbsrc_ss,
                         unsigned int   crsrc_ss,
                         Xil_unsigned8* dst,
                         unsigned int   dst_ps,
                         unsigned int   dst_ss);

    void        cvtMacroBlock(Xil_signed16*  yBlk,
                              Xil_signed16*  cbBlk,
                              Xil_signed16*  crBlk,
                              unsigned int   y_ss,
                              unsigned int   cb_ss,
                              unsigned int   cr_ss,
                              Xil_unsigned8* dst,
                              unsigned int   dst_ps,
                              unsigned int   dst_ss);

    void        cvtMacroBlock422(Xil_signed16*  yBlk,
                                 Xil_signed16*  cbBlk,
                                 Xil_signed16*  crBlk,
                                 unsigned int   y_ss,
                                 unsigned int   cb_ss,
                                 unsigned int   cr_ss,
                                 Xil_unsigned8* dst,
                                 unsigned int   dst_ps,
                                 unsigned int   dst_ss);

    void        cvtMacroBlock(Xil_unsigned8* yBlk,
                              Xil_unsigned8* cbBlk,
                              Xil_unsigned8* crBlk,
                              unsigned int   y_ss,
                              unsigned int   cb_ss,
                              unsigned int   cr_ss,
                              Xil_unsigned8* dst,
                              unsigned int   dst_ps,
                              unsigned int   dst_ss);

    void        cvtCellB(Xil_unsigned8  y0,
                         Xil_unsigned8  y1,
                         Xil_unsigned8  u,
                         Xil_unsigned8  v,
                         Xil_unsigned16 mask,
                         Xil_unsigned8* dst,
                         unsigned int   dst_ps,
                         unsigned int   dst_ss);

    Xil_boolean isOK();

    Ycc2RgbConverter(int tbl_offset);
    ~Ycc2RgbConverter();

private:
    int*           table_buffer;
    int*           yTable;
    int*           bluTable;
    int*           redTable;
    Xil_unsigned8* clamp;
    unsigned int   ref_count;

    Xil_boolean    isOKFlag;
};

#endif // _YCC2RGBCONVERTER_HH_
