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
//  File:	_XilDeviceStorage.hh
//  Project:	XIL
//  Revision:	1.22
//  Last Mod:	10:22:06, 03/10/00
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
#pragma ident	"@(#)_XilDeviceStorage.hh	1.22\t00/03/10  "

#ifndef _XIL_DEVICE_STORAGE_H
#define _XIL_DEVICE_STORAGE_H

//
//  XIL Includes
//
#include "_XilDefines.h"
#include "_XilClasses.hh"

class XilDeviceStorage {
public:
    //
    //  Allocates a new region on this device by filling in the storage
    //  object with the description of that storage in the format
    //  requested. If type_requested is XIL_STORAGE_TYPE_UNDEFINED, then 
    //  any format is acceptable. The image you're allocating for is the 
    //  one being passed in on the storage object.
    //
    virtual XilStatus   allocate(XilStorage*      storage,
                                 unsigned int     xsize,
                                 unsigned int     ysize,
                                 XilStorageType   type_requested,
                                 XilStorageAccess access,
                                 void*            attribs)=0;
    
    //
    //  Deallocates the resorces associated with the given storage
    //  description.
    //
    virtual XilStatus   deallocate(XilStorage*    storage)=0;

    //
    //  Cobbles the given descriptions together to produce a
    //  contiguous buffer of memory representing the area described
    //  by the box, in the format requested.
    //
    virtual XilStatus   cobble(XilStorage*     storage,
                               XilBox*         box,
                               XilTileList*    tile_list,
                               XilStorageType  type_requested,
                               void*           attribs)=0;

    //
    //  The opposite of cobbleStorage. Take a contiguous storage
    //  region and copies it into a set of tiles.
    //
    virtual XilStatus   decobble(XilStorage*     src_storage,
                                 XilBox*         box,
                                 XilTileList*    tile_list)=0;

    //
    //  Deallocates the resorces associated with the given cobbled
    //  storage description.
    //
    virtual XilStatus   deallocateCobble(XilStorage* storage)=0;

    //
    //  Copies the given tile from the memory device (XilStorage
    //  description) into this storage devices description.  This
    //  routine is used to propagate a tile from the XilMemory
    //  storage device this storage device.
    //
    virtual XilStatus   copyFromMemory(XilStorage*  mem_storage,
                                       XilStorage*  dst_storage,
                                       unsigned int xsize,
                                       unsigned int ysize)=0;

    //
    //  Copies the given tile from this device's description
    //  into the memory storage device's XilStorage description.
    //  this is used to propagate a tile from this device to the
    //  XilMemory storage device.
    //
    virtual XilStatus   copyToMemory(XilStorage*  src_storage,
                                     XilStorage*  mem_storage,
                                     unsigned int xsize,
                                     unsigned int ysize)=0;

    //
    //  Set and Get a pixel.
    //
    //  The storage that has been passed in represents a tile containing the
    //  image location required by the user.
    //
    //  The offset_x, offset_y and offset_band are the required offsets to
    //  access the correct pixel in the given storage.
    //
    //  The num_bands indicates how many bands need to be updated at that
    //  location.
    //
    //  The storage device then just has to set/get the values at
    //  location given.
    //
    virtual XilStatus    setPixel(XilStorage*    storage,
				  float*         values,
                                  unsigned int   offset_x,
                                  unsigned int   offset_y,
				  unsigned int   offset_band,
				  unsigned int   num_bands)=0;

    virtual XilStatus    getPixel(XilStorage*    storage,
				  float*         values,
                                  unsigned int   offset_x,
                                  unsigned int   offset_y,
				  unsigned int   offset_band,
				  unsigned int   num_bands)=0;
    

    //
    //  Offers the storage device an opportunity to provide the storage
    //  description for a device other than its own.  Thus, it make it
    //  possible for this device to emulate another storage format.  The
    //  value returned is XIL_FAILURE if the device cannot complete the
    //  request.
    //
    //  The first argument is the storage object to modify, the second
    //  argument is the target storage type and any attributes provided by
    //  the compute device.  This routine is called before the target device
    //  is asked to attempt a copy from this device's description.  The
    //  modified storage object becomes the "current" storage description
    //  for the tile.  If a compute routine requests the tile in the base
    //  format again, modifyForEmulation() is called with the same storage
    //  object and the owner's type name (permitting this device to revert
    //  it to its original description).
    //
    virtual XilStatus   modifyForEmulation(XilStorage*  storage,
                                           const char*  target_storage_type,
                                           void*        target_attribs,
                                           unsigned int xsize,
                                           unsigned int ysize);

