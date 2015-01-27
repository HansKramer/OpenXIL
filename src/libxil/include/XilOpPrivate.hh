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
//  File:	XilOpPrivate.hh
//  Project:	XIL
//  Revision:	1.98
//  Last Mod:	10:20:55, 03/10/00
//
//  Description:
//	The base Op class.  IHVs derive off of this to create the
//      different flavors of operations.  Thus, notice that the
//      private data is not only reserved for libxil to see.  This is
//      because the derived classes must know the size of the base
//      class at this stage of the game...
//      
//	You should not use any interface that is not in this
//      header file.
//
//      NOTE:  The XilOp class would be a good candidate for Object
//             Binary Interface support if the XilOp interface is
//             exposed beyond a Sun Private Interface. 
//	
//------------------------------------------------------------------------
//	COPYRIGHT
//------------------------------------------------------------------------
#pragma ident	"@(#)XilOpPrivate.hh	1.98\t00/03/10  "

#ifdef _XIL_PRIVATE_INCLUDES

#include "_XilSystemState.hh"
#include "_XilRoi.hh"
#include "_XilCondVar.hh"

#include "XilError.hh"
#include "XiliOpTreeNode.hh"
#include "XiliMarker.hh"
#include "XiliStack.hh"
#include "XiliConvexRegion.hh"

#define _XILI_OP_QUEUE_INVALID_POSITION -1

typedef int XiliOpQueuePosition;

#ifndef _XILI_OP_STATUS
#define _XILI_OP_STATUS

//
//  Enumerations which indicate the deferred state of an op.
//
enum XiliOpStatus { XILI_DEFERRED,
                    XILI_EXECUTING,
                    XILI_EVALUATED };

//
//  Enumerations which are used for determining what type of I/O operation
//  the op is...  
//
enum XiliOpIOType { XILI_OP_IO_TYPE_ANY,
                    XILI_OP_IO_TYPE_CAPTURE,
                    XILI_OP_IO_TYPE_DISPLAY };

//
//  Enumerations for whether an op argument should be deleted, not deleted
//  or destroyed (i.e. is an object) when the op goes away.
//
enum XilDestroyMethod { XIL_DONT_DELETE,
                        XIL_DELETE,
                        XIL_DESTROY,
                        XIL_RELEASE_REF };

//
//  Function structure for per-function compute information.
//
class XiliFuncData {
public:
    XilDeviceIO*          ioDevice;
    XilDeviceCompression* codecDevice;
    
    XiliFunctionDef*      funcDef;
    unsigned int          funcId;
    void*                 funcInfo;
    XilStatus             funcStatus;

    XiliFuncData*         next;
};

#endif // _XILI_OP_STATUS

#endif // _XIL_PRIVATE_INCLUDES

#ifdef _XIL_PRIVATE_DATA

//
//  NOTE:  These constants are used below to compute how large the
//         XilOp data segment for objects that derive from the XilOp
//         class.
//

#define XIL_MAX_SRC_DEFOBJS   4
#define XIL_MAX_DST_DEFOBJS   2
#define XIL_MAX_OPPARAM      10         // max number of params to an op


union XilParam {		// union structure for op parameters
    int                     i;
    unsigned int            ui;
    XilLongLong             ll;
    float                   f;
    double                  d;
    void*                   p;
    XilNonDeferrableObject* o;
};

//
//  The assorted types an op parameter can assume.
//
typedef enum { XILTYPE_UNDEF,
               XILTYPE_INT,
               XILTYPE_UINT,
               XILTYPE_LONGLONG,
               XILTYPE_FLOAT,
               XILTYPE_DOUBLE,
               XILTYPE_PTR,
               XILTYPE_OBJECT } XilType;


