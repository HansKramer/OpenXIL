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
//  File:	_XilLookup.hh
//  Project:	XIL
//  Revision:	1.12
//  Last Mod:	10:21:01, 03/10/00
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
#pragma ident   "@(#)_XilLookup.hh	1.12\t00/03/10  "

#ifndef _XIL_LOOKUP_BASE_HH
#define _XIL_LOOKUP_BASE_HH

enum XilLookupType {
    XIL_LOOKUP_SINGLE,
    XIL_LOOKUP_COLORCUBE,
    XIL_LOOKUP_COMBINED
};

//
//  C++ Includes
//
#include "_XilNonDeferrableObject.hh"
#include "_XilMutex.hh"


class XilLookup : public XilNonDeferrableObject {
public:
    //
    //  Returns the datatype of the input.
    //
    XilDataType         getInputDataType();

    //
    //  Return lookup type - XIL_SINGLE, XIL_COLORCUBE or XIL_COMBINED
    //
    XilLookupType       getLookupType();

    //
    //  Returns the datatype of the output.
    //
    XilDataType         getOutputDataType();

    //
    //  Returns the number of bands for the output
    //
    unsigned int        getOutputNBands();

    //
    //  Returns the number of bands in the input.
    //
    unsigned int        getInputNBands();

    //
    // Returns the size in bytes of the lut output value for each band
    //
    unsigned int        getBytesPerBand();
 
    //
    // Returns the total size in bytes of the lut output value for all bands
    //
    unsigned int        getBytesPerEntry();

    //
    //  Returns TRUE if the LUT is formatted as a colorcube, FALSE otherwise.
    //
    Xil_boolean         isColorcube();	

#ifdef _XIL_LIBXIL_PRIVATE
#define _XIL_PRIVATE_DATA
    
#include "XilLookupBasePrivate.hh"
    
#undef _XIL_PRIVATE_DATA
#else
    //
    //  Make it clear to the compiler that a destructor exists.  This is done
    //  so GPI users will get a compile-time error if they attempt to delete
    //  this class.
    //
                        ~XilLookup();
#endif // _XIL_PRIVATE_DATA
};

#endif // _XIL_LOOKUP_BASE_HH

