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
//  File:       Mpeg1Splatter.cc
//  Project:    XIL
//  Revision:   1.2
//  Last Mod:   10:14:49, 03/10/00
//
//  Description:
//
//    Dequantize function for Mpeg1 decoding
//
//
//------------------------------------------------------------------------
//    COPYRIGHT
//------------------------------------------------------------------------
#pragma ident   "@(#)Mpeg1Splatter.cc	1.2\t00/03/10  "

#include <xil/xilGPI.hh>
#include "Mpeg1Splatter.hh"

//
// The Sign function must return values as follows:
//    (X > 0) --->  1
//    (X = 0) --->  0
//    (X < 0) ---> -1
//
#define Sign(X) ((((int) (X)) >> 31) | 1)

//
// "Dequantize" the 8x8 block in preparation for the splatter 
//  IDCT algorithm. The "type" code is:
//     0: Intra-coded blocks
//     1: Inter-coded blocks
//

void 
Mpeg1Splatter::dequantize(int            qscale, 
                          int*           icoeff, 
                          Xil_unsigned8* qmatrix,
                          int            type)
{
    int  level;
    int* ocoeff = icoeff;
    if(type == 0) {
        level = *icoeff++;
        level >>= 8;
        *ocoeff++ = (int) &cosine[level * 32];
    }

    int i;
    int index;
    int key;
    int qval;
    while((level = *icoeff++) != 0) {
        i = level & 63;
        level >>= 8;
        qval = qmatrix[i];

        // Pack the key
        key = ((level << 16) + (qval << 8) + (type << 5) + qscale);

        // Calculate a hash index
        index = ((level + ((qval - 16) << 4) 
                + ((qscale - 8) << 8)
                + (type << 11)) & 0xfff);
        //
        // TODO: (lperry)
        //       The key transformation and hash index generation
        //       above requires 15 operations. Is this defeating the
        //       gain from the splatter algorithm.
        //       The AAN scaled IDCT uses about 8 adds and 1.25
        //       multiplies per pixel. Since we have fast floating
        //       on Sparc now, try switching to AAN. We'll
        //       save a ton of table memory.
        //


        if(cache[index].key == key) {
            //
            // Its already cached, so fetch the value from the cache
            //
            level = cache[index].entry;
        } else {
            if(type == 0) {
                level = (level * qscale * qval) >> 3;
            } else {
                level = ((2 * level + Sign(level)) * qscale * qval) >> 4;
            }
            if(level == 0) {
                continue;
            }
            if((level & 1) == 0) {
                level -= Sign(level);
            }

            //
            // Clamp to 11 bit signed range (-2048 t0 +2047)
            //
            if(level > 2047) {
                level = 2047;
            } else if(level < -2048) {
                level = -2048;
            }
            level = (int) &cosine[level * 32];

            //
            // Store this decoded entry in the cache
            //
            cache[index].key   = key;
            cache[index].entry = level;
        }
        level += i;
        *ocoeff++ = level;
    }
    *ocoeff = level;
}
