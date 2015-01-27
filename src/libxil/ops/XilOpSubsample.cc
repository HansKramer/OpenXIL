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
//  File:	XilOpSubsample.cc
//  Project:	XIL
//  Revision:	1.5
//  Last Mod:	10:07:54, 03/10/00
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
#pragma ident	"@(#)XilOpSubsample.cc	1.5\t00/03/10  "

#include "XilOpSubsample.hh"

//
//  generateIntersectedRoi() takes into account all of
//  the mappings between source and destination to generate the
//  intersectedRoi.
//
XilStatus
XilOpSubsample::generateIntersectedRoi()
{
    //
    //  Start with the source ROI to produce the intersected ROI 
    //
    XilRoi* sgs_roi = getSrcGlobalSpaceRoi(0);
    if(sgs_roi == NULL) {
        return XIL_FAILURE;
    }

    XilRoi* int_roi = getIntersectedRoi();
    if(int_roi == NULL) {
        return XIL_FAILURE;
    }

    //
    //  Forward map source ROI into destination and add those rects to the
    //  intersected ROI.  Then, we'll intersect it with the destination's
    //  ROI.
    //
    XiliRectInt* rect   = sgs_roi->getRectList();
    while(rect != NULL) {
        int          dst_x = _XILI_ROUND(rect->getX1() * xScale);
        int          dst_y = _XILI_ROUND(rect->getY1() * yScale);
        unsigned int dst_w = _XILI_ROUND((rect->getX2() - rect->getX1() + 1) * xScale);
        unsigned int dst_h = _XILI_ROUND((rect->getY2() - rect->getY1() + 1) * yScale);

        //
        //  Move onto the next rect.
        //
        rect = (XiliRectInt*)rect->getNext();

        //
        //  If it doesn't clip to nothing, add it to the intersected ROI.
        //
        if(dst_w <= 0 || dst_h <=0) {
            continue;
        }

        int_roi->addRect(dst_x, dst_y, dst_w, dst_h);
    }

    XilRoi* dgs_roi = getDstGlobalSpaceRoi();
    if(dgs_roi == NULL) {
        return XIL_FAILURE;
    }

    //
    //  Intersect with destination ROI
    //
    if(dgs_roi->intersect_inplace(int_roi) == XIL_FAILURE) {
        return XIL_FAILURE;
    }

    return XIL_SUCCESS;
}

//
//  Forward map the given source rectangle and provide a rectangle within
//  the destination that is the area required to process the given region in
//  the source.  For us, it's a forward map and then clip it against the
//  destination.
//
XilStatus
XilOpSubsample::gsForwardMap(XiliRect*    src_rect,
                             unsigned int ,
                             XiliRect*    dst_rect)
{
    XiliRectInt* src_irect = (XiliRectInt*)src_rect;
    XiliRectInt* dst_irect = (XiliRectInt*)dst_rect;

    int          dst_x = _XILI_ROUND(src_irect->getX1() * xScale);
    int          dst_y = _XILI_ROUND(src_irect->getY1() * yScale);
    unsigned int dst_w =
        _XILI_ROUND((src_irect->getX2() - src_irect->getX1() + 1) * xScale);
    unsigned int dst_h =
        _XILI_ROUND((src_irect->getY2() - src_irect->getY1() + 1) * yScale);

    dst_irect->set(dst_x, dst_y,
                   dst_x + dst_w - 1,
                   dst_y + dst_h - 1);

    return XIL_SUCCESS;
}

