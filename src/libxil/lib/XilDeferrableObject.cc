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
//  File:	XilDeferrableObject.cc
//  Project:	XIL
//  Revision:	1.117
//  Last Mod:	10:08:13, 03/10/00
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
#pragma ident	"@(#)XilDeferrableObject.cc	1.117\t00/03/10  "

#ifdef _WINDOWS
#include <time.h>
#else
#include <sys/time.h>
#endif

#include "_XilDefines.h"
#include "_XilGlobalState.hh"
#include "_XilDeferrableObject.hh"
#include "_XilOp.hh"
#include "_XilTileList.hh"

#include "XiliDag.hh"
#include "XiliDagManager.hh"
#include "XiliOpQueueEntry.hh"

//
//  This is the implementation of xil_sync().  It only calls sync on the tail
//  of the queue because that is the only representation of this image at the
//  API.
//
XilStatus
XilDeferrableObject::sync(XiliOpQueuePosition qposition)
{
    XilTileList  tile_list(getSystemState());
    if(getTileList(&tile_list) == XIL_FAILURE) {
        XIL_OBJ_ERROR(getSystemState(),
                      XIL_ERROR_INTERNAL, "di-422", FALSE, this);
        return XIL_FAILURE;
    }

    if(syncForReading(&tile_list, NULL, qposition) == XIL_FAILURE) {
        XIL_OBJ_ERROR(getSystemState(),
                      XIL_ERROR_SYSTEM, "di-384", FALSE, this);
        return XIL_FAILURE;
    }

    return XIL_SUCCESS;
}

XilStatus
XilDeferrableObject::toss()
{
    if(qLength()) {
        //
        //  Mark the entry on the tail of the op queue as "tossed".  Then,
        //  call tossIfNoDependents().
        //
        XiliOpQueuePosition qpos    = qTail();
        XiliOpQueueEntry*   opentry = qRef(qpos);

        opentry->setTossed();

        tossIfNoDependents();
    }

    return XIL_SUCCESS;
}

XilStatus
XilDeferrableObject::syncForReading(XilTileList*        tile_list,
                                    XilOp*              op_flushing,
                                    XiliOpQueuePosition qposition)
{
    if(qposition == _XILI_OP_QUEUE_INVALID_POSITION) {
        if((qposition = qTail()) == _XILI_OP_QUEUE_INVALID_POSITION) {
            return XIL_SUCCESS;
        }
    }

    XiliOpQueueEntry* opentry = qRef(qposition);

    TNF_PROBE_3(xilop_sync_area, "xilop", "xilop_sync_area",
                tnf_opaque, "this", opentry->getOp(),
                tnf_opaque, "opentry", opentry,
                tnf_opaque, "qpos", qposition);

    XilOp* op = opentry->getOp();
    if(op != NULL && op != op_flushing) {
        //
        //  Flush the op...
        //
        if(op->flush(tile_list) == XIL_FAILURE) {
            return XIL_FAILURE;
        }
    }
    
    return XIL_SUCCESS;
}

void
XilDeferrableObject::setSynchronized(Xil_boolean onoff)
{
    _XIL_TEST_FOR_NULL_THIS_VOID("di-268");

    isSynchronized = onoff;
}


Xil_boolean
XilDeferrableObject::getSynchronized(void)
{
    _XIL_TEST_FOR_NULL_THIS(FALSE, "di-268");

    return isSynchronized;
}

//
//  Used by events like setting an origin or ROI to force synchronization with
//  a state change. 
//
XilStatus
XilDeferrableObject::allSync()
{
    //
    //  It's actually better to call flushDependents() prior to sync() because
    //  you'll get better tile/operation scheduling.  If there are dependents,
    //  they'll cause the operation writing into here to be flushed and
    //  nothing will occur on the sync() call.  If there are no dependents,
    //  the sync() will pick things up.
    //
    flushDependents(_XILI_OP_QUEUE_INVALID_POSITION);
    sync();

    return XIL_SUCCESS;
}

