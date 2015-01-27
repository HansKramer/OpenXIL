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
//  File:	XiliRoiRect.cc
//  Project:	XIL
//  Revision:	1.55
//  Last Mod:	10:08:17, 03/10/00
//
//  Description: This file is the RectList implementation of the ROI.
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
#pragma ident	"@(#)XiliRoiRect.cc	1.55\t00/03/10  "

#include "_XilDefines.h"
#include "_XilBox.hh"
#include "_XilRoi.hh"
#include "_XilImage.hh"
#include "_XilStorage.hh"
#include "_XilSystemState.hh"

#include "XiliRect.hh"
#include "XiliUtils.hh"
#include "XiliScanlineListInt.hh"


//------------------------------------------------------------------------
//
//	Private Routines
//
//------------------------------------------------------------------------

XiliRectInt*
XiliRoiRect::intersectBands(XiliRectInt* rect1, 
                            XiliRectInt* rect2, 
                            int y1, 
                            int y2, 
                            int* out_numrects)
{
    XiliRectInt* head=NULL;
    XiliRectInt* rectlistptr;
    XiliRectInt* newrect;
    int rect1y= rect1->getY1();
    int rect2y= rect2->getY1();
    int numrects;
    
    numrects = 0;
    while((rect1y==rect1->getY1()) && (rect2y==rect2->getY1())) {
        // do they intersect in X?
        if((rect1->getX1() >= rect2->getX1()) && (rect1->getX1() <= rect2->getX2())) {
	    if(rect1->getX2() < rect2->getX2()) {
	        newrect= new XiliRectInt(rect1->getX1(), y1, rect1->getX2(), y2);
	    } else {
	        newrect= new XiliRectInt(rect1->getX1(), y1, rect2->getX2(), y2);
	    }
	    if(newrect==NULL) {
	        XIL_ERROR(myRoi->getSystemState(), XIL_ERROR_RESOURCE, "di-1",TRUE);
		while(head) {
		    newrect= (XiliRectInt*)head->getNext();
		    delete head;
		    numrects--;
		    head= newrect;
		}
// TODO:maynard:There must be a better way to do this
		return((XiliRectInt*)-1);
	    }
	    numrects++;
	    if(head==NULL) {
	        rectlistptr= head = newrect;
	    } else {
	        rectlistptr->setNext(newrect);
		rectlistptr= newrect;
	    }
	} else if((rect2->getX1() >= rect1->getX1()) && (rect2->getX1() <= rect1->getX2())) {
	    if(rect2->getX2() < rect1->getX2()) {
	        newrect= new XiliRectInt(rect2->getX1(), y1, rect2->getX2(), y2);
	    } else {
	        newrect= new XiliRectInt(rect2->getX1(), y1, rect1->getX2(), y2);
	    }
	    if(newrect==NULL) {
	        XIL_ERROR(myRoi->getSystemState(), XIL_ERROR_RESOURCE,"di-1",TRUE);
		while(head) {
		    newrect= (XiliRectInt*)head->getNext();
		    delete head;
		    numrects--;
		    head= newrect;
		}
		return((XiliRectInt*)-1);
	    }
	    numrects++;
	    if(head==NULL) {
	        rectlistptr= head = newrect;
	    } else {
	        rectlistptr->setNext(newrect);
		rectlistptr= newrect;
	    }
	}
	
	// advance the one with the smallest X 
	if(rect1->getX2() < rect2->getX2()) {
	    rect1= (XiliRectInt*)rect1->getNext();
	    if(rect1==NULL) {
	        break;
	    }
	} else {
	    rect2= (XiliRectInt*)rect2->getNext();
	    if (rect2==NULL) {
	        break;
	    }
	}
    }
    
    *out_numrects = numrects;

    return head;
}

XiliRoiRect::XiliRoiRect(XilRoi* calling_roi) :
    singleRect(0, 0, 0, 0),
    bbox(0, 0, 0, 0)
{
    first     = NULL;
    num_rects = 0;
    myRoi     = calling_roi;
    rectList  = NULL;
    valid     = FALSE;
}


XiliRoiRect::~XiliRoiRect()
{
    XiliRectInt* rect_list;
    int count = 0;

    if(num_rects > 1) {
        while(first) {
            count++;
            rect_list = (XiliRectInt*)first->getNext();
	    delete first;
	    first = rect_list;
        }
#ifdef DEBUG
        if(count!=num_rects) {
            fprintf(stderr,"count conflict in ROI destructor:\n");
            fprintf(stderr,"deleted %d rects, %d rects in ROI\n",count,num_rects);
        }
#endif // DEBUG
    }
}

XilStatus
XiliRoiRect::clear()
{

    XiliRectInt* rect_list;

    if(num_rects > 1) {
        while(first) {
            rect_list = (XiliRectInt*)first->getNext();
	    delete first;
	    first = rect_list;
        }
    }

    singleRect.set(0, 0, 0, 0);
    bbox.set(0, 0, 0, 0);

    first     = NULL;
    num_rects = 0;
    rectList  = NULL;
    valid     = FALSE;

    return XIL_SUCCESS;
}

const XiliRoiRect&
XiliRoiRect::operator =(XiliRoiRect& from)
{
    //
    // Clean up anything existing in this roiRect
    //
    if(num_rects > 1) {
        XiliRectInt* tmprect;
        while(first != NULL) {
            tmprect = (XiliRectInt*)first->getNext();
            delete first;
            first = tmprect;
        }
    }

    num_rects  = from.num_rects;
    myRoi      = from.myRoi;
    valid      = from.valid;
    rectList   = NULL;
    singleRect = from.singleRect;

    //
    //  Set the bounding boxes equal to each other
    //
    bbox       = from.bbox;

    if(num_rects > 1) {
        //
        // Copy first rect
        //
        if(from.first != NULL) {
            first = new XiliRectInt(from.first);
            if(first == NULL) {
                XIL_ERROR(myRoi->getSystemState(),XIL_ERROR_RESOURCE,"di-1",TRUE);
                return *this;
            }
            first->setNext(NULL);

            //
            // Copy rest of rect's
            //
            XiliRectInt* iptr = (XiliRectInt*)from.first->getNext();
            XiliRectInt* optr = first;
            while (iptr) {
                optr->setNext(new XiliRectInt(iptr));
                if (!optr->getNext()) {
                    XIL_ERROR(myRoi->getSystemState(),XIL_ERROR_RESOURCE,"di-1",TRUE);
                    return *this;
                }
                optr = (XiliRectInt*)optr->getNext();
                optr->setNext(NULL);
                iptr = (XiliRectInt*)iptr->getNext();
            }
        }
    }

    return *this;
}

void
XiliRoiRect::setCallingRoi(XilRoi* top_roi)
{
    myRoi = top_roi;
}

void
XiliRoiRect::setValid(Xil_boolean valid_flag)
{
    valid = valid_flag;
}

Xil_boolean
XiliRoiRect::isValid() const
{
    return valid;
}


XilStatus
XiliRoiRect::addRect(int x, int y, unsigned int width, unsigned int height)
{
    //
    //  ROI cannot have either dimension <= 0
    //
    if((width <= 0) || (height <= 0)) {
	XIL_ERROR(myRoi->getSystemState(), XIL_ERROR_USER,"di-347",TRUE);
	return XIL_FAILURE;
    }

    //
    //  Calculate extent of region
    //
    int x1 = x;
    int y1 = y;
    int x2 = x + width - 1;
    int y2 = y + height - 1;

    if(num_rects == 0) { //optimal support
        num_rects = 1;
        singleRect.set(x1, y1, x2, y2);
	bbox.set(x1, y1, x2, y2);
        return XIL_SUCCESS;
    }
    if(num_rects == 1) { // move to full support
        int sx1,sx2,sy1,sy2;

        singleRect.get(&sx1,&sy1,&sx2,&sy2);
        first = new XiliRectInt(sx1, sy1, sx2, sy2);
        singleRect.set(0,0,0,0);
    }           
    
    XiliRectInt *current_rect,*temp_rect,*new_rects,*new_rect,*prev_rect;

    /* introduce the edges of the new rect as new Y boundaries */
    current_rect=first;
    while (current_rect) {
        if ((y1 > current_rect->getY1()) && (y1 <= current_rect->getY2())) {
            // the upper end of the new rect is within this rect
            temp_rect= new XiliRectInt(current_rect);
            if (!temp_rect) {
                XIL_ERROR(myRoi->getSystemState(), XIL_ERROR_RESOURCE,"di-1",TRUE);
                return XIL_FAILURE;
            }
            num_rects++;
            current_rect->setY2(y1-1);
            temp_rect->setY1(y1);
            // insert the newly created rect correctly according to y
            prev_rect = current_rect;
            while (prev_rect->getNext()) {
            if (temp_rect->getY1() >= ((XiliRectInt*)prev_rect->getNext())->getY1())
                prev_rect = (XiliRectInt*)prev_rect->getNext();
            else
                break;
            }
            temp_rect->setNext(prev_rect->getNext());
            prev_rect->setNext(temp_rect);
        } else if ((y2 >= current_rect->getY1()) && (y2 < current_rect->getY2())) {
            // the lower end of the new rect is within this rect */
            temp_rect= new XiliRectInt(current_rect);
            if (!temp_rect) {
	        XIL_ERROR(myRoi->getSystemState(), XIL_ERROR_RESOURCE,"di-1",TRUE);
	        return XIL_FAILURE;
            }
	    num_rects++;
            current_rect->setY2(y2);
            temp_rect->setY1(y2+1);
            // insert the newly created rect correctly according to y
            prev_rect = current_rect;
            while (prev_rect->getNext()) {
            if(temp_rect->getY1() >= ((XiliRectInt*)prev_rect->getNext())->getY1())
                prev_rect = (XiliRectInt*)prev_rect->getNext();
            else
                break;
            }
            temp_rect->setNext(prev_rect->getNext());
            prev_rect->setNext(temp_rect);
        }
        current_rect= (XiliRectInt*)current_rect->getNext();
    }

    // divide the new rect using all of the existing Y boundaries 
    current_rect=first;
    new_rects=new_rect= new XiliRectInt(x1, y1, x2, y2);
    if(!new_rects) {
        XIL_ERROR(myRoi->getSystemState(), XIL_ERROR_RESOURCE,"di-1",TRUE);
        return XIL_FAILURE;
    }
    num_rects++;
    while(current_rect) {
        if((new_rect->getY1() < current_rect->getY1()) && (new_rect->getY2() >= current_rect->getY1())) {
            temp_rect=new XiliRectInt(new_rect);
            if(!temp_rect) {
	        XIL_ERROR(myRoi->getSystemState(), XIL_ERROR_RESOURCE,"di-1",TRUE);
	        while(new_rects) {
                    temp_rect= (XiliRectInt*)new_rects->getNext();
		    delete new_rects;
		    num_rects--;
		    new_rects= temp_rect;
	        }
	        return XIL_FAILURE;
            }
            num_rects++;
            new_rect->setNext(temp_rect);
            new_rect->setY2(current_rect->getY1()-1);
            temp_rect->setY1(current_rect->getY1());
            new_rect= temp_rect;
        }
        if((new_rect->getY1() <= current_rect->getY2()) && (new_rect->getY2() > current_rect->getY2())) {
            temp_rect=new XiliRectInt(new_rect);
            if(!temp_rect) {
	        XIL_ERROR(myRoi->getSystemState(), XIL_ERROR_RESOURCE,"di-1",TRUE);
	        while(new_rects) {
                    temp_rect= (XiliRectInt*)new_rects->getNext();
		    delete new_rects;
		    num_rects--;
		    new_rects= temp_rect;
	        }
	        return XIL_FAILURE;
            }
            num_rects++;
            new_rect->setNext(temp_rect);
            new_rect->setY2(current_rect->getY2());
            temp_rect->setY1(current_rect->getY2()+1);
            new_rect= temp_rect;
        }
        current_rect= (XiliRectInt*)current_rect->getNext();
    }

    //insert the new rects   
    new_rect= new_rects;

    if(first) {
        if(new_rect->getY1() < first->getY1()) {
            temp_rect= (XiliRectInt*)new_rect->getNext();
            new_rect->setNext(first);
            first=new_rect;
            new_rect=temp_rect;
        } else if ((new_rect->getY1()==first->getY1()) && (new_rect->getX1() < first->getX1())) {
            temp_rect= (XiliRectInt*)new_rect->getNext();
            new_rect->setNext(first);
            first=new_rect;
            new_rect=temp_rect;
        }
    } else {
        first=new_rect;
        new_rect=NULL;
    }
    current_rect= first;
    while(current_rect->getNext() && new_rect) {
        if(new_rect->getY1() < ((XiliRectInt*)current_rect->getNext())->getY1()) {
            temp_rect= (XiliRectInt*)new_rect->getNext();
            new_rect->setNext(current_rect->getNext());
            current_rect->setNext(new_rect);
            new_rect=temp_rect;
        } else if ((new_rect->getY1()==((XiliRectInt*)current_rect->getNext())->getY1()) && (new_rect->getX1() < ((XiliRectInt*)current_rect->getNext())->getX1())) {
            temp_rect= (XiliRectInt*)new_rect->getNext();
            new_rect->setNext(current_rect->getNext());
            current_rect->setNext(new_rect);
            new_rect=temp_rect;
        }
        current_rect= (XiliRectInt*)current_rect->getNext();
    }
    // the rest go on the end
    if(new_rect) current_rect->setNext(new_rect);

    // now remove redundant rectangles and clip overlapping ones */
    current_rect=first;
    while(current_rect->getNext()) {
        if(current_rect->getY1()==((XiliRectInt*)current_rect->getNext())->getY1()) {
            if(current_rect->getX2() >= ((XiliRectInt*)current_rect->getNext())->getX2()) {
                // the next one is redundant */
                temp_rect= (XiliRectInt*)current_rect->getNext();
                current_rect->setNext(current_rect->getNext()->getNext());
                delete temp_rect;
	        num_rects--;
            } else if(current_rect->getX2() >= (((XiliRectInt*)current_rect->getNext())->getX1()-1)) {
                // merge the two rectes
                current_rect->setX2(((XiliRectInt*)current_rect->getNext())->getX2());
                temp_rect= (XiliRectInt*)current_rect->getNext();
                current_rect->setNext(current_rect->getNext()->getNext());
                delete temp_rect;
	        num_rects--;
            } else {
                current_rect=(XiliRectInt*)current_rect->getNext();
            }
        } else {
            current_rect=(XiliRectInt*)current_rect->getNext();
        }
    }

    // quick check to see if we're down to optimal case
    if(num_rects == 1) {
        // optimize
        singleRect.set(first);
	bbox.set(first);
        delete first;
        first = NULL;
    } else {
	//TODO:maynard: It may be better to catch the addition as it gets added, as opposed
	// to post-processing for bounding box.
	updateBoundingBox();
    }
    
    return XIL_SUCCESS;
}


