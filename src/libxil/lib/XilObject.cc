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
//  File:	XilObject.cc
//  Project:	XIL
//  Revision:	1.45
//  Last Mod:	10:07:59, 03/10/00
//
//  Description:
//	
//	
//------------------------------------------------------------------------
//	COPYRIGHT
//------------------------------------------------------------------------
#pragma ident	"@(#)XilObject.cc	1.45\t00/03/10  "

//
//  System Includes
//
#include <string.h>
#include <stdlib.h>

//
//  XIL Includes
//
#include "_XilDefines.h"
#include "_XilObject.hh"
#include "_XilGlobalState.hh"
#include "_XilSystemState.hh"
#include "XiliDag.hh"

//
//  The XilVersion Constructor/Destructor
//
XilVersion::XilVersion()
{
    objectId      = 0;
    versionNumber = 0;

    infoId        = 0;
    infoVersion   = 0;
}

//
//  The statics for the object identifier.  Note that the objectIdCounter is
//  initialized to 1.  By definition, the objectId is always non-zero.
//
XilMutex    XilObject::objectIdCounterMutex;
XilObjectId XilObject::objectIdCounter       = 1;

//
//  The XilObject Constructor
//
XilObject::XilObject(XilSystemState*  system_state,
                     XilObjectType    object_type) :
    objectId(getNewObjectId()),
    objectType(object_type)
{
    //
    //  Initialize our data elements.
    //
    systemState                 = system_state;
    objectName                  = NULL;
    errorString                 = NULL;
    validState                  = TRUE;

    //
    //  Initialize our version information.
    //
    //  We always start the version information at non-zero.  Zero indicates
    //  an invalid or uninitialized version number.
    //
    objectVersion.versionNumber = 1;
    objectVersion.objectId      = objectId;

    //
    //  At this point, the info version information is "invalid" in that we
    //  represent ourself.  A call to copyVersionInfo() would flip this until
    //  newVersion() is called.  At that point, the info version information
    //  reverts to "invalid."
    //
    objectVersion.infoId        = 0;
    objectVersion.infoVersion   = 0;

    //
    //  Initially, an object doesn't belong to a DAG.
    //
    dagRef                      = NULL;
    dagRefCnt                   = 0;
    oldDagRef                   = NULL;
}

XilObject::~XilObject()
{
    //
    //  objectName and errorString are created by strdup().
    //
    //  TODO: 6/21/96 jlf   Although free() is defined to be able to take a
    //                      NULL pointer, even if the pointer is NULL, free()
    //                      still locks and unlocks a static mutex which makes
    //                      it a heavy call to make -- even with a NULL
    //                      pointer.
    //
    if(objectName != NULL) {
        //
        //  If we were put into the system state's named hash table, then we
        //  need remove us from the table.
        //
        systemState->removeNamedObject(objectName, this);
        free(objectName);
    }

    if(errorString != NULL) {
        free(errorString);
    }

    if(dagRef != NULL) {
        dagRef->unlockDag();
        dagRef->release();
    }

    if(oldDagRef != NULL) {
        if(oldDagRef != dagRef) {
            oldDagRef->unlockDag();
        }
        oldDagRef->release();
    }

}

Xil_boolean
XilObject::isOK()
{
    _XIL_ISOK_TEST();
}

//
//  The static member function that aquires a new object id for newly
//  constructed objects.
//
XilObjectId
XilObject::getNewObjectId()
{
    objectIdCounterMutex.lock();
    
    XilObjectId ret_val = objectIdCounter++;
    
    objectIdCounterMutex.unlock();

    return ret_val;
}

//------------------------------------------------------------------------
//
//  Function:	getName()/setName()
//
//  Description:
//	
//	Handles associating a name with this object.  These routines make a
//      copy of the name when storing and when returning the name.
//	
//  MT-level:   SAFE
//
//------------------------------------------------------------------------
char*
XilObject::getName()
{
    if(objectName != NULL) {
        char* copy = strdup(objectName);
        if(copy == NULL) {
            XIL_ERROR(this->getSystemState(),
                      XIL_ERROR_RESOURCE, "di-1", TRUE);
        }

        return copy;
    }

    return NULL;
}

void
XilObject::setName(const char* new_name)
{
    if(new_name != NULL) {
        //
        //  Remove ourself from the named object list kept in the system state.
        //
        if(systemState != NULL && objectName != NULL) {
            systemState->removeNamedObject(objectName, this);
        }

        objectName = strdup(new_name);
        
        if(objectName==NULL) {
            XIL_ERROR(this->getSystemState(), XIL_ERROR_RESOURCE,"di-1",TRUE);
            XIL_ERROR(this->getSystemState(), XIL_ERROR_SYSTEM,"di-279",FALSE);
            return;
        }

        //
        //  Add ourself to the named object list kept in the system state.
        //
        if(systemState != NULL) {
            systemState->addNamedObject(objectName, this);
        }
    } else {
        //
        //  Remove ourself from the named object list kept in the system state.
        //
        if(systemState != NULL && objectName != NULL) {
            systemState->removeNamedObject(objectName, this);
        }

        free(objectName);

        objectName = NULL;
    }
}

