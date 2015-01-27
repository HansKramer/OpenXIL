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
//  File:	XilConvexRegionList.cc
//  Project:	XIL
//  Revision:	1.36
//  Last Mod:	10:08:44, 03/10/00
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
//  MT-level: SAFE
//  Note that the XilRoi that the ConvexRegionList stems from is inherently
//  MT-unsafe. A ROI contains 3 implementations of a region (convex region,
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
#pragma ident	"@(#)XilConvexRegionList.cc	1.36\t00/03/10  "

#include "_XilDefines.h"
#include "_XilConvexRegionList.hh"
#include "_XilSystemState.hh"
#include "_XilRoi.hh"

//
// Constructor, takes a roi and a box, list is returned
// relative to the box, ie the regions are translated to
// always start at 0, 0
//
XilConvexRegionList::XilConvexRegionList(XilRoi* roi,
					 XilBox* dest_box)
{
    //
    //  Keep a version of the system state for error generation
    //
    stateptr = roi->getSystemState();

    //
    // Need to keep a copy of the roi pointer in case of a reinit
    //
    roiptr = roi;

    if(roi->isConvexRegionValid()) {
        if(dest_box == NULL) {
            //
            // Not clipping, therefore use reference to full list
            //
            if((regionRef = roi->getConvexRegion()) == NULL) {
                regionListRef = roi->getConvexRegionList();
            } else {
                regionListRef = NULL;
            }
            copyFlag = FALSE;
        } else {
            copyFlag = TRUE;
            if(generateRegionList(roi, dest_box, TRUE) == XIL_FAILURE) {
                //
                // Something bad happened generating the copy list
                // clear everything out of the list
                //
                delete regionRef;
                regionRef = NULL;
                delete regionListRef;
                copyFlag = FALSE;
            }
        }
    } else {
        if(roi->isRectValid() == TRUE) {
            //
            // This roi exists as a rectlist. We cannot let the
            // ROI update itself internally because it isn't mt-safe.
            // We'll have to get the rectlist and generate our
            // clipped convex region list on the fly.
            //
            
            //
            // This routine will fill regionListRef if there is
            // more than one rectangle in the list, and set
            // regionRef if there is only one.
            // This will return failure if there's a problem,
            // but there's not much a constructor can do except cleanup.
            //

            //
            // Get a reference to the rectlist 
            //
            if(createAndClipConvexRegionList(dest_box, TRUE) == XIL_FAILURE) {
                delete regionRef;
                regionRef = NULL;
                delete regionListRef;
                copyFlag = FALSE;
                return;
            } 
            copyFlag = TRUE;
        } else {
            //
            //  Since we currently only support rectlist and convex region ROi's
            //  If we get there then the ROI is empty.
            //
            regionRef = NULL;
            regionListRef = NULL;
            copyFlag = FALSE;
        }
    }
}


//
// Construct a convex region list and intersect with the
// area defined by x, y, w, h. The regions are not translated.
//
XilConvexRegionList::XilConvexRegionList(XilRoi*      roi,
					 int          x,
					 int          y,
					 unsigned int xsize,
					 unsigned int ysize)
{
    //
    //  Keep a version of the system state for error generation
    //
    stateptr = roi->getSystemState();

    //
    // Need to keep a copy of the roi pointer in case of a reinit
    //
    roiptr = roi;

    copyFlag = TRUE;

    //
    // Construct a tmp box and call the generateRegionList roi box
    // method but tell it not to do the translation
    //
    XilBox tmp_box(x, y, xsize, ysize);
    if(roi->isConvexRegionValid()) {
        if(generateRegionList(roi, &tmp_box, FALSE) == XIL_FAILURE) {
            //
            // Something bad happened generating the copy list
            // clear everything out of the list
            //
            delete regionRef;
            regionRef = NULL;
            delete regionListRef;
            copyFlag = FALSE;
        }
    } else {
        if(roi->isRectValid() == TRUE) {
            //
            // This roi exists as a rectlist. We cannot let the
            // ROI update itself internally because it isn't mt-safe.
            // We'll have to get the rectlist and generate our
            // clipped convex region list on the fly.
            //
            //
            // This routine will fill regionListRef if there is
            // more than one rectangle in the list, and set
            // regionRef if there is only one.
            // This will return failure if there's a problem,
            // but there's not much a constructor can do except cleanup.
            //
            if(createAndClipConvexRegionList(&tmp_box,
                                             FALSE) == XIL_FAILURE) {
                delete regionRef;
                regionRef = NULL;
                delete regionListRef;
                copyFlag = FALSE;
            }
        } else {
            //
            //  Since we currently only support rect and convex region ROI's
            //  This means that the ROI is empty.
            //
            regionRef = NULL;
            regionListRef = NULL;
            copyFlag = FALSE;
        }
    }
}

