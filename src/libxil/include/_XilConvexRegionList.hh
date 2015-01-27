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
//  File:	_XilConvexRegionList.hh
//  Project:	XIL
//  Revision:	1.15
//  Last Mod:	10:21:37, 03/10/00
//
//  Description:
//	
//
//  MT-level:  SAFE
//	
//------------------------------------------------------------------------
//	COPYRIGHT
//------------------------------------------------------------------------
#pragma ident	"@(#)_XilConvexRegionList.hh	1.15\t00/03/10  "

#ifndef _XIL_CONV_REGION_LIST_HH
#define _XIL_CONV_REGION_LIST_HH

#ifdef _XIL_LIBXIL_PRIVATE
#define _XIL_PRIVATE_INCLUDES

#include "XilConvexRegionListPrivate.hh"

#undef _XIL_PRIVATE_INCLUDES
#endif

class XilConvexRegionList {
public:
    //
    //  Get the next convex region on the list
    //
    Xil_boolean            getNext(const double** x_array,
				   const double** y_array,
				   unsigned int*  point_count);

    //
    //  Allows the original full ROI convex region list to be
    //  clipped by a different box
    //
    XilStatus              reinit(XilBox* dest_box);

    //
    //  Construction using a roi and a box.  The clipped regions in the list 
    //  are all relative to (0, 0) in the box.
    //
                           XilConvexRegionList(XilRoi* roi,
                                               XilBox* dest_box);
    
#ifdef _XIL_HAS_LIBDGA
    //
    //  The following constructor is only intended for use with I/O devices
    //  which may need to clip with a DGA cliplist. The convex region list
    //  is not translated.
    //
                            XilConvexRegionList(XilRoi* roi,
						short*  cliplist,
						int     winX,
						int     winY);
#endif // _XIL_HAS_LIBDGA

    //
    //  The following constructor confines the convex region list
    //  to the rect specified in the clip rectangle co-ordinates. The
    //  list is not translated to 0, 0 of the box.
    //
                           XilConvexRegionList(XilRoi*      roi,
					       int          x,
					       int          y,
					       unsigned int xsize,
					       unsigned int ysize);

    //
    //  Constructs a convex region list from a previous convex region list
    //  and a box. The list is translated so that the top corner of the box is
    //  0,0.
    //
                           XilConvexRegionList(XilConvexRegionList* clippedlist,
					       XilBox*              dest_box);


    //
    //  Constructs a convex region list from a previous convex region list and
    //  a co-ordinate area the list is not translated.
    //
                           XilConvexRegionList(XilConvexRegionList* clippedlist,
					       int                  x1,
					       int                  y1,
					       int                  x2,
					       int                  y2);


    //
    //  Destructor
    //
                           ~XilConvexRegionList();

private:
#ifdef _XIL_LIBXIL_PRIVATE
#define _XIL_PRIVATE_DATA
#include "XilConvexRegionListPrivate.hh"
#undef  _XIL_PRIVATE_DATA
#else
    //
    //  Data matching size of class.
    //
    void*                  _classData[256];
#endif
};
#endif // _XIL_CONV_REGION_LIST_HH
