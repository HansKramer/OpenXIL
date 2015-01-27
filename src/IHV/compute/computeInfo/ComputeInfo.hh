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
//  Revision:	1.24
//  Last Mod:	10:22:25, 03/10/00
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
#pragma ident	"@(#)ComputeInfo.hh	1.24\t00/03/10  "

#ifndef _COMPUTE_INFO_HH
#define _COMPUTE_INFO_HH

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
//  Class:	ComputeInfoFLOAT
//
//  Description:
//	XIL_FLOAT-specific implementation of base ComputeInfo.  This only
//      handles the case where the srcs and the destination are all of the
//      XIL_FLOAT data type.
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
class ComputeInfoFLOAT : public ComputeInfo {
public:
    //
    //  Tests whether the object was created ok.
    //
    Xil_boolean     isOK();

    //
    //  Constructor/Destructor
    //
    ComputeInfoFLOAT(XilOp*       op,
                     unsigned int op_count,
                     XilRoi*      roi,
                     XilBoxList*  bl);

    //
    //  Arbitrary Data Access
    //
    Xil_float32*            getSrc1Data(unsigned int band_num);
    Xil_float32*            getSrc2Data(unsigned int band_num);
    Xil_float32*            getSrc3Data(unsigned int band_num);
    Xil_float32*            getDestData(unsigned int band_num);

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
    Xil_float32*            src1Data;
    Xil_float32*            src2Data;
    Xil_float32*            src3Data;
    Xil_float32*            destData;
    
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

//
//  Some convenience macros that assist with using the ComputeInfo classes.
//
#define COMPUTE_GENERAL_1S_1D(src1_datatype, dest_datatype, \
                              one_band_case, other_two_bands_case) \
    {  \
        const unsigned int num_bands = ci.destNumBands; \
        const unsigned int xsize     = ci.xsize;        \
        \
        if(ci.isStorageType(XIL_PIXEL_SEQUENTIAL)) { \
            const unsigned int src1_sstride  = ci.src1ScanlineStride; \
            const unsigned int dest_sstride  = ci.destScanlineStride; \
            \
            const unsigned int src1_pstride  = ci.src1PixelStride; \
            const unsigned int dest_pstride  = ci.destPixelStride; \
            \
            src1_datatype*     src1_scanline = (src1_datatype*)ci.src1Data; \
            dest_datatype*     dest_scanline = (dest_datatype*)ci.destData; \
            \
            if(num_bands == 1) {\
                if(src1_pstride == 1 && \
                   dest_pstride == 1) {\
                    for(unsigned int y=ci.ysize; y!=0; y--) {\
                        src1_datatype* src1 = src1_scanline;\
                        dest_datatype* dest = dest_scanline;\
                        \
                        for(unsigned int x=xsize; x!=0; x--) {\
                            {one_band_case;} \
                            src1++;\
                            dest++;\
                        }\
                        \
                        src1_scanline += src1_sstride;\
                        dest_scanline += dest_sstride;\
                    } \
                } else { \
                    for(unsigned int y=ci.ysize; y!=0; y--) {\
                        src1_datatype* src1 = src1_scanline;\
                        dest_datatype* dest = dest_scanline;\
                        \
                        for(unsigned int x=xsize; x!=0; x--) {\
                            {one_band_case;} \
                            src1 += src1_pstride;\
                            dest += dest_pstride;\
                        }\
                        \
                        src1_scanline += src1_sstride;\
                        dest_scanline += dest_sstride;\
                    }\
                } \
            } else if(num_bands == 3) {\
                if(src1_pstride == 3 && \
                   dest_pstride == 3) { \
                    for(unsigned int y=ci.ysize; y!=0; y--) {\
                        src1_datatype* src1 = src1_scanline;\
                        dest_datatype* dest = dest_scanline;\
                        \
                        for(unsigned int x=xsize; x!=0; x--) {\
                            {one_band_case;} \
                            {other_two_bands_case;} \
                            \
                            src1 += 3;\
                            dest += 3;\
                        }\
                        \
                        src1_scanline += src1_sstride;\
                        dest_scanline += dest_sstride;\
                    }\
                } else { \
                    for(unsigned int y=ci.ysize; y!=0; y--) {\
                        src1_datatype* src1 = src1_scanline;\
                        dest_datatype* dest = dest_scanline;\
                        \
                        for(unsigned int x=xsize; x!=0; x--) {\
                            {one_band_case;} \
                            {other_two_bands_case;} \
                            \
                            src1 += src1_pstride;\
                            dest += dest_pstride;\
                        }\
                        \
                        src1_scanline += src1_sstride;\
                        dest_scanline += dest_sstride;\
                    }\
                } \
            } else {\
                for(unsigned int y=ci.ysize; y!=0; y--) {\
                    src1_datatype* src1_pixel = src1_scanline;\
                    dest_datatype* dest_pixel = dest_scanline;\
                    \
                    for(unsigned int x=xsize; x!=0; x--) {\
                        src1_datatype* src1 = src1_pixel;\
                        dest_datatype* dest = dest_pixel;\
                        \
                        for(unsigned int band=0; band<num_bands; band++) {\
                             {one_band_case;} \
                             \
                            src1++;\
                            dest++;\
                        }\
                        \
                        src1_pixel += src1_pstride;\
                        dest_pixel += dest_pstride;\
                    }\
                    \
                    src1_scanline += src1_sstride;\
                    dest_scanline += dest_sstride;\
                }\
            }\
        } else {\
            for(unsigned int band=0; band<num_bands; band++) { \
                const unsigned int src1_pstride = ci.getSrc1PixelStride(band);\
                const unsigned int dest_pstride = ci.getDestPixelStride(band);\
                \
                const unsigned int src1_sstride = ci.getSrc1ScanlineStride(band);\
                const unsigned int dest_sstride = ci.getDestScanlineStride(band);\
                \
                src1_datatype*     src1_scanline = (src1_datatype*)ci.getSrc1Data(band);\
                dest_datatype*     dest_scanline = (dest_datatype*)ci.getDestData(band);\
                \
                for(unsigned int y=ci.ysize; y!=0; y--) { \
                    src1_datatype* src1 = src1_scanline;\
                    dest_datatype* dest = dest_scanline;\
                    \
                    for(unsigned int x=xsize; x!=0; x--) { \
                        {one_band_case;} \
                        \
                        src1 += src1_pstride;\
                        dest += dest_pstride;\
                    }\
                    \
                    src1_scanline += src1_sstride;\
                    dest_scanline += dest_sstride;\
                }\
            }\
        }\
    }

