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
//  File:	XilDeviceManagerIOcg6.hh
//  Project:	XIL
//  Revision:	1.7
//  Last Mod:	10:22:34, 03/10/00
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
#pragma ident	"@(#)XilDeviceManagerIOcg6.hh	1.7\t00/03/10  "

#ifndef _XIL_DEVICE_MANAGER_IO_CG6_HH
#define _XIL_DEVICE_MANAGER_IO_CG6_HH

//
//  C++ Includes
//
#include <xil/xilGPI.hh>

//
// System includes
//
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/fbio.h>
#include <sys/cg6fbc.h>
#include <dga/dga.h>

//
//  A structure that describes a mapping of a CG6 device.  Only one
//  mapping is created for all of the windows on the screen.  A list
//  is kept where each node represents a single mapping of a different
//  CG6 device.
//
struct CG6Description {
   int            fd;
   unsigned char* fb_mem;
   int            fb_height;
   int            fb_width;
   int            fb_size;
   fbc*           fb_fbc;
   char           name[32];
   struct CG6Description* next;
};

class XilDeviceManagerIOcg6 : public XilDeviceManagerIO {
public:
    //
    // Creates a new XilDeviceIO object from a display pointer
    // and window.
    //
    XilDeviceIO*           constructDisplayDevice(XilSystemState* state,
                                                  Display*        display,
                                                  Window          window);

    //
    //  Required function that returns the name of this device.
    //
    const char*            getDeviceName();

    XilStatus              describeMembers();
    
    //
    // Constructor Destructor
    //
                           XilDeviceManagerIOcg6();
                           ~XilDeviceManagerIOcg6();

    //
    //  This routine returns a full description of the given CG6
    //  device.  A list of CG6 descriptions is kept in this class so
    //  multiple windows on the same device will not have multiple
    //  mappings.
    //
    CG6Description*        getCG6Description(char*           device_name,
					     XilSystemState* state);

    //
    //  Lookup table caches...each table caches a single band of a lookup 8->8
    //  operation.
    //
    int                    getLookupTable(XilSystemState*  state,
                                          XilLookupSingle* lookup);

    void                   releaseLookupTable(int table);

    Xil_unsigned8*         refLookupCache(int table)
    {
        return lookupCache[table];
    }

    //
    //  Rescale table caches...each table caches a single band of a rescale
    //  operation.
    //
    int                    getRescaleTable(XilSystemState* state,
                                           float           mult_const,
                                           float           add_const);

    void                   releaseRescaleTable(int table);

    Xil_unsigned8*         refRescaleCache(int table)
    {
        return rescaleCache[table];
    }

    //
    //  Window level table caches...each table caches a window level
    //  operation.
    //
    int                    getWinlevTable(XilSystemState* state,
                                          float           mult_const,
                                          float           add_const,
                                          Xil_signed16    low1,
                                          Xil_signed16    high1,
                                          Xil_signed16    map1,
                                          Xil_signed16    low2,
                                          Xil_signed16    high2,
                                          Xil_signed16    map2);

    void                   releaseWinlevTable(int table);

    Xil_unsigned8*         refWinlevCache(int table)
    {
        return winlevCache[table];
    }

private:
#define _XILI_NUM_RESCALE_TABLES 4
    struct XilRescaleArrayDesc {
	Xil_float32 multConst;
	Xil_float32 addConst;
    };

    Xil_unsigned8*      rescaleCache[_XILI_NUM_RESCALE_TABLES];
    XilRescaleArrayDesc rescaleCacheInfo[_XILI_NUM_RESCALE_TABLES];
    unsigned int        rescaleRefCnts[_XILI_NUM_RESCALE_TABLES];
    XilMutex            rescaleCacheMutex;

#define _XILI_NUM_LOOKUP_TABLES 4
    Xil_unsigned8*      lookupCache[_XILI_NUM_LOOKUP_TABLES];
    XilVersion          lookupCacheInfo[_XILI_NUM_LOOKUP_TABLES];
    unsigned int        lookupRefCnts[_XILI_NUM_LOOKUP_TABLES];
    XilMutex            lookupCacheMutex;

#define _XILI_NUM_WINLEV_TABLES 4
    struct XilWinlevArrayDesc {
	float        multConst;
	float        addConst;
        Xil_signed16 low1; 
        Xil_signed16 high1;
        Xil_signed16 map1; 
        Xil_signed16 low2; 
        Xil_signed16 high2;
        Xil_signed16 map2; 
    };

    Xil_unsigned8*      winlevCache[_XILI_NUM_WINLEV_TABLES];
    XilWinlevArrayDesc  winlevCacheInfo[_XILI_NUM_WINLEV_TABLES];
    unsigned int        winlevRefCnts[_XILI_NUM_WINLEV_TABLES];
    XilMutex            winlevCacheMutex;

    //
    //  The CG6 description list...
    //
    CG6Description* baseCG6Description;

    //
    // Mutex to lock access to opening of the registers and
    // frame buffer mappings
    //
    XilMutex        mutex;

};
#endif // _XIL_DEVICE_MANAGER_IO_CG6_HH
