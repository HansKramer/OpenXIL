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
//  File:	XilStorage.cc
//  Project:	XIL
//  Revision:	1.51
//  Last Mod:	10:08:25, 03/10/00
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
#pragma ident	"@(#)XilStorage.cc	1.47\t97/04/25  "

#include "_XilDefines.h"
#include "_XilStorage.hh"
#include "_XilImage.hh"

#include "XiliUtils.hh"

//
//  IMPORTANT:  The simple compute interface assumes that it can pass
//              a NULL pointer in as image.  This currently works, be
//              careful not to change it without changing the simple
//              compute interface.
//
//  TODO:  Should passing a NULL pointer be allowed?
//
XilStorage::XilStorage(XilImage* img) :
    _XIL_NUM_STORAGE_ARRAY_UNION_MEMBERS(6)
{
    image                         = img;
    storageInfo.d.dataPtr         = NULL;
    storageInfo.d.dataReleaseFunc = NULL;
    storageInfo.d.userArgs        = NULL;
    storageInfo.d.pixelStride     = 1;
    storageInfo.d.scanlineStride  = 0;
    storageInfo.d.bandStride      = 1;
    storageInfo.d.offset          = 0;
    arraysInitialized             = FALSE;
    tagSet                        = FALSE;
    deviceInfo                    = NULL;
    xcoord                        = 0;
    ycoord                        = 0;
    storageType                   = XIL_STORAGE_TYPE_UNDEFINED;
    
    if(img != NULL) {
        numBands                  = image->getNumBands();
        dataType                  = image->getDataType();
        dataTypeSize              = xili_sizeof(dataType);
    }
}

//
//  libxil Private constructor.
//
XilStorage::XilStorage(XilImage*       img,
                       unsigned int    pixel_stride,
                       unsigned int    scanline_stride,
                       unsigned int    band_stride,
                       unsigned int    offset,
                       void*           data_ptr) :
    _XIL_NUM_STORAGE_ARRAY_UNION_MEMBERS(6)
{
    image                         = img;
    deviceInfo                    = NULL;
    xcoord                        = 0;
    ycoord                        = 0;
    storageType                   = XIL_STORAGE_TYPE_UNDEFINED;
    
    if(img != NULL) {
        numBands                  = image->getNumBands();
        dataType                  = image->getDataType();
        dataTypeSize              = xili_sizeof(dataType);
    }

    storageInfo.d.dataPtr         = data_ptr;
    storageInfo.d.dataReleaseFunc = NULL;
    storageInfo.d.userArgs        = NULL;
    storageInfo.d.pixelStride     = pixel_stride;
    storageInfo.d.scanlineStride  = scanline_stride;
    storageInfo.d.bandStride      = band_stride;
    storageInfo.d.offset          = offset;

    arraysInitialized             = FALSE;
    tagSet                        = FALSE;
}

XilStorage::~XilStorage()
{
    if(image != NULL && tagSet == TRUE) {
        image->releaseStorage(this);
    }

    if(arraysInitialized) {
        //
        //  Remember, the initialization of the array is optimized to only
        //  allocate a single buffer of pointers instead of seperate buffers
        //  for each data entry.
        //
        delete [] storageInfo.a.dataPtrs;
    }
}

//
//  Set/Get Device-Specific Information
//
void*
XilStorage::getDeviceSpecificInfo()
{
    return deviceInfo;
}

void
XilStorage::setDeviceSpecificInfo(void* dev_info)
{
    deviceInfo = dev_info;
}

//
//  Get the type of storage stored in this object.
//
Xil_boolean
XilStorage::isType(XilStorageType target_type)
{
    if(target_type == XIL_STORAGE_TYPE_UNDEFINED) {
        return TRUE;
    }

    if(storageType == XIL_STORAGE_TYPE_UNDEFINED) {
        ((XilStorage*)this)->determineType();
    }

    if(numBands == 1) {
        //
        // XIL_BIT will not be pixel sequential, even in 1banded case
        //
        if((target_type == XIL_PIXEL_SEQUENTIAL) && (dataType == XIL_BIT)) {
            return storageType == target_type;
        }
        //
        //  Special case where it can be multiple types.
        //
        if(arraysInitialized) {
            //
            //  TODO:  10/31/95 jlf  Don't think this is TRUE in all cases.
            //
            return TRUE;
        } else {
            if(storageInfo.d.pixelStride == 1) {
                return TRUE;
            } else {
                return storageType == target_type;
            }
        }
    } else {
        return storageType == target_type;
    }
}

