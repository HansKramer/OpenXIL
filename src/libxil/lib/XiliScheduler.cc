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
//  File:	XiliScheduler.cc
//  Project:	XIL
//  Revision:	1.91
//  Last Mod:	10:08:26, 03/10/00
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
#pragma ident	"@(#)XiliScheduler.cc	1.91\t00/03/10  "

#ifdef _WINDOWS
#include <signal.h>
#else
#include <sys/signal.h>
#endif

#include "_XilDefines.h"
#include "_XilGlobalState.hh"
#include "_XilDeviceManagerCompute.hh"
#include "_XilDeviceIO.hh"
#include "_XilDeferrableObject.hh"

//
//  TODO:  2/8/96 jlf Temporarily needed.
//
#include "_XilCis.hh"
#include "_XilImage.hh"

#include "XiliScheduler.hh"

//
//  This is a thread entry point which kicks off the compute routine processing
//  routine.
//
void*
xili_start_compute_routine(void* args)
{
    XiliExecContext* context   = (XiliExecContext*)args;
    XiliScheduler*   scheduler = context->scheduler;

    if(context->sleepUntilDone) {
        while(! context->isDone) {
            scheduler->startComputeOperation(context);

            //
            //  Put this thread to sleep and have it call the routine again when
            //  awaken. 
            //
            scheduler->readyListMutex.lock();

            scheduler->readyList.append(context);

            context->listPosition = _XILI_LIST_INVALID_POSITION;

            while(context->listPosition == _XILI_LIST_INVALID_POSITION) {
                context->isReady.wait(&context->scheduler->readyListMutex);
            }

            scheduler->readyListMutex.unlock();
        }
    } else {
        scheduler->startComputeOperation(context);
    }

    return NULL;
}

//
//  For thr_setconcurrency().
//
#ifdef SOLARIS
#include <thread.h>
#endif

XiliScheduler::XiliScheduler()
{
#if defined(_XIL_USE_PTHREADS) || defined(_XIL_USE_SOLTHREADS) || \
    defined(_XIL_USE_WINTHREADS)
    unsigned int nthreads = XilGlobalState::theXGS->getNumThreads();

    if(nthreads == 1) {
        //
        //  Initialize flag...
        //
        noThreads   = TRUE;
        threadSplit = FALSE;
    } else {
        noThreads = FALSE;

        if(nthreads != 0) {
            numCPUs = nthreads;
        } else {
            numCPUs = xili_sysconf(_SC_NPROCESSORS_ONLN);
        }

        //
        //  On MP systems, we split tiles into strips based on the
        //  number of CPUs in the system.
        //
        threadSplit = TRUE;

        splitThreshold = XilGlobalState::theXGS->getSplitThreshold();
        if(splitThreshold == 0) {
            splitThreshold = 32;
        }

#ifdef SOLARIS
        //
        //  We need to set the concurrency so the CPUs are utilized.
        //
        thr_setconcurrency(numCPUs+1);
#endif
    }
#else
    //
    //  No thread library is being compiled in so disable scheduling threads.
    //
    noThreads   = TRUE;
    threadSplit = FALSE;
#endif
}

XiliScheduler::~XiliScheduler()
{
    //
    //  Delete any threads we may have created.
    //
    readyListMutex.lock();

    while(readyList.head() != _XILI_SLLIST_INVALID_POSITION) {
        XiliExecContext* ec;

        readyList.remove(readyList.head(), ec);

        //
        //  Indicate to the thread that we're done with it, signal
        //  it to wake up and release the lock it's waiting on.
        //
        ec->isDone       = TRUE;
        ec->listPosition = (void*)TRUE;

        ec->isReady.signal();

        readyListMutex.unlock();

        //
        //  Joining the thread with the current thread.
        //
        ec->thread->join();

        //
        //  Delete the thread class.
        //
        delete ec->thread;

        //
        //  Delete the exec context.
        //
        delete ec;

        //
        //  Relock the readyList to test the head() and remove the next entry.
        //
        readyListMutex.lock();
    }

    readyListMutex.unlock();
}

