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
//  File:	_XilCondVar.hh
//  Project:	XIL
//  Revision:	1.8
//  Last Mod:	10:21:21, 03/10/00
//
//  Description:
//	
//	
//	
//	
//------------------------------------------------------------------------
//	COPYRIGHT
//------------------------------------------------------------------------
#pragma ident	"@(#)_XilCondVar.hh	1.8\t00/03/10  "

#ifndef _XIL_COND_VAR
#define _XIL_COND_VAR

#include "_XilDefines.h"
#include "_XilMutex.hh"

//
//  Private Includes
//
#ifdef  _XIL_LIBXIL_PRIVATE
#define _XIL_PRIVATE_INCLUDES

#include "XilCondVarPrivate.hh"

#undef  _XIL_PRIVATE_INCLUDES
#endif

class XilCondVar {
public:
    //
    //  Constructors
    //
                      XilCondVar();
                      XilCondVar(XilMutex* mutex);

    //
    //  Destructor
    //
                      ~XilCondVar();

    //
    //  Methods
    //
    void              wait();
    
    void              wait(XilMutex* mutex);
    
    void              signal();
    void              broadcast();

private:
#ifdef _XIL_LIBXIL_PRIVATE
#define _XIL_PRIVATE_DATA
#include "XilCondVarPrivate.hh"
#undef  _XIL_PRIVATE_DATA
#else
    //
    //  Data matching size of class.
    //
    unsigned long     _classData[64 / sizeof(unsigned long)];
#endif
};

#endif //  _XIL_COND_VAR
