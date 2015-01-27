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
//  File:	_XilLookupSingle.hh
//  Project:	XIL
//  Revision:	1.11
//  Last Mod:	10:22:06, 03/10/00
//
//  Description:
//		
//  The XilLookupSingle class describes a table of data that is used to
//  convert a single input band of data to one or more bands of output data.
//	
//  MT Level:   UNSAFE
//
//------------------------------------------------------------------------
//  COPYRIGHT
//------------------------------------------------------------------------
#pragma ident   "@(#)_XilLookupSingle.hh	1.11\t00/03/10  "

#ifndef _XIL_LOOKUP_SINGLE_HH
#define _XIL_LOOKUP_SINGLE_HH

//
//  C++ Includes
//
#include "_XilLookup.hh"

class XilLookupSingle : public XilLookup {
public:
    //
    //  Returns the total number of entries in the table.
    //
    unsigned int        getNumEntries();
    
    //
    //  Returns a pointer to the actual data
    //
    const void*         getData();

    //
    //  Returns the offset that describes the input value 
    //  corresponding to the first table value.
    //
    int                 getOffset();

    //
    //  Copies 'count' data values from the LUT starting at the table entry
    //    position 'start' into the buffer 'data'
    //
    void                getValues(int          start,
                                  unsigned int count,
                                  void*        data);


    //
    //  Sets the offset that describes the input value corresponding to the
    //    first table value.
    //
    //  This should not be called by the GPI on lookups not explicitly created
    //  by users of the GPI.  The API can call this to set values, but GPI
    //  functions should not alter the values.
    //
    void                setOffset(int);

    //
    //  Copies 'count' data values from the buffer 'data' into the LUT
    //    starting at the table entry position 'start'
    //
    //  This should not be called by the GPI on lookups not explicitly created
    //  by users of the GPI.  The API can call this to set values, but GPI
    //  functions should not alter the values.
    //
    void                setValues(int          start,
                                  unsigned int count,
                                  const void*  data);


#ifdef _XIL_LIBXIL_PRIVATE
#define _XIL_PRIVATE_DATA
    
#include "XilLookupSinglePrivate.hh"
    
#undef _XIL_PRIVATE_DATA
#else
    //
    //  Make it clear to the compiler that a destructor exists.  This is done
    //  so GPI users will get a compile-time error if they attempt to delete
    //  this class.
    //
                        ~XilLookupSingle();
#endif // _XIL_PRIVATE_DATA
};

#endif // _XIL_LOOKUP_SINGLE_HH

