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
//  File:	XiliScanlineListInt.hh
//  Project:	XIL
//  Revision:	1.2
//  Last Mod:	10:22:05, 03/10/00
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
//  MT-level:  UNSAFE
//	
//------------------------------------------------------------------------
//	COPYRIGHT
//------------------------------------------------------------------------
#pragma ident	"@(#)XiliScanlineListInt.hh	1.2\t00/03/10  "

#ifndef _XIL_SCANLINE_LIST_HH
#define _XIL_SCANLINE_LIST_HH

class XiliScanlineInt {
public:
    int y;
    int x1;
    int x2;
};

class XiliScanlineListInt {
public:
    //
    //  Constructors
    //
                     XiliScanlineListInt(const double* x_array,
                                         const double* y_array,
                                         unsigned int  num_points);
    
                     ~XiliScanlineListInt();
    
    //
    //  Get a pointer to an array of XilScanline structures.
    //
    XiliScanlineInt* getScanlines(unsigned int* num_scanlines);

private:
    XilStatus        initValues(unsigned int max_cnt);

    void             initAsRect(double x1,
                                double y1,
                                double x2,
                                double y2);

    XiliScanlineInt* scanList;
    unsigned int     numEntries;
};

#endif // _XIL_SCANLINE_LIST_HH
