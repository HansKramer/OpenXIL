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
//  Last Mod:   10:14:20, 03/10/00
//
//  Description:
//
//    Fax G4 decompressor
//
//
//  MT-level:  <??????>
//
//------------------------------------------------------------------------
//  COPYRIGHT
//------------------------------------------------------------------------
#pragma ident   "@(#)decompress.cc	1.14\t00/03/10  "

#include <malloc.h>
#include <xil/xilGPI.hh>
#include "XilDeviceCompressionFaxG4.hh"
#include "XiliUtils.hh"

XilStatus
XilDeviceCompressionFaxG4::decompress(XilOp*     op,
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
XilDeviceCompressionFaxG4::decompressFrame(DecompressInfo* di)
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
        int size = decompressBand_2d(band, di, cis_addr, cis_end);
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
XilDeviceCompressionFaxG4::decompressBand_2d(unsigned int    band,
                                             DecompressInfo* di,
                                             Xil_unsigned8*  cis_addr,
                                             Xil_unsigned8*  cis_end)
{
    unsigned int    outbits;          // bit buffer for output image
    unsigned int    tblindex;         // 13 bit index into lookup
    unsigned int    inbits, inbits2;  // 64-bit cis buffer

    unsigned int    allblack = 0xff;
    int             shift, i;
    int*            storage[2];
    int             toggle=0;

    Xil_unsigned8* line_addr = (Xil_unsigned8*)di->image_box_dataptr + 
                               band*di->image_bs;
    int height = di->image_box_height;
    Xil_unsigned8* ref_addr = initial_ref;

    //
    // Calculate 32 bit aligned start and end words
    //
    unsigned int*  base = (unsigned int *)((int)cis_addr & ~3);
    unsigned int*  end  = (unsigned int *)((int)(cis_end) & ~3) + 1;

    int cis_ndx = ((int)cis_addr & 0x3) << 3;
    GETWORD(inbits, base, end);
    GETWORD(inbits2, base, end);
#if defined(_WINDOWS) || defined (HPUX)
    storage[0] = (int *)malloc(sizeof(int)*di->image_box_width);
    storage[1] = (int *)malloc(sizeof(int)*di->image_box_width);
#else
    storage[0] = (int *)memalign(8, sizeof(int)*di->image_box_width);
    storage[1] = (int *)memalign(8, sizeof(int)*di->image_box_width);
#endif
    storage[0][0] = di->image_box_width;
    storage[0][1] = storage[0][2] = 0;

    while(height--) {
        //
        // Clear the output line to white
        // TODO: Isn't this redundant. The decompressor should
        //       fill with zeroes anyway. If it doesn't we should
        //       be able to take advantage of the pre-cleared area
        //       and skip writing the white runs.
        //
        xili_bit_setvalue(line_addr, 0, di->image_box_width, 
                          di->image_box_offset);

        int color = WHITE;
        int* refruns = storage[toggle];
        toggle ^= 0x1;
        int* newruns = storage[toggle];
        Xil_unsigned8* byte = line_addr;
        int out_ndx = di->image_box_offset;
        int linelen = 0;
        int runlen = 0;
        int extra = 0;
        *newruns = 0;

        //
        // TODO : Comments please !!!
        //
        int code;
        while(linelen < di->image_box_width) {
            nopass:   while(linelen < di->image_box_width) { // pass flag is false
                GETINDEX(8)
                code = table_2d[tblindex & 0xff];
                cis_ndx += CODELEN(code);
                GETMOREBITS
                switch (CODETYPE(code)) {
                  case V0:
                    if(!((*(int *)&refruns ^ *(int *)&newruns) & 0x4)) {
                        runlen += *refruns++;
                    } else if(runlen == 0) {
                        runlen = refruns[0] + refruns[1];
                        refruns += 2;
                    }
                    *newruns++ = runlen;
                    linelen += runlen;
                    WRITERUN
                    runlen = extra = 0;
                    color ^= 1;
                    break;

                  case V_1:
                    if(!((*(int *)&refruns ^ *(int *)&newruns) & 0x4)) {
                        runlen += *refruns;
                    } else if(runlen == 0) {
                        runlen = refruns[0] + refruns[1];
                    }
                    runlen--;
                    *newruns++ = runlen;
                    linelen += runlen;
                    WRITERUN
                    BUMPREF
                    runlen = extra;
                    break;

                  case V1:
                    if(!((*(int *)&refruns ^ *(int *)&newruns) & 0x4)) {
                        runlen += *refruns++;
                    } else if(runlen == 0) {
                        runlen = refruns[0] + refruns[1];
                        refruns += 2;
                    }
                    runlen++;
                    *newruns++ = runlen;
                    linelen += runlen;
                    WRITERUN
                    runlen = extra = *refruns++ - 1;
                    color ^= 1;
                    break;

                  case HORIZONTAL:
                    for(i = 0; i < 2; i++) {
                        runlen = 0;
                        do {    // loop to catch repeat counts
                            GETINDEX(13)
                            code = uc_table[color][tblindex & 0x1fff];
                            runlen += CODERUN(code);
                            cis_ndx += CODELEN(code);
                            GETMOREBITS
                        } while MORERUN(code);
                        *newruns++ = runlen;
                        linelen += runlen;
                        WRITERUN
                        BUMPREF
                    }
                    runlen = extra;
                    break;

                  case PASS:
                    if(!((*(int *)&refruns ^ *(int *)&newruns) & 0x4)) {
                        runlen += refruns[0] + refruns[1];
                        refruns += 2;
                    } else {
                        if(runlen) {
                            runlen += *refruns++;
                        } else {
                            runlen = refruns[0] + refruns[1] + refruns[2];
                            refruns += 3;
                        }
                    }
                    *newruns++ = runlen;
                    linelen += runlen;
                    WRITERUN
                    runlen = extra = 0;
                    goto pass;

                  case V_2:
                    if(!((*(int *)&refruns ^ *(int *)&newruns) & 0x4)) {
                        runlen += *refruns;
                    } else if(runlen == 0) {
                        runlen = refruns[0] + refruns[1];
                    }
                    runlen -= 2;
                    *newruns++ = runlen;
                    linelen += runlen;
                    WRITERUN
                    BUMPREF
                    runlen = extra;
                    break;

                  case V2:
                    if(!((*(int *)&refruns ^ *(int *)&newruns) & 0x4)) {
                        runlen += *refruns++;
                    } else if(runlen == 0) {
                        runlen = refruns[0] + refruns[1];
                        refruns += 2;
                    }
                    runlen += 2;
                    *newruns++ = runlen;
                    linelen += runlen;
                    WRITERUN
                    extra = -2;
                    while((extra += *refruns++) < 0);
                    runlen = extra;
                    color ^= 1;
                    break;

                  case V_3:
                    if(!((*(int *)&refruns ^ *(int *)&newruns) & 0x4)) {
                        runlen += *refruns;
                    } else if(runlen == 0) {
                        runlen = refruns[0] + refruns[1];
                    }
                    runlen -= 3;
                    *newruns++ = runlen;
                    linelen += runlen;
                    WRITERUN
                    BUMPREF
                    runlen = extra;
                    break;

                  case V3:
                    if(!((*(int *)&refruns ^ *(int *)&newruns) & 0x4)) {
                        runlen += *refruns++;
                    } else if(runlen == 0) {
                        runlen = refruns[0] + refruns[1];
                        refruns += 2;
                    }
                    runlen += 3;
                    *newruns++ = runlen;
                    linelen += runlen;
                    WRITERUN
                    extra = -3;
                    while((extra += *refruns++) < 0);
                    runlen = extra;
                    color ^= 1;
                    break;

                  case UNCOMPRESSED:
                    GETINDEX(2)
                    cis_ndx += 2;
                    GETMOREBITS
                    if(tblindex & 0x3) {
                        return 0;   // undefined extension code
                    }

                    // we're in uncompressed mode

                    break;

                    default:
                    return 0;   // unsupported 2-D mode (or bitstream error)
                }
            }

            pass:     while(linelen < di->image_box_width) { // pass flag is true
                GETINDEX(8)
                code = table_2d[tblindex & 0xff];
                cis_ndx += CODELEN(code);
                GETMOREBITS
                switch (CODETYPE(code)) {
                  case V0:
                    runlen += *refruns++;
                    newruns[-1] += runlen;
                    linelen += runlen;
                    WRITERUN
                    runlen = extra = 0;
                    color ^=1;
                    goto nopass;

                  case V_1:
                    runlen += *refruns - 1;
                    newruns[-1] += runlen;
                    linelen += runlen;
                    WRITERUN
                    BUMPREF
                    runlen = extra;
                    goto nopass;

                  case V1:
                    runlen += *refruns++ + 1;
                    newruns[-1] += runlen;
                    linelen += runlen;
                    WRITERUN
                    runlen = extra = *refruns++ - 1;
                    color ^= 1;
                    goto nopass;

                  case HORIZONTAL:
                    runlen = 0;
                    do {    // loop to catch repeat counts
                        GETINDEX(13)
                        code = uc_table[color][tblindex & 0x1fff];
                        runlen += CODERUN(code);
                        cis_ndx += CODELEN(code);
                        GETMOREBITS
                    } while MORERUN(code);
                    newruns[-1] += runlen;
                    linelen += runlen;
                    WRITERUN
                    BUMPREF
                    runlen = 0;
                    do {    // loop to catch repeat counts
                        GETINDEX(13)
                        code = uc_table[color][tblindex & 0x1fff];
                        runlen += CODERUN(code);
                        cis_ndx += CODELEN(code);
                        GETMOREBITS
                    } while MORERUN(code);
                    *newruns++ = runlen;
                    linelen += runlen;
                    WRITERUN
                    BUMPREF
                    runlen = extra;
                    goto nopass;

                  case PASS:
                    runlen += refruns[0] + refruns[1];
                    refruns += 2;
                    newruns[-1] += runlen;
                    linelen += runlen;
                    WRITERUN
                    runlen = extra = 0;
                    break;

                  case V_2:
                    runlen += *refruns - 2;
                    newruns[-1] += runlen;
                    linelen += runlen;
                    WRITERUN
                    BUMPREF
                    runlen = extra;
                    goto nopass;

                  case V2:
                    runlen += *refruns++ + 2;
                    newruns[-1] += runlen;
                    linelen += runlen;
                    WRITERUN
                    extra = -2;
                    while((extra += *refruns++) < 0);
                    runlen = extra;
                    color ^= 1;
                    goto nopass;

                  case V_3:
                    runlen += *refruns - 3;
                    newruns[-1] += runlen;
                    linelen += runlen;
                    WRITERUN
                    BUMPREF
                    runlen = extra;
                    goto nopass;

                  case V3:
                    runlen += *refruns++ + 3;
                    newruns[-1] += runlen;
                    linelen += runlen;
                    WRITERUN
                    extra = -3;
                    while((extra += *refruns++) < 0);
                    runlen = extra;
                    color ^= 1;
                    goto nopass;

                    default:
                    return 0;   // unsupported 2-D mode (or bitstream error)
                }
            }
        }
        *byte = outbits;    // flush last byte
        *newruns++ = 0;
        *newruns = 0;
        ref_addr = line_addr;
        line_addr += di->image_ss;
    }
    free(storage[0]);
    free(storage[1]);
    return ((int)(base) - 8 - (int)cis_addr + (cis_ndx >> 3)
            + (cis_ndx & 0x7 ? 1 : 0) + EOFB4);
}
