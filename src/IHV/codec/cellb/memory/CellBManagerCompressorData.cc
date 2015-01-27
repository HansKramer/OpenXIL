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
//  File:   CellBManagerCompressorData.cc
//  Project:    XIL
//  Revision:   1.2
//  Last Mod:   10:15:32, 03/10/00
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
#pragma ident   "@(#)CellBManagerCompressorData.cc	1.2\t00/03/10  "


#include "CellBManagerCompressorData.hh"

#include "CellBTables.hh"

// OK_FUNC(CellBManagerCompressorData);

//------------------------------------------------------------------------
//
//  Function(s):
//
//  Description:
//    The constructor and destructor for CellBManagerCompressorData.  These
//    setup the compressor values to their default.
//
//  Parameters:
//
//  Returns:
//    
//  Side Effects:
//------------------------------------------------------------------------

CellBManagerCompressorData::CellBManagerCompressorData()
{
    int i,j;
    Xil_unsigned8* dptr;

    isOKFlag = FALSE;
    
    uvclose = (Xil_unsigned8**)uvclose_init;
    uvtable = (Xil_unsigned16*)uvtable_init;
    uvremap = (Xil_unsigned8*)uvremap_init;
    
    yytable = (Xil_unsigned16*)yytable_init;
    yyremap = (Xil_unsigned8*)yyremap_init;
    //
    //    .... initialize division table ....
    //
    dptr = table;
    divtable[0] = dptr;
    for (i = 1; i < BITS_IN_CELL_PLUS_ONE; i++) {
      divtable[i] = dptr;
      for (j = 0; j < MAX_BYTE_VAL * i; j++)
        *dptr++ = (j + (i >> 1)) / i;  // round the result
    }

#define UVCLOSE(X,Y) (((X) * (X)) + ((Y) * (Y)) <= 64)

    //
    //    .... initialize uv closeness look-up table ....
    //
    for (i = 0; i < UVTABLE_SIZE; i++) {
      int u0 = uvtable[i];
      int v0 = u0 & 255;
      u0 = u0 >> 8;
      for (j = 0; j < UVTABLE_SIZE; j++) {
        int index;
        int u1 = uvtable[j];
        int v1 = u1 & 255;
        u1 = u1 >> 8;
        u1 = u1 - u0;
        v1 = v1 - v0;
        if (UVCLOSE(u1, v1)) {
          index = (i << 5) | (j >> 3);
          uvlookup[index] |= 1 << (j & 7);
        }
      }
    }

    //
    //    .... initialize error mult table ....
    //
    for (j = 0; j < BITS_IN_CELL_PLUS_ONE; j++) {
      for (i = 0; i < 512; i++) {
        int index = i - MAX_BYTE_VAL;
        int absindex = (index < 0) ? -index : index;
        int tmp = absindex * j;
        tmp = (tmp > 255) ? 255 : tmp;
        error[j][i] = tmp;
      }
    }    
    
    convertTables = NULL;

    isOKFlag = TRUE;
};


CellBManagerCompressorData::~CellBManagerCompressorData(void)
{
    delete convertTables;
}

void
CellBManagerCompressorData::initConvertTables()
{
    convertTables = new CellBColorConvertTable(yytable, uvtable);
}
