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
//  File:	XiliRoiConvexRegion.cc
//  Project:	XIL
//  Revision:	1.53
//  Last Mod:	10:08:42, 03/10/00
//
//  Description: This file is the Convex Region List implementation of the ROI.
//	
//
//  MT-level:  <??????>
//	
//------------------------------------------------------------------------
//	COPYRIGHT
//------------------------------------------------------------------------
#pragma ident	"@(#)XiliRoiConvexRegion.cc	1.53\t00/03/10  "

#include "_XilDefines.h"
#include "_XilSystemState.hh"
#include "_XilBox.hh"
#include "_XilRoi.hh"

#include "XiliConvexRegion.hh"
#include "XiliList.hh"
#include "XiliUtils.hh"

//------------------------------------------------------------------------
// 
// Private Routines
//
//------------------------------------------------------------------------


//
// update the bounding box with latest
//
void
XiliRoiConvexRegion::updateBoundingBox(double extent_x1,
                                       double extent_y1,
                                       double extent_x2,
                                       double extent_y2)
{
    //
    // bbox is an XiliRectDbl - which can be accessed
    // both as integers or doubles 
    //
    double box_x1;
    double box_x2;
    double box_y1;
    double box_y2;

    if(bbox.isEmpty()) {
        bbox.set(extent_x1,extent_y1,extent_x2,extent_y2);
        return;
    }
    bbox.get(&box_x1, &box_y1, &box_x2, &box_y2);
    
    if(box_x1 > extent_x1) {
        box_x1 = extent_x1;
    }
    if(box_x2 < extent_x2) {
        box_x2 = extent_x2;
    }
    if(box_y1 > extent_y1) {
        box_y1 = extent_y1;
    }
    if(box_y2 < extent_y2) {
        box_y2 = extent_y2;
    }
    bbox.set(box_x1,box_y1, box_x2, box_y2);
}

XiliRoiConvexRegion::XiliRoiConvexRegion(XilRoi* calling_roi)
{
    myRoi         = calling_roi;
    num_regions   = 0;
    
    //
    // simpleRegions is TRUE until we rotate or do some operation to turn
    // rectangle into shapes that would have to be scan-converted back to a rectlist
    //
    simpleRegions = TRUE;

    valid         = FALSE;
}

XiliRoiConvexRegion::~XiliRoiConvexRegion()
{
    if(num_regions > 1) {
	//
	// We need to delete the regions appended into the
	// list
	//
	XiliListPosition posn;

	while((posn = regionList.head()) != _XILI_LIST_INVALID_POSITION) {
	    XiliConvexRegion* current_region = regionList.reference(posn);

	    delete current_region;
	    regionList.remove(posn);
	}
    }
}

XilStatus
XiliRoiConvexRegion::clear()
{
    bbox.set(0, 0, 0, 0);
    regionList.emptyList();
    num_regions = 0;
    simpleRegions = TRUE;
    valid = FALSE;
    return XIL_SUCCESS;
}

const XiliRoiConvexRegion&
XiliRoiConvexRegion::operator =(XiliRoiConvexRegion& from)
{
    //
    // Set the bounding box
    //
    bbox        = from.bbox;

    //
    //  Copy other variables.
    //
    num_regions = from.num_regions;
    myRoi       = from.myRoi;
    valid       = from.valid;


    //
    // Copy the single region
    //
    region      = from.region;

    //
    //  Copy the region list.
    //
    regionList  = from.regionList;
    
    return *this;
}

unsigned int
XiliRoiConvexRegion::getNumRegions()
{
    return num_regions;
}

void
XiliRoiConvexRegion::setCallingRoi(XilRoi* top_roi)
{
    myRoi = top_roi;
}

void
XiliRoiConvexRegion::setValid(Xil_boolean valid_flag)
{
    valid = valid_flag;
}

Xil_boolean
XiliRoiConvexRegion::isValid() const
{
    return valid;
}

XiliList<XiliConvexRegion>*
XiliRoiConvexRegion::getRegionList() 
{
    return &regionList;
}

XiliConvexRegion*
XiliRoiConvexRegion::getRegion()
{
    return &region;
}

