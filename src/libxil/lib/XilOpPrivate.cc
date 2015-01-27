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
//  File:	XilOpPrivate.cc
//  Project:	XIL
//  Revision:	1.123
//  Last Mod:	10:08:28, 03/10/00
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
#pragma ident	"@(#)XilOpPrivate.cc	1.123\t00/03/10  "

//
//  System Includes
//
#include <stdlib.h>
#include <string.h>

//
//  XIL Includes
//
#include "_XilDefines.h"
#include "_XilGlobalState.hh"
#include "_XilSystemState.hh"
#include "_XilImage.hh"
#include "_XilOp.hh"
#include "_XilBoxList.hh"
#include "_XilRoi.hh"
#include "_XilDeviceManagerIO.hh"
#include "_XilDeviceIO.hh"
#include "_XilTileList.hh"

#include "XiliDagManager.hh"
#include "XiliDag.hh"
#include "XiliList.hh"
#include "XiliMarker.hh"
#include "XiliScheduler.hh"
#include "XiliUtils.hh"
#include "XiliConvexRegion.hh"

//--------------------------------------------------------------
//
//  XilOp LIBXIL PRIVATE Routines
//
//--------------------------------------------------------------
inline
Xil_boolean
_XILI_CHECK_ROI_MISMATCH(XilRoi*     src_roi,
                         XilRoi*     dst_roi,
                         XilVersion* roi_version)
{
    Xil_boolean ret_val = FALSE;

    //
    //  Check the ROI version.
    //
    if(dst_roi == NULL) {
        if(src_roi != NULL) {
            ret_val = TRUE;
        }
    } else {
        if(src_roi == NULL) {
            ret_val = TRUE;
        } else if(! src_roi->isSameAs(roi_version)) {
            ret_val = TRUE;
        }
    }

    return ret_val;
}
    
//------------------------------------------------------------------------
//
//  Function:	thisOpCoversPreviousOp()
//
//  Description:
//
//      Do I overwrite all of the area the previous op writing into my
//      destination object write?
//	
//------------------------------------------------------------------------
Xil_boolean
XilOp::thisOpCoversPreviousOp()
{
    //
    //  If we don't have any destinations, then we have nothing to check.
    //
    if(numDsts == 0) {
        return FALSE;
    }

    XilOp* prev_op = dst[0]->getOp();

    //
    //  If no other op wrote into this destination object, then 
    //  move on...no need for execute() to call toss so we return FALSE.
    //
    if(prev_op == NULL) {
        return FALSE;
    }

    //
    //  Get the object the previous op writes into...if it's NULL, there's
    //  no need for execute() to call toss so we return FALSE.
    //
    XilDeferrableObject* prev_dst = prev_op->dst[0];
    if(prev_dst == NULL) {
        return FALSE;
    }

    //
    //  We do more simple tests first and then call generateIntersectedRoi()
    //  to do a complete test.
    //
    //  - If the destination objects are not equal (i.e. one is a child and
    //    the other is a parent), then (for now) we just consider the new
    //    operation does not overwrite the previous operation.
    //
    //  - If there is a mismatch in the version number between any of the ROIs
    //    then we assume the entire destination is not overwritten.
    //
    //  - Next, generate a destination image box for the two operations
    //    based solely on the size of the images, the op transformation and
    //    origins.  If the new op's box is greater than or equal to the
    //    previous op's box, we can continue testing.
    //
    //  - If we get this far, then we need to generate the intersected ROI for
    //    each of the operations and test that the intersected ROI for op2 is
    //    greater than or equal to the intersected ROI for op1 (in that it
    //    subsumes all of the pixels written by op1).  For now, this step is
    //    left out as it may be more expensive than the benefits.
    //
    //  It is still possible to make this routine more accurate, but the
    //  time/accuracy tradeoff is unclear.  We want to do a limited check, but
    //  beyond that if the user asked for the operation to be executed, it
    //  probably should be executed.
    //

    //
    //  If our destinations are not the same object, then indicate we don't
    //  overwrite the previous operation.  At this point, we reject
    //  parent/child mismatches when its possible a child overwrites the
    //  entire area of an earlier operation by another child or parent.
    //
    XilDeferrableObject* dst_obj = dst[0];

    if(prev_dst != dst_obj) {
        return FALSE;
    }

    //
    //  NOTE:  From this point on, I just use dst_obj since prev_dst and
    //         dst_obj are on in the same.
    //

    //
    //  Test the ROI version on all of the images.  If they're not the same,
    //  then we can skip generating a box represenation of the destination
    //  area and either fail or move onto calling generateIntersectedRoi().
    //
    XilVersion           roi_version;
    XilRoi*              dst_roi      = dst_obj->refRoi();
    Xil_boolean          roi_mismatch = FALSE;
    if(dst_roi != NULL) {
        dst_roi->getVersion(&roi_version);
    }

    if(prev_op->numSrcs > 0) {
        for(unsigned int i=0; i<prev_op->numSrcs; i++) {
            //
            //  Check the ROI version.
            //
            roi_mismatch =
                _XILI_CHECK_ROI_MISMATCH(prev_op->src[i]->refRoi(),
                                         dst_roi, &roi_version);
            if(roi_mismatch) {
                break;
            }
        }
    }

    if(numSrcs > 0) {
        for(unsigned int i=0; i<numSrcs; i++) {
            //
            //  Check the ROI version.
            //
            roi_mismatch =
                _XILI_CHECK_ROI_MISMATCH(src[i]->refRoi(),
                                         dst_roi, &roi_version);
            if(roi_mismatch) {
                break;
            }
        }
    }

    if(roi_mismatch) {
        //
        //  We could call generateIntersectedRoi() to do a pixel-by-pixel
        //  comparison of the results in the images. 
        //
        return FALSE;
    }

    //
    //  Generate an estimate of the destination image area written by forward
    //  mapping all of the sources into the respective destination image
    //  space and comparing sizes.
    //
    //  Above, we reject children/parent mismatches.  It would be possible to
    //  let the case fall on to here and then add a depth/nband test here as
    //  well as the area test.
    //
    XiliRect*   src_rect;
    XiliRectInt dst_rect;
    XiliRectInt prev_dst_rect;
    XiliRectInt cur_dst_rect;
    unsigned int         i;
    if(prev_op->numSrcs > 0) {
        for(i=0; i<prev_op->numSrcs; i++) {
            //
            //  Do all of the forward mapping and intersecting in global space.
            //
            src_rect = prev_op->src[i]->getGlobalSpaceRect();

            prev_op->gsForwardMap(src_rect, i, &dst_rect);

            //
            //  Insersect this destination rect with the cumulative
            //  destination rect.
            //
            if(prev_dst_rect.isEmpty()) {
                prev_dst_rect.set(&dst_rect);
            } else {
                prev_dst_rect.clip(&dst_rect);
            }
        }
    } else {
        prev_dst_rect.set(prev_op->dst[0]->getGlobalSpaceRect());
    }

    if(numSrcs > 0) {
        for(i=0; i<numSrcs; i++) {
            //
            //  Do all of the forward mapping and intersecting in global space.
            //
            src_rect = src[i]->getGlobalSpaceRect();

            gsForwardMap(src_rect, i, &dst_rect);

            //
            //  Insersect this destination rect with the cumulative
            //  destination rect.
            //
            if(cur_dst_rect.isEmpty()) {
                cur_dst_rect.set(&dst_rect);
            } else {
                cur_dst_rect.clip(&dst_rect);
            }
        }
    } else {
        cur_dst_rect.set(dst[0]->getGlobalSpaceRect());
    }

    //
    //  If the rect representing the area written into the destination (in
    //  object space) is smaller than the previous operation's rect (in object
    //  space), then we don't overwrite it's results.
    //
    prev_op->moveIntoObjectSpace(&prev_dst_rect, dst_obj);
    moveIntoObjectSpace(&cur_dst_rect, dst_obj);
    if(cur_dst_rect < prev_dst_rect) {
        return FALSE;
    }

    return TRUE;
}

