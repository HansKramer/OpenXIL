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
//  File:	XilImage.cc
//  Project:	XIL
//  Revision:	1.265
//  Last Mod:	10:08:19, 03/10/00
//
//  Description:
//	
//	Implementation of the XilImage class.
//	
//	
//------------------------------------------------------------------------
//	COPYRIGHT
//------------------------------------------------------------------------
#pragma ident	"@(#)XilImage.cc	1.265\t00/03/10  "

//
//  System Includes
//

#ifndef _WINDOWS
#include <unistd.h>
#endif
#include <stdlib.h>

//
//  C++ Includes
//
#include "_XilDefines.h"
#include "_XilGlobalState.hh"
#include "_XilBox.hh"
#include "_XilImage.hh"
#include "_XilStorage.hh"
#include "_XilSystemState.hh"
#include "_XilDeviceManagerStorage.hh"
#include "_XilDeviceStorage.hh"
#include "_XilDeviceIO.hh"
#include "_XilTileList.hh"

#include "XilStorageAPI.hh"
#include "XiliThread.hh"

//-----------------------------------------------------------------------
//  CONST declarations
//-----------------------------------------------------------------------

#if !defined(HPUX)
inline
#endif
XilOp*
createCopyOp(void* src,
             void* dst)
{
    const char*                   op_name = "copy";
    static XilOpCreateFunctionPtr op_create_function = NULL;
    
    if(op_create_function == NULL) {
        op_create_function =
            XilGlobalState::getXilGlobalState()->getXilOpCreateFunc(op_name);
        if(op_create_function == NULL) {
            return NULL;
        }
    }

    void* args[3];
    args[0] = src;
    args[1] = dst;
    args[2] = NULL;

    XilOp* op = (*op_create_function)(op_name, args, 2);

    return op;
}

//------------------------------------------------------------------------
//
//  Function:	XilImage::XilImage (for xil_create())
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
//  Parameters:
//	
//	
//  Returns:
//	
//	
//  Side Effects:
//	
//	
//  Notes:
//	
//	
//  Deficiencies/ToDo:
//	
//	
//------------------------------------------------------------------------
XilImage::XilImage(XilSystemState* system_state,
                   unsigned int    x_size, 
                   unsigned int    y_size, 
                   unsigned int    num_bands,
                   XilDataType     data_type,
                   Xil_boolean     temporary) :
    XilImageFormat(system_state,
                   x_size, y_size, num_bands, data_type, XIL_IMAGE),
    attributeHashTable(23, system_state, FALSE)
{
    if(isOKFlag == FALSE) {
        //
        //  XilImageFormat didn't initialize.
        //
        return;
    }

    isOKFlag = FALSE;

    if(initializeBasicDataMembers(temporary) == XIL_FAILURE) {
        return;
    }

    //
    //  XilImageFormat doesn't check for these to be 0...
    //
    if(x_size == 0 || y_size == 0 || num_bands == 0) {
        XIL_OBJ_ERROR(getSystemState(), XIL_ERROR_USER,
                      "di-266", TRUE, this);
        return;
    }

    //
    //  Make sure size will not overflow 32 bits
    //
    if(INT_MAX/xSize/ySize < nBands) {
        XIL_OBJ_ERROR(getSystemState(), XIL_ERROR_USER,
                      "di-129", TRUE, this);
        return;
    }

    //
    //  Initialize our tile size to the defaults in the system state.
    //
    unsigned int default_xsize;
    unsigned int default_ysize;
    getDefaultTileSize(&default_xsize, &default_ysize);

    if(initTileSize(default_xsize, default_ysize) == XIL_FAILURE) {
        XIL_OBJ_ERROR(getSystemState(), XIL_ERROR_INTERNAL,
                      "di-392", FALSE, this);
        return;
    }

    isOKFlag = TRUE;
}

//------------------------------------------------------------------------
//
//  Function:	XilImage::XilImage (for xil_create_from_type())
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
//  Parameters:
//	
//	
//  Returns:
//	
//	
//  Side Effects:
//	
//	
//  Notes:
//	
//	
//  Deficiencies/ToDo:
//	
//	
//------------------------------------------------------------------------
XilImage::XilImage(XilSystemState* system_state,
                   XilImageFormat* image_format,
                   Xil_boolean     temporary) :
    XilImageFormat(system_state, image_format, XIL_IMAGE),
    attributeHashTable(23, system_state, FALSE)
{
    if(isOKFlag == FALSE) {
        //
        //  XilImageFormat didn't initialize.
        //
        return;
    }

    isOKFlag = FALSE;

    if(initializeBasicDataMembers(temporary) == XIL_FAILURE) {
        return;
    }

    //
    //  Make sure size will not overflow 32 bits
    //
    if(INT_MAX/xSize/ySize < nBands) {
        XIL_OBJ_ERROR(getSystemState(), XIL_ERROR_USER,
                      "di-129", TRUE, this);
        return;
    }

    //
    //  Initialize our tile size to the defaults in the system state.
    //
    unsigned int default_xsize;
    unsigned int default_ysize;
    getDefaultTileSize(&default_xsize, &default_ysize);

    if(initTileSize(default_xsize, default_ysize) == XIL_FAILURE) {
        XIL_OBJ_ERROR(getSystemState(), XIL_ERROR_INTERNAL,
                      "di-392", FALSE, this);
        return;
    }

    //
    //  Propagate the colorspace from the given image format.
    //
    //  This must be done after tiles are initialized because setting the
    //  colorspace causes a sync() to occur which requires tile sizes to be
    //  set.
    //
    if(image_format->refColorspace() != NULL) {
        setColorspace(image_format->refColorspace());
    }

    isOKFlag = TRUE;
}

//------------------------------------------------------------------------
//
//  Function:	XilImage::XilImage (for xil_create_from_device())
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
//  Parameters:
//	
//	
//  Returns:
//	
//	
//  Side Effects:
//	
//	
//  Notes:
//	
//	
//  Deficiencies/ToDo:
//	
//	
//------------------------------------------------------------------------
XilImage::XilImage(XilSystemState* system_state,
                   XilDevice*      ) :
    XilImageFormat(system_state, XIL_IMAGE),
    attributeHashTable(23, system_state, FALSE)
{
    if(isOKFlag == FALSE) {
        //
        //  XilImageFormat didn't initialize.
        //
        return;
    }

    isOKFlag = FALSE;

    if(initializeBasicDataMembers() == XIL_FAILURE) {
        return;
    }

    //
    //  Make sure size will not overflow 32 bits
    //
    if(INT_MAX/xSize/ySize < nBands) {
        XIL_OBJ_ERROR(getSystemState(), XIL_ERROR_USER,
                      "di-129", TRUE, this);
        return;
    }

    //
    //  TODO:  10/4/95  jlf  Exported state for devices
    //    I/O Devices should contain the information on whether their backing
    //    storage can be exported and it probably should not be a blanket
    //    statement which is made for all I/O devices as in XIL 1.2.
    //
    exportMode = XIL_NOT_EXPORTABLE;

    //
    //  Initialize our tile size to the defaults in the system state.
    //
    unsigned int default_xsize;
    unsigned int default_ysize;
    getDefaultTileSize(&default_xsize, &default_ysize);

    if(initTileSize(default_xsize, default_ysize) == XIL_FAILURE) {
        XIL_OBJ_ERROR(getSystemState(), XIL_ERROR_INTERNAL,
                      "di-392", FALSE, this);
        return;
    }

    //
    //  The storage information associated with I/O devices is not valid at
    //  construction time -- i.e. it requires a capture to get the current
    //  state of the device in order to get the storage completely valid.
    //
    setStorageValidFlag(FALSE);

    isOKFlag = TRUE;
}

//------------------------------------------------------------------------
//
//  Function:	XilImage::XilImage (used by XilImage::createChild())
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
//  Parameters:
//	
//	
//  Returns:
//	
//	
//  Side Effects:
//	
//	
//  Notes:
//	
//	
//  Deficiencies/ToDo:
//	
//	
//------------------------------------------------------------------------
XilImage::XilImage(XilSystemState* system_state,
                   XilImage*       init_parent,
                   unsigned int    x_offset,
                   unsigned int    y_offset,
                   unsigned int    x_size, 
                   unsigned int    y_size, 
                   unsigned int    band_offset,
                   unsigned int    num_bands) :
    XilImageFormat(system_state, XIL_IMAGE),
    attributeHashTable(23, system_state, FALSE)
{
    if(isOKFlag == FALSE) {
        //
        //  XilImageFormat didn't initialize.
        //
        return;
    }

    isOKFlag = FALSE;

    if(initializeBasicDataMembers() == XIL_FAILURE) {
        return;
    }
    
    //
    //  Make sure size will not overflow 32 bits
    //
    if(INT_MAX/x_size/y_size < num_bands) {
        XIL_OBJ_ERROR(getSystemState(), XIL_ERROR_USER,
                      "di-129", TRUE, this);
        return;
    }

    //
    //  Initialize the Image Size in XilImageFormat
    //
    xSize      = x_size;
    ySize      = y_size;
    xPixelSize = -1.0;
    yPixelSize = -1.0;
    nBands     = num_bands;
    dataType   = init_parent->getDataType();

    //
    //  Children are not parents of other children.  There is only one parent.
    //
    if(init_parent->parent != NULL) {
        parent = init_parent->parent;
    } else {
        parent = init_parent;
    }

    //
    //  Initialize child offsets.
    //
    offsetX       = init_parent->offsetX    + x_offset;
    offsetY       = init_parent->offsetY    + y_offset;
    offsetBand    = init_parent->offsetBand + band_offset;

    //
    //  Initialize other inherited attributes from parent.
    //
    exportMode = parent->exportMode;

    //
    //  Children don't need their tile sizes initialize since they inherit
    //  their tile sizes from their parent.
    //

    isOKFlag = TRUE;
}

//------------------------------------------------------------------------
//
//  Function:	XilImage::initializeBasicDataMembers()
//
//  Description:
//	Initializes most all of the data members to a known state.
//	
//	
//	
//  MT-level:  Safe
//	
//------------------------------------------------------------------------
XilStatus
XilImage::initializeBasicDataMembers(Xil_boolean temporary)
{
    //
    //  Child image supoprt
    //
    parent               = NULL;
    immediateParent      = NULL;
    offsetX              = 0;
    offsetY              = 0;
    offsetBand           = 0;

    //
    //  Image origin support
    //
    originX              = 0.0F;
    originY              = 0.0F;

    //
    // Initialize the roi
    //
    roi = NULL;
    globalSpaceRoi = NULL;
    globalSpaceRoiWithDoubles = NULL;
    extendedGlobalSpaceRoi = NULL;

    //
    //  Tiled image data
    //
    tileArray            = NULL;
    tileXSize            = 0;
    tileYSize            = 0;
    tileSizeIsSetFlag    = FALSE;
    numXTiles            = 0;
    numYTiles            = 0;
    numTiles             = 0;

    //
    //  Exported image information
    //
    storageMovement      = XIL_ALLOW_MOVE;
    exportMode           = XIL_IMPORTED;
    
    //
    //  Variable for tracking storage aquisition via
    //  getExportedMemoryStorage() 
    //
    exportedImageStorage = NULL;

    //
    //  User defined function for data supply
    //  and the user arguments to be used in calling routine
    //
    dataSupplyFunc = NULL;
    supplyUserArgs = NULL;

    //
    //  No I/O device attached to the image at this point.
    //
    ioDevice = NULL;

    //
    //  Initialize the alternate image to NULL.
    //
    altImage = NULL;

    //
    //  Set our memory storage record cache to NULL so it gets initialized
    //
    memoryStorageRecord = NULL;

    //
    //  Check the construction of the hash table.
    //
    if(attributeHashTable.isOK() == XIL_FAILURE) {
        return XIL_FAILURE;
    }

    //
    //  Set out tilingMode from our system state.  This is done here because
    //  with deferred execution, the user may have changed the tiling mode by
    //  the time the image is used if we wait and the result will be
    //  incorrect.
    //
    tilingMode = getSystemState()->getDefaultTilingMode();

    //
    //  Indicate whether this is a temporary image.  
    //
    if(temporary) {
        //
        //  Indicate the object is "temporary"
        //
        markTemp();

        //
        //  A temporary image cannot be exported.
        //
        exportMode = XIL_NOT_EXPORTABLE;
    }

    //
    //  We succeeded...
    //
    return XIL_SUCCESS;
}

XilImage::~XilImage()
{
    //
    //  Things wern't initialized properly so return immediately.
    //
    if(isOKFlag == FALSE) {
        return;
    }

    //
    //  If I'm a child, then I need to remove myself from my parent's child
    //  list and release my parent's lock.
    //
    if(immediateParent != NULL) {
        immediateParent->removeChild(this);
        parent->unlock();
    }

    //
    //  If I'm a parent, then I need to destroy all of the children.
    //  It's important to note that this is done before the storage is
    //  deallcoated below because destroying a child may cause an operation to
    //  be flushed on the child.
    //
    XiliBagIterator it(&children);
    XilImage*       child;
    while(child = (XilImage*)it.getNext()) {
        //
        //  Clear the child's I/O device if it has one because if it has
        //  one, it's the same reference as ours and would cause the child
        //  to destroy us a second time if it's not set to NULL.
        //
        if(ioDevice != NULL) {
            child->ioDevice = NULL;
        }
        child->destroy();
    }

    //
    //  Deallocate the image storage.
    //
    deallocateAllStorage();

    //
    //  Clean up the tileArray
    //
    if(tileArray != NULL) {
        delete [] tileArray;
        tileArray = NULL;
        tileSizeIsSetFlag = FALSE;
    }

    //
    //  Remove all of the information from our storageList
    //
    XiliSLListIterator<XiliStorageRecord*> sli(&storageList);
    XiliStorageRecord*                     rec;
    while(sli.getNext(rec) == XIL_SUCCESS) {
        delete rec;
    }

    //
    //  Destroy the ROIs.
    //
    if(roi != NULL) {
        roi->destroy();
    }
    if(globalSpaceRoi != NULL) {
        globalSpaceRoi->destroy();
    }
    if(globalSpaceRoiWithDoubles != NULL) {
        globalSpaceRoiWithDoubles->destroy();
    }
    if(extendedGlobalSpaceRoi != NULL) {
        extendedGlobalSpaceRoi->destroy();
    }

    //
    //  For the case where the I/O device provided a controlling image that
    //  was a child.
    //
    if(ioDevice != NULL && parent != NULL) {
        parent->destroy();
    } else {
	delete ioDevice;
    }

    //
    //  If there is an alternate image attached to this image, then destroy it
    //  now as well.
    //
    if(altImage != NULL) {
        altImage->destroy();
    }
}

//------------------------------------------------------------------------
//
//  Function:	XilImage::createChild()
//
//  Description:
//	Creates a new child image from this parent.
//	
//	
//	
//	
//	
//	
//  MT-level:  <??????>
//	
//  Parameters:
//	
//	
//  Returns:
//	
//	
//  Side Effects:
//	
//	
//  Notes:
//	
//	
//  Deficiencies/ToDo:
//	
//	
//------------------------------------------------------------------------
XilImage*
XilImage::createChild(unsigned int    x_offset,
                      unsigned int    y_offset,
                      unsigned int    x_size, 
                      unsigned int    y_size, 
                      unsigned int    band_offset,
                      unsigned int    num_bands)
{
    //
    //  Verify the validity of the arguments
    //
    if((x_offset >= xSize)                ||
       (y_offset >= ySize)                ||
       (band_offset >= nBands)            ||
       ((x_offset+x_size) > xSize)        ||
       ((y_offset+y_size) > ySize)        ||
       ((band_offset+num_bands) > nBands) ||
       (x_size == 0)                      ||
       (y_size == 0)                      ||
       (num_bands == 0)                   ||
       (isTemp() == TRUE)) {
        //
        // prevents deadlock in error handler (see other unlocks below)
        // TODO:  sigel 1-Oct-1996 should locks be used in the error handlers?
        //                         unlock should probably not be required here.
        //
        unlock();
        XIL_OBJ_ERROR(getSystemState(), XIL_ERROR_USER, "di-146", TRUE, this);
        return NULL;
    }

    //
    //  Create the child image
    //
    XilImage* child_image =
        new XilImage(getSystemState(), this, x_offset, y_offset,
                     x_size, y_size, band_offset, num_bands);

    if(child_image == NULL) {
        unlock();
        XIL_OBJ_ERROR(getSystemState(), XIL_ERROR_RESOURCE,
                      "di-1", TRUE, this);
        XIL_OBJ_ERROR(getSystemState(), XIL_ERROR_SYSTEM,
                      "di-147", FALSE, this);
        return NULL;
    }

    if(child_image->isOK() == FALSE) {
        unlock();
        XIL_OBJ_ERROR(getSystemState(), XIL_ERROR_SYSTEM,
                      "di-147", FALSE, this);
        return NULL;
    }

    if(addChild(child_image) == XIL_FAILURE) {
        unlock();
        child_image->destroy();
        return NULL;
    }

    //
    //  Set us as the immediate parent so if it goes away, we know to remove
    //  it from our list of children.  This *is* necessary because the user
    //  could destroy this image and we only want to destroy its children, not
    //  all of the children in the head parent.
    //
    child_image->immediateParent = this;

    //
    //  If the child has the same number of bands as the parent, then the
    //  child inherits the the colorspace of the parent.
    //
    if(num_bands == nBands && colorspace != NULL) {
        child_image->setColorspace(colorspace);
    }
    
    child_image->xPixelSize = this->xPixelSize;
    child_image->yPixelSize = this->yPixelSize;

    return child_image;
}

XilStatus
XilImage::addChild(XilImage* child)
{
    if(children.insert(child) == XIL_FAILURE) {
        XIL_OBJ_ERROR(getSystemState(),
                      XIL_ERROR_SYSTEM, "di-147", FALSE, this);
        return XIL_FAILURE;
    }

    return XIL_SUCCESS;
}

XilStatus
XilImage::removeChild(XilImage* child)
{
    children.remove(child);

    return XIL_SUCCESS;
}

//------------------------------------------------------------------------
//
//  Function:	XilImage::createCopy
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
//  Parameters:
//	
//	
//  Returns:
//	
//	
//  Side Effects:
//	
//	
//  Notes:
//	
//	
//  Deficiencies/ToDo:
//	
//	
//------------------------------------------------------------------------
XilObject*
XilImage::createCopy()
{
    return createCopy(0, 0, xSize, ySize, 0, nBands);
}

XilImage*
XilImage::createCopy(unsigned int x_offset,
                     unsigned int y_offset,
                     unsigned int x_size, 
                     unsigned int y_size, 
                     unsigned int band_offset,
                     unsigned int num_bands)
{
    //
    //  Verify the validity of the arguments
    //
    if((x_offset >= xSize)                ||
       ((x_offset+x_size) > xSize)        ||
       (x_size==0)                        ||
       (y_offset >= ySize)                ||
       ((y_offset+y_size) > ySize)        ||
       (y_size==0)                        ||
       (band_offset >= nBands)            ||
       ((band_offset+num_bands) > nBands) ||
       (x_size == 0)                      ||
       (y_size == 0)                      ||
       (num_bands == 0)                   ||
       (isTemp() == TRUE)) {
        XIL_OBJ_ERROR(getSystemState(),XIL_ERROR_USER,
                      "di-146",TRUE,this);
 	XIL_OBJ_ERROR(getSystemState(), XIL_ERROR_SYSTEM,
                      "di-255", FALSE, this);
        return NULL;
    }

    //
    //  Bring this image up-to-date so we can read from it.  If there are
    //  operations deferred on the image, this will cause the creation of
    //  storage (and the addition of a copy operation).  If there are no
    //  operations deferred on the image, then no data will be generated and
    //  no copy will be done.
    //
    if(sync() == XIL_FAILURE) {
 	XIL_OBJ_ERROR(getSystemState(), XIL_ERROR_SYSTEM,
                      "di-255", FALSE, this);
        return NULL;
    }

    //
    // Create the new image
    //
    XilImage* new_image =
        getSystemState()->createXilImage(x_size, y_size, num_bands, dataType);
    if(new_image == NULL) {
 	XIL_OBJ_ERROR(getSystemState(), XIL_ERROR_SYSTEM,
                      "di-255", FALSE, this);
        return NULL;
    }

    //
    //  Copy appropriate attributes
    //  note that ROI and origin are not copied
    //
    new_image->setColorspace(getColorspace());

    //
    //  If the images are identical, then copy over the versioning
    //  information.
    //
    //  NOTE:  Don't use getRoi() because it returns a copy.
    //
    if((x_offset == 0) && (x_size  == xSize) &&
       (y_offset == 0) && (y_size == ySize) &&
       (band_offset == 0) && (num_bands == nBands) &&
       (roi == NULL) &&
       (originX == 0.0F) &&
       (originY == 0.0F)) {
        new_image->copyVersionInfo(this);
    }

    //
    //  If there is data associated with this image, then copy the data from
    //  this image to the copy.
    //
    //  Or, if the data stored in this image is not valid, then we should
    //  force a copy operation which will insert a capture which in turn will
    //  update this image with the valid storage information.
    //
    //  If the images do not have an equal number of bands, create a temporary
    //  child image to facilitate the copy.
    //
    Xil_boolean has_data = FALSE;
    has_data = !((parent != NULL) ?
          (parent->storageList.isEmpty()) : storageList.isEmpty()); 

    if(has_data || ! isStorageValid()) {
        //
        // TODO: maynard 6/26/96
        //
        // Origins and ROIs are a problem. In order to make the operation
        // deferrable I want to try another solution like using the translate
        // op. For now, I'll copy from a child of the original src, which
        // resets the origin and roi to 0.0,0.0 and NULL respectively.
        //
        if(num_bands != nBands) {
            XilImage* child_image = createChild(x_offset,y_offset,x_size,y_size,
                                                band_offset, num_bands);
            if(child_image == NULL) {
                new_image->destroy();
                XIL_OBJ_ERROR(getSystemState(), XIL_ERROR_RESOURCE,
                              "di-1", TRUE, this);
                XIL_OBJ_ERROR(getSystemState(), XIL_ERROR_SYSTEM,
                              "di-255", FALSE, this);
                return NULL;
            }

            XilOp* copy_op = createCopyOp(child_image, new_image);
            if(copy_op == NULL) {
                child_image->destroy();
                new_image->destroy();
                XIL_OBJ_ERROR(getSystemState(), XIL_ERROR_OTHER,
                              "di-21", FALSE, this);
                XIL_OBJ_ERROR(getSystemState(), XIL_ERROR_SYSTEM,
                              "di-255", FALSE, this);
                return NULL;
            }

            //
            //  Indicate the op is not supposed to lock the DAG in this case
            //  becase we already have the DAG locked.
            //
            copy_op->insert(FALSE);

            child_image->destroy();
        } else {
            XilImage* child_image = createChild(x_offset,y_offset,x_size,y_size,
                                                band_offset, num_bands);
            if(child_image == NULL) {
                new_image->destroy();
                XIL_OBJ_ERROR(getSystemState(), XIL_ERROR_RESOURCE,
                              "di-1", TRUE, this);
                XIL_OBJ_ERROR(getSystemState(), XIL_ERROR_SYSTEM,
                              "di-255", FALSE, this);
                return NULL;
            }

            XilOp* copy_op = createCopyOp(child_image, new_image);
            if(copy_op == NULL) {
                child_image->destroy();
                new_image->destroy();
                XIL_OBJ_ERROR(getSystemState(), XIL_ERROR_OTHER,
                              "di-21", FALSE, this);
                XIL_OBJ_ERROR(getSystemState(), XIL_ERROR_SYSTEM,
                              "di-255", FALSE, this);
                return NULL;
            }

            //
            //  Indicate the op is not supposed to lock the DAG in this case
            //  becase we already have the DAG locked.
            //
            copy_op->insert(FALSE);

            child_image->destroy();
        }
    }
    
    return new_image;
}

