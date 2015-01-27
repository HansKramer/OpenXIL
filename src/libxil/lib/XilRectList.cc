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
//  File:	XilRectList.cc
//  Project:	XIL
//  Revision:	1.50
//  Last Mod:	10:08:00, 03/10/00
//
//  Description:
//	
//	
//  MT-level:  SAFE
//  Note that the XilRoi that the rectlist stems from is inherently
//  MT-unsafe. A ROI contains 3 implementations of a region (convexregionlist,
//  rectlist and bitmask) any combination of which may be valid.
//  Translation from one "type" to another happens automatically. 
//  So if 2 threads were to request a rectlist from a ROI that only had
//  a convex region implementation, both would try to cause the translation
//  and step on each other. Locking the ROI would be performance ruining.
//  Since the XilRectList and XilConvexRegionList are the only places
//  where the GPI can access the ROI with multiple threads, we catch
//  the need for translation here and simply make a copy.
//	
//------------------------------------------------------------------------
//	COPYRIGHT
//------------------------------------------------------------------------
#pragma ident	"@(#)XilRectList.cc	1.2\t95/07/12  "

#ifdef _XIL_HAS_LIBDGA
//
// For DGA_Y_EOL, DGA_X_EOL
//
#include <dga/dga.h>
#endif

//
// C++ includes
//
#include "_XilDefines.h"
#include "_XilRectList.hh"
#include "_XilSystemState.hh"
#include "_XilRoi.hh"
#include "_XilBox.hh"
#include "_XilScanlineList.hh"
//
// Private includes
//
#include "XiliRect.hh"
#include "XiliUtils.hh"


//----------------------------------------------------------------
//
//
// XilRectList utility routines
//
//----------------------------------------------------------------

//----------------------------------------------------------------
//
//  copyAndAdjustRectlist
// 
//  This routine takes a pointer to the full Roi rectlist and
//  makes a copy of it while clipping it to the passed in Box.
//  and origin adjusting it to the passed in box.
//  It also marks that it has a copy for later cleanup.
//  It also sets the numRects and bounding box of the region
//
//----------------------------------------------------------------
int XilRectList::getNumRects() 
{
    return numRects;
}


void
XilRectList::copyAndAdjustRectlist(XiliRectInt* first)
{
    int adjustX1=0;
    int adjustY1=0;
    int adjustX2=0;
    int adjustY2=0;
    int clipX1 = bboxX;
    int clipY1 = bboxY;
    int clipX2 = bboxX+bboxXsize-1;
    int clipY2 = bboxY+bboxYsize-1;
        
  
    //
    //  Handle the case of empty rectlist
    //
    numRects = 0;
    endPtr = NULL;
    if(first == NULL) {
	return;
    }
    //
    //  Copy the first valid rect
    //
    XiliRectInt* iptr = first;
    while((iptr) && (endPtr == NULL)) {
        adjustX1 = _XILI_MAX(iptr->getX1(),clipX1) - clipX1;
        adjustX2 = _XILI_MIN(iptr->getX2(),clipX2) - clipX1;
        adjustY1 = _XILI_MAX(iptr->getY1(),clipY1) - clipY1; 
        adjustY2 = _XILI_MIN(iptr->getY2(),clipY2) - clipY1;

        if((adjustX2 < adjustX1) || (adjustY2 < adjustY1)) {
            iptr = (XiliRectInt*)iptr->getNext();
        } else {
            oneRect.set(adjustX1, adjustY1, adjustX2, adjustY2);
            oneRect.setNext(NULL);
            endPtr = &oneRect;

            //
            //  Initialize new bounding box values
            //
            bboxX = adjustX1;
            bboxY = adjustY1;
            bboxXsize = adjustX2-adjustX1+1;
            bboxYsize = adjustY2-adjustY1+1;

            numRects = 1;
            iptr = (XiliRectInt*)iptr->getNext();
        }
    }

    //
    //  Copy subsequent valid rects, adding them on to oneRect,
    //  and update bounding box
    // 
    while(iptr) {
        adjustX1 = _XILI_MAX(iptr->getX1(),clipX1) - clipX1;
        adjustX2 = _XILI_MIN(iptr->getX2(),clipX2) - clipX1;
        adjustY1 = _XILI_MAX(iptr->getY1(),clipY1) - clipY1; 
        adjustY2 = _XILI_MIN(iptr->getY2(),clipY2) - clipY1;

        if((adjustX2 < adjustX1) || (adjustY2 < adjustY1)) {
            iptr = (XiliRectInt*)iptr->getNext();
        } else {
            endPtr->setNext(new XiliRectInt(adjustX1, adjustY1, adjustX2, adjustY2));
	    if(!endPtr->getNext()) {
	        XIL_ERROR(stateptr, XIL_ERROR_RESOURCE, "di-1",TRUE);
                cleanup();
		return;
	    }

            bboxX = _XILI_MIN(bboxX, adjustX1);
            bboxY = _XILI_MIN(bboxY, adjustY1);
            bboxXsize = _XILI_MAX(bboxXsize, (Xil_unsigned32)adjustX2-adjustX1+1);
            bboxYsize = _XILI_MAX(bboxYsize, (Xil_unsigned32)adjustY2-adjustY1+1);

            numRects += 1;
            endPtr = (XiliRectInt*)endPtr->getNext();
            endPtr->setNext(NULL);
            iptr = (XiliRectInt*)iptr->getNext();
	}
    }
    return;
}

