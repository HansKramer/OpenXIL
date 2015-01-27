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
//  File:	xili_bit_utils.cc
//  Project:	XIL
//  Revision:	1.9
//  Last Mod:	10:16:31, 03/10/00
//
//  Description:
//	Highly accelerated bit primitives.
//	
//------------------------------------------------------------------------
//
//	COPYRIGHT
//
//------------------------------------------------------------------------
#pragma ident	"@(#)xili_bit_utils.cc	1.9\t00/03/10  "

#include "XiliUtils.hh"

//
//  Masks to clear least significant bits without altering most significant
//  bits (byte & mask[i]); index is number of bits to clear.
//
static Xil_unsigned8 clr_lsb_mask[8] = {
    0xFF, 0xFE, 0xFC, 0xF8, 0xF0, 0xE0, 0xC0, 0x80 
};

//
//  Masks to clear most significant bits without altering least significant
//  bits (byte & mask[i]); index is number of bits to clear.
//
static Xil_unsigned8 clr_msb_mask[8] = { 
    0xFF, 0x7F, 0x3F, 0x1F, 0x0F, 0x07, 0x03, 0x01
};

#define SETVALUE_CUTOFF    7
#define ZEROCHECK_CUTOFF   7

void
xili_bit_setvalue(Xil_unsigned8* dst_scanline, 
                  unsigned int   value,
                  unsigned int   xsize,
                  unsigned int   dst_offset)
{
    //
    //  Too small for overhead?
    //
    if(xsize < SETVALUE_CUTOFF) {
        for(unsigned int j=0; j<xsize; j++) {
            if(value) {
                XIL_BMAP_SET(dst_scanline, dst_offset+j);
            } else {
                XIL_BMAP_CLR(dst_scanline, dst_offset+j);
            }
        }
    } else {
        if(value) {
            value = 0xff;
        }
        
        //
        //  Do we have a bit offset?
        //
        if(dst_offset) {
            //
            //  Set first bits based on offset
            //

            //
            //  TODO: 9/5/96 jlf  Compiler bug causes code below to fail.
            //
#ifdef XILI_BIT_SETVALUE_COMPILER_BUG_FIXED
            *dst_scanline++ = 
                ((*dst_scanline) & clr_lsb_mask[8 - dst_offset]) |
                (value & clr_msb_mask[dst_offset]);
#else
            Xil_unsigned8 val =
                ((*dst_scanline) & clr_lsb_mask[8 - dst_offset]) |
                (value & clr_msb_mask[dst_offset]);

            *dst_scanline = val;

            dst_scanline++;
#endif
            
            xsize -= (8 - dst_offset);
        }

        unsigned int set_length = (xsize>>3);
        unsigned int set_tail   = (xsize & 0x7);
        
        xili_memset(dst_scanline, value, set_length);
        
        if(set_tail) {
            Xil_unsigned8* tmp_dst = dst_scanline + set_length;
            
            *tmp_dst =
                ((*tmp_dst) & clr_msb_mask[set_tail]) |
                (value & clr_lsb_mask[8 - set_tail]);
        }
    }
}

