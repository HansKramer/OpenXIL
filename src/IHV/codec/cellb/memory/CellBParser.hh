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
//  File:   CellBParser.hh
//  Project:    XIL
//  Revision:   1.2
//  Last Mod:   10:23:24, 03/10/00
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
#pragma ident   "@(#)CellBParser.hh	1.2\t00/03/10  "

#ifndef CELLBPARSER_H
#define CELLBPARSER_H

#include "XilDeviceCompressionCellB.hh"
#include "CellBDecompressorData.hh"
#include "CellBFrame.hh"

class CellBParser 
{
public:
    // set history_buffer_valid if you have a decoded history buffer
    // such that you can just skip over skip codes.  Otherwise we will
    // return the CellB from the frame.
    // Stride3 is the distance from the 1st pixel on the one row to the
    // first pixel on the next MINUS the length of the first row. (i.e.
    // 4 * stride - blanking.
    CellBParser(XilDeviceCompressionCellB* dc,
                void*                      dest,
                Xil_boolean                history_buffer_valid,
                unsigned long              dst_cellb_stride, 
                unsigned long              pixel_stride);

    Xil_boolean ok() { return isOKFlag; }

// We would really like to have all the following things private.
// and getNextCell to be an inline function.  Unfortunately, with
// the limitations on inline functions, we are forced to resort
// to a macro instead (for speed).  We had lost around 6-7 fps
// by calling a function.
#if 0
    void*  getNextCell(CellB &cellb);  // returns NULL at end of frame
private:
#endif

    CellBFrame* cellb_frame;

    XilCisBufferManager* cbm;

    Xil_unsigned8 *bp;

    int width;
    int height;
    int row;
    int col;
    int count;

    Xil_boolean history_buffer;

    Xil_unsigned8* dest;

    unsigned long cellb_stride;
    unsigned long stride3;

    Xil_boolean done;

    CellB* cur_cellb;

private:
    Xil_boolean isOKFlag;

};

// If we are handling a skipcode but we don't trust the history buffer,
// then we get all our information from the cellb_frame.  count is used
// as a flag to indicate how many times more we need to process this byte.
// If count <= 0, then this is the first time we've seen this byte.

#define GETNEXTCELL(dst,dsttype,parse,cellb,label)         \
{                                   \
    register Xil_unsigned8* bp = parse.bp;                      \
    if (parse.done) {                        \
    parse.cbm->decompressedFrame(bp,0);            \
    dst = (dsttype)NULL;                    \
    goto label##end;                        \
    } else {                            \
    unsigned int pattern = bp[0];                \
    if (pattern < SKIPCODE) goto label##nonskip;        \
    if (!parse.history_buffer) {                \
        if (parse.count <= 1) {                \
        if (parse.count <= 0)                           \
            parse.count = pattern - (SKIPCODE - 1);    \
            if (--parse.count <= 0)                \
            bp++;                    \
        } else {                                            \
                --parse.count;                     \
        }                            \
    } else {                        \
                                \
            do {                        \
        bp++;                        \
        unsigned int temp;                \
        temp  = pattern - (SKIPCODE - 1);        \
        parse.dest += parse.cellb_stride * temp;    \
        parse.col  += temp;                \
        while (parse.col >= parse.width) {        \
            if (++parse.row >= parse.height) {        \
            parse.cbm->decompressedFrame(bp,0);    \
            dst = (dsttype)NULL;            \
               goto label##end;            \
            }                        \
            parse.col -= parse.width;            \
            parse.dest += parse.stride3;        \
        }                        \
                                \
        pattern = bp[0];                \
        } while (pattern >= SKIPCODE);            \
                                \
        parse.cur_cellb = &parse.cellb_frame->cellB(parse.row,parse.col); \
                                \
label##nonskip:                            \
        pattern = (pattern<<24) | (bp[1]<<16) | (bp[2]<<8) | bp[3]; \
        parse.cur_cellb->setCellB(pattern);            \
        bp+=4;                        \
    }                            \
                                \
    cellb = *parse.cur_cellb;                \
    dst = (dsttype)parse.dest;                \
                                                            \
    parse.dest += parse.cellb_stride;            \
    if (++parse.col >= parse.width) {            \
        parse.col = 0;                    \
        if (++parse.row >= parse.height)            \
        parse.done = TRUE;                \
        parse.cur_cellb = (*parse.cellb_frame)[parse.row];    \
        parse.dest+= parse.stride3;                \
    } else {                        \
        parse.cur_cellb++;                    \
    }                            \
    parse.bp = bp;                        \
    }                                \
}                                \
label##end:

#endif 