//------------------------------------------------------------------------
//
//  Function:	isOkForMolecule()
//
//  Description:
//	Does a quick check to verify the operation can be inserted into
//	a molecule.
//
//      It follows XIL 1.2 rules.
//	
//------------------------------------------------------------------------
Xil_boolean
XilOp::isOkForMolecule()
{
    XilDeferrableObject* dst_obj = dst[0];

    //
    //  If our destination is a device image, then it doesn't matter.
    //
    if(dst_obj->getDeviceIO() != NULL) {
        return TRUE;
    }

    //
    //  Test the origin on all of the images...if they're not all equal, then
    //  we're not OK for molecule use.
    //
    if(dst_obj->getType() == XIL_IMAGE) {
        XilImage* dst_img = (XilImage*)dst_obj;
        float x_origin;
        float y_origin;
        dst_img->getOrigin(&x_origin, &y_origin);

        for(unsigned int i=0; i<numSrcs; i++) {
            if(src[i]->getType() == XIL_IMAGE) {
                XilImage* src_img = (XilImage*)src[i];

                if(x_origin != src_img->getOriginX() ||
                   y_origin != src_img->getOriginY()) {
                    return FALSE;
                }
            } else {
                if(x_origin != 0.0F ||
                   y_origin != 0.0F) {
                    return FALSE;
                }
            }
        }
    } else {
        //
        //  Verify the sources have 0.0 origins to match the non-image
        //  destination.
        //
        for(unsigned int i=0; i<numSrcs; i++) {
            if(src[i]->getType() == XIL_IMAGE) {
                XilImage* src_img = (XilImage*)src[i];

                if(src_img->getOriginX() != 0.0F ||
                   src_img->getOriginY() != 0.0F) {
                    return FALSE;
                }
            }
        }
    }

    //
    //  Test the ROI version on all of the images.  If they're not the same,
    //  then we can't be put into a molecule.
    //
    XilVersion           roi_version;
    XilRoi*              dst_roi      = dst_obj->refRoi();
    Xil_boolean          roi_mismatch = FALSE;
    if(dst_roi != NULL) {
        dst_roi->getVersion(&roi_version);
    }

    if(numSrcs > 0) {
        for(unsigned int i=0; i<numSrcs; i++) {
            //
            //  Check the ROI version.
            //
            roi_mismatch =
                _XILI_CHECK_ROI_MISMATCH(src[i]->refRoi(),
                                         dst_roi, &roi_version);
            if(roi_mismatch) {
                return FALSE;
            }
        }
    }

    return TRUE;
}


//------------------------------------------------------------------------
//
//  Function:	flushForward()
//
//  Description:
//	Cycles the DAG back to the top forward-mapping operation and 
//	begins flushing down from there -- much like XIL 1.2.
//	
//	
//  Deficiencies/ToDo:
//	TODO: 3/13/96 jlf  flushForward() isn't fully implemented
//
//        The algorithms for forward flushing from a capture only work
//        for the single op directly below the capture operation.
//	
//------------------------------------------------------------------------
XilStatus
XilOp::flushForward(XiliRect*     rect,
                    unsigned int  ,
                    XilOp*        ,
                    XilTileNumber )
{
    //
    //  Get the tile list for the destination region so we know what tiles to
    //  flush out of our depdendents.
    //
    XilTileList tile_list(dst[0]->getSystemState());
    if(dst[0]->getTileList(&tile_list, rect) == XIL_FAILURE) {
        return XIL_FAILURE;
    }

    //
    //  We now flush all of the operations that are dependent upon our
    //  destination image because the destination data has been filled in now.
    //
    XilTileNumber tile_number;
    while(tile_list.getNextTileNumber(&tile_number)) {
        //
        //  Mark the tile as evaluated since the data has been updated.
        //
        //  TODO: 7/25/96 jlf  We need a check that ensures the captured area
        //                     really is an entire tile.
        //
        dst[0]->setOpStatus(tile_number, dstQueuePos[0], XILI_EVALUATED);
    }

    //
    //  Wake up all of the threads that are waiting on the op
    //  status changing. 
    //
    opStatusCondVar.broadcast();

    if(dst[0]->flushDependents(&tile_list, this, dstQueuePos[0]) == XIL_FAILURE) {
        return XIL_FAILURE;
    }

    return XIL_SUCCESS;
}

