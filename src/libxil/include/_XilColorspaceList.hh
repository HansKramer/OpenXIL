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
//  File:	_XilColorspaceList.hh
//  Project:	XIL
//  Revision:	1.8
//  Last Mod:	10:21:06, 03/10/00
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
#pragma ident	"@(#)_XilColorspaceList.hh	1.8\t00/03/10  "

#ifndef XIL_COLORSPACE_LIST_HH
#define XIL_COLORSPACE_LIST_HH

//
//  System Includes
//

//
//  XIL Includes
//
#include "_XilNonDeferrableObject.hh"

//
//  Private Includes
//
#ifdef  _XIL_LIBXIL_PRIVATE
#define _XIL_PRIVATE_INCLUDES

#include "XilColorspaceListPrivate.hh"

#undef  _XIL_PRIVATE_INCLUDES
#endif

class XilColorspaceList : public XilNonDeferrableObject {
public:
    //
    //  Returns a pointer to the given entry in the list.
    //
    XilColorspace*   getColorspace(unsigned int entry_number);

    //
    //  Gets the number of colorspaces in the list.
    //
    unsigned int     getNumColorspaces();

    //
    //  Sets a data pointer to cache that represents this object.  It remains
    //  on this object until something chagnes the object or it is destroyed.
    //  At that time, the specified function pointer is called with the data
    //  pointer given as an argument.
    //
    void             setCachedData(void*                      cache_data,
                                   XIL_FUNCPTR_DONE_WITH_DATA fptr);
    
    void*            getCachedData();
    
private:
#ifdef _XIL_LIBXIL_PRIVATE
#define _XIL_PRIVATE_DATA

#include "XilColorspaceListPrivate.hh"

#undef _XIL_PRIVATE_DATA
#else
    //
    //  Make it clear to the compiler that a destructor exists.  This is done
    //  so GPI users will get a compile-time error if they attempt to delete
    //  this class.
    //
                      ~XilColorspaceList();
#endif // _XIL_PRIVATE_DATA
};

#endif