#ifdef _XIL_HAS_LIBDGA

//
// For DGA_Y_EOL, DGA_X_EOL
//
#include <dga/dga.h>

//
// The following constructor is only intended for use with IO devices
// which may need to clip with a DGA cliplist.
//
XilConvexRegionList::XilConvexRegionList(XilRoi* roi,
					 short*  cliplist,
					 int     winX,
					 int     winY)
{
    //
    //  Keep a version of the system state for error generation
    //
    stateptr = roi->getSystemState();

    //
    // Need to keep a copy of the roi pointer in case of a reinit
    //
    roiptr = roi;

    //
    // Always uses a list so just make this NULL
    //
    regionRef = NULL;
    regionRefFirst = FALSE;
    regionListRef = NULL;

    //
    // Do the real work
    //
    copyFlag = TRUE;
    if(roi->isConvexRegionValid()) {
        if(generateRegionList(roi, cliplist,
                              winX, winY, FALSE) == XIL_FAILURE) {
            //
            // Something bad happened generating the copy list
            // clear everything out of the list
            //
            delete regionListRef;
            copyFlag = FALSE;
        }
    } else {
        //
        // This roi exists as a rectlist. We cannot let the
        // ROI update itself internally because it isn't mt-safe.
        // We'll have to get the rectlist and generate our
        // clipped convex region list on the fly.
        //
        if(roi->isRectValid() == TRUE) {
            //
            // This is so unlikely to happen that I'm going to
            // take the easy, bad-performance route.
            // Simply make a copy of the ROI and call the
            // generateRegionList routine, then destroy the
            // roi copy.
            //
            XilRoi* tmp_roi = (XilRoi*)roi->createCopy();
            if(tmp_roi == NULL) {
                XIL_ERROR(stateptr, XIL_ERROR_RESOURCE, "di-1", TRUE);
                return;
            }
            if(generateRegionList(tmp_roi, cliplist,
                                  winX, winY, FALSE) == XIL_FAILURE) {
                //
                // Something bad happened generating the copy list
                // clear everything out of the list
                //
                delete regionListRef;
                copyFlag = FALSE;
                
            }
            tmp_roi->destroy();
        } else {
            //
            //  Since we currently only support rect and convex region ROI's
            //  This means that the ROI is empty.
            //
            regionRef = NULL;
            regionListRef = NULL;
            copyFlag = FALSE;
        }
    }
}

#endif // _XIL_HAS_LIBDGA

//
// Constructs a convex region list from a previous convex region list
// and a box. Regions are translated to have 0, 0 at the box.
//
XilConvexRegionList::XilConvexRegionList(XilConvexRegionList* crlist,
					 XilBox*              dest_box)
{
    //
    //  Keep a version of the system state for error generation
    //
    stateptr = crlist->stateptr;

    //
    // Need to keep a copy of the roi pointer in case of a reinit
    //
    roiptr = crlist->roiptr;
    
    if((crlist->regionListRef == NULL) && (crlist->regionRef == NULL)) {
        //
        //  crlist is empty
        //
        regionRef = NULL;
        regionListRef = NULL;
        copyFlag = FALSE;
        return;
    }

    if(dest_box == NULL) {
        //
        // Not clipping, therefore use reference to full list
        //
	if(crlist->regionListRef) {
	    regionListRef = crlist->regionListRef;
	} else if(crlist->regionRef) {
	    regionRef = crlist->regionRef;
	    regionRefFirst = TRUE;
	}
	copyFlag = FALSE;
    } else {
	copyFlag = TRUE;
	if(generateRegionList(crlist, dest_box, TRUE) == XIL_FAILURE) {
	    //
	    // Something bad happened generating the copy list
	    // clear everything out of the list
	    //
	    delete regionListRef;
	    delete regionRef;
	    regionRef = NULL;
	    copyFlag = FALSE;
	}
    }
}

