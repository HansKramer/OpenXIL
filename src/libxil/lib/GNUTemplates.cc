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
//  File:	GNUTemplates.cc
//  Project:	XIL
//  Revision:	1.15
//  Last Mod:	10:08:48, 03/10/00
//
//  Description:
//	This file contains static template instantiations which is 
//      required by the GNU C++ compiler.
//	
//	The headerfiles and instantiations are order-dependent to 
//	avoid linking problems.
//	
//	
//	
//	
//------------------------------------------------------------------------
//	COPYRIGHT
//------------------------------------------------------------------------
#pragma ident	"@(#)GNUTemplates.cc	1.15\t00/03/10  "

#include "_XilDefines.h"

//
//  Used in XilSystemState
//
#include "_XilSystemState.hh"

//
//  XiliHashTable
//
#include "XiliHashTable.hh"
#include "XiliHashTable.cc"

template class XiliHashTable<void*>;


//
//  Used in XilGlobalState
//
#include "_XilGlobalState.hh"

template class XiliHashTable<XiliDeviceRecord*>;
template class XiliHashTable<XiliOpOpenRecord*>;
template class XiliHashTable<XiliOpRecord*>;
template class XiliHashTable<XiliOpNameRecord*>;

//
//  Used in XilDeviceManagerCompression
//
#include "_XilDeviceManagerCompression.hh"

template class XiliHashTable<AttrRecord*>;

#include "XiliProcessEnv.hh"
template class XiliHashTable<XiliEnvRecord*>;

//  XiliList
//
#include "XiliList.hh"
#include "XiliList.cc"

//
//  Used in XilCisBufferManager
//
#include "_XilCisBufferManager.hh"

template class XiliList<XilCisBuffer>;
template class XiliListEntry<XilCisBuffer>;

//
//  Used in XilCisBuffer
//
#include "_XilCisBuffer.hh"

template class XiliList<XiliFrameInfo>;
template class XiliListEntry<XiliFrameInfo>;

//
//  Used in XilConvexRegionList
//
#include "_XilConvexRegionList.hh"

template class XiliList<XiliConvexRegion>;
template class XiliListEntry<XiliConvexRegion>;

//
//  Used in XilFunctionInfo
//
#include "_XilFunctionInfo.hh"

template class XiliList<XiliDirection>;
template class XiliListEntry<XiliDirection>;

//
//  Used in XilGlobalState
//
#include "_XilGlobalState.hh"

template class XiliList<XilSystemState>;
template class XiliListEntry<XilSystemState>;

//
//  Used in XilOp
//
#include "_XilOp.hh"

template class XiliList<XiliMarker>;
template class XiliListEntry<XiliMarker>;

//
//  Used in XilRoi
//
#include "_XilRoi.hh"

template class XiliList<XiliRectInt>;
template class XiliListEntry<XiliRectInt>;

//
//  Used in XiliScheduler
//
#include "XiliScheduler.hh"

template class XiliList<XiliExecContext>;
template class XiliListEntry<XiliExecContext>;
template class XiliList<XiliOperation>;
template class XiliListEntry<XiliOperation>;

//
//  Iterators
//
template class XiliListIterator<XiliConvexRegion>;
template class XiliListIterator<XilSystemState>;
template class XiliListIterator<XiliExecContext>;
template class XiliListIterator<XiliMarker>;

//
//  XiliSLList
//
#include "XiliSLList.hh"
#include "XiliSLList.cc"

//
//  Used in XilBoxList
//
#include "_XilBoxList.hh"

template class XiliSLList<XiliBoxListEntry*>;

//
//  Used in XilColorspaceList
//
#include "_XilColorspaceList.hh"

template class XiliSLList<XilColorspace*>;

//
//  Used in XilImage
//
#include "_XilImage.hh"

template class XiliSLList<XiliStorageRecord*>;

//
//  Used in XilSystemState
//
#include "_XilSystemState.hh"

template class XiliSLList<XilObject*>;
template class XiliSLList<XilErrorFunc>;

//
//  Used in XiliScheduler
//
#include "XiliScheduler.hh"

template class XiliSLList<XilTileNumber>;
template class XiliSLList<XiliExecContext*>;

//
//  Used in ../ops/XilOpGeometricAffine
//
#include "XiliConvexRegion.hh"

template class XiliSLList<XiliConvexRegion*>;

//
//  Iterators
//
template class XiliSLListIterator<XiliBoxListEntry*>;
template class XiliSLListIterator<XiliStorageRecord*>;
template class XiliSLListIterator<XilObject*>;
template class XiliSLListIterator<XilTileNumber>;
template class XiliSLListIterator<XilColorspace*>;

//
//  XiliStack
//
#include "XiliStack.hh"

template class XiliStack<XilOp*>;

