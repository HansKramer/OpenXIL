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
//  File:	ComputeInfo.hh
//  Project:	XIL
//  Revision:	1.22
//  Last Mod:	12:34:34, 03/13/97
//
//  Description:
//	This contains the "simple" compute interface for easing
//    development of compute routines for the XIL library.
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
#pragma ident	"@(#)ComputeInfo.hh	1.22\t97/03/13  "

//
//  C++ Includes
//
#include <xil/xilGPI.hh>
#include "XiliUtils.hh"

//
//  Private Includes
//
#ifdef  _XIL_SIMPLE_COMPUTE_PRIVATE
#define _XIL_PRIVATE_INCLUDES

#include "ComputeInfoPrivate.hh"

#undef  _XIL_PRIVATE_INCLUDES
#endif

//
//  The maximum number of sources and destinations ComputeInfo supports.
//
#define COMPUTE_INFO_MAX_SRCS  3
#define COMPUTE_INFO_MAX_DSTS  1

class ComputeInfo {
public:
    //
    //  This is the function that gets new information and sets all of the
    //    data fields.  Although, all of the storage-related data fields are
    //    not useful until getStorageType() has been called.
    //
    virtual Xil_boolean     hasMoreInfo();

    //
    //  In order to use any of the storage data members, the storage type must
    //    be known.  Storage of type XIL_GENERAL does not use the data fields
    //    below -- only the member functions.  The type of storage may change
    //    after every call to hasMoreInfo().  This call will indicate whether
    //    ALL of the images are of this type.  Use the storage objects below
    //    to test for mismatches.
    //
    Xil_boolean             isStorageType(XilStorageType target_type);

    //
    //  TODO:  Should there be more special-case tests of the storage
    //         information about to be used so that optimizations can be made?
    //         We often had incorrect or insufficient tests in the XIL 1.2
    //         compute routines for optimizations.  It might be good to put
    //         some tests in here for word-alignment, offset-alighnment,
    //         packed data storage, etc.
    //

    //
    //  THESE ARE VALID IMMEDIATELY AFTER OBJECT CONSTRUCTION
    //
    unsigned int            src1NumBands;
    unsigned int            src2NumBands;
    unsigned int            src3NumBands;
    unsigned int            destNumBands;

    //
    //  The Rect and Number of Bands to operate on.
    //
    //  THESE ARE ONLY VALID AFTER hasMoreInfo() HAS BEEN CALLED.
    //
    int                     x;
    int                     y;
    unsigned int            xsize;
    unsigned int            ysize;

    //
    //  The storage objects.
    //
    //  THESE ARE ONLY VALID AFTER hasMoreInfo() HAS BEEN CALLED.
    //
    XilStorage              src1Storage;
    XilStorage              src2Storage;
    XilStorage              src3Storage;
    XilStorage              destStorage;

    //
    //  Pixel-Sequential-Specific Non-DataType-Specific Data Members
    //
    //  THESE ARE ONLY VALID AFTER BOTH hasMoreInfo() AND getStorageType()
    //  HAVE BEEN CALLED -- AND WHEN getStorageType() returns
    //  XIL_PIXEL_SEQUENTIAL. 
    // 
    unsigned int            src1ScanlineStride;
    unsigned int            src2ScanlineStride;
    unsigned int            src3ScanlineStride;
    unsigned int            destScanlineStride;

    unsigned int            src1PixelStride;
    unsigned int            src2PixelStride;
    unsigned int            src3PixelStride;
    unsigned int            destPixelStride;

    //
    //  Band-Sequential-Specific Non-DataType-Specific Data Members
    //
    //
    //  THESE ARE ONLY VALID AFTER BOTH hasMoreInfo() AND getStorageType()
    //  HAVE BEEN CALLED -- AND WHEN getStorageType() returns
    //  XIL_BAND_SEQUENTIAL. 
    //
    unsigned int            src1BandStride;
    unsigned int            src2BandStride;
    unsigned int            src3BandStride;
    unsigned int            destBandStride;

