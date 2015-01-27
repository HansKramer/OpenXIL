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
//  File:	_XilGPIDefines.hh
//  Project:	XIL
//  Revision:	1.18
//  Last Mod:	10:21:21, 03/10/00
//
//  Description:
//	Assorted defines/const/declarations for the XIL GPI.
//	
//------------------------------------------------------------------------
//	COPYRIGHT
//------------------------------------------------------------------------
#pragma ident	"@(#)_XilGPIDefines.hh	1.18\t00/03/10  "

#ifndef _XIL_GPI_DEFINES_H
#define _XIL_GPI_DEFINES_H

//
//  The major and minor version numbers of the XIL GPI.
//
#ifndef XIL_GPI_MAJOR_VERSION
#define XIL_GPI_MAJOR_VERSION    2
#endif

#ifndef XIL_GPI_MINOR_VERSION
#define XIL_GPI_MINOR_VERSION    1
#endif

#define XIL_BASIC_GPI_VERSION_TEST(lib_major,lib_minor,ptr_major,ptr_minor) \
    {                                              \
        if((int)lib_major != XIL_GPI_MAJOR_VERSION ||   \
           (int)lib_minor <  XIL_GPI_MINOR_VERSION) {   \
            return NULL;                           \
        } else {                                   \
            *ptr_major = XIL_GPI_MAJOR_VERSION;    \
            *ptr_minor = XIL_GPI_MINOR_VERSION;    \
        }                                          \
    }

//
//  Set XIL_LITTLE_ENDIAN, this tells GPI routines
//  which byte order the machine hardware is using.
//  Default is big-endian so it is never set.
//
#ifdef SOLARIS
#include <sys/isa_defs.h>
#endif

#ifdef _LITTLE_ENDIAN
#define XIL_LITTLE_ENDIAN
#endif // _LITTLE_ENDIAN

//
//  Storage access definitions.  These are used for the getStorage() 
//
enum XilStorageAccess {
    XIL_READ_ONLY,
    XIL_WRITE_ONLY,
    XIL_READ_WRITE
};

//
//  Supported Colorspaces : To follow LSARC specifications
//
enum XilColorspaceOpCode {
    XIL_CS_INVALID,
    XIL_CS_RGBLINEAR,
    XIL_CS_RGB709,
    XIL_CS_PHOTOYCC,
    XIL_CS_YCC601,
    XIL_CS_YCC709,
    XIL_CS_YLINEAR,
    XIL_CS_Y601,
    XIL_CS_Y709,
    XIL_CS_CMY,
    XIL_CS_CMYK
};

//
//  Enum used to identify the type of box for convolve, erode and dilate
//  operations.  Each box indicates how to handle the image boundaries.
//
//  Ten cases exposed to the GPI routines and the XIL core ensures that the
//  boxes passed in will fit into one of these 10 cases.
//
enum XilBoxAreaType {
    XIL_AREA_TOP_LEFT_CORNER,
    XIL_AREA_TOP_EDGE,
    XIL_AREA_TOP_RIGHT_CORNER,
    XIL_AREA_LEFT_EDGE,
    XIL_AREA_CENTER,
    XIL_AREA_RIGHT_EDGE,
    XIL_AREA_BOTTOM_LEFT_CORNER,
    XIL_AREA_BOTTOM_EDGE,
    XIL_AREA_BOTTOM_RIGHT_CORNER,
    XIL_AREA_SMALL_SOURCE
};

//
//  Enum used to identify the type of box for geometric compute routines
//  after splitOnTileBoundaries() is called.
//
//
enum XilBoxGeomType {
    XIL_GEOM_UNDEFINED = 0,  // For INTERNAL Use Only -- Tag wasn't set
    
    XIL_GEOM_INTERNAL,       // Data is within a tile
    XIL_GEOM_VERTICAL,       // Vertical strip 2*border_width wide
    XIL_GEOM_HORIZONTAL,     // Horizonatal strip 2*border_width high
    XIL_GEOM_CORNERS         // Touches four corners of a tile
};

//
//  Directions for describing molecules to XIL via the XilFunctionInfo.
//
enum XilDirection {
    XIL_PUSH,
    XIL_POP,
    XIL_STEP
};


//
//  Op Numbers
//
typedef int XilOpNumber;

//
//  Tile Numbers
//
typedef int XilTileNumber;

//
//  XIL object identifier (matches version number in XilDefines.h)
//
typedef Xil_unsigned64  XilObjectId;

#endif //_XIL_GPI_DEFINES_H

