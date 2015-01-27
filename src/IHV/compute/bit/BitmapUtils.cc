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
//  File:	BitmapUtils.cc
//  Project:	XIL
//  Revision:	1.5
//  Last Mod:	10:10:00, 03/10/00
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
//	COPYRIGHT
//------------------------------------------------------------------------
#pragma ident	"@(#)BitmapUtils.cc	1.5\t00/03/10  "

#include <math.h>
#include <stdlib.h>
#include "XiliUtils.hh"
#include "xili_geom_utils.hh"
#include "xili_interp_utils.hh"

//
// Creates a lookup table for bilinear interpolation
// of bit images. There are 16 different bit configurations
// on 4 pixels. For each configuration we precompute the
// value of the dst pixel, based on the location of the
// sample point. The location of the sample point is
// chosen on a 16x16 grid. (x and y of the sample point
// are determined within 1/15th.) Pixels labeled as:
//                   p00   p10
//                   p01   p11
//

int*
xili_make_BL_table()
{
    int    i;
    int    x, y;
    int    p00, p10, p01, p11;
    int    p0, p1, p;
    float  fracx, fracy;
    int *  table;

    table = new int[4096];

    for (i = 0; i < 16; i++) {	// for each configuration
	
	p00 = (i & 0x8) << 1;	// values of the corner pixels
	p10 = (i & 0x4) << 2;
	p01 = (i & 0x2) << 3;
	p11 = (i & 0x1) << 4;

	for (y = 0; y < 16; y++) {
	    
	    fracy = (float) y / 15.0;

	    for (x = 0; x < 16; x++) {
		
		fracx = (float)x / 15.0;

		XILI_GEOM_BLEND(p0, p00, p10, fracx); // interpolate along x
		XILI_GEOM_BLEND(p1, p01, p11, fracx);
		XILI_GEOM_BLEND(p, p0, p1, fracy);    // interpolate along y
		//
		// for each configuration determine dst pixel
		// value based on the sample point location
		//
		table[(x<<8)+(y<<4)+i] = p; 
	    }
	}
    }
    return table;
}

//
// Creates a lookup table for bicubic interpolation
// of bit images along row pixels. There are 16 different
// bit configurations on 4 pixels. For each configuration we
// precompute the value of the dst pixel, based on the location
// of the sample point. Sample point is chosen on 16 point
// subgrid. For the top row we have pixels
//
//              p__   p0_   p1_  p2_
//                        p_
//
// Note that you can choose the same table for both x and y
// interpolation.
//
int*
xili_make_BC_table()
{
    int i, x;			// table indeces
    float fracx, xx;
    int p__, p0_, p1_, p2_;
    int p_, q_;
    int *table;

    table = new int[256];

    for (i = 0; i < 16; i++) {	// for each of the configurations
	p__ = (i & 0x8) << 1;	
	p0_ = (i & 0x4) << 2;
	p1_ = (i & 0x2) << 3;
	p2_ = (i & 0x1) << 4;

	for (x = 0; x < 16; x++) { // subsample in x direction
	    fracx = (float)x / 15.0;
	    xx = fracx * (1.0 - fracx);
	    XILI_GEOM_BLEND(p_, p0_, p1_, fracx);
	    XILI_GEOM_BLEND(q_, (p1_+p__), (p2_+p0_), fracx);
	    q_ = p_ - (q_>>1);
	    XILI_GEOM_PERTURB(p_, q_, xx);

	    table[(x<<4)+i] = p_;
	}
    }
    return table;
}



