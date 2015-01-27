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
//  File:	XilDeviceManagerCompressionIdentity.hh
//  Project:	XIL
//  Revision:	1.3
//  Last Mod:	10:22:41, 03/10/00
//
//  Description:
//	This is the class that maintains the Identity compression
//	type information.  It is derived from the more generic
//	XilDeviceManagerCompression class and is responsible for
//	registering the attribute setting/getting functions for
//      Identity compression and decompression.
//
//      The class is also used to maintain information which is not
//      specific to any single instantiation of the Identity
//      compressor.  There will be only one instantiation of this
//      class for the Identity compression irregardless of how many
//      XilCis objects are created.
//	
//------------------------------------------------------------------------
#pragma ident	"@(#)XilDeviceManagerCompressionIdentity.hh	1.3\t00/03/10  "

#ifndef XILDEVICEMANAGERCOMPRESSIONIDENTITY_H
#define XILDEVICEMANAGERCOMPRESSIONIDENTITY_H

#include <xil/xilGPI.hh>

class XilDeviceManagerCompressionIdentity : public XilDeviceManagerCompression
{
public:
    XilDeviceCompression*  constructNewDevice(XilCis*  xcis);

    //
    // The required virtual functions from XilDeviceManager
    //
    const char* getDeviceName();
    XilStatus   describeMembers();

    XilDeviceManagerCompressionIdentity(void);
    ~XilDeviceManagerCompressionIdentity(void);
};

#endif  // XILDEVICEMANAGERCOMPRESSIONIDENTITY_H
