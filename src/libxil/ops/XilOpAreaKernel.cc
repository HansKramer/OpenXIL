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
//  File:	XilOpAreaKernel.cc
//  Project:	XIL
//  Revision:	1.44
//  Last Mod:	10:07:43, 03/10/00
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
#pragma ident	"@(#)XilOpAreaKernel.cc	1.44\t00/03/10  "

#include <xil/xilGPI.hh>
#include "XilOpAreaKernel.hh"

//
// How we treat the destination box is based on what the
// corresponding area in the source maps to. ie we can
// treat the destination as a center box if it maps to
// the source image. Hence the destination type is based
// on the corresponding source box. This should be considered
// when looking at this code.
//

//
// Macro for testing box creation, we also test to
// to make sure that the remaining original box needs
// any further splitting
//
#define CHECK_BOX_CREATE(s, nb) { \
    if(s == XIL_SUCCESS) { \
        nb++; \
    } else { \
	delete new_ble[nb]; \
    } \
}	 

XilOpAreaKernel::XilOpAreaKernel(XilOpNumber op_number) : 
	XilOpArea(op_number)
{
    // Initialize private variables
    k_width = 0;
    k_height = 0;
    key_x = 0;
    key_y = 0;
    r_border = 0;
    l_border = 0;
    t_border = 0;
    b_border = 0;

    edge_condition = XIL_EDGE_NO_WRITE;
    notEnoughSourceToProcess = FALSE;

    base_src_sx1 = 0;
    base_src_sy1 = 0;
    base_src_sx2 = 0;
    base_src_sy2 = 0;
}

XilOpAreaKernel::~XilOpAreaKernel() { }

//
// Virtual methods over-ridden
//

//
//  setBoxStorage() keeps the same pixel values and expands the storage if
//  it's the source iamge. 
//
XilStatus
XilOpAreaKernel::setBoxStorage(XiliRect*            rect,
                               XilDeferrableObject* defobj,
			       XilBox*              box)
{
    *box = *rect;

    if(defobj->setBoxStorage(box) == XIL_FAILURE) {
        return XIL_FAILURE;
    }

    if(defobj == source) {
        int          x1;
        int          y1;
        unsigned int xsize;
        unsigned int ysize;
        int          band;

        //
        //  Extend the source storage area...
        //
        box->getStorageLocation(&x1, &y1, &xsize, &ysize, &band);

        int x2 = x1 + xsize - 1;
        int y2 = y1 + ysize - 1;

        extendSourceStorageBox(XIL_AREA_CENTER, &x1, &y1, &x2, &y2);

        //
        //  Update the storage location...
        //
        box->setStorageLocation(x1, y1, (x2-x1+1), (y2-y1+1), band);
    }

    return XIL_SUCCESS;
}

//
//  The region written in the destination is shrunk due to an absence of
//  data in the source.  We need to overload gsFowardMap() for this
//  reason.
//
XilStatus
XilOpAreaKernel::gsForwardMap(XiliRect*    src_rect,
                              unsigned int ,
                              XiliRect*    dst_rect)
{
    //
    //  Clip with the available source area because we can't read outside of
    //  the source image. 
    //
    dst_rect->set(src_rect);
    dst_rect->clip(&srcAreaRect);

    return XIL_SUCCESS;
}