void
XilStorage::determineType()
{
    //
    //  We set target_type to the base types for each datatype.  For BIT, this
    //  is XIL_BAND_SEQUENTIAL and for the others, it XIL_PIXEL_SEQUENTIAL.
    //  Then, we change the type if it's different than the target type.
    //
    switch(dataType) {
      case XIL_BIT:
      case XIL_UNSIGNED_4:
        storageType = XIL_BAND_SEQUENTIAL;
        break;
            
      default:
        storageType = XIL_PIXEL_SEQUENTIAL;
        break;
    }

    if(numBands != 1) {
        if(!arraysInitialized) {
            if(storageInfo.d.bandStride != 1) {
                storageType = XIL_BAND_SEQUENTIAL;
            }
        } else {
            //
            //  Arrays are populated, so it could be any storage type.
            //
            //  User may have unnecessarily set pointers and strides for all
            //  bands.  So, check the distance between ALL data ptrs for
            //  consistency.
            //
            Xil_boolean is_seq = TRUE;
            for(unsigned int i=1; i<numBands && is_seq; i++) {
                is_seq =
                    (((Xil_unsigned8*)storageInfo.a.dataPtrs[i] -
                      (Xil_unsigned8*)storageInfo.a.dataPtrs[i-1]) == 
                      (Xil_unsigned8) dataTypeSize);
            }
            if(! is_seq) {
                is_seq = TRUE;

                unsigned int txsize;
                unsigned int tysize;
                image->getTileSize(&txsize, &tysize);

                for(i=1; i<numBands && is_seq; i++) {
                    is_seq = (((Xil_unsigned8*)storageInfo.a.dataPtrs[i] -
                             (Xil_unsigned8*)storageInfo.a.dataPtrs[i-1]) ==
                             (Xil_unsigned8) (tysize*storageInfo.a.scanlineStrides[i]*dataTypeSize));
                }

                if(is_seq) {
                    storageType = XIL_BAND_SEQUENTIAL;
                } else {
                    storageType = XIL_GENERAL;
                }
            }
        }
    }
}

//
//  Get the image this object represents.
//
XilImage*
XilStorage::getImage()
{
    return image;
}

//
//  Set which image this object represents.
//
void
XilStorage::setImage(XilImage* init_image)
{
    image = init_image;

    if(image != NULL) {
        numBands = image->getNumBands();
        dataType = image->getDataType();
    }
}
    

//
//  Release the pertinent information and the storage represented by this
//  object.  It it like destruction except the object remains.
//
void
XilStorage::release()
{
    //
    //  Effectively destroy it.
    //
    this->XilStorage::~XilStorage();

    //
    //  Set the pertinent flags to FALSE.
    //
    tagSet            = FALSE;
    arraysInitialized = FALSE;

    //
    //  This is necessary so determineType can start fresh from the new
    //  type and stride information.
    //
    storageType = XIL_STORAGE_TYPE_UNDEFINED;
}

//
//  Methods for XIL_GENERAL the storage type.  These use the arrays.
//
unsigned int
XilStorage::getPixelStride(unsigned int band_num)
{
    if(band_num >= numBands) {
        XIL_ERROR(image == NULL ? NULL : image->getSystemState(),
                  XIL_ERROR_USER, "di-426", TRUE);
        return 0;
    }

    if(arraysInitialized) {
        return storageInfo.a.pixelStrides[band_num];
    } else {
        return storageInfo.d.pixelStride;
    }
}
    
unsigned int
XilStorage::getScanlineStride(unsigned int band_num)
{
    if(band_num >= numBands) {
        XIL_ERROR(image == NULL ? NULL : image->getSystemState(),
                  XIL_ERROR_USER, "di-426", TRUE);
        return 0;
    }

    if(arraysInitialized) {
        return storageInfo.a.scanlineStrides[band_num];
    } else {
        return storageInfo.d.scanlineStride;
    }
}
    
unsigned int
XilStorage::getOffset(unsigned int band_num)
{
    if(band_num >= numBands) {
        XIL_ERROR(image == NULL ? NULL : image->getSystemState(),
                  XIL_ERROR_USER, "di-426", TRUE);
        return 0;
    }

    if(arraysInitialized) {
        return storageInfo.a.offsets[band_num];
    } else {
        return storageInfo.d.offset;
    }
}

void*
XilStorage::getDataPtr(unsigned int band_num)
{
    if(band_num >= numBands) {
        XIL_ERROR(image == NULL ? NULL : image->getSystemState(),
                  XIL_ERROR_USER, "di-426", TRUE);
        return NULL;
    }

    if(arraysInitialized) {
        return storageInfo.a.dataPtrs[band_num];
    } else {
        //
        //  TODO: 11/5/95 jlf  The use of isType() seems excessive.
        //
        //    There seems to be a bug in that the rest of the storage
        //    information can be set and the type not being set at this point.
        //    Fixing it in this fashion may be a better solution rather than
        //    fixing things by calling isType().
        //
        if(isType(XIL_PIXEL_SEQUENTIAL)) {
            return (void*)(((char*)storageInfo.d.dataPtr) +
                           band_num*dataTypeSize);
        } else if(isType(XIL_BAND_SEQUENTIAL)) {
            return (void*)(((char*)storageInfo.d.dataPtr) +
                           band_num*storageInfo.d.bandStride*dataTypeSize);
        } else {
            XIL_ERROR(image->getSystemState(), XIL_ERROR_INTERNAL,
                      "di-407", TRUE);
            return NULL;
        }
    }
}

