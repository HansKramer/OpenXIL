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
//  File:   AdaptiveColormapSelection.hh
//  Project:    XIL
//  Revision:   1.2
//  Last Mod:   10:23:32, 03/10/00
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
#pragma ident   "@(#)AdaptiveColormapSelection.hh	1.2\t00/03/10  "

#ifndef ADAPTIVECOLORMAPSELECTION_HH
#define ADAPTIVECOLORMAPSELECTION_HH

#include <math.h>
#include <memory.h>
#include <stdlib.h>
#include <stdio.h>

#include "xil/xilGPI.hh"

#include "CmapTable.hh"

struct SubCube
{
    int         mask[2];
    union {
        Xil_unsigned8*  ptr;
        int             index;
    };

    SubCube(void) {
        mask[0] = 0;
        mask[1] = 0;
        index = 0;
    }
};

struct HeadBucket
{
    int*      blist;
    SubCube*  sc;

    HeadBucket(void) {
        blist = NULL;
        sc = NULL;
    }
};


class AdaptiveColormapSelection
{
public:
    AdaptiveColormapSelection(void);

    ~AdaptiveColormapSelection(void);

    //
    //  Selection Member Functions
    //
    void           useNewColormap(const UsedCmapTable& cmap);
    void           clearStatTables(void);

    int  selectIndex(int r, int g, int b);
    int  selectIndexAdaptive(int r, int g, int b);

    void           getNextColorMap(UsedCmapTable& cmap);

    double  getColormapError(void) {
        return average_err;
    }
    AdaptiveColormapSelection*  ok(void);
    void           printStats(void);

private:
    void DeleteBucketList(HeadBucket* ptr) {
        if(ptr) {
            delete ptr->sc;
            delete ptr->blist;
            delete ptr;
        }
    }

    void           initBucket(int key);
    Xil_unsigned8  selectCheck(int key, int r, int g, int b);

    //  The ok flag and function
    Xil_boolean  isok;
    
    // Absolute Value Table
    int*  abs_table;
    int*  absval;

    // Used color lookup cube
    int*  ucube;

    // 2nd level used color lookup cube
    Xil_unsigned8 (*cache2)[64];
    unsigned int  next_entry;

    // Array of pointers for Buckets 
    HeadBucket** colors;
    int          init_colors[257];

    //  An array of random values for selecting thresholds
    double* randtbl;
    int     randcnt;
    int     randplace;
    double  optdist;  // optimum distribution (calls/colknt)

    // The Tables
    int     colknt;
    int	    red[256],grn[256],blu[256],use[256];
    long    trd[256],tgn[256],tbl[256],tkt[256];
    long    tr2[256],tg2[256],tb2[256];
    long    erm[256];
    int     rcode[256],gcode[256],bcode[256];
    int     rentr[256],gentr[256],bentr[256];
    int     clampthresh[256];

    // The cutoff thresholds
    int	    threlo, threhi;

    // Average Error for the Image
    double  average_err;

    // Some statistics
    int     kntbuckets;
    int     kntfirst;
    int     kntquick;
    int     kntquick2;
    int     kntslow;
    int     kntslow2;
    int     kntnocache;
};

#endif  // ADAPTIVECOLORMAPSELECTION_HH
