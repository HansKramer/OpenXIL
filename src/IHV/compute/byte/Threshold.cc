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
//  File:	Threshold.cc
//  Project:	XIL
//  Revision:	1.10
//  Last Mod:	10:10:25, 03/10/00
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
#pragma ident	"@(#)Threshold.cc	1.10\t00/03/10  "

#include "XilDeviceManagerComputeBYTE.hh"
#include "ComputeInfo.hh"

XilStatus
XilDeviceManagerComputeBYTE::Threshold(XilOp*       op,
                                      unsigned     op_count,
                                      XilRoi*      roi,
                                      XilBoxList*  bl)
{
    ComputeInfoBYTE  ci(op, op_count, roi, bl);

    if(ci.isOK() == FALSE) {
        return ci.returnValue;
    }

    Xil_unsigned8* op_low;
    op->getParam(1, (void**)&op_low);

    Xil_unsigned8* op_high;
    op->getParam(2, (void**)&op_high);

    Xil_unsigned8* op_map;
    op->getParam(3, (void**)&op_map);

    while(ci.hasMoreInfo()) {
        Xil_unsigned8* low  = op_low;
        Xil_unsigned8* high = op_high;
        Xil_unsigned8* map  = op_map;

	COMPUTE_GENERAL_1S_1D_W_BAND(Xil_unsigned8, Xil_unsigned8,

                                     if(*src1 > *high) {
                                         *dest = *src1;
                                     } else if(*src1 < *low) {
                                         *dest = *src1;
                                     } else {
                                         *dest = *map;
                                     },

                                     if(*(src1+1) > *(high+1)) {
                                         *(dest+1) = *(src1+1);
                                     } else if(*(src1+1) < *(low+1)) {
                                         *(dest+1) = *(src1+1);
                                     } else {
                                         *(dest+1) = *(map+1);
                                     }
                                     if(*(src1+2) > *(high+2)) {
                                         *(dest+2) = *(src1+2);
                                     } else if(*(src1+2) < *(low+2)) {
                                         *(dest+2) = *(src1+2);
                                     } else {
                                         *(dest+2) = *(map+2);
                                     },

                                     if(*src1 > *(high+band)) {
                                         *dest = *src1;
                                     } else if(*src1 < *(low+band)) {
                                         *dest = *src1;
                                     } else {
                                         *dest = *(map+band);
                                     }
	);
    }

    return ci.returnValue;
}