XiliRectInt*
XiliRoiRect::getRectList()
{
    // Returns reference to full rectlist
    if(num_rects == 1) {
        // only singleRect is valid
        return &singleRect;
    } 
    return first;
}

unsigned int
XiliRoiRect::numRects()
{
    return num_rects;
}

XiliRect*
XiliRoiRect::getBbox()
{
    return &bbox;
}


//
// Given a bitmask (image) representation of a Roi, turn it
// into a valid rectlist implementation of the same Roi.
// For obvious reasons this shares a fair bit of code with addImage()
//
XilStatus
XiliRoiRect::translateBitmask(XiliRoiBitmask* bmroi)
{
    XilImage*    bitmask_image;
    unsigned int bm_width;
    unsigned int bm_height;

    if(bmroi == NULL) {
        return XIL_FAILURE;
    }
    if((bitmask_image = bmroi->getBitmask()) == NULL) {
        return XIL_FAILURE;
    }
    bitmask_image->getSize(&bm_width, &bm_height);
    if((bm_height == 0) || (bm_width == 0)) {
        this->clear();
        return XIL_SUCCESS;
    }

    XilStorage storage(bitmask_image);
    XilBox bitmaskbox(0, 0, bm_width, bm_height); 
    if(bitmask_image->getStorage(&storage, &bitmaskbox,
                                 "XilMemory", XIL_READ_ONLY) == XIL_FAILURE) {
        XIL_ERROR(myRoi->getSystemState(),XIL_ERROR_SYSTEM,"di-140",FALSE);
        return XIL_FAILURE;
    }
    
    unsigned int next_scanline, base_offset;
    Xil_unsigned8* base_addr;
    //
    // We can assume that the image is a 1-band, BIT image and therefore, 
    // next_pixel and next_band are unused...
    //
    if(storage.getStorageInfo((unsigned int*)NULL,
			      &next_scanline,
			      (unsigned int*)NULL,
			      &base_offset,
			      (void**)&base_addr) == XIL_FAILURE) {
        return XIL_FAILURE;
    }
    

    //
    // Clear rectlist for easier implementation
    // 
    this->clear();
    
    //
    // scan over image, adding ROI scanlines for each run of pixels > 0
    //
    Xil_unsigned8* scanline = base_addr;
    unsigned int  offset = base_offset;
    float foriginx,foriginy;
    int originx, originy;

    first = new XiliRectInt(0, 0, 1, 1);
    if(!first) {
        XIL_ERROR(myRoi->getSystemState(),XIL_ERROR_RESOURCE,"di-1",TRUE);
        return XIL_FAILURE;
    }
    num_rects++;
    XiliRectInt* current_rect = first;
    
    //
    //  Scan whole bitmask, using origin to offset rectlist from 0,0
    //
    bitmask_image->getOrigin(&foriginx,&foriginy);
    originx = _XILI_ROUND(foriginx);
    originy = _XILI_ROUND(foriginy);
    for(unsigned int y = 0; y < (unsigned int)bm_height; y++) {
        for(unsigned int x = 0; x < (unsigned int)bm_width; x++) {
            
            //
            //  look for runlengths of set pixels
            //
            if(XIL_BMAP_TST(scanline, offset + x)) {
                //
                //  find out how long run is 
                //
                unsigned int runlength = 1;
                while((XIL_BMAP_TST(scanline, offset + x + runlength)) &&
                      (x+runlength < bm_width))
                    runlength++;
                
                //
                // add this run-length to ROI
                //
                XiliRectInt *temp_rect= new XiliRectInt(current_rect);
                if(!temp_rect) {
                    XIL_ERROR(myRoi->getSystemState(),XIL_ERROR_RESOURCE,"di-1",TRUE);
                    return XIL_FAILURE;
                }
                num_rects++;
                temp_rect->setX1(x + originx);
                temp_rect->setY1(y + originy);
                temp_rect->setX2(x + runlength - 1 + originx);
                temp_rect->setY2(y + originy);
                current_rect->setNext(temp_rect);
                current_rect = temp_rect;
                
                //
                // advance x ptr past this runlength 
                //
                x += runlength;
            }
        }
        scanline += next_scanline;
    }
    
    //
    // get rid of dummy first rect - this only works because we know the
    // original rect won't have been "absorbed" away because we didn't use addRect
    //
    current_rect = first;
    first = (XiliRectInt*)current_rect->getNext();
    delete current_rect;
    num_rects--;
    consolidateRects();
    updateBoundingBox();
    valid = TRUE;
    return XIL_SUCCESS;
}

XilStatus
XiliRoiRect::translateConvexRegion(XiliRoiConvexRegion* crroi)
{
    //
    // Clear rectlist for easier implementation
    // 
    this->clear();

    first = new XiliRectInt(0, 0, 1, 1);
    if(!first) {
        XIL_ERROR(myRoi->getSystemState(),XIL_ERROR_RESOURCE,"di-1",TRUE);
        return XIL_FAILURE;
    }
    num_rects++;
    XiliRectInt* current_rect = first;

    //
    // Test for only one convex region being present.
    //
    if(crroi->getNumRegions() == 1) {
	//
	// Construct a scanline list
	//
	XiliConvexRegion*  cr = crroi->getRegion();

	if(cr->isRect() != TRUE) {
	    XiliScanlineListInt sl_list(cr->xPtArray,
                                        cr->yPtArray,
                                        cr->pointCount);
            unsigned int     num_lines; 
            XiliScanlineInt* scanlines = sl_list.getScanlines(&num_lines);

            for(unsigned int i=0; i<num_lines; i++) {
		//
		// add scanlines to the Roi
		//
		XiliRectInt *temp_rect= new XiliRectInt(current_rect);
		if(!temp_rect) {
		    XIL_ERROR(myRoi->getSystemState(),XIL_ERROR_RESOURCE,"di-1",TRUE);
		    return XIL_FAILURE;
		}
		num_rects++;

                temp_rect->set(scanlines[i].x1,
                               scanlines[i].y,
                               scanlines[i].x2,
                               scanlines[i].y);

		current_rect->setNext(temp_rect);
		current_rect = temp_rect;
	    }
	} else {
	    //
	    // Get the bounding box and add in a new
	    // rect.
	    //
	    XiliRectInt *temp_rect= new XiliRectInt(current_rect);
	    if(!temp_rect) {
		XIL_ERROR(myRoi->getSystemState(),XIL_ERROR_RESOURCE,"di-1",TRUE);
		return XIL_FAILURE;
	    }
	    num_rects++;

            temp_rect->set(cr->lowX,
                           cr->lowY,
                           cr->highX,
                           cr->highY);

	    current_rect->setNext(temp_rect);
	    current_rect = temp_rect;
	}
    } else {
	//
	// For each convex region in the list convert it to
	// a scanline object and then convert ito a rect list.
	//
	XiliList<XiliConvexRegion>*        region_list = crroi->getRegionList();
	XiliListIterator<XiliConvexRegion> iterator(region_list);
	XiliConvexRegion*                  cr;

	while((cr = iterator.getNext()) != _XILI_LIST_INVALID_POSITION) {
	    if(cr->isRect() != TRUE) {
		//
		// Construct a scanline list
		//
                XiliScanlineListInt sl_list(cr->xPtArray,
                                            cr->yPtArray,
                                            cr->pointCount);
                unsigned int     num_lines; 
                XiliScanlineInt* scanlines = sl_list.getScanlines(&num_lines);

                for(unsigned int i=0; i<num_lines; i++) {
		    //
		    // add scanlines to the Roi
		    //
		    XiliRectInt *temp_rect= new XiliRectInt(current_rect);
		    if(!temp_rect) {
			XIL_ERROR(myRoi->getSystemState(),
                                  XIL_ERROR_RESOURCE, "di-1", TRUE);
			return XIL_FAILURE;
		    }
		    num_rects++;

                    temp_rect->set(scanlines[i].x1,
                                   scanlines[i].y,
                                   scanlines[i].x2,
                                   scanlines[i].y);

		    current_rect->setNext(temp_rect);
		    current_rect = temp_rect;
		}
	    } else {
		//
		// Get the bounding box and add in a new
		// rect.
		//
		XiliRectInt *temp_rect= new XiliRectInt(current_rect);
		if(!temp_rect) {
		    XIL_ERROR(myRoi->getSystemState(),
                              XIL_ERROR_RESOURCE, "di-1", TRUE);
		    return XIL_FAILURE;
		}
		num_rects++;

                temp_rect->set(cr->lowX,
                               cr->lowY,
                               cr->highX,
                               cr->highY);

		current_rect->setNext(temp_rect);
		current_rect = temp_rect;
	    }
	}
    }
    
    //
    // get rid of dummy first rect - this only works because we know the
    // original rect won't have been "absorbed" away because we didn't use addRect
    //
    current_rect = first;
    first = (XiliRectInt*)current_rect->getNext();
    delete current_rect;
    num_rects--;
    if(num_rects == 1) {
        // consolidated down to 1 rectangle, optimize
        singleRect.set(first);
        delete first;
        first = NULL;
    }
    consolidateRects();
    updateBoundingBox();
    valid = TRUE;
    return XIL_SUCCESS;
}