XilStatus
XiliScheduler::execute(XiliOperation* operation)
{
    //
    //  The tile list may have a mutex for us to unlock.  If it does not,
    //  then there is no problem.
    //
    XilMutex* tile_list_mutex = operation->tileList->getMutex();
    if(tile_list_mutex != NULL) {
        tile_list_mutex->unlock();
    }

    operation->scheduler = this;

    XilOp*        op        = operation->op;
    XilRoi*       roi       = operation->roi;
    XilTileList*  tile_list = operation->tileList;

    //
    //  Construct the context for this thread.
    //
    XiliExecContext primary_context(operation, FALSE);

    //
    //  Copy the XiliRect representing the ROI bounding box that will be the
    //  bounding box we continue to modify.
    //
    //  XilOpCopyPattern::vSplitOnTileBoundaries currently modifies the
    //  intersected Roi. When running threads this can collide with
    //  the subsequent setting of gsbbox unless we store the bbox of
    //  the intersected roi before entering the while loop.
    //
    XiliRect*   roi_bbox = roi->getBoundingBox();

    XiliRectInt full_gsbbox_int;
    XiliRectDbl full_gsbbox_dbl;
    XiliRect*   full_gsbbox;

    XiliRectInt gsbbox_int;
    XiliRectDbl gsbbox_dbl;
    XiliRect*   gsbbox;
    if(roi_bbox->getType() == XILI_RECT_TYPE_INT) {
        full_gsbbox_int = *((XiliRectInt*)roi_bbox);
        full_gsbbox     = &full_gsbbox_int;
        gsbbox          = &gsbbox_int;
    } else {
        full_gsbbox_dbl = *((XiliRectDbl*)roi_bbox);
        full_gsbbox     = &full_gsbbox_dbl;
        gsbbox          = &gsbbox_dbl;
    }

    XilTileNumber tnum;
    while(tile_list->getNextTileNumber(&tnum)) {
        //
        //  Set the operation's primary deferrable object.
        //
        XilDeferrableObject* defobj;
        XiliOpQueuePosition  qpos;
        if(op->getNumDsts() == 0) {
            defobj = op->getSrc(1);
            qpos   = op->getSrcOpQueuePosition((unsigned int)0);
        } else {
            defobj = op->getDst(1);
            qpos   = op->getDstOpQueuePosition((unsigned int)0);
        }

        //
        //  TODO: 1/16/97 jlf  Although each tile in the list was checked by
        //                     updateStatus() in flush(), there is no
        //                     mechanism to communicate which tiles have an
        //                     haven't been except by removing them from a
        //                     list which is shared with the storage routine
        //                     in XilImage.  Either a new list must be
        //                     constructed or we must check here.  For now, I
        //                     check here, but it may be better to construct a
        //                     list.
        //
        //  Verify this particular tile hasn't been evaluated yet.
        //
        if(defobj->getOpStatus(tnum, qpos) != XILI_EXECUTING) {
            continue;
        }

        //
        //  Reset the global space bounding box to be the entire ROI's
        //  bounding box for processing each tile.  Reset the operation's
        //  flags... 
        //
        gsbbox->set(full_gsbbox);

        //
        //  Indicate the operation has not completed yet.
        //
        operation->operationComplete = FALSE;

        //
        //  We clip the bounding box of the intersectedRoi to this tile.
        //
        //  NOTE:  This is all done in IMAGE SPACE since it matches the ROI.
        //
        Xil_boolean threads_started = FALSE;
        if(op->clipToTile(defobj, tnum, gsbbox) == XIL_FAILURE) {
            //
            //  This tile has no area which requires execution, but
            //  there may be others on the opQueue that need
            //  flushing. 
            //
            operation->addEmptyFlushTile(tnum);
            goto complete_operation;
        }

        //
        //  The gsbbox needs to stay in object space until after it
        //  is split for threads. This ensures that the object space
        //  rects produced will have internal integer boundaries which
        //  is necessary to avoid the rounding needed to go from
        //  XiliRects to XilBoxes causing the boxes to overlap.
        //

        //
        //  Are we starting threads?  If so, then go start the threads and
        //  then the gsbbox will be left with just what this thread is
        //  expected to execute.
        //
        if(threadSplit) {
            if(startThreads(operation, gsbbox,
                            tnum, &threads_started) == XIL_FAILURE) {
                //
                //  A major execution error occured while attempting to start
                //  new threads...
                //
                return XIL_FAILURE;
            }
        }
        
        //
        //  Move the remainder bounding box into global space.
        //  If no threads were split then this contains the whole gsbbox
        //  for a given tile.
        //
        if(op->moveIntoGlobalSpace(gsbbox, defobj) == XIL_FAILURE) {
            //
            //  This tile has no area which requires execution, but
            //  there may be others on the opQueue that need
            //  flushing. 
            //
            operation->addEmptyFlushTile(tnum);
            goto complete_operation;
        }

        if(op->generateBoxList(&primary_context.boxList,
                               gsbbox) == XIL_FAILURE) {
            //
            //  This tile has no area which requires execution, but
            //  there may be others on the opQueue that need
            //  flushing. 
            //
            operation->addEmptyFlushTile(tnum);
            goto complete_operation;
        }

        //
        //  If the bbox is emtpy after generateBoxList(), then we don't need
        //  to execute this portion of the operation.  The caller to
        //  execOperation() can check the bbox as well to see if it is
        //  empty and handle the empty case.
        //
        if(gsbbox->isEmpty()) {
            //
            //  This tile has no area which requires execution, but
            //  there may be others on the opQueue that need
            //  flushing. 
            //
            operation->addEmptyFlushTile(tnum);
            goto complete_operation;
        }

        startComputeOperation(&primary_context);

        //
        //  This is here so its easy to skip from an empty flush and possibly
        //  wait for other threads to finish.
        //
 complete_operation:
        if(threads_started) {
            //
            //  Lock the operation so we can read it when we're done.
            //
            operation->lock();

            while(!operation->operationComplete) {
                operation->completeCond.wait(&operation->mutex);
            }

            operation->unlock();

            //
            //  If one of the tiles failed to be processed, then the entire
            //  operation is considered failed.
            //
            if(operation->status == XIL_FAILURE) {
                break;
            }

        }
    }

    //
    //  Reaquire our tile list lock before returning.
    //
    if(tile_list_mutex != NULL) {
        tile_list_mutex->lock();
    }

    return operation->status;
}

