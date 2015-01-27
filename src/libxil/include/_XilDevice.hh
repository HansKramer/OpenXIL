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
//  File:	_XilDevice.hh
//  Project:	XIL
//  Revision:	1.9
//  Last Mod:	10:21:40, 03/10/00
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
//  MT-level:  UNSAFE
//	
//------------------------------------------------------------------------
//	COPYRIGHT
//------------------------------------------------------------------------
#pragma ident	"@(#)_XilDevice.hh	1.9\t00/03/10  "

#ifndef _XIL_DEVICE_H
#define _XIL_DEVICE_H

//
//  System Includes
//

//
//  XIL Includes
//
#include "_XilDefines.h"
#include "_XilNonDeferrableObject.hh"

//
//  Structure which contains the key/value pair of the attribute.
//
struct XilAttributeData {
    const char*  name;
    void*        value;
};

class XilDevice : public XilNonDeferrableObject {
public:
    //
    //  Returns the name of the device.
    //
    const char*             getDeviceName();

    //
    //  Provides a list of attributes that are active and set on this device
    //  for processing.
    //
    const XilAttributeData* getAttributes(unsigned int* num_attrs);

    
private:
#ifdef _XIL_LIBXIL_PRIVATE
#define _XIL_PRIVATE_DATA

#include "XilDevicePrivate.hh"

#undef _XIL_PRIVATE_DATA
#endif // _XIL_PRIVATE_DATA
};

#endif // _XIL_DEVICE_H