XilStatus
XiliRoiConvexRegion::translateRectList(XiliRoiRect* rl)
{
    XiliRectInt* next_rect;
    int          num_rects;

    //
    //       If convexregionlist already exists, clear it out.
    //
    if(regionList.isEmpty() == FALSE) {
        regionList.emptyList();
    }

    num_rects = rl->numRects();
    next_rect = rl->getRectList();
    if(num_rects == 1) {
        //
        // Simple case - create one 4-pt convex region
        //
        num_regions = 1;
	region.set(next_rect);
        bbox.set(next_rect);
    } else {
        //
        // Potentially complex case of multiple convex regions
        //
        num_regions = 0;
        while(next_rect != NULL) {
            //
            // Use convenience constructor for rectangle (4-pt) region
            //
            XiliConvexRegion* new_region = new XiliConvexRegion(next_rect);
            regionList.append(new_region);
            //
            // Turn the rect's pixel values into pixel extent doubles
            //
            updateBoundingBox((double)(next_rect->getX1()),
                              (double)(next_rect->getY1()),
                              (double)(next_rect->getX2()),
                              (double)(next_rect->getY2()));
            next_rect = (XiliRectInt*)next_rect->getNext();
	    num_regions++;
        }
    }
    valid = TRUE;
    return XIL_SUCCESS;
}
XilStatus
XiliRoiConvexRegion::translateBitmask(XiliRoiBitmask*)
{
    //
    // TODO: dtb 3/26/96, implement a real bitmask to
    // CR convertor, this just gets us kick started.

    //
    //  Translate the bitmask to a rectlist, then translate
    //  that into a convex region.
    //
    return XIL_FAILURE;
}

//
//  Even though addRect is taking in double values,
//  it is assumed to be coming from a rectangle
//
XilStatus
XiliRoiConvexRegion::addRect(double x,
			     double y,
			     double width,
			     double height)
{
    XiliConvexRegion tmp_region(x,
                                y,
				x+width-1,
                                y+height-1);

    addConvexRegion(&tmp_region);

    return XIL_SUCCESS;
}


XilStatus
XiliRoiConvexRegion::subtractRect(int /*x*/,
				  int /*y*/,
				  unsigned int /*width*/,
				  unsigned int /*height*/)
{
    return XIL_FAILURE;
}

XiliRect*
XiliRoiConvexRegion::getBbox()
{
    return &bbox;
}

XilStatus
XiliRoiConvexRegion::getCopyRoiConvexRegion(XiliRoiConvexRegion* copy)
{
    if(copy == NULL) {
	return XIL_FAILURE;
    }     

    copy->num_regions   = num_regions;
    copy->bbox          = bbox;
    copy->simpleRegions = simpleRegions;
    copy->myRoi         = myRoi;
	
    if(num_regions == 1) {
	copy->region = region;
	return XIL_SUCCESS;
    }

    //
    // Copy the region list, if there is more than one region
    //
    copy->regionList = regionList;
    
    return XIL_SUCCESS;
}


//
//  Intersect a convex region roi with another convex region roi
//  we need to handle different possibilities depending on the
//  number of regions in the different rois.
//
XilStatus
XiliRoiConvexRegion::intersect(XiliRoiConvexRegion* other_cr,
			       XiliRoiConvexRegion* intersection)
{
    //
    // TODO : dtb 4/15/96 can we ever have 0 regions in either
    //        roi?
    //
    if((num_regions == 1) && (other_cr->num_regions == 1)) {
	//
	// If there is only one region in each Roi just intersect
	// them. Note the intersection can produce more than one
	// convex region.
	//
	XiliConvexRegion new_region;

        if(region.intersect(&other_cr->region, &new_region) == XIL_FAILURE) {
	    return XIL_FAILURE;
	}

        if(new_region.pointCount != 0) {
	    //
	    // Add the new region into the intersected area
	    //
	    intersection->addConvexRegion(&new_region);
	}

	return XIL_SUCCESS;
    } else if((num_regions == 1) || (other_cr->num_regions == 1)) {
	//
	// Set up pointers to the single region and to the list.
	// Once this is done the code is the same.
	//
	XiliList<XiliConvexRegion>* theList;
	XiliConvexRegion*           theRegion;
	
	if(num_regions == 1) {
	    theList = &other_cr->regionList;
	    theRegion = &region;
	} else {
	    theList = &regionList;
	    theRegion = &other_cr->region;
	}
	
	XiliListIterator<XiliConvexRegion> iterator(theList);
	XiliConvexRegion*                  cr;
	while((cr = iterator.getNext()) != _XILI_LIST_INVALID_POSITION) {
            XiliConvexRegion new_region;

            if(theRegion->intersect(cr, &new_region) == XIL_FAILURE) {
                return XIL_FAILURE;
            }

            if(new_region.pointCount != 0) {
                //
                // Add the new region into the intersected area
                //
                intersection->addConvexRegion(&new_region);
            }
	}

	return XIL_SUCCESS;
    } else {
	//
	// Need to loop around both lists and place the results
	// in the intersection roi.
	//
	
	//
	// Loop around the region lists intersecting and adding regions
	// as needed.
	//
	XiliListIterator<XiliConvexRegion> iterator(&regionList);
	XiliConvexRegion*                  cr;

	while((cr = iterator.getNext()) != _XILI_LIST_INVALID_POSITION) {

	    XiliListIterator<XiliConvexRegion> other_iterator(&other_cr->regionList);
	    XiliConvexRegion*                  other_region;

	    while((other_region = other_iterator.getNext()) != _XILI_LIST_INVALID_POSITION) {
                XiliConvexRegion new_region;

                if(cr->intersect(other_region, &new_region) == XIL_FAILURE) {
                    return XIL_FAILURE;
                }

                if(new_region.pointCount != 0) {
                    //
                    // Add the new region into the intersected area
                    //
                    intersection->addConvexRegion(&new_region);
                }
            }
	}
    }

    return XIL_SUCCESS;
}

