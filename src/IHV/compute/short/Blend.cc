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
//  Revision:	1.12
//  Last Mod:	10:11:53, 03/10/00
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
//  MT-level:  SAFE
//	
//------------------------------------------------------------------------
//	COPYRIGHT
//------------------------------------------------------------------------
#pragma ident	"@(#)Blend.cc	1.12\t00/03/10  "

#include "XilDeviceManagerComputeSHORT.hh"
#include "ComputeInfo.hh"

XilStatus
XilDeviceManagerComputeSHORT::Blenda1(XilOp*       op,
				      unsigned     op_count,
				      XilRoi*      roi,
				      XilBoxList*  bl)

{
    ComputeInfoGENERAL  ci(op, op_count, roi, bl);

    if(ci.isOK() == FALSE) {
        return ci.returnValue;
    }

    unsigned int nbands = ci.destNumBands;

    //
    //  Loop iterators.
    //
    int          x;
    int          y;
    int          o;

    while(ci.hasMoreInfo()) {
	if((ci.src1Storage.isType(XIL_PIXEL_SEQUENTIAL)) &&
           (ci.src2Storage.isType(XIL_PIXEL_SEQUENTIAL)) &&
           (ci.destStorage.isType(XIL_PIXEL_SEQUENTIAL)))
	{
            //
            //  The Alpha image.
            //
            Xil_unsigned8* srcA_scanline        = (Xil_unsigned8*) ci.src3Data;
            unsigned int   srcA_scanline_stride = ci.src3ScanlineStride;
            unsigned int   srcA_offset          = ci.src3Offset;

            //
            //  The input images.
            //
	    Xil_signed16*  src1_scanline        = (Xil_signed16*) ci.src1Data;
            Xil_signed16*  src2_scanline        = (Xil_signed16*) ci.src2Data;
            Xil_signed16*  dest_scanline        = (Xil_signed16*) ci.destData;

            unsigned int   src1_scanline_stride = ci.src1ScanlineStride;
            unsigned int   src2_scanline_stride = ci.src2ScanlineStride;
            unsigned int   dest_scanline_stride = ci.destScanlineStride;

            unsigned int   src1_pixel_stride    = ci.src1PixelStride;
            unsigned int   src2_pixel_stride    = ci.src2PixelStride;
            unsigned int   dest_pixel_stride    = ci.destPixelStride;

            if(nbands == 1) {
                for(y=ci.ysize; y>0; y--) {
                    Xil_signed16* src1 = src1_scanline;
                    Xil_signed16* src2 = src2_scanline;
                    Xil_signed16* dest = dest_scanline;

                    for(x=0, o=x+srcA_offset; x<ci.xsize; x++, o++) {
                        if(XIL_BMAP_TST(srcA_scanline, o) == 0) {
                            *dest = *src1;
                        } else {
                            *dest = *src2;
                        }

                        src1 += src1_pixel_stride;
                        src2 += src2_pixel_stride;
                        dest += dest_pixel_stride;
                    }

                    src1_scanline += src1_scanline_stride;
                    src2_scanline += src2_scanline_stride;
                    srcA_scanline += srcA_scanline_stride;
                    dest_scanline += dest_scanline_stride;
                }
            } else if(nbands == 3) {
                for(y=ci.ysize; y>0; y--) {
                    Xil_signed16* src1 = src1_scanline;
                    Xil_signed16* src2 = src2_scanline;
                    Xil_signed16* dest = dest_scanline;

                    for(x=0, o=x+srcA_offset; x<ci.xsize; x++, o++) {
                        if(XIL_BMAP_TST(srcA_scanline, o) == 0) {
                            *dest     = *src1;
                            *(dest+1) = *(src1+1);
                            *(dest+2) = *(src1+2);
                        } else {
                            *dest     = *src2;
                            *(dest+1) = *(src2+1);
                            *(dest+2) = *(src2+2);
                        }

                        src1 += src1_pixel_stride;
                        src2 += src2_pixel_stride;
                        dest += dest_pixel_stride;
                    }

                    src1_scanline += src1_scanline_stride;
                    src2_scanline += src2_scanline_stride;
                    srcA_scanline += srcA_scanline_stride;
                    dest_scanline += dest_scanline_stride;
                }
            } else {
                for(y=ci.ysize; y>0; y--) {
                    Xil_signed16* src1_pixel = src1_scanline;
                    Xil_signed16* src2_pixel = src2_scanline;
                    Xil_signed16* dest_pixel = dest_scanline;

                    for(x=0, o=x+srcA_offset; x<ci.xsize; x++, o++) {
                        Xil_signed16* src1 = src1_pixel;
                        Xil_signed16* src2 = src2_pixel;
                        Xil_signed16* dest = dest_pixel;

                        if(XIL_BMAP_TST(srcA_scanline, o) == 0) {
                            for(int band=0; band<nbands; band++) {
                                *dest++ = *src1++;
                            }
                        } else {
                            for(int band=0; band<nbands; band++) {
                                *dest++ = *src2++;
                            }
                        }

                        src1_pixel += src1_pixel_stride;
                        src2_pixel += src2_pixel_stride;
                        dest_pixel += dest_pixel_stride;
                    }

                    src1_scanline += src1_scanline_stride;
                    src2_scanline += src2_scanline_stride;
                    srcA_scanline += srcA_scanline_stride;
                    dest_scanline += dest_scanline_stride;
                }
	    }
	} else {
            //
            //  The Alpha image.
            //
            Xil_unsigned8* srcA_data            = (Xil_unsigned8*) ci.src3Data;
            unsigned int   srcA_scanline_stride = ci.src3ScanlineStride;
            unsigned int   srcA_offset          = ci.src3Offset;

	    for(int band=0; band<nbands; band++) { 
                //
                //  The input images.
                //
                Xil_signed16*  src1_scanline        =
                    (Xil_signed16*)ci.getSrc1Data(band);
                Xil_signed16*  src2_scanline        =
                    (Xil_signed16*)ci.getSrc2Data(band);
                Xil_signed16*  dest_scanline        =
                    (Xil_signed16*)ci.getDestData(band);

                unsigned int   src1_scanline_stride =
                    ci.getSrc1ScanlineStride(band);
                unsigned int   src2_scanline_stride =
                    ci.getSrc2ScanlineStride(band);
                unsigned int   dest_scanline_stride =
                    ci.getDestScanlineStride(band);

                unsigned int   src1_pixel_stride    =
                    ci.getSrc1PixelStride(band); 
                unsigned int   src2_pixel_stride    =
                    ci.getSrc2PixelStride(band); 
                unsigned int   dest_pixel_stride    =
                    ci.getDestPixelStride(band); 

                Xil_unsigned8* srcA_scanline        = srcA_data;

                for(y=ci.ysize; y>0; y--) { 
                    Xil_signed16* src1 = src1_scanline;
                    Xil_signed16* src2 = src2_scanline;
                    Xil_signed16* dest = dest_scanline;

                    for(x=0, o=x+srcA_offset; x<ci.xsize; x++, o++) {
                        if(XIL_BMAP_TST(srcA_scanline, o) == 0) {
                            *dest = *src1;
                        } else {
                            *dest = *src2;
                        }

                        src1 += src1_pixel_stride;
                        src2 += src2_pixel_stride;
                        dest += dest_pixel_stride;
                    }

                    src1_scanline += src1_scanline_stride;
                    src2_scanline += src2_scanline_stride;
                    srcA_scanline += srcA_scanline_stride;
                    dest_scanline += dest_scanline_stride;
                }
	    }
	}
    }

    return ci.returnValue;
}


