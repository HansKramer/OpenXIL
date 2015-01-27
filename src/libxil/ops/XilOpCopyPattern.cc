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
//  File:	XilOpCopyPattern.cc
//  Project:	XIL
//  Revision:	1.34
//  Last Mod:	10:07:46, 03/10/00
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
#pragma ident	"@(#)XilOpCopyPattern.cc	1.34\t00/03/10  "

#include <stdlib.h>

#include <xil/xilGPI.hh>
#include "XilOpPoint.hh"
#include "XilOpCopy.hh"
#include "XiliOpUtils.hh"

class XilOpCopyPattern : public XilOpPoint {
public:
    static XilOp* create(char* function_name,
                         void* args[],
                         int count);
protected:
    XilOpCopyPattern(XilOpNumber op_num,
		     XilImage*   src,
		     XilImage*   dst);
    virtual ~XilOpCopyPattern();

    //
    //  We need to have a special version of generateIntersectedRoi as we
    //  delay truly intersecting with the source roi because we want to leave
    //  the option of source roi replication for potential hardware speedup.
    //
    virtual XilStatus generateIntersectedRoi();

    //
    //  On backward map we currently do a hack.  We figure out the
    //  leftmost/topmost src needed to cover the upper left of the dest ROI
    //  bbox, and then we intersect this leftmost source with the dest.
    //
    virtual XilStatus gsBackwardMap(XiliRect*    dst_rect,
				    XiliRect*    src_rect,
                                    unsigned int src_number);

    //    
    //  Move into Object Space if destination, but if source, replicate
    //  source field over destination and set rect relative to individual
    //  source image. Do not take origins into account, that was already
    //  done during backwardMap.
    //
    virtual XilStatus moveIntoObjectSpace(XiliRect*            rect,
					  XilDeferrableObject* object);
                      
    //    
    // Call the parent
    //
    virtual XilStatus moveIntoObjectSpace(XilRoi*              roi,
					  XilDeferrableObject* object);


    //
    // For the source image, this should set the box to the whole src image
    // as you will probably need all of it as you replicate over the source.
    //
    virtual XilStatus      setBoxStorage(XiliRect*            rect,
                                         XilDeferrableObject* object,
                                         XilBox*              box);

    //
    // Split on tile boundaries is responsible for effectively turing
    // copy_pattern into copy. It does so by taking the destination
    // area to be calculated and generating a boxlist of source areas
    // to be copied into the destination.
    // 
    // The original box list has the source always being the whole
    // source image. This allows hardware acceleration to work if
    // they have a "repl_rop" (see pixrect :-) type of functionality.
    //
    virtual XilStatus vSplitOnTileBoundaries(XilBoxList* bl);
    
private:
    // Used to lock check/update of complex intersected roi
    XilMutex              mutex;
    Xil_boolean           fullIntersectedRoiIsSet;
    
    XilImage*             source;
    XilImage*             dest;
    int                   dst_iox;
    int                   dst_ioy;
    unsigned int          src_w;
    unsigned int          src_h;
    int                   src_iox; // source Image origins
    int                   src_ioy;
    int                   fullsrc_x1; // x1, y1 for full replicated src field
    int                   fullsrc_y1;
    //
    // The source rectangle that covers the upper/left corner of the dst
    //
    XiliRectInt*          topleft_rect;
    
};