//
// Constructs a convex region list from a previous convex region list
// and a box. Regions are translated to have 0, 0 at the box.
//
XilConvexRegionList::XilConvexRegionList(XilConvexRegionList* crlist,
					 int                  x1,
					 int                  y1,
					 int                  x2,
					 int                  y2)
{
    //
    //  Keep a version of the system state for error generation
    //
    stateptr = crlist->stateptr;

    //
    // Need to keep a copy of the roi pointer in case of a reinit
    //
    roiptr = crlist->roiptr;
    
    if((crlist->regionListRef == NULL) && (crlist->regionRef == NULL)) {
        //
        //  crlist is empty
        //
        regionRef = NULL;
        regionListRef = NULL;
        copyFlag = FALSE;
        return;
    }

    copyFlag = TRUE;

    //
    // A tmp box, allows us to use the same code from
    // this and the other crlist constructor
    //
    XilBox tmp_box(x1, y1, x2, y2);
    
    if(generateRegionList(crlist, &tmp_box, FALSE) == XIL_FAILURE) {
	//
	// Something bad happened generating the copy list
	// clear everything out of the list
	//
	delete regionListRef;
	delete regionRef;
	regionRef = NULL;
	copyFlag = FALSE;
    }
}

//
// Destructor
//
XilConvexRegionList::~XilConvexRegionList()
{
    //
    // Delete stuff if we made a copy
    //
    if(copyFlag == TRUE) {
	
	//
	// Delete each of the entries in the list,
	// then delete the container. Test for regionListRef
	// means we make sure it exists before deleting the
	// contents of the list.
	//
	if(regionListRef) {
	    XiliListPosition posn;

	    while((posn = regionListRef->head()) != _XILI_LIST_INVALID_POSITION) {
		XiliConvexRegion* region = regionListRef->reference(posn);

		delete region;
		regionListRef->remove(posn);
	    }
	    delete regionListRef;
	}
	
	//
	// We can always delete this entry
	//
	delete regionRef;
    }
}

//
// Get the next region in the list
//
Xil_boolean
XilConvexRegionList::getNext(const double** x_array,
			     const double** y_array,
			     unsigned int*  point_count)
{
    if(regionRef) {
	//
	// Its only a single region return it once
	// and then return FALSE
	//
	if(regionRefFirst == TRUE) {
	    *x_array     = regionRef->getXPtArray();
	    *y_array     = regionRef->getYPtArray();
	    *point_count = regionRef->getPointCount();

	    regionRefFirst = FALSE;
	    return TRUE;
	} else {
	    return FALSE;
	}
    } else if(regionListRef) {
	//
	// List is handled by starting at the head of the
	// list and moving through it by tracking the current
	// position. If we are on the tail of the list
	//
	if(currentPosition == _XILI_LIST_INVALID_POSITION) {
	    return FALSE;
	} else {
	    XiliConvexRegion* current_region = regionListRef->reference(currentPosition);
	    
	    *x_array     = current_region->getXPtArray();
	    *y_array     = current_region->getYPtArray();
	    *point_count = current_region->getPointCount();

	    currentPosition = regionListRef->next(currentPosition);
	    return TRUE;
	}
    } else {
	//
	//  Empty convex region list
	//
	return FALSE;
    }
}

