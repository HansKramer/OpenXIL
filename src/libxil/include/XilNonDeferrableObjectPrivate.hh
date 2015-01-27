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
//  File:	XilNonDeferrableObjectPrivate.hh
//  Project:	XIL
//  Revision:	1.7
//  Last Mod:	10:22:07, 03/10/00
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
//  MT-level:  <??????>
//	
//------------------------------------------------------------------------
//	COPYRIGHT
//------------------------------------------------------------------------
#pragma ident	"@(#)XilNonDeferrableObjectPrivate.hh	1.7\t00/03/10  "

#ifdef _XIL_PRIVATE_INCLUDES

#include "_XilDefines.h"
#include "XiliBag.hh"

class XiliDependentInfo {
public:
    XilOp*               op;
    XilDeferrableObject* defobj;

    XiliDependentInfo(XilOp*               init_op     = NULL,
                      XilDeferrableObject* init_defobj = NULL) {
        op     = init_op;
        defobj = init_defobj;
    }

    _XIL_NEW_DELETE_OVERLOAD_PUBLIC(XiliDependentInfo)

private:
    _XIL_NEW_DELETE_OVERLOAD_PRIVATE(XiliDependentInfo)
};

#endif // _XIL_PRIVATE_INCLUDES

#ifdef _XIL_PRIVATE_DATA

public:
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
    XilNonDeferrableObject*  aquireDefRef(XilOp*               op);
    XilNonDeferrableObject*  aquireDefRef(XilDeferrableObject* defobj);

    //
    //  Remove the given deferred reference.  The deferred object no longer
    //  needs this object.
    //
    XilStatus                releaseDefRef(XilOp*               op);
    XilStatus                releaseDefRef(XilDeferrableObject* defobj);

    //
    //  When a new version of the object is created, we need to make a copy of
    //  ourselves and then update our dependents to reference the new copy.
    //
    virtual void             newVersion();

    //
    //  We overload preDestroy() to give us a chance to copy the object and
    //  pass it on to any dependents we may have. 
    //
    virtual Xil_boolean      preDestroy();

protected:
                             XilNonDeferrableObject(XilSystemState* system_state,
                                                    XilObjectType   object_type)
                                 : XilObject(system_state, object_type),
                                   dependents(system_state)
    {
    }
public:

                             ~XilNonDeferrableObject()
    {
    }

private:
    XiliBag                  dependents;

#endif // _XIL_PRIVATE_DATA
