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
//  File:	ComputeInfoBYTE.cc
//  Project:	XIL
//  Revision:	1.7
//  Last Mod:	10:13:30, 03/10/00
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
#pragma ident	"@(#)ComputeInfoBYTE.cc	1.7\t00/03/10  "

#include "ComputeInfo.hh"

ComputeInfoBYTE:: ComputeInfoBYTE(XilOp*       init_op,
                                  unsigned int init_op_count,
                                  XilRoi*      init_roi,
                                  XilBoxList*  init_bl) :
    ComputeInfo(init_op, init_op_count, init_roi, init_bl)
{
}

Xil_boolean
ComputeInfoBYTE::isOK()
{
    if(this == NULL) { 
        return FALSE;
    } else {
        return isOKFlag;
    }
}

XilStatus
ComputeInfoBYTE::updateForNewRect()
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
        src3Data  = (Xil_unsigned8*)
            src3BaseData + (y*src3ScanlineStride) + (x*src3PixelStride);
    }

    if(src2) {
        src2Data  = (Xil_unsigned8*)
            src2BaseData + (y*src2ScanlineStride) + (x*src2PixelStride);
    }        

    if(src1) {
        src1Data  = (Xil_unsigned8*)
            src1BaseData + (y*src1ScanlineStride) + (x*src1PixelStride);
    }        

    if(dest) {
        destData  = (Xil_unsigned8*)
            destBaseData + (y*destScanlineStride) + (x*destPixelStride);
    }

    return XIL_SUCCESS;
}

//
//  General Data Access
//
Xil_unsigned8*
ComputeInfoBYTE::getSrc1Data(unsigned int band_num)
{
    return (Xil_unsigned8*)src1Storage.getDataPtr(band_num) +
        (y*src1ScanlineStride) + (x*src1PixelStride);
}

Xil_unsigned8*
ComputeInfoBYTE::getSrc2Data(unsigned int band_num)
{
    return (Xil_unsigned8*)src2Storage.getDataPtr(band_num) +
        (y*src2ScanlineStride) + (x*src2PixelStride);
}

Xil_unsigned8*
ComputeInfoBYTE::getSrc3Data(unsigned int band_num)
{
    return (Xil_unsigned8*)src3Storage.getDataPtr(band_num) +
        (y*src3ScanlineStride) + (x*src3PixelStride);
}

Xil_unsigned8*
ComputeInfoBYTE::getDestData(unsigned int band_num)
{
    return (Xil_unsigned8*)destStorage.getDataPtr(band_num) +
        (y*destScanlineStride) + (x*destPixelStride);
}


unsigned int
ComputeInfoBYTE::getSrc1ScanlineStride(unsigned int band_num)
{
    return src1Storage.getScanlineStride(band_num);
}

unsigned int
ComputeInfoBYTE::getSrc2ScanlineStride(unsigned int band_num)
{
    return src2Storage.getScanlineStride(band_num);
}

unsigned int
ComputeInfoBYTE::getSrc3ScanlineStride(unsigned int band_num)
{
    return src3Storage.getScanlineStride(band_num);
}

unsigned int
ComputeInfoBYTE::getDestScanlineStride(unsigned int band_num)
{
    return destStorage.getScanlineStride(band_num);
}



unsigned int
ComputeInfoBYTE::getSrc1PixelStride(unsigned int band_num)
{
    return src1Storage.getPixelStride(band_num);
}


unsigned int
ComputeInfoBYTE::getSrc2PixelStride(unsigned int band_num)
{
    return src2Storage.getPixelStride(band_num);
}

unsigned int
ComputeInfoBYTE::getSrc3PixelStride(unsigned int band_num)
{
    return src3Storage.getPixelStride(band_num);
}

unsigned int
ComputeInfoBYTE::getDestPixelStride(unsigned int band_num)
{
    return destStorage.getPixelStride(band_num);
}