//
//  Copy the roi contents and clip against the destination box
//  This is never called with an empty ROI, so I don't have to check
//  inside the routine.
//
XilStatus
XilConvexRegionList::generateRegionList(XilRoi*     roi,
					XilBox*     dest_box,
					Xil_boolean translate_flag)
{
    int x1, y1, x2, y2;
    dest_box->getAsCorners(&x1, &y1, &x2, &y2);

    //
    // Create the region we will intersect against, we
    // do this by getting the convex region we generated
    // when doing the forward mapping from src to destination
    //
    XiliConvexRegion* box_region = (XiliConvexRegion*)dest_box->getPrivateData();
    XiliConvexRegion  intersect;
    XiliConvexRegion* inRegion;
    
    Xil_boolean isRegion_set = FALSE;

    if(box_region == NULL) {
	//
	// Never set anything on the destination box, didn't have
	// tiles or didn't call splitOnTileBoundaries.
        // Set from rect will turn the pixel coordinates of the
        // values into the region's pixel extent values.
	//
	intersect.set(x1, y1, x2, y2);
    } else {
	//
	// There was a convex region associated with the box
	//
	intersect = *box_region;

        isRegion_set = TRUE;
    }

    //
    // Now intersect with the roi to get a list of
    // areas to be processed by the compute routine
    //
    if((inRegion = roi->getConvexRegion()) == NULL) {
	//
	// Get a pointer to the region list
	//
	XiliList<XiliConvexRegion>* inRegionList = roi->getConvexRegionList();

	regionListRef = new XiliList<XiliConvexRegion>;
	if(regionListRef == NULL) {
	    XIL_ERROR(stateptr, XIL_ERROR_RESOURCE, "di-1", TRUE);
	    return XIL_FAILURE;
	}

	//
	// Loop the regions in the roi and intersect with
	// the destination box
	//
	XiliConvexRegion*                  current_region;
	XiliListIterator<XiliConvexRegion> crl_iterator(inRegionList);
	
	while((current_region = crl_iterator.getNext()) != NULL) {
	    if(regionListIntersect(current_region,
                                   &intersect,
                                   translate_flag,
				   (float) x1, (float) y1) == XIL_FAILURE) {
		return XIL_FAILURE;
	    }
	}
	currentPosition = regionListRef->head();

        regionRef = NULL;

	return XIL_SUCCESS;
    } else {
	//
	// We got a single region from the incoming convex region list
	//
	regionListRef = NULL;

	regionRef = new XiliConvexRegion;
	if(regionRef == NULL) {
	    XIL_ERROR(stateptr, XIL_ERROR_RESOURCE, "di-1", TRUE);
	    return XIL_FAILURE;
	}

        return regionRefIntersect(inRegion,
                                  &intersect,
                                  translate_flag,
                                  (float) x1, (float) y1);
    }
}

#ifdef _XIL_HAS_LIBDGA
//
// Copy the roi contents and clip against the DGA cliplist
// We do this by holding the roi constant and intersecting
// against the cliplist. This means we don't have to do
// multiple checks for a single region.
//
// Ignores the translate flag, doesn't do the translation.
//  This is never called with an empty ROI, so I don't have to check
//  inside the routine.
//
XilStatus
XilConvexRegionList::generateRegionList(XilRoi*     roi,
					short*      cliplist,
					int         winX,
					int         winY,
					Xil_boolean )
{
    XiliConvexRegion* inRegion;
    
    //
    // Does the roi contain a single convex region.
    //
    if((inRegion = roi->getConvexRegion()) == NULL) {
	//
	// Get a pointer to the region list
	//
	XiliList<XiliConvexRegion>* inRegionList = roi->getConvexRegionList();;

	regionListRef = new XiliList<XiliConvexRegion>;
	if(regionListRef == NULL) {
	    XIL_ERROR(stateptr, XIL_ERROR_RESOURCE, "di-1", TRUE);
	    return XIL_FAILURE;
	}
	
	//
	// Loop the regions in the roi and intersect with
	// the DGA clip list
	//
	XiliConvexRegion*                  current_region;
	XiliListIterator<XiliConvexRegion> crl_iterator(inRegionList);
		
	while((current_region = crl_iterator.getNext()) != NULL) {
	    //
	    // Loop over the cliplist intersecting it with
	    // the roi
	    //
	    short  cx1;
	    short  cy1;
	    short  cx2;
	    short  cy2;
	    short* cptr = cliplist;
    
	    while((cy1 = *cptr++) != DGA_Y_EOL) {
		cy2 = *cptr++;
		while((cx1 = *cptr++) != DGA_X_EOL) {
		    cx2 = *cptr++;

		    //
		    // Create the region we will intersect against using
		    // the current entry in the cliplist
		    //
                    XiliConvexRegion intersect(cx1 - winX,
                                               cy1 - winY,
                                               cx2 - winX - 1,
                                               cy2 - winY - 1);

		    if(regionListIntersect(current_region,
					   &intersect,
                                           FALSE,
                                           0.0F,
                                           0.0F) == XIL_FAILURE) {
			return XIL_FAILURE;
		    }
		}
	    }
	}
    } else {
	//
	// Loop over the cliplist intersecting it with
	// the single region in the roi
	//
	short       cx1;
	short       cy1;
	short       cx2;
	short       cy2;
	short*      cptr = cliplist;

	while((cy1 = *cptr++) != DGA_Y_EOL) {
	    cy2 = *cptr++;
	    while((cx1 = *cptr++) != DGA_X_EOL) {
		cx2 = *cptr++;

		if(regionListRef == NULL) {
		    //
		    // Always use a list, its easier than having to special
		    // case the one region, one cliplist entry and probably not
		    // that much more overhead.
		    //
		    regionListRef = new XiliList<XiliConvexRegion>;
		    if(regionListRef == NULL) {
			XIL_ERROR(stateptr, XIL_ERROR_RESOURCE, "di-1", TRUE);
			return XIL_FAILURE;
		    }
		}

		//
		// Create the region we will intersect against using
		// the current entry in the cliplist
		//
		XiliConvexRegion intersect(cx1 - winX,
                                           cy1 - winY,
                                           cx2 - winX - 1,
                                           cy2 - winY - 1);

		if(regionListIntersect(inRegion,
				       &intersect,
                                       FALSE,
                                       0.0F,
                                       0.0F) == XIL_FAILURE) {
		    return XIL_FAILURE;
		}
	    }
	}
    }

    //
    //  If the incoming cliplist is empty, the list may never be created.
    //
    if(regionListRef == NULL) {
        currentPosition = _XILI_LIST_INVALID_POSITION;
    } else {
        currentPosition = regionListRef->head();
    }

    return XIL_SUCCESS;
}
#endif // _XIL_HAS_LIBDGA

