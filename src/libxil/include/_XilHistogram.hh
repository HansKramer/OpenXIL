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
//  File:	_XilHistogram.hh
//  Project:	XIL
//  Revision:	1.18
//  Last Mod:	10:21:17, 03/10/00
//
//  Description:
//		
//	The XilHistogram class describes a multidimensional histogram. 
//	This object can be used to gather statistical information on images. 
//	
//  MT Level:   UNSAFE
//
//------------------------------------------------------------------------
//	COPYRIGHT
//------------------------------------------------------------------------
#pragma ident   "@(#)_XilHistogram.hh	1.18\t00/03/10  "
 
#ifndef _XIL_HISTOGRAM_HH
#define _XIL_HISTOGRAM_HH

//
//  C++ Includes
//
#include "_XilNonDeferrableObject.hh"


class XilHistogram : public XilNonDeferrableObject {
public:
    //
    //  Get the number of bands in the histogram.
    //
    unsigned int         getNumBands();

    //
    //  Get the total number of elements in the array
    //
    unsigned int         getNumElements();

    //
    // Get ptr to number of bins for each band
    //
    const unsigned int*  getNumBins();

    //
    // Get ptr to low value points for each band
    //
    const float*         getLowValues();

    //
    // Get ptr to high value points for each band
    //
    const float*         getHighValues();

    //
    //  Returns a pointer to the actual data.
    //
    const unsigned int*  getData();

    //
    //  Copies the histogram data to the supplied buffer
    //
    void                 getValues(unsigned int* data);

#ifdef _XIL_LIBXIL_PRIVATE
#define _XIL_PRIVATE_DATA
    
#include "XilHistogramPrivate.hh"
    
#undef  _XIL_PRIVATE_DATA
#else
    //
    //  Make it clear to the compiler that a destructor exists.  This is done
    //  so GPI users will get a compile-time error if they attempt to delete
    //  this class.
    //
                        ~XilHistogram();
#endif // _XIL_PRIVATE_DATA
};

#endif // _XIL_HISTOGRAM_HH
