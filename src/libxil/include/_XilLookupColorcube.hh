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
//  File:	_XilLookupColorcube.hh
//  Project:	XIL
//  Revision:	1.9
//  Last Mod:	10:21:14, 03/10/00
//
//  Description:
//		
//  The XilLookup class describes a table of data that is used to
//  convert or interpret image data.  The lookup table can have multiple
//  output data for each input value; that is, it can convert a single
//  band image into a multiple band image. In the special case of three
//  banded output, it can be thought of as a colormap.
//	
//  MT Level:   UNSAFE
//
//------------------------------------------------------------------------
//  COPYRIGHT
//------------------------------------------------------------------------
#pragma ident   "@(#)_XilLookupColorcube.hh	1.9\t00/03/10  "

#ifndef _XIL_LOOKUP_COLORCUBE_HH
#define _XIL_LOOKUP_COLORCUBE_HH

//
//  C++ Includes
//
#include "_XilLookupSingle.hh"


class XilLookupColorcube : public XilLookupSingle {
public:
    //
    //  Get the colorcube's multipliers and dimensions.
    //
    const int*          getMultipliers();

    const unsigned int* getDimensions();

    int                 getAdjustedOffset();
    
    //
    //  Often, compute routines subtract one from the dimensions for
    //  performing the ordered dither operation.  This is a convenience
    //  routine which returns the dimensions minus one (dims[i] - 1).
    //
    const unsigned int* getDimsMinus1();

#ifdef _XIL_LIBXIL_PRIVATE
#define _XIL_PRIVATE_DATA
    
#include "XilLookupColorcubePrivate.hh"
    
#undef _XIL_PRIVATE_DATA
#else
    //
    //  Make it clear to the compiler that a destructor exists.  This is done
    //  so GPI users will get a compile-time error if they attempt to delete
    //  this class.
    //
                        ~XilLookupColorcube();
#endif // _XIL_PRIVATE_DATA
};

#endif // _XIL_LOOKUP_COLORCUBE_HH