XilOp*
XilOpCopyPattern::create(char  function_name[],
                  void* args[],
                  int)
{
    XilImage* src = (XilImage*)args[0];
    XilImage* dst = (XilImage*)args[1];

    static XilOpCache  copy_pattern_op_cache;
    XilOpNumber opnum = xili_verify_op_args(function_name,
                                            &copy_pattern_op_cache,
                                            dst, src);

    if(opnum == -1) {
        return NULL;
    }

    //
    // Check to see if the source is greater than or
    // equal to the dest, in that case just do a copy
    //
    if((src->getWidth() >= dst->getWidth()) &&
       (src->getHeight() >= dst->getHeight())) {
	return XilOpCopy::create("copy", args, 2);
    }

    XilOpCopyPattern* op = new XilOpCopyPattern(opnum, src, dst);

    op->setSrc(1, src);
    op->setDst(1, dst);
    XilRoi* src_roi_copy = (XilRoi*)((src->getGlobalSpaceRoi())->createCopy());
    //
    // In the event that the compute routine wants to calculate the
    // replicated source roi itself we give it to them. Note that the
    // global space means it has been offset by its origin, but it
    // doesn't indicate where it intersects the dest in a field of
    // replicated sources
    //
    op->setParam(1, src_roi_copy);

    return op;
}

XilOpCopyPattern::XilOpCopyPattern(XilOpNumber op_num,
				   XilImage*   s,
				   XilImage*   d) :
    XilOpPoint(op_num)
{

    //
    // Store some things which we make use of later in
    // several places.
    //
    source = s;
    dest = d;

    //
    // Src Image dimensions
    //
    src_w = source->getWidth();
    src_h = source->getHeight();

    //
    // Image origins
    // Used to calculate the uppermost/leftmost source
    // which covers dest
    //
    float ox,oy;
    
    dest->getOrigin(&ox,&oy);
    dst_iox = _XILI_ROUND(ox);
    dst_ioy = _XILI_ROUND(oy);


    source->getOrigin(&ox, &oy);
    src_iox = _XILI_ROUND(ox);
    src_ioy = _XILI_ROUND(oy);

    fullsrc_x1 = -src_iox;
    fullsrc_y1 = -src_ioy;
    fullIntersectedRoiIsSet = FALSE;

    topleft_rect = NULL;
}

XilOpCopyPattern::~XilOpCopyPattern()
{
    delete topleft_rect;
}

//
// This version of generateIntersectedRoi simply sets
// the destination ROI as the intersected ROI.
// In order to generate the true intersected ROI, we'd need
// to replicate the source ROI and since a compute device
// may be able to do that quickly through hardware, we
// want to delay doing it as far as possible. The true
// intersected ROI will be generated by splitOnTileBoundaries.
//
XilStatus
XilOpCopyPattern::generateIntersectedRoi()
{

    //
    // We assuming only one src and dst for this operation.
    // 
    XilRoi*          dgs_roi = getDstGlobalSpaceRoi(0);
    XilRoi*          intersected_roi = getIntersectedRoi();

    //
    // First check to see if the intersectedRoi is valid.
    //
    if(intersected_roi == NULL) {
	XIL_ERROR(getSystemState(), XIL_ERROR_SYSTEM, "di-1", TRUE);
	return XIL_FAILURE;
    }
    
    *(intersected_roi) = *(dgs_roi);

    return XIL_SUCCESS;
}


