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
//  File:	XilBoxList.cc
//  Project:	XIL
//  Revision:	1.20
//  Last Mod:	10:08:22, 03/10/00
//
//  Description:
//	
//	This file contains the exposed formats that an ROI can
//	take for use by the compute devices. The compute device may
//	request the ROI (intersected with the box) as a rectlist, as 
//	a bitmask or as a convex region. In the case of the bitmask,
//	they may also query whether or not the region fills the box
//	entirely for a performance benefit.
//	
//  MT-level:  <??????>
//	
//------------------------------------------------------------------------
//	COPYRIGHT
//------------------------------------------------------------------------
#pragma ident	"@(#)XilBoxList.cc	1.20\t00/03/10  "

#include "_XilDefines.h"
#include "_XilBoxList.hh"
#include "_XilMutex.hh"
#include "_XilSystemState.hh"

//
//  Expect that we may need a number of box lists at any one time so we create
//  8 at a time to a maximum of 64.
//
_XIL_NEW_DELETE_OVERLOAD_CC_FILE(XilBoxList, 64, 8)

XilBoxList::XilBoxList(XilSystemState* system_state,
                       unsigned int    num_srcs,
                       unsigned int    num_dsts)
{
	this->constructorVars(system_state, num_srcs, num_dsts);
}

//
//  Constructor for constructing arrays of the box list for the overload.
//
XilBoxList::XilBoxList()
{
    outstandingBoxListEntry = NULL;
    systemState             = NULL;
    currentPos              = _XILI_SLLIST_INVALID_POSITION;

}

XilBoxList::~XilBoxList()
{
	this->destructorVars();
}

//------------------------------------------------------------------------
//
//  Function:	getNumBoxes()
//
//  Description:
//	Returns the number of source and destintion boxes represented
//	by this box list object.
//	
//  MT-level:  UNSAFE
//	
//------------------------------------------------------------------------
void
XilBoxList::getNumBoxes(unsigned int* num_srcs,
                        unsigned int* num_dsts)
{
    *num_srcs = numSrcs;
    *num_dsts = numDsts;
}

//------------------------------------------------------------------------
//
//  Function:	setupNextBox()
//
//  Description:
//	Pulls the next box from the list and makes outstandingBoxListEntry
//      point to it. 
//	
//	Used by the getNext() routines below to standardize skipping
//	though the list.
//	
//  MT-level:  UNSAFE
//	
//------------------------------------------------------------------------
inline
void
XilBoxList::setupNextBox()
{
    //
    //  Delete the outstanding box which is the one we retrieved last time.
    //

    delete outstandingBoxListEntry;
    outstandingBoxListEntry = NULL;

    //
    //  Retrieve the next entry from the list.
    //
    XiliBoxListEntry* ble;
    if(list.remove(list.head(), ble) == XIL_FAILURE) {
        //
        //  There was no more entries on the list.  Thus, there is no
        //  outstandingBoxListEntry so reset it to NULL.
        //
        outstandingBoxListEntry = NULL;
    } else {
        //
        //  Set the outstandingBoxListEntry so we can delete it next time
        //  getNext() is called or the list is deleted.
        //
        outstandingBoxListEntry = ble;
    }
}

//------------------------------------------------------------------------
//
//  Function:	getNext()
//
//  Description:
//	Returns the next set of boxes on the list.
//	
//  MT-level:  UNSAFE
//	
//  Parameters:
//	
//	
//  Returns:
//	TRUE:  More entries on the list.
//      FALSE: No more entries on the list.
//	
//  Side Effects:
//	Updated outstandingBoxListEntry to the next position, removes the
//      element from the box list.
//	
//------------------------------------------------------------------------
Xil_boolean
XilBoxList::getNext(XilBox** im1_box)
{
    //
    //  Are the calling the right function?
    //
    if(numSrcs+numDsts != 1) {
        XIL_ERROR(systemState, XIL_ERROR_SYSTEM, "di-365", TRUE);
        return FALSE;
    }

    setupNextBox();

    if(outstandingBoxListEntry == NULL) {
        return FALSE;
    } else {
        //
        //  Fill-in the boxes requested by the caller.
        //
        *im1_box = &outstandingBoxListEntry->boxes[0];

        return TRUE;
    }
}

Xil_boolean
XilBoxList::getNext(XilBox** im1_box,
                    XilBox** im2_box)
{
    //
    //  Are the calling the right function?
    //
    if(numSrcs+numDsts != 2) {
        XIL_ERROR(systemState, XIL_ERROR_SYSTEM, "di-365", TRUE);
        return FALSE;
    }

    setupNextBox();

    if(outstandingBoxListEntry == NULL) {
        return FALSE;
    } else {
        //
        //  Fill-in the boxes requested by the caller.
        //
        *im1_box = &outstandingBoxListEntry->boxes[0];
        *im2_box = &outstandingBoxListEntry->boxes[1];

        return TRUE;
    }
}