    //
    //  This is called to ask whether this device can transfer the storage for
    //  a tile directly from the current storage device (given by
    //  current_storage_type) to the target storage device (given by
    //  target_storage_type).  The storage object provided as the first
    //  is the description allocated on the device which currently owns the
    //  tile.  The routine updates the given (target) storage object to
    //  contain a description of the target type if it can directly perform
    //  the propagation.  It's important to note that the target_storage will
    //  be considered to be owned by a device of the target_storage_type.  The
    //  final argument is a flag indicating whether the old device should be
    //  called to deallocate the existing storage.
    //
    //  This routine is called on the current storage device first and then on
    //  the target storage device giving each a chance to know about the other
    //  and directly propagate the storage.  This is done prior to using the
    //  required copyToMemory() and copyFromMemory() calls.
    //
    virtual XilStatus   transferToDescription(XilStorage*  current_storage,
                                              const char*  current_storage_type,
                                              XilStorage*  target_storage,
                                              const char*  target_storage_type,
                                              void*        target_attribs,
                                              unsigned int xsize,
                                              unsigned int ysize,
                                              Xil_boolean* dealloc_old);

    //
    //  Fill in the given storage object given a new x offset, y offset and
    //  band offset.  The routine is meant to handle child and non-tile
    //  aligned requests.  The storage device is expected to fill in the new
    //  storage object taking the offsets into account.
    //
    //  The derived implementation of this function is optional.  It is only
    //  necessary for a storage device to implement it if the storage device
    //  needs to change the information contained in the XilStorage object's
    //  Device Specific Info pointer.  The default implementation will take
    //  care of changing the data pointer in the storage object.
    //
    virtual XilStatus   setStorage(XilStorage*  new_storage_descrip,
                                   XilStorage*  tile_storage_descrip,
                                   int          x_offset,
                                   int          y_offset,
                                   int          band_offset);

    //
    //  This is called by the XIL core when it knows it needs a tile loaded
    //  for imminent processing.  The storage device can overload this method
    //  to permit prefetching of tile's data.
    //
    //  The requested memory is expected to be "paged into" the device when
    //  the willNeed() call returns.  If the device is out of space and cannot
    //  make the requested region of storage available, it can indicate this
    //  by returning XIL_FAILURE.  Depending upon the operation, the number of
    //  outstanding operations, the XIL core may take this as a failed
    //  propagation or wait until dontNeed can be called to try willNeed
    //  again.
    //
    //  The dontNeed() method is the opposite of willNeed() in that it means
    //  the XIL core does not expect to need the specified storage for a
    //  relatively long time.
    //
    //  The device can use as many or as few of the additional arguments in
    //  determining how much storage to make available.  The XIL core provides
    //  the band details for cases like XIL_BAND_SEQUENTIAL or XIL_GENERAL
    //  where its possible to only page in a single band.
    //
    //  The default implementation of willNeed() indicates the memory is
    //  available by returning XIL_SUCCESS.
    //
    virtual XilStatus    willNeed(XilStorage*  storage,
                                  unsigned int x,
                                  unsigned int y,
                                  unsigned int xsize,
                                  unsigned int ysize,
                                  unsigned int band_offset,
                                  unsigned int nbands);

    virtual void         dontNeed(XilStorage*  storage,
                                  unsigned int x,
                                  unsigned int y,
                                  unsigned int xsize,
                                  unsigned int ysize,
                                  unsigned int band_offset,
                                  unsigned int nbands);
    
    //
    //  Non-virtual routines used by the storage devices to access the
    //  secondary box that represents the actual storage to aquire for
    //  an area.
    //
    void                 getBoxStorageLocation(XilBox*       box,
                                               int*          x,
                                               int*          y,
                                               unsigned int* xsize,
                                               unsigned int* ysize,
                                               int*          band);
    
    //
    //  Constructor.
    //
    //  The image is required because every device is associated with
    //  a single image associated.  Each device will need to know
    //  which image it's associated with to obtain certain image-specific
    //  parameters.
    //
    XilDeviceStorage(XilImage* parent_image);

protected:
    XilImage*          image;

    //
    //  Extra growth space for future since this is seen by derived classes.
    //
    void*              _extraData[256];
};

#endif // _XIL_DEVICE_STORAGE_H
