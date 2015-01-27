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
//  File:	_XilInterpolationTable.hh
//  Project:	XIL
//  Revision:	1.7
//  Last Mod:	10:21:12, 03/10/00
//
//  Description:
//	The XilInterpolationTable class describes an array of 1xn
//      seperable kernels.  These kernels represent the interpolation
//      filter to be used by the "general" interpolation geometric
//      routines when interpolating in the source image.
//	
//  MT-level:  UNSAFE
//	
//------------------------------------------------------------------------
//	COPYRIGHT
//------------------------------------------------------------------------
#pragma ident	"@(#)_XilInterpolationTable.hh	1.7\t00/03/10  "

#include "_XilNonDeferrableObject.hh"

#ifndef _XIL_INTERPOLATION_TABLE_HH
#define _XIL_INTERPOLATION_TABLE_HH

class XilInterpolationTable : public XilNonDeferrableObject {
public:
    //
    //  Get the size of each kernel.
    //
    unsigned int   getKernelSize();

    //
    //  Get the number of subsamples between pixels.
    //
    unsigned int   getNumSubsamples();

    //
    //  Get a pointer to the data contained in the table.
    //
    //  The data in the object is NOT copied by this call.  The amount of data
    //  in an interpolation table can be quite large and most often it is not
    //  necessary to copy it in order to use it.  In the default case, the
    //  given pointer is valid only as long as this interpolation table is
    //  still around.  The object id can be checked to see if it's the same
    //  object and the object version number can be used to check if it's the
    //  same data as before.
    //
    const float*   getData();

private:
#ifdef _XIL_LIBXIL_PRIVATE
#define _XIL_PRIVATE_DATA
    
#include "XilInterpolationTablePrivate.hh"
    
#undef  _XIL_PRIVATE_DATA
#else
    //
    //  Make it clear to the compiler that a destructor exists.  This is done
    //  so GPI users will get a compile-time error if they attempt to delete
    //  this class.
    //
                   ~XilInterpolationTable();
#endif // _XIL_PRIVATE_DATA
};

#endif // _XIL_INTERPOLATION_TABLE_HH
