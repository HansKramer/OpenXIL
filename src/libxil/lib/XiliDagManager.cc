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
//  File:	XiliDagManager.cc
//  Project:	XIL
//  Revision:	1.4
//  Last Mod:	10:08:39, 03/10/00
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
#pragma ident	"@(#)XiliDagManager.cc	1.4\t00/03/10  "

#include "XiliDagManager.hh"
#include "XiliDag.hh"

//
//  Put the new/delete overloads for XiliDag and XiliDagRef here.
//
//  Don't expect to have too many DAGs out at one time so we limit the maximum
//  number to 512.
//
_XIL_NEW_DELETE_OVERLOAD_CC_FILE(XiliDag, 512, 16)
_XIL_NEW_DELETE_OVERLOAD_CC_FILE(XiliDagRef, 2048, 64)

//
//  Get the next available unused DAG.
//
XiliDagRef*
XiliDagManager::getNewDAG(XilSystemState* system_state)
{
    //
    //  Create a new DAG with a new number.
    //
    XiliDag*    new_dag = new XiliDag(this, maxCount++);
    XiliDagRef* new_ref = new XiliDagRef(new_dag);

    if(new_dag == NULL || new_ref == NULL) {
        delete new_dag;
        delete new_ref;
        XIL_ERROR(system_state, XIL_ERROR_RESOURCE, "di-1", TRUE);
        return NULL;
    } else {
        return new_ref;
    }
}

//
//  Merge the second DAG into the first DAG -- generating a single DAG.
//
//  We take care here to lock the managerMutex which permits us to look at
//  the numbers stored in the XiliDag class.  Since the managerMutex is
//  global, only one thread can be inspecting the XiliDag numbers at one
//  time.  Then, we get the numbers and subsequently attempt to lock both
//  DAGs starting with the lower numbered DAG first.
//
XiliDagRef*
XiliDagManager::mergeDAGs(XiliDagRef* dagref1,
                          XiliDagRef* dagref2)
{
    XiliDag* dag1 = dagref1->getDag();
    XiliDag* dag2 = dagref2->getDag();

    //
    //  Check if both DAG references are pointing at the same DAG.
    //
    if(dag1 == dag2) {
        return dagref1;
    }

    Xil_unsigned64 dag1_num = dag1->dagNumber;
    Xil_unsigned64 dag2_num = dag2->dagNumber;

    //
    //  Lock the dags so we can manipulate them.
    //
    if(dag1_num < dag2_num) {
        dagref1->lockDag();

        //
        //  Verify the refs are still pointing at the same two dags, if not,
        //  return -- this is an expected condition. 
        //
        if(dagref1->getDag() != dag1 || dagref2->getDag() != dag2) {
            dagref1->unlockDag();
            return dagref1;
        }

        dagref2->lockDag();

        //
        //  Verify the refs are still pointing at the same two dags, if not,
        //  return -- this is an expected condition. 
        //
        if(dagref1->getDag() != dag1 || dagref2->getDag() != dag2) {
            dagref2->unlockDag();
            dagref1->unlockDag();
            return dagref1;
        }
    } else {
        dagref2->lockDag();

        //
        //  Verify the refs are still pointing at the same two dags, if not,
        //  return -- this is an expected condition. 
        //
        if(dagref1->getDag() != dag1 || dagref2->getDag() != dag2) {
            dagref2->unlockDag();
            return dagref1;
        }

        dagref1->lockDag();

        //
        //  Verify the refs are still pointing at the same two dags, if not,
        //  return -- this is an expected condition. 
        //
        if(dagref1->getDag() != dag1 || dagref2->getDag() != dag2) {
            dagref1->unlockDag();
            dagref2->unlockDag();
            return dagref1;
        }
    }

    //
    //  Change all of dag2 references to point at dag1.
    //
    //  The XiliDagRef class is smart enough to delete the dag when there
    //  are no more references to it by other XiliDagRefs so we don't
    //  worry about deleting dag2 here because its references will take
    //  care of deleting it once we've changed all of the references.
    //
    XiliBagIterator bag_it(&(dag2->dagRefBag));
    XiliDagRef*     tmp_ref;
    while(tmp_ref = (XiliDagRef*)bag_it.getNext()) {
        tmp_ref->setDag(dag1);
    }

    if(dag1_num < dag2_num) {
        dagref2->unlockDag();
        dagref1->unlockDag();
    } else {
        dagref1->unlockDag();
        dagref2->unlockDag();
    }

    return dagref1;
}
