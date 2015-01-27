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
//  File:	BlackGeneration.cc
//  Project:	XIL
//  Revision:	1.8
//  Last Mod:	10:11:37, 03/10/00
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
#pragma ident	"@(#)BlackGeneration.cc	1.8\t00/03/10  "

#include "XilDeviceManagerComputeSHORT.hh"
#include "ComputeInfo.hh"

#ifdef MIN
#undef MIN
#endif

#define MIN(a,b,c) ((a)>(b)?((b)>(c)?(c):(b)):((a)>(c)?(c):(a)))

XilStatus
XilDeviceManagerComputeSHORT::BlackGeneration(XilOp*       op,
                                              unsigned     op_count,
                                              XilRoi*      roi,
                                              XilBoxList*  bl)
{
    ComputeInfoSHORT  ci(op, op_count, roi, bl);

    if(ci.isOK() == FALSE) {
        return ci.returnValue;
    }

    float black;
    float undercolor;
    op->getParam(1, &black);
    op->getParam(2, &undercolor);

#ifdef _XIL_USE_TABLE_FLT_CNV
    //
    //  Get short to float array
    //
    Xil_float32* short_to_float = getShortToFloat(ci.getSystemState());
    if(short_to_float == NULL) {
        return XIL_FAILURE;
    }
#endif // _XIL_USE_TABLE_FLT_CNV
    
    while(ci.hasMoreInfo()) {
        if(ci.isStorageType(XIL_PIXEL_SEQUENTIAL)) { 
            Xil_signed16*  src1_scanline = ci.src1Data;
            Xil_signed16*  dest_scanline = ci.destData;

            unsigned int   src1_pstride  = ci.src1PixelStride;
            unsigned int   dest_pstride  = ci.destPixelStride;
            
            unsigned int   src1_sstride  = ci.src1ScanlineStride;
            unsigned int   dest_sstride  = ci.destScanlineStride;

            float          blk           = black;
            float          unc           = undercolor;

#ifdef _XIL_USE_TABLE_FLT_CNV
            float*         s2f           = short_to_float;
#endif

            for(int y=ci.ysize; y>0; y--) {
                Xil_signed16*  src1 = src1_scanline;
                Xil_signed16*  dest = dest_scanline;
                
                for(int x=ci.xsize; x>0; x--) {
		    Xil_signed16 short_min =
                        MIN(*src1, *(src1 + 1), *(src1 + 2));

#ifdef _XIL_USE_TABLE_FLT_CNV
                    float min   = s2f[short_min];
		    float under = unc*min;

                    float tmp0  = s2f[*src1]     - under;
                    float tmp1  = s2f[*(src1+1)] - under;
                    float tmp2  = s2f[*(src1+2)] - under;
#else
                    float min   = (float)short_min;
		    float under = unc*min;

                    float tmp0  = ((float)*src1)     - under;
                    float tmp1  = ((float)*(src1+1)) - under;
                    float tmp2  = ((float)*(src1+2)) - under;
#endif //_XIL_USE_TABLE_FLT_CNV

                    *dest       = _XILI_ROUND_S16(tmp0);
                    *(dest+1)   = _XILI_ROUND_S16(tmp1);
                    *(dest+2)   = _XILI_ROUND_S16(tmp2);
                    *(dest+3)   = _XILI_ROUND_S16(blk*min);

                    src1 += src1_pstride;
                    dest += dest_pstride;
                }
                
                src1_scanline += src1_sstride;
                dest_scanline += dest_sstride;
            }
        } else {
            //
	    //  General Case
            //
	    Xil_signed16*  srcC_scanline    = ci.getSrc1Data(0);
	    int            srcC_pixelStride = ci.getSrc1PixelStride(0);
	    int            srcC_lineStride  = ci.getSrc1ScanlineStride(0);
	    Xil_signed16*  srcM_scanline    = ci.getSrc1Data(1);
	    int            srcM_pixelStride = ci.getSrc1PixelStride(1);
	    int            srcM_lineStride  = ci.getSrc1ScanlineStride(1);
	    Xil_signed16*  srcY_scanline    = ci.getSrc1Data(2);
	    int            srcY_pixelStride = ci.getSrc1PixelStride(2);
	    int            srcY_lineStride  = ci.getSrc1ScanlineStride(2);
	    
	    Xil_signed16*  dstC_scanline    = ci.getDestData(0);
	    int            dstC_pixelStride = ci.getDestPixelStride(0);
	    int            dstC_lineStride  = ci.getDestScanlineStride(0);
	    Xil_signed16*  dstM_scanline    = ci.getDestData(1);
	    int            dstM_pixelStride = ci.getDestPixelStride(1);
	    int            dstM_lineStride  = ci.getDestScanlineStride(1);
	    Xil_signed16*  dstY_scanline    = ci.getDestData(2);
	    int            dstY_pixelStride = ci.getDestPixelStride(2);
	    int            dstY_lineStride  = ci.getDestScanlineStride(2);
	    Xil_signed16*  dstK_scanline    = ci.getDestData(3);
	    int            dstK_pixelStride = ci.getDestPixelStride(3);
	    int            dstK_lineStride  = ci.getDestScanlineStride(3);
	    
            float          blk              = black;
            float          unc              = undercolor;

#ifdef _XIL_USE_TABLE_FLT_CNV
            float*         s2f              = short_to_float;
#endif

	    for(int y=ci.ysize; y>0; y--) {
                Xil_signed16*  srcC = srcC_scanline;
                Xil_signed16*  srcM = srcM_scanline;
                Xil_signed16*  srcY = srcY_scanline;
	      
                Xil_signed16*  dstC = dstC_scanline;
                Xil_signed16*  dstM = dstM_scanline;
                Xil_signed16*  dstY = dstY_scanline;
                Xil_signed16*  dstK = dstK_scanline;
	      
                for(int x=ci.xsize; x>0; x--) {
		    Xil_signed16 short_min =
                        MIN(*srcC, *srcM, *srcY);

#ifdef _XIL_USE_TABLE_FLT_CNV
                    float min   = s2f[short_min];
		    float under = unc*min;

                    float tmp0  = s2f[*srcC] - under;
                    float tmp1  = s2f[*srcM] - under;
                    float tmp2  = s2f[*srcY] - under;
#else
                    float min   = (float)short_min;
		    float under = unc*min;

                    float tmp0  = (float)*srcC - under;
                    float tmp1  = (float)*srcM - under;
                    float tmp2  = (float)*srcY - under;
#endif // _XIL_USE_TABLE_FLT_CNV

                    *dstC       = _XILI_ROUND_S16(tmp0);
                    *dstM       = _XILI_ROUND_S16(tmp1);
                    *dstY       = _XILI_ROUND_S16(tmp2);
                    *dstK       = _XILI_ROUND_S16(blk*min);

                    srcC += srcC_pixelStride;
                    srcM += srcM_pixelStride;
                    srcY += srcY_pixelStride;

                    dstC += dstC_pixelStride;
                    dstM += dstM_pixelStride;
                    dstY += dstY_pixelStride;
                    dstK += dstK_pixelStride;
                }
	      
                srcC_scanline += srcC_lineStride;
                srcM_scanline += srcM_lineStride;
                srcY_scanline += srcY_lineStride;

                dstC_scanline += dstC_lineStride;
                dstM_scanline += dstM_lineStride;
                dstY_scanline += dstY_lineStride;
                dstK_scanline += dstK_lineStride;
	    }
        }
    }

    return ci.returnValue;
}
