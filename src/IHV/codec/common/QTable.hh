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
//  File:       QTable.hh
//  Project:    XIL
//  Revision:   1.3
//  Last Mod:   10:23:45, 03/10/00
//
//  Description:
//
//    Declaration of QTable Class
//
//     A QTable is a two dimensional array of elements then represent
//     quantization coefficients which are used to quantize a Block
//     of data by the Quantizer class.  Data in the table represent
//     a zig-zag ordering of an 8x8 block.
//
//------------------------------------------------------------------------
//    COPYRIGHT
//------------------------------------------------------------------------
#pragma ident   "@(#)QTable.hh	1.3\t00/03/10  "



#ifndef QTABLE_H
#define QTABLE_H

#include <xil/xilGPI.hh>

#define QTABLE_WIDTH 8
#define QTABLE_HEIGHT 8

#define BIT_PRECISION_8   0
#define BIT_PRECISION_16  1
#define UNKNOWN_PRECISION 2


class QTable {
public:

    QTable(unsigned int p = BIT_PRECISION_8);
    QTable( const QTable & );        

    int    table[QTABLE_WIDTH][QTABLE_HEIGHT];
    int    orig_table[QTABLE_WIDTH][QTABLE_HEIGHT];  

    void   Initialize(int val = 0);
    void   Initialize(int x[QTABLE_WIDTH][QTABLE_HEIGHT]);

    void   Scale(int value);

    int    Precision() const        { return precision; }
    int&   operator()(int i, int j) { return table[i][j]; }

    int    operator[](int i) { return table[i/QTABLE_WIDTH][i%QTABLE_HEIGHT]; }

    void   operator=(const QTable& );

private:
    Xil_unsigned8 precision;    // 8 or 16
    int            mask;

};

#endif