Xil_boolean
XilBoxList::getNext(XilBox** im1_box,
                    XilBox** im2_box,
                    XilBox** im3_box)
{
    //
    //  Are the calling the right function?
    //
    if(numSrcs+numDsts != 3) {
        XIL_ERROR(systemState, XIL_ERROR_SYSTEM, "di-365", TRUE);
        return FALSE;
    }

    setupNextBox();

    if(outstandingBoxListEntry == NULL) {
        return FALSE;
    } else {
        //
        //  Fill-in the boxes requested by the caller.
        //
        *im1_box = &outstandingBoxListEntry->boxes[0];
        *im2_box = &outstandingBoxListEntry->boxes[1];
        *im3_box = &outstandingBoxListEntry->boxes[2];

        return TRUE;
    }
}

Xil_boolean
XilBoxList::getNext(XilBox** im1_box,
                    XilBox** im2_box,
                    XilBox** im3_box,
                    XilBox** im4_box)
{
    //
    //  Are the calling the right function?
    //
    if(numSrcs+numDsts != 4) {
        XIL_ERROR(systemState, XIL_ERROR_SYSTEM, "di-365", TRUE);
        return FALSE;
    }

    setupNextBox();

    if(outstandingBoxListEntry == NULL) {
        return FALSE;
    } else {
        //
        //  Fill-in the boxes requested by the caller.
        //
        *im1_box = &outstandingBoxListEntry->boxes[0];
        *im2_box = &outstandingBoxListEntry->boxes[1];
        *im3_box = &outstandingBoxListEntry->boxes[2];
        *im4_box = &outstandingBoxListEntry->boxes[3];

        return TRUE;
    }
}

Xil_boolean
XilBoxList::getNextArray(XilBox* box_array[])
{
    setupNextBox();

    if(outstandingBoxListEntry == NULL) {
        return FALSE;
    } else {
        //
        //  Fill-in the array provided by the caller.
        //
        unsigned int num_boxes = numSrcs + numDsts;

        for(unsigned int i=0; i<num_boxes; i++) {
            box_array[i] = &outstandingBoxListEntry->boxes[i];
        }

        return TRUE;
    }
}

//------------------------------------------------------------------------
//
//  Function:	markAsFailed()
//
//  Description:
//	
//    This routine takes the current box on the list and marks it as
//    having failed.  This box entry remains on the list and a flag is
//    set indicating the box list contains failed boxes.
//
//    TODO: 2/16/96 jlf  Implement this functionality.
//
//    Currently, it returns XIL_FAILURE automatically and doesn't take
//    the time handle the list handling operations.
//	
//  MT-level:  Safe
// 
//  Returns:
//    XIL_SUCCESS:  Box list entry marked as failed, continue processing.
//    XIL_FAILURE:  Box list entry marked as failed, cease processing.
//    
//    
//  Side Effects:
//    Box list entries remain on the list and are not removed by the next
//    getNext() call.
//	
//------------------------------------------------------------------------
XilStatus
XilBoxList::markAsFailed()
{
    //
    //  Put the outstandingBoxListEntry onto the "failed" list.
    //
    if(outstandingBoxListEntry != NULL) {
        if(failedList.append(outstandingBoxListEntry) ==
           _XILI_SLLIST_INVALID_POSITION) {
            return XIL_FAILURE;
        }

        outstandingBoxListEntry = NULL;

        //
        //  If there are more boxes to do, then indicate that the compute
        //  routine can continue.  Otherwise, indicate that it should fail
        //  immediately. 
        //
        if(list.length() != 0) {
            return XIL_SUCCESS;
        } else {
            return XIL_FAILURE;
        }
    } else {
        //
        //  If there wasn't an outstanding box, then it's "by definition" a
        //  failure and things should stop here.
        //
        return XIL_FAILURE;
    }
}

XilStatus
XilBoxList::resetFromFailed()
{
    XiliSLListPosition blp;
    while((blp = failedList.head()) != _XILI_SLLIST_INVALID_POSITION) {
        XiliBoxListEntry* ble;

        //
        //  Remove it from the failedList...
        //
        if(failedList.remove(blp, ble) == XIL_FAILURE) {
            return XIL_FAILURE;
        }

        //
        //  And put it on the box list...
        //
        if(list.prepend(ble) == _XILI_SLLIST_INVALID_POSITION) {
            return XIL_FAILURE;
        }
    }

    //
    //  This handles the case where the compute routine didn't call
    //  markAsFailed() before returning.  It's kinda considered a bug, but we
    //  can work around it here by adding it back to the list.
    //
    if(outstandingBoxListEntry != NULL) {
        if(list.prepend(outstandingBoxListEntry) ==
           _XILI_SLLIST_INVALID_POSITION) {
            return XIL_FAILURE;
        }
    }

    outstandingBoxListEntry = NULL;

    return XIL_SUCCESS;
}

XilStatus
XilBoxList::addEntry(XiliBoxListEntry* new_ble)
{
    new_ble->boxCount = numSrcs+numDsts;

    if(list.append(new_ble) == _XILI_SLLIST_INVALID_POSITION) {
        //
        //  Oddly enough, insertion failed.
        //
        //  TODO: 2/26/96 jlf  Generate secondary failure?
        //
        return XIL_FAILURE;
    }

    return XIL_SUCCESS;
}
