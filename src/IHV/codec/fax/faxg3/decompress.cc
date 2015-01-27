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
//  File:       decompress.cc
//  Project:    XIL
//  Revision:   1.14
//  Last Mod:   10:14:12, 03/10/00
//
//  Description:
//
//    A CCITT group 3 fax decompressor derived from kharp's Toy compressor.
//
//
//------------------------------------------------------------------------
//    COPYRIGHT
//------------------------------------------------------------------------
#pragma ident   "@(#)decompress.cc	1.14\t00/03/10  "


#include <xil/xilGPI.hh>
#include "XilDeviceCompressionFaxG3.hh"
#include "XiliUtils.hh"
#include "xili_codec_utils.hh"
#include "DecompressInfo.hh"

XilStatus
XilDeviceCompressionFaxG3::decompress(XilOp*     op,
                                    unsigned int op_count,
                                    XilRoi*      roi,
                                    XilBoxList*  bl)
{
    //
    // Roll all of the info into a DecompressInfo obhject
    //
    DecompressInfo di(op, op_count, roi, bl);
    if(! di.isOK()) {
        return XIL_FAILURE;
    }

    //
    // Seek to the proper frame. Deferred execution may have
    // caused other frames to be decompressed before this one.
    //
    seek(di.frame_number);

    if(cisFitsInDst(roi)) {

        //
        // Decompress directly to the dst image
        //
        return decompressFrame(&di);

    } else {
        //
        // Create a temporary buffer to decompress into.
        //
        DecompressInfo tmp_di(&di);
        if(! tmp_di.isOK()) {
            return XIL_FAILURE;
        }

        //
        // Decompress (into the temporary buffer)
        //
        if(decompressFrame(&tmp_di) == XIL_FAILURE) {
            XIL_ERROR(getSystemState(), XIL_ERROR_RESOURCE, "di-1", FALSE);
            return XIL_FAILURE;
        }

        //
        // Copy the temporary to the dst image, using the Rectlist.
        //
        di.copyRects(&tmp_di);
    }

    return XIL_SUCCESS;
}

XilStatus
XilDeviceCompressionFaxG3::decompressFrame(DecompressInfo* di)
{
    //
    // Calculate the size of the compressed bitmap
    //
    Xil_unsigned8* cis_end;
    Xil_unsigned8* cis_addr = cbm->nextFrame(&cis_end);
    if(cis_addr == NULL) {
        XIL_ERROR(system_state, XIL_ERROR_SYSTEM,"di-100",TRUE); 
        return XIL_FAILURE;
    }

    //
    // Decompress each band of the image
    //
    for(unsigned int band=0; band<di->image_nbands; band++) {
        int size = decompressBand(band, di, cis_addr, cis_end);
        if(size > 0) {
            cis_addr += size;
        } else {        // Internal error
            XIL_ERROR(system_state, XIL_ERROR_SYSTEM,"di-95",TRUE); 
            return XIL_FAILURE;
        }
    }

    //
    // Report that we've decompressed the frame
    //
    cbm->decompressedFrame(cis_addr);

    return XIL_SUCCESS;
}  


int
XilDeviceCompressionFaxG3::decompressBand(unsigned int    band,
                                          DecompressInfo* di,
                                          Xil_unsigned8*  cis_addr,
                                          Xil_unsigned8*  cis_end)
{
    unsigned int    tblindex;         // 13 bit index into lookup
    unsigned int    inbits, inbits2;  // 64-bit cis buffer

    unsigned int    testbit, allblack = 0xff;
    int             runlen, code, shift;

    Xil_unsigned8* line_addr = (Xil_unsigned8*)di->image_box_dataptr + 
                               band*di->image_bs;

    //
    // Calculate 32 bit aligned start and end words
    //
    unsigned int*  base      = (unsigned int *)((int)cis_addr & ~3);
    unsigned int*  end       = (unsigned int *)((int)(cis_end) & ~3) + 1;

    int cis_ndx   = find_eol(base, end, ((int)cis_addr & 0x3) << 3);
    base     += cis_ndx >> 5;
    cis_ndx  &= 0x1f;
    GETWORD(inbits, base, end);
    GETWORD(inbits2, base, end);

    //
    // Decompress all lines 
    //
    for(unsigned int line=0; line<di->image_box_height; line++) {
        //
        // Clear the output line to white
        // TODO: Isn't this redundant. The decompressor should 
        //       fill with zeroes anyway. If it doesn't we should
        //       be able to take advantage of the pre-cleared area
        //       and skip writing the white runs.
        //
        xili_bit_setvalue(line_addr, 0, di->image_box_width, 
                          di->image_box_offset);
        
        int            color = WHITE;
        Xil_unsigned8* byte  = line_addr;
        unsigned int   outbits = 0;
        int out_ndx = di->image_box_offset;

        while (1) {        // pull token, decode runlenth, and write run
            GETINDEX(13)
            if ((tblindex & 0x1fff) < 32)        // exit loop on EOL
                break;
            code = ss_table[color][tblindex & 0x1fff];
            if (IS_MULTI(code)) {                // superscalar decompression
                cis_ndx += MULTILEN(code);
                color = NEXTCOLOR(code);
                WRITEMULTIRUN
                }
            else {                                // decode single run
                cis_ndx += CODELEN(code);
                runlen = CODERUN(code);
                WRITERUN
                if (!MORERUN(code))
                    color ^= 0x1;
                }
            GETMOREBITS
            }
        *byte = outbits;                        // write last byte
        STEPOVER_EOL
        line_addr += di->image_ss;
        }
    STEPOVER_EOFB3
    return ((int)base - (int)cis_addr - 8 + (cis_ndx >> 3));
}
