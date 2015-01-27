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
//  File:	XilConvexRegionListPrivate.hh
//  Project:	XIL
//  Revision:	1.12
//  Last Mod:	10:21:43, 03/10/00
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
#include "XiliConvexRegion.hh"
#endif

#ifdef _XIL_PRIVATE_DATA
private:
    //
    // WARNING: make sure that if you add anything to the
    // data members for this class that you correctly decrement
    // the size of the extra_data area listed below. This keeps
    // the size of the object the same moving forward and aims to
    // preserve binary compatibility at the GPI.
    //

    //
    //  The roiptr is a pointer to the full roi. This is used if the
    //  roi isn't modified at all to generate the convex region list,
    //  or when calling "reinit" to clip to a new box
    // 
    XilRoi*                              roiptr;

    //
    //  Keep track of the system state for error reporting
    //
    XilSystemState*                      stateptr;

    //
    //  Did we make a private copy of the region list while clipping it
    //  or are we pointing directly at the core's copy of the ROI.
    //
    Xil_boolean                          copyFlag;

    //
    //  If it's only one region, use this
    //
    XiliConvexRegion*                    regionRef;

    //
    //  When using regionRef, this flag is used to indicate when it's
    //  been read from so that the next call to getNext() can return NULL
    //
    Xil_boolean                          regionRefFirst;

    //
    //  If it's more than one region, use a region list
    //
    XiliList<XiliConvexRegion>*          regionListRef;

    //
    //  Where are we in the current list (used by getNext())
    //
    XiliListPosition                     currentPosition;

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
    void*                                extraData[256 - 7];


    //
    // Methods used by the different constructors to generate
    // the convex region list.
    //

    // roi & box
    XilStatus generateRegionList(XilRoi*     roi,
				 XilBox*     box,
				 Xil_boolean translateFlag);

    // roi a DGA cliplist
    XilStatus generateRegionList(XilRoi*    roi,
				 short*      cliplist,
				 int         winX,
				 int         winY,
				 Xil_boolean translateFlag);

    // XilConvexRegionList & box
    XilStatus generateRegionList(XilConvexRegionList* crlist,
				 XilBox*              box,
				 Xil_boolean          translateFlag);

    //
    // For those cases where a convexregion list is being created
    // from a roi that has an internal rectlist implementation,
    // we have to be generate the convexregion list on the fly
    // due to roi being MT-unsafe.
    //
    XilStatus createAndClipConvexRegionList(XilBox* box,
                                            Xil_boolean translate);


    //
    // Utilities to perform the intersection of the current
    // region and the intersect region and place the results
    // in the regionList or regionRef. Used by the generate* calls
    //
    // When translateFlag is set to FALSE, x1, y1 are ignored
    //
    XilStatus regionListIntersect(XiliConvexRegion* currentRegion,
				  XiliConvexRegion* intersect,
				  Xil_boolean       translateFlag,
				  float             x1,
				  float             y1);

    XilStatus regionRefIntersect(XiliConvexRegion*  currentRegion,
				 XiliConvexRegion*  intersect,
				 Xil_boolean        translateFlag,
				 float              x1,
				 float              y1);

#endif  // _XIL_PRIVATE_DATA




