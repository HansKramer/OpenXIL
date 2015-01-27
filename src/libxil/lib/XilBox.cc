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
//  File:	XilBox.cc
//  Project:	XIL
//  Revision:	1.32
//  Last Mod:	10:08:31, 03/10/00
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
#pragma ident	"@(#)XilBox.cc	1.32\t00/03/10  "

#include "_XilDefines.h"
#include "_XilBox.hh"
#include "XiliUtils.hh"

void
XilBox::getAsRect(int*          ret_x,
                  int*          ret_y,
                  unsigned int* ret_xsize,
                  unsigned int* ret_ysize)
{
    *ret_x     = x;
    *ret_y     = y;
    *ret_xsize = xSize;
    *ret_ysize = ySize;
}

void
XilBox::getAsCorners(int* ret_x1,
                     int* ret_y1,
                     int* ret_x2,
                     int* ret_y2)
{
    *ret_x1 = x;
    *ret_y1 = y;
    *ret_x2 = x + xSize - 1;
    *ret_y2 = y + ySize - 1;
}

void*
XilBox::getTag()
{
    return tag;
}

//
//  TODO: jlf  7/18/96  Need to think about clipping as rects instead of as
//                      corners...
//
Xil_boolean
XilBox::clip(int  clip_x1,
             int  clip_y1,
             int  clip_x2,
             int  clip_y2)
{
    //
    //  Convert to corners...
    //
    int x1;
    int y1;
    int x2;
    int y2;
    getAsCorners(&x1, &y1, &x2, &y2);

    x1 = _XILI_MAX(x1, clip_x1);
    x2 = _XILI_MIN(x2, clip_x2);
    y1 = _XILI_MAX(y1, clip_y1);
    y2 = _XILI_MIN(y2, clip_y2);

    //
    //  If it's clipped to an empty box, then set it as a 0 sized rect.
    //  and return FALSE allows callers to check for empty box
    //
    if((x2 < x1) || (y2 < y1)) {
        setAsRect(0, 0, 0, 0);
	return FALSE;
    }

    setAsCorners(x1, y1, x2, y2);

    return TRUE;
}

Xil_boolean
XilBox::clip(int           clip_x1,
             int           clip_y1,
             unsigned int  clip_xsize,
             unsigned int  clip_ysize)
{
    int clip_x2 = clip_x1 + clip_xsize - 1;
    int clip_y2 = clip_y1 + clip_ysize - 1;

    return clip(clip_x1, clip_y1, clip_x2, clip_y2);
}

Xil_boolean
XilBox::clip(XilBox* box)
{
    int clip_x1;
    int clip_y1;
    int clip_x2;
    int clip_y2;

    box->getAsCorners(&clip_x1, &clip_y1, &clip_x2, &clip_y2);

    return clip(clip_x1, clip_y1, clip_x2, clip_y2);
}

//
//  Right now, clip() is really an intersect().
//
Xil_boolean
XilBox::intersect(XilBox* box)
{
    int clip_x1;
    int clip_y1;
    int clip_x2;
    int clip_y2;

    box->getAsCorners(&clip_x1, &clip_y1, &clip_x2, &clip_y2);

    return clip(clip_x1, clip_y1, clip_x2, clip_y2);
}

//
//  Check for intersection but don't modify the current box.
//
Xil_boolean
XilBox::intersects(XilBox* box)
{
    //
    //  Convert to corners...
    //
    int x1;
    int y1;
    int x2;
    int y2;
    getAsCorners(&x1, &y1, &x2, &y2);

    int ox1;
    int oy1;
    int ox2;
    int oy2;
    box->getAsCorners(&ox1, &oy1, &ox2, &oy2);

    x1 = _XILI_MAX(x1, ox1);
    x2 = _XILI_MIN(x2, ox2);
    y1 = _XILI_MAX(y1, oy1);
    y2 = _XILI_MIN(y2, oy2);

    //
    //  If it's clipped to an empty box, then return FALSE -- no intersection.
    //
    if((x2 < x1) || (y2 < y1)) {
	return FALSE;
    }

    return TRUE;
}
