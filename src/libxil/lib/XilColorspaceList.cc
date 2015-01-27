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
//  File:	XilColorspaceList.cc
//  Project:	XIL
//  Revision:	1.9
//  Last Mod:	10:08:51, 03/10/00
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
#pragma ident	"@(#)XilColorspaceList.cc	1.9\t00/03/10  "

#include "_XilDefines.h"
#include "_XilSystemState.hh"
#include "_XilColorspaceList.hh"
#include "_XilColorspace.hh"
#include "XiliSLList.hh"

//
//  Returns a reference to the specified colorspace from the list.
//
XilColorspace*
XilColorspaceList::getColorspace(unsigned int entry_number)
{
    XilColorspace* ret_val;

    //
    //  If the requested entry is the next one we expected, just reference
    //  from the current node -- faster.
    //
    if(entry_number == nextQuery) {
        ret_val = list.reference(nextPosition);

        nextQuery++;
        nextPosition = list.next(nextPosition);
    } else {
        nextPosition = list.head();
	
        for(unsigned int i=0; i<entry_number; i++) {
            nextPosition = list.next(nextPosition);
        }

        ret_val      = list.reference(nextPosition);

        nextQuery    = i+1;
        nextPosition = list.next(nextPosition);
    }        

    return ret_val;       
}

//
//  Gets the number of colorspaces in the list.
//
unsigned int
XilColorspaceList::getNumColorspaces()
{
    return list.length();
}

//
//  Sets/Gets the data cache information.
//
void
XilColorspaceList::setCachedData(void*                      cache_data,
                                 XIL_FUNCPTR_DONE_WITH_DATA fptr)
{
    cachedData      = cache_data;
    doneWithDataPtr = fptr;
}
    
void*
XilColorspaceList::getCachedData()
{
    return cachedData;
}

XilObject*
XilColorspaceList::createCopy()
{
    //
    //  Since this object just maintains pointers to colorspaces, when it is
    //  copied we don't do a deep copy -- we just copy the pointers.
    //
    XilColorspace** cspace_array = new XilColorspace*[list.length()];
    if(cspace_array == NULL) {
        XIL_OBJ_ERROR(getSystemState(), XIL_ERROR_RESOURCE, "di-1", TRUE, this);
        return NULL;
    }

    XiliSLListIterator<XilColorspace*> li(&list);
    XilColorspace*                      cspace;
    int                                 i = 0;
    while(li.getNext(cspace) == XIL_SUCCESS) {
        cspace_array[i++] = cspace;
    }

    XilColorspaceList* new_copy =
        getSystemState()->createXilColorspaceList(cspace_array, list.length());
    if(new_copy == NULL) {
	XIL_OBJ_ERROR(getSystemState(), XIL_ERROR_SYSTEM, "di-170", FALSE, this);
	return NULL;
    }

    new_copy->copyVersionInfo(this);

    return new_copy;
}


