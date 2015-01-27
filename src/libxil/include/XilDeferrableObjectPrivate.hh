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
//  File:	XilDeferrableObjectPrivate.hh
//  Project:	XIL
//  Revision:	1.73
//  Last Mod:	10:21:10, 03/10/00
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
#pragma ident	"@(#)XilDeferrableObjectPrivate.hh	1.73\t00/03/10  "

#ifdef    _XIL_PRIVATE_INCLUDES

#include "XiliOpQueueEntry.hh"

const unsigned int _XILI_MAX_OPQUEUE_LENGTH = 16;

//
//  These are used to keep exportMode clear.  They match the API definition of
//  what xil_get_exported() returns.
//
const int XIL_NOT_EXPORTABLE = -1;
const int XIL_IMPORTED       = 0;
const int XIL_EXPORTED       = 1;

#endif

#ifdef    _XIL_PRIVATE_DATA
public:
    //------------------------------------------------------------------
    //
    //  API-Based Deferred Execution.  When objects are sync'd and
    //  tossed via the API or GPI, the operation affects the entire
    //  object.
    //
    //------------------------------------------------------------------

    //
    //  The *entire* contents of this object are evaluated immediately.
    //    The call does not complete until the sync is complete.
    //
    virtual XilStatus  sync(XiliOpQueuePosition qposition =
                            _XILI_OP_QUEUE_INVALID_POSITION);

    //
    //  The *entire* contents of this object are no longer necessary.
    //
    virtual XilStatus  toss();

    //----------------------------------------------------------------------------
    //
    //  GPI/Internal Deferred Execution.  Internally XIL may ask for
    //    only portions of an object to be operated on or
    //    brought-up-to-date.  Generally, a single tile of an image.
    //    Objects that do not have any sub-area management can use the
    //    default behavior for these which is to operate on the entire
    //    contents of the object.
    //
    virtual XilStatus           syncForReading(XilTileList*        tile_list,
                                               XilOp*              op_flushing = NULL,
                                               XiliOpQueuePosition qposition =
                                               _XILI_OP_QUEUE_INVALID_POSITION);
    //
    //  syncForWriting() is equivalent to evaluateToPosition().  It's the
    //  evaluating of operations in order to write into the given area of
    //  image storage.
    //
    //  Evaluate all of the operations on this object to bring the given op
    //  queue entry to the head of the queue.  This includes flushing all of
    //  the operations which depend upon the given instance of the object.
    //
    //
    virtual XilStatus           syncForWriting(XilTileList*        tile_list,
                                               XilOp*              op_flushing = NULL,
                                               XiliOpQueuePosition qposition =
                                               _XILI_OP_QUEUE_INVALID_POSITION);

    virtual XilStatus           allSync();

    //
    //  Set the op that generates results into this object.  This returns the
    //  op's entry in the op queue which is need whenever referencing this
    //  object again.
    //
    virtual XiliOpQueuePosition setOp(XilOp* new_op);
    
    //
    //  Get a reference to the op that generates results into this object.  It
    //  will be the entry which is at end of the op Queue.
    //
    virtual XilOp*              getOp();

    //
    //  Cleanup all references to the given operation at the queue position.
    //
    virtual void                cleanup(XiliOpQueuePosition qposition,
                                        XilOp*              op);
    virtual void                creatorCleanup(XiliOpQueuePosition qposition,
                                               XilOp*              op);

    //
    //  Add a dependent to this object.  The dependent is added for the
    //  active instance of the object (which is the op queue entry at the end
    //  of the queue).
    //
    virtual XiliOpQueuePosition addDependent(XilOp*       dependent_op,
                                             unsigned int src_branch);

    //
    //  Call XilOp::flush() for each of the dependents of this object to
    //  ensure that all of the operations dependent upon this source image
    //  will be completed before the current operation overwrites the data.
    //
    virtual XilStatus           flushDependents(XiliOpQueuePosition qposition =
                                                _XILI_OP_QUEUE_INVALID_POSITION);
    virtual XilStatus           flushDependents(XilTileList*        tile_list,
                                                XilOp*              op_flushing,
                                                XiliOpQueuePosition qposition);

    //
    //  Toss the operation associated with the current instance of this
    //  object if nobody depends on me.  The return value indicates whether
    //  the operation was tossed.
    //
    virtual Xil_boolean         tossIfNoDependents();

    //
    //  Test to indicate whether this object is >= -- and thus an operation
    //  writing into this object will cover all of the results contained in
    //  the given object.
    //
    virtual Xil_boolean         overwrites(XilDeferrableObject* obj)
    {
        //
        //  TODO: 12/12/95 jlf  This test needs more as does the op calling it
        //
        if(this == obj) {
            return TRUE;
        } else {
            return FALSE;
        }
    }
    //
    //----------------------------------------------------------------------------

    //----------------------------------------------------------------------------
    //
    //  Tile size support.  The operation scheduler will use this to
    //    information to split operations into smaller pieces.
    //  
    //
    //  Does this object support being tiled?
    //
    virtual Xil_boolean         canBeTiled();

    //
    //  Is the object's tile size set?
    //
    virtual Xil_boolean         isTileSizeSet();

    //
    //  Get the object's current tile size.
    //
    virtual XilStatus           getTileSize(unsigned int* xsize,
                                            unsigned int* ysize);

    //
    //  Initialize the object's tile size.
    //
    virtual XilStatus           initTileSize(unsigned int xsize,
                                             unsigned int ysize,
                                             XilTile*     new_tile_array = NULL);

    virtual XilStatus           initTileSize(XilDeferrableObject* def_object);

    //
    //  Get the number of tiles in the object
    //
    virtual unsigned int        getNumXTiles();
    virtual unsigned int        getNumYTiles();

    //
    //  This is required to be overrided by the derived classes because it's
    //  used by this object and is expected to return an accurate answer.
    //
    virtual unsigned int        getNumTiles() = 0;

    //
    //  Replace/Fill in the given tile list object with those tiles which
    //  intersect the storage area represented by the given box.
    //
    //  DEFAULT:  Ignores the box and returns a list of every tile in the
    //            object.
    //
    virtual XilStatus           getTileList(XilTileList*     tile_list,
                                            XilBox*          area = NULL);

    //
    //  Replace/Fill in the given tile list object with those tiles which
    //  intersect the object area of the given rect.
    //
    virtual XilStatus           getTileList(XilTileList*     tile_list,
                                            XiliRect*        rect);

    //
    //  Fill in the rect with object area represented by the specified tile
    //  number.
    //
    virtual XilStatus           getTileRect(XiliRect*     rect,
                                            XilTileNumber tile_number);

    //
    //  Clip the given XiliRect with the dimensions of the region represented
    //  by the given tile number. 
    //
    virtual XilStatus           clipToTile(XilTileNumber  tile_number,
                                           XiliRect*      rect);
    //
    //  Gets the current state of evaluation for the given tile.  It returns
    //  the status of the operation which writes into this object.
    //
    virtual XiliOpStatus        getOpStatus(XilTileNumber       tile_number,
                                            XiliOpQueuePosition qposition);

    virtual void                setOpStatus(XilTileNumber       tile_number,
                                            XiliOpQueuePosition qposition,
                                            XiliOpStatus        op_status);

    virtual Xil_boolean         allTilesDone(XiliOpQueuePosition qposition);

    //
    //  Indicate the given tile is no longer needed.
    //
    virtual void                releaseTile(XilTileNumber tnum);
    //
    //----------------------------------------------------------------------------

    //----------------------------------------------------------------------------
    //
    //  Method for getting the adjusted clipped region associated with
    //  object.  For Images, this means the origin adjusted roi clipped to the
    //  image. This may have different or nonexistent meaning for other
    //  deferrable objects. 
    //

    //
    //  Get any ROI the user may have set on this object.
    //
    virtual XilRoi*             getRoi();

    //
    //  Reference any ROI the user may have set on this object.
    //
    virtual XilRoi*             refRoi();

    //
    //  Return the global space roi of the object, in integer precision.
    //
    virtual XilRoi*             getGlobalSpaceRoi()=0;
    //
    //  Return the global space roi of the object with double precision   
    //
    virtual XilRoi*             getGlobalSpaceRoiWithDoublePrecision()=0;
    //
    //  Return the global space roi of the object with double precision,
    //  in "extent" space as opposed to "coordinate" space.
    //
    virtual XilRoi*             getExtentGlobalSpaceRoi()=0;

    //
    //  Return a reference to a rect which represents this object in global
    //  space.
    //
    virtual XiliRect*           getGlobalSpaceRect()=0;
    //
    //----------------------------------------------------------------------------

    //----------------------------------------------------------------------------
    //
    //  Methods for adjusting back and forth from global space to object space.
    //  For Images, this means origin adjustment. This may have different or 
    //  nonexistent meaning for other deferrable objects.
    //
    //----------------------------------------------------------------------------
    virtual XilStatus           convertToObjectSpace(XiliRect* rect);
    virtual XilStatus           convertToObjectSpace(XilRoi* roi);
    virtual XilStatus           convertToObjectSpace(XiliConvexRegion* cr);
    virtual XilStatus           convertToGlobalSpace(XiliRect* rect);
    virtual XilStatus           convertToGlobalSpace(XilRoi* roi);
    virtual XilStatus           convertToGlobalSpace(XiliConvexRegion* cr);

    //
    // Take a box and do the appropriate mapping of box to storage box
    //
    virtual XilStatus           setBoxStorage(XilBox* box);

    //----------------------------------------------------------------------------
    //
    //  Attach an IO device to the object, return a pointer to the associated
    //  IO device.
    //
    virtual XilStatus           setDeviceIO(XilDeviceIO* ioDevice);
    virtual XilDeviceIO*        getDeviceIO();
    virtual XilOp*              createCaptureOp(XilOp*       constructing_op,
                                                unsigned int branch_num);
    virtual XilOp*              createDisplayOp(XilOp* constructing_op,
                                                unsigned int branch_num);
    //
    //----------------------------------------------------------------------------

    //----------------------------------------------------------------------------
    //
    //  Change our refereces to the given non-deferrable object to the new
    //  non-deferrable object.  Since we selectively determine which objects
    //  to obtain via reference versus those obtained via copy, only a few
    //  of our objects need checking. 
    //
    virtual void                updateObjectReferences(XilNonDeferrableObject* oldobj,
                                                       XilNonDeferrableObject* newobj,
                                                       Xil_boolean             destroy);
    //
    //----------------------------------------------------------------------------

    //----------------------------------------------------------------------------
    //
    //  Reset our op queue and other DE information from the given object.
    //  It's a transfer of information.  The given image will have no
    //  dependencies or other information left in the queue while this object
    //  will have exactly the same information contained in the given object.
    //
    virtual XilStatus           transferOpQueueInfo(XilDeferrableObject* def_obj);
    //
    //----------------------------------------------------------------------------

    //----------------------------------------------------------------------------
    //
    //  Keep track of whether the storage represented by this image is valid
    //  with respect to what is currently on the I/O device.  When a molecule is 
    //  executed involving an object with an I/O device, any storage managed by
    //  XIL on this object is no longer valid.
    //
    virtual Xil_boolean         isStorageValid();
    virtual void                setStorageValidFlag(Xil_boolean valid_flag);
    //
    //----------------------------------------------------------------------------

    //----------------------------------------------------------------------------
    //
    virtual XiliOpQueueEntry*   getQueueEntry(XiliOpQueuePosition qpos);
    //
    //----------------------------------------------------------------------------

    //----------------------------------------------------------------------------
    //
    //  Get the current export mode for this object.
    //
    virtual int                 getExported();
    //
    //----------------------------------------------------------------------------

    //----------------------------------------------------------------------------
    //
    //  Determine whether this is a temporary deferrable object.
    //
    Xil_boolean         isTemp()
    {
        return isTempFlag;
    }

    //
    //  This is called to indicate this is a temporary object.  Once it's
    //  marked as such, it's considered a "write once, read once" object.
    //  It's usually marked as temporary during construction of a derived
    //  object... 
    //
    void                markTemp()
    {
        //
        //  A temporary object gets marked as an "invalid" image until it's
        //  written into at which time it's marked to be a "valid" image.  As
        //  soon as the temporary image is used as a source, it gets marked as
        //  "destroy when no dependents"
        //
        setValid(FALSE); 

        isTempFlag = TRUE;
    }

    //
    //  Get/Set whether the object will be destroyed when there are no
    //  dependents. 
    //
    void                        setDestroyWhenNoDependents(Xil_boolean flag)
    {
        deleteWhenNoDependents = flag;
    }

    Xil_boolean                 getDestroyWhenNoDependents()
    {
        return deleteWhenNoDependents;
    }
    //
    //----------------------------------------------------------------------------

