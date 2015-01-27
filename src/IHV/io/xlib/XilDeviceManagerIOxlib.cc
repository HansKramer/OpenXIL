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
//  File:	XilDeviceManagerIOxlib.cc
//  Project:	XIL
//  Revision:	1.15
//  Last Mod:	10:13:48, 03/10/00
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
//  MT-level:  <SAFEc>
//	
//------------------------------------------------------------------------
//	COPYRIGHT
//------------------------------------------------------------------------
#pragma ident	"@(#)XilDeviceManagerIOxlib.cc	1.15\t00/03/10  "

#include "XilDeviceManagerIOxlib.hh"
#include "XilDeviceIOxlib.hh"

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

    XilDeviceManagerIOxlib* mgr = new XilDeviceManagerIOxlib;

    if(mgr == NULL) {
        XIL_ERROR(NULL, XIL_ERROR_RESOURCE, "di-1", TRUE);
        return NULL;
    }

    return mgr;
}

//
// Destructor
//
XilDeviceManagerIOxlib::~XilDeviceManagerIOxlib()
{
}

//
// Create a new device
//
XilDeviceIO*
XilDeviceManagerIOxlib::constructDisplayDevice(XilSystemState* state,
                                               Display*        display,
                                               Window          window)
{
    XilDeviceIOxlib* device;

    //
    //  We need to initialze Xlib so it will be MT-SAFE.
    //
    //  The Xlib spec says this must be done prior to any Xlib call being made
    //  by an application.   On Solaris, this has been fixed so it can be
    //  called at any time and from that point on, Xlib is expected to be
    //  MT-SAFE.
    //
    XInitThreads();

    device = new XilDeviceIOxlib(this, state, display, window);
    if(device == NULL) {
	XIL_ERROR(state, XIL_ERROR_RESOURCE, "di-1", TRUE);
	return NULL;
    }
    
    return device;
}


XilDeviceIO*
XilDeviceManagerIOxlib::constructDoubleBufferedDisplayDevice(XilSystemState* state,
                                                             Display*        display,
                                                             Window          window)
{
    XilDeviceIOxlib* device = NULL;

    device = (XilDeviceIOxlib *) constructDisplayDevice(state, display, window);
    if (device)
        device->setDoubleBuffered(TRUE);

    return device;
}

//
// Return the device name
//
const char*
XilDeviceManagerIOxlib::getDeviceName()
{
    return "SUNWxlib";
}

//
//  Describe the functions we implement to the XIL Core
//
XilStatus
XilDeviceManagerIOxlib::describeMembers()
{
    XilFunctionInfo*  func_info;

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "display_SUNWxlib");
    func_info->setFunction((XilIOFunctionPtr) &XilDeviceIOxlib::display);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "capture_SUNWxlib");
    func_info->setFunction((XilIOFunctionPtr) &XilDeviceIOxlib::capture);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "display_SUNWxlib");
    func_info->describeOp(XIL_STEP, 1, "copy;16");
    func_info->setPreprocessFunction((XilIOPreprocessFunctionPtr)
                                     &XilDeviceIOxlib::copyDisplayPreprocess);
    func_info->setFunction((XilIOFunctionPtr) &XilDeviceIOxlib::copyDisplay,
                           "display_SUNWxlib(copy;16())");
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "display_SUNWxlib");
    func_info->describeOp(XIL_STEP, 1, "copy;8");
    func_info->setPreprocessFunction((XilIOPreprocessFunctionPtr)
                                     &XilDeviceIOxlib::copyDisplayPreprocess);
    func_info->setFunction((XilIOFunctionPtr) &XilDeviceIOxlib::copyDisplay,
                           "display_SUNWxlib(copy;8())");
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "display_SUNWxlib");
    func_info->describeOp(XIL_STEP, 1, "copy;1");
    func_info->setPreprocessFunction((XilIOPreprocessFunctionPtr)
                                     &XilDeviceIOxlib::copyDisplayPreprocess);
    func_info->setFunction((XilIOFunctionPtr) &XilDeviceIOxlib::copyDisplay,
                           "display_SUNWxlib(copy;1())");
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "display_SUNWxlib");
    func_info->describeOp(XIL_STEP, 1, "set_value;8");
    func_info->setPreprocessFunction((XilIOPreprocessFunctionPtr)
                                     &XilDeviceIOxlib::setValueDisplayPreprocess);
    func_info->setFunction((XilIOFunctionPtr) &XilDeviceIOxlib::setValueDisplay,
                           "display_SUNWxlib(set_value;8())");
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "display_SUNWxlib");
    func_info->describeOp(XIL_STEP, 1, "cast;1->8");
    func_info->setPreprocessFunction((XilIOPreprocessFunctionPtr)
                                     &XilDeviceIOxlib::cast1to8DisplayPreprocess);
    func_info->setFunction((XilIOFunctionPtr) &XilDeviceIOxlib::cast1to8Display,
                           "display_SUNWxlib(cast;1->8())");
    addFunction(func_info);
    func_info->destroy();

    return XIL_SUCCESS;
}
