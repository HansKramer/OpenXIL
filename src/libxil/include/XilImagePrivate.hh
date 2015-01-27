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
//  File:	XilImagePrivate.hh
//  Project:	XIL
//  Revision:	1.110
//  Last Mod:	10:21:08, 03/10/00
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
//------------------------------------------------------------------------
//	COPYRIGHT
//------------------------------------------------------------------------

//
//  INCLUDE Portion of private header file
//
#ifdef _XIL_PRIVATE_INCLUDES
#include "_XilTile.hh"
#include "_XilDeviceStorage.hh"
#include "_XilDeviceManagerStorage.hh"
#include "_XilGlobalState.hh"
#include "XiliSLList.hh"
#include "XiliStorageRecord.hh"

//
//  For exported memory storage structure definitions
//
#include "XiliMemoryDefines.hh"

#endif // _XIL_PRIVATE_INCLUDES

//
//  DATA Portion of private header file
//
#ifdef _XIL_PRIVATE_DATA

public:
    //
    //  For xil_create()
    //
                        XilImage(XilSystemState* system_state,
                                 unsigned int    x_size, 
                                 unsigned int    y_size, 
                                 unsigned int    num_bands,
                                 XilDataType     data_type,
                                 Xil_boolean     temporary = FALSE);

    //
    //  For xil_create_from_type()
    //
                        XilImage(XilSystemState* system_state,
                                 XilImageFormat* image_format,
                                 Xil_boolean     temporary = FALSE);

    //
    //  For xil_create_from_device()
    //
                        XilImage(XilSystemState* system_state,
                                 XilDevice*      device);

    //
    //  Used by createChild() to create a child image.
    //
                        XilImage(XilSystemState* system_state,
                                 XilImage*       parent,
                                 unsigned int    x_offset,
                                 unsigned int    y_offset,
                                 unsigned int    x_size, 
                                 unsigned int    y_size, 
                                 unsigned int    band_offset,
                                 unsigned int    num_bands);

    //
    //  Required XilObject virtual funcions...
    //
    XilObject*          createCopy();

    //
    //  ---- XilDeferrableObject Methods ----
    //
    //  Does this object support being tiled?
    //
    Xil_boolean         canBeTiled();

    //
    //  Is the object's tile size set?
    //
    Xil_boolean         isTileSizeSet();

    //
    //  Initialize the object's tile size.
    //
    XilStatus           initTileSize(unsigned int xsize,
                                     unsigned int ysize,
                                     XilTile*     new_tile_array = NULL);
    XilStatus           initTileSize(XilDeferrableObject* def_object);

    //
    //  Get the number of tiles for this image
    //
    unsigned int        getNumXTiles();
    unsigned int        getNumYTiles();
    unsigned int        getNumTiles();

    XilStatus           getTileList(XilTileList* tile_list,
                                    XilBox*      area = NULL);

    XilStatus           getTileList(XilTileList*     tile_list,
                                    XiliRect*        rect);

    XilStatus           getTileRect(XiliRect*     rect,
                                    XilTileNumber tile_number);

    //
    //  The given tile is no longer needed -- its storage can be destroyed.
    //
    void                releaseTile(XilTileNumber tnum);

    //
    //  I/O Device support for images.
    //
    XilStatus           setDeviceIO(XilDeviceIO* ioDevice);

    XilOp*              createCaptureOp(XilOp*       constructing_op,
                                        unsigned int branch_num);
    XilOp*              createDisplayOp(XilOp*       constructing_op,
                                        unsigned int branch_num);

    //
    //  Handle the setting up of and the aquisition of the alternate image.
    //
    //  These are MT-unsafe because they should only be called when the DAG is
    //  already locked.
    //
    XilStatus           setAlternateImage(XilImage* alt_image)
    {
        if(altImage == NULL) {
            altImage = alt_image;

            return XIL_SUCCESS;
        } else {
            return XIL_FAILURE;
        }
    }

    XilImage*           getAlternateImage()
    {
        return altImage;
    }

    //
    //  Image reinitialization is only supported for I/O device images and is
    //  called from the XilDeviceIO::reinitControllingImage() method.  If this
    //  image is a child, then it is reinitialized and maintains the same
    //  offsets as before the call.
    //
    //  It is not supported calling this method from other places or other
    //  images besides device images.
    //
    XilStatus           reinit(XilImageFormat* image_format);

    //
    //  Deferred execution overloads to support child images.  Child images
    //  maintain all of their dependencies on their parent's op queue.  So, we
    //  always need to reference through the parent pointer.
    //
    //  For more information on what these functions do, see
    //  XilDeferrableObjectPrivate.hh
    //
    XiliOpQueuePosition setOp(XilOp* new_op);
    XilOp*              getOp();
    void                cleanup(XiliOpQueuePosition qposition,
                                XilOp*              op);
    void                creatorCleanup(XiliOpQueuePosition qposition,
                                       XilOp*              op);
    XiliOpQueuePosition addDependent(XilOp*       dependent_op,
                                     unsigned int src_branch);
    XilStatus           flushDependents(XiliOpQueuePosition qposition);
    XilStatus           flushDependents(XilTileList*        tile_list,
                                        XilOp*              op_flushing,
                                        XiliOpQueuePosition qposition);
    Xil_boolean         tossIfNoDependents();
    XilStatus           APIsync();
    XilStatus           sync(XiliOpQueuePosition qposition =
                             _XILI_OP_QUEUE_INVALID_POSITION);
    XilStatus           syncForReading(XilTileList*        tile_list,
                                       XilOp*              op_flushing = NULL,
                                       XiliOpQueuePosition qposition =
                                       _XILI_OP_QUEUE_INVALID_POSITION);
    XilStatus           syncForWriting(XilTileList*        tile_list,
                                       XilOp*              op_flushing = NULL,
                                       XiliOpQueuePosition qposition =
                                       _XILI_OP_QUEUE_INVALID_POSITION);
    Xil_boolean         preDestroy();
    Xil_boolean         isStorageValid();
    void                setStorageValidFlag(Xil_boolean valid_flag);
    XiliOpStatus        getOpStatus(XilTileNumber       tile_number,
                                    XiliOpQueuePosition qposition);

    void                setOpStatus(XilTileNumber       tile_number,
                                    XiliOpQueuePosition qposition,
                                    XiliOpStatus        op_status);

    Xil_boolean         allTilesDone(XiliOpQueuePosition qposition);
    XilStatus           getTileSize(unsigned int* txsize,
                                  unsigned int* tysize);
    XiliOpQueueEntry*   getQueueEntry(XiliOpQueuePosition qpos);


    //
    //  Overloads for XilObject methods so we can support child images.
    //
    void                setDagRef(XiliDagRef* new_ref);
    XiliDagRef*         getDagRef();
    void                lock();
    XilStatus           trylock();
    void                unlock();

    //
    //  ----  XilImage Methods ----
    //
    //
    //  Given a coordinate return the tile that it resides in...if the
    //  additional pointers are set, it sets the remaining offsets within that
    //  tile.
    //
    XilTileNumber       getTileForCoordinate(unsigned int  x,
					     unsigned int  y,
                                             unsigned int* xoffset = NULL,
                                             unsigned int* yoffset = NULL);

    //
    //  This getStorage() call doesn't take an op as an argument and is used
    //  by the export routines.  The virtual routines are those that can be
    //  overloaded by derived classes -- specifically XiliImageChild.
    //
    XilStatus           getStorage(XilStorage*      storage,
                                   XilBox*          box,
                                   char*            storage_name,
                                   XilStorageAccess access,
                                   XilStorageType   type_requested =
                                       XIL_STORAGE_TYPE_UNDEFINED,
                                   void*            attribs = NULL)
    {
        return getStorage(storage, NULL, box, storage_name,
                          access, type_requested, attribs);
    }

    XilStatus           releaseStorage(XilStorage* storage);

    //
    //  Change our refereces to the given non-deferrable object to the new
    //  non-deferrable object.  Since we selectively determine which objects
    //  to obtain via reference versus those obtained via copy, only a few
    //  of our objects need checking. 
    //
    void                updateObjectReferences(XilNonDeferrableObject* oldobj,
                                               XilNonDeferrableObject* newobj,
                                               Xil_boolean             )
    {
        if(roi == oldobj) {
            roi = (XilRoi*)newobj;
        }
    }

    //
    //  -- The public routines below here are directly called by the API --
    //
    //
    //  Get the parent of this image -- NULL if is a parent
    //
    XilImage*           getParent();

    //
    //  Get and set the image origin.
    //
    void                setOrigin(float x,
                                  float y);

    void                getOrigin(float* x,
                                  float* y)
    {
        if(x != NULL) {
            *x = originX;
        }

        if(y != NULL) {
            *y = originY;
        }
    }

    float               getOriginX()
    {
        return originX;
    }

    float               getOriginY()
    {
        return originY;
    }

    //
    //  Method to get the user-set roi
    //
    XilRoi*             getRoi();

    //
    //  Method to simply reference the user-set roi (internal to the library,
    //  we don't need to make a copy).
    //
    XilRoi*             refRoi()
    {
        return roi;
    }

    //
    //  Image attribute support.
    //
    XilStatus           setAttribute(const char* key,
                                     void*       value);

    XilStatus           getAttribute(const char* key,
                                     void**      ret_value);

    //
    //  Device image attribute support.
    //
    //    NOTE: Only valid for images with a device attached.  
    //
    XilStatus           setDeviceAttribute(const char* key,
                                           void*       value);

    XilStatus           getDeviceAttribute(const char* key,
                                           void**      ret_value);

    //
    //  Double buffering support routines for selecting active buffer and
    //  swapping back buffer to the front.  
    //
    //    NOTE: Only valid for images with a double-buffering device image
    //          attached. 
    //
    XilStatus           setActiveBuffer(XilBufferId id);

    XilBufferId         getActiveBuffer();

    XilStatus           swapBuffers();

    //
    //  Create a copy of the image including all of the data from the source
    //  image.  ROIs are supposed to be ignored.
    //
    XilImage*           createCopy(unsigned int x_offset,
                                   unsigned int y_offset,
                                   unsigned int x_size, 
                                   unsigned int y_size, 
                                   unsigned int band_offset,
                                   unsigned int num_bands);

    //
    //  Method for setting storage on exported images.  It is used by
    //  the C bindings for assigning storage.
    //
    //  Method which supports the (OLD) xil_set_memory_storage() API.
    //
    Xil_boolean         setExportedMemoryStorage(XilMemoryStorage* mem_storage);

    //
    //  Methods for aquiring storage on exported images.  These are used by
    //  the C bindings for returning storage.
    //
    //  Method which supports the (OLD) xil_get_memory_storage() API.
    //
    Xil_boolean         getExportedMemoryStorage(XilMemoryStorage* mem_storage);


    //
    //  getExportedStorage() is like getStorage(), but only works for exported
    //  images.  This one takes a given region and if that region intersects
    //  multiple tiles, then the storage will be cobbled together from the
    //  multiple tiles.
    //
    Xil_boolean         getExportedStorage(XilStorageAPI* storage,
                                           unsigned int   x,
                                           unsigned int   y,
                                           unsigned int   xsize,
                                           unsigned int   ysize);

    //
    //  This returns the storage for the entire exported image as a single
    //  buffer -- if necessary.  It is equivalent to xil_get_memory_storage()
    //  in that the data will be cobbled together if there are tiles in the
    //  image.
    //
    Xil_boolean         getExportedStorage(XilStorageAPI* storage);

    //
    //  This getExportedStorage() returns the storage associated with a single
    //  tile at the given coordinates.
    //
    Xil_boolean         getExportedStorage(XilStorageAPI* storage,
                                           unsigned int   x,
                                           unsigned int   y);

    //
    //  set/getExportedStorageWithCopy() are like their single tile
    //  counterparts except they use cobble/decobble to either copy the given
    //  storage buffer into seperate tiles or return a copy of the tiles as a
    //  single buffer.
    //
    XilStorageAPI*      getExportedStorageWithCopy();

    XilStatus           setExportedStorageWithCopy(XilStorageAPI* storage);

    //
    //  Returns whether the image is exported or not.
    //
    int                 getExported();

    //
    // This routine is used to associate a given roi with an image
    //
    void                setRoi(XilRoi* roi);

    // 
    // Get child offsets
    //
    void                getChildOffsets(unsigned int* x,
					unsigned int* y,
					unsigned int* band);

    //
    // Set and get pixel
    //
    void                setPixel(unsigned int x,
				 unsigned int y,
				 float* values);

    void                getPixel(unsigned int x,
				 unsigned int y,
				 float* values);

    XilRoi*             getGlobalSpaceRoi();
    XiliRect*           getGlobalSpaceRect();
    XilRoi*             getGlobalSpaceRoiWithDoublePrecision();
    XilRoi*             getExtentGlobalSpaceRoi();


    XilStatus           convertToObjectSpace(XiliRect* rect);
    XilStatus           convertToObjectSpace(XilRoi* roi);
    XilStatus           convertToObjectSpace(XiliConvexRegion* cr);

    XilStatus           convertToGlobalSpace(XiliRect* rect);
    XilStatus           convertToGlobalSpace(XilRoi* roi);
    XilStatus           convertToGlobalSpace(XiliConvexRegion* cr);

    XilStatus           setBoxStorage(XilBox* box);

    XilStatus           clipToTile(XilTileNumber  tile_number,
                                   XiliRect*      rect);

    void                setDataSupplyFunc(XilDataSupplyFuncPtr supply_func,
                                          void*                user_args);

    //
    //  Copy the storage information from another image.
    //
    XilStatus           setStorageInfo(XilImage* img);

