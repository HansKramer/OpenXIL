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
//  File:	_XilDeviceManagerIO.hh
//  Project:	XIL
//  Revision:	1.11
//  Last Mod:	10:21:45, 03/10/00
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
#pragma ident	"@(#)_XilDeviceManagerIO.hh	1.11\t00/03/10  "

#ifndef _XIL_DEVICE_MANAGER_IO_HH
#define _XIL_DEVICE_MANAGER_IO_HH

//
//  C++ Includes
//
#include "_XilDeviceManager.hh"
#include "_XilDeviceIO.hh"

class XilDeviceManagerIO : public XilDeviceManager {
public:
    //
    //  The create() function lives in every I/O pipeline and constructs
    //    a class derived from XilDeviceMangerIO.
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
    static XilDeviceManagerIO* create(unsigned int  libxil_gpi_major,
                                      unsigned int  libxil_gpi_minor,
                                      unsigned int* devhandler_gpi_major,
                                      unsigned int* devhandler_gpi_minor);

    //
    //  The derived I/O device manager should overload only one of these
    //  construct*Device() calls because they are distinct in which API call
    //  uses them.  Devices can only be constructed one way.
    //

    //
    //  Creates a new XilDeviceIO object for a particular device.
    //
    virtual XilDeviceIO*       constructFromDevice(XilSystemState* state,
                                                   XilDevice*      device);

#if defined(_XIL_HAS_X11WINDOWS) || defined(_WINDOWS)
    //
    //  Creates a new XilDeviceIO object from a display pointer
    //  and window.
    //
    virtual XilDeviceIO*       constructDisplayDevice(XilSystemState* state,
                                                      Display*        display,
                                                      Window          window);

    //
    //  Creates a new XilDeviceIO object which supports double buffering from
    //  a display pointer and window.  At construction, the device is expected
    //  to be referencing the BACK buffer of the two buffers.
    //
    virtual XilDeviceIO*
                constructDoubleBufferedDisplayDevice(XilSystemState* state,
                                                     Display*        display,
                                                     Window          window);

    //
    // Creates a new XilDeviceIO object which supports stereo display
    // from a display pointer and window. On creation, the device is
    // expected to be referencing the LEFT buffer.
    //
    virtual XilDeviceIO*
                constructSpecialDisplayDevice(XilSystemState* state,
                                              Display*        display,
                                              Window          window,
                                              XilWindowCaps   wincaps);


#endif // _XIL_HAS_X11WINDOWS || _WINDOWS
    
protected:
    //
    //  Base Constructor/Destructor
    //
                               XilDeviceManagerIO();
    virtual                    ~XilDeviceManagerIO();

private:
    //
    //  Base member data
    //
    void*                      _extra_data[256];
};

#endif  // _XIL_DEVICE_MANAGER_IO_HH
