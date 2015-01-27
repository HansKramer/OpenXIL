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
//  File:       XilDeviceCompressionFax.cc
//  Project:    XIL
//  Revision:   1.7
//  Last Mod:   10:14:09, 03/10/00
//
//  Description:
//
//    Abstract base class for fax compressors (faxG3, faxG4).
//    Contains methods common to both
//
//
//------------------------------------------------------------------------
//    COPYRIGHT
//------------------------------------------------------------------------
#pragma ident   "@(#)XilDeviceCompressionFax.cc	1.7\t00/03/10  "

#include "XilDeviceCompressionFax.hh"

#define FRAMES_PER_BUFFER 3
// If you have a compression where the number of bytes per frame
// varies a great deal, this should be large (to minimize wasted space).
// However, if MaxFrameSize is a good predictor of the actual space
// used, then this can be small.

XilDeviceCompressionFax::XilDeviceCompressionFax(
    XilDeviceManagerCompression* xdct,
    XilCis*                      xcis)
: XilDeviceCompression(xdct, xcis, 0, FRAMES_PER_BUFFER)
{
    isOKFlag = FALSE;

    //
    // Check that the construction of XilDeviceCompression succeeded.
    // This creates a CisBufferManager Object, using new, so it could fail.
    //
    if(! deviceCompressionValid()) {
        //  Couldn't create internal base XilDeviceCompression object
        XIL_ERROR(system_state, XIL_ERROR_SYSTEM, "di-278", FALSE);
        return;
    }


    if(initValues() == XIL_FAILURE) {
        //  Couldn't create internal fax compressor object
        XIL_ERROR(system_state, XIL_ERROR_SYSTEM, "di-300", FALSE);
        return;
    }

    isOKFlag = TRUE;
}

//
// Destructor
//
XilDeviceCompressionFax::~XilDeviceCompressionFax()
{
    //
    // Destroy the ImageFormat object(s).
    //
    if(inputType != outputType) {
        outputType->destroy();
    }

    inputType->destroy();
}


XilStatus 
XilDeviceCompressionFax::initValues()
{
    const unsigned int zero = 0;

    XilImageFormat* t = 
        system_state->createXilImageFormat(zero, zero, zero, XIL_BIT);
    if(t == NULL) {
        XIL_ERROR(system_state, XIL_ERROR_RESOURCE, "di-1", TRUE);
        return XIL_FAILURE;
    }

    inputType  = t;
    outputType = t;
    fax_width  = 0;
    fax_height = 0;
    fax_bands  = 0;

    //
    // Supply initial values for bit packer variables
    //
    bits = 0;
    ndex = 0;

    //
    // Initialize the pointers to the black/white code tables
    //
    c_codes[0]  = c_codes_w;
    c_codes[1]  = c_codes_b;

    xc_codes[0] = xc_codes_w;
    xc_codes[1] = xc_codes_b;

    pass[0]     = pass_mode;
    pass[1]     = pass_mode;

    vert[0]     = vert_mode;
    vert[1]     = vert_mode;

    horz[0]     = horz_mode;
    horz[1]     = horz_mode;

    return XIL_SUCCESS;
}

void
XilDeviceCompressionFax::reset()
{
    if(inputType != outputType) {
        outputType->destroy();
    }
    inputType->destroy();

    initValues();

    XilDeviceCompression::reset();
}

Xil_boolean
XilDeviceCompressionFax::isOK()
{
    if(this == NULL) { 
        return FALSE; 
    } else { 
        if(isOKFlag == TRUE) { 
            return TRUE; 
        } else { 
            delete this; 
            return FALSE; 
        } 
    }
}       


int 
XilDeviceCompressionFax::nextstate(unsigned char* base,
           unsigned int  bit_offset,
           unsigned int  max_offset)
{    
    //
    // Return min of:                        
    //    max_offset                
    //  or    
    //    offset of 1st pixel different from pixel at bit_offset    
    //

    unsigned char* byte  = base + (bit_offset>>3);
    unsigned char* end   = base + (max_offset>>3);
    int            extra = bit_offset & 0x7;

    int  testbyte;

    if(*byte & (0x80 >> extra)) {    // look for "0" 
        testbyte = ~(*byte) & (0xff >> extra);
        while (byte < end) {
            if (testbyte) {
                break;
            }
            testbyte = ~(*++byte) & 0xff;
        }
    } else {                // look for "1"
        if (testbyte = *byte & (0xff >> extra)) {
            FINISH_UP
        }
        while (byte < end) {
            if (testbyte = *++byte) {        // "1" is in current byte
                FINISH_UP
            }

#ifndef XIL_LITTLE_ENDIAN
            if (!((int)byte & 0x3)) {   // current byte on word boundary
                testbyte = *(int *)byte;    // examine 4 bytes at a time
                while (byte <= end) {
                    if (testbyte) {        // "1" is in  current word
                        testbyte = *byte;
                        while (byte <= end) {// examine word byte at a time
                            if (testbyte) {
                                FINISH_UP
                            }
                            testbyte = *++byte;
                        }
                        FINISH_UP
                    }
                    byte += 4;        // get next word
                    testbyte = *(int *)byte;
                }
            }
#endif

        }
    }
    FINISH_UP
}

