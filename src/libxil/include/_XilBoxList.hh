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
//  File:	_XilBoxList.hh
//  Project:	XIL
//  Revision:	1.12
//  Last Mod:	10:21:24, 03/10/00
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
//  MT-level:  UN-SAFE
//	
//------------------------------------------------------------------------
//	COPYRIGHT
//------------------------------------------------------------------------
#pragma ident	"@(#)_XilBoxList.hh	1.12\t00/03/10  "

#ifndef _XIL_BOXLIST_H
#define _XIL_BOXLIST_H

//
//  C Includes
//

//
//  C++ Includes
//
#include "_XilBox.hh"

//
//  Private Includes
//
#ifdef  _XIL_LIBXIL_PRIVATE
#define _XIL_PRIVATE_INCLUDES

#include "XilBoxListPrivate.hh"

#undef  _XIL_PRIVATE_INCLUDES
#endif

//
//  The XilBoxList class...
//
class XilBoxList {
public:
    //
    //  These get the next group of entries in the list of storage boxes.
    //    There is a box for each of the sources in an operation and one for
    //    the destination.  Each entry in the list corresponds to an area of
    //    storage to be aquired in the respective images.
    //
    //  - For these calls, there is at most ONE destination.
    //  - Additional images are source images.
    //  - The source images are given in order starting with the first source.
    //  - The last image returned is always the destination.
    //
    Xil_boolean  getNext(XilBox** im1_box);

    Xil_boolean  getNext(XilBox** im1_box,
                         XilBox** im2_box);

    Xil_boolean  getNext(XilBox** im1_box,
                         XilBox** im2_box,
                         XilBox** im3_box);

    Xil_boolean  getNext(XilBox** im1_box,
                         XilBox** im2_box,
                         XilBox** im3_box,
                         XilBox** im4_box);

    //
    //  Get the number of sources and number of destinations represented by
    //  this box list.  This can be used to determine the size of the array to
    //  pass into the getNext() call.
    //
    void         getNumBoxes(unsigned int* num_srcs,
                             unsigned int* num_dsts);

    //
    //  This is the generic getNext call.  It is used by operations that
    //    utilize more than 3 sources or more than 1 destination.
    //
    //  The box_array contains all of the source images and then all of the
    //    destination images in that order with the counts given by num_srcs
    //    and num_dsts.
    //
    //  The returned array contains pointers to boxes for the number of
    //    sources and number of destinations described by this box list.  The
    //    called can use getNumBoxes() to verify the size or can always pass
    //    in the documented maximum number of sources and destinations for a
    //    given version of the XIL library.
    //
    //  The information returned in the array is valid until the next call to
    //    getNext() or getNextArray() on this object.
    //
    Xil_boolean  getNextArray(XilBox* box_array[]);

    //
    //  If a compute routine cannot process the given boxes, then it is
    //  supposed to call this routine to mark the current box as having
    //  failed.
    //
    //  The compute routine is expected to test the value returned by this
    //  routine for XIL_SUCCESS or XIL_FAILURE.  If the value is XIL_FAILURE,
    //  then the compute routine is expected to return XIL_FAILURE
    //  immediately.  If the value is XIL_SUCCESS, the compute routine is
    //  expected to continue processing the next box in the list.
    //
    XilStatus    markAsFailed();

private:
#ifdef _XIL_LIBXIL_PRIVATE
#define _XIL_PRIVATE_DATA
    
#include "XilBoxListPrivate.hh"
    
#undef  _XIL_PRIVATE_DATA
#else
    //
    //  Make it clear to the compiler that a destructor exists.  This is done
    //  so GPI users will get a compile-time error if they attempt to delete
    //  this class.
    //
                 ~XilBoxList();
#endif // _XIL_PRIVATE_DATA
};

#endif // _XIL_BOXLIST_H
