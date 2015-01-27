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
//  File:	XilScanlineListPrivate.hh
//  Project:	XIL
//  Revision:	1.9
//  Last Mod:	10:22:13, 03/10/00
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
//------------------------------------------------------------------------
//	COPYRIGHT
//------------------------------------------------------------------------

#ifdef _XIL_PRIVATE_INCLUDES
#include "XiliList.hh"
#endif

#ifdef _XIL_PRIVATE_DATA
public:

                 XilScanlineList(unsigned int x,
                                 unsigned int y,
                                 unsigned int xsize,
                                 unsigned int ysize);

private:
    XilStatus    initValues(unsigned int max_cnt);

    void         initAsRect(double x1,
                            double y1,
                            double x2,
                            double y2);

    XilScanline* scanList;
    unsigned int numEntries;
    unsigned int curEntry;

    double       x1PixelAdj;
    double       y1PixelAdj;
    double       x2PixelAdj;
    double       y2PixelAdj;

    //
    //  The size of this is 256 - the sizeof/4 all the private
    //  data members listed above. If you add data members to this class
    //  you must add to the count to be subtracted from 256. Pointers
    //  count for 1, because they are the same size as a void*.
    //  Xil_boolean == 1 and XiliListPosition == 1, as they occupy four
    //  bytes the size of one void*.
    //
    //  SOL64:  Note in 64 bit Solaris a pointer is 8 bytes so this
    //          needs to be revised.
    // 
    void*        extraData[256 - 12];

#endif  // _XIL_PRIVATE_DATA




