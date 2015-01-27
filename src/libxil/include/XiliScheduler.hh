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
//  File:	XiliScheduler.hh
//  Project:	XIL
//  Revision:	1.39
//  Last Mod:	10:21:09, 03/10/00
//
//  Description:
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
#pragma ident	"@(#)XiliScheduler.hh	1.39\t00/03/10  "

#include "_XilClasses.hh"
#include "_XilMutex.hh"
#include "_XilCondVar.hh"

#include "_XilBoxList.hh"

#include "XiliThread.hh"
#include "XiliOpTreeNode.hh"
#include "XiliList.hh"
#include "XiliSLList.hh"

#ifndef _XILI_SCHEDULER_HH
#define _XILI_SCHEDULER_HH

//
//  Forward class declarations
//
class XiliScheduler;
class XiliExecContext;

//
//  Operation description
//
class XiliOperation {
public:
    XiliSLList<XilTileNumber>*  getEmptyFlushList()
    {
        return emptyFlushList;
    }

                                XiliOperation(XilOp*          new_op,
                                              unsigned int    op_length,
                                              XiliOpTreeNode* node,
                                              XilTileList*    tile_list,
                                              XilRoi*         new_roi)
    {
        emptyFlushList    = NULL;
        operationComplete = FALSE;
        status            = XIL_SUCCESS;

        op                = new_op;
        functionLength    = op_length;
        opTreeNode        = node;
        tileList          = tile_list;
        roi               = new_roi;
    }

                                XiliOperation()
    {
        emptyFlushList    = NULL;
        operationComplete = FALSE;
        status            = XIL_SUCCESS;

        op                = NULL;
        functionLength    = 0;
        opTreeNode        = NULL;
        tileList          = NULL;
        roi               = NULL;
    }
                                   
                                ~XiliOperation()
    {
        delete emptyFlushList;
    }

#if defined(GCC) || defined(_WINDOWS) || defined(HPUX)
    //
    //  For placating explicit template instantiation.
    //
    int operator == (XiliOperation&) {
        return TRUE;
    }
#endif
    
private:
    void                        lock()
    {
        mutex.lock();
    }

    void                        unlock()
    {
        mutex.unlock();
    }

    void                        addEmptyFlushTile(XilTileNumber tile_number)
    {
        if(emptyFlushList == NULL) {
            emptyFlushList =
                new XiliSLList<XilTileNumber>(op->getSystemState());
        }

        if(emptyFlushList != NULL) {
            emptyFlushList->append(tile_number);
        } else {
            XIL_ERROR(op->getSystemState(), XIL_ERROR_RESOURCE, "di-1", TRUE);
        }
    }

    XiliOpTreeNode*             opTreeNode;
    unsigned int                functionLength;

    XilOp*                      op;
    XilRoi*                     roi;
    XilTileList*                tileList;
    XiliSLList<XilTileNumber>*  emptyFlushList;

    XilMutex                    mutex;
    unsigned int                operationNumber;
    XilStatus                   status;
    XilCondVar                  completeCond;
    Xil_boolean                 operationComplete;
    XiliList<XiliExecContext>   contextList;
 
    XiliScheduler*              scheduler;

    friend class XiliScheduler;
    friend class XiliExecContext;

    friend void* xili_start_compute_routine(void*);
};

//
//  Execution Context
//
class XiliExecContext {
public:
                      XiliExecContext(XiliOperation* init_op       = NULL,
                                      Xil_boolean    do_sleep_init = TRUE) :
                          boxList(NULL,
                                  init_op->op->getNumSrcs(),
                                  init_op->op->getNumDsts()),
                          sleepUntilDone(do_sleep_init)
    {
        operation    = init_op;
        scheduler    = operation->scheduler;
        listPosition = _XILI_LIST_INVALID_POSITION;
        isDone       = FALSE;

        boxList.setSystemState(init_op->op->getSystemState());
    }

#if defined(GCC) || defined(_WINDOWS) || defined(HPUX)
    //
    //  For placating explicit template instantiation.
    //
    int operator == (XiliExecContext&) {
        return TRUE;
    }
#endif

    XilBoxList        boxList;
    XiliOperation*    operation;
    XiliThread*       thread;
    XilCondVar        isReady;
    XilMutex          readyMutex;
    XiliScheduler*    scheduler;
    XilStatus         status;
    XiliListPosition  listPosition;
    Xil_boolean       sleepUntilDone;
    Xil_boolean       isDone;
};

class XiliScheduler {
public:
    //
    //  Process the given operation which is described by XiliOperation.  The
    //  execute() call returns only after the given operation has completed.
    //
    XilStatus                    execute(XiliOperation* operation);
    XilStatus                    executeWithNoSplit(XiliOperation* operation);
    XilStatus                    callPreprocess(XiliOperation*   operation,
                                                XiliFunctionDef* func_def);

    //
    //  Constructor/Destructor -- called by XilGlobalState
    //
                                 XiliScheduler();
                                 ~XiliScheduler();

private:
    void                         startComputeOperation(XiliExecContext* context);
    XilStatus                    startThreads(XiliOperation* operation,
                                              XiliRect*      gsbbox,
                                              XilTileNumber  tnum,
                                              Xil_boolean*   threads_started);
    XilStatus                    execOperation(XiliOperation* operation,
                                               XiliRect*      bbox);
    void                         recoverFailure(XiliOperation* operation);

    XilMutex                     queueMutex;
    XilCondVar                   queueNotEmpty;
    XiliList<XiliOperation>      queue;

    XiliSLList<XiliExecContext*> readyList;
    XilMutex                     readyListMutex;

    XiliThread*                  threadReaper;

    unsigned int                 numCPUs;

    Xil_boolean                  noThreads;
    Xil_boolean                  threadSplit;
    unsigned int                 splitThreshold;

    friend void* xili_start_compute_routine(void*);
};

#endif // _XILI_SCHEDULER_HH