void
XilStorage::getDataReleasePtr(unsigned int           band_num,
                              XilDataReleaseFuncPtr* release_func,
                              void**                 user_args)
{
    if(band_num >= numBands) {
        XIL_ERROR(image == NULL ? NULL : image->getSystemState(),
                  XIL_ERROR_USER, "di-426", TRUE);
        return;
    }

    if(arraysInitialized) {
        *release_func = storageInfo.a.dataReleaseFuncs[band_num];
        *user_args    = storageInfo.a.userArgs[band_num];
    } else {
        *release_func = storageInfo.d.dataReleaseFunc;
        *user_args    = storageInfo.d.userArgs;
    }
}

XilStatus
XilStorage::getStorageInfo(unsigned int  band_num,
                           unsigned int* pixel_stride,
                           unsigned int* scanline_stride,
                           unsigned int* offset,
                           void**        data_ptr)
{
    if(arraysInitialized) {
        if(pixel_stride) {
            *pixel_stride    = storageInfo.a.pixelStrides[band_num];
        }
        
        if(scanline_stride) {
            *scanline_stride = storageInfo.a.scanlineStrides[band_num];
        }
        
        if(offset) {
            *offset          = storageInfo.a.offsets[band_num];
        }
        
        if(data_ptr) {
            *data_ptr        = storageInfo.a.dataPtrs[band_num];
        }
    } else {
        if(pixel_stride) {
            *pixel_stride    = storageInfo.d.pixelStride;
        }
        
        if(scanline_stride) {
            *scanline_stride = storageInfo.d.scanlineStride;
        }
        
        if(offset) {
            *offset          = storageInfo.d.offset;
        }
        
        if(data_ptr) {
            *data_ptr        =
                (void*)(((char*)storageInfo.d.dataPtr) +
                        band_num*storageInfo.d.bandStride*dataTypeSize);
        }
    }

    return XIL_SUCCESS;
}

//
//  Methods for non-XIL_GENERAL types of storage.  They only work when the
//  type of storage is not XIL_GENERAL. 
//
unsigned int
XilStorage::getPixelStride()
{
    if(arraysInitialized) {
        return storageInfo.a.pixelStrides[0];
    } else {
        return storageInfo.d.pixelStride;
    }
}
    
unsigned int
XilStorage::getScanlineStride()
{
    if(arraysInitialized) {
        return storageInfo.a.scanlineStrides[0];
    } else {
        return storageInfo.d.scanlineStride;
    }
}
    
unsigned int
XilStorage::getBandStride()
{
    return storageInfo.d.bandStride;
}

unsigned int
XilStorage::getOffset()
{
    if(arraysInitialized) {
        return storageInfo.a.offsets[0];
    } else {
        return storageInfo.d.offset;
    }
}

void*
XilStorage::getDataPtr()
{
    if(arraysInitialized) {
        return storageInfo.a.dataPtrs[0];
    } else {
        return storageInfo.d.dataPtr;
    }
}

void
XilStorage::getDataReleasePtr(XilDataReleaseFuncPtr* release_func,
                              void**                 user_args)
{
    if(arraysInitialized) {
        *release_func = storageInfo.a.dataReleaseFuncs[0];
        *user_args    = storageInfo.a.userArgs[0];
    } else {
        *release_func = storageInfo.d.dataReleaseFunc;
        *user_args    = storageInfo.d.userArgs;
    }
}

XilStatus
XilStorage::getStorageInfo(unsigned int* pixel_stride,
                           unsigned int* scanline_stride,
                           unsigned int* band_stride,
                           unsigned int* offset,
                           void**        data_ptr)
{
    if(arraysInitialized) {
        if(pixel_stride) {
            *pixel_stride    = storageInfo.a.pixelStrides[0];
        }
        
        if(scanline_stride) {
            *scanline_stride = storageInfo.a.scanlineStrides[0];
        }
        
        if(offset) {
            *offset          = storageInfo.a.offsets[0];
        }
        
        if(data_ptr) {
            *data_ptr        = storageInfo.a.dataPtrs[0];
        }
    } else {
        if(pixel_stride) {
            *pixel_stride    = storageInfo.d.pixelStride;
        }
        
        if(scanline_stride) {
            *scanline_stride = storageInfo.d.scanlineStride;
        }
        
        if(offset) {
            *offset          = storageInfo.d.offset;
        }
        
        if(data_ptr) {
            *data_ptr        = storageInfo.d.dataPtr;
        }
    }
    
    if(band_stride) {
        *band_stride         = storageInfo.d.bandStride;
    }
    

    return XIL_SUCCESS;
}