//
//  A special version of preDestroy() which takes care of leaving the object
//  around until it is really no longer needed.
//
Xil_boolean
XilDeferrableObject::preDestroy()
{
    //
    //  If we don't delete it, then we want to make sure that if the user
    //  attempts to use it in an operation, the operation will fail.
    //
    //  TODO:  11/4/96 jlf  Need stronger flag...valid only affects sources.
    //
    setValid(FALSE);

    if(qLength() == 0) {
        //
        //  If there are no entries on the op queue, then we can just delete
        //  this object because nobody depends on it or write into it.
        //
        return TRUE;
    } else {
        XiliOpQueueEntry* opentry = qRef(qTail());

        //
        //  Now, if nobody depends on the last op queue entry then we can
        //  destroy the op that writes into it -- and we may be able to really
        //  delete ourselves.
        //
        if(opentry->getNumDependents() == 0) {
            //
            //  Delete the op that writes into us.  This will have a chain
            //  reaction of destroying things up-the-chain.  Destroying the op
            //  that writes into has the effect of removing the opentry from
            //  the opQueue by creatorCleanup().
            //
            opentry->getOp()->destroy();

            //
            //  If we have no op queue entries, then we know we can be deleted.
            //  There would only be multiple op queue entries if there were
            //  dependencies.
            //
            if(qLength() == 0) {
                return TRUE;
            }
        } else {
            //
            //  Set the flag that indicates to the deferrable object to delete
            //  this object when it's dependency list is reduced to zero.
            //
            deleteWhenNoDependents = TRUE;
        }
    }

    //
    //  Don't destroy the object yet, it's got some kind of dependency...we'll
    //  destroy it later.  
    //
    return FALSE;
}


//---------------------------------------------------------------------------
//
//  Tile size support.  The operation scheduler and others will use this to
//    information to split operations into smaller pieces.
//  
//
//  Does this object support being tiled?
//
Xil_boolean
XilDeferrableObject::canBeTiled()
{
    return FALSE;
}

//
//  Is the object's tile size set?
//
Xil_boolean
XilDeferrableObject::isTileSizeSet()
{
    return FALSE;
}

//
//  Get the object's current tile size.
//
XilStatus
XilDeferrableObject::getTileSize(unsigned int*,
                                 unsigned int*)
{
    return XIL_FAILURE;
}

//
//  Set/Reset the object's tile size.
//
XilStatus
XilDeferrableObject::setTileSize(unsigned int tile_xsize,
                                 unsigned int tile_ysize)
{
    return initTileSize(tile_xsize, tile_ysize);
}

//
//  Initialize the object's tile size.
//
XilStatus
XilDeferrableObject::initTileSize(unsigned int,
                                  unsigned int,
                                  XilTile*    )
{
    return XIL_FAILURE;
}

XilStatus
XilDeferrableObject::initTileSize(XilDeferrableObject*)
{
    return XIL_FAILURE;
}

unsigned int
XilDeferrableObject::getNumXTiles()
{
    return numXTiles;
}

unsigned int
XilDeferrableObject::getNumYTiles()
{
    return numYTiles;
}

//
//  By default, this is a list of all the tiles.
//
XilStatus
XilDeferrableObject::getTileList(XilTileList* tile_list,
                                 XilBox*      )
{
    unsigned int num_tiles = getNumTiles();

    //
    //  Loose the loop if numTiles == 1
    //
    if(num_tiles == 1) {
        tile_list->setNumTiles(1);
        tile_list->setEntry(0, 0);
    } else {
        tile_list->setNumTiles(num_tiles);

        for(unsigned int i=0; i<num_tiles; i++) {
            tile_list->setEntry(i, i);
        }
    }

    return XIL_SUCCESS;
}