public:
    //
    //  Destroy this op.  this can be NULL
    //
    void                  destroy()
    {
        if(this != NULL) {
            opStatus = XILI_EVALUATED;

            if(deleteMe == TRUE && refCount == 0) {
                delete this;
            }
        }
    }

    //
    //  Can be called from inside the library to flush this operation only if
    //  the caller is holding the DAG lock.  The caller cannot be holding
    //  other locks aside from the DAG lock that may be used elsewhere in the
    //  library.
    //
    XilStatus             flush();

    //
    //  DAG Manipulation Routines
    //
    XilStatus             flush(XilTileList* tile_list);

    //
    //  A special version of flush which flushes an operation in a
    //  forward-mapping chain of operations.
    //
    XilStatus             flushForward(XiliRect*     src_rect,
                                       unsigned int  src_num,
                                       XilOp*        bottom_op,
                                       XilTileNumber bottom_tile);

    //
    //  Removes any references to the given op from this op.
    //
    void                  cleanupReferences(XilOp* cleanup_op)
    {
        for(unsigned int i=0; i<numSrcs; i++) {
            if(srcOp[i] == cleanup_op) {
                srcOp[i] = NULL;
            }
        }
    }

    void                  cleanupReferences(XilDeferrableObject* cleanup_obj)
    {
        unsigned int i;
        for(i=0; i<numSrcs; i++) {
            if(src[i] == cleanup_obj) {
                src[i] = NULL;
            }
        }
        for(i=0; i<numDsts; i++) {
            if(dst[i] == cleanup_obj) {
                dst[i] = NULL;
            }
        }
    }

    //
    //  Used in the case where the entire operation or a single tile of the
    //  operation is reduced to no processing area in the destination.  We
    //  still want to update any predecessors that live on the op-queue by
    //  calling evaluateTileToPosition().  This routine handles taking care of
    //  finishing the empty-execution flush. 
    //
    XilStatus             finishEmptyFlush(XilTileNumber tile_number,
                                           XilMutex*     tile_list_mutex);
    XilStatus             finishEmptyFlush(XilTileList*  tile_list);

    //
    //  Executes this operation by potentially deferring the op, etc.
    //
    XilStatus             insert(Xil_boolean lock_dag = TRUE);
    XilStatus             execute();

    //
    //  For DAG manipulation routines to aquire DeferrableObjects
    //
    XilDeferrableObject*  getSrc(unsigned int n)
    {
        return n > numSrcs ? NULL : src[n-1];
    }

    XilDeferrableObject*  getDst(unsigned int n)
    {
        return n > numDsts ? NULL : dst[n-1];
    }

    //
    //  Get the number of sources or destinations for this operation.
    //
    unsigned int          getNumSrcs() const
    {
        return numSrcs;
    }

    unsigned int          getNumDsts() const
    {
        return numDsts;
    }

    //
    //  Aquire the operation which generates the given numbered source.
    //
    XilOp*                getSrcOp(unsigned int n)
    {
        return srcOp[n];
    }

    //
    //  Set the data returned by a preprocess routine on the op.
    //
    XiliFuncData*         addFuncData(XiliFunctionDef*      func_def,
                                      unsigned int          func_id,
                                      void*                 func_info,
                                      XilDeviceIO*          func_io_device,
                                      XilDeviceCompression* func_codec_device,
                                      XilStatus             func_status)
    {
        XiliFuncData* tmpxfd  = new XiliFuncData;

        if(tmpxfd == NULL) {
            XIL_ERROR(getSystemState(), XIL_ERROR_RESOURCE, "di-1", TRUE);
        } else {
            tmpxfd->funcDef     = func_def;
            tmpxfd->funcId      = func_id;
            tmpxfd->funcInfo    = func_info;
            tmpxfd->ioDevice    = func_io_device;
            tmpxfd->codecDevice = func_codec_device;
            tmpxfd->funcStatus  = func_status;

            funcDataListMutex.lock();

            tmpxfd->next       = funcDataList;
            funcDataList       = tmpxfd;

            funcDataListMutex.unlock();
        }

        return tmpxfd;
    }

    //
    //  Check whether the preprocess function was called for the given funcDef
    //  and if so indicate its status.
    //
    XilStatus             checkFuncData(XiliFunctionDef* func_def,
                                        XilStatus*       func_status)
    {
        funcDataListMutex.lock();

        XiliFuncData* tmpxfd = funcDataList;
        while(tmpxfd) {
            if(tmpxfd->funcDef == func_def) {
                //
                //  We found the function we're looking for so return
                //  XIL_SCUCESS indicating a match and return the preprocess
                //  function's status information.
                //
                *func_status = tmpxfd->funcStatus;

		funcDataListMutex.unlock();
		
                return XIL_SUCCESS;
            }

            tmpxfd = tmpxfd->next;
        }

	funcDataListMutex.unlock();
	
        return XIL_FAILURE;
    }

    //
    //  Change our refereces to the given non-deferrable object to the new
    //  non-deferrable object. 
    //
    void                  updateObjectReferences(XilNonDeferrableObject* oldobj,
                                                 XilNonDeferrableObject* newobj,
                                                 Xil_boolean             destroy);

    //
    //  Routines so the XilImage can get the queue position for this
    //  specific operation. 
    //
    XiliOpQueuePosition   getSrcOpQueuePosition(XilDeferrableObject* defobj) const
    {
        for(unsigned int i=0; i<numSrcs; i++) {
            if(src[i] == defobj) {
                return srcQueuePos[i];
            }
        }

        return _XILI_OP_QUEUE_INVALID_POSITION;
    }

    XiliOpQueuePosition   getDstOpQueuePosition(XilDeferrableObject* defobj) const
    {
        if(dst[0] == defobj) {
            return dstQueuePos[0];
        }

        for(unsigned int i=1; i<numDsts; i++) {
            if(dst[i] == defobj) {
                return dstQueuePos[i];
            }
        }

        return _XILI_OP_QUEUE_INVALID_POSITION;
    }

    XiliOpQueuePosition   getSrcOpQueuePosition(unsigned int src_num) const
    {
        return srcQueuePos[src_num];
    }

    XiliOpQueuePosition   getDstOpQueuePosition(unsigned int dst_num) const
    {
        return dstQueuePos[dst_num];
    }

    void                  setSrcOpQueuePosition(XilDeferrableObject* defobj,
                                                XiliOpQueuePosition  pos)
    {
        for(unsigned int i=0; i<numSrcs; i++) {
            if(src[i] == defobj) {
                srcQueuePos[i] = pos;
            }
        }
    }

    //
    //  Do best job to get a system state from the objects we have attached to
    //  us...it's used for error reporting.
    //
    XilSystemState*       getSystemState();

    //
    //  Functions that determine the characteristics of the op.
    //  Derived classes can overload these functions to modify the
    //  characterisitcs of the op.
    //
    virtual Xil_boolean   isInPlace() const;
    virtual Xil_boolean   isCommutative() const;
    virtual Xil_boolean   isIOOperation(XiliOpIOType type = XILI_OP_IO_TYPE_ANY);
    virtual Xil_boolean   flushOnInsert() const;

    //
    //  Whether this operation can be split by the core into multiple strips
    //  for threading purposes.
    //
    virtual Xil_boolean   canBeSplit();

    //
    //  Test whether this operation can map a region in its source to a region
    //  in its destination.  By default, the answer is TRUE.
    //
    virtual Xil_boolean   canForwardMap();

    //
    //  Virtual function that implements the splitOnTileBoundaries() for a
    //  specific operation.
    //
    virtual XilStatus     vSplitOnTileBoundaries(XilBoxList* boxlist);

    //
    //  This routine is used by vSplitOnTileBoundaries.
    //
    //  The routine takes as input which image it's splitting on,
    //  and the x and y delta's for tile.
    //
    //  The BoxList then increases the number of BoxListEntries every
    //  time it divides the specified image on a boundary.
    //
    virtual Xil_boolean   divideBoxList(XilBoxList*   boxlist,
                                        unsigned int  box_number,
                                        unsigned int  tile_xdelta,
                                        unsigned int  tile_ydelta);

    //
    //  generateIntersectedRoi() intersects the src and destination ROIs to
    //  produce an intersected ROI for the destination in global space which
    //  describes all of the pixels the operation is to touch.
    //
    virtual XilStatus     generateIntersectedRoi();

    //
    //  Clips the given rect (which is expected to be valid in the given
    //  object's image space) to tile number in the specified object.
    //
    virtual XilStatus     clipToTile(XilDeferrableObject* defobj,
                                     XilTileNumber        tile_number,
                                     XiliRect*            rect);

    //
    //  These MAY BE in-place operations, and that case needs to be checked in
    //  overloaded routines. We don't default the src_number because
    //  we want to make the core clean although in the cases where these
    //  are overloaded there generally is only one source and destination.
    //
    virtual XilStatus     gsBackwardMap(XiliRect*    dst_rect,
                                        XiliRect*    src_rect,
                                        unsigned int src_number);
    
    virtual XilStatus     gsForwardMap(XiliRect*     src_rect,
                                       unsigned int  src_number,
                                       XiliRect*     dst_rect);

    //
    //  Readjustment of box storage is used to give an I/O operation the
    //  opportunity to undo changes it may have made to the box in
    //  setBoxStorage() to deal with controllingImage/op mismatch.
    //
    //  This is used by splitOnTileBoundaries() to get boxes associated with
    //  I/O operations valid for their destination or source operation.
    //
    virtual XilStatus     readjustBoxStorage(XiliRect* dst_rect);
    
    //
    //  The overload function for supporting backward mapping a single
    //  point in the destination to the source.
    //
    //  Backward map a single point in destination box space to the
    //  corresponding point in source box space.  The last (optional) argument
    //  indicates which source to backward map into.
    //
    virtual XilStatus     vBackwardMap(XilBox*       dst_box,
                                       double        dx,
                                       double        dy,
                                       XilBox*       src_box,
                                       double*       sx,
                                       double*       sy,
                                       unsigned int  src_number);

    //
    //  Data collection operator methods to report intermediate
    //  results and final results. These are callled by the compute
    //  routine.
    //
    virtual XilStatus	   vReportResults(void* results[]);
    virtual XilStatus	   completeResults();

    //
    //  Set the storage information on the box for the given rect (which has 
    //  image space coordinates for the given deferrable object).
    //
    //  This is used in the final step (which is the generation of the box
    //  list to pass into a compute routine) prior to calling the compute
    //  routine which puts everything into image space.  generateBoxList()
    //  calls this virtual function (which by default just calls the given
    //  object to move the box into its space from global space) to convert
    //  the given box into object space.  Specific operations may need to do
    //  more than just take the origins into account.
    //
    virtual XilStatus      setBoxStorage(XiliRect*            rect,
                                         XilDeferrableObject* object,
                                         XilBox*              box);

    //
    //  Routines that are used in order to move ROIs and rects between global
    //  and object space.
    //
    virtual XilStatus      moveIntoObjectSpace(XiliRect*            rect,
                                               XilDeferrableObject* object);

    virtual XilStatus      moveIntoObjectSpace(XilRoi*              roi,
                                               XilDeferrableObject* object);

    virtual XilStatus      moveIntoGlobalSpace(XiliRect*            rect,
                                               XilDeferrableObject* object);

    virtual XilStatus      moveIntoGlobalSpace(XilRoi*              roi,
                                               XilDeferrableObject* object);

    //
    //  Switch this op to use any alternate operation that it may support.
    //  This can be used if an operation has a fallback operation it wants to
    //  represent if the first one fails.  For example, if an op wants
    //  multiple GPI entry points for special cases of its operation, then it
    //  would initially create itself with the special case.  If no compute
    //  device implements the operation, this routine is called and if it
    //  returns XIL_SUCCESS, the operation is flushed with its new
    //  characteristics.
    //
    //  The only things that can change in this call are:
    //
    //    - the op number
    //    - the parameters
    //
    //  Specifically, the operation must have the same number of sources and
    //  destinations and must not alter the order of these images.
    //
    virtual XilStatus      switchToAlternateOp();

    //
    //  Get global space ROI for the requested source or destination.
    //
    virtual XilRoi*        getSrcGlobalSpaceRoi(unsigned int src_num);
    virtual XilRoi*        getDstGlobalSpaceRoi(unsigned int dst_num = 0);

    //
    //  Reference the intersectedRoi.
    //
    XilRoi*                getIntersectedRoi();
    
    //
    //  Generates the XilBoxList for an operation.  This routine handles the
    //  generation of the box list for for compute operations with this as the
    //  destination node in a molecule chain.  So, if the operation is a
    //  molecule, this routine backward maps the destination region into the
    //  original sources to generate a box list.
    //
    XilStatus              generateBoxList(XilBoxList*  box_list,
                                           XiliRect*    dst_rect);

    //
    //  Get the moleculeBottom member.  moleculeBottom indicates whether this
    //  op is in the process of being flushed as part of a molecule and
    //  which op is at the bottom of the molecule.  It's used to stop from
    //  having the dependencies of ops in the middle of the molecule being
    //  flushed when evaluating the bottom ops (think in-place operations).
    //
    XilOp*                 getMoleculeBottom()
    {
        return moleculeBottom;
    }

    //
    //  Return whether the intersectedRoi is in object space.
    //
    Xil_boolean            getIntersectedRoiIsInObjectSpace()
    {
        return intersectedRoiIsInObjectSpace;
    }

    //
    //  Set whether the intersectedRoi is in object space.
    //
    void                   setIntersectedRoiIsInObjectSpace(Xil_boolean flag)
    {
        intersectedRoiIsInObjectSpace = flag;
    }

    //
    //  Return the current evaluation status of the op
    //
    XiliOpStatus           getStatus()
    {
        return opStatus;
    }

    //
    //  Set the isNewFrameFlag (cannot be done while op is in compute routine
    //  since setting the flag is MT-UNSAFE) -- it's expected to be done when
    //  the op is constructed. 
    //
    void                   setIsNewFrameFlag(Xil_boolean flag_val)
    {
        isNewFrameFlag = flag_val;
    }

