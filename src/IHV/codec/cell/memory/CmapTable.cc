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
//  File:   CmapTable.cc
//  Project:    XIL
//  Revision:   1.2
//  Last Mod:   10:15:43, 03/10/00
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
#pragma ident   "@(#)CmapTable.cc	1.2\t00/03/10  "

#define INCL_STDLIB

#include "CmapTable.hh"
#include "XiliUtils.hh"

//------------------------------------------------------------------------
//
//  Function:    CmapTable::CmapTable
//
//  Description:
//    CmapTable constructors.
//    
//------------------------------------------------------------------------

CmapTable::CmapTable(ColorSpace cspace,
                     unsigned int size, 
                     unsigned int ne)
: colorSpace(cspace),
  actualTableSize(size),
  numberOfEntries(ne),
  numberOfBands(3)
{
    colorTable = new ColorValue[actualTableSize];

    if (colorTable == NULL) {
      // out of memory
      XIL_ERROR( NULL, XIL_ERROR_RESOURCE,"di-1",TRUE); 
    }
}

CmapTable::CmapTable(Xil_unsigned8* data,
                     ColorSpace cspace,
                     unsigned int size,
                     unsigned int ne) 
: colorSpace(cspace),
  actualTableSize(size),
  numberOfEntries(ne),
  numberOfBands(3)
{
    colorTable = NULL;
    if (setAsColormapTable(data, cspace, numberOfEntries) == XIL_SUCCESS)
      isok = TRUE;
    else
      isok = FALSE;
}

CmapTable::CmapTable(const CmapTable& t) 
: actualTableSize(t.actualTableSize),
  numberOfBands(3)
{
    isok = FALSE;
    
    colorTable = new ColorValue[actualTableSize];

    if (colorTable == NULL) {
      // out of memory
      XIL_ERROR( NULL, XIL_ERROR_RESOURCE,"di-1",TRUE);
      return;
    }

    //
    //  The value of colorTable is checked in set.  If it's NULL, it just
    //  returns.
    //
    if (set(t) == XIL_SUCCESS)
      isok = TRUE;
}

//------------------------------------------------------------------------
//
//  Function:    CmapTable::to*
//
//  Description:
//        The to* routines convert the colorTable managed by CmapTable to
//     the new colorspace.
//    
//------------------------------------------------------------------------

Xil_boolean  
CmapTable::toYUV(void) 
{
    if (isColorSpace(RGB)) {
      for(unsigned int i=0; i<getTableSize(); i++)
        colorTable[i].RGBtoYUV();
      setColorSpace(YUV);
      return TRUE;
    } else if(isColorSpace(BGR)) {
      for(unsigned int i=0; i<getTableSize(); i++)
        colorTable[i].BGRtoYUV();
      setColorSpace(YUV);
      return TRUE;
    } else {
      return FALSE;
    }
}

Xil_boolean  
CmapTable::toRGB(void) 
{
    if (isColorSpace(YUV)) {
      for(unsigned int i=0; i<getTableSize(); i++)
        colorTable[i].YUVtoRGB();

      setColorSpace(RGB);
      return TRUE;
    } else if(isColorSpace(BGR)) {
      for(unsigned int i=0; i<getTableSize(); i++)
        colorTable[i].BGRtoRGB();
    
      setColorSpace(RGB);
      return TRUE;
    } else {
      return FALSE;
    }
}

Xil_boolean  
CmapTable::toBGR(void) 
{
    if (isColorSpace(YUV)) {
      for(unsigned int i=0; i<getTableSize(); i++)
        colorTable[i].YUVtoBGR();
    
      setColorSpace(BGR);
      return TRUE;
    } else if(isColorSpace(RGB)) {
      for(unsigned int i=0; i<getTableSize(); i++)
        colorTable[i].RGBtoBGR();
    
      setColorSpace(BGR);
      return TRUE;
    } else {
      return FALSE;
    }
}

//------------------------------------------------------------------------
//
//  Function:    CmapTable::operator=
//
//  Description:
//    Sets this CmapTable equal to the given CmapTable.
//    
//------------------------------------------------------------------------

CmapTable&
CmapTable::operator= (const CmapTable& t)
{
    set(t);
    return *this;
}

int
CmapTable::set(const CmapTable& t)
{
    int assert_fail=FALSE;
    if (assert_fail ||
       (getTableSize() < t.getTableSize()) ||
       colorTable == NULL || t.colorTable == NULL) {
      XIL_ERROR( NULL, XIL_ERROR_SYSTEM, "di-95", TRUE);
      return XIL_FAILURE;
    }
    
    actualTableSize = t.actualTableSize;
    setNumEntries(t.getNumEntries());
    setColorSpace(t.getColorSpace());

    xili_memcpy(colorTable, t.colorTable, getTableSize()*sizeof(ColorValue));

    return XIL_SUCCESS;
}

