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
//  File:	XiliStorageRecord.hh
//  Project:	XIL
//  Revision:	1.2
//  Last Mod:	10:21:47, 03/10/00
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
#pragma ident	"@(#)XiliStorageRecord.hh	1.2\t00/03/10  "

#include <stdlib.h>
#include <string.h>
#include "_XilClasses.hh"
#include "_XilDeviceStorage.hh"

#ifndef _XILI_STORAGE_RECORD
#define _XILI_STORAGE_RECORD

class XiliStorageRecord {
public:
    XiliStorageRecord(const char*              storage_name,
                      XilDeviceManagerStorage* dms,
                      XilDeviceStorage*        ds) :
        deviceManager(dms), device(ds)
    {
        //
        //  No error should be generated because these are constructed from the
        //  getStorage() call which should not generate errors.
        //
        name = strdup(storage_name);
    }

    ~XiliStorageRecord()
    {
        //
        //  Delete the storage device contained in this record.  If the hash
        //  table is going away then the image is being destroyed and we have
        //  no use for this storage device any longer.  There is only one
        //  storage device per image and there is only one storage record per
        //  storage device in an image.
        //
        delete device;
        free(name);
    }

    char*                    getName()
    {
        return name;
    }
    
    XilDeviceManagerStorage* getDeviceManager()
    {
        return deviceManager;
    }

    XilDeviceStorage*        getDevice()
    {
        return device;
    }
    
private:
    char*                    name;
    XilDeviceManagerStorage* deviceManager;
    XilDeviceStorage*        device;
};

#endif // _XILI_STORAGE_RECORD