//----------------------------------------------------------------
//
//  copyAndClipRectlist
// 
//  This routine takes a pointer to the full Roi rectlist and
//  makes a copy of it while clipping it to the passed in Box.
//  This routine does no origina adjustment.
//  It also marks that it has a copy for later cleanup.
//  It also sets the numRects and bounding box of the region
//
//----------------------------------------------------------------
void
XilRectList::copyAndClipRectlist(XiliRectInt* first,
                                 int x,
                                 int y,
                                 unsigned int xsize,
                                 unsigned int ysize)
{
    int clipX1 = x;
    int clipY1 = y;
    int clipX2 = x+xsize-1;
    int clipY2 = y+ysize-1;
    int clippedX1 = 0;
    int clippedY1 = 0;
    int clippedX2 = 0;
    int clippedY2 = 0;

    //
    //  Handle the case of empty rectlist
    //
    endPtr = NULL;
    numRects = 0;
    if(first == NULL) {
	return;
    }
    //
    //  Copy the first valid rect
    //
    XiliRectInt* iptr = first;
    while((iptr) && (endPtr == NULL)) {
        clippedX1 = _XILI_MAX(iptr->getX1(), clipX1);
        clippedX2 = _XILI_MIN(iptr->getX2(), clipX2);
        clippedY1 = _XILI_MAX(iptr->getY1(), clipY1); 
        clippedY2 = _XILI_MIN(iptr->getY2(), clipY2);

        if((clippedX2 < clippedX1) || (clippedY2 < clippedY1)) {
            iptr = (XiliRectInt*)iptr->getNext();
        } else {
            oneRect.set(clippedX1, clippedY1, clippedX2, clippedY2);
            oneRect.setNext(NULL);
            endPtr = &oneRect;

            //
            // Initialize new bounding box values
            //
            bboxX = clippedX1;
            bboxY = clippedY1;
            bboxXsize = clippedX2-clippedX1+1;
            bboxYsize = clippedY2-clippedY1+1;
	    numRects = 1;
	    iptr = (XiliRectInt*)iptr->getNext();
	}
    }

    //
    // Copy subsequent valid rects and update bounding box
    // 
    while(iptr) {
        clippedX1 = _XILI_MAX(iptr->getX1(), clipX1);
        clippedX2 = _XILI_MIN(iptr->getX2(), clipX2);
        clippedY1 = _XILI_MAX(iptr->getY1(), clipY1); 
        clippedY2 = _XILI_MIN(iptr->getY2(), clipY2);

        if((clippedX2 < clippedX1) || (clippedY2 < clippedY1)) {
            iptr = (XiliRectInt*)iptr->getNext();
        } else {
	    endPtr->setNext(new XiliRectInt(clippedX1, clippedY1, clippedX2, clippedY2));
	    if(!endPtr->getNext()) {
	        XIL_ERROR(stateptr, XIL_ERROR_RESOURCE, "di-1",TRUE);
                cleanup();
		return;
	    }

            bboxX = _XILI_MIN(bboxX, clippedX1);
            bboxY = _XILI_MIN(bboxY, clippedY1);
            bboxXsize = _XILI_MAX(bboxXsize, (Xil_unsigned32)clippedX2-clippedX1+1);
            bboxYsize = _XILI_MAX(bboxYsize, (Xil_unsigned32)clippedY2-clippedY1+1);

            numRects += 1;
            endPtr = (XiliRectInt*)endPtr->getNext();
            endPtr->setNext(NULL);
            iptr = (XiliRectInt*)iptr->getNext();
	}
    }
}

#ifdef _XIL_HAS_LIBDGA

//----------------------------------------------------------------
//
//  addNewRect()
// 
//  Used by intersectBands below, this routine adds the
//  new rectangle specified by new_x1,new_y1 --> new_x2,new_y2
//  to list. Because list might be pointing to oneRect (initially)
//  the routine first checks to see if list is pointing to a valid
//  rect. If it's pointing to NULL, then the routines knows that
//  it needs to use "new".
//
//----------------------------------------------------------------