void
XilStorage::setPixelStride(unsigned int pixel_stride,
                           unsigned int band_num)
{
    if(band_num >= numBands || pixel_stride == 0) {
        XIL_ERROR(image == NULL ? NULL : image->getSystemState(),
                  XIL_ERROR_USER, "di-426", TRUE);
        return;
    }

    if((storageType == XIL_GENERAL || band_num > 0) && arraysInitialized == FALSE) {
        if(initializeArrays() == XIL_FAILURE) {
            return;
        }
    }
    
    if(arraysInitialized) {
        storageInfo.a.pixelStrides[band_num] = pixel_stride;
    } else {
        storageInfo.d.pixelStride           = pixel_stride;
    }
}
    
void
XilStorage::setScanlineStride(unsigned int scanline_stride,
                              unsigned int band_num)
{
    if(band_num >= numBands || scanline_stride == 0) {
        XIL_ERROR(image == NULL ? NULL : image->getSystemState(),
                  XIL_ERROR_USER, "di-426", TRUE);
        return;
    }

    if((storageType == XIL_GENERAL || band_num > 0) && arraysInitialized == FALSE) {
        if(initializeArrays() == XIL_FAILURE) {
            return;
        }
    }
    
    if(arraysInitialized) {
        storageInfo.a.scanlineStrides[band_num] = scanline_stride;
    } else {
        storageInfo.d.scanlineStride           = scanline_stride;
    }
}

void
XilStorage::setBandStride(unsigned int band_stride)
{
    if(band_stride == 0) {
        XIL_ERROR(image == NULL ? NULL : image->getSystemState(),
                  XIL_ERROR_USER, "di-426", TRUE);
        return;
    }

    storageInfo.d.bandStride = band_stride;
}
    
void
XilStorage::setOffset(unsigned int offset,
                      unsigned int band_num)
{
    if(band_num >= numBands) {
        XIL_ERROR(image == NULL ? NULL : image->getSystemState(),
                  XIL_ERROR_USER, "di-426", TRUE);
        return;
    }

    if((offset > 0) && (dataType != XIL_BIT)) {
        XIL_ERROR(image == NULL ? NULL : image->getSystemState(),
                  XIL_ERROR_USER, "di-408", TRUE);
        return;
    }

    if((storageType == XIL_GENERAL || band_num > 0) && arraysInitialized == FALSE) {
        if(initializeArrays() == XIL_FAILURE) {
            return;
        }
    }
    
    if(arraysInitialized) {
        storageInfo.a.offsets[band_num] = offset;
    } else {
        storageInfo.d.offset            = offset;
    }
}

void
XilStorage::setDataPtr(void*                      data_ptr,
                       unsigned int               band_num)
{
    if(band_num >= numBands) {
        XIL_ERROR(image == NULL ? NULL : image->getSystemState(),
                  XIL_ERROR_USER, "di-426", TRUE);
        return;
    }

    if((storageType == XIL_GENERAL || band_num > 0) && arraysInitialized == FALSE) {
        if(initializeArrays() == XIL_FAILURE) {
            return;
        }
    }
    
    if(arraysInitialized) {
        storageInfo.a.dataPtrs[band_num]         = data_ptr;
    } else {
        storageInfo.d.dataPtr                    = data_ptr;
    }
}

void
XilStorage::setDataReleaseFunc(XilDataReleaseFuncPtr  release_func,
                               void*                  user_args)
{
    if(arraysInitialized) {
        storageInfo.a.dataReleaseFuncs[0] = release_func;
        storageInfo.a.userArgs[0]         = user_args;
    } else {
        storageInfo.d.dataReleaseFunc     = release_func;
        storageInfo.d.userArgs            = user_args;
    }
}

void
XilStorage::setCoordinates(unsigned int x,
                           unsigned int y)
{
    xcoord = x;
    ycoord = y;
}
    
void
XilStorage::getCoordinates(unsigned int* x,
                           unsigned int* y)
{
    if(x != NULL) {
        *x = xcoord;
    }

    if(y != NULL) {
        *y = ycoord;
    }
}

const unsigned int  _XIL_DATA_PTRS_UNION_OFFSET         = 0;
const unsigned int  _XIL_DATA_RELEASE_FUNC_UNION_OFFSET = 1;
const unsigned int  _XIL_USER_ARGS_UNION_OFFSET         = 2;
const unsigned int  _XIL_SCANLINE_STRIDES_UNION_OFFSET  = 3;
const unsigned int  _XIL_PIXEL_STRIDES_UNION_OFFSET     = 4;
const unsigned int  _XIL_OFFSETS_UNION_OFFSET           = 5;