//
//  Intersect a convex region roi with another convex region roi
//  In this case, the calling ROI and the "other_cr" are in
//  pixel coordinate space, but the roi being created needs
//  to be in extent space. Do the increasse on the fly while
//  intersecting.
//
XilStatus
XiliRoiConvexRegion::extentIntersect(XiliRoiConvexRegion* other_cr,
                                     XiliRoiConvexRegion* intersection)
{
    //
    // Two convex regions on the stack to hold the "extent" equivalent
    // of the calling roiCR and the "other" roiCR to be intersected.
    //
    XiliConvexRegion other_equiv_region;
    XiliConvexRegion this_extent_region;

    if((num_regions == 1) && (other_cr->num_regions == 1)) {
	//
	// If there is only one region in each Roi just intersect
	// them. Note the intersection can produce more than one
	// convex region.
	//
        XiliConvexRegion new_region;

        other_cr->region.extend(&other_equiv_region);
        region.extend(&this_extent_region);
        if(this_extent_region.intersect(&other_equiv_region, &new_region) == XIL_FAILURE) {
            return XIL_FAILURE;
        }

        if(new_region.pointCount != 0) {
            //
            // Add the new region into the intersected area
            //
            intersection->addConvexRegion(&new_region);
        }
	
	return XIL_SUCCESS;
    } else if((num_regions == 1) || (other_cr->num_regions == 1)) {
	//
	// Set up pointers to the single region and to the list.
	// Once this is done the code is the same.
	//
	XiliList<XiliConvexRegion>* theList;
	XiliConvexRegion            theRegion;
	
	if(num_regions == 1) {
	    theList = &other_cr->regionList;
            region.extend(&theRegion);
	} else {
	    theList = &regionList;
            other_cr->region.extend(&theRegion);
	}
	
	XiliListIterator<XiliConvexRegion> iterator(theList);
	XiliConvexRegion*                  cr;
	while((cr = iterator.getNext()) != _XILI_LIST_INVALID_POSITION) {
            XiliConvexRegion new_region;

            cr->extend(&other_equiv_region);
            if(theRegion.intersect(&other_equiv_region, &new_region) == XIL_FAILURE) {
                return XIL_FAILURE;
            }

            if(new_region.pointCount != 0) {
                //
                // Add the new region into the intersected area
                //
                intersection->addConvexRegion(&new_region);
            }
        }

	return XIL_SUCCESS;
    } else {
	//
	// Need to loop around both lists and place the results
	// in the intersection roi.
	//
	
	//
	// Loop around the region lists intersecting and adding regions
	// as needed.
	//
	XiliListIterator<XiliConvexRegion> iterator(&regionList);
	XiliConvexRegion*                  cr;

	while((cr = iterator.getNext()) != _XILI_LIST_INVALID_POSITION) {
	    XiliListIterator<XiliConvexRegion> other_iterator(&other_cr->regionList);
	    XiliConvexRegion*                  other_region;

            cr->extend(&this_extent_region);
	    while((other_region = other_iterator.getNext()) != _XILI_LIST_INVALID_POSITION) {
		XiliConvexRegion new_region;
	    
                other_region->extend(&other_equiv_region);
		if(this_extent_region.intersect(&other_equiv_region,
                                                &new_region) == XIL_FAILURE) {
		    return XIL_FAILURE;
		}

		if(new_region.pointCount != 0) {
		    //
		    // Add the new region into the intersected area
		    //
		    intersection->addConvexRegion(&new_region);
                }
	    }
	}
    }

    return XIL_SUCCESS;
}


