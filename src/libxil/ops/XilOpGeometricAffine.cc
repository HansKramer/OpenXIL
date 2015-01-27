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
//  File:	XilOpGeometricAffine.cc
//  Project:	XIL
//  Revision:	1.91
//  Last Mod:	10:07:42, 03/10/00
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
#pragma ident	"@(#)XilOpGeometricAffine.cc	1.91\t00/03/10  "

#include <string.h> 
#include <math.h>
#ifndef _WINDOWS
#include <values.h>
#endif
#include <xil/xilGPI.hh>

#include "XilOpGeometricAffine.hh"
#include "XiliConvexRegion.hh"

//
//  Used to get/set src, dst box entry numbers
//
const unsigned int XIL_GEOAFFINE_OP_SRC_BOX_ENTRY = 0;
const unsigned int XIL_GEOAFFINE_OP_DST_BOX_ENTRY = 1;

//
//  Amount to add to the right and bottom to get values flush to the next
//  tile.
//
const double _XILI_PIXEL_FLUSH = 1.0 - 1.0e-12;

XilOpGeometricAffine::XilOpGeometricAffine(XilOpNumber            op_num,
                                           XilImage*              src_image,
                                           XilImage*              dst_image,
                                           XiliInterpolationType  type,
                                           AffineTr               affine_tr,
                                           XilInterpolationTable* h_table,
                                           XilInterpolationTable* v_table) : 
    XilOpGeometric(op_num, src_image, dst_image, type, h_table, v_table)
{
    //
    //  Copy the transformation matrix.
    //
    affineTr = affine_tr;

    //
    //  Adjust the affine transformation matrix to account for our coordinate
    //  shift of 1/2 pixel that occurs when moving from coordinates to the
    //  center of pixels and back again.  For the non-nearest interpolation
    //  types, we do this.  For nearest, we don't need to because we can
    //  simply go after the truncated pixel instead of having to interpolate
    //  and the rounding is handled for us.
    //
    if(interpolationType != XiliNearest) {
        affineTr.tx -= (1 - affineTr.a - affineTr.b)/2;
        affineTr.ty -= (1 - affineTr.c - affineTr.d)/2;
    } else {
        affineTr.tx -= 0.5;
        affineTr.ty -= 0.5;
    }

    //
    //  Create the inverse affine transformation.
    //
    xili_invert(affineTr, &inverseTr);

    //
    //  Src global space representation of the images include the edges and
    //  represents the image AREA.
    //
    srcgs_X1 += leftEdge;
    srcgs_Y1 += topEdge;
    srcgs_X2 += rightEdge  + XILI_BOT_RT_EXTENT;
    srcgs_Y2 += bottomEdge + XILI_BOT_RT_EXTENT;

    //
    //  Src object space representation of the images include the edges and
    //  represents the image COORDINATES.
    //
    srcos_X1 += leftEdge;
    srcos_Y1 += topEdge;
    srcos_X2 += rightEdge;
    srcos_Y2 += bottomEdge;

    //
    //  Forward map the point we use to represent the open-endedness of the
    //  right and bottom edges of the image and tile and the start of the next
    //  tile edge into the destination to generate an estimage of how we're to
    //  adjust pixels in the destination that close the delta between two
    //  tiles.
    //
    Vertex v1(XILI_BOT_RT_EXTENT,
              XILI_BOT_RT_EXTENT);
    xili_affmap(&v1, affineTr);

    Vertex v2(1.0, 1.0);
    xili_affmap(&v2, affineTr);

    xPixelAdj = v2.x - v1.x;
    yPixelAdj = v2.y - v1.y;

    //
    //  Remove any trailing garbage and shrink slightly so as not to adjust
    //  too far...
    //
    if(xPixelAdj < 0.0) {
        xPixelAdj += 1.0e-12;
    } else {
        xPixelAdj -= 1.0e-12;
    }

    if(yPixelAdj < 0.0) {
        yPixelAdj += 1.0e-12;
    } else {
        yPixelAdj -= 1.0e-12;
    }

    //
    //  Pointer to hold the clipped src ROI when it's non-nearest.
    //
    clippedSrcRoi        = NULL;

    //
    //  Pointer to hold the extended src ROI when it's a molecule.
    //
    extentMoleculeSrcROI = NULL;
}

