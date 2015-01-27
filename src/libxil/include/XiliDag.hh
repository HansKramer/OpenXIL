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
//  File:	XiliDag.hh
//  Project:	XIL
//  Revision:	1.16
//  Last Mod:	10:21:33, 03/10/00
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
#pragma ident	"@(#)XiliDag.hh	1.16\t00/03/10  "

//
//  System Includes
//

//
//  C Includes
//
#include "_XilDefines.h"

//
//  C++ Includes
//
#include "_XilMutex.hh"
#include "XiliDagManager.hh"
#include "XiliBag.hh"

#include "XiliThread.hh"

#ifndef _XILI_DAG_HH
#define _XILI_DAG_HH

//
//  DAG-specific Types.
//
class XiliDagRef;
class XiliDag;

//
//  The XiliDag holds the mutex for an entire DAG.  It's referenced through
//  XiliDagRef objects that the real objects and ops reference.  Merges are
//  done through the DAG manager and locks are obtained through the
//  XiliDagRefs while holding a lock on the DAG manager.
//
class XiliDag {
private:
    //
    //  Befriend XiliDagRef and XiliDagManager so they can access private
    //  members directly.
    //
    friend class XiliDagRef;
    friend class XiliDagManager;

    XiliDag::XiliDag(XiliDagManager* dag_manager,
                     Xil_unsigned64  assigned_dag_number) {
        dagManager = dag_manager;
        dagNumber  = assigned_dag_number;
    }
    
    //
    //  The mutex for locking the DAG
    //
    XilMutex            dagMutex;

    //
    //  Pointer to the DAG manager that is responsible for us -- this is for
    //  signalling and waiting when a dagMutex cannot be obtained.
    //
    XiliDagManager*     dagManager;
    
    //
    //  The number assigned to this DAG
    //
    Xil_unsigned64      dagNumber;

    //
    //  Count of the ops in this DAG
    //
    unsigned int        numOps;

    //
    //  This bag keeps track of all of the XiliDagRef objects pointing to this
    //  DAG.  This is needed in order to perform merges because multiple DAG
    //  References may be pointing to the same DAG.
    //
    XiliBag             dagRefBag;

    //
    //  New/Delete overload to accelerate creation/destruction and default
    //  constructor that makes them work. 
    //
    _XIL_NEW_DELETE_OVERLOAD_PUBLIC(XiliDag)
    _XIL_NEW_DELETE_OVERLOAD_PRIVATE(XiliDag)
                        XiliDag() { }
};

//
//  Lock a Dag
//
//  To manipulate a DAG:
//
//      1.  Lock the dag manager
//      2.  Lock the dag (via XiliDagRef::lockDag())
//      3.  Unlock the dag manager
//      4.  Manipulate the dag
//      5.  Unlock the dag (via XiliDagRef::unlockDag())
//
//  To aquire a reference to a DAG or manipulate a reference to a DAG
//  (i.e. an XiliDagRef), the DAG manager mutex lock must be held.
//
class XiliDagRef {
public:
    XiliDag*            lockDag()
    {
        TNF_PROBE_3(dagref_lockdag, "dafref_refcnts", "dagref_lockdag",
                    tnf_opaque, "this", this,
                    tnf_opaque, "dag", dag,
                    tnf_long, "refcnt", refCount);

#ifdef DAG_DEBUG
        fprintf(stderr, "%02d: dag locking %p : %p\n", XiliThread::self(), this, dag);
        fflush(stderr);
#endif

        //
        //  Try to obtain a lock on the DAG.  If we can't, we go to sleep
        //  waiting for a signal that a DAG has been unlocked and we try
        //  again.
        //
        while(dag->dagMutex.tryLock() == XIL_FAILURE) {
#ifdef DAG_DEBUG
            fprintf(stderr, "%02d: dag waiting %p : %p\n",
                    XiliThread::self(), this, dag);
            fflush(stderr);
#endif

            //
            //  Since the caller of this routine needed to obtain a mutex on
            //  the DAG manager, we know it's locked and the dag manger knows
            //  which lock to use.
            //
            dag->dagManager->waitForDagUnlock();
        }

#ifdef DAG_DEBUG
        fprintf(stderr, "%02d: dag locked %p : %p\n", XiliThread::self(), this, dag);
        fflush(stderr);
#endif

        return dag;
    }

    XiliDag*            trylockDag()
    {
        TNF_PROBE_3(dagref_lockdag, "dafref_refcnts", "dagref_trylockdag",
                    tnf_opaque, "this", this,
                    tnf_opaque, "dag", dag,
                    tnf_long, "refcnt", refCount);

#ifdef DAG_DEBUG
        fprintf(stderr, "%02d: dag trylock %p : %p\n", XiliThread::self(), this, dag);
        fflush(stderr);
#endif

        if(dag->dagMutex.tryLock() == XIL_FAILURE) {
            return NULL;
        } else {
            return dag;
        }
    }