//
//  Backward map a box and return a box, for copy pattern
//  the source is the whole source image, replicated
//  over the destination image (regardless of destination roi
//  bbox which is represented by dst_rect).
//  We then clip the uppermost/topmost copy of the source image
//  to that part of the dest image that it overlaps. This is
//  what we return as new source bbox. It provides the compute
//  routine with the INITIAL source box for processing.
//
//  When the destination contains tiles -
//  This should still generate the source box that covers the
//  upper-left corner of the destination image, regardless of
//  the destination tile rect passed in.
//
XilStatus
XilOpCopyPattern::gsBackwardMap(XiliRect*    ,
				XiliRect*    src_rect,
                                unsigned int )
{

    //
    // Only calculate the topleft_rect once. It stays the same
    // regardless of destination tile.
    // We don't have to lock because we're single-threaded here.
    // Subsequently just pick up topleft_rect again.
    //
    if(topleft_rect == NULL) {
        int delta_x, delta_y;
        int extra_srcs_x, extra_srcs_y;
        Xil_boolean go_left;
        Xil_boolean go_up;
        
        //
        //  We don't use the dst_rect because it represents the bbox of the 
        //  destination ROI and we'd rather know the source that covers the
        //  the upper/left of the destination regardless of roi.
        //
        //
        int dx1 = -dst_iox;
        int dy1 = -dst_ioy;
        // Calculate where in a field of srcs you'd have to place the
        // leftmost/uppermost source to cover the upper left corner of the dest
        // in global space.


        //
        // fullsrc_x1 and fullsrc_y1 will represent this global space x1,y1
        // for a virtual "big" src that would cover the whole dest.
        //
        
        fullsrc_x1 = -src_iox;
        go_left = (dx1 < fullsrc_x1);
        if(go_left) {
            delta_x = fullsrc_x1 - dx1;
            extra_srcs_x = (delta_x+src_w-1)/src_w;
            fullsrc_x1 = fullsrc_x1 - (extra_srcs_x*src_w);
        } else {
            delta_x = dx1 - fullsrc_x1;
            extra_srcs_x = (delta_x)/src_w;
            fullsrc_x1 = fullsrc_x1 + (extra_srcs_x*src_w);
        }
        
        fullsrc_y1 = -src_ioy;
        go_up = (dy1 < fullsrc_y1);
        if(go_up) {
            delta_y = fullsrc_y1 - dy1;
            extra_srcs_y = (delta_y+src_h-1)/src_h;
            fullsrc_y1 = fullsrc_y1 - (extra_srcs_y*src_h);
        } else {
            delta_y = dy1 - fullsrc_y1;
            extra_srcs_y = delta_y/src_h;
            fullsrc_y1 = fullsrc_y1 + (extra_srcs_y*src_h);
        }        
        
        //
        // The startx/starty need to be clipped to the upper left 
        // of the destination, not to fullsrc_x1, fullsrc_y1
        // We *know* that dx1, dy1 are <= fullsrc_x1, fullsrc_y1
        // We *know* from our calculations that this is >0 rectangle
        //
        topleft_rect = new XiliRectInt(dx1,dy1,
                                       fullsrc_x1+(int)src_w -1,
                                       fullsrc_y1 + (int)src_h -1);
    }        

    src_rect->set(topleft_rect);

    return XIL_SUCCESS;
}