#define COMPUTE_GENERAL_2S_1D(src1_datatype, src2_datatype, dest_datatype, \
                              one_band_case, other_two_bands_case) \
    {  \
        const unsigned int num_bands = ci.destNumBands; \
        const unsigned int xsize     = ci.xsize;        \
        \
        if(ci.isStorageType(XIL_PIXEL_SEQUENTIAL)) { \
            const unsigned int src1_sstride  = ci.src1ScanlineStride; \
            const unsigned int src2_sstride  = ci.src2ScanlineStride; \
            const unsigned int dest_sstride  = ci.destScanlineStride; \
            \
            const unsigned int src1_pstride  = ci.src1PixelStride; \
            const unsigned int src2_pstride  = ci.src2PixelStride; \
            const unsigned int dest_pstride  = ci.destPixelStride; \
            \
            src1_datatype*     src1_scanline = (src1_datatype*)ci.src1Data; \
            src2_datatype*     src2_scanline = (src2_datatype*)ci.src2Data; \
            dest_datatype*     dest_scanline = (dest_datatype*)ci.destData; \
            \
            if(num_bands == 1) {\
                if(src1_pstride == 1 && \
                   src2_pstride == 1 && \
                   dest_pstride == 1) {\
                    for(unsigned int y=ci.ysize; y!=0; y--) {\
                        src1_datatype* src1 = src1_scanline;\
                        src2_datatype* src2 = src2_scanline;\
                        dest_datatype* dest = dest_scanline;\
                        \
                        for(unsigned int x=xsize; x!=0; x--) {\
                            {one_band_case;} \
                            src1++;\
                            src2++;\
                            dest++;\
                        }\
                        \
                        src1_scanline += src1_sstride;\
                        src2_scanline += src2_sstride;\
                        dest_scanline += dest_sstride;\
                    } \
                } else { \
                    for(unsigned int y=ci.ysize; y!=0; y--) {\
                        src1_datatype* src1 = src1_scanline;\
                        src2_datatype* src2 = src2_scanline;\
                        dest_datatype* dest = dest_scanline;\
                        \
                        for(unsigned int x=xsize; x!=0; x--) {\
                            {one_band_case;} \
                            src1 += src1_pstride;\
                            src2 += src2_pstride;\
                            dest += dest_pstride;\
                        }\
                        \
                        src1_scanline += src1_sstride;\
                        src2_scanline += src2_sstride;\
                        dest_scanline += dest_sstride;\
                    }\
                } \
            } else if(num_bands == 3) {\
                if(src1_pstride == 3 && \
                   src2_pstride == 3 && \
                   dest_pstride == 3) { \
                    for(unsigned int y=ci.ysize; y!=0; y--) {\
                        src1_datatype* src1 = src1_scanline;\
                        src2_datatype* src2 = src2_scanline;\
                        dest_datatype* dest = dest_scanline;\
                        \
                        for(unsigned int x=xsize; x!=0; x--) {\
                            {one_band_case;} \
                            {other_two_bands_case;} \
                            \
                            src1 += 3;\
                            src2 += 3;\
                            dest += 3;\
                        }\
                        \
                        src1_scanline += src1_sstride;\
                        src2_scanline += src2_sstride;\
                        dest_scanline += dest_sstride;\
                    }\
                } else { \
                    for(unsigned int y=ci.ysize; y!=0; y--) {\
                        src1_datatype* src1 = src1_scanline;\
                        src2_datatype* src2 = src2_scanline;\
                        dest_datatype* dest = dest_scanline;\
                        \
                        for(unsigned int x=xsize; x!=0; x--) {\
                            {one_band_case;} \
                            {other_two_bands_case;} \
                            \
                            src1 += src1_pstride;\
                            src2 += src2_pstride;\
                            dest += dest_pstride;\
                        }\
                        \
                        src1_scanline += src1_sstride;\
                        src2_scanline += src2_sstride;\
                        dest_scanline += dest_sstride;\
                    }\
                } \
            } else {\
                for(unsigned int y=ci.ysize; y!=0; y--) {\
                    src1_datatype* src1_pixel = src1_scanline;\
                    src2_datatype* src2_pixel = src2_scanline;\
                    dest_datatype* dest_pixel = dest_scanline;\
                    \
                    for(unsigned int x=xsize; x!=0; x--) {\
                        src1_datatype* src1 = src1_pixel;\
                        src2_datatype* src2 = src2_pixel;\
                        dest_datatype* dest = dest_pixel;\
                        \
                        for(unsigned int band=0; band<num_bands; band++) {\
                             {one_band_case;} \
                             \
                            src1++;\
                            src2++;\
                            dest++;\
                        }\
                        \
                        src1_pixel += src1_pstride;\
                        src2_pixel += src2_pstride;\
                        dest_pixel += dest_pstride;\
                    }\
                    \
                    src1_scanline += src1_sstride;\
                    src2_scanline += src2_sstride;\
                    dest_scanline += dest_sstride;\
                }\
            }\
        } else {\
            for(unsigned int band=0; band<num_bands; band++) { \
                const unsigned int src1_pstride = ci.getSrc1PixelStride(band);\
                const unsigned int src2_pstride = ci.getSrc2PixelStride(band);\
                const unsigned int dest_pstride = ci.getDestPixelStride(band);\
                \
                const unsigned int src1_sstride = ci.getSrc1ScanlineStride(band);\
                const unsigned int src2_sstride = ci.getSrc2ScanlineStride(band);\
                const unsigned int dest_sstride = ci.getDestScanlineStride(band);\
                \
                src1_datatype*     src1_scanline = (src1_datatype*)ci.getSrc1Data(band);\
                src2_datatype*     src2_scanline = (src2_datatype*)ci.getSrc2Data(band);\
                dest_datatype*     dest_scanline = (dest_datatype*)ci.getDestData(band);\
                \
                for(unsigned int y=ci.ysize; y!=0; y--) { \
                    src1_datatype* src1 = src1_scanline;\
                    src2_datatype* src2 = src2_scanline;\
                    dest_datatype* dest = dest_scanline;\
                    \
                    for(unsigned int x=xsize; x!=0; x--) { \
                        {one_band_case;} \
                        \
                        src1 += src1_pstride;\
                        src2 += src2_pstride;\
                        dest += dest_pstride;\
                    }\
                    \
                    src1_scanline += src1_sstride;\
                    src2_scanline += src2_sstride;\
                    dest_scanline += dest_sstride;\
                }\
            }\
        }\
    }

