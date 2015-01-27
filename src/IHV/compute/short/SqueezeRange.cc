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
//  File:	SqueezeRange.cc
//  Project:	XIL
//  Revision:	1.9
//  Last Mod:	10:11:46, 03/10/00
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
#pragma ident	"@(#)SqueezeRange.cc	1.9\t00/03/10  "

#include "XilDeviceManagerComputeSHORT.hh"
#include "ComputeInfo.hh"
#include <string.h>

XilStatus
XilDeviceManagerComputeSHORT::SqueezeRange(XilOp*       op,
                                           unsigned     op_count,
                                           XilRoi*      roi,
                                           XilBoxList*  bl)
{
    ComputeInfoSHORT  ci(op, op_count, roi, bl);
    if(ci.isOK() == FALSE) {
        return ci.returnValue;
    }

    //
    // Initialize values
    //
    Xil_unsigned8  flags_array[65536];
    Xil_unsigned8* flags = &flags_array[32768];
    unsigned int   values_seen = 0;
    int	           imin        = 32767;
    int	           imax        = -32768;

    xili_memset(flags_array, 0, 65536);

    //
    // Squeeze range by definition only works on single band
    // images so we can remove all of the band loops, and hence
    // special case loops. The op checks that the image is single banded 
    // before we ever get here.
    //
    while(ci.hasMoreInfo()) {
        const unsigned int ysize         = ci.ysize;
        const unsigned int xsize         = ci.xsize;
        const unsigned int src1_sstride  = ci.src1ScanlineStride;
        const unsigned int src1_pstride  = ci.src1PixelStride;
        Xil_signed16*      src1_scanline = (Xil_signed16*)ci.src1Data;

        if(src1_pstride == 1) {
            for(unsigned int y=ysize; y!=0; y--) {
                Xil_signed16* src1 = src1_scanline;
                        
                for(unsigned int x=xsize; x!=0; x--) {
                    Xil_signed16 pixel = *src1++;

                    if(! flags[pixel]) {
                        flags[pixel] = 1;
                        if(pixel > imax) {
                            imax = pixel;
                        }
                        if(pixel < imin) {
                            imin = pixel;
                        }
                        if(++values_seen == 65536) {
                            goto range_full;
                        }
                    }
                }

                src1_scanline += src1_sstride;
            } 
        } else { 
            for(unsigned int y=ysize; y!=0; y--) {
                Xil_signed16* src1 = src1_scanline;

                for(unsigned int x=xsize; x!=0; x--) {
                    Xil_signed16 pixel = *src1;

                    if(! flags[pixel]) {
                        flags[pixel] = 1;
                        if(pixel > imax) {
                            imax = pixel;
                        }
                        if(pixel < imin) {
                            imin = pixel;
                        }
                        if(++values_seen == 65536) {
                            goto range_full;
                        }
                    }

                    src1 += src1_pstride;
                }
                        
                src1_scanline += src1_sstride;
            }
        }
    }   

 range_full:
    if(ci.returnValue != XIL_SUCCESS) {
        //
	//  Something went wrong...return
        //
	return ci.returnValue;
    }
    
    //
    //  Report the results...
    //
    void* results[3];
    results[0] = (void*)&imin;
    results[1] = (void*)&imax;
    results[2] = (void*)flags_array;
    return op->reportResults(results);
}
