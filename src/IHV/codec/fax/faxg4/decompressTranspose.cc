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
//  File:       decompressTranspose.cc
//  Project:    XIL
//  Revision:   1.10
//  Last Mod:   10:14:21, 03/10/00
//
//  Description:
//
//    Molecule to decompress a fax and rotate it by some multiple
//    of 90 degrees or transpose it.
//
//
//------------------------------------------------------------------------
//    COPYRIGHT
//------------------------------------------------------------------------
#pragma ident   "@(#)decompressTranspose.cc	1.10\t00/03/10  "

#include <malloc.h>
#include <xil/xilGPI.hh>
#include "XilDeviceCompressionFaxG4.hh"
#include "XiliUtils.hh"
#include "decompressTranspose.hh"

XilStatus
XilDeviceCompressionFaxG4::decompressTranspose(XilOp*       op,
                                               unsigned int op_count,
                                               XilRoi*      roi,
                                               XilBoxList*  bl)
{
    DecompressInfo di(op, op_count, roi, bl);
    if(! di.isOK()) {
        return XIL_FAILURE;
    }

    setInMolecule(TRUE);

    //
    // Get the op_list for determining molecule components.
    // The cis will always be on op_list[op_count-1].
    //
    XilOp**   op_list = op->getOpList();

    //
    // Get the transpose operation parameters
    //
    XilImage* src_image = op_list[0]->getSrcImage();
    XilImage* dst_image = op_list[0]->getDstImage();
    XilFlipType fliptype;
    op_list[0]->getParam(1, (int*)&fliptype);

    //
    // Test that the transposed image fits into the destination
    //
    switch(fliptype) {
      case XIL_FLIP_X_AXIS:
        if(di.cis_width  != di.image_box_width || 
           di.cis_height != di.image_box_height) {
            return XIL_FAILURE;
        }
        break;
      case XIL_FLIP_Y_AXIS:
      case XIL_FLIP_180:
        //
        // We can only deal with full bytes
        //
        if(di.cis_width%8 != 0) {
            return XIL_FAILURE;
        }
        if(di.cis_width  != di.image_box_width || 
           di.cis_height != di.image_box_height) {
            return XIL_FAILURE;
        }
        break;
      case XIL_FLIP_MAIN_DIAGONAL:
      case XIL_FLIP_ANTIDIAGONAL:
      case XIL_FLIP_90:
      case XIL_FLIP_270:
        if(di.cis_width != di.image_box_height || 
           di.cis_height != di.image_box_width) {
            return XIL_FAILURE;
        }
        break;
      default:
        //
        // Unknown fliptype
        //
        return XIL_FAILURE;
    }

    if(decTransposeFrame(&di, fliptype) == 0) {
        return XIL_FAILURE;
    }

    return XIL_SUCCESS;
}