XilObjectType
XilObject::getType()
{
    return objectType;
}


XilObjectId
XilObject::getObjectId()
{
    return objectId;
}

XilObject*
XilObject::getCopy()
{
    return createCopy();
}

//------------------------------------------------------------------------
//
//  Function:	setErrorString()/getErrorString()
//
//  Description:
//	Set the error string or get the error string.  Uses
//      vSetErrorString() and vGetErrorString() to actually do
//	the modifications.  vSetErrorString() and vGetErrorString()
//      are not expected to be MT-SAFE.
//	
//  MT-level:  SAFE
//	
//------------------------------------------------------------------------
void
XilObject::setErrorString(const char* error_string)
{
    vSetErrorString(error_string);
}

void
XilObject::getErrorString(char*        string_storage,
                          unsigned int string_size)
{
    vGetErrorString(string_storage, string_size);
}

void
XilObject::vSetErrorString(const char* error_string)
{
    if(errorString != NULL) {
        free(errorString);
    }

    if(error_string == NULL) {
        errorString = NULL;
    } else {
        errorString = strdup(error_string);
    }
}

void
XilObject::vGetErrorString(char*        string_storage,
                           unsigned int string_size)
{
    if(string_storage == NULL) {
        XIL_OBJ_ERROR(getSystemState(), XIL_ERROR_USER, "di-259", TRUE, this);
        return;
    }

    if(errorString == NULL) {
        string_storage[0] = '\0';
    } else {
        strncpy(string_storage, errorString, string_size);
    }
}

//
//  NEW VERSIONING...
//
//  The version information is kept in the XilVersion class.  It is made
//  up of an object id (which identifies which object this is) and a
//  version number (which identifies which version of the object it is).
//
//  Testing whether two objects are the same is done via the isSameAs()
//  method which will test whether the *information* represented by this
//  object is the same as what was represented by the given XilVersion
//  information.  It is possible that two distinct objects represent the
//  same information.  When an object is copied, its object identifier is
//  updated (since it is a uniquely new object) and it's version number is
//  reset to an initial state.  But, the information contained in the
//  object is the same as the original object.  The version information is
//  kept so that the new object can determine if it represents the same
//  infomation as the object described by the given XilVersion.
//
void
XilObject::getVersion(XilVersion* ret_version)
{
    if(this == NULL) {
        XIL_ERROR(NULL, XIL_ERROR_USER, "di-268", TRUE);
        // Return a zeroed XilVersion
        ret_version->objectId      = (Xil_unsigned64) 0;
        ret_version->versionNumber = (Xil_unsigned64) 0;

        return;
    }

    *ret_version = objectVersion;
}

Xil_boolean
XilObject::isSameAs(XilVersion* other_version)
{
    //
    //  Check to see if we're storing information that was copied from another
    //  object.
    //
    if(objectVersion.infoId == 0) {
        //
        //  We're not keeping information copied from another object.  So,
        //  test to see if the our version information matches that of either
        //  the other object's version id or the information represented by
        //  that object.
        //
        if((objectVersion.objectId == other_version->objectId ||
            objectVersion.objectId == other_version->infoId) &&
           (objectVersion.versionNumber == other_version->versionNumber ||
            objectVersion.versionNumber == other_version->infoVersion)) {

            return TRUE;
        }
    } else {
        //
        //  We're keeping information that was copied from another object.
        //  So, test to see if the version of information we're representing
        //  matches that of either the other object's version id or the
        //  information represented by that object.
        //
        if((objectVersion.infoId == other_version->objectId ||
            objectVersion.infoId == other_version->infoId) &&
           (objectVersion.infoVersion == other_version->versionNumber ||
            objectVersion.infoVersion == other_version->infoVersion)) {

            return TRUE;
        }
    }

    return FALSE;
}

void
XilObject::newVersion()
{
    //
    //  Increment the versionNumber and clear the infoId to an invalid value
    //  (by definition) to indicate we're representing our own information
    //  now.
    //
    objectVersion.versionNumber++;

    objectVersion.infoId = 0;
}

void
XilObject::copyVersionInfo(XilObject* object)
{
    if(object->objectVersion.infoId == 0) {
        objectVersion.infoId      = object->objectVersion.objectId;
        objectVersion.infoVersion = object->objectVersion.versionNumber;
    } else {
        objectVersion.infoId      = object->objectVersion.infoId;
        objectVersion.infoVersion = object->objectVersion.infoVersion;
    }        
}


