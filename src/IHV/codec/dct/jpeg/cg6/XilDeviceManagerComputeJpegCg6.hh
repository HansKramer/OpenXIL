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
//  File:       XilDeviceManagerComputeJpegCg6.hh
//  Project:    XIL
//  Revision:   1.3
//  Last Mod:   10:22:47, 03/10/00
//
//  Description:
//
//    TODO: Enter some descriptive text here
//
//
//------------------------------------------------------------------------
//    COPYRIGHT
//------------------------------------------------------------------------
#pragma ident   "@(#)XilDeviceManagerComputeJpegCg6.hh	1.3\t00/03/10  "

#ifndef _XIL_DEVICE_MANAGER_COMPUTE_JPEG_CG6_HH 
#define _XIL_DEVICE_MANAGER_COMPUTE_JPEG_CG6_HH 

#include <xil/xilGPI.hh>

class XilDeviceManagerComputeJpegCg6 : public XilDeviceManagerCompute {
public:

    //
    // Constructor/deconstructor
    //
    XilDeviceManagerComputeJpegCg6();
    ~XilDeviceManagerComputeJpegCg6();
  
    //
    //  REQUIRED PURE VIRTUAL IMPLEMENTATIONS
    //  (from XilDeviceManager)
    //
    const char* getDeviceName();
    XilStatus   describeMembers();

    XilStatus decompressDither8Cg6(XilOp*      op,
                                   unsigned    op_count,
                                   XilRoi*     roi,
                                   XilBoxList* bl);

};

#endif // _XIL_DEVICE_MANAGER_COMPUTE_JPEG_CG6_HH 