//------------------------------------------------------------------------
//
//  Function:    CmapTable::operator==
//
//  Description:
//    Tests for eqality between this CmapTable an the given
//    CmapTable.  Only the numEntries are checked.
//    
//    
//------------------------------------------------------------------------

int
CmapTable::operator== (const CmapTable& t)
{
    if (getNumEntries() != t.getNumEntries()) 
      return 0;
    
    for (unsigned int i=0; i<getNumEntries(); i++) {
      if (!(colorTable[i] == t.colorTable[i])) 
        return 0;
    }

    return 1;
}

//------------------------------------------------------------------------
//
//  Function:    CmapTable::{set,get}AsColorMapTable
//
//  Description:
//    Set and gets the colormap table maintained in the CmapTable
//    class as a 3-banded, XIL_BYTE table.
//    
//    These routines do any processing necessary format conversion.
//    
//------------------------------------------------------------------------

Xil_unsigned8*
CmapTable::getAsColormapTable(void)
{
    Xil_unsigned8* returnTable =
        new Xil_unsigned8[actualTableSize * numberOfBands];
    
    if (returnTable == NULL) {
      // out of memory
      XIL_ERROR( NULL, XIL_ERROR_RESOURCE,"di-1",TRUE); 
      return NULL;
    }
    
    for (unsigned int i=0,j=0; i<actualTableSize; i++, j+=3) {
      returnTable[j]   = colorTable[i].band0();
      returnTable[j+1] = colorTable[i].band1();
      returnTable[j+2] = colorTable[i].band2();
    }
    
    return (Xil_unsigned8*)returnTable;
}

int
CmapTable::setAsColormapTable(const Xil_unsigned8* cmap_table,
                              ColorSpace           cmap_space,
                              unsigned int         num_ents)
{
    setColorSpace(cmap_space);
    delete colorTable;        // delete the old colorTable first
    colorTable = new ColorValue[actualTableSize];

    if (colorTable == NULL) {
      // out of memory 
      XIL_ERROR( NULL, XIL_ERROR_RESOURCE,"di-1",TRUE);
      return XIL_FAILURE;
    }
    
    numberOfEntries = num_ents;
    
    for (unsigned int i=0,j=0; i<numberOfEntries; i++,j+=3) {
      colorTable[i].setColor(cmap_table[j],
                             cmap_table[j+1],
                             cmap_table[j+2]);
    }
    
    return XIL_SUCCESS;
}

//------------------------------------------------------------------------
//
//  Function:    CmapTable::ok()
//
//  Description:
//    Verifies class was created ok.
//    
//------------------------------------------------------------------------
CmapTable*
CmapTable::ok(Xil_boolean destroy) 
{
    if (this == NULL) {
      return NULL;
    } else {
      if (isok == TRUE) {
        return this;
      } else {
        if (destroy == TRUE) 
          delete this;
        return NULL;
      }
    }
}

//------------------------------------------------------------------------
//
//  Function:    UsedCmapTable::operator=
//
//  Description:
//    Sets this UsedCmapTable equal to the given UsedCmapTable.
//    
//------------------------------------------------------------------------

UsedCmapTable&
UsedCmapTable::operator= (const UsedCmapTable& t)
{
    ((CmapTable&)*this) = ((const CmapTable&)t);
    
    if (usedTable == NULL || t.usedTable == NULL) {
      XIL_ERROR( NULL, XIL_ERROR_SYSTEM, "di-95", TRUE);
    } else
      xili_memcpy(usedTable, t.usedTable, getTableSize()*sizeof(int));

    return *this;
}

UsedCmapTable::UsedCmapTable(ColorSpace t,
                             unsigned int s,
                             unsigned int ne)
: CmapTable(t,s,ne)
{
    isok = FALSE;
    usedTable = new int[getTableSize()];

    if (usedTable == NULL) {
      // out of memory
      XIL_ERROR( NULL, XIL_ERROR_RESOURCE,"di-1",TRUE);  
      return;
    }
    
    for (unsigned int i=0; i<getTableSize(); i++) 
      usedTable[i] = 1;
    isok = TRUE;
}

UsedCmapTable::UsedCmapTable(Xil_unsigned8* data,
                             ColorSpace cspace,
                             unsigned int size,
                             unsigned int ne) 
