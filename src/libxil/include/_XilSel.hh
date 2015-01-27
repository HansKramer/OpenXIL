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
//  File:	_XilSel.hh
//  Project:	XIL
//  Revision:	1.10
//  Last Mod:	10:20:59, 03/10/00
//
//  Description:
//		
//      The XilSel class describes a structuring element.
//
//	
//  MT-level:  UNSAFE
//	
//------------------------------------------------------------------------
//  COPYRIGHT
//------------------------------------------------------------------------
#pragma ident   "@(#)_XilSel.hh	1.10\t00/03/10  "

#ifndef _XIL_SEL_HH
#define _XIL_SEL_HH

//
//  C++ Includes
//
#include "_XilNonDeferrableObject.hh"


class XilSel : public XilNonDeferrableObject {
public:
    //
    //  Returns dimensions
    //
    unsigned int        getWidth();
    unsigned int        getHeight();

    //
    // Return key (origin) pixel location
    //
    int		        getKeyX();
    int		        getKeyY();

    //
    // Return structure element data array
    //
    const unsigned int* getData();

    //
    // Copy the sel values to the supplied array
    //
    void                getValues(unsigned int* data);

#ifdef _XIL_LIBXIL_PRIVATE
#define _XIL_PRIVATE_DATA
    
#include "XilSelPrivate.hh"
    
#undef _XIL_PRIVATE_DATA
#else
    //
    //  Make it clear to the compiler that a destructor exists.  This is done
    //  so GPI users will get a compile-time error if they attempt to delete
    //  this class.
    //
                        ~XilSel();
#endif // _XIL_PRIVATE_DATA
};

#endif // _XIL_SEL_HH
