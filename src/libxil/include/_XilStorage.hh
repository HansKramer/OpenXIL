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
//  File:	_XilStorage.hh
//  Project:	XIL
//  Revision:	1.26
//  Last Mod:	10:21:02, 03/10/00
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
//  MT-level:  UNSAFE
//	
//------------------------------------------------------------------------
//	COPYRIGHT
//------------------------------------------------------------------------
#pragma ident	"@(#)_XilStorage.hh	1.26\t00/03/10  "

#ifndef _XIL_STORAGE_HH
#define _XIL_STORAGE_HH

//
//  C Includes
//
#include "_XilDefines.h"

//
//  Private Includes
//
#ifdef  _XIL_LIBXIL_PRIVATE
#define _XIL_PRIVATE_INCLUDES

#include "XilStoragePrivate.hh"

#undef  _XIL_PRIVATE_INCLUDES
#endif

class XilStorage {
public:
    //
    //  Check the type of storage represented by this object.
    //
    Xil_boolean     isType(XilStorageType type);

    //
    //  Get the image represented by this object.
    //
    XilImage*       getImage();
    
    //
    //  Set which image this object represents.
    //
    void            setImage(XilImage* image);
    
    //
    //  Storage information aquisition members.
    //
    //  This first set of methods is for non-XIL_GENERAL storage.
    //
    unsigned int    getPixelStride();
    unsigned int    getScanlineStride();
    unsigned int    getOffset();
    void*           getDataPtr();
    XilStatus       getStorageInfo(unsigned int* pixel_stride,
                                   unsigned int* scanline_stride,
                                   unsigned int* band_stride,
                                   unsigned int* offset,
                                   void**        data_ptr);
    void            getDataReleasePtr(XilDataReleaseFuncPtr* release_func,
                                      void**                 user_args);


    //
    //  This second set of methods is for all types of storage.
    //
    unsigned int    getPixelStride(unsigned int band_num);
    unsigned int    getScanlineStride(unsigned int band_num);
    unsigned int    getOffset(unsigned int band_num);
    void*           getDataPtr(unsigned int band_num);
    XilStatus       getStorageInfo(unsigned int  band_num,
                                   unsigned int* pixel_stride,
                                   unsigned int* scanline_stride,
                                   unsigned int* offset,
                                   void**        data_ptr);
    void            getDataReleasePtr(unsigned int           band_num,
                                      XilDataReleaseFuncPtr* release_func,
                                      void**                 user_args);

    //
    //  Only valid for XIL_BAND_SEQUENTIAL and XIL_PIXEL_SEQUENTIAL (always
    //  returns 1) storage.  Zero will be returned for XIL_GENERAL storage.
    //
    unsigned int    getBandStride();

    //
    //  Release and reset the storage information contained in this object.
    //  This is equivalent to destroying the object but the object itself will
    //  persist, most of the information represented by the object will be
    //  cleared.
    //
    //  NOTE:  Which image the storage object represents does NOT get cleared.
    //
    void            release();

    //
    //  Set/Get Device-Specific Information
    //
    void*           getDeviceSpecificInfo();
    void            setDeviceSpecificInfo(void* dev_info);
    
    //
    //  Methods to set the storage information for a new chunk of data.  These
    //  are used by storage devices, I/O devices and the XilStorageAPI object.
    //
    void           setPixelStride(unsigned int pixel_stride,
                                  unsigned int band_num = 0);
    void           setScanlineStride(unsigned int scanline_stride,
                                     unsigned int band_num = 0);
    void           setOffset(unsigned int offset,
                             unsigned int band_num = 0);
    void           setDataPtr(void*                      data_ptr,
                              unsigned int               band_num = 0);
    void           setDataReleaseFunc(XilDataReleaseFuncPtr  release_func,
                                      void*                  user_args);
    void           setBandStride(unsigned int band_stride);
    
    //
    //  Upper Left-hand coordinates for the Data
    //
    void           setCoordinates(unsigned int x,
                                  unsigned int y);
    
    void           getCoordinates(unsigned int* x,
                                  unsigned int* y);
    
    //
    //  CONSTRUCTOR/DESTRUCTOR
    //
                   XilStorage(XilImage* image);
                   ~XilStorage();

    //
    //  The infomation about the data this storage object represents.
    //
    class XilStorageData {
    public:
        void*                  dataPtr;
        XilDataReleaseFuncPtr  dataReleaseFunc;
        void*                  userArgs;
        unsigned int           scanlineStride;
        unsigned int           pixelStride;
        unsigned int           offset;
        unsigned int           bandStride;
    };
    
    class XilStorageDataArray {
    public:
        void**                 dataPtrs;
        XilDataReleaseFuncPtr* dataReleaseFuncs;
        void**                 userArgs;
        unsigned int*          scanlineStrides;
        unsigned int*          pixelStrides;
        unsigned int*          offsets;
    };

private:
    //
    //  The datatype and its size.
    //
    XilDataType             dataType;
    unsigned int            dataTypeSize;

    //
    //  The XilImage this storage represents.
    //
    XilImage*               image;

    //
    //  Upper Left-hand coordinates of the base data
    //
    unsigned int            xcoord;
    unsigned int            ycoord;
    
    //
    //  The type of storage layout represented by this object.
    //
    XilStorageType          storageType;

    //
    //  Union for either arrays or single data entries.
    //
    union {
        XilStorageData      d;
        XilStorageDataArray a;
    }                       storageInfo;

    //
    //  Flag as to whether the arrays for the multiple bands have been initialized.
    //
    Xil_boolean             arraysInitialized;
    
    //
    //  The number of bands (and thus the size of the arrays) for this object.
    //
    unsigned int            numBands;

    //
    //  Device-specific information a storage device may attach to the object.
    //
    void*                   deviceInfo;

    //
    //  Storage-specific information the XilImage attaches to the object.
    //
    Xil_boolean             tagSet;

    //
    //  The number of array members in the array union.
    //
    const unsigned int      _XIL_NUM_STORAGE_ARRAY_UNION_MEMBERS;
    
    //
    //  Extra data for future growth of the object.
    //
    void*                   _extraData[64];

#ifdef _XIL_LIBXIL_PRIVATE
#define _XIL_PRIVATE_DATA
    
#include "XilStoragePrivate.hh"
    
#undef  _XIL_PRIVATE_DATA
#else // _XIL_LIBXIL_PRIVATE
    //
    //  We have some hidden data to account for...
    //
    void*                   _classData[128];
#endif
};

#endif // _XIL_STORAGE_HH
