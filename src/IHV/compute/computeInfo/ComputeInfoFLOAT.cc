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
//  File:	ComputeInfoFLOAT.cc
//  Project:	XIL
//  Revision:	1.6
//  Last Mod:	10:13:34, 03/10/00
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
#pragma ident	"@(#)ComputeInfoFLOAT.cc	1.6\t00/03/10  "

#include "ComputeInfo.hh"

ComputeInfoFLOAT:: ComputeInfoFLOAT(XilOp*       init_op,
                                    unsigned int init_op_count,
                                    XilRoi*      init_roi,
                                    XilBoxList*  init_bl) :
    ComputeInfo(init_op, init_op_count, init_roi, init_bl)
{
}

Xil_boolean
ComputeInfoFLOAT::isOK()
{
    if(this == NULL) { 
        return FALSE;
    } else {
        return isOKFlag;
    }
}

XilStatus
ComputeInfoFLOAT::updateForNewRect()
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
        src3Data  = (Xil_float32*)
            src3BaseData + (y*src3ScanlineStride) + (x*src3PixelStride);
    }

    if(src2) {
        src2Data  = (Xil_float32*)
            src2BaseData + (y*src2ScanlineStride) + (x*src2PixelStride);
    }

    if(src1) {
        src1Data  = (Xil_float32*)
            src1BaseData + (y*src1ScanlineStride) + (x*src1PixelStride);
    }

    if(dest) {
        destData  = (Xil_float32*)
            destBaseData + (y*destScanlineStride) + (x*destPixelStride);
    }

    return XIL_SUCCESS;
}


//
//  General Data Access
//
Xil_float32*
ComputeInfoFLOAT::getSrc1Data(unsigned int band_num)
{
    return (Xil_float32*)src1Storage.getDataPtr(band_num) +
        (y*src1ScanlineStride) + (x*src1PixelStride);
}

Xil_float32*
ComputeInfoFLOAT::getSrc2Data(unsigned int band_num)
{
    return (Xil_float32*)src2Storage.getDataPtr(band_num) +
        (y*src2ScanlineStride) + (x*src2PixelStride);
}

Xil_float32*
ComputeInfoFLOAT::getSrc3Data(unsigned int band_num)
{
    return (Xil_float32*)src3Storage.getDataPtr(band_num) +
        (y*src3ScanlineStride) + (x*src3PixelStride);
}

Xil_float32*
ComputeInfoFLOAT::getDestData(unsigned int band_num)
{
    return (Xil_float32*)destStorage.getDataPtr(band_num) +
        (y*destScanlineStride) + (x*destPixelStride);
}


unsigned int
ComputeInfoFLOAT::getSrc1ScanlineStride(unsigned int band_num)
{
    return src1Storage.getScanlineStride(band_num);
}

unsigned int
ComputeInfoFLOAT::getSrc2ScanlineStride(unsigned int band_num)
{
    return src2Storage.getScanlineStride(band_num);
}

unsigned int
ComputeInfoFLOAT::getSrc3ScanlineStride(unsigned int band_num)
{
    return src3Storage.getScanlineStride(band_num);
}

unsigned int
ComputeInfoFLOAT::getDestScanlineStride(unsigned int band_num)
{
    return destStorage.getScanlineStride(band_num);
}



unsigned int
ComputeInfoFLOAT::getSrc1PixelStride(unsigned int band_num)
{
    return src1Storage.getPixelStride(band_num);
}


unsigned int
ComputeInfoFLOAT::getSrc2PixelStride(unsigned int band_num)
{
    return src2Storage.getPixelStride(band_num);
}

unsigned int
ComputeInfoFLOAT::getSrc3PixelStride(unsigned int band_num)
{
    return src3Storage.getPixelStride(band_num);
}

unsigned int
ComputeInfoFLOAT::getDestPixelStride(unsigned int band_num)
{
    return destStorage.getPixelStride(band_num);
}