//------------------------------------------------------------------------
//
//  Function:	flushStartOp()
//
//  Description:
//	This start/sets up/begins a capture from a PUSH I/O device 
//      which in turn flushes the operation(s) below the capture
//	operation with the newly available data.
//	
//------------------------------------------------------------------------
XilStatus
XilOp::flushStartOp(XilOp*       bottom_op,
                    XilTileList* )
{
    //
    //  Get the I/O device we're capturing from...
    //
    XilDeviceIO* device_io = src[0]->getDeviceIO();
    if(device_io == NULL) {
        return XIL_FAILURE;
    }

    //
    //  Setup the area in the final (bottom) operation that needs results
    //  generated.
    //
    if(bottom_op->generateIntersectedRoi() == XIL_FAILURE) {
        return XIL_FAILURE;
    }

    //
    //  Get its bounding box
    //
    XiliRect* clipped_gsbbox =
        bottom_op->intersectedRoi.getBoundingBox()->createCopy();
    if(clipped_gsbbox == NULL) {
        XIL_ERROR(getSystemState(), XIL_ERROR_RESOURCE, "di-1", TRUE);
        return XIL_FAILURE;
    }

    //
    //  Move it into global space so we're all talking the same stuff.
    //
    if(bottom_op->numDsts == 0) {
        if(bottom_op->moveIntoGlobalSpace(clipped_gsbbox,
                                          bottom_op->src[0]) == XIL_FAILURE) {
            return XIL_FAILURE;
        }
    } else {
        if(bottom_op->moveIntoGlobalSpace(clipped_gsbbox,
                                          bottom_op->dst[0]) == XIL_FAILURE) {
            return XIL_FAILURE;
        }
    }

    //
    //  TODO: 5/13/96 jlf  Capture op really MUST know what op is directly
    //                     below it.  The routines that set things up will
    //                     need to set a "nextOp" or some such on the capture
    //                     operation so we have more than just the "bottomOp".
    //                     We'll also need the branch number.
    //
    //  Figure out which source of the operation we go into.
    //  
    for(unsigned int i=0; i<bottom_op->numSrcs; i++) {
        if(this == bottom_op->srcOp[i]) {
            break;
        }
    }

    //
    //  Backward map into it's source (this op's destination -- except for the
    //  controlling image versus real image).  Remeber gsBackwardMap takes a
    //  source "number" of 1,2,3, etc. not 0,1,2, etc.
    //
    if(bottom_op->gsBackwardMap(clipped_gsbbox,
                                clipped_gsbbox, i+1) == XIL_FAILURE) {
        return XIL_FAILURE;
    }

    //
    //  Move the bbox into our source's object space.
    //
    if(moveIntoObjectSpace(clipped_gsbbox, src[0]) == XIL_FAILURE) {
        return XIL_FAILURE;
    }

    //
    //  Set the storage area for the box.
    //
    XilBox capture_box;
    if(setBoxStorage(clipped_gsbbox, src[0], &capture_box) == XIL_FAILURE) {
        return XIL_FAILURE;
    }
    
    //
    //  Clear the flag indicating that this is an unresolved forward capture
    //  operation.
    //
    forwardStartOp = NULL;

    //
    //  Handle show_action
    //
    Xil_boolean show_action = getSystemState()->getShowActionFlag();

    if(show_action) {
        fprintf(stderr, "--t%03d-- '%s' : 'capture()' Started\n",
                XiliThread::self(),
                device_io->getDeviceManager()->getDeviceName());
    }

    //
    //  Tell the device to capture its area in the source.  When
    //  XilDeviceIO::dataReady() is called, flushForward() is called to map
    //  the completed areas into the final destination image and flush the
    //  intermediate operations.
    //
    //  Set the capture information on the device so once it receives data
    //  from the device it can being a forward flush of the tile.
    //
    //  TODO: 12/13/96 jlf  tile_number is set to 0 which is ignored by the
    //                      forwardFlush routine because we've requested the
    //                      entire required area to be filled with the
    //                      startCapture() call.
    //
    device_io->setCaptureOpInfo(this, bottom_op, 0);

    //
    //  Actually do the entire capture.
    //
    XilStatus status = device_io->startCapture(&capture_box);

    if(show_action) {
        if(status == XIL_FAILURE) {
            fprintf(stderr, "--t%03d-- '%s' : 'capture()' Returned XIL_FAILURE\n",
                    XiliThread::self(),
                    device_io->getDeviceManager()->getDeviceName());
        } else {
            fprintf(stderr, "--t%03d-- '%s' : 'capture()' Finished\n",
                    XiliThread::self(),
                    device_io->getDeviceManager()->getDeviceName());
        }
    }

    return status;
}

//------------------------------------------------------------------------
//
//  Function:	updateStatus()
//
//  Description:
//	Updates the op status and returns whether flushTile() should
//	exit or not.
//	
//	
//  Returns:
//      TRUE if flushTile() should return, FALSE otherwise.
//	
//	
//  Side Effects:
//	Puts threads to sleep waiting on status to change.
//	
//  TODO: 7/25/96 jlf  Changed to answer per-tile
//	Right now, this just responds FALSE if any of the tiles need
//      flushing which causes all of the tiles in the list to be
//      executed.  Also, it waits until each of the tiles have been
//      moved out of "executing" state before returning an answer.
//      We want to have this return per-tile information stored on
//      the list.
//	
//------------------------------------------------------------------------
Xil_boolean
XilOp::updateStatus(XilTileList* tile_list)
{
    //
    //  If we have a destination, then the image we update is the parimary
    //  destionation which is stored in dst[0].  In the absence of a
    //  destination image, we use src[0].
    //
    XilDeferrableObject* pobj;
    XiliOpQueuePosition  qpos;
    if(numDsts == 0) {
        pobj = src[0];
        qpos = srcQueuePos[0];
    } else {
        pobj = dst[0];
        qpos = dstQueuePos[0];
    }

    //
    //  We assume that we don't need to flush any of the tiles in the list.
    //  If we encounter any that do require flushing, we reset the tile list
    //  and return FALSE. 
    //
    XilTileNumber tile_number;
    while(tile_list->getNextTileNumber(&tile_number)) {
        XiliOpStatus op_status = pobj->getOpStatus(tile_number, qpos);

        switch(op_status) {
          case XILI_DEFERRED:
            TNF_PROBE_2(xilop_flush_deferred, "xilop", "xilop_deferred",
                        tnf_opaque, "this", this,
                        tnf_long, "tile", tile_number);
            //
            //  Go ahead and flush.
            //
            tile_list->reset();
            return FALSE;

          case XILI_EXECUTING:
          {
              //
              //  Only internally created threads should find tiles in the
              //  EXECUTING state because the locks at the API only permit
              //  one API thread to be executing within a DAG at a time.
              //  This means that tile_list should have a mutex for us to
              //  sleep on...if it does not, we've got a problem.
              //
              TNF_PROBE_2(xilop_flush_evaluating, "xilop", "xilop_evaluating",
                          tnf_opaque, "this", this,
                          tnf_long, "tile", tile_number);

              //
              //  Bump the reference count so the op doesn't get deleted
              //  while we're asleep.
              //
              aquire();

              //
              //  Put the thread that called us asleep until the tile is
              //  completed.  Get the mutex stored on the tile_list.  If
              //  there isn't one, then we've got an internal problem.
              //
              XilMutex* mutex = tile_list->getMutex();

              if(mutex == NULL) {
                  XIL_ERROR(getSystemState(), XIL_ERROR_INTERNAL,
                            "di-430", TRUE);
                  tile_list->reset();
                  return TRUE;
              }

              while(pobj->getOpStatus(tile_number, qpos) != XILI_EVALUATED) {
                  opStatusCondVar.wait(mutex);
              }

              release();

              TNF_PROBE_2(xilop_flush_evaluating2, "xilop", "xilop_evaluating2",
                          tnf_opaque, "this", this,
                          tnf_long, "tile", tile_number);
          }
          //
          //  It's done already so keep our "don't have to flush status"
          //
          break;

          case XILI_EVALUATED:
            //
            //  It's done already so keep our "don't have to flush status"
            //
            break;

          default:
            //
            //  It's an UNKNOWN which is a BUG.  Don't try to flush
            //  something with a status we know nothing about how it got
            //  set.
            //
            XIL_ERROR(getSystemState(), XIL_ERROR_INTERNAL, "di-425", TRUE);
        }
    }

    return TRUE;
}

//------------------------------------------------------------------------
//
//  Function:	makeTileListForEntireDst()
//
//  Description:
//	
//      If an operation cannot be split, then we may need to generate a
//      new tile list representing the entire destination.  This function
//      takes a tile list and makes sure it contains all of the tiles
//      in the destination or it will create a new tile list containing
//      all of the tiles in the entire destination.  It's used by
//      flush(XilTileList*). 
//	
//------------------------------------------------------------------------
XilTileList*
XilOp::makeTileListForEntireDst(XilDeferrableObject* pobj,
                                XilTileList*         tile_list)
{
    XilTileList* tl = new XilTileList(getSystemState());
    if(tl == NULL) {
        XIL_ERROR(getSystemState(), XIL_ERROR_RESOURCE, "di-1", TRUE);
        return NULL;
    }

    //
    //  Initialize the tile_list to contain the one tile which is the
    //  entire object's region.
    //
    if(pobj->getTileList(tl) == XIL_FAILURE) {
        delete tl;
        return NULL;
    }

    //
    //  Copy the mutex reference from the given tile list.
    //
    tl->setMutex(tile_list->getMutex());

    return tl;
}