//
// Over-ride the virtual method to add additional boxes
// to take care of tile boundaries.
//
// This routine is a real bear to follow. Pay close attention
// to pixel values vs. widths. It's all correct, but hard to 
// follow. 
//
XilStatus
XilOpAreaKernel::vSplitOnTileBoundaries(XilBoxList* bl)
{
    //
    // We need to reset the initial source storage box as it
    // may be slightly bigger than we want so that we can call the
    // base class method to get our initial boxlist that we
    // further split.
    //
    XiliSLListIterator<XiliBoxListEntry*> bl_iterator(bl->getList());
    XiliBoxListEntry*                     ble;
    while(bl_iterator.getNext(ble) == XIL_SUCCESS) {
	int     x1;
	int     y1;
	int     x2;
	int     y2;
	int     sx1;
	int     sy1;
	int     sx2;
	int     sy2;
	int     sband;
	XilBox* active_box = &ble->boxes[0];
	XilBox  box;

	active_box->getAsCorners(&x1, &y1, &x2, &y2);
	
	//
	// Reset the storage by getting the storage
	// for the starting area.
	//
	box.setAsCorners(x1, y1, x2, y2);
	source->setBoxStorage(&box);
	box.getStorageAsCorners(&sx1, &sy1, &sx2, &sy2, &sband);

	active_box->setStorageAsCorners(sx1, sy1, sx2, sy2, sband);
    }
    
    //
    // Now call the parent method to split the image
    // into tile boundaries. We will then take each of those
    // box pairings and generate the side boxes on the edge of
    // tiles as needed.
    //
    XilStatus status;

    status = XilOp::vSplitOnTileBoundaries(bl);
    if(status == XIL_FAILURE) {
	return status;
    }

    //
    //  Now we can loop over the boxes and split them
    //  along the edges and internallly along tile boundaries
    //
    XiliSLListIterator<XiliBoxListEntry*> bl_iter(bl->getList());
    while(bl_iter.getNext(ble) == XIL_SUCCESS) {
	int                  src_x1, src_y1, src_x2, src_y2;
	int                  dst_x1, dst_y1, dst_x2, dst_y2;
	int                  dx1, dy1, dx2, dy2;
	int                  src_sx1, src_sy1, src_sx2, src_sy2;
	int                  dst_sx1, dst_sy1, dst_sx2, dst_sy2;
	int                  src_band;	
	int                  dst_band;
	unsigned int         image_x2 = source->getWidth()-1;
	unsigned int         image_y2 = source->getHeight()-1;
	int                  new_boxes = 0;
	XilBox*              src_box = &ble->boxes[0];
	XilBox*              dst_box = &ble->boxes[1];
	XiliBoxListEntry*    new_ble[12];
	
	//
	// Get the original box co-ordinates and storage co-ordinates
	// for src and dst
	//
	src_box->getAsCorners(&src_x1, &src_y1, &src_x2, &src_y2);
	src_box->getStorageAsCorners(&src_sx1, &src_sy1, &src_sx2,
				     &src_sy2, &src_band);
	
	dst_box->getAsCorners(&dst_x1, &dst_y1, &dst_x2, &dst_y2);
	dst_box->getStorageAsCorners(&dst_sx1, &dst_sy1, &dst_sx2,
				     &dst_sy2, &dst_band);

	// Initialize the deltas so we know how much to take off the edges.
	dx1 = 0;
	dy1 = 0;
	dx2 = 0;
	dy2 = 0;

	
	//
	// Test to see if we need to split the box at all
	//
	if(needsSplitting(src_x1, src_y1, src_x2, src_y2,
			  dst_sx1, dst_sy1, dst_sx2, dst_sy2) == FALSE) {
            //
            //  This box doesn't need to be split because
            //      it's too small OR
            //      it's contained within an edge or corner box OR
            //      it's contained within the image center box (without tiles) OR
            //      it's an internal box and contained wholly within a tile
            //  so just tag the box and go to the next in the list
            //
	    setupBoxes(src_box,
		       src_x1, src_y1, src_x2, src_y2,
		       src_sx1, src_sy1, src_sx2, src_sy2,
		       src_band,
		       dst_box,
		       dst_x1, dst_y1, dst_x2, dst_y2,
		       dst_sx1, dst_sy1, dst_sx2, dst_sy2,
		       dst_band);
	    continue;
	}

	//
        // Slice off the edges of the box if they lie on any
	// of the image edges. Doing this allows us to make
	// all of the other boxes center boxes and the compute
	// routine can handle the edge cases depending on the
	// filling operation.
	//
	
	// Right edge
        // if src_x2 is within the border but the box is not wholly
        // contained within the border
        //
	if(((unsigned int)src_x2 > (image_x2-r_border)) &&
           ((unsigned int)src_x1 <= (image_x2-r_border))) {
	    //
	    // Always need to calculate the diffs on the edges.
	    // We add 1 to the difference because we
	    // use it to set up co-ordinates. We will increment
	    // it at the end of the loop.
	    //
	    dx2 = r_border - (image_x2-src_x2) - 1;

		//
		// These tell us how much to take off for the corners.
		//
		int edy1 = 0;
		int edy2 = 0;
		
		if((unsigned int)src_y1 < t_border) {
		    //
                    // RIGHT-UPPER CORNER of IMAGE
		    // Generate a top corner box, the size of which
		    // is sufficient to cover the top piece of the
		    // kernel and no more.
		    //

		    if((unsigned int)src_y2 < t_border) {
		        //
		        //  It's fully contained within the upper border.
		        //  only split the box in the x direction.
		        //  
		        edy1 = src_y2 - src_y1;
		    } else {
		        edy1 = t_border - src_y1 - 1;
		    }

                    //
                    // Although we need to strip off the edge boxes,
                    // we don't want to add them to the box list if
                    // the edge condition is NO_WRITE
                    //
                    if(edge_condition != XIL_EDGE_NO_WRITE) {
                        new_ble[new_boxes] = new XiliBoxListEntry;
                        if(new_ble[new_boxes] == NULL) {
                            // TODO clean up generated boxes
                            return XIL_FAILURE;
                        }
                        status = setupBoxes(&new_ble[new_boxes]->boxes[0],
                                            src_x2-dx2, src_y1, src_x2, src_y1+edy1,
                                            src_sx2-dx2, src_sy1, src_sx2, src_sy1+edy1,
                                            src_band,
                                            &new_ble[new_boxes]->boxes[1],
                                            dst_x2-dx2, dst_y1, dst_x2, dst_y1+edy1,
                                            dst_sx2-dx2, dst_sy1, dst_sx2, dst_sy1+edy1,
                                            dst_band);
                        CHECK_BOX_CREATE(status, new_boxes);
                    }
		    edy1++;
		}

		if((unsigned int)src_y2 > (image_y2-b_border)) {
		    //
                    // RIGHT-BOTTOM CORNER of IMAGE
		    // Generate a bottom corner box, the size of which
		    // is sufficient to cover the bottom piece of the
		    // kernel and no more.
		    //

		    if((unsigned int)src_y1 > (image_y2-b_border)) {
		        //
		        //  We're fully contained within the bottom border
		        //  Only divide the box along X, that is, the
		        //  height of the box should remain the same.
		        //
		        edy2 = src_y2 - src_y1;
		    } else {
		        edy2 = b_border - (image_y2-src_y2) - 1;		    
		    }
                    //
                    // Although we need to strip off the edge boxes,
                    // we don't want to add them to the box list if
                    // the edge condition is NO_WRITE
                    //
                    if(edge_condition != XIL_EDGE_NO_WRITE) {
                        new_ble[new_boxes] = new XiliBoxListEntry;
                        if(new_ble[new_boxes] == NULL) {
                            // TODO clean up generated boxes
                            return XIL_FAILURE;
                        }
                        status = setupBoxes(&new_ble[new_boxes]->boxes[0],
                                            src_x2-dx2, src_y2-edy2,
                                            src_x2, src_y2,
                                            src_sx2-dx2, src_sy2-edy2,
                                            src_sx2, src_sy2,
                                            src_band,
                                            &new_ble[new_boxes]->boxes[1],
                                            dst_x2-dx2, dst_y2-edy2,
                                            dst_x2, dst_y2,
                                            dst_sx2-dx2, dst_sy2-edy2,
                                            dst_sx2, dst_sy2,
                                            dst_band);
                        CHECK_BOX_CREATE(status, new_boxes);
                    }
		    edy2++;
		}
		
		if(((src_y2-edy2)-(src_y1+edy1)+1) > 0) {
		    //
                    // RIGHT EDGE of IMAGE after any CORNERS
		    // Generate an image edge box
		    // the width of the kernel.
		    //
                    //
                    // Although we need to strip off the edge boxes,
                    // we don't want to add them to the box list if
                    // the edge condition is NO_WRITE
                    //
                    if(edge_condition != XIL_EDGE_NO_WRITE) {
                        new_ble[new_boxes] = new XiliBoxListEntry;
                        if(new_ble[new_boxes] == NULL) {
                            return XIL_FAILURE;
                        }
                        status = setupBoxes(&new_ble[new_boxes]->boxes[0],
                                            src_x2-dx2, src_y1+edy1,
                                            src_x2, src_y2-edy2,
                                            src_sx2-dx2, src_sy1+edy1,
                                            src_sx2, src_sy2-edy2,
                                            src_band,
                                            &new_ble[new_boxes]->boxes[1],
                                            dst_x2-dx2, dst_y1+edy1,
                                            dst_x2, dst_y2-edy2,
                                            dst_sx2-dx2, dst_sy1+edy1,
                                            dst_sx2, dst_sy2-edy2,
                                            dst_band);
                        CHECK_BOX_CREATE(status, new_boxes);
                    }
		}
	    dx2++;
	}

	// Left edge
        // if src_x1 is within the border area, but the box isn't wholly
        // contained within the border area
        //
        if(((unsigned int)src_x1 < l_border) &&
           ((unsigned int)src_x2 >= l_border)) {
	    //
	    // Always need to set up the edge deltas
	    // subtract 1 for co-ordinate arithmetic
	    //
	    dx1 = l_border - src_x1 - 1;
	    
		
		//
		// These tell us how much to take off for the corners.
		//
		int edy1 = 0;
		int edy2 = 0;
		
		if((unsigned int)src_y1 < t_border) {
		    //
                    // LEFT-UPPER CORNER of IMAGE
		    // Generate a top corner box, the size of which
		    // is sufficient to cover the top piece of the
		    // kernel and no more.
		    //

		    if((unsigned int)src_y2 < t_border) {
		        //
		        //  Fully contained, split only in X
		        //
		        edy1 = src_y2 - src_y1;
		    } else {
		        edy1 = t_border - src_y1 - 1;
		    }
                    // Although we need to strip off the edge boxes,
                    // we don't want to add them to the box list if
                    // the edge condition is NO_WRITE
                    //
                    if(edge_condition != XIL_EDGE_NO_WRITE) {
                        new_ble[new_boxes] = new XiliBoxListEntry;
                        if(new_ble[new_boxes] == NULL) {
                            // TODO clean up generated boxes
                            return XIL_FAILURE;
                        }
                        status = setupBoxes(&new_ble[new_boxes]->boxes[0],
                                            src_x1, src_y1, src_x1+dx1, src_y1+edy1,
                                            src_sx1, src_sy1, src_sx1+dx1, src_sy1+edy1,
                                            src_band,
                                            &new_ble[new_boxes]->boxes[1],
                                            dst_x1, dst_y1, dst_x1+dx1, dst_y1+edy1,
                                            dst_sx1, dst_sy1, dst_sx1+dx1, dst_sy1+edy1,
                                            dst_band);
                        CHECK_BOX_CREATE(status, new_boxes);
                    }
		    edy1++;
		}

		if((unsigned int)src_y2 > (image_y2-b_border)) {
		    //
                    // LEFT-LOWER CORNER of IMAGE
		    // Generate a bottom corner box, the size of which
		    // is sufficient to cover the bottom piece of the
		    // kernel and no more.
		    //
		    if((unsigned int)src_y1 > (image_y2-b_border)) {
		        //
		        //  Fully contained in y, only split in X
		        //

		        edy2 = src_y2 - src_y1;
		    } else {
		        edy2 = b_border - (image_y2-src_y2) - 1;
		    }
		    //
                    // Although we need to strip off the edge boxes,
                    // we don't want to add them to the box list if
                    // the edge condition is NO_WRITE
                    //
                    if(edge_condition != XIL_EDGE_NO_WRITE) {
                        new_ble[new_boxes] = new XiliBoxListEntry;
                        if(new_ble[new_boxes] == NULL) {
                            return XIL_FAILURE;
                        }
                        status = setupBoxes(&new_ble[new_boxes]->boxes[0],
                                            src_x1, src_y2-edy2, src_x1+dx1, src_y2,
                                            src_sx1, src_sy2-edy2, src_sx1+dx1, src_sy2,
                                            src_band,
                                            &new_ble[new_boxes]->boxes[1],
                                            dst_x1, dst_y2-edy2, dst_x1+dx1, dst_y2,
                                            dst_sx1, dst_sy2-edy2, dst_sx1+dx1, dst_sy2,
                                            dst_band);
                        CHECK_BOX_CREATE(status, new_boxes);
                    }
		    edy2++;
		}

		if(((src_y2-edy2)-(src_y1+edy1)+1) > 0) {
		    //
                    // REMAINING LEFT EDGE of IMAGE
		    // Generate a bottom corner box, the size of which
		    // is sufficient to cover the bottom piece of the
		    // kernel and no more.
		    //
		    //
                    // Although we need to strip off the edge boxes,
                    // we don't want to add them to the box list if
                    // the edge condition is NO_WRITE
                    //
                    if(edge_condition != XIL_EDGE_NO_WRITE) {
                        new_ble[new_boxes] = new XiliBoxListEntry;
                        if(new_ble[new_boxes] == NULL) {
                            // TODO clean up generated boxes
                            return XIL_FAILURE;
                        }
                        
                        status = setupBoxes(&new_ble[new_boxes]->boxes[0],
                                            src_x1, src_y1+edy1,
                                            src_x1+dx1, src_y2-edy2,
                                            src_sx1, src_sy1+edy1,
                                            src_sx1+dx1, src_sy2-edy2,
                                            src_band,
                                            &new_ble[new_boxes]->boxes[1],
                                            dst_x1, dst_y1+edy1,
                                            dst_x1+dx1, dst_y2-edy2,
                                            dst_sx1, dst_sy1+edy1,
                                            dst_sx1+dx1, dst_sy2-edy2,
                                            dst_band);
                        CHECK_BOX_CREATE(status, new_boxes);
                    }
		}
	    dx1++;
	}

	// Top edge
        // if src_y1 is above the border AND crosses it
        //
        if(((unsigned int)src_y1 < t_border) &&
           ((unsigned int)src_y2 >= t_border)) {
	    //
	    // Always need to set up the diffs on the edges
	    // subtract 1 for co-ordinate arithmetic.
	    //
	    dy1 = t_border - src_y1 - 1;

		//
		// These tell us how much to compensate for a corner
		// box.
		//
		int edx1 = 0;
		int edx2 = 0;

		//
		// If the left/right edges didn't create a corner box
		// need to do it here.
		//
		if((unsigned int)(src_x1+dx1) < l_border) {
		    //
                    // LEFT-UPPER CORNER of IMAGE
		    // Generate a top corner box, the size of which
		    // is sufficient to cover the top piece of the
		    // kernel and no more.
                    //

                    if( (unsigned int)(src_x2-dx2) < l_border) {
		        //
		        //  We're fully contained within the left border
		        //  Only divide the box along y, that is, the
		        //  width of the box should remain the same.
		        //
                        
                        //
                        //  calculate edx1 so the box won't be changed
                        //  in setup boxes call
                        //
		        edx1 = (src_x2-dx2) - (src_x1+dx1);
		    } else {
                        edx1 = l_border - src_x1 - 1;
		    }

		    //
                    // Although we need to strip off the edge boxes,
                    // we don't want to add them to the box list if
                    // the edge condition is NO_WRITE
                    //
                    if(edge_condition != XIL_EDGE_NO_WRITE) {
                        new_ble[new_boxes] = new XiliBoxListEntry;
                        if(new_ble[new_boxes] == NULL) {
                            // TODO clean up generated boxes
                            return XIL_FAILURE;
                        }
		    
                        status = setupBoxes(&new_ble[new_boxes]->boxes[0],
                                            src_x1+dx1, src_y1,
                                            src_x1+dx1+edx1, src_y1+dy1,
                                            src_sx1+dx1, src_sy1,
                                            src_sx1+dx1+edx1, src_sy1+dy1,
                                            src_band,
                                            &new_ble[new_boxes]->boxes[1],
                                            dst_x1+dx1, dst_y1,
                                            dst_x1+dx1+edx1, dst_y1+dy1,
                                            dst_sx1+dx1, dst_sy1,
                                            dst_sx1+dx1+edx1, dst_sy1+dy1,
                                            dst_band);
                        CHECK_BOX_CREATE(status, new_boxes);
                    }
		    edx1++;
		}

		//
		// If the left/right edges didn't create a corner box
		// need to do it here.
		//
		if((unsigned int)(src_x2-dx2) > (image_x2-r_border)) {
		    //
                    // RIGHT-UPPER CORNER of IMAGE
		    // Generate a top corner box, the size of which
		    // is sufficient to cover the top piece of the
		    // kernel and no more.
		    //

                    if( (unsigned int)(src_x1+dx1) > (image_x2-r_border)) {
		        //
		        //  We're fully contained within the right border
		        //  Only divide the box along y, that is, the
		        //  width of the box should remain the same.
		        //
                        
                        //
                        //  calculate edx2 so the box won't be changed
                        //  in setup boxes call
                        //
		        edx2 = (src_x2-dx2) - (src_x1+dx1);
		    } else {
                        edx2 = r_border - (image_x2-src_x2) - 1;
		    }
		    //
                    // Although we need to strip off the edge boxes,
                    // we don't want to add them to the box list if
                    // the edge condition is NO_WRITE
                    //
                    if(edge_condition != XIL_EDGE_NO_WRITE) {
                        new_ble[new_boxes] = new XiliBoxListEntry;
                        if(new_ble[new_boxes] == NULL) {
                            // TODO clean up generated boxes
                            return XIL_FAILURE;
                        }
		    
                        status = setupBoxes(&new_ble[new_boxes]->boxes[0],
                                            src_x2-dx2-edx2, src_y1,
                                            src_x2-dx2, src_y1+dy1,
                                            src_sx2-dx2-edx2, src_sy1,
                                            src_sx2-dx2, src_sy1+dy1,
                                            src_band,
                                            &new_ble[new_boxes]->boxes[1],
                                            dst_x2-dx2-edx2, dst_y1,
                                            dst_x2-dx2, dst_y1+dy1,
                                            dst_sx2-dx2-edx2, dst_sy1,
                                            dst_sx2-dx2, dst_sy1+dy1,
                                            dst_band);
                        CHECK_BOX_CREATE(status, new_boxes);
                    }
		    edx2++;
		}
		

		if(((src_x2-dx2-edx2)-(src_x1+dx1+edx1)+1) > 0) {
		    //
                    // REMAINING UPPER EDGE of IMAGE
		    // Generate a top edge box, the height of which
		    // is the kernel
		    //
		    //
                    // Although we need to strip off the edge boxes,
                    // we don't want to add them to the box list if
                    // the edge condition is NO_WRITE
                    //
                    if(edge_condition != XIL_EDGE_NO_WRITE) {
                        new_ble[new_boxes] = new XiliBoxListEntry;
                        if(new_ble[new_boxes] == NULL) {
                            // TODO clean up generated boxes
                            return XIL_FAILURE;
                        }
                        
                        status = setupBoxes(&new_ble[new_boxes]->boxes[0],
                                            src_x1+dx1+edx1, src_y1,
                                            src_x2-dx2-edx2, src_y1+dy1,
                                            src_sx1+dx1+edx1, src_sy1,
                                            src_sx2-dx2-edx2, src_sy1+dy1,
                                            src_band,
                                            &new_ble[new_boxes]->boxes[1],
                                            dst_x1+dx1+edx1, dst_y1,
                                            dst_x2-dx2-edx2, dst_y1+dy1,
                                            dst_sx1+dx1+edx1, dst_sy1,
                                            dst_sx2-dx2-edx2, dst_sy1+dy1,
                                            dst_band);
                        CHECK_BOX_CREATE(status, new_boxes);
                    }
		}
	    dy1++;
	}

	// Bottom edge
        // if src_y2 is below the border AND the box crosses the border
        //
	if(((unsigned int)src_y2 > (image_y2-b_border)) &&
           ((unsigned int)src_y1 <= (image_y2-b_border))) {
	    //
	    // Still need to set up the diffs on the edges
	    // Add 1 for co-ordinate arithmetic.
	    //
	    dy2 = b_border - (image_y2-src_y2) - 1;

		//
		// These tell us how much to compensate for a corner
		// box.
		//
		int edx1 = 0;
		int edx2 = 0;
		
		//
		// If the left/right edges didn't create a corner box
		// need to do it here.
		//
		if((unsigned int)(src_x1+dx1) < l_border) {
		    //
                    // LEFT-LOWER CORNER of IMAGE
		    // Generate a bottom corner box, the size of which
		    // is sufficient to cover the bottom piece of the
		    // kernel and no more.
		    //
                    if( (unsigned int)(src_x2-dx2) < l_border) {
		        //
		        //  We're fully contained within the left border
		        //  Only divide the box along y, that is, the
		        //  width of the box should remain the same.
		        //
                        
                        //
                        //  calculate edx1 so the box won't be changed
                        //  in setup boxes call
                        //
		        edx1 = (src_x2-dx2) - (src_x1+dx1);
		    } else {
                        edx1 = l_border - src_x1 - 1;
		    }

		    //
                    // Although we need to strip off the edge boxes,
                    // we don't want to add them to the box list if
                    // the edge condition is NO_WRITE
                    //
                    if(edge_condition != XIL_EDGE_NO_WRITE) {
                        new_ble[new_boxes] = new XiliBoxListEntry;
                        if(new_ble[new_boxes] == NULL) {
                            // TODO clean up generated boxes
                            return XIL_FAILURE;
                        }
                        
                        status = setupBoxes(&new_ble[new_boxes]->boxes[0],
                                            src_x1+dx1, src_y2-dy2,
                                            src_x1+dx1+edx1, src_y2,
                                            src_sx1+dx1, src_sy2-dy2,
                                            src_sx1+dx1+edx1, src_sy2,
                                            src_band,
                                            &new_ble[new_boxes]->boxes[1],
                                            dst_x1+dx1, dst_y2-dy2,
                                            dst_x1+dx1+edx1, dst_y2,
                                            dst_sx1+dx1, dst_sy2-dy2,
                                            dst_sx1+dx1+edx1, dst_sy2,
                                            dst_band);
                        CHECK_BOX_CREATE(status, new_boxes);
                    }
		    edx1++;
		}
		
		//
		// If the left/right edges didn't create a corner box
		// need to do it here.
		//
		if((unsigned int)(src_x2-dx2) > (image_x2-r_border)) {
		    //
                    // RIGHT-LOWER CORNER of IMAGE
		    // Generate a bottom corner box, the size of which
		    // is sufficient to cover the bottom piece of the
		    // kernel and no more.
		    //
                    if( (unsigned int)(src_x1+dx1) > (image_x2-r_border)) {
		        //
		        //  We're fully contained within the right border
		        //  Only divide the box along y, that is, the
		        //  width of the box should remain the same.
		        //
                        
                        //
                        //  calculate edx2 so the box won't be changed
                        //  in setup boxes call
                        //
		        edx2 = (src_x2-dx2) - (src_x1+dx1);
		    } else {
                        edx2 = r_border - (image_x2-src_x2) - 1;
		    }
		    //
                    // Although we need to strip off the edge boxes,
                    // we don't want to add them to the box list if
                    // the edge condition is NO_WRITE
                    //
                    if(edge_condition != XIL_EDGE_NO_WRITE) {
                        new_ble[new_boxes] = new XiliBoxListEntry;
                        if(new_ble[new_boxes] == NULL) {
                            // TODO clean up generated boxes
                            return XIL_FAILURE;
                        }
                        
                        status = setupBoxes(&new_ble[new_boxes]->boxes[0],
                                            src_x2-dx2-edx2, src_y2-dy2,
                                            src_x2-dx2, src_y2,
                                            src_sx2-dx2-edx2, src_sy2-dy2,
                                            src_sx2-dx2, src_sy2,
                                            src_band,
                                            &new_ble[new_boxes]->boxes[1],
                                            dst_x2-dx2-edx2, dst_y2-dy2,
                                            dst_x2-dx2, dst_y2,
                                            dst_sx2-dx2-edx2, dst_sy2-dy2,
                                            dst_sx2-dx2, dst_sy2,
                                            dst_band);
                        
                        CHECK_BOX_CREATE(status, new_boxes);
                    }
		    edx2++;
		}

		if(((src_x2-dx2-edx2)-(src_x1+dx1+edx1)+1) > 0) {
		    //
                    // REMAINING LOWER EDGE of IMAGE
		    // Generate a bottom box, the size of which
		    // is the width of the border.
		    //
		    //
                    // Although we need to strip off the edge boxes,
                    // we don't want to add them to the box list if
                    // the edge condition is NO_WRITE
                    //
                    if(edge_condition != XIL_EDGE_NO_WRITE) {
                        new_ble[new_boxes] = new XiliBoxListEntry;
                        if(new_ble[new_boxes] == NULL) {
                            // TODO clean up generated boxes
                            return XIL_FAILURE;
                        }
                        
                        status = setupBoxes(&new_ble[new_boxes]->boxes[0],
                                            src_x1+dx1+edx1, src_y2-dy2,
                                            src_x2-dx2-edx2, src_y2,
                                            src_sx1+dx1+edx1, src_sy2-dy2,
                                            src_sx2-dx2-edx2, src_sy2,
                                            src_band,
                                            &new_ble[new_boxes]->boxes[1],
                                            dst_x1+dx1+edx1, dst_y2-dy2,
                                            dst_x2-dx2-edx2, dst_y2,
                                            dst_sx1+dx1+edx1, dst_sy2-dy2,
                                            dst_sx2-dx2-edx2, dst_sy2,
                                            dst_band);
                        CHECK_BOX_CREATE(status, new_boxes);
                    }
		}
	    dy2++;
	}

	XilBoxAreaType  src_box_type;

        //
        //  Now that we've stripped off any edges, we should be left with
        //  a valid box type.
        //
        if(getValidBoxAreaType(source, 
                               src_x1+dx1, src_y1+dy1,
                               src_x2-dx2, src_y2-dy2,&src_box_type) == FALSE) {
            //
            //  Generate an error. If all the edges have been correctly
            //  stripped off the box type MUST be valid.
            //
            // TODO clean up generated boxes and perhaps generate secondary error?
            //
            return XIL_FAILURE;
        }	

	//
	// Now that we have stripped off the IMAGE edges we generate
	// internal boxes as needed. We only want to do this stage
	// if we are left with a center box and there is more than
	// one tile on the the source image.
	//
	if((src_box_type == XIL_AREA_CENTER) &&
	   (source->getNumTiles() > 1)) {

	    //
	    // Save the original source co-ordinates for
	    // testing for image edges.
	    //
	    unsigned int orig_src_x1 = src_x1;
	    unsigned int orig_src_y1 = src_y1;
	    unsigned int orig_src_x2 = src_x2;
	    unsigned int orig_src_y2 = src_y2;
	    
            //
	    // Update the internal src box by the edge deltas
            //
	    src_x1 += dx1;
	    src_y1 += dy1;
	    src_x2 -= dx2;
	    src_y2 -= dy2;
	    src_sx1 += dx1;
	    src_sy1 += dy1;
	    src_sx2 -= dx2;
	    src_sy2 -= dy2;

            //
	    // Update the internal dst box by the edge deltas
            //
	    dst_x1 += dx1;
	    dst_y1 += dy1;
	    dst_x2 -= dx2;
	    dst_y2 -= dy2;
	    dst_sx1 += dx1;
	    dst_sy1 += dy1;
	    dst_sx2 -= dx2;
	    dst_sy2 -= dy2;

	    dx1 = 0;
	    dy1 = 0;
	    dx2 = 0;
	    dy2 = 0;

	    // Compute tile boundaries
	    unsigned int tile_xsize, tile_ysize;
	    source->getTileSize(&tile_xsize, &tile_ysize);

	    int tile_x1 = (src_sx1/tile_xsize)*tile_xsize;
	    int tile_y1 = (src_sy1/tile_ysize)*tile_ysize;
	    int tile_x2 = tile_x1+tile_xsize-1;
	    int tile_y2 = tile_y1+tile_ysize-1;

	    //
	    // Generate a right internal box if we are sitting
	    // on the right edge of a destination tile, and
	    // we aren't on the edge of the image and
            // the box isn't wholly contained within the border area
	    //
	    if((((unsigned int)src_sx2 > (tile_x2-r_border)) &&
		((unsigned int)orig_src_x2 < (image_x2-r_border)) &&
                ((unsigned int) src_sx1 <= (tile_x2-r_border)))) {
		
		dx2 = r_border - (tile_x2-src_sx2) - 1;
		
		new_ble[new_boxes] = new XiliBoxListEntry;
		if(new_ble[new_boxes] == NULL) {
		    // TODO clean up generated boxes
		    return XIL_FAILURE;
		}
		status = setupBoxes(&new_ble[new_boxes]->boxes[0],
				    src_x2-dx2, src_y1, src_x2, src_y2,
				    src_sx2-dx2, src_sy1, src_sx2, src_sy2,
				    src_band,
				    &new_ble[new_boxes]->boxes[1],
				    dst_x2-dx2, dst_y1, dst_x2, dst_y2,
				    dst_sx2-dx2, dst_sy1, dst_sx2, dst_sy2,
				    dst_band);
		CHECK_BOX_CREATE(status, new_boxes);
		dx2++;

	    }

	    // Left internal box
            // if src_sx1 is within the border area, it's not the
            // edge of an image AND the box is not wholly contained
            // within the border area.
            //
	    if(((unsigned int) src_sx1 < (tile_x1+l_border)) &&
	       (orig_src_x1 > l_border) &&
               ((unsigned int) src_sx2 >= (tile_x1+l_border))) {

		dx1 = tile_x1 + l_border - src_sx1 - 1;
		
		new_ble[new_boxes] = new XiliBoxListEntry;
		if(new_ble[new_boxes] == NULL) {
		    // TODO clean up generated boxes
		    return XIL_FAILURE;
		}
		status = setupBoxes(&new_ble[new_boxes]->boxes[0],
				    src_x1, src_y1, src_x1+dx1, src_y2,
				    src_sx1, src_sy1, src_sx1+dx1, src_sy2,
				    src_band,
				    &new_ble[new_boxes]->boxes[1],
				    dst_x1, dst_y1, dst_x1+dx1, dst_y2,
				    dst_sx1, dst_sy1, dst_sx1+dx1, dst_sy2,
				    dst_band);
		CHECK_BOX_CREATE(status, new_boxes);
		dx1++;
	    }

	    // Top internal box
            // if src_sy1 is within the border area AND src_y2 is not with the
            // border area AND it isn't an image edge
            //
	    if(((unsigned int) src_sy1 < (tile_y1+t_border)) &&
	       (orig_src_y1 > t_border) &&
               ((unsigned int) src_sy2 >= (tile_y1+t_border))) {

		dy1 = tile_y1 + t_border - src_sy1 - 1;
		
		new_ble[new_boxes] = new XiliBoxListEntry;
		if(new_ble[new_boxes] == NULL) {
		    // TODO clean up generated boxes
		    return XIL_FAILURE;
		}
		
		status = setupBoxes(&new_ble[new_boxes]->boxes[0],
				    src_x1+dx1, src_y1, src_x2-dx2, src_y1+dy1,
				    src_sx1+dx1, src_sy1, src_sx2-dx2, src_sy1+dy1,
				    src_band,
				    &new_ble[new_boxes]->boxes[1],
				    dst_x1+dx1, dst_y1, dst_x2-dx2, dst_y1+dy1,
				    dst_sx1+dx1, dst_sy1, dst_sx2-dx2, dst_sy1+dy1,
				    dst_band);
		CHECK_BOX_CREATE(status, new_boxes);
		dy1++;
	    }

	    // Bottom internal box
            // if src_sy2 is within the border area but the box isn't fully
            // contained within the border area and it's not the edge of
            // the image
            //
	    if(((unsigned int) src_sy2 > (tile_y2-b_border)) &&
	       (orig_src_y2 < (image_y2-b_border)) &&
               ((unsigned int) src_sy1 <= (tile_y2-b_border))) {
		
		dy2 = b_border - (tile_y2-src_sy2) - 1;
		
		new_ble[new_boxes] = new XiliBoxListEntry;
		if(new_ble[new_boxes] == NULL) {
		    // TODO clean up generated boxes
		    return XIL_FAILURE;
		}
		status = setupBoxes(&new_ble[new_boxes]->boxes[0],
				    src_x1+dx1, src_y2-dy2, src_x2-dx2, src_y2,
				    src_sx1+dx1, src_sy2-dy2, src_sx2-dx2, src_sy2,
				    src_band,
				    &new_ble[new_boxes]->boxes[1],
				    dst_x1+dx1, dst_y2-dy2, dst_x2-dx2, dst_y2,
				    dst_sx1+dx1, dst_sy2-dy2, dst_sx2-dx2, dst_sy2,
				    dst_band);
		CHECK_BOX_CREATE(status, new_boxes);
		dy2++;
	    }
	}
	
	//
	// Shrink the size of the src, dst pixel areas for
	// the main box.
	//
	status = setupBoxes(src_box,
			    src_x1+dx1, src_y1+dy1, src_x2-dx2, src_y2-dy2,
			    src_sx1+dx1, src_sy1+dy1, src_sx2-dx2, src_sy2-dy2,
			    src_band,
			    dst_box,
			    dst_x1+dx1, dst_y1+dy1, dst_x2-dx2, dst_y2-dy2,
			    dst_sx1+dx1, dst_sy1+dy1, dst_sx2-dx2, dst_sy2-dy2,
			    dst_band);
	
	//
	// Add in the new boxes
	//
	int i;
	for(i=0; i<new_boxes; i++) {
	    if(bl->getList()->insertAfter(new_ble[i],
                                          bl_iter.getCurrentPosition()) ==
               _XILI_SLLIST_INVALID_POSITION) {
                //
                //  Oddly enough, insertion failed.
                //
                //  TODO: 2/26/96 jlf  Generate secondary failure?
                //
                return XIL_FAILURE;
            }
        }

	// 
	// Skip over the new boxes we added to get to the next unsplit box
	//
        for(int cnt=0; cnt<new_boxes; cnt++) {
	    if(bl_iter.getNext(ble) == XIL_FAILURE) {
                //
                //  TODO: 2/26/96 jlf  Generate secondary failure?
                //
                return XIL_FAILURE;
            } else {
                //
                // boxCount never got set during splitting
                //
                ble->boxCount = 2;
            }
	}
    }
    
    return XIL_SUCCESS;
}

