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
//  File:	ComputeInfoGENERAL.cc
//  Project:	XIL
//  Revision:	1.10
//  Last Mod:	10:13:33, 03/10/00
//
//  Description:
//	
//	Implementation of the ComputeInfoGENERAL class which supports
//	arbitrary datatypes between sources and destination images.
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
#pragma ident	"@(#)ComputeInfoGENERAL.cc	1.10\t00/03/10  "

#include "ComputeInfo.hh"
#include "XiliUtils.hh"

ComputeInfoGENERAL::ComputeInfoGENERAL(XilOp*       init_op,
                                       unsigned int init_op_count,
                                       XilRoi*      init_roi,
                                       XilBoxList*  init_bl) :
    ComputeInfo(init_op, init_op_count, init_roi, init_bl)
{

    if(src3) {
        src3DataType     = src3->getDataType();
        src3DataTypeSize = xili_sizeof(src3DataType);
    }

    if(src2) {
        src2DataType     = src2->getDataType();
        src2DataTypeSize = xili_sizeof(src2DataType);
    }

    if(src1) {
        src1DataType     = src1->getDataType();
        src1DataTypeSize = xili_sizeof(src1DataType);
    }

    if(dest) {
        destDataType     = dest->getDataType();
        destDataTypeSize = xili_sizeof(destDataType);
    }
}

Xil_boolean
ComputeInfoGENERAL::isOK()
{
    if(this == NULL) { 
        return FALSE;
    } else {
        return isOKFlag;
    }
}

XilStatus
ComputeInfoGENERAL::updateForNewRect()
{
    //
    //  Called by the base class to fill in the datatype-specific pointers and
    //    other data from the XilStorage objects.
    //
    //
    //  Set the data pointers to be used by the compute routine, correctly
    //    offset for the x and y position of the image.
    //
    if(src3) {
        src3Offset = src3Storage.getOffset();

        if(src3DataType == XIL_BIT) {
            src3Data   = (char*)src3BaseData +
                (y*src3ScanlineStride)  +
                (src3Offset + x) / XIL_BIT_ALIGNMENT;
            src3BandStride = src3Storage.getBandStride();
            src3Offset     = (src3Offset + x) % XIL_BIT_ALIGNMENT;
        } else {
            src3Data   = (char*)src3BaseData +
                (y*src3ScanlineStride*src3DataTypeSize) +
                (x*src3PixelStride*src3DataTypeSize);
        }
    }

    if(src2) {
        src2Offset = src2Storage.getOffset();

        if(src2DataType == XIL_BIT) {
            src2Data   = (char*)src2BaseData +
                (y*src2ScanlineStride)  +
                (src2Offset + x) / XIL_BIT_ALIGNMENT;
            src2BandStride = src2Storage.getBandStride();
            src2Offset     = (src2Offset + x) % XIL_BIT_ALIGNMENT;
        } else {
            src2Data   = (char*)src2BaseData +
                (y*src2ScanlineStride*src2DataTypeSize) +
                (x*src2PixelStride*src2DataTypeSize);
        }
    }

    if(src1) {
        src1Offset = src1Storage.getOffset();

        if(src1DataType == XIL_BIT) {
            src1Data   = (char*)src1BaseData +
                (y*src1ScanlineStride)  +
                (src1Offset + x) / XIL_BIT_ALIGNMENT;
            src1BandStride = src1Storage.getBandStride();
            src1Offset     = (src1Offset + x) % XIL_BIT_ALIGNMENT;
        } else {
            src1Data   = (char*)src1BaseData +
                (y*src1ScanlineStride*src1DataTypeSize) +
                (x*src1PixelStride*src1DataTypeSize);
        }
    }
    
    if(dest) {
        destOffset = destStorage.getOffset();

        if(destDataType == XIL_BIT) {
            destData   = (char*)destBaseData +
                (y*destScanlineStride)  +
                (destOffset + x) / XIL_BIT_ALIGNMENT;
            destBandStride = destStorage.getBandStride();
            destOffset     = (destOffset + x) % XIL_BIT_ALIGNMENT;
        } else {
            destData   = (char*)destBaseData +
                (y*destScanlineStride*destDataTypeSize) +
                (x*destPixelStride*destDataTypeSize);
        }
    }
    
    return XIL_SUCCESS;
}

//
//  General Data Access
//
void*
ComputeInfoGENERAL::getSrc1Data(unsigned int band_num)
{
    if(src1DataType == XIL_BIT) {
        return (char*)src1Storage.getDataPtr(band_num) +
            (y*src1Storage.getScanlineStride(band_num))  +
            (src1Storage.getOffset(band_num) + x) / XIL_BIT_ALIGNMENT;
    } else {
        return (char*)src1Storage.getDataPtr(band_num) +
            (y*src1Storage.getScanlineStride(band_num)*src1DataTypeSize) +
            (x*src1Storage.getPixelStride(band_num)*src1DataTypeSize);
    }
}