//
// splitOnTileBoundaries can be called multiple times if the destination
// is tiled. We only want to calculate the full/complicated intersected
// roi one time for the whole image. Subsequently, just clip it to the 
// tile in question.
//
//  Split on tile boundaries is responsible for effectively turning
//  copy_pattern into copy. It does so by taking the destination
//  area to be calculated and generating a boxlist of source areas
//  to be copied into the destination. 
// 
//  The original box list has the source always being the whole
//  source image. This allows hardware acceleration to work if
//  they have a "repl_rop" (see pixrect :-) type of functionality.
//
XilStatus
XilOpCopyPattern::vSplitOnTileBoundaries(XilBoxList* bl)
{
    //
    // Lock before accessing the fullIntersectedRoi information
    //
    mutex.lock();

    if(fullIntersectedRoiIsSet == FALSE) {

        //
        //  These rois are used to build an accumulated
        //  roi which represents the larger area of the source roi
        //
        XilRoi*      working_roi;
        XilRoi*      cum_roi = NULL;
        
        //
        //  Since we generated intersectedRoi, we know that it
        //  is the same as the destination GS Roi at this point
        //  The source ROI has not been taken into account up
        //  until this point to allow for GPI hardware acceleration.
        //
        XilRoi* sgs_roi         = getSrcGlobalSpaceRoi(0);
        XilRoi* intersected_roi = getIntersectedRoi();
        
        if(getIntersectedRoiIsInObjectSpace() == TRUE) {
            moveIntoGlobalSpace(intersected_roi, dest);
        }

        working_roi = (XilRoi*)sgs_roi->createCopy();
        
        //
        //  Move the working roi to the uppermost,leftmost roi
        //  Get the bbox of this top/left src_roi for boxlist
        //
        working_roi->translate_inplace(-(float)(-src_iox-fullsrc_x1),
                                       -(float)(-src_ioy-fullsrc_y1));
        
	int          subsrc_x1, subsrc_y1;
	int          dst_x1, dst_y1, dst_x2, dst_y2;
        unsigned int dst_w, dst_h;
	int          int_x1, int_y1, int_x2, int_y2;
        int          bbox_x1,bbox_y1;
        unsigned int bbox_w, bbox_h;

        intersected_roi->getIntBoundingBox(&dst_x1,&dst_y1,&dst_w, &dst_h);
        dst_x2 = dst_x1 + (int)dst_w - 1;
        dst_y2 = dst_y1 + (int)dst_h - 1;
        
        working_roi->getIntBoundingBox(&bbox_x1, &bbox_y1, &bbox_w, &bbox_h);
        int bbox_x2     = bbox_x1 + (int)bbox_w -1;
        int bbox_y2     = bbox_y1 + (int)bbox_h -1;
        int bbox_basex1 = bbox_x1;
        
        //
        // This isn't totally clear because some variables drop out.
        // Here's the full calculation
        // num_srcs_x = [ pixel-extent-to-cover + (size - 1) ] / size
        // the addition of size-1 covers the fact that any remains would mean an extra
        // the pixel-extent-to-cover is dst2 - nearest + 1 (to cover pixel extent)
        // so the one's cancel out, leaving :
        int num_srcs_x = (dst_x2 - fullsrc_x1 + src_w )/src_w;
        int num_srcs_y = (dst_y2 - fullsrc_y1 + src_h )/src_h;
        
        //
        //  Use a loop to handle all the base source "tiles" in the replicated
        //  field.
        //
        subsrc_y1 = fullsrc_y1;
        for(int j=0; j < num_srcs_y; j++) {
            subsrc_x1 = fullsrc_x1;
            
            for(int i=0; i < num_srcs_x; i++) {
                //
                //  Find the intersection of the two boxes in global space
                //
                int_x1 = _XILI_MAX(bbox_x1, dst_x1);
                int_y1 = _XILI_MAX(bbox_y1, dst_y1);
                int_x2 = _XILI_MIN(bbox_x2, dst_x2);
                int_y2 = _XILI_MIN(bbox_y2, dst_y2);
                
                //
                //  If the bbox intersection wasn't empty...
                //
                if((int_x2 >= int_x1) && (int_y2 >= int_y1)) {

                    if(cum_roi == NULL) {
                        //
                        //  We've found the first intersection of working_roi
                        //  and destination roi. Start accumulating an ROI.
                        //  Unite can't handle an empty ROI.
                        //
                        cum_roi = (XilRoi*)working_roi->createCopy();
                    } else {
                        working_roi->unite_inplace(cum_roi);
                    }

                    //
                    //  TODO: maynard 8/26/96 - correct this
                    //
                    //  This call was
                    //  working_roi->unite_disjoint_inplace(cum_roi); 
                    //  but unite_disjoint_inplace breaks roirect ordering
                    //  which causes intersect to do the wrong thing.  Until I
                    //  look at that, leave this as the slower "unite" call.
                    //
                }

                //
                //  Increment in the x direction
                //
                subsrc_x1 += (int)src_w;
                working_roi->translate_inplace((float)src_w, 0.0);
                bbox_x1 += (int)src_w;
                bbox_x2 += (int)src_w;
	    }

            //
            //  Move the roi back to the left side, down one row
            //
            working_roi->translate_inplace(-(float)(i*src_w), (float)src_h);

            //
            //  Incrememt in the y direction
            //
            bbox_x1 = bbox_basex1;
            bbox_x2 = bbox_x1 + (int)bbox_w -1;

            bbox_y1   += (int)src_h;
            bbox_y2   += (int)src_h;
            subsrc_y1 += (int)src_h;
        }

        // 
        //  Intersect the new replicated source roi with this roi
        //  We know they're both in global space
        //
        
        cum_roi->intersect_inplace(intersected_roi);
        
        moveIntoObjectSpace(intersected_roi,dest);
        
        setIntersectedRoiIsInObjectSpace(TRUE);
        
        //
        //  We're done with the copy and clip roi, destroy it
        //
        cum_roi->destroy();
        working_roi->destroy();

        fullIntersectedRoiIsSet = TRUE;
    }
    mutex.unlock();


    //
    // Even though the intersectedRoi is now complete, we'll need to get the
    // bbox of a src starting roi for the boxlist generation.
    //
    int          bbox_x1, bbox_x2, bbox_basex1;
    int          bbox_y1, bbox_y2;
    unsigned int bbox_w;
    unsigned int bbox_h;

    XilRoi* sgs_roi         = getSrcGlobalSpaceRoi(0);
    sgs_roi->getIntBoundingBox(&bbox_x1, &bbox_y1, &bbox_w, &bbox_h);
    
    //
    //  Now loop over the boxlist to add necessary boxes
    //
    XiliSLListIterator<XiliBoxListEntry*> iterator(bl->getList());
    XiliBoxListEntry*                     ble;
    while(iterator.getNext(ble) == XIL_SUCCESS) {
	int                  subsrc_x1, subsrc_y1;
	int                  dst_x1, dst_y1, dst_x2, dst_y2;
	int                  int_x1, int_y1, int_x2, int_y2;
        int                  nearest_x1, nearest_y1;
	int                  new_boxes = 0;
        //
        // src_box is *always* the box of the src_image that overlaps the
        // upper left corner of the destination image, not necessarily this
        // tile.
        //
	XilBox*              src_box = &ble->boxes[0];
        //
        // dst_box represents the bbox of the dst roi clipped to the current tile
        //
	XilBox*              dst_box = &ble->boxes[1];
        int                  num_srcs_x; // # of srcs in x that cover this dest region
        int                  num_srcs_y; // # of src in y that cover this dest region
        Xil_boolean          first_entry_done = FALSE;


	//
	//  Move the area represented by the dst_box into global space
	//
        XiliRectInt dst_rect(dst_box);
	dest->convertToGlobalSpace(&dst_rect);
	dst_rect.get(&dst_x1, &dst_y1, &dst_x2, &dst_y2);
        //
        // Since dst_rect may represent a tile of the destination, calculate the
        // startx/starty for the source covering the upper/left corner of the
        // dest_rect. No sense replicating sources that will clip out.
        //
        nearest_x1 = fullsrc_x1 + ((dst_x1-fullsrc_x1)/src_w)*src_w;
        nearest_y1 = fullsrc_y1 + ((dst_y1-fullsrc_y1)/src_h)*src_h;

        //
        //  Move the bbox of the src roi to the nearest upper/left location.
        //  Effectively, translate it.
        //
        bbox_x1 = bbox_x1 - (-src_iox-nearest_x1);
        bbox_y1 = bbox_y1 - (-src_ioy-nearest_y1);
        bbox_x2     = bbox_x1 + (int)bbox_w -1;
        bbox_y2     = bbox_y1 + (int)bbox_h -1;
        bbox_basex1 = bbox_x1;

        //
        // This isn't totally clear because some variables drop out.
        // Here's the full calculation
        // num_srcs_x = [ pixel-extent-to-cover + (size - 1) ] / size
        // the addition of size-1 covers the fact that any remains would mean an extra
        // the pixel-extent-to-cover is dst2 - nearest + 1 (to cover pixel extent)
        // so the one's cancel out, leaving :
        //
        num_srcs_x = (dst_x2 - nearest_x1 + src_w )/src_w;
        num_srcs_y = (dst_y2 - nearest_y1 + src_h )/src_h;
        //
        //  Use a loop to handle all the base source "tiles" in the replicated
        //  field.
        //
        //  NOTE:  The use of first_entry_done to avoid duplicating the
        //         initial entry.  This was done this way instead of outside
        //         the loop on purpose.
        //
        subsrc_y1 = nearest_y1;
        for(int j=0; j < num_srcs_y; j++) {
            subsrc_x1 = nearest_x1;

            for(int i=0; i < num_srcs_x; i++) {
                //
                //  Check for non-null intersection of the translated src roi bbox
                //  with the destination roi bbox before generating a new ble.
                //
                int_x1 = _XILI_MAX(bbox_x1, dst_x1);
                int_y1 = _XILI_MAX(bbox_y1, dst_y1);
                int_x2 = _XILI_MIN(bbox_x2, dst_x2);
                int_y2 = _XILI_MIN(bbox_y2, dst_y2);

                //
                //  If the intersection wasn't empty...
                //
                if((int_x2 >= int_x1) && (int_y2 >= int_y1)) {
                    //
                    //  I know this test is in the inner loop, but special
                    //  casing the first XilBoxListEntry became very
                    //  complicated because any of the full intersections
                    //  might go to NULL.
                    //
                    if(first_entry_done) {
                        XiliBoxListEntry* new_ble = new XiliBoxListEntry;
                        
                        if(new_ble == NULL) {
                            XIL_ERROR(getSystemState(), XIL_ERROR_SYSTEM, "di-1", TRUE);
                            return XIL_FAILURE;
                        }

                        //
                        //  First, set the dst rect...
                        //
                        dst_rect.set(int_x1, int_y1, int_x2, int_y2);

                        //
                        //  Convert to object space and set the storage
                        //  information.
                        //
                        if(dest->convertToObjectSpace(&dst_rect) == XIL_FAILURE) {
                            return XIL_FAILURE;
                        }

                        //
                        //  Copy the information from the rect...
                        //
                        new_ble->boxes[1] = dst_rect;

                        if(dest->setBoxStorage(&new_ble->boxes[1]) == XIL_FAILURE) {
                            return XIL_FAILURE;
                        }

                        //
                        //  Now, set the src box.
                        //
                        //  This is done in "object space" which in this case
                        //  means the box is set relative to a subsrc.
                        //
                        new_ble->boxes[0].setAsCorners(int_x1 - subsrc_x1,
                                                       int_y1 - subsrc_y1,
                                                       int_x2 - subsrc_x1,
                                                       int_y2 - subsrc_y1);
                        if(source->setBoxStorage(&new_ble->boxes[0]) == XIL_FAILURE) {
                            return XIL_FAILURE;
                        }

                        //
                        //  Insert this entry.
                        //
                        bl->getList()->insertAfter(new_ble,
                                                   iterator.getCurrentPosition());

                        new_boxes++;
                    } else {
                        //
                        //  First, set the dst rect...
                        //
                        dst_rect.set(int_x1, int_y1, int_x2, int_y2);

                        //
                        //  Convert to object space and set the storage
                        //  information.
                        //
                        if(dest->convertToObjectSpace(&dst_rect) == XIL_FAILURE) {
                            return XIL_FAILURE;
                        }

                        //
                        //  Copy the information from the rect...
                        //
                        *dst_box = dst_rect;

                        if(dest->setBoxStorage(dst_box) == XIL_FAILURE) {
                            return XIL_FAILURE;
                        }

                        //
                        //  Now, set the src box.
                        //
                        //  This is done in "object space" which in this case
                        //  means the box is set relative to a subsrc.
                        //
                        src_box->setAsCorners(int_x1 - subsrc_x1,
                                              int_y1 - subsrc_y1,
                                              int_x2 - subsrc_x1,
                                              int_y2 - subsrc_y1);
                        if(source->setBoxStorage(src_box) == XIL_FAILURE) {
                            return XIL_FAILURE;
                        }

                        //
                        //  Now, we'll have to start creating and inserting.
                        //
                        first_entry_done = TRUE;
                    }

                }

                //
                //  Increment in the x direction
                //
                subsrc_x1 += (int)src_w;
                bbox_x1 += (int)src_w;
                bbox_x2 += (int)src_w;
	    }

            //
            //  Incrememt in the y direction
            //
            bbox_x1 = bbox_basex1;
            bbox_x2 = bbox_x1 + (int)bbox_w -1;

            bbox_y1   += (int)src_h;
            bbox_y2   += (int)src_h;
            subsrc_y1 += (int)src_h;
        }

        //
        //  So, why would we EVER have more than on ble at this call?
        //
	//  Skip over the new boxes we added to get to the next
	//  destination box
	//
        for(int cnt=0; cnt<new_boxes; cnt++) {
	    iterator.getNext(ble);
	}
        //
        // get starting src roi bbox again
        //
        sgs_roi->getIntBoundingBox(&bbox_x1, &bbox_y1, &bbox_w, &bbox_h);
    }

    //
    //  The source may still need to be split on tile boundaries
    //  if the source is relatively large (unusual case) and if
    //  origins cause it to be offset from the destination or if
    //  the source has a different tilesize than the dest. Well, okay,
    //  it's unlikely, but it *could* happen.
    //  We should just be able to call parent vSplitOnTileBoundaries
    //  because all that does is split src and dests on src tile
    //  boundaries without bwmaps or fwmaps so it should work.
    //
// TODO: maynard 10/30/96
//    turn this on and test
//    return XilOp::vSplitOnTileBoundaries(bl);
    return XIL_SUCCESS;
}