//
// Protected methods
//

//
// Initialize with some base information from the derived
// class
//
void
XilOpAreaKernel::initializeAreaKernel(unsigned int     w,
				      unsigned int     h,
				      unsigned int     kx,
				      unsigned int     ky,
				      XilEdgeCondition condition,
                                      Xil_boolean      small_source)
{
    //
    //  Get our source and dest images
    //
    source = getSrcImage(1);
    dest = getDstImage(1);

    //
    //  Assume the sub-class did appropriate checks on
    //  the kernel parameters so just set them
    //
    k_width = w;
    k_height = h;
    key_x = kx;
    key_y = ky;
    r_border = k_width - (key_x + 1);
    b_border = k_height - (key_y + 1);
    l_border = key_x;
    t_border = key_y;

    //
    //  Save the edge condition
    //
    edge_condition = condition;

    //
    //  Set a flag to note when the source image is too small to
    //  provide any data using the kernel
    //  This has to be set outside of this routine because in some
    //  cases (see XilOpConvolve) the op doesn't get created at all.
    //
    notEnoughSourceToProcess = small_source;
    
    //
    //  Store a shrunk global space rect for use in forward mapping from
    //  source to destination.
    //
    {
        srcAreaRect.set(source->getGlobalSpaceRect());

        int x1;
        int y1;
        int x2;
        int y2;
        srcAreaRect.get(&x1, &y1, &x2, &y2);

        x1 += key_x;
        y1 += key_y;
        x2 -= key_x;
        y2 -= key_y;

        srcAreaRect.set(x1, y1, x2, y2);
    }

    //
    //  We need to store the base co-ordinates of the storage
    //  area for the source image so that we can clip against
    //  it later when expanding storage.
    //
    XilBox       box;
    int          band;

    box.setAsRect(0, 0, source->getWidth(), source->getHeight());
    source->setBoxStorage(&box);
    box.getStorageAsCorners(&base_src_sx1, &base_src_sy1,
			    &base_src_sx2, &base_src_sy2, &band);
}

