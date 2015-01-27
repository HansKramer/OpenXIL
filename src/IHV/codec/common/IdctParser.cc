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
//  File:       IdctParser.cc
//  Project:    XIL
//  Revision:   1.5
//  Last Mod:   10:16:16, 03/10/00
//
//  Description:
//
//    TODO: Enter some descriptive text here
//
//
//------------------------------------------------------------------------
//    COPYRIGHT
//------------------------------------------------------------------------
#pragma ident   "@(#)IdctParser.cc	1.5\t00/03/10  "

#include "IdctParser.hh"

void IdctParser::initParser()
{
    nbits = 0;
    savedBits = 0;
}

void IdctParser::reset()
{
    nbits = 0;
    savedBits = 0;
    bitstreamWidth = 0;
    bitstreamHeight = 0;
}

IdctParser::IdctParser()
{
    reset();
}

//
// Extract a field of kount bits from a packed bitstream.
// Maintain internal bit positions and counts.
//
int IdctParser::getbits(int kount)
{
    // This routine will work for kount < 32
    int have;
    int left;
    int tempword;

    have = nbits;
    left = savedBits;

    if(kount <= have) {
        tempword = left >> (have-kount);
        have -= kount;
        left = left & ((1<<have) -1);
    } else {
        tempword = left;
        kount -=have ;
        have  = 0;
        left = 0;
        while(kount >= 8) {
            if(rdptr == endOfBuffer) {
                return BAD_BIT_VALUE;
            }
            tempword = (tempword << 8) | *rdptr++;
            kount -=8;
        }
        if(kount !=0) {
            if(rdptr == endOfBuffer) {
                return BAD_BIT_VALUE;
            }
            left = *rdptr++;
            have = 8;
            tempword = (tempword << kount) | (left >> (have-kount));
            have -= kount;
            left = left & ((1<<have)-1);
        }
    }

    nbits = have;
    savedBits = left;

    return tempword;
}

