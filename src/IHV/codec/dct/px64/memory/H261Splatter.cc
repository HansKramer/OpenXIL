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
//  File:       H261Splatter.cc
//  Project:    XIL
//  Revision:   1.5
//  Last Mod:   10:15:12, 03/10/00
//
//  Description:
//
//    Implementation of dequantize function
//
//
//------------------------------------------------------------------------
//    COPYRIGHT
//------------------------------------------------------------------------
#pragma ident   "@(#)H261Splatter.cc	1.5\t00/03/10  "

#include "xil/xilGPI.hh"
#include "H261Splatter.hh"

#define Sign(X) ((((int) (X)) >> 31) | 1)
#define PACK(LEVEL, QVAL) ((LEVEL << 8) + QVAL)
#define HASH(LEVEL, QVAL) (((QVAL << 7) ^ LEVEL) & 0xfff)


void H261Splatter::dequantize(int qscale, int *coeff, int type)

{
    int i, index, key;
    int level;
    int sign;
    CacheEntry* pcache = (CacheEntry*)cache;

    if (type == 0) {
	level = *coeff;
	*coeff++ = (int) &cosine[level * 32];
    }

    while ((level = *coeff) != 0) {
        i = level & 63;
        level >>= 8;
	key = PACK(level, qscale);
	index = HASH(level, qscale);
	if (pcache[index].key == key) {
	    level = pcache[index].entry;
	} else {
            sign = Sign(level);
            level = qscale * (2 * level + sign);
            if ((qscale & 1) == 0)
                level -= sign;
	    if (level > 2047)
		level = 2047;
	    else if (level < -2048)
		level = -2048;
            level = (int) &cosine[level * 32];
	    pcache[index].key = key;
	    pcache[index].entry = level;
	}
        level += i;
        *coeff++ = level;
    }
}
