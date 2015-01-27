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
//  File:	_XilDeviceManager.hh
//  Project:	XIL
//  Revision:	1.8
//  Last Mod:	10:22:02, 03/10/00
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
//  MT Level:   UNSAFE
//
//------------------------------------------------------------------------
//	COPYRIGHT
//------------------------------------------------------------------------
#pragma ident	"@(#)_XilDeviceManager.hh	1.8\t00/03/10  "

//
//  C++ Includes
//
#include "_XilDefines.h"
#include "_XilClasses.hh"

//
//  Private Includes
//
#ifdef _XIL_LIBXIL_PRIVATE
#define _XIL_PRIVATE_INCLUDES
    
#include "XilDeviceManagerPrivate.hh"
    
#undef  _XIL_PRIVATE_INCLUDES
#endif

#ifndef _XIL_DEVICE_MANAGER_HH
#define _XIL_DEVICE_MANAGER_HH

class XilDeviceManager {
public:
    virtual const char* getDeviceName()=0;

    //
    //  describeMembers() is called by the XIL core to have the derived
    //  manager provide XIL a description of the operations and functions it
    //  implements.  The derived version of describeMembers() uses the
    //  protected addFunction() method to describe its members. 
    //
    virtual XilStatus   describeMembers()=0;

    //
    //  Called by the XIL core to destroy this device once it is no longer
    //    needed.
    //
    void                destroy();

protected:
    //
    //  This adds a new compute function to the XIL library.  It is used by
    //    the derived implementation of describeMembers().
    //
    XilStatus           addFunction(XilFunctionInfo* func_info);

                        XilDeviceManager();
    virtual             ~XilDeviceManager();

private:
#ifdef _XIL_LIBXIL_PRIVATE
#define _XIL_PRIVATE_DATA
    
#include "XilDeviceManagerPrivate.hh"
    
#undef  _XIL_PRIVATE_DATA
#endif

    //
    //  Base class data members
    //
    XilGlobalState*                 globalState;
    const char*                     deviceName;

    //
    //  Extra data members which can be replaced with different data in future
    //    versions without breaking the derived classes.
    //
    void*                           _extra_data[256];
};
#endif  // _XIL_DEVICE_MANAGER_HH