    //
    //  Indicates whether the object has been created successfully.
    //
    virtual Xil_boolean     isOK()=0;

    //
    //  The value to be returned to the XIL Core at the conclusion of the
    //  compute routine.
    //
    XilStatus               returnValue;

    //
    //  Gets the system state for this operation.
    //
    XilSystemState*         getSystemState() {
        if(dest == NULL) {
            return src1->getSystemState();
        } else {
            return dest->getSystemState();
        }
    }

protected:
    ComputeInfo(XilOp*       op,
                unsigned int op_count,
                XilRoi*      roi,
                XilBoxList*  bl);

    //
    //  There is nothing to destroy so we don't expose one to save a function call.
    //

    //
    //  Get the Next Box From the BoxList, etc.
    //
    virtual Xil_boolean     getNextBox();

    //
    //  Used by the base getNextBox() routine to indicate to the derived
    //    classes that the storage objects have changed and that they should
    //    fill in their datatype-specific storage information from the
    //    XilStorage objects.
    //
    virtual XilStatus       updateForNewRect()=0;
    
    //
    //  Data Members...
    //    
    XilOp*                  srcOp;
    XilOp*                  dstOp;
    unsigned int            op_count;
    XilRoi*                 roi;
    XilBoxList*             bl;

    XilImage*               src1;
    XilImage*               src2;
    XilImage*               src3;
    XilImage*               dest;

    XilBox*                 boxes[COMPUTE_INFO_MAX_SRCS+COMPUTE_INFO_MAX_DSTS];
    const unsigned int      src1BoxesOffset;
    const unsigned int      src2BoxesOffset;
    const unsigned int      src3BoxesOffset;
    unsigned int            destBoxesOffset;
    
    void*                   src1BaseData;
    void*                   src2BaseData;
    void*                   src3BaseData;
    void*                   destBaseData;

    XilRectList             rl;

    unsigned int            numSrcs;
    unsigned int            numDsts;
    unsigned int            boxCount;

    Xil_boolean             doneWithRectList;
    Xil_boolean             onNewBox;
    Xil_boolean             pixelSequentialFlag;
    Xil_boolean             bandSequentialFlag;
    
    Xil_boolean             isOKFlag;
};

//------------------------------------------------------------------------
//
//  Class:	ComputeInfoBIT
//
//  Description:
//	XIL_BIT-specific implementation of base ComputeInfo.  This only
//      handles the case where the srcs and the destination are all of the
//      XIL_BIT data type.
//	
//  MT-level:  Unsafe
//	
//  Notes:
//	XIL_BIT images are not allowed to be XIL_GENERAL or 
//	XIL_PIXEL_SEQUENTIAL.  Thus, those interfaces have been 
//	removed from this class.
//	
//  Deficiencies/ToDo:
//	
//	
//------------------------------------------------------------------------
class ComputeInfoBIT : public ComputeInfo {
public:
    //
    //  Tests whether the object was created ok.
    //
    Xil_boolean    isOK();

    //
    //  Constructor/Destructor
    //
                   ComputeInfoBIT(XilOp*       op,
                                  unsigned int op_count,
                                  XilRoi*      roi,
                                  XilBoxList*  bl);

    //
    //  Arbitrary Data Access
    //
    Xil_unsigned8*          getSrc1Data(unsigned int band_num);
    Xil_unsigned8*          getSrc2Data(unsigned int band_num);
    Xil_unsigned8*          getSrc3Data(unsigned int band_num);
    Xil_unsigned8*          getDestData(unsigned int band_num);

    unsigned int            getSrc1ScanlineStride(unsigned int band_num);
    unsigned int            getSrc2ScanlineStride(unsigned int band_num);
    unsigned int            getSrc3ScanlineStride(unsigned int band_num);
    unsigned int            getDestScanlineStride(unsigned int band_num);
    
    unsigned int            getSrc1Offset(unsigned int band_num);
    unsigned int            getSrc2Offset(unsigned int band_num);
    unsigned int            getSrc3Offset(unsigned int band_num);
    unsigned int            getDestOffset(unsigned int band_num);
    
