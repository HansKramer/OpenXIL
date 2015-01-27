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
//  File:	_XilNonDeferrableObject.hh
//  Project:	XIL
//  Revision:	1.12
//  Last Mod:	10:22:15, 03/10/00
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
#pragma ident	"@(#)_XilNonDeferrableObject.hh	1.12\t00/03/10  "

#ifndef _XIL_NON_DEFERRABLE_OBJECT_HH
#define _XIL_NON_DEFERRABLE_OBJECT_HH

//
//  C++ Includes
//
#include "_XilObject.hh"

//
//  Pull in Private Interface Include Files
//
#ifdef  _XIL_LIBXIL_PRIVATE
#define _XIL_PRIVATE_INCLUDES

#include "XilNonDeferrableObjectPrivate.hh"

#undef  _XIL_PRIVATE_INCLUDES
#endif // _XIL_LIBXIL_PRIVATE

class XilNonDeferrableObject : public XilObject {
    //
    //  Pull in Private Data and Interface Information
    //
#ifdef _XIL_LIBXIL_PRIVATE
#define _XIL_PRIVATE_DATA
  
#include "XilNonDeferrableObjectPrivate.hh"
  
#undef  _XIL_PRIVATE_DATA
#else
    //
    //  Make it clear to the compiler that a destructor exists.  This is done
    //  so GPI users will get a compile-time error if they attempt to delete
    //  this class.
    //
protected:
                        ~XilNonDeferrableObject();
#endif // _XIL_PRIVATE_DATA
};

#endif // _XIL_NON_DEFERRABLE_OBJECT_HH