//
//  By default, this is a list of all the tiles.
//
XilStatus
XilDeferrableObject::getTileList(XilTileList* tile_list,
                                 XiliRect*    )
{
    return getTileList(tile_list, (XilBox*)NULL);
}

XilStatus
XilDeferrableObject::getTileRect(XiliRect*     ,
                                 XilTileNumber )
{
    return XIL_FAILURE;
}

XiliOpQueueEntry*
XilDeferrableObject::getQueueEntry(XiliOpQueuePosition qpos)
{
    return qRef(qpos);
}

int
XilDeferrableObject::getExported()
{
    return exportMode;
}

//
//  By default, this routine does nothing to the box.
//
XilStatus
XilDeferrableObject::clipToTile(XilTileNumber ,
                                XiliRect*     )
{
    return XIL_SUCCESS;
}

void
XilDeferrableObject::releaseTile(XilTileNumber)
{
}

XiliOpStatus
XilDeferrableObject::getOpStatus(XilTileNumber       tile_number,
                                 XiliOpQueuePosition qposition)
{
    return qRef(qposition)->getOpStatus(tile_number);
}

void
XilDeferrableObject::setOpStatus(XilTileNumber       tile_number,
                                 XiliOpQueuePosition qposition,
                                 XiliOpStatus        op_status)
{
    qRef(qposition)->setOpStatus(tile_number, op_status);
}

Xil_boolean
XilDeferrableObject::allTilesDone(XiliOpQueuePosition qposition)
{
    return qRef(qposition)->allTilesDone();
}

//
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
//
//  Deferred Execution.
//
XiliOpQueuePosition
XilDeferrableObject::setOp(XilOp* new_op)
{
    XiliOpQueuePosition pos = qNewEntry(getNumTiles());
    if(pos == _XILI_OP_QUEUE_INVALID_POSITION) {
        return _XILI_OP_QUEUE_INVALID_POSITION;
    }
    
    XiliOpQueueEntry* opentry = qRef(pos);

    opentry->setOp(new_op);
    
    return pos;
}

//
//  Get the op which writes into the current instance of this object.
//
XilOp*
XilDeferrableObject::getOp()
{
    XilOp*            ret_op  = NULL;
    
    if(qLength() != 0) {
        XiliOpQueueEntry* opentry = qRef(qTail());

        ret_op = opentry->getOp();
    }

    return ret_op;
}

//
//  Clean all references to the given op from the given queue position.
//
void
XilDeferrableObject::creatorCleanup(XiliOpQueuePosition qposition,
                                    XilOp*              op)
{
    if(qposition != _XILI_OP_QUEUE_INVALID_POSITION) {
        XiliOpQueueEntry* opentry = qRef(qposition);

        opentry->creatorCleanup(op);

        //
        //  Now, if nobody is dependent on this entry anymore, remove it from
        //  the queue.
        //
        if(opentry->getNumDependents() == 0 &&
           opentry->getOp() == NULL) {
            qRemove(qposition);

            //
            //  If we have no entries on our queue, then we no longer belong
            //  to a DAG and should release ourselves from the DAG.
            //
            if(qLength() == 0) {
                //
                //  Tell the op cleaning up that we're going away.
                //
                op->cleanupReferences(this);

                //
                //  Clear out our dag reference.
                //
                XiliDagManager* dm = XilGlobalState::theXGS->getDagManager();
                dm->lock();
                setDagRef(NULL);
                dm->unlock();

                //
                //  If we're marked to be deleted when no dependencies
                //  remain on us and there is nothing left in the opQueue,
                //  then it's time for this object to be deleted.
                //
                if(deleteWhenNoDependents) {
                    delete this;
                    return;
                }
            }
        }
    }
}