XilStatus
XiliRoiRect::intBoundingBox(int*          x,
                            int*          y,
                            unsigned int* xsize,
                            unsigned int* ysize)
{
    int x2;
    int y2;

    if(num_rects != 0) {
        bbox.get(x, y, &x2, &y2);
        *xsize = x2-*x+1;
        *ysize = y2-*y+1;
        return XIL_SUCCESS;
    } else {
        *x     = 0;
        *y     = 0;
        *xsize = 0;
        *ysize = 0;

        return XIL_FAILURE;
    }
}

void
XiliRoiRect::updateBoundingBox()
{
    int	extent_x1;
    int	extent_x2;
    int	extent_y1;
    int	extent_y2;

    if(num_rects == 1) {
        singleRect.get(&extent_x1, &extent_y1, &extent_x2, &extent_y2);
        bbox.set(extent_x1, extent_y1, extent_x2, extent_y2);
        return;
    }

    XiliRectInt *next_rect=first;
    if (next_rect) {
        /* check each rectangle */
        extent_x1 = next_rect->getX1();
	extent_x2 = next_rect->getX2();
	extent_y1 = next_rect->getY1();
	extent_y2 = next_rect->getY2();
	while (next_rect) {
	    if (extent_x1 > next_rect->getX1())
	        extent_x1 = next_rect->getX1();
	    if (extent_x2 < next_rect->getX2())
	        extent_x2 = next_rect->getX2();
	    if (extent_y1 > next_rect->getY1())
	        extent_y1 = next_rect->getY1();
	    if (extent_y2 < next_rect->getY2())
	        extent_y2 = next_rect->getY2();
	    next_rect = (XiliRectInt*)next_rect->getNext();
       }
       bbox.set(extent_x1, extent_y1, extent_x2, extent_y2);	
       return;
    } else {
       bbox.set(0, 0, 0, 0);
    }
    return;
}

XilStatus
XiliRoiRect::addImage(XilImage* image)
{
    //
    //  Take an image from the user and turn all the runs of "on" pixels
    //  into rectangles of the ROI.
    //  The origins of the image, should be added to the x,y location
    //  of each rectangle as the origin of an image indicates it's
    //  placement in the ROI.
    //
    if (!image) {
        XIL_ERROR(myRoi->getSystemState(), XIL_ERROR_USER,"di-207",TRUE);
	return XIL_FAILURE;
    }

    //
    // Get the storage - cobbled together for now
    // this also forces the sync of the image
    //
    unsigned int width  = image->getWidth();
    unsigned int height = image->getHeight();
    
    //
    //  Pick up the image origins to place the bits within
    //  the existing ROI.
    //
    float float_ox;
    float float_oy;
    image->getOrigin(&float_ox,&float_oy);
    int originx = _XILI_ROUND(float_ox);
    int originy = _XILI_ROUND(float_oy);

    XilStorage storage(image);
    XilBox     imagebox(0, 0, width, height); 
    if(image->getStorage(&storage, &imagebox,
                         "XilMemory", XIL_READ_ONLY) == XIL_FAILURE) {
        XIL_ERROR(myRoi->getSystemState(),XIL_ERROR_SYSTEM,"di-140",FALSE);
        return XIL_FAILURE;
    }

    unsigned int   next_scanline;
    unsigned int   base_offset;
    Xil_unsigned8* base_addr;
    //
    //  We can assume that the image is a 1-band, BIT image and therefore, 
    //  next_pixel and next_band are unused...
    //
    if(storage.getStorageInfo((unsigned int*)NULL,
			      &next_scanline,
			      (unsigned int*)NULL,
			      &base_offset,
			      (void**)&base_addr) == XIL_FAILURE) {
      return XIL_FAILURE;
    }
      
   // scan over image, adding ROI scanlines for each run of pixels > 0 */
   Xil_unsigned8* scanline = base_addr;
   unsigned int  offset = base_offset;

   // if ROI is intially empty... */
   if(first==NULL) {
       first = new XiliRectInt(0, 0, 1, 1);
       if (!first) {
           XIL_ERROR(myRoi->getSystemState(),XIL_ERROR_RESOURCE,"di-1",TRUE);
           return XIL_FAILURE;
       }
       num_rects++;
       XiliRectInt* current_rect = first;
    
       /* scan whole image, ignoring ROIs */ 
      for (unsigned int y = 0; y < (unsigned int)height; y++) {
           for (unsigned int x = 0; x < (unsigned int)width; x++) {
    
	       /* look for runlengths of set pixels */
	       if (XIL_BMAP_TST(scanline, offset + x)) {
    
	           /* find out how long run is */
	           unsigned int runlength = 1;
	           while((XIL_BMAP_TST(scanline, offset + x + runlength)) &&
		         (x+runlength < width))
	               runlength++;
    
	           /* add this run-length to ROI */
   	           XiliRectInt *temp_rect= new XiliRectInt(current_rect);
   	           if (!temp_rect) {
		       XIL_ERROR(myRoi->getSystemState(),XIL_ERROR_RESOURCE,"di-1",TRUE);
		       return XIL_FAILURE;
   	           }
       		   num_rects++;
                   //
                   //  Subtract out the originx and originy to place the
                   //  image ROI properly in the ROI.
                   //
                   temp_rect->setX1(x - originx);
                   temp_rect->setY1(y - originy);
                   temp_rect->setX2(x + runlength - 1 - originx);
                   temp_rect->setY2(y - originy);
                   current_rect->setNext(temp_rect);
	           current_rect = temp_rect;
    
	           /* advance x ptr past this runlength */
	           x += runlength;
	       }
           }
           scanline += next_scanline;
       }
    
       // get rid of dummy first rect - this only works because we know the
       // original rect won't have been "absorbed" away because we didn't use addRect
       current_rect = first;
       first = (XiliRectInt*)current_rect->getNext();
       delete current_rect;
       num_rects--;
       if(num_rects == 1) {
           // consolidated down to 1 rectangle, optimize
           singleRect.set(first);
           delete first;
           first = NULL;
       }
       consolidateRects();
       updateBoundingBox();
   } else { /* adding to existing ROI, have to do addRects... */
    
       /* scan whole image, ignoring ROIs */
       for (unsigned int y = 0; y < (unsigned int)height; y++) {
           for (unsigned int x = 0; x < (unsigned int)width; x++) {
    
	       /* look for runlengths of set pixels */
	       if (XIL_BMAP_TST(scanline, offset + x)) {
    
	           /* find out how long run is */
	           int runlength = 1;
	           while((XIL_BMAP_TST(scanline, offset + x + runlength)) &&
		         (x+runlength < width))
	               runlength++;
    
                   //
	           //  add this run-length to ROI 
		   //  Note, addRect subtracts one from width, we don't need to
                   //  Remeber to subtract out the image origin to correctly place
                   //  the image ROI within the ROI.
                   //
		   if (addRect((x-originx), (y-originy), x + runlength, 1)==XIL_FAILURE) {
                      return XIL_FAILURE;
                   }
    
	           /* advance x ptr past this runlength */
	           x += runlength;
	       }
           }
           scanline += next_scanline;
       }
       consolidateRects();
   }
   return XIL_SUCCESS;
}

XilImage*
XiliRoiRect::getAsImage()
{
    XiliRectInt* current_rect;
    
    if(num_rects == 1) {
        current_rect = &singleRect;
    } else {
        current_rect = first;
    }
    if(current_rect)  { //if there is a region...

        //figure out how big to make the image
        int x1;
        int y1;
        int x2;
        int y2;
	bbox.get(&x1, &y1, &x2, &y2);
    
	// create the image
	XilImage* image = (myRoi->getSystemState())->createXilImage((unsigned int)(x2 - x1 + 1),
								    (unsigned int)(y2 - y1 + 1),
								    1,
								    XIL_BIT);
	if (!image) {
	    XIL_ERROR(myRoi->getSystemState(), XIL_ERROR_SYSTEM, "di-147", FALSE);
	    return(NULL);
	}
    
	// get the storage - cobbled together for now
	// this also forces the sync of the image
	unsigned int width  = image->getWidth();
	unsigned int height = image->getHeight();
	XilStorage storage(image);
	XilBox     imagebox(0, 0, width, height); 
	if(image->getStorage(&storage, &imagebox,
                             "XilMemory", XIL_WRITE_ONLY) == XIL_FAILURE) {
	    XIL_ERROR(myRoi->getSystemState(), XIL_ERROR_SYSTEM, "di-140", FALSE);
	    return NULL;
	}

	unsigned int next_scanline, base_offset;
	Xil_unsigned8* base_addr;
	// We can assume that the image is a 1-band, BIT image and therefore, 
	// next_pixel and next_band are unused...
	if(storage.getStorageInfo((unsigned int*)NULL,
				  &next_scanline,
				  (unsigned int*)NULL,
				  &base_offset,
				  (void**)&base_addr) == XIL_FAILURE) {
	    return NULL;
	}
      
	Xil_unsigned8* scanline = base_addr;
	unsigned int  offset = base_offset;

	// clear image
	for(unsigned int k = 0; k < height; k++) {
	    for(unsigned int m = 0; m < width; m++) {
	        XIL_BMAP_CLR(scanline, offset + m);
	    }
	    scanline += next_scanline;
	}
	
	// set the appropriate pixels in the image
	while(current_rect != NULL) {
	    unsigned int xstart = (unsigned int)(current_rect->getX1() - x1);
	    unsigned int ystart = (unsigned int)(current_rect->getY1() - y1);

	    width =  (unsigned int)(current_rect->getX2() - current_rect->getX1() + 1);
	    height = (unsigned int)(current_rect->getY2() - current_rect->getY1() + 1);
	    scanline = base_addr + next_scanline * ystart;
	    offset = (unsigned int)base_offset + xstart;
	    
	    for(unsigned int i = 0; i < height; i++) {
	        for (unsigned int j = 0; j < width; j++) {
		    XIL_BMAP_SET(scanline, offset + j);
		}
		scanline += next_scanline;
	    }
	    current_rect = (XiliRectInt*)current_rect->getNext();
	}
	
        //
        // Set the image origin to reflect the values of the starting Roi.
        // Note that the original roi may have had a bbox starting at other than 0,0
        // The image is offset by these amounts, and the origin must reflect this.
        //
        image->setOrigin((float)(-x1), (float)(-y1));
	return image;
    } else
        return NULL;
}