: CmapTable(data, cspace, size, ne)
{
    isok = FALSE;
    usedTable = new int[getTableSize()];

    if (usedTable == NULL) {
      // out of memory  
      XIL_ERROR( NULL, XIL_ERROR_RESOURCE,"di-1",TRUE);
      return;
    }

    for (unsigned int i=0; i<getTableSize(); i++) 
      usedTable[i] = 1;
    isok = TRUE;
}    

//------------------------------------------------------------------------
//
//  Function:    UsedCmapTable::ok()
//
//  Description:
//    Verifies class was created ok.
//    
//------------------------------------------------------------------------

UsedCmapTable*
UsedCmapTable::ok(Xil_boolean destroy) 
{
    if (this == NULL) {
      return NULL;
    } else {
      if (CmapTable::ok(FALSE) == this && isok == TRUE) {
        return this;
      } else {
        if (destroy == TRUE)
          delete this;
        return NULL;
      }
    }
}

//------------------------------------------------------------------------
//
//  Function:    LumCmapTable::LumCmapTable()
//
//  Description:
//    LumCmapTable constructors.
//    
//------------------------------------------------------------------------

LumCmapTable::LumCmapTable(ColorSpace t,
                           unsigned int s,
                           unsigned int ne)
: UsedCmapTable(t,s,ne)
{
    isok = FALSE;
    luminanceTable = new double[getTableSize()];

    if (luminanceTable == NULL) {
      // out of memory
      XIL_ERROR( NULL, XIL_ERROR_RESOURCE,"di-1",TRUE);
      return;
    }
    isok = TRUE;
}

LumCmapTable::LumCmapTable(const UsedCmapTable& t) 
{
    isok = FALSE;
    *this = t;
    isok = TRUE;
}

LumCmapTable::~LumCmapTable(void) 
{
    delete[] luminanceTable;
    luminanceTable = NULL;
    isok = FALSE;
}

//------------------------------------------------------------------------
//
//  Function:    LumCmapTable::computeLum
//
//  Description:
//    Computes the luminance array.
//    
//------------------------------------------------------------------------

void
LumCmapTable::computeLum(void) 
{
    if (isColorSpace(YUV)) {
      for(unsigned int i=0; i<getNumEntries(); i++) {
        luminanceTable[i] = (float)colorTable[i].band0();
      }
    } else if (isColorSpace(RGB)) {
      for (unsigned int i=0; i<getNumEntries(); i++) {
        luminanceTable[i] = rgb2y(colorTable[i].band0(),
                                  colorTable[i].band1(),
                                  colorTable[i].band2());
      }
    } else if (isColorSpace(BGR)) {
      for (unsigned int i=0; i<getNumEntries(); i++) {
        luminanceTable[i] = rgb2y(colorTable[i].band2(),
                                  colorTable[i].band1(),
                                  colorTable[i].band0());
      }
    }
}

//------------------------------------------------------------------------
//
//  Function:    LumCmapTable::operator=
//
//  Description:
//    Sets the UsedCmapTable portion of this LumCmapTable equal to the given
//      UsedCmapTable.
//    
//------------------------------------------------------------------------
LumCmapTable&  LumCmapTable::operator= (const UsedCmapTable& t)
{
    ((UsedCmapTable&)*this) = ((const UsedCmapTable&)t);
    
    return *this;
}
    
//------------------------------------------------------------------------
//
//  Function:    LumCmapTable::operator=
//
//  Description:
//    Sets this LumCmapTable equal to the given LumCmapTable.
//    
//------------------------------------------------------------------------
LumCmapTable&  LumCmapTable::operator= (const LumCmapTable& t)
{
    ((UsedCmapTable&)*this) = ((const UsedCmapTable&)t);
    
    if (luminanceTable == NULL || t.luminanceTable == NULL) {
      XIL_ERROR( NULL, XIL_ERROR_SYSTEM, "di-95", TRUE);
    } else
        xili_memcpy(luminanceTable,
                   t.luminanceTable,
                   getTableSize()*sizeof(double));

    return *this;
}

//------------------------------------------------------------------------
//
//  Function:    LumCmapTable::ok()
//
//  Description:
//    Verifies class was created ok.
//    
//------------------------------------------------------------------------

LumCmapTable*
LumCmapTable::ok(Xil_boolean destroy) 
{
    if (this == NULL) {
      return NULL;
    } else {
      if (UsedCmapTable::ok(FALSE) == this && isok == TRUE) {
        return this;
      } else {
        if (destroy == TRUE) delete this;
          return NULL;
      }
    }
}