XilStatus
XilStorage::initializeArrays()
{
    //
    //  Has the image been initialized?  We need it for dataTypeSize and numBands.
    //
    if(image == NULL) {
        return XIL_FAILURE;
    }

    //
    //  SOL64: 10/17/95  jlf   Treating pointers and ints the same.
    //
    //    Here I optimize our allocation and initialization by allocating and
    //    initializing a large array of 5*numBands and then pointing the union
    //    elements to a different location in the array.  It significantly
    //    speeds allocation, initialization and copying but it slightly
    //    complicates the code.  In addition, it is an issue if
    //    pointers and unsigned ints are not the same.
    //
    //    If you change this, be sure to update the destructor.
    //
    void** base_ptr = new void*[_XIL_NUM_STORAGE_ARRAY_UNION_MEMBERS*numBands];
    xili_memset(base_ptr, 0, _XIL_NUM_STORAGE_ARRAY_UNION_MEMBERS*numBands*sizeof(void*));
    
    //
    //  Initialize our data pointers.
    //
    {
        void** tmp_ptr = &base_ptr[_XIL_DATA_PTRS_UNION_OFFSET*numBands];
        if(tmp_ptr == NULL) {
            XIL_ERROR(image->getSystemState(), XIL_ERROR_RESOURCE, "di-1", TRUE);
            return XIL_FAILURE;
        }
        
        //
        //  Before setting the union to point at the new array, check to see if
        //  the information has already been set.
        //
        if(storageInfo.d.dataPtr != NULL) {
            if(storageType == XIL_PIXEL_SEQUENTIAL) {
                //
                //  The storage is XIL_PIXEL_SEQUENTIAL so we'll set all of
                //  the data pointers.
                //
                void* tmp_data = storageInfo.d.dataPtr;
                for(unsigned int i=0; i<numBands; i++) {
                    tmp_ptr[i] = tmp_data;
                    
                    tmp_data   = ((Xil_unsigned8*)tmp_data) + dataTypeSize;
                }
            } else if(storageType == XIL_BAND_SEQUENTIAL &&
                      storageInfo.d.bandStride != 0) {
                //
                //  The storage is XIL_BAND_SEQUENTIAL and band stride has been
                //  set so we'll set all of the data pointers using our band stride.
                //
                void* tmp_data = storageInfo.d.dataPtr;
                for(unsigned int i=0; i<numBands; i++) {
                    tmp_ptr[i] = tmp_data;
                    
                    tmp_data   = ((Xil_unsigned8*)tmp_data) +
                        storageInfo.d.bandStride*dataTypeSize; 
                }
            } else {
                //
                //  This means that we can only assume that the dataPtr is for
                //  band 0.
                //
                tmp_ptr[0] = storageInfo.d.dataPtr;
            }
        }
        storageInfo.a.dataPtrs = tmp_ptr;
    }
    
    //
    //  Initialize data release pointers.
    //
    {
        XilDataReleaseFuncPtr* tmp_ptr = (XilDataReleaseFuncPtr*)
            &base_ptr[_XIL_DATA_RELEASE_FUNC_UNION_OFFSET*numBands];
        if(tmp_ptr == NULL) {
            XIL_ERROR(image->getSystemState(), XIL_ERROR_RESOURCE, "di-1", TRUE);
            return XIL_FAILURE;
        }
        
        //
        //  Before setting the union to point at the new array, check to see if
        //  the information has already been set.
        //
        if(storageInfo.d.dataReleaseFunc != NULL) {
            tmp_ptr[0] = storageInfo.d.dataReleaseFunc;
        }
        
        storageInfo.a.dataReleaseFuncs = tmp_ptr;
    }
    
    //
    //  Initialize user arg pointers.
    //
    {
        void** tmp_ptr = &base_ptr[_XIL_USER_ARGS_UNION_OFFSET*numBands];
        if(tmp_ptr == NULL) {
            XIL_ERROR(image->getSystemState(), XIL_ERROR_RESOURCE, "di-1", TRUE);
            return XIL_FAILURE;
        }
        
        //
        //  Before setting the union to point at the new array, check to see if
        //  the information has already been set.
        //
        if(storageInfo.d.userArgs != NULL) {
            tmp_ptr[0] = storageInfo.d.userArgs;
        }
        
        storageInfo.a.userArgs = tmp_ptr;
    }
    
    //
    //  Initialize scanline stride array.
    //
    {
        unsigned int* tmp_ptr = (unsigned int*)
            &base_ptr[_XIL_SCANLINE_STRIDES_UNION_OFFSET*numBands];
        if(tmp_ptr == NULL) {
            XIL_ERROR(image->getSystemState(), XIL_ERROR_RESOURCE, "di-1", TRUE);
            return XIL_FAILURE;
        }
        
        //
        //  Before setting the union to point at the new array, check to see if
        //  the information has already been set for the single data case.
        //
        if(storageInfo.d.scanlineStride != 0) {
            for(unsigned int i=0; i<numBands; i++) {
                tmp_ptr[i] = storageInfo.d.scanlineStride;
            }
        }
            
        storageInfo.a.scanlineStrides = tmp_ptr;
    }
    
    //
    //  Initialize pixel stride array.
    //
    {
        unsigned int* tmp_ptr = (unsigned int*)
            &base_ptr[_XIL_PIXEL_STRIDES_UNION_OFFSET*numBands];
        if(tmp_ptr == NULL) {
            XIL_ERROR(image->getSystemState(), XIL_ERROR_RESOURCE, "di-1", TRUE);
            return XIL_FAILURE;
        }
        
        //
        //  Before setting the union to point at the new array, check to see if
        //  the information has already been set for the single data case.
        //
        if(storageInfo.d.pixelStride != 0) {
            for(unsigned int i=0; i<numBands; i++) {
                tmp_ptr[i] = storageInfo.d.pixelStride;
            }
        }
            
        storageInfo.a.pixelStrides = tmp_ptr;
    }
    
    //
    //  Initialize offsets array.
    //
    {
        unsigned int* tmp_ptr = (unsigned int*)
            &base_ptr[_XIL_OFFSETS_UNION_OFFSET*numBands];
        if(tmp_ptr == NULL) {
            XIL_ERROR(image->getSystemState(), XIL_ERROR_RESOURCE, "di-1", TRUE);
            return XIL_FAILURE;
        }
        
        //
        //  Before setting the union to point at the new array, check to see if
        //  the information has already been set for the single data case.
        //
        if(storageInfo.d.offset != 0) {
            for(unsigned int i=0; i<numBands; i++) {
                tmp_ptr[i] = storageInfo.d.offset;
            }
        }
            
        storageInfo.a.offsets = tmp_ptr;
    }

    arraysInitialized = TRUE;
    
    return XIL_SUCCESS;
}