void
XiliRoiRect::consolidateRects()
{
    XiliRectInt* current_rect, *temp_rect;

    if((num_rects <2) || (first==NULL)) {
      return;
    }
    //Look for rectangles sitting right on top of each other
    current_rect=first;
    while(current_rect->getNext()) {
        if((current_rect->getX1() == ((XiliRectInt*)current_rect->getNext())->getX1()) &&
	   (current_rect->getX2() == ((XiliRectInt*)current_rect->getNext())->getX2())) {
	    // Check that the rectangles are adjacent
	    if(current_rect->getY2() == (((XiliRectInt*)current_rect->getNext())->getY1() - 1)) {
	        // merge the rectangles
	        current_rect->setY2(((XiliRectInt*)current_rect->getNext())->getY2());
		temp_rect = (XiliRectInt*)current_rect->getNext();
		current_rect->setNext(current_rect->getNext()->getNext());
		delete temp_rect;
		num_rects--;
	    } else {
	        current_rect = (XiliRectInt*)current_rect->getNext();
	    }
	} else {
	    current_rect = (XiliRectInt*)current_rect->getNext();
	}
    }
    if(num_rects == 1) {
        // consolidated down to 1 rectangle, optimize
        singleRect.set(first);
        delete first;
        first = NULL;
    }
}

XilStatus
XiliRoiRect::subtractRect(int x,
			  int y,
			  unsigned int width,
			  unsigned int height) 
{
    XiliRectInt *current_rect,*temp_rect,*new_rects,*new_rect;

    int x1 = x;
    int y1 = y;
    int x2 = x + width - 1;
    int y2 = y + height - 1;

    if(num_rects == 1) { // move to full support
        int sx1,sx2,sy1,sy2;

        singleRect.get(&sx1,&sy1,&sx2,&sy2);
        if( (x1==sx1) && (x2==sx2) && (y1==sy1) && (y2==sy2) ) {
            singleRect.set(0,0,0,0);
            bbox.set(0,0,0,0);
            num_rects = 0;
            return XIL_SUCCESS;
        }
        first = new XiliRectInt(sx1, sy1, sx2, sy2);
        singleRect.set(0,0,0,0);
    }           

    // introduce the edges of the new box as new Y boundaries 
    current_rect=first;
    // skip over the uninteresting boxes
    while(current_rect && (current_rect->getY2() < y1)) {
        current_rect= (XiliRectInt*)current_rect->getNext();
    }
    if (current_rect && (current_rect->getY1() < y1)) {
        // find the end of the row
        XiliRectInt* insert_pt= current_rect;
	int ytmp=current_rect->getY1();
	while(insert_pt->getNext() && (((XiliRectInt*)insert_pt->getNext())->getY1()==ytmp)) {
         insert_pt= (XiliRectInt*)insert_pt->getNext();
        }
	// now go ahead and split the rectangles, I don't need to worry
        // about running off the end of the list since I'm adding some on with
        // a different Y myself
	do {
	    temp_rect= new XiliRectInt(current_rect);
	    if (!temp_rect) {
                XIL_ERROR(myRoi->getSystemState(), XIL_ERROR_RESOURCE,"di-1",TRUE);
		return XIL_FAILURE;
	    }
	    num_rects++;
	    // divide 
	    current_rect->setY2(y1-1);
	    temp_rect->setY1(y1);
	    // insert
	    temp_rect->setNext(insert_pt->getNext());
	    insert_pt->setNext(temp_rect);
	    // 
	    current_rect= (XiliRectInt*)current_rect->getNext();
	    insert_pt= (XiliRectInt*)insert_pt->getNext();
	} while (current_rect->getY1() == ytmp);
    }
    while(current_rect && (current_rect->getY2() <=y2)) {
        current_rect= (XiliRectInt*)current_rect->getNext();
    }
    if(current_rect  && (current_rect->getY1() <= y2)) {
        // find the end of the row 
        XiliRectInt* insert_pt= current_rect;
        int tmpy= current_rect->getY1();
	while(insert_pt->getNext() && (((XiliRectInt*)insert_pt->getNext())->getY1()==tmpy)) {
	    insert_pt= (XiliRectInt*)insert_pt->getNext();
	}  

	// now go ahead and split the rectangles, I don't need to worry
        // about running off the end of the list since I'm adding some on with
        // a different Y myself
	do{
	    temp_rect= new XiliRectInt(current_rect);
	    if(!temp_rect) {
	        XIL_ERROR(myRoi->getSystemState(), XIL_ERROR_RESOURCE,"di-1",TRUE);
		return XIL_FAILURE; 
	    }
	    num_rects++;
	    // divide
	    current_rect->setY2(y2);   
	    temp_rect->setY1(y2+1);
	    // insert
	    temp_rect->setNext(insert_pt->getNext());
	    insert_pt->setNext(temp_rect);
	    // advance
	    current_rect= (XiliRectInt*)current_rect->getNext(); 
	    insert_pt= (XiliRectInt*)insert_pt->getNext(); 
	} while(current_rect->getY1() == tmpy);
    }   
    // TODO:maynard: make addRect use the above algorithm

    // divide the new box using all of the existing Y boundaries
    current_rect=first;
    new_rects=new_rect= new XiliRectInt(x1, y1, x2, y2);
    if(!new_rects) {
        XIL_ERROR(myRoi->getSystemState(), XIL_ERROR_RESOURCE,"di-1",TRUE);
	return XIL_FAILURE;
    }
    num_rects++;
    while(current_rect) {
        if((new_rect->getY1() < current_rect->getY1()) && (new_rect->getY2() >= current_rect->getY1())) {
	    temp_rect=new XiliRectInt(new_rect);
	    if(!temp_rect) {
	        XIL_ERROR(myRoi->getSystemState(), XIL_ERROR_RESOURCE,"di-1",TRUE);
		return XIL_FAILURE;
	    }
	    num_rects++;
	    new_rect->setNext(temp_rect);
	    new_rect->setY2(current_rect->getY1()-1);
	    temp_rect->setY1(current_rect->getY1());
	    new_rect= temp_rect;
	}
	if((new_rect->getY1() <= current_rect->getY2()) && (new_rect->getY2() > current_rect->getY2())) {
	    temp_rect=new XiliRectInt(new_rect);
	    if(!temp_rect) {
	        XIL_ERROR(myRoi->getSystemState(), XIL_ERROR_RESOURCE,"di-1",TRUE);
		while(new_rects) {
		    temp_rect=(XiliRectInt*)new_rects->getNext();
		    delete new_rects;
		    num_rects--;
		    new_rects= temp_rect;
		}
		return XIL_FAILURE;
	    }
	    num_rects++;
	    new_rect->setNext(temp_rect);
	    new_rect->setY2(current_rect->getY2());
	    temp_rect->setY1( current_rect->getY2()+1);
	    new_rect= temp_rect;
	}
	current_rect= (XiliRectInt*)current_rect->getNext();
    }

    // subtract the new boxes
    new_rect= new_rects;

    if(first) {
        if(new_rect->getY1() < first->getY1()) {
	    // there are no rect in this area, just throw it out
            temp_rect= (XiliRectInt*)new_rect->getNext();
	    delete new_rect;
	    num_rects--;
	    new_rect=temp_rect;
	} else if((new_rect->getY1()==first->getY1()) && (new_rect->getX1() < first->getX1())) {
	    if(new_rect->getX2() < first->getX1()) {
	        temp_rect= (XiliRectInt*)new_rect->getNext();
		delete new_rect;
		num_rects--;
		new_rect=temp_rect;
	    }
	}
    } else { 
        // throw them all out, the ROI is already empty
        while(new_rect) {
	    temp_rect= (XiliRectInt*)new_rect->getNext();
	    delete new_rect;
	    num_rects--;
	    new_rect= temp_rect;
	}
    }

    // insert a dummy box to simplify the code
    temp_rect= new XiliRectInt(0,0,0,0);
    if (temp_rect==NULL) {
        XIL_ERROR(myRoi->getSystemState(), XIL_ERROR_RESOURCE,"di-1",TRUE);
	while(new_rects) {
	    temp_rect=(XiliRectInt*)new_rects->getNext();
	    delete new_rects;
	    num_rects--;
	    new_rects = temp_rect;
	}
	return XIL_FAILURE;
    }
    num_rects++;
    temp_rect->setNext(first);
    first= temp_rect;
   
    current_rect= first;
    while(current_rect->getNext() && new_rect) {
        if(new_rect->getY1() < ((XiliRectInt*)current_rect->getNext())->getY1()) {
//
// -------
// -     -
// -     -
// -------
//     ++++++++
//     +      +
//     +      +
//     ++++++++
//
	    // there are no rect in this area, just throw it out
	    temp_rect= (XiliRectInt*)new_rect->getNext();
	    delete new_rect;
	    num_rects--;
	    new_rect=temp_rect;

	    // current_rect is still ahead of new_rect so don't advance
	      
	  } else if((new_rect->getY1()==((XiliRectInt*)current_rect->getNext())->getY1())) {
	      if(new_rect->getX1() <= ((XiliRectInt*)current_rect->getNext())->getX1()) {
		  if(new_rect->getX2() < ((XiliRectInt*)current_rect->getNext())->getX1()) {
//
// --------   +++++++++
// -      -   +       +
// -      -   +       +
// --------   +++++++++
//
		      // there are no rect in this area, just throw it out
		      temp_rect= (XiliRectInt*)new_rect->getNext();
		      delete new_rect;
		      num_rects--;
		      new_rect=temp_rect;
		      
		      // the next subtraction box MUST have a different Y coordinate so go
		      //ahead and advance current_rect 
		      current_rect->setNext(current_rect->getNext());
		  } else if(new_rect->getX2() >= ((XiliRectInt*)current_rect->getNext())->getX2()) {
//
// -----++++++---------
// -    +    +        -
// -    +    +        -
// -----++++++---------
//
		      // the rect is being completely subtracted, throw it out
		      temp_rect= (XiliRectInt*)current_rect->getNext()->getNext();
		      current_rect->getNext()->destroy();
		      num_rects--;
		      current_rect->setNext(temp_rect);

		      // there may be more rects that clip against this subtraction
		      //rect so don't advance new_rect 
		  } else {
/*
--------+-+-++++++++
-       +  -       +
-       +  -       +
--------+-+-++++++++
*/
		      // the rect is partly subtracted, clip it
		      ((XiliRectInt*)current_rect->getNext())->setX1(new_rect->getX2()+1);

		      // toss the subtraction box since we're done with it
		      temp_rect= (XiliRectInt*)new_rect->getNext();
		      delete new_rect;
		      num_rects--;
		      new_rect= temp_rect;

		      // the next subtraction box MUST have a different Y coordinate so go
		      //  ahead and advance current_rect
		      current_rect->setNext(current_rect->getNext());
		  }
	      } else if(new_rect->getX1() <= ((XiliRectInt*)current_rect->getNext())->getX2()) {
		  if(new_rect->getX2() < ((XiliRectInt*)current_rect->getNext())->getX2()) {
//
// +++++++------+++++++
// +      -    -      +
// +      -    -      +
// +++++++------+++++++
//
		      // the rectangle is being split by the subtraction rectangle
		      int newx2=((XiliRectInt*)current_rect->getNext())->getX2();
		      // clip the existing box to be the left side and advance 
		      ((XiliRectInt*)current_rect->getNext())->setX2(new_rect->getX1()-1);
		      current_rect= (XiliRectInt*)current_rect->getNext();
		      // make the subtraction box into the right side
		      new_rect->setX1(new_rect->getX2()+1);
		      new_rect->setX2(newx2);
		      // insert it
		      temp_rect= (XiliRectInt*)new_rect->getNext(); // save this for later
		      new_rect->setNext(current_rect->getNext());
		      current_rect->setNext(new_rect);
		      // advance the subtraction list
		      new_rect= temp_rect;
		  } else {
//
// +++++++++-+-+-------
// +        -  +      -
// +        -  +      -
// +++++++++-+-+-------
//
		      // the rectangle is partly subtracted, clip it
		      ((XiliRectInt*)current_rect->getNext())->setX2(new_rect->getX1()-1);
		      current_rect= (XiliRectInt*)current_rect->getNext();
		  }
	      } else {
//
// +++++++++ ---------
// +       + -       -
// +       + -       -
// +++++++++ ---------
//
		  // the subtraction rect is ahead of the existing rect to just
  		  //advance current_rect
		  current_rect= (XiliRectInt*)current_rect->getNext();
	      }
	  } else {
//
// +++++++++
// +       +
// +       +
// +++++++++
//     ---------
//     -       -
//     -       -
//     ---------
//
	      // the subtraction box is ahead of the existing rec, advance current_rect
	      current_rect= (XiliRectInt*)current_rect->getNext();
	  }
    }
    // nothing to subtract from down this far
    while(new_rect) {
        temp_rect= (XiliRectInt*)new_rect->getNext();
	delete new_rect;
	num_rects--;
	new_rect= temp_rect;
    }
    
    // throw away the dummy that was added earlier
    temp_rect= (XiliRectInt*)first->getNext();
    delete first;
    num_rects--;
    first= temp_rect;
    //quick check to see if we're down to optimal case
    if(num_rects == 1) {
        // optimize
        singleRect.set(first);
        delete first;
        first = NULL;
    }
    return XIL_SUCCESS;
}

