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
//  File:	Blend.cc
//  Project:	XIL
//  Revision:	1.5
//  Last Mod:	10:09:49, 03/10/00
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
#pragma ident	"@(#)Blend.cc	1.5\t00/03/10  "

#include "XilDeviceManagerComputeBIT.hh"
#include "ComputeInfo.hh"

#define HALF_XIL_MAXBYTE  (Xil_unsigned8)(XIL_MAXBYTE / 2)
#define HALF_XIL_MAXSHORT (Xil_signed16)(0)

XilStatus
XilDeviceManagerComputeBIT::Blenda1(XilOp*       op,
				    unsigned     op_count,
				    XilRoi*      roi,
				    XilBoxList*  bl)

{
    ComputeInfoBIT  ci(op, op_count, roi, bl);

    if(ci.isOK() == FALSE) {
        return ci.returnValue;
    }

    unsigned int nbands = ci.destNumBands;

    while(ci.hasMoreInfo()) {
        //
        //  src3 (Alpha) is always 1 banded so its information doesn't change
        //  per-band. 
        //
        unsigned int   srcA_scanline_stride = ci.src3ScanlineStride;
        unsigned int   srcA_offset          = ci.src3Offset;
        Xil_unsigned8* srcA_data            = ci.src3Data;
            
	for(int band=0; band<nbands; band++) { 
	    unsigned int   src1_scanline_stride = ci.getSrc1ScanlineStride(band);
	    unsigned int   src2_scanline_stride = ci.getSrc2ScanlineStride(band);
	    unsigned int   dest_scanline_stride = ci.getDestScanlineStride(band);

	    Xil_unsigned8* src1_scanline        = ci.getSrc1Data(band);
	    Xil_unsigned8* src2_scanline        = ci.getSrc2Data(band);
	    Xil_unsigned8* dest_scanline        = ci.getDestData(band);

	    unsigned int   src1_offset          = ci.getSrc1Offset(band);
	    unsigned int   src2_offset          = ci.getSrc2Offset(band);
	    unsigned int   dest_offset          = ci.getDestOffset(band);

            Xil_unsigned8* srcA_scanline        = srcA_data;

	    for(int y=ci.ysize; y>0; y--) { 
		for(int x=0; x<ci.xsize; x++) {
		    if(XIL_BMAP_TST(srcA_scanline, srcA_offset + x)) {
			if(XIL_BMAP_TST(src2_scanline, src2_offset + x)) {
			    XIL_BMAP_SET(dest_scanline, dest_offset + x);
			} else {
			    XIL_BMAP_CLR(dest_scanline, dest_offset + x);
			}
		    } else {
			if(XIL_BMAP_TST(src1_scanline, src1_offset + x)) {
			    XIL_BMAP_SET(dest_scanline, dest_offset + x);
			} else {
			    XIL_BMAP_CLR(dest_scanline, dest_offset + x);
			}
		    }
		}

		src1_scanline += src1_scanline_stride;
		src2_scanline += src2_scanline_stride;
		srcA_scanline += srcA_scanline_stride;
		dest_scanline += dest_scanline_stride;
	    }
	}
    }

    return ci.returnValue;
}


XilStatus
XilDeviceManagerComputeBIT::Blenda8(XilOp*       op,
				    unsigned     op_count,
				    XilRoi*      roi,
				    XilBoxList*  bl)

