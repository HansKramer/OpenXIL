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
//  File:	Cast.cc
//  Project:	XIL
//  Revision:	1.1
//  Last Mod:	16:19:46, 10/11/95
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
//  MT-level:  Safe
//	
//------------------------------------------------------------------------
//	COPYRIGHT
//------------------------------------------------------------------------
#pragma ident	"@(#)Cast.cc	1.1\t95/10/11  "

#include "XilDeviceManagerComputeFLOAT.hh"
#include "ComputeInfo.hh"
#include "XiliUtils.hh"

XilStatus
XilDeviceManagerComputeFLOAT::CastTo1(XilOp*       op,
                                      unsigned int op_count,
                                      XilRoi*      roi,
                                      XilBoxList*  bl)
{
    ComputeInfoGENERAL ci(op, op_count, roi, bl);

    if(ci.isOK() == FALSE) {
        return ci.returnValue;
    }

    unsigned int nbands = ci.destNumBands;

    while(ci.hasMoreInfo()) {
	if((ci.src1Storage.isType(XIL_PIXEL_SEQUENTIAL)) &&
           (ci.destStorage.isType(XIL_BAND_SEQUENTIAL))) {
	    Xil_float32*   src1_scanline        = (Xil_float32*)ci.src1Data; 
            unsigned int   src1_pixel_stride    = ci.src1PixelStride;
            unsigned int   src1_scanline_stride = ci.src1ScanlineStride;

	    Xil_unsigned8* dest_scanline = (Xil_unsigned8*) ci.destData;
            unsigned int   dest_scanline_stride = ci.destScanlineStride;
            unsigned int   dest_offset   = ci.destOffset;

	    for(unsigned int height = 0; height < ci.ysize; height++) {
		Xil_float32* src1 = src1_scanline;

		for(unsigned int width = 0; width < ci.xsize; width++) {

		    for(int band = 0; band < nbands; band++) {
			Xil_unsigned8* dest = dest_scanline + (band * ci.destBandStride);

			if(((int)*(src1 + band)) & 0x1) {
			    XIL_BMAP_SET(dest, dest_offset + width);
			} else {
			    XIL_BMAP_CLR(dest, dest_offset + width);
			}
		    }

		    src1 += src1_pixel_stride;
		}

		src1_scanline += src1_scanline_stride;
		dest_scanline += dest_scanline_stride;
	    }
	} else {
	    for(int band = 0; band < nbands; band++) {
		Xil_float32*   src1_scanline = (Xil_float32* )
                    ci.getSrc1Data(band); 
		unsigned int src1_scanline_stride =
                    ci.getSrc1ScanlineStride(band);
		unsigned int src1_pixel_stride    = ci.getSrc1PixelStride(band);

		Xil_unsigned8* dest_scanline        = (Xil_unsigned8*)
                    ci.getDestData(band);
		unsigned int   dest_scanline_stride =
                    ci.getDestScanlineStride(band);
		unsigned int   dest_offset          = ci.getDestOffset(band);
		
		for(unsigned int height = 0; height < ci.ysize; height++) {
		    Xil_float32 *src1 = src1_scanline;

		    for(unsigned int width = 0; width < ci.xsize; width++) {
			if(((int)*src1) & 0x1) {
			    XIL_BMAP_SET(dest_scanline, dest_offset + width);
			} else {
			    XIL_BMAP_CLR(dest_scanline, dest_offset + width);
			}

			src1 += src1_pixel_stride;
		    }

		    src1_scanline += src1_scanline_stride;
		    dest_scanline += dest_scanline_stride;
		}
	    }
	}
    }
    
    return XIL_SUCCESS;
}


XilStatus
XilDeviceManagerComputeFLOAT::CastTo8(XilOp*       op,
                                      unsigned int op_count,
                                      XilRoi*      roi,
                                      XilBoxList*  bl)
{
    ComputeInfoGENERAL ci(op, op_count, roi, bl);

    if(ci.isOK() == FALSE) {
        return ci.returnValue;
    }

    while(ci.hasMoreInfo()) {
        COMPUTE_GENERAL_1S_1D(Xil_float32, Xil_unsigned8,
                              
                              *dest = (Xil_unsigned8)*src1,

                              *(dest+1) = (Xil_unsigned8)*(src1+1);
                              *(dest+2) = (Xil_unsigned8)*(src1+2)
            );
    }

    return ci.returnValue;
}


XilStatus
XilDeviceManagerComputeFLOAT::CastTo16(XilOp*       op,
                                       unsigned int  op_count,
                                       XilRoi*      roi,
                                       XilBoxList*  bl)
{
    ComputeInfoGENERAL  ci(op, op_count, roi, bl);

    if(ci.isOK() == FALSE) {
        return ci.returnValue;
    }

    while(ci.hasMoreInfo()) {
        COMPUTE_GENERAL_1S_1D(Xil_float32, Xil_signed16,
                              
                              *dest = (Xil_signed16)*src1,

                              *(dest+1) = (Xil_signed16)*(src1+1);
                              *(dest+2) = (Xil_signed16)*(src1+2)
            );
    }

    return ci.returnValue;
}


