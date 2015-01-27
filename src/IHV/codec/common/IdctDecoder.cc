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
//This line lets emacs recognize this as -*- C++ -*- Code
//------------------------------------------------------------------------
//
//  File:       IdctDecoder.cc
//  Project:    XIL
//  Revision:   1.5
//  Last Mod:   10:16:07, 03/10/00
//
//  Description:
//
//    TODO: Enter some descriptive text here
//
//
//------------------------------------------------------------------------
//    COPYRIGHT
//------------------------------------------------------------------------
#pragma ident   "@(#)IdctDecoder.cc	1.5\t00/03/10  "

#include "xil/xilGPI.hh"
#include "IdctDecoder.hh"


void 
IdctDecoder::free_decode_table(int *decode_table)
{
    //
    // First check this table for any "pointers" to 2nd level decode table
    //
    for(int i=0; i<256; i++) {
        if((decode_table[i] & 0x3) == 0) {
            free_decode_table((int*)decode_table[i]);
        }
    }
    // now free this table
    delete decode_table;
}

int* 
IdctDecoder::create_decode_table(int*   hptr, 
                                 Codes* huffman, 
                                 int    size)
{
    //
    // If hptr == 0x03 then this is a recursive stage of the function
    // meaning that the huffman bit length is greater than 8.
    //
    int i;
    if(hptr == NULL || hptr == ((int *)0x03)) {
        hptr = new int[256];
        if(hptr == NULL) {
            XIL_ERROR(NULL,XIL_ERROR_RESOURCE,"di-1",TRUE);
            return NULL;
        }
        for(i=0; i<256; i++) {
            hptr[i] = 3;
        }
    }

    for(i=0; i<size; i++) {
        int code = huffman[i].code;
        int bits = huffman[i].length;
        int valu = huffman[i].value;

        if(bits == 0) {
            continue;
        } else if(bits <= 8) {
            int xbits = 8 - bits;
            int xcount = 1 << xbits;
            code = code << xbits;
            for(int j=0; j<xcount; j++) {
                if(hptr[code] != 3) {
                    // ERROR: clash, this location already loaded with value
                    XIL_ERROR(NULL,XIL_ERROR_SYSTEM,"di-313",TRUE);
                    return NULL;  
                }
                hptr[code] = (bits<<24) | ((valu & 0xffff)<<2) | 1;
                code += 1;
            }
        } else {
            int vbits = code >> (bits-8);
            Codes temp;
            temp.code   = code & ((1<<bits-8)-1);
            temp.length = bits - 8;
            temp.value  = valu;
            hptr[vbits] = (int) create_decode_table((int *)hptr[vbits],
                                                    &temp, 1);
            if(hptr[vbits] == 0) {
                return NULL;
            }

        }
    }
    return hptr;
}

int 
IdctDecoder::decode(int *ptr)
{
    int                bits;
    int                result;
    int                valu;
    int                size;

    do {
        GETBITS(8,bits);
        valu = ptr[bits];
        result = valu & 0x03;
        if(result == 0x01) {
            break;
        }
        if(result == 0x00) {
            ptr = (int*)valu;
        } else if(result & 0x02) {
            // Uninitialized entry on the Huffman table.
            // This means that we have an ilegal bitstream.
            // Return EOB_CODE and keep on decoding
            if(result == 0x03) {
                return INVALID_DECODE_RETURN;
            }
            if(result == 0x02) {
                // Entry not legal.
                // The way I see how the Huffman tables are created
                // this entry can never occurr.  Scott will have to
                // into this.
                return INVALID_DECODE_RETURN;
            }
        }
    } while(1);

    result = (valu>>2) & 0xffff;
    size = 8 - (valu >> 24);
    putbits(size,(bits & ((1<<(size))-1)));
    return result;

    ErrorReturn:
    //
    // First, let's see if the remaining bits in the 'savedBits' buffer
    // can be used as a Huffman code.  If not, then return the error
    // condition.  We might only need to use some of the 'savedBits'.
    //
    if(nbits > 0) {
        valu = ptr[savedBits<<(8-nbits)];
        if(((valu & 0x3) == 0x01) && ((valu >> 24) <= nbits)) {
            nbits -= valu >> 24;
            savedBits = savedBits & ((1<<nbits) -1);
            return (valu>>2) & 0xffff;;
        }
    }
    return INVALID_DECODE_RETURN;
}