    void                unlockDag()
    {
        TNF_PROBE_3(dagref_unlockdag, "dafref_refcnts", "dagref_unlockdag",
                    tnf_opaque, "this", this,
                    tnf_opaque, "dag", dag,
                    tnf_long, "refcnt", refCount);

#ifdef DAG_DEBUG
        fprintf(stderr, "%02d: dag UNlocking %p : %p\n", XiliThread::self(), this, dag);
        fflush(stderr);
#endif

        //
        //  The DAG manager lock is supposed to be held at this point so we
        //  can signal our unlock.
        //
        dag->dagMutex.unlock();

        dag->dagManager->signalUnlock();

#ifdef DAG_DEBUG
        fprintf(stderr, "%02d: dag UNlocked %p : %p\n", XiliThread::self(), this, dag);
        fflush(stderr);
#endif
    }

    //
    //  Aquire a reference to this object.  This must be called in order to
    //  store a reference to the object.
    //
    XiliDagRef*         aquire()
    {
        refCount++;
        
#ifdef DAG_DEBUG
        fprintf(stderr, "%02d: aquire ref %d on dag ref %p\n",
                XiliThread::self(), refCount, this);
        fflush(stderr);
#endif

        TNF_PROBE_2(dagref_aquire, "dagref_refcnts", "dagref_aquire",
                    tnf_opaque, "this", this,
                    tnf_long, "refcnt", refCount);

        return this;
    }

    void                release()
    {
        if(this) {
            TNF_PROBE_2(dagref_release, "dagref_refcnts", "dagref_release",
                        tnf_opaque, "this", this,
                        tnf_long, "refcnt", refCount - 1);

#ifdef DAG_DEBUG
            fprintf(stderr, "%02d: release ref %d on dag ref %p\n",
                    XiliThread::self(), refCount, this);
            fflush(stderr);
#endif

            if(--refCount == 0) {
                delete this;
            }
        }
    }

    Xil_boolean         isEqual(XiliDagRef* other_dag_ref)
    {
        return dag == other_dag_ref->dag;
    }

private:
    //
    //  New/Delete overload to accelerate creation/destruction and default
    //  constructor that makes them work. 
    //
    _XIL_NEW_DELETE_OVERLOAD_PUBLIC(XiliDagRef)
    _XIL_NEW_DELETE_OVERLOAD_PRIVATE(XiliDagRef)
    
                        XiliDagRef() { }
        
    //
    //  Replace the current dag referenced by this object with the new dag.
    //
    //  This includes removing this reference from the list of references kept
    //  with the DAG.  In addition, we must add ourselves to the list of
    //  references kept with the new DAG.
    //
    void                setDag(XiliDag* new_dag)
    {
#ifdef DAG_DEBUG
            fprintf(stderr, "%02d: setting DAG on %p from %p to %p\n",
                    XiliThread::self(), this, dag, new_dag);
#endif

        if(dag == new_dag) {
            //
            //  No work needs to be done, we're already referencing it.
            //
        } else if(dag == NULL) {
            //
            //  DAG isn't set so just set it.
            //
            dag = new_dag;

            dag->dagRefBag.insert(this);
        } else {
            dag->dagRefBag.remove(this);

            if(dag->dagRefBag.isEmpty()) {
                //
                //  Since no one is referencing the dag, go ahead and delete it.
                //
                delete dag;
            }
            
            new_dag->dagRefBag.insert(this);
            
            dag = new_dag;
        }
    }
    
    XiliDag*            getDag() const
    {
        return dag;
    }

    //
    //  Constructing an XiliDagRef indicates someone references it.
    //
                        XiliDagRef(XiliDag* initial_reference)
    {
        //
        //  Indicate that we don't have a reference yet.
        //
        refCount = 1;
        dag      = NULL;
        
        setDag(initial_reference);
    }

                        ~XiliDagRef()
    {
        if(dag != NULL) {
#ifdef DAG_DEBUG
            fprintf(stderr, "%02d: removing dag ref %p from dag %p\n",
                    XiliThread::self(), this, dag);
#endif
            dag->dagRefBag.remove(this);

            if(dag->dagRefBag.isEmpty()) {
                //
                //  Since no one is referencing the dag, go ahead and delete it.
                //
#ifdef DAG_DEBUG
                fprintf(stderr, "%02d: deleting dag %p\n", XiliThread::self(), dag);
                fflush(stderr);
#endif
                delete dag;
            }
        }
    }

    
    //
    //  Befriend XiliDagManager so it can access private members directly to
    //  set and create new DAG references.
    //
    friend class XiliDagManager;
    
    //
    //  The dag is a pointer to the table entry that actually
    //    contains the information for this DAG.  This extra level of
    //    indirection permits the DAG to which many objects belong to change
    //    while only updating a single table entry.
    //
    //  So, all of the objects belonging to this DAG are pointed at this
    //    entry.  Then, this entry contains a reference to the "real" DAG
    //    which may be itself.  When a merge occurs, this entry is modified to
    //    point to the merged result.  Thus, all of the lookups to determine
    //    which DAG an object actually resides in uses the DAG contained in
    //    this pointer and not this entry.
    //
    //  We always keep it so there is only a single level of indirection to
    //    the real DAG.  So, there is no need to loop or recurse to find the
    //    real DAG.
    //
    XiliDag*            dag;

    //
    //  Count the number of objects referencing this DagRef.
    //
    unsigned int        refCount;
};

#endif //  _XILI_DAG_HH
