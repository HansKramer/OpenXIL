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
//  File:	XiliDependentList.cc
//  Project:	XIL
//  Revision:	1.12
//  Last Mod:	10:08:21, 03/10/00
//
//  Description:
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
#pragma ident	"@(#)XiliDependentList.cc	1.12\t00/03/10  "

#include "_XilOp.hh"
#include "XiliDependentList.hh"

//
//  Flush every op in the list
//
XilStatus
XiliDependentList::flush()
{
    unsigned int current_index = 0;
    unsigned int visited_entries = 0;
    unsigned int total_to_visit = bagCount;

    XilStatus return_val = XIL_SUCCESS;

    //
    //  Since calling flushTile may remove entries from the bag, we need a
    //  different loop here which also tests the bagCount.
    //
    while(visited_entries < total_to_visit && bagCount != 0) {
        while(entryArray[current_index] == NULL) {
            current_index++;
        }

        XilOp* dependent = (XilOp*)entryArray[current_index];
        if(dependent->flush() == XIL_FAILURE) {
            //
            //  If a single tile in an operation fails to execute, the
            //  whole operation is considered to have failed and can
            //  be destroyed.
            //
            dependent->destroy();

            //
            //  Indicate we encountered a failure.
            //
            return_val = XIL_FAILURE;
        }
        
        current_index++;
        visited_entries++;
    }

    return return_val;
}

//
//  Flush every op in the list
//
XilStatus
XiliDependentList::flushTile(XilTileNumber tile_number)
{
    unsigned int current_index = 0;
    unsigned int visited_entries = 0;
    unsigned int total_to_visit = bagCount;

    XilStatus return_val = XIL_SUCCESS;

    //
    //  Since calling flushTile may remove entries from the bag, we need a
    //  different loop here which also tests the bagCount.
    //
    while(visited_entries < total_to_visit && bagCount != 0) {
        while(entryArray[current_index] == NULL) {
            current_index++;
        }

        XilOp* dependent = (XilOp*)entryArray[current_index];
        if(dependent->flushTile(tile_number) == XIL_FAILURE) {
            //
            //  If a single tile in an operation fails to execute, the
            //  whole operation is considered to have failed and can
            //  be destroyed.
            //
            dependent->destroy();

            //
            //  Indicate we encountered a failure.
            //
            return_val = XIL_FAILURE;
        }
        
        current_index++;
        visited_entries++;
    }

    return return_val;
}

XilStatus
XiliDependentList::cleanup(XilOp* cleanup_op)
{
    unsigned int current_index = 0;
    unsigned int visited_entries = 0;
    unsigned int total_to_visit = bagCount;
    
    while(visited_entries < total_to_visit) {
        while(entryArray[current_index] == NULL) {
            current_index++;
        }

        //
        //  Cleanup references to this op from the dependent.
        //
        ((XilOp*)entryArray[current_index])->cleanupOpReferences(cleanup_op);

        //
        //  Go ahead and remove this entry if it matches the op we're cleaning up.
        //
        if(entryArray[current_index] == cleanup_op) {
            entryArray[current_index] = NULL;
            bagCount--;
            if(current_index < nextOpenEntry)
                nextOpenEntry = current_index;
        }
        
        current_index++;
        visited_entries++;
    }
    
    return XIL_SUCCESS;
}
