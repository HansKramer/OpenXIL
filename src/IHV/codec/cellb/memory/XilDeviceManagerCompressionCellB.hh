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
//  File:   XilDeviceManagerCompressionCellB.hh
//  Project:    XIL
//  Revision:   1.3
//  Last Mod:   10:23:24, 03/10/00
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
//  COPYRIGHT
//------------------------------------------------------------------------
#pragma ident   "@(#)XilDeviceManagerCompressionCellB.hh	1.3\t00/03/10  "


#ifndef _XIL_DEVICE_MANAGER_COMPRESSION_CELLB_HH
#define _XIL_DEVICE_MANAGER_COMPRESSION_CELLB_HH

#include <xil/xilGPI.hh>
#include "CellBManagerCompressorData.hh"
#include "Ycc2RgbConverter.hh"
#include "XiliOrderedDitherLut.hh"

class XilDeviceManagerCompressionCellB : public XilDeviceManagerCompression 
{
public:
    //
    //  REQUIRED PURE VIRTUAL IMPLEMENTATIONS
    //  (from XilDeviceManager)
    //
    const char* getDeviceName();
    XilStatus   describeMembers();

    //
    //  REQUIRED PURE VIRTUAL IMPLEMENTATION 
    //  (from XilDeviceManagerCompression)
    //
    XilDeviceCompression* constructNewDevice(XilCis* xcis);

    //
    // Constructor/deconstructor
    //
    XilDeviceManagerCompressionCellB();
    ~XilDeviceManagerCompressionCellB();

    CellBManagerCompressorData compmgrData;

    //
    // Create or retrieve a color conversion, dither table
    // and rescale table objects
    //
    Ycc2RgbConverter*     getColorConverter();

private:
    //
    // Pointers to the YCbCr to Rgb709 conversion table
    // and the ordered dither lookup table
    //
    Ycc2RgbConverter*     colorConverter;

    XilMutex              mutex;

};

#endif // _XIL_DEVICE_MANAGER_COMPRESSION_CELLB_HH