//------------------------------------------------------------------------
//
//  Function:	finishEmptyFlush()
//
//  Description:
//	The scheduler needs to finish empty flushes on a single tile
//	because the scheduler determines that a single tile may collapse
//	having nothing to execute.
//	
//	We need to finish empty tiles on the entire list because we've
//	found the ROI is empty and thus the operation is complete.
//	
//------------------------------------------------------------------------
XilStatus
XilOp::finishEmptyFlush(XilTileNumber tile_number,
                        XilMutex*     tile_list_mutex)
{
    //
    //  The operation isn't going to be done.  We do need to
    //  verify that any remaining operations on the op queue are
    //  completed.  Normally, calling the operation would
    //  accomplish this by its aquisition of storage on the
    //  destination.  In this case, no operation will be done so
    //  we bring things up-to-date here.
    //
    XilDeferrableObject* pobj;
    XiliOpQueuePosition  qpos;
    if(numDsts == 0) {
        pobj = src[0];
        qpos = srcQueuePos[0];
    } else {
        pobj = dst[0];
        qpos = dstQueuePos[0];

        XilTileList tl(pobj->getSystemState());

        tl.setNumTiles(1);
        tl.setEntry(0, tile_number);
        tl.setMutex(tile_list_mutex);

        if(pobj->syncForWriting(&tl, this, qpos) == XIL_FAILURE) {
            release();
            XIL_ERROR_WITH_ARG(getSystemState(),
                               XIL_ERROR_SYSTEM, "di-199", FALSE,
                               XilGlobalState::theXGS->lookupOpName(opNumber));
            return XIL_FAILURE;
        }
    }

    //
    //  We've "evaluated" the tile now, so mark it as EVALUATED.
    //
    pobj->setOpStatus(tile_number, qpos, XILI_EVALUATED);

    //
    //  Wake up all of the threads that are waiting on the op
    //  status changing. 
    //
    opStatusCondVar.broadcast();

    //
    //  If we've completed all of the tiles for this operation, then we
    //  consider ourself evaluated
    //
    if(pobj->allTilesDone(qpos)) {
        opStatus = XILI_EVALUATED;
    }

    //
    //  Since this is called multiple times, we don't call release(),
    //  XilOp::flush() takes care of releasing the reference it aquired.
    //

    return XIL_SUCCESS;
}

XilStatus
XilOp::finishEmptyFlush(XilTileList* tile_list)
{
    //
    //  The operation isn't going to be done.  We do need to
    //  verify that any remaining operations on the op queue are
    //  completed.  Normally, calling the operation would
    //  accomplish this by its aquisition of storage on the
    //  destination.  In this case, no operation will be done so
    //  we bring things up-to-date here.
    //
    XilDeferrableObject* pobj;
    XiliOpQueuePosition  qpos;
    if(numDsts == 0) {
        pobj = src[0];
        qpos = srcQueuePos[0];
    } else {
        pobj = dst[0];
        qpos = dstQueuePos[0];

        if(pobj->syncForWriting(tile_list, this, qpos) == XIL_FAILURE) {
            release();
            XIL_ERROR_WITH_ARG(getSystemState(),
                               XIL_ERROR_SYSTEM, "di-199", FALSE,
                               XilGlobalState::theXGS->lookupOpName(opNumber));
            return XIL_FAILURE;
        }
    }

    //
    //  We've "evaluated" the tile now, so mark it as EVALUATED.
    //
    XilTileNumber tile_number;
    while(tile_list->getNextTileNumber(&tile_number)) {
        pobj->setOpStatus(tile_number, qpos, XILI_EVALUATED);
    }

    //
    //  Wake up all of the threads that are waiting on the op
    //  status changing. 
    //
    opStatusCondVar.broadcast();

    //
    //  If we've completed all of the tiles for this operation, then we
    //  destroy ourselves...reference counting takes care of not having the op
    //  go away too soon...
    //
    if(pobj->allTilesDone(qpos)) {
        opStatus = XILI_EVALUATED;
    }

    //
    //  Release the reference aquired in flush().
    //
    release();

    return XIL_SUCCESS;
}

