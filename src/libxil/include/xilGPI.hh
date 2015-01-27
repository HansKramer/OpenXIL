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
//  File:	xilGPI.hh
//  Project:	XIL
//  Revision:	1.34
//  Last Mod:	10:20:56, 03/10/00
//
//  Description:
//	
//	The XIL Graphics Porting Interface (GPI).
//	
//------------------------------------------------------------------------
//	COPYRIGHT
//------------------------------------------------------------------------
#ifdef _WINDOWS
#pragma warning ( disable : 4068 )
#else
#pragma ident	"@(#)xilGPI.hh	1.34\t00/03/10  "
#endif

#ifndef _XIL_GPI_HH
#define _XIL_GPI_HH

//
//  Define this to indicate to the include files to export the C++ GPI versus
//    the C++ API.
//
#define XIL_GPI

//
//  GPI Defines
//

#ifdef _WINDOWS
#include <windows.h>
#ifndef XILI_PATH_MAX
#ifdef _WINDOWS
#define XILI_PATH_MAX    FILENAME_MAX
#else
#define XILI_PATH_MAX    PATH_MAX
#endif
#endif
#endif
#include <xil/_XilDefines.h>
#include <xil/_XilGPIDefines.hh>

//
//  The XIL Global State and System State
//
#include <xil/_XilGlobalState.hh>
#include <xil/_XilSystemState.hh>

//
//  XIL Objects
//
#include <xil/_XilImage.hh>
#include <xil/_XilCis.hh>
#include <xil/_XilColorspace.hh>
#include <xil/_XilColorspaceList.hh>
#include <xil/_XilDitherMask.hh>
#include <xil/_XilLookupSingle.hh>
#include <xil/_XilLookupColorcube.hh>
#include <xil/_XilLookupCombined.hh>
#include <xil/_XilKernel.hh>
#include <xil/_XilHistogram.hh>
#include <xil/_XilInterpolationTable.hh>
#include <xil/_XilRoi.hh>
#include <xil/_XilSel.hh>
#include <xil/_XilStorage.hh>

//
//  Compute Interface Classes
//
#include <xil/_XilFunctionInfo.hh>
#include <xil/_XilConvexRegionList.hh>
#include <xil/_XilScanlineList.hh>
#include <xil/_XilOp.hh>
#include <xil/_XilRectList.hh>
#include <xil/_XilBox.hh>
#include <xil/_XilBoxList.hh>
#include <xil/_XilTile.hh>
#include <xil/_XilTileList.hh>

//
//  Compression Interface Classes
//
#include <xil/_XilCisBuffer.hh>
#include <xil/_XilCisBufferManager.hh>

//
//  MT Support Classes
//
#include <xil/_XilMutex.hh>
#include <xil/_XilCondVar.hh>

//
//  Device Managers
//
#include <xil/_XilDeviceManagerCompute.hh>
#include <xil/_XilDeviceManagerStorage.hh>
#include <xil/_XilDeviceManagerCompression.hh>
#include <xil/_XilDeviceManagerIO.hh>

//
//  Devices
//
#include <xil/_XilDeviceStorage.hh>
#include <xil/_XilDeviceCompression.hh>
#include <xil/_XilDeviceIO.hh>
#include <xil/_XilDevice.hh>

#endif // _XIL_GPI_HH