XilStatus
XiliScheduler::executeWithNoSplit(XiliOperation* operation)
{
    //
    //  The tile list may have a mutex for us to unlock.  If it does not,
    //  then there is no problem.
    //
    XilMutex* tile_list_mutex = operation->tileList->getMutex();
    if(tile_list_mutex != NULL) {
        tile_list_mutex->unlock();
    }

    operation->scheduler = this;

    XilOp*        op        = operation->op;
    XilRoi*       roi       = operation->roi;

    //
    //  Copy the XiliRect representing the ROI bounding box that will be the
    //  bounding box we continue to modify.
    //
    //  XilOpCopyPattern::vSplitOnTileBoundaries currently modifies the
    //  intersected Roi. When running threads this can collide with
    //  the subsequent setting of gsbbox unless we store the bbox of
    //  the intersected roi before entering the while loop.
    //
    XiliRect*   roi_bbox = roi->getBoundingBox();

    XiliRectInt gsbbox_int;
    XiliRectDbl gsbbox_dbl;
    XiliRect*   gsbbox;
    if(roi_bbox->getType() == XILI_RECT_TYPE_INT) {
        gsbbox_int = *((XiliRectInt*)roi_bbox);
        gsbbox     = &gsbbox_int;
    } else {
        gsbbox_dbl = *((XiliRectDbl*)roi_bbox);
        gsbbox     = &gsbbox_dbl;
    }

    //
    //  Construct the context for this thread.
    //
    XiliExecContext primary_context(operation, FALSE);

    //
    //  Set the operation's primary deferrable object.
    //
    XilDeferrableObject* defobj;
    XiliOpQueuePosition  qpos;
    if(op->getNumDsts() == 0) {
        defobj = op->getSrc(1);
        qpos   = op->getSrcOpQueuePosition((unsigned int)0);
    } else {
        defobj = op->getDst(1);
        qpos   = op->getDstOpQueuePosition((unsigned int)0);
    }

    //
    //  Indicate the operation has not completed yet.
    //
    operation->operationComplete = FALSE;

    //
    //  Move the remainder bounding box into global space.
    //  If no threads were split then this contains the whole gsbbox
    //  for a given tile.
    //
    if(op->moveIntoGlobalSpace(gsbbox, defobj) == XIL_FAILURE) {
        //
        //  This tile has no area which requires execution, but
        //  there may be others on the opQueue that need
        //  flushing. 
        //
        operation->addEmptyFlushTile(0);
        goto complete_operation;
    }

    if(op->generateBoxList(&primary_context.boxList,
                           gsbbox) == XIL_FAILURE) {
        //
        //  This tile has no area which requires execution, but
        //  there may be others on the opQueue that need
        //  flushing. 
        //
        operation->addEmptyFlushTile(0);
        goto complete_operation;
    }

    //
    //  If the bbox is emtpy after generateBoxList(), then we don't need
    //  to execute this portion of the operation.  The caller to
    //  execOperation() can check the bbox as well to see if it is
    //  empty and handle the empty case.
    //
    if(gsbbox->isEmpty()) {
        //
        //  This tile has no area which requires execution, but
        //  there may be others on the opQueue that need
        //  flushing. 
        //
        operation->addEmptyFlushTile(0);
        goto complete_operation;
    }

    startComputeOperation(&primary_context);

    //
    //  This is here so its easy to skip from an empty flush.
    //
 complete_operation:
    //
    //  Reaquire our tile list lock before returning.
    //
    if(tile_list_mutex != NULL) {
        tile_list_mutex->lock();
    }

    return operation->status;
}