XilStatus
XilDeviceManagerComputeSHORT::Blenda8(XilOp*       op,
                                      unsigned     op_count,
                                      XilRoi*      roi,
                                      XilBoxList*  bl)

{
    ComputeInfoGENERAL  ci(op, op_count, roi, bl);

    if(ci.isOK() == FALSE) {
        return ci.returnValue;
    }

    unsigned int nbands = ci.destNumBands;

    //
    //  Loop iterators.
    //
    int          x;
    int          y;

    while(ci.hasMoreInfo()) {
	if((ci.src1Storage.isType(XIL_PIXEL_SEQUENTIAL)) &&
           (ci.src2Storage.isType(XIL_PIXEL_SEQUENTIAL)) &&
           (ci.destStorage.isType(XIL_PIXEL_SEQUENTIAL)))
	{
            //
            //  The Alpha image.
            //
            Xil_unsigned8* srcA_scanline        = (Xil_unsigned8*) ci.src3Data;
            unsigned int   srcA_scanline_stride = ci.src3ScanlineStride;
            unsigned int   srcA_pixel_stride    = ci.src3PixelStride;

            //
            //  The input images.
            //
	    Xil_signed16*  src1_scanline        = (Xil_signed16*) ci.src1Data;
            Xil_signed16*  src2_scanline        = (Xil_signed16*) ci.src2Data;
            Xil_signed16*  dest_scanline        = (Xil_signed16*) ci.destData;

            unsigned int   src1_scanline_stride = ci.src1ScanlineStride;
            unsigned int   src2_scanline_stride = ci.src2ScanlineStride;
            unsigned int   dest_scanline_stride = ci.destScanlineStride;

            unsigned int   src1_pixel_stride    = ci.src1PixelStride;
            unsigned int   src2_pixel_stride    = ci.src2PixelStride;
            unsigned int   dest_pixel_stride    = ci.destPixelStride;

            if(nbands == 1) {
                for(y=ci.ysize; y>0; y--) {
                    Xil_signed16*  src1 = src1_scanline;
                    Xil_signed16*  src2 = src2_scanline;
                    Xil_signed16*  dest = dest_scanline;
                    Xil_unsigned8* srcA = srcA_scanline;

                    for(x=ci.xsize; x>0; x--) {
                        float ratio = (float)(*srcA) / (float)XIL_MAXBYTE;
                        float sum   =
                            ((1.0F - ratio) * *src1) +
                            (ratio          * *src2);

                        if(sum < 0.0F) {
                            *dest = (Xil_signed16)(sum);
                        } else {
                            *dest = (Xil_signed16)(sum + 0.5F);
                        }

                        src1 += src1_pixel_stride;
                        src2 += src2_pixel_stride;
                        srcA += srcA_pixel_stride;
                        dest += dest_pixel_stride;
                    }

                    src1_scanline += src1_scanline_stride;
                    src2_scanline += src2_scanline_stride;
                    srcA_scanline += srcA_scanline_stride;
                    dest_scanline += dest_scanline_stride;
                }
            } else if(nbands == 3) {
                for(y=ci.ysize; y>0; y--) {
                    Xil_signed16*  src1 = src1_scanline;
                    Xil_signed16*  src2 = src2_scanline;
                    Xil_signed16*  dest = dest_scanline;
                    Xil_unsigned8* srcA = srcA_scanline;

                    for(x=ci.xsize; x>0; x--) {
                        float ratio = (float)(*srcA) / (float)XIL_MAXBYTE;

                        float sum   =
                            ((1.0F - ratio) * *src1) +
                            (ratio          * *src2);
                        if(sum < 0.0F) {
                            *dest = (Xil_signed16)(sum);
                        } else {
                            *dest = (Xil_signed16)(sum + 0.5F);
                        }

                        sum =
                            ((1.0F - ratio) * *(src1+1)) +
                            (ratio          * *(src2+1));
                        if(sum < 0.0F) {
                            *(dest+1) = (Xil_signed16)(sum);
                        } else {
                            *(dest+1) = (Xil_signed16)(sum + 0.5F);
                        }

                        sum =
                            ((1.0F - ratio) * *(src1+2)) +
                            (ratio          * *(src2+2));
                        if(sum < 0.0F) {
                            *(dest+2) = (Xil_signed16)(sum);
                        } else {
                            *(dest+2) = (Xil_signed16)(sum + 0.5F);
                        }

                        src1 += src1_pixel_stride;
                        src2 += src2_pixel_stride;
                        srcA += srcA_pixel_stride;
                        dest += dest_pixel_stride;
                    }

                    src1_scanline += src1_scanline_stride;
                    src2_scanline += src2_scanline_stride;
                    srcA_scanline += srcA_scanline_stride;
                    dest_scanline += dest_scanline_stride;
                }
            } else {
                for(y=ci.ysize; y>0; y--) {
                    Xil_signed16*  src1_pixel = src1_scanline;
                    Xil_signed16*  src2_pixel = src2_scanline;
                    Xil_signed16*  dest_pixel = dest_scanline;
                    Xil_unsigned8* srcA_pixel = srcA_scanline;

                    for(x=ci.xsize; x>0; x--) {
                        Xil_signed16*  src1 = src1_pixel;
                        Xil_signed16*  src2 = src2_pixel;
                        Xil_unsigned8* srcA = srcA_pixel;
                        Xil_signed16*  dest = dest_pixel;

                        float ratio = (float)(*srcA) / (float)XIL_MAXBYTE;

                        for(int band=0; band<nbands; band++) {
                            float sum =
                                ((1.0F - ratio) * *src1) + (ratio  * *src2);

                            if(sum < 0.0F) {
                                *dest = (Xil_signed16)(sum);
                            } else {
                                *dest = (Xil_signed16)(sum + 0.5F);
                            }

                            src1++;
                            src2++;
                            dest++;
                        }

                        src1_pixel += src1_pixel_stride;
                        src2_pixel += src2_pixel_stride;
                        srcA_pixel += srcA_pixel_stride;
                        dest_pixel += dest_pixel_stride;
                    }

                    src1_scanline += src1_scanline_stride;
                    src2_scanline += src2_scanline_stride;
                    srcA_scanline += srcA_scanline_stride;
                    dest_scanline += dest_scanline_stride;
                }
	    }
	} else {
            //
            //  The Alpha image.
            //
            Xil_unsigned8* srcA_data            = (Xil_unsigned8*) ci.src3Data;
            unsigned int   srcA_scanline_stride = ci.src3ScanlineStride;
            unsigned int   srcA_pixel_stride    = ci.src3PixelStride;

	    for(int band=0; band<nbands; band++) { 
                //
                //  The input images.
                //
                Xil_signed16*  src1_scanline        = (Xil_signed16*)
                    ci.getSrc1Data(band);
                Xil_signed16*  src2_scanline        = (Xil_signed16*)
                    ci.getSrc2Data(band);
                Xil_signed16*  dest_scanline        = (Xil_signed16*)
                    ci.getDestData(band);

                unsigned int   src1_scanline_stride =
                    ci.getSrc1ScanlineStride(band);
                unsigned int   src2_scanline_stride =
                    ci.getSrc2ScanlineStride(band);
                unsigned int   dest_scanline_stride =
                    ci.getDestScanlineStride(band);

                unsigned int   src1_pixel_stride    =
                    ci.getSrc1PixelStride(band);
                unsigned int   src2_pixel_stride    =
                    ci.getSrc2PixelStride(band);
                unsigned int   dest_pixel_stride    =
                    ci.getDestPixelStride(band);

                Xil_unsigned8* srcA_scanline        = srcA_data;

                for(y=ci.ysize; y>0; y--) { 
                    Xil_signed16*  src1 = src1_scanline;
                    Xil_signed16*  src2 = src2_scanline;
                    Xil_unsigned8* srcA = srcA_scanline;
                    Xil_signed16*  dest = dest_scanline;

                    for(x=ci.xsize; x>0; x--) {
                        float ratio = (float)(*srcA) / (float)XIL_MAXBYTE;
                        float sum   =
                            ((1.0F - ratio) * *src1) +
                            (ratio          * *src2);

                        if(sum < 0.0F) {
                            *dest = (Xil_signed16)(sum);
                        } else {
                            *dest = (Xil_signed16)(sum + 0.5F);
                        }

                        src1 += src1_pixel_stride;
                        src2 += src2_pixel_stride;
                        srcA += srcA_pixel_stride;
                        dest += dest_pixel_stride;
                    }

                    src1_scanline += src1_scanline_stride;
                    src2_scanline += src2_scanline_stride;
                    srcA_scanline += srcA_scanline_stride;
                    dest_scanline += dest_scanline_stride;
                }
	    }
	}
    }

    
    return ci.returnValue;
}


