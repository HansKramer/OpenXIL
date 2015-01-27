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
//  File:	XilDeviceManagerIOxshm.hh
//  Project:	XIL
//  Revision:	1.2
//  Last Mod:	10:22:35, 03/10/00
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
//  MT-level:  <SAFE>
//	
//------------------------------------------------------------------------
//	COPYRIGHT
//------------------------------------------------------------------------
#pragma ident	"@(#)XilDeviceManagerIOxshm.hh	1.2\t00/03/10  "

#ifndef _XIL_DEVICE_MANAGER_IO_XSHM_HH
#define _XIL_DEVICE_MANAGER_IO_XSHM_HH

//
//  C++ Includes
//
#include <xil/xilGPI.hh>
#include "XilDeviceIOxshm.hh"

class XilDeviceManagerIOxshm : public XilDeviceManagerIO {
public:
    //
    // Creates a new XilDeviceIO object from a display pointer
    // and window.
    //
    XilDeviceIO*           constructDisplayDevice(XilSystemState* state,
                                                  Display*        display,
                                                  Window          window);

    XilDeviceIO*           constructDoubleBufferedDisplayDevice(XilSystemState* state,
                                                                Display*        display,
                                                                Window          window);
    //
    //  Required function that returns the name of this device.
    //
    const char*            getDeviceName();

    //
    //  describeMembers() is called by the XIL core to have the device
    //  manager provide XIL a description of the operations and functions it
    //  implements.
    //
    XilStatus              describeMembers();

    //
    // Destructor
    //
                           ~XilDeviceManagerIOxshm();

};
#endif // _XIL_DEVICE_MANAGER_IO_XSHM_HH


