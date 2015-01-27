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
//  File:	_XilDeviceManagerStorage.hh
//  Project:	XIL
//  Revision:	1.8
//  Last Mod:	10:22:12, 03/10/00
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
//  MT-level:  SAFE
//	
//------------------------------------------------------------------------
//	COPYRIGHT
//------------------------------------------------------------------------
#pragma ident	"@(#)_XilDeviceManagerStorage.hh	1.8\t00/03/10  "

#ifndef _XIL_DEVICE_MANAGER_STORAGE_HH
#define _XIL_DEVICE_MANAGER_STORAGE_HH

//
//  C++ Includes
//
#include "_XilDeviceManager.hh"
#include "_XilDeviceStorage.hh"

class XilDeviceManagerStorage : public XilDeviceManager {
public:
    //
    //  The create() function lives in every storage pipeline and constructs
    //    a class derived from XilDeviceMangerStorage.
    //
    //  XIL provides the pipeline with the highest major and minor version
    //  numbers of the GPI it supports.  The compute pipeline is expected to
    //  fail if the version is not one that is supported by the pipeline.  For
    //  example, there is a mismatch in the major version numbers or the minor
    //  version is lower than the one required by the pipeline.
    //
    //  At the same time, the compute pipeline is expected to provide XIL with
    //  the highest version of the GPI it supports.  XIL may decide not to
    //  load the pipeline or it may decide to alter its behavior in order to
    //  support an older version of the interface.
    //
    static XilDeviceManagerStorage* create(unsigned int  libxil_gpi_major,
                                           unsigned int  libxil_gpi_minor,
                                           unsigned int* devhandler_gpi_major,
                                           unsigned int* devhandler_gpi_minor);

    //
    //  Creates a new XilDeviceStorage object of this type for the given
    //     XilImage.
    //
    virtual XilDeviceStorage*       constructNewDevice(XilImage* image)=0;


protected:
    //
    //  Base Constructor/Destructor
    //
                                     XilDeviceManagerStorage();
    virtual                          ~XilDeviceManagerStorage();

private:
    //
    //  The describeMembers function which is not needed by classes derived
    //  from XilDeviceManagerStorage.
    //
    XilStatus                        describeMembers();

    //
    //  Base member data
    //
    void*                            _extra_data[256];
};

#endif  // _XIL_DEVICE_MANAGER_STORAGE_HH
