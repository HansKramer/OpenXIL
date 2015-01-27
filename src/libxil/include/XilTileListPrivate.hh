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
//  File:	XilTileListPrivate.hh
//  Project:	XIL
//  Revision:	1.9
//  Last Mod:	10:22:08, 03/10/00
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
//  MT-level:  UNsafe
//	
//------------------------------------------------------------------------
//	COPYRIGHT
//------------------------------------------------------------------------
#pragma ident	"@(#)XilTileListPrivate.hh	1.9\t00/03/10  "


#ifdef _XIL_PRIVATE_INCLUDES

#include "_XilSystemState.hh"
#include "_XilBox.hh"
#include "_XilTile.hh"

#include "XilError.hh"

#endif


#ifdef _XIL_PRIVATE_DATA

#define _XILI_TILELIST_NUM_ON_STACK  64

public:
    //
    //  Constructor
    //
                        XilTileList(XilSystemState* system_state)
    {
        systemState  = system_state;
        heapTileList = NULL;
        tileList     = NULL;
        tileArrayPtr = NULL;
        mutex        = NULL;
        numTiles     = 0;
        currentEntry = 0;

        //
        //  We only initialize the xsize to 0 which indicates the area isn't
        //  valid.
        //
        xsize        = 0;
    }

    //
    //  Need to initialize the number of tiles to be represented by the list
    //  prior to adding tile numbers to the list.
    //
    XilStatus            setNumTiles(unsigned int num_tiles)
    {
        //
        //  Delete existing heap-based list.
        //
        if(heapTileList != NULL) {
            delete heapTileList;
        }

        //
        //  Initialize our numTiles and tileList variables.
        //
        numTiles = num_tiles;

        XilStatus ret_val = XIL_SUCCESS;

        if(numTiles <= _XILI_TILELIST_NUM_ON_STACK) {
            tileList     = stackTileList;
            heapTileList = NULL;
        } else {
            heapTileList = new XilTileNumber[numTiles];
            tileList     = heapTileList;

            if(tileList == NULL) {
                XIL_ERROR(systemState, XIL_ERROR_RESOURCE, "di-1", TRUE);
                ret_val = XIL_FAILURE;
            }
        }

        return ret_val;
    }

    //
    //  Set the given list entry to the given tile number.
    //
    void                setEntry(unsigned int  entry_num,
                                 XilTileNumber tile_num)
    {
        tileList[entry_num] = tile_num;
    }

    //
    //  Get what's in the the given list entry.
    //
    XilTileNumber       getTileNumber(unsigned int entry_num)
    {
        return tileList[entry_num];
    }

    //
    //  For iterating with tile numbers.
    //
    Xil_boolean         getNextTileNumber(XilTileNumber* tile_num)
    {
        if(currentEntry == numTiles) {
            currentEntry = 0;
            return FALSE;
        } else {
            *tile_num = tileList[currentEntry++];
            return TRUE;
        }
    }

    //
    //  Internal routine for iterating with tiles.
    //
    XilTile*             getNextTile()
    {
        if(currentEntry == numTiles) {
            currentEntry = 0;
            return NULL;
        } else {
            return &tileArrayPtr[tileList[currentEntry++]];
        }
    }

    //
    //  Set the tileArrayPtr so we can return XilTile references.
    //
    void                setTileArray(XilTile* tarray)
    {
        tileArrayPtr = tarray;
    }

    XilTile*            getTileArray()
    {
        return tileArrayPtr;
    }

    //
    //  Optionally set the area which the tile list represents
    //
    void                setArea(int          new_x,
                                int          new_y,
                                unsigned int new_xsize,
                                unsigned int new_ysize)
    {
        x     = new_x;
        y     = new_y;
        xsize = new_xsize;
        ysize = new_ysize;
    }

    void                getArea(int*          ret_x,
                                int*          ret_y,
                                unsigned int* ret_xsize,
                                unsigned int* ret_ysize)
    {
        *ret_x     = x;
        *ret_y     = y;
        *ret_ysize = ysize;
        *ret_xsize = xsize;
    }

    //
    //  Optionally set a mutex that should be released if the current thread
    //  must suspend processing on the tile list.
    //
    void                setMutex(XilMutex* new_mutex)
    {
        mutex = new_mutex;
    }

    XilMutex*           getMutex()
    {
        return mutex;
    }

    void                transferList(XilTileList* tile_list)
    {
        //
        //  Delete existing heap-based list.
        //
        delete heapTileList;

        //
        //  Copy all the variables...including the stack tile list.
        //
        *this = *tile_list;

        //
        //  Always reset the list to the start upon transfer.
        //
        currentEntry = 0;

        //
        //  We've transfered the list which means the other list is now invalid.
        //
        tile_list->heapTileList = NULL;
        tile_list->tileList     = NULL;

        //
        //  Point tileList at the correct stack if the information is on the
        //  stack. 
        //
        if(numTiles <= _XILI_TILELIST_NUM_ON_STACK) {
            tileList = stackTileList;
        }
    }

    void               setEqual(XilTileList* tile_list)
    {
        //
        //  Delete existing heap-based list.
        //
        delete heapTileList;

        //
        //  Copy all the variables...including the stack tile list.
        //
        *this = *tile_list;

        //
        //  Always reset the list to the start upon transfer.
        //
        currentEntry = 0;

        //
        //  Now, we would need to copy the heap if it's set.
        //
        if(heapTileList != NULL) {
            heapTileList = new XilTileNumber[numTiles];
            tileList     = heapTileList;

            if(tileList == NULL) {
                XIL_ERROR(systemState, XIL_ERROR_RESOURCE, "di-1", TRUE);
            } else {
                xili_memcpy(heapTileList, tile_list->heapTileList,
                            numTiles*sizeof(XilTileNumber));
            }
        } else {
            tileList = stackTileList;
        }
    }

                        ~XilTileList()
    {
        if(heapTileList != NULL) {
            delete heapTileList;
        }
    }

    void                dump()
    {
        fprintf(stderr, "TL %p: ", this);

        for(unsigned int i=0; i<numTiles; i++) {
            fprintf(stderr, "%02d : ", tileList[i]);
        }

        fprintf(stderr, "\n");
    }

private:
    //
    //  Private Data.
    //
    XilTileNumber    stackTileList[_XILI_TILELIST_NUM_ON_STACK];
    XilTileNumber*   heapTileList;
    XilTileNumber*   tileList;
    unsigned int     numTiles;
    unsigned int     currentEntry;

    XilTile*         tileArrayPtr;

    XilSystemState*  systemState;

    int              x;
    int              y;
    unsigned int     xsize;
    unsigned int     ysize;

    XilMutex*        mutex;
#endif
