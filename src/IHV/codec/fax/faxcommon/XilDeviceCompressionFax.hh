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
//  File:       XilDeviceCompressionFax.hh
//  Project:    XIL
//  Revision:   1.9
//  Last Mod:   10:22:42, 03/10/00
//
//  Description:
//
//    Header file for: 
//      Abstract base class for fax compressors (faxG3, faxG4).
//      Contains methods common to both
//
//
//------------------------------------------------------------------------
//    COPYRIGHT
//------------------------------------------------------------------------
#pragma ident   "@(#)XilDeviceCompressionFax.hh	1.9\t00/03/10  "

#ifndef _XILDEVICECOMPRESSIONFAX_HH
#define _XILDEVICECOMPRESSIONFAX_HH

#include <xil/xilGPI.hh>

class XilDeviceCompressionFax : public XilDeviceCompression
{
public:
    int           nextstate(unsigned char* base,
                            unsigned int   bit_offset,
                            unsigned int   max_offset);

    void          setWidth(unsigned int w);

    void          setHeight(unsigned int h);

    void          setBands(unsigned int n);

    XilStatus     deriveOutputType();

    void          burnFrames(int nframes);

    unsigned int  maxFrameSize(unsigned int width, 
                               unsigned int height, 
                               unsigned int bands);

    int           getMaxFrameSize();

    XilStatus     initValues();

    void          reset();

    void          init_bitbuf();
 
    int           add_1d_bits(unsigned char* buf,
                              int            where,
                              int            count,
                              int            color);
 
    int           add_2d_bits(unsigned char*       buf,
                              int                  where,
                              const unsigned int** mode,
                              int                  entry);
 

protected:
    XilDeviceCompressionFax(XilDeviceManagerCompression* xdmc,
                            XilCis*                      xcis);

    ~XilDeviceCompressionFax();

    Xil_boolean isOK();

    //
    // Mutex for locking around compress/decompress ops
    //
    XilMutex  mutex;

    //
    // Bit packer variables
    // TODO: probably need to lock around  uses of these
    //
    unsigned int bits;  // bits go here
    int          ndex;  // index into bits

    //
    // Make the image dimensions accessible to subclasses
    //

    //
    // Constant tables accessible to subclasses
    //
    static unsigned char byte_table[256];
    static unsigned int  c_codes_b[64];
    static unsigned int  c_codes_w[64];
    static unsigned int  xc_codes_b[60];
    static unsigned int  xc_codes_w[60];
    static unsigned int  pass_mode[1];
    static unsigned int  vert_mode[7];
    static unsigned int  horz_mode[1];

    const unsigned int* c_codes[2];
    const unsigned int* xc_codes[2];

    const unsigned int* pass[2];
    const unsigned int* vert[2];
    const unsigned int* horz[2];

    Xil_boolean         isOKFlag;

    enum colors {
        WHITE = 0,
        BLACK = 1
    };

    enum fax_modes {
        V0           = 0,
        V_1          = 1,
        V1           = 2,
        HORIZONTAL   = 3,
        PASS         = 4,
        V_2          = 5,
        V2           = 6,
        V_3          = 7,
        V3           = 8,
        UNCOMPRESSED = 9
    };

private:
    //
    // Private image params accessible only with get/set functions
    //
    unsigned int fax_width;
    unsigned int fax_height;
    unsigned int fax_bands;


}; // End class definition

//
// Defines for Fax compression
// TODO: Determine which of these are common and
//       which are specific to G3 or G4.
//


#define CODELEN(code)	((code >> 16) & 0xff)
#define CODETYPE(code)	(code & 0xff)
#define CODERUN(code)	((int) (((code >> 8) & 0xff) * 32) + (code & 0xff))
#define IS_MULTI(code)	(code & 0x40000000)
#define MULTILEN(code)	(((code >> 26) & 0x7) + 6)
#define DATALEN(code)	((code >> 21) & 0x1f)
#define NEXTCOLOR(code)	((code >> 29) & 0x1)
#define MORERUN(code)	(code & 0x80000000)
#define CIS_INC(cisbits)	(cisbits & 0x1f)
#define SRC_INC(cisbits)	((cisbits >> 5) & 0xf)
#define	TESTBIT(address, bit_offset) 					\
	((*(address+(bit_offset>>3))>>(0x7-(bit_offset & 0x7))) & 0x1)