XilStatus
XilRectList::addNewRect(int          new_x1,
                        int          new_y1,
                        int          new_x2,
                        int          new_y2,
                        XiliRectInt** list)
{
    if(*list == NULL) {
        //
        //  We know that list is at the beginning, so just fill
        //  in oneRect. Do not use "new".
        //
        oneRect.set(new_x1, new_y1, new_x2, new_y2);
        oneRect.setNext(NULL);
        *list = &oneRect;
    } else {
        //
        //  oneRect is already full. We'll need to allocate this
        //  rect and attach it to the "next" of the list.
        //
        XiliRectInt* newrect;

        newrect= new XiliRectInt(new_x1, new_y1,
                                 new_x2, new_y2);
                                 
        if(newrect==NULL) {
            return XIL_FAILURE;
        }
        newrect->setNext(NULL);
        (*list)->setNext(newrect);
        (*list) = newrect;
        //
        //  list gets incremented by the calling routine
        //
    }
    return XIL_SUCCESS;
}


//----------------------------------------------------------------
//
//  intersectBands()
// 
//  This routine is used by copyAndIntersectWithDGACliplist.
//  It is a routine to take two bands which are known to overlap
//  and find the intersection.
//  It puts the intersection into listPtr. It checks whether
//  listPtr is null before "new"ing an XiliRectInt because
//  the first entry in the list is a static entry.
//  
//
//----------------------------------------------------------------
XilStatus
XilRectList::intersectBands(XiliRectInt* rect1,
                            short* cliplist,
                            int winX,
                            int winY,
                            int y1,
                            int y2,
                            int* count_numrects,
                            XiliRectInt** listPtr)
{
    int x1,x2;
    int rect1y= rect1->getY1();
    int rect2y= cliplist[0]-winY;
    int rect_counter;
    XilStatus status;
    
    rect_counter = 0;
    cliplist+=2; // skip the Y coordinates
    while((rect1y==rect1->getY1()) && ((x1=cliplist[0])!= DGA_X_EOL)) {
        x1-=winX;
        x2= cliplist[1]-winX-1;
        //
        // do they intersect in X? 
        //
        if((rect1->getX1() >= x1) && (rect1->getX1() <= x2)) {
            if(rect1->getX2() < x2) {
                status = addNewRect(rect1->getX1(), y1,
                                    rect1->getX2(), y2, listPtr);
            } else {
                status = addNewRect(rect1->getX1(), y1, x2, y2, listPtr);
            }
            if(status == XIL_FAILURE) {
                XIL_ERROR(NULL, XIL_ERROR_RESOURCE,"di-1",TRUE);
                //
                //  The calling routine copyAndIntersectWithDGACliplist
                //  will clean up the whole list on a return of FAILURE
                //
                return status;
            }
            rect_counter++;
        } else if((x1 >= rect1->getX1()) && (x1 <= rect1->getX2())) {
            if(x2 < rect1->getX2()) {
                status = addNewRect(x1, y1, x2, y2, listPtr);
            } else {
                status = addNewRect(x1, y1, rect1->getX2(), y2, listPtr);
            }
            if(status == XIL_FAILURE) {
                XIL_ERROR(NULL, XIL_ERROR_RESOURCE, "di-1",TRUE);
                //
                //  The calling routine copyAndIntersectWithDGACliplist
                //  will clean up the whole list on a return of FAILURE
                //
                return status;
            }
            rect_counter++;
        }
        
        //
        // advance the one with the smallest X
        //
        if(rect1->getX2() < x2) {
            rect1= (XiliRectInt*)rect1->getNext();
            if(rect1==NULL) break;
        } else {
            cliplist+=2;
        }
    }
    
    *count_numrects = rect_counter;
    return XIL_SUCCESS;
}

//----------------------------------------------------------------
//
//  copyAndIntersectWithDGAClipList
// 
//  This routine takes a pointer to the full Roi rectlist and
//  makes a copy of it while clipping it to the passed in ClipList.
//  It also sets the numRects and bounding box of the region
//
//----------------------------------------------------------------
void
XilRectList::copyAndIntersectWithDGAClipList(XiliRectInt* first,
                                             short* cliplist,
                                             int winX,
                                             int winY)
{
    //
    // TODO: maynard - update the bounding box to the new dimensions
    //

    XiliRectInt* thisrect;
    int this_y1, this_y2, y1, y2, common_y1, common_y2;

    numRects = 0;

    //
    // if either of the lists is empty just return an empty ROI 
    //
    if ((first == NULL) || (cliplist[0]==DGA_Y_EOL)) {
        return;
    }

    //
    //  endPtr must start pointing to NULL for the rect
    //  addition to correctly put the first rect into
    //  the static "oneRect" before "newing" and putting any
    //  additional rects onto the end.
    //
    endPtr = NULL;
    thisrect= first;

    while((thisrect != NULL) && ((y1=cliplist[0]) != DGA_Y_EOL)) {
        int count_numrects = 0;
        XilStatus status;

        y1-=winY;
        y2= cliplist[1]-winY-1;
        this_y1= thisrect->getY1();
        this_y2= thisrect->getY2();
        
        //
        // do they intersect in Y?
        //
        if((this_y1 >= y1) && (this_y1 <= y2)) {
            //
            // they intersect, what do they have in common?
            //
            common_y1= this_y1;
            if(y2 < this_y2) {
                common_y2= y2;
            } else {
                common_y2= this_y2;
            }
            
            //
            // intersect the bands
            //
            status = intersectBands(thisrect, cliplist, winX, winY,
                                    common_y1, common_y2,
                                    &count_numrects, &endPtr);
            if(status == XIL_FAILURE) {
                //
                //  Error message came from intersectBands - we just
                //  need to clean up the rest of the list
                //  
                cleanup();
                return;
            }
            numRects += count_numrects;
            
        } else if((y1 >= this_y1) && (y1 <= this_y2)) {
            //
            // they intersect, what do they have in common?
            //
            common_y1= y1;
            if(this_y2 < y2) {
                common_y2= this_y2;
            } else {
                common_y2= y2;
            }
            
            //
            // intersect the bands
            //
            status = intersectBands(thisrect, cliplist, winX, winY,
                                    common_y1, common_y2,
                                    &count_numrects, &endPtr);

            if(status == XIL_FAILURE) {
                //
                //  Error message came from intersectBands - we just
                //  need to clean up the rest of the list
                //  
                cleanup();
                return;
            }
                
            numRects += count_numrects;
        }
        
        //
        // advance the one that has advanced in the Y direction the least
        //
        if(y2 < thisrect->getY2()) {
            while(*cliplist++ != DGA_X_EOL);
        } else {
            while(thisrect && (thisrect->getY2()==this_y2)) {
                thisrect= (XiliRectInt*)thisrect->getNext();
            }
        }
    }
}