//
//
//  Over-riding generateIntersectedRoi to catch the case where the
//  source is too small for the kernel and the edge condition is
//  EDGE_NO_WRITE. In this case generateIntersectedRoi should return
//  XIL_FAILURE so that the operation does not proceed. Otherwise,
//  call the default generateIntersectdRoi().
//
XilStatus
XilOpAreaKernel::generateIntersectedRoi()
{
    if((notEnoughSourceToProcess) &&
       (edge_condition == XIL_EDGE_NO_WRITE)) {
        return XIL_FAILURE;
    }
    return XilOp::generateIntersectedRoi();
}

//
// Private methods
//

//
// Determine a valid XilBoxAreaType for the given
// box. If the given box doesn't fit a valid type, then
// return a FALSE.
// 
Xil_boolean
XilOpAreaKernel::getValidBoxAreaType(XilImage*      image,
				    int             x1,
				    int             y1,
				    int             x2,
				    int             y2,
                                    XilBoxAreaType* type)
{
    int    image_x2;
    int    image_y2;
    *type = XIL_AREA_CENTER;

    //
    // get the width and height, Subtract 1 for comparison
    // with co-ordinates
    //
    image_x2 = image->getWidth()-1;
    image_y2 = image->getHeight()-1;

    //
    //  If the whole image is smaller than the kernel
    //  in either dimension *or* 
    //
    if(notEnoughSourceToProcess) {
        *type = XIL_AREA_SMALL_SOURCE;
        return TRUE;
    }

    //
    //  There are nine valid box options.
    //  3 positions in Y times 3 positions in X
    //  Identify the X and Y separately and
    //  then identify the X,Y pair to identify the
    //  box position
    //
    //  It doesn't matter if the box touches an image
    //  edge so long as the appropriate borders (which
    //  could be zero) are taken in to account.
    //
    enum horiz_type {
        LEFT_BOX,
        HMID_BOX,
        RIGHT_BOX,
        H_INVALID
    }; 

    enum vert_type {
        TOP_BOX,
        VMID_BOX,
        BOT_BOX,
        V_INVALID
    };
    
    horiz_type x_position = H_INVALID;
    vert_type  y_position = V_INVALID;
    int        RightEdge;
    int        BottomEdge;

    RightEdge = image_x2 - r_border;
    BottomEdge = image_y2 - b_border;

    //
    //  First identify horizontal positions.
    //
    if( (x1< l_border) && (x2 < l_border)) {
        x_position = LEFT_BOX;
    } else if ( (x1 >= l_border) && (x2 <= RightEdge)) {
        x_position = HMID_BOX;
    } else if ( (x1 > RightEdge) && (x2 > RightEdge)) {
        x_position = RIGHT_BOX;
    } else {
        //
        //  We must straddle an edge in the horizontal
        //
        return FALSE;
    }
    
    //
    //  Now identify vertical position
    //
    if( (y1< t_border) && (y2 < t_border)) {
        y_position = TOP_BOX;
    } else if ( (y1 >= t_border) && (y2 <= BottomEdge)) {
        y_position = VMID_BOX;
    } else if ( (y1 > BottomEdge) && (y2 > BottomEdge)) {
        y_position = BOT_BOX;
    } else {
        //
        //  We must straddle an edge in the vertical
        //
        return FALSE;
    }
    
    //
    //  Now figure out which box we are given the
    //  horizontal and vertical positions
    //
    if(x_position == LEFT_BOX) {
        switch(y_position) {
          case TOP_BOX:
            *type = XIL_AREA_TOP_LEFT_CORNER;
            break;
          case VMID_BOX:
            *type = XIL_AREA_LEFT_EDGE;
            break;
          case BOT_BOX:
            *type = XIL_AREA_BOTTOM_LEFT_CORNER;
            break;
        }
    } else if (x_position == HMID_BOX) {
        switch(y_position) {
          case TOP_BOX:
            *type = XIL_AREA_TOP_EDGE;
            break;
          case VMID_BOX:
            *type = XIL_AREA_CENTER;
            break;
          case BOT_BOX:
            *type = XIL_AREA_BOTTOM_EDGE;
            break;
        }
    } else {
        switch(y_position) {
          case TOP_BOX:
            *type = XIL_AREA_TOP_RIGHT_CORNER;
            break;
          case VMID_BOX:
            *type = XIL_AREA_RIGHT_EDGE;
            break;
          case BOT_BOX:
            *type = XIL_AREA_BOTTOM_RIGHT_CORNER;
            break;
        }
    }
    return TRUE;
    
}