Xil_boolean
XilImage::canBeTiled()
{
    if(parent) {
	return parent->canBeTiled();
    }
    
    return TRUE;
}

Xil_boolean
XilImage::isTileSizeSet()
{
    if(parent) {
	return parent->isTileSizeSet();
    }
    
    return tileSizeIsSetFlag;
}

XilStatus
XilImage::initTileSize(XilDeferrableObject* def_object)
{
    if(parent) {
	parent->initTileSize(def_object);
    }
    
    //
    //  Don't reset our tile size with this call, just initialize a new size.
    //
    if(isTileSizeSet()) {
        return XIL_FAILURE;
    }

    unsigned int txsize;
    unsigned int tysize;
    def_object->getTileSize(&txsize, &tysize);

    return initTileSize(txsize, tysize);
}

//
//  This routine can be used to initialize tilesize for the first time,
//  or to reinitialize the tileArray and tilesize if it changes.
//  The new_tileArray is an optional argument. If it is NULL, the routine
//  will set the image tileArray directly. If it is not NULL, then it
//  has storage associated with it (from a tile resize) and the image's
//  tileArray needs to be that.
//
XilStatus
XilImage::initTileSize(unsigned int txsize,
                       unsigned int tysize,
                       XilTile* new_tileArray)
{
    if(parent) {
	return parent->initTileSize(txsize, tysize,new_tileArray);
    }
    
    if((txsize == 0) || (txsize > xSize)) {
        txsize = xSize;
    }

    if((tysize == 0) || (tysize > ySize)) {
        tysize = ySize;
    }
    
    tileXSize = txsize;
    tileYSize = tysize;

    //
    //  tilingMode was set at creation of the image and determines
    //  how the tileXSize and tileYSize get used.
    //  But we can catch invalid settings here
    //
    if((tileXSize != xSize) && tilingMode == XIL_STRIPPING) {
        //
        //  For stripping, the tileXSize must be the width of the image
        //
        XIL_OBJ_ERROR(getSystemState(), XIL_ERROR_USER, "di-361", TRUE, this);
        return XIL_FAILURE;
    }    

    numXTiles = ((xSize+tileXSize-1)/tileXSize);
    numYTiles = ((ySize+tileYSize-1)/tileYSize);
    numTiles  = numXTiles*numYTiles;

    //
    // TODO : maynard 7/18/96 - check this
    //  If the old tileArray still exists, delete it before continuing. We are 
    //  assuming here that there is no storage set on the old tileArray. That 
    //  should get caught by a higher routine, eg : setExportedTileSize.
    //  It would be too expensive to add the test in here?
    //
    if(tileArray != NULL) {
        delete [] tileArray;
        tileArray = NULL;
        tileSizeIsSetFlag = FALSE;
    }

    if(new_tileArray == NULL) {
        tileArray = new XilTile[numTiles];
        if(tileArray == NULL) {
            XIL_OBJ_ERROR(getSystemState(), XIL_ERROR_RESOURCE, "di-1",TRUE, this);
            return XIL_FAILURE;
        }
        for(unsigned int i=0; i<numTiles; i++) {
            tileArray[i].getBox()->setAsRect((i%numXTiles) * tileXSize,
                                             ((i/numXTiles)%numYTiles) * tileYSize,
                                             tileXSize, tileYSize);
        }
    } else {
        tileArray = new_tileArray;
        //
        // TODO: maynard 7/18/96 SEGV potential
        // If the outside routine (in this case setExportedTileSize) miscalculates the
        // numTiles when creating new_tileArray, then later loops through tileArray
        // will segv. Should I pass in the creating numTiles or assume that we will
        // write clean code?
        //
    }

    //
    //  Inidicate the tile size has been set.
    //
    tileSizeIsSetFlag = TRUE;
    
    return XIL_SUCCESS;
}

void
XilImage::getDefaultTileSize(unsigned int* txsize,
                             unsigned int* tysize)
{
    //
    //  If the tilingMode is set to no tiling, then the tile size is defined
    //  to be the entire image.
    //
    if(tilingMode == XIL_WHOLE_IMAGE) {
        *txsize = 0;
        *tysize = 0;
        return;
    }

    //
    //  This routine is called when the image tile size has not been set by
    //  the user to determine an appropriate tile size for the image.  First,
    //  we check the system state to see if the application has explicitly set
    //  a default tile size.  If one has not been set, then we use the
    //  bytesPerTile variable from the global state to set a tile size for the
    //  image based upon the image's datatype and number of bands.
    //
    getSystemState()->getDefaultTileSize(txsize, tysize);

    //
    //  If the tilingMode is set to XIL_STRIPPING then we only return 
    //  the present tysize. txsize will be set to the width of the image.
    //
    if(tilingMode == XIL_STRIPPING) {
        *txsize = 0;
    }

    if(*txsize == 0 && *tysize == 0) {
        //
        //  We'll need to calculate a new size based on bytesPerTile.
        //
        unsigned int bpt = XilGlobalState::theXGS->getBytesPerTile();

        //
        //  Divide bpt by the size of a pixel (number of bands in the image
        //  multiplied by the datatype size) to get the number of pixels in
        //  each tile.
        //
        unsigned int ppt = bpt / (xili_sizeof(dataType) * nBands);

        if(dataType == XIL_BIT) {
            //
            //  There are 8 pixels in each datatype element.
            //
            ppt *= 8;
        } else if(dataType == XIL_UNSIGNED_4) {
            //
            //  There are 2 pixels in each datatype element.
            //
            ppt *= 2;
        }

        //
        //  Based on the tiling mode, set the txsize and tysize.
        //
        if(tilingMode == XIL_STRIPPING) {
            *txsize = 0;

            //
            //  Calculate the ysize of a tile that's xSize long -- shrink
            //  based on the fact we want strips to contain significantly less
            //  data than tiles.
            //
            *tysize = ppt/xSize/4;

            if(*tysize < 2) {
                *tysize = 1;
            } else {
                //
                //  Make it a multiple of two for simplicity.
                //
                *tysize &= 0xfffffffe;
            }
        } else {
            //
            //  Now do a sqrt() to calculate the xsize/ysize for the tile.
            //
            unsigned int tsize = (unsigned int)sqrt((double)ppt);

            //
            //  If tsize is less than XIL's minimum default tile size of 64x64,
            //  set it to 64x64.  Otherwise, set it to a size that is 64-pixel
            //  aligned and smaller than the calculated tile dimension -- all
            //  based on the tiling mode.
            //
            if(tsize < 64) {
                tsize = 64;
            } else {
                tsize &= 0xffffff80;
            }

            *txsize = tsize;
            *tysize = tsize;
        }
    }
}

XilStatus
XilImage::getTileSize(unsigned int* txsize,
                      unsigned int* tysize)
{
    if(parent) {
	return parent->getTileSize(txsize, tysize);
    }
	
    *txsize = tileXSize;
    *tysize = tileYSize;

    return XIL_SUCCESS;
}

unsigned int
XilImage::getNumTiles()
{
    if(parent) {
        return parent->getNumTiles();
    }

    return numTiles;
}

unsigned int
XilImage::getNumXTiles()
{
    if(parent) {
        return parent->getNumXTiles();
    }

    return numXTiles;
}

unsigned int
XilImage::getNumYTiles()
{
    if(parent) {
        return parent->getNumYTiles();
    }

    return numYTiles;
}


void
XilImage::getExportedTileSize(unsigned int* txsize,
                              unsigned int* tysize)
{
    if(parent) {
	parent->getExportedTileSize(txsize, tysize);
	return;
    }
    
    if(exportMode != XIL_EXPORTED) {
        *txsize = 0;
        *tysize = 0;
        XIL_OBJ_ERROR(getSystemState(), XIL_ERROR_USER,
                      "di-396", TRUE, this);
    } else {
        if(getTileSize(txsize, tysize) == XIL_FAILURE) {
            XIL_OBJ_ERROR(getSystemState(), XIL_ERROR_USER,
                          "di-395", FALSE, this);
        }
    }
}

XilStatus
XilImage::setExportedTileSize(unsigned int txsize,
                              unsigned int tysize)
{

    //
    //  Setting tile size on a child image is not supported.
    //
    if(parent != NULL) {
        XIL_OBJ_ERROR(getSystemState(), XIL_ERROR_USER, "di-451",
                      TRUE, this);
        return XIL_FAILURE;
    }

    //
    //  Image must be exported.
    //
    if(exportMode != XIL_EXPORTED) {
        XIL_OBJ_ERROR(getSystemState(), XIL_ERROR_USER,
                      "di-396", TRUE, this);
        return XIL_FAILURE;
    }

    if((txsize != tileXSize) && (tysize != tileYSize)) {
        //
        //  Tiling mode must support tiles...
        //
        if(tilingMode == XIL_WHOLE_IMAGE) {
            XIL_OBJ_ERROR(getSystemState(), XIL_ERROR_USER,
                          "di-452", TRUE, this);
            return XIL_FAILURE;
        }
    
        //
        //  Sync the image since we cannot defer through tile size changes.
        //
        allSync();

        //
        //  Update the version number before changing the object.
        //
        newVersion();
        
        if(isTileSizeSet() == FALSE) {
            if(initTileSize(txsize, tysize) == XIL_FAILURE) {
                XIL_OBJ_ERROR(getSystemState(), XIL_ERROR_SYSTEM,
                              "di-392", FALSE, this);
                return XIL_FAILURE;
            }
        } else {
            //
            //  Find out whether the image has storage allocated, by iterating
            //  over the tileArray for the whole image. 
            //
            Xil_boolean  storage_exists = FALSE;
            unsigned int i;
            
            for(i=0; i<getNumTiles(); i++) {
                if(tileArray[i].getStorage() != NULL) {
                    //
                    //  We have a tile with storage...
                    //
                    storage_exists = TRUE;
                    break;
                }
            }

            //
            //  If no storage is set on the image, simply change the tilesize
            //
            if(!storage_exists) {
                if(initTileSize(txsize,tysize) == XIL_FAILURE) {
                    XIL_OBJ_ERROR(getSystemState(), XIL_ERROR_SYSTEM,
                                  "di-392", FALSE, this);
                    return XIL_FAILURE;
                }
            } else {
                //
                //  If storage is set on the image, the existing storage needs
                //  to be reformatted to the new tilesize.
                //  We know we (memory storage) have ownership of the storage
                //  because the image is exported.
                //
                XilTile* new_tileArray;
                unsigned int new_numXTiles;
                unsigned int new_numYTiles;
                unsigned int new_numTiles;
                unsigned int new_tileXSize;
                unsigned int new_tileYSize;

                if(txsize > xSize) {
                    txsize = xSize;
                }
                
                if(tysize > ySize) {
                    tysize = ySize;
                }
                
                new_tileXSize = txsize;
                new_tileYSize = tysize;
                
                //
                //  If the given tile size is 0, then we set the tile size so there is
                //    only 1 tile and it's the size of the image.
                //
                if(new_tileXSize == 0 && new_tileYSize == 0) {
                    new_tileXSize = xSize;
                    new_tileYSize = ySize;
                } else if(new_tileXSize == 0) {
                    //
                    //  A strip is ok
                    //
                    new_tileXSize = xSize;
                } else if(new_tileYSize == 0 || tilingMode == XIL_STRIPPING) {
                    XIL_OBJ_ERROR(getSystemState(), XIL_ERROR_USER,
                                  "di-361", TRUE, this);
                    return XIL_FAILURE;
                }
                
                new_numXTiles = ((xSize+new_tileXSize-1)/new_tileXSize);
                new_numYTiles = ((ySize+new_tileYSize-1)/new_tileYSize);
                new_numTiles  = new_numXTiles*new_numYTiles;
                
                new_tileArray = new XilTile[new_numTiles];
                if(new_tileArray == NULL) {
                    XIL_OBJ_ERROR(getSystemState(), XIL_ERROR_RESOURCE,
                                  "di-1", TRUE, this);
                    return XIL_FAILURE;
                }

                for(i=0; i<new_numTiles; i++) {
                    new_tileArray[i].getBox()->setAsRect(
                        (i%new_numXTiles) * new_tileXSize,
                        ((i/new_numXTiles)%new_numYTiles) * new_tileYSize,
                        new_tileXSize, new_tileYSize);
                    //
                    // This is ok to do because we know all tiles have
                    // to be on the memory device - this image is exported
                    //
                    new_tileArray[i].setStorageRecord(tileArray[0].getStorageRecord());
                }

                //
                // Now that you have a new tileArray set up, reformat the
                // existing tileArray's storage into its new home, deleting
                // storage after copying it over so that we don't end up
                // with two complete copies prior to deletion.
                //

                //
                // For each of the original tiles, calculate which of the
                // new (bigger or smaller) tiles it intersects. Then
                // decobble the original tile into the tile list. Then
                // delete the original tile storage. I go from 
                // original to new so I can delete an original tile as 
                // soon as possible.
                //
                XilTileNumber*   intersect_tile_list =
                    new XilTileNumber[new_numTiles];

                if(intersect_tile_list == NULL) {
                    XIL_OBJ_ERROR(getSystemState(), XIL_ERROR_RESOURCE,
                                  "di-1", TRUE, this);
                    return XIL_FAILURE;
                }

                unsigned int     num_intersect_tiles;
                XilBox*          orig_tile_box;
                void*            attribs = NULL;

                for(unsigned int k=0; k<numTiles; k++) {
                    num_intersect_tiles = 0;
                    orig_tile_box = tileArray[k].getBox();
                    for(unsigned int j=0; j<new_numTiles; j++) {
                        if(tileArray[k].tilesOverlap(&new_tileArray[j])) {
                            intersect_tile_list[num_intersect_tiles] = j;

                            //
                            // If this is the first time we're decobbling into a 
                            // new tile then we'll need to allocate the storage first. 
                            //
                            if(new_tileArray[j].getStorage() == NULL) {
                               XilStorage* tmp_storage = new XilStorage(this);

                               if(tmp_storage == NULL) {
                                   XIL_OBJ_ERROR(getSystemState(), XIL_ERROR_RESOURCE,
                                                 "di-1", TRUE, this);
                                   return XIL_FAILURE;
                               }

                               //
                               //  It doesn't really matter what the access is
                               //  because it's not used, but if it were, we would
                               //  definitely be allocating for both reading and writing
                               //
                               tileArray[k].getStorageDevice()->allocate(tmp_storage,
                                                                         new_tileXSize,
                                                                         new_tileYSize,
                                                                         XIL_STORAGE_TYPE_UNDEFINED,
                                                                         XIL_READ_WRITE,
                                                                         attribs);

                               new_tileArray[j].setStorage(tmp_storage);
                            }
                            num_intersect_tiles++;
                        }
                    }

                    if(num_intersect_tiles > 0) {
                        //
                        // Create a tile list to handle the decobble.
                        //
                        XilTileList new_tile_list(getSystemState());

                        //
                        // Associate the tile list with the new_tileArray
                        // and appropriate box.
                        //
                        new_tile_list.setTileArray(new_tileArray);
                        new_tile_list.setArea(orig_tile_box->getX(), orig_tile_box->getY(),
                                              orig_tile_box->getXSize(),
                                              orig_tile_box->getYSize());

                        //
                        //  Set the number of tiles to be contained in the
                        //  list. 
                        //
                        if(new_tile_list.setNumTiles(num_intersect_tiles) ==
                           XIL_FAILURE) { 
                            XIL_OBJ_ERROR(getSystemState(), XIL_ERROR_INTERNAL,
                                          "di-422", FALSE, this);
                            return XIL_FAILURE;
                        }

                        //
                        //  Add the tiles to the list.
                        //
                        for(i=0; i<num_intersect_tiles; i++) {
                            new_tile_list.setEntry(i, intersect_tile_list[i]);
                        }

                        //
                        //  We know old and new tiles will all be on memorystorage 
                        //  because we're exported, so we can use the storageDevice
                        //  from the old tiles to decobble to the new tiles.
                        //
                        if(tileArray[k].getStorageDevice()->decobble(
                            tileArray[k].getStorage(), orig_tile_box,
                            &new_tile_list) == XIL_FAILURE) {
                            //
                            //  TODO - maynard 7/16/96 Clean up sufficient?
                            //  clean up the new tile array - is this right? 
                            //
                            XIL_OBJ_ERROR(getSystemState(), XIL_ERROR_SYSTEM,
                                          "di-TODO", FALSE, this);

                            //
                            //  First I need to deallocate any storage on
                            //  new_tileArray.
                            //
                            delete [] new_tileArray;
                            delete intersect_tile_list;
                            return XIL_FAILURE;

                        }
                    }

                    deallocateTileStorage(&tileArray[k]);
                }

                //
                // We have now removed all the original tiles
                //
                delete intersect_tile_list;

                //
                // Reset the tileArray and information. This will also delete old
                // empty tileArray;
                //
                if(initTileSize(txsize, tysize, new_tileArray) == XIL_FAILURE) {
                    XIL_OBJ_ERROR(getSystemState(), XIL_ERROR_SYSTEM,
                                  "di-392", FALSE, this);
                    return XIL_FAILURE;
                }
            }
        }
    }

    return XIL_SUCCESS;
}

//------------------------------------------------------------------------
//
//  Function:	XilImage::getTileList()
//
//  Description:
//	This routine intersects the given box with the tiles in the 
//      image and fills in the given tile list object with tile numbers.
//
//------------------------------------------------------------------------
XilStatus
XilImage::getTileList(XilTileList* tile_list,
                      XilBox*      area)
{
    //
    //  The parent has the storage and tile array information.
    //
    if(parent != NULL) {
        if(area == NULL) {
            XilBox box(offsetX, offsetY, xSize, ySize);

            return parent->getTileList(tile_list, &box);
        } else {
            return parent->getTileList(tile_list, area);
        }
    }

    //
    //  Initialize/Get the tile size.
    //
    unsigned int txsize;
    unsigned int tysize;
    if(getTileSize(&txsize, &tysize) == XIL_FAILURE) {
        XIL_OBJ_ERROR(getSystemState(), XIL_ERROR_INTERNAL,
                      "di-422", TRUE, this);
        return XIL_FAILURE;
    }

    //
    //  Once the tile size is initialized, we have a tileArray.  Make the tile
    //  list aware of our tile array.
    //
    tile_list->setTileArray(tileArray);

    //
    //  Special case for single tiled images.
    //
    if(numTiles == 1) {
        //
        //  Initialize the tile_list to contain the one tile which is the
        //  entire image. 
        //
        tile_list->setNumTiles(1);
        tile_list->setEntry(0, 0);
        tile_list->setArea(0, 0, xSize, ySize);

        return XIL_SUCCESS;
    }

    //
    //  Default case of more than one tile in the image.
    //
    int          box_x;
    int          box_y;
    unsigned int box_xsize;
    unsigned int box_ysize;
    int          band;
    if(area == NULL) {
        box_x     = 0;
        box_y     = 0;
        box_xsize = xSize;
        box_ysize = ySize;
    } else {
        area->getStorageLocation(&box_x, &box_y,
                                 &box_xsize, &box_ysize, &band);
    }

    //
    //  Store the area the tile list represents.
    //
    tile_list->setArea(box_x, box_y, box_xsize, box_ysize);

    //
    //  Compute which tile we start iterating at and the size of the tile area
    //  we iterate over.
    //
    unsigned int xstart = box_x/txsize;
    unsigned int ystart = box_y/tysize;
    unsigned int xend   = (box_x + box_xsize - 1)/txsize;
    unsigned int yend   = (box_y + box_ysize - 1)/tysize;

    unsigned int xtiles  = xend - xstart + 1;
    unsigned int ytiles  = yend - ystart + 1;

    //
    //  Set the number of tiles to be contained in the list.
    //
    if(tile_list->setNumTiles(xtiles*ytiles) == XIL_FAILURE) {
        XIL_OBJ_ERROR(getSystemState(), XIL_ERROR_INTERNAL,
                      "di-422", FALSE, this);
        return XIL_FAILURE;
    }

    //
    //  Loop over the tile area and set the entries on the list.
    //
    unsigned int entry    = 0;
    unsigned int tile_num = ystart*numXTiles + xstart;
    for(unsigned int y=ystart; y <= yend; y++) {
        unsigned int tile = tile_num;

        for(unsigned int x=xstart; x <= xend; x++) {
            tile_list->setEntry(entry++, tile++);
        }

        tile_num += numXTiles;
    }

    return XIL_SUCCESS;
}