XilStatus
XiliScheduler::startThreads(XiliOperation* operation,
                            XiliRect*      gsbbox,
                            XilTileNumber  tnum,
                            Xil_boolean*   threads_started)
{
    *threads_started = FALSE;

    //
    //  Lock the portion of the operation that allows us to update the
    //  context list and other shared data in the structure.  The
    //  operation doesn't need to be locked prior to here because none of
    //  the data would have been shared between running threads until we're
    //  into this routine.
    //
    operation->lock();


    //
    //  Split the destination rect into strips based on the number of
    //  CPUs on the system.  We detect whether the rect is stored as
    //  integers or floats and use different routines to handle the
    //  precision differences.
    //
    //  This routine will split the rectangle and then move the
    //  smaller rectangle into Global Space to ensure that the
    //  object space values will be on integer boundaries (except
    //  for the start and end point).  While this means that we must
    //  do a moveIntoGlobalSpace for *each* smaller rectangle, it
    //  ensures that the destination boxes will not overlap due to
    //  rounding.
    //

    //
    // Move to object space needs the deferrable object
    //
    XilDeferrableObject* defobj;
    if(operation->op->getNumDsts() == 0) {
        defobj = operation->op->getSrc(1);
    } else {
        defobj = operation->op->getDst(1);
    }
        
    if(gsbbox->getType() == XILI_RECT_TYPE_DOUBLE) {
        //
        //  Split using doubles...
        //
        //  Get the bouding box info...
        //
        double x1;
        double y1;
        double x2;
        double y2;
        gsbbox->get(&x1, &y1, &x2, &y2);

        //
        //  Because the operation's box is in global space at this point
        //  and time, y1 and y2 can be negative.  This means that
        //  strip_size needs to be an integer and not a u_int.
        //
        double strip_size = (y2 - y1 + 1)/numCPUs;
        double stop_y     = y2 - strip_size;

        if(strip_size < splitThreshold) {
            operation->unlock();
            return XIL_SUCCESS;
        }

        Xil_boolean empty_segment  = FALSE;
        XiliRectDbl split_rect;
        for(double i = y1; i < stop_y;) {
            //
            //  The internal breaks must fall on integer boundaries
            //
            double c = floor(i+strip_size);

            split_rect.set(x1, i, x2, c);

            i = ceil(i+strip_size);

            //
            //  If it so happens that both floor/ceil are the same
            //  value, then bump up the ceil value. This is to
            //  prevent 2 threads from writing to the same pixels
            //  in the destination
            //
            if(i == c) {
                i += 1.0F;
            }

            //
            //  Move the newly-clippd bounding rect into global space
            //  This could not happen earlier, because the thread split
            //  must ensure an integer boundary and this bbox will get moved
            //  back into object space for execution. It must happen now
            //  because generateBoxList (from execOperation) expects the
            //  gsbbox to be in global space.
            //
            if(operation->op->moveIntoGlobalSpace(&split_rect, defobj) == XIL_FAILURE) {
                recoverFailure(operation);
                operation->unlock();
                return XIL_FAILURE;
            }

            if(execOperation(operation, &split_rect) == XIL_FAILURE) {
                //
                //  If execOperation() fails, something has really gone
                //  wrong.  recoverFailure() takes care of killing any
                //  outstanding threads and waking up the calling thread
                //  with an XIL_FAILURE return status.
                //
                recoverFailure(operation);
                operation->unlock();
                return XIL_FAILURE;
            } else {
                //
                //  The operation we were requested to execute reduces to
                //  nothing.  So, indicate that one of our segments
                //  encountered this condition.
                //
                if(split_rect.isEmpty()) {
                    empty_segment = TRUE;
                } else {
                    *threads_started = TRUE;
                }
            }
        }

        //
        //  Change the size of the gsbbox to be what's leftover.
        //  The remainder will get moved to global space before calling
        //  generateBoxList
        //
        gsbbox->set(x1, i, x2, y2);

        //
        //  If we had some of our image segments reduced to nothing and no
        //  other segment was started successfully, then a thread hasn't
        //  been started that will wake up the calling thread.  Thus, we
        //  need to do it ourselves.
        //
        if(empty_segment    == TRUE &&
           *threads_started == FALSE) {
            //
            //  This tile has no area which requires execution, but
            //  there may be others on the opQueue that need
            //  flushing.  Mark the operation as complete...this means
            //  it's complete unless the remaining porition must be
            //  executed... 
            //
            operation->operationComplete = TRUE;
            operation->addEmptyFlushTile(tnum);
        }
    } else {
        //
        //  Get the bounding box info...
        //
        int x1;
        int y1;
        int x2;
        int y2;
        gsbbox->get(&x1, &y1, &x2, &y2);

        //
        //  Because the operation's box is in global space at this point
        //  and time, y1 and y2 can be negative.  This means that
        //  strip_size needs to be an integer and not a u_int.
        //
        int         strip_size = (y2 - y1 + 1)/numCPUs;
        int         stop_y     = y2 - strip_size;

        if((unsigned int) strip_size < splitThreshold) {
            operation->unlock();
            return XIL_SUCCESS;
        }

        Xil_boolean empty_segment  = FALSE;
        XiliRectInt split_rect;
        for(int i = y1; i < stop_y; i += strip_size) {
            split_rect.set(x1, i, x2, i+strip_size - 1);

            //
            //  Move the newly-clippd bounding rect into global space
            //  This could not happen earlier, because for the double case
            //  we must ensure an integer boundary and this bbox will get moved
            //  back into object space for execution. It must happen now
            //  because generateBoxList (from execOperation) expects the
            //  gsbbox to be in global space.
            //
            if(operation->op->moveIntoGlobalSpace(&split_rect, defobj) == XIL_FAILURE) {
                recoverFailure(operation);
                operation->unlock();
                return XIL_FAILURE;
            }

            if(execOperation(operation, &split_rect) == XIL_FAILURE) {
                //
                //  If execOperation() fails, something has really gone
                //  wrong.  recoverFailure() takes care of killing any
                //  outstanding threads and waking up the calling thread
                //  with an XIL_FAILURE return status.
                //
                recoverFailure(operation);
                operation->unlock();
                return XIL_FAILURE;
            } else {
                //
                //  The operation we were requested to execute reduces to
                //  nothing.  So, indicate that one of our segments
                //  encountered this condition.
                //
                if(split_rect.isEmpty()) {
                    empty_segment = TRUE;
                } else {
                    *threads_started = TRUE;
                }
            }
        }

        //
        //  The remainder will get moved to global space before calling
        //  generateBoxList
        //
        gsbbox->set(x1, i, x2, y2);

        //
        //  If we had some of our image segments reduced to nothing and no
        //  other segment was started successfully, then a thread hasn't
        //  been started that will wake up the calling thread.  Thus, we
        //  need to do it ourselves.
        //
        if(empty_segment    == TRUE &&
           *threads_started == FALSE) {
            //
            //  This tile has no area which requires execution, but
            //  there may be others on the opQueue that need
            //  flushing. 
            //
            operation->operationComplete = TRUE;
            operation->addEmptyFlushTile(tnum);
        }
    }

    operation->unlock();

    return XIL_SUCCESS;
}

