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
//  File:       JpegMacros.hh
//  Project:    XIL
//  Revision:   1.3
//  Last Mod:   10:23:42, 03/10/00
//
//  Description:
//
//	Common macros used in Jpeg Decoder
//
//
//------------------------------------------------------------------------
//    COPYRIGHT
//------------------------------------------------------------------------
#pragma ident   "@(#)JpegMacros.hh	1.3\t00/03/10  "

#define JPEG_FRAME_TYPE	1

#define BLOCK_SIZE      8
#define MACRO_BLOCK_SIZE_411 16

#define MAX_JPEG_QUANT_INDEX	3
#define MAX_JPEG_HTABLE_INDEX	3

#define JPEG_BASELINE 0
#define JPEG_EXTENDED 1

#define JPEG_MAX_NUM_BANDS 256

// JPEG Marker definitions ....

#define MARKER 0xff
#define TMP    0x01
#define SOF(N) (0xc0 + N)
#define DHT    0xc4
#define DAC    0xcc
#define RST(N) (0xd0 + N)
#define SOI    0xd8
#define EOI    0xd9
#define SOS    0xda
#define DQT    0xdb
#define DNL    0xdc
#define DRI    0xdd
#define DHP    0xde
#define EXP    0xdf
#define APP(N) (0xe0 + N)
#define JPG(N) (0xf0 + N)
#define COM    0xfe


// huffman table types 
#define DC_HTABLE_TYPE	0
#define AC_HTABLE_TYPE	1
#define IS_DC(X)    (((X) & 0xf0) == 0)
#define IS_AC(X)    (((X) & 0xf0) > 0)
#define DC_LUMA	    0x00
#define DC_CHROMA   0x01
#define AC_LUMA	    0x10
#define AC_CHROMA   0x11


  
#define NEXTBYTE(dptr,endptr,rval) { \
                                if (dptr >= endptr) \
                                    goto ErrorReturn;  \
                                rval = *dptr++; \
                            }
 
#define GETSHORT(dptr, endptr, rval) { \
                                if (dptr+1 >= endptr) \
                                    goto ErrorReturn;  \
                                 rval = (dptr[0] << 8) | dptr[1]; \
                                 dptr += 2; \
                             }