{
    ComputeInfoGENERAL  ci(op, op_count, roi, bl);

    if(ci.isOK() == FALSE) {
        return ci.returnValue;
    }

    unsigned int nbands = ci.destNumBands;

    while(ci.hasMoreInfo()) {
        //
        //  src3 (Alpha) is always 1 banded so its information doesn't change
        //  per-band. 
        //
        int            srcA_scanline_stride = ci.src3ScanlineStride;
        unsigned int   srcA_pixel_stride    = ci.src3PixelStride;
        Xil_unsigned8* srcA_data            = (Xil_unsigned8*)ci.src3Data;
            
	for(int band=0; band<nbands; band++) { 
	    unsigned int   src1_scanline_stride =
                ci.getSrc1ScanlineStride(band); 
	    unsigned int   src2_scanline_stride =
                ci.getSrc2ScanlineStride(band);
	    unsigned int   dest_scanline_stride =
                ci.getDestScanlineStride(band);

	    Xil_unsigned8* src1_scanline        =
                (Xil_unsigned8*)ci.getSrc1Data(band);
	    Xil_unsigned8* src2_scanline        =
                (Xil_unsigned8*)ci.getSrc2Data(band);
	    Xil_unsigned8* dest_scanline        =
                (Xil_unsigned8*)ci.getDestData(band);

	    unsigned int   src1_offset          = ci.getSrc1Offset(band);
	    unsigned int   src2_offset          = ci.getSrc2Offset(band);
	    unsigned int   dest_offset          = ci.getDestOffset(band);

            Xil_unsigned8* srcA_scanline        = srcA_data;

	    for(int y=ci.ysize; y>0; y--) {
		Xil_unsigned8* srcA = srcA_scanline;

		for(int x=0; x<ci.xsize; x++) {
		    if(*srcA > HALF_XIL_MAXBYTE) {
			if(XIL_BMAP_TST(src2_scanline, src2_offset + x)) {
			    XIL_BMAP_SET(dest_scanline, dest_offset + x);
			} else {
			    XIL_BMAP_CLR(dest_scanline, dest_offset + x);
			}
		    } else {
			if(XIL_BMAP_TST(src1_scanline, src1_offset + x)) {
			    XIL_BMAP_SET(dest_scanline, dest_offset + x);
			} else {
			    XIL_BMAP_CLR(dest_scanline, dest_offset + x);
			}
		    }

		    srcA += srcA_pixel_stride;
		}

		src1_scanline += src1_scanline_stride;
		src2_scanline += src2_scanline_stride;
		srcA_scanline += srcA_scanline_stride;
		dest_scanline += dest_scanline_stride;
	    }
	}
    }

    return ci.returnValue;
}

XilStatus
XilDeviceManagerComputeBIT::Blenda16(XilOp*       op,
				     unsigned     op_count,
				     XilRoi*      roi,
				     XilBoxList*  bl)

{
    ComputeInfoGENERAL  ci(op, op_count, roi, bl);

    if(ci.isOK() == FALSE) {
        return ci.returnValue;
    }

    unsigned int nbands = ci.destNumBands;

    while(ci.hasMoreInfo()) {
        //
        //  src3 (Alpha) is always 1 banded so its information doesn't change
        //  per-band. 
        //
        int           srcA_scanline_stride = ci.src3ScanlineStride;
        unsigned int  srcA_pixel_stride    = ci.src3PixelStride;
        Xil_signed16* srcA_data            = (Xil_signed16*)ci.src3Data;
            
        for(int band=0; band<nbands; band++) { 
            unsigned int   src1_scanline_stride =
                ci.getSrc1ScanlineStride(band);
            unsigned int   src2_scanline_stride =
                ci.getSrc2ScanlineStride(band); 
            unsigned int   dest_scanline_stride =
                ci.getDestScanlineStride(band); 

	    Xil_unsigned8* src1_scanline        =
                (Xil_unsigned8*)ci.getSrc1Data(band);
	    Xil_unsigned8* src2_scanline        =
                (Xil_unsigned8*)ci.getSrc2Data(band);
	    Xil_unsigned8* dest_scanline        =
                (Xil_unsigned8*)ci.getDestData(band);

	    unsigned int   src1_offset          = ci.getSrc1Offset(band);
	    unsigned int   src2_offset          = ci.getSrc2Offset(band);
	    unsigned int   dest_offset          = ci.getDestOffset(band);

            Xil_signed16*  srcA_scanline        = srcA_data;

	    for(int y=ci.ysize; y>0; y--) {
		Xil_signed16* srcA = srcA_scanline;

		for(int x=0; x<ci.xsize; x++) {
		    if(*srcA >= HALF_XIL_MAXSHORT) {
			if(XIL_BMAP_TST(src2_scanline, src2_offset + x)) {
			    XIL_BMAP_SET(dest_scanline, dest_offset + x);
			} else {
			    XIL_BMAP_CLR(dest_scanline, dest_offset + x);
			}
		    } else {
			if(XIL_BMAP_TST(src1_scanline, src1_offset + x)) {
			    XIL_BMAP_SET(dest_scanline, dest_offset + x);
			} else {
			    XIL_BMAP_CLR(dest_scanline, dest_offset + x);
			}
		    }
		    srcA += srcA_pixel_stride;
		}

		src1_scanline += src1_scanline_stride;
		src2_scanline += src2_scanline_stride;
		srcA_scanline += srcA_scanline_stride;
		dest_scanline += dest_scanline_stride;
	    }
	}
    }

    return ci.returnValue;
}

