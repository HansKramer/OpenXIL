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
//  File:	_XilGlobalState.hh
//  Project:	XIL
//  Revision:	1.16
//  Last Mod:	10:21:00, 03/10/00
//
//  MT Level:   Partially Safe (Exceptions in comments)
//
//  Description:
//	
//	
//------------------------------------------------------------------------
//	COPYRIGHT
//------------------------------------------------------------------------
#pragma ident	"@(#)_XilGlobalState.hh	1.16\t00/03/10  "

#ifndef _XIL_GLOBAL_STATE_HH
#define _XIL_GLOBAL_STATE_HH

#include "_XilDefines.h"
#include "_XilClasses.hh"
#include "_XilMutex.hh"

//
//  Declare the XilOp::create function pointer.
//
typedef
XilOp*  (*XilOpCreateFunctionPtr) (const char* operation_name,
                                   void*       args[],
                                   int         arg_count);

#ifdef _XIL_LIBXIL_PRIVATE
#define _XIL_PRIVATE_INCLUDES

#include "XilGlobalStatePrivate.hh"

#undef _XIL_PRIVATE_INCLUDES
#endif // _XIL_LIBXIL_PRIVATE

class  XilGlobalState {
public:
    //
    //  Static function to aquire a pointer to the global state.
    //
    //  MT-safe except for first caller.  Only a single thread can
    //  create the global state and currently XilOpen() ensures this
    //  is the case.  
    //
    static XilGlobalState* getXilGlobalState();
    
    //
    //  Return the Op number associated with the given name.
    //
    //  MT-safe
    //
    XilOpNumber             lookupOpNumber(const char* operation_name);

    //
    //  Describe an Op creation function.
    //
    XilStatus               describeOpFunction(const char* operation_name,
                                               const char* op_class_name,
                                               void*       attribs        = NULL);

#ifdef _XIL_LIBXIL_PRIVATE
#include "XilGlobalStatePrivate.hh"
#else
    //
    //  Make it clear to the compiler that a destructor exists.  This is done
    //  so GPI users will get a compile-time error if they attempt to delete
    //  this class.
    //
                            ~XilGlobalState();
#endif
};

#endif  // _XIL_GLOBAL_STATE_HH