//----------------------------------------------------------------
//
//  XilRectList constructor
// 
//  This constructor is intended for use only with DGA which needs
//  to clip the roi against a given DGA cliplist, and offset by
//  the winX and winY
//
//----------------------------------------------------------------
XilRectList::XilRectList(XilRoi* roi,
                         short* cliplist,
                         int winX,
                         int winY)
{

    if(roi != NULL) {
        myRoi = roi;
        if(!roi->isValid() || (cliplist == NULL)) {
            //
            // If the ROI is empty set the rectlist to NULL
            // If NULL cliplist, set the rectlist to NULL
            //
            numRects = 0;
            rectAddr = NULL;
            rectlistCopy = TRUE;
        } else {
            if(roi->isRectValid() == TRUE) {
                //
                // numRects and clipbox get updated in copyAndIntersectWithDGAClipList
                //
                copyAndIntersectWithDGAClipList(myRoi->getRectList(), cliplist, winX, winY);
            } else {
                //
                // This roi is in some other form. We'll need to 
                // make a translate it on the fly, making a copy and clipping
                // it to the necessary cliplist. We can't rely on the ROI's
                // internal translation routines because they're not MT-safe.
                // TRUE indicates we want to adjust relative to 0,0 of clipbox
                //
                myRoi->getIntBoundingBox(&bboxX, &bboxY, &bboxXsize, &bboxYsize);
                translateAndClipRectlist(NULL,FALSE);
                
                //
                //        This code should be optimized to avoid copying the
                //        list. The copy is required because copyAndIntersectWithDGACliplist
                //        needs a rectlist that it can read from while writing to
                //        oneRect.
                //
                
                //
                //  Create the first rectangle of the copy on the stack
                //  for better performance (no "new")
                //
                XiliRectInt copy_rect(oneRect);
                XiliRectInt* copy_list = &copy_rect;
                
                //
                //  Move everything after oneRect to the new list's start
                //  and detail the list from oneRect 
                //
                copy_list->setNext((XiliRectInt*)oneRect.getNext());
                oneRect.setNext(NULL);
                
                //
                // Take the full rectlist copy and clip against the cliplist
                //
                copyAndIntersectWithDGAClipList(copy_list, cliplist,winX, winY);
                
                //
                //  If there was more than one rect copied, clean up
                //  the remainder beyond the first static rect.
                //  copy_rect is cleaned up automatically.
                //
                XiliRectInt* temp;
                temp = (XiliRectInt*)copy_list->getNext();
                while(temp) {
                    copy_list = (XiliRectInt*)temp->getNext();
                    delete temp;
                    temp = copy_list;
                }        
            }
            
            if(numRects > 0) {
                rectAddr = &oneRect;
            } else {
                rectAddr = NULL;
            }
            rectlistCopy = TRUE;
        }
        //
        //  Keep a reference to the system state around so
        //  we can generate errors
        //
        stateptr = myRoi->getSystemState();
    }
}


#endif //#ifdef _XIL_HAS_LIBDGA