XilStatus
XilDeviceManagerComputeSHORT::Blenda16(XilOp*       op,
				       unsigned     op_count,
				       XilRoi*      roi,
				       XilBoxList*  bl)

{
    ComputeInfoSHORT  ci(op, op_count, roi, bl);

    if(ci.isOK() == FALSE) {
        return ci.returnValue;
    }

    unsigned int nbands      = ci.destNumBands;
    const float  short_range = (float)XIL_MAXSHORT - (float)XIL_MINSHORT;

    //
    //  Loop iterators.
    //
    int          x;
    int          y;

    while(ci.hasMoreInfo()) {
	if((ci.src1Storage.isType(XIL_PIXEL_SEQUENTIAL)) &&
           (ci.src2Storage.isType(XIL_PIXEL_SEQUENTIAL)) &&
           (ci.destStorage.isType(XIL_PIXEL_SEQUENTIAL)))
	{
            //
            //  The Alpha image.
            //
            Xil_signed16* srcA_scanline        = ci.src3Data;
            unsigned int  srcA_scanline_stride = ci.src3ScanlineStride;
            unsigned int  srcA_pixel_stride    = ci.src3PixelStride;

            //
            //  The input images.
            //
	    Xil_signed16* src1_scanline        = ci.src1Data;
            Xil_signed16* src2_scanline        = ci.src2Data;
            Xil_signed16* dest_scanline        = ci.destData;

            unsigned int  src1_scanline_stride = ci.src1ScanlineStride;
            unsigned int  src2_scanline_stride = ci.src2ScanlineStride;
            unsigned int  dest_scanline_stride = ci.destScanlineStride;

            unsigned int  src1_pixel_stride    = ci.src1PixelStride;
            unsigned int  src2_pixel_stride    = ci.src2PixelStride;
            unsigned int  dest_pixel_stride    = ci.destPixelStride;

            if(nbands == 1) {
                for(y=ci.ysize; y>0; y--) {
                    Xil_signed16* src1 = src1_scanline;
                    Xil_signed16* src2 = src2_scanline;
                    Xil_signed16* dest = dest_scanline;
                    Xil_signed16* srcA = srcA_scanline;

                    for(x=ci.xsize; x>0; x--) {
                        float ratio =
                            ((float)(*srcA) - (float)XIL_MINSHORT) / short_range;
                        float sum   =
                            ((1.0F - ratio) * *src1) +
                            (ratio          * *src2);

                        if(sum < 0.0F) {
                            *dest = (Xil_signed16)(sum);
                        } else {
                            *dest = (Xil_signed16)(sum + 0.5F);
                        }

                        src1 += src1_pixel_stride;
                        src2 += src2_pixel_stride;
                        srcA += srcA_pixel_stride;
                        dest += dest_pixel_stride;
                    }

                    src1_scanline += src1_scanline_stride;
                    src2_scanline += src2_scanline_stride;
                    srcA_scanline += srcA_scanline_stride;
                    dest_scanline += dest_scanline_stride;
                }
            } else if(nbands == 3) {
                for(y=ci.ysize; y>0; y--) {
                    Xil_signed16* src1 = src1_scanline;
                    Xil_signed16* src2 = src2_scanline;
                    Xil_signed16* dest = dest_scanline;
                    Xil_signed16* srcA = srcA_scanline;

                    for(x=ci.xsize; x>0; x--) {
                        float ratio =
                            ((float)(*srcA) - (float)XIL_MINSHORT) / short_range;

                        float sum   =
                            ((1.0F - ratio) * *src1) +
                            (ratio          * *src2);
                        if(sum < 0.0F) {
                            *dest = (Xil_signed16)(sum);
                        } else {
                            *dest = (Xil_signed16)(sum + 0.5F);
                        }

                        sum =
                            ((1.0F - ratio) * *(src1+1)) +
                            (ratio          * *(src2+1));
                        if(sum < 0.0F) {
                            *(dest+1) = (Xil_signed16)(sum);
                        } else {
                            *(dest+1) = (Xil_signed16)(sum + 0.5F);
                        }

                        sum =
                            ((1.0F - ratio) * *(src1+2)) +
                            (ratio          * *(src2+2));
                        if(sum < 0.0F) {
                            *(dest+2) = (Xil_signed16)(sum);
                        } else {
                            *(dest+2) = (Xil_signed16)(sum + 0.5F);
                        }

                        src1 += src1_pixel_stride;
                        src2 += src2_pixel_stride;
                        srcA += srcA_pixel_stride;
                        dest += dest_pixel_stride;
                    }

                    src1_scanline += src1_scanline_stride;
                    src2_scanline += src2_scanline_stride;
                    srcA_scanline += srcA_scanline_stride;
                    dest_scanline += dest_scanline_stride;
                }
            } else {
                for(y=ci.ysize; y>0; y--) {
                    Xil_signed16* src1_pixel = src1_scanline;
                    Xil_signed16* src2_pixel = src2_scanline;
                    Xil_signed16* dest_pixel = dest_scanline;
                    Xil_signed16* srcA_pixel = srcA_scanline;

                    for(x=ci.xsize; x>0; x--) {
                        Xil_signed16* src1 = src1_pixel;
                        Xil_signed16* src2 = src2_pixel;
                        Xil_signed16* srcA = srcA_pixel;
                        Xil_signed16* dest = dest_pixel;

                        float ratio =
                            ((float)(*srcA) - (float)XIL_MINSHORT) / short_range;

                        for(int band=0; band<nbands; band++) {
                            float sum =
                                ((1.0F - ratio) * *src1) + (ratio  * *src2);

                            if(sum < 0.0F) {
                                *dest = (Xil_signed16)(sum);
                            } else {
                                *dest = (Xil_signed16)(sum + 0.5F);
                            }

                            src1++;
                            src2++;
                            dest++;
                        }

                        src1_pixel += src1_pixel_stride;
                        src2_pixel += src2_pixel_stride;
                        srcA_pixel += srcA_pixel_stride;
                        dest_pixel += dest_pixel_stride;
                    }

                    src1_scanline += src1_scanline_stride;
                    src2_scanline += src2_scanline_stride;
                    srcA_scanline += srcA_scanline_stride;
                    dest_scanline += dest_scanline_stride;
                }
	    }
	} else {
            //
            //  The Alpha image.
            //
            Xil_signed16* srcA_data            = ci.src3Data;
            unsigned int  srcA_scanline_stride = ci.src3ScanlineStride;
            unsigned int  srcA_pixel_stride    = ci.src3PixelStride;

	    for(int band=0; band<nbands; band++) { 
                //
                //  The input images.
                //
                Xil_signed16*  src1_scanline        = ci.getSrc1Data(band);
                Xil_signed16*  src2_scanline        = ci.getSrc2Data(band);
                Xil_signed16*  dest_scanline        = ci.getDestData(band);

                unsigned int  src1_scanline_stride =
                    ci.getSrc1ScanlineStride(band);
                unsigned int  src2_scanline_stride =
                    ci.getSrc2ScanlineStride(band);
                unsigned int  dest_scanline_stride =
                    ci.getDestScanlineStride(band);

                unsigned int  src1_pixel_stride    =
                    ci.getSrc1PixelStride(band);
                unsigned int  src2_pixel_stride    =
                    ci.getSrc2PixelStride(band);
                unsigned int  dest_pixel_stride    =
                    ci.getDestPixelStride(band);

                Xil_signed16* srcA_scanline        = srcA_data;

                for(y=ci.ysize; y>0; y--) { 
                    Xil_signed16* src1 = src1_scanline;
                    Xil_signed16* src2 = src2_scanline;
                    Xil_signed16* srcA = srcA_scanline;
                    Xil_signed16* dest = dest_scanline;

                    for(x=ci.xsize; x>0; x--) {
                        float ratio =
                            ((float)(*srcA) - (float)XIL_MINSHORT) / short_range;
                        float sum   =
                            ((1.0F - ratio) * *src1) +
                            (ratio          * *src2);

                        if(sum < 0.0F) {
                            *dest = (Xil_signed16)(sum);
                        } else {
                            *dest = (Xil_signed16)(sum + 0.5F);
                        }

                        src1 += src1_pixel_stride;
                        src2 += src2_pixel_stride;
                        srcA += srcA_pixel_stride;
                        dest += dest_pixel_stride;
                    }

                    src1_scanline += src1_scanline_stride;
                    src2_scanline += src2_scanline_stride;
                    srcA_scanline += srcA_scanline_stride;
                    dest_scanline += dest_scanline_stride;
                }
	    }
	}
    }
    
    return ci.returnValue;
}



