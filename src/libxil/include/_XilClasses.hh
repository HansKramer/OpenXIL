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
//  File:	_XilClasses.hh
//  Project:	XIL
//  Revision:	1.30
//  Last Mod:	10:21:55, 03/10/00
//
//  Description:
//	A forward declaration of XIL classes.
//	
//------------------------------------------------------------------------
//	COPYRIGHT
//------------------------------------------------------------------------
#pragma ident	"@(#)_XilClasses.hh	1.30\t00/03/10  "

#ifndef _XIL_CLASSES_HH
#define _XIL_CLASSES_HH

//
//  Forward Class Declarations
//
class XilGlobalState;
class XilSystemState;

//
//  XIL Objects
//
class XilObject;
class XilDeferrableObject;
class XilNonDeferrableObject;

class XilCis;
class XilColorspace;
class XilColorspaceList;
class XilDitherMask;
class XilHistogram;
class XilImageFormat;
class XilImage;
class XilInterpolationTable;
class XilKernel;
class XilLookupSingle;
class XilLookupColorcube;
class XilLookupCombined;
class XilRoi;
class XilSel;
class XilStorage;
class XilStorageAPI;

//
//  Device Managers
//
class XilDeviceManager;
class XilDeviceManagerCompute;
class XilDeviceManagerStorage;
class XilDeviceManagerCompression;
class XilDeviceManagerIO;

//
//  Devices
//
class XilDevice;
class XilDeviceCompute;
class XilDeviceStorage;
class XilDeviceCompression;
class XilDeviceIO;

//
//  Compute/IO/Compress Function Description
//
class XilFunctionInfo;

//
//  Op Manipulation
//
class XilOp;

//
//  ROI support 
//
class XilBox;
class XilBoxList;
class XilRectList;
class XilConvexRegionList;


//
//  Thread Abstraction
//
class XilMutex;
class XilCondVar;

//
//  Miscelaneous
//
class XilTile;
class XilTileList;

#ifdef _XIL_LIBXIL_PRIVATE

#include "XilClassesPrivate.hh"

#endif // _XIL_LIBXIL_PRIVATE

#endif // _XIL_CLASSES_HH
