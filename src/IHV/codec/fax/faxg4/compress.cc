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
//  File:       compress.cc
//  Project:    XIL
//  Revision:   1.7
//  Last Mod:   10:14:16, 03/10/00
//
//  Description:
//
//    Compression function for G4 Fax (2D) compression
//    (ITU T-006)
//
//
//------------------------------------------------------------------------
//    COPYRIGHT
//------------------------------------------------------------------------
#pragma ident   "@(#)compress.cc	1.7\t00/03/10  "

#include <xil/xilGPI.hh>
#include "XilDeviceCompressionFaxG4.hh"

XilStatus
XilDeviceCompressionFaxG4::compress(XilOp*       op,
                                    unsigned int op_count,
                                    XilRoi*      roi,
                                    XilBoxList*  bl)
{
    CompressInfo ci(op, op_count, roi, bl);
    if(! ci.isOK()) {
        return XIL_FAILURE;
    }

    setInMolecule(op_count > 1);

    //
    // Get the next buffer space. We're going to
    // write into it directly rather than use the
    // CisBuffer addBytes function.
    //
    Xil_unsigned8* cis_addr = cbm->nextBufferSpace();
    if(cis_addr == NULL) {
        // Error should already be reported sufficiently
        return XIL_FAILURE ;
    }

    //
    // Fax compress each band of the iamge
    // Accumulate the total of bytes added to the cis
    //
    Xil_unsigned8* cis_ptr = cis_addr;
    for(unsigned int band=0; band<ci.image_nbands; band++) {
        cis_ptr += compressBand_2d(band, &ci, cis_ptr);
    }

    if(cbm->doneBufferSpace(cis_ptr - cis_addr)) {
        return XIL_SUCCESS;
    } else {
        return XIL_FAILURE;
    }

}


unsigned int
XilDeviceCompressionFaxG4::compressBand_2d(unsigned int   band,
                                           CompressInfo*  ci,
                                           Xil_unsigned8* cis_addr)
{

    //
    // ao, a1, a2 are bit indices in the current line
    // b1 and b2  are bit indices in the reference line (line above)
    // color is the current color (WHITE or BLACK)
    //
    Xil_unsigned8* ref_addr  = initial_ref;
    int            height    = ci->image_box_height;
    Xil_unsigned8* line_addr = (Xil_unsigned8*)ci->image_box_dataptr + 
                               band * ci->image_bs;

    init_bitbuf();

    //
    // Iterate over all lines
    //
    unsigned int  out_index = 0;
    while(height--) {
        int a0   = ci->image_box_offset;
        int last = a0 + ci->image_box_width;

        int a1 = (TESTBIT(line_addr, a0)) ? a0 : nextstate(line_addr, a0, last);
        int b1 = (TESTBIT(ref_addr, a0)) ? a0 : nextstate(ref_addr, a0, last);

        //
        // The current color is set to WHITE at line start
        //
        int color = WHITE;

        while(1) {
            int b2 = nextstate(ref_addr, b1, last);
            if(b2 < a1) {          // pass mode
                out_index += add_2d_bits(cis_addr, out_index, pass, 0);
                a0 = b2;
            } else {
                int tmp = b1 - a1 + 3;
                if((tmp <= 6) && (tmp >= 0)) { // vertical mode
                    out_index += add_2d_bits(cis_addr, out_index, vert, tmp);
                    a0 = a1;
                } else {            // horizontal mode
                    int a2 = nextstate(line_addr, a1, last);
                    out_index += add_2d_bits(cis_addr, out_index, horz, 0);
                    out_index += add_1d_bits(cis_addr, out_index, a1-a0, color);
                    out_index += add_1d_bits(cis_addr, out_index, a2-a1, color^1);
                    a0 = a2;
                }
            }
            if(a0 >= last) {
                break;
            }
            color = TESTBIT(line_addr, a0);
            a1 = nextstate(line_addr, a0, last);
            b1 = nextstate(ref_addr, a0, last);
            if(TESTBIT(ref_addr, b1) == color) {
                b1 = nextstate(ref_addr, b1, last);
            }
        }

        ref_addr = line_addr;
        line_addr += ci->image_ss;

    } // End while(height--)

    //
    // append eofb
    //
    out_index += add_eofb(cis_addr, out_index);

    return out_index;
}