XilStatus
XiliScheduler::execOperation(XiliOperation* operation,
                             XiliRect*      bbox)
{
    readyListMutex.lock();

    if(readyList.isEmpty()) {
        //
        //  Create a new context and new thread for this operation because we
        //  don't have any already waiting for something to do.
        //
        readyListMutex.unlock();

        XiliExecContext* new_context = new XiliExecContext(operation);

        if(new_context == NULL) {
            XIL_ERROR(operation->op->getSystemState(),
                      XIL_ERROR_RESOURCE, "di-1", TRUE);
            return XIL_FAILURE;
        }

        if(operation->op->generateBoxList(&new_context->boxList,
                                          bbox) == XIL_FAILURE) {
            return XIL_FAILURE;
        }

        //
        //  If the bbox is emtpy after generateBoxList(), then we don't need
        //  to execute this portion of the operation.  The caller to
        //  execOperation() can check the bbox as well to see if it is
        //  empty and handle the empty case.
        //
        if(bbox->isEmpty()) {
            return XIL_SUCCESS;
        }

        new_context->thread = new XiliThread(xili_start_compute_routine,
                                             new_context);

        if(!new_context->thread->isOK()) {
            XIL_ERROR(operation->op->getSystemState(),
                      XIL_ERROR_RESOURCE, "di-1", TRUE);
            return XIL_FAILURE;
        }

        new_context->listPosition =
            operation->contextList.append(new_context);
    } else {
        //
        //  Get a thread that is waiting for something to do.
        //
        XiliExecContext* new_context;

        if(readyList.remove(readyList.head(), new_context) == XIL_FAILURE) {
            readyListMutex.unlock();
            return XIL_FAILURE;
        }

        new_context->operation    = operation;

        new_context->boxList.reset(NULL,
                                   operation->op->getNumSrcs(),
                                   operation->op->getNumDsts());
        new_context->boxList.setSystemState(operation->op->getSystemState());

        if(operation->op->generateBoxList(&new_context->boxList,
                                          bbox) == XIL_FAILURE) {
            //
            //  Put the conext back on the ready list...
            //
            readyList.append(new_context);

            readyListMutex.unlock();
            return XIL_FAILURE;
        }

        //
        //  If the bbox is emtpy after generateBoxList(), then we don't need
        //  to execute this portion of the operation.  The caller to
        //  execOperation() can check the bbox as well to see if it is
        //  empty and handle the empty case.
        //
        if(bbox->isEmpty()) {
            //
            //  Put the conext back on the ready list...
            //
            readyList.append(new_context);

            readyListMutex.unlock();
            return XIL_SUCCESS;
        }

        new_context->listPosition =
            operation->contextList.append(new_context);

        new_context->isReady.signal();

        readyListMutex.unlock();
    }

    return XIL_SUCCESS;
}