//------------------------------------------------------------------------
//
//  Function:	flush()
//
//  Description:
//	The flush routines for getting a list of tiles in an operation 
//	to be executed.
//	
//	
//	
//	
//	
//	
//  MT-level:  UNsafe
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
XilStatus
XilOp::flush(XilTileList* tile_list)
{
    //
    //  The counter we use throughout the routine.
    //
    unsigned int i;

    //
    //  The tile_number variable we use throughout the routine in those cases
    //  we need to iterate over the tile_list.
    //
    XilTileNumber tile_number;

    //
    //  If we have a destination, then the image we update is the parimary
    //  destionation which is stored in dst[0].  In the absence of a
    //  destination image, we use src[0].
    //
    XilDeferrableObject* pobj;
    XiliOpQueuePosition  qpos;
    if(numDsts == 0) {
        pobj = src[0];
        qpos = srcQueuePos[0];
    } else {
        pobj = dst[0];
        qpos = dstQueuePos[0];
    }

    //
    //  For operations that cannot be split in the destination, we create a
    //  new tile list that consumes the entire destination image and use it
    //  for record keeping and syncing all of the tiles correctly.
    //
    //  We set a flag which is valid for this routine indicating whether the
    //  scheduler needs to be notified not to split the operation we're
    //  requesting to have executed.  For example, it's possible that we'll
    //  form a molecule which has an operation that cannot be split.  This
    //  will require the scheduler not split the destination on threads or
    //  tiles.
    //
    XilTileList* tl;
    Xil_boolean  can_be_split = canBeSplit();
    if(! can_be_split) {
        //
        //  First check to see if the given tile list consumes all of the
        //  tiles in the destination.
        //
        if(pobj->getNumTiles() == tile_list->getNumTiles()) {
            tl = tile_list;
        } else {
            //
            //  makeTileListForEntireDst() will ensure the tile list contains all
            //  of the tiles in the destination image.  If it doesn't already have
            //  all the tiles available in the destination, it will generate a new
            //  tile list.
            //
            tl = makeTileListForEntireDst(pobj, tile_list);
            if(tl == NULL) {
                XIL_ERROR_WITH_ARG(getSystemState(),
                                   XIL_ERROR_SYSTEM, "di-199", FALSE,
                                   XilGlobalState::theXGS->lookupOpName(opNumber));
                return XIL_FAILURE;
            }
        }
    } else if(reordersTiles) {
        tl = new XilTileList(getSystemState());
        if(tl == NULL) {
            XIL_ERROR(getSystemState(), XIL_ERROR_RESOURCE, "di-1", TRUE);
            XIL_ERROR_WITH_ARG(getSystemState(),
                               XIL_ERROR_SYSTEM, "di-199", FALSE,
                               XilGlobalState::theXGS->lookupOpName(opNumber));
            return XIL_FAILURE;
        }

        if(reorderTileProcessing(tile_list, tl) == XIL_FAILURE) {
            for(i=0; i<numDsts; i++) {
                dst[i]->setValid(FALSE);
            }
            
            XIL_ERROR_WITH_ARG(getSystemState(),
                               XIL_ERROR_SYSTEM, "di-199", TRUE,
                               XilGlobalState::theXGS->lookupOpName(opNumber));
            return XIL_FAILURE;
        }

        tl->setMutex(tile_list->getMutex());
    } else {
        //
        //  Generally, it's the entire list of tiles.
        //
        tl = tile_list;
    }

    //
    //  Check the status of the tiles to see if any are deferred and need
    //  flushing.  If updateStaus() returns TRUE, then there isn't anything
    //  that needs to be flushed.
    //
    if(updateStatus(tl)) {
        if(tl != tile_list) {
            delete tl;
        }
        return XIL_SUCCESS;
    }

    //
    //  Aquire a reference to the op.
    //
    aquire();

    //
    //  Check to see if the operation is forward-mapping or backward-mapping
    //  and execute the appropriate algorithm.  For forward-mapping, we flush
    //  the ops starting at the top op and for backward-mapping, we flush the
    //  ops starting at the bottom op (this one).
    //
    //  TODO: 3/13/96 jlf  Only works for single op below capture
    //
    //    Currently all of the forward mapping code only works for the single
    //    op that sits below the capture.  XilOp::execute() ensures that
    //    operations starting at the capture flush immediately.
    //
    if(forwardStartOp != NULL) {
        XilOp* sop = forwardStartOp;

        forwardStartOp = NULL;

        XilStatus status = sop->flushStartOp(this, tl);

        //
        //  If we've completed all of the tiles for this operation, then we
        //  indicate the operation is done (as well as the starting
        //  operation)...reference counting takes care of not having the op go
        //  away too soon... 
        //
        if(pobj->allTilesDone(qpos)) {
            opStatus      = XILI_EVALUATED;
            sop->opStatus = XILI_EVALUATED;
        }

        release();

        if(tl != tile_list) {
            delete tl;
        }
        return status;
    }

    //
    //  The status of the most recent attempted execution of an operation.
    //
    XilStatus exec_status = XIL_FAILURE;

    do {
        //
        //  Test to see if the optimizedData class has already been filled by
        //  another thread or tile.  If it hasn't been filled, then look for
        //  a function to call for this operation.
        //
        if(funcDef == NULL) {
            //
            //  Find the longest chain of operations that can be executed.
            //
            funcDef = findLongestOpChain(&funcLength);
            if(funcDef == NULL) {
                //
                //  Ok, we've tried to find an operation by this operation's
                //  current name/number.  Now, it has an opportunity to alter
                //  itself to a different op (with significant limitations)
                //  and we'll keep looking.
                //
                if(switchToAlternateOp() == XIL_SUCCESS) {
                    //
                    //  Continue by going back to at the top after resetting
                    //  the maximum length to UINT_MAX.
                    //
                    funcLength = UINT_MAX;
                    continue;
                } else {
                    //
                    //  No alternate, so we have an operation that cannot be
                    //  executed.  Mark all of the destination objects in this
                    //  operation as invalid so they cannot be used as sources
                    //  in future operations until they've been written into
                    //  again.
                    //
                    for(i=0; i<numDsts; i++) {
                        dst[i]->setValid(FALSE);
                    }

                    XIL_ERROR_WITH_ARG(getSystemState(),
                                       XIL_ERROR_SYSTEM, "di-199", TRUE,
                                       XilGlobalState::theXGS->lookupOpName(opNumber));


                    //
                    //  None of the operation is going to be done.  We do need to
                    //  verify that any remaining operations on the op queue are
                    //  completed.  Normally, calling the operation would
                    //  accomplish this by its aquisition of storage on the
                    //  destination.  In this case, no operation will be done so
                    //  we bring things up-to-date here.  We don't care
                    //  whether syncForWriting() returns failure because we're
                    //  already in a failure condition.  We also get a new
                    //  tile list which is made up of all the tiles in the
                    //  object because if one tile fails to execute, all of
                    //  the tiles are considered to have failed.
                    //
                    XilTileList tmp_tl(pobj->getSystemState());
                    if(pobj->getTileList(&tmp_tl) == XIL_SUCCESS) {
                        pobj->syncForWriting(&tmp_tl, this, qpos);
                    }

                    //
                    //  It's considered evaluated since we've done everything
                    //  we can to execute it.
                    //
                    while(tl->getNextTileNumber(&tile_number)) {
                        pobj->setOpStatus(tile_number, qpos, XILI_EVALUATED);
                    }

                    //
                    //  Wake up all of the threads that are waiting on the op
                    //  status changing. 
                    //
                    opStatusCondVar.broadcast();
                    
                    opStatus = XILI_EVALUATED;

                    release();

                    if(tl != tile_list) {
                        delete tl;
                    }

                    return XIL_FAILURE;
                }
            }

            //
            //  Generate the op list if this is a molecule.  This is aquired
            //  by the compute routine and is used to step through the
            //  molecule to perform ROI intersections to determine if the
            //  molecule can be executed. 
            //
            if(funcLength > 1) {
                //
                //  Allocate enough space for a NULL-terminated list of ops.
                //
                opList = new XilOp*[funcLength+1];
                if(opList == NULL) {
                    //
                    //  Although, it implies that we won't be able to
                    //  continue, it's a molecule so we might be able to
                    //  execute an atomic operation.
                    //
                    funcDef    = NULL;
                    funcLength = UINT_MAX;
                    continue;
                }

                Xil_boolean molecule_op_cannot_be_split = FALSE;
                XilOp* opptr = this;
                for(i = 0; i<funcLength; i++) {
                    //
                    //  Check if the op can be inserted into a molecule.  This
                    //  follows XIL 1.2 guidelines...no origins, no mismatched
                    //  ROIs, etc.
                    //
                    if(! opptr->isOkForMolecule()) {
                        //
                        //  Break out and we'll detect the failure after the
                        //  loop. 
                        //
                        break;
                    }

                    //
                    //  If it's not the top op, then ensure it doesn't have
                    //  more than one source...at this point, molecules are
                    //  not supported with operations that have more than one
                    //  source for middle operations because the box list
                    //  entry does not support more than 4 boxes.  Thus, we
                    //  can't generate a list of boxes for every source yet.
                    //
                    if(i != (funcLength-1) && opptr->numSrcs != 1) {
                        //
                        //  Break out and we'll detect the failure after the
                        //  loop. 
                        //
                        break;
                    }

                    //
                    //  We can check for validity of the ops comprising the
                    //  molecule here.  For XIL 1.2 semantics, it should be an
                    //  op-specific test to determine if the operation
                    //  overwrites its entire destination.
                    //
                    opList[i] = opptr;

                    //
                    //  Check to see if the operation can or cannot be split
                    //  and update our can_be_split flag.
                    //
                    if(! opptr->canBeSplit()) {
                        molecule_op_cannot_be_split = TRUE;
                    }

                    //
                    //  Mark each of the atomic operations in the molecule as
                    //  being part of a molecule.  This is used to detect that
                    //  a molecule is in process as well as by
                    //  generateIntersectedRoi() so it knows to grab the ROI
                    //  from the op above it instead of the image.  Other
                    //  areas use this as well to ensure executing this
                    //  molecule doesn't attempt to flush operations in the
                    //  middle of it (this can occur with in-place
                    //  operations). 
                    //
                    opptr->moleculeBottom = this;

                    opptr = opptr->srcOp[0];
                }

                //
                //  Is the molecule good?
                //
                if(i != funcLength) {
                    //
                    //  Clear our moleculeBottom, funcDef, opList and shrink
                    //  the maximum length to find by 1. 
                    //
                    for(unsigned int k=0; k<i; k++) {
                        opList[k]->moleculeBottom = NULL;
                    }
                    funcDef = NULL;
                    delete opList;
                    opList  = NULL;
                    funcLength--;
                    continue;
                }

                //
                //  Apparently, one of the operations in the molecule cannot
                //  be split.
                //
                if(molecule_op_cannot_be_split) {
                    can_be_split = FALSE;
                    //
                    //  First check to see if the given tile list consumes all
                    //  of the tiles in the destination.  If it does, then
                    //  we're set.
                    //
                    if(pobj->getNumTiles() == tile_list->getNumTiles()) {
                        tl = tile_list;
                    } else {
                        //
                        //  makeTileListForEntireDst() will ensure the tile
                        //  list contains all of the tiles in the destination
                        //  image.  If it doesn't already have all the tiles
                        //  available in the destination, it will generate a
                        //  new tile list.
                        //
                        tl = makeTileListForEntireDst(pobj, tile_list);
                        if(tl == NULL) {
                            XIL_ERROR_WITH_ARG(getSystemState(),
                                               XIL_ERROR_SYSTEM, "di-199", FALSE,
                                               XilGlobalState::theXGS->lookupOpName(opNumber));
                            if(tl != tile_list) {
                                delete tl;
                            }
                            return XIL_FAILURE;
                        }
                    }
                }
            }
        }


        //
        //  Only need to mark the botom op as being evaluated because it's the
        //  only destination image which is actually having results generated
        //  into it.  The other ops in the molecule may need to be evaluated
        //  later so they are still considered "deferred" until their
        //  destinations are overwritten, tossed or destroyed.
        //
        while(tl->getNextTileNumber(&tile_number)) {
            pobj->setOpStatus(tile_number, qpos, XILI_EXECUTING);
        }

        //
        //  If the intersected ROIs have not already been set by a prior
        //  flushTile, perform the ROI intersections.
        //
        //  NOTE:  The resultant intersected ROIs are in IMAGE SPACE.  This is
        //         the ROI the compute routine will receive and is valid on
        //         the destination image (or source image if no destinations).
        //
        if(funcLength > 1) {
            //
            //  Generate the global space intersected ROIs.
            //
            for(int j = funcLength - 1; j>=0; j--) {
                if(opList[j]->intersectedRoi.isValid() == FALSE) {
                    if(opList[j]->generateIntersectedRoi() == XIL_FAILURE) {
                        //
                        //  This operation has no tiles to flush, but there
                        //  may be others on the opQueue that need flushing.
                        //
                        exec_status = finishEmptyFlush(tl);
                        if(tl != tile_list) {
                            delete tl;
                        }
                        return exec_status;
                    }
                }
            }
        } else {
            if(intersectedRoi.isValid() == FALSE) {
                if(generateIntersectedRoi() == XIL_FAILURE) {
                    //
                    //  This operation has no tiles to flush, but there may be
                    //  others on the opQueue that need flushing.
                    //
                    exec_status = finishEmptyFlush(tl);
                    if(tl != tile_list) {
                        delete tl;
                    }
                    return exec_status;
                }
            }
        }

        if(funcLength > 1) {
            //
            //  Move those ROIs into image space.
            //
            for(int j = funcLength - 1; j>=0; j--) {
                if(opList[j]->intersectedRoiIsInObjectSpace == FALSE) {
                    if(opList[j]->moveIntoObjectSpace(&opList[j]->intersectedRoi,
                                                      pobj) == XIL_FAILURE) {
                        //
                        //  This operation has no tiles to flush, but there
                        //  may be others on the opQueue that need flushing.
                        //
                        exec_status = finishEmptyFlush(tl);
                        if(tl != tile_list) {
                            delete tl;
                        }
                        return exec_status;
                    }

                    opList[j]->intersectedRoiIsInObjectSpace = TRUE;
                }
            }
        } else if(intersectedRoiIsInObjectSpace == FALSE) {
            //
            //  Move the ROI into image space.
            //
            if(moveIntoObjectSpace(&intersectedRoi, pobj) == XIL_FAILURE) {
                //
                //  This operation has no tiles to flush, but there
                //  may be others on the opQueue that need flushing.
                //
                exec_status = finishEmptyFlush(tl);
                if(tl != tile_list) {
                    delete tl;
                }
                return exec_status;
            }

            intersectedRoiIsInObjectSpace = TRUE;
        }

        //
        //  If this is the destination of a molecule and it's an I/O
        //  operation, then we need to call syncForWriting() because
        //  molecules that write directly to display devices do not aquire
        //  storage and thus syncForWriting() is never called.
        //
        //  NOTE:  Normally, we call syncForWriting() with the
        //         destination op queue position.  But since this is an I/O
        //         operation, it's in-place which means the preceeding
        //         operation has an op queue entry as well.  It's actually the
        //         queue position of the preceeding operation's destination
        //         to which we need evaluated.  This simulates the behavior of
        //         having the preceeding operation calling getStorage() on the
        //         backing image.
        //
        Xil_boolean is_display_molecule = FALSE;
        if(funcLength > 1 && numDsts != 0 && isIOOperation()) {
            if(pobj->syncForWriting(tl, this, qpos) == XIL_FAILURE) {
                opStatus = XILI_EVALUATED;
                release();
                if(tl != tile_list) {
                    delete tl;
                }
                XIL_ERROR_WITH_ARG(getSystemState(),
                                   XIL_ERROR_SYSTEM, "di-199", FALSE,
                                   XilGlobalState::theXGS->lookupOpName(opNumber));
                return XIL_FAILURE;
            }

            if(pobj->getDeviceIO() != NULL) {
                is_display_molecule = TRUE;
            }
        }

        TNF_PROBE_2(xilop_flush_executing, "xilop", "xilop_executing",
                    tnf_opaque, "this", this,
                    tnf_long, "tile", tile_number);

        //
        //  Now, go ahead and setup this tile to be scheduled for operation by
        //  the scheduler.
        //
        XiliOperation cop(this, funcLength, funcDef, tl, &intersectedRoi);

        if(can_be_split) {
            exec_status =
                XilGlobalState::theXGS->getOpScheduler()->execute(&cop);
        } else {
            exec_status =
                XilGlobalState::theXGS->getOpScheduler()->executeWithNoSplit(&cop);
        }            

        //
        //  See if we have empty tiles we need to flush...
        //
        if(cop.getEmptyFlushList() != NULL) {
            XiliSLList<XilTileNumber>* eflist = cop.getEmptyFlushList();

            XiliSLListIterator<XilTileNumber> li(eflist);
            while(li.getNext(tile_number) == XIL_SUCCESS) {
                if(finishEmptyFlush(tile_number,
                                    tl->getMutex()) == XIL_FAILURE) {
                    exec_status = XIL_FAILURE;
                }
            }
        }

        if(exec_status == XIL_FAILURE) {
            //
            //  Unable to execute the specified function
            //
            if(funcLength > 1) {
                for(i=0; i<funcLength; i++) {
                    opList[i]->moleculeBottom = NULL;
                }

                //
                //  Clear the op list...
                //
                delete opList;
                opList = NULL;
            }

            //
            //  Clear our funcDef and shrink the maximum length to find by 1.
            //
            funcDef = NULL;
            funcLength--;

            //
            //  Clear the intersected ROI to an initial state.
            //
            intersectedRoi.clear();
        } else {
            //
            //  If it was a molecule writing to an I/O device, then we're
            //  going to need to mark the backing object storage as invalid.
            //  The capture op's destructor will mark the object's storage as
            //  valid.
            //
            if(is_display_molecule) {
                pobj->setStorageValidFlag(FALSE);
            }

            //
            //  Operation completed successfully -- we're outta here.
            //
            break;
        }
    } while(exec_status == XIL_FAILURE);

    TNF_PROBE_2(xilop_flush_executed, "xilop", "xilop_executed",
                tnf_opaque, "this", this,
                tnf_long, "tile", tile_number);

    //
    //  We've evaluated the tile now, so mark it as EVALUATED.
    //
    while(tl->getNextTileNumber(&tile_number)) {
        pobj->setOpStatus(tile_number, qpos, XILI_EVALUATED);
    }

    //
    //  Wake up all of the threads that are waiting on the op status changing.
    //
    opStatusCondVar.broadcast();

    //
    //  If we've completed all of the tiles for this operation, then we
    //  destroy ourselves...reference counting takes care of not having the op
    //  go away too soon...
    //
    if(pobj->allTilesDone(qpos)) {
        opStatus = XILI_EVALUATED;
    }

    release();

    if(tl != tile_list) {
        delete tl;
    }
    return XIL_SUCCESS;
}


