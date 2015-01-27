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
//  File:	_XilDeferrableObject.hh
//  Project:	XIL
//  Revision:	1.18
//  Last Mod:	10:21:59, 03/10/00
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
//  MT Level:   UNSAFE
//
//------------------------------------------------------------------------
//	COPYRIGHT
//------------------------------------------------------------------------
#pragma ident	"@(#)_XilDeferrableObject.hh	1.18\t00/03/10  "

#ifndef _XIL_DEFERRABLE_OBJECT_HH
#define _XIL_DEFERRABLE_OBJECT_HH

#include "_XilDefines.h"
#include "_XilObject.hh"

//
//  Pull in Private Interface Include Files
//
#ifdef  _XIL_LIBXIL_PRIVATE
#define _XIL_PRIVATE_INCLUDES

#include "XilDeferrableObjectPrivate.hh"

#undef  _XIL_PRIVATE_INCLUDES
#endif // _XIL_LIBXIL_PRIVATE

class XilDeferrableObject : public XilObject {
public:
    //
    //  Mark this object as "synchronized" so no operations can be
    //    deferred with this object.
    //
    void               setSynchronized(Xil_boolean onoff);

    //
    //  Check to see if this object is "synchronized"
    //
    Xil_boolean        getSynchronized(void);

    //
    //  Get any XilDeviceIO that lives on this object.
    //
    XilDeviceIO*       getXilDeviceIO();

    //
    //  Set the tilesize for this object.  This call should be used with
    //  extreme care.  If the object already has a tile size, then resetting
    //  the tilesize with this call may require a lot of data reformatting.
    //
    //  The use of '0' indicates the extent of object should be used.
    //
    XilStatus          setTileSize(unsigned int tile_xsize,
                                   unsigned int tile_ysize);

private:
    //
    //  Pull in Private Data and Interface Information
    //
#ifdef _XIL_LIBXIL_PRIVATE
#define _XIL_PRIVATE_DATA
  
#include "XilDeferrableObjectPrivate.hh"
  
#undef  _XIL_PRIVATE_DATA
#else
    //
    //  Make it clear to the compiler that a destructor exists.  This is done
    //  so GPI users will get a compile-time error if they attempt to delete
    //  this class.
    //
                       ~XilDeferrableObject();
#endif // _XIL_PRIVATE_DATA
};

#endif   // _XIL_DEFERRABLE_OBJECT_HH