XilStatus
XiliRoiRect::getCopyRoiRect(XiliRoiRect* copy)
{

   if(copy == NULL) {
       return XIL_FAILURE;
   }     

   if(num_rects == 1) {
       copy->singleRect = singleRect;
       copy->bbox       = bbox;
       copy->num_rects  = num_rects;
       copy->first      = NULL;
       copy->myRoi      = myRoi;

       return XIL_SUCCESS;
   }

   /* handle the case of an empty roi */
   if(first==NULL) {
      copy->first=NULL;
      return XIL_FAILURE;
   }

   // Copy first rect
   copy->first = new XiliRectInt(first);
   if (copy->first == NULL) {
       XIL_ERROR(myRoi->getSystemState(),XIL_ERROR_RESOURCE,"di-1",TRUE);
       return XIL_FAILURE;
   }
   copy->first->setNext(NULL);

   // Copy rest of rect's
   XiliRectInt* iptr = (XiliRectInt*)first->getNext();
   XiliRectInt* optr = copy->first;
   while (iptr) {
      optr->setNext(new XiliRectInt(iptr));
      if (!optr->getNext()) {
	  XIL_ERROR(myRoi->getSystemState(),XIL_ERROR_RESOURCE,"di-1",TRUE);
	  return XIL_FAILURE;
      }
      optr = (XiliRectInt*)optr->getNext();
      optr->setNext(NULL);
      iptr = (XiliRectInt*)iptr->getNext();
   }
 
   /* give the copy the same version number */
   copy->num_rects = num_rects;
//TODO:maynard: copy.bbox needs to get set 
   return XIL_SUCCESS;
}

#ifdef DEBUG
void
XiliRoiRect::dump()
{
    XiliRectInt *print_rect;

    if(num_rects == 1) {
        print_rect = &singleRect;
    } else {
        print_rect=first;
    }
    fprintf(stderr, "  Number of rects: %d\n", num_rects);
    fprintf(stderr, "  x1    x2    y1    y2\n");
    fprintf(stderr, " ----  ----  ----  ----\n");
    while (print_rect) {
        fprintf(stderr, "%5d %5d %5d %5d\n",print_rect->getX1(),print_rect->getX2(),print_rect->getY1(),print_rect->getY2());
	print_rect= (XiliRectInt*)print_rect->getNext();
    }
}
#endif


XilStatus
XiliRoiRect::intersect(XiliRoiRect* other_rl,
                       XiliRoiRect* intersection)
{
    XiliRectInt* endptr;
    XiliRectInt* thisrect;
    XiliRectInt* rect;
    int this_y1, this_y2, y1, y2, common_y1, common_y2;

    if(intersection == NULL) {
        return XIL_FAILURE;
    }

    //
    //  Clear resultant intersection ROI
    //
    intersection->num_rects = 0;
    while(intersection->first) {
	endptr = (XiliRectInt*)intersection->first->getNext();
	delete intersection->first;
	intersection->first = endptr;
    }

    //
    // There are several possibilities :
    //
    //   - If either of the lists is empty, return the empty XiliRectIntList
    //
    //   - If both the rectlists are singleRect format, do simple intersect
    //
    //   - If either of the rectlists are singleRect, simplify intersect - not
    //        currently done
    //
    //   - Otherwise do full intersect routine
    //
    if(this->num_rects == 0) {
        *intersection = *this;
        return XIL_SUCCESS;
    }
    if(other_rl->num_rects == 0) {
        *intersection = *other_rl;
        return XIL_SUCCESS;
    }

    if((this->num_rects == 1) && (other_rl->num_rects == 1)) {
        //
        //  Simple single-rect intersection case.
        //
        int i_x1 = _XILI_MAX(singleRect.getX1(), other_rl->singleRect.getX1());
        int i_x2 = _XILI_MIN(singleRect.getX2(), other_rl->singleRect.getX2());
        int i_y1 = _XILI_MAX(singleRect.getY1(), other_rl->singleRect.getY1());
        int i_y2 = _XILI_MIN(singleRect.getY2(), other_rl->singleRect.getY2());

        //
        //  Check to see if the intersection resulted in a NULL box by the
        //  fact that the ending points of the box (x2, y2) are before the
        //  beginning point of the intersected box (x1, y1).
        //
        if((i_x2 < i_x1) || (i_y2 < i_y1)) {
            intersection->num_rects = 0;
            intersection->bbox.set(0, 0, 0, 0);
            intersection->singleRect.set(0, 0, 0, 0);
            intersection->first = NULL;
            return XIL_SUCCESS;
        } else {
            intersection->num_rects = 1;
            intersection->bbox.set(i_x1, i_y1, i_x2, i_y2);
            intersection->singleRect.set(i_x1, i_y1, i_x2, i_y2);
            intersection->first = NULL;
            return XIL_SUCCESS;
        }

    }
    if(this->num_rects == 1) {
        thisrect = &singleRect;
    } else {
        thisrect = first;
    }
    if(other_rl->num_rects == 1) {
        rect = &(other_rl->singleRect);
    } else {
        rect = other_rl->first;
    }

    // go ahead and add a rect so that the loop doesn't have to deal with a special case
    intersection->first= new XiliRectInt(0,0,1,1);
    if (intersection->first==NULL) {
        XIL_ERROR(myRoi->getSystemState(),XIL_ERROR_RESOURCE,"di-1",TRUE);
	return XIL_FAILURE;
    }
    intersection->num_rects++;
    endptr= intersection->first;

    while(thisrect && rect) {
        this_y1= thisrect->getY1();
	this_y2= thisrect->getY2();
	y1= rect->getY1();
	y2= rect->getY2();

	// do they intersect in Y?
	if((this_y1 >= y1) && (this_y1 <= y2)) {
	    // they intersect, what do they have in common?
	    common_y1= this_y1;
	    if(y2 < this_y2) {
	        common_y2= y2;
	    } else {
	        common_y2= this_y2;
	    }

	    // intersect the bands
	    int numrects;
	    endptr->setNext(intersectBands(thisrect,rect,common_y1,common_y2, &numrects));
// TODO:maynard : This is an ugly... see if there's a better way to do this
	    if((int)endptr->getNext() == -1) {
	        XIL_ERROR(myRoi->getSystemState(), XIL_ERROR_SYSTEM,"di-7",FALSE);
		delete intersection->first;
                intersection->first = NULL;
		return XIL_FAILURE;
	    }
	    intersection->num_rects += numrects;

	    // move to the end
	    while(endptr->getNext()) {
	        endptr= (XiliRectInt*)endptr->getNext();
	    }
	} else if((y1 >= this_y1) && (y1 <= this_y2)) {
	    // they intersect, what do they have in common? 
	    common_y1= y1;
	    if(this_y2 < y2) {
	        common_y2= this_y2;
	    } else {
	        common_y2= y2;
	    }

	    // intersect the bands 
	    int numrects;
	    endptr->setNext(intersectBands(thisrect,rect,common_y1,common_y2, &numrects));
// TODO:maynard : This is an ugly... see if there's a better way to do this
	    if((int)endptr->getNext() == -1) {
	        XIL_ERROR(myRoi->getSystemState(), XIL_ERROR_SYSTEM,"di-7",FALSE);
		delete intersection->first;
                intersection->first = NULL;
		return XIL_FAILURE;
	    }
	    intersection->num_rects += numrects;
	    
	    // move to the end 
	    while(endptr->getNext()) {
	        endptr= (XiliRectInt*)endptr->getNext();
	    }
	}
	
	// advance the one that has advanced in the Y direction the least
	if(rect->getY2() < thisrect->getY2()) {
	    while(rect && (rect->getY2()==y2)) {
	        rect= (XiliRectInt*)rect->getNext();
	    }
	} else {
	    while(thisrect && (thisrect->getY2()==this_y2)) {
	        thisrect= (XiliRectInt*)thisrect->getNext();
	    }
	}
    } //     while(thisrect && rect)

    // get rid of the extra box
    rect= intersection->first;
    intersection->first= (XiliRectInt*)intersection->first->getNext();
    delete rect;
    intersection->num_rects--;
    intersection->updateBoundingBox();
    if(intersection->num_rects == 1) {
        intersection->singleRect.set(intersection->first);
        delete intersection->first;
        intersection->first = NULL;
    }
    return XIL_SUCCESS;
}

