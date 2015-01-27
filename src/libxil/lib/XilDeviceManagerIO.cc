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
//  File:	XilDeviceManagerIO.cc
//  Project:	XIL
//  Revision:	1.10
//  Last Mod:	10:08:52, 03/10/00
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
//------------------------------------------------------------------------
//	COPYRIGHT
//------------------------------------------------------------------------
#pragma ident	"@(#)XilDeviceManagerIO.cc	1.10\t00/03/10  "

#include "_XilDefines.h"
#include "_XilDeviceManagerIO.hh"

XilDeviceManagerIO::XilDeviceManagerIO()
{
}

XilDeviceManagerIO::~XilDeviceManagerIO()
{
}

#if defined(_XIL_HAS_X11WINDOWS) || defined(_WINDOWS)
//
//  Creates a new XilDeviceIO object from a display pointer
//  and window.
//
XilDeviceIO*
XilDeviceManagerIO::constructDisplayDevice(XilSystemState* ,
                                           Display*        ,
                                           Window          )
{
    return NULL;
}

//
//  Creates a new XilDeviceIO object which supports double buffering from
//  a display pointer and window.
//
XilDeviceIO*
XilDeviceManagerIO::constructDoubleBufferedDisplayDevice(XilSystemState* ,
                                                         Display*        ,
                                                         Window          )
{
    return NULL;
}

//
//  Creates a new XilDeviceIO object which supports stereo and other
//  enhance display capabilities from a display pointer and window.
//
XilDeviceIO*
XilDeviceManagerIO::constructSpecialDisplayDevice(XilSystemState* ,
                                                  Display*        ,
                                                  Window          ,
                                                  XilWindowCaps   )
{
    return NULL;
}


#endif // _XIL_HAS_X11WINDOWS || _WINDOWS
    
//
// Creates a new XilDeviceIO object for a particular device.
//
XilDeviceIO*
XilDeviceManagerIO::constructFromDevice(XilSystemState* ,
                                        XilDevice*      )
{
    return NULL;
}

