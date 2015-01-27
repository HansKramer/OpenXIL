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

//This line lets emacs recognize this as -*- C++ -*- Code
//------------------------------------------------------------------------
//
//  File:	XilDeviceManagerStorageMemory.hh
//  Project:	XIL
//  Revision:	1.2
//  Last Mod:	10:23:55, 03/10/00
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
#pragma ident	"@(#)XilDeviceManagerStorageMemory.hh	1.2\t00/03/10  "

//
//  Include XIL GPI
//
#include <xil/xilGPI.hh>

class XilDeviceManagerStorageMemory : public XilDeviceManagerStorage {
    //
    //  Creates a new XilDeviceStorage object of this type for the given
    //     XilImage.
    //
    XilDeviceStorage*       constructNewDevice(XilImage* image);

    //
    //  Required function that returns the name of this device.
    //
    const char* getDeviceName();
    
    //
    //  This releases all the resources used by the memory device manager.
    //
    ~XilDeviceManagerStorageMemory();
};
