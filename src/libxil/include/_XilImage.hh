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
//  File:	_XilImage.hh
//  Project:	XIL
//  Revision:	1.43
//  Last Mod:	10:20:46, 03/10/00
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
//  MT Level:   UNSAFE
//
//------------------------------------------------------------------------
//	COPYRIGHT
//------------------------------------------------------------------------
#pragma ident	"@(#)_XilImage.hh	1.43\t00/03/10  "

#ifndef _XIL_IMAGE_HH
#define _XIL_IMAGE_HH

//
//  System Includes
//

//
//  C++ Includes
//
#include "_XilImageFormat.hh"
#include "_XilStorage.hh"
#include "_XilRoi.hh"
#include "_XilGPIDefines.hh" 

//
//  Private Includes
//
#ifdef  _XIL_LIBXIL_PRIVATE
#define _XIL_PRIVATE_INCLUDES

#include "XilImagePrivate.hh"

#undef  _XIL_PRIVATE_INCLUDES
#endif

//
//  The XilImage Class...
//
class XilImage : public XilImageFormat {
public:
    //
    //  Create a child image of this image.
    //
    //  Its public because I/O devices may need to be able to create
    //  child images to represent band offsets.
    //
    XilImage*           createChild(unsigned int x_offset,
                                    unsigned int y_offset,
                                    unsigned int x_size, 
                                    unsigned int y_size, 
                                    unsigned int band_offset,
                                    unsigned int num_bands);


    //
    //  These routines export an imported image and import and exported image.
    //
    //  exportStorage() takes an argument indicating whether device images
    //  (i.e. controlling images) should be permitted to be exported.   An I/O
    //  device may want to leave its controlling image exported so image
    //  attributes like the tile size and storage can be changed, etc.
    //
    XilStatus           exportStorage(Xil_boolean permit_device_export = TRUE);
    void                import(Xil_boolean image_changed_flag = FALSE);

    //
    //  Get/Set the movement flags for imported storage handling.
    //
    void                setStorageMovement(XilStorageMovement move_flag);
    XilStorageMovement  getStorageMovement();

    //
    //  Get/Set the tile size on an exported image.
    //
    void                getExportedTileSize(unsigned int* tile_xsize,
                                            unsigned int* tile_ysize);
    XilStatus           setExportedTileSize(unsigned int  tile_xsize,
                                            unsigned int  tile_ysize);

    //
    //  Set the storage for a tile within an image.  This call replaces the
    //  storage of the tile where the coordinate on the tile lands within.
    //
    XilStatus           setExportedTileStorage(XilStorage* storage);

    //
    //  Aquisition of storage.
    //
    XilStatus           getStorage(XilStorage*      storage,
                                   XilOp*           op,
                                   XilBox*          box,
                                   char*            storage_dev,
                                   XilStorageAccess access,
                                   XilStorageType   type_requested =
                                       XIL_STORAGE_TYPE_UNDEFINED,
                                   void*            attribs = NULL);

    //
    //  getTiledStorage() always returns the tiles in their existing format.
    //
    //  Unlike getStorage(), getTiledStorage() will never cobble tiles
    //  together to generate a contiguous region of memory.
    //
    XilTileList*        getTiledStorage(XilOp*           op,
                                         XilBox*          box,
                                         char*            storage_dev,
                                         XilStorageAccess access,
                                         void*            attribs = NULL);

    //
    //  Determine if the image is supported for reading and/or writing
    //
    Xil_boolean         isReadable();
    Xil_boolean         isWritable();

    //
    //  Get the I/O device associated with this image.
    //
    XilDeviceIO*        getDeviceIO();

private:
#ifdef _XIL_LIBXIL_PRIVATE
#define _XIL_PRIVATE_DATA
    
#include "XilImagePrivate.hh"
    
#undef  _XIL_PRIVATE_DATA
#else
    //
    //  Make it clear to the compiler that a destructor exists.  This is done
    //  so GPI users will get a compile-time error if they attempt to delete
    //  this class.
    //
                        ~XilImage();
#endif
};

#endif // _XIL_IMAGE_HH
