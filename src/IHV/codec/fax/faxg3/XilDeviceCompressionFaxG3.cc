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
//  File:       XilDeviceCompressionFaxG3.cc
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
#pragma ident   "@(#)XilDeviceCompressionFaxG3.cc	1.7\t00/03/10  "


#include "XilDeviceCompressionFaxG3.hh"

#define FRAMES_PER_BUFFER 3

// If you have a compression where the number of bytes per frame
// varies a great deal, this should be large (to minimize wasted space).
// However, if MaxFrameSize is a good predictor of the actual space
// used, then this can be small.

XilDeviceCompressionFaxG3::XilDeviceCompressionFaxG3(
    XilDeviceManagerCompression* xdct,
    XilCis*  xcis)
: XilDeviceCompressionFax(xdct, xcis)
{

    //
    // Test that XilDeviceCompressionFax was constructed OK
    //
    if(! isOK()) {
        //  Couldn't create XilDeviceCompressionFax object
        XIL_ERROR(system_state, XIL_ERROR_SYSTEM, "di-278", FALSE);
        return;
    }

    //
    // Reset the flag
    //
    isOKFlag = FALSE;

    ss_table[0] = ss_table_w;
    ss_table[1] = ss_table_b;

    isOKFlag = TRUE;
}

XilDeviceCompressionFaxG3::~XilDeviceCompressionFaxG3() {}

//
//
// Parse the bitstream to get to the next frame
//
// TODO - It would be sort of nice if some COMMENTS were
//        in here so we can know what the hell is happening
//
int 
XilDeviceCompressionFaxG3::findNextFrameBoundary()
{
    Xil_unsigned8*  ptr;
    int             value;
    int             zcount;
    int             eolcount;
    unsigned int    w, h, nb;
    XilImageFormat* outType;
    XilDataType     datatype;

    outType = getOutputType();
    outType->getInfo(&w, &h, &nb, &datatype) ;

    while(nb--) {    // find end of each band
        if((ptr = cbm->getNextByte()) == NULL) {
          return XIL_FAILURE;
        } else {
          value = *ptr;
        }
        do {
            do {        // find 1st EOL of sequence
                zcount = trailing_zeroes[value];
                do {
                    if((ptr = cbm->getNextByte()) == NULL) {
                        return XIL_FAILURE;
                    } else {
                        value = *ptr;
                    }
                    zcount += leading_zeroes[value];
                } while(value == 0);
            } while(zcount < 11);
            eolcount = 1;    // count successive EOLs
            while(leading_zeroes[value] + trailing_zeroes[value] == 7) {
                zcount = trailing_zeroes[value];
                do {
                    if((ptr = cbm->getNextByte()) == NULL) {
                        return XIL_FAILURE;
                    } else {
                        value = *ptr;
                    }
                    zcount += leading_zeroes[value];
                } while(value == 0);
                if(zcount < 11) {
                    break;
                }
                if(++eolcount == 6) {
                    break;    
                }
            }
        } while(eolcount < 5);
    }

    return cbm->foundNextFrameBoundary(ptr+1);
}



//
// Add an "EOL" (0x0001) to the compressed bitstream 
//
int 
XilDeviceCompressionFaxG3::add_eol(
        Xil_unsigned8* buf,         // cis buffer address
        int            where)       // current byte offset into cis
{
    int len = where;

    //
    // Flush pending bits
    //
    if(bits) {
        buf[len++] = (char)(bits >> 24);
    }

    // Write EOL into buffer
    buf[len++] = 0x0;
    buf[len++] = 0x1;

    //  
    // Reset bit buffer machinery
    //
    ndex = 0;
    bits = 0x00000000;

    return(len - where);
}

//
// TODO: Is this routine safe? What happens if a corrupted bitsteam
//       is encountered. (Link)

//
// Return index of 1st bit past next EOL in stream beyond base+index 
//
int 
XilDeviceCompressionFaxG3::find_eol(unsigned int* base, 
                                    unsigned int* end, 
                                    int           index)
{
    int          shift, test, mask, tmask;
    unsigned int xbits, morebits;

    base += index >> 5;
    shift = index & 0x1f;
    index &= 0xffffffe0;
    mask = 0xffe00000;
    GETWORD(morebits, base, end);

    //
    // Look for 11 consecutive zeros 
    //
    while(1) {
        xbits = morebits;
        GETWORD(morebits, base, end);                
        while(shift < 32) {
            test = xbits << shift;
            if(shift > 21) {
                test |= morebits >> (32-shift);
            }
            if(mask & test) {
                //
                // Missed; revert to last "1" 
                //
                tmask = 0x00200000;
                while((test & tmask) == 0) {
                    shift--;
                    tmask <<= 1;
                }
            } else {
                //
                // Hit; keep going to next "1" 
                //
                index += shift;
                tmask = 0x00100000;
                if(test == 0) {
                    index += 32-shift-11;
                    if(morebits == 0) {
                        index += 32;
                        while(*base++ == 0) {
                            index += 32;
                        }
                    }
                    test = *(base-1);
                    UNSCRAMBLE(test)
                    tmask = 0x80000000;
                }
                while((test & tmask) == 0){
                    test <<= 1;
                    index++;
                }
                return index+12;
            }
            shift += 11;
        }
        shift -= 32;
        index += 32;
    }
}

int 
XilDeviceCompressionFaxG3::grab_12 (Xil_unsigned8* base,
                                    unsigned int   bit_offset)
{        // get 12 bits starting at base + bit_offset and return rt justified

    unsigned int* word;
    unsigned int  word0;
    unsigned int  word1;
    int                  byte, startbit;

    byte = (int)(base + (bit_offset>>3)) ;
    word = (unsigned int *)(byte & 0xfffffffc);
    word0 = word[0];
    UNSCRAMBLE(word0)
    startbit = ((byte & 0x3) << 3) + (bit_offset & 0x7);
    if(startbit < 21)
        return (word0>>(20-startbit)) & 0xfff;  
    else {
        word1 = word[1];
        UNSCRAMBLE(word1)
        return ((word0<<(startbit-20)) | (word1>>(52-startbit))) & 0xfff;
        }
}