//
// Fax is different from most codecs in not having 
// the width/height/nbands derivable from the bitstream.
// Hence we need explicit set methods to allow the 
// type (dimensions and nbands) to be set from the API.
//
void
XilDeviceCompressionFax::setWidth(unsigned int w)
{
    fax_width = w;
}

void
XilDeviceCompressionFax::setHeight(unsigned int h)
{
    fax_height = h;
}

void
XilDeviceCompressionFax::setBands(unsigned int n)
{
    fax_bands = n;
}

XilStatus 
XilDeviceCompressionFax::deriveOutputType()
{
    XilImageFormat* newtype;

    //
    // If any fields are still zero, we fail
    //
    if(!fax_width || !fax_height || !fax_bands) {
        return XIL_FAILURE;
    }

    newtype = system_state->createXilImageFormat(fax_width, 
                                                 fax_height, 
                                                 fax_bands, 
                                                 XIL_BIT);
    if(newtype == NULL) {
      XIL_ERROR(system_state, XIL_ERROR_RESOURCE, "di-1", TRUE);
      return XIL_FAILURE;
    }
    
    setInputType(newtype);

    newtype->destroy();
    return XIL_SUCCESS;
}


void
XilDeviceCompressionFax::burnFrames(int nframes)
{
    XilImage *dst ;
    int i;
    unsigned int w, h, nb;
    XilImageFormat *outType;
    XilDataType datatype ;

    outType = getOutputType();
    outType->getInfo(&w, &h, &nb, &datatype) ;
    dst = system_state->createXilImage(w, h, nb, datatype) ;
    
    for (i=0; i<nframes; i++) {
        // TODO: - fix this (tlp)
        // decompress( dst );
    }
}


unsigned int 
XilDeviceCompressionFax::maxFrameSize(unsigned int width, 
                                      unsigned int height, 
                                      unsigned int bands)
{
    return bands * width * height/2;
}

// The number of bytes that you can guarentee will be large enough
// for the worst compression you can get.

#define FUDGE 100        // for small images
int 
XilDeviceCompressionFax::getMaxFrameSize()
{
    return
    ((int)(inputType->getWidth() *
           inputType->getHeight() *
           inputType->getNumBands()))/2 + FUDGE;
}

//
// Initialize bit buffer machinery
//
void
XilDeviceCompressionFax::init_bitbuf()
{   
    ndex = 0;
    bits = 0x00000000;
}

//
// Get code for run and add to compressed bitstream
//
int
XilDeviceCompressionFax::add_1d_bits(
    unsigned char* buf,   // cis buffer addr
    int            where, // byte offs
    int            count, // #pixels in run
    int            color) // color of run
{       
    int                 sixtyfours;
    unsigned int        mask;
    int                 len = where;
 
    sixtyfours = count >> 6;    // count / 64;
    count = count & 0x3f;       // count % 64
    if (sixtyfours) {
        for ( ; sixtyfours > 40; sixtyfours -= 40) {
            FEEDBITS(xc_codes, 40)
        }
        FEEDBITS(xc_codes, sixtyfours)
    }
    FEEDBITS(c_codes, count)
    return(len - where);
}

//
// place entry from mode table into compressed bitstream
//
int
XilDeviceCompressionFax::add_2d_bits(
    unsigned char*       buf,      // cis buffer addr
    int                  where,    // byte offset into cis
    const unsigned int   *mode[2], // 2-D mode to be encoded
    int                  entry)    // mode entry (0 unless vertical)
{       
    unsigned int        mask;
    int                 len = where;
    int                 color = 0;   

    //
    // TODO : FEEDBITS can be a routine. It needs color which is
    //        not obvious it being a macro.
    //
 
    FEEDBITS(mode, entry)

    return(len - where);
}