//----------------------------------------------------------------
//
//  XilRectList constructor
// 
// Constructs a rectlist from the rectlist implementation of the ROI clipped to the
// dest box. Each rectangle in the new list will be relative to 0,0 of the box.
// If the box is null, the rectlist is the full rectlist of the ROI.
// If the rectlist implementation of the roi isn't valid, it creates a new
// rectlist which holds the translated/clipped/adjusted rects from the
// valid convex region implementation.
//  
//
//----------------------------------------------------------------
XilRectList::XilRectList(XilRoi* roi,
                         XilBox* clipbox)
{
    if(roi != NULL) {
        myRoi = roi;
        if(!roi->isValid()) {
            //
            //  The ROI is empty, so return an empty Rectlist
            //
            numRects = 0;
            rectAddr = NULL;
            rectlistCopy = TRUE;
        } else if(roi->isRectValid()==TRUE) {
            if(clipbox == NULL) {
                //
                // Not clipping, therefore use reference to full list
                //
                rectAddr = myRoi->getRectList();
                myRoi->getIntBoundingBox(&bboxX, &bboxY, &bboxXsize, &bboxYsize);
                rectlistCopy = FALSE;
                numRects = myRoi->numRects();
            } else {
                clipbox->getAsRect(&bboxX, &bboxY, &bboxXsize, &bboxYsize);
                copyAndAdjustRectlist(myRoi->getRectList());
                //
                // numRects and clipbox get updated in copyAndAdjustRectlist
                //
                if(numRects > 0) {
                    rectAddr = &oneRect;
                } else {
                    rectAddr = NULL;
                }
                rectlistCopy = TRUE;
            }
        } else {
            //
            // This ROI is in some other form. We'll need to
            // make a translate it on the fly, making a copy and clipping
            // it to the necessary region. We can't rely on the ROI's
            // internal translation routines because they're not MT-safe.
            // TRUE indicates we want to adjust relative to 0,0 of clipbox
            //
            if(clipbox == NULL) {
                myRoi->getIntBoundingBox(&bboxX, &bboxY, &bboxXsize, &bboxYsize);
                translateAndClipRectlist(NULL,FALSE);
            } else {
                int x2,y2;
                XiliRectInt clip_rect(clipbox);

                clip_rect.get(&bboxX, &bboxY, &x2, &y2);
                bboxXsize = x2-bboxX+1;
                bboxYsize = y2-bboxY+1;
                translateAndClipRectlist(&clip_rect,TRUE);
            }
            if(numRects > 0) {
                rectAddr = &oneRect;
            } else {
                rectAddr = NULL;
            }
            rectlistCopy = TRUE;
        }
        //
        //  Keep a reference to the system state around so
        //  we can generate errors
        //
        stateptr = myRoi->getSystemState();
    } else {
        //
        // This is a special case. If box in this case is valid,
        // create a simple roi which is just the box. Otherwise,
        // it's just empty.
        //
        myRoi = NULL;
        if(clipbox != NULL) {
            oneRect.set(clipbox);
            oneRect.setNext(NULL);
            clipbox->getAsRect(&bboxX, &bboxY, &bboxXsize, &bboxYsize);
            rectAddr = &oneRect;
            numRects = 1;
            rectlistCopy = TRUE;
        } else {
            numRects = 0;
            rectlistCopy = FALSE;
        }
    }
}

//------------------------------------------------------------------
//
// XilRectList(XilRectList*,XilBox*)
// Create new XilRectList with a copy of the incoming XiliRectInt*
// information, copied and adjusted for the incoming box.
// This is for use solely by IO devices which when using
// DGA need to have the ROI clipped against a cliplist as
// well as against the boxes in the boxlist.
// We know that we're safe from an MT standpoint because the
// rectlist already exists.
// Note however, that the rectlist may be empty, in which case
// return an empty rectlist.
//
//------------------------------------------------------------------
XilRectList::XilRectList(XilRectList* cliprl,
                         XilBox*      clipbox)
{
    
    XiliRectInt* cliprl_ref;

    myRoi = cliprl->myRoi;
    if(cliprl->numRects == 0) {
        //
        //  Special case - the clipped rectlist is empty
        //
        rectAddr = NULL;
        numRects = 0;
        rectlistCopy = TRUE;
        return;
    }
    if(cliprl->rectlistCopy == FALSE) {
        //
        //  myRoi can't be NULL in this case.
        //
        cliprl_ref = myRoi->getRectList();
    } else {
        cliprl_ref = &(cliprl->oneRect);
    }
    if(clipbox == NULL) {
        
        //
        //  Set the box to the bbox of the list
        //  we're copying from.
        //
        bboxX = cliprl->bboxX;
        bboxY = cliprl->bboxY;
        bboxXsize = cliprl->bboxXsize;
        bboxYsize = cliprl->bboxYsize;

        //
        //  Don't adjust because there wasn't really a box.
        //  Because the bbox matches, this is effectivndo
        //  list copy.
        //
        copyAndClipRectlist(cliprl_ref,bboxX, bboxY, bboxXsize, bboxYsize);
    } else {
        clipbox->getAsRect(&bboxX, &bboxY, &bboxXsize, &bboxYsize);
        copyAndAdjustRectlist(cliprl_ref);
    }

    //
    // numRects and bbox get updated in copyAndAdjustRectlist
    //
    if(numRects > 0) {
        rectAddr = &oneRect;
    } else {
        rectAddr = NULL;
    }
    rectlistCopy = TRUE;
}

