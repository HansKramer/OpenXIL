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
//  File:	XilDeviceIOPull.cc
//  Project:	XIL
//  Revision:	1.5
//  Last Mod:	10:08:55, 03/10/00
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
#pragma ident	"@(#)XilDeviceIOPull.cc	1.5\t00/03/10  "

#include "_XilDefines.h"
#include "XilDeviceIOPull.hh"

XilDeviceIOPull::XilDeviceIOPull(XilDeviceManagerIO* device_manager) :
    XilDeviceIO(device_manager)
{
}

XilDeviceIOPull::~XilDeviceIOPull()
{
}

XiliIOType
XilDeviceIOPull::getType()
{
    return XILI_IO_PULL;
}

XilStatus
XilDeviceIOPull::capture(XilOp*       ,
                         unsigned int ,
                         XilRoi*      ,
                         XilBoxList*  )
{
    return XIL_FAILURE;
}