#ifdef XIL_LITTLE_ENDIAN
#define UNSCRAMBLE(word) 				\
		word = (word << 24) | 			\
		       ((word & 0xff00) << 8) | 	\
		       ((word >> 8) & 0xff00) | 	\
		       (word >> 24);
#else
#define UNSCRAMBLE(word)
#endif

#define GETWORD(word, ptr, end)        \
    if (ptr > end) {                   \
        return 0;                      \
    }                                  \
    word = *ptr++;  UNSCRAMBLE(word)

//
// Macro to add code to bit buffer and flush if necessary 
//
#define FEEDBITS(table, entry)                                          \
{                                                                   \
    mask = table[color][entry];             /* get mask from table*/\
    bits |= (mask & 0xfff80000) >> ndex;    /* extract code bits */ \
    ndex += (int)(mask & 0x0000ffff);       /* extract code length*/\
    while (ndex > 7) {      /* flush a byte if >7 pending bits */   \
        buf[len++] = (char)(bits >> 24);                            \
        bits <<= 8;                                                 \
        ndex -= 8;                                                  \
    }                                                           \
}

#define GETINDEX(LENGTH)                                           \
    if ((shift = 32-(LENGTH+cis_ndx)) < 0) /* bits span both */    \
    tblindex = (inbits<<-shift)|(inbits2>>(32+shift));             \
    else                   /* all bits in 1st */                   \
    tblindex = (inbits >> shift);

#define GETMOREBITS                                            \
    if (cis_ndx > 31) {  /* get next word from bitstream */    \
    inbits = inbits2;                                          \
    GETWORD(inbits2, base, end)                                \
    cis_ndx -= 32;                                             \
    }

#define WRITERUN                                           \
    if (color) {            /* append black bits */        \
    outbits |= allblack >> out_ndx;                        \
    out_ndx += runlen;                                     \
    while (out_ndx > 8) {                                  \
        *byte++ = outbits;        /* write buffer    */    \
        outbits = allblack;        /* set buffer black */  \
        out_ndx -= 8;                                      \
        }                                                  \
    outbits ^= allblack >> out_ndx;/* whiten extra bits */ \
    }                                                      \
    else {                /* append white bits */          \
    outbits &= allblack << (8-out_ndx);                    \
    out_ndx += runlen;                                     \
    if (out_ndx > 8) {                                     \
        *byte++ = outbits;        /* flush buffer    */    \
        byte += (out_ndx-8) >> 3; /* skip white bytes */   \
        out_ndx &= 0x7;                                    \
        outbits = 0;        /* set buffer white */         \
        }                                                  \
    }

#define WRITEMULTIRUN                                                   \
    outbits = (outbits << 24) | ((code & 0x1fffff) << (11 - out_ndx));  \
    out_ndx += DATALEN(code);                                           \
    while (out_ndx > 8) {                                               \
    *byte++ = outbits >> 24;        /* write buffer    */               \
    outbits <<= 8;                                                      \
    out_ndx -= 8;                                                       \
    }                                                                   \
    outbits >>= 24;

#define BUMPREF                         \
    extra -= runlen;                    \
    while (extra < 0)                   \
        extra += *refruns++;            \
    color ^= 1;

#define STEPOVER_EOL                                            \
    if ((inbits & ((unsigned)0xffffffff >> cis_ndx)) == 0) {    \
    if (inbits2 == 0)                                           \
        while (*base++ == 0);    /* allow for padding */        \
    inbits = *(base-1);        /* get to last word */           \
    UNSCRAMBLE(inbits)                                          \
    GETWORD(inbits2, base, end)                                 \
    cis_ndx = 0;                                                \
    }                                                           \
    testbit = (unsigned)0x80000000 >> cis_ndx;                  \
    while ((inbits & testbit) == 0){    /* find end of EOL */   \
    testbit >>= 1;                                              \
    cis_ndx++;                                                  \
    }                                                           \
    cis_ndx++;                /* 1 bit beyond EOL */            \
    GETMOREBITS

#define STEPOVER_EOFB3                  \
    STEPOVER_EOL                        \
    STEPOVER_EOL                        \
    STEPOVER_EOL                        \
    STEPOVER_EOL                        \
    STEPOVER_EOL

#define EOFB4    3

#define FINISH_UP \
{ \
    bit_offset = (int)(byte-base)*8 + byte_table[(unsigned char)testbyte]; \
    return ((bit_offset < max_offset) ? bit_offset : max_offset); \
}

#endif // _XILDEVICECOMPRESSIONFAX_HH
