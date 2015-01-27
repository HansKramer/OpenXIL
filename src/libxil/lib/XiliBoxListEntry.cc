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
//  File:	XiliBoxListEntry.cc
//  Project:	XIL
//  Revision:	1.13
//  Last Mod:	10:08:33, 03/10/00
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
//  MT-level:  <??????>
//	
//------------------------------------------------------------------------
//	COPYRIGHT
//------------------------------------------------------------------------
#pragma ident	"@(#)XiliBoxListEntry.cc	1.13\t00/03/10  "

#include <stdio.h>

//
// C++ Includes
//
#include "_XilDefines.h"
#include "_XilMutex.hh"
#include "XiliBoxListEntry.hh"

//
//  Use generic macro for the overloaded new/delete functions
//
_XIL_NEW_DELETE_OVERLOAD_CC_FILE(XiliBoxListEntry, 1024, 64)

XiliBoxListEntry::XiliBoxListEntry()
{
    boxCount = 0;
}

XiliBoxListEntry::~XiliBoxListEntry()
{
}

int
XiliBoxListEntry::operator==(const XiliBoxListEntry& ) const
{
    // TODO:  Implement this method...
    return FALSE;
}

#ifdef DEBUG
void
XiliBoxListEntry::dump()
{
    fprintf(stderr, "\n    entry %d - ", 0);
    boxes[0].dump();
    for(unsigned int i=1; i<boxCount; i++) {
        fprintf(stderr, "    entry %d - ", i);
        boxes[i].dump();
    }
}
#endif