#define COMPUTE_GENERAL_3S_1D(src1_datatype, src2_datatype, \
                              src3_datatype, dest_datatype, \
                              one_band_case, other_two_bands_case) \
    {  \
        const unsigned int num_bands = ci.destNumBands; \
        const unsigned int xsize     = ci.xsize;        \
        \
        if(ci.isStorageType(XIL_PIXEL_SEQUENTIAL)) { \
            const unsigned int src1_sstride  = ci.src1ScanlineStride; \
            const unsigned int src2_sstride  = ci.src2ScanlineStride; \
            const unsigned int src3_sstride  = ci.src3ScanlineStride; \
            const unsigned int dest_sstride  = ci.destScanlineStride; \
            \
            const unsigned int src1_pstride  = ci.src1PixelStride; \
            const unsigned int src2_pstride  = ci.src2PixelStride; \
            const unsigned int src3_pstride  = ci.src3PixelStride; \
            const unsigned int dest_pstride  = ci.destPixelStride; \
            \
            src1_datatype*     src1_scanline = (src1_datatype*)ci.src1Data; \
            src2_datatype*     src2_scanline = (src2_datatype*)ci.src2Data; \
            src3_datatype*     src3_scanline = (src3_datatype*)ci.src3Data; \
            dest_datatype*     dest_scanline = (dest_datatype*)ci.destData; \
            \
            if(num_bands == 1) {\
                if(src1_pstride == 1 && \
                   src2_pstride == 1 && \
                   src3_pstride == 1 && \
                   dest_pstride == 1) {\
                    for(unsigned int y=ci.ysize; y!=0; y--) {\
                        src1_datatype* src1 = src1_scanline;\
                        src2_datatype* src2 = src2_scanline;\
                        src3_datatype* src3 = src3_scanline;\
                        dest_datatype* dest = dest_scanline;\
                        \
                        for(unsigned int x=xsize; x!=0; x--) {\
                            {one_band_case;} \
                            src1++;\
                            src2++;\
                            src3++;\
                            dest++;\
                        }\
                        \
                        src1_scanline += src1_sstride;\
                        src2_scanline += src2_sstride;\
                        src3_scanline += src3_sstride;\
                        dest_scanline += dest_sstride;\
                    } \
                } else { \
                    for(unsigned int y=ci.ysize; y!=0; y--) {\
                        src1_datatype* src1 = src1_scanline;\
                        src2_datatype* src2 = src2_scanline;\
                        src3_datatype* src3 = src3_scanline;\
                        dest_datatype* dest = dest_scanline;\
                        \
                        for(unsigned int x=xsize; x!=0; x--) {\
                            {one_band_case;} \
                            src1 += src1_pstride;\
                            src2 += src2_pstride;\
                            src3 += src3_pstride;\
                            dest += dest_pstride;\
                        }\
                        \
                        src1_scanline += src1_sstride;\
                        src2_scanline += src2_sstride;\
                        src3_scanline += src3_sstride;\
                        dest_scanline += dest_sstride;\
                    }\
                } \
            } else if(num_bands == 3) {\
                if(src1_pstride == 3 && \
                   src2_pstride == 3 && \
                   src3_pstride == 3 && \
                   dest_pstride == 3) { \
                    for(unsigned int y=ci.ysize; y!=0; y--) {\
                        src1_datatype* src1 = src1_scanline;\
                        src2_datatype* src2 = src2_scanline;\
                        src3_datatype* src3 = src3_scanline;\
                        dest_datatype* dest = dest_scanline;\
                        \
                        for(unsigned int x=xsize; x!=0; x--) {\
                            {one_band_case;} \
                            {other_two_bands_case;} \
                            \
                            src1 += 3;\
                            src2 += 3;\
                            src3 += 3;\
                            dest += 3;\
                        }\
                        \
                        src1_scanline += src1_sstride;\
                        src2_scanline += src2_sstride;\
                        src3_scanline += src3_sstride;\
                        dest_scanline += dest_sstride;\
                    }\
                } else { \
                    for(unsigned int y=ci.ysize; y!=0; y--) {\
                        src1_datatype* src1 = src1_scanline;\
                        src2_datatype* src2 = src2_scanline;\
                        src3_datatype* src3 = src3_scanline;\
                        dest_datatype* dest = dest_scanline;\
                        \
                        for(unsigned int x=xsize; x!=0; x--) {\
                            {one_band_case;} \
                            {other_two_bands_case;} \
                            \
                            src1 += src1_pstride;\
                            src2 += src2_pstride;\
                            src3 += src3_pstride;\
                            dest += dest_pstride;\
                        }\
                        \
                        src1_scanline += src1_sstride;\
                        src2_scanline += src2_sstride;\
                        src3_scanline += src3_sstride;\
                        dest_scanline += dest_sstride;\
                    }\
                } \
            } else {\
                for(unsigned int y=ci.ysize; y!=0; y--) {\
                    src1_datatype* src1_pixel = src1_scanline;\
                    src2_datatype* src2_pixel = src2_scanline;\
                    src3_datatype* src3_pixel = src3_scanline;\
                    dest_datatype* dest_pixel = dest_scanline;\
                    \
                    for(unsigned int x=xsize; x!=0; x--) {\
                        src1_datatype* src1 = src1_pixel;\
                        src2_datatype* src2 = src2_pixel;\
                        src3_datatype* src3 = src3_pixel;\
                        dest_datatype* dest = dest_pixel;\
                        \
                        for(unsigned int band=0; band<num_bands; band++) {\
                             {one_band_case;} \
                             \
                            src1++;\
                            src2++;\
                            src3++;\
                            dest++;\
                        }\
                        \
                        src1_pixel += src1_pstride;\
                        src2_pixel += src2_pstride;\
                        src3_pixel += src3_pstride;\
                        dest_pixel += dest_pstride;\
                    }\
                    \
                    src1_scanline += src1_sstride;\
                    src2_scanline += src2_sstride;\
                    src3_scanline += src3_sstride;\
                    dest_scanline += dest_sstride;\
                }\
            }\
        } else {\
            for(unsigned int band=0; band<num_bands; band++) { \
                const unsigned int src1_pstride = ci.getSrc1PixelStride(band);\
                const unsigned int src2_pstride = ci.getSrc2PixelStride(band);\
                const unsigned int src3_pstride = ci.getSrc3PixelStride(band);\
                const unsigned int dest_pstride = ci.getDestPixelStride(band);\
                \
                const unsigned int src1_sstride = ci.getSrc1ScanlineStride(band);\
                const unsigned int src2_sstride = ci.getSrc2ScanlineStride(band);\
                const unsigned int src3_sstride = ci.getSrc3ScanlineStride(band);\
                const unsigned int dest_sstride = ci.getDestScanlineStride(band);\
                \
                src1_datatype*     src1_scanline = (src1_datatype*)ci.getSrc1Data(band);\
                src2_datatype*     src2_scanline = (src2_datatype*)ci.getSrc2Data(band);\
                src3_datatype*     src3_scanline = (src3_datatype*)ci.getSrc3Data(band);\
                dest_datatype*     dest_scanline = (dest_datatype*)ci.getDestData(band);\
                \
                for(unsigned int y=ci.ysize; y!=0; y--) { \
                    src1_datatype* src1 = src1_scanline;\
                    src2_datatype* src2 = src2_scanline;\
                    src3_datatype* src3 = src3_scanline;\
                    dest_datatype* dest = dest_scanline;\
                    \
                    for(unsigned int x=xsize; x!=0; x--) { \
                        {one_band_case;} \
                        \
                        src1 += src1_pstride;\
                        src2 += src2_pstride;\
                        src3 += src3_pstride;\
                        dest += dest_pstride;\
                    }\
                    \
                    src1_scanline += src1_sstride;\
                    src2_scanline += src2_sstride;\
                    src3_scanline += src3_sstride;\
                    dest_scanline += dest_sstride;\
                }\
            }\
        }\
    }


