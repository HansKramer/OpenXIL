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
//  File:   CellBFrame.hh
//  Project:    XIL
//  Revision:   1.3
//  Last Mod:   10:23:18, 03/10/00
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
#pragma ident   "@(#)CellBFrame.hh	1.3\t00/03/10  "

#ifndef CELLBFRAME_HH
#define CELLBFRAME_HH

#include "xil/xilGPI.hh"

//------------------------------------------------------------------------
//
//  Class:    CellB
//  Created:    92/11/30
//
// Description:
//    Keeps a single CellB which consists of a 16-bit mask and two 8-bit
//  indices.  The data can be modified and retrieved through the member 
//  functions.
//    
//    In some member functions, the class data is treated as an int for
//  speed.
//    
//------------------------------------------------------------------------

class CellB 
{
public:
    CellB(Xil_signed16 init_mask = 0, 
          Xil_unsigned8 init_uv = 0,
          Xil_unsigned8 init_yy = 0)
    : mask(init_mask), uv(init_uv), yy(init_yy) { };

    Xil_unsigned8&   UV(void) { return uv; }
    Xil_unsigned8&   YY(void) { return yy; }
    Xil_unsigned16&  MASK(void) { return mask; }

    Xil_unsigned8    UV(void) const { return uv; }
    Xil_unsigned8    YY(void) const { return yy; }

    Xil_unsigned16   MASK(void) const {

#ifdef XIL_LITTLE_ENDIAN
    return (mask<<8) | (mask>>8);
#else
    return mask;
#endif
    }

    int   operator== (const CellB& rval) {
      return (*((int*)this)  == *((const int*)&rval));
    }
    
    CellB&  operator= (const CellB& rval) {
      *((int*)this) = *((const int*)&rval);
      return *this;
    }
    
    //
    // There are two versions of the setCellB member function.  The first
    // takes the mask & indices separately and combines them.  The second
    // takes a single 32-bit value containing all three pieces
    //
    void             setCellB(Xil_signed16  m,
                              Xil_unsigned8 uv1,
                              Xil_unsigned8 yy1) 
    {
      *((int*)this) = (m<<16) | (uv1<<8) | yy1;
    }
    
    void             setCellB(Xil_unsigned32 m_uv_yy) 
    {
       *((int*)this) = m_uv_yy;
    }
    
protected:

#ifdef XIL_LITTLE_ENDIAN
    Xil_unsigned8   yy;
    Xil_unsigned8   uv;
    Xil_unsigned16  mask;
#else
    Xil_unsigned16  mask;
    Xil_unsigned8   uv;
    Xil_unsigned8   yy;
#endif
};

//------------------------------------------------------------------------
//
//  Class:    CellBFrame
//  Created:    92/11/30
//
// Description:
//    Maintains the all of the data associated with a single intra-encoded
//  frame's worth of data.
//    
//------------------------------------------------------------------------

class CellBFrame 
{
public:
    CellBFrame(unsigned short init_width, unsigned short init_height);
    ~CellBFrame(void);

    //
    //  General Frame Attributes
    //
    unsigned int  getFrameHeight(void) { return frame_height; }
    unsigned int  getFrameWidth(void)  { return frame_width;  }

    CellB*  operator[] (const unsigned int row) { return frame[row]; }

    CellB&  cellB(int y, int x) { return frame[y][x]; }

    CellBFrame& operator= (const CellBFrame& rval);
    CellBFrame* ok(void);
    
protected:
    CellB**             frame;
    const unsigned int  frame_height, frame_width;
    Xil_boolean         isOKFlag;
};

#endif  // CELLBFRAME_HH