protected:
    //
    //   Does the current operation cover up all of the pixels in the
    //   destination images that the previous ops writing into those
    //   images?
    //
    virtual Xil_boolean  thisOpCoversPreviousOp();

    //
    //  Set the source and destination deferrable objects.
    //
    void                 setSrc(unsigned int n,
                                XilDeferrableObject* src);  // set nth input
    void                 setDst(unsigned int n,
                                XilDeferrableObject* dst);  // set nth output

    //
    //  Set Parameter Methods
    //
    //  NOTE:  The dstroy argument indicates whether the XilOp
    //         destructor should destroy the given object or data
    //         pointer.
    //
    void                 setParam(unsigned int n,
                                  int         param);
    void                 setParam(unsigned int n,
                                  unsigned int param);
    void                 setParam(unsigned int n,
                                  XilLongLong  param);
    void                 setParam(unsigned int n,
                                  float        param);
    void                 setParam(unsigned int n,
                                  double       param);
    void                 setParam(unsigned int     n,
                                  void*            param,
                                  XilDestroyMethod destroy = XIL_DELETE);
    void                 setParam(unsigned int            n,
                                  XilNonDeferrableObject* param,
                                  XilDestroyMethod        destroy = XIL_DESTROY);

    //
    //  Set the op number for this operation.
    //
    //  NOTE:  Use this function with care.  It modifies the operation number
    //         that is usually set at construction time.  It's generally meant
    //         to be used by switchToAlternateOp()
    //
    void                 setOpNumber(XilOpNumber op_num);

    //
    //  Set the "forward map" attribute on this op.  This should be set
    //  when an operation needs data pushed from the source to the
    //  destination.
    //
    void                 setForwardMapping(Xil_boolean flag = TRUE);

    //
    //  Set the "tile reordering" attribute on this op.  This should be set
    //  when an operation needs to verify and potentially change the tile
    //  ordering when processing. 
    //
    void                 setReordersTiles(Xil_boolean flag = TRUE)
    {
        reordersTiles = flag;
    }

    //
    //  Permits the op to reorder the processing of tiles by the core.  This
    //  is only called if reorderTiles is set.
    //
    virtual XilStatus    reorderTileProcessing(XilTileList* old_list,
                                               XilTileList* new_list);

    //
    //  Constructor
    //
                         XilOp(XilOpNumber op_num);

    //
    //  Destructor
    //
    virtual              ~XilOp();