//
//  Takes an existing XilConvexRegionList and clips it against
//  the destination box to produce a new convex region list
//  This is never called with an empty ROI, so I don't have to check
//  inside the routine.
//
XilStatus
XilConvexRegionList::generateRegionList(XilConvexRegionList* crlist,
					XilBox*              dest_box,
					Xil_boolean          translate_flag)
{
    int x1, y1, x2, y2;
    dest_box->getAsCorners(&x1, &y1, &x2, &y2);

    //
    //  Get the region we will intersect against.  We do this by getting the
    //  convex region we generated when doing the forward mapping from src to
    //  destination (i.e. whatever may be set on the box).
    //
    XiliConvexRegion* box_region =
        (XiliConvexRegion*)dest_box->getPrivateData();
    XiliConvexRegion  intersect;

    if(box_region == NULL) {
	//
	// Never set anything on the destination box, didn't have
	// tiles or didn't call splitOnTileBoundaries.
        // Set from rect will turn the pixel coordinates of the
        // values into the region's pixel extent values.
	//
	intersect.set(x1, y1, x2, y2);
    } else {
	//
	// There was a convex region associated with the box
	//
	intersect = *box_region;
    }

    //
    // Now intersect with the roi to get a list of
    // areas to be processed by the compute routine
    //
    if(crlist->regionListRef) {
	//
	// Get a copy of the region list
	//
	regionListRef = new XiliList<XiliConvexRegion>;
	if(regionListRef == NULL) {
	    XIL_ERROR(stateptr, XIL_ERROR_RESOURCE, "di-1", TRUE);
	    return XIL_FAILURE;
	}
	
	//
	// Loop over the regions in the incoming list and intersect with
	// the destination box
	//
	XiliConvexRegion*                  current_region;
	XiliListIterator<XiliConvexRegion> crl_iterator(crlist->regionListRef);
	
	while((current_region = crl_iterator.getNext()) != NULL) {
	    if(regionListIntersect(current_region,
                                   &intersect,
                                   translate_flag,
				   (float) x1, (float) y1) == XIL_FAILURE) {
		return XIL_FAILURE;
	    }
	}

	currentPosition = regionListRef->head();
	regionRef = NULL;
	regionRefFirst = FALSE;
    } else if (crlist->regionRef) {
	//
	// We got a single region from the incoming convex region list
	//
	regionListRef = NULL;

	regionRef = new XiliConvexRegion;
	if(regionRef == NULL) {
	    XIL_ERROR(stateptr, XIL_ERROR_RESOURCE, "di-1", TRUE);
	    return XIL_FAILURE;
	}

	return regionRefIntersect(crlist->regionRef,
                                  &intersect,
                                  translate_flag,
				  (float) x1, (float) y1);
    } else {
        //
        // If the incoming list had no region list or single
        // region something was wrong with the incoming list
        // return XIL_FAILURE
        //
    }

    return XIL_SUCCESS;
}