void
XiliScheduler::recoverFailure(XiliOperation* operation)
{
    //
    //  Here we run through the list and kill all of the threads that were
    //  created for this operation.
    //
    XiliListIterator<XiliExecContext> li(&operation->contextList);

    XiliExecContext* context;
    while(context = li.getNext()) {
        XiliThread* thread = context->thread;
        
        thread->kill(SIGTERM);
        thread->join();

        delete thread;
        delete context;
    }
}

XilStatus
XiliScheduler::callPreprocess(XiliOperation*   operation,
                              XiliFunctionDef* func_def)
{
    //
    //  Call the preprocess operation.
    //
    TNF_PROBE_1(call_preprocess, "pre_post_process", "call_preprocess",
                tnf_opaque, "op", operation->op);

    void*        func_info = NULL;
    unsigned int func_id   = 0;

    XilStatus    func_status;

    //
    //  Call the function given to use by the pipeline.
    //
    XilDeviceIO*          func_io_device    = NULL;
    XilDeviceCompression* func_codec_device = NULL;
    
	// Porting Note.  The HP compiler has trouble with complicated
	// statements involving pointers to members.  Expressions have
	// been simplified
    switch(func_def->functionType) {
      case XILI_COMPUTE_FUNC:
	  {
		XilComputePreprocessFunctionPtr	preFunc = 
			func_def->funcInfo.getComputePreFunction();
        func_status =
            (((XilDeviceManagerCompute*)func_def->deviceManager)->*(preFunc))
            (operation->op, operation->functionLength, operation->roi,
             &func_info, &func_id);
	   }
       break;

      case XILI_IO_FUNC:
        //
        //  TODO:  1/11/96 jlf  This is insufficient and slow.  Fix it.
        //
        XilOp* src_op;
              
        if(operation->functionLength > 1) {
            src_op = operation->op->getOpList()[operation->functionLength - 1];
        } else {
            src_op = operation->op;
        }
              
        if(src_op->isIOOperation(XILI_OP_IO_TYPE_CAPTURE)) {
			XilIOPreprocessFunctionPtr preFunc = 
				func_def->funcInfo.getIOPreFunction();
			XilDeviceIO* oper = operation->op->getSrc(1)->getDeviceIO();
            func_status =
                ((oper)->*(preFunc))(
                    operation->op,
                    operation->functionLength,
                    operation->roi,
                    &func_info,
                    &func_id);

            func_io_device = operation->op->getSrc(1)->getDeviceIO();
        } else {
			XilIOPreprocessFunctionPtr preFunc = 
				func_def->funcInfo.getIOPreFunction();
			XilDeviceIO* oper = operation->op->getDst(1)->getDeviceIO();
            func_status =
                ((oper)->*(preFunc))(
                    operation->op,
                    operation->functionLength,
                    operation->roi,
                    &func_info,
                    &func_id);
            
            func_io_device = operation->op->getDst(1)->getDeviceIO();
        }
        break;

      case XILI_CODEC_FUNC:
        if(operation->op->getSrc(1)->getType() == XIL_CIS) {
            //
            //  Src as CIS -- decompression
            //
			XilCodecPreprocessFunctionPtr codecFunc = 
				func_def->funcInfo.getCodecPreFunction();
			XilDeviceCompression* comp = ((XilCis*)operation->op->getSrc(1))->getDeviceCompression();
            func_status =
                ((comp)->*(codecFunc))(
                    operation->op,
                    operation->functionLength,
                    operation->roi,
                    &func_info,
                    &func_id);

            func_codec_device =
                ((XilCis*)operation->op->getSrc(1))->getDeviceCompression();
        } else {
            //
            //  Otherwise -- compression
            //
			XilCodecPreprocessFunctionPtr codecFunc = 
				func_def->funcInfo.getCodecPreFunction();
			XilDeviceCompression* comp = ((XilCis*)operation->op->getDst(1))->getDeviceCompression();
            func_status =
                ((comp)->*(codecFunc))(
                    operation->op,
                    operation->functionLength,
                    operation->roi,
                    &func_info,
                    &func_id);

            func_codec_device =
                ((XilCis*)operation->op->getDst(1))->getDeviceCompression();
        }
        break;
    }

    //
    //  Add the information obtained from this preprocess routine to
    //  the op so its associated compute routine can obtain the data
    //  when it executes.  The func_status indicates to future threads 
    //  whether this preprocess routine returned XIL_FAILURE.
    //
    if(operation->op->addFuncData(func_def,
                                  func_id,
                                  func_info,
                                  func_io_device,
                                  func_codec_device,
                                  func_status) == NULL) {
        XIL_ERROR(operation->op->getSystemState(),
                  XIL_ERROR_INTERNAL, "di-415", FALSE);
        return XIL_FAILURE;
    }

    return func_status;
}

