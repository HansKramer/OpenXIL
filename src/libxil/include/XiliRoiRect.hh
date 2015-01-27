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

//This line lets emacs recognize this as -*- C++ -*- Code
//------------------------------------------------------------------------
//
//  File:	XiliRoiRect.hh
//  Project:	XIL
//  Revision:	1.7
//  Last Mod:	10:21:15, 03/10/00
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
#pragma ident	"@(#)XiliRoiRect.hh	1.7\t00/03/10  "

#ifndef _XIL_I_ROIRECT_H
#define _XIL_I_ROIRECT_H

//
//  C Includes
//

//
//  C++ Includes
//
#include "XiliRect.hh"
#include "XiliRoiBitmask.hh"
#include "XiliRoiConvexRegion.hh"
#include "XiliBox.hh"

class XiliRoiRect {
public:
    // Add the specified rectangle to the current rectlist representation of the ROI.
    XilStatus	      addRect(int x, int y, unsigned int width, unsigned int height);
    
    // Return a full copy of the XiliRoiRect and it's associated rectlist.
    XiliRoiRect*      getCopyRoiRect();

    // Return a reference to the current reclist representation of the ROI
    XiliRect*	      getRectList();
    
    // Return number of rects in the rectlist
    unsigned int      numRects();

    // Translate the bitmask roi representation to a rectlist representation
    XilStatus         translateBitmask(XiliRoiBitmask* bitmaskroi);

    // Translate the convex region representation to a rectlist representation
    XilStatus         translateConvexRegion(XiliRoiConvexRegion* convexregionroi);

    XilStatus         boundingBox(int* x1,
				  int* y1,
				  unsigned int* xsize,
				  unsigned int* ysize);

    XilStatus         addImage(XilImage* image);
    XilImage*         getAsImage();
    XilStatus         subtractRect(int x,
				   int y,
				   unsigned int xsize,
				   unsigned int ysize);
    XiliRoiRect*      intersect(XiliRoiRect* other_rl);
    XiliRoiRect*      unite(XiliRoiRect* other_rl);
    XiliRoiRect*      translate(int x,
				int y);
    void              dump();

                      XiliRoiRect(XilRoi* calling_roi);
                      ~XiliRoiRect();
private:
    void              consolidateRects(); 
    void              updateBoundingBox();
    XiliRect*         first;
    unsigned int      num_rects;
    XiliBox*          bbox;
    XilRoi*           myRoi;
};

#endif // _XIL_I_ROIRECT_H

