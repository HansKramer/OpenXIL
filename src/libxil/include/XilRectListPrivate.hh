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
//   File:	XilRectListPrivate.hh
//   Project:	XIL
//   Revision:	1.17
//   Last Mod:	10:21:36, 03/10/00
//	
//	
//  Description:
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
#include "XiliRect.hh"
#endif

#ifdef _XIL_PRIVATE_DATA
public: 
private:
    //
    //  Each of the following 5 void return functions takes in a XiliRectInt*
    //  list and while doing some modification (clip and maybe adjust) copies
    //  some portion of the original list to a list starting at oneRect. They
    //  also are responsible for updating numRects and the bbox information
    //  appropriately. 
    //
    void                copyAndAdjustRectlist(XiliRectInt* first);
    void                copyAndClipRectlist(XiliRectInt* first,
                                            int x,
                                            int y,
                                            unsigned int xsize,
                                            unsigned int ysize);
    void                translateAndClipRectlist(XiliRectInt* clip_rect,
                                                 Xil_boolean adjust);
    void                clipRegionToRects(XiliConvexRegion* regionRef,
                                          XiliRectInt* clip_rect,
                                          Xil_boolean adjust);


#ifdef _XIL_HAS_LIBDGA
    void                copyAndIntersectWithDGAClipList(XiliRectInt* first,
                                                       short* cliplist,
                                                       int winX,
                                                       int winY);
    XilStatus           intersectBands(XiliRectInt* rect1,
                                       short* cliplist,
                                       int winX,
                                       int winY,
                                       int y1,
                                       int y2,
                                       int* count_numrects,
                                       XiliRectInt** listPtr);

    XilStatus          addNewRect(int new_x1,
                                  int new_y1,
                                  int new_x2,
                                  int new_y2,
                                  XiliRectInt** listPtr);
#endif //_XIL_HAS_LIBDGA

#ifdef DEBUG
    //
    //  Print the contents to stderr
    //
    void                  dump();
#endif

    //
    //  Used whenever there is an allocation error, to clean up
    //  any "new"d rects on the list and set numRects to zero.
    //
    void               cleanup();

    //
    //  This pointer keeps current position in the XilRect list (for getNext).
    //
    XiliRectInt*           rectAddr;

    //
    //  This pointer is a reference to the Roi's full rectlist representation.
    //
    XilRoi*                myRoi;

    //
    //  To optimize the single rectangle case, this avoids having to use new
    //  which in turns avoids calling a mutex.
    //
    XiliRectInt            oneRect;

    //
    //  This is used for building the copy of the rectlist when necessary.
    //  It points to the end of the rectlist for the next addition.
    //
    XiliRectInt*           endPtr;

    //
    //  The following is the bbox of the XilRectList. This may be different
    //  than the ROI's bbox because of adjustment and clipping to a box
    //
    int                    bboxX;
    int                    bboxY;
    unsigned int           bboxXsize;
    unsigned int           bboxYsize;
    
    //
    //  The number of rects in the XiliRect list
    //
    unsigned int           numRects;

    //
    //  Is the XiliRect list a copy of the rectlist_ref or reference?
    //
    Xil_boolean            rectlistCopy;

    //
    //  Keep a copy of the system state from the ROI object
    //  in case of any errors occuring 
    //
    XilSystemState*        stateptr;

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
    void*                  extraData[256 - 17];

#endif  // _XIL_PRIVATE_DATA