//------------------------------------------------------------------
//
// XilRectList(XilRectList*, x1, y1, x2, y2)
// Create new XilRectList with a copy of the incoming XiliRectInt*
// information, copied and clipped but not adjusted for the incoming coordinates.
// We're safe from an MT standpoint because a rectlist already exists
//
//------------------------------------------------------------------
XilRectList::XilRectList(XilRectList* cliprl,
                         int x1,
                         int y1,
                         int x2,
                         int y2)
{
    
    XiliRectInt* cliprl_ref;
    unsigned int xsize = x2 - x1 + 1;
    unsigned int ysize = y2 - y1 + 1;

    myRoi = cliprl->myRoi;
    if(cliprl->numRects == 0) {
        //
        //  Special case - the clipped rectlist is empty
        //
        rectAddr = NULL;
        numRects = 0;
        rectlistCopy = TRUE;
        return;
    }
    if(cliprl->rectlistCopy == FALSE) {
        //
        //  myRoi cannot be NULL in this case.
        //
        cliprl_ref = myRoi->getRectList();
    } else {
        cliprl_ref = &(cliprl->oneRect);
    }
    copyAndClipRectlist(cliprl_ref, x1, y1, xsize, ysize);
    //
    // numRects and bbox get updated in copyAndClipRectlist
    //
    if(numRects > 0) {
        rectAddr = &oneRect;
    } else {
        rectAddr = NULL;
    }
    rectlistCopy = TRUE;
}

//------------------------------------------------------------------
// XilRectList constructor
//
// In this case the box values are taken directly and the returned
// rectlist is not relative to 0,0 of the box.
//
//------------------------------------------------------------------

XilRectList::XilRectList(XilRoi* roi,
                         int clipx,
                         int clipy,
                         unsigned int clipxsize,
                         unsigned int clipysize)
{
    if(roi != NULL) {
        myRoi = roi;
        if(!roi->isValid()) {
            //
            //  The ROI is empty, so return an empty Rectlist
            //
            numRects = 0;
            rectAddr = NULL;
            rectlistCopy = TRUE;
        } else if(roi->isRectValid() == TRUE) {
            copyAndClipRectlist(myRoi->getRectList(), clipx, clipy, clipxsize, clipysize);

            //
            // numRects and bbox get updated in copyAndClipRectlist
            //
            if(numRects > 0) {
                rectAddr = &oneRect;
            } else {
                rectAddr = NULL;
            }
            rectlistCopy = TRUE;
            
        } else {
            //
            // This ROI is in some other form. We'll need to
            // make a translate it on the fly, making a copy and clipping
            // it to the necessary region. We can't rely on the ROI's
            // internal translation routines because they're not MT-safe.
            // FALSE indicates that we don't want to adjust relative to
            // the clipbox 0,0
            //

            XiliRectInt clip_rect(clipx,clipy,clipx+clipxsize-1,clipy+clipysize-1);
            translateAndClipRectlist(&clip_rect,FALSE);
            if(numRects > 0) {
                rectAddr = &oneRect;
            } else {
                rectAddr = NULL;
            }
            rectlistCopy = TRUE;

            //
            //TODO: maynard - this isn't really right. it might
            // be much smaller.
            //
            bboxX = clipx;
            bboxY = clipy;
            bboxXsize = clipxsize;
            bboxYsize = clipysize;
        }
        //
        //  Keep a reference to the system state around so
        //  we can generate errors
        //
        stateptr = myRoi->getSystemState();
    }
}

    
XilRectList::~XilRectList()
{
    //
    // If we made a copy of the rectlist reference, we must destroy it.
    //
    if((rectlistCopy) && (numRects >0)) {
        cleanup();
    }
    rectAddr = NULL;
}

//------------------------------------------------------------------
//
// XilRectList::getNext()
//
// The GPI call to allow getting the next 4 values off a given rectlist.
// If there are no more rects in the rectlist, the return value is FALSE
//
//------------------------------------------------------------------
Xil_boolean
XilRectList::getNext(int*          ret_x,
                     int*          ret_y,
                     unsigned int* ret_xsize,
                     unsigned int* ret_ysize)
{
    if(rectAddr == NULL) {
        //
        //There are no more in the list
        //
        return FALSE;
    } else {
        //
        // We have an active rectlist, return the requested data
        //
        *ret_x = rectAddr->getX1();
	*ret_y = rectAddr->getY1();
	*ret_xsize = (unsigned int)(rectAddr->getX2()-rectAddr->getX1()+1);
	*ret_ysize = (unsigned int)(rectAddr->getY2()-rectAddr->getY1()+1);
        rectAddr = (XiliRectInt*)rectAddr->getNext();
	return TRUE;
    }
}


