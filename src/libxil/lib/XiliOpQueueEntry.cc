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
//  File:	XiliOpQueueEntry.cc
//  Project:	XIL
//  Revision:	1.40
//  Last Mod:	10:08:08, 03/10/00
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
#pragma ident	"@(#)XiliOpQueueEntry.cc	1.40\t00/03/10  "

#include "_XilDefines.h"
#include "XiliOpQueueEntry.hh"

#include "_XilDeferrableObject.hh"
#include "_XilImage.hh"
#include "_XilOp.hh"

//
//  The op status refers to the current state of the operation which is
//  writing into the given instance of this object.  The op uses this to
//  determine if flushTile() needs to process anything to bring a tile
//  up-to-date.
//
void
XiliOpQueueEntry::setOpStatus(XilTileNumber  tile_number,
                              XiliOpStatus   op_status)
{
    if(opStatusArray == NULL) {
        if(initOpStatusArray() == XIL_FAILURE) {
            return;
        }
    }

    //
    //  TODO: 2/13/97 jlf  Now that operations that cannot be split still mark
    //                     all of their tiles, this test should no longer be
    //                     needed.  But, I'm going to leave it in for Beta and
    //                     generate an error if the condition occurs.
    //
    if((unsigned int) tile_number < numTiles) {
        //
        //  If the tile has already been marked evaluated, then we want to do
        //  nothing in this routine...it's an extraneous call.
        //
        if(opStatusArray[tile_number] == XILI_EVALUATED) {
            return;
        }

        opStatusArray[tile_number] = op_status;
    } else {
        XIL_ERROR(defobj->getSystemState(), XIL_ERROR_INTERNAL, "di-443", TRUE);
        return;
    }

    if(op_status != XILI_EVALUATED) {
        return;
    }

    //
    //  Notify the specified op queue entry if we've been marked.
    //
    if(notifyList != NULL) {
        //
        //  Do notification...we backward map this tile into the source of our
        //  creator op and notify the source tiles that this tile has been
        //  evaluated. 
        //
        if(op == NULL) {
            //
            //  Nobody to notify anymore...
            //
            notifyList = NULL;
        } else {
            XiliNotifyEntry* nlist = notifyList;
            while(nlist) {
                XiliRectInt dst_rect;

                defobj->getTileRect(&dst_rect, tile_number);

                //
                //  Move into global space for backward mapping.
                //
                op->moveIntoGlobalSpace(&dst_rect, defobj);

                //
                //  Backward map into specified source...
                //
                XiliRectInt src_rect;
                if(op->gsBackwardMap(&dst_rect, &src_rect,
                                     nlist->srcNum) == XIL_SUCCESS) {
                    XilDeferrableObject* src_obj = op->getSrc(nlist->srcNum);
                    if(op->moveIntoObjectSpace(&src_rect,
                                               src_obj) == XIL_SUCCESS) {
                        XilBox tmp_box;
                        if(op->setBoxStorage(&src_rect, src_obj,
                                             &tmp_box) == XIL_SUCCESS) {
                            int sx1, sy1, sx2, sy2, sb;
                            tmp_box.getStorageAsCorners(&sx1, &sy1, &sx2, &sy2, &sb);
                            src_rect.set(sx1, sy1, sx2, sy2);
                        }

                        //
                        //  src_rect now contains the parent coordinates for
                        //  the area in the source.  The op may have a
                        //  child of the parent as its source.  We need to
                        //  convert the parent coordinates into child
                        //  coordinates (if the object is an XIL_IMAGE).
                        //
                        if(src_obj->getType() == XIL_IMAGE) {
                            XilImage* src_img = (XilImage*)src_obj;

                            unsigned int x_child_offset;
                            unsigned int y_child_offset;
                            unsigned int b_child_offset;
                            src_img->getChildOffsets(&x_child_offset,
                                                     &y_child_offset,
                                                     &b_child_offset);

                            src_rect.setX1(src_rect.getX1() - x_child_offset);
                            src_rect.setX2(src_rect.getX2() - x_child_offset);
                            src_rect.setY1(src_rect.getY1() - y_child_offset);
                            src_rect.setY2(src_rect.getY2() - y_child_offset);
                        }

                        //
                        //  Some portions of the tile may fall outside the
                        //  source -- so clip to the source.
                        //
                        XiliRectInt full_src_rect(src_obj->getGlobalSpaceRect());

                        if(op->moveIntoObjectSpace(&full_src_rect,
                                                   src_obj) == XIL_SUCCESS) {
                            if(src_rect.clip(&full_src_rect)) {
                                XilTileList tl(src_obj->getSystemState());

                                if(src_obj->getTileList(&tl,
                                                        &src_rect) == XIL_SUCCESS) {
                                    XilTileNumber tnum;
                                    while(tl.getNextTileNumber(&tnum)) {
                                        nlist->entry->notify(tnum);
                                    }
                                }
                            }
                        }
                    }
                }
                nlist = nlist->next;
            }
        }
    }
    
    //
    //  If we're marking a temporary object or we're tossed, then we need to
    //  indicate to our dependent to call us when any of the tiles we forward
    //  map to are evaluated.  Then, we check to see if all of the tiles we
    //  forward map to have been evaluated and if so, the storage for this
    //  tile can be released.
    //
    if(defobj->isTemp() || opIsTossed) {
        //
        //  If we've been marked as "all needed," then all of our tiles
        //  are required by any pixel in the destination so we don't need to
        //  set up notification.
        //
//        if(! allTilesNeeded) {
            if(depTileCnts == NULL) {
                depTileCnts = new int[numTiles];
                if(depTileCnts == NULL) {
                    XIL_OBJ_ERROR(defobj->getSystemState(),
                                  XIL_ERROR_RESOURCE, "di-1", TRUE, defobj);
                    return;
                }

                for(int i=numTiles-1; i>=0; i--) {
                    depTileCnts[i] = 0;
                }
            }

            //
            //  For each of our dependents, forward map this tile into their
            //  destination and generate a list of tiles.
            //
            for(unsigned int i=0; i<numDependents; i++) {
                XilOp*               dependent   = dependents[i].op;

                XilDeferrableObject* op_dst_obj  =
                    op ? op->getDst(1) : defobj;

                unsigned int         dep_src_num =
                    dependents[i].branch + 1;

                XilDeferrableObject* dep_src_obj =
                    dependent->getSrc(dep_src_num);

                XilDeferrableObject* dep_dst_obj =
                    dependent->getNumDsts() == 0 ?
                    dependent->getSrc(dep_src_num) :
                    dependent->getDst(1);

                //
                //  Ok, we'll have to translate the area in our creator op's
                //  destination to the source of the dependent and then
                //  forward map that into the destination of the dependent.
                //
                XiliRectInt  op_dst_rect;
                XiliRectInt  dep_src_rect;
                XiliRectInt  dep_dst_rect;

                op_dst_obj->getTileRect(&op_dst_rect, tile_number);

                //
                //  Translate the area in the creator op's destination
                //  (i.e. this entry's deferrable object) into the dependent's
                //  source -- leaving it in global space.
                //
                if(translateOpDstToDepSrcGS(op,
                                            dependent,
                                            op_dst_obj,
                                            dep_src_obj,
                                            &op_dst_rect,
                                            &dep_src_rect) == XIL_FAILURE) {
                    continue;
                }

                //
                //  Generate a new tile list valid in the destination from the
                //  rect in the source.
                //
                XiliOpQueuePosition qpos = dependent->getNumDsts() ?
                    dependent->getDstOpQueuePosition((unsigned int)0) :
                    dependent->getSrcOpQueuePosition(dependents[i].branch);

                XilTileList dst_tile_list(dep_dst_obj->getSystemState());

                if(fwdMapRect(&dep_src_rect, dep_src_num,
                              dependent, dep_dst_obj,
                              &dst_tile_list) == XIL_FAILURE) {
                    continue;
                }

                //
                //  If we can't setup the notify event (maybe because
                //  another source already set one), then we can't release
                //  storage.
                //
                XiliOpQueueEntry* dst_entry =
                    dep_dst_obj->getQueueEntry(qpos);
                if(dst_entry->setNotifyEntry(this,
                                             dep_src_num) == XIL_FAILURE) {
                    continue;
                }

                //
                //  Bump the count of how many notifications this tile must
                //  receive before it's considered "released"
                //
                depTileCnts[tile_number] += dst_tile_list.getNumTiles();
            }
//        }
    }
}
    
