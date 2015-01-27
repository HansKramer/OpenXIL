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
//  File:       XilDeviceCompressionFaxG4.cc
//  Project:    XIL
//  Revision:   1.6
//  Last Mod:   10:14:15, 03/10/00
//
//  Description:
//
//
//
//
//
//
//
//
//  MT-level:  <??????>
//
//------------------------------------------------------------------------
//  COPYRIGHT
//------------------------------------------------------------------------
#pragma ident   "@(#)XilDeviceCompressionFaxG4.cc	1.6\t00/03/10  "

#include <string.h>
#include "XilDeviceCompressionFaxG4.hh"

#define FRAMES_PER_BUFFER 3

//
// If you have a compression where the number of bytes per frame
// varies a great deal, this should be large (to minimize wasted space).
// However, if MaxFrameSize is a good predictor of the actual space
// used, then this can be small.
//
XilDeviceCompressionFaxG4::XilDeviceCompressionFaxG4(
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

    isOKFlag = FALSE;

    //
    // Allocate and clear to zero an area to use as an
    // initial reference line
    //
    initial_ref = new Xil_unsigned8[1024];
    if(initial_ref == NULL) {
        XIL_ERROR(system_state, XIL_ERROR_RESOURCE, "di-1", FALSE);
        return;
    }
    memset((void*)initial_ref, 0, 1024);

    uc_table[0] = uc_table_w;
    uc_table[1] = uc_table_b;

    isOKFlag = TRUE;
}

XilDeviceCompressionFaxG4::~XilDeviceCompressionFaxG4()
{
    delete [] initial_ref;
}


//
// Add an "EOFB" (0x001001) to the compressed bitstream 
//
int 
XilDeviceCompressionFaxG4::add_eofb(
        Xil_unsigned8* buf,         // cis buffer address
        int            where)       // current byte offset into cis
{
    int len = where;

    //
    // eofb code
    //
    bits |= 0x00100100 >> ndex;

    //
    // eofb code length
    //
    ndex += 24;

    //
    // flush all pending bits
    //
    while(ndex > 0) {
        buf[len++] = (char)(bits >> 24);
        bits <<= 8;
        ndex -= 8;
    }

    return(len - where);
}

//
//
// Parse the bitstream to get to the next frame
//
// TODO - It would be sort of nice if some COMMENTS were
//        in here so we can know what the hell is happening
//
int 
XilDeviceCompressionFaxG4::findNextFrameBoundary()
{
    Xil_unsigned8*  ptr;
    int             value;
    int             zcount;
    int             no_eofb=1;
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

            // Is there another ?
            if(leading_zeroes[value] + trailing_zeroes[value] == 7) { 
                zcount = trailing_zeroes[value];
                do {
                    if((ptr = cbm->getNextByte()) == NULL) {
                        return XIL_FAILURE;
                    } else {
                        value = *ptr;
                    }
                    zcount += leading_zeroes[value];
                } while(value == 0);
                if(zcount > 10) {
                    no_eofb=0;  // found EOFB
                }
            }
        } while(no_eofb);
    }

    return cbm->foundNextFrameBoundary(ptr+1);
}
