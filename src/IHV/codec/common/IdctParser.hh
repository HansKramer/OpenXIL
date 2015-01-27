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
//  File:       IdctParser.hh
//  Project:    XIL
//  Revision:   1.4
//  Last Mod:   10:23:41, 03/10/00
//
//  Description:
//
//    TODO: Enter some descriptive text here
//
//
//------------------------------------------------------------------------
//    COPYRIGHT
//------------------------------------------------------------------------
#pragma ident   "@(#)IdctParser.hh	1.4\t00/03/10  "

#ifndef PARSER_H
#define PARSER_H

#include "xil/xilGPI.hh"

#define BAD_BIT_VALUE   0xffffffff


#define GETBITS(no_bits,bs_value) \
                  if((bs_value = getbits(no_bits)) == BAD_BIT_VALUE) \
                    goto ErrorReturn; 
                

class IdctParser {
public:

    Xil_unsigned8* rdptr;
    Xil_unsigned8* endOfBuffer;
    int            bitstreamWidth;
    int            bitstreamHeight;


    int  getbits(int knt);
    void initParser();
    void putbits(int knt, int val);

    void reset();

    // Constructor
    IdctParser();

protected:
    Xil_signed32               savedBits;
    int                        nbits; 

};

inline void IdctParser::putbits(int knt, int val)
{
    if(knt > 0) {
        savedBits  = savedBits | (val<<nbits);
        nbits += knt;
        //        if(nbits > 32)
        //TODO: Need Error return ??
        //            fprintf(stderr,"putbits bomb\n");
    }
}


#endif // PARSER_H
