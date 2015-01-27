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
//  File:	XiliOpQueueEntry.hh
//  Project:	XIL
//  Revision:	1.38
//  Last Mod:	10:21:16, 03/10/00
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
#pragma ident	"@(#)XiliOpQueueEntry.hh	1.38\t00/03/10  "

#ifndef _XILI_OPQUEUE_ENTRY_HH
#define _XILI_OPQUEUE_ENTRY_HH

#include "_XilDefines.h"

#include "_XilCondVar.hh"
#include "_XilMutex.hh"
#include "_XilOp.hh"

const unsigned int _XILI_MAX_STACK_DEPENDENTS = 8;

class XiliOpQueueEntry {
public:
    //
    //  Dependents list management
    //
    unsigned int   getNumDependents()
    {
        return numDependents;
    }

    void           addDependent(XilOp*       additional_op,
                                unsigned int branch)
    {
        dependents[numDependents].op       = additional_op;
        dependents[numDependents++].branch = branch;

        if(numDependents == maxDependents) {
            maxDependents  = maxDependents<<1;

            heapDependents =
                (XiliDepInfo*)realloc(heapDependents,
                                      maxDependents*sizeof(XiliDepInfo));
            if(heapDependents == NULL) {
                XIL_ERROR(NULL, XIL_ERROR_RESOURCE, "di-1", TRUE);
            } else {
                if(maxDependents == (_XILI_MAX_STACK_DEPENDENTS<<1)) {
                    xili_memcpy(heapDependents, stackDependents,
                                _XILI_MAX_STACK_DEPENDENTS*sizeof(XiliDepInfo));
                }

                dependents = heapDependents;
            }
        }
    }

    void           removeDependent(XilOp* remove_op)
    {
        for(unsigned int i=0; i<numDependents; i++) {
            //
            //  Do we remove it from our list?
            //
            if(dependents[i].op == remove_op) {
                numDependents--;

                for(unsigned int j=i; j<numDependents; j++) {
                    dependents[j] = dependents[j+1];
                }

                if(numDependents != 0) {
                    i--;
                }
            }
        }
    }

    Xil_boolean    isDependent(XilOp* remove_op)
    {
        for(unsigned int i=0; i<numDependents; i++) {
            //
            //  Do we remove it from our list?
            //
            if(dependents[i].op == remove_op) {
                return TRUE;
            }
        }

        return FALSE;
    }

    void           flushDependents()
    {
        for(unsigned int i=0; i<numDependents; i++) {
            (dependents[i].op)->flush();
        }
    }

    void           cleanupDependents(XilOp* cleanup_op)
    {
        for(unsigned int i=0; i<numDependents; i++) {
            //
            //  Do we remove it from our list?
            //
            //  If so, we pack the list, decrease i and numDependents.
            //
            if(dependents[i].op == cleanup_op) {
                numDependents--;

                for(unsigned int j=i; j<numDependents; j++) {
                    dependents[j] = dependents[j+1];
                }

                if(numDependents != 0) {
                    i--;
                }
            }
        }
    }

    //
    //  Cleanup references of the op from this entry of the queue and the ops
    //  which depend on this entry in the queue. 
    //
    void           creatorCleanup(XilOp* cleanup_op)
    {
        for(unsigned int i=0; i<numDependents; i++) {
            //
            //  Cleanup references to the given op from all of our dependents.
            //
            (dependents[i].op)->cleanupReferences(cleanup_op);
        }

        if(op == cleanup_op) {
            op = NULL;
        }
    }

    //
    //  Evaluate all of the dependents by flushing them.
    //
    XilStatus      evaluate(XilTileList* tile_list,
                            XilOp*       op_flushing);

    //
    //  Op set/get routines
    //
    XilOp*         getOp()
    {
        return op;
    }
    
    void           setOp(XilOp* new_op)
    {
        op = new_op;
    }

    //
    //  Tossed set/get routines
    //
    Xil_boolean    getTossed()
    {
        return opIsTossed;
    }
    
    void           setTossed(Xil_boolean new_val = TRUE)
    {
        opIsTossed = new_val;
    }

    //
    //  The op status refers to the current state of the operation which is
    //  writing into the given instance of this object.  The op uses this to
    //  determine if flush() needs to process anything to bring a tile
    //  up-to-date.
    //
    void           setOpStatus(XilTileNumber  tile_number,
                               XiliOpStatus   op_status);
    
    XiliOpStatus   getOpStatus(XilTileNumber tile_number);

    //
    //  Test to see if all of the tiles for this image have been flushed and
    //  are up-to-date.
    //
    //  TODO: 1/22/96 jlf  Fix bug with child images.
    //
    //    When the opStatusArray is created for a child image, it's too
    //    large because it encompases all of the tiles of the parent.
    //    We need to construct a smaller array comprising only the tiles
    //    contained in the child image and a conversion factor (something to
    //    add to the offset to get the child's first tile) or routine of some
    //    kind?  This falls into a problem with having the parent keep track
    //    of everything, but I think it's a fixable problem.
    //
    Xil_boolean    allTilesDone();

