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
//  File:	XilDeviceIOPull.hh
//  Project:	XIL
//  Revision:	1.4
//  Last Mod:	10:21:42, 03/10/00
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
#pragma ident	"@(#)XilDeviceIOPull.hh	1.4\t00/03/10  "

#ifndef _XIL_DEVICE_IO_PULL_HH
#define _XIL_DEVICE_IO_PULL_HH

//
// C++ Includes
//
#include "XilDeviceIO.hh"

class XilDeviceIOPull : public XilDeviceIO {
public:
    //
    //  Capture an image from the device
    //
    virtual XilStatus    capture(XilOp*       op,
                                 unsigned int op_count,
                                 XilRoi*      roi,
                                 XilBoxList*  bl);
	
    //
    //  Constructor....
    //
                         XilDeviceIOPull(XilDeviceManagerIO* device_manager);

    //
    //  Destructor....
    //
    virtual              ~XilDeviceIOPull();

private:
    //
    // Returns XILI_IO_PULL
    //
    XiliIOType           getType();

    //
    // Base member data
    //
    void*        _extra_data[256];
};

#endif // _XIL_DEVICE_IO_PULL_HH