//------------------------------------------------------------------------
//
//  Function:	flush()
//
//  Description:
//	
//	Flush the entire destination object.
//	
//------------------------------------------------------------------------
XilStatus
XilOp::flush()
{
    XilTileList tl(getSystemState());

    if(numDsts == 0) {
        //
        //  Special case of no destination image.
        //
        if(src[0]->getTileList(&tl) == XIL_FAILURE) {
            XIL_ERROR_WITH_ARG(getSystemState(),
                               XIL_ERROR_SYSTEM, "di-199", TRUE,
                               XilGlobalState::theXGS->lookupOpName(opNumber));
            opStatus = XILI_EVALUATED;
            return XIL_FAILURE;
        }
    } else {
        if(dst[0]->getTileList(&tl) == XIL_FAILURE) {
            XIL_ERROR_WITH_ARG(getSystemState(),
                               XIL_ERROR_SYSTEM, "di-199", TRUE,
                               XilGlobalState::theXGS->lookupOpName(opNumber));
            opStatus = XILI_EVALUATED;
            return XIL_FAILURE;
        }
    }

    //
    //  Flush the tiles in the tile list...
    //
    XilStatus status = flush(&tl);

    return status;
}

//------------------------------------------------------------------------
//
//  Function:	findLongestOpChain()
//
//  Description:
//	
//	Locate the longest chain of operations we can execute from this
//	op -- excluding those given to us on the "exclude list".
//	
//------------------------------------------------------------------------
XiliOpTreeNode*
XilOp::findLongestOpChain(unsigned int* max_length)
{
    XiliOpTreeNode* current_node   = XilGlobalState::theXGS->getOpTreeBase();
    XiliOpTreeNode* found_node     = NULL;
    int             current_length = 0;
    int             found_length   = 0;
    XilOp*          opptr          = this;

    while(opptr) {
        XiliOpTreeNode* next_node;

        //
        //  Is it possible to go further?
        //
        if((current_node->getNextState() == NULL)            ||
           (opNumber < 0)                                    ||
           (current_node->getStateSize() <= (unsigned int )opptr->opNumber) ||
           ((unsigned int)current_length >= *max_length)                   ||
           ((next_node =
             (current_node->getNextState()[opptr->opNumber])) == NULL)) {
            break;
        }

        //
        //  Yes, continue further down the chain...
        //
        opptr = opptr->srcOp[0];
        current_node = next_node;

        //
        //  Since it is a single chain, we keep our own length instead of
        //  making the extra effort to look at the function list information.
        //
        current_length++;

        //
        //  Are there any functions here?  If so, we've found a potential
        //  stopping point.
        //
        if(current_node->getFunctionList()) {
            found_node   = current_node;
            found_length = current_length;
        }

        if(opptr && (opptr->opStatus != XILI_DEFERRED)) break;
    }

    //
    //  We need to release our lock on the Op Tree
    //
    XilGlobalState::theXGS->releaseOpTreeBase();

    *max_length = found_length;

    return found_node;
}