XilStatus
XiliRoiRect::unite(XiliRoiRect* other_rl,
		   XiliRoiRect* united_rl)
{
    if(united_rl == NULL) {
        return XIL_FAILURE;
    }
    
    //TODO: maynard - do I need to worry about clearing copy?

    if(num_rects > 0) { // if 1st ROI is non-NULL...
	if(other_rl->num_rects == 0) {
	    // Other rect was empty just copy this roi
	    *united_rl = *this;
	    return XIL_SUCCESS;
	}
	
	//
	// Check to see if the two bounding boxes intersect
	//
	int x1, y1, x2, y2;
	int o_x1, o_y1, o_x2, o_y2;
	int int_x1, int_y1, int_x2, int_y2;

	bbox.get(&x1, &y1, &x2, &y2);
	other_rl->bbox.get(&o_x1, &o_y1, &o_x2, &o_y2);

	int_x1 = _XILI_MAX(x1, o_x1);
	int_y1 = _XILI_MAX(y1, o_y1);
	int_x2 = _XILI_MIN(x2, o_x2);
	int_y2 = _XILI_MIN(y2, o_y2);

	if((int_x2 >= int_x1) && (int_y2 >= int_y1)) {
	    //
	    // We have an intersection
	    //
	    
	    //
	    // Copy over this roi
	    //
	    *united_rl = *this;

	    //
	    // Add in the other_rl
	    //
	    XiliRectInt *rect;
	    if(other_rl->num_rects == 1) {
		rect = &(other_rl->singleRect);
	    } else {
		rect = other_rl->first;
	    }

	    //
	    // TODO:maynard: this can be done faster by doing the
	    // actual union rather than just doing addrects
	    //
	    while(rect) {
		if(united_rl->addRect(rect->getX1(), rect->getY1(),
				      rect->getX2() - rect->getX1() + 1,
				      rect->getY2() - rect->getY1() + 1) == XIL_FAILURE) {
		    return XIL_FAILURE;
		}
		rect = (XiliRectInt*)rect->getNext();
	    }
	    
	    // addrect takes care of updating the bounding box
	    return XIL_SUCCESS;
	} else {
	    //
	    // No intersection just join the lists together
	    // maintain y ordering by checking the bounding box y1
	    // values
	    //
	    XiliRoiRect* entry1;
	    XiliRoiRect* entry2;
	    if(y1 == o_y1) {
		if(x1 < o_x1) {
		    entry1 = this;
		    entry2 = other_rl;
		} else {
		    entry1 = other_rl;
		    entry2 = this;
		}
	    } else if(y1 < o_y1) {
		entry1 = this;
		entry2 = other_rl;
	    } else {
		entry1 = other_rl;
		entry2 = this;
	    }
	    
	    //
	    // Use the copy constructor to build the
	    // initial list. We will insert entry1 before
	    // entry2
	    //
	    *united_rl = *entry2;

	    //
	    // Save or copy a pointer to the first rect in
	    // the list
	    //
	    XiliRectInt *tmp_first;
	    if(united_rl->num_rects == 1) {
		tmp_first = new XiliRectInt(&united_rl->singleRect);
		
		if (tmp_first == NULL) {
		    XIL_ERROR(myRoi->getSystemState(), XIL_ERROR_RESOURCE,
			      "di-1", TRUE);
		    return XIL_FAILURE;
		}
		tmp_first->setNext(NULL);
	    } else {
		tmp_first =united_rl->first;
	    }

	    XiliRectInt *rect;
	    if(entry1->num_rects == 1) {
		rect = &(entry1->singleRect);
	    } else {
		rect = entry1->first;
	    }
	    
	    united_rl->first = new XiliRectInt(rect);
	    if(united_rl->first == NULL) {
		XIL_ERROR(myRoi->getSystemState(), XIL_ERROR_RESOURCE,
			  "di-1", TRUE);
		return XIL_FAILURE;
	    }
	    united_rl->first->setNext(NULL);

	    //
	    // Copy rest of rect's
	    //
	    XiliRectInt* iptr = (XiliRectInt*)rect->getNext();
	    XiliRectInt* optr = united_rl->first;
	    while(iptr) {
		optr->setNext(new XiliRectInt(iptr));
		if(optr->getNext() == NULL) {
		    XIL_ERROR(myRoi->getSystemState(), XIL_ERROR_RESOURCE,
			      "di-1", TRUE);
		    return XIL_FAILURE;
		}
		optr = (XiliRectInt*)optr->getNext();
		optr->setNext(NULL);
		iptr = (XiliRectInt*)iptr->getNext();
	    }

	    //
	    // Attach the two lists together
	    //
	    optr->setNext(tmp_first);

	    //
	    // Finish updating the information in the output roi
	    //
	    united_rl->num_rects = entry1->num_rects + entry2->num_rects;
	    united_rl->bbox.set(_XILI_MIN(x1, o_x1), _XILI_MIN(y1, o_y1),
					 _XILI_MAX(x2, o_x2), _XILI_MAX(y2, o_y2));
	    return XIL_SUCCESS;
	}
    } else {
        *united_rl = *other_rl;
	return XIL_SUCCESS;
    }
}

//
// The caller is assuring us that the two rois are disjoint
// and therefore we can safely just join the two together
//
//
// TODO: maynard 8/26/96 - broken
// This call can break roiRect ordering which causes incorrect
// results from subsequent roiRect calls (like intersect).
// An example would be uniting 0,0 -> 2,2 : 3,3 -> 5,5
// with 6,0 -> 8,2 : 9,3 ->11,5 (same  y, offset in x by 6 )
// Note - ordering maintains y-ordering first, with x-ordering
// second. No touching Roi's in the x-direction.
//
XilStatus
XiliRoiRect::unite_disjoint(XiliRoiRect* other_rl,
			    XiliRoiRect* united_rl)
{
    if(united_rl == NULL) {
        return XIL_FAILURE;
    }
    
    if(num_rects > 0) { // if 1st ROI is non-NULL...
	if(other_rl->num_rects == 0) {
	    // Other rect was empty just copy this roi
	    *united_rl = *this;
	    return XIL_SUCCESS;
	}
	
	//
	// No intersection just join the lists together
	// maintain y ordering by checking the bounding box y1
	// values
	//
	int x1, y1, x2, y2;
	int o_x1, o_y1, o_x2, o_y2;

	bbox.get(&x1, &y1, &x2, &y2);
	other_rl->bbox.get(&o_x1, &o_y1, &o_x2, &o_y2);

	XiliRoiRect* entry1;
	XiliRoiRect* entry2;
	if(y1 == o_y1) {
	    if(x1 < o_x1) {
		entry1 = this;
		entry2 = other_rl;
	    } else {
		entry1 = other_rl;
		entry2 = this;
	    }
	} else if(y1 < o_y1) {
	    entry1 = this;
	    entry2 = other_rl;
	} else {
	    entry1 = other_rl;
	    entry2 = this;
	}
	    
	//
	// Use the copy constructor to build the
	// initial list. We will insert entry1 before
	// entry2
	//
	*united_rl = *entry2;

	//
	// Save or copy a pointer to the first rect in
	// the list
	//
	XiliRectInt *tmp_first;
	if(united_rl->num_rects == 1) {
	    tmp_first = new XiliRectInt(&united_rl->singleRect);
		
	    if (tmp_first == NULL) {
		XIL_ERROR(myRoi->getSystemState(), XIL_ERROR_RESOURCE,
			  "di-1", TRUE);
		return XIL_FAILURE;
	    }
	    tmp_first->setNext(NULL);
	} else {
	    tmp_first =united_rl->first;
	}

	XiliRectInt *rect;
	if(entry1->num_rects == 1) {
	    rect = &(entry1->singleRect);
	} else {
	    rect = entry1->first;
	}
	    
	united_rl->first = new XiliRectInt(rect);
	if(united_rl->first == NULL) {
	    XIL_ERROR(myRoi->getSystemState(), XIL_ERROR_RESOURCE,
		      "di-1", TRUE);
	    return XIL_FAILURE;
	}
	united_rl->first->setNext(NULL);

	    //
	    // Copy rest of rect's
	    //
	XiliRectInt* iptr = (XiliRectInt*)rect->getNext();
	XiliRectInt* optr = united_rl->first;
	while(iptr) {
	    optr->setNext(new XiliRectInt(iptr));
	    if(optr->getNext() == NULL) {
		XIL_ERROR(myRoi->getSystemState(), XIL_ERROR_RESOURCE,
			  "di-1", TRUE);
		return XIL_FAILURE;
	    }
	    optr = (XiliRectInt*)optr->getNext();
	    optr->setNext(NULL);
	    iptr = (XiliRectInt*)iptr->getNext();
	}

	//
	// Attach the two lists together
	//
	optr->setNext(tmp_first);

	//
	// Finish updating the information in the output roi
	//
	united_rl->num_rects = entry1->num_rects + entry2->num_rects;
	united_rl->bbox.set(_XILI_MIN(x1, o_x1), _XILI_MIN(y1, o_y1),
				     _XILI_MAX(x2, o_x2), _XILI_MAX(y2, o_y2));
	return XIL_SUCCESS;
    } else {
        *united_rl = *other_rl;
	return XIL_SUCCESS;
    }
}

XilStatus
XiliRoiRect::translate(int          x,
		       int          y,
		       XiliRoiRect* copy)  
{
    if(copy == NULL) {
        return XIL_FAILURE;
    }
    //TODO: maynard - do I need to worry about clearing copy?
    if(num_rects == 1) {
        int x1,x2,y1,y2;
        copy->num_rects =1;
        this->singleRect.get(&x1, &y1, &x2, &y2);
        copy->singleRect.set(x1+x, y1+y, x2+x, y2+y);
        copy->bbox.set(x1+x, y1+y, x2+x, y2+y);
        copy->first = NULL;
        return XIL_SUCCESS;
    }
    if(first) {
        // Copy first rect with translation
        copy->first = new XiliRectInt(first->getX1()+x, first->getY1()+y,
                                   first->getX2()+x, first->getY2()+y);
        if(copy->first == NULL) {
	    XIL_ERROR(myRoi->getSystemState(), XIL_ERROR_RESOURCE,"di-1",TRUE);
            return XIL_FAILURE;
        }
        copy->first->setNext(NULL);

        // Copy rest of rect's
        XiliRectInt* iptr = (XiliRectInt*)first->getNext();
        XiliRectInt* optr = copy->first;
        while(iptr) {
	    optr->setNext(new XiliRectInt(iptr->getX1()+x, iptr->getY1()+y,
                                      iptr->getX2()+x, iptr->getY2()+y));
            if(optr->getNext()==NULL) {
	        XIL_ERROR(myRoi->getSystemState(),XIL_ERROR_RESOURCE,"di-1",TRUE);
                delete copy->first;
                copy->first = NULL;
                return XIL_FAILURE;
            }
            optr = (XiliRectInt*)optr->getNext();
            optr->setNext(NULL);
            iptr = (XiliRectInt*)iptr->getNext();
        }
        copy->num_rects = num_rects;

	//
	// Updating the bounding box just requires a translation
	// of this bounding box
	//
	int x1,x2,y1,y2;
	this->bbox.get(&x1, &y1, &x2, &y2);
        copy->bbox.set(x1+x, y1+y, x2+x, y2+y);
	return XIL_SUCCESS;
   } else {
       return XIL_SUCCESS;
   }
}