XilStatus
XilDeviceManagerComputeBYTE::ThresholdThreshold(XilOp*       op,
                                                unsigned     op_count,
                                                XilRoi*      roi,
                                                XilBoxList*  bl)
{
    ComputeInfoBYTE  ci(op, op_count, roi, bl);

    if(ci.isOK() == FALSE) {
        return ci.returnValue;
    }

    XilOp* op1 = op->getOpList()[1];
    XilOp* op2 = op;

    //
    //  First Threshold
    //
    Xil_unsigned8* op1_low;
    op1->getParam(1, (void**)&op1_low);

    Xil_unsigned8* op1_high;
    op1->getParam(2, (void**)&op1_high);

    Xil_unsigned8* op1_map;
    op1->getParam(3, (void**)&op1_map);

    //
    //  Second Threshold
    //
    Xil_unsigned8* op2_low;
    op2->getParam(1, (void**)&op2_low);

    Xil_unsigned8* op2_high;
    op2->getParam(2, (void**)&op2_high);

    Xil_unsigned8* op2_map;
    op2->getParam(3, (void**)&op2_map);

    while(ci.hasMoreInfo()) {
        Xil_unsigned8* low1  = op1_low;
        Xil_unsigned8* high1 = op1_high;
        Xil_unsigned8* map1  = op1_map;

        Xil_unsigned8* low2  = op2_low;
        Xil_unsigned8* high2 = op2_high;
        Xil_unsigned8* map2  = op2_map;

	COMPUTE_GENERAL_1S_1D_W_BAND(Xil_unsigned8, Xil_unsigned8,

                                     Xil_unsigned8 val = *src1;
                                     if(val > *high1) {
                                         if(val > *high2) {
                                             *dest = val;
                                         } else if(val < *low2) {
                                             *dest = val;
                                         } else {
                                             *dest = *map2;
                                         }
                                     } else if(val < *low1) {
                                         if(val > *high2) {
                                             *dest = val;
                                         } else if(val < *low2) {
                                             *dest = val;
                                         } else {
                                             *dest = *map2;
                                         }
                                     } else {
                                         if(*map1 > *high2) {
                                             *dest = *map1;
                                         } else if(*map1 < *low2) {
                                             *dest = *map1;
                                         } else {
                                             *dest = *map2;
                                         }
                                     },

                                     Xil_unsigned8 val = *(src1+1);
                                     if(val > *(high1+1)) {
                                         if(val > *(high2+1)) {
                                             *(dest+1) = val;
                                         } else if(val < *(low2+1)) {
                                             *(dest+1) = val;
                                         } else {
                                             *(dest+1) = *(map2+1);
                                         }
                                     } else if(val < *(low1+1)) {
                                         if(val > *(high2+1)) {
                                             *(dest+1) = val;
                                         } else if(val < *(low2+1)) {
                                             *(dest+1) = val;
                                         } else {
                                             *(dest+1) = *(map2+1);
                                         }
                                     } else {
                                         if(*(map1+1) > *(high2+1)) {
                                             *(dest+1) = *(map1+1);
                                         } else if(*(map1+1) < *(low2+1)) {
                                             *(dest+1) = *(map1+1);
                                         } else {
                                             *(dest+1) = *(map2+1);
                                         }
                                     }
                                     val = *(src1+2);
                                     if(val > *(high1+2)) {
                                         if(val > *(high2+2)) {
                                             *(dest+2) = val;
                                         } else if(val < *(low2+2)) {
                                             *(dest+2) = val;
                                         } else {
                                             *(dest+2) = *(map2+2);
                                         }
                                     } else if(val < *(low1+2)) {
                                         if(val > *(high2+2)) {
                                             *(dest+2) = val;
                                         } else if(val < *(low2+2)) {
                                             *(dest+2) = val;
                                         } else {
                                             *(dest+2) = *(map2+2);
                                         }
                                     } else {
                                         if(*(map1+2) > *(high2+2)) {
                                             *(dest+2) = *(map1+2);
                                         } else if(*(map1+2) < *(low2+2)) {
                                             *(dest+2) = *(map1+2);
                                         } else {
                                             *(dest+2) = *(map2+2);
                                         }
                                     },
                                     
                                     Xil_unsigned8 val = *src1;
                                     if(val > *(high1+band)) {
                                         if(val > *(high2+band)) {
                                             *dest = val;
                                         } else if(val < *(low2+band)) {
                                             *dest = val;
                                         } else {
                                             *dest = *(map2+band);
                                         }
                                     } else if(val < *(low1+band)) {
                                         if(val > *(high2+band)) {
                                             *dest = val;
                                         } else if(val < *(low2+band)) {
                                             *dest = val;
                                         } else {
                                             *dest = *(map2+band);
                                         }
                                     } else {
                                         if(*(map1+band) > *(high2+band)) {
                                             *dest = *(map1+band);
                                         } else if(*(map1+band) < *(low2+band)) {
                                             *dest = *(map1+band);
                                         } else {
                                             *dest = *(map2+band);
                                         }
                                     }

	);
    }

    return ci.returnValue;
}