XilStatus
XilImage::getTileList(XilTileList*     tile_list,
                      XiliRect*        rect)
{
    //
    //  The parent has the storage and tile array information.
    //
    if(parent != NULL) {
        if(rect == NULL) {
            XiliRectInt r(offsetX, offsetY, xSize, ySize);

            return parent->getTileList(tile_list, &r);
        } else {
            return parent->getTileList(tile_list, rect);
        }
    }

    //
    //  Initialize/Get the tile size.
    //
    unsigned int txsize;
    unsigned int tysize;
    if(getTileSize(&txsize, &tysize) == XIL_FAILURE) {
        XIL_OBJ_ERROR(getSystemState(), XIL_ERROR_INTERNAL,
                      "di-422", TRUE, this);
        return XIL_FAILURE;
    }

    //
    //  Once the tile size is initialized, we have a tileArray.  Make the tile
    //  list aware of our tile array.
    //
    tile_list->setTileArray(tileArray);

    //
    //  Special case for single tiled images.
    //
    if(numTiles == 1) {
        //
        //  Initialize the tile_list to contain the one tile which is the
        //  entire image. 
        //
        tile_list->setNumTiles(1);
        tile_list->setEntry(0, 0);
        tile_list->setArea(0, 0, xSize, ySize);

        return XIL_SUCCESS;
    }

    //
    //  Default case of more than one tile in the image.
    //
    int x1;
    int y1;
    int x2;
    int y2;
    if(rect == NULL) {
        x1 = 0;
        y1 = 0;
        x2 = xSize - 1;
        y2 = ySize - 1;
    } else {
        rect->get(&x1, &y1, &x2, &y2);
    }

    //
    //  Store the area the tile list represents.
    //
    tile_list->setArea(x1, y1, (x2 - x1 + 1), (y2 - y1 + 1));

    //
    //  Compute which tile we start iterating at and the size of the tile area
    //  we iterate over.
    //
    unsigned int xstart = x1/txsize;
    unsigned int ystart = y1/tysize;
    unsigned int xend   = x2/txsize;
    unsigned int yend   = y2/tysize;

    unsigned int xtiles  = xend - xstart + 1;
    unsigned int ytiles  = yend - ystart + 1;

    //
    //  Set the number of tiles to be contained in the list.
    //
    if(tile_list->setNumTiles(xtiles*ytiles) == XIL_FAILURE) {
        XIL_OBJ_ERROR(getSystemState(), XIL_ERROR_INTERNAL,
                      "di-422", FALSE, this);
        return XIL_FAILURE;
    }

    //
    //  Loop over the tile area and set the entries on the list.
    //
    unsigned int entry    = 0;
    unsigned int tile_num = ystart*numXTiles + xstart;
    for(unsigned int y=ystart; y <= yend; y++) {
        unsigned int tile = tile_num;

        for(unsigned int x=xstart; x <= xend; x++) {
            tile_list->setEntry(entry++, tile++);
        }

        tile_num += numXTiles;
    }

    return XIL_SUCCESS;
}

XilStatus
XilImage::getTileRect(XiliRect*     rect,
                      XilTileNumber tile_number)
{
    //
    //  The parent has the storage and tile array information.
    //
    if(parent != NULL) {
        return parent->getTileRect(rect, tile_number);
    }

    //
    //  Initialize/Get the tile size.
    //
    unsigned int txsize;
    unsigned int tysize;
    if(getTileSize(&txsize, &tysize) == XIL_FAILURE) {
        XIL_OBJ_ERROR(getSystemState(), XIL_ERROR_INTERNAL,
                      "di-422", TRUE, this);
        return XIL_FAILURE;
    }

    int x1 = (tile_number % numXTiles) * txsize;
    int y1 = (tile_number / numXTiles) * tysize;

    rect->set(x1, y1, (x1 + txsize - 1), (y1 + tysize - 1));

    return XIL_SUCCESS;
}

XilStatus
XilImage::clipToTile(XilTileNumber  tile_number,
                     XiliRect*      rect)
{
    // 
    //  If this is a child image, clip in the parent space
    //
    if(parent) {
        //
        //  If the tile_number is -1 (no tile), do nothing
        //
        if(parent->tileArray != NULL && tile_number != -1) {
            //
            //  Pick up the parent's tile box
            //
            unsigned int x = parent->tileArray[tile_number].getBox()->getX();
            unsigned int y = parent->tileArray[tile_number].getBox()->getY();

            //
            //  Clip the incoming box against the tile box in the parent space
            //
            rect->translate(offsetX, offsetY);

            XiliRectInt tile_rect(x, y,
                                  (x + parent->tileXSize - 1),
                                  (y + parent->tileYSize - 1));
            rect->clip(&tile_rect);

            //
            //  Set coordinate box back to child image coordinates 
            //
            rect->translate(-((float)offsetX), -((float)offsetY));

            if(rect->isEmpty()) {
                return XIL_FAILURE;
            }
        }
        
        return XIL_SUCCESS;
    }

    //
    //  If we're getting the information for the entire object
    //  (tile_number == -1) or the tile information has not been set on this
    //  object, do nothing, the box will stay as is.
    //
    if(tileArray != NULL && tile_number != -1) {
        XilBox*      tile_box = tileArray[tile_number].getBox();
        unsigned int x = tile_box->getX();
        unsigned int y = tile_box->getY();

        XiliRectInt tile_rect(x, y, (x + tileXSize - 1), (y + tileYSize - 1));
        rect->clip(&tile_rect);
    }

    if(rect->isEmpty()) {
        return XIL_FAILURE;
    }

    return XIL_SUCCESS;
}

XiliRect*
XilImage::getGlobalSpaceRect()
{
    if(globalSpaceRect.isEmpty()) {
        globalSpaceRect.set(-originX, -originY,
                            -originX + xSize - 1,
                            -originY + ySize - 1);
    }

    return &globalSpaceRect;
}

//
//  API/GPI Functions
//
//------------------------------------------------------------------------
//
//  Function:	XilImage::setRoi()
//
//  Description:
//	Set's the images Roi to that passed in
//	
//------------------------------------------------------------------------
void
XilImage::setRoi(XilRoi* newroi)
{
    if(isTemp()) {
        //
        //  If it's a temporary image and has been written into (i.e. it's
        //  in the valid state), then it cannot be changed.
        //
        if(isValid()) {
            XIL_OBJ_ERROR(getSystemState(), XIL_ERROR_USER, "di-428",
                          TRUE, this);
            return;
        }
    }

    //
    //  Synchronize the image since an attribute is about to change
    //
    allSync();

    //
    //  Update the version number before changing the image.
    //
    newVersion();
    
    if(globalSpaceRoi != NULL) {
        globalSpaceRoi->destroy();
	globalSpaceRoi=NULL;
    }
    if(globalSpaceRoiWithDoubles != NULL) {
        globalSpaceRoiWithDoubles->destroy();
        globalSpaceRoiWithDoubles = NULL;
    }
    if(extendedGlobalSpaceRoi != NULL) {
        extendedGlobalSpaceRoi->destroy();
        extendedGlobalSpaceRoi = NULL;
    }

    if(newroi != NULL) {
        //
        //  Copy the new ROI
        //
        XilRoi* new_roi = (XilRoi*)newroi->createCopy();
	if(new_roi==NULL) {
	    XIL_OBJ_ERROR(getSystemState(), XIL_ERROR_SYSTEM,
                          "di-269", FALSE, this);
	    return;
	}

        //
	//  Destroy the existing roi
        //
	if(roi != NULL) {
	    roi->destroy();
	}

	roi = new_roi;
    } else {
        //
        //  Just destroy the existing ROI
        //
        if(roi != NULL) {
            roi->destroy();
            roi = NULL;
        }
    }
}

//------------------------------------------------------------------------
//
//  Function:	XilImage::getRoi()
//
//  Description:
//	Pass back a copy of the images roi
//	
//------------------------------------------------------------------------
XilRoi*
XilImage::getRoi()
{
    if(roi != NULL) {
        return (XilRoi*)roi->createCopy();
    } else {
        return NULL;
    }
}

XilImage*
XilImage::getParent()
{
    return parent;
}

void
XilImage::getChildOffsets(unsigned int* x, 
			  unsigned int* y,
			  unsigned int* band)
{
    if(x != NULL) {
        *x = offsetX;
    }

    if(y != NULL) {
        *y = offsetY;
    }

    if(band != NULL) {
        *band = offsetBand;
    }
}

//------------------------------------------------------------------------
//
//  Function:	XilImage::import()
//
//  Description:
//	Imports this image to back under XIL's control.
//	
//	
//	
//	
//  MT-level:  Unsafe
//	
//  Parameters:
//	
//	
//  Returns:
//	
//	
//  Side Effects:
//	
//	
//  Notes:
//	
//	
//  Deficiencies/ToDo:
//	
//	
//------------------------------------------------------------------------
void
XilImage::import(Xil_boolean image_changed_flag)
{
    //
    // If we have a parent just ask them to do the work
    //
    if(parent) {
        parent->import(image_changed_flag);
        return;
    }

    //
    //  Was it even exported?
    //
    if(exportMode != XIL_EXPORTED) {
        XIL_OBJ_ERROR(getSystemState(), XIL_ERROR_USER, "di-153", TRUE, this);
        return;
    }

    //
    //  Update our version number if the image changed
    //
    if(image_changed_flag == TRUE) {
        newVersion();
        //
        //  We do not need to decobble the exportedImageStorage explicitly,
        //  because releaseStorage() will do so when called below (via
        //  destroyExportedStorage()).
        //
    }

    //
    //  Mark the image as not exported.
    //
    exportMode = XIL_IMPORTED;

    //
    // We can only decobble the exportedImageStorage buffer if
    // the storageMovment flag allows the storage address to
    // change. REPLACE and NO_MOVE are currently both implemented
    // as NO_MOVE.
    //
    if(storageMovement == XIL_ALLOW_MOVE) {
        //
        //  Destroy any exportedImageStorage that was allocated.
        //  This includes destroying all of those on our children because they are
        //  no longer valid either. Then, set each of the exportedImageStorage
        //  pointers to NULL to flag its creation next time.
        //
        destroyExportedStorage();
    }

    //  
    // TODO: maynard 3/13/96`
    //       We still need to figure out how we're handling sync'ing of the
    //       cobbled buffer and the tiles. If we hand out references (for
    //       tiled access) into the cobbled region, we'd need to check
    //       refcounts before destroying - and possibly block before changing.
    //
}

//
//  Destroy all the exportedImageStorage objects on the given image and its
//  children.
//
void
XilImage::destroyExportedStorage()
{
    XiliBagIterator it(&children);
    XilImage*       child;
    while(child = (XilImage*)it.getNext()) {
        child->destroyExportedStorage();
    }

    if(exportedImageStorage != NULL) {
        exportedImageStorage->destroy();
        exportedImageStorage = NULL;
    }
}

//------------------------------------------------------------------------
//
//  Function:	XilImage::exportStorage()
//
//  Description:
//	Changes this image or the parent of this image to be in
//      the exported state.
//	
//	
//	
//	
//	
//	
//	
//  MT-level:  Unsafe
//	
//  Parameters:
//	
//	
//  Returns:
//	
//	
//  Side Effects:
//	
//	
//  Notes:
//	
//	
//  Deficiencies/ToDo:
//	
//	
//------------------------------------------------------------------------
XilStatus
XilImage::exportStorage(Xil_boolean permit_device_export)
{
    if(parent) {
        XilStatus status = parent->exportStorage();

        return status;
    }

    //
    //  Bring this image up-to-date if we need to.  When we're given complete
    //  permission to move the storage however we want, the application has no
    //  valid pointer to the storage so we defer sync'ing until they request
    //  storage. 
    //
    if(storageMovement != XIL_ALLOW_MOVE) {
        allSync();
    }

    //
    //  Are we already exported?
    //
    if(exportMode == XIL_EXPORTED) {
        return XIL_SUCCESS;
    }
    
    //
    //  TODO: jlf 10/4/95  Add the else-if case for devices.
    //
    //    We need to see if this is a device image and if so, ask it to export
    //    itself.  If that succeeds, we're exported otherwise, we're still
    //    imported.
    //
    //  If this is a device image it can't be exported - or at least that
    //  was consistently the case for 1.2
    //
    if(exportMode == XIL_NOT_EXPORTABLE && ! permit_device_export) {
        XIL_OBJ_ERROR(getSystemState(), XIL_ERROR_USER, "di-152", TRUE, this);
        if(isTemp()) {
            //
            //  If it's a temporary image, then we need to generate an error
            //  that an illegal operation has been attempted.
            //
            XIL_OBJ_ERROR(getSystemState(), XIL_ERROR_USER, "di-428",
                          FALSE, this);
        }

        return XIL_FAILURE;
    }

    //
    //  For now, just mark this image as exported.
    //
    exportMode = XIL_EXPORTED;

    return XIL_SUCCESS;
}

int
XilImage::getExported()
{
    if(parent) {
	return parent->getExported();
    } else {
        return exportMode;
    }
}

//------------------------------------------------------------------------
//
//  Function:	syncForStorage()
//
//  Description:
//
//    Perform the necessary sync'ing required to get this image ready for
//    storage aquisition.
//
//  MT-level:  UNsafe
//	
//------------------------------------------------------------------------
XilStatus
XilImage::syncForStorage(XilTileList*     tile_list,
                         XilOp*           op,
                         XilImage*        image,
                         XilStorageAccess access)
{
    if(op == NULL) {
        //
        //  We're not being called for a particular op.
        //
        //  These calls refer to the final entry on the queue if a
        //  queue position is not specified.  The final entry on the queue
        //  is the current representation at the API which is where a call
        //  with op == NULL should have originated.  If it didn't, then
        //  syncing the tail entry will only mean more is sync'd than may
        //  have been necessary.
        //
        if(access == XIL_READ_ONLY || access == XIL_READ_WRITE) {
            if(syncForReading(tile_list) == XIL_FAILURE) {
                return XIL_FAILURE;
            }
        }

        if(access == XIL_WRITE_ONLY || access == XIL_READ_WRITE) {
            if(syncForWriting(tile_list) == XIL_FAILURE) {
                return XIL_FAILURE;
            }
        }
    } else {
        if(image == NULL) {
            image = this;
        }

        //
        //  If the image is being used for reading, then it needs to be
        //  brought up-to-date via a sync().
        //
        if(access == XIL_READ_ONLY || access == XIL_READ_WRITE) {
            //
            //  Check to see if this image is a source image to the given
            //  operation.  If so, we need to sync() it in order to bring its
            //  data up-to-date.
            //
            XiliOpQueuePosition pos = op->getSrcOpQueuePosition(image);

            if(pos != _XILI_OP_QUEUE_INVALID_POSITION) {
                if(syncForReading(tile_list, op, pos) == XIL_FAILURE) {
                    return XIL_FAILURE;
                }
            }
        }

        //
        //  If we're planning on writing into the image, then we need to
        //  evaluate all of the other operations that write into it prior
        //  to obtaining it for writing.
        //
        if(access == XIL_WRITE_ONLY || access == XIL_READ_WRITE) {
            //
            //  Check to see if this image is a destination image to the given
            //  operation.  If so, we need to evalute all of the previous
            //  ops writing into this operation and flush and ops that are
            //  dependent upon the current values it in order to use it
            //  for writing.
            //
            XiliOpQueuePosition pos = op->getDstOpQueuePosition(image);

            if(pos != _XILI_OP_QUEUE_INVALID_POSITION) {
                if(syncForWriting(tile_list, op, pos) == XIL_FAILURE) {
                    return XIL_FAILURE;
                }
            }
        }
    }

    return XIL_SUCCESS;
}

//------------------------------------------------------------------------
//
//  Function:	stPrepareChild()
//
//  Description:
//	Sets up the child_box and tile_list information so getStorage()
//	can be called on the parent image.
//
//      The child_box is expected to be set equal to the box given to
//      getStorage() or getTiledStorage() prior to calling this routine.
//
//  Deficiencies/TODO:
//      Too large to be inlined by existing compilers.
//	
//------------------------------------------------------------------------
XilStatus
XilImage::stPrepareChild(XilOp*           op,
                         XilStorageAccess access,
                         XilBox*          child_box,
                         XilTileList*     tile_list)
{
    //
    //  To handle our child status, we offset the front box to take our
    //  child x and y offsets into account for the box.
    //
    child_box->offsetFront(offsetX, offsetY);

    //
    //  We only need to lock and sync if we're accessing tiles.
    //
    //  If we're exported and there is memory storage for this
    //  image, then we can go right to the getStorage call on the parent.
    //
    if(parent->exportedImageStorage == NULL) {
        //
        //  We need to lock our storage mutex prior to syncing for storage and
        //  setting it on our tile_list. 
        //
        parent->storageMutex.lock();

        //
        //  Now set this mutex on the tile list so that if the DE algorithms need
        //  to block for other operations, they can release the storage lock.
        //
        tile_list->setMutex(&parent->storageMutex);
            
        //
        //  Get the list of tiles we need to sync and bring the area we're
        //  aquiring storage for up-to-date.  This potentially involves
        //  flushing operations which depend on this image and flushing the
        //  operations which generated this image.
        //
        if(parent->getTileList(tile_list, child_box) == XIL_FAILURE ||
           parent->syncForStorage(tile_list, op,
                                  this, access) == XIL_FAILURE) {
            return XIL_FAILURE;
        }

        //
        //  We unlock our parent's storage mutex because it's going to
        //  reaquire it the next time.
        //
        parent->storageMutex.unlock();
    }

    return XIL_SUCCESS;
}
    
//------------------------------------------------------------------------
//
//  Function:	stGetToDevice()
//
//  Description:
//	Utility for syncing the image's storage and getting the tiles
//      to the correct device.
//	
//  Deficiencies/TODO:
//      Too large to be inlined by existing compilers.
//
//------------------------------------------------------------------------
XilStatus
XilImage::stGetToDevice(XilBox*             box,
                        XilOp*              op,
                        XilStorageAccess    access,
                        XilStorageType      type_requested,
                        void*               attribs,
                        XilTileList*        tile_list,
                        XiliStorageRecord*  srec)
{
    //
    //  Now set this mutex on the tile list so that if the DE algorithms need
    //  to block for other operations, they can release the storage lock.
    //
    tile_list->setMutex(&storageMutex);

    //
    //  Get the list of tiles we're going need to process.
    //
    if(getTileList(tile_list, box) == XIL_FAILURE) {
        return XIL_FAILURE;
    }
        
    //
    //  Bring the area we're aquiring storage for up-to-date.  This
    //  potentially involves flushing operations which depend on this image
    //  and flushing the operations which generated this image.
    //
    if(syncForStorage(tile_list, op, this, access) == XIL_FAILURE) {
        return XIL_FAILURE;
    }
    
    //
    //  Ensure all of the tiles we need are on the target storage device.
    //
    if(getToDevice(tile_list, srec, type_requested,
                   access, attribs) == XIL_FAILURE) {
        return XIL_FAILURE;
    }

    return XIL_SUCCESS;
}