XilStatus
XiliRoiRect::translate_inplace(int x,
			       int y)
{
    int x1,x2,y1,y2;

    if(num_rects == 1) {
        singleRect.get(&x1,&y1,&x2,&y2);
        singleRect.set(x1+x,y1+y,x2+x,y2+y);
        bbox.set(x1+x,y1+y,x2+x,y2+y);

        return XIL_SUCCESS;
    }
    XiliRectInt* iptr = first;
    while(iptr) {
        iptr->set(iptr->getX1() + x,
                  iptr->getY1() + y,
                  iptr->getX2() + x,
                  iptr->getY2() + y);
        iptr = (XiliRectInt*)iptr->getNext();
    }
    bbox.get(&x1, &y1, &x2, &y2);
    bbox.set(x1+x, y1+y, x2+x, y2+y);

    return XIL_SUCCESS;
}


XilStatus
XiliRoiRect::transpose(XilFlipType fliptype,
                       float xorigin,
                       float yorigin,
                       XiliRoiRect* tpose_copy)
{
    XiliRectInt* rect;

    if(num_rects == 1) {
        rect = &singleRect;
    } else {
        rect = first;
    }
    //
    // transpose each rect and insert into new ROI
    //
    switch(fliptype) {
      case XIL_FLIP_X_AXIS:
        while(rect) { // swap and negate y
            float x1 =  ((float)rect->getX1() - xorigin) + xorigin;
            float y1 = -((float)rect->getY2() - yorigin) + yorigin;
            float x2 =  ((float)rect->getX2() - xorigin) + xorigin;
            float y2 = -((float)rect->getY1() - yorigin) + yorigin;
            int width =  (int)(x2 - x1 + 1);
            int height = (int)(y2 - y1 + 1);
            if(tpose_copy->addRect((int)x1,(int)y1,(unsigned int)width,(unsigned int)height)==XIL_FAILURE) {
                delete tpose_copy;
                return XIL_FAILURE;
            }
            rect = (XiliRectInt*)rect->getNext();
        }
        break;
      case XIL_FLIP_Y_AXIS:
        while(rect) { // swap and negate x 
            float x1 = -((float)rect->getX2() - xorigin) + xorigin;
            float y1 =  ((float)rect->getY1() - yorigin) + yorigin;
            float x2 = -((float)rect->getX1() - xorigin) + xorigin;
            float y2 =  ((float)rect->getY2() - yorigin) + yorigin;
            int width =  (int)(x2 - x1 + 1);
            int height = (int)(y2 - y1 + 1);
            if(tpose_copy->addRect((int)x1,(int)y1,(unsigned int)width,(unsigned int)height)==XIL_FAILURE) {
                delete tpose_copy;
                return XIL_FAILURE;
            }
            rect = (XiliRectInt*)rect->getNext();
        }
        break;
      case XIL_FLIP_MAIN_DIAGONAL:
        while(rect) { // swap x&y 
            float x1 = ((float)rect->getY1() - yorigin) + xorigin;
            float y1 = ((float)rect->getX1() - xorigin) + yorigin;
            float x2 = ((float)rect->getY2() - yorigin) + xorigin;
            float y2 = ((float)rect->getX2() - xorigin) + yorigin;
            int width =  (int)(x2 - x1 + 1);
            int height = (int)(y2 - y1 + 1);
            if(tpose_copy->addRect((int)x1,(int)y1,(unsigned int)width,(unsigned int)height)==XIL_FAILURE) {
                delete tpose_copy;
                return XIL_FAILURE;
            }
            rect = (XiliRectInt*)rect->getNext();
        }
        break;
      case XIL_FLIP_ANTIDIAGONAL:
        while(rect) { // interchange and swap x&y, negate both
            float x1 = -((float)rect->getY2() - yorigin) + xorigin;
            float y1 = -((float)rect->getX2() - xorigin) + yorigin;
            float x2 = -((float)rect->getY1() - yorigin) + xorigin;
            float y2 = -((float)rect->getX1() - xorigin) + yorigin;
            int width =  (int)(x2 - x1 + 1);
            int height = (int)(y2 - y1 + 1);
            if(tpose_copy->addRect((int)x1,(int)y1,(unsigned int)width,(unsigned int)height)==XIL_FAILURE) {
                delete tpose_copy;
                return XIL_FAILURE;
            }
            rect = (XiliRectInt*)rect->getNext();
        }
        break;
      case XIL_FLIP_90:
        while(rect) { // x1 = y1, y1 = -x2, x2 = y2, y2 = -x1 
            float x1 =  ((float)rect->getY1() - yorigin) + xorigin;
            float y1 = -((float)rect->getX2() - xorigin) + yorigin;
            float x2 =  ((float)rect->getY2() - yorigin) + xorigin;
            float y2 = -((float)rect->getX1() - xorigin) + yorigin;
            int width =  (int)(x2 - x1 + 1);
            int height = (int)(y2 - y1 + 1);
            if(tpose_copy->addRect((int)x1,(int)y1,(unsigned int)width,(unsigned int)height)==XIL_FAILURE) {
                delete tpose_copy;
                return XIL_FAILURE;
            }
            rect = (XiliRectInt*)rect->getNext();
        }
        break;
      case XIL_FLIP_180:
        while(rect) { // x1 = -x2, y1 = -y2, x2 = -x1, y2 = -y1
            float x1 = -((float)rect->getX2() - xorigin) + xorigin;
            float y1 = -((float)rect->getY2() - yorigin) + yorigin;
            float x2 = -((float)rect->getX1() - xorigin) + xorigin;
            float y2 = -((float)rect->getY1() - yorigin) + yorigin;
            int width =  (int)(x2 - x1 + 1);
            int height = (int)(y2 - y1 + 1);
            if(tpose_copy->addRect((int)x1,(int)y1,(unsigned int)width,(unsigned int)height)==XIL_FAILURE) {
                delete tpose_copy;
                return XIL_FAILURE;
            }
            rect = (XiliRectInt*)rect->getNext();
        }
        break;
      case XIL_FLIP_270:
        while(rect) { // x1 = -y2, y1 = x1, x2 = -y1, y2 = x2 
            float x1 = -((float)rect->getY2() - yorigin) + xorigin;
            float y1 =  ((float)rect->getX1() - xorigin) + yorigin;
            float x2 = -((float)rect->getY1() - yorigin) + xorigin;
            float y2 =  ((float)rect->getX2() - xorigin) + yorigin;
            int width =  (int)(x2 - x1 + 1);
            int height = (int)(y2 - y1 + 1);
            if(tpose_copy->addRect((int)x1,(int)y1,(unsigned int)width,(unsigned int)height)==XIL_FAILURE) {
                delete tpose_copy;
                return XIL_FAILURE;
            }
            rect = (XiliRectInt*)rect->getNext();
        }
        break;
    }
    return XIL_SUCCESS;
}

XilStatus
XiliRoiRect::scale(float  xscale,
                   float  yscale,
                   float  xorigin,
                   float  yorigin,
                   XiliRoiRect* scaled_copy)
{
    //
    // TODO: this could be done much faster directly. This way is order N^2.
    //
    int x1,x2,y1,y2;
    int width,height; // these need to be int, not unsigned here

    if(num_rects == 1) {
        int sx1,sx2,sy1,sy2;
        singleRect.get(&sx1,&sy1,&sx2,&sy2);
        x1 = (int)((((float)sx1 - xorigin) * xscale) + xorigin);
        y1 = (int)((((float)sy1 - yorigin) * yscale) + yorigin);
        x2 = (int)((((float)sx2 - xorigin + 1) * xscale) + xorigin) - 1;
        y2 = (int)((((float)sy2 - yorigin + 1) * yscale) + yorigin) - 1;
        scaled_copy->singleRect.set(x1,y1,x2,y2);
        scaled_copy->num_rects = 1;
        scaled_copy->bbox.set(x1,y1,x2,y2);
        return XIL_SUCCESS;
    }

    XiliRectInt* rect=first;
    //
    // scale each rect and insert into new ROI
    //
    while(rect) {
        x1 = (int)((((float)rect->getX1() - xorigin) * xscale) + xorigin);
        y1 = (int)((((float)rect->getY1() - yorigin) * yscale) + yorigin);
        x2 = (int)((((float)rect->getX2() - xorigin + 1) * xscale) + xorigin) - 1;
        y2 = (int)((((float)rect->getY2() - yorigin + 1) * yscale) + yorigin) - 1;
        width =  x2 - x1 + 1;
        height = y2 - y1 + 1;
        
        //
        //  It's ok if the width or height drops below 0, just don't add
        //   the rect.
        //
        if(width > 0 && height > 0) {
            if (scaled_copy->addRect(x1, y1, (unsigned int)width, (unsigned int)height)==XIL_FAILURE) {
                delete scaled_copy;
                return XIL_FAILURE;
            }
        }
        rect = (XiliRectInt*)rect->getNext();
    }
    return XIL_SUCCESS;
}

//------------------------------------------------------------------------
//
//  Function:	XiliRoiRect::getAsRegion/addRegion
//
//  Description: The following two routines are used to get/set X11 Region 
//	         information on a Roi. This should ultimately be EOL'd because
//	         they depend on now opaque data structrues from X11.
//	
//	
//  MT-level:  <??????>
//	
//  Parameters:
//	
//	
//  Returns:
//	
//	
//  Side Effects:
//	
//	
//  Notes:
//	
//	
//  Deficiencies/ToDo:
//	
//	
//------------------------------------------------------------------------

#ifdef _XIL_HAS_X11WINDOWS
//
// Region definitions from X11
// rather incorrectly defined here because they're now opaque types.
// I retrieved this definition from header file Xutils.h and ????.h
//
typedef struct {
    short x1, x2, y1, y2;
} BOX;

typedef struct _XRegion {
    long size;
    long numRects;
    BOX *rects;
    BOX  extents;
} REGION;
#endif // _XIL_HAS_X11WINDOWS