XiliOpStatus
XiliOpQueueEntry::getOpStatus(XilTileNumber tile_number)
{
    if(opStatusArray == NULL) {
        initOpStatusArray();
    }

    TNF_PROBE_3(xilop_status_get, "xilop", "xilop_status_get",
                tnf_opaque, "this", op,
                tnf_long, "tile", tile_number,
                tnf_long, "status", opStatusArray[tile_number]);

    //
    //  TODO: 2/13/97 jlf  Now that operations that cannot be split still mark
    //                     all of their tiles, this test should no longer be
    //                     needed.  But, I'm going to leave it in for Beta and
    //                     generate an error if the condition occurs.
    //
    if((unsigned int) tile_number < numTiles) {
        return opStatusArray[tile_number];
    } else {
        XIL_ERROR(defobj->getSystemState(), XIL_ERROR_INTERNAL, "di-444", TRUE);
        return XILI_EVALUATED;
    }
}

XilStatus
XiliOpQueueEntry::initOpStatusArray()
{
    if(numTiles == 1) {
        singleTileOpStatus = XILI_DEFERRED;
        opStatusArray      = &singleTileOpStatus;
    } else {
        opStatusArray = new XiliOpStatus[numTiles];

        if(opStatusArray == NULL) {
            XIL_OBJ_ERROR(defobj->getSystemState(),
                          XIL_ERROR_RESOURCE, "di-1", TRUE, defobj);
            return XIL_FAILURE;
        }

        for(int i=numTiles-1; i>=0; i--) {
            opStatusArray[i] = XILI_DEFERRED;
        }
    }

    return XIL_SUCCESS;
}