//
//  Backward map a rect in the destination into the source.
//
//  This is called by the core with a rect in global space
//  and hence we always clip against the source image in global
//  space.
//
XilStatus
XilOpSubsample::gsBackwardMap(XiliRect*    dst_rect,
                              XiliRect*    src_rect,
                              unsigned int )
{
    XiliRectInt* src_irect = (XiliRectInt*)src_rect;
    XiliRectInt* dst_irect = (XiliRectInt*)dst_rect;

    int          src_x = _XILI_ROUND(dst_irect->getX1() * xInvScale);
    int          src_y = _XILI_ROUND(dst_irect->getY1() * yInvScale);
    unsigned int src_w =
        _XILI_ROUND((dst_irect->getX2() - dst_irect->getX1() + 1 + xScale) * xInvScale);
    unsigned int src_h =
        _XILI_ROUND((dst_irect->getY2() - dst_irect->getY1() + 1 + yScale) * yInvScale);

    src_irect->set(src_x,
                   src_y,
                   src_x + src_w - 1,
                   src_y + src_h - 1);

    src_irect->clip(&srcGSRect);

    return XIL_SUCCESS;
}

//
//  Special version of divideBoxList which takes into account backward and
//  forward mapping.
//
//  And the fact that this called from the GPI side so boxes are in object
//  space. 
//
Xil_boolean
XilOpSubsample::divideBoxList(XilBoxList*   boxlist,
                              unsigned int  ,
                              unsigned int  tile_xdelta,
                              unsigned int  tile_ydelta)
{
//    return TRUE;

    //
    //  Get the box list entry from the boxlist.
    // 
    XiliSLList<XiliBoxListEntry*>* list = boxlist->getList();
    XiliBoxListEntry*              ble  = list->reference(list->head());

    //
    //  Get the source and destination box
    //
    XilBox* src_box = &ble->boxes[0];
    XilBox* dst_box = &ble->boxes[1];

    //
    //  Get the dst box values
    //
    int          bx;
    int          by;
    unsigned int bw;
    unsigned int bh;
    dst_box->getAsRect(&bx, &by, &bw, &bh);

    XiliRectInt  dst_box_rect(bx, by, bx+bw-1, by+bh-1);

    //
    //  Boolean to indicate if boxes are added
    //
    Xil_boolean add_boxes = FALSE;

    //
    //  Backward map an extended destination box into the source and process
    //  all the tiles in the source that fall within it.  This is necessary to
    //  deal with clamping and minor origin shifts.
    //
    int          src_x = _XILI_ROUND((bx - xScale - dst_ox) * xInvScale);
    int          src_y = _XILI_ROUND((by - yScale - dst_oy) * yInvScale);
    unsigned int src_w = (bw + xScale) * xInvScale;
    unsigned int src_h = (bh + yScale) * yInvScale;

    src_x += src_ox;
    src_y += src_oy;

    if(src_x < 0) {
        src_x = 0;
    }
    if(src_y < 0) {
        src_y = 0;
    }
    
    //
    //  Compute which tile we start iterating at and the size of the tile area
    //  we iterate over.
    //
    unsigned int xstart = src_x/tile_xdelta;
    unsigned int ystart = src_y/tile_ydelta;
    unsigned int xend   = (src_x + src_w - 1)/tile_xdelta;
    unsigned int yend   = (src_y + src_h - 1)/tile_ydelta;

    for(unsigned int j=ystart; j<=yend; j++) {
        for(unsigned int i=xstart; i<=xend; i++) {
            //
            //  Get the tile coordinates in global space for forward mapping.
            //
            int tx = i * tile_xdelta - src_ox;
            int ty = j * tile_ydelta - src_oy;
            int tw = tile_xdelta;
            int th = tile_ydelta;
            
            //
            //  Now we'll forward map the tile into the dest and clip
            //  it against the original box.  To generate the area to
            //  be processed for this tile.  The rounding is in XiliRectInt.
            //
            XiliRectInt dst_rect;

            dst_rect.setX1(floor(tx * xScale));
            dst_rect.setY1(floor(ty * yScale));
            dst_rect.setX2(dst_rect.getX1() + ceil(tw * xScale) - 1);
            dst_rect.setY2(dst_rect.getY1() + ceil(th * yScale) - 1);

            dst_rect.translate(dst_ox, dst_oy);

            if(! dst_rect.clip(&dst_box_rect)) {
                continue;
            }

            //
            //  Create a new box list entry to append to the list.
            //
            XiliBoxListEntry* new_ble = new XiliBoxListEntry;
            if(new_ble == NULL) {
                XIL_ERROR(dstImage->getSystemState(), XIL_ERROR_RESOURCE,
                          "di-1", TRUE);
                continue;
            }
            src_box = &new_ble->boxes[0];
            dst_box = &new_ble->boxes[1];

            //
            //  Now that we've got the destination region, we must align the
            //  start (0, 0) of the destination box with the (0, 0) of the
            //  source box which means the source box must live at coordinates
            //  equal to backward mapping the start and end of the destination
            //  box.  This may be larger than a single tile in some cases, but
            //  for integer scale down by factors of 2 in both x and y this
            //  should not occur. 
            //
            //  This is due to the fact that there really is a "kernel" a la
            //  bicubic occuring in these subsample operations, but we're not
            //  setup to handle the complex splitting like the geometric
            //  affine operations.
            //
            //  TODO: 5/27/97 jlf  Implement source tile boundary splitting
            //                     for adaptive source kernels correctly.
            //
            XiliRectInt src_rect((dst_rect.getX1() - dst_ox) * xInvScale,
                                 (dst_rect.getY1() - dst_oy)* yInvScale,
                                 ceil((dst_rect.getX2() - dst_ox) * xInvScale),
                                 ceil((dst_rect.getY2() - dst_oy) * yInvScale));

            src_rect.translate(src_ox, src_oy);

            //
            //  We use setBoxStorage() to ensure the proper storage
            //  setting.
            //
            if(setBoxStorage(&src_rect, srcImage, src_box) == XIL_FAILURE) {
                delete new_ble;
                continue;
            }

            if(setBoxStorage(&dst_rect, dstImage, dst_box) == XIL_FAILURE) {
                delete new_ble;
                continue;
            }

            //
            //  Set the TAG.
            //
            src_box->setTag((void*)XIL_GEOM_INTERNAL);
            dst_box->setTag((void*)XIL_GEOM_INTERNAL);

            //
            //  Insert the new boxes into the list.
            //
            if(list->append(new_ble) == _XILI_SLLIST_INVALID_POSITION) {
                delete new_ble;
                continue;
            }

            //
            //  We added boxes so indicate this.
            //
            add_boxes = TRUE;
        }
    }
        
    //
    //  Remove and delete the original box list entry.
    //
    if(add_boxes == TRUE) {
        XiliBoxListEntry* entry;
        list->remove(list->head(), entry);
        delete entry;
    }
 
    return TRUE;        
}

