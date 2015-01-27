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
//  File:	ComputeInfoBIT.cc
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
//  MT-level:  <??????>
//	
//------------------------------------------------------------------------
//	COPYRIGHT
//------------------------------------------------------------------------
#pragma ident	"@(#)ComputeInfoBIT.cc	1.7\t00/03/10  "

#include "ComputeInfo.hh"

ComputeInfoBIT:: ComputeInfoBIT(XilOp*       init_op,
                                  unsigned int init_op_count,
                                  XilRoi*      init_roi,
                                  XilBoxList*  init_bl) :
    ComputeInfo(init_op, init_op_count, init_roi, init_bl)
{
}

Xil_boolean
ComputeInfoBIT::isOK()
{
    if(this == NULL) { 
        return FALSE;
    } else {
        return isOKFlag;
    }
}

XilStatus
ComputeInfoBIT::updateForNewRect()
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
        src3Data       = (Xil_unsigned8*)src3BaseData +
            (y*src3ScanlineStride) +
            (src3Storage.getOffset() + x) / XIL_BIT_ALIGNMENT;
        src3BandStride = src3Storage.getBandStride();
        src3Offset     = (src3Storage.getOffset() + x) % XIL_BIT_ALIGNMENT;
    }

    if(src2) {
        src2Data       = (Xil_unsigned8*)src2BaseData +
            (y*src2ScanlineStride) + 
            (src2Storage.getOffset() + x) / XIL_BIT_ALIGNMENT;
        src2BandStride = src2Storage.getBandStride();
        src2Offset     = (src2Storage.getOffset() + x) % XIL_BIT_ALIGNMENT;
    }

    if(src1) {
        src1Data       = (Xil_unsigned8*)src1BaseData +
            (y*src1ScanlineStride)  +
            (src1Storage.getOffset() + x) / XIL_BIT_ALIGNMENT;
        src1BandStride = src1Storage.getBandStride();
        src1Offset     = (src1Storage.getOffset() + x) % XIL_BIT_ALIGNMENT;
    }

    if(dest) {
        destData       = (Xil_unsigned8*)destBaseData +
            (y*destScanlineStride) + 
            (destStorage.getOffset() + x) / XIL_BIT_ALIGNMENT;
        destBandStride = destStorage.getBandStride();
        destOffset     = (destStorage.getOffset() + x) % XIL_BIT_ALIGNMENT;
    }

    return XIL_SUCCESS;
}

//
//  General Data Access
//
Xil_unsigned8*
ComputeInfoBIT::getSrc1Data(unsigned int band_num)
{
    return (Xil_unsigned8*)src1Storage.getDataPtr(band_num) +
        y*src1Storage.getScanlineStride(band_num) +
        (src1Storage.getOffset(band_num) + x) / XIL_BIT_ALIGNMENT;
;
}

Xil_unsigned8*
ComputeInfoBIT::getSrc2Data(unsigned int band_num)
{
    return (Xil_unsigned8*)src2Storage.getDataPtr(band_num) +
        y*src2Storage.getScanlineStride(band_num) +
        (src2Storage.getOffset(band_num) + x) / XIL_BIT_ALIGNMENT;
}

Xil_unsigned8*
ComputeInfoBIT::getSrc3Data(unsigned int band_num)
{
    return (Xil_unsigned8*)src3Storage.getDataPtr(band_num) +
        y*src3Storage.getScanlineStride(band_num) +
        (src3Storage.getOffset(band_num) + x) / XIL_BIT_ALIGNMENT;
}

Xil_unsigned8*
ComputeInfoBIT::getDestData(unsigned int band_num)
{
    return (Xil_unsigned8*)destStorage.getDataPtr(band_num) +
        y*destStorage.getScanlineStride(band_num) +
        (destStorage.getOffset(band_num) + x) / XIL_BIT_ALIGNMENT;
}


unsigned int
ComputeInfoBIT::getSrc1ScanlineStride(unsigned int band_num)
{
    return src1Storage.getScanlineStride(band_num);
}

unsigned int
ComputeInfoBIT::getSrc2ScanlineStride(unsigned int band_num)
{
    return src2Storage.getScanlineStride(band_num);
}

unsigned int
ComputeInfoBIT::getSrc3ScanlineStride(unsigned int band_num)
{
    return src3Storage.getScanlineStride(band_num);
}

unsigned int
ComputeInfoBIT::getDestScanlineStride(unsigned int band_num)
{
    return destStorage.getScanlineStride(band_num);
}



unsigned int
ComputeInfoBIT::getSrc1Offset(unsigned int band_num)
{
    return (src1Storage.getOffset(band_num) + x) % XIL_BIT_ALIGNMENT;
}

unsigned int
ComputeInfoBIT::getSrc2Offset(unsigned int band_num)
{
    return (src2Storage.getOffset(band_num) + x) % XIL_BIT_ALIGNMENT;
}

unsigned int
ComputeInfoBIT::getSrc3Offset(unsigned int band_num)
{
    return (src3Storage.getOffset(band_num) + x) % XIL_BIT_ALIGNMENT;
}

unsigned int
ComputeInfoBIT::getDestOffset(unsigned int band_num)
{
    return (destStorage.getOffset(band_num) + x) % XIL_BIT_ALIGNMENT;
}

