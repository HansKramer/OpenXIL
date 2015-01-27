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
//  File:   CellBColorConvertTable.hh
//  Project:    XIL
//  Revision:   1.3
//  Last Mod:   10:23:16, 03/10/00
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
#pragma ident   "@(#)CellBColorConvertTable.hh	1.3\t00/03/10  "

#ifndef CELLBCOLORCONVERTTABLE_HH
#define CELLBCOLORCONVERTTABLE_HH

#include <xil/xilGPI.hh>
#include "CellBDefines.hh"
#include "CellBFrame.hh"


class YYrec {
 public:
    Xil_signed16 y1,y2;
    void setValues(Xil_unsigned8 Y1, Xil_unsigned8 Y2);
 private:
    Xil_signed16 ycomponent(Xil_unsigned8 yval);
};

class UVrec { 
 public:
    Xil_signed16 r,g,b; 
    void setValues(Xil_unsigned8 u, Xil_unsigned8 v);
};

class CellBColorConvertTable {
public:
    //
    //  Constructor
    //
    CellBColorConvertTable(Xil_unsigned16* yytable, Xil_unsigned16* uvtable);
    
    //
    //  Destructor
    //
    ~CellBColorConvertTable(void);
    
    //
    //  Initializer
    //
    CellBColorConvertTable* ok(Xil_boolean destroy = TRUE);

    //  Access to the table
    //  For Intel, or for other frame buffers, we may want to add other
    //  access methods.
    void convertBGRX(CellB cell, Xil_unsigned32 *rgb1, Xil_unsigned32 *rgb2);
    void convertXBGR(CellB cell, Xil_unsigned32 *rgb1, Xil_unsigned32 *rgb2);
    void convertRGB(CellB cell, 
                    Xil_unsigned8 *r1,
                    Xil_unsigned8 *g1,
                    Xil_unsigned8 *b1,
                    Xil_unsigned8 *r2,
                    Xil_unsigned8 *g2,
                    Xil_unsigned8 *b2);

private:

    // These tables consist of the rgb components for YUV->RGB conversion
    // indexed by the cellb index. They are fixed point numbers with a single
    // binary digit fractional part.
    YYrec* YYtable;
    UVrec* UVtable;
    Xil_boolean      isOKFlag;
};

#endif  // CELLBCOLORCONVERTTABLE_HH