private:
    //
    //  Returns a pointer to the node in the tree that is the longest chain
    //  beginning at this node as limited by max_length.
    //
    //  max_length provides a limit on how long the chain can be (usually one
    //  less than one that just failed) or UINT_MAX, which indicates there is
    //  no maximum. 
    //
    XiliOpTreeNode*       findLongestOpChain(unsigned int* max_length);

    //
    //  Determines which DAG the op and the objects attached to the op will
    //  be insterted into.  At the conclusion of the call, all of the objects
    //  will have their DAG information set and the DAG associated with 
    //  this op will be locked.
    //
    //  The function returns an aquired dag ref for the calling routine to use
    //  and must release.
    //
    XiliDagRef*           setupAndLockDAG();

    //
    //  Look at an array of full boxes and adjust and clip with the origin
    //  information.
    //
    XilStatus             originAdjustInitialBoxes(XilBox* box_array);

    //
    //  Internal reference counting so threads sleeping on a flush still have
    //  an op when they're done.
    //
    void                  aquire()
    {
        refCount++;
    }

    void                  release()
    {
        if(--refCount == 0 && opStatus == XILI_EVALUATED && deleteMe == TRUE) {
            destroy();
        }
    }

    //
    //  Update the status of each of the given tiles and return whether any
    //  need to be flushed.
    //
    Xil_boolean           updateStatus(XilTileList* tile_list);

    //
    //  Flush the op which starts a forward-flushing chain of operations.
    //
    XilStatus             flushStartOp(XilOp*       bottom_op,
                                       XilTileList* bottom_tile_list);

    //
    //  If an operation cannot be split, then we may need to generate a new
    //  tile list representing the entire destination.  This function takes a
    //  tile list and makes sure it contains all of the tiles in the
    //  destination or it will create a new tile list containing all of the
    //  tiles in the entire destination.  It's used by flush(XilTileList*).
    //
    XilTileList*          makeTileListForEntireDst(XilDeferrableObject* pobj,
                                                   XilTileList* tile_list);

    //
    //  Call any outstanding postprocess routines.
    //
    void                  callPostprocessRoutines();

    //
    //  Test whether this operation is part of a "forward mapping" chain.
    //
    Xil_boolean           isForwardMapping();

    //
    //  Return whether the op can be safely put in a molecule.
    //
    //  TODO: 3/6/97 jlf  We can do better than this for molecule acceptance
    //                    by intersecting ROIs down the chain to produce an
    //                    area in the destination.  
    //
    Xil_boolean           isOkForMolecule();

    //
    //  ---***--- CRITICAL! ---***---
    //
    //   If you add or remove data
    //     here, then you MUST bump
    //      the count in XilOp.hh!
    //
    //  ---***--- CRITICAL! ---***---
    //

    //
    //  Source Deferrable Objects
    //
    XilDeferrableObject*  src[XIL_MAX_SRC_DEFOBJS];

    //
    //  Destination Deferrable Objects.
    //
    //  XIL supports multiple destinations by designating one destination as
    //  the primary destination object and other destinations are "alternate"
    //  destinations who are not treated with the full level of capabilities
    //  as a regular destination.  They effectively "mirror" the primary
    //  destination.
    //
    //  What we do is set dst to point to dsts[0].  Those portions of the core
    //  that only deal with the primary destination reference dst.  Those that
    //  have code that needs to do things with each destination uses dsts. 
    //
    XilDeferrableObject*  dst[XIL_MAX_DST_DEFOBJS];
    
    //
    //  Ops
    //
    XilOp*                srcOp[XIL_MAX_SRC_DEFOBJS];

    //
    //  Queue Positions
    //
    XiliOpQueuePosition   srcQueuePos[XIL_MAX_SRC_DEFOBJS];
    XiliOpQueuePosition   dstQueuePos[XIL_MAX_DST_DEFOBJS];
    
    //
    //  Capture is a special case where if the op derived from capture goes
    //  away, then the capture op goes away.
    //
    Xil_boolean           srcIsCapture[XIL_MAX_SRC_DEFOBJS];

    //
    //  Intersected Roi for primary dst image 
    //
    XilRoi                intersectedRoi;
    Xil_boolean           intersectedRoiIsSet;
    Xil_boolean           intersectedRoiIsInObjectSpace;

    //
    //  Parameters
    //
    unsigned int          numParams;
    XilParam              params[XIL_MAX_OPPARAM];
    XilDestroyMethod      paramDestroyMethod[XIL_MAX_OPPARAM];
    XilType               paramType[XIL_MAX_OPPARAM];

    //
    //  Counts
    //
    unsigned int          numSrcs;
    unsigned int          numDsts;
    
    //
    //  Status of Op
    //
    XiliOpStatus          opStatus;

    //
    //  Condition variable to wait for the op status to change.
    //
    XilCondVar            opStatusCondVar;
    unsigned int          refCount;

    //
    //  Op Number
    //
    XilOpNumber           opNumber;

    //
    //  Reference to our DAG
    //
    XiliDagRef*           dagRef;

    //
    //  Indicates whether cleanup should delete this op or not.  It is used by
    //  execute() to tell cleanup() not to delete it.  execute() will delete
    //  it after the op is no longer needed (for things like
    //  completeResults()).
    //
    Xil_boolean           deleteMe;

    //
    //  funcData is a linked list of information set by the preprocess
    //  routines associated with functions.  It also keeps track of functions
    //  that should no longer be called for this operation (due to a
    //  preprocess call failing for example).
    //
    XiliFuncData*         funcDataList;
    XilMutex              funcDataListMutex;

    //
    //  List of operations starting at this op.  It's a list of operations for
    //  a molecule and contains the ops in a depth-first order.
    //
    XilOp**               opList;

    //
    //  Cache the information about the function this op should be calling.
    //  Once one thread or a flush of an area has done the work to determine
    //  which opTreeNode function should be called for this operation, store
    //  the information here so subsequent threads don't duplicate the effort.
    //
    XiliOpTreeNode*       funcDef;
    unsigned int          funcLength;

    //
    //  Indicate whether the chain concluding at this op is being processed as
    //  a "forward mapped" chain -- usually due to a PUSH type of capture
    //  device.  If we're forward mapping, then it follows a different path 
    //  for being executed.
    //
    Xil_boolean           forwardMappingFlag;
    XilOp*                forwardStartOp;

    //
    //  Points to the bottom operation of a molecule.  It's set when this
    //  operation is part of a molecule.
    //
    XilOp*                moleculeBottom;

    //
    //  The effective system state for this operation.
    //
    XilSystemState*       systemState;

    //
    //  Flags indicating the behavior attributes of this operation.
    //
    Xil_boolean           reordersTiles;

    //
    //  Flag that indicates whether a capture routine should capture a new
    //  image (i.e. TRUE) or continuing returning pieces of the current
    //  image.  Since its called from compute routines and it changes shared
    //  data during the call, the flag needs to be protected in isNewFrame()
    //
    Xil_boolean           isNewFrameFlag;
    XilMutex              isNewFrameMutex;

#endif // _XIL_PRIVATE_DATA
