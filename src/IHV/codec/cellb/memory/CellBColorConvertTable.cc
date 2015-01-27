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
//  File:   CellBColorConvertTable.cc
//  Project:    XIL
//  Revision:   1.2
//  Last Mod:   10:15:26, 03/10/00
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
#pragma ident   "@(#)CellBColorConvertTable.cc	1.2\t00/03/10  "

#include <xil/xilGPI.hh>

#include "CellBColorConvertTable.hh"

#define CLAMP(X) (((X) & ~255) ? (((X) < 0) ? 0 : 255) : (X))

#define  RY_VAL  1.164385
#define  RV_VAL  1.596431

#define  GY_VAL  1.164382
#define  GU_VAL -0.392973
#define  GV_VAL -0.812944

#define  BY_VAL  1.164383
#define  BU_VAL  2.016403


// OK_FUNC(CellBColorConvertTable);

//------------------------------------------------------------------------
//
//  Function(s):
//    CellBColorConvertTable::CellBColorConvertTable
//    CellBColorConvertTable::~CellBColorConvertTable
//    CellBColorConvertTable::convertBGRX
//
//    YYrec::YYrec
//    UVrec::UVrec
//  Created:    92/11/11
//
//  Description:
//    Create tables that can be used to compute rgb from yuv CellB values.
//
//  Parameters:
//    CellBColorConvertTable::CellBColorConvertTable
//        yytable and uvtable are the values that cells can have.
//    CellBColorConvertTable::~CellBColorConvertTable
//        void
//
//  Returns:
//    
//  Side Effects:
//------------------------------------------------------------------------

Xil_signed16
YYrec::ycomponent(Xil_unsigned8 yval) 
{
    return (Xil_signed16)((((float)yval - 16.0) * RY_VAL + 0.5) * 2.0);
}


void
YYrec::setValues(Xil_unsigned8 Y1, Xil_unsigned8 Y2)
{
    y1 = ycomponent(Y1);
    y2 = ycomponent(Y2);
}


void
UVrec::setValues(Xil_unsigned8 u, Xil_unsigned8 v)
{
    float fu = ((float)u - 128.0) * 2.0;
    float fv = ((float)v - 128.0) * 2.0;
    r = (Xil_signed16)(RV_VAL * fv);
    g = (Xil_signed16)(GU_VAL * fu + GV_VAL * fv);
    b = (Xil_signed16)(BU_VAL * fu);
}

CellBColorConvertTable::CellBColorConvertTable(Xil_unsigned16 *yytable,
                           Xil_unsigned16 *uvtable)
{
    isOKFlag = FALSE;

    YYtable = new YYrec[MAX_BYTE_VAL];
    UVtable = new UVrec[UVTABLE_SIZE];

    if (!YYtable || !UVtable) {
      XIL_ERROR( NULL, XIL_ERROR_RESOURCE, "di-1", TRUE);
      return;
    }
    
    int i;
    for (i=0;i < MAX_BYTE_VAL; i++) {
      YYtable[i].setValues((yytable[i] >> 8) & 0xff, yytable[i] & 0xff);
    }

    for (i=0;i < UVTABLE_SIZE; i++)
      UVtable[i].setValues((uvtable[i] >> 8) & 0xff, uvtable[i] & 0xff);

    isOKFlag = TRUE;
};


CellBColorConvertTable::~CellBColorConvertTable(void)
{
    delete YYtable;
    YYtable = NULL;
    delete UVtable;
    UVtable = NULL;
}

#define DO_convertRGB(cell,r1,g1,b1,r2,g2,b2) \
    YYrec yy = YYtable[cell.YY()];    \
    UVrec uv = UVtable[cell.UV()];    \
    int ruv = uv.r;            \
    int guv = uv.g;            \
    int buv = uv.b;            \
    int y1 =  yy.y1;            \
    int y2 =  yy.y2;            \
    b1 = CLAMP((buv + y1) >> 1);    \
    g1 = CLAMP((guv + y1) >> 1);    \
    r1 = CLAMP((ruv + y1) >> 1);    \
    b2 = CLAMP((buv + y2) >> 1);    \
    g2 = CLAMP((guv + y2) >> 1);    \
    r2 = CLAMP((ruv + y2) >> 1);    \

// This one sets things up properly for the GS memory image.
void 
CellBColorConvertTable::convertXBGR(CellB cell,
                                    Xil_unsigned32 *rgb1,
                                    Xil_unsigned32 *rgb2)
{
    Xil_unsigned8 r1,g1,b1,r2,g2,b2;    
    DO_convertRGB(cell,r1,g1,b1,r2,g2,b2);
    *rgb1 = (b1 << 16) + (g1 << 8) + r1;
    *rgb2 = (b2 << 16) + (g2 << 8) + r2;
}


void 
CellBColorConvertTable::convertBGRX(CellB cell,
                                    Xil_unsigned32 *rgb1,
                                    Xil_unsigned32 *rgb2)
{
    Xil_unsigned8 r1,g1,b1,r2,g2,b2;    
    DO_convertRGB(cell,r1,g1,b1,r2,g2,b2);
    *rgb1 = (b1 << 24) + (g1 << 16) + (r1 << 8);
    *rgb2 = (b2 << 24) + (g2 << 16) + (r2 << 8);
}


void 
CellBColorConvertTable::convertRGB(CellB cell,
                                   Xil_unsigned8 *r1,
                                   Xil_unsigned8 *g1,
                                   Xil_unsigned8 *b1,
                                   Xil_unsigned8 *r2,
                                   Xil_unsigned8 *g2,
                                   Xil_unsigned8 *b2)
{
    DO_convertRGB(cell,*r1,*g1,*b1,*r2,*g2,*b2);
}


