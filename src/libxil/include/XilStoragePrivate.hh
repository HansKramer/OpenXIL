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
//  File:	XilStoragePrivate.hh
//  Project:	XIL
//  Revision:	1.16
//  Last Mod:	10:21:18, 03/10/00
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
#pragma ident	"@(#)XilStoragePrivate.hh	1.16\t00/03/10  "

#ifdef _XIL_PRIVATE_INCLUDES

#include "_XilTileList.hh"

//
//  This class must remain exactly 128 sizeof(pointer) in size.
//
class XiliStorageTag {
public:
    XilDeviceStorage*          storageDevice;
    XilBox*                    box;
    XilTileList*               tileList;
    Xil_boolean                isWritable;
    XilTile*                   tile;
    Xil_boolean                ownsData;

    //
    //  Offset by these values on destruction to put the pointer at the
    //  beginning of the data to be free'd.
    //
    Xil_boolean                useOffsets;
    unsigned int               xOffset;
    unsigned int               yOffset;
    unsigned int               bandOffset;

private:
    //
    //  Necessary padding...
    //
    void*                      _extra[118];
};
#endif // _XIL_PRIVATE_INCLUDES

#ifdef _XIL_PRIVATE_DATA

public:
                     XilStorage(XilImage*       img,
                                unsigned int    pixel_stride,
                                unsigned int    scanline_stride,
                                unsigned int    band_stride,
                                unsigned int    offset,
                                void*           data_ptr);
 

    //
    //  setDataInfo() is used only be setExportedMemoryStorage. Since it is
    //  only used on XilMemory storage, certain assumptions can be made about
    //  layout. 
    //
    
    //
    //  setDataInfo for  XIL_BYTE and XIL_SHORT images
    //
    XilStatus        setDataInfo(XilDataType  data_type,
                                 unsigned int pixel_stride,
                                 unsigned int scanline_stride,
                                 void*        data_ptr,
                                 unsigned int band_num);

    //
    //  setDataInfo for XIL_BIT image
    //
    XilStatus        setDataInfo(XilDataType  data_type,
                                 unsigned int scanline_stride,
                                 unsigned int band_stride,
                                 unsigned int offset,
                                 void*        data_ptr,
                                 unsigned int band_num);

    //
    //  Equality Operator
    //
    const XilStorage& operator = (const XilStorage& new_val);

    //
    //  This will set the actual storage information from another storage
    //  object and not a verbatim copy.
    //
    XilStatus      setInfoFromStorage(const XilStorage* other_storage,
                                      int               x_offset     = 0,
                                      int               y_offset     = 0,
                                      int               band_offset  = 0);
    //
    //  Used to set just what's needed by XilImage::getStorage().  We can make
    //  assumptions about how/what, etc. the object contains when it's set.
    //
    void             setInfo(const XilStorage* storage);

    void             determineType();

    XilStatus        initializeArrays();

    XiliStorageTag*  refStorageTag()
    {
        return &storageTag;
    }

    void             storageTagIsSet()
    {
        tagSet = TRUE;
    }

    Xil_boolean      isStorageTagSet()
    {
        return tagSet;
    }

private:
    //
    //  This is the ONLY data allowed in the private section of this class.
    //
    XiliStorageTag          storageTag;

#endif  // _XIL_PRIVATE_DATA