//
// Make sure that the source storage never goes beyond
// the bounds of the storage for the source image. We
// also pass back how much of the image was clipped off
// for later use.
//
Xil_boolean
XilOpAreaKernel::clipSourceStorageBox(int* sx1,
				      int* sy1,
				      int* sx2,
				      int* sy2,
				      int* dx1,
				      int* dy1,
				      int* dx2,
				      int* dy2)
{
    Xil_boolean clipped = FALSE;
    
    if(*sx1 < (int)base_src_sx1) {
	*dx1 = base_src_sx1 - *sx1;
	*sx1 = base_src_sx1;
	clipped = TRUE;
    }

    if(*sy1 < (int)base_src_sy1) {
	*dy1 = base_src_sy1 - *dy1;
	*sy1 = base_src_sy1;
	clipped = TRUE;
    }

    if(*sx2 > (int)base_src_sx2) {
	*dx2 = *sx2 - base_src_sx2;
	*sx2 = base_src_sx2;
	clipped = TRUE;
    }

    if(*sy2 > (int)base_src_sy2) {
	*dy2 = *sy2 - base_src_sy2;
	*sy2 = base_src_sy2;
	clipped = TRUE;
    }
    return clipped;
}

//       
//  This method takes all of the box parameters, labels the
//  boxes, expands the storage based on the type, checks against
//  the storage.
//  The boxes generated here should only be XIL_BOX_AREA_TYPE
//  because they will get passed to the GPI.
//
XilStatus
XilOpAreaKernel::setupBoxes(XilBox*      src_box,
			    int          src_x1,
			    int          src_y1,
			    int          src_x2,
			    int          src_y2,
			    int          src_sx1,
			    int          src_sy1,
			    int          src_sx2,
			    int          src_sy2,
			    unsigned int src_band,
			    XilBox*      dst_box,
			    int          dst_x1,
			    int          dst_y1,
			    int          dst_x2,
			    int          dst_y2,
			    int          dst_sx1,
			    int          dst_sy1,
			    int          dst_sx2,
			    int          dst_sy2,
			    unsigned int dst_band)
{
    // Set up a few basic things
    XilBoxAreaType src_type;

    if (getValidBoxAreaType(source,
                            src_x1, src_y1,
                            src_x2, src_y2,
                            &src_type) == FALSE) {
        //
        //  Generate an error as setUpBoxes should only be called
        //  for valid boxes!
        //  TODO: Should we generate a secondary error?
        //
        return XIL_FAILURE;
    }

    XilBoxAreaType dst_type = src_type;
    
    //
    // Before storing the values check to see if its
    // one of the edge box types and if XIL_EDGE_EXTEND
    // is set then make the storage bigger.
    //
    int     n_sx1 = src_sx1;
    int     n_sy1 = src_sy1;
    int     n_sx2 = src_sx2;
    int     n_sy2 = src_sy2;
    int     dx1 = 0;
    int     dy1 = 0;
    int     dx2 = 0;
    int     dy2 = 0;

    //
    // Adjust the source storage based on the edge condition
    // and type of destination box.
    // While logically the source box would be extended for all
    // edge types, it isn't necessary because the compute routine
    // never uses the source box for the edges for zero_fill
    // and no_write. 
    //
    if(((edge_condition == XIL_EDGE_EXTEND) && edgeBox(src_type)) ||
	(src_type == XIL_AREA_CENTER)) {
	extendSourceStorageBox(src_type, &n_sx1, &n_sy1, &n_sx2, &n_sy2);
    }
    Xil_boolean clipped;

    // Also make sure to clip the storage box against the image bounds.
    clipped = clipSourceStorageBox(&n_sx1, &n_sy1, &n_sx2, &n_sy2,
				   &dx1, &dy1, &dx2, &dy2);

    if(clipped) {
	//
	// If the source storage was clipped we also don't
	// write those destination pixels. The storage has
	// already been clipped. In addition,
	//
	// Check after clipping if the destination box is
	// now bogus. ie by adding the diffs back in is x2 < x1
	// or y2 < y1, in which case don't generate the box.
	//
	dst_x1 += dx1;
	dst_y1 += dy1;
	dst_x2 -= dx2;
	dst_y2 -= dy2;
	if((dst_x2 < dst_x1) || (dst_y2 < dst_y1)) {
	    return XIL_FAILURE;
	}

	// Now go ahead and deal with the other pieces
	src_x1 += dx1;
	src_y1 += dy1;
	src_x2 -= dx2;
	src_y2 -= dy2;
	dst_sx1 += dx1;
	dst_sy1 += dy1;
	dst_sx2 -= dx2;
	dst_sy2 -= dy2;
    }

    // Set up the source box
    src_box->setAsCorners(src_x1, src_y1, src_x2, src_y2);
    src_box->setStorageAsCorners(n_sx1, n_sy1, n_sx2, n_sy2, src_band);
    src_box->setTag((void*)src_type);

    dst_box->setAsCorners(dst_x1, dst_y1, dst_x2, dst_y2);
    dst_box->setStorageAsCorners(dst_sx1, dst_sy1, dst_sx2, dst_sy2, dst_band);
    dst_box->setTag((void*)dst_type);
    return XIL_SUCCESS;
}