//
//  Macros for loops which differ between 1-band case and n-band case.
//  Generally those loops which reference constants.
//
#define COMPUTE_GENERAL_1S_1D_W_BAND(src1_datatype, dest_datatype, \
                                     one_band_case, other_two_bands_case, \
                                     band_loop_case) \
    {  \
        const unsigned int num_bands = ci.destNumBands; \
        const unsigned int xsize     = ci.xsize;        \
        \
        if(ci.isStorageType(XIL_PIXEL_SEQUENTIAL)) { \
            const unsigned int src1_sstride  = ci.src1ScanlineStride; \
            const unsigned int dest_sstride  = ci.destScanlineStride; \
            \
            const unsigned int src1_pstride  = ci.src1PixelStride; \
            const unsigned int dest_pstride  = ci.destPixelStride; \
            \
            src1_datatype*     src1_scanline = (src1_datatype*)ci.src1Data; \
            dest_datatype*     dest_scanline = (dest_datatype*)ci.destData; \
            \
            if(num_bands == 1) {\
                if(src1_pstride == 1 && \
                   dest_pstride == 1) {\
                    for(unsigned int y=ci.ysize; y!=0; y--) {\
                        src1_datatype* src1 = src1_scanline;\
                        dest_datatype* dest = dest_scanline;\
                        \
                        for(unsigned int x=xsize; x!=0; x--) {\
                            {one_band_case;} \
                            src1++;\
                            dest++;\
                        }\
                        \
                        src1_scanline += src1_sstride;\
                        dest_scanline += dest_sstride;\
                    } \
                } else { \
                    for(unsigned int y=ci.ysize; y!=0; y--) {\
                        src1_datatype* src1 = src1_scanline;\
                        dest_datatype* dest = dest_scanline;\
                        \
                        for(unsigned int x=xsize; x!=0; x--) {\
                            {one_band_case;} \
                            src1 += src1_pstride;\
                            dest += dest_pstride;\
                        }\
                        \
                        src1_scanline += src1_sstride;\
                        dest_scanline += dest_sstride;\
                    }\
                } \
            } else if(num_bands == 3) {\
                if(src1_pstride == 3 && \
                   dest_pstride == 3) { \
                    for(unsigned int y=ci.ysize; y!=0; y--) {\
                        src1_datatype* src1 = src1_scanline;\
                        dest_datatype* dest = dest_scanline;\
                        \
                        for(unsigned int x=xsize; x!=0; x--) {\
                            {one_band_case;} \
                            {other_two_bands_case;} \
                            \
                            src1 += 3;\
                            dest += 3;\
                        }\
                        \
                        src1_scanline += src1_sstride;\
                        dest_scanline += dest_sstride;\
                    }\
                } else { \
                    for(unsigned int y=ci.ysize; y!=0; y--) {\
                        src1_datatype* src1 = src1_scanline;\
                        dest_datatype* dest = dest_scanline;\
                        \
                        for(unsigned int x=xsize; x!=0; x--) {\
                            {one_band_case;} \
                            {other_two_bands_case;} \
                            \
                            src1 += src1_pstride;\
                            dest += dest_pstride;\
                        }\
                        \
                        src1_scanline += src1_sstride;\
                        dest_scanline += dest_sstride;\
                    }\
                } \
            } else {\
                for(unsigned int y=ci.ysize; y!=0; y--) {\
                    src1_datatype* src1_pixel = src1_scanline;\
                    dest_datatype* dest_pixel = dest_scanline;\
                    \
                    for(unsigned int x=xsize; x!=0; x--) {\
                        src1_datatype* src1 = src1_pixel;\
                        dest_datatype* dest = dest_pixel;\
                        \
                        for(unsigned int band=0; band<num_bands; band++) {\
                             {band_loop_case;} \
                             \
                            src1++;\
                            dest++;\
                        }\
                        \
                        src1_pixel += src1_pstride;\
                        dest_pixel += dest_pstride;\
                    }\
                    \
                    src1_scanline += src1_sstride;\
                    dest_scanline += dest_sstride;\
                }\
            }\
        } else {\
            for(unsigned int band=0; band<num_bands; band++) { \
                const unsigned int src1_pstride = ci.getSrc1PixelStride(band);\
                const unsigned int dest_pstride = ci.getDestPixelStride(band);\
                \
                const unsigned int src1_sstride = ci.getSrc1ScanlineStride(band);\
                const unsigned int dest_sstride = ci.getDestScanlineStride(band);\
                \
                src1_datatype*     src1_scanline = (src1_datatype*)ci.getSrc1Data(band);\
                dest_datatype*     dest_scanline = (dest_datatype*)ci.getDestData(band);\
                \
                for(unsigned int y=ci.ysize; y!=0; y--) { \
                    src1_datatype* src1 = src1_scanline;\
                    dest_datatype* dest = dest_scanline;\
                    \
                    for(unsigned int x=xsize; x!=0; x--) { \
                        {band_loop_case;} \
                        \
                        src1 += src1_pstride;\
                        dest += dest_pstride;\
                    }\
                    \
                    src1_scanline += src1_sstride;\
                    dest_scanline += dest_sstride;\
                }\
            }\
        }\
    }

