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
//  File:	XilDeviceIOPush.hh
//  Project:	XIL
//  Revision:	1.4
//  Last Mod:	10:21:41, 03/10/00
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
#pragma ident	"@(#)XilDeviceIOPush.hh	1.4\t00/03/10  "

#ifndef _XIL_DEVICE_IO_PUSH_HH
#define _XIL_DEVICE_IO_PUSH_HH

//
// C++ Includes
//
#include "XilDeviceIO.hh"

class XilDeviceIOPush : public XilDeviceIO {
public:
    //
    //  Start pushing data for a new capture.
    //
    virtual XilStatus  startCapture() = 0;

    //
    //  Stop pushing data for the currently running capture.  This is not a
    //  pause, it is more like a "stop and reset".
    //
    virtual XilStatus  stopCapture() = 0;

    //
    // Constructor
    //
                       XilDeviceIOPush(XilDeviceManagerIO* device_manager);

    //
    // Destructor
    //
    virtual            ~XilDeviceIOPush();

protected:
    //
    //  Give XIL the data which has been captured and is now ready to be
    //  passed on down the line.   Derived classes call this method when
    //  performing a capture to to present each buffer of new data to the XIL
    //  core.
    //
    //  The regions must be non-overlapping.  If the same region or a portion
    //  of a previously provided region is given to XIL, this function returns
    //  XIL_FAILURE and stopCapture() will be called.  The application will be
    //  told that the device failed to perform the capture.
    //
    XilStatus          giveData(XilStorage*  storage,
                                unsigned int x,
                                unsigned int y,
                                unsigned int xsize,
                                unsigned int ysize);

private:
    //
    // Returns XILI_IO_PUSH
    //
    XiliIOType        getType();

    //
    // Base member data
    //
    void*             _extra_data[256];
};

#endif // _XIL_DEVICE_IO_PUSH_HH