void
xili_bit_memcpy(Xil_unsigned8* src_scanline, 
                Xil_unsigned8* dst_scanline,
                unsigned int   xsize,
                unsigned int   src_offset,
                unsigned int   dst_offset)
{
    //
    //  Do we have bit offsets?
    //
    if(!src_offset && !dst_offset) {
        unsigned int cpy_length = (xsize >> 3);
        unsigned int src_tail   = (xsize & 0x7);

        if(cpy_length) {
            xili_memcpy(dst_scanline, src_scanline, cpy_length);
        }
        
        if(src_tail) {
            Xil_unsigned8* tmp_src = src_scanline + cpy_length;
            Xil_unsigned8* tmp_dst = dst_scanline + cpy_length;
            
            *tmp_dst =
                ((*tmp_dst) & clr_msb_mask[src_tail]) |
                ((*tmp_src) & clr_lsb_mask[8 - src_tail]);
        }
    } else {
        //
        //  Check if too small for general case (must be more than 32 bits)...
        //
        if(xsize <= 32) {
            for(unsigned int j=0; j<xsize; j++) {
                if(XIL_BMAP_TST(src_scanline, src_offset+j)) {
                    XIL_BMAP_SET(dst_scanline, dst_offset+j);
                } else {
                    XIL_BMAP_CLR(dst_scanline, dst_offset+j);
                }
            }
        } else {
            //
            //  General case for handling bit offsets...
            //
            unsigned int first_full_byte = (dst_offset + 7)/8;
            unsigned int xtra_bits_L     = ((8 - dst_offset) & 0x7);

            //
            //  Find the number of bits until the start of a long
            //
            unsigned int nbits_L         = xtra_bits_L +
                (((unsigned int)(dst_scanline+first_full_byte+3)&(~0x3)) - 
                 (unsigned int)(dst_scanline+first_full_byte)) * 8;

            unsigned int nlongs  = (xsize - nbits_L)/32;

            //
            //  Since our center algorithm handles 2 longs at a time 
            //  (n and n+1), we can't handle fewer than 2 longs.
            //
            if(nlongs < 2) {
                nlongs = 0;
            }
            unsigned int nbits_R = xsize - nbits_L - nlongs*32;

            //
            //  Get the 32-bit aligned start for src and dst
            //
            unsigned int* src_lstart = (unsigned int*)
                ((unsigned int)(src_scanline + (src_offset+nbits_L)/8) & (~0x3));
            unsigned int* dst_lstart = (unsigned int*)
                ((unsigned int)(dst_scanline + (dst_offset+nbits_L)/8));

            unsigned int  shiftv = (src_offset + nbits_L) -
                (((unsigned int)src_lstart - (unsigned int)src_scanline))*8;

            //
            //  Process any bits on left side if needed
            //
            if(nbits_L) {
                for(unsigned int i=0; i<nbits_L; i++) {
                    if(XIL_BMAP_TST(src_scanline, src_offset+i)) {
                        XIL_BMAP_SET(dst_scanline, dst_offset+i);
                    } else {
                        XIL_BMAP_CLR(dst_scanline, dst_offset+i);
                    }
                }
            }

            //
            //  Do center part.
            //
            if(shiftv > 0) {
                //
                //  Shift left to dst
                //
#ifdef XIL_LITTLE_ENDIAN
                for(unsigned int i=nbits_L; i<xsize - nbits_R; i++) {
                    if(XIL_BMAP_TST(src_scanline, src_offset+i)) {
                        XIL_BMAP_SET(dst_scanline, dst_offset+i);
                    } else {
                        XIL_BMAP_CLR(dst_scanline, dst_offset+i);
                    }
                }
#else
                for(int j=0; j<nlongs; j++) {
                    dst_lstart[j] =
                        ((src_lstart[j]) << shiftv) |
                        ((src_lstart[j+1]) >> (32-shiftv));
                }
#endif
            } else {
                for(unsigned int j=0; j<nlongs; j++) {
                    dst_lstart[j] = src_lstart[j];
                }
            }

            //
            //  Handle the right side
            //
            if(nbits_R) {
                for(unsigned int i=xsize - nbits_R; i<xsize; i++) {
                    if(XIL_BMAP_TST(src_scanline, src_offset+i)) {
                        XIL_BMAP_SET(dst_scanline, dst_offset+i);
                    } else {
                        XIL_BMAP_CLR(dst_scanline, dst_offset+i);
                    }
                }
            }
        }
    }
}