//
//  It is NOT ok for this method to be called on a NULL XilObject pointer.
//
XilSystemState*
XilObject::getSystemState()
{
    return systemState;
}

XilStatus
XilObject::setSystemState(XilSystemState* state)
{
    if(systemState != NULL || state == NULL) {
        return XIL_FAILURE;
    }

    systemState = state;

    return XIL_SUCCESS;
}

//
//  Instead of deleting objects directly, this is the method that
//  actually deletes an object.  This controls the destruction so
//  reference counts could be used if desired.  We go ahead and call
//  vDestroy() 
//
void
XilObject::destroy()
{
    //
    //  If it's not NULL and our derived object agrees, delete it.
    //
    if(this != NULL) {
        if(preDestroy()) {
            delete this;
        } else {
            //
            //  Unlock any locks since deleting the object causes the mutexes
            //  to be destroyed we need to ensure the locks are not held after
            //  destruction... 
            //
            unlock();
        }
    }
}

//
//  By default, the preDestroy() function just returns TRUE.
//
Xil_boolean
XilObject::preDestroy()
{
    return TRUE;
}

Xil_boolean
XilObject::isValid()
{
    Xil_boolean ret_valid = validState;

    return ret_valid;
}

void
XilObject::setValid(Xil_boolean valid)
{
    validState = valid;
}

//
//  Methods that lock and unlock this object.
//
void
XilObject::lock()
{
    //
    //  To lock a DAG, we obtain a lock on the DAG manager and then lock the
    //  DAG.  After locking a DAG, we need to ensure that it still matches the
    //  one this object uses since it may have changed or been cleared while
    //  we were waiting to lock the DAG.
    //
    XiliDagManager* dm = XilGlobalState::theXGS->getDagManager();

    dm->lock();

    XiliDagRef* ref = dagRef;

    if(ref != NULL) {
        while(1) {
            ref = ref->aquire();

            ref->lockDag();

            if(dagRef == NULL) {
                //
                //  The dag ref was cleared...
                //
                objectMutex.lock();

                ref->unlockDag();
                ref->release();
                    
                dm->unlock();
                break;
            } else if(! ref->isEqual(dagRef)) {
                //
                //  The dag ref changed...
                //
                ref->unlockDag();
                ref->release();
                ref = dagRef;
            } else {
                dm->unlock();
                oldDagRef = ref;
                break;
            }
        }
    } else {
        dm->unlock();

        objectMutex.lock();
    }
}

XilStatus
XilObject::trylock()
{
    //
    //  Check the dag ref (the dag ref lock is always held for a very short
    //  period of time since it just pretects the changing of the dag ref.
    //
    XiliDagRef* ref = dagRef;

    if(ref != NULL) {
        if(ref->trylockDag() == NULL) {
            return XIL_FAILURE;
        } else {
            return XIL_SUCCESS;
        }
    } else {
        return objectMutex.tryLock();
    }
}

void
XilObject::unlock()
{
    //
    //  To unlock a DAG, we obtain a lock on the DAG manager and then unlock
    //  the DAG.
    //
    XiliDagManager* dm = XilGlobalState::theXGS->getDagManager();

    dm->lock();

    XiliDagRef* ref = dagRef;

    if(ref != NULL) {
        ref->unlockDag();
    }

    if(oldDagRef != NULL) {
        if(oldDagRef != ref) {
            oldDagRef->unlockDag();
        }

        oldDagRef->release();
        oldDagRef = NULL;
    }

    dm->unlock();

    //
    //  Always unlock the objectMutex because the dagRef could have been
    //  changed since when lock() was called.  There is no problem in doing
    //  this since unlocking an non-locked mutex is a noop.
    //
    objectMutex.unlock();
}

//
//  Callers to set/getDagRef must be holding the DAG manager lock.
//
void
XilObject::setDagRef(XiliDagRef* new_ref)
{
    if(new_ref == NULL) {
        if(--dagRefCnt == 0) {
            if(dagRef != NULL) {
                dagRef->release();
            }
            dagRef = NULL;
        }
    } else {
        //
        //  Release any reference we have to the DAG before aquiring a new one.
        //
        if(dagRef != NULL) {
            dagRef->release();
        }

        //
        //  Whomever is providing the dag ref is expected to have aquired the
        //  reference in order to give it to us.
        //
        dagRef    = new_ref;
        dagRefCnt = 1;
    }
}

//
//  Routine to get the DAG Reference for this object.
//
XiliDagRef*
XilObject::getDagRef()
{
    dagRefCnt++;

    return dagRef;
}