//
//  Change our refereces to the given non-deferrable object to the new
//  non-deferrable object. 
//
void
XilOp::updateObjectReferences(XilNonDeferrableObject* oldobj,
                              XilNonDeferrableObject* newobj,
                              Xil_boolean             destroy)
{
    for(unsigned int i=0; i<numParams; i++) {
        if(params[i].o == oldobj) {
            //
            //  Found the old object, now update our pointer with the new value.
            //
            //  TODO: 3/1/96 jlf  Should getting a reference allow addl. data?
            //
            //    When the op gets a reference, if the object permitted us to
            //    store extra data, then we wouldn't need to search our
            //    parameters to find the object. 
            //
            params[i].o = newobj;

            if(destroy == TRUE) {
                paramDestroyMethod[i] = XIL_DESTROY;
            }
        }
    }
}

XilSystemState*
XilOp::getSystemState()
{
    //
    //  In the normal case, we'll always find a system state on one of our
    //  deferrable objects.  It's not worth testing for anything other than
    //  NULL because its a degenerate case where we would end up leaving this
    //  routine without having systemState set.
    //
    if(systemState == NULL) {
        unsigned int i;
        for(i=0; i<numDsts; i++) {
            if(dst[i]->getSystemState() != NULL) {
                systemState = dst[i]->getSystemState();
            }
        }

        for(i=0; i<numSrcs; i++) {
            if(src[i]->getSystemState() != NULL) {
                systemState = src[i]->getSystemState();
            }
        }
    }

    return systemState;
}