//
//  Clean all references to the given op from the given queue position.
//
void
XilDeferrableObject::cleanup(XiliOpQueuePosition qposition,
                             XilOp*              op)
{
    if(qposition != _XILI_OP_QUEUE_INVALID_POSITION) {
        XiliOpQueueEntry* opentry = qRef(qposition);

        opentry->cleanupDependents(op);

        //
        //  If nobody depends on us and there is an op that writes into us and
        //  there is another op writing into us that replaces the results of
        //  the other op writing into us or the op writing into us has already
        //  been evaluated, then destroy it because it's no longer needed.
        //
        unsigned int num_dependents = opentry->getNumDependents();
        XilOp*       creator_op     = opentry->getOp();

        if(num_dependents == 0 && creator_op != NULL &&
           (qposition != qTail() ||
            opentry->getTossed() ||
            creator_op->getStatus() == XILI_EVALUATED)) {
            //
            //  If we have no dependents, then we probably just got rid of our
            //  last dependent (op) so in destroying this, creatorCleanup()
            //  will not find any dependents to notify about the creator_op's
            //  destruction.  Thus, we must notify our dependent op (op) that
            //  the creator_op is going away and that references must go
            //  away.
            //
            op->cleanupReferences(creator_op);

            creator_op->destroy();
        }

        //
        //  Now, if we have no dependents and no op writing into us, then the
        //  entry can be removed from the list.
        //
        if(num_dependents == 0 && creator_op == NULL) {
            //
            //  Remove our entry from the queue.
            //
            qRemove(qposition);

            //
            //  If we have no entries on our queue, then we no longer belong
            //  to a DAG and should release ourselves from the DAG.
            //
            if(qLength() == 0) {
                //
                //  Tell the op cleaning up that we're going away.
                //
                op->cleanupReferences(this);

                //
                //  Clear out our dag reference.
                //
                XiliDagManager* dm = XilGlobalState::theXGS->getDagManager();
                dm->lock();
                setDagRef(NULL);
                dm->unlock();

                //
                //  If we're marked to be deleted when no dependencies
                //  remain on us and there is nothing left in the opQueue,
                //  then it's time for this object to be deleted.
                //
                if(deleteWhenNoDependents) {
                    delete this;
                    return;
                }
            }
        }
    }
}

XiliOpQueuePosition
XilDeferrableObject::addDependent(XilOp*       dependent_op,
                                  unsigned int src_branch)
{
    //
    //  If the op queue is empty, then create a new entry, add the dependent
    //  and append the entry to the queue.
    //
    XiliOpQueuePosition pos;

    if(qLength()) {
        //
        //  Add the dependent to the current instance of the object.  The
        //  information about the current instance of the object is considered
        //  the end of the queue.
        //
        pos = qTail();
    } else {
        //
        //  Otherwise, get a new entry on the queue.
        //
        pos = qNewEntry(getNumTiles());
        if(pos == _XILI_OP_QUEUE_INVALID_POSITION) {
            return _XILI_OP_QUEUE_INVALID_POSITION;
        }
    }

    qRef(pos)->addDependent(dependent_op, src_branch);

    return pos;
}

//
//  Flush all of the operations which are dependent upon the given instance of
//  this image.
//
XilStatus
XilDeferrableObject::flushDependents(XiliOpQueuePosition qposition)
{
    if(qposition == _XILI_OP_QUEUE_INVALID_POSITION) {
        //
        //  Since the entries on the queue may disappear while we're in this
        //  routine, we make a copy of the opQueue and iterate over it.
        //
        if(qLength() != 0) {
            unsigned int       q_len  = qLength();
            XiliOpQueueEntry** oelist = new XiliOpQueueEntry*[q_len];
            if(oelist == NULL) {
                XIL_ERROR(getSystemState(), XIL_ERROR_RESOURCE, "di-1", TRUE);
                return XIL_FAILURE;
            }

            XiliOpQueuePosition pos;
            unsigned int        i;
            for(i = 0, pos = qHead(); pos != _XILI_OP_QUEUE_INVALID_POSITION; pos = qNext(pos), i++) {
                oelist[i] = qRef(pos);
            }

            for(i = 0; i<q_len; i++) {
                oelist[i]->flushDependents();
            }

            delete [] oelist;
        }
    } else {
        qRef(qposition)->flushDependents();
    }

    return XIL_SUCCESS;
}

