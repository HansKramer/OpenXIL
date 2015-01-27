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
//  File:	XilNonDeferrableObject.cc
//  Project:	XIL
//  Revision:	1.14
//  Last Mod:	10:08:34, 03/10/00
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
//  MT-level:  UNSAFE
//	
//------------------------------------------------------------------------
//	COPYRIGHT
//------------------------------------------------------------------------
#pragma ident	"@(#)XilNonDeferrableObject.cc	1.14\t00/03/10  "

//
//  C++ Includes
//
#include "_XilDefines.h"
#include "_XilGlobalState.hh"
#include "_XilNonDeferrableObject.hh"
#include "_XilDeferrableObject.hh"
#include "_XilOp.hh"
#include "XiliDagManager.hh"

//
//  For XiliDependentInfo...
//
_XIL_NEW_DELETE_OVERLOAD_CC_FILE(XiliDependentInfo, 512, 32)

Xil_boolean
XilNonDeferrableObject::preDestroy()
{
    if(!dependents.isEmpty()) {
        //
        //  Here, we call newVersion() to make a copy of ourselves and
        //  give it to those who are dependent on us.
        //
        newVersion();

        //
        //  Another option is to mark this object as "destroy when zero
        //  dependents" in ::destroy() which will cause the release
        //  routines to actually delete the object.  It's not clear that
        //  the added complexity is worth it.
        //
    }

    return TRUE;
}

    
//
//  These return a reference to this object which can be attached to the
//  given operation or deferrable object.
//
//  If this object is modified while a reference is set, a copy is made of
//  the object and all of the objects dependent on this version of the
//  object are called to update their reference to the copy.
//
//  This means these objects do not require copying when they're deferred.
//  For some objects, this can be a huge time savings (64k XIL_SHORT
//  lookups for example).
//
XilNonDeferrableObject*
XilNonDeferrableObject::aquireDefRef(XilOp* op)
{
    XiliDependentInfo* depinfo = new XiliDependentInfo(op, NULL);

    if(depinfo == NULL) {
        XIL_OBJ_ERROR(getSystemState(), XIL_ERROR_RESOURCE,
                      "di-1", TRUE, this);
        return NULL;
    }

    dependents.insert(depinfo);

    return this;
}

XilNonDeferrableObject*
XilNonDeferrableObject::aquireDefRef(XilDeferrableObject* defobj)
{
    XiliDependentInfo* depinfo = new XiliDependentInfo(NULL, defobj);

    if(depinfo == NULL) {
        XIL_OBJ_ERROR(getSystemState(), XIL_ERROR_RESOURCE,
                      "di-1", TRUE, this);
        return NULL;
    }

    dependents.insert(depinfo);

    return this;
}

//
//  Remove the given deferred reference.  The deferred object no longer
//  needs this object.
//
XilStatus
XilNonDeferrableObject::releaseDefRef(XilOp* op)
{
    XiliBagIterator bi(&dependents);

    XiliDependentInfo* depinfo;
    while(depinfo = (XiliDependentInfo*)bi.getNext()) {
        if(depinfo->op == op) {
            dependents.remove(depinfo);

            delete depinfo;

            if(dependents.isEmpty()) {
                //
                //  Clear out our dag reference.
                //
                XiliDagManager* dm = XilGlobalState::theXGS->getDagManager();
                dm->lock();
                setDagRef(NULL);
                dm->unlock();
            }

            return XIL_SUCCESS;
        }
    }

    return XIL_FAILURE;
}

XilStatus
XilNonDeferrableObject::releaseDefRef(XilDeferrableObject* defobj)
{
    XiliBagIterator bi(&dependents);

    XiliDependentInfo* depinfo;
    while(depinfo = (XiliDependentInfo*)bi.getNext()) {
        if(depinfo->defobj == defobj) {
            dependents.remove(depinfo);

            delete depinfo;

            if(dependents.isEmpty()) {
                //
                //  Clear out our dag reference.
                //
                XiliDagManager* dm = XilGlobalState::theXGS->getDagManager();
                dm->lock();
                setDagRef(NULL);
                dm->unlock();
            }

            return XIL_SUCCESS;
        }
    }

    return XIL_FAILURE;
}

//
//  If we're part of a DAG, then we're expected to have our dagRef set so that
//  locking this object locks the DAG -- which permits us to flush the DAG.
//
void
XilNonDeferrableObject::newVersion()
{
    //
    //  If we have any dependents on the current version of this object, we
    //    need to make a copy of the object and have our dependents point to
    //    the new lookup object.
    //
    if(!dependents.isEmpty()) {
        XilObject*              obj_copy = createCopy();
        XilNonDeferrableObject* copy     =
            (XilNonDeferrableObject*)obj_copy;

        if(copy == NULL) {
            //
            //  If the copy failed, then we're either not allowed to copy it
            //  or there is another resource problem.  Either way, since we
            //  can't copy it, we must flush our op dependencies.  We can't do
            //  anything about our deferrable object dependencies -- so if we
            //  have one, it causes an error and failure condition.
            //
            //  Run through our dependents list and flush the ones we can.
            //
            XiliBagIterator    bi(&dependents);

            XiliDependentInfo* depinfo;
            Xil_boolean        generate_error = FALSE;
            while(depinfo = (XiliDependentInfo*)bi.getNext()) {
                if(depinfo->op != NULL) {
                    depinfo->op->flush();
                } else {
                    generate_error = TRUE;
                }

                //
                //  Delete the dependency info class for this object.  Now
                //  they reference the copy so nobody depends on us any more.
                //  We clear the bag of its references below in one step
                //  instead of removing them one-at-a-time. 
                //
                delete depinfo;
            }

            if(generate_error) {
                //
                //
                //  TODO:  3/1/96 jlf  If this occurs, we have a big problem.
                //                     Can it be fixed better than this? 
                //
                //
                XIL_OBJ_ERROR(getSystemState(), XIL_ERROR_INTERNAL,
                              "di-399", FALSE, this);
            }
        } else {
            //
            //  Run through our dependents list and ask each of our dependents
            //  to use the copy.
            //
            XiliBagIterator    bi(&dependents);

            XiliDependentInfo* depinfo;
            while(depinfo = (XiliDependentInfo*)bi.getNext()) {
                if(depinfo->op != NULL) {
                    depinfo->op->updateObjectReferences(this, copy, TRUE);
                } else {
                    depinfo->defobj->updateObjectReferences(this, copy, TRUE);
                }

                //
                //  Delete the dependency info class for this object.  Now
                //  they reference the copy so nobody depends on us any more.
                //  We clear the bag of its references below in one step
                //  instead of removing them one-at-a-time. 
                //
                delete depinfo;
            }
        }

        dependents.clear();

        //
        //  TODO: 5/6/96 jlf  clear DAG ref
        //
    }

    XilObject::newVersion();
}