//------------------------------------------------------------------------
//
//  Function:	XilImage::getStorage()
//
//  Description:
//	Returns the storage for the given region.
//	If the type_requested is XIL_STORAGE_TYPE_UNDEFINED, then
//      any type is acceptable.  Otherwise, the type_requested 
//	will be returned.
//      If exportedImageStorage is set (there exists a cobbled version
//      of the data storage) pick up the storage from the cobbled storage
//      to stay in Sync.
//      xil_import() will copy from exportedImageStorage to the tile
//      array and destroy exportedImageStorage if the storage movment
//      flag allows.
//      If the storageMovment flags were DONT_MOVE or REPLACE then
//      exportedImageStorage can even exist after import.
//	
//  MT-level:  SAFE
//	
//  Side Effects:
//	Can cause storage to be propagated from one storage device
//      to another.
//	
//  Notes:
//	It should not produce error messages because the compute
//      routine may decide to try other storage types.
//	
//  Deficiencies/ToDo:
//	
//	
//------------------------------------------------------------------------
XilStatus
XilImage::getStorage(XilStorage*      storage,
                     XilOp*           op,
                     XilBox*          box,
                     char*            storage_name,
                     XilStorageAccess access,
                     XilStorageType   type_requested,
                     void*            attribs)
{
    //////////////////////////////////////////////////////////////////////
    //
    //  NOTE:  We do not generate error messages in this routine because
    //         if it fails, the compute routine may look for another
    //         storage device.
    //
    //////////////////////////////////////////////////////////////////////

    //
    //  So we don't SEGV, check the pointers we'll dereference.
    //
    if(storage      == NULL ||
       storage_name == NULL ||
       box          == NULL) {
        return XIL_FAILURE;
    }

    //
    //  The list which holds the tiles the given box corresponds to.
    //  This is filled in by a call to getTileList() once the box has been
    //  appropriately adjusted.
    //
    XilTileList tile_list(getSystemState());

    //
    //  Clear any storage information already set on the storage object we're
    //  going to fill.  We do this prior to aquiring the storage lock because
    //  releasing the storage information may call releaseStorage() which
    //  continues to protect the image's storage information.
    //
    storage->release();

    //
    //  The parent image has the storage for the children.
    //
    if(parent) {
        XilBox child_box(box);

        if(stPrepareChild(op, access,
                          &child_box, &tile_list) == XIL_FAILURE) {
            return XIL_FAILURE;
        }

        XilStatus status = parent->getStorage(storage, op, &child_box,
                                              storage_name, access,
                                              type_requested, attribs);
        
        return status;
    }

    //
    //  Use this boolean to indicate whether we're using the memory storage
    //  device -- we cache the record and other information if it is...
    //
    Xil_boolean is_memory_device = (strcmp(storage_name, "XilMemory") == 0);

    //
    //  If the image is exported and they're requesting storage on a device
    //  other than XilMemory, then the request will fail.
    //
    //  If the image is imported but the storage movement flags
    //  are set to DONT_MOVE or to REPLACE (which is currently handled
    //  as DONT_MOVE) and they're requesting storage on a device other
    //  than  XilMemory, then the request will fail.
    //
    if((exportMode == XIL_EXPORTED || storageMovement != XIL_ALLOW_MOVE) &&
       !is_memory_device) {
        return XIL_FAILURE;
    }

    //
    //  Special case of getStorage when the image is being accessed via
    //  exportedImageStorage - so long as the exposed cobbled region exists,
    //  all getStorage calls will pick up pointers into the cobbled region and
    //  the image operations must stay sync'd.
    //
    //  xil_import() will copy the data from the cobbled region into the
    //  tiles (except in the case of storageMovement flags set to DONT_MOVE
    //  or REPLACE) where it stays around.
    //
    if(exportedImageStorage != NULL) {
        //
        //  Because memory storage is exported non-bit images must be
        //  XIL_PIXEL_SEQUENTIAL and bit images must be requested as
        //  XIL_BAND_SEQUENTIAL.  STORAGE_TYPE_UNDEFINED is always okay.
        //
        XilStorageType type_needed =
            (dataType == XIL_BIT ? XIL_BAND_SEQUENTIAL : XIL_PIXEL_SEQUENTIAL);

        if((type_requested != XIL_STORAGE_TYPE_UNDEFINED) &&
           (type_requested != type_needed)) {
            return XIL_FAILURE;
        } 

        //
        //  We have a single mutex for this image which protects the access to
        //  storage by the image.
        //
        storageMutex.lock();

        //
        //  The "front" box will give us the spatial offset information.  The
        //  storage portion of the box contains the band offset.
        //
        int boxx = box->getX();
        int boxy = box->getY();
        int band = box->stGetBand();
        if(boxx != 0 || boxy != 0 || band != 0) {
            //
            //  We know the device is XilMemory
            //
            XiliStorageRecord* srec;
            if(memoryStorageRecord == NULL) {
                srec = getStorageRecord("XilMemory");
                if(srec == NULL) {
                    storageMutex.unlock();
                    return XIL_FAILURE;
                }

                memoryStorageRecord = srec;
            } else {
                srec = memoryStorageRecord;
            }

            if(srec->getDevice()->setStorage(storage,
                                             exportedImageStorage,
                                             boxx,
                                             boxy,
                                             band) == XIL_FAILURE) {
                storageMutex.unlock();
                return XIL_FAILURE;
            }
        } else {
            //
            //  Set the existing storage structure into the one provided.
            // 
            storage->setInfo(exportedImageStorage);
        }

        //
        //  Now reset the storageTag information to be unique to
        //  this storage so exportedImageStorage's pointers don't
        //  get munged when storage is destroyed.
        //
        XiliStorageTag* tmp_tag = storage->refStorageTag();

        //
        //  We don't need to decobble when done.
        //
        tmp_tag->tileList = NULL;

        //
        //  We don't need box if tileList is NULL;
        //
        tmp_tag->box      = NULL;

        //
        //  This data belongs to exportedImageStorage.
        //
        tmp_tag->ownsData = FALSE;

        storageMutex.unlock();

        return XIL_SUCCESS;
    }

    //
    //  If we've gotten to here, exportedImageStorage is NOT set.
    //

    //
    //  We have a single mutex for this image which protects the access to
    //  storage by the image.
    //
    storageMutex.lock();

    //
    //  Get the storage record which contains the storage device for the given
    //  storage type.
    //
    XiliStorageRecord* srec;
    if(is_memory_device) {
        if(memoryStorageRecord == NULL) {
            memoryStorageRecord = getStorageRecord(storage_name);
            if(memoryStorageRecord == NULL) {
                return XIL_FAILURE;
            }
        }
        srec = memoryStorageRecord;
    } else {
        srec = getStorageRecord(storage_name);
        if(srec == NULL) {
            return XIL_FAILURE;
        }
    }

    //
    //  Setup the tile_list, sync the storage and get the tiles to the
    //  target device.
    //
    if(stGetToDevice(box, op, access, type_requested, attribs,
                     &tile_list, srec) == XIL_FAILURE) { 
        storageMutex.unlock();
        return XIL_FAILURE;
    }

    //
    //  If there is more than one tile in the list and storage is handled as
    //  tiles (as opposed to stripps), then we need to request that the
    //  storage device to cobble the data into a single contiguous buffer.
    //  At this point we know that all of the tiles are on the target storage
    //  device.  So, just ask it to cobble them together for us.
    //
    //  Another reason we need to cobble is the case where the type of storage
    //  requested by the caller and the type of storage on the device differ.
    //
    XilStorage* tile0_storage =
        tileArray[tile_list.getTileNumber(0)].getStorageDescription();

    if((tilingMode == XIL_TILING && tile_list.getNumTiles() > 1) ||
       ! tile0_storage->isType(type_requested)) {
        //
        //  TODO: jlf 3/18/96  We want to keep a list of cobbled regions.
        //
        //    We should keep a list of cobbled regions to see if we've already
        //    cobbled a subset of the region the caller is requesting.
        //    In addition, this is a place to put the real storage information
        //    for the cobbled region so we can reset it for the caller's
        //    storage object.
        //
        if(srec->getDevice()->cobble(storage,
                                     box,
                                     &tile_list,
                                     type_requested,
                                     attribs) == XIL_FAILURE) {
            storageMutex.unlock();
            return XIL_FAILURE;
        }

        //
        //  Fill in a storage tag that's part of the storage object. 
        //
        XiliStorageTag* tmp_tag = storage->refStorageTag();

        XilTileList* tmp_tl = new XilTileList(getSystemState());
        if(tmp_tl == NULL) {
            storageMutex.unlock();
            return XIL_FAILURE;
        }

        tmp_tl->transferList(&tile_list);

        XilBox*      tmp_box = new XilBox(box);
        if(tmp_box == NULL) {
            delete tmp_tl;
            storageMutex.unlock();
            return XIL_FAILURE;
        }

        tmp_tag->storageDevice = srec->getDevice();
        tmp_tag->tileList      = tmp_tl;
        tmp_tag->box           = tmp_box;
        tmp_tag->isWritable    = (access == XIL_WRITE_ONLY ||
                                  access == XIL_READ_WRITE) ? TRUE : FALSE;
        //
        //  We can free data upon release of storage.
        //
        tmp_tag->ownsData      = TRUE;

        //
        //  Indicate the information in the storage tag is valid.
        //
        storage->storageTagIsSet();

        //
        //  Get the location of the "front box".  Its coordinates should have
        //  been moved into parent space by the child prior to calling this
        //  routine on the parent.  We want to adjust the pointer to the front
        //  box as opposed to the storage location so it's (0, 0) in box
        //  space. The storage location is used to guarentee the size of the
        //  aquired storage is large enough, but the pointer should be at the
        //  image coordinated described by the "front box".
        //
        int boxx = box->getX();
        int boxy = box->getY();

        //
        //  Get the storage location from the box.
        //
	int          storage_x;
        int          storage_y;
        unsigned int storage_xsize;
        unsigned int storage_ysize;
        int          storage_band;
        box->getStorageLocation(&storage_x, &storage_y,
                                &storage_xsize, &storage_ysize, &storage_band);

        unsigned int x_offset = (boxx - storage_x);
        unsigned int y_offset = (boxy - storage_y);
        
        if(x_offset != 0 || y_offset != 0) {
            //
            //  Move the storage pointer to the "front box"
            //
            tmp_tag->useOffsets = TRUE;

            tmp_tag->xOffset    = (unsigned int ) -x_offset;
            tmp_tag->yOffset    = (unsigned int ) -y_offset;
            tmp_tag->bandOffset  = 0;

            //
            //  This routine fills the given storage object and takes the
            //  given offsets into account.
            //
            if(srec->getDevice()->setStorage(storage,
                                             storage,
                                             x_offset,
                                             y_offset,
                                             0) == XIL_FAILURE) {
                storageMutex.unlock();
                return XIL_FAILURE;
            }
        } else {
            //
            //  Otherwise, initialize that we're not using offsets.
            //
            tmp_tag->useOffsets = FALSE;
        }
    } else {
        //
        //  Get the only tile in the list.
        //
        XilTile* tile = tile_list.getTile(0);

        //
        //  Get the location of the "front box".  Its coordinates should have
        //  been moved into parent space by the child prior to calling this
        //  routine on the parent.  We want to adjust the pointer to the front
        //  box as opposed to the storage location so it's (0, 0) in box
        //  space. The storage location is used to guarentee the size of the
        //  aquired storage is large enough, but the pointer should be at the
        //  image coordinated described by the "front box".
        //
        //  The subtraction here is ok because the routines that determined
        //  which tile the box corresponds to guarantees the box will be a
        //  subset of the tile.
        //
        XilBox*      tile_box = tile->getBox();
        unsigned int x_offset = (box->getX() - tile_box->getX());
        unsigned int y_offset = (box->getY() - tile_box->getY());
        int          b_offset = box->stGetBand();

        if(x_offset != 0 || y_offset != 0 || b_offset != 0) {
            //
            //  This routine fills the given storage object and takes the
            //  given offsets into account.
            //
            if(srec->getDevice()->setStorage(storage,
                                             tile0_storage,
                                             x_offset,
                                             y_offset,
                                             b_offset) == XIL_FAILURE) {
                storageMutex.unlock();
                return XIL_FAILURE;
            }
        } else {
            //
            //  Set the existing storage structure into the one provided.
            // 
            storage->setInfo(tile0_storage);
        }

        //
        //  Store tile information in the storage object for our use upon
        //  release. We don't need to fill in all the information, because
        //  in this case releaseStorage will know the tile is pointing into
        //  the tile array and will not decobble.
        //
        XiliStorageTag* tmp_tag = storage->refStorageTag();
        tmp_tag->tileList   = NULL;
        tmp_tag->tile       = tile;
        tmp_tag->useOffsets = FALSE;
        
        storage->storageTagIsSet();
    }

    storageMutex.unlock();

    return XIL_SUCCESS;
}

//------------------------------------------------------------------------
//
//  Function:	getTiledStorage()
//
//  Description:
//	Get the image's storage as a list of tiles instead of a contiguous
//      region of storage.
//	
//  MT-level:  SAFE
//
//------------------------------------------------------------------------
XilTileList*
XilImage::getTiledStorage(XilOp*           op,
                          XilBox*          box,
                          char*            storage_name,
                          XilStorageAccess access,
                          void*            attribs)
{
    //////////////////////////////////////////////////////////////////////
    //
    //  NOTE:  We do not generate error messages in this routine because
    //         if it fails, the compute routine may look for another
    //         storage device.
    //
    //////////////////////////////////////////////////////////////////////

    //
    //  So we don't SEGV, check the pointers we'll dereference.
    //
    if(storage_name == NULL ||
       box          == NULL) {
        return NULL;
    }

    //
    //  The list which holds the tiles the given box corresponds to.
    //  This is filled in by a call to getTileList() once the box has been
    //  appropriately adjusted.  The compute routine is returned a copy of
    //  this tile_list.
    //
    XilTileList tile_list(getSystemState());

    //
    //  The parent image has the storage for the children.
    //
    if(parent) {
        XilBox child_box(box);

        if(stPrepareChild(op, access,
                          &child_box, &tile_list) == XIL_FAILURE) {
            return NULL;
        }

        XilTileList* tl = parent->getTiledStorage(op, &child_box,
                                                  storage_name, access,
                                                  attribs);

        return tl;
    }

    //
    //  Use this boolean to indicate whether we're using the memory storage
    //  device -- we cache the record and other information if it is...
    //
    Xil_boolean is_memory_device = (strcmp(storage_name, "XilMemory") == 0);

    //
    //  If the image is exported and they're requesting storage on a device
    //  other than XilMemory, then the request will fail.
    //
    //  If the image is imported but the storage movement flags
    //  are set to DONT_MOVE or to REPLACE (which is currently handled
    //  as DONT_MOVE) and they're requesting storage on a device other
    //  than  XilMemory, then the request will fail.
    //
    if((exportMode == XIL_EXPORTED || storageMovement != XIL_ALLOW_MOVE) &&
       !is_memory_device) {
        return NULL;
    }

    //
    //  Special case of getStorage when the image is being accessed via
    //  exportedImageStorage - so long as the exposed cobbled region exists,
    //  all getStorage() calls will pick up pointers into the cobbled region
    //  and the image operations must stay sync'd.
    //
    //  xil_import() will copy the data from the exported cobbled region into
    //  the tiles. 
    //
    if(exportedImageStorage != NULL) {
        //
        //  TODO: 11/4/96 jlf  Handle this case by providing a tile that
        //                     contains a storage pointer which points to the
        //                     exportedImageStorage.
        return NULL;
    }

    //
    //  If we've gotten to here, exportedImageStorage is NOT set.
    //

    //
    //  We have a single mutex for this image which protects the access to
    //  storage by the image.
    //
    storageMutex.lock();

    //
    //  Get the storage record which contains the storage device for the given
    //  storage type.
    //
    XiliStorageRecord* srec;
    if(is_memory_device) {
        if(memoryStorageRecord == NULL) {
            memoryStorageRecord = getStorageRecord(storage_name);
            if(memoryStorageRecord == NULL) {
                return NULL;
            }
        }
        srec = memoryStorageRecord;
    } else {
        srec = getStorageRecord(storage_name);
        if(srec == NULL) {
            return NULL;
        }
    }

    //
    //  Setup the tile_list, sync the storage and get the tiles to the
    //  target device.
    //
    if(stGetToDevice(box, op, access, XIL_STORAGE_TYPE_UNDEFINED,
                     attribs, &tile_list, srec) == XIL_FAILURE) {
        storageMutex.unlock();
        return NULL;
    }

    //
    //  The XilTileList which is returned to the caller.
    //
    XilTileList* tl = new XilTileList(getSystemState());
    if(tl == NULL) {
        storageMutex.unlock();
        return NULL;
    }

    storageMutex.unlock();

    //
    //  TODO: 11/4/96 jlf  Need to put a flag on tile list so the destructor
    //                     can release the reference counts aquired for each
    //                     tile.

    //
    //  Transfer the tile list information from our stack tile_list to the
    //  returned tile list.
    //
    tl->transferList(&tile_list);

    return tl;
}

//------------------------------------------------------------------------
//
//  Function:	XilImage::releaseStorage()
//
//  Description:
//	Releases the reference to storage returned by getStorage().
//	
//	
//	
//	
//	
//	
//	
//  Side Effects:
//	
//  Deficiencies/ToDo:
//	
//	
//------------------------------------------------------------------------
XilStatus
XilImage::releaseStorage(XilStorage* storage)
{
    XiliStorageTag* tag = storage->refStorageTag();

    if(tag->tileList == NULL) {
        if(tag->tile == NULL) {
            //
            //  Storage is not associated with a tile.  If we own the data,
            //  then deallocate the storage.
            //
            if(tag->ownsData) {
                tag->storageDevice->deallocateCobble(storage);
            }
        }
    } else {
        //
        //  Offset the data pointer if necessary...
        //
        if(tag->useOffsets) {
            //
            //  This routine fills the given storage object and takes the
            //  given offsets into account.
            //
            if(tag->storageDevice->setStorage(storage,
                                              storage,
                                              tag->xOffset,
                                              tag->yOffset,
                                              tag->bandOffset) == XIL_FAILURE) {
                storageMutex.unlock();
                return XIL_FAILURE;
            }
        }

        //
        //  A cobbled region of storage.  Either decobble the region or
        //  release it.
        //
        if(tag->isWritable) {
            tag->storageDevice->decobble(storage, tag->box,
                                         tag->tileList);
        }

        //
        // Only free the data if we were responsible for allocating it
        //
        if(tag->ownsData) {
            tag->storageDevice->deallocateCobble(storage);
        }

        delete tag->box;
        delete tag->tileList;
    }

    return XIL_SUCCESS;
}

//
//  The given tile is no longer needed -- its storage can be destroyed.
//
void
XilImage::releaseTile(XilTileNumber tnum)
{
    if(parent) {
        parent->releaseTile(tnum);
    }

    deallocateTileStorage(&tileArray[tnum]);
}

//------------------------------------------------------------------------
//
//  Function:	XilImage::setStorage()
//
//  Description:
//	Replaces the tile's storage for the given coordinates.
//
//  MT-level:  SAFE
//	
//  Deficiencies/ToDo:
//	
//	
//------------------------------------------------------------------------
XilStatus
XilImage::setExportedTileStorage(XilStorage* storage)
{
    if(parent != NULL) {
        //
        //  We don't support setting storage on a child.
        //
        XIL_OBJ_ERROR(getSystemState(),
                      XIL_ERROR_USER, "di-155", TRUE, this);
        return XIL_FAILURE;
    }

    //
    //  Is the image still imported?
    //
    if(exportMode != XIL_EXPORTED) {
        XIL_OBJ_ERROR(getSystemState(), XIL_ERROR_USER,
                      "di-154", TRUE, this);
        return XIL_FAILURE;
    }

    //
    //  Invalid storage argument?
    //
    if(storage == NULL) {
        XIL_OBJ_ERROR(getSystemState(),
                      XIL_ERROR_USER, "di-383", TRUE, this);
        return XIL_FAILURE;
    }

    //
    //  Obviously incorrect values in the storage object??
    //  Check the 0th band because that one will have to
    //  have values set.
    //  Don't check band_stride or offset - they can be zero.
    //  Pixel stride cannot be > 1 on BIT images
    //
    //  TODO maynard 4/25/97 ; Do we need a more robust check here
    //  or elsewhere 
    //
    if((storage->getDataPtr(0)==NULL) ||
       ((storage->getPixelStride(0) > 1) && (getDataType() == XIL_BIT)) ||
       (storage->getScanlineStride(0) == 0) ) {
        XIL_OBJ_ERROR(getSystemState(),
                      XIL_ERROR_USER, "di-407", TRUE, this);
        return XIL_FAILURE;

    }

    //
    //  Cannot use this call if older calls xil_get/set_memory_storage() are
    //  being used. 
    //
    if(exportedImageStorage != NULL) {
        XIL_OBJ_ERROR(getSystemState(),
                      XIL_ERROR_USER, "di-146", TRUE, this);
        return XIL_FAILURE;
    }

    //
    //  Get the tile the storage object is representing.
    //
    unsigned int xcoord;
    unsigned int ycoord;
    storage->getCoordinates(&xcoord, &ycoord);

    XilTileNumber tilenum = getTileForCoordinate(xcoord, ycoord);
    XilTile*      tile = &tileArray[tilenum];

    //
    //  The list which holds the tile we're replacing for sync'ing, etc.
    //
    XilTileList tile_list(getSystemState());

    tile_list.setTileArray(tileArray);
    tile_list.setNumTiles(1);
    tile_list.setEntry(0, tilenum);
    tile_list.setArea((xcoord/tileXSize) * tileXSize,
                      (ycoord/tileYSize) * tileYSize,
                      tileXSize, tileYSize);

    //
    //  Get the tile up-to-date with any other operations...
    //
    if(syncForStorage(&tile_list) == XIL_FAILURE) {
        return XIL_FAILURE;
    }

    //
    //  Since we're exported, it's not possible that a thread from the XIL
    //  library can be attempting to manipulat storage while we're in this
    //  routine.  Thus, there is no need to aquire the storageMutex here.
    //

    //
    //  Release the storage associated with the tile we're replacing.
    //
    deallocateTileStorage(tile);

    //
    //  Create a new storage object and copy the information from the given
    //  storage object.
    //
    XilStorage* tile_storage = new XilStorage(this);
    if(tile_storage == NULL) {
        XIL_OBJ_ERROR(getSystemState(),
                      XIL_ERROR_RESOURCE, "di-1", TRUE, this);
        return XIL_FAILURE;
    }

    tile_storage->setInfoFromStorage(storage);

    tile->setStorage(tile_storage);

    return XIL_SUCCESS;
}

//------------------------------------------------------------------------
//
//  Function:	deallocateAllStorage()
//
//  Description:
//	Deallocates all of the storage associated with this image.
//	
//------------------------------------------------------------------------
void
XilImage::deallocateAllStorage()
{
    //
    //  Destroy any outstanding image storage.
    //
    if(exportedImageStorage != NULL) {
        exportedImageStorage->destroy();
        exportedImageStorage = NULL;
    }

    //
    //  Go through and deallocate all of the storage assocated with this
    //  image.
    //
    if(tileArray != NULL) {
        for(unsigned int i=0; i<numTiles; i++) {
            deallocateTileStorage(&tileArray[i]);
        }
    }

    //
    //  Do no destroy the tileArray - that is associated with the
    //  Image when tilesize is set, not when storage is allocated.
    //
}

//------------------------------------------------------------------------
//
//  Function:	XilImage::getExportedStorage()
//
//  Description:
//	There are 2 versions of this function.  The first aquires storage
//      for the entire image and (of course) cobbles if necessary.  The other
//	aquires the storage for a single tile which intersects a point
//	in the image.
//	
//	
//	
//	
//  MT-level:  <??????>
//	
//  Parameters:
//	
//	
//  Returns:
//	
//	
//  Side Effects:
//	
//	
//  Notes:
//	
//	
//  Deficiencies/ToDo:
//     o  These functions need to keep track of the XilStorage 
//        objects that they store data into so when xil_import is
//        called, we can release the storage objects.
//
//------------------------------------------------------------------------
Xil_boolean
XilImage::getExportedStorage(XilStorageAPI* storage)
{
    if(parent) {
        //
        //  Is the image still imported?
        //
        if(parent->exportMode != XIL_EXPORTED) {
            XIL_OBJ_ERROR(getSystemState(), XIL_ERROR_USER,
                          "di-142", TRUE, this);
            return FALSE;
        }
    } else {
        //
        //  Is the image still imported?
        //
        if(exportMode != XIL_EXPORTED) {
            XIL_OBJ_ERROR(getSystemState(), XIL_ERROR_USER,
                          "di-142", TRUE, this);
            return FALSE;
        }

    }

    //
    //  This is used to support xil_get_memory_storage, so
    //  request the memory PIXEL_SEQUENTIAL, unless it's a BIT image.
    //
    XilBox tmp_box(0L, 0L, getWidth(), getHeight());
    XilDataType datatype = getDataType();
    XilStorageType request_type =
        (datatype == XIL_BIT ? XIL_BAND_SEQUENTIAL : XIL_PIXEL_SEQUENTIAL);
    
    //
    //  If this is a child image, and the parent exportedImageStorage exists,
    //  then getStorage is smart enough to fill in the child's
    //  exportedImageStorage with the offset into the parent's data.
    //
    if(getStorage(storage, &tmp_box, "XilMemory", 
                  XIL_READ_WRITE, request_type, NULL) == XIL_FAILURE) {
        XIL_OBJ_ERROR(getSystemState(), XIL_ERROR_SYSTEM,
                      "di-140", FALSE, this);
        return FALSE;
    }
    
    return TRUE;
}

Xil_boolean
XilImage::getExportedStorage(XilStorageAPI* storage,
                             unsigned int   xcoord,
                             unsigned int   ycoord)
{
    if(parent) {
        //
        //  The parent will indicate an error if the coordinates are outside
        //  the image.
        //
        Xil_boolean ret_val = parent->getExportedStorage(storage,
                                                         xcoord + offsetX,
                                                         ycoord + offsetY);
        return ret_val;
    }
    
    //
    //  Is the image still imported?
    //
    if(exportMode != XIL_EXPORTED) {
        XIL_OBJ_ERROR(getSystemState(), XIL_ERROR_USER,
                      "di-142", TRUE, this);
        return FALSE;
    }

    unsigned int txsize;
    unsigned int tysize;
    getTileSize(&txsize, &tysize);

    //
    // TODO: maynard 3/14/96
    //       It is still under discussion whether get_tile_storage would return
    //       a pointer into exportedImageStorage if that cobbled buffer already
    //       existed for the image. It would handle the syncronization issue,
    //       but could cause problems depending on the order the calls were
    //       made and staying sync'd up with compute_routines use of the tileArray
    //

    //
    //  Fill in the box that describes the extent of the tile we're
    //  looking for and then call getStorage().
    //
    XilBox tmp_box((long)((xcoord/txsize)*txsize),
                   (long)((ycoord/tysize)*tysize),
                   txsize, tysize);
    if(getStorage(storage, &tmp_box, "XilMemory",
		  XIL_READ_WRITE, XIL_STORAGE_TYPE_UNDEFINED, NULL) == XIL_FAILURE) {
	XIL_OBJ_ERROR(getSystemState(), XIL_ERROR_SYSTEM,
		      "di-140", FALSE, this);
	return FALSE;
    }
    //
    //  Now set the upper-left coordinate of the tile represented
    //  by the storage.
    //
    storage->setCoordinates(tmp_box.getX(), tmp_box.getY());
    return TRUE;
}

