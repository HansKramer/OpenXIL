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
//  File:	_XilObject.hh
//  Project:	XIL
//  Revision:	1.29
//  Last Mod:	10:21:23, 03/10/00
//
//  Description:
//	
//	This is the base class from which all deferrable and
//	non-deferrable objects are derived.
//
//	XilObject is an abstract class.
//
//  MT Level:   UNSAFE
//
//------------------------------------------------------------------------
//	COPYRIGHT
//------------------------------------------------------------------------
#pragma ident	"@(#)_XilObject.hh	1.29\t00/03/10  "

#ifndef _XIL_OBJECT_HH
#define _XIL_OBJECT_HH

//
//  XIL Includes
//
#include "_XilDefines.h"
#include "_XilGPIDefines.hh"
#include "_XilClasses.hh"
#include "_XilMutex.hh"

//
//  Private Includes
//
#ifdef  _XIL_LIBXIL_PRIVATE
#define _XIL_PRIVATE_INCLUDES

#include "XilObjectPrivate.hh"

#undef  _XIL_PRIVATE_INCLUDES
#endif

//------------------------------------------------------------------------
//
//  Class:	XilVersion
//
//  Description:
//	
//	Encapsulates all of the version information that differentiates
//	one XilObject from another.
//	
//	When an object is copied, the version information is copied as
//	well so the copy is considered the same version as its
//	
//------------------------------------------------------------------------

class XilVersion {
public:
    XilObjectId      objectId;
    XilVersionNumber versionNumber;

                     XilVersion();

private:
    XilObjectId      infoId;
    XilVersionNumber infoVersion;

    void*            info[4];

    friend class XilObject;
};


class XilObject {
public:
    //
    //  Returns the name of the object
    //
    char*                      getName();

    //
    //  Sets the name of the object
    //
    void                       setName(const char* name);

    //
    //  Returns the XilObjectType of the object
    //
    XilObjectType              getType();

    //
    //  Returns a pointer to the system state which created this object.
    //
    //  Calling this on a NULL object is not supported.  If there is a
    //  possibility the object may be NULL, test for NULL before calling
    //  getSystemState().
    //
    XilSystemState*            getSystemState();

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
    void                       getVersion(XilVersion* version);
    Xil_boolean                isSameAs(XilVersion* version);
    
    //
    //  Returns the unique object number.  It is unique for objects
    //  created in the entire library -- regardless of the system state.
    //
    XilObjectId                getObjectId();

    //
    //  Objects can be either valid or invalid.  Once an object is invalid,
    //  operations using it fail until its state is valid again.
    //
    Xil_boolean                isValid();

    //
    //  Routine to destroy objects rather than directly using "delete".
    //  Objects to not publicly expose their destructors.
    //
    void                       destroy();

    //
    //  Check to see if the object was created successfully
    //
    Xil_boolean                isOK();

    //
    //  Returns an exact copy of the object.  The version numbers are the same
    //  until the object is modified. 
    //
    XilObject*                 getCopy();

    //
    //  Sets and gets the error string associated with this object.
    //
    void                       setErrorString(const char*  error_string);
    void                       getErrorString(char*        string_storage,
                                              unsigned int storage_size);
    
    //
    //  XilObjects are only created through the XilSystemState.  In
    //  fact, this is an abstract class in that it can not be created
    //  directly, only the subclasses can be created.
    //

private:
#ifdef _XIL_LIBXIL_PRIVATE
#define _XIL_PRIVATE_DATA
    
#include "XilObjectPrivate.hh"
    
#undef  _XIL_PRIVATE_DATA
#else
    //
    //  Make it clear to the compiler that a destructor exists.  This is done
    //  so GPI users will get a compile-time error if they attempt to delete
    //  this class.
    //
                               ~XilObject();
#endif
};
#endif // _XIL_OBJECT_HH