void
xili_bit_not(Xil_unsigned8* src_scanline, 
             Xil_unsigned8* dst_scanline,
             unsigned int   xsize,
             unsigned int   src_offset,
             unsigned int   dst_offset)
{
    //
    //  Check if too small for general case (must be more than 32 bits)...
    //
    if(xsize <= 32) {
        for(unsigned int j=0; j<xsize; j++) {
            if(XIL_BMAP_TST(src_scanline, src_offset+j)) {
                XIL_BMAP_CLR(dst_scanline, dst_offset+j);
            } else {
                XIL_BMAP_SET(dst_scanline, dst_offset+j);
            }
        }
    } else {
        //
        //  General case for handling bit offsets...
        //
        unsigned int first_full_byte = (dst_offset + 7)/8;
        unsigned int xtra_bits_L     = ((8 - dst_offset) & 0x7);

        //
        //  Find the number of bits until the start of a long
        //
        unsigned int nbits_L         = xtra_bits_L +
            (((unsigned int)(dst_scanline+first_full_byte+3)&(~0x3)) - 
             (unsigned int)(dst_scanline+first_full_byte)) * 8;

        unsigned int nlongs  = (xsize - nbits_L)/32;

        //
        //  Since our center algorithm handles 2 longs at a time 
        //  (n and n+1), we can't handle fewer than 2 longs.
        //
        if(nlongs < 2) {
            nlongs = 0;
        }
        unsigned int nbits_R = xsize - nbits_L - nlongs*32;

        //
        //  Get the 32-bit aligned start for src and dst
        //
        unsigned int* src_lstart = (unsigned int*)
            ((unsigned int)(src_scanline + (src_offset+nbits_L)/8) & (~0x3));
        unsigned int* dst_lstart = (unsigned int*)
            ((unsigned int)(dst_scanline + (dst_offset+nbits_L)/8));

        unsigned int  shiftv = (src_offset + nbits_L) -
            (((unsigned int)src_lstart - (unsigned int)src_scanline))*8;

        //
        //  Process any bits on left side if needed
        //
        if(nbits_L) {
            for(unsigned int i=0; i<nbits_L; i++) {
                if(XIL_BMAP_TST(src_scanline, src_offset+i)) {
                    XIL_BMAP_CLR(dst_scanline, dst_offset+i);
                } else {
                    XIL_BMAP_SET(dst_scanline, dst_offset+i);
                }
            }
        }

        //
        //  Do center part.
        //
        if(shiftv > 0) {
#ifdef XIL_LITTLE_ENDIAN
                for(unsigned int i=nbits_L; i<xsize - nbits_R; i++) {
                    if(XIL_BMAP_TST(src_scanline, src_offset+i)) {
                        XIL_BMAP_CLR(dst_scanline, dst_offset+i);
                    } else {
                        XIL_BMAP_SET(dst_scanline, dst_offset+i);
                    }
                }
#else
                for(unsigned int j=0; j<nlongs; j++) {
                    dst_lstart[j] =
                        ((~src_lstart[j]) << shiftv) |
                        ((~src_lstart[j+1]) >> (32-shiftv));
                }
#endif
        } else {
            for(unsigned int j=0; j<nlongs; j++) {
                dst_lstart[j] = ~src_lstart[j];
            }
        }

        //
        //  Handle the right side
        //
        if(nbits_R) {
            for(unsigned int i=xsize - nbits_R; i<xsize; i++) {
                if(XIL_BMAP_TST(src_scanline, src_offset+i)) {
                    XIL_BMAP_CLR(dst_scanline, dst_offset+i);
                } else {
                    XIL_BMAP_SET(dst_scanline, dst_offset+i);
                }
            }
        }
    }
}

Xil_boolean
xili_bit_check_for_zero(const Xil_unsigned8* dst_scanline, 
                        unsigned int         xsize,
                        unsigned int         dst_offset)
{
    //
    //  Too small for overhead?
    //
    if(xsize < ZEROCHECK_CUTOFF) {
        for(unsigned int j=0; j<xsize; j++) {
            if(!(XIL_BMAP_TST(dst_scanline, dst_offset+j))) {
                return TRUE;
            }
        }
    } else {
        //
        //  Do we have a bit offset?
        //
        if(dst_offset) {
            //
            //  check the first bits based on offset
            //
            if((~(*dst_scanline++ & clr_msb_mask[dst_offset])) &
               clr_msb_mask[dst_offset] & 0x000000ff) {
                return TRUE;
            }

            xsize -= (8 - dst_offset);
        }

        if(((unsigned int)dst_scanline) & 3) { // not 4-byte aligned
            unsigned int num_bytes_left = xsize>>3;
            unsigned int bytes_to_alignment =
                4 - (((unsigned int)dst_scanline) & 3);
            if(bytes_to_alignment < num_bytes_left) {
                while(bytes_to_alignment--) {
                    if((~(*dst_scanline++)) & 0x000000ff) {
                        return TRUE;
                    }

                    xsize -= 8;
                }
            }
        }
        
        unsigned int long_length = (xsize >> 5);
        unsigned int byte_length = (xsize - (long_length<<5))>>3;
        unsigned int set_tail    = (xsize & 0x7);
    
        
        if(long_length) {
            for(unsigned int i=0; i<long_length; i++) {
                if(~(*((const unsigned long*)dst_scanline))) {
                    return TRUE;
                }

                dst_scanline += 4;
            }
        }
        
        if(byte_length) {
            for(unsigned int i=0; i<byte_length; i++) {
                if((~(*dst_scanline++)) & 0x000000ff) {
                    return TRUE;
                }
            }
        }
        if(set_tail) {
            if((~(*dst_scanline & clr_lsb_mask[8 - set_tail])) &
               clr_lsb_mask[8 - set_tail] & 0x000000ff) {
                return TRUE;
            }
        }
    }

    return FALSE;
}
