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
//  File:	XilDeviceManagerStorageMemory.cc
//  Project:	XIL
//  Revision:	1.4
//  Last Mod:	10:16:27, 03/10/00
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
#pragma ident	"@(#)XilDeviceManagerStorageMemory.cc	1.4\t00/03/10  "

#include "XilDeviceManagerStorageMemory.hh"
#include "XilDeviceStorageMemory.hh"

XilDeviceManagerStorage*
XilDeviceManagerStorage::create(unsigned int  libxil_gpi_major,
                                unsigned int  libxil_gpi_minor,
                                unsigned int* devhandler_gpi_major,
                                unsigned int* devhandler_gpi_minor)
{
    XIL_BASIC_GPI_VERSION_TEST(libxil_gpi_major,
                               libxil_gpi_minor,
                               devhandler_gpi_major,
                               devhandler_gpi_minor);

    XilDeviceManagerStorageMemory* mgr = new XilDeviceManagerStorageMemory;

    if(mgr == NULL) {
        XIL_ERROR(NULL, XIL_ERROR_RESOURCE, "di-1", TRUE);
        return NULL;
    }

    return mgr;
}

XilDeviceStorage*
XilDeviceManagerStorageMemory::constructNewDevice(XilImage* image)
{
    return new XilDeviceStorageMemory(image);
}

XilDeviceManagerStorageMemory::~XilDeviceManagerStorageMemory()
{
}

const char*
XilDeviceManagerStorageMemory::getDeviceName()
{
    return "XilMemory";
}
