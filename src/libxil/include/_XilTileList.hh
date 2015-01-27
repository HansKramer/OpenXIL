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
//  File:	_XilTileList.hh
//  Project:	XIL
//  Revision:	1.6
//  Last Mod:	10:21:45, 03/10/00
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
//  MT-level:  UNSAFE
//	
//------------------------------------------------------------------------
//	COPYRIGHT
//------------------------------------------------------------------------
#pragma ident	"@(#)_XilTileList.hh	1.6\t00/03/10  "

#ifndef _XIL_TILE_LIST
#define _XIL_TILE_LIST

//
//  System Includes
//

//
//  XIL Includes
//
#include "_XilNonDeferrableObject.hh"

//
//  Private Includes
//
#ifdef  _XIL_LIBXIL_PRIVATE
#define _XIL_PRIVATE_INCLUDES

#include "XilTileListPrivate.hh"

#undef  _XIL_PRIVATE_INCLUDES
#endif

//
//  The XilTileList Class...
//
class XilTileList {
public:
    //-------------------------------------------------------------------------
    //
    //  Counting/Random Access Methods:
    //

    //
    //  Get the nth tile on the list.
    //
    XilTile*          getTile(unsigned int entry_num);

    //
    //  Find out how many tiles are in the list.
    //
    unsigned int      getNumTiles();
    //
    //-------------------------------------------------------------------------

    //-------------------------------------------------------------------------
    //
    //  Iterator Method:
    //    A self-resetting tile list.  Once getNext() returns NULL, the next
    //    call to getNext() will return the tile at the beginning of the
    //    list.   Upon construction, the iterator points at the beginning of
    //    the list.

    //
    //  Get the next entry in the list.
    //
    XilTile*          getNext();

    //
    //  Reset the list so that the next getNext() call returns the entry at
    //  the beginning of the list.  Useful for resetting the list from failure
    //  conditions encountered while iterating over the list.
    //
    void              reset();
    //
    //-------------------------------------------------------------------------

private:
#ifdef _XIL_LIBXIL_PRIVATE
#define _XIL_PRIVATE_DATA
    
#include "XilTileListPrivate.hh"
    
#undef  _XIL_PRIVATE_DATA
#else
    //
    //  Make it clear to the compiler that a destructor exists.  This is done
    //  so GPI users will get a compile-time error if they attempt to delete
    //  this class.
    //
                      ~XilTileList();
#endif
};

#endif // _XIL_TILE_LIST