protected:
                                XilDeferrableObject(XilSystemState* system_state,
                                                    XilObjectType   object_type)
                                    : XilObject(system_state, object_type)
    {
        isSynchronized         = FALSE;
        isTempFlag             = FALSE;
        deleteWhenNoDependents = FALSE;
        storageValidFlag       = TRUE;

        //
        //  The system state needs to keep track of all the potentially
        //  deferred objects in XIL in order to sync() them is
        //  setSynchronized() is called.
        //
        deferredObjectListPosition = getSystemState()->addDefObject(this);

        //
        //  Initialize the opQueue.
        //
        queueHead   = NULL;
        queueTail   = NULL;
        queueLength = 0;

        //
        //  By default, a deferrable object cannot be exported.  Derived
        //  objects can override this by initializing exportMode to something
        //  else in their construction.
        //
        exportMode  = XIL_NOT_EXPORTABLE;
    }

                                ~XilDeferrableObject()
    {
        if(deferredObjectListPosition != _XILI_SLLIST_INVALID_POSITION) {
            getSystemState()->removeDefObject(this,
                                              deferredObjectListPosition);
        }
    }

    //
    //  We overload preDestroy() to have an opportunity to flush the
    //  dependents and such prior to the actual destruction of the object. 
    //
    virtual Xil_boolean         preDestroy();

    //
    //  Tile information for this object
    //
    XilTile*                    tileArray;

    unsigned int                numXTiles;
    unsigned int                numYTiles;
    unsigned int                numTiles;

    XilTilingMode               tilingMode;

    Xil_boolean                 tileSizeIsSetFlag;
    unsigned int                tileXSize;
    unsigned int                tileYSize;

    Xil_boolean                 deleteWhenNoDependents;

    Xil_boolean                 storageValidFlag;

    //
    //  Variable that indicates whether the object is considered exported to
    //  the API (and thus, is owned by the application) or whether it's
    //  "imported".
    //
    int                         exportMode;

    //
    //  Op Queue Manipulation Methods
    //

    //
    //  Return the position next available entry on the op queue.  If the
    //  queue is larger than its maximum supported length, then the head entry
    //  on the queue is sync'd and dependents flushed.
    //
    XiliOpQueuePosition         qNewEntry(unsigned int num_tiles)
    {
        XiliOpQueuePosition pos = _XILI_OP_QUEUE_INVALID_POSITION;

        if(queueLength == _XILI_MAX_OPQUEUE_LENGTH &&
           _qFlushQueueHead() == XIL_FAILURE) {
            XIL_OBJ_ERROR(getSystemState(), XIL_ERROR_INTERNAL, "di-424",
                          FALSE, this);
        } else {
            //
            //  If we have entries already, march to the next entry in the queue
            //  otherwise, just use the one we're currently pointed at.
            //
            if(queueLength++ != 0) {
                queueTail =
                    (queueTail == (_XILI_MAX_OPQUEUE_LENGTH - 1)) ? 0 : queueTail + 1;
            }

            //
            //  We need to reinitialize the elements in the XiliOpQueueEntry we're
            //  returning.
            //
            queueData[queueTail].reinit(this, num_tiles);

            pos = queueTail;
        }

        return pos;
    }

    //
    //  Reference the queue entry at the given position.
    //
    XiliOpQueueEntry*           qRef(XiliOpQueuePosition pos)
    {
        return &queueData[pos];
    }

    //
    //  Release/remove the entry at the given position from the queue.  The
    //  position must be either the head or the tail of the queue.
    //
    void                        qRemove(XiliOpQueuePosition pos)
    {
        if(queueLength != 0) {
            if(pos == queueHead) {
                if(pos != queueTail) {
                    //
                    //  Remove the head by skipping to the next entry.
                    //
                    queueHead =
                        (queueHead == (_XILI_MAX_OPQUEUE_LENGTH - 1)) ? 0 :
                        queueHead + 1;
                }
            
                queueLength--;
            } else if(pos == queueTail) {
                //
                //  Remove the tail by skipping to the previous entry.
                //
                queueTail =
                    (queueTail == 0) ? (_XILI_MAX_OPQUEUE_LENGTH - 1) : queueTail - 1;
                
                queueLength--;
            }
        }

        //
        //  Can't remove entries from the middle of the queue.  If we're
        //  given a position other than head or tail, it's probably just
        //  an invalid position or removing something that has already
        //  been removed.
        //
    }

    //
    //  Return the head, the tail and the length.
    //
    XiliOpQueuePosition         qHead()
    {
        return queueLength == 0 ? _XILI_OP_QUEUE_INVALID_POSITION : queueHead;
    }

    XiliOpQueuePosition         qTail()
    {
        return queueLength == 0 ? _XILI_OP_QUEUE_INVALID_POSITION : queueTail;
    }

    unsigned int                qLength()
    {
        return queueLength;
    }

    //
    //  Return the position of the next entry on the queue.
    //  _XILI_OP_QUEUE_INVALID_POSITION is returned if there are no more
    //  entries on the queue.
    //
    XiliOpQueuePosition         qNext(XiliOpQueuePosition pos)
    {
        if(pos == queueTail) {
            return _XILI_OP_QUEUE_INVALID_POSITION;
        } else {
            return (pos == (_XILI_MAX_OPQUEUE_LENGTH - 1)) ? 0 : pos + 1;
        }
    }

private:
    //
    //  Support routine for qNewEntry() to handle flushing the head entry of
    //  the queue.
    //
    XilStatus                   _qFlushQueueHead();

    XiliOpQueueEntry            queueData[_XILI_MAX_OPQUEUE_LENGTH];
    XiliOpQueuePosition         queueHead;
    XiliOpQueuePosition         queueTail;
    unsigned int                queueLength;

    Xil_boolean                 isSynchronized;

    XiliSLListPosition          deferredObjectListPosition;

    //
    //  Temporary obejct flag and control variables.
    //
    //  This indicates whether this is a temporary object which means it is a
    //  'write once, read once' object and does not support storage aquisition,
    //  changing it after writing into it once and is destroyed when there are
    //  no dependents on it by XIL (i.e. once it's been used as a source to an
    //  operation).  It also means that tiles within this object may never be
    //  fully populated.  Since the object has effectively been destroyed, all
    //  of the storage associated with tiles which have no more dependencies
    //  can be deleted immediately.  A temporary object is effectively
    //  destroyed as soon as it's used as the source of an operation.
    //
    Xil_boolean                 isTempFlag;
#endif // _XIL_PRIVATE_DATA
