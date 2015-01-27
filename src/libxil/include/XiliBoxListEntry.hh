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
//  File:	XiliBoxListEntry.hh
//  Project:	XIL
//  Revision:	1.8
//  Last Mod:	10:21:15, 03/10/00
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
#pragma ident	"@(#)XiliBoxListEntry.hh	1.8\t00/03/10  "

    
#ifndef _XILI_BOX_LIST_ENTRY_HH
#define _XILI_BOX_LIST_ENTRY_HH

#include "_XilDefines.h"
#include "_XilBox.hh"

class XiliBoxListEntry {
public:
    //
    //  Test for equality
    //
    int                   operator==(const XiliBoxListEntry& rval) const;

#ifdef DEBUG
    //
    //  Print the contents to stderr on a single line
    //
    void                  dump();
#endif

                          XiliBoxListEntry();
                          ~XiliBoxListEntry();
    
    //
    //  The boxes contained in the entry.
    //
    XilBox                boxes[4];
    unsigned int          boxCount;

    //
    //  Overload new and delete for speed
    //
    _XIL_NEW_DELETE_OVERLOAD_PUBLIC(XiliBoxListEntry)

private:
    //  More new/delete overloading
    _XIL_NEW_DELETE_OVERLOAD_PRIVATE(XiliBoxListEntry)
};

#endif // _XILI_BOX_LIST_ENTRY_HH
