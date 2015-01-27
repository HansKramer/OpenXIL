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
//  File:	XilDeviceStorageMemory.hh
//  Project:	XIL
//  Revision:	1.19
//  Last Mod:	10:23:56, 03/10/00
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
//  MT-level:  SAFE
//	
//------------------------------------------------------------------------
//	COPYRIGHT
//------------------------------------------------------------------------
#pragma ident	"@(#)XilDeviceStorageMemory.hh	1.19\t00/03/10  "

#ifndef _XIL_XIL_DEVICE_STORAGE_MEMORY_HH
#define _XIL_XIL_DEVICE_STORAGE_MEMORY_HH

#include <stdlib.h>

//
//  Include XIL GPI
//
#include <xil/xilGPI.hh>

class XilDeviceStorageMemory : public XilDeviceStorage {
public:    
    //
    //  Allocates a new tile on the memory device. 
    //
    XilStatus   allocate(XilStorage*      storage,
                         unsigned int     xsize,
                         unsigned int     ysize,
                         XilStorageType   type_requested,
                         XilStorageAccess access,
                         void*            attribs);
    
    //
    //  Deallocates a tile from the memory device. 
    //
    XilStatus   deallocate(XilStorage*    storage);
    
    //
    //  Cobbles the given descriptions together to produce a
    //    contiguous buffer of memory representing the area described
    //    by the box.
    //
    XilStatus   cobble(XilStorage*     storage,
                       XilBox*         box,
                       XilTileList*    tile_list,
                       XilStorageType  type_requested,
                       void*           attribs);
    
    //
    //  The opposite of cobbleStorage. Take a contiguous storage
    //  region and copies it into a set of tiles.
    //
    XilStatus   decobble(XilStorage*  src_storage,
                         XilBox*      box,
                         XilTileList* tile_list);

    //
    //  Deallocates a storage cobble from the memory device. 
    //
    XilStatus   deallocateCobble(XilStorage*    storage);
    
    //
    //  Copies the given tile from the memory device (XilStorage
    //    description) into this storage devices description.  This
    //    routine is used to propagate a tile from the XilMemory
    //    storage device this storage device.
    //
    XilStatus   copyFromMemory(XilStorage*  dst_storage,
                               XilStorage*  mem_storage,
                               unsigned int xsize,
                               unsigned int ysize);

    //
    //  Copies the given tile from this device's description
    //    into the memory storage device's XilStorage description.
    //    This is used to propagate a tile from this device to the
    //    XilMemory storage device.
    //
    XilStatus   copyToMemory(XilStorage*   mem_storage,
                             XilStorage*   src_storage,
                             unsigned int  xsize,
                             unsigned int  ysize);

    //
    //  Set and get a pixel value in the storage. Assumes
    //  that the storage passed in has been adjusted to
    //  be just the single pixel.
    //
    XilStatus   setPixel(XilStorage*    storage,
			 float*         values,
                         unsigned int   xoffset,
                         unsigned int   yoffset,
			 unsigned int   offset_band,
			 unsigned int   nbands);

    XilStatus   getPixel(XilStorage*    storage,
			 float*         values,
                         unsigned int   xoffset,
                         unsigned int   yoffset,
			 unsigned int   offset_band,
			 unsigned int   nbands);

    //
    //  Constructor/Destructor for initializing and releasing
    //    resources associated with this storage device.  
    //
    XilDeviceStorageMemory(XilImage* parent_image);
    ~XilDeviceStorageMemory();

private:
    //
    //  Cache this so we're not looking it up in setPixel/getPixel every time
    //  we need it.
    //
    XilDataType dataType;

    //
    //  Allocates a chunk of data of the given size.
    //
    void*      allocateChunk(unsigned int size);
    
    //
    // Intersects tile with the box to be cobbled.
    // Returns the src region to be pasted into the dst.
    //
    void       intersectTile(XilTile*       tile,
                             XilBox*        box,
                             unsigned int*  x1,
                             unsigned int*  y1,
                             unsigned int*  w,
                             unsigned int*  h);


    //
    // Two-dimensional block copy function
    // Uses bytes_per_line fields to move
    // between scanlines. Follows same calling
    // API as the VIS g_copy functions.
    // Correctly handles overlapping copies
    //
    void       copy2D(void*        dst,
                      unsigned int dst_linebytes,
                      unsigned int width,
                      unsigned int height,
                      void*        src,
                      unsigned int src_linebytes);
     
     
    //
    // Loop thru the tile list and pick the most common storage type
    //
    Xil_boolean diverseTileStorage(XilTileList*    tile_list,
                                   XilStorageType* dst_storage_type,
                                   Xil_boolean*    diverse_ps);

    //
    // Calculate the starting address of an arbitrary point
    // within a storage region. Also returns the ps, ss, bs, and offset
    //
    Xil_unsigned8* calcAddress(XilStorage*  storage,
                               unsigned int band,
                               unsigned int x1, 
                               unsigned int y1,
                               unsigned int* ps,
                               unsigned int* ss,
                               unsigned int* bs,
                               unsigned int* offset);

    //
    //  The file descriptor for /dev/zero.
    //
#ifndef _WINDOWS
    static int            devZeroFd;
    static int            refCount;
#endif
    static XilMutex       staticMutex;
    static unsigned int   pageSize;

#ifdef _WINDOWS
#define lrand48() rand()
#endif

    unsigned int          getNextPtrOffset()
    {
        //
        //  Use lrand() to randomize the offsets within a page.  Shift to
        //  ensure that the returned offset leaves things 8-byte aligned.
        //
        long ret_val = ((lrand48() % pageSize) >> 3) << 3;

        //
        //  The size must always be greater or equal to the size of our info
        //  structure...
        //
        if(ret_val < sizeof(XilMemoryDataInfo)) {
            ret_val = sizeof(XilMemoryDataInfo);
        }

        return ret_val;
    }
        
    
    //
    //  A structure which is used to hold the information associated with the
    //  data pointer in order to munmap the right data pointer.
    //
    class XilMemoryDataInfo {
    public:
        int          dataOffset;
        unsigned int dataSize;
    };
};

#endif // _XIL_XIL_DEVICE_STORAGE_MEMORY_HH
