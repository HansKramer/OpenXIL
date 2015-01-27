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
//  File:   CellFrame.hh
//  Project:    XIL
//  Revision:   1.6
//  Last Mod:   10:23:26, 03/10/00
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
#pragma ident   "@(#)CellFrame.hh	1.6\t00/03/10  "

#ifndef CELLFRAME_HH
#define CELLFRAME_HH

#include "xil/xilGPI.hh"

#include "CmapTable.hh"
#include "CellDefines.hh"

extern unsigned int bitson_table[];


//------------------------------------------------------------------------
//
//  Class:    Cell
//
// Description:
//  Keeps a single Cell which consists of a 16-bit mask and two 8-bit
//  colors.  The data can be modified and retrieved through the member 
//  functions.
//    
//  In some member functions, the class data is treated as an int for
//  speed.
//    
//------------------------------------------------------------------------
class Cell {
  public:
    Cell(Xil_signed16 init_mask = 0,
         Xil_unsigned8 init_c0 = 0,
         Xil_unsigned8 init_c1 = 0)
    : mask(init_mask), color0(init_c0), color1(init_c1) { };

    Xil_unsigned8&   C0(void) {
        return color0;
    }
    Xil_unsigned8&   C1(void) {
        return color1;
    }
    Xil_unsigned16&  MASK(void) {
        return mask;
    }

  
    Xil_unsigned8    C0(void) const {
        return color0;
    }
    Xil_unsigned8    C1(void) const {
        return color1;
    }
    Xil_unsigned16   MASK(void) const {
        return mask;
    }

    void             clearCell(void) {
        *((int*)this) = 0;
    }

    void             flipCell(void) {
        Xil_unsigned8 tmp = color0;
        color0 = color1;
        color1 = tmp;
        mask   = ~mask;
    }

    int   operator== (const Cell& rval) {
        return (*((int*)this)  == *((const int*)&rval));
    }
    
    Cell&  operator= (const Cell& rval) {
        *((int*)this) = *((const int*)&rval);
        return *this;
    }

    void   setCell(Xil_unsigned16 m,
                   Xil_unsigned8 c0,
                   Xil_unsigned8 c1) {
        mask   = m;
        color0 = c0;
        color1 = c1;
    }
    
protected:
    Xil_unsigned16  mask; 
    Xil_unsigned8   color0;
    Xil_unsigned8   color1;
};

//------------------------------------------------------------------------
//
//  Class:    CellUserData
//
// Description:
//  Contains the data structure for holding any user data that is
//  associated with the current frame.
//    
//------------------------------------------------------------------------
class CellUserData {
public:
    unsigned int    length;
    Xil_unsigned8*  data;

    CellUserData(void) {
        data   = NULL;
        length = 0;
    }
};


//------------------------------------------------------------------------
//
//  Class:    CellFrame
//
// Description:
//  Maintains the all of the data associated with a single intra-encoded
//  frame's worth of data.
//    
//------------------------------------------------------------------------
class CellFrame {
  public:
    CellFrame(unsigned int init_width, unsigned int init_height);
    ~CellFrame(void);

    //
    //  General Frame Attributes
    //
    unsigned int  getFrameHeight(void) { return frame_height; }
    unsigned int  getFrameWidth(void)  { return frame_width;  }

    //
    //  Cell Portion of the Frame Manipulation Functions
    //
    void clearFrame(void);
    void setFrame(const Cell& set_val);
    
    Cell*  operator[] (const unsigned int row) {
        return frame[row];
    }

    Cell&  cell(int y, int x) {
        return frame[y][x];
    }

    //
    //  Flips a Cell including setting the flipped flag.
    //
    Xil_boolean  flipCell(unsigned int row, unsigned int column) {
        frame[row][column].flipCell();
        return (flipped[row][column] = !flipped[row][column]);
    }

    //
    //  Colormap Portion of the Frame
    //
    void setColormap(const CmapTable& set_cmap) {
        cmap = set_cmap;
    }
    void getColormap(CmapTable& ret_cmap) const {
        ret_cmap = cmap;
    }

    //
    //  Colormap Flag Functions
    //
    void          setColormapChanged(Xil_boolean flag) {
        cmapChanged = flag;
    }
    Xil_boolean   getColormapChanged(void) const {
        return cmapChanged;
    }
    