XilStatus
XilDeviceManagerComputeBIT::Blendaf32(XilOp*       op,
				      unsigned     op_count,
				      XilRoi*      roi,
				      XilBoxList*  bl)

{
    ComputeInfoGENERAL  ci(op, op_count, roi, bl);

    if(ci.isOK() == FALSE) {
        return ci.returnValue;
    }

    unsigned int nbands = ci.destNumBands;

    while(ci.hasMoreInfo()) {
        //
        //  src3 (Alpha) is always 1 banded so its information doesn't change
        //  per-band. 
        //
        int          srcA_scanline_stride = ci.src3ScanlineStride;
        unsigned int srcA_pixel_stride    = ci.src3PixelStride;
        Xil_float32* srcA_data            = (Xil_float32*)ci.src3Data;
            
        for(int band=0; band<nbands; band++) { 
            unsigned int   src1_scanline_stride =
                ci.getSrc1ScanlineStride(band);
            unsigned int   src2_scanline_stride =
                ci.getSrc2ScanlineStride(band); 
            unsigned int   dest_scanline_stride =
                ci.getDestScanlineStride(band); 

	    Xil_unsigned8* src1_scanline        =
                (Xil_unsigned8*)ci.getSrc1Data(band);
	    Xil_unsigned8* src2_scanline        =
                (Xil_unsigned8*)ci.getSrc2Data(band);
	    Xil_unsigned8* dest_scanline        =
                (Xil_unsigned8*)ci.getDestData(band);

	    unsigned int   src1_offset          = ci.getSrc1Offset(band);
	    unsigned int   src2_offset          = ci.getSrc2Offset(band);
	    unsigned int   dest_offset          = ci.getDestOffset(band);

            Xil_float32*   srcA_scanline        = srcA_data;

	    for(int y=ci.ysize; y>0; y--) {
		Xil_float32* srcA = srcA_scanline;

		for(int x=0; x<ci.xsize; x++) {
		    if (*srcA >= .5) {
			if (XIL_BMAP_TST(src2_scanline, src2_offset + x)) {
			    XIL_BMAP_SET(dest_scanline, dest_offset + x);
			} else {
			    XIL_BMAP_CLR(dest_scanline, dest_offset + x);
			}
		    } else {
			if (XIL_BMAP_TST(src1_scanline, src1_offset + x)) {
			    XIL_BMAP_SET(dest_scanline, dest_offset + x);
			} else {
			    XIL_BMAP_CLR(dest_scanline, dest_offset + x);
			}
		    }

		    srcA += srcA_pixel_stride;
		}

		src1_scanline += src1_scanline_stride;
		src2_scanline += src2_scanline_stride;
		srcA_scanline += srcA_scanline_stride;
		dest_scanline += dest_scanline_stride;
	    }
	}
    }

    return ci.returnValue;
}

