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
//  File:	XilDeviceManagerIOxshm.cc
//  Project:	XIL
//  Revision:	1.8
//  Last Mod:	10:14:00, 03/10/00
//  SID:        %Z% %F% %I% %U% %E%
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
#pragma ident	"@(#)XilDeviceManagerIOxshm.cc	1.8\t00/03/10  "

#include "XilDeviceManagerIOxshm.hh"
#include "XilDeviceIOxshm.hh"

#if XlibSpecificationRelease < 6
//
//  There is no prototype for this in R5 header files.
//  but there is in R6. XlibSpecificationRelease is
//  defined in Xlib.h.
//
//  Needs more work if we ever build on something less
//  than R5.
//
//  X11 specifies the XInitThreads must be called prior to ANY other Xlib call
//  being made by the application.  For a library like XIL, this is
//  impossible since we don't know when the application started calling Xlib
//  calls (as most likely happens prior to xil_open()).  On Solaris, we have
//  decided to fix this ourselves since the reason for the requirement seems
//  fairly arbitrary and short-sided at best.  On other platforms, this may be
//  a problem.
//
extern "C" {
    Status XInitThreads(void);
}
#endif

#include <stdio.h>

//
// Create a new instance of the device manager
//
XilDeviceManagerIO*
XilDeviceManagerIO::create(unsigned int  libxil_gpi_major,
                           unsigned int  libxil_gpi_minor,
                           unsigned int* devhandler_gpi_major,
                           unsigned int* devhandler_gpi_minor)
{
    XIL_BASIC_GPI_VERSION_TEST(libxil_gpi_major,
                               libxil_gpi_minor,
                               devhandler_gpi_major,
                               devhandler_gpi_minor);

    XilDeviceManagerIOxshm* mgr = new XilDeviceManagerIOxshm;

    if(mgr == NULL) {
        XIL_ERROR(NULL, XIL_ERROR_RESOURCE, "di-1", TRUE);
        return NULL;
    }

    //
    //  We need to initialze Xlib so it will be MT-SAFE.
    //
    //  The Xlib spec says this must be done prior to any Xlib call being made
    //  by an application.   On Solaris, this has been fixed so it can be
    //  called at any time and from that point on, Xlib is expected to be
    //  MT-SAFE.
    //
    XInitThreads();

    return mgr;
}

//
// Destructor
//
XilDeviceManagerIOxshm::~XilDeviceManagerIOxshm()
{
}

//
//  We don't seem to delare these in XShm.h
//
extern "C" {
    Bool XShmQueryExtension(Display* display);
    int  XShmGetEventBase(Display* display);
}


XilDeviceIO*
XilDeviceManagerIOxshm::constructDoubleBufferedDisplayDevice(XilSystemState* state,
                                                             Display*        display,
                                                             Window          window)
{
    XilDeviceIOxshm *device =(XilDeviceIOxshm *) constructDisplayDevice(state, display, window);
    if (device)
        device->setDoubleBuffered(TRUE);

    return device;
}


//
// Create a new device
//
XilDeviceIO*
XilDeviceManagerIOxshm::constructDisplayDevice(XilSystemState* state,
                                               Display*        display,
                                               Window          window)
{
    //
    //  Determine if the X Shared Memory Extension is available on the display
    //  before proceeding further.
    //
    Status status = XShmQueryExtension(display);

    if(status == False) {
        //
        //  We can't so we'll have to use Xlib only
        //
        return NULL;
    } else {
        int firstEvent;
        int firstError;

        if(! XQueryExtension(display, "MIT-SHM",
                             &xili_xshm_attach_info.major_code,
                             &firstEvent, &firstError)) {
            return NULL;
        }
    }

    //
    //  Actually construct the device...
    //
    XilDeviceIOxshm* device =
        new XilDeviceIOxshm(this, state, display, window);
    if(device == NULL) {
	XIL_ERROR(state, XIL_ERROR_RESOURCE, "di-1", TRUE);
	return NULL;
    }

    if(! device->isOK()) {
        delete device;
        return NULL;
    }

    return device;
}

//
// Return the device name
//
const char*
XilDeviceManagerIOxshm::getDeviceName()
{
    return "SUNWxshm";
}

//
//  Describe the functions we implement to the XIL Core
//
XilStatus
XilDeviceManagerIOxshm::describeMembers()
{
    XilFunctionInfo*  func_info;

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "display_SUNWxshm");
    func_info->setFunction((XilIOFunctionPtr) &XilDeviceIOxshm::display);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "capture_SUNWxshm");
    func_info->setFunction((XilIOFunctionPtr) &XilDeviceIOxshm::capture);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "display_SUNWxshm");
    func_info->describeOp(XIL_STEP, 1, "set_value;8");
    func_info->setPreprocessFunction((XilIOPreprocessFunctionPtr)
                                     &XilDeviceIOxshm::setValueDisplayPreprocess);
    func_info->setFunction((XilIOFunctionPtr) &XilDeviceIOxshm::setValueDisplay,
                           "display_SUNWxshm(set_value;8())");
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "display_SUNWxshm");
    func_info->describeOp(XIL_STEP, 1, "cast1->8");
    func_info->setPreprocessFunction((XilIOPreprocessFunctionPtr)
                                     &XilDeviceIOxshm::cast1to8DisplayPreprocess);
    func_info->setFunction((XilIOFunctionPtr) &XilDeviceIOxshm::cast1to8Display,
                           "display_SUNWxshm(cast1->8())");
    addFunction(func_info);
    func_info->destroy();

    return XIL_SUCCESS;
}