XilStorageAPI*
XilImage::getExportedStorageWithCopy()
{

    XilStorageAPI* storage = getSystemState()->createXilStorageAPI(this);

    if(storage == NULL) {
        XIL_OBJ_ERROR(getSystemState(), XIL_ERROR_SYSTEM, "di-374",
                      FALSE, this);
        XIL_OBJ_ERROR(getSystemState(), XIL_ERROR_SYSTEM, "di-140",
                      FALSE, this);
        return NULL;
    }

    //
    //  Set which image (if we have a parent) to make the storage calls on
    //  since the parent has all of the information, but we need to create a
    //  storage object corresponding to our size.
    //
    XilImage* img = (parent == NULL) ? this : parent;

    //
    //  Is the image still exported?
    //
    if(img->exportMode != XIL_EXPORTED) {
        XIL_OBJ_ERROR(getSystemState(), XIL_ERROR_USER, "di-142",
                      TRUE, this);
        storage->destroy();
        return NULL;
    }

    //
    //  The list of tiles we'll be setting storage.
    //
    XilBox      img_box(offsetX, offsetY, xSize, ySize, offsetBand);
    XilTileList tile_list(getSystemState());

    //
    //  Get the list of tiles corresponding to this region.
    //
    if(img->getTileList(&tile_list, &img_box) == XIL_FAILURE) {
        XIL_OBJ_ERROR(getSystemState(), XIL_ERROR_SYSTEM, "di-140",
                      FALSE, this);
        storage->destroy();
        return NULL;
    }

    //
    //  Sync any oustanding operations utilizing the existing storage.
    //
    if(img->syncForStorage(&tile_list,
                           NULL, img, XIL_READ_WRITE) == XIL_FAILURE) {
        XIL_OBJ_ERROR(getSystemState(), XIL_ERROR_SYSTEM, "di-140",
                      FALSE, this);
        storage->destroy();
        return NULL;
    }

    //
    //  Now allocate appropriate buffer space for the storage object on the
    //  memory device and cobble the tiles into the object.
    //

    //
    //  Get the storage device for the memory storage
    //
    XiliStorageRecord* srec = img->getStorageRecord("XilMemory");
    if(srec == NULL) {
        XIL_OBJ_ERROR(getSystemState(), XIL_ERROR_SYSTEM, "di-140",
                      FALSE, this);
        storage->destroy();
        return NULL;
    }

    //
    //  Ensure all of the tiles we need are on the target storage device.
    //
    if(img->getToDevice(&tile_list, srec, XIL_STORAGE_TYPE_UNDEFINED,
                        XIL_READ_WRITE, NULL) == XIL_FAILURE) {
        XIL_OBJ_ERROR(getSystemState(), XIL_ERROR_SYSTEM, "di-423",
                      FALSE, this);
        storage->destroy();
        return NULL;
    }

    //
    //  Now we know all the tiles are on the memory device, so we can cobble
    //  the tiles into the allocated storage.
    //
    //  cobble should work for both multiple tiles and for a single tile...
    //
    if(srec->getDevice()->cobble(storage, &img_box,
                                 &tile_list, XIL_STORAGE_TYPE_UNDEFINED,
                                 NULL) == XIL_FAILURE) {
        XIL_OBJ_ERROR(getSystemState(), XIL_ERROR_SYSTEM, "di-423",
                      FALSE, this);
        storage->destroy();
        return NULL;
    }

    //
    //  TODO: 8/27/96 jlf  This routine (and others) need to handle the case
    //                     where exportedImageStorage is *THE* representation
    //                     of the image storage.  When an image is exported
    //                     and xil_get_memory_storage() has been called,
    //                     exportedImageStorage is the image storage, not the
    //                     data in the tile_list.
    //

    //
    //  Set the storage tag so the storage will be released/destroyed when the
    //  storage object is destroyed.
    //

    //
    //  TODO: 8/28/96 jlf  For now, point at the 0th tile.  The storage tag
    //                     really just needs the srec in this case to get at
    //                     the storage device to call deallocate.  It doesn't
    //                     need the tile since the storage has no tile
    //                     associated with it.
    //
    XiliStorageTag* tmp_tag = storage->refStorageTag();
    tmp_tag->storageDevice = srec->getDevice();
    tmp_tag->tileList      = NULL;
    tmp_tag->tile          = NULL;
    tmp_tag->ownsData      = TRUE;
    tmp_tag->useOffsets    = FALSE;

    storage->storageTagIsSet();
    
    //
    //  In the event that the user queries the upper-left
    //  corner of the region represented by storage, they
    //  must be 0,0.
    //
    storage->setCoordinates(0,0);
    return storage;
}

XilStatus
XilImage::setExportedStorageWithCopy(XilStorageAPI* storage)
{
    //
    //  Setting memory storage on a child image is not supported.
    //
    if(parent != NULL) {
        XIL_OBJ_ERROR(getSystemState(), XIL_ERROR_USER, "di-155",
                      TRUE, this);
        return XIL_FAILURE;
    }
                      
    //
    //  Is the image still exported?
    //
    if(exportMode != XIL_EXPORTED) {
        XIL_OBJ_ERROR(getSystemState(), XIL_ERROR_USER, "di-154",
                      TRUE, this);
        return XIL_FAILURE;
    }

    //
    //  Invalid storage argument?
    //
    if(storage == NULL) {
        XIL_OBJ_ERROR(getSystemState(),
                      XIL_ERROR_USER, "di-383", TRUE, this);
        return XIL_FAILURE;
    }

    //
    //  Obviously incorrect values in the storage object??
    //  Check the 0th band because that one will have to
    //  have values set.
    //  Don't check band_stride or offset - they can be zero.
    //  Pixel stride cannot be > 1 on BIT images
    //
    //  TODO maynard 4/25/97 ; Do we need a more robust check here
    //  or elsewhere 
    //
    if((storage->getDataPtr(0)==NULL) ||
       ((storage->getPixelStride(0) > 1) && (getDataType() == XIL_BIT)) ||
       (storage->getScanlineStride(0) == 0) ) {
        XIL_OBJ_ERROR(getSystemState(),
                      XIL_ERROR_USER, "di-407", TRUE, this);
        return XIL_FAILURE;

    }
    //
    //  The list of tiles we'll be setting storage.
    //
    XilBox      img_box(0, 0, xSize, ySize);
    XilTileList tile_list(getSystemState());

    //
    //  Get the list of tiles corresponding to this region.
    //
    if(getTileList(&tile_list, &img_box) == XIL_FAILURE) {
        XIL_OBJ_ERROR(getSystemState(), XIL_ERROR_SYSTEM, "di-423",
                      FALSE, this);
        return XIL_FAILURE;
    }

    //
    //  Sync any oustanding operations utilizing the existing storage.
    //
    if(syncForStorage(&tile_list) == XIL_FAILURE) {
        XIL_OBJ_ERROR(getSystemState(), XIL_ERROR_SYSTEM, "di-423",
                      FALSE, this);
        return XIL_FAILURE;
    }

    //
    //  Update the version number before changing the image storage.
    //
    newVersion();

    //
    //  Get the storage device for the memory storage
    //
    XiliStorageRecord* srec = getStorageRecord("XilMemory");
    if(srec == NULL) {
        XIL_OBJ_ERROR(getSystemState(), XIL_ERROR_SYSTEM, "di-423",
                      FALSE, this);
        return XIL_FAILURE;
    }
    
    //
    //  Run through the list of tiles to get all of the tiles onto
    //  the memory device.
    //
    if(getToDevice(&tile_list, srec,
                   XIL_STORAGE_TYPE_UNDEFINED,
                   XIL_READ_WRITE, NULL) == XIL_FAILURE) {
        XIL_OBJ_ERROR(getSystemState(), XIL_ERROR_SYSTEM, "di-423",
                      FALSE, this);
        return XIL_FAILURE;
    }

    //
    //  Now we know all the tiles are on the memory device, so we can decobble the
    //  data from exportedImageStorage into the tiles....
    //
    //  decobble should work for both multiple tiles and for a single tile...
    //
    if(srec->getDevice()->decobble(storage,
                                   &img_box, &tile_list) == XIL_FAILURE) {
        XIL_OBJ_ERROR(getSystemState(), XIL_ERROR_SYSTEM, "di-423",
                      FALSE, this);
        return XIL_FAILURE;
    }

    //
    //  TODO: 8/27/96 jlf  This routine (and others) need to handle the case
    //                     where exportedImageStorage is *THE* representation
    //                     of the image storage.  When an image is exported
    //                     and xil_get_memory_storage() has been called,
    //                     exportedImageStorage is the image storage, not the
    //                     data in the tile_list.
    //

    return XIL_SUCCESS;
}



//------------------------------------------------------------------------
//
//  Function:	XilImage::getExportedMemoryStorage()
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
//  Parameters:
//	
//	
//  Returns:
//	
//	
//  Side Effects:
//	
//	
//  Notes:
//	This routine cannot use the start_data member of XilMemoryStorage
//      because it does not exist at the API so accessing it would cause
//      a severe memory access problem.
//	
//  Deficiencies/ToDo:
//	
//	
//------------------------------------------------------------------------
Xil_boolean
XilImage::getExportedMemoryStorage(XilMemoryStorage* mem_storage)
{

    //
    // The data buffer only exists for the parent image. The child's
    // exportedImageStorage object only provides and offset into the parent
    // data buffer.
    //
    if(parent) {
        if(parent->exportedImageStorage == NULL) {
            //
            // Don't actually fill exportedImageStorage until after
            // a temporary storage buffer is all complete.
            // This is necessary because getStorage()'s behavior varies
            // according to the existence of exportedImageStorage
            // 
            XilStorageAPI*  tmpImageStorage;        
            
            tmpImageStorage = getSystemState()->createXilStorageAPI(parent);
            
            if(tmpImageStorage == NULL) {
                //
                //  TODO:  10/5/95 jlf Do we need an error here?
                //
                return FALSE;
            }
            
            if(parent->getExportedStorage(tmpImageStorage) == FALSE) {
                //
                //  Do not generate an error because getExportedStorage() will
                //  have generated enough errors.
                //
                tmpImageStorage->destroy();
                return FALSE;
            }
            
            //
            // point exportedImageStorage to the filled XilStorageAPI object.
            // but don't destroy tmpImageStorage. That will happen through
            // destroyExportedStorage().
            //
            parent->exportedImageStorage = tmpImageStorage;
        }
    }

    //
    //  Have we generated exported storage for this image?
    //  
    if(exportedImageStorage == NULL) {
        //
        // Don't actually fill exportedImageStorage until after
        // a temporary storage buffer is all complete.
        // This is necessary because getStorage()'s behavior varies
        // according to the existence of exportedImageStorage
        // 
        XilStorageAPI*      tmpImageStorage;        

        tmpImageStorage = getSystemState()->createXilStorageAPI(this);

        if(tmpImageStorage == NULL) {
            //
            //  TODO:  10/5/95 jlf Do we need an error here?
            //
            return FALSE;
        }
               
        //
        // If this image is a child, getExportedStorage will be smart enough
        // not to allocate data space.
        //
        if(getExportedStorage(tmpImageStorage) == FALSE) {
            //
            //  Do not generate an error because getExportedStorage() will
            //  have generated enough errors.
            //
            tmpImageStorage->destroy();
            return FALSE;
        }

        //
        // point exportedImageStorage to the filled XilStorageAPI object.
        // but don't destroy tmpImageStorage. That will happen through
        // destroyExportedStorage().
        //
        exportedImageStorage = tmpImageStorage;
    }

    switch(getDataType()) {
        case XIL_BIT:
        exportedImageStorage->getStorageInfo((unsigned int*)NULL,
                                             &mem_storage->bit.scanline_stride,
                                             &mem_storage->bit.band_stride,
                                             (unsigned int*)NULL,
                                             (void**)&mem_storage->bit.data);
        mem_storage->bit.offset = exportedImageStorage->getOffset();
        //
        //  Now adjust the data pointer for the case of a band child image
        //
        mem_storage->bit.data += mem_storage->bit.band_stride*offsetBand;
        break;
            
        case XIL_BYTE:
        exportedImageStorage->getStorageInfo(&mem_storage->byte.pixel_stride,
                                             &mem_storage->byte.scanline_stride,
                                             (unsigned int*)NULL,
                                             (unsigned int*)NULL,
                                             (void**)&mem_storage->byte.data);
        //
        //  Now adjust the data pointer for the case of a band child image
        //
        mem_storage->byte.data += offsetBand;
        break;
            
        case XIL_SHORT:
        exportedImageStorage->getStorageInfo(&mem_storage->shrt.pixel_stride,
                                             &mem_storage->shrt.scanline_stride,
                                             (unsigned int*)NULL,
                                             (unsigned int*)NULL,
                                             (void**)&mem_storage->shrt.data);
        //
        //  Now adjust the data pointer for the case of a band child image
        //
        mem_storage->shrt.data += offsetBand;
        break;

        //
        //  FLOAT isn't here because it's not supported in XIL 1.2
        //
        default:
        XIL_OBJ_ERROR(getSystemState(), XIL_ERROR_SYSTEM, "di-157",
                      FALSE, this);
        return FALSE;
    }
    
    return TRUE;
}

//------------------------------------------------------------------------
//
//  Function:	XilImage::setExportedMemoryStorage()
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
//  Notes:
//	This routine cannot use the start_data member of XilMemoryStorage
//      because it does not exist at the API so accessing it would cause
//      a severe memory access problem.
//	
//  Deficiencies/ToDo:
//	
//	
//------------------------------------------------------------------------
Xil_boolean
XilImage::setExportedMemoryStorage(XilMemoryStorage* mem_storage)
{
    //
    //  Setting memory storage on a child image is not supported.
    //
    if(parent != NULL) {
        XIL_OBJ_ERROR(getSystemState(), XIL_ERROR_USER, "di-155", TRUE, this);
        return FALSE;
    }
                      
    //
    //  Is the image still exported?
    //
    if(exportMode != XIL_EXPORTED) {
        XIL_OBJ_ERROR(getSystemState(), XIL_ERROR_USER, "di-154", TRUE, this);
        return FALSE;
    }

    //
    //  Is it a datatype we support via xil_set_memory_storage()?
    //
    switch(getDataType()) {
      case XIL_BIT:
      case XIL_BYTE:
      case XIL_SHORT:
        break;

      default:
        XIL_OBJ_ERROR(getSystemState(), XIL_ERROR_USER, "di-371", TRUE, this);
        return FALSE;
    }

    //
    //  We don't sync here because the image is exported.  Since the image is
    //  exported, it would have been sync'd in exportStorage() if it was required.
    //  If things wern't sync'd in exportStorage(), then only aquiring the storage
    //  means we must generate results (there is no use in generating results
    //  for an image the user is replacing).  If they did operations while
    //  the image is exported, they would occur synchronously and they'd be
    //  done by now.
    //

    //
    //  Update the version number before changing the image storage.
    //
    newVersion();

    //
    //  Setting memory storage replaces all of the storage for the entire
    //  image.  First, we deallocate (i.e. release) all the existing storage.
    //  Since we're exported, we don't need to sync().
    //  This does not delete the tileArray, but there will be no storage
    //  associated with it.
    //
    deallocateAllStorage();

    //
    //  Since we're replacing the image with a single buffer of storage,
    //  we must reinitialize the tile size (and the tileArray) to be the
    //  entire image. 
    //
    if(initTileSize(xSize, ySize) == XIL_FAILURE) {
        XIL_OBJ_ERROR(getSystemState(), XIL_ERROR_SYSTEM, "di-392",
                      FALSE, this);
        return FALSE;
    }

    //
    //  Allocate the storage object for the tile.
    //
    XilStorage* storage = new XilStorage(this);
    if(storage == NULL) {
        XIL_OBJ_ERROR(getSystemState(), XIL_ERROR_RESOURCE,
                      "di-1", TRUE, this);
        return XIL_FAILURE;
    }

    //
    //  Set all the information and pointers from the incoming
    //  XilMemoryStorage on the first "tile" in our tile list.
    //
    tileArray[0].setStorage(storage);
    switch(getDataType()) {
        case XIL_BIT:
        storage->setDataInfo(XIL_BIT,
                             mem_storage->bit.scanline_stride,
                             mem_storage->bit.band_stride,
                             mem_storage->bit.offset,
                             (void*)mem_storage->bit.data,
                             0); // band number - always zero
        break;
            
        case XIL_BYTE:
        storage->setDataInfo(XIL_BYTE,
                             mem_storage->byte.pixel_stride,
                             mem_storage->byte.scanline_stride,
                             (void*)mem_storage->byte.data,
                             0); // band number - always zero
        break;
            
        case XIL_SHORT:
        storage->setDataInfo(XIL_SHORT,
                             mem_storage->shrt.pixel_stride,
                             mem_storage->shrt.scanline_stride,
                             (void*)mem_storage->shrt.data,
                             0); // band number - always zero
        break;
    }

    //
    //  Update the information in the storage tag to indicate the data was
    //  allocated by the user and we should not deallocate it.
    //
    XiliStorageTag* tag = storage->refStorageTag();

    //
    //  Indicate the storage is a tile within this image and that the user
    //  allocated the data...we cannot free upon release. 
    //
    tag->tileList = NULL;
    tag->box      = NULL;
    tag->tile     = &tileArray[0];
    tag->ownsData = FALSE;

    storage->storageTagIsSet();

    return TRUE;
}

//
//  Get the tile given an x and y coordinate
//
XilTileNumber
XilImage::getTileForCoordinate(unsigned int  x,
			       unsigned int  y,
                               unsigned int* xoffset,
                               unsigned int* yoffset)
{
    if(parent) {
	//
	// Take into account child offsets
	// and call the parent
	//
	unsigned int new_x = x + offsetX;
	unsigned int new_y = y + offsetY;

	return parent->getTileForCoordinate(new_x, new_y,
                                            xoffset, yoffset);
    }

    //
    //  Get the size of the tile
    //
    unsigned int txsize;
    unsigned int tysize;
    getTileSize(&txsize, &tysize);

    unsigned int xdiv = x/txsize;
    unsigned int ydiv = y/tysize;

    XilTileNumber tilenum = ydiv*numXTiles + xdiv;

    if(xoffset != NULL) {
        *xoffset = x % txsize;
        *yoffset = y % tysize;
    }

    return tilenum;
}

XiliStorageRecord*
XilImage::getStorageRecord(char* storage_type)
{
    //
    //  Do we have a storage device for this type?
    //
    XiliStorageRecord*                     srec;
    XiliSLListIterator<XiliStorageRecord*> li(&storageList);
    while(li.getNext(srec) == XIL_SUCCESS) {
        if(! strcmp(srec->getName(), storage_type)) {
            return srec;
        }
    }

    //
    //  We don't so get the storage device manager requested by the
    //    compute routine.  This will load it and create it if
    //    necessary.
    //
    XilDeviceManagerStorage* mgr =
        XilGlobalState::getXilGlobalState()->getDeviceManagerStorage(storage_type); 
    if(mgr == NULL) {
        return NULL;
    }
        
    //
    //  Now, create a storage device for this image.
    //
    XilDeviceStorage*  storage_device = mgr->constructNewDevice(this);
    if(storage_device == NULL) {
        return NULL;
    }

    //
    //  Store the information into list for future requests.
    //
    srec = new XiliStorageRecord(storage_type, mgr, storage_device);
    if(srec == NULL || srec->getName() == NULL) {
        return NULL;
    }

    //
    //  Prepend the entry to the list for a most-recently-used effect.
    //
    if(storageList.prepend(srec) == _XILI_SLLIST_INVALID_POSITION) {
        return NULL;
    }
    
    return srec;
}