int
XilDeviceCompressionFaxG4::decTransposeFrame(DecompressInfo* di,
                                             XilFlipType     fliptype)
{
    Xil_unsigned8* bitstream_address;
    Xil_unsigned8* buffer_end;
    if (!(bitstream_address = cbm->nextFrame(&buffer_end))) {
        // Internal error
        XIL_ERROR(di->system_state, XIL_ERROR_SYSTEM,"di-100",TRUE); 
        return 0;
    }
    int buffer_size = buffer_end - bitstream_address;

    unsigned int    tblindex;        // bit group to be table decoded
    unsigned int*   base;            // current address in cis
    unsigned int*   end;             // end of current cis buffer
    unsigned int    inbits, inbits2; // 64-bit buffer for cis
    long            cis_ndx;         // bit offset from base into cis buffer
    int             runlen, code, color, bytewidth, height, shift, i;
    int             *refruns, *newruns, *storage, *runs[8], *copy;
    int             j, extra, linelen, pad;
    Xil_unsigned8   *top_left, *top_right, *bottom_left, *bottom_right;

    pad = (8 - (di->image_box_width%8))%8;
    bytewidth = di->image_box_width/8;
    if (pad)
        bytewidth++; 
    top_left = (unsigned char *)di->image_box_dataptr;
    top_right = top_left + bytewidth - 1;
    bottom_left = top_left + di->image_ss*(di->image_box_height-1);
    bottom_right = bottom_left + bytewidth - 1;

    if ((fliptype != XIL_FLIP_ANTIDIAGONAL) && (fliptype != XIL_FLIP_270))
        pad = 0;

    //
    // Clear the whole destination image
    // TODO: (lperry) This doesn't seem right, so I've commented it out.
    //       Were going to overwrite the whole dst box anyway.
    //       This also seems to be overwriting the whole parent image.
    //      
    //
    // xili_memset(di->image_box_dataptr, 0, di->image_box_height*di->image_ss);
    height = di->cis_height;
    base = (unsigned int *)((long)bitstream_address & ~3L);
    end = (unsigned int *)(((long)(bitstream_address+buffer_size)& ~3L)+4);
    cis_ndx = ((long)bitstream_address & 0x3) << 3;
    GETWORD(inbits, base, end);
    GETWORD(inbits2, base, end);
#if defined(_WINDOWS) || defined(HPUX)
    refruns = storage = (int *)malloc(sizeof(int)*di->cis_width*9);
#else
    refruns = storage = (int *)memalign(8, sizeof(int)*di->cis_width*9);
#endif
    storage[0] = di->cis_width;
    storage[1] = storage[2] = 0;
    newruns = &storage[4];
    for (j=0; j<pad; j++) {                // fill out to a multiple of 8
        runs[j] = newruns;
        newruns[0] = di->cis_width;
        newruns[1] = newruns[2] = 0;
        newruns += 3;
        }
    for (j=pad; j<height+pad; j++) {
        color = WHITE;
        runs[j%8] = newruns;
        *newruns = linelen = runlen = extra = 0;
        while(linelen < di->cis_width) {
nopass:          while (linelen < di->cis_width) {        // pass flag is false
            GETINDEX(8)
            code = table_2d[tblindex & 0xff];
            cis_ndx += CODELEN(code);
            GETMOREBITS
            switch (CODETYPE(code)) {
                case V0:
                    if (!((*(int *)&refruns ^ *(int *)&newruns) & 0x4))
                        runlen += *refruns++;
                    else if (runlen == 0) {
                            runlen = refruns[0] + refruns[1];
                            refruns += 2;
                            }
                    *newruns++ = runlen;
                    linelen += runlen;
                    runlen = extra = 0;
                    color ^= 1;
                    break;
                case V_1:
                    if (!((*(int *)&refruns ^ *(int *)&newruns) & 0x4))
                        runlen += *refruns;
                    else if (runlen == 0)
                        runlen = refruns[0] + refruns[1];
                    runlen--;
                    *newruns++ = runlen;
                    linelen += runlen;
                    BUMPREF
                    runlen = extra;
                    break;
                case V1:
                    if (!((*(int *)&refruns ^ *(int *)&newruns) & 0x4))
                        runlen += *refruns++;
                    else if (runlen == 0) {
                        runlen = refruns[0] + refruns[1];
                        refruns += 2;
                        }
                    runlen++;
                    *newruns++ = runlen;
                    linelen += runlen;
                    runlen = extra = *refruns++ - 1;
                    color ^= 1;
                    break;
                case HORIZONTAL:
                    for (i = 0; i < 2; i++) {
                        runlen = 0;
                        do {        // loop to catch repeat counts
                            GETINDEX(13)
                            code = uc_table[color][tblindex & 0x1fff];
                            runlen += CODERUN(code);
                            cis_ndx += CODELEN(code);
                            GETMOREBITS
                        } while MORERUN(code);
                        *newruns++ = runlen;
                        linelen += runlen;
                        BUMPREF
                        }
                    runlen = extra;
                    break;
                case PASS:
                    if (!((*(int *)&refruns ^ *(int *)&newruns) & 0x4)) {
                        runlen += refruns[0] + refruns[1];
                        refruns += 2;
                        }
                    else {
                        if (runlen)
                            runlen += *refruns++;
                        else {
                            runlen = refruns[0] + refruns[1] + refruns[2];
                            refruns += 3;
                            }
                        }
                    *newruns++ = runlen;
                    linelen += runlen;
                    runlen = extra = 0;
                    goto pass;
                case V_2:
                    if (!((*(int *)&refruns ^ *(int *)&newruns) & 0x4))
                        runlen += *refruns;
                    else if (runlen == 0)
                        runlen = refruns[0] + refruns[1];
                    runlen -= 2;
                    *newruns++ = runlen;
                    linelen += runlen;
                    BUMPREF
                    runlen = extra;
                    break;
                case V2:
                    if (!((*(int *)&refruns ^ *(int *)&newruns) & 0x4))
                        runlen += *refruns++;
                    else if (runlen == 0) {
                        runlen = refruns[0] + refruns[1];
                        refruns += 2;
                        }
                    runlen += 2;
                    *newruns++ = runlen;
                    linelen += runlen;
                    extra = -2;
                    while ((extra += *refruns++) < 0);
                    runlen = extra;
                    color ^= 1;
                    break;
                case V_3:
                    if (!((*(int *)&refruns ^ *(int *)&newruns) & 0x4))
                        runlen += *refruns;
                    else if (runlen == 0)
                        runlen = refruns[0] + refruns[1];
                    runlen -= 3;
                    *newruns++ = runlen;
                    linelen += runlen;
                    BUMPREF
                    runlen = extra;
                    break;
                case V3:
                    if (!((*(int *)&refruns ^ *(int *)&newruns) & 0x4))
                        runlen += *refruns++;
                    else if (runlen == 0) {
                        runlen = refruns[0] + refruns[1];
                        refruns += 2;
                        }
                    runlen += 3;
                    *newruns++ = runlen;
                    linelen += runlen;
                    extra = -3;
                    while ((extra += *refruns++) < 0);
                    runlen = extra;
                    color ^= 1;
                    break;
                default:
                    return 0;        // unsupported 2-D mode (or bitstream error)
                }
            }
pass:          while (linelen < di->cis_width) {        // pass flag is true
            GETINDEX(8)
            code = table_2d[tblindex & 0xff];
            cis_ndx += CODELEN(code);
            GETMOREBITS
            switch (CODETYPE(code)) {
                case V0:
                    runlen += *refruns++;
                    newruns[-1] += runlen;
                    linelen += runlen;
                    runlen = extra = 0;
                    color ^=1;
                    goto nopass;
                case V_1:
                    runlen += *refruns - 1;
                    newruns[-1] += runlen;
                    linelen += runlen;
                    BUMPREF
                    runlen = extra;
                    goto nopass;
                case V1:
                    runlen += *refruns++ + 1;
                    newruns[-1] += runlen;
                    linelen += runlen;
                    runlen = extra = *refruns++ - 1;
                    color ^= 1;
                    goto nopass;
                case HORIZONTAL:
                    runlen = 0;
                    do {        // loop to catch repeat counts
                        GETINDEX(13)
                        code = uc_table[color][tblindex & 0x1fff];
                        runlen += CODERUN(code);
                        cis_ndx += CODELEN(code);
                        GETMOREBITS
                    } while MORERUN(code);
                    newruns[-1] += runlen;
                    linelen += runlen;
                    BUMPREF
                    runlen = 0;
                    do {        // loop to catch repeat counts
                        GETINDEX(13)
                        code = uc_table[color][tblindex & 0x1fff];
                        runlen += CODERUN(code);
                        cis_ndx += CODELEN(code);
                        GETMOREBITS
                    } while MORERUN(code);
                    *newruns++ = runlen;
                    linelen += runlen;
                    BUMPREF
                    runlen = extra;
                    goto nopass;
                case PASS:
                    runlen += refruns[0] + refruns[1];
                    refruns += 2;
                    newruns[-1] += runlen;
                    linelen += runlen;
                    runlen = extra = 0;
                    break;
                case V_2:
                    runlen += *refruns - 2;
                    newruns[-1] += runlen;
                    linelen += runlen;
                    BUMPREF
                    runlen = extra;
                    goto nopass;
                case V2:
                    runlen += *refruns++ + 2;
                    newruns[-1] += runlen;
                    linelen += runlen;
                    extra = -2;
                    while ((extra += *refruns++) < 0);
                    runlen = extra;
                    color ^= 1;
                    goto nopass;
                case V_3:
                    runlen += *refruns - 3;
                    newruns[-1] += runlen;
                    linelen += runlen;
                    BUMPREF
                    runlen = extra;
                    goto nopass;
                case V3:
                    runlen += *refruns++ + 3;
                    newruns[-1] += runlen;
                    linelen += runlen;
                    extra = -3;
                    while ((extra += *refruns++) < 0);
                    runlen = extra;
                    color ^= 1;
                    goto nopass;
                default:
                    return 0;        // unsupported 2-D mode (or bitstream error)
                }
            }
          }
        *newruns++ = 0;
        *newruns++ = 0;
        if ((long)newruns & 0x4)
            newruns++;
        if (j%8 == 7) {
            refruns = newruns;
            copy = runs[7];
            while (copy < newruns)
                *refruns++ = *copy++;
            refruns = newruns;
            newruns = storage;

            switch (fliptype) {
                case XIL_FLIP_X_AXIS:
                    strip_x (runs, bottom_left-(j-7)*di->image_ss, di->image_ss, 8);
                    break;
                case XIL_FLIP_Y_AXIS:
                    strip_y (runs, top_right+(j-7)*di->image_ss, di->image_ss, 8);
                    break;
                case XIL_FLIP_180:
                    strip_180 (runs, bottom_right-(j-7)*di->image_ss, di->image_ss, 8);
                    break;
                case XIL_FLIP_MAIN_DIAGONAL:
                    strip_d (runs, di->image_box_height, top_left+((j-7)>>3), di->image_ss);
                    break;
                case XIL_FLIP_ANTIDIAGONAL:
                    strip_a (runs, di->image_box_height, bottom_right-((j-7)>>3), di->image_ss);
                    break;
                case XIL_FLIP_90:
                    strip_90 (runs, di->image_box_height, bottom_left+((j-7)>>3), di->image_ss);
                    break;
                case XIL_FLIP_270:
                    strip_270 (runs, di->image_box_height, top_right-((j-7)>>3), di->image_ss);
                    break;
                }
            }
        else
            refruns = runs[j%8];
        }
    if (i = (height+pad)%8) {
        for (j=i; j<8; j++) {        // fill out to a multiple of 8
            runs[j] = newruns;
            newruns[0] = di->cis_width;
            newruns[1] = newruns[2] = 0;
            newruns += 3;
        }

        switch (fliptype) {
            case XIL_FLIP_X_AXIS:
                strip_x (runs, top_left+(i-1)*di->image_ss, di->image_ss, i);
                break;
            case XIL_FLIP_Y_AXIS:
                strip_y (runs, bottom_right-(i-1)*di->image_ss, di->image_ss, i);
                break;
            case XIL_FLIP_180:
                strip_180 (runs, top_right+(i-1)*di->image_ss, di->image_ss, i);
                break;
            case XIL_FLIP_MAIN_DIAGONAL:
                strip_d (runs, di->image_box_height, top_right, di->image_ss);
                break;
            case XIL_FLIP_ANTIDIAGONAL:
                // Padding should already have occurred
                break;
            case XIL_FLIP_90:
                strip_90 (runs, di->image_box_height, bottom_right, di->image_ss);
                break;
            case XIL_FLIP_270:
                // Padding should already have occurred
                break;
        }
    }

    free(storage);

    unsigned long bitstream_size =  ((Xil_unsigned8*)base - 8 
                                   - bitstream_address + (cis_ndx >> 3)
                                   + (cis_ndx & 0x7 ? 1 : 0) + EOFB4);


        if (bitstream_size != 0) {
            bitstream_address += bitstream_size;
                // add in the band stride
            }
        else {
            // Internal error
            XIL_ERROR(di->system_state, XIL_ERROR_SYSTEM,"di-95",TRUE);
            return 0;
            }

        cbm->decompressedFrame(bitstream_address);
        return 1;
    }
