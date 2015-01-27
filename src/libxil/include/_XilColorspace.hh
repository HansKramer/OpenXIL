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
//  File:	_XilColorspace.hh
//  Project:	XIL
//  Revision:	1.10
//  Last Mod:	10:20:55, 03/10/00
//
//  Description:
//      Definition of public interface to XilColorspace object
//
//
//  MT Level:   UNSAFE
//
//------------------------------------------------------------------------
//	COPYRIGHT
//------------------------------------------------------------------------
#pragma ident	"@(#)_XilColorspace.hh	1.10\t00/03/10  "

#ifndef XIL_COLORSPACE_HH
#define XIL_COLORSPACE_HH

#ifdef _XIL_HAS_KCMS
#include <kcms/kcs.h>
#endif

#include "_XilNonDeferrableObject.hh"

class XilColorspace : public XilNonDeferrableObject {
public:
    //
    //  Used by color_convert to aquire the XIL colorspace opcode and number
    //  of bands represented by this object.   These values will be valid even
    //  if the object was created using a type and data -- but only in the
    //  case where the type was specified as XIL_COLORSPACE_NAME.  For all
    //  other types, if the general colorspace data is set then XIL_CS_INVALID
    //  is returned for the op code and 0 for nbands.
    //
    XilColorspaceOpCode getOpcode();
    unsigned int        getNBands();

    //
    //  The more generic colorspace interface.  It specifies which type of
    //  data is represented by this object and the data given to us by the
    //  user.
    //
    XilColorspaceType   getColorspaceType();
    
    void*               getColorspaceData();


#ifdef _XIL_LIBXIL_PRIVATE
#define _XIL_PRIVATE_DATA

#include "XilColorspacePrivate.hh"

#undef _XIL_PRIVATE_DATA
#else
    //
    //  Make it clear to the compiler that a destructor exists.  This is done
    //  so GPI users will get a compile-time error if they attempt to delete
    //  this class.
    //
                        ~XilColorspace();
#endif // _XIL_PRIVATE_DATA
};

#endif