//
//  Used to set just what's needed by XilImage::getStorage().  We can make
//  assumptions about how/what, etc. the object contains when it's set.
//
void
XilStorage::setInfo(const XilStorage* storage)
{
    //
    //  Initiailze our data members from the other object.
    //
    xcoord               = storage->xcoord;
    ycoord               = storage->ycoord;
    storageType          = storage->storageType;
    arraysInitialized    = storage->arraysInitialized;
    numBands             = storage->numBands;
    deviceInfo           = storage->deviceInfo;

    if(arraysInitialized) {
        void** base_ptr =
            new void*[_XIL_NUM_STORAGE_ARRAY_UNION_MEMBERS*numBands];

        if(base_ptr == NULL) {
            XIL_ERROR(image->getSystemState(), XIL_ERROR_RESOURCE,
                      "di-1", TRUE);
        } else {
            xili_memcpy(base_ptr,
                        storage->storageInfo.a.dataPtrs,
                        _XIL_NUM_STORAGE_ARRAY_UNION_MEMBERS*numBands*sizeof(void*));

            storageInfo.a.dataPtrs         =
                &base_ptr[_XIL_DATA_PTRS_UNION_OFFSET*numBands];
            storageInfo.a.dataReleaseFuncs = (XilDataReleaseFuncPtr*)
                &base_ptr[_XIL_DATA_RELEASE_FUNC_UNION_OFFSET*numBands];
            storageInfo.a.userArgs         =
                &base_ptr[_XIL_USER_ARGS_UNION_OFFSET*numBands];
            storageInfo.a.scanlineStrides  = (unsigned int*)
                &base_ptr[_XIL_SCANLINE_STRIDES_UNION_OFFSET*numBands];
            storageInfo.a.pixelStrides     = (unsigned int*)
                &base_ptr[_XIL_PIXEL_STRIDES_UNION_OFFSET*numBands];
            storageInfo.a.offsets          = (unsigned int*)
                &base_ptr[_XIL_OFFSETS_UNION_OFFSET*numBands];
        }
    } else {
        storageInfo.d        = storage->storageInfo.d;
    }
}