XilStatus
XilDeferrableObject::flushDependents(XilTileList*        tile_list,
                                     XilOp*              op_flushing,
                                     XiliOpQueuePosition qposition)
{
    return qRef(qposition)->evaluate(tile_list, op_flushing);
}

//
//  Toss (delete) the operation which writes into the current instance of this
//  image if no one depends on this object.
//
Xil_boolean
XilDeferrableObject::tossIfNoDependents()
{
    Xil_boolean did_toss = FALSE;

    if(qLength() != 0) {
        XiliOpQueuePosition qtail   = qTail();
        XiliOpQueueEntry*   opentry = qRef(qtail);
        XilOp*              op      = opentry->getOp();
        
        if(opentry->getNumDependents() == 0) {
            //
            //  Destroying the op should cause the entry to be removed from
            //  our queue by creatorCleanup().
            //
            if(op) {
                op->destroy();
            } else {
                qRemove(qtail);
            }
            did_toss = TRUE;
        }
    }

    return did_toss;
}

XilStatus
XilDeferrableObject::syncForWriting(XilTileList*        tile_list,
                                    XilOp*              op_flushing,
                                    XiliOpQueuePosition qposition)
{
    if(qposition == _XILI_OP_QUEUE_INVALID_POSITION) {
        if((qposition = qTail()) == _XILI_OP_QUEUE_INVALID_POSITION) {
            return XIL_SUCCESS;
        }
    }

    //
    //  TODO: 7/17/96 jlf  Should this just remain NULL?
    //
    if(op_flushing == NULL) {
        op_flushing = qRef(qposition)->getOp();
    }

    TNF_PROBE_2(xilop_sync_writing, "xilop", "xilop_sync_writing",
                tnf_opaque, "def_obj", this,
                tnf_opaque, "qpos", qposition);

    XilStatus           ret_val = XIL_SUCCESS;
    XiliOpQueuePosition pos     = qHead();
    XiliOpQueueEntry*   opentry = NULL;
    while(pos != qposition && pos != _XILI_OP_QUEUE_INVALID_POSITION) {
        //
        //  Get the op entry and evaluate it.
        //
        opentry = qRef(pos);

        if(opentry->evaluate(tile_list, op_flushing) == XIL_FAILURE) {
            return XIL_FAILURE;
        }

        //
        //  Don't flush the op which is responsible for flushing us.  Note
        //  that in the case of molecules, we don't want to flush the
        //  the dependent op if op_flushing is the bottom operation of the
        //  molecule the dependent's a part of...
        //
        XilOp* op = opentry->getOp();
        if(op != NULL) {
            XilOp* molecule_bottom = op->getMoleculeBottom();

            if(op != op_flushing &&
               (molecule_bottom == NULL || molecule_bottom != op_flushing)) {
                TNF_PROBE_2(xilop_eval_to_pos_flush, "xilop", "xilop_eval_to_pos_flush",
                            tnf_opaque, "this", opentry->getOp(),
                            tnf_opaque, "op_flushing", op_flushing);

                if(op->flush(tile_list) == XIL_FAILURE) {
                    //
                    //  Eventhough one flush() failed, we must continue with
                    //  the other ops so they'll fail/succeed as needed and
                    //  will be cleaned up appropriately.
                    //
                    ret_val = XIL_FAILURE;
                }
            }
        }

        pos = qNext(pos);
    }

    return ret_val;
}


