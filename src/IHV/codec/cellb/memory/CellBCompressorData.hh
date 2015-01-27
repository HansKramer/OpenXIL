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
//  File:   CellBCompressorData.hh
//  Project:    XIL
//  Revision:   1.2
//  Last Mod:   10:23:17, 03/10/00
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
#pragma ident   "@(#)CellBCompressorData.hh	1.2\t00/03/10  "

#ifndef CELLBCOMPRESSORDATA_HH
#define CELLBCOMPRESSORDATA_HH

#include <xil/xilGPI.hh>

class CellBCompressorData {
public:
    //
    //  Constructor
    //
    CellBCompressorData();
    
    //
    //  Destructor
    //
    ~CellBCompressorData();
    
    //
    //  Initializer
    //
    XilStatus initialize(unsigned int width,
                         unsigned int height);

    // an integer represents which version of the history is the valid one.
    int  initializeOtherHistory() { return ++validHistory; }

    void initializeAtomicHistory();

    Xil_boolean atomicHistoryValid()
    {
      return atomicHistory == validHistory; 
    }

    Xil_boolean historyValid(int myHistory)  
    {
      return myHistory == validHistory; 
    }

    void initValues();

    void deleteValues();

    void reset();

    CellBCompressorData* ok(Xil_boolean destroy = TRUE);
    
    //
    //  The current image to filter and compress
    //    and the previous image for filtering.
    //
    XilImage*     currentImage;
    //    XilImage*     previousImage;

    unsigned int           cellHeight, cellWidth;

    //
    //    history buffer of previously generated codes 
    //    (indexed by (y * cellWidth) + x)
    //
    Xil_unsigned32*    cellHistory;

    //
    //    How long ago this cell was updated (to make sure that it doesn\'t
    //    get skipped for too long)
    //    (indexed by (y * cellWidth) + x)
    //
    int*         updateHistory;

    //
    //  Flag for if I've been initialized
    //
    Xil_boolean      initialized;


private:
    Xil_boolean      isOKFlag;

    int             atomicHistory;

    int             validHistory;
    // which history buffer is currently valid.  If 0, then none are valid.
    // Otherwise, it must match a saved value to indicate that the current
    // history buffer matches the valid one.  This allows molecules to
    // keep a separate history buffer.
};

#endif  // CELLBCOMPRESSORDATA_HH