const XilStorage&
XilStorage::operator =(const XilStorage& rval)
{
    //
    //  Destroy existing information in us before setting the new information
    //  from the rval.
    //
    this->~XilStorage();

    //
    //  Initiailze our data members from the other object.
    //
    dataType             = rval.dataType;
    dataTypeSize         = rval.dataTypeSize;
    image                = rval.image;
    xcoord               = rval.xcoord;
    ycoord               = rval.ycoord;
    storageType          = rval.storageType;
    arraysInitialized    = rval.arraysInitialized;
    numBands             = rval.numBands;

    //
    //  TODO:  10/18/95 jlf  deviceInfo may need to be copied not referenced
    //
    //    Here we only copy a reference to whatever the deviceInfo pointer
    //    contains.  This may not be sufficient.  We may need to make a class
    //    which has a virtual copy method.
    //
    deviceInfo           = rval.deviceInfo;

    //
    // TODO: maynard - 1/26/96
    //       note that we may want to create an =operator for XiliStorageTag
    //       in order to increment the refcounter so that subsequent
    //       decrements don't go below zero.
    //
    storageTag           = rval.storageTag;
    tagSet               = rval.tagSet;
    
    if(arraysInitialized) {
        void** base_ptr =
            new void*[_XIL_NUM_STORAGE_ARRAY_UNION_MEMBERS*numBands];

        if(base_ptr == NULL) {
            XIL_ERROR(image->getSystemState(), XIL_ERROR_RESOURCE,
                      "di-1", TRUE);
            return *this;
        }

        xili_memcpy(base_ptr,
                    rval.storageInfo.a.dataPtrs,
                    _XIL_NUM_STORAGE_ARRAY_UNION_MEMBERS*numBands*sizeof(void*));

        storageInfo.a.dataPtrs         =
            &base_ptr[_XIL_DATA_PTRS_UNION_OFFSET*numBands];
        storageInfo.a.dataReleaseFuncs = (XilDataReleaseFuncPtr*)
            &base_ptr[_XIL_DATA_RELEASE_FUNC_UNION_OFFSET*numBands];
        storageInfo.a.userArgs         =
            &base_ptr[_XIL_USER_ARGS_UNION_OFFSET*numBands];
        storageInfo.a.scanlineStrides  = (unsigned int*)
            &base_ptr[_XIL_SCANLINE_STRIDES_UNION_OFFSET*numBands];
        storageInfo.a.pixelStrides     = (unsigned int*)
            &base_ptr[_XIL_PIXEL_STRIDES_UNION_OFFSET*numBands];
        storageInfo.a.offsets          = (unsigned int*)
            &base_ptr[_XIL_OFFSETS_UNION_OFFSET*numBands];
    } else {
        storageInfo.d        = rval.storageInfo.d;
    }

    return *this;
}

//------------------------------------------------------------------------
//
//  Function:	XilStorage::setInfoFromStorage()
//
//  Description:
//	This function initializes the information in this storage object
//	from another storage object while taking x, y, and band offsets 
//      into account based on the information it has available.
//	
//	It does not have access to what is contained in the deviceInfo
//	member so it only stores a reference to it.
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
XilStatus
XilStorage::setInfoFromStorage(const XilStorage* other_storage,
                               int               x_offset,
                               int               y_offset,
                               int               band_offset)
{
    if(dataType != other_storage->dataType) {
        XIL_ERROR(image->getSystemState(), XIL_ERROR_INTERNAL, "di-405", TRUE);
        return XIL_FAILURE;
    }

    //
    //  If the number of bands our storage object expects is larger than is
    //  available in the other storage object, flag an error.
    //
    if((band_offset+numBands) > other_storage->numBands ||
       (band_offset+numBands) <= 0) {
        XIL_ERROR(image->getSystemState(), XIL_ERROR_INTERNAL, "di-406", TRUE);
        return XIL_FAILURE;
    }

    dataTypeSize         = other_storage->dataTypeSize;
    storageType          = other_storage->storageType;
    deviceInfo           = other_storage->deviceInfo;

    //
    //  Use the arrays if their used in the other object.
    //
    //  TODO: 10/18/95 jlf  Can optimize a special case.
    //
    //    We can optimize a special case where the other object uses arrays,
    //    but it doesn't need to.  In this case, we can just set the .d
    //    portion of storageInfo.  It may be better to do this in getType()
    //    instead of here.
    //
    if(other_storage->arraysInitialized) {
        //
        //  If we haven't initialized our arrays, initialize them.
        //
        if(!arraysInitialized) {
            void** base_ptr =
                new void*[_XIL_NUM_STORAGE_ARRAY_UNION_MEMBERS*numBands];

            storageInfo.a.dataPtrs         =
                &base_ptr[_XIL_DATA_PTRS_UNION_OFFSET*numBands];
            storageInfo.a.dataReleaseFuncs = (XilDataReleaseFuncPtr*)
                &base_ptr[_XIL_DATA_RELEASE_FUNC_UNION_OFFSET*numBands];
            storageInfo.a.userArgs         =
                &base_ptr[_XIL_USER_ARGS_UNION_OFFSET*numBands];
            storageInfo.a.scanlineStrides  = (unsigned int*)
                &base_ptr[_XIL_SCANLINE_STRIDES_UNION_OFFSET*numBands];
            storageInfo.a.pixelStrides     = (unsigned int*)
                &base_ptr[_XIL_PIXEL_STRIDES_UNION_OFFSET*numBands];
            storageInfo.a.offsets          = (unsigned int*)
                &base_ptr[_XIL_OFFSETS_UNION_OFFSET*numBands];

            arraysInitialized = TRUE;
        }

        //
        //  Copy the information while and take the band offset into account
        //  by skipping over entries in the other storage.
        //
        for(unsigned int i=band_offset, j=0; j<numBands; i++, j++) {
           
            storageInfo.a.dataReleaseFuncs[j] =
                other_storage->storageInfo.a.dataReleaseFuncs[i];
            
            storageInfo.a.userArgs[j] =
                other_storage->storageInfo.a.userArgs[i];
            
            storageInfo.a.scanlineStrides[j] =
                other_storage->storageInfo.a.scanlineStrides[i];
            
            storageInfo.a.pixelStrides[j] =
                other_storage->storageInfo.a.pixelStrides[i];
            
	    if(dataType == XIL_BIT) {
		storageInfo.a.dataPtrs[j] =
		    ((Xil_unsigned8*)other_storage->storageInfo.a.dataPtrs[i]) + 
		    (y_offset*other_storage->storageInfo.a.scanlineStrides[i]*dataTypeSize) +
		    (x_offset/8);
 	  
		storageInfo.a.offsets[j] = x_offset%8;
	    } else if(dataType == XIL_UNSIGNED_4) {
		storageInfo.a.dataPtrs[j] =
		    ((Xil_unsigned8*)other_storage->storageInfo.a.dataPtrs[i]) + 
		    (y_offset*other_storage->storageInfo.a.scanlineStrides[i]*dataTypeSize) +
		    (x_offset/2);

		storageInfo.a.offsets[j] = x_offset%4;
	    } else {
		storageInfo.a.dataPtrs[j] =
		    ((Xil_unsigned8*)other_storage->storageInfo.a.dataPtrs[i]) + 
		    (y_offset*other_storage->storageInfo.a.scanlineStrides[i]*dataTypeSize) +
		    (x_offset*other_storage->storageInfo.a.pixelStrides[i]*dataTypeSize);
		storageInfo.a.offsets[j] =
		    other_storage->storageInfo.a.offsets[i];
	    }
		
	}
    } else {
        storageInfo.d          = other_storage->storageInfo.d;

	if(dataType == XIL_BIT) {
	    storageInfo.d.dataPtr  =
		(Xil_unsigned8*)storageInfo.d.dataPtr +
		(y_offset*storageInfo.d.scanlineStride*dataTypeSize) +
		(x_offset/8) +
		(band_offset*storageInfo.d.bandStride*dataTypeSize);
	    storageInfo.d.offset = x_offset%8;
	} else if(dataType == XIL_UNSIGNED_4) {
	    storageInfo.d.dataPtr  =
		(Xil_unsigned8*)storageInfo.d.dataPtr +
		(y_offset*storageInfo.d.scanlineStride*dataTypeSize) +
		(x_offset/2) +
		(band_offset*storageInfo.d.bandStride*dataTypeSize);
	    storageInfo.d.offset = x_offset%4;
	} else {
	    storageInfo.d.dataPtr  = (Xil_unsigned8*)storageInfo.d.dataPtr +
		(y_offset*storageInfo.d.scanlineStride*dataTypeSize) +
		(x_offset*storageInfo.d.pixelStride*dataTypeSize) +
		(band_offset*storageInfo.d.bandStride*dataTypeSize);
	}
    }

    return XIL_SUCCESS;
}