//
//  This version of intersect is a simpler case of the previous since it is only
//  intersecting rectangles into the convex region...is this a win?
//
XilStatus
XiliRoiConvexRegion::intersect(XiliRoiRect*         rectlist,
			       XiliRoiConvexRegion* intersection)
{
    XiliRectInt* next_rect;
    int          num_rects;

    num_rects = rectlist->numRects();
    next_rect = rectlist->getRectList();
    
    if((num_regions == 1) && (num_rects == 1)) {
	XiliConvexRegion new_region;
	XiliConvexRegion rect_region(next_rect);

	if(region.intersect(&rect_region, &new_region) == XIL_FAILURE) {
	    return XIL_FAILURE;
	}

        if(new_region.pointCount != 0) {
            //
            // Add the new region into the intersected area
            //
            intersection->addConvexRegion(&new_region);
        }

	return XIL_SUCCESS;
    } else if(num_rects == 1) {
	//
	// Only one rect but many regions
	//	
	XiliListIterator<XiliConvexRegion> iterator(&regionList);
	XiliConvexRegion*                  cr;
	XiliConvexRegion                   rect_region(next_rect);

	while((cr = iterator.getNext()) != _XILI_LIST_INVALID_POSITION) {
            XiliConvexRegion new_region;

            if(cr->intersect(&rect_region, &new_region) == XIL_FAILURE) {
                return XIL_FAILURE;
            }

            if(new_region.pointCount != 0) {
                //
                // Add the new region into the intersected area
                //
                intersection->addConvexRegion(&new_region);
            }
	}

	return XIL_SUCCESS;
    } else if(num_regions == 1) {
	//
	// Only one region but many rects
	//
	while(next_rect != NULL) {
            XiliConvexRegion new_region;
	    XiliConvexRegion rect_region(next_rect);

            if(region.intersect(&rect_region, &new_region) == XIL_FAILURE) {
                return XIL_FAILURE;
            }

            if(new_region.pointCount != 0) {
                //
                // Add the new region into the intersected area
                //
                intersection->addConvexRegion(&new_region);
            }

            next_rect = (XiliRectInt*)next_rect->getNext();
        }

	return XIL_SUCCESS;
    } else {
	//
	// Many rects and many regions
	//
	while(next_rect != NULL) {
	    XiliConvexRegion                   rect_region(next_rect);
	    XiliListIterator<XiliConvexRegion> iterator(&regionList);
	    XiliConvexRegion*                  cr;

	    while((cr = iterator.getNext()) != _XILI_LIST_INVALID_POSITION) {
                XiliConvexRegion new_region;

                if(cr->intersect(&rect_region, &new_region) == XIL_FAILURE) {
                    return XIL_FAILURE;
                }

                if(new_region.pointCount != 0) {
                    //
                    // Add the new region into the intersected area
                    //
                    intersection->addConvexRegion(&new_region);
                }
	    }
	    next_rect = (XiliRectInt*)next_rect->getNext();
	}

	return XIL_SUCCESS;
    }
}

XilStatus
XiliRoiConvexRegion::unite(XiliRoiConvexRegion* /*other_cr*/,
			   XiliRoiConvexRegion* /*unite_cr*/)
{
    return XIL_FAILURE;
}