//------------------------------------------------------------------------
//
//  Function:	setBoxStorage()
//
//  Description:
//      Sets the storage information on a box for the given rect and
//      object.
//
//------------------------------------------------------------------------
XilStatus
XilOpSubsample::setBoxStorage(XiliRect*            rect,
                              XilDeferrableObject* object,
                              XilBox*              box)
{
    //
    //  Make certain the source box doesn't extend outside of the
    //  source...this is possible given we use integers which can cause
    //  small errors in the backward mapping.
    //
    if(object == srcImage) {
        if(rect->clip(&srcOSRect) == FALSE) {
            return XIL_FAILURE;
        }
    }

    *box = *rect;

    if(object->setBoxStorage(box) == XIL_FAILURE) {
        return XIL_FAILURE;
    }

    return XIL_SUCCESS;
}

//
// Backward map a single point
//
XilStatus
XilOpSubsample::vBackwardMap(XilBox*       dst_box,
                             double        dx,
                             double        dy,
                             XilBox*       src_box,
                             double*       sx,
                             double*       sy,
                             unsigned int  )
{
    //
    //  Move the destination coordinates into global space by first adjusting
    //  the destination coordinates by the box offsets to move them into image
    //  space and then subtracting the destination origin.  Then, multiply by
    //  the inverse scale factor and put it into source box space.
    //
    *sx = ((dx + (double)dst_box->getX() - dst_ox) * xInvScale) +
        src_ox - (double)src_box->getX();
    *sy = ((dy + (double)dst_box->getY() - dst_oy) * yInvScale) +
        src_oy - (double)src_box->getY();

    return XIL_SUCCESS;
}
