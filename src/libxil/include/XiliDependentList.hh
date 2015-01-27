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
//  File:	XiliDependentList.hh
//  Project:	XIL
//  Revision:	1.11
//  Last Mod:	10:21:13, 03/10/00
//
//  Description:
//	Manages a deferrable object's dependents.  Every entry on the
//      Op Queue has a list of dependents. 
//
//  MT-level:  UNsafe
//	
//------------------------------------------------------------------------
//	COPYRIGHT
//------------------------------------------------------------------------
#pragma ident	"@(#)XiliDependentList.hh	1.11\t00/03/10  "

#ifndef _XILI_DEPENDENT_LIST_HH
#define _XILI_DEPENDENT_LIST_HH

#include "XiliBag.hh"
#include "_XilClasses.hh"

class XiliDependentList : public XiliBag {
public:
    //
    //  Run through the list and flush all of the ops.
    //
    XilStatus  flush();
    XilStatus  flushTile(XilTileNumber tile_number);

    //
    //  Run through the list and cleanup references to this op.
    //
    XilStatus  cleanup(XilOp* cleanup_op);

    //
    //  Constructor
    //
                XiliDependentList() : XiliBag()
    {
    }
};

#endif   // _XILI_DEPENDENT_LIST_HH