//
// Adjust the values of source storage based on the box
// type.
//
void
XilOpAreaKernel::extendSourceStorageBox(XilBoxAreaType type,
					int*        sx1,
					int*        sy1,
					int*        sx2,
					int*        sy2)
{
    switch(type) {
      case XIL_AREA_SMALL_SOURCE:
	// The box is the whole image, don't do anything
	break;
	
      case XIL_AREA_TOP_LEFT_CORNER:
	*sx1 = base_src_sx1;
	*sy1 = base_src_sy1;
	*sx2 += r_border;
	*sy2 += b_border;
	break;
	
      case XIL_AREA_TOP_EDGE:
	*sx1 -= l_border;
	*sy1 = base_src_sy1;
	*sx2 += r_border;
	*sy2 += b_border;
	break;
	
      case XIL_AREA_TOP_RIGHT_CORNER:
	*sx1 -= l_border;
	*sy1 = base_src_sy1;
	*sx2 = base_src_sx2;
	*sy2 += b_border;
	break;
	
      case XIL_AREA_LEFT_EDGE:
	*sx1 = base_src_sx1;
	*sy1 -= t_border;
	*sx2 += r_border;
	*sy2 += b_border;
	break;

      case XIL_AREA_CENTER:
	*sx1 -= l_border;
	*sy1 -= t_border;
	*sx2 += r_border;
	*sy2 += b_border;
	break;

      case XIL_AREA_RIGHT_EDGE:
	*sx1 -= l_border;
	*sy1 -= t_border;
	*sx2 = base_src_sx2;
	*sy2 += b_border;
	break;
	
      case XIL_AREA_BOTTOM_LEFT_CORNER:
	*sx1 = base_src_sx1;
	*sy1 -= t_border;
	*sx2 += r_border;
	*sy2 = base_src_sy2;
	break;
	    
      case XIL_AREA_BOTTOM_EDGE:
	*sx1 -= l_border;
	*sy1 -= t_border;
	*sx2 += r_border;
	*sy2 = base_src_sy2;
	break;

      case XIL_AREA_BOTTOM_RIGHT_CORNER:
	*sx1 -= l_border;
	*sy1 -= t_border;
	*sx2 = base_src_sx2;
	*sy2 = base_src_sy2;
	break;
    }
}