XilStatus
XiliRoiConvexRegion::translate(double               x,
			       double               y,
			       XiliRoiConvexRegion* copy)
{
    if(copy == NULL) {
	return XIL_FAILURE;
    }

    if(num_regions == 1) {

	//
	//  Only a single convex region in the list
	//
	for(unsigned int i=0; i<region.pointCount; i++) {
	    copy->region.xPtArray[i] = region.xPtArray[i] + x;
	    copy->region.yPtArray[i] = region.yPtArray[i] + y;
	}

        copy->region.pointCount = region.pointCount;

	copy->region.lowX  = region.lowX + x;
	copy->region.lowY  = region.lowY + y;
	copy->region.highX = region.highX + x;
	copy->region.highY = region.highY + y;

        copy->num_regions  = 1;
    } else {
	//
	//  More than one entry
        //
	XiliListIterator<XiliConvexRegion> iterator(&regionList);
	XiliConvexRegion*                  cr;

	while((cr = iterator.getNext()) != _XILI_LIST_INVALID_POSITION) {
	    XiliConvexRegion* copy_cr = new XiliConvexRegion;

	    if(copy_cr == NULL) {
		XIL_ERROR(myRoi->getSystemState(), XIL_ERROR_RESOURCE,"di-1",TRUE);
		return XIL_FAILURE;
	    }

	    for(unsigned int i=0; i<cr->pointCount; i++) {
		copy_cr->xPtArray[i] = cr->xPtArray[i] + x;
		copy_cr->yPtArray[i] = cr->yPtArray[i] + y;
	    }

            copy_cr->pointCount = cr->pointCount;

	    copy_cr->lowX  = cr->lowX + x;
	    copy_cr->lowY  = cr->lowY + y;
	    copy_cr->highX = cr->highX + x;
	    copy_cr->highY = cr->highY + y;

	    copy->regionList.append(copy_cr);
	}

        copy->num_regions = num_regions;
    }

    //
    //  Update the bounding box.
    //
    copy->bbox = bbox;
    copy->bbox.translate(x, y);

    return XIL_SUCCESS;
}

XilStatus
XiliRoiConvexRegion::translate_inplace(double x,
				       double y)
{
    if(num_regions == 1) {
	//
	// Only a single convex region in the list
	//
	for(unsigned int i=0; i<region.pointCount; i++) {
	    region.xPtArray[i] = region.xPtArray[i] + x;
	    region.yPtArray[i] = region.yPtArray[i] + y;
	}

	region.lowX  = region.lowX + x;
	region.lowY  = region.lowY + y;
	region.highX = region.highX + x;
	region.highY = region.highY + y;
    } else {
	//
	// More than one entry
	//
	XiliListIterator<XiliConvexRegion> iterator(&regionList);
	XiliConvexRegion*                  cr;

	while((cr = iterator.getNext()) != _XILI_LIST_INVALID_POSITION) {
	    for(unsigned int i=0; i<cr->pointCount; i++) {
		cr->xPtArray[i] += x;
		cr->yPtArray[i] += y;
	    }

	    cr->lowX  += x;
	    cr->lowY  += y;
	    cr->highX += x;
	    cr->highY += y;
	}
    }

    //
    // Update the bounding box
    //
    bbox.translate(x, y);

    return XIL_SUCCESS;
}

Xil_boolean
XiliRoiConvexRegion::isSimple()
{
    return simpleRegions;
}

