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
//  File:	XilError.hh
//  Project:	XIL
//  Revision:	1.10
//  Last Mod:	10:21:04, 03/10/00
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
#pragma ident	"@(#)XilError.hh	1.10\t00/03/10  "

#ifndef _XIL_ERROR_HH
#define _XIL_ERROR_HH

//
//  System Includes
//
#include <stdio.h>

//
//  XIL Includes
//
#include "_XilDefines.h"
#include "_XilClasses.hh"

//
//  Private Includes
//
#ifdef  _XIL_LIBXIL_PRIVATE
#define _XIL_PRIVATE_INCLUDES

#include "XilErrorPrivate.hh"

#undef  _XIL_PRIVATE_INCLUDES
#endif

//
//  Definition of error handling function type, XilErrorFunc.
//
typedef  Xil_boolean (*XilErrorFunc)(XilError*);

class XilError {
public:

private:
#ifdef  _XIL_LIBXIL_PRIVATE
#define _XIL_PRIVATE_DATA

#include "XilErrorPrivate.hh"

#undef  _XIL_PRIVATE_DATA
#endif
};

#endif // _XIL_ERROR_HH