//
// Is the box passed an edge box that will be passed
// on to the compute routines.
//
Xil_boolean
XilOpAreaKernel::edgeBox(XilBoxAreaType type)
{
    if(type != XIL_AREA_CENTER) {
        return TRUE;
    }
    return FALSE;
}

//
// Do we need to split the box any further,
// this allows us to check for many edge cases that
// do not require any further action except to extend
// the source storage and label the box.
//
Xil_boolean
XilOpAreaKernel::needsSplitting(unsigned int src_x1,
				unsigned int src_y1,
				unsigned int src_x2,
				unsigned int src_y2,
				unsigned int dst_sx1,
				unsigned int dst_sy1,
				unsigned int dst_sx2,
				unsigned int dst_sy2)
{
    unsigned int w = (src_x2-src_x1) + 1;
    unsigned int h = (src_y2-src_y1) + 1;


    //
    // Is it a single pixel
    //
    if((w==1) && (h==1)) {
	return FALSE;
    }

    //
    //  Try and get the type of the source box. If it
    //  isn't valid, then we know it needs splitting.
    //
    XilBoxAreaType src_type;
    if(getValidBoxAreaType(source, src_x1, src_y1,
                           src_x2, src_y2,&src_type) != TRUE) {
        return TRUE;
    }
    //
    //  It's either fully within an edge/corner box, or it's
    //  an XIL_AREA_CENTER, or it's XIL_AREA_SMALL_SOURCE
    //

    if(src_type == XIL_AREA_SMALL_SOURCE) {
        return FALSE;
    }

    //
    // If it's entirely within a corner box don't split it
    //
    if( (src_type == XIL_AREA_TOP_LEFT_CORNER) || 
        (src_type == XIL_AREA_TOP_RIGHT_CORNER) ||
        (src_type == XIL_AREA_BOTTOM_LEFT_CORNER) ||
        (src_type == XIL_AREA_BOTTOM_RIGHT_CORNER) ) {
	return FALSE;
    }

    //
    // Now we know that we're an edge box or a center box.
    // Tile boundary checks, if there aren't tiles
    // in the destination no need to test.
    //
    if(dest->isTileSizeSet() == FALSE) {
	return FALSE;
    }

    // Compute tile boundaries
    unsigned int tile_xsize, tile_ysize;
    dest->getTileSize(&tile_xsize, &tile_ysize);

    int tile_x1 = (dst_sx1/tile_xsize)*tile_xsize;
    int tile_y1 = (dst_sy1/tile_ysize)*tile_ysize;
    int tile_x2 = tile_x1+tile_xsize-1;
    int tile_y2 = tile_y1+tile_ysize-1;

    //
    // If it's completely within a right or left edge box, check the
    // Y dimensions for touching tile edges.
    //
    if( (src_type == XIL_AREA_LEFT_EDGE) ||
        (src_type == XIL_AREA_RIGHT_EDGE)) {

        
	//
	// If the height of the  is the length of the tile needs splitting.
	//
	if(h >= tile_ysize) {
	    return TRUE;
	}

	//
	// If destination touches top edge of the tile
	//
	if(dst_sy1 < (tile_y1+t_border)) {
	    return TRUE;
	}

	//
	// Touches bottom edge of the tile
	//
	if(dst_sy2 > (tile_y2-b_border)) {
	    return TRUE;
	}

	// Failed all above tests no need to split
	return FALSE;
	
    }
    
    //
    // If it's completely within a top or bottom edge box, check the
    // X dimensions for touching tile edges.
    //
    if((src_type == XIL_AREA_TOP_EDGE) ||
       (src_type == XIL_AREA_BOTTOM_EDGE) ) {

	//
	// Height is the length of the tile needs splitting.
	//
	if(w >= tile_xsize) {
	    return TRUE;
	}

	//
	// Touches left edge of the tile
	//
	if(dst_sx1 < (tile_x1+l_border)) {
	    return TRUE;
	}

	//
	// Touches right edge of the tile
	//
	if(dst_sx2 > (tile_x2-r_border)) {
	    return TRUE;
	}
	
	// Failed all above tests no need to split
	return FALSE;
    }

    //
    // Check for a center box within a tile, need
    // to do X and Y checks together.
    //
    if(src_type == XIL_AREA_CENTER) {

	//
	// Width or height is length of the tile
	//
	if((w >= tile_xsize) || (h >= tile_ysize)) {
	    return TRUE;
	}

	//
	// X or Y checks for all sides of the tile
	//
	if((dst_sy1 < (tile_y1+t_border)) ||
	   (dst_sy2 > (tile_y2-b_border)) ||
	   (dst_sx1 < (tile_x1+l_border)) ||
	   (dst_sx2 > (tile_x2-r_border)) ) {
	    return TRUE;
	}

	return FALSE;
    }

    //
    // Ok if we got through all those tests we do need
    // to split the box.
    // CAN WE ACTUALLY GET HERE??
    //
    return TRUE;
}

