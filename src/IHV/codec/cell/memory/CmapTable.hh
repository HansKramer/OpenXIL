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
//  File:   CmapTable.hh
//  Project:    XIL
//  Revision:   1.3
//  Last Mod:   10:23:29, 03/10/00
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
#pragma ident   "@(#)CmapTable.hh	1.3\t00/03/10  "

#ifndef CMAPTABLE_HH
#define CMAPTABLE_HH

#include <stdlib.h>
#include <memory.h>
#include <xil/xilGPI.hh>
#include "ColorValue.hh"

enum ColorSpace {
    RGB, BGR, YUV
};

//------------------------------------------------------------------------
//
//  Class:    CmapTable
//
// Description:
//   The CmapTable class is intended to represent a colormap and
//   contain the operations to work on a colormap.
//    
//   The operator[] makes the table look like an array.  The table
//   is formated via the ColorValue class.
//    
//   The entire table can be given to an XilLookup class by using
//   the getAsTable() method.  The table can be reset by using the
//   setAsTable() method.
//    
//------------------------------------------------------------------------
class CmapTable {
public:
    CmapTable(ColorSpace cspace = RGB,
              unsigned int size = 256, 
              unsigned int ne = 256);
    CmapTable(Xil_unsigned8* data,
              ColorSpace cspace = RGB,
              unsigned int size = 256,
              unsigned int ne = 256);
    CmapTable(const CmapTable& t);
    
    ~CmapTable(void) {
        delete[] colorTable;
        colorTable = NULL;
    }

    //
    //  Lookup methods
    //
    ColorValue& operator[](const int i) {
        return colorTable[i];
    }

    ColorValue operator[](const int i) const {
        return colorTable[i];
    }

    //
    //  ColorSpace
    //
    Xil_boolean isColorSpace(ColorSpace s) const {
        return (colorSpace == s ? TRUE : FALSE);
    }

    ColorSpace  getColorSpace(void) const {
        return colorSpace;
    }
    
    void        setColorSpace(ColorSpace s) {
        colorSpace = s;
    }

    //
    //  TableSize
    //
    unsigned int getTableSize(void) const {
        return actualTableSize;
    }

    //
    //  Number of Entries
    //
    unsigned int getNumEntries(void) const {
        return numberOfEntries;
    }

    void setNumEntries(unsigned int num) {
        numberOfEntries = num;
    }

    //
    //  Number of Bands
    //
    unsigned int getNumBands(void) const {
        return numberOfBands;
    }

    //
    //  Colorspace Conversion
    //
    Xil_boolean toYUV(void);
    Xil_boolean toRGB(void);
    Xil_boolean toBGR(void);

    //
    //  Similarity
    //
    Xil_boolean isSimilar(unsigned int index1,
                          unsigned int index2,
                          int thresh) {
      int retval = FALSE;

      if (thresh >= 0) {
        if (index1 == index2) {
          retval = TRUE;
        } else if(thresh != 0) {
          if (colorTable[index1].distance(colorTable[index2]) <= thresh)
            retval = TRUE;
        }
      }

      return retval;
    }


    //
    //  Distance
    //
    int         distance(unsigned int index1,
                         unsigned int index2) {
      return colorTable[index1].distance(colorTable[index2]);
    }

    //
    //  Table Access
    //
    Xil_unsigned8* getAsColormapTable(void);

    int  setAsColormapTable(const Xil_unsigned8* cmap_table,
                            ColorSpace           cmap_space = RGB,
                            unsigned int         num_ents = 256);

    //
    //  Set as one
    //
    int  set(const CmapTable& t);
  
    //
    //  Operators
    //
    CmapTable& operator=  (const CmapTable& t);
    int        operator== (const CmapTable& t);

    //
    //  Allocation/Creation verification
    //
    CmapTable*  ok(Xil_boolean destroy = TRUE);
    
protected:
    ColorValue*   colorTable;
    ColorSpace    colorSpace;
    unsigned int  actualTableSize;
    unsigned int  numberOfEntries;

    // This is fixed based on the number of bands ColorValue supports
    const unsigned int  numberOfBands;
    
private:
    Xil_boolean     isok;
    
};


//------------------------------------------------------------------------
//
//  Class:    UsedCmapTable
//
// Description:
//    Same as CmapTable except it also keeps track of a "used" value for
//      every entry in the colormap.  This is used by the Adaptive Colormap
//      Selection algorithm.
//    
//------------------------------------------------------------------------
class UsedCmapTable : public CmapTable {
public:
    UsedCmapTable(ColorSpace t = RGB,
                  unsigned int s = 256,
                  unsigned int ne = 256);

    UsedCmapTable(Xil_unsigned8* data,
                  ColorSpace cspace = RGB,
                  unsigned int size = 256,
                  unsigned int ne = 256);

    ~UsedCmapTable(void) {
        delete[] usedTable;
        usedTable = NULL;
        isok = FALSE;
    }

    int&  used(const int i) {
        return usedTable[i];
    }

    int   used(const int i) const {
        return usedTable[i];
    }

    int*  usedArray(void) {
        return usedTable;
    }
    
    void  setUsedIndexes(int* indexlist) {
        memcpy(usedTable, indexlist, getNumEntries()*sizeof(int));
    }
    
    UsedCmapTable& operator=  (const UsedCmapTable& t);

    //
    //  Allocation/Creation verification
    //
    UsedCmapTable*  ok(Xil_boolean destroy = TRUE);
    
protected:
    int*   usedTable;
    
private:
    Xil_boolean isok;
};

//------------------------------------------------------------------------
//
//  Class:    LumCmapTable
//
// Description:
//    Same as UsedCmapTable except that it maintains a separate luminance
//      value for each entry.  This is redundant for YUV tables, but useful
//      for RGB and BGR tables. 
//
//------------------------------------------------------------------------
class LumCmapTable : public UsedCmapTable {
public:
    LumCmapTable(ColorSpace t = RGB,
                 unsigned int s = 256,
                 unsigned int ne = 256);
    LumCmapTable(const UsedCmapTable& t);
    ~LumCmapTable(void);

    double&  lum(unsigned int i) {
        return luminanceTable[i];
    }
    
    double   lum(unsigned int i) const {
        return luminanceTable[i];
    }

    void     computeLum(void);

    double*  lumArray(void) {
        return luminanceTable;
    }

    LumCmapTable& operator= (const UsedCmapTable& t);
    LumCmapTable& operator= (const LumCmapTable&  t);

    //
    //  Allocation/Creation verification
    //
    LumCmapTable*  ok(Xil_boolean destroy = TRUE);
    
protected:
    double*  luminanceTable;

private:
    Xil_boolean isok;
};

#endif  // CMAPTABLE_HH