protected:
                        ~XilImage();
    
private:
    //
    //  Data for child image support
    //
    XilImage*           parent;
    XilImage*           immediateParent;
    unsigned int        offsetX;
    unsigned int        offsetY;
    unsigned int        offsetBand;
    XiliBag             children;

    //
    //  Image origin support
    //
    float               originX;
    float               originY;

    //
    //  Roi support
    //
    XilRoi*             roi;

    //
    //  This roi is origin adjusted and clipped to image dimensions.
    //
    XilRoi*             globalSpaceRoi;

    //
    //  A rect which represents this object in global space
    //
    XiliRectDbl         globalSpaceRect;

    //
    //  A version of the global Space Roi - convex region
    //  with floating point precision maintained.
    //
    XilRoi*             globalSpaceRoiWithDoubles;

    //
    //  A version of the global Space Roi - convex region
    //  with floating point precision maintained.
    //  This roi represents the pixel extent region of the ROI.
    //
    XilRoi*             extendedGlobalSpaceRoi;

    //
    //  The I/O device on the image
    //
    XilDeviceIO*        ioDevice;

    //
    //  For I/O devices, we support the concept of having this image have a
    //  fallback or alternate image.  When set, the alternate image is used
    //  by the capture and display op fallback code to switch the images on
    //  the op.
    //
    XilImage*           altImage;

    //
    //  Data initialization routine
    //
    XilStatus           initializeBasicDataMembers(Xil_boolean temporary = FALSE);

    //
    //  List of storage devices with this image's storage.
    //
    XiliSLList<XiliStorageRecord*> storageList;

    //
    //  Image attributes
    //
    //    NOTE:  In our constructors, the attributeHashTable is constructed so
    //           it will not delete the values.  This is because the values
    //           are set and controlled by the user.
    //
    XiliHashTable<void*> attributeHashTable;

    //
    //  Variable to keep track of storage exported via getExportedMemoryStorage()
    //
    XilStorageAPI*      exportedImageStorage;

    //
    //  Variable for this image's storage movement mode
    //
    XilStorageMovement  storageMovement;

    //
    //  A routine that may be set by the user for data supply
    //
    XilDataSupplyFuncPtr  dataSupplyFunc;
    void*                 supplyUserArgs;

    //
    //  Single lock for protecting storage aquisition.
    //
    //  This lock protects the storage aquisition and all of the DAG
    //  algorithms from the multiple threads we've created.
    //
    XilMutex            storageMutex;

    //
    //  We cache the memory storage device here so getStorage() doesn't need
    //  to look for it every time its called with "XilMemory" which is the
    //  most common case. 
    //
    XiliStorageRecord*  memoryStorageRecord;

    //
    //  Private routines
    //

    //
    //  Method that calculates the default tile size for the image.  It's used
    //  by getTileSize() to appropriately initialize the tile size for the
    //  image when one has not be set.  It takes the tiling mode of the system
    //  state into effect.
    //
    void                getDefaultTileSize(unsigned int* txsize,
                                           unsigned int* tysize);

    //-------------------------------------------------------------------------
    //
    //  The getStorage()/getTiledStorage() support routines which reduce code
    //  duplication between the two routines.  All are prefixed by "st".
    //
    //
    //  When getStorage() or getTiledStorage() is called on a child image, we
    //  need to prepare the tile list, box and other information prior to
    //  calling the parent's getStorage() or getTiledStorage().  This utility
    //  routine takes care of preparing everything for the child image.
    //
    //
    //  The child_box is expected to be set equal to the box given to
    //  getStorage() or getTiledStorage() prior to calling this routine.
    //
    XilStatus           stPrepareChild(XilOp*           op,
                                       XilStorageAccess access,
                                       XilBox*          child_box,
                                       XilTileList*     tile_list);

    XilStatus           stGetToDevice(XilBox*            box,
                                      XilOp*             op,
                                      XilStorageAccess   access,
                                      XilStorageType     type_requested,
                                      void*              attribs,
                                      XilTileList*       tile_list,
                                      XiliStorageRecord* srec);
                                      
    //
    //-------------------------------------------------------------------------

    //
    //  Perform the necessary sync'ing required to get this image ready for
    //  storage aquisition.
    //
    XilStatus           syncForStorage(XilTileList*     tile_list,
                                       XilOp*           op     = NULL,
                                       XilImage*        image  = NULL,
                                       XilStorageAccess access = XIL_READ_WRITE);

    //
    //  Get the specified tile to a storage device.
    //
    XilStatus           getToDevice(XilTile*           tile,
                                    XiliStorageRecord* srec,
                                    XilStorageType     type_requested,
                                    XilStorageAccess   access,
                                    void*              attribs);

    //
    //  Get the given list of tiles to the given storage device.
    //
    XilStatus           getToDevice(XilTileList*       tile_list,
                                    XiliStorageRecord* srec,
                                    XilStorageType     type_requested,
                                    XilStorageAccess   access,
                                    void*              attribs)
    {
        XilTileNumber tnum;
        while(tile_list->getNextTileNumber(&tnum)) {
            if(getToDevice(&tileArray[tnum], srec, type_requested,
                           access, attribs) == XIL_FAILURE) {
                return XIL_FAILURE;
            }
        }

        return XIL_SUCCESS;
    }

    //
    //  Deallocate all of the storage associated with this image.
    //
    void                deallocateAllStorage();

    //
    //  Deallocate the storage associated with the given tile.  The function
    //  properly detects whether the tag has been set (in which case, the
    //  storage object has enough information to release itself) and then
    //  deallocate it appropriately.  Finally, it clears the fields of the
    //  tile to NULL.
    //
    void                deallocateTileStorage(XilTile* tile)
    {
        XilStorage* storage = tile->getStorage();

        if(storage != NULL) {
            //
            //  Check whether the storage object has a TAG associated with it
            //  -- if it does, then just delet ing it will cause
            //  releaseStorage() to be called and the proper deallocation to
            //  be carried out.
            //
            //  If not, check whether there is a device associated with the
            //  tile -- if there is a device, call it to deallocate the
            //  storage.
            //
            if(! storage->isStorageTagSet()) {
                XilDeviceStorage* device = tile->getStorageDevice();

                if(device != NULL) {
                    if(tilingMode == XIL_TILING) {
                        //
                        //  Deallocate every tile.
                        //
                        device->deallocate(storage);
                    } else if(tile == &tileArray[0]) {
                        //
                        //  Deallocate just the first tile in the array.
                        //
                        device->deallocate(storage);
                    }
                }
            }

            //
	    //  Check whether the storage has a data release function associated with
	    //  it --  if it does, call it.
            //
            XilDataReleaseFuncPtr data_release_func;    
            void* user_args;

	    if(storage->isType(XIL_GENERAL)) {
	      //
	      //  Get the data release function for each band
	      //
	      for(unsigned int i=0; i<nBands; i++) {
	        storage->getDataReleasePtr(i,
					   &data_release_func,
					   &user_args);
                if(data_release_func != NULL) {
                    data_release_func((void*)(storage->getDataPtr(i)),(void*)user_args);
                }
		
	      }
	    } else {
	        storage->getDataReleasePtr(&data_release_func,
					   &user_args);
                if(data_release_func != NULL) {
                    data_release_func((void*)(storage->getDataPtr()),(void*)user_args);
                }
	    }


            delete storage;
        }

        tile->setStorage(NULL);
        tile->setStorageRecord(NULL);
        tile->clearEmulatedName();
    }

    //
    //  Recursively destroy all of the exportedImageStorage structures in
    //  a parent's hierarchy.
    //
    void                destroyExportedStorage();

    //
    //  Return the storage record for the given device name.
    //
    XiliStorageRecord*  getStorageRecord(char* storage_type);

    //
    //  Add or remove a child from a parent.
    //
    XilStatus           addChild(XilImage* child);
    XilStatus           removeChild(XilImage* child);

    // 
    //  generate the imageSpaceRoi from the full roi pointer
    //
    void                createGlobalSpaceRoi(XilRoi**    clippedRoi);
    // 
    //  generate the imageSpaceRoi with double precision from the full roi pointer
    //
    void                createGlobalSpaceRoiWithDoubles(XilRoi**    clippedRoi);
    // 
    //  generate the imageSpaceRoi with double precision in extent spact
    //  from the full roi pointer
    //
    void                createExtentGlobalSpaceRoi(XilRoi**    clippedRoi);


    
#endif  // _XIL_PRIVATE_DATA