XilStatus
XilDeviceManagerComputeSHORT::Blendaf32(XilOp*       op,
					unsigned     op_count,
					XilRoi*      roi,
					XilBoxList*  bl)

{
    ComputeInfoGENERAL  ci(op, op_count, roi, bl);

    if(ci.isOK() == FALSE) {
        return ci.returnValue;
    }

    unsigned int nbands = ci.destNumBands;

    //
    //  Loop iterators.
    //
    int          x;
    int          y;

    while(ci.hasMoreInfo()) {
	if((ci.src1Storage.isType(XIL_PIXEL_SEQUENTIAL)) &&
           (ci.src2Storage.isType(XIL_PIXEL_SEQUENTIAL)) &&
           (ci.destStorage.isType(XIL_PIXEL_SEQUENTIAL)))
	{
            //
            //  The Alpha image.
            //
            Xil_float32*  srcA_data            = (Xil_float32*) ci.src3Data;
            unsigned int  srcA_scanline_stride = ci.src3ScanlineStride;
            unsigned int  srcA_pixel_stride    = ci.src3PixelStride;

            //
            //  The input images.
            //
	    Xil_signed16* src1_scanline        = (Xil_signed16*) ci.src1Data;
            Xil_signed16* src2_scanline        = (Xil_signed16*) ci.src2Data;
            Xil_signed16* dest_scanline        = (Xil_signed16*) ci.destData;

            unsigned int  src1_scanline_stride = ci.src1ScanlineStride;
            unsigned int  src2_scanline_stride = ci.src2ScanlineStride;
            unsigned int  dest_scanline_stride = ci.destScanlineStride;

            unsigned int  src1_pixel_stride    = ci.src1PixelStride;
            unsigned int  src2_pixel_stride    = ci.src2PixelStride;
            unsigned int  dest_pixel_stride    = ci.destPixelStride;

            Xil_float32*  srcA_scanline        = srcA_data;

            if(nbands == 1) {
                for(y=ci.ysize; y>0; y--) {
                    Xil_signed16* src1 = src1_scanline;
                    Xil_signed16* src2 = src2_scanline;
                    Xil_signed16* dest = dest_scanline;
                    Xil_float32*  srcA = srcA_scanline;

                    for(x=ci.xsize; x>0; x--) {
                        float sum   =
                            ((1.0F - *srcA) * *src1) +
                            (*srcA          * *src2);

                        if(sum < 0.0F) {
                            *dest = (Xil_signed16)(sum);
                        } else {
                            *dest = (Xil_signed16)(sum + 0.5F);
                        }

                        src1 += src1_pixel_stride;
                        src2 += src2_pixel_stride;
                        srcA += srcA_pixel_stride;
                        dest += dest_pixel_stride;
                    }

                    src1_scanline += src1_scanline_stride;
                    src2_scanline += src2_scanline_stride;
                    srcA_scanline += srcA_scanline_stride;
                    dest_scanline += dest_scanline_stride;
                }
            } else if(nbands == 3) {
                for(y=ci.ysize; y>0; y--) {
                    Xil_signed16* src1 = src1_scanline;
                    Xil_signed16* src2 = src2_scanline;
                    Xil_signed16* dest = dest_scanline;
                    Xil_float32*  srcA = srcA_scanline;

                    for(x=ci.xsize; x>0; x--) {
                        float sum   =
                            ((1.0F - *srcA) * *src1) +
                            (*srcA          * *src2);
                        if(sum < 0.0F) {
                            *dest = (Xil_signed16)(sum);
                        } else {
                            *dest = (Xil_signed16)(sum + 0.5F);
                        }

                        sum =
                            ((1.0F - *srcA) * *(src1+1)) +
                            (*srcA          * *(src2+1));
                        if(sum < 0.0F) {
                            *(dest+1) = (Xil_signed16)(sum);
                        } else {
                            *(dest+1) = (Xil_signed16)(sum + 0.5F);
                        }

                        sum =
                            ((1.0F - *srcA) * *(src1+2)) +
                            (*srcA          * *(src2+2));
                        if(sum < 0.0F) {
                            *(dest+2) = (Xil_signed16)(sum);
                        } else {
                            *(dest+2) = (Xil_signed16)(sum + 0.5F);
                        }

                        src1 += src1_pixel_stride;
                        src2 += src2_pixel_stride;
                        srcA += srcA_pixel_stride;
                        dest += dest_pixel_stride;
                    }

                    src1_scanline += src1_scanline_stride;
                    src2_scanline += src2_scanline_stride;
                    srcA_scanline += srcA_scanline_stride;
                    dest_scanline += dest_scanline_stride;
                }
            } else {
                for(y=ci.ysize; y>0; y--) {
                    Xil_signed16* src1_pixel = src1_scanline;
                    Xil_signed16* src2_pixel = src2_scanline;
                    Xil_signed16* dest_pixel = dest_scanline;
                    Xil_float32*  srcA_pixel = srcA_scanline;

                    for(x=ci.xsize; x>0; x--) {
                        Xil_signed16*  src1 = src1_pixel;
                        Xil_signed16*  src2 = src2_pixel;
                        Xil_float32* srcA = srcA_pixel;
                        Xil_signed16*  dest = dest_pixel;

                        for(int band=0; band<nbands; band++) {
                            float sum =
                                ((1.0F - *srcA) * *src1) + (*srcA  * *src2);

                            if(sum < 0.0F) {
                                *dest = (Xil_signed16)(sum);
                            } else {
                                *dest = (Xil_signed16)(sum + 0.5F);
                            }

                            src1++;
                            src2++;
                            dest++;
                        }

                        src1_pixel += src1_pixel_stride;
                        src2_pixel += src2_pixel_stride;
                        srcA_pixel += srcA_pixel_stride;
                        dest_pixel += dest_pixel_stride;
                    }

                    src1_scanline += src1_scanline_stride;
                    src2_scanline += src2_scanline_stride;
                    srcA_scanline += srcA_scanline_stride;
                    dest_scanline += dest_scanline_stride;
                }
	    }
	} else {
            //
            //  The Alpha image.
            //
            Xil_float32* srcA_data            = (Xil_float32*) ci.src3Data;
            unsigned int srcA_scanline_stride = ci.src3ScanlineStride;
            unsigned int srcA_pixel_stride    = ci.src3PixelStride;

	    for(int band=0; band<nbands; band++) { 
                //
                //  The input images.
                //
                Xil_signed16* src1_scanline        = (Xil_signed16*)
                    ci.getSrc1Data(band);
                Xil_signed16* src2_scanline        = (Xil_signed16*)
                    ci.getSrc2Data(band);
                Xil_signed16* dest_scanline        = (Xil_signed16*)
                    ci.getDestData(band);

                unsigned int  src1_scanline_stride =
                    ci.getSrc1ScanlineStride(band);
                unsigned int  src2_scanline_stride =
                    ci.getSrc2ScanlineStride(band);
                unsigned int  dest_scanline_stride =
                    ci.getDestScanlineStride(band);

                unsigned int  src1_pixel_stride    =
                    ci.getSrc1PixelStride(band);
                unsigned int  src2_pixel_stride    =
                    ci.getSrc2PixelStride(band);
                unsigned int  dest_pixel_stride    =
                    ci.getDestPixelStride(band);

                Xil_float32* srcA_scanline         = srcA_data;

                for(y=ci.ysize; y>0; y--) { 
                    Xil_signed16* src1 = src1_scanline;
                    Xil_signed16* src2 = src2_scanline;
                    Xil_float32*  srcA = srcA_scanline;
                    Xil_signed16* dest = dest_scanline;

                    for(x=ci.xsize; x>0; x--) {
                        float sum   =
                            ((1.0F - *srcA) * *src1) +
                            (*srcA          * *src2);

                        if(sum < 0.0F) {
                            *dest = (Xil_signed16)(sum);
                        } else {
                            *dest = (Xil_signed16)(sum + 0.5F);
                        }

                        src1 += src1_pixel_stride;
                        src2 += src2_pixel_stride;
                        srcA += srcA_pixel_stride;
                        dest += dest_pixel_stride;
                    }

                    src1_scanline += src1_scanline_stride;
                    src2_scanline += src2_scanline_stride;
                    srcA_scanline += srcA_scanline_stride;
                    dest_scanline += dest_scanline_stride;
                }
	    }
	}
    }
    
    return ci.returnValue;
}