XilStatus
XiliOpQueueEntry::setNotifyEntry(XiliOpQueueEntry* entry,
                                 unsigned int      src_num)
{
    //
    //  Verify we don't already have this entry on our list.
    //
    XiliNotifyEntry* tmp_entry = notifyList;
    while(tmp_entry != NULL) {
        if(tmp_entry->entry  == entry &&
           tmp_entry->srcNum == src_num) {
            return XIL_SUCCESS;
        }

        tmp_entry = tmp_entry->next;
    }

    XiliNotifyEntry* nentry = new XiliNotifyEntry;
    if(nentry == NULL) {
        XIL_OBJ_ERROR(defobj->getSystemState(),
                      XIL_ERROR_RESOURCE, "di-1", TRUE, defobj);
        return XIL_FAILURE;
    }

    nentry->next   = notifyList;
    nentry->entry  = entry;
    nentry->srcNum = src_num;

    notifyList = nentry;

    return XIL_SUCCESS;
}

void
XiliOpQueueEntry::notify(XilTileNumber tile_number)
{
    if(depTileCnts == NULL) {
        return;
    }

    if(depTileCnts[tile_number] != 0) {
        //
        //  When we've received enough notifications, release the tile.
        //
        if(--depTileCnts[tile_number] == 0) {
            defobj->releaseTile(tile_number);
        }
    }
}

Xil_boolean
XiliOpQueueEntry::allTilesDone()
{
    if(opStatusArray == NULL || op == NULL) {
        return FALSE;
    } else {
        for(int i=numTiles-1; i>=0; i--) {
            if(opStatusArray[i] != XILI_EVALUATED) {
                return FALSE;
            }
        }

        return TRUE;
    }
}