XilStatus
XilImage::getToDevice(XilTile*           tile,
                      XiliStorageRecord* srec,
                      XilStorageType     type_requested,
                      XilStorageAccess   access,
                      void*              attribs)
{
    XilDeviceStorage* target_device = srec->getDevice();
    const char*       target_name   = srec->getName();

    XilDeviceStorage* current_device  = tile->getStorageDevice();
    XilStorage*       current_storage = tile->getStorage();

    if(current_storage == NULL) {
        //
        //  Allocate the storage object for this tile.
        //
        XilStorage* tmp_storage = new XilStorage(this);
        if(tmp_storage == NULL) {
            XIL_OBJ_ERROR(getSystemState(), XIL_ERROR_RESOURCE,
                          "di-1", TRUE, this);
            return XIL_FAILURE;
        }
        if(dataSupplyFunc != NULL) {
            //
            // The data for this tile will be provided by
            // the user from a callback
            // In order to do this we need to create a storageAPI object
            // for the user to fill in.
            //
            XilStorageAPI* api_tmp_storage = getSystemState()->createXilStorageAPI(this);
            if(api_tmp_storage == NULL) {
                XIL_OBJ_ERROR(getSystemState(), XIL_ERROR_SYSTEM, "di-374",
                              FALSE, this);
                XIL_OBJ_ERROR(getSystemState(), XIL_ERROR_SYSTEM, "di-140",
                              FALSE, this);
                return XIL_FAILURE;
            }

            //
            // First pick up the data supplied by the user.
            // This will be XIL_MEMORY type. 
            //
            if(tilingMode == XIL_STRIPPING) {
                // 
                //  Are we the 0th tile?
                //
                if(tile == &tileArray[0]) {
                    //
                    //  If we are, we know that the tile0 storage
                    //  has not been allocated. If it had been,
                    //  we never would have entered the original
                    //  if(current_storage == NULL)  branch.
                    //
                    if(dataSupplyFunc(this, api_tmp_storage, 0,0,
                                      xSize, ySize,
                                      supplyUserArgs) == XIL_FAILURE) {
                        api_tmp_storage->destroy();
                        delete tmp_storage;
                        return XIL_FAILURE;
                    }

                    tmp_storage->setInfo(api_tmp_storage);
                    //
                    // Don't set the storage record, because
                    // the memory is supplied by the user
                    //
                    tileArray[0].setStorage(tmp_storage);
                } else {
                    //
                    //  In this case, we know we're not tile0.
                    //  tile0 may not yet have been allocated and in
                    //  stripping we're offset from that, so we
                    //  need to check.
                    //
                    if(tileArray[0].getStorage() == NULL) {
                        //
                        //  Allocate a single chunck of storage the size of the entire
                        //  image for the 0th tile (if not alreay done).
                        //
                        if(dataSupplyFunc(this, api_tmp_storage, 0,0,
                                          xSize, ySize,
                                          supplyUserArgs) == XIL_FAILURE) {
                            api_tmp_storage->destroy();
                            delete tmp_storage;
                            return XIL_FAILURE;
                        }

                        //
                        //  Create a storage object to hold the tile)
                        //  information.
                        //
                        XilStorage* tile0_storage = new XilStorage(this);
                        if(tile0_storage == NULL) {
                            XIL_OBJ_ERROR(getSystemState(), XIL_ERROR_RESOURCE,
                                          "di-1", TRUE, this);
                            api_tmp_storage->destroy();
                            delete tmp_storage;
                            return XIL_FAILURE;
                        }
                        //
                        //  Copy the information from the API object into
                        //  the tile0 storage object
                        //
                        tile0_storage->setInfo(api_tmp_storage);
                        //
                        // Don't set the storage record, because
                        // the memory is supplied by the user
                        //
                        tileArray[0].setStorage(tile0_storage);
                    }
                    //
                    // Now establish offsets for the given tile from
                    // the whole storage 
                    // 
                    target_device->setStorage(tmp_storage,
                                              tileArray[0].getStorage(),
                                              tile->getBox()->getX(),
                                              tile->getBox()->getY(),
                                              0);
                    //
                    //  Store the device on which the tile resides,
                    //  in this case memory device.
                    //  Current device should remain NULL, since the user set
                    //  the storage, so don't set the storageRecord.
                    //
                    tile->setStorage(tmp_storage);
                }
            } else {
                if(dataSupplyFunc(this, api_tmp_storage, tile->getBox()->getX(),
                                  tile->getBox()->getY(), tileXSize, tileYSize,
                                  supplyUserArgs) == XIL_FAILURE) {
                    
                    api_tmp_storage->destroy();
                    delete tmp_storage;
                    return XIL_FAILURE;
                }
                //
                //  Copy the information from the API object into
                //  the storage object
                //
                tmp_storage->setInfo(api_tmp_storage);
                //
                //  If we were successful, then store the device on which the
                //  tile resides, in this case memory device.
                //  Current device should remain NULL, since the user set
                //  the storage, so don't set the storageRecord.
                //
                tile->setStorage(tmp_storage);
            }

            //
            //  Destroy the API object.
            //
            api_tmp_storage->destroy();

            //
            //  If the request was really for something other
            //  than memory type, then we will call this routine
            //  again to move from the memory tile to the real
            //  type requested. This time the tile will not
            //  be NULL, so we won't enter this part of the loop,
            //  we'll just do the propogation.
            //
            //  Otherwise, They requested memory, so we're all set.
            //
            if(strcmp(target_name, "XilMemory")) {
                if(getToDevice(tile, srec, type_requested, access,
                               attribs) == XIL_FAILURE) {
                    return XIL_FAILURE;
                }
            }
        } else {
            //
            // The more common case, the data will be allocated
            // through XIL.
            //

            //
            //  Attempt to allocate a tile from the storage device.
            //
            if(tilingMode == XIL_STRIPPING) {
                //
                //  Allocate a single chunck of storage the size of the entire
                //  image for the 0th tile (if not alreay done).
                //

                // 
                //  Are we the 0th tile?
                //
                if(tile == &tileArray[0]) {
                    //
                    //  If we are, we know that the tile0 storage
                    //  has not been allocated. If it had been,
                    //  we never would have entered the original
                    //  if(current_storage == NULL)  branch.
                    //
                    if(target_device->allocate(tmp_storage,
                                               xSize, ySize,
                                               type_requested, access,
                                               attribs) == XIL_FAILURE) {
                        delete tmp_storage;
                        return XIL_FAILURE;
                    }
                    //
                    //  Set the information for tileArray[0]. It's
                    //  the same as tile, so we're done.
                    //
                    tileArray[0].setStorageRecord(srec);
                    tileArray[0].setStorage(tmp_storage);
                } else {
                    //
                    //  In this case, we know we're not tile0.
                    //  tile0 may not yet have been allocated and in
                    //  stripping we're offset from that, so we
                    //  need to check.
                    //
                    if(tileArray[0].getStorageDevice() == NULL) {
                        //
                        // We must create a storage object to represent the
                        // tileArray[0] strip if this isn't that strip.
                        //
                        XilStorage* tile0_storage = new XilStorage(this);
                        if(tile0_storage == NULL) {
                            XIL_OBJ_ERROR(getSystemState(), XIL_ERROR_RESOURCE,
                                          "di-1", TRUE, this);
                            delete tmp_storage;
                            return XIL_FAILURE;
                        }
                        //
                        //  Allocate a single chunck of storage the size of the entire
                        //  image for the 0th tile (if not alreay done).
                        //
                        if(target_device->allocate(tile0_storage,
                                                   xSize, ySize,
                                                   type_requested, access,
                                                   attribs) == XIL_FAILURE) {
                            delete tile0_storage;
                            delete tmp_storage;
                            return XIL_FAILURE;
                        }
                        tileArray[0].setStorageRecord(srec);
                        tileArray[0].setStorage(tile0_storage);
                    }
                    //
                    //  Now that we are sure tile0 storage exists, simply
                    //  set tile storage using offsets from tileArray[0] storage.
                    //
                    target_device->setStorage(tmp_storage,
                                              tileArray[0].getStorage(),
                                              tile->getBox()->getX(),
                                              tile->getBox()->getY(),
                                              0);
                    //
                    //  Store the device on which the tile resides.
                    //
                    tile->setStorageRecord(srec);
                    tile->setStorage(tmp_storage);
                }
            } else {
                //
                //  TILING mode - Just allocate space for this tile.
                //
                if(target_device->allocate(tmp_storage,
                                           tileXSize, tileYSize,
                                           type_requested, 
                                           access, attribs) == XIL_FAILURE) {
                    delete tmp_storage;
                    return XIL_FAILURE;
                }

                //
                //  Store the device on which the tile resides.
                //
                tile->setStorageRecord(srec);
                tile->setStorage(tmp_storage);
            }

        }
    } else if(current_device == NULL) {
        //
        //  This means we have a storage object for the tile -- set by the
        //  user -- but no device.  If the target device is "XilMemory",
        //  then we're done.  Otherwise, we need to tell the target device
        //  to copyFromMemory().
        //
        if(strcmp(target_name, "XilMemory")) {
            //
            //  Ok, we want to get the tile on a device other than "XilMemory".
            //

            //
            //  Allocate the new storage object for this tile.
            //
            XilStorage* target_storage = new XilStorage(this);
            if(target_storage == NULL) {
                XIL_OBJ_ERROR(getSystemState(), XIL_ERROR_RESOURCE,
                              "di-1", TRUE, this);
                return XIL_FAILURE;
            }

            //
            //  Now, see if the target owner can transfer from the
            //  "XilMemory" storage description.
            //
            Xil_boolean dealloc_flag = TRUE;
            if(target_device->transferToDescription(current_storage,
                                                    "XilMemory",
                                                    target_storage,
                                                    target_name,
                                                    attribs,
                                                    tileXSize, tileYSize,
                                                    &dealloc_flag) == XIL_SUCCESS) {
                //
                //  If dealloc_flag == FALSE, then clear the information
                //  on the storage object so the user is not called.
                //
                //  TODO:  11/6/96 jlf  Needs to be a new flag on the tag.
                //
                current_storage->refStorageTag()->ownsData = FALSE;

                //
                //  Delete the old storage object.
                //
                delete current_storage;

                //
                //  We've successfully switched the storage to the new type.
                //
                tile->setStorageRecord(srec);
                tile->setStorage(target_storage);

                //
                //  Move onto the next tile.
                //
                return XIL_SUCCESS;
            }

            //
            //  Allocate a tile on the target device...
            //
            //  TODO: jlf  This should be done prior to the copies, etc.
            //
            if(target_device->allocate(target_storage,
                                       tileXSize, tileYSize,
                                       type_requested, 
                                       access, attribs) == XIL_FAILURE) {
                delete target_storage;
                return XIL_FAILURE;
            }

            //
            //  Request the target storage device copy the provided memory
            //  storage into its storage.
            //
            if(target_device->copyFromMemory(current_storage,
                                             target_storage,
                                             tileXSize,
                                             tileYSize) == XIL_FAILURE) {
                delete target_storage;
                return XIL_FAILURE;
            }

            //
            //  We've successfully switched the storage to the new type.
            //
            tile->setStorageRecord(srec);
            tile->setStorage(target_storage);
        }
    } else if(current_device != target_device ||
              (tile->isEmulated() &&
               strcmp(tile->getStorageName(), target_name) != 0)) {
        //
        //  TODO: 2/20/96 jlf Make work for stripping case.
        //
        //  There is really only one tile in the stripping case, not many.
        //

        //
        //  Can the existing device provide a description of the storage in
        //  the target format?
        //
        if(current_device->modifyForEmulation(current_storage,
                                              target_name,
                                              attribs,
                                              tileXSize,
                                              tileYSize) == XIL_SUCCESS) {
            //
            //  We've switched the storage to the new type.
            //
            //  Store the alternate storage type in the
            //  emulatedStorageType field of the tileArray[]. 
            //
            if(tile->setEmulatedName(target_name) == XIL_FAILURE) {
                return XIL_FAILURE;
            }

            return XIL_SUCCESS;
        }

        //
        //  Allocate the new storage object for this tile.
        //
        XilStorage* target_storage = new XilStorage(this);
        if(target_storage == NULL) {
            XIL_OBJ_ERROR(getSystemState(), XIL_ERROR_RESOURCE,
                          "di-1", TRUE, this);
            return XIL_FAILURE;
        }

        //
        //  Check to see if either of the devices can copy directly to the
        //  other device.
        //
        //  First, see if the current owner can copy to the target device.
        //
        const char* current_name = tile->getStorageName();
        Xil_boolean dealloc_flag = TRUE;
        if(current_device->transferToDescription(current_storage,
                                                 current_name,
                                                 target_storage,
                                                 target_name,
                                                 attribs,
                                                 tileXSize, tileYSize,
                                                 &dealloc_flag) == XIL_SUCCESS) {
            //
            //  Deallocate the old storage if we're expected to do so...
            //
            if(dealloc_flag) {
                current_device->deallocate(current_storage);
            }

            //
            //  Delete the old storage object.
            //
            delete current_storage;

            //
            //  We've successfully switched the storage to the new type.
            //
            tile->setStorageRecord(srec);
            tile->setStorage(target_storage);

            //
            //  Move onto the next tile.
            //
            return XIL_SUCCESS;
        }

        //
        //  Now, see if the target owner can copy from the current device.
        //
        if(target_device->transferToDescription(current_storage,
                                                current_name,
                                                target_storage,
                                                target_name,
                                                attribs,
                                                tileXSize, tileYSize,
                                                &dealloc_flag) == XIL_SUCCESS) {
            //
            //  Deallocate the old storage if we're expected to do so...
            //
            if(dealloc_flag) {
                current_device->deallocate(current_storage);
            }

            //
            //  Delete the old storage object.
            //
            delete current_storage;

            //
            //  We've successfully switched the storage to the new type.
            //
            tile->setStorageRecord(srec);
            tile->setStorage(target_storage);

            //
            //  Move onto the next tile.
            //
            return XIL_SUCCESS;
        }

        //
        //  Ok, none of the direct methods worked.  So, we'll allocate a
        //  tile on the "XilMemory" storage device, tell the current device
        //  (provided is not "XilMemory") to copy into it and then tell
        //  the target device to copy from it.
        //

        //
        //  Get the XilMemory device...
        //
        XiliStorageRecord* memory_rec = getStorageRecord("XilMemory");

        if(memory_rec == NULL) {
            //
            //  Houston, we've got a problem...
            //
            delete target_storage;
            return XIL_FAILURE;
        }

        //
        //  If we're not on the XilMemory device, then tell the current
        //  device to copy
        //
        XilStorage* memory_storage = NULL;

        if(strcmp(current_name, "XilMemory") != 0) {
            memory_storage = new XilStorage(this);
            if(memory_storage == NULL) {
                XIL_OBJ_ERROR(getSystemState(), XIL_ERROR_RESOURCE,
                              "di-1", TRUE, this);
                return XIL_FAILURE;
            }
                
            //
            //  Allocate the XilMemory tile...
            //
            if(memory_rec->getDevice()->allocate(memory_storage,
                                                 tileXSize, tileYSize,
                                                 type_requested, 
                                                 access, attribs) == XIL_FAILURE) {
                delete memory_storage;
                delete target_storage;
                return XIL_FAILURE;
            }

            //
            //  Request the current storage device copy its current
            //  storage into the provided memory storage.
            //
            if(current_device->copyToMemory(current_storage,
                                            memory_storage,
                                            tileXSize,
                                            tileYSize) == XIL_FAILURE) {
                delete memory_storage;
                delete target_storage;
                return XIL_FAILURE;
            }
        } else {
            memory_storage  = current_storage;
            current_storage = NULL;
        }

        //
        //  If the target device is not the XilMemory device, then tell
        //  the target device to copy from the memory storage.
        //
        if(strcmp(target_name, "XilMemory") != 0) {
            //
            //  Allocate a tile on the target device...
            //
            //  TODO: jlf  This should be done prior to the copies, etc.
            //
            if(target_device->allocate(target_storage,
                                       tileXSize, tileYSize,
                                       type_requested, 
                                       access, attribs) == XIL_FAILURE) {
                delete memory_storage;
                delete target_storage;
                return XIL_FAILURE;
            }

            //
            //  Request the target storage device copy the provided memory
            //  storage into its storage.
            //
            if(target_device->copyFromMemory(memory_storage,
                                             target_storage,
                                             tileXSize,
                                             tileYSize) == XIL_FAILURE) {
                delete memory_storage;
                delete target_storage;
                return XIL_FAILURE;
            }
        } else {
            //
            //  Ok, we don't need target_storage because we were done when
            //  we generated memory_storage.
            //
            delete target_storage;

            target_storage = memory_storage;
            memory_storage = NULL;
        }

        //
        //  We've successfully switched the storage to the new type.
        //
        tile->setStorageRecord(srec);
        tile->setStorage(target_storage);

        if(current_storage != NULL) {
            current_device->deallocate(current_storage);
            delete current_storage;
        }

        if(memory_storage != NULL) {
            memory_rec->getDevice()->deallocate(memory_storage);
            delete memory_storage;
        }
    }

    return XIL_SUCCESS;
}

//------------------------------------------------------------------------
//
//  Function:	setStorageInfo()
//
//  Description:
//	Copies the pertinant storage information from the given image
//	for use as this image's storage.
//
//      This is specifically for setting the alt_img storage information
//      to be the same as the front image storage.
//	
//  MT-level:  UNsafe
//	
//  Deficiencies/TODO:  4/25/96 jlf  Needs to update a ref counter? 
//	
//------------------------------------------------------------------------
XilStatus
XilImage::setStorageInfo(XilImage* img)
{
    if(exportMode == XIL_EXPORTED) {
        //
        //  Cannot be done on exported images.
        //
        return XIL_FAILURE;
    }

    //
    //  Update the version information prior to changing the image.
    //
    newVersion();
    
    tileArray         = img->tileArray;
    tileXSize         = img->tileXSize;
    tileYSize         = img->tileYSize;
    tileSizeIsSetFlag = img->tileSizeIsSetFlag;
    numXTiles         = img->numXTiles;
    numYTiles         = img->numYTiles;
    numTiles          = img->numTiles;

    storageMovement   = img->storageMovement;
    tilingMode        = img->tilingMode;

    //
    //  Copy the storage list.
    //
    //  TODO: 4/26/96 jlf  Bitwise copy ok?
    //
    storageList = img->storageList;

    return XIL_SUCCESS;
}


//
// Set the value of pixel at x, y location with values
//
void
XilImage::setPixel(unsigned int x,
		   unsigned int y,
		   float* values)
{
    //
    //  This routine is about 2x slower than the setPixel() routine in XIL 1.2.
    //
    //  There are two major reasons for this.  We must grab a Mutex in this
    //  release which takes nearly as long as the time the entire old routine
    //  took.  The second is due to missing functionality from the XIL 1.2
    //  routine which caused deferred execution bugs as well as mismatched
    //  data between the display (i.e. I/O device).
    //
    if(isTemp()) {
        //
        //  Cannot manipulate the pixels in a temporary image.
        //
        XIL_OBJ_ERROR(getSystemState(), XIL_ERROR_USER, "di-428",
                      TRUE, this);
        return;
    }

    //
    //  Make sure coordinates are inside the image
    //
    if((x >= xSize) || (y >= ySize)) {
	XIL_OBJ_ERROR(getSystemState(), XIL_ERROR_USER, "di-137", TRUE, this);
	return;
    }

    //
    //  Update the version information prior to changing the image.
    //
    newVersion();

    //
    //  Change the x and y offset to be correct in parent coordinates so we
    //  can exclusively deal in parent storage space.
    //
    x += offsetX;
    y += offsetY;

    //
    // Check to see if its a device image, if it is then
    // ask the IO device to set the pixel.
    //
    XilDeviceIO* io_device = getDeviceIO();

    if(io_device) {
        //
	//  Make sure device is writable
        //
	if(io_device->isWritable() == FALSE) {
	    XIL_OBJ_ERROR(getSystemState(),
                          XIL_ERROR_USER, "di-360", TRUE, this);
	    return;
	} else {
	    //
	    // Pass in the absolute co-ordinates of the image. accounting
	    // for child offsets.
	    //
	    if(io_device->setPixel(x, y, values, offsetBand,
				   getNumBands()) == XIL_FAILURE) {
		XIL_OBJ_ERROR(getSystemState(),
                              XIL_ERROR_SYSTEM, "di-376", TRUE, this);
		return;
	    }
	}

        //
        //  Mark the storage as being invalid now that we've updated the
        //  device without updating the backing image.
        //
        setStorageValidFlag(FALSE);

        return;
    }

    //
    //  Update a memory image via the storage device...first we sync for
    //  writing.
    //
    XilImage*          p = (parent == NULL) ? this : parent;
    XiliStorageRecord* srec;
    XilTile*           tile;
    XilDeviceStorage*  storage_device;
    if(numTiles != 1) {
        //
        //  General case
        //
        //  Get the appropriate tile and its device storage.
        //
        //  The start of the storage has been adjusted to point to the single
        //  pixel no need to pass in adjusted coordinates.
        //
        XilTileNumber tilenum =
            p->getTileForCoordinate(x, y, &x, &y);

        tile = &(p->tileArray[tilenum]);
        srec = tile->getStorageRecord();

        //
        //  The list which holds the tile we're replacing for sync'ing, etc.
        //
        XilTileList tile_list(getSystemState());

        //
        //  We need to lock our storage mutex prior to syncing for storage and
        //  setting it on our tile_list. 
        //
        p->storageMutex.lock();

        //
        //  Now set this mutex on the tile list so that if the DE algorithms need
        //  to block for other operations, they can release the storage lock.
        //
        tile_list.setMutex(&p->storageMutex);
        tile_list.setTileArray(p->tileArray);
        tile_list.setNumTiles(1);
        tile_list.setEntry(0, tilenum);
        tile_list.setArea((x/p->tileXSize) * p->tileXSize,
                          (y/p->tileYSize) * p->tileYSize,
                          p->tileXSize, p->tileYSize);

        //
        //  Sync any oustanding operations utilizing the existing storage.
        //
        if(p->syncForStorage(&tile_list) == XIL_FAILURE) {
            XIL_OBJ_ERROR(getSystemState(), XIL_ERROR_SYSTEM, "di-423",
                          FALSE, this);
            p->storageMutex.unlock();
            return;
        }
    } else {
        //
        //  Special case for single-tiled images
        //
        tile = &(p->tileArray[0]);
        srec = tile->getStorageRecord();

        //
        //  The list which holds the tile we're replacing for sync'ing, etc.
        //
        XilTileList tile_list(getSystemState());

        //
        //  Initialize the tile_list to contain the one tile which is the
        //  entire image. 
        //
        tile_list.setNumTiles(1);
        tile_list.setEntry(0, 0);
        tile_list.setArea(offsetX, offsetY, xSize, ySize);

        //
        //  We need to lock our storage mutex prior to syncing for storage and
        //  setting it on our tile_list. 
        //
        p->storageMutex.lock();

        //
        //  Now set this mutex on the tile list so that if the DE algorithms need
        //  to block for other operations, they can release the storage lock.
        //
        tile_list.setMutex(&p->storageMutex);

        //
        //  Sync any oustanding operations utilizing the existing storage.
        //
        if(p->syncForStorage(&tile_list) == XIL_FAILURE) {
            XIL_OBJ_ERROR(getSystemState(), XIL_ERROR_SYSTEM, "di-423",
                          FALSE, this);
            p->storageMutex.unlock();
            return;
        }
    }

    if(srec == NULL) {
        //
        //  Default to allocating the tile on the memory device.
        //
        srec = p->getStorageRecord("XilMemory");
        if(srec == NULL) {
            XIL_OBJ_ERROR(getSystemState(), XIL_ERROR_SYSTEM,
                          "di-377", TRUE, this);
            p->storageMutex.unlock();
            return;
        }
    }

    if(p->getToDevice(tile, srec, XIL_STORAGE_TYPE_UNDEFINED,
                      XIL_WRITE_ONLY, NULL) == XIL_FAILURE) {
        XIL_OBJ_ERROR(getSystemState(), XIL_ERROR_SYSTEM,
                      "di-377", TRUE, this);
        p->storageMutex.unlock();
        return;
    }

    storage_device = tile->getStorageDevice();

    //
    //  storage_device can be NULL if there is no storage device associated
    //  with the tile -- i.e. the user set the image.  In that case, the
    //  memory storage device can be used.
    //
    if(storage_device == NULL) {
        srec = p->getStorageRecord("XilMemory");
        if(srec == NULL) {
            XIL_OBJ_ERROR(getSystemState(), XIL_ERROR_SYSTEM,
                          "di-377", TRUE, this);
            p->storageMutex.unlock();
            return;
        }
        storage_device = srec->getDevice();
            
        if(storage_device == NULL) {
            XIL_OBJ_ERROR(getSystemState(), XIL_ERROR_SYSTEM,
                          "di-377", TRUE, this);
            p->storageMutex.unlock();
            return;
        }
    }

    //
    //  We can unlock it now because we're sync'd up and done reading the
    //  internal image structures.
    //
    p->storageMutex.unlock();

    if(storage_device->setPixel(tile->getStorage(), values,
                                x, y, offsetBand, nBands) == XIL_FAILURE) { 
        XIL_OBJ_ERROR(getSystemState(), XIL_ERROR_SYSTEM,
                      "di-377", TRUE, this);
        return;
    }
}