//
//  USING THIS MACRO REQUIRES a const variable 'nbands' to already be
//  available in the scope of its use and set to the number of bands.
//
#define COMPUTE_GENERAL_1S_W_BAND(src1_datatype, \
                                  one_band_case, other_two_bands_case, \
                                  band_loop_case) \
    {  \
        const unsigned int xsize = ci.xsize;        \
        \
        if(ci.isStorageType(XIL_PIXEL_SEQUENTIAL)) { \
            const unsigned int src1_sstride  = ci.src1ScanlineStride; \
            const unsigned int src1_pstride  = ci.src1PixelStride; \
            src1_datatype*     src1_scanline = (src1_datatype*)ci.src1Data; \
            \
            if(nbands == 1) {\
                if(src1_pstride == 1) {\
                    for(unsigned int y=ci.ysize; y!=0; y--) {\
                        src1_datatype* src1 = src1_scanline;\
                        \
                        for(unsigned int x=xsize; x!=0; x--) {\
                            {one_band_case;} \
                            src1++;\
                        }\
                        \
                        src1_scanline += src1_sstride;\
                    } \
                } else { \
                    for(unsigned int y=ci.ysize; y!=0; y--) {\
                        src1_datatype* src1 = src1_scanline;\
                        \
                        for(unsigned int x=xsize; x!=0; x--) {\
                            {one_band_case;} \
                            src1 += src1_pstride;\
                        }\
                        \
                        src1_scanline += src1_sstride;\
                    }\
                } \
            } else if(nbands == 3) {\
                if(src1_pstride == 3) { \
                    for(unsigned int y=ci.ysize; y!=0; y--) {\
                        src1_datatype* src1 = src1_scanline;\
                        \
                        for(unsigned int x=xsize; x!=0; x--) {\
                            {one_band_case;} \
                            {other_two_bands_case;} \
                            \
                            src1 += 3;\
                        }\
                        \
                        src1_scanline += src1_sstride;\
                    }\
                } else { \
                    for(unsigned int y=ci.ysize; y!=0; y--) {\
                        src1_datatype* src1 = src1_scanline;\
                        \
                        for(unsigned int x=xsize; x!=0; x--) {\
                            {one_band_case;} \
                            {other_two_bands_case;} \
                            \
                            src1 += src1_pstride;\
                        }\
                        \
                        src1_scanline += src1_sstride;\
                    }\
                } \
            } else {\
                for(unsigned int y=ci.ysize; y!=0; y--) {\
                    src1_datatype* src1_pixel = src1_scanline;\
                    \
                    for(unsigned int x=xsize; x!=0; x--) {\
                        src1_datatype* src1 = src1_pixel;\
                        \
                        for(unsigned int band=0; band<nbands; band++) {\
                             {band_loop_case;} \
                             \
                            src1++;\
                        }\
                        \
                        src1_pixel += src1_pstride;\
                    }\
                    \
                    src1_scanline += src1_sstride;\
                }\
            }\
        } else {\
            for(unsigned int band=0; band<nbands; band++) { \
                const unsigned int src1_pstride = ci.getSrc1PixelStride(band);\
                const unsigned int src1_sstride = ci.getSrc1ScanlineStride(band);\
                src1_datatype*     src1_scanline = (src1_datatype*)ci.getSrc1Data(band);\
                \
                for(unsigned int y=ci.ysize; y!=0; y--) { \
                    src1_datatype* src1 = src1_scanline;\
                    \
                    for(unsigned int x=xsize; x!=0; x--) { \
                        {band_loop_case;} \
                        \
                        src1 += src1_pstride;\
                    }\
                    \
                    src1_scanline += src1_sstride;\
                }\
            }\
        }\
    }

#endif // _COMPUTE_INFO_HH