XilStatus
XilDeviceManagerComputeBYTE::Threshold_1BAND(XilOp*       op,
                                             unsigned     op_count,
                                             XilRoi*      roi,
                                             XilBoxList*  bl)
{
    if(op->getDstImage(1)->getNumBands() != 1 ||
       sizeof(Xil_unsigned8) != 1) {
        return XIL_FAILURE;
    }

    ComputeInfoBYTE  ci(op, op_count, roi, bl);

    if(ci.isOK() == FALSE) {
        return ci.returnValue;
    }

    Xil_unsigned8* op_low;
    op->getParam(1, (void**)&op_low);

    Xil_unsigned8* op_high;
    op->getParam(2, (void**)&op_high);

    Xil_unsigned8* op_map;
    op->getParam(3, (void**)&op_map);

    while(ci.hasMoreInfo()) {
        if(ci.destScanlineStride == ci.xsize &&
           ci.src1ScanlineStride == ci.xsize) {
            Xil_unsigned8  low       = *op_low;
            Xil_unsigned8  high      = *op_high;
            Xil_unsigned8  map       = *op_map;

            Xil_unsigned8* src_data  = ci.src1Data;
            Xil_unsigned8* dst_data  = ci.destData;
            Xil_unsigned8* src       = src_data;
            Xil_unsigned8* stop      = src_data + (ci.xsize*ci.ysize);

            unsigned int   status    = 0;
            unsigned int   status_i  = 0;
            unsigned int   current_i = 0;

            const int _XIL_FUNC_THRESH = 32;

            while(src < stop) {
                if(status == 0) {
                    status = 1;

                    while((src<stop) && ((*src > high) || (*src < low))) {
                        src++;
                    }

                    current_i = src - src_data;

                    int bytes = current_i - status_i;
                    if(bytes < _XIL_FUNC_THRESH) {
                        Xil_unsigned8* s = src_data+status_i;
                        Xil_unsigned8* d = dst_data+status_i;

                        while(bytes--) {
                            *d++ = *s++;
                        }
                    } else {
                        xili_memcpy(dst_data+status_i, src_data+status_i,
                                    bytes);
                    }
                } else {
                    status = 0;

                    while((src<stop) && ((*src <= high) && (*src >= low))) {
                        src++;
                    }

                    current_i = src - src_data;

                    int bytes = current_i - status_i;
                    if(bytes < _XIL_FUNC_THRESH) {
                        Xil_unsigned8* d = dst_data+status_i;

                        while(bytes--) {
                            *d++ = map;
                        }
                    } else {
                        xili_memset(dst_data+status_i, map, bytes);
                    }
                }

                status_i  = current_i;
            }
        } else {
            unsigned int   xsize = ci.xsize;

            Xil_unsigned8  low       = *op_low;
            Xil_unsigned8  high      = *op_high;
            Xil_unsigned8  map       = *op_map;

            Xil_unsigned8* src1_scanline = ci.src1Data;
            Xil_unsigned8* dest_scanline = ci.destData;
           
            unsigned int   src1_sstride  = ci.src1ScanlineStride;
            unsigned int   dest_sstride  = ci.destScanlineStride;
           
            unsigned int   src1_pstride  = ci.src1PixelStride;
            unsigned int   dest_pstride  = ci.destPixelStride;

            if(src1_pstride == 1 && dest_pstride == 1) {
                for(unsigned int y=ci.ysize; y!=0; y--) {
                    Xil_unsigned8* src1 = src1_scanline;
                    Xil_unsigned8* dest = dest_scanline;
                       
                    for(unsigned int x=xsize; x!=0; x--) {
                        if(*src1 > high) {
                            *dest = *src1;
                        } else if(*src1 < low) {
                            *dest = *src1;
                        } else {
                            *dest = map;
                        }

                        src1++;
                        dest++;
                    }
                       
                    src1_scanline += src1_sstride;
                    dest_scanline += dest_sstride;
                }
            } else {
                for(unsigned int y=ci.ysize; y!=0; y--) {
                    Xil_unsigned8* src1 = src1_scanline;
                    Xil_unsigned8* dest = dest_scanline;
                    
                    for(unsigned int x=xsize; x!=0; x--) {
                        if(*src1 > high) {
                            *dest = *src1;
                        } else if(*src1 < low) {
                            *dest = *src1;
                        } else {
                            *dest = map;
                        }

                        src1 += src1_pstride;
                        dest += dest_pstride;
                    }
                    
                    src1_scanline += src1_sstride;
                    dest_scanline += dest_sstride;
                }
            }
        }
    }

    return ci.returnValue;
}