//
//  Get the value of pixel at x, y location and place the result in values.
//
void
XilImage::getPixel(unsigned int x,
		   unsigned int y,
		   float* values)
{
    //
    //  This routine is about 2x slower than the getPixel() routine in XIL 1.2.
    //
    //  The major reason for this is that we must grab a Mutex in this
    //  release which takes nearly as long as the time the entire old routine
    //  took.  The second reason is due to missing functionality from the
    //  XIL 1.2 routine which had deferred execution bugs.
    //
    if(isTemp()) {
        //
        //  Cannot aquire the pixels of a temporary image.
        //
        XIL_OBJ_ERROR(getSystemState(), XIL_ERROR_USER, "di-428",
                      TRUE, this);
        return;
    }

    //
    //  Make sure coordinates are inside the image
    //
    if((x >= xSize) || (y >= ySize)) {
	XIL_OBJ_ERROR(getSystemState(), XIL_ERROR_USER, "di-137", TRUE, this);
	return;
    }

    //
    //  Change the x and y offset to be correct in parent coordinates so we
    //  can exclusively deal in parent storage space.
    //
    x += offsetX;
    y += offsetY;

    //
    // Check to see if its a device image, if it is then
    // ask the IO device to set the pixel.
    //
    XilDeviceIO* io_device = getDeviceIO();

    if(io_device) {
        //
	// Make sure device is readable
        //
	if(io_device->isReadable() == FALSE) {
	    XIL_OBJ_ERROR(getSystemState(),
                          XIL_ERROR_USER, "di-359", TRUE, this);
	    return;
	} else {
            //
	    //  Pass in the absolute co-ordinates of the image
            //
	    if(io_device->getPixel(x, y, values, offsetBand,
				   getNumBands()) == XIL_FAILURE) {
		XIL_OBJ_ERROR(getSystemState(),
                              XIL_ERROR_SYSTEM, "di-376", FALSE, this);
		return;
	    }
	}

        return;
    }

    //
    //  Get the value from a memory image via the storage device...first we
    //  sync for reading.
    //
    XilImage*          p    = (parent == NULL) ? this : parent;
    XiliStorageRecord* srec = NULL;
    XilTile*           tile;
    XilDeviceStorage*  storage_device;
    if(numTiles != 1) {
        //
        //  General case
        //
        //  Get the appropriate tile and its device storage.
        //
        //  The start of the storage has been adjusted to point to the single
        //  pixel no need to pass in adjusted coordinates.
        //
        XilTileNumber tilenum =
            p->getTileForCoordinate(x, y, &x, &y);

        tile = &(p->tileArray[tilenum]);
        srec = tile->getStorageRecord();

        //
        //  The list which holds the tile we're replacing for sync'ing, etc.
        //
        XilTileList tile_list(getSystemState());

        //
        //  We need to lock our storage mutex prior to syncing for storage and
        //  setting it on our tile_list. 
        //
        p->storageMutex.lock();

        //
        //  Now set this mutex on the tile list so that if the DE algorithms need
        //  to block for other operations, they can release the storage lock.
        //
        tile_list.setMutex(&p->storageMutex);
        tile_list.setTileArray(p->tileArray);
        tile_list.setNumTiles(1);
        tile_list.setEntry(0, tilenum);
        tile_list.setArea((x/p->tileXSize) * p->tileXSize,
                          (y/p->tileYSize) * p->tileYSize,
                          p->tileXSize, p->tileYSize);

        //
        //  Sync any oustanding operations utilizing the existing storage.
        //
        if(p->syncForReading(&tile_list) == XIL_FAILURE) {
            XIL_OBJ_ERROR(getSystemState(), XIL_ERROR_SYSTEM, "di-423",
                          FALSE, this);
            p->storageMutex.unlock();
            return;
        }
    } else {
        //
        //  Special case for single-tiled images
        //
        tile = &(p->tileArray[0]);
        srec = tile->getStorageRecord();

        //
        //  The list which holds the tile we're replacing for sync'ing, etc.
        //
        XilTileList tile_list(getSystemState());

        //
        //  Initialize the tile_list to contain the one tile which is the
        //  entire image. 
        //
        tile_list.setNumTiles(1);
        tile_list.setEntry(0, 0);
        tile_list.setArea(offsetX, offsetY, xSize, ySize);

        //
        //  We need to lock our storage mutex prior to syncing for storage and
        //  setting it on our tile_list. 
        //
        p->storageMutex.lock();

        //
        //  Now set this mutex on the tile list so that if the DE algorithms need
        //  to block for other operations, they can release the storage lock.
        //
        tile_list.setMutex(&p->storageMutex);

        //
        //  Sync any oustanding operations utilizing the existing storage.
        //
        if(p->syncForReading(&tile_list) == XIL_FAILURE) {
            XIL_OBJ_ERROR(getSystemState(), XIL_ERROR_SYSTEM, "di-423",
                          FALSE, this);
            p->storageMutex.unlock();
            return;
        }
    }

    if(srec == NULL) {
        //
        //  Default to allocating the tile on the memory device.
        //
        srec = p->getStorageRecord("XilMemory");
        if(srec == NULL) {
            XIL_OBJ_ERROR(getSystemState(), XIL_ERROR_SYSTEM,
                          "di-377", TRUE, this);
            p->storageMutex.unlock();
            return;
        }
    }

    if(p->getToDevice(tile, srec, XIL_STORAGE_TYPE_UNDEFINED,
                      XIL_WRITE_ONLY, NULL) == XIL_FAILURE) {
        XIL_OBJ_ERROR(getSystemState(), XIL_ERROR_SYSTEM,
                      "di-377", TRUE, this);
        p->storageMutex.unlock();
        return;
    }

    storage_device = tile->getStorageDevice();

    //
    //  storage_device can be NULL if there is no storage device associated
    //  with the tile -- i.e. the user set the image.  In that case, the
    //  memory storage device can be used.
    //
    if(storage_device == NULL) {
        srec = p->getStorageRecord("XilMemory");
        if(srec == NULL) {
            XIL_OBJ_ERROR(getSystemState(), XIL_ERROR_SYSTEM,
                          "di-377", TRUE, this);
            p->storageMutex.unlock();
            return;
        }
        storage_device = srec->getDevice();
            
        if(storage_device == NULL) {
            XIL_OBJ_ERROR(getSystemState(), XIL_ERROR_SYSTEM,
                          "di-377", TRUE, this);
            p->storageMutex.unlock();
            return;
        }
    }

    //
    //  We can unlock it now because we're sync'd up and done reading the
    //  internal image structures.
    //
    p->storageMutex.unlock();

    if(storage_device->getPixel(tile->getStorage(), values, x, y,
                                offsetBand, nBands) == XIL_FAILURE) { 
        XIL_OBJ_ERROR(getSystemState(), XIL_ERROR_SYSTEM,
                      "di-377", TRUE, this);
        return;
    }
}

XilRoi*
XilImage::getGlobalSpaceRoi()
{
    if(globalSpaceRoi == NULL) {
        createGlobalSpaceRoi(&globalSpaceRoi);
    }

    return globalSpaceRoi;
}

XilRoi*
XilImage::getGlobalSpaceRoiWithDoublePrecision()
{
    if(globalSpaceRoiWithDoubles == NULL) {
        createGlobalSpaceRoiWithDoubles(&globalSpaceRoiWithDoubles);
    }
    return globalSpaceRoiWithDoubles;
}

XilRoi*
XilImage::getExtentGlobalSpaceRoi()
{
    if(extendedGlobalSpaceRoi == NULL) {
        createExtentGlobalSpaceRoi(&extendedGlobalSpaceRoi);
    }
    return extendedGlobalSpaceRoi;
}

//------------------------------------------------------------------------
//
//  Function:	createGlobalSpaceRoi()
//
//  Description:
//	Create the origin-translated ROI of the image.  This is called
//      when the image has no globalSpaceRoi that is currently active.
//	
//------------------------------------------------------------------------
void
XilImage::createGlobalSpaceRoi(XilRoi** roi_ptr)
{
    //
    //  If there is a ROI on the image, construct a ROI which corresponds to
    //  the image ROI in global space.
    //
    XilRoi* gsroi = NULL;
    if(roi != NULL) {
        gsroi = roi->translate(_XILI_ROUND(-originX),
                               _XILI_ROUND(-originY));

        if(gsroi == NULL) {
            XIL_OBJ_ERROR(getSystemState(), XIL_ERROR_SYSTEM,
                          "di-145", FALSE, this);
            *roi_ptr = NULL;
            return;
        }
    }

    //
    //  Construct a ROI that represents the image in global space.
    //
    XilRoi* imgroi = getSystemState()->createXilRoi();

    if(imgroi == NULL) {
        XIL_OBJ_ERROR(getSystemState(), XIL_ERROR_SYSTEM,
                      "di-145", FALSE, this);
        *roi_ptr = NULL;
        return;
    }

    //
    //  Add a rect to the image ROI which represents the image in global
    //  space.
    //
    if(imgroi->addRect(_XILI_ROUND(-originX),
                       _XILI_ROUND(-originY),
                       xSize,
                       ySize) == XIL_FAILURE) {
        *roi_ptr = NULL;
        imgroi->destroy();
        gsroi->destroy();
        return;
    }

    //
    //  If a ROI was set on the image, intersect the Global Space ROI with the
    //  ROI representing the image -- otherwise, just return the image (rect)
    //  in global space.
    //
    if(gsroi != NULL) {
        //
        //  Clip the gsroi to the image.
        //
        if(imgroi->intersect_inplace(gsroi) == XIL_FAILURE) {
            *roi_ptr = NULL;
            imgroi->destroy();
            gsroi->destroy();
            return;
        }

        *roi_ptr = gsroi;

        imgroi->destroy();
        return;
    } else {
        *roi_ptr = imgroi;
    }
}


//------------------------------------------------------------------------
//
//  Function:	createGlobalSpaceRoiWithDoubles()
//
//  Description:
//	Create the origin-translated ROI of the image, but maintain the
//      floating point precision.  This is called when the image has no
//      globalSpaceRoiWithDoubles that is currently active. 
//	
//------------------------------------------------------------------------
void
XilImage::createGlobalSpaceRoiWithDoubles(XilRoi** roi_ptr)

{
    //
    //  If there is a ROI on the image, construct a ROI which corresponds to
    //  the image ROI in global space.
    //
    XilRoi* gsroi = NULL;
    if(roi != NULL) {
        //
        //  Prior to doing the translation, we must convert the ROI into
        //  convex regions so floating point accuracy will be maintained.
        //
        if(roi->getConvexRegionList() != NULL) {
            gsroi = roi->translate(-originX, -originY);
        }

        if(gsroi == NULL) {
            XIL_OBJ_ERROR(getSystemState(), XIL_ERROR_SYSTEM,
                          "di-145", FALSE, this);
            *roi_ptr = NULL;
            return;
        }
    }

    //
    //  Construct a ROI that represents the image in global space.
    //
    XilRoi* imgroi = getSystemState()->createXilRoi();

    if(imgroi == NULL) {
        XIL_OBJ_ERROR(getSystemState(), XIL_ERROR_SYSTEM,
                      "di-145", FALSE, this);
        *roi_ptr = NULL;
        return;
    }

    //
    //  Add a rect to the image ROI which represents the image in global space.
    //
    if(imgroi->addRect(-originX,
                       -originY,
                       (float)xSize,
                       (float)ySize) == XIL_FAILURE) {
        *roi_ptr = NULL;
        imgroi->destroy();
        gsroi->destroy();
        return;
    }

    //
    //  If a ROI was set on the image, intersect the Global Space ROI with the
    //  ROI representing the image -- otherwise, just return the image (rect)
    //  in global space.
    //
    if(gsroi != NULL) {
        //
        //  Clip the gsroi to the image.
        //
        if(imgroi->intersect_inplace(gsroi) == XIL_FAILURE) {
            *roi_ptr = NULL;
            imgroi->destroy();
            gsroi->destroy();
            return;
        }

        *roi_ptr = gsroi;

        imgroi->destroy();
        return;
    } else {
        *roi_ptr = imgroi;
    }
}

//------------------------------------------------------------------------
//
//  Function:	createExtentGlobalSpaceRoi()
//
//  Description:
//	Create the origin-translated ROI of the image, but maintain the
//      floating point precision and represent the extent of the pixels,
//      not just the coordinates.
//	
//------------------------------------------------------------------------
void
XilImage::createExtentGlobalSpaceRoi(XilRoi** roi_ptr)

{
    //
    //  If there is a ROI on the image, construct a ROI which corresponds to
    //  the image ROI in global space.
    //
    XilRoi* gsroi = NULL;
    if(roi != NULL) {
        //
        //  Prior to doing the translation, we must convert the ROI into
        //  convex regions so floating point accuracy will be maintained.
        //

        //
        //  This Roi is still in pixel coordinate space. It will not be
        //  extended, but the extension will be taken into account
        //  during the intersection below.
        //
        if(roi->getConvexRegionList() != NULL) {
            gsroi = roi->translate(-originX, -originY);
        }

        if(gsroi == NULL) {
            XIL_OBJ_ERROR(getSystemState(), XIL_ERROR_SYSTEM,
                          "di-145", FALSE, this);
            *roi_ptr = NULL;
            return;
        }
    }

    //
    //  Construct a ROI that represents the image in global space.
    //
    XilRoi* imgroi = getSystemState()->createXilRoi();
    if(imgroi == NULL) {
        XIL_OBJ_ERROR(getSystemState(), XIL_ERROR_SYSTEM,
                      "di-145", FALSE, this);
        *roi_ptr = NULL;
        return;
    }


    //
    //  If a ROI was set on the image, intersect the Global Space ROI with the
    //  ROI representing the image -- otherwise, just return the image (rect)
    //  in global space.
    //
    if(gsroi != NULL) {
        //
        //  Add a rect to the image ROI which represents the image in global space.
        //  This rectangle is in pixel coordinate space. It will move into
        //  be taken into extent space on the fly during the intersection
        //  if there is one.
        //
        if(imgroi->addRect(-originX,
                           -originY,
                           (float)xSize,
                           (float)ySize) == XIL_FAILURE) {
            *roi_ptr = NULL;
            imgroi->destroy();
            gsroi->destroy();
        } else {
            //
            //  Clip the gsroi to the image.
            //  This intersection must clip with the gsroi in pixel extent space,
            //  which is done on the fly for performance reasons.
            //
            if(imgroi->intersect_extent(gsroi) == XIL_FAILURE) {
                *roi_ptr = NULL;
                imgroi->destroy();
                gsroi->destroy();
            } else {
                *roi_ptr = gsroi;
                imgroi->destroy();
            }
        }
    } else {
        //
        //  Add a rect to the image ROI which represents the image in global space.
        //  Since there will be no intersection - put it into extent space immediately
        //
        if(imgroi->addRect(-originX - XILI_TOP_LF_EXTENT,
                           -originY - XILI_TOP_LF_EXTENT,
                           (float)xSize + XILI_TOP_LF_EXTENT + XILI_BOT_RT_EXTENT,
                           (float)ySize + XILI_TOP_LF_EXTENT + XILI_BOT_RT_EXTENT) == XIL_FAILURE) {
            *roi_ptr = NULL;
            imgroi->destroy();
        } else {
            *roi_ptr = imgroi;
        }
    }
}

//------------------------------------------------------------------
//
// The following conversion routines allow for graceful conversion
// of Roi and boxes from global space to image space and back
//
//------------------------------------------------------------------
XilStatus
XilImage::convertToObjectSpace(XiliRect* rect)
{
    //
    //  Origin-adjust back from global space to image space
    //
    if((originX != 0.0F) || (originY != 0.0F)) {
        rect->translate(originX, originY);
    }

    return XIL_SUCCESS;
}

XilStatus
XilImage::convertToObjectSpace(XilRoi* converted_roi)
{
    //
    //  Origin-adjust back from global space to image space
    //
    if((originX != 0.0F) || (originY != 0.0F)) {
        converted_roi->translate_inplace(originX, originY);
    }

    return XIL_SUCCESS;
}

XilStatus
XilImage::convertToObjectSpace(XiliConvexRegion* cr)
{
    //
    //  Origin-adjust back from global space to image space
    //
    if((originX != 0.0F) || (originY != 0.0F)) {
        cr->translate(originX, originY);
    }

    return XIL_SUCCESS;
}

XilStatus
XilImage::convertToGlobalSpace(XiliRect* rect)
{
    //
    //  Origin-adjust from image space to global space
    //
    if((originX != 0.0F) || (originY != 0.0F)) {
        rect->translate(-originX, -originY);
    }

    return XIL_SUCCESS;
}

XilStatus
XilImage::convertToGlobalSpace(XilRoi* croi)
{
    //
    //  Origin-adjust from image space to global space
    //
    if((originX != 0.0F) || (originY != 0.0F)) {
        croi->translate_inplace(-originX, -originY);
    }

    return XIL_SUCCESS;
}

XilStatus
XilImage::convertToGlobalSpace(XiliConvexRegion* cr)
{
    //
    //  Origin-adjust back from global space to image space
    //
    if((originX != 0.0F) || (originY != 0.0F)) {
        for(unsigned int i=0; i<cr->pointCount; i++) {
            cr->xPtArray[i] -= originX;
            cr->yPtArray[i] -= originY;
        }
        
        cr->lowX  -= originX;
        cr->highX -= originX;
        cr->lowY  -= originY;
        cr->highY -= originY;
    }

    return XIL_SUCCESS;
}

XilStatus
XilImage::setBoxStorage(XilBox* box)
{
    //
    //  We only need to set/change the storage if it's a child because by
    //  default the storage location is the same as the front box location
    //  unless specified otherwise by setting it on the box.
    //
    if(parent != NULL) {
        int          x;
        int          y;
        unsigned int xs;
        unsigned int ys;
	
	if(box->isStorageSet()) {
            int ret_band;
            //
            // We know that the storage was set for a reason and
            // we can't just overwrite with offset front box values
            // work from the set storage values - adding in the
            // offsets.
            //
            box->getStorageLocation(&x, &y, &xs, &ys,&ret_band);
	} else {
            //
            // simply take the child offsets into account
            //
            box->getAsRect(&x, &y, &xs, &ys);
	}
	box->setStorageLocation(x + offsetX,
				y + offsetY,
				xs, ys, offsetBand);
    }

    return XIL_SUCCESS;
}


//------------------------------------------------------------------------
//
//  Function:	setOrigin()
//
//  Description:
//	Set the image origin.  This requires destroying the 
//	globalSpaceRoi because it is tied to the image origin.
//	
//  MT-level:  SAFE
//	
//------------------------------------------------------------------------
void
XilImage::setOrigin(float x, float y)
{
    if(isTemp()) {
        //
        //  If it's a temporary image and has been written into (i.e. it's
        //  in the valid state), then it cannot be changed.
        //
        if(isValid()) {
            XIL_OBJ_ERROR(getSystemState(), XIL_ERROR_USER, "di-428",
                          TRUE, this);
            return;
        }
    }

    sync();

    flushDependents(_XILI_OP_QUEUE_INVALID_POSITION);

    //
    //  Update the version information prior to changing the image.
    //
    newVersion();
    
    //
    //  Destroy and reset the globalSpaceRoi because we're changing the origin
    //  on which the ROI was generated.
    //
    if(globalSpaceRoi != NULL) {
        globalSpaceRoi->destroy();
	globalSpaceRoi = NULL;
    }
    if(globalSpaceRoiWithDoubles != NULL) {
        globalSpaceRoiWithDoubles->destroy();
        globalSpaceRoiWithDoubles = NULL;
    }
    if(extendedGlobalSpaceRoi != NULL) {
        extendedGlobalSpaceRoi->destroy();
        extendedGlobalSpaceRoi = NULL;
    }

    //
    //  Clear out our global space representation as a rect.
    //
    globalSpaceRect.empty();
    
    originX = x;
    originY = y;
}    

//------------------------------------------------------------------------
//
//  Function:	isReadable()/isWritable()
//
//  Description:
//	Indicate whether this image can be read from or written to.  By
//	default, all memory images can be both written to and read from.
//	It's for device images that may not support one or the other.
//
//      The I/O device is expected to be MT-SAFE so we don't lock here
//      because if locking is required, it's the I/O device that needs
//      to lock.  We know the ioDevice value never changes after the
//      construction of the image.
//	
//  MT-level:  SAFE
//	
//------------------------------------------------------------------------
Xil_boolean
XilImage::isReadable()
{
    //
    //  If it's a temporary image, return whether the current capability of
    //  the image is to be able to read from it -- i.e. it's valid.
    //
    if(isTemp()) {
        if(isValid()) {
            return TRUE;
        } else {
            return FALSE;
        }
    }

    //
    //  We don't need to lock because ioDevice is set at construction by the
    //  XilSystemState.  So, if there is going to be an ioDevice, it must have
    //  been set prior to this method being called.
    //
    if(ioDevice != NULL) {
        return ioDevice->isReadable();
    }

    //
    //  Currently, all non-device images are considered both readable and
    //  writable.
    //
    return TRUE;
}

Xil_boolean
XilImage::isWritable()
{
    //
    //  If it's a temporary image, return whether the current capability of
    //  the image is to be able to write into it -- i.e. it's invalid and has
    //  not been marked for destruction.
    //
    if(isTemp()) {
        if(! isValid() && ! deleteWhenNoDependents) {
            return TRUE;
        } else {
            return FALSE;
        }
    }

    //
    //  We don't need to lock because ioDevice is set at construction by the
    //  XilSystemState.  So, if there is going to be an ioDevice, it must have
    //  been set prior to this method being called.
    //
    if(ioDevice != NULL) {
        return ioDevice->isWritable();
    }

    //
    //  Currently, all non-device images are considered both readable and
    //  writable.
    //
    return TRUE;
}

//------------------------------------------------------------------------
//
//  Function:	setAttribute()/getAttribute()
//
//  Description:
//	Set or get an image attribute.  It's a key/value pairing for
//	which permits arbitrary data to be stored on an image by the
//	application.
//	
//------------------------------------------------------------------------
XilStatus
XilImage::setAttribute(const char* key,
                       void*       value)
{
    //
    //  Update the version information prior to changing the image.
    //
    newVersion();
    
    if(attributeHashTable.insert(key, value) == XIL_FAILURE) {
        XIL_OBJ_ERROR(getSystemState(), XIL_ERROR_SYSTEM, "di-239", TRUE, this);
        return XIL_FAILURE;
    }

    return XIL_SUCCESS;
}

XilStatus
XilImage::getAttribute(const char* key,
                       void**      value)
{
    if(attributeHashTable.lookup(key, *value) == XIL_SUCCESS) {
        return XIL_SUCCESS;
    } else if(immediateParent != NULL) {
        return immediateParent->getAttribute(key, value);
    } else {
        return XIL_FAILURE;
    }
}