XiliDagRef*
XilOp::setupAndLockDAG()
{
    unsigned int i;

    //
    //  Start by locking the dag manager so we're the only ones accessing and
    //  modifying the DAG information.  It's also required to be held in order
    //  to lock a DAG.
    //
    XiliDagManager* dag_manager = XilGlobalState::theXGS->getDagManager();
    dag_manager->lock();

    if(dagRef == NULL) {
        XiliDagRef* src_dag_refs[XIL_MAX_SRC_DEFOBJS];
        XiliDagRef* dst_dag_refs[XIL_MAX_DST_DEFOBJS];
        //
        //  For now, we use a simple algorithm to determine which DAG to insert
        //    ourselves into.   Basically, we pick the first one we find.
        //
        //  Also, we aquire a reference to those DAGs as well so they don't go
        //  away between the time we read them and then merge/set them.
        //
        for(i=0; i<numSrcs; i++) {
            src_dag_refs[i] = src[i]->getDagRef();
            if(src_dag_refs[i] != NULL) {
                src_dag_refs[i]->aquire();
                if(dagRef == NULL) {
                    dagRef = src_dag_refs[i];
                }
            }
        }
        for(i=0; i<numDsts; i++) {
            dst_dag_refs[i] = dst[i]->getDagRef();
            if(dst_dag_refs[i] != NULL) {
                dst_dag_refs[i]->aquire();
                if(dagRef == NULL) {
                    dagRef = dst_dag_refs[i];
                }
            }
        }

        //
        //  None of the objects we're attached to already reside in a DAG.  So,
        //    create a new DAG.
        //
        if(dagRef == NULL) {
            dagRef = dag_manager->getNewDAG(getSystemState());

            if(dagRef == NULL) {
                return NULL;
            }
        } else {
            //
            //  We've got a dag.  So, in order to store it we need to aquire a
            //  new reference to the DagRef.
            //
            dagRef = dagRef->aquire();
        }

        //
        //  Now go through and merge all the sources and destinations.
        //
 iterate_object_DAGs:
        for(i=0; i<numSrcs; i++) {
            if(src_dag_refs[i] == NULL) {
                //
                //  Since src[i] doesn't have a DAG, we just set a new one.
                //
                dagRef->aquire();
                src[i]->setDagRef(dagRef);
            } else if(! dagRef->isEqual(src_dag_refs[i])) {
                //
                //  They're different so merge srcDagRef into dagRef...the dag
                //  manager takes care of locking the src's DAG and releasing
                //  the old dag reference.
                //
                //  After merging, another thread may have modified an image's
                //  dag reference.  So, we need to reverify our sources are
                //  setup ok.
                //
                dagRef = dag_manager->mergeDAGs(dagRef, src_dag_refs[i]);
                goto iterate_object_DAGs;
            }
        }
        for(i=0; i<numDsts; i++) {
            if(dst_dag_refs[i] == NULL) {
                //
                //  Since dst[i] doesn't have a DAG, we just set a new one.
                //
                dagRef->aquire();
                dst[i]->setDagRef(dagRef);
            } else if(! dagRef->isEqual(dst_dag_refs[i])) {
                //
                //  They're different so merge dstDagRef into dagRef...the dag
                //  manager takes care of locking the dst's DAG and releasing
                //  the old dag reference.
                //
                //  After merging, another thread may have modified an image's
                //  dag reference.  So, we need to reverify our sources are
                //  setup ok.
                //
                dagRef = dag_manager->mergeDAGs(dagRef, dst_dag_refs[i]);
                goto iterate_object_DAGs;
            }
        }
        //
        //  For those parameters that we're holding as references, we need
        //  to set a DAG reference for them as well since they're effectively
        //  deferred along with us.  Those which are copies are ours and
        //  we don't need to worry about other operations using them.
        //
        for(i=0; i<numParams; i++) {
            if(paramDestroyMethod[i] == XIL_RELEASE_REF) {
                XiliDagRef* dref = params[i].o->getDagRef();

                if(dref == NULL) {
                    //
                    //  Since the object doesn't have a DAG, we just set a
                    //  new one. 
                    //
                    dagRef->aquire();
                    params[i].o->setDagRef(dagRef);
                } else if(! dagRef->isEqual(dref)) {
                    //
                    //  They're different so merge dref into dagRef...the dag
                    //  manager takes care of locking the obj's DAG and
                    //  releasing the old dag reference.
                    //
                    //  After merging, another thread may have modified an
                    //  obj's dag reference.  So, we need to reverify our
                    //  the objects are setup ok.
                    //
                    dagRef = dag_manager->mergeDAGs(dagRef, dref);
                    goto iterate_object_DAGs;
                }
            }
        }

        //
        //  Now, we aquire the lock to the DAG.
        //
        dagRef->lockDag();

        //
        //  Release our references...
        //
        for(i=0; i<numSrcs; i++) {
            if(src_dag_refs[i] != NULL) {
                src_dag_refs[i]->release();
            }
        }
        for(i=0; i<numDsts; i++) {
            if(dst_dag_refs[i] != NULL) {
                dst_dag_refs[i]->release();
            }
        }
    } else {
        //
        //  Just aquire the lock.
        //
        dagRef->lockDag();
    }

    XiliDagRef* ret_ref = dagRef->aquire();

    dag_manager->unlock();

    return ret_ref;
}

//
//  The default implementation of the virtual split on tile boundaries.
//
XilStatus
XilOp::vSplitOnTileBoundaries(XilBoxList* boxlist)
{
    //
    //  For each of our sources, divide the box list based on the tile
    //  boundaries in each source.  No need to check for dividing the box list
    //  if there is only one tile in the source.
    //
    for(unsigned int i=0; i<numSrcs; i++) {
        if(src[i]->getNumTiles() > 1) {
            //
            //  Get the size of the tiles.
            //
            unsigned int txsize, tysize;
            src[i]->getTileSize(&txsize, &tysize);

            //
            //  Call divideBoxList() which does the real work.
            //
            if(divideBoxList(boxlist, i, txsize, tysize) == XIL_FAILURE) {
                return XIL_FAILURE;
            }
        }
    }

    return XIL_SUCCESS;
}

//
//  Default version of gsBackwardMap just copies the rect
//
XilStatus
XilOp::gsBackwardMap(XiliRect*    dst_rect,
		     XiliRect*    src_rect,
		     unsigned int )
{
    if(dst_rect == src_rect) {
        //
        //  In-place -- do nothing
        // 
        return XIL_SUCCESS;
    }

    src_rect->set(dst_rect);

    return XIL_SUCCESS;
}

//
//  Default version of gsForwardMap just copies the rect
//
XilStatus
XilOp::gsForwardMap(XiliRect*     src_rect,
		    unsigned int  ,
		    XiliRect*     dst_rect)
{
    if(src_rect == dst_rect) {
        //
        //  In-place -- do nothing
        // 
        return XIL_SUCCESS;
    }

    dst_rect->set(src_rect);

    return XIL_SUCCESS;
}

//
//  The default case of backward mapping a single point from a destination box
//  to source box is a noop.  Box space is the same between source and
//  destination for most operations.
//
XilStatus
XilOp::vBackwardMap(XilBox*      ,
                    double       dx,
                    double       dy,
                    XilBox*      ,
                    double*      sx,
                    double*      sy,
                    unsigned int )
{
    *sx = dx;
    *sy = dy;
    
    return XIL_SUCCESS;
}

//--------------------------------------------------------------
//
//  XilOp Data Collection operators
//
//--------------------------------------------------------------
XilStatus
XilOp::vReportResults(void*[])
{
    return XIL_SUCCESS;
}

XilStatus
XilOp::completeResults()
{
    return XIL_SUCCESS;
}

//
//  Permits the op to reorder the processing of tiles by the core.  This
//  is only called if reorderTiles is set.
//
XilStatus
XilOp::reorderTileProcessing(XilTileList*,
                             XilTileList*)
{
    return XIL_FAILURE;
}