    //
    //  When the op queue information is transferred from one deferrable
    //  object to another deferrable object, the defobj information needs to
    //  be updated.
    //
    void           setDeferrableObject(XilDeferrableObject* parent_obj)
    {
        defobj = parent_obj;
    }

    void           reinit(XilDeferrableObject* def_obj,
                          unsigned int         num_tiles)
    {
        this->~XiliOpQueueEntry();

        initVars(def_obj, num_tiles);
    }

    //
    //  Constructor
    //
                   XiliOpQueueEntry(XilDeferrableObject* parent_obj,
                                    unsigned int         num_tiles)
    {
        initVars(parent_obj, num_tiles);
    }

    //
    //  Constructor for array in XilDeferrableObject.
    //
                   XiliOpQueueEntry()
    {
        //
        //  For this one, just do what is required to make the destructor work
        //  ok because we reinit things everytime the DefObj gets a new one of
        //  us.
        //
        opStatusArray   = NULL;
        heapDependents  = NULL;
        depTileCnts     = NULL;
        notifyList      = NULL;
    }

    //
    //  Destructor
    //
                   ~XiliOpQueueEntry()
    {
        if(opStatusArray != &singleTileOpStatus) {
            delete [] opStatusArray;
        }

        if(heapDependents != NULL) {
            delete heapDependents;
        }

        if(depTileCnts != NULL) {
            delete depTileCnts;
        }

        if(notifyList != NULL) {
            while(notifyList) {
                XiliNotifyEntry* nlist = notifyList->next;
                delete notifyList;
                notifyList = nlist;
            }
        }
    }

private:
    //
    //  Initialize the member variables...called from a constructor and the
    //  reinit() method.
    //
    void           initVars(XilDeferrableObject* def_obj,
                            unsigned int         num_tiles)
    {
        defobj          = def_obj;
        numTiles        = num_tiles;
        op              = NULL;
        opIsTossed      = FALSE;
        notifyList      = NULL;
        depTileCnts     = NULL;
        opStatusArray   = NULL;
        heapDependents  = NULL;
        dependents      = &stackDependents[0];
        maxDependents   = _XILI_MAX_STACK_DEPENDENTS;
        numDependents   = 0;
    }

    //
    //  Routine to create and initialize the data associated with the
    //  opStatusArray.
    //
    XilStatus      initOpStatusArray();

    //
    //  Class data
    //
    XilDeferrableObject*        defobj;
    unsigned int                numTiles;
    XilOp*                      op;
    Xil_boolean                 opIsTossed;
    unsigned int                referenceCount;
    XiliOpStatus*               opStatusArray;
    XiliOpStatus                singleTileOpStatus;

    //
    //  Variables for the handling of calling entries on source deferrable
    //  objects which have requested notification when tiles are evaluated.
    //
    XilStatus                   setNotifyEntry(XiliOpQueueEntry* entry,
                                               unsigned int      src_num);
    struct XiliNotifyEntry {
        XiliNotifyEntry*  next;

        XiliOpQueueEntry* entry;
        unsigned int      srcNum;
    };
    XiliNotifyEntry*            notifyList;

    //
    //  Variables for caching the results of forward mapping a tile into a
    //  dependent operation's destination for use when the dependent provides
    //  notification that one of the tiles have been evaluated.  We then check
    //  to see if all of the tiles dependent on us have been evaluated and if
    //  they have the storage associated with the tile in this object can be
    //  released.  We actually use counts since tiles are only evaluated
    //  once.
    //
    Xil_boolean                 allTilesNeeded;
    int*                        depTileCnts;

    //
    //  Notify tile tile_number that one of its dependent tiles has been
    //  evaluated.
    //
    void                        notify(XilTileNumber tnum);

    //
    //  Forward map an area in global space from this object into the given
    //  dependent operation's destination and fill in the given tile list.
    //
    XilStatus                   fwdMapRect(XiliRect*            src_rect,
                                           unsigned int         src_num,
                                           XilOp*               dependent,
                                           XilDeferrableObject* dst_obj,
                                           XilTileList*         dst_tile_list);

    //
    //  Translate the area in the creator op's destination
    //  (i.e. this entry's deferrable object) into the dependent's
    //  source -- leaving it in global space.
    //
    XilStatus                   translateOpDstToDepSrcGS(XilOp*               op,
                                                         XilOp*               dependent,
                                                         XilDeferrableObject* op_dst_obj,
                                                         XilDeferrableObject* dep_src_obj,
                                                         XiliRectInt*         op_dst_rect,
                                                         XiliRectInt*         dep_src_rect);

    //
    //  Dependent List
    //
    struct XiliDepInfo {
        XilOp*       op;
        unsigned int branch;
    };

    XiliDepInfo                 stackDependents[_XILI_MAX_STACK_DEPENDENTS];
    XiliDepInfo*                heapDependents;
    XiliDepInfo*                dependents;

    unsigned int                numDependents;
    unsigned int                maxDependents;
};

#endif  // XILI_OPQUEUE_ENTRY_HH