    //
    //  Pixel and Band Sequential-Specific DataType-Specific Data Pointers
    //
    //
    //  THESE ARE ONLY VALID AFTER BOTH hasMoreInfo() AND getStorageType()
    //  HAVE BEEN CALLED. 
    //
    Xil_unsigned8* src1Data;
    Xil_unsigned8* src2Data;
    Xil_unsigned8* src3Data;
    Xil_unsigned8* destData;
    
    unsigned int   src1Offset;
    unsigned int   src2Offset;
    unsigned int   src3Offset;
    unsigned int   destOffset;

protected:
    XilStatus      updateForNewRect();
    
private:
    unsigned int   src1PixelStride;
    unsigned int   src2PixelStride;
    unsigned int   src3PixelStride;
    unsigned int   destPixelStride;
};


//------------------------------------------------------------------------
//
//  Class:	ComputeInfoBYTE
//
//  Description:
//	XIL_BYTE-specific implementation of base ComputeInfo.  This only
//      handles the case where the srcs and the destination are all of the
//      XIL_BYTE data type. 
//	
//	
//	
//	
//  MT-level:  Unsafe
//	
//  Notes:
//	
//	
//  Deficiencies/ToDo:
//	
//	
//------------------------------------------------------------------------
class ComputeInfoBYTE : public ComputeInfo {
public:
    //
    //  Tests whether the object was created ok.
    //
    Xil_boolean     isOK();

    //
    //  Constructor/Destructor
    //
    ComputeInfoBYTE(XilOp*       op,
                    unsigned int op_count,
                    XilRoi*      roi,
                    XilBoxList*  bl);

    //
    //  Arbitrary Data Access
    //
    Xil_unsigned8*          getSrc1Data(unsigned int band_num);
    Xil_unsigned8*          getSrc2Data(unsigned int band_num);
    Xil_unsigned8*          getSrc3Data(unsigned int band_num);
    Xil_unsigned8*          getDestData(unsigned int band_num);

    unsigned int            getSrc1ScanlineStride(unsigned int band_num);
    unsigned int            getSrc2ScanlineStride(unsigned int band_num);
    unsigned int            getSrc3ScanlineStride(unsigned int band_num);
    unsigned int            getDestScanlineStride(unsigned int band_num);
    
    unsigned int            getSrc1PixelStride(unsigned int band_num);
    unsigned int            getSrc2PixelStride(unsigned int band_num);
    unsigned int            getSrc3PixelStride(unsigned int band_num);
    unsigned int            getDestPixelStride(unsigned int band_num);
    
    //
    //  Pixel and Band Sequential-Specific DataType-Specific Data Pointers
    //
    //
    //  THESE ARE ONLY VALID AFTER BOTH hasMoreInfo() AND getStorageType()
    //  HAVE BEEN CALLED. 
    //
    Xil_unsigned8*          src1Data;
    Xil_unsigned8*          src2Data;
    Xil_unsigned8*          src3Data;
    Xil_unsigned8*          destData;
    
protected:
    XilStatus               updateForNewRect();
};


//------------------------------------------------------------------------
//
//  Class:	ComputeInfoSHORT
//
//  Description:
//	XIL_SHORT-specific implementation of base ComputeInfo.  This only
//      handles the case where the srcs and the destination are all of the
//      XIL_SHORT data type.
//	
//	
//	
//	
//	
//  MT-level:  Unsafe
//	
//  Notes:
//	
//	
//  Deficiencies/ToDo:
//	
//	
//------------------------------------------------------------------------
class ComputeInfoSHORT : public ComputeInfo {
public:
    //
    //  Tests whether the object was created ok.
    //
    Xil_boolean     isOK();

    //
    //  Constructor/Destructor
    //
    ComputeInfoSHORT(XilOp*       op,
                     unsigned int op_count,
                     XilRoi*      roi,
                     XilBoxList*  bl);

    //
    //  Arbitrary Data Access
    //
    Xil_signed16*           getSrc1Data(unsigned int band_num);
    Xil_signed16*           getSrc2Data(unsigned int band_num);
    Xil_signed16*           getSrc3Data(unsigned int band_num);
    Xil_signed16*           getDestData(unsigned int band_num);