XilOpGeometricAffine::~XilOpGeometricAffine()
{
    //
    //  Clean up our list of regions that were stored on the boxes.
    //
    while(regionList.head() != _XILI_SLLIST_INVALID_POSITION) {
        XiliConvexRegion* region;
        regionList.remove(regionList.head(), region);
        delete region;
    }

    //
    //  If we created a clippedSrcRoi, destroy it.
    //
    if(clippedSrcRoi != NULL) {
        clippedSrcRoi->destroy();
    }

    //
    //  If we created an extentMoleculeSrcROI, destroy it.
    //
    if(extentMoleculeSrcROI != NULL) {
        extentMoleculeSrcROI->destroy();
    }
}

//
//  generateIntersectedRoi() takes into account all of
//  the mappings between source and destination to generate the
//  intersectedRoi.
//
XilStatus
XilOpGeometricAffine::generateIntersectedRoi()
{
    //
    //  Intersect with the source ROI to produce the intersected ROI
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
    //  Copy the SRC ROI into the intersected ROI and then forward map the
    //  intersected ROI for intersection with the destination ROI.
    //
    *int_roi = *sgs_roi;

    //
    //  Forward map intersected ROI into destination
    //
    if(sgs_roi->numRegions() == 1) {
        XiliConvexRegion* cr = int_roi->getConvexRegion();

        if(cr == NULL) {
            return XIL_FAILURE;
        }

        forwardMapCR(cr);
    } else {
        XiliList<XiliConvexRegion>* rlist =
            int_roi->getConvexRegionList();

        if(rlist == NULL) {
            return XIL_FAILURE;
        }

        XiliListIterator<XiliConvexRegion> li(rlist);
        XiliConvexRegion* cr;
        while((cr = li.getNext()) != _XILI_LIST_INVALID_POSITION) {
            forwardMapCR(cr);
        }
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

//------------------------------------------------------------------------
//
//  Function:   getSrcGlobalSpaceRoi()/getDstGlobalSpaceRoi()
//
//  Description:
//  Gets the global space ROI for the specified source or
//      destination.  It may not be what is on the deferrable object
//  when molecules are concered.  It may be the intersected ROI
//      from another operation.
// 
//------------------------------------------------------------------------
XilRoi*
XilOpGeometricAffine::getSrcGlobalSpaceRoi(unsigned int src_num)
{
    //
    //  For molecule support, we call the base op's routine and see if it
    //  returns the src image's ROI or something else.  If it's something
    //  other than the src image's ROI, we create a new ROI and extend it.
    //
    XilRoi* src_roi = XilOp::getSrcGlobalSpaceRoi(src_num);
    if(src_roi != srcImage->getGlobalSpaceRoi()) {
        //
        //  Molecular case.  We need to take the ROI the op provided us and
        //  extend it.
        //
        if(src_roi->isConvexRegionValid()) {
            //
            //  It's a convex region ROI so we can just return it.
            //
            return src_roi;
        } else if(extentMoleculeSrcROI == NULL) {
            //
            //  It's a rect ROI so we'll need to translate and extend it into
            //  a new ROI.
            //
            extentMoleculeSrcROI = dstImage->getSystemState()->createXilRoi();
            if(extentMoleculeSrcROI == NULL) {
                XIL_ERROR(dstImage->getSystemState(), XIL_ERROR_RESOURCE,
                          "di-1", TRUE);
                return NULL;
            }

            //
            //  Add extended rects to our new extended ROI.
            //
            XiliRectInt* rl = src_roi->getRectList();
            while(rl) {
                XiliRectDbl      dr(rl->getX1(),
                                    rl->getY1(),
                                    rl->getX2() + XILI_BOT_RT_EXTENT,
                                    rl->getY2() + XILI_BOT_RT_EXTENT);
                XiliConvexRegion cr(&dr);

                extentMoleculeSrcROI->addConvexRegion(&cr);

                rl = (XiliRectInt*)rl->getNext();
            }
        }

        //
        //  We now have an extended ROI that represents the area provided
        //  by XilOp::getSrcGlobalSpaceRoi().  Just set src_roi to point
        //  at this and it will be destroyed in the destructor.
        //
        src_roi = extentMoleculeSrcROI;
    } else {
        src_roi = srcImage->getExtentGlobalSpaceRoi();
    }

    //
    //  If we're an interpolation type other than nearest, then the area in
    //  the source is reduced due to the lack of storage for the higher-order
    //  interpolation types.
    //
    //  So, we need to clip our source ROI by the smaller edges.
    //
    if(interpolationType != XiliNearest) {
        if(clippedSrcRoi == NULL) {
            clippedSrcRoi = dstImage->getSystemState()->createXilRoi();
            if(clippedSrcRoi == NULL) {
                XIL_ERROR(dstImage->getSystemState(), XIL_ERROR_RESOURCE,
                          "di-1", TRUE);
                return NULL;
            }

            if(clippedSrcRoi->addRect(srcgs_X1,
                                      srcgs_Y1,
                                      srcgs_X2 - srcgs_X1 + 1,
                                      srcgs_Y2 - srcgs_Y1 + 1) == XIL_FAILURE) {
                return NULL;
            }

            if(src_roi->intersect_inplace(clippedSrcRoi) == XIL_FAILURE) {
                return NULL;
            }
        }

        //
        //  clippedSrcROI will be destroyed in the destructor.
        //
        src_roi = clippedSrcRoi;
    }

    return src_roi;
}

XilRoi*
XilOpGeometricAffine::getDstGlobalSpaceRoi(unsigned int)
{
    //
    //  Need to get/represent the area represented by the image.
    //
    return dstImage->getExtentGlobalSpaceRoi();
}

//
//  In this implementation of divideBoxList, the following are the
//  sequence of steps
//    1. Backward map the destination box
//       to get the corresponding source ConvexRegion
//    2. Determine which source tiles intersect this convex region.
//    3. Forward map each intersecting tile and clip it to the destination
//       box.
//    4. Set the resulting convex region on a new destination box that is used
//       by XilConvexRegionList to perform an exact intersection.
//
//  There should only ever be one box list entry on the incoming box list.
//
Xil_boolean
XilOpGeometricAffine::divideBoxList(XilBoxList*   boxlist,
                                    unsigned int  ,
                                    unsigned int  tile_xdelta,
                                    unsigned int  tile_ydelta)
{
    //
    //  Get the box list entry from the boxlist.
    // 
    XiliSLList<XiliBoxListEntry*>* list = boxlist->getList();
    XiliBoxListEntry*              ble  = list->reference(list->head());

    //
    //  Get the source and destination box
    //
    XilBox* src_box = &ble->boxes[XIL_GEOAFFINE_OP_SRC_BOX_ENTRY];
    XilBox* dst_box = &ble->boxes[XIL_GEOAFFINE_OP_DST_BOX_ENTRY];

    //
    //  Get the dst box values
    //
    int          bx;
    int          by;
    unsigned int bw;
    unsigned int bh;
    dst_box->getAsRect(&bx, &by, &bw, &bh);

    //
    //  Boolean to indicate if boxes are added and a pointer to hold the new
    //  box list entry we create.
    //
    Xil_boolean       add_boxes = FALSE;
    XiliBoxListEntry* new_ble   = NULL;

    //
    //  Backward map an extended destination box into the source and process
    //  all the tiles in the source that fall within it.
    //
    XiliConvexRegion src_cr(((double)bx) - dst_ox - _XILI_PIXEL_FLUSH,
                            ((double)by) - dst_oy - _XILI_PIXEL_FLUSH,
                            ((double)(bx+bw-1)) - dst_ox + _XILI_PIXEL_FLUSH,
                            ((double)(by+bh-1)) - dst_oy + _XILI_PIXEL_FLUSH);

    backwardMapCR(&src_cr);

    src_cr.translate(src_ox, src_oy);

    //
    //  Grow the box based on the interpolation type edges since we need to
    //  consider the entire storage area.
    //
    unsigned int src_bbox_x1 =
        (src_cr.lowX < 0.0)  ? 0 : (unsigned int)floor(src_cr.lowX - leftEdge);
    unsigned int src_bbox_y1 =
        (src_cr.lowY < 0.0)  ? 0 : (unsigned int)floor(src_cr.lowY - topEdge);
    unsigned int src_bbox_x2 =
        (src_cr.highX < 0.0) ? 0 : (unsigned int)ceil(src_cr.highX - rightEdge);
    unsigned int src_bbox_y2 =
        (src_cr.highY < 0.0) ? 0 : (unsigned int)ceil(src_cr.highY - bottomEdge);

    //
    //  Must clip to the bottom/right edge of the source image -- implicit
    //  clip of left/top above.
    //
    src_bbox_x2 = _XILI_MIN(src_bbox_x2, srcImage->getWidth());
    src_bbox_y2 = _XILI_MIN(src_bbox_y2, srcImage->getHeight());

    //
    //  Compute which tile we start iterating at and the size of the tile area
    //  we iterate over.
    //
    unsigned int xstart = src_bbox_x1/tile_xdelta;
    unsigned int ystart = src_bbox_y1/tile_ydelta;
    unsigned int xend   = src_bbox_x2/tile_xdelta;
    unsigned int yend   = src_bbox_y2/tile_ydelta;

    for(unsigned int j=ystart; j<=yend; j++) {
        for(unsigned int i=xstart; i<=xend; i++) {
            //
            //  Get the tile coordinates in image space -- flush to the
            //  next source tile.
            //
            int    tx1    = i * tile_xdelta;
            int    ty1    = j * tile_ydelta;
            int    tx2    = tx1 + tile_xdelta - 1;
            int    ty2    = ty1 + tile_ydelta - 1;

            double tx1_gs = tx1 - src_ox;
            double ty1_gs = ty1 - src_oy;
            double tx2_gs = tx2 - src_ox;
            double ty2_gs = ty2 - src_oy;

            //
            //  Set how many regions we'll be forward mapping into the
            //  destination for this tile based on the interpolation type.
            //
            unsigned int num_regions =
                (interpolationType == XiliNearest) ? 1 : 4;

            //
            //  We mark the boxes with a tag to indicate how they
            //  correspond to a region in the source that straddles multiple
            //  tiles or whether it's for just one tile.
            //
            XilBoxGeomType region_tag;

            //
            //  Run through our regions for this tile.
            //
            for(unsigned int r=0; r<num_regions; r++) {
                //
                //  Set the tile as a convexRegion in global space -- extend the
                //  edges per the interpolation type.
                //
                XiliConvexRegion tile_region;
                if(interpolationType == XiliNearest) {
                    tile_region.set(tx1_gs - XILI_TOP_LF_EXTENT,
                                    ty1_gs - XILI_TOP_LF_EXTENT,
                                    tx2_gs + XILI_BOT_RT_EXTENT,
                                    ty2_gs + XILI_BOT_RT_EXTENT);
                    region_tag = XIL_GEOM_INTERNAL;
                } else {
                    switch(r) {
                      case 0:
                        //
                        //  Center Region
                        //
                        tile_region.set(tx1_gs + leftEdge,
                                        ty1_gs + topEdge,
                                        tx2_gs - 1.0 + rightEdge + _XILI_PIXEL_FLUSH,
                                        ty2_gs - 1.0 + bottomEdge + _XILI_PIXEL_FLUSH);
                        region_tag = XIL_GEOM_INTERNAL;
                        break;

                      case 1:
                        //
                        //  Right Edge Split
                        //
                        tile_region.set(tx2_gs - 1.0 + rightEdge,
                                        ty1_gs + topEdge,
                                        tx2_gs + leftEdge + _XILI_PIXEL_FLUSH,
                                        ty2_gs - 1.0 + bottomEdge + _XILI_PIXEL_FLUSH);
                        region_tag = XIL_GEOM_VERTICAL;
                        break;

                      case 2:
                        //
                        //  Bottom Edge Split
                        //
                        tile_region.set(tx1_gs + leftEdge,
                                        ty2_gs - 1.0 + bottomEdge,
                                        tx2_gs - 1.0 + rightEdge + _XILI_PIXEL_FLUSH,
                                        ty2_gs + topEdge + _XILI_PIXEL_FLUSH);
                        region_tag = XIL_GEOM_HORIZONTAL;
                        break;

                      case 3:
                        //
                        //  Right Corner Split
                        //
                        tile_region.set(tx2_gs - 1.0 + rightEdge,
                                        ty2_gs - 1.0 + bottomEdge,
                                        tx2_gs + leftEdge + _XILI_PIXEL_FLUSH,
                                        ty2_gs + topEdge + _XILI_PIXEL_FLUSH);
                        region_tag = XIL_GEOM_CORNERS;
                        break;
                    }
                }

                //
                //  Now we'll forward map the tile into the dest and clip
                //  it against the original box.  To generate the area to
                //  be processed for this tile.
                //
                XiliConvexRegion dst_region(&tile_region);
                forwardMapCR(&dst_region);

                //
                //  We'll be attaching the destination convexRegion to the
                //  box so it can be used for intersection...of course,
                //  after translating it into image space and clipping it
                //  against the destination box.
                //
                dst_region.translate(dst_ox, dst_oy);

                if(dst_region.clip(bx, by, bx+bw-1, by+bh-1) == XIL_FAILURE) {
                    continue;
                }

                //
                //  Compute the pixel adjacencies to take into account.
                //
                double x1_pix_adj;
                double y1_pix_adj;
                double x2_pix_adj;
                double y2_pix_adj;
                if(xPixelAdj < 0.0) {
                    x1_pix_adj = xPixelAdj;
                    x2_pix_adj = XILI_PIXEL_ADJACENCY;
                } else {
                    x1_pix_adj = -XILI_PIXEL_ADJACENCY;
                    x2_pix_adj = xPixelAdj;
                }
                if(yPixelAdj < 0.0) {
                    y1_pix_adj = yPixelAdj;
                    y2_pix_adj = XILI_PIXEL_ADJACENCY;
                } else {
                    y1_pix_adj = -XILI_PIXEL_ADJACENCY;
                    y2_pix_adj = yPixelAdj;
                }

                //
                //  Get what will become the destination box and clip against
                //  it.
                //
                XiliRectDbl dst_rect(ceil(dst_region.lowX + x1_pix_adj),
                                     ceil(dst_region.lowY + y1_pix_adj),
                                     floor(dst_region.highX + x2_pix_adj),
                                     floor(dst_region.highY + y2_pix_adj));
                if(dst_region.clip(&dst_rect) == XIL_FAILURE) {
                    continue;
                }

                //
                //  If there's not one outstanding, create a new box list
                //  entry to append to the list. 
                //
                if(new_ble == NULL) {
                    new_ble = new XiliBoxListEntry;
                    if(new_ble == NULL) {
                        XIL_ERROR(dstImage->getSystemState(), XIL_ERROR_RESOURCE,
                                  "di-1", TRUE);
                        return FALSE;
                    }
                }

                src_box = &new_ble->boxes[XIL_GEOAFFINE_OP_SRC_BOX_ENTRY];
                dst_box = &new_ble->boxes[XIL_GEOAFFINE_OP_DST_BOX_ENTRY];

                //
                //  We use setBoxStorage() to ensure the proper storage
                //  setting.
                //
                XiliRectDbl src_rect;
                if(interpolationType == XiliNearest) {
                    src_rect.set(tx1, ty1, tx2, ty2);
                } else {
                    src_rect.set(tile_region.lowX,
                                 tile_region.lowY,
                                 tile_region.highX,
                                 tile_region.highY);
                    src_rect.translate(src_ox, src_oy);
                }

                //
                //  Attach the convex region to our destination box.
                //
                if(updateBoxConvexRegion(dst_box, &dst_region) == XIL_FAILURE) {
                    continue;
                }

                //
                //  Set the TAG.
                //
                src_box->setTag((void*)region_tag);
                dst_box->setTag((void*)region_tag);

                if(setBoxStorage(&src_rect, srcImage, src_box) == XIL_FAILURE) {
                    continue;
                }

                if(setBoxStorage(&dst_rect, dstImage, dst_box) == XIL_FAILURE) {
                    continue;
                }

                //
                //  Insert the new boxes into the list.
                //
                if(list->append(new_ble) == _XILI_SLLIST_INVALID_POSITION) {
                    continue;
                }

                //
                //  We added boxes so indicate this and clear out new_ble so
                //  we create a new one next time.
                //
                add_boxes = TRUE;
                new_ble   = NULL;
            }
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

    //
    //  Do we have an unused box list entry?
    //
    if(new_ble != NULL) {
        delete new_ble;
    }
    
    return TRUE;        
}

//
//  Given the rect for the given source or destination image, set the area of
//  storage required for the rect.  In the source, this means taking edges
//  into account.
//
XilStatus
XilOpGeometricAffine::setBoxStorage(XiliRect*            rect,
                                    XilDeferrableObject* object,
                                    XilBox*              box)
{
    if(object == srcImage) {
        //
        //  Clip against the source image to see if we need to process this
        //  area at all. Note we reduce the image size to the interior area
        //  depending on the interpolation type. 
        //
        XiliRectDbl tmp_rect(rect);

        //
        //  Clip the source rect against the available source.
        //
        XiliRectDbl src_extent(srcos_X1,
                               srcos_Y1,
                               srcos_X2,
                               srcos_Y2);
        if(tmp_rect.clip(&src_extent) == FALSE) {
            return XIL_FAILURE;
        }

        //
        //  Using the rect --> box converter does not work for the source
        //  because we need to get all the pixels that consume the rect.  The
        //  rect may land between pixels which is still valid.
        //
        {
            int sx1 = (int)(tmp_rect.getX1());
            int sy1 = (int)(tmp_rect.getY1());
            int sx2 = (int)ceil(tmp_rect.getX2());
            int sy2 = (int)ceil(tmp_rect.getY2());

            box->setAsCorners(sx1, sy1, sx2, sy2);
        }
        
        //
        //  A source box MAY shrink to nothing and we have to detect this case.
        //
        if(box->isEmpty()) {
            return XIL_FAILURE;
        }

        //
        //  Call the default routine to update the storage area on the box.
        //
        if(srcImage->setBoxStorage(box) == XIL_FAILURE) {
            return XIL_FAILURE;
        }

        //
        //  Finally, expand the storage to be the larger area for non-nearest
        //  interpolation types.
        //
        if(interpolationType != XiliNearest) {
            int          sx;
            int          sy;
            unsigned int sw;
            unsigned int sh;
            int          sband;
            box->getStorageLocation(&sx, &sy, &sw, &sh, &sband);

            //
            //  Compute new locations by expanding the region we shrunk before
            //  by subtracting the same edges we added.
            //
            sx -= (int)leftEdge;
            sy -= (int)topEdge;
            sw -= (int)rightEdge;
            sh -= (int)bottomEdge;

            box->setStorageLocation(sx, sy, sw, sh, sband);
        }
    } else {
        //
        //  Using the rect --> box converter works for the destination because
        //  we're looking for the integer pixels that fall in the rect (within
        //  pixel adjacency)
        //
        *box = *rect;

        //
        //  Since we're picking the integer coordinates inside the rect, it
        //  can collapse to an empty box. 
        //
        if(box->isEmpty()) {
            return XIL_FAILURE;
        }

        //
        //  Call the default routine to update the storage area on the box.
        //
        if(dstImage->setBoxStorage(box) == XIL_FAILURE) {
            return XIL_FAILURE;
        }

        //
        //  We need to propagate the pixel adjacency information in the form
        //  of attaching a convex region to the box so the scanline conversion
        //  code will use the correct adjacency information if it doesn't
        //  already have one.
        //
        if(box->getPrivateData() == NULL) {
            int sx1;
            int sy1;
            int sx2;
            int sy2;
            box->getAsCorners(&sx1, &sy1, &sx2, &sy2);
            
            XiliConvexRegion cr(sx1, sy1, sx2, sy2);
            if(updateBoxConvexRegion(box, &cr) == XIL_FAILURE) {
                return XIL_FAILURE;
            }
        }
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
XilOpGeometricAffine::gsForwardMap(XiliRect*    src_rect,
                                   unsigned int ,
                                   XiliRect*    dst_rect)
{
    XiliConvexRegion region(src_rect);

    if(interpolationType != XiliNearest) {
        //
        //  We need to clip the given rect against the source image since it may
        //  effectively shrink for non-nearest interpolation types.
        //
        region.clip(srcgs_X1,
                    srcgs_Y1,
                    srcgs_X2,
                    srcgs_Y2);
    }

    forwardMapCR(&region);

    if(region.clip(dstgs_X1, dstgs_Y1, dstgs_X2, dstgs_Y2) == XIL_FAILURE) {
        return XIL_FAILURE;
    }

    dst_rect->set(region.lowX,  region.lowY,
                  region.highX, region.highY);
    
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
XilOpGeometricAffine::gsBackwardMap(XiliRect*    dst_rect,
                                    XiliRect*    src_rect,
                                    unsigned int )
{
    double x1;
    double y1;
    double x2;
    double y2;
    dst_rect->get(&x1, &y1, &x2, &y2);

    //
    //  Map the resulting rectangle from the destintion into the source using
    //  the inverse affine transform.
    //
    XiliConvexRegion src_region(x1, y1, x2, y2);
    backwardMapCR(&src_region);

    //
    //  Note that the source image is being modified to take into account
    //  image edge conditions for the different interpolation types.
    //
    //  For nearest interpolation, the source is considered to have a
    //  half-pixel extent around the image to account for our pixel coordinate
    //  system which has the center of the pixel at (0.0, 0.0).
    //
    //  For the others, we clip to the edges which are set based upon the
    //  interpolation type.
    //    
    if(src_region.clip(srcgs_X1,
                       srcgs_Y1,
                       srcgs_X2,
                       srcgs_Y2) == XIL_FAILURE) {
        return XIL_FAILURE;
    }

    //
    //  Set the source rect from the convex region bounding box which is the
    //  best estimation of what rectangle a rectangle in the destination maps
    //  to in the source.  
    //
    src_rect->set(src_region.lowX,  src_region.lowY,
                  src_region.highX, src_region.highY);

    return XIL_SUCCESS;
}

//
//  Backward map a single point in the destination box to the corresponding
//  point in the source box.
//
XilStatus
XilOpGeometricAffine::vBackwardMap(XilBox*       dst_box,
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
    //  space and then subtracting the destination origin.  We're filling in
    //  the vertex information for the xili_affmap() utility routine.
    //
    Vertex v(dx + (double)dst_box->getX() - dst_ox,
             dy + (double)dst_box->getY() - dst_oy);

    // 
    //  Perform the inverse affine transform.
    //
    xili_affmap(&v, inverseTr);

    //
    //  Convert back into source box space.
    //
    *sx = v.x + src_ox - (double)src_box->getX();
    *sy = v.y + src_oy - (double)src_box->getY();

    return XIL_SUCCESS;
}

//
// Private methods
//

//
//  Perform mapping from one convexregion to another convex region using the
//  forward affine transform.
// 
XilStatus
XilOpGeometricAffine::forwardMapCR(XiliConvexRegion* cr)
{
    //
    //  For each co-ordinate in the convexRegion map
    //  it onto another convex region using the supplied
    //  affine transform. The utilities use the vertex
    //  structure to help us do this.
    //

    //
    // Initialize lowX, lowY, highX, highY values
    //
    cr->lowX = XIL_MAXDOUBLE;
    cr->lowY = XIL_MAXDOUBLE;
    cr->highX = -XIL_MAXDOUBLE;
    cr->highY = -XIL_MAXDOUBLE;

    //
    // Do the mapping
    // 
    for(unsigned int i=0; i<cr->pointCount; i++) {
        Vertex v(cr->xPtArray[i], cr->yPtArray[i]);

        xili_affmap(&v, affineTr);

        //
        //  Copy vertexes into the convex region
        //
        cr->xPtArray[i] = v.x;
        cr->yPtArray[i] = v.y;

        //
        //  Set up the bounding box
        //
        cr->lowX  = _XILI_MIN(cr->lowX, cr->xPtArray[i]);
        cr->lowY  = _XILI_MIN(cr->lowY, cr->yPtArray[i]);
        cr->highX = _XILI_MAX(cr->highX, cr->xPtArray[i]);
        cr->highY = _XILI_MAX(cr->highY, cr->yPtArray[i]);
    }

    //
    //  The forward mapping may have caused the points to become out of order
    //  based on our definition of a convex region (clockwise), we must
    //  reorder them so the convex region handling code works. 
    //
    reorder(cr);

    return XIL_SUCCESS;
}

//
//  Perform mapping from one convexregion to another convex region using the
//  forward affine transform.
// 
XilStatus
XilOpGeometricAffine::backwardMapCR(XiliConvexRegion* cr)
{
    //
    //  For each co-ordinate in the convexRegion map
    //  it onto another convex region using the supplied
    //  affine transform. The utilities use the vertex
    //  structure to help us do this.
    //

    //
    // Initialize lowX, lowY, highX, highY values
    //
    cr->lowX = XIL_MAXDOUBLE;
    cr->lowY = XIL_MAXDOUBLE;
    cr->highX = -XIL_MAXDOUBLE;
    cr->highY = -XIL_MAXDOUBLE;

    //
    // Do the mapping
    // 
    for(unsigned int i=0; i<cr->pointCount; i++) {
        Vertex v(cr->xPtArray[i], cr->yPtArray[i]);

        xili_affmap(&v, inverseTr);

        //
        //  Copy vertexes into the convex region
        //
        cr->xPtArray[i] = v.x;
        cr->yPtArray[i] = v.y;

        //
        //  Set up the bounding box
        //
        cr->lowX  = _XILI_MIN(cr->lowX, cr->xPtArray[i]);
        cr->lowY  = _XILI_MIN(cr->lowY, cr->yPtArray[i]);
        cr->highX = _XILI_MAX(cr->highX, cr->xPtArray[i]);
        cr->highY = _XILI_MAX(cr->highY, cr->yPtArray[i]);
    }

    //
    //  The backward mapping may have caused the points to become out of order
    //  based on our definition of a convex region (clockwise), we must
    //  reorder them so the convex region handling code works. 
    //
    reorder(cr);

    return XIL_SUCCESS;
}

//
// Stores the convex region associated with a box
// as a private data pointer on the box.
//
XilStatus
XilOpGeometricAffine::updateBoxConvexRegion(XilBox*           box,
                                            XiliConvexRegion* region)
{
    XiliConvexRegion* regionPtr;

    //
    //  Update the information on the new valid box.
    //
    regionPtr = (XiliConvexRegion*)box->getPrivateData();
    
    if(regionPtr == NULL) {
        //
        //  Need to create a new region to be stored with the box.
        //
        regionPtr = new XiliConvexRegion(region);
        if(regionPtr == NULL) {
            XIL_ERROR(dstImage->getSystemState(), XIL_ERROR_RESOURCE, "di-1", TRUE);
            return XIL_FAILURE;
        }

        //
        //  Multiple threads may be calling splitOnTileBoundaries() which
        //  calls divideBoxList() which calls us.  The only shared data
        //  accessed in this chain between threads is the regionList so we
        //  must mutex lock around appending to the regionList.
        //
        regionListMutex.lock();

        if(regionList.append(regionPtr) == NULL) {
            return XIL_FAILURE;
        }

        regionListMutex.unlock();

        box->setPrivateData(regionPtr);
    } else {
        //
        //  Just update the contents of the existing region.
        //
        *regionPtr = *region;
    }

    //
    //  Set the pixel adjacencies used in scan converting...
    //
    regionPtr->xPixelAdj = xPixelAdj;
    regionPtr->yPixelAdj = yPixelAdj;
    
    return XIL_SUCCESS;
}

//
//  Re-order a ConvexRegion.
//
//  It is possible (due to backwardMap & forwardMap) that the convexRegion is
//  not ordered as required (a clockwise list of points).
//
void
XilOpGeometricAffine::reorder(XiliConvexRegion* in_cr)
{
    //
    //  Check and see if the convexRegion is already ordered clockwise.
    //
    //  Get the first 3 vertices of the convexRegion
    //
    double x1 = in_cr->xPtArray[0];
    double y1 = in_cr->yPtArray[0];
    
    double x2 = in_cr->xPtArray[1];
    double y2 = in_cr->yPtArray[1];
    
    double x3 = in_cr->xPtArray[2];
    double y3 = in_cr->yPtArray[2];

    double x12 = x2 - x1;
    double y12 = y2 - y1;
    double x13 = x3 - x1;
    double y13 = y3 - y1;

    double det = (x12 * y13) - (x13 * y12);

    if(det > 0.0) {
        //
        // It's clockwise
        //
        return;
    }

    if(det == 0.0) {
        //
        // So far its a line.
        // To confirm that it's truly a line we'll have
        // check again using the next 3 points.
        // If it turns out that the next 3 points too
        // are a line, the we'll assume that the convexRegion
        // is indeed a line
        //
        
        //
        // Get the first 3 vertices of the convexRegion
        //
        x1 = in_cr->xPtArray[1];
        y1 = in_cr->yPtArray[1];
    
        x2 = in_cr->xPtArray[2];
        y2 = in_cr->yPtArray[2];
    
        x3 = in_cr->xPtArray[3];
        y3 = in_cr->yPtArray[3];

        x12 = x2 - x1;
        y12 = y2 - y1;
        x13 = x3 - x1;
        y13 = y3 - y1;

        det = (x12 * y13) - (x13 * y12);

        if (det >= 0.0) {
            //
            // It's clockwise if > 0 and is a line if == 0.
            // Return in bith cases
            //
            return;
        }
    }

    
    //
    // So the ConvexRegion is ordered counterclockwise.
    //
    XiliConvexRegion tmp_cr;
    unsigned int locn = 0;
    
    //
    //  Do the reordering
    //

    //
    // These values do not change
    //
    tmp_cr.pointCount = in_cr->pointCount;
    tmp_cr.lowX       = in_cr->lowX;
    tmp_cr.lowY       = in_cr->lowY;
    tmp_cr.highX      = in_cr->highX;
    tmp_cr.highY      = in_cr->highY;
    
    //
    // Set the first point to the lowest x coordinate value
    //
    tmp_cr.xPtArray[0] = in_cr->lowX;

    //
    // Find the corresponding y value
    //
    for(unsigned int i=0; i < tmp_cr.pointCount; i++) {
        if(in_cr->xPtArray[i] == tmp_cr.xPtArray[0]) {
            //
            // Set the corresponding y value
            //
            tmp_cr.yPtArray[0] = in_cr->yPtArray[i];
            locn = i;
            break;
        }
    }

    //
    // Now reorder the other values of the temporary convexRegion
    // assuming clockwise walking.
    //
    
    //
    // Walk backwards through the given convexRegion (in_cr)
    //
    unsigned int m = 1;
    unsigned int j = locn;
    while (j > 0) {
        tmp_cr.xPtArray[m] = in_cr->xPtArray[j-1];
        tmp_cr.yPtArray[m] = in_cr->yPtArray[j-1];
        m++;
        j--;
    }

    unsigned int k = tmp_cr.pointCount - 1;
    while (k > locn) {
        tmp_cr.xPtArray[m] = in_cr->xPtArray[k];
        tmp_cr.yPtArray[m] = in_cr->yPtArray[k];
        m++;
        k--;
    }

    //
    // Return the reordered convexRegion
    //
    *in_cr = tmp_cr;
}