//------------------------------------------------------------------
// XilRectList::reinit()
//
// This function takes a new clipbox and reclips the full ROI rectlist
// to this new box. It is a convenience.
//
//------------------------------------------------------------------
XilStatus
XilRectList::reinit(XilBox* clipbox)
{
    if(clipbox == NULL) {
        return XIL_SUCCESS;
    }

    //
    //  First clean up the existing list beyond oneRect
    //  if there is one.
    //
    if((rectlistCopy) && (numRects > 0)){
        cleanup();
    }
    rectAddr = NULL;
    numRects = 0;

    //
    //  Now create a new list using the new clipbox.
    //
    clipbox->getAsRect(&bboxX, &bboxY, &bboxXsize, &bboxYsize);

    if(myRoi != NULL) {
        if(!myRoi->isValid()) {
            //
            //  The ROI is empty, so return an empty Rectlist
            //
            numRects = 0;
        } else if(myRoi->isRectValid() == TRUE) {
            copyAndAdjustRectlist(myRoi->getRectList());
        } else {
            //
            // This ROI is in some other form. We'll need to
            // make a translate it on the fly, making a copy and clipping
            // it to the necessary region. We can't rely on the ROI's
            // internal translation routines because they're not MT-safe.
            // TRUE indicates we want to adjust relative to 0,0 of clipbox
            //
            XiliRectInt clip_rect(clipbox);
            translateAndClipRectlist(&clip_rect,TRUE);
        }
    } else {
        //
        // Special case - no ROI, just copy box.
        //
        oneRect.set(clipbox);
        oneRect.setNext(NULL);
        numRects = 1;
    }
    if(numRects > 0) {
        rectAddr = &oneRect;
    } else {
        rectAddr = NULL;
    }
    rectlistCopy = TRUE;

    return XIL_SUCCESS;
}


//------------------------------------------------------------------
// XilRectList::reset()
//
// This function resets the rectlist walking back
// to the beginning of the rectlist
//
//------------------------------------------------------------------
void
XilRectList::reset()
{
    if(rectlistCopy == TRUE) {
        if(numRects > 0) {
            rectAddr = &oneRect;
        } else {
            rectAddr = NULL;
        }
    } else {
        rectAddr = myRoi->getRectList();
    }
    return;
}


void
XilRectList::clipRegionToRects(XiliConvexRegion* regionRef,
                               XiliRectInt*      clip_rect,
                               Xil_boolean       adjust)
{
    int adjust_x = 0;
    int adjust_y = 0;
        
    if((adjust) && (clip_rect != NULL)) {
        adjust_x = clip_rect->getX1();
        adjust_y = clip_rect->getY1();
    }

    if(regionRef->isRect() == TRUE) {
        //
        //  The convex region is a rectangle in disguise. Do the
        //  simpler clip and rectangle generation.
        //
        XiliRectInt new_region(regionRef->lowX,
                               regionRef->lowY,
                               regionRef->highX,
                               regionRef->highY);

        //
        //  If we have something to clip it against, do so.
        //
        if(clip_rect != NULL) {
            new_region.clip(clip_rect);
        }

        //
        //  Converting the region from doubles to integers could have caused
        //  it to clip to nothing if either the width or height was something
        //  less than one.
        //
        if(new_region.isEmpty()) {
            return;
        }

        new_region.translate(-adjust_x, -adjust_y);

        if(numRects == 0) {
            //
            //  Put the values in oneRect
            //
            oneRect.set(&new_region);
            oneRect.setNext(NULL);
            endPtr = &oneRect;
            numRects = 1;
        } else {
            //
            //  Create a new XiliRect, add it to the end and update numRects
            //
            XiliRectInt* new_rect;
            new_rect = new XiliRectInt(&new_region);
            if(new_rect == NULL) {
                XIL_ERROR(stateptr, XIL_ERROR_RESOURCE, "di-1",TRUE);
                cleanup();
                return;
            }
            endPtr->setNext(new_rect);
            endPtr = new_rect;
            endPtr->setNext(NULL);
            numRects += 1;
        }
    } else {
        //
        // There's only one region, but it's not a rectangle.
        // Intersect the region with the clipping region and
        // scan convert the result into a rectlist.
        //
        XiliConvexRegion small_region;
        XiliConvexRegion clip_region;

        if(clip_rect != NULL) {
            clip_region.set(clip_rect);
            if(regionRef->intersect(&clip_region,
                                    &small_region) == XIL_FAILURE) {
                return;
            }
        } else {
            small_region = *regionRef;
        }

        if(small_region.pointCount == 0) {
            //
            //  The intersection generated an empty convex region
            //
            return;
        }

        if(small_region.isRect()) {
            //
            // The convex region is a rectangle in disguise. Do the
            // simpler clip and rectangle generation.
            //
            XiliRectDbl new_region(regionRef->lowX,
                                   regionRef->lowY,
                                   regionRef->highX,
                                   regionRef->highY);


            new_region.translate(-adjust_x, -adjust_y);

            //
            // We have a valid rect to add, 
            //
            if(numRects == 0) {
                //
                //  Put the values in oneRect
                // 
                oneRect.set(&new_region);
                oneRect.setNext(NULL);
                endPtr = &oneRect;
                numRects = 1;
            } else {
                //
                //  Create a new XiliRect, add it to the end and update numRects
                //
                XiliRectInt* new_rect;
                new_rect = new XiliRectInt(&new_region);
                if(new_rect == NULL) {
                    XIL_ERROR(stateptr, XIL_ERROR_RESOURCE, "di-1",TRUE);
                    cleanup();
                    return;
                }
                endPtr->setNext(new_rect);
                endPtr = new_rect;
                endPtr->setNext(NULL);
                numRects += 1;
            }
        } else {
            //
            //  It's not a rectangle, we need to scan convert the small region
            //  into rectangles  
            //
            XilScanlineList  sl_list(NULL,
                                     small_region.xPtArray,
                                     small_region.yPtArray,
                                     small_region.pointCount);

            unsigned int sly;
            unsigned int slx1, slx2;
            while(sl_list.getNext(&sly, &slx1, &slx2) == TRUE) {
                //
                // add scanlines to the rectlist
                //
                if(numRects == 0) {
                    //
                    //  Don't create the first rectangle
                    //
                    oneRect.set((int)slx1 - adjust_x,
                                (int)sly  - adjust_y,
                                (int)slx2 - adjust_x,
                                (int)sly  - adjust_y);
                    oneRect.setNext(NULL);
                    endPtr = &oneRect;
                    numRects = 1;
                } else {
                    XiliRectInt* new_rect =
                        new XiliRectInt(slx1 - adjust_x, sly - adjust_y,
                                        slx2 - adjust_x, sly - adjust_y);
                    if(!new_rect) {
                        XIL_ERROR(myRoi->getSystemState(),
                                  XIL_ERROR_RESOURCE, "di-1", TRUE);
                        cleanup();
                        return;
                    }
                    endPtr->setNext(new_rect);
                    endPtr = new_rect;
                    endPtr->setNext(NULL);
                    numRects += 1;
                }
            }
        }
    }
}