XilStatus
XiliRoiConvexRegion::addConvexRegion(XiliConvexRegion* cr)
{
    //
    // If there are no entries on the list add a single
    // convex region
    //
    if(num_regions == 0) {
	region = *cr;
	num_regions = 1;

        //
        // Update the bounding box. 
        //
	bbox.set(cr->lowX,cr->lowY,
                 cr->highX,cr->highY);

        return XIL_SUCCESS;
    }

    //
    // Ok we have a list entry need to go to full
    // support.
    //
    if(num_regions == 1) {
	XiliConvexRegion*          first_region;

	first_region = new XiliConvexRegion(region.pointCount,
					    region.xPtArray,
					    region.yPtArray);
	if(first_region == NULL) {
	    XIL_ERROR(myRoi->getSystemState(), XIL_ERROR_RESOURCE,"di-1",TRUE);
	    return XIL_FAILURE;
	}

	// Append the first region to the list
	regionList.append(first_region);
    }
    
    //
    //  TODO: 9/26/96 jlf  Try and make the addConvexRegion() at least be a
    //                     semblence of a working routine.  For now, just add
    //                     the convex region to our list and they'll overlap
    //                     but at least they won't exponentially explode like
    //                     the code below does.  
    //
    XiliConvexRegion* new_region;

    new_region = new XiliConvexRegion(cr);
    if(new_region == NULL) {
        XIL_ERROR(myRoi->getSystemState(), XIL_ERROR_RESOURCE,"di-1",TRUE);
        return XIL_FAILURE;
    }

    //
    //  Append the region to the list
    //
    regionList.append(new_region);

    updateBoundingBox(new_region->lowX,
                      new_region->lowY,
                      new_region->highX,
                      new_region->highY);
    num_regions++;


#if 0
    //
    //  Now add the incoming convex region to the list, loop
    //  over the list and intersect it with the existing list.
    //
    //  So that we have no overlapping regions.
    //
    XiliListIterator<XiliConvexRegion> iterator(&regionList);
    XiliConvexRegion*                  current_cr;

    fprintf(stderr, "adding convex region-- num_regions = %d\n", num_regions);

    while((current_cr = iterator.getNext()) != _XILI_LIST_INVALID_POSITION) {
	XiliConvexRegion tmp1, tmp2, tmp3;
	unsigned int     new_regions;
	unsigned int     regions_added = 0;
	
	current_cr->intersect(cr, &tmp1, &tmp2, &tmp3, &new_regions);

        fprintf(stderr, "--> new_regions = %d\n", new_regions);

	switch(new_regions) {
	  case 0: {
	    //
	    // Regions didn't intersect copy and add the entry to the
	    // list.
	    //
	    XiliConvexRegion*          cregion;
	    
	    cregion = new XiliConvexRegion(cr->pointCount,
					   cr->xPtArray,
					   cr->yPtArray);
	    if(cregion == NULL) {
		XIL_ERROR(myRoi->getSystemState(), XIL_ERROR_RESOURCE,"di-1",TRUE);
		return XIL_FAILURE;
	    }

	    //
	    // Insert the new region into the list, update region count and
	    // bounding box
	    //
	    if(regionList.insertAfter(cregion, iterator.getCurrentPosition()) ==
	       _XILI_LIST_INVALID_POSITION) {
		// Things are messed up insertion failed
		delete cregion;
		return XIL_FAILURE;
	    }
	    updateBoundingBox(cregion->lowX,
			      cregion->lowY,
			      cregion->highX,
			      cregion->highY);
	    num_regions++;
	    regions_added = 1;
	    break;
	  }
	  
	  case 1: {
	    //
	    // Intersection didn't create any more regions, update the
	    // current entry with the new entry.
	    //
	    *current_cr = tmp1;
	    updateBoundingBox(tmp1.lowX,
			      tmp1.lowY,
			      tmp1.highX,
			      tmp1.highY);
	    regions_added = 0;
	    break;
	  }
	  
	  case 2: {
	  case 3:
	    //
	    // We created new regions and need to adjust the
	    // current entry.
	    //
	    *current_cr = tmp1;
	    updateBoundingBox(tmp1.lowX,
			      tmp1.lowY,
			      tmp1.highX,
			      tmp1.highY);

	    //
	    // Add new entries
	    //
	    XiliConvexRegion*          cregion;
	    cregion = new XiliConvexRegion(tmp2.pointCount,
					   tmp2.xPtArray,
					   tmp2.yPtArray);
	    if(cregion == NULL) {
		XIL_ERROR(myRoi->getSystemState(), XIL_ERROR_RESOURCE,"di-1",TRUE);
		return XIL_FAILURE;
	    }

	    //
	    // Insert the new region into the list, update region count and
	    // bounding box
	    //
	    if(regionList.insertAfter(cregion, iterator.getCurrentPosition()) ==
	       _XILI_LIST_INVALID_POSITION) {
		// Things are messed up insertion failed
		delete cregion;
		return XIL_FAILURE;
	    }
	    updateBoundingBox(tmp2.lowX,
			      tmp2.lowY,
			      tmp2.highX,
			      tmp2.highY);
	    num_regions++;
	    regions_added = 1;

	    if(new_regions == 3) {
		XiliConvexRegion*          cregion1;
		
		cregion1 = new XiliConvexRegion(tmp3.pointCount,
						tmp3.xPtArray,
						tmp3.yPtArray);
		if(cregion1 == NULL) {
		    XIL_ERROR(myRoi->getSystemState(), XIL_ERROR_RESOURCE,"di-1",TRUE);
		    delete cregion;
		    return XIL_FAILURE;
		}

		//
		// Insert the new region into the list, update region count and
		// bounding box
		//
		if(regionList.insertAfter(cregion1, iterator.getCurrentPosition()) ==
		   _XILI_LIST_INVALID_POSITION) {
		    // Things are messed up insertion failed
		    delete cregion;
		    delete cregion1;
		    return XIL_FAILURE;
		}

		updateBoundingBox(tmp3.lowX,
				  tmp3.lowY,
				  tmp3.highX,
				  tmp3.highY);
		
		num_regions++;
		regions_added = 2;
	    }
	    break;
	  }
	}

	//
	// Skip over any new regions we just added
	//
	for(int i=0; i<regions_added; i++) {
	    if((current_cr = iterator.getNext()) == _XILI_LIST_INVALID_POSITION) {
		//
                // this shouldn't fail if we inserted correctly
                //
                return XIL_FAILURE;
            }
	}
    }
#endif

    return XIL_SUCCESS;
}

