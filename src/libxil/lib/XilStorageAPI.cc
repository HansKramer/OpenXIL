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
//  File:	XilStorageAPI.cc
//  Project:	XIL
//  Revision:	1.13
//  Last Mod:	10:08:47, 03/10/00
//
//  Description:
//	
//	
//	
//------------------------------------------------------------------------
//	COPYRIGHT
//------------------------------------------------------------------------
#pragma ident	"@(#)XilStorageAPI.cc	1.13\t00/03/10  "

#include "_XilDefines.h"
#include "XilStorageAPI.hh"

#include "_XilSystemState.hh"
#include "_XilImage.hh"

XilStorageAPI::XilStorageAPI(XilSystemState* state,
                             XilImage*       init_image) :
        XilStorage(init_image),
        XilNonDeferrableObject(state, XIL_STORAGE)
{
    isOKFlag = FALSE;

    //
    //  If the image is either NULL or is a temporary_image
    //  then generate an error and leave isOK as FALSE
    //
    if((init_image == NULL) || (init_image->isTemp())) {
        XIL_OBJ_ERROR(this->getSystemState(), XIL_ERROR_USER, "di-388", TRUE, this);
        //
        //  Nothing needs destruction because this will get caught
        //  in XilSystemState from the isOK flag.
        //
    } else {
        isOKFlag = TRUE;
    }
}

XilStorageAPI::~XilStorageAPI()
{
}

//
//  Create a copy of this storage object
//
XilObject*
XilStorageAPI::createCopy()
{
    XilStorageAPI* new_copy =
        getSystemState()->createXilStorageAPI(getImage());
    if(new_copy == NULL) {
        XIL_ERROR(this->getSystemState(), XIL_ERROR_SYSTEM, "di-441", FALSE);
	return NULL;
    }

    new_copy->copyVersionInfo(this);

    return new_copy;
}