void
XiliScheduler::startComputeOperation(XiliExecContext* context)
{
    XiliOperation*   operation = context->operation;
    XiliOpTreeNode*  node      = operation->opTreeNode;

    //
    //  Handle XIL 1.2 show_action equivalent
    //
    Xil_boolean show_action =
        operation->op->getSystemState()->getShowActionFlag();

    context->status = XIL_FAILURE;
    
    XiliFunctionDef* func_def = node->getFunctionList();
    while(func_def) {
        //
        //  Check for any preprocess functions needed by the function.
        //
        Xil_boolean call_preprocess = FALSE;
        switch(func_def->functionType) {
          case XILI_COMPUTE_FUNC:
            if(func_def->funcInfo.getComputePreFunction() != NULL) {
                call_preprocess = TRUE;
            }
            break;

          case XILI_IO_FUNC:
            if(func_def->funcInfo.getIOPreFunction() != NULL) {
                call_preprocess = TRUE;
            }
            break;

          case XILI_CODEC_FUNC:
            if(func_def->funcInfo.getCodecPreFunction() != NULL) {
                call_preprocess = TRUE;
            }
            break;
        }

        if(call_preprocess) {
            //
            //  Only one thread should be calling the preprocess function for
            //  a given operation which requires obtaining a mutex here prior
            //  to checking and potentially calling the preprocess function.
            //
            operation->lock();

            XilStatus preprocess_status;
            if(operation->op->checkFuncData(func_def,
                                            &preprocess_status) == XIL_FAILURE) {
                //
                //  We didn't find a match so it hasn't been called yet.
                //  We call the preprocess function and only skip on to the
                //  next one if it returns XIL_FAILURE;
                //
                if(callPreprocess(operation, func_def) == XIL_FAILURE) {
                    func_def = func_def->next;
                    operation->unlock();
                    continue;
                }
            } else if(preprocess_status == XIL_FAILURE) {
                func_def = func_def->next;
                operation->unlock();
                continue;
            }

            operation->unlock();
        }

        //
        //  Do we print out what we're doing?
        //
        if(show_action) {
            if(func_def->funcInfo.getFunctionName() == NULL) {
                fprintf(stderr, "--t%03d-- '%s' : '%s()' Started\n",
                        XiliThread::self(),
                        func_def->deviceManager->getDeviceName(),
                        XilGlobalState::getXilGlobalState()->lookupOpName(operation->op->getOpNumber()));
            } else {
                fprintf(stderr, "--t%03d-- '%s' : '%s' Started\n",
                        XiliThread::self(),
                        func_def->deviceManager->getDeviceName(),
                        func_def->funcInfo.getFunctionName());
            }
        }

        if(func_def->funcInfo.getFunctionName() == NULL) {
            TNF_PROBE_2(start_operation, "xilop pre_post_process", "start_operation",
                        tnf_opaque, "op", operation->op,
                        tnf_string, "name", XilGlobalState::getXilGlobalState()->lookupOpName(operation->op->getOpNumber()));
        } else {
            TNF_PROBE_2(start_operation, "xilop pre_post_process", "start_operation",
                        tnf_opaque, "op", operation->op,
                        tnf_string, "name", func_def->funcInfo.getFunctionName());
        }
            
        //
        //  Call the function given to use by the pipeline.
        //
		// Porting Note.  The HP compiler has trouble with complicated
		// statements involving pointers to members.  Expressions have
		// been simplified
        switch(func_def->functionType) {
          case XILI_COMPUTE_FUNC:
		  {
			XilComputeFunctionPtr comp = 
				func_def->funcInfo.getComputeFunction();
            context->status =
                (((XilDeviceManagerCompute*)func_def->deviceManager)->*(comp))(
                    operation->op,
                    operation->functionLength,
                    operation->roi,
                    &context->boxList);
		  }
		  break;

          case XILI_IO_FUNC:
          {
              //
              //  TODO:  1/11/96 jlf  This is insufficient and slow.  Fix it.
              //
              XilOp* src_op;
              
              if(operation->functionLength > 1) {
                  src_op = operation->op->getOpList()[operation->functionLength - 1];
              } else {
                  src_op = operation->op;
              }
              
              if(src_op->isIOOperation(XILI_OP_IO_TYPE_CAPTURE)) {
				  XilIOFunctionPtr ioFunc = 
					func_def->funcInfo.getIOFunction();
				  XilDeviceIO* devIO = src_op->getSrc(1)->getDeviceIO();
                  context->status =
                      ((devIO)->*(ioFunc))(
                          operation->op,
                          operation->functionLength,
                          operation->roi,
                          &context->boxList);
              } else {
                  XilIOFunctionPtr ioFunc =
                      func_def->funcInfo.getIOFunction();
				  XilDeviceIO* devIO = operation->op->getDst(1)->getDeviceIO();
                  context->status =
                      ((devIO)->*(ioFunc))(
                          operation->op,
                          operation->functionLength,
                          operation->roi,
                          &context->boxList);
              }
          }
          break;

          case XILI_CODEC_FUNC:
          {
              //
              //  TODO:  1/11/96 jlf  This is insufficient and slow.  Fix it.
              //
              XilOp* src_op;
              
              if(operation->functionLength > 1) {
                  src_op = operation->op->getOpList()[operation->functionLength - 1];
              } else {
                  src_op = operation->op;
              }
              
              if(src_op->getSrc(1)->getType() == XIL_CIS) {
                  //
                  //  Src as CIS -- decompression
                  //
				  XilCodecFunctionPtr codec = 
					  func_def->funcInfo.getCodecFunction();
				  XilDeviceCompression* devCom = 
				      src_op->getSrcCis(1)->getDeviceCompression();
                  context->status =
                      ((devCom)->*(codec))(
                          operation->op,
                          operation->functionLength,
                          operation->roi,
                          &context->boxList);
              } else {
                  //
                  //  Otherwise -- compression
                  //
				  XilCodecFunctionPtr codec =
					  func_def->funcInfo.getCodecFunction();
				  XilDeviceCompression* devCom =
					  operation->op->getDstCis(1)->getDeviceCompression();
                  context->status =
                      ((devCom)->*(codec))(
                          operation->op,
                          operation->functionLength,
                          operation->roi,
                          &context->boxList);
              }
              break;
          }
        }

        if(func_def->funcInfo.getFunctionName() == NULL) {
            TNF_PROBE_2(finish_operation, "xilop pre_post_process", "finish_operation",
                        tnf_opaque, "op", operation->op,
                        tnf_string, "name", XilGlobalState::getXilGlobalState()->lookupOpName(operation->op->getOpNumber()));
        } else {
            TNF_PROBE_2(finish_operation, "xilop pre_post_process", "finish_operation",
                        tnf_opaque, "op", operation->op,
                        tnf_string, "name", func_def->funcInfo.getFunctionName());
        }

        if(show_action) {
            const char* status_message;

            if(context->status == XIL_SUCCESS) {
                if(context->boxList.getNumFailed() != 0) {
                    status_message = "Returned FAILED Boxes";
                } else {
                    status_message = "Finished";
                }                    
            } else {
                status_message = "Returned XIL_FAILURE";
            }

            if(func_def->funcInfo.getFunctionName() == NULL) {
                fprintf(stderr, "--t%03d-- '%s' : '%s()' %s\n",
                        XiliThread::self(),
                        func_def->deviceManager->getDeviceName(),
                        XilGlobalState::getXilGlobalState()->lookupOpName(operation->op->getOpNumber()),
                        status_message);
            } else {
                fprintf(stderr, "--t%03d-- '%s' : '%s' %s\n",
                        XiliThread::self(),
                        func_def->deviceManager->getDeviceName(),
                        func_def->funcInfo.getFunctionName(),
                        status_message);
            }
        }
        
        if(context->status                 == XIL_FAILURE ||
           context->boxList.getNumFailed() != 0) {
            //
            //  Here we need to reset the box list to switch to the "FAILED"
            //  list of boxes for the next compute operation.
            //
            if(context->boxList.resetFromFailed() == XIL_FAILURE) {
                break;
            }

            //
            //  Consider this operation as "FAILED" at this point since some
            //  of the boxes were not completed.
            //
            context->status = XIL_FAILURE;
            
            func_def = func_def->next;

            continue;
        } else {
            break;
        }
    }

    //
    //  Set the operation status and remove this exec context from the list.
    //
    operation->lock();

    if(context->status == XIL_FAILURE) {
        operation->status = XIL_FAILURE;
    }

    if(context->listPosition != _XILI_SLLIST_INVALID_POSITION) {
        operation->contextList.remove(context->listPosition);
    }

    //
    //  Indicate that the compute operation has been completed.
    //
    if(operation->contextList.isEmpty()) {
        operation->operationComplete = TRUE;
        operation->completeCond.signal();
    }

    operation->unlock();
}
