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
//  File:	XilDeviceStorage.cc
//  Project:	XIL
//  Revision:	1.17
//  Last Mod:	10:08:24, 03/10/00
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
#pragma ident	"@(#)XilDeviceStorage.cc	1.17\t00/03/10  "

#include "_XilDefines.h"
#include "_XilDeviceStorage.hh"
#include "_XilStorage.hh"
#include "_XilBox.hh"
#include "XiliUtils.hh"

XilDeviceStorage::XilDeviceStorage(XilImage* parent_image)
{
    image = parent_image;
}

XilStatus
XilDeviceStorage::modifyForEmulation(XilStorage*  ,
                                     const char*  ,
                                     void*        ,
                                     unsigned int ,
                                     unsigned int )
{
    return XIL_FAILURE;
}

XilStatus
XilDeviceStorage::transferToDescription(XilStorage*  ,
                                        const char*  ,
                                        XilStorage*  ,
                                        const char*  ,
                                        void*        ,
                                        unsigned int ,
                                        unsigned int ,
                                        Xil_boolean* )
{
    return XIL_FAILURE;
}
    
XilStatus
XilDeviceStorage::setStorage(XilStorage*  new_storage,
                             XilStorage*  tile_storage,
                             int          x_offset,
                             int          y_offset,
                             int          band_offset)
{
    //
    //  By default, use the utility routine that handles all of the storage
    //  cases possible for the XilStorage object.
    //
    return new_storage->setInfoFromStorage(tile_storage,
                                           x_offset, y_offset, band_offset);
}

//
//  getBoxStorageLocation is an exposed routine that allows
//  a storage device to get information without opening
//  access onto the private XilBox::getStorageLocation routine.
//
void
XilDeviceStorage::getBoxStorageLocation(XilBox*       box,
                                        int*          x,
                                        int*          y,
                                        unsigned int* xsize,
                                        unsigned int* ysize,
                                        int*          band)
{
    box->getStorageLocation(x, y, xsize, ysize, band);
}

XilStatus
XilDeviceStorage::willNeed(XilStorage*  ,
                           unsigned int ,
                           unsigned int ,
                           unsigned int ,
                           unsigned int ,
                           unsigned int ,
                           unsigned int )
{
    return XIL_SUCCESS;
}

void
XilDeviceStorage::dontNeed(XilStorage*  ,
                           unsigned int ,
                           unsigned int ,
                           unsigned int ,
                           unsigned int ,
                           unsigned int ,
                           unsigned int )
{
}
