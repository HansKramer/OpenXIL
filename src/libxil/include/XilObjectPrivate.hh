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
//  File:	XilObjectPrivate.hh
//  Project:	XIL
//  Revision:	1.16
//  Last Mod:	10:22:02, 03/10/00
//
//  Description:
//	
//	
//	
//	
//	
//	
//  MT-level:  <??????>
//	
//------------------------------------------------------------------------
//	COPYRIGHT
//------------------------------------------------------------------------
#pragma ident	"@(#)XilObjectPrivate.hh	1.16\t00/03/10  "

//
//  INCLUDE Portion of private header file
//
#ifdef _XIL_PRIVATE_INCLUDES

//
//  Forward declarations.
//
class XiliDagRef;

#include "_XilMutex.hh"

#endif // _XIL_PRIVATE_INCLUDES

#ifdef _XIL_PRIVATE_DATA
public:
    //
    //  Create a copy of the object...
    //
    virtual XilObject*         createCopy() = 0;

    //------------------------------------------------------------------------
    //
    //  Primitives to lock this object and all objects that are dependent on
    //  the object.  Basically, this will lock the DAG when a dagRef exists
    //  for the object and lock a mutex in the object when the object isn't
    //  in a DAG.
    //
    //  These locks are NOT recursive within a thread.
    //
    virtual void               lock();
    virtual XilStatus          trylock();

    virtual void               unlock();
    //
    //------------------------------------------------------------------------

    //------------------------------------------------------------------------
    //
    //  Methods for managing our references to the DAG to which we belong.
    //
    //  Sets the DAG Reference which this object uses to determine to
    //  which DAG it belongs.  It can only be called while the DAG Manager
    //  is locked by the caller.
    //
    virtual void                setDagRef(XiliDagRef* new_ref);

    //
    //  Routine to get the DAG Reference for this object.
    //
    virtual XiliDagRef*         getDagRef();
    //
    //------------------------------------------------------------------------

    //------------------------------------------------------------------------
    //
    //  Increments this object's version number.  It will also invalidate the
    //  fact that this object is storing information obtained from another
    //  object.
    //
    //  IMPORTANT:  This routine should be called any time any *information*
    //              contained in an object changes or even if it may have
    //              changed.  AND -- UNLIKE XIL 1.2 -- it should be called
    //              *PRIOR* to changing the information.
    //
    virtual void               newVersion();
    //
    //------------------------------------------------------------------------

    //------------------------------------------------------------------------
    //
    //  Many objects need the ability to intercept the destruction of the
    //  object prior to the destructor being called.  The destroy() function
    //  will take care of deleting the object after the preDestroy() if it
    //  returns TRUE.  Otherwise, we don't delete it and assume that the 
    //  derived object will take care of deleting it at a later date.
    //
    virtual Xil_boolean        preDestroy();
    //
    //------------------------------------------------------------------------

    //------------------------------------------------------------------------
    //
    //  Return a reference to our name instead of a copy.
    //
    //------------------------------------------------------------------------
    const char*                referenceName()
    {
        this->lock();

        const char* ret_val = objectName;

        this->unlock();

        return ret_val;
    }

    //------------------------------------------------------------------------
    //
    //  Sets the system state on the object.  This function is needed by
    //  objects that are created on the stack inside the XIL library or are
    //  members of another class.
    //
    //  IMPORTANT:  The system state cannot be changed, it can only be
    //              initialized by this routine.  So, it must be NULL in order
    //              for this function to succeed.
    //
    //  IMPORTANT:  This function is NOT MT-SAFE.
    //
    //  IMPORTANT:  If a setName() has been done on the object prior to this
    //              call, then this object will never have been added to the
    //              system state's named list.
    //
    XilStatus                  setSystemState(XilSystemState* state);
    //
    //------------------------------------------------------------------------

    //------------------------------------------------------------------------
    //
    //  Set the object's valid state flag.
    //
    void                       setValid(Xil_boolean valid);
    //
    //------------------------------------------------------------------------


protected:
    //
    //  Constructor
    //
                               XilObject(XilSystemState*  system_state,
                                         XilObjectType    object_type);

    //
    //  Destructor
    //
    virtual                    ~XilObject();

    //
    //  Copies the version information from the given object.
    //
    virtual void               copyVersionInfo(XilObject* object);

    //
    //  Overload control for the setting/getting of the error string.
    //
    //  vSetErrorString() and vGetErrorString() are not expected to
    //  be MT-SAFE.
    //
    virtual void               vSetErrorString(const char* error_string);
    virtual void               vGetErrorString(char*        string_storage,
                                               unsigned int storage_size);

    //
    //  Derived objects set this flag at the beginning of their constructor
    //  and to TRUE at the conclusion of their constructor.  The
    //  XilObject::isOK() then tests this flag to indicate whether the object
    //  was successfully created.
    //
    Xil_boolean                isOKFlag;

    //
    //  These objects belong to a particular DAG which is setup at the time
    //    the operation is created.  All of the information about the assorted
    //    DAGs is kept in a DAG table object.  We are given an entry of the
    //    table which can be used in turn to reference and lock the DAG to
    //    which we belong.
    //
    XiliDagRef*                dagRef;
    unsigned int               dagRefCnt;

    //
    //  If we clear our DAG reference, then we may still have an unlock()
    //  called that's supposed to unlock the DAG we were pointing at.  In this
    //  special case, we store a copy of the old reference and then unlock it 
    //  and clear it upon a call to unlock().
    //
    XiliDagRef*                oldDagRef;

    //
    //  The error string associated with this object.
    //
    char*                      errorString;

private:
    //
    //  Object Id Management
    //
    static XilMutex            objectIdCounterMutex;
    static XilObjectId         objectIdCounter;
    static XilObjectId         getNewObjectId();

    //
    //  This object's unique identifier it's type.
    //
    const XilObjectId          objectId;
    const XilObjectType        objectType;

    //
    //  The assorted data for the object.
    //
    char*                      objectName;
    Xil_boolean                validState;
    
    XilSystemState*            systemState;

    //
    //  After a copy has been made, this object has a second set of version
    //  information active.  It describes which version of "information" is
    //  valid and it is not necessarily the same as the object id and version
    //  number.  But, once the object is updated, this information is marked
    //  as no longer valid and is ignored.
    //
    XilVersion                 objectVersion;

    //
    //  The data that supports locking the object recursively.
    //  In case of no dagRef being set, we lock the objectMutex.
    //
    XilMutex                   objectMutex;

    //
    //  Extra Buffer space until ABI works right
    //
    void*                      extraData[64];
#endif // _XIL_PRIVATE_DATA
