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
//  File:	XiliNearestIndexSelector.hh
//  Project:	XIL
//  Revision:	1.2
//  Last Mod:	10:22:27, 03/10/00
//
//  Description:
//	A highly cached and well optimized nearest color algorithm for
//	3 banded XIL_BYTE images.  It is used by the BYTE nearest color
//	compute routine and the Cell Adaptive Colormap Selection
//	algorithm.
//	
//  MT-level:  UnSAFE
//
//------------------------------------------------------------------------
//	COPYRIGHT
//------------------------------------------------------------------------
#pragma ident	"@(#)XiliNearestIndexSelector.hh	1.2\t00/03/10  "

#include <xil/xilGPI.hh>

//
//  Private classes which store the cache.
//
class XiliSubCube
{
public:
    int         mask[2];
    union {
        Xil_unsigned8*  ptr;
        int             index;
    };
    
    XiliSubCube(void) {
        mask[0] = 0;
        mask[1] = 0;
        index   = 0;
    }
};

class XiliHeadBucket
{
public:
    int*          blist;
    XiliSubCube*  sc;
    
    XiliHeadBucket(void) {
        blist = NULL;
        sc    = NULL;
    }
};

class XiliNearestIndexSelector
{
public:
    //
    //  Reset the table with a new lookup table.  Must be 3 banded XIL_BYTE
    //  lookup table. 
    //
    XilStatus     newColormap(XilLookupSingle* lookup);

    //
    //  Returns the lookup table entry closet to the given color.
    //
    //  -1 is returned in case of an error...
    //
    int           selectIndex(Xil_unsigned8 b0,
                              Xil_unsigned8 b1,
                              Xil_unsigned8 b2);

    //
    //  Update with a new system state...
    //
    void          setNewSystemState(XilSystemState* state);

                  XiliNearestIndexSelector(XilSystemState*  state,
                                           XilLookupSingle* lookup = NULL);

                  ~XiliNearestIndexSelector();

private:
    void          DeleteBucketList(XiliHeadBucket* ptr)
    {
        if(ptr) {
            delete ptr->sc;
            delete ptr->blist;
            delete ptr;
        }
    }

    //
    //  Initialize the specified bucket given by the key.
    //
    void          initBucket(int key);

    //
    //  Check whether the given color is in the bucket represented by the
    //  given key.
    //
    int             selectCheck(int key,
                                int b0,
                                int b1,
                                int b2);

    Xil_boolean      isOKFlag;
    
    //
    //  For error reporting...
    //
    XilSystemState* systemState;

    //
    //  The version of the lookup we're using...
    //
    XilVersion      version;

    //
    //  Table for computing the absolute value
    //
    int             absValTable[514];
    int*            absVal;

    //
    //  Table for computing the squares -- both positive and negative
    //
    int             squareValTable[514];
    int*            squareVal;

    //
    //  Used color lookup cube
    //
    int*    ucube;

    //
    //  The number of colors in the lookup table.
    //
    unsigned int     numEntries;
    unsigned int     offset;
    
    //
    //  The color tables.
    //
    Xil_unsigned8    b0[256];
    Xil_unsigned8    b1[256];
    Xil_unsigned8    b2[256];

    //
    //  2nd level used color lookup cube
    //
    Xil_unsigned8 (*cache2)[64];
    unsigned int  next_entry;

    //
    //  Array of pointers for Buckets
    //
    XiliHeadBucket** colors;
    int              init_colors[257];

    //
    //  Lookups for quickly building codes and entries...
    //
    unsigned int     codeB0[256];
    unsigned int     codeB1[256];
    unsigned int     codeB2[256];
    
    unsigned int     entrB0[256];
    unsigned int     entrB1[256];
    unsigned int     entrB2[256];
};
    
