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
//  Last Mod:   10:14:14, 03/10/00
//
//  Description:
//
//    A CCITT group 3 fax compressor derived from kharp's Toy compressor.
//
//
//------------------------------------------------------------------------
//    COPYRIGHT
//------------------------------------------------------------------------
#pragma ident   "@(#)compress.cc	1.7\t00/03/10  "

#include <xil/xilGPI.hh>
#include "XilDeviceCompressionFaxG3.hh"
#include "CompressInfo.hh"

XilStatus
XilDeviceCompressionFaxG3::compress(XilOp*       op,
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
        cis_ptr += compressBand(band, &ci, cis_ptr);
    }

    if(cbm->doneBufferSpace(cis_ptr - cis_addr)) {
        return XIL_SUCCESS;
    } else {
        return XIL_FAILURE;
    }

    
}


unsigned int
XilDeviceCompressionFaxG3::compressBand(unsigned int   band,
                                        CompressInfo*  ci,
                                        Xil_unsigned8* cis_addr)
{
    //
    // Add EOL for syncing
    //
    unsigned int   out_index = 0;
    unsigned int   ht        = ci->image_box_height;
    Xil_unsigned8* line_addr = (Xil_unsigned8*)ci->image_box_dataptr +
                               band * ci->image_bs;

    //
    // Add EOL fo syncing
    //
    out_index = add_eol(cis_addr, out_index);
    
    init_bitbuf();

    //
    // Iterate over all lines
    //
    unsigned int        srcbits;
    unsigned int        cisbits;
    while(ht--) {
        int bit_index     = ci->image_box_offset;
        int last          = bit_index + ci->image_box_width;
        int current_color = BLACK;

        //
        // Is first pixel black
        //
        if(TESTBIT(line_addr, bit_index)) {
            out_index += add_1d_bits(cis_addr, out_index, 0, WHITE);
        } else {
            current_color = WHITE;
        }

        //
        // Run-length encode line
        //
        while(bit_index < last) {
            srcbits = grab_12(line_addr, bit_index);
            if((last-bit_index >= 12) && 
                (cisbits = cmpr_table[srcbits])) {
                bits |= cisbits >> ndex;
                ndex += CIS_INC(cisbits);
                bits &= 0xffffffff << (32-ndex);

                //
                // Flush all pending bits
                //
                while (ndex > 7) {
                    cis_addr[out_index++] = (char)(bits >> 24);
                    bits <<= 8;
                    ndex -= 8;
                }
                bit_index += SRC_INC(cisbits);

            } else {

                unsigned int bitcount;
                current_color = (srcbits >> 11) & 0x1;     
                bitcount = nextstate(line_addr, bit_index, last) - bit_index;
                out_index += add_1d_bits(cis_addr, out_index, 
                                         bitcount, current_color);
                bit_index += bitcount;
                    
            }

        } // End while(bit_index < last)

        out_index += add_eol(cis_addr, out_index);
        line_addr += ci->image_ss;

    } // End while(ht--)

    //
    // Add terminating sequence
    //
    for(int count=0; count<5; count++){
        out_index += add_eol(cis_addr, out_index);
    }

    return out_index;
}