//
// Allows the original full ROI  to be clipped by a different box.
// In order to do this we effectively call the destructor and
// then reconstruct the lists with the original roi and new box.
//
XilStatus
XilConvexRegionList::reinit(XilBox* dest_box)
{
    //
    // Destructor code....
    //
    if(copyFlag == TRUE) {
	
	//
	// Delete each of the entries in the list,
	// then delete the container. Test for regionListRef
	// means we make sure it exists before deleting the
	// contents of the list.
	//
	if(regionListRef) {
	    XiliListPosition posn;

	    while((posn = regionListRef->head()) !=
                  _XILI_LIST_INVALID_POSITION) {
		XiliConvexRegion* region = regionListRef->reference(posn);

		delete region;
		regionListRef->remove(posn);
	    }
	    delete regionListRef;
	}
	
	//
	// We can always delete this entry
	//
	delete regionRef;
    }

    //
    // Now construct with the new box and original roi which
    // we saved a pointer to.
    //
    if(roiptr->isValid()) {
        //
        //  Only the if ROI isn't empty
        //
        if(dest_box == NULL) {
            //
            // Not clipping, therefore use reference to full list
            //
            if((regionRef = roiptr->getConvexRegion()) == NULL) {
                regionListRef = roiptr->getConvexRegionList();
            } else {
                regionListRef = NULL;
            }
            copyFlag = FALSE;
        } else {
            copyFlag = TRUE;
            if(generateRegionList(roiptr, dest_box, TRUE) == XIL_FAILURE) {
                //
                // Something bad happened generating the copy list
                // clear everything out of the list
                //
                delete regionRef;
                delete regionListRef;
                copyFlag = FALSE;
                return XIL_FAILURE;
            }
        }
    }        
    return XIL_SUCCESS;
}

//
// Utility code to deal with the intersection of the current
// region and the intersect region and add the appropriate entries
// to the regionList. Assumes that the regionListRef has been created.
//
XilStatus
XilConvexRegionList::regionListIntersect(XiliConvexRegion* current_region,
					 XiliConvexRegion* intersect,
					 Xil_boolean       translate_flag,
					 float             x1,
					 float             y1)
{
    XiliConvexRegion new_region;

    //
    //  We call intersect->intersect() instead of current_region->intersect()
    //  because the convex region intersection code can work better when the
    //  smaller convex region is being intersected into the bigger convex
    //  region.  A common case is intersect being a small box and
    //  current_region (the ROI) being the whole image.
    //
    if(intersect->intersect(current_region, &new_region) == XIL_FAILURE) {
        return XIL_FAILURE;
    }

    if(new_region.pointCount != 0) {
        //
        // Add the new region to the intersection
        //
        // Create a copy of the region and append to the list
        //
        XiliConvexRegion* list_cr = new XiliConvexRegion(&new_region);
        if(list_cr == NULL) {
            XIL_ERROR(stateptr, XIL_ERROR_RESOURCE, "di-1", TRUE);
            return XIL_FAILURE;
        }

        //
        //  Do we need to translate the region?
        //
        if(translate_flag == TRUE) {
            list_cr->translate(-x1, -y1);
        }

        regionListRef->append(list_cr);
    }

    return XIL_SUCCESS;
}

//
// Utility code to deal with the intersection of the current
// region and the intersect region and copy the entries into
// the single regionRef. Assumes that the regionRef is valid
//
XilStatus
XilConvexRegionList::regionRefIntersect(XiliConvexRegion* current_region,
					XiliConvexRegion* intersect,
					Xil_boolean       translate_flag,
					float             x1,
					float             y1)
{
    XiliConvexRegion new_region;

    //
    //  We call intersect->intersect() instead of current_region->intersect()
    //  because the convex region intersection code can work better when the
    //  smaller convex region is being intersected into the bigger convex
    //  region.  A common case is intersect being a small box and
    //  current_region (the ROI) being the whole image.
    //
    if(intersect->intersect(current_region, &new_region) == XIL_FAILURE) {
        return XIL_FAILURE;
    }

    if(new_region.pointCount != 0) {
	//
	// Clipped ok no new regions created,
	//
	*regionRef = new_region;

	if(translate_flag == TRUE) {
	    regionRef->translate(-x1, -y1);
	}

	regionRefFirst = TRUE;
    }

    return XIL_SUCCESS;
}