XilStatus
XiliOpQueueEntry::evaluate(XilTileList* tile_list,
                           XilOp*       op_flushing)
{
    //
    //  If we have any dependents in our list, go through and flush them.
    //  Flushing them will cause the op writing into us to be evaluated.
    //
    XilStatus return_val = XIL_SUCCESS;
    
    for(unsigned int i=0; i<numDependents; i++) {
        //
        //  Don't flush the op which is responsible for flushing us.  Note
        //  that in the case of molecules, we don't want to flush the
        //  the dependent op if op_flushing is the bottom operation of the
        //  molecule the dependent's a part of...
        //
        XilOp* dependent       = dependents[i].op;
        XilOp* molecule_bottom = dependent->getMoleculeBottom();

        if(dependent != op_flushing &&
           (molecule_bottom == NULL || molecule_bottom != op_flushing)) {
            //
            //  The given tile_list is valid in op_flushing's destination
            //  which is either THE dependent's source image or the parent of
            //  the dependent's source image.  We know this because evaluate()
            //  is only ever called when someone needs to write into an image
            //  and dependents only cumulate on the source images to
            //  operations.  So, if we're being called, then the op must have
            //  a destination image and that destination image must have a
            //  relationship to the source of our dependent (which is or is a
            //  child of our deferrable object).
            //
            //  Obviously, tile numbers may not align between the destination
            //  of the flushing op and the source to our dependent.  So, we
            //  first determine the area of the destination that needs
            //  evaluating.  Then, move that area into the source of our
            //  dependent (since our deferrable object is the common link,
            //  it's possible to do this).  Forward map this region into the
            //  dependent's destination and then flush the resultant
            //  tile_list.
            //
            //  When we add the dependent, the branch is kept 0, 1, 2...
            //  XilImage::getSrc() and XilOp::gsForwardMap() handle the source
            //  number as 1, 2, 3... 
            //
            //  TODO: 11/4/96 jlf  Develop special case for whole images.
            //
            XilDeferrableObject* op_dst_obj  = op_flushing->getDst(1);
            unsigned int         dep_src_num = dependents[i].branch + 1;
            XilDeferrableObject* dep_src_obj = dependent->getSrc(dep_src_num);
            XilDeferrableObject* dep_dst_obj = dependent->getDst(1);

            //
            //  A special case for when they're the exact same object (in
            //  which case, the tile list is valid for both).
            //
            if(op_dst_obj  == dep_dst_obj ||
               dep_dst_obj == NULL) {
                if(dependent->flush(tile_list) == XIL_FAILURE) {
                    //
                    //  If a single tile in an operation fails to execute, the
                    //  whole operation is considered to have failed and can
                    //  be destroyed.
                    //
                    dependent->destroy();

                    //
                    //  Indicate we encountered a failure.
                    //
                    return_val = XIL_FAILURE;
                }
            } else if(! op_flushing->canBeSplit() ||
                      ! dependent->canBeSplit()) {
                //
                //  Or, if our flushing_op or dependent can't be split on tile
                //  boundaries, we need to flush the entire dependent
                //  operation because if we're going to touch any part of the
                //  destination, the entire operations need to be flushed.
                //
                XilTileList tl(dep_dst_obj->getSystemState());

                dep_dst_obj->getTileList(&tl, (XiliRect*)NULL);

                //
                //  Copy any mutex reference from our tile_list.
                //
                tl.setMutex(tile_list->getMutex());

                if(dependent->flush(&tl) == XIL_FAILURE) {
                    //
                    //  If a single tile in an operation fails to execute, the
                    //  whole operation is considered to have failed and can
                    //  be destroyed.
                    //
                    dependent->destroy();

                    //
                    //  Indicate we encountered a failure.
                    //
                    return_val = XIL_FAILURE;
                }
            } else {
                //
                //  Ok, we'll have to translate the area in the flushing_op's
                //  destination to the source of the dependent and then
                //  forward map that into the destination of the dependent.
                //
                XiliRectInt  op_dst_rect;
                XiliRectInt  dep_src_rect;
                XiliRectInt  dep_dst_rect;

                //
                //  Get the extent of the tile list on the flushing_op's
                //  destination.
                int          x;
                int          y;
                unsigned int xsize;
                unsigned int ysize;
                tile_list->getArea(&x, &y, &xsize, &ysize);

                //
                //  Translate the area in the flushing_op's destination to the
                //  source of the dependent operation.
                //
                if(xsize == 0) {
                    //
                    //  The area was never set on the tile list.  So, we will
                    //  translate the entire extent of the flushing_op's
                    //  destination to the source of the dependent.
                    //
                    op_dst_rect.set(op_dst_obj->getGlobalSpaceRect());
                } else {
                    //
                    //  The area the tile list represents in the flushing_op's
                    //  destination.
                    //
                    op_dst_rect.set(x, y, (x + xsize - 1), (y + ysize -1));
                }

                //
                //  Translate the area in the creator op's destination
                //  (i.e. this entry's deferrable object) into the dependent's
                //  source -- leaving it in global space.
                //
                if(translateOpDstToDepSrcGS(op_flushing,
                                            dependent,
                                            op_dst_obj,
                                            dep_src_obj,
                                            &op_dst_rect,
                                            &dep_src_rect) == XIL_FAILURE) {
                    continue;
                }

                //
                //  Generate a new tile list valid for the dependent's
                //  destination.
                //
                XilTileList dst_tile_list(dep_dst_obj->getSystemState());

                //
                //  Must copy the mutex if we're going to use it to flush()
                //
                dst_tile_list.setMutex(tile_list->getMutex());

                //
                //  Forward map the rectangle in the source to a rectangle in
                //  the destination and generate a tile list.
                //
                if(fwdMapRect(&dep_src_rect, dep_src_num, dependent,
                              dep_dst_obj, &dst_tile_list) == XIL_FAILURE) {
                    continue;
                }

                if(dependent->flush(&dst_tile_list) == XIL_FAILURE) {
                    //
                    //  If a single tile in an operation fails to execute, the
                    //  whole operation is considered to have failed and can
                    //  be destroyed.
                    //
                    dependent->destroy();

                    //
                    //  Indicate we encountered a failure.
                    //
                    return_val = XIL_FAILURE;
                }
            }
        }
    }

    return return_val;
}        

