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
//  File:	_XilBox.hh
//  Project:	XIL
//  Revision:	1.21
//  Last Mod:	10:21:31, 03/10/00
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
//------------------------------------------------------------------------
//	COPYRIGHT
//------------------------------------------------------------------------
#pragma ident	"@(#)_XilBox.hh	1.21\t00/03/10  "

#ifndef _XIL_BOX_HH
#define _XIL_BOX_HH

//
//  System Includes
//

//
//  C++ Includes
//
#include "_XilDefines.h"

//
//  Private Includes
//
#ifdef  _XIL_LIBXIL_PRIVATE
#define _XIL_PRIVATE_INCLUDES

#include "XilBoxPrivate.hh"

#undef  _XIL_PRIVATE_INCLUDES
#endif

class XilBox {
public:
    //
    //  Methods to get the information contained in the box.
    //
    //  We support aquiring either as an x,y location and size or as two
    //  points indicating the two corners of the box.
    //
    void         getAsRect(int*          x,
                           int*          y,
                           unsigned int* xsize,
                           unsigned int* ysize);
    
    void         getAsCorners(int* x1,
                              int* y1,
                              int* x2,
                              int* y2);
    
    //
    //  Methods to aquire an arbitrary tag of information on the Box.  This is
    //    used by various routines to provide additional information about the
    //    box.
    //
    void*        getTag();

private:
#ifdef _XIL_LIBXIL_PRIVATE
#define _XIL_PRIVATE_DATA
    
#include "XilBoxPrivate.hh"
    
#undef  _XIL_PRIVATE_DATA
#else
    //
    //  Make it clear to the compiler that a destructor exists.  This is done
    //  so GPI users will get a compile-time error if they attempt to delete
    //  this class.
    //
                 ~XilBox();
#endif
};

#endif  // _XIL_BOX_HH