Region
XiliRoiRect::getAsRegion()
{
    XiliRectInt* rect;
    Region  region;
#ifdef _XIL_HAS_X11WINDOWS
    BOX* Xrect;
#endif
    
    if(num_rects > 0) {
        //
        // allocate region structure
        //
#ifdef _XIL_HAS_X11WINDOWS
        //
        // TODO bpb 11/24/1997 Shouldn't XCreateRegion () be used below
        // instead of "new"? Calling XDestroyRegion () on the Region returned
        // by this method currently results in a segmentation fault.
        //
        region = (Region) new REGION;
        if(!region) {
            XIL_ERROR(myRoi->getSystemState(), XIL_ERROR_RESOURCE,"di-1",TRUE);
            return(NULL);
        }
        
        //
        // allocate an Xrect BOX for each rectangle 
        //
        region->numRects = num_rects;
        region->rects = (BOX *) new BOX[region->numRects];
        if(!region->rects) {
            XIL_ERROR(myRoi->getSystemState(), XIL_ERROR_RESOURCE,"di-1",TRUE);
            delete region;
            return(NULL);
        }
#endif // _XIL_HAS_X11WINDOWS

        //
        // fill them in 
        //
        //  The Xrect is the boundary of the region, it is NOT
        //    inclusive.  We store our BOXes with an inclusive
        //    region so we must extend our box by 1 when creating the
        //    Xrect.
        //

        //
        // special case for only one rect in roi
        //
        if(num_rects == 1) {
#ifdef _XIL_HAS_X11WINDOWS
            Xrect = region->rects;
            region->extents.x1 = (short)singleRect.getX1();
            region->extents.x2 = (short)singleRect.getX2() + 1;
            region->extents.y1 = (short)singleRect.getY1();
            region->extents.y2 = (short)singleRect.getY2() + 1;
            Xrect->x1 = ((short)singleRect.getX1());
            Xrect->x2 = ((short)singleRect.getX2() + 1);
            Xrect->y1 = ((short)singleRect.getY1());
            Xrect->y2 = ((short)singleRect.getY2() + 1);
#elif _WINDOWS
            if((region = CreateRectRgn((int)singleRect.getX1(),
                                       (int)singleRect.getY1(),
                                       (int)singleRect.getX2() + 1,
                                       (int)singleRect.getY2() + 1)) == NULL) {
                XIL_ERROR(myRoi->getSystemState(), XIL_ERROR_RESOURCE,
                          "di-8", TRUE);
                return(NULL);
            }
#endif // _XIL_HAS_X11WINDOWS
            return region;
        }

        rect=first;
#ifdef _XIL_HAS_X11WINDOWS
        Xrect = region->rects;
        region->extents.x1 = (short)rect->getX1();
        region->extents.x2 = (short)rect->getX2() + 1;
        region->extents.y1 = (short)rect->getY1();
        region->extents.y2 = (short)rect->getY2() + 1;
#elif _WINDOWS
        if((region = CreateRectRgn((int)rect->getX1(),
                                   (int)rect->getY1(),
                                   (int)rect->getX2() + 1,
                                   (int)rect->getY2() + 1)) == NULL) {
            XIL_ERROR(myRoi->getSystemState(), XIL_ERROR_RESOURCE, "di-8",TRUE);
            return(NULL);
        }
#endif // _XIL_HAS_X11WINDOWS
        while(rect) {
#ifdef _XIL_HAS_X11WINDOWS
            Xrect->x1 = (short)rect->getX1();
            Xrect->x2 = (short)rect->getX2() + 1;
            Xrect->y1 = (short)rect->getY1();
            Xrect->y2 = (short)rect->getY2() + 1;
            if (region->extents.x1 > Xrect->x1)
                region->extents.x1 = Xrect->x1;
            if (region->extents.x2 < Xrect->x2)
                region->extents.x2 = Xrect->x2;
            if (region->extents.y1 > Xrect->y1)
                region->extents.y1 = Xrect->y1;
            if (region->extents.y2 < Xrect->y2)
                region->extents.y2 = Xrect->y2;
            Xrect++;
#elif _WINDOWS
            Region rgn;
            if((rgn = CreateRectRgn((int)rect->getX1(),
                                    (int)rect->getY1(),
                                    (int)rect->getX2() + 1,
                                    (int)rect->getY2() + 1)) == NULL) {
                DeleteObject(region);
                XIL_ERROR(myRoi->getSystemState(), XIL_ERROR_RESOURCE,
                          "di-8", TRUE);
                return(NULL);
            }
            if(CombineRgn(region, region, rgn, RGN_OR) == ERROR) {
                DeleteObject(rgn);
                DeleteObject(region);
                XIL_ERROR(myRoi->getSystemState(), XIL_ERROR_RESOURCE,
                          "di-8", TRUE);
                return(NULL);
            }
            DeleteObject(rgn);
#endif // _XIL_HAS_X11WINDOWS
            rect= (XiliRectInt*)rect->getNext();
        }
        return(region);
    } else {
        // TODO: this shouldn't be NULL if a region can have zero rectangles 
        return(NULL);
    }
}

XilStatus
XiliRoiRect::addRegion(Region region)
{
#ifdef _WINDOWS
    unsigned int region_data_size = GetRegionData(region, 0, NULL);
    RGNDATA* region_data = (RGNDATA*)new Xil_unsigned8[region_data_size];

    if(region_data == NULL) {
#ifdef DEBUG
        fprintf(stderr, "%s %d\n", __FILE__, __LINE__);
#endif
        XIL_ERROR(myRoi->getSystemState(),XIL_ERROR_RESOURCE, "di-1", TRUE);
        return XIL_FAILURE;
    }

    if(GetRegionData(region, region_data_size, region_data) == 0) {
        //
        // TODO bpb 11/24/1997 What error should be here?
        //
        delete region_data;
#ifdef DEBUG
        fprintf(stderr, "%s %d\n", __FILE__, __LINE__);
#endif
        return XIL_FAILURE;
    }

    int region_numrects = (int)region_data->rdh.nCount;
    RECT* Wrect = (RECT *)region_data->Buffer;
#elif _XIL_HAS_X11WINDOWS
    int region_numrects = region->numRects;
    BOX* Xrect = region->rects;
#endif // _WINDOWS

    if(region_numrects == 0) {
        return XIL_SUCCESS;
    }

    if(num_rects >0) {
        //
        // Check for single rect case.... expand to non-optimal case
        //
        if(num_rects ==1) { 
            int sx1,sx2,sy1,sy2;

            singleRect.get(&sx1,&sy1,&sx2,&sy2);
            first = new XiliRectInt(sx1, sy1, sx2, sy2);
            singleRect.set(0,0,0,0);
        }           
            
        //
        // if there is a region...
        //
        for(int i = 0; i < region_numrects; i++) {
            //
            // add each XRectangle to ROI
            //
#ifdef _XIL_HAS_X11WINDOWS
            int x = (int)Xrect->x1;
            int y = (int)Xrect->y1;
            unsigned int width = (int)Xrect->x2 - x + 1;
            unsigned int height = (int)Xrect->y2 - y + 1;
#elif _WINDOWS
            int x = (int)Wrect->left;
            int y = (int)Wrect->top;
            unsigned int width = (int)Wrect->right - x + 1;
            unsigned int height = (int)Wrect->bottom - y + 1;
#endif // _XIL_HAS_X11WINDOWS
            if(this->addRect(x, y, width, height)==XIL_FAILURE) {
#ifdef DEBUG
                fprintf(stderr, "%s %d\n", __FILE__, __LINE__);
#endif
#ifdef _WINDOWS
                delete region_data;
#endif
                return XIL_FAILURE;
            }
#ifdef _XIL_HAS_X11WINDOWS
            Xrect++;
#elif _WINDOWS
            Wrect++;
#endif // _XIL_HAS_X11WINDOWS
        }
        
    } else {
        // No Roi - just convert Xrects to rects
        // This depends on the Xrect implementation maintaining the following
        // conditions: 
        // Each region is implemented as a "y-x-banded" array of rectangles.
        // To explain: Each Region is made up of a certain number of 
        // rectangles sorted by y coordinate first, and then by x coordinate.
        //
        // Furthermore, the rectangles are banded such that every rectangle 
        // with a given upper-left y coordinate (y1) will have the same lower
        // -right y coordinate (y2) and vice versa. If a rectangle has 
        // scanlines in a band, it will span the entire vertical distance of
        // the band. This means that some areas that could be merged into a 
        // taller rectangle will be represented as several shorter rectangles
        // to account for shorter rectangles to its left or right but within
        // its "vertical scope".
        
        // Special case when region_numrects is only 1 - use singleRect implementation
        if(region_numrects == 1) {
            num_rects = 1;
#ifdef _XIL_HAS_X11WINDOWS
            singleRect.set((int)Xrect->x1,(int)Xrect->y1,(int)Xrect->x2-1,(int)Xrect->y2-1);
            bbox.set((int)Xrect->x1,(int)Xrect->y1,(int)Xrect->x2-1,(int)Xrect->y2-1);
#elif _WINDOWS
            singleRect.set((int)Wrect->left,(int)Wrect->top,(int)Wrect->right-1,(int)Wrect->bottom-1);
            bbox.set((int)Wrect->left,(int)Wrect->top,(int)Wrect->right-1,(int)Wrect->bottom-1);
            delete region_data;
#endif // _XIL_HAS_X11WINDOWS
            return XIL_SUCCESS;
        }

        //
        // more than one rect 
        // add a rect so that the loop doesn't have to deal with a special case
        //
        first= new XiliRectInt(0,0,1,1);
        if(first==NULL) {
#ifdef _WINDOWS
            delete region_data;
#endif
            XIL_ERROR(myRoi->getSystemState(),XIL_ERROR_RESOURCE,"di-1",TRUE);
            return XIL_FAILURE;
        }
        XiliRectInt* current_rect = first;
        num_rects++;
        
        for(int i = 0; i < region_numrects; i++) {
            //
            // convert each XRectangle to ROI
            //
            XiliRectInt* temp_rect= new XiliRectInt(current_rect);
            if(!temp_rect) {
#ifdef _WINDOWS
                delete region_data;
#endif
                XIL_ERROR(myRoi->getSystemState(), XIL_ERROR_RESOURCE,"di-1",TRUE);
                return XIL_FAILURE;
            }
            num_rects++;
            
            //
            //  The Xrect is the boundary of the region, it is NOT
            //    inclusive.  We store our rectes with an inclusive
            //    region so we must knock off X's boundary.
            //
#ifdef _XIL_HAS_X11WINDOWS
            temp_rect->setX1((int)Xrect->x1);
            temp_rect->setX2((int)Xrect->x2 - 1);
            temp_rect->setY1((int)Xrect->y1);
            temp_rect->setY2((int)Xrect->y2 - 1);
            Xrect++;
#elif _WINDOWS
            temp_rect->setX1((int)Wrect->left);
            temp_rect->setX2((int)Wrect->right - 1);
            temp_rect->setY1((int)Wrect->top);
            temp_rect->setY2((int)Wrect->bottom - 1);
            Wrect++;
#endif // _XIL_HAS_X11WINDOWS
            current_rect->setNext(temp_rect);
            current_rect= temp_rect;
        }
        
        //
        // get rid of dummy first rect 
        //
        current_rect = first;
        first = (XiliRectInt*)current_rect->getNext();
        delete current_rect;
        num_rects--;
    }
#ifdef _WINDOWS
    delete region_data;
#endif
    return XIL_SUCCESS;
}


//------------------------------------------------------------------------
// End of X11 Region functions
//------------------------------------------------------------------------


