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
//  File:	Copy.cc
//  Project:	XIL
//  Revision:	1.11
//  Last Mod:	10:09:12, 03/10/00
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
#pragma ident	"@(#)Copy.cc	1.11\t00/03/10  "

#include "XilDeviceManagerComputeBIT.hh"
#include "ComputeInfo.hh"
#include "XiliUtils.hh"

XilStatus
XilDeviceManagerComputeBIT::Copy(XilOp*       op,
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
	for(int b=0; b<nbands; b++) {
	    Xil_unsigned8* src1_scanline        = ci.getSrc1Data(b);
	    Xil_unsigned8* dest_scanline        = ci.getDestData(b);

	    unsigned int   src1_scanline_stride = ci.getSrc1ScanlineStride(b);
	    unsigned int   dest_scanline_stride = ci.getDestScanlineStride(b);

	    unsigned int   src1_offset 	        = ci.getSrc1Offset(b);
	    unsigned int   dest_offset          = ci.getDestOffset(b);
	    
	    //
	    // Default copy is top to bottom.
            //
	    // The testing got really complicated because of the !@$% bit stuff.
	    // So break things out into seperate cases.
	    //
	    // test1 - see if the destination start after the start of the source
	    // test2 - see if the destination starts before the end of the source's
	    //         first scanline.  test1 && test2 means right to left copy
	    // test3 - see if the destination starts before the end of the source's
	    //         last scanline.  test1 && test3 mean bottom to top copy.
	    //
	    int right_to_left = FALSE;
	    int bottom_up = FALSE;
	    int test1 = FALSE;
	    if(((unsigned int)dest_scanline > (unsigned int)src1_scanline) ||
		(((unsigned int)dest_scanline == (unsigned int) src1_scanline) &&
		 (dest_offset > src1_offset)))
		test1 = TRUE;

	    if(test1) {
		//
		// test 2 - see if dest starts before end of src1's first scanline
		//
		unsigned int src1_end_scanline = (unsigned int)(src1_scanline +
					       (ci.xsize + ci.src1Offset - 1) / 8);
		unsigned int src1_end_offset = (ci.xsize + ci.src1Offset - 1) % 8;
		if(((unsigned int)dest_scanline < src1_end_scanline) ||
		    (((unsigned int)dest_scanline == src1_end_scanline) &&
		     (dest_offset < src1_end_offset))) {
		    right_to_left = TRUE;
		}

		if(!right_to_left) {
		    //
		    // test 3 - see if dest starts before end of source
		    //
		    unsigned int src1_end = (unsigned int)(src1_scanline +
					     (ci.ysize - 1) * src1_scanline_stride + 
					     ((ci.xsize + src1_offset - 1) / 8));
		    if(((unsigned int)dest_scanline < src1_end) ||
			(((unsigned int)dest_scanline == src1_end) &&
			 (dest_offset < src1_end_offset))) {
			bottom_up = TRUE;
		    }
		}
	    }
		
	    if(bottom_up) {
		src1_scanline += (ci.ysize - 1) * src1_scanline_stride;
		dest_scanline += (ci.ysize - 1) * dest_scanline_stride;
		src1_scanline_stride *= -1;
		dest_scanline_stride *= -1;
	    }

	    Xil_unsigned8* save_src_ptr = NULL;
	    if(right_to_left) {
		//
		// right to left isn't really supported in xili_bit_memcpy
		// Therefore, it will probably be faster to create a temporary
		// buffer and use that as the source.
		//
		int buffer_size = ci.ysize * src1_scanline_stride;
		Xil_unsigned8* tmp_src_ptr = new Xil_unsigned8[buffer_size];
		save_src_ptr = tmp_src_ptr;
		for (int y = ci.ysize; y>0; y--) {
		    xili_bit_memcpy(src1_scanline, tmp_src_ptr,
				    ci.xsize, src1_offset, src1_offset);
		    src1_scanline += src1_scanline_stride;
		    tmp_src_ptr   += src1_scanline_stride;
		}

		src1_scanline = save_src_ptr;
	    }

	    for(int y=ci.ysize; y>0; y--) {
		xili_bit_memcpy(src1_scanline, dest_scanline,
                                ci.xsize, src1_offset, dest_offset);
		
		src1_scanline += src1_scanline_stride;
		dest_scanline += dest_scanline_stride;
	    }

	    if(right_to_left) {
		//
		// Clean up after creating the temporary buffer
		//
		delete [] save_src_ptr;
	    }
	}
    }

    return ci.returnValue;
}

