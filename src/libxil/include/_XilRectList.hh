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
//   File:	_XilRectList.hh
//   Project:	XIL
//   Revision:	1.26
//   Last Mod:	10:21:13, 03/10/00
//  
//   Description:
//	
//	
//  MT-level:  UNSAFE
//	
//------------------------------------------------------------------------
//	COPYRIGHT
//------------------------------------------------------------------------
#pragma ident	"@(#)XilRectList.hh	1.2\t95/07/12  "

#ifndef _XIL_RECT_LIST_HH
#define _XIL_RECT_LIST_HH

#include "_XilDefines.h"


#ifdef _XIL_LIBXIL_PRIVATE
#define _XIL_PRIVATE_INCLUDES

#include "XilRectListPrivate.hh"

#undef _XIL_PRIVATE_INCLUDES
#endif

class XilRectList {
public:
    //
    //  Fills in the pointers with the values from the next rect in the
    //  rect list.  The boolean will return FALSE when there are no more
    //  rectangles available. 
    //
    Xil_boolean            getNext(int*          x,
				   int*          y,
				   unsigned int* xsize,
				   unsigned int* ysize);

    int                    getNumRects();

    //
    //  Allows the ROI used in the constructor to be reclipped by a different
    //  box.
    //
    //  If the internal ROI is null (special case of XilRoi(roi, box)) then
    //  the rect list is reset to be one rectangle, that of the box.
    //
    XilStatus              reinit(XilBox* box);

    //
    //  Resets the rect list walking to start at the beginning of the list
    //
    void                   reset();

    
    //
    //  Constructors
    //

    //  Constructs a list of rectangles which represent the given ROI clipped
    //  by the given box.
    //
    //  Each rectangle in the new list will be relative to (0,0) being the
    //  start of the box.
    //
    //  If the ROI is NULL, but the box is valid, then the rect list contains
    //  a single rectangle representing the box.
    //
    //  If the box is NULL, the rect list is the entire ROI.
    //
                           XilRectList(XilRoi* roi,
                                       XilBox* box);

    //
    //  Constructs a list of rectangles which represent the given XilRectList
    //  clipped by the given (coordinate represented) rectangle.
    //
    //  The rectangles in the new list will be only be clipped and remain
    //  unchanged from their position in the XilRectList. 
    //
    //
                           XilRectList(XilRectList* rectlist,
                                       int          x1,
                                       int          y1,
                                       int          x2,
                                       int          y2);

    //
    //  The following three constructors are generally used by I/O devices.
    //
    //  Constructs a list of rectangles which represent the given ROI clipped
    //  by a specified rectangle.
    //
    //  Unlike being clipped by a box, the rectangles in the new list will be
    //  only be clipped and remain unchanged from their position in the ROI.
    //
    //
                           XilRectList(XilRoi*      roi,
                                       int          x,
                                       int          y,
                                       unsigned int xsize,
                                       unsigned int ysize);

#ifdef _XIL_HAS_LIBDGA
    //
    //  This constructor is a convenience for I/O display devices which need
    //  to intersect XIL's ROI with the DGA cliplist.
    //
    //  Unlike being clipped by a box, the rectangles in the new list will be
    //  only be clipped and remain unchanged from their position in the ROI.
    //
                           XilRectList(XilRoi* roi,
                                       short*  cliplist,
                                       int     win_x,
                                       int     win_y);
#endif // _XIL_HAS_LIBDGA    

    //
    //  Constructs a list of rectangles which represent the given XilRectList
    //  clipped by the given box.
    //
    //  Each rectangle in the new list will be relative to (0,0) being the
    //  start of the box.
    //
                           XilRectList(XilRectList* rectlist,
                                       XilBox*      box);

    //
    //  Destructor
    //
                           ~XilRectList();

private:
#ifdef _XIL_LIBXIL_PRIVATE
#define _XIL_PRIVATE_DATA
#include "XilRectListPrivate.hh"
#undef  _XIL_PRIVATE_DATA
#else
    //
    //  Data matching size of class.
    //
    void*                  _classData[256];
#endif
};
#endif // _XIL_RECT_LIST_HH