XilStatus
XilDeferrableObject::transferOpQueueInfo(XilDeferrableObject* def_obj)
{
    //
    //  Copy the op queue from the given object into our list.
    //
    for(unsigned int i=0; i<_XILI_MAX_OPQUEUE_LENGTH; i++) {
        queueData[i]   = def_obj->queueData[i];
    }

    queueHead   = def_obj->queueHead;
    queueTail   = def_obj->queueTail;
    queueLength = def_obj->queueLength;

    XiliOpQueuePosition pos;
    for(pos = qHead(); pos != _XILI_OP_QUEUE_INVALID_POSITION; pos = qNext(pos)) {
        qRef(pos)->setDeferrableObject(this);
    }

    //
    //  TODO: 4/26/96 jlf  There is a bug here.
    //
    //    If dagRef is not set, then the local mutex on the def_obj remains
    //    locked.  It should not be possible for this routine to be called in
    //    this state because it's used when mutating one operation into
    //    another with different deferrable objects.
    //
    dagRef  = def_obj->dagRef;
    
    return XIL_SUCCESS;
}

//----------------------------------------------------------------------------
//
//  Virtual functions on the deferrable object that allow images to deal in
//  both global and object space.
//
//----------------------------------------------------------------------------
XilRoi*
XilDeferrableObject::getRoi()
{
    return NULL; 
}

XilRoi*
XilDeferrableObject::refRoi()
{
    return NULL; 
}

//
//  By default, these routines do nothing.  The assumption is that the ROI is
//  already in "object space" so they indicate success.
//
XilStatus
XilDeferrableObject::convertToObjectSpace(XiliRect*)
{
    return XIL_SUCCESS;
}

XilStatus
XilDeferrableObject::convertToObjectSpace(XilRoi*)
{
    return XIL_SUCCESS;
}

XilStatus
XilDeferrableObject::convertToObjectSpace(XiliConvexRegion*)
{
    return XIL_SUCCESS;
}

XilStatus
XilDeferrableObject::convertToGlobalSpace(XiliRect*)
{
    return XIL_SUCCESS;
}

XilStatus
XilDeferrableObject::convertToGlobalSpace(XilRoi*)
{
    return XIL_SUCCESS;
}

XilStatus
XilDeferrableObject::convertToGlobalSpace(XiliConvexRegion*)
{
    return XIL_SUCCESS;
}

XilStatus
XilDeferrableObject::setBoxStorage(XilBox*)
{
    return XIL_SUCCESS;
}

XilDeviceIO*
XilDeferrableObject::getXilDeviceIO()
{
    return getDeviceIO();
}

XilStatus
XilDeferrableObject::setDeviceIO(XilDeviceIO*)
{
    return XIL_FAILURE;
}

XilDeviceIO*
XilDeferrableObject::getDeviceIO()
{
    return NULL;
}

XilOp*
XilDeferrableObject::createCaptureOp(XilOp*,
                                     unsigned int)
{
    return NULL;
}

XilOp*
XilDeferrableObject::createDisplayOp(XilOp*,
                                     unsigned int)
{
    return NULL;
}

Xil_boolean
XilDeferrableObject::isStorageValid()
{
    return storageValidFlag;
}

void
XilDeferrableObject::setStorageValidFlag(Xil_boolean valid_flag)
{
    storageValidFlag = valid_flag;
}

void
XilDeferrableObject::updateObjectReferences(XilNonDeferrableObject* ,
                                            XilNonDeferrableObject* ,
                                            Xil_boolean             )
{
}


XilStatus
XilDeferrableObject::_qFlushQueueHead()
{
    //
    //  The queue is as large as we permit it to be so we need to
    //  flush its dependents and sync the head position.
    //
    if(flushDependents(queueHead) == XIL_FAILURE) {
        return XIL_FAILURE;
    }

    if(sync(queueHead) == XIL_FAILURE) {
        return XIL_FAILURE;
    }

    //
    //  This shouldn't happen, but if we still have too many entries
    //  on the queue, then we're in trouble.
    //
    if(queueLength == _XILI_MAX_OPQUEUE_LENGTH) {
        return XIL_FAILURE;
    }

    return XIL_SUCCESS;
}