//------------------------------------------------------------------------
//
//  Function:	setDeviceAttribute()/getDeviceAttribute()
//
//  Description:
//	Set or get an attribute on the attached I/O device.
//
//      The I/O device is expected to be MT-SAFE so we don't lock here
//      because if locking is required, it's the I/O device that needs
//      to lock.  We know the ioDevice value never changes after the
//      construction of the image.
//	
//  MT-level:  SAFE
//	
//------------------------------------------------------------------------
XilStatus
XilImage::setDeviceAttribute(const char* key,
                             void*       value)
{
    //
    //  Don't need to lock the image because the I/O device information is not
    //  supposed to change after creation.
    //

    XilDeviceIO* device_io = getDeviceIO();

    if(device_io == NULL) {
        XIL_OBJ_ERROR(getSystemState(), XIL_ERROR_USER, "di-135", TRUE, this);
        return XIL_FAILURE;
    }

    //
    //  newVersion() is MT-safe -- remember it must be called PRIOR to
    //  changing the object.
    //
    newVersion();

    return device_io->setAttribute(key, value);
}

XilStatus
XilImage::getDeviceAttribute(const char* key,
                             void**      value)
{
    //
    //  Don't need to lock the image because the I/O device information is not
    //  supposed to change after creation.
    //

    XilDeviceIO* device_io = getDeviceIO();

    if(device_io == NULL) {
        XIL_OBJ_ERROR(getSystemState(), XIL_ERROR_USER, "di-136", TRUE, this);
        return XIL_FAILURE;
    }

    return device_io->getAttribute(key, value);
}

//------------------------------------------------------------------------
//
//  Function:	setActiveBuffer()/getActiveBuffer()
//
//  Description:
//	Calls the I/O device to set or get the active buffer information
//	for the image.  This call is only valid for device which support
//	double buffering.  Otherwise, its use is an error.
//	
//      The I/O device is expected to be MT-SAFE so we don't lock here
//      because if locking is required, it's the I/O device that needs
//      to lock.  We know the ioDevice value never changes after the
//      construction of the image.
//	
//  MT-level:  SAFE
//	
//------------------------------------------------------------------------
XilStatus
XilImage::setActiveBuffer(XilBufferId id)
{
    //
    //  Don't need to lock the image because the I/O device information is not
    //  supposed to change after creation.
    //

    //
    //  Update the version information prior to changing the image.
    //
    newVersion();
    
    XilDeviceIO* device_io = getDeviceIO();

    if(device_io == NULL) {
        XIL_OBJ_ERROR(getSystemState(), XIL_ERROR_USER, "di-135", TRUE, this);
        return XIL_FAILURE;
    }

    //
    // Verify that the setting is valid for this device
    //
    switch(id) {
      case XIL_BACK_BUFFER:
      case XIL_FRONT_BUFFER:
        if(! device_io->isDoubleBufferingDevice()) {
            XIL_OBJ_ERROR(getSystemState(), XIL_ERROR_USER, "di-378", TRUE, this);
            return XIL_FAILURE;
        }
        break;
      case XIL_LEFT_BUFFER:
      case XIL_RIGHT_BUFFER:
        if(! device_io->isStereoDevice()) {
            XIL_OBJ_ERROR(getSystemState(), XIL_ERROR_USER, "di-378", TRUE, this);
            return XIL_FAILURE;
        }
        break;
      case XIL_FRONT_LEFT_BUFFER:
      case XIL_FRONT_RIGHT_BUFFER:
      case XIL_BACK_LEFT_BUFFER:
      case XIL_BACK_RIGHT_BUFFER:
        if(! device_io->isStereoDevice() || 
           ! device_io->isDoubleBufferingDevice()) {
            XIL_OBJ_ERROR(getSystemState(), XIL_ERROR_USER, "di-378", TRUE, this);
            return XIL_FAILURE;
        }
        break;
      default:
        XIL_OBJ_ERROR(getSystemState(), XIL_ERROR_USER, "di-378", TRUE, this);
        return XIL_FAILURE;
    }

    return device_io->setActiveBuffer(id);
}

XilBufferId
XilImage::getActiveBuffer()
{
    //
    //  Don't need to lock the image because the I/O device information is not
    //  supposed to change after creation.
    //

    XilDeviceIO* device_io = getDeviceIO();

    if(device_io == NULL) {
        XIL_OBJ_ERROR(getSystemState(), XIL_ERROR_USER, "di-136", TRUE, this);
        return XIL_BACK_BUFFER;
    }

    if(! device_io->isDoubleBufferingDevice() &&
       ! device_io->isStereoDevice() ) {
        XIL_OBJ_ERROR(getSystemState(), XIL_ERROR_USER, "di-378", TRUE, this);
        // 
        // Kinda arbitrary what to return here, since its
        // an error condition. But the error is reported.
        //
        return XIL_BACK_BUFFER;
    }

    return device_io->getActiveBuffer();
}

//------------------------------------------------------------------------
//
//  Function:	swapBuffers()
//
//  Description:
//	Swaps the back buffer and the front buffer on a double buffering
//	I/O device.  The contents of the back buffer are undefined.
//	
//      The I/O device is expected to be MT-SAFE so we don't lock here
//      because if locking is required, it's the I/O device that needs
//      to lock.  We know the ioDevice value never changes after the
//      construction of the image.
//	
//  MT-level:  SAFE
//	
//  Returns:   XIL_SUCCESS/XIL_FAILURE
//	
//------------------------------------------------------------------------
XilStatus
XilImage::swapBuffers()
{
    //
    //  Don't need to lock the image because the I/O device information is not
    //  supposed to change after creation.
    //

    XilDeviceIO* device_io = getDeviceIO();

    if(device_io == NULL) {
        XIL_OBJ_ERROR(getSystemState(), XIL_ERROR_USER, "di-135", TRUE, this);
        return XIL_FAILURE;
    }


    if(! device_io->isDoubleBufferingDevice()) {
        XIL_OBJ_ERROR(getSystemState(), XIL_ERROR_USER, "di-378", TRUE, this);
        return XIL_FAILURE;
    }

    return device_io->swapBuffers();
}

//------------------------------------------------------------------------
//
//  Function:	Deferrable object DE Overload Methods
//
//  Description:
//	All of the deferred execution information is kept on the parent 
//	image.  So, the XilImage object needs to overload these calls
//	so child images use the information on their parent image.
//	
//------------------------------------------------------------------------
XilStatus
XilImage::sync(XiliOpQueuePosition qposition)
{
    if(parent) {
        return parent->sync(qposition);
    }

    return XilDeferrableObject::sync(qposition);
}

XilStatus
XilImage::syncForReading(XilTileList*        tile_list,
                         XilOp*              op_flushing,
                         XiliOpQueuePosition qposition)
{
    if(parent) {
        return parent->syncForReading(tile_list, op_flushing, qposition);
    }

    return XilDeferrableObject::syncForReading(tile_list, op_flushing, qposition);
}

XilStatus
XilImage::syncForWriting(XilTileList*        tile_list,
                         XilOp*              op_flushing,
                         XiliOpQueuePosition qposition)
{
    if(parent) {
        return parent->syncForWriting(tile_list, op_flushing, qposition);
    }

    return XilDeferrableObject::syncForWriting(tile_list, op_flushing, qposition);
}

void
XilImage::setDagRef(XiliDagRef* new_ref)
{
    if(parent) {
        parent->setDagRef(new_ref);
    } else {
        XilObject::setDagRef(new_ref);
    }
}

XiliDagRef*
XilImage::getDagRef()
{
    if(parent) {
        return parent->getDagRef();
    }

    return XilObject::getDagRef();
}

void
XilImage::lock()
{
    if(parent) {
        parent->lock();
    } else {
        XilObject::lock();
    }
}

XilStatus
XilImage::trylock()
{
    if(parent) {
        return parent->trylock();
    }

    return XilObject::trylock();
}

void
XilImage::unlock()
{
    if(parent) {
        parent->unlock();
    } else {
        XilObject::unlock();
    }
}

XiliOpQueuePosition
XilImage::setOp(XilOp* new_op)
{
    if(parent) {
        XiliOpQueuePosition pos = parent->setOp(new_op);

        //
        //  If the operation can be split in the destination, then we mark all
        //  of the tiles we don't intersect in the parent's array as EVALUATED
        //  so it's clear they're done.  Currently, deferred execution just
        //  handles the parent's tile information. 
        //
        if(new_op->canBeSplit()) {
            XiliOpQueueEntry*   opentry = parent->qRef(pos);

            //
            //  Get the parent's tile size.
            //
            parent->getTileSize(&tileXSize, &tileYSize);

            unsigned int tile_num = 0;
            for(unsigned int y=0; y<parent->ySize; y += tileYSize) {
                for(unsigned int x=0; x<parent->xSize; x += tileXSize) {
                    unsigned int x1 = x;
                    unsigned int y1 = y;
                    unsigned int x2 = x + tileXSize;
                    unsigned int y2 = y + tileYSize;
                    x1 = _XILI_MAX(x1, offsetX);
                    x2 = _XILI_MIN(x2, offsetX + xSize);
                    y1 = _XILI_MAX(y1, offsetY);
                    y2 = _XILI_MIN(y2, offsetY + ySize);

                    //
                    //  If it's clipped to an empty box, then mark it as EVALUATED.
                    //
                    if((x2 < x1) || (y2 < y1)) {
                        opentry->setOpStatus(tile_num, XILI_EVALUATED);
                    }       

                    tile_num++;
                }
            }
        }

        return pos;
    }

    return XilDeferrableObject::setOp(new_op);
}

XilOp*
XilImage::getOp()
{
    if(parent) {
        return parent->getOp();
    }

    return XilDeferrableObject::getOp();
}

XiliOpStatus
XilImage::getOpStatus(XilTileNumber       tile_number,
                      XiliOpQueuePosition qposition)
{
    if(parent) {
        return parent->getOpStatus(tile_number, qposition);
    }

    return XilDeferrableObject::getOpStatus(tile_number, qposition);
}

void
XilImage::setOpStatus(XilTileNumber       tile_number,
                      XiliOpQueuePosition qposition,
                      XiliOpStatus        op_status)
{
    if(parent) {
        parent->setOpStatus(tile_number, qposition, op_status);
    } else {
        XilDeferrableObject::setOpStatus(tile_number, qposition, op_status);
    }
}    

Xil_boolean
XilImage::allTilesDone(XiliOpQueuePosition qposition)
{
    if(parent) {
        return parent->allTilesDone(qposition);
    }

    return XilDeferrableObject::allTilesDone(qposition);
}
 
void
XilImage::cleanup(XiliOpQueuePosition qposition,
                  XilOp*              op)
{
    if(parent) {
        parent->cleanup(qposition, op);
    } else {
        XilDeferrableObject::cleanup(qposition, op);
    }
}

void
XilImage::creatorCleanup(XiliOpQueuePosition qposition,
                         XilOp*              op)
{
    if(parent) {
        parent->creatorCleanup(qposition, op);
    } else {
        XilDeferrableObject::creatorCleanup(qposition, op);
    }
}

XiliOpQueuePosition
XilImage::addDependent(XilOp*       dependent_op,
                       unsigned int src_branch)
{
    if(parent) {
        return parent->addDependent(dependent_op,
                                    src_branch);
    }

    return XilDeferrableObject::addDependent(dependent_op,
                                             src_branch);
}

XilStatus
XilImage::flushDependents(XiliOpQueuePosition qposition)
{
    if(parent) {
        return parent->flushDependents(qposition);
    }

    return XilDeferrableObject::flushDependents(qposition);
}

XilStatus
XilImage::flushDependents(XilTileList*        tile_list,
                          XilOp*              op_flushing,
                          XiliOpQueuePosition qposition)
{
    if(parent) {
        return parent->flushDependents(tile_list,
                                       op_flushing,
                                       qposition);
    } else {
        return XilDeferrableObject::flushDependents(tile_list,
                                                    op_flushing,
                                                    qposition);
    }
}

XiliOpQueueEntry*
XilImage::getQueueEntry(XiliOpQueuePosition qpos)
{
    if(parent) {
        return parent->getQueueEntry(qpos);
    }

    return XilDeferrableObject::getQueueEntry(qpos);
}

Xil_boolean
XilImage::tossIfNoDependents()
{
    if(parent) {
        return parent->tossIfNoDependents();
    }

    return XilDeferrableObject::tossIfNoDependents();
}

Xil_boolean
XilImage::preDestroy()
{
    if(parent) {
        //
        //  Just sync this image because trying to defer a child through a
        //  destroy doesn't really work very well.
        //
        //  TODO: 2/21/96 jlf  Can this be made to work?
        //
        allSync();

        return TRUE;
    } else {
        //
        //  Per BugId# 4067192, we must be certain we own all of the data
        //  associated with this image.  If the user provided us with the
        //  image data, then upon calling xil_destroy(), the user will
        //  expect that the memory can be deallocated.  Thus, in this
        //  case, we cannot defer through an xil_destroy().
        //
        Xil_boolean must_sync = FALSE;

        //
        //  Check any outstanding exported image storage.
        //
        if(exportedImageStorage != NULL &&
           exportedImageStorage->isStorageTagSet()) {
            XiliStorageTag* tag = exportedImageStorage->refStorageTag();

            if(tag->ownsData == FALSE) {
                must_sync = TRUE;
            }
        }

        //
        //  Go through the tile array and check the storage associated with
        //  each tile to determine if we own all of the data.
        //
        if(! must_sync) {
            if(tileArray != NULL) {
                for(unsigned int i=0; i<numTiles; i++) {
                    XilStorage* storage = tileArray[i].getStorage();

                    if(storage != NULL &&
                       storage->isStorageTagSet()) {
                        XiliStorageTag* tag = storage->refStorageTag();

                        if(tag->ownsData == FALSE) {
                            must_sync = TRUE;
                            break;
                        }
                    }
                }
            }
        }

        if(must_sync) {
            allSync();
            return TRUE;
        } else {
            return XilDeferrableObject::preDestroy();
        }
    }
}

Xil_boolean
XilImage::isStorageValid()
{
    if(parent) {
        return parent->isStorageValid();
    } else {
        return XilDeferrableObject::isStorageValid();
    }
}

void
XilImage::setStorageValidFlag(Xil_boolean valid_flag)
{
    if(parent) {
        parent->setStorageValidFlag(valid_flag);
    } else {
        XilDeferrableObject::setStorageValidFlag(valid_flag);
    }
}


//------------------------------------------------------------------------
//
//  Function:	reinit()
//
//  Description:
//	Reinitializes the image to a constructed state with the given
//	XilImageFormat.  Any outstanding operations on the image are
//	flushed and depending upon the size of the new image, storage
//	is deallocated and then reallocated.
//	
//  MT-level:  safe
//	
//------------------------------------------------------------------------
XilStatus
XilImage::reinit(XilImageFormat* image_format)
{
    if(ioDevice == NULL) {
        //
        //  Can only reinitialze a device image.  Currently, resizing regular
        //  images is not supported. 
        //
        XIL_OBJ_ERROR(getSystemState(), XIL_ERROR_SYSTEM,
                      "di-322", TRUE, this);
        return XIL_FAILURE;
    }

    if(exportMode == XIL_EXPORTED) {
        //
        //  Can only reinitialze an imported device image.
        //
        XIL_OBJ_ERROR(getSystemState(), XIL_ERROR_SYSTEM,
                      "di-366", TRUE, this);
        return XIL_FAILURE;
    }

    //
    //  TODO: jlf 4/23/96  Can we permit reinitializing children too?
    //
    if(parent != NULL) {
        //
        //  Only support reinitialization on a parent image.
        //
        XIL_OBJ_ERROR(getSystemState(), XIL_ERROR_SYSTEM,
                      "di-321", TRUE, this);
        return XIL_FAILURE;
    }

    //
    //  Sync the image and flush dependents to ensure no more operations are
    //  using the current image.
    //
    allSync();

    //
    //  Mark our children as invalid...
    //
    XiliBagIterator bi(&children);
    XilImage*       child;
    while(child = (XilImage*)bi.getNext()) {
        child->setValid(FALSE);
    }

    //
    //  For now, just deallocate any existing storage.
    //
    deallocateAllStorage();

    //
    //  Destroy the globalSpaceRoi...
    //
    if(globalSpaceRoi != NULL) {
        globalSpaceRoi->destroy();
        globalSpaceRoi = NULL;
    }
    if(globalSpaceRoiWithDoubles != NULL) {
        globalSpaceRoiWithDoubles->destroy();
        globalSpaceRoiWithDoubles = NULL;
    }
    if(extendedGlobalSpaceRoi != NULL) {
        extendedGlobalSpaceRoi->destroy();
        extendedGlobalSpaceRoi = NULL;
    }
    
    //
    //  Reinitialize our dimensions...
    //
    xSize    = image_format->getWidth();
    ySize    = image_format->getHeight();
    nBands   = image_format->getNumBands();
    dataType = image_format->getDataType();

    //
    //  Reset tile information...
    //
    if(tileArray != NULL) {
        delete [] tileArray;
    }
    tileArray         = NULL;
    tileSizeIsSetFlag = FALSE;
        
    //
    //  Initialize our tile size to the defaults in the system state.
    //
    unsigned int default_xsize;
    unsigned int default_ysize;
    getDefaultTileSize(&default_xsize, &default_ysize);

    if(initTileSize(default_xsize, default_ysize) == XIL_FAILURE) {
        XIL_OBJ_ERROR(getSystemState(), XIL_ERROR_INTERNAL,
                      "di-392", FALSE, this);
        return XIL_FAILURE;
    }

    return XIL_SUCCESS;
}

//------------------------------------------------------------------------
//
//  Function:	setDeviceIO()/getDeviceIO()
//
//  Description:
//	Attach or aquire the attached I/O device for this image.  
//	setDeviceIO() is only expected to be called while a single 
//	thread has access to the device and only during construction
//	of the image.
//	
//	getDeviceIO() returns the I/O device associated with this 
//	image.
//	
//  MT-level:  setDeviceIO() -- UN-SAFE
//	       getDeviceIO() -- SAFE (because ioDevice doesn't change)
//	
//------------------------------------------------------------------------
XilStatus
XilImage::setDeviceIO(XilDeviceIO* device)
{
    //
    //  When there is a parent, this child is the controlling image.  In this
    //  case, both the child and the parent have pointers to the device so the
    //  destructor can detect that the controlling image (child) has been
    //  destroyed and it is also time for the parent to be destroyed.
    //
    if(parent) {
	parent->ioDevice = device;
        parent->exportMode = XIL_NOT_EXPORTABLE;
    }

    ioDevice = device;
    exportMode = XIL_NOT_EXPORTABLE;

    //
    //  The storage information associated with I/O devices is not valid at
    //  construction time -- i.e. it requires a capture to get the current
    //  state of the device in order to get the storage completely valid.
    //
    setStorageValidFlag(FALSE);

    return XIL_SUCCESS;
}


XilDeviceIO*
XilImage::getDeviceIO()
{
    if(parent) {
	return parent->ioDevice;
    }

    return ioDevice;
}

//------------------------------------------------------------------------
//
//  Function:	createCaptureOp()/createDisplayOp()
//
//  Description:
//	These routines are called by XilOp::execute() to create a capture or
//      display operation that corresponds to the device attached to the
//      image.  It is the responsibility of this routine to check the validity
//      of the device for the particular operation (for instance that it's
//      readable for a capture).
//	
//  MT-level:  SAFE
//	
//------------------------------------------------------------------------
XilOp*
XilImage::createCaptureOp(XilOp*       constructing_op,
                          unsigned int branch_num)
{
    const char*                   op_name = "capture";
    static XilOpCreateFunctionPtr op_create_function = NULL;
    
    if(op_create_function == NULL) {
        op_create_function =
            XilGlobalState::getXilGlobalState()->getXilOpCreateFunc(op_name);
        if(op_create_function == NULL) {
            return NULL;
        }
    }

    void* args[4];
    args[0] = this;
    args[1] = constructing_op;
    args[2] = (void*)branch_num;
    args[3] = NULL;

    XilOp* op = (*op_create_function)(op_name, args, 3);

    return op;
}

XilOp*
XilImage::createDisplayOp(XilOp*       constructing_op,
                          unsigned int branch_num)
{
    const char*                   op_name = "display";
    static XilOpCreateFunctionPtr op_create_function = NULL;
    
    if(op_create_function == NULL) {
        op_create_function =
            XilGlobalState::getXilGlobalState()->getXilOpCreateFunc(op_name);
        if(op_create_function == NULL) {
            return NULL;
        }
    }

    void* args[4];
    args[0] = this;
    args[1] = constructing_op;
    args[2] = (void*)branch_num;
    args[3] = NULL;

    XilOp* op = (*op_create_function)(op_name, args, 3);

    return op;
}


//------------------------------------------------------------------------
//
//  Function:	setStorageMovment()/getStorageMovmement()
//
//  Description:
//	These routines are called through the API to change the ability of
//      XIL to modify the location of storage once it is imported.
//      The image must be exported in order to set these flags.
//	
//  MT-level:  SAFE
//	
//------------------------------------------------------------------------
void
XilImage::setStorageMovement(XilStorageMovement move_flag)
{
    if(parent) {
        parent->setStorageMovement(move_flag);
    } else {
        //
        //  Is the image exported?
        //
        if(exportMode != XIL_EXPORTED) {
            XIL_OBJ_ERROR(getSystemState(), XIL_ERROR_USER,
                          "di-438", TRUE, this);
            return;
        }

        //
        //  Update the version information prior to changing the image.
        //
        newVersion();
    
        if(move_flag != storageMovement) {
            storageMovement = move_flag;
        }
    }
}

XilStorageMovement
XilImage::getStorageMovement()
{
    XilStorageMovement ret_val;

    if(parent) {
	ret_val = parent->getStorageMovement();
    } else {
        //
        //  Is the image exported?
        //
        if(exportMode != XIL_EXPORTED) {
            XIL_OBJ_ERROR(getSystemState(), XIL_ERROR_USER,
                          "di-438", TRUE, this);
            return XIL_ALLOW_MOVE;
        }

        ret_val = storageMovement;
    }

    return ret_val;
}

void
XilImage::setDataSupplyFunc(XilDataSupplyFuncPtr  supply_func,
                            void*                 user_args)
{
    //
    //  We're a new version now...
    //
    newVersion();

    //
    //  If the user has provided a data supply routine, the current data (if
    //  any) is obsolete.  The tileArray's storage must be deallocated, since
    //  this is the check used for calling the specified dataSupplyFunc.  This
    //  does not, however, remove the tileArray which is associated with the
    //  image and tilesize. 
    //
    deallocateAllStorage();

    dataSupplyFunc = supply_func;
    supplyUserArgs = user_args;
}
