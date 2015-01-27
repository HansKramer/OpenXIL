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
//  File:	XilTilePrivate.hh
//  Project:	XIL
//  Revision:	1.13
//  Last Mod:	10:21:25, 03/10/00
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
#pragma ident	"@(#)XilTilePrivate.hh	1.13\t00/03/10  "

#ifdef _XIL_PRIVATE_INCLUDES

//
//  Standard includes
//
#include <stdlib.h>
#include <string.h>

//
//  XIL Public Includes
//
#include "_XilBox.hh"

//
//  XIL Private Includes
//
#include "XiliStorageRecord.hh"

#endif // _XIL_PRIVATE_INCLUDES

#ifdef _XIL_PRIVATE_DATA

public:
    //
    //  Set the box information...
    //
    void               setBox(XilBox* new_box)
    {
        box = *new_box;
    }

    //
    //  Get the storage device for this tile.
    //
    XilDeviceStorage*  getStorageDevice()
    {
        return (storageRecord == NULL) ? NULL : storageRecord->getDevice();
    }

    //
    //  Get/Set the storage description for this tile.
    //
    XilStorage*        getStorage()
    {
        return storage;
    }

    void               setStorage(XilStorage* new_storage)
    {
        storage = new_storage;
    }

    //
    //  Get the current storage type for this tile.
    //  The storageRecord could be NULL if storage was set by the user
    //
    const char*        getStorageName()
    {
        if(emulatedStorageName != NULL) {
            return emulatedStorageName;
        } else {
            return (storageRecord == NULL) ? NULL : storageRecord->getName();
        }
    }

    //
    //  Is the storage being emulated?
    //
    Xil_boolean        isEmulated()
    {
        return emulatedStorageName != NULL;
    }

    //
    //  Set the emulated type name...
    //
    XilStatus          setEmulatedName(const char* new_name)
    {
        XilStatus ret_val = XIL_SUCCESS;

        free(emulatedStorageName);

        emulatedStorageName = strdup(new_name);

        if(emulatedStorageName == NULL) {
            ret_val = XIL_FAILURE;
        }

        return ret_val;
    }

    //
    //  Clear the emulated type name...
    //
    void              clearEmulatedName()
    {
        free(emulatedStorageName);

        emulatedStorageName = NULL;
    }

    //
    //  Set the storage record for this tile.  The storage record contains the
    //  storage device, the storage manager and the storage type name. 
    //
    void               setStorageRecord(XiliStorageRecord* record)
    {
        storageRecord = record;
    }

    XiliStorageRecord* getStorageRecord()
    {
        return storageRecord;
    }

    //
    //  Determine if this tile overlaps another tile.
    //
    Xil_boolean        tilesOverlap(XilTile* tile)
    {
        return box.intersects(tile->getBox());
    }

    //
    //  Constructor
    //
                       XilTile()
    {
        storageRecord       = NULL;
        storage             = NULL;
        emulatedStorageName = NULL;
    }

private:
    //
    //  Data for this class/structure.
    //
    XiliStorageRecord* storageRecord;
    XilStorage*        storage;
    char*              emulatedStorageName;

    XilBox             box;
#endif // _XIL_PRIVATE_DATA