XilStatus
XilConvexRegionList::createAndClipConvexRegionList(XilBox* box,
                                                   Xil_boolean translate)
{
    //
    // We know that roiptr has been set and than the
    // roi implementation is that of a rectlist.
    //
   XiliRectInt box_rect(0,0,1,1);
   int bx1 = 0;
   int by1 = 0;
   XiliRectInt* rectlist = roiptr->getRectList();

   if(box != NULL) {
       int x2,y2;
       box->getAsCorners(&bx1,&by1,&x2,&y2);
       box_rect.set(bx1, by1,x2,y2);
    }

   int x1,y1,x2,y2;
   //
   // Separate the one-rectangle case from multi-rectangle case
   //
   if(roiptr->numRects() == 1) {
       //
       // regionListRef is NULL, set regionRef
       //
       regionListRef = NULL;
       if(box == NULL) {
           regionRef = new XiliConvexRegion(rectlist);
           if(regionRef == NULL) {
               XIL_ERROR(stateptr, XIL_ERROR_RESOURCE, "di-1", TRUE);
               return XIL_FAILURE;
           }
       } else {
           rectlist->get(&x1,&y1,&x2,&y2);
           //
           // make a temporary copy to clip and translate
           //
           XiliRectInt list_rect(x1,y1,x2,y2);
           if(list_rect.clip(&box_rect) == FALSE) {
               // No intersection
               regionRef = NULL;
           } else {
               //
               //  Do we need to translate the region?
               //
               if(translate == TRUE) {
                   list_rect.translate(-bx1,-by1);
               }
               regionRef = new XiliConvexRegion(&list_rect);
               if(regionRef == NULL) {
                   XIL_ERROR(stateptr, XIL_ERROR_RESOURCE, "di-1", TRUE);
                   return XIL_FAILURE;
               }
           }
       }
       //
       // This flag is used to make sure we only return the
       // single region ONCE in getNext();
       //
       regionRefFirst = TRUE;
   } else {
       //
       // We know that there is more than one rect in the
       // rectlist, so we'll be filling in regionListRef
       // and setting regionRef to NULL
       //
       regionRef = NULL;
       regionRefFirst = FALSE;
       regionListRef = new XiliList<XiliConvexRegion>;
       if(regionListRef == NULL) {
           XIL_ERROR(stateptr, XIL_ERROR_RESOURCE, "di-1", TRUE);
           return XIL_FAILURE;
       }
       if(box == NULL) {
           while(rectlist != NULL) {
               XiliConvexRegion* list_cr = new XiliConvexRegion(rectlist);
               if(list_cr == NULL) {
                   XIL_ERROR(stateptr, XIL_ERROR_RESOURCE, "di-1", TRUE);
                   // cleanup of regionListRef is in calling routine.
                   return XIL_FAILURE;
               }
               regionListRef->append(list_cr);
               rectlist = (XiliRectInt*)rectlist->getNext();
           }
       } else {
           while(rectlist != NULL) {
               rectlist->get(&x1,&y1,&x2,&y2);
               //
               // Must make a temporary copy because we clip
               //
               XiliRectInt list_rect(x1,y1,x2,y2);
               
               if(list_rect.clip(&box_rect)) {
                   //
                   // If the rect is within the box add it to
                   // the convex region list...
                   //
                   XiliConvexRegion* list_cr = new
                       XiliConvexRegion(&list_rect);
                   if(list_cr == NULL) {
                       XIL_ERROR(stateptr, XIL_ERROR_RESOURCE, "di-1", TRUE);
                       // cleanup of regionListRef is in calling routine.
                       return XIL_FAILURE;
                   }
                   //
                   //  Do we need to translate the region?
                   //
                   if(translate == TRUE) {
                       list_cr->translate(-bx1, -by1);
                   }
                   regionListRef->append(list_cr);
               }
               rectlist = (XiliRectInt*)rectlist->getNext();
           }
           
       }
       currentPosition = regionListRef->head();
   }
   return XIL_SUCCESS;
}