//------------------------------------------------------------------------
//
//  Function:	XilStorage::setDataInfo
//
//  Description:
//	These functions are used by setExportedMemoryStorage to set the
//      information in a XilStorage object. There are two versions of the
//	routine, one for BIT and the other for BYTE and SHORT. Since the
//      routines are used to set from the XilMemoryStorage object, certain
//      assumptions can be made about the layout.
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

//
// This version if for XIL_BYTE and XIL_SHORT type, containing no offset information
// The other storage values have defaults set in the constructor.
//
XilStatus
XilStorage::setDataInfo(XilDataType  data_type,
                        unsigned int pixel_stride,
                        unsigned int scanline_stride,
                        void*        data_ptr,
                        unsigned int /* band_num */)
{

    // I know that I can use only the non-general form of the storage since
    // it is from an XilMemoryStorage object.
    dataType = data_type;
    if(pixel_stride) {
        storageInfo.d.pixelStride = pixel_stride;
    }
    
    if(scanline_stride) {
        storageInfo.d.scanlineStride = scanline_stride;
    }

    if(data_ptr) {
        storageInfo.d.dataPtr = data_ptr;
    }
    return XIL_SUCCESS;
}

//
// This->version is for XIL_BIT type, containing offset information. 
// The other storage values have defaults set in the constructor.
//
XilStatus
XilStorage::setDataInfo(XilDataType  data_type,
                        unsigned int scanline_stride,
                        unsigned int band_stride,
                        unsigned int offset,
                        void*        data_ptr,
                        unsigned int /*band_num*/)
{
    // I know that I can use only the non-general form of the storage since
    // it is from an XilMemoryStorage object.
    dataType = data_type;
    if(scanline_stride) {
        storageInfo.d.scanlineStride = scanline_stride;
    }

    if(band_stride) {
        storageInfo.d.bandStride = band_stride;
    }
    
    if(offset) {
        storageInfo.d.offset = offset;
    }
    
    if(data_ptr) {
        storageInfo.d.dataPtr = data_ptr;
    }
    return XIL_SUCCESS;
}