    //
    //  Flipped State portion of the Frame
    //
    void         setFlipped(unsigned int row,
                            unsigned int column,
                            Xil_boolean val) {
        flipped[row][column] = val;
    }

    void         clearFlipped(void);
    
    Xil_boolean  isFlipped(unsigned int row, unsigned int column) const {
        return flipped[row][column];
    }

    //
    //  Mark whether frame should be dropped.
    //
    void       setDrop(void) {
        drop_flag = TRUE;
    }
    void       clearDrop(void) {
        drop_flag = FALSE;
    }
    Xil_boolean getDrop(void) const {
        return drop_flag;
    }

    //
    //  Set and get the user data associated with this frame.
    //
    void          setUserData(CellUserData* udata) {
        userData = udata;
    }
    CellUserData* getUserData(void) {
        return userData;
    }
    

    CellFrame& operator= (const CellFrame& rval);
    CellFrame* ok(void);
    
protected:
    Cell**              frame;
    Xil_boolean**       flipped;
    CellUserData*       userData;
    CmapTable           cmap;
    Xil_boolean         cmapChanged;
    Xil_boolean         drop_flag;
    const unsigned int  frame_height, frame_width;
    Xil_boolean         isok;
};

//------------------------------------------------------------------------
//
//  Class:    ErrorInfo
//
// Description:
//    A class to keep the error information about a Cell for bit-rate
//  control.
//    
//------------------------------------------------------------------------
#define ERROR_INFO_DEFAULT_SIZE  32

struct SubErrorInfo {
    unsigned int  L;
    unsigned int  C;
};

class ErrorInfo
{
public:
    //
    //  See resize()
    //
    ErrorInfo(const int size = 0);
    
    ~ErrorInfo(void) {
        delete err_array;
    }

    //
    //  I need this resize because array construction can't take
    //  initialization numbers.  So, they're constructed by default without
    //  any memory (by default) and then resize is called to allocate the
    //  necessary amount of memory.  Specifying a size to the constructor WILL
    //  initialize this class with memory.
    //
    void resize(const int newsize);
    
    void setPartition(const unsigned int part) {
        partition = part;
    }
    unsigned int getPartition(void) const {
        return partition;
    }
    
    void setErr(const unsigned int offset,
                const unsigned int L,
                const unsigned int C) {
        err_array[offset].L = L;
        err_array[offset].C = C;
    }

    unsigned int  getLumaErr(const unsigned int offset) const {
        return err_array[offset].L;
    }
    unsigned int  getChroErr(const unsigned int offset) const {
        return err_array[offset].C;
    }

    ErrorInfo* ok(void);

private:
    SubErrorInfo* err_array;
    unsigned int  partition;
    unsigned int  numEntries;
    Xil_boolean   isok;
    
};

//------------------------------------------------------------------------
//
//  Class:    ErrorInfoFrame
//
// Description:
//    A class that maintains the error information about an entire
//  CellFrame.  Usually, one of these is created for the entire frame group.
//    
//------------------------------------------------------------------------
class ErrorInfoFrame
{
public:
    ErrorInfoFrame(unsigned int init_width, unsigned int init_height,
                   unsigned int infosize = ERROR_INFO_DEFAULT_SIZE);
    ~ErrorInfoFrame(void);

    //
    //  General Frame Attributes
    //
    unsigned int  getFrameHeight(void) { return frame_height; }
    unsigned int  getFrameWidth(void)  { return frame_width;  }

    //
    //  General Access
    //
    ErrorInfo*   operator[] (const unsigned int row) {
        return frame[row];
    }

    ErrorInfoFrame* ok(void);
    
protected:
    ErrorInfo**         frame;
    const unsigned int  frame_height, frame_width;
    Xil_boolean         isok;
};

//------------------------------------------------------------------------
//
//  Function:    XILBITSON()
//
//  Description:
//    Looks up into a table to determine how many bits are different between
//  two Cell masks.
//    
//------------------------------------------------------------------------
inline unsigned int     
BitsOn(const Cell& cell1, const Cell& cell2) 
{
    unsigned int mask = cell1.MASK() ^ cell2.MASK();
    
    int top_byte    = mask>>8;
    int bottom_byte = mask&0xff;

    return (bitson_table[top_byte] + bitson_table[bottom_byte]);
}

#endif  // CELLFRAME_HH