XilStatus
XilOpCopyPattern::moveIntoObjectSpace(XiliRect*            rect,
                                      XilDeferrableObject* object)
{
    XilImage* image = (XilImage*)object;

    //
    // Is the object the destination image
    //
    if(image == dest) {
	return image->convertToObjectSpace(rect);
    }

    //
    // Source image
    //
    if(image == source) {
        int src_numx, src_numy;
        int sx1,sy1, sx2,sy2;
        unsigned int sw,sh;
        int sub_srcx1, sub_srcy1;
        int box_x1, box_y1;
	//
	// rect is in global space. Figure out where this rect
        // would be in a replicated field of source images.
        // and change x1,y1 to be relative to the base src it's in.
        // fullsrc_x1 and fullsrc_y1 are stored on Op.
	// We can assume that the coordinates are integer, since
	// copy pattern is not a geometric.
	//
        rect->get(&sx1,&sy1,&sx2,&sy2);
	sw = sx2-sx1+1;
	sh = sy2-sy1+1;
        src_numx = (sx1-fullsrc_x1)/src_w;
        src_numy = (sy1-fullsrc_y1)/src_h;
        sub_srcx1 = fullsrc_x1 + (src_numx * src_w);
        sub_srcy1 = fullsrc_y1 + (src_numy * src_h);
        
        //
        // Now calculate the box x1,y1 relative to the
        // start of this base src.
        //
        box_x1 = sx1 - sub_srcx1;
        box_y1 = sy1 - sub_srcy1;
        //
        // Since we know the original box was clipped to a source
        // we can assume that the original w,h will not overflow the src
        // dimensions.
        //
        rect->set(box_x1,box_y1,(box_x1+sw-1),(box_y1+sh-1));
	return XIL_SUCCESS;
    }
    return XIL_FAILURE;
}

    
XilStatus
XilOpCopyPattern::moveIntoObjectSpace(XilRoi*              roi,
                                      XilDeferrableObject* object)
{
    return XilOp::moveIntoObjectSpace(roi, object);
}



XilStatus
XilOpCopyPattern::setBoxStorage(XiliRect*            rect,
              XilDeferrableObject* object,
              XilBox*              box)
{
    if(object == source) {
        box->setAsRect(0, 0, src_w, src_h);
    } else {
        *box = *rect;
    }

    return object->setBoxStorage(box);
}