void*
ComputeInfoGENERAL::getSrc2Data(unsigned int band_num)
{
    if(src2DataType == XIL_BIT) {
        return (char*)src2Storage.getDataPtr(band_num) +
            (y*src2Storage.getScanlineStride(band_num))  +
            (src2Storage.getOffset(band_num) + x) / XIL_BIT_ALIGNMENT;
    } else {
        return (char*)src2Storage.getDataPtr(band_num) +
            (y*src2Storage.getScanlineStride(band_num)*src2DataTypeSize) +
            (x*src2Storage.getPixelStride(band_num)*src2DataTypeSize);
    }
}

void*
ComputeInfoGENERAL::getSrc3Data(unsigned int band_num)
{
    if(src3DataType == XIL_BIT) {
        return (char*)src3Storage.getDataPtr(band_num) +
            (y*src3Storage.getScanlineStride(band_num))  +
            (src3Storage.getOffset(band_num) + x) / XIL_BIT_ALIGNMENT;
    } else {
        return (char*)src3Storage.getDataPtr(band_num) +
            (y*src3Storage.getScanlineStride(band_num)*src3DataTypeSize) +
            (x*src3Storage.getPixelStride(band_num)*src3DataTypeSize);
    }
}

void*
ComputeInfoGENERAL::getDestData(unsigned int band_num)
{
    if(destDataType == XIL_BIT) {
        return (char*)destStorage.getDataPtr(band_num) +
            (y*destStorage.getScanlineStride(band_num))  +
            (destStorage.getOffset(band_num) + x) / XIL_BIT_ALIGNMENT;
    } else {
        return (char*)destStorage.getDataPtr(band_num) +
            (y*destStorage.getScanlineStride(band_num)*destDataTypeSize) +
            (x*destStorage.getPixelStride(band_num)*destDataTypeSize);
    }
}

unsigned int
ComputeInfoGENERAL::getSrc1ScanlineStride(unsigned int band_num)
{
    return src1Storage.getScanlineStride(band_num);
}

unsigned int
ComputeInfoGENERAL::getSrc2ScanlineStride(unsigned int band_num)
{
    return src2Storage.getScanlineStride(band_num);
}

unsigned int
ComputeInfoGENERAL::getSrc3ScanlineStride(unsigned int band_num)
{
    return src3Storage.getScanlineStride(band_num);
}

unsigned int
ComputeInfoGENERAL::getDestScanlineStride(unsigned int band_num)
{
    return destStorage.getScanlineStride(band_num);
}



unsigned int
ComputeInfoGENERAL::getSrc1PixelStride(unsigned int band_num)
{
    return src1Storage.getPixelStride(band_num);
}


unsigned int
ComputeInfoGENERAL::getSrc2PixelStride(unsigned int band_num)
{
    return src2Storage.getPixelStride(band_num);
}

unsigned int
ComputeInfoGENERAL::getSrc3PixelStride(unsigned int band_num)
{
    return src3Storage.getPixelStride(band_num);
}

unsigned int
ComputeInfoGENERAL::getDestPixelStride(unsigned int band_num)
{
    return destStorage.getPixelStride(band_num);
}



unsigned int
ComputeInfoGENERAL::getSrc1Offset(unsigned int band_num)
{
    if(src1DataType == XIL_BIT) {
        return (src1Storage.getOffset(band_num) + x) % XIL_BIT_ALIGNMENT;
    } else {
        return src1Storage.getOffset(band_num);
    }
}


unsigned int
ComputeInfoGENERAL::getSrc2Offset(unsigned int band_num)
{
    if(src2DataType == XIL_BIT) {
        return (src2Storage.getOffset(band_num) + x) % XIL_BIT_ALIGNMENT;
    } else {
        return src2Storage.getOffset(band_num);
    }
}

unsigned int
ComputeInfoGENERAL::getSrc3Offset(unsigned int band_num)
{
    if(src3DataType == XIL_BIT) {
        return (src3Storage.getOffset(band_num) + x) % XIL_BIT_ALIGNMENT;
    } else {
        return src3Storage.getOffset(band_num);
    }
}

unsigned int
ComputeInfoGENERAL::getDestOffset(unsigned int band_num)
{
    if(destDataType == XIL_BIT) {
        return (destStorage.getOffset(band_num) + x) % XIL_BIT_ALIGNMENT;
    } else {
        return destStorage.getOffset(band_num);
    }
}