void
XilRectList::translateAndClipRectlist(XiliRectInt* clip_rect,
                                      Xil_boolean adjust)
{
    //
    //  Because multiple threads could be generating an XilRectList
    //  from the same roi, we can't rely on the ROI's internal
    //  translation methods as they're completely MT-unsafe.
    //  When translation is required, we need to translate a copy.
    //
    numRects = 0;
    if(myRoi->isConvexRegionValid()) {
        XiliConvexRegion* regionRef;
        //
        //  Get convex region list reference
        //  If there is more than one convex region, get the list instead
        //
        if((regionRef = myRoi->getConvexRegion()) == NULL) {
                
            //
            //  Get a pointer to the region list
            //
            XiliList<XiliConvexRegion>* regionListRef = myRoi->getConvexRegionList();

            //
            // Loop the regions in the myRoi and intersect with
            // the destination box
            //
            XiliListIterator<XiliConvexRegion> crl_iterator(regionListRef);

            XiliConvexRegion*                  current_region;
            while((current_region = crl_iterator.getNext()) != NULL) {

                //
                //  This starts with oneRect and adds additional rects
                //  on to the end as needed. numRects gets updated.
                //
                clipRegionToRects(current_region,clip_rect,adjust);
            }
        } else {
            //
            // We got a single region from the incoming convex region list
            // Clip the region to the clip_rect, and turn it into
            // a rectlist, putting in oneRect and adding to that as needed.
            //
            clipRegionToRects(regionRef,clip_rect,adjust);
        }
    } else {
        //
        // either Bitmask or nothing valid. Not reasonable
        //
        XIL_ERROR(myRoi->getSystemState(), XIL_ERROR_SYSTEM, "di-8", TRUE);
    }
}

//
//  This needs to be done in so many places, it's worth
//  making it a routine. Anytime a "new" fails, the
//  whole list gets cleaned up and we return.
//
void
XilRectList::cleanup()
{
    XiliRectInt* tmp1;
    XiliRectInt* tmp2; 

    //
    // Don't remove oneRect
    //
    tmp1 = (XiliRectInt*)oneRect.getNext();
    while(tmp1) {
        tmp2 = (XiliRectInt*)tmp1->getNext();
        delete tmp1;
        tmp1 = tmp2;  
    }        
    oneRect.setNext(NULL);
    numRects = 0;
}


#ifdef DEBUG
void
XilRectList::dump()
{
    if(numRects == 0) {
        fprintf(stderr,"\n EMPTY RECTLIST\n");
        return;
    }
    if(rectlistCopy) {
        fprintf(stderr, "\n    entry %d - ", 0);
        oneRect.dump();
        XiliRectInt* tmp = &oneRect;
        for(unsigned int i=1; i<numRects; i++) {
            tmp = (XiliRectInt*)tmp->getNext();
            fprintf(stderr, "    entry %d - ", i);
            tmp->dump();
        }
        fprintf(stderr,"\n");
        //
        //  Check to make sure that the list ends with nil
        //
        if(((XiliRectInt*)tmp->getNext()) != NULL) {
            fprintf(stderr,"Rectlist did not end with NULL!\n");
        }
        fflush(stderr);
    } else {
        myRoi->dump();
    }
}
#endif

