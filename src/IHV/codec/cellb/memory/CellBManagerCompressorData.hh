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
//  File:   CellBManagerCompressorData.hh
//  Project:    XIL
//  Revision:   1.3
//  Last Mod:   10:23:20, 03/10/00
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
#pragma ident   "@(#)CellBManagerCompressorData.hh	1.3\t00/03/10  "

#ifndef CELLBTYPECOMPRESSORDATA_HH
#define CELLBTYPECOMPRESSORDATA_HH

#include <xil/xilGPI.hh>
#include "CellBDefines.hh"
#include "CellBColorConvertTable.hh"

typedef  Xil_unsigned8 error_array[BITS_IN_CELL_PLUS_ONE][MAX_BYTE_VAL * 2];

class CellBManagerCompressorData 
{
public:
    //
    //  Constructor
    //
    CellBManagerCompressorData();
    
    //
    //  Destructor
    //
    ~CellBManagerCompressorData(void);
    
    //
    //  Initializer
    //
    CellBManagerCompressorData* ok(Xil_boolean destroy = TRUE);

    //
    //  Tables used to precompute divisions
    //        We want to precompute all divisions of x / y, where x
    //        is the total of all the byte vals that are set, and y
    //        is the number of set pixels.  Thus, the size of each
    //        row of the table is size(y) (256 * y).  The total table
    //        size is the sum of size(y) for 0 <= y <= 16.  
    //        
    Xil_unsigned8*    divtable[BITS_IN_CELL_PLUS_ONE];
    Xil_unsigned8     table[MAX_BYTE_VAL *

    BITS_IN_CELL_PLUS_ONE * (BITS_IN_CELL_PLUS_ONE - 1) / 2];

    //
    //  Tables used in computing uv and error
    //

    Xil_unsigned16*     uvtable;
    Xil_unsigned8*      uvremap;
    Xil_unsigned8**     uvclose;

    Xil_unsigned8*    yyremap;
    Xil_unsigned16*   yytable;
    
    //  The uvlookup table is actually a bitvector
    //  where each bit-position is set or cleared according
    //  to whether, UV(old) is close to UV(new), The
    //  heuristic for closeness is embodied by the
    //  uvclose[ ] vector which in the first version
    //  was based on euclidean distances ....    
    //  The index is based on 8 bits of uv for the previous
    //  cell + the top 5 bytes of the current cell.
    Xil_unsigned8    uvlookup[8192];

    error_array        error;

    // Color conversion tables ycc601 -> rgb709.  If we ever support
    // other optimized conversions we may want to move these out of here.
    CellBColorConvertTable *convertTables;
    void initConvertTables();

private:
    Xil_boolean      isOKFlag;
};

#endif  // CELLBTYPECOMPRESSORDATA_HH
