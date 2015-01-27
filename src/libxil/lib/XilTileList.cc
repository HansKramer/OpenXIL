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
//  File:	XilTileList.cc
//  Project:	XIL
//  Revision:	1.4
//  Last Mod:	10:08:53, 03/10/00
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
//  MT-level:  UNsafe
//
//------------------------------------------------------------------------
//	COPYRIGHT
//------------------------------------------------------------------------
#pragma ident	"@(#)XilTileList.cc	1.4\t00/03/10  "

#include "_XilTile.hh"
#include "_XilTileList.hh"

XilTile*
XilTileList::getNext()
{
    return getNextTile();
}

XilTile*
XilTileList::getTile(unsigned int entry_number)
{
    if(entry_number < numTiles) {
        return &tileArrayPtr[tileList[entry_number]];
    } else {
        return NULL;
    }
}

unsigned int
XilTileList::getNumTiles()
{
    return numTiles;
}

void
XilTileList::reset()
{
    currentEntry = 0;
}