XilStatus
XiliOpQueueEntry::translateOpDstToDepSrcGS(XilOp*               flushing_op,
                                           XilOp*               dependent,
                                           XilDeferrableObject* op_dst_obj,
                                           XilDeferrableObject* dep_src_obj,
                                           XiliRectInt*         op_dst_rect,
                                           XiliRectInt*         dep_src_rect)
{
    //
    //  Use setBoxStorage() to extend the area (if necessary)
    //  and to handle any child offsets.  Thus, the resultant
    //  box's storage is set and describes the area in the
    //  parent (i.e. the calling deferrable object).
    //
    XilBox tmp_box;
    if(flushing_op->setBoxStorage(op_dst_rect, op_dst_obj,
                                  &tmp_box) == XIL_FAILURE) {
        return XIL_FAILURE;
    }

    int    sx1;
    int    sy1;
    int    sx2;
    int    sy2;
    int    sb;
    tmp_box.getStorageAsCorners(&sx1, &sy1, &sx2, &sy2, &sb);
    op_dst_rect->set(sx1, sy1, sx2, sy2);

    //
    //  Set dep_src_rect equal to op_dst_rect and then deal with
    //  translating dep_src_rect. 
    //
    dep_src_rect->set(op_dst_rect);

    //
    //  The dependent op may modify the area in the source in
    //  setBoxStorage() (i.e. convolution) so we need to call
    //  setBoxStorage() and update the dep_src_rect with the
    //  storage area. 
    //
    if(dependent->setBoxStorage(dep_src_rect, dep_src_obj,
                                &tmp_box) == XIL_FAILURE) {
        return XIL_FAILURE;
    }
    tmp_box.getStorageAsCorners(&sx1, &sy1, &sx2, &sy2, &sb);
    dep_src_rect->set(sx1, sy1, sx2, sy2);
                
    //
    //  dep_src_rect now contains the parent coordinates for
    //  the area we need to forward map into the dependent's
    //  destination.  But, the dependent may have a child of the
    //  parent as its real source.  We need to convert the parent
    //  coordinates into child coordinates (if the dependent's
    //  source is an XIL_IMAGE).
    //
    //  In fact, we need to remove the child offsets twice to get
    //  child coordinates because setBoxStorage() will have added
    //  the child offsets.
    //
    if(dep_src_obj->getType() == XIL_IMAGE) {
        XilImage* dep_src_img = (XilImage*)dep_src_obj;

        unsigned int x_child_offset;
        unsigned int y_child_offset;
        unsigned int b_child_offset;
        dep_src_img->getChildOffsets(&x_child_offset,
                                     &y_child_offset,
                                     &b_child_offset);

        dep_src_rect->setX1(dep_src_rect->getX1() - (x_child_offset<<1));
        dep_src_rect->setX2(dep_src_rect->getX2() - (x_child_offset<<1));
        dep_src_rect->setY1(dep_src_rect->getY1() - (y_child_offset<<1));
        dep_src_rect->setY2(dep_src_rect->getY2() - (y_child_offset<<1));
    }

    //
    //  Move the dep_src_rect into global space so we can forward
    //  map it into the dependent's destination.
    //
    dependent->moveIntoGlobalSpace(dep_src_rect, dep_src_obj);

    return XIL_SUCCESS;
}


//
//  Forward map an area in global space from this object into the given
//  dependent operation's destination and fill in the given tile list.
//
XilStatus
XiliOpQueueEntry::fwdMapRect(XiliRect*            src_rect,
                             unsigned int         src_num,
                             XilOp*               dependent,
                             XilDeferrableObject* dst_obj,
                             XilTileList*         dst_tile_list)
{
    XiliRectInt dst_rect;

    if(dependent->gsForwardMap(src_rect, src_num, &dst_rect) == XIL_FAILURE) {
        return XIL_FAILURE;
    }

    //
    //  Get the dst_rect into object space.
    //
    if(dependent->moveIntoObjectSpace(&dst_rect, dst_obj) == XIL_FAILURE) {
        return XIL_FAILURE;
    }

    //
    //  Some portions of the tile may fall outside the destination -- so clip
    //  to the destination.
    //
    XiliRectInt full_dst_rect(dst_obj->getGlobalSpaceRect());

    if(dependent->moveIntoObjectSpace(&full_dst_rect, dst_obj) == XIL_FAILURE) {
        return XIL_FAILURE;
    }

    if(! dst_rect.clip(&full_dst_rect)) {
        return XIL_FAILURE;
    }

    //
    //  Generate a new tile list valid in the destination.
    //
    if(dst_obj->getTileList(dst_tile_list, &dst_rect) == XIL_FAILURE) {
        return XIL_FAILURE;
    }

    return XIL_SUCCESS;
}