    unsigned int            getSrc1ScanlineStride(unsigned int band_num);
    unsigned int            getSrc2ScanlineStride(unsigned int band_num);
    unsigned int            getSrc3ScanlineStride(unsigned int band_num);
    unsigned int            getDestScanlineStride(unsigned int band_num);
    
    unsigned int            getSrc1PixelStride(unsigned int band_num);
    unsigned int            getSrc2PixelStride(unsigned int band_num);
    unsigned int            getSrc3PixelStride(unsigned int band_num);
    unsigned int            getDestPixelStride(unsigned int band_num);
    
    //
    //  Pixel and Band Sequential-Specific DataType-Specific Data Pointers
    //
    //
    //  THESE ARE ONLY VALID AFTER BOTH hasMoreInfo() AND getStorageType()
    //  HAVE BEEN CALLED. 
    //
    Xil_signed16*           src1Data;
    Xil_signed16*           src2Data;
    Xil_signed16*           src3Data;
    Xil_signed16*           destData;
    
protected:
    XilStatus               updateForNewRect();
};

//------------------------------------------------------------------------
//
//  Class:	ComputeInfoGENERAL
//
//  Description:
//	Non-datatype-specific implementation of the ComputeInfo class.  
//      It supports arbitrary datatypes between src and destination 
//	images.
//	
//	
//	
//	
//	
//	
//  MT-level:  Unsafe
//	
//  Notes:
//	
//	
//  Deficiencies/ToDo:
//	
//	
//------------------------------------------------------------------------
class ComputeInfoGENERAL : public ComputeInfo {
public:
    //
    //  Tests whether the object was created ok.
    //
    Xil_boolean    isOK();

    //
    //  Constructors/Destructor
    //
                   ComputeInfoGENERAL(XilOp*       op,
                                      unsigned int op_count,
                                      XilRoi*      roi,
                                      XilBoxList*  bl);
    
    //
    //  Arbitrary Data Access
    //
    void*          getSrc1Data(unsigned int band_num);
    void*          getSrc2Data(unsigned int band_num);
    void*          getSrc3Data(unsigned int band_num);
    void*          getDestData(unsigned int band_num);

    unsigned int   getSrc1ScanlineStride(unsigned int band_num);
    unsigned int   getSrc2ScanlineStride(unsigned int band_num);
    unsigned int   getSrc3ScanlineStride(unsigned int band_num);
    unsigned int   getDestScanlineStride(unsigned int band_num);
    
    unsigned int   getSrc1PixelStride(unsigned int band_num);
    unsigned int   getSrc2PixelStride(unsigned int band_num);
    unsigned int   getSrc3PixelStride(unsigned int band_num);
    unsigned int   getDestPixelStride(unsigned int band_num);
    
    unsigned int   getSrc1Offset(unsigned int band_num);
    unsigned int   getSrc2Offset(unsigned int band_num);
    unsigned int   getSrc3Offset(unsigned int band_num);
    unsigned int   getDestOffset(unsigned int band_num);
    
    //
    //  Pixel and Band Sequential-Specific DataType-Specific Data Pointers
    //
    //
    //  THESE ARE ONLY VALID AFTER BOTH hasMoreInfo() AND getStorageType()
    //  HAVE BEEN CALLED. 
    //
    void*          src1Data;
    void*          src2Data;
    void*          src3Data;
    void*          destData;
    
    unsigned int   src1Offset;
    unsigned int   src2Offset;
    unsigned int   src3Offset;
    unsigned int   destOffset;
    
protected:
    XilStatus      updateForNewRect();

    XilDataType    src1DataType;
    XilDataType    src2DataType;
    XilDataType    src3DataType;
    XilDataType    destDataType;
    
    unsigned int   src1DataTypeSize;
    unsigned int   src2DataTypeSize;
    unsigned int   src3DataTypeSize;
    unsigned int   destDataTypeSize;
};


