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
//  File:       findNextFrameBoundary.cc
//  Project:    XIL
//  Revision:   1.4
//  Last Mod:   10:15:21, 03/10/00
//
//  Description:
//
//    Parse bitstream for next frame
//
//
//------------------------------------------------------------------------
//    COPYRIGHT
//------------------------------------------------------------------------
#pragma ident   "@(#)findNextFrameBoundary.cc	1.4\t00/03/10  "


#include <xil/xilGPI.hh>
#include "XilDeviceCompressionH261.hh"
#include "H261DecompressorData.hh"

unsigned char tail[256] = 
	{
	8, 0, 1, 0, 2, 0, 2, 0, 3, 0, 1, 0, 2, 0, 1, 0,
	4, 0, 1, 0, 2, 0, 2, 0, 3, 0, 1, 0, 2, 0, 1, 0,
	5, 0, 1, 0, 2, 0, 2, 0, 3, 0, 1, 0, 2, 0, 1, 0,
	4, 0, 1, 0, 2, 0, 2, 0, 3, 0, 1, 0, 2, 0, 1, 0,
	6, 0, 1, 0, 2, 0, 2, 0, 3, 0, 1, 0, 2, 0, 1, 0,
	4, 0, 1, 0, 2, 0, 2, 0, 3, 0, 1, 0, 2, 0, 1, 0,
	5, 0, 1, 0, 2, 0, 2, 0, 3, 0, 1, 0, 2, 0, 1, 0,
	4, 0, 1, 0, 2, 0, 2, 0, 3, 0, 1, 0, 2, 0, 1, 0,
	7, 0, 1, 0, 2, 0, 2, 0, 3, 0, 1, 0, 2, 0, 1, 0,
	4, 0, 1, 0, 2, 0, 2, 0, 3, 0, 1, 0, 2, 0, 1, 0,
	5, 0, 1, 0, 2, 0, 2, 0, 3, 0, 1, 0, 2, 0, 1, 0,
	4, 0, 1, 0, 2, 0, 2, 0, 3, 0, 1, 0, 2, 0, 1, 0,
	6, 0, 1, 0, 2, 0, 2, 0, 3, 0, 1, 0, 2, 0, 1, 0,
	4, 0, 1, 0, 2, 0, 2, 0, 3, 0, 1, 0, 2, 0, 1, 0,
	5, 0, 1, 0, 2, 0, 2, 0, 3, 0, 1, 0, 2, 0, 1, 0,
	4, 0, 1, 0, 2, 0, 2, 0, 3, 0, 1, 0, 2, 0, 1, 0
	};

unsigned char head[256] = 
	{
	8, 7, 6, 6, 5, 5, 5, 5, 4, 4, 4, 4, 4, 4, 4, 4,
	3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
	};

#define NEXTBYTE(PTR, VALUE)				\
	if (PTR = cbm->getNextByte())			\
	    VALUE = *PTR;				\
	else						\
	    return 0;


Xil_unsigned8* H261DecompressorData::findPSC()

    {	// return pointer to 1st complete byte in 1st PCS encountered 
	// after any non-PSC byte, or null if no PSC found.
 
    Xil_unsigned8	*ptr, test, tmp;
    int		 	 run, index, failed_once=0, backup;

    NEXTBYTE(ptr, test)
    while (1) {
	run = 0;
	while (run < 15) {	// look for 15 zeros
	    while (test) {
		run = tail[test];
		NEXTBYTE(ptr, test)
		failed_once = 1;
	   	}
	    while (!test) {
		run += 8;
		NEXTBYTE(ptr, test)
		}
	    run += head[test];
	    }
        backup = 2;
	index = head[test];		// we have 15 zeros
	tmp = (test << (index+1));
	if (index > 3) {
	    NEXTBYTE(ptr, test)
	    tmp |= test >> (7-index);
            backup = 3;
	    }
	if (!(tmp & 0xf0) && failed_once)
            return(cbm->ungetBytes(ptr,backup));
	else
	    failed_once = 1;
	}
    }


// this routine is VERY SIMILAR to the one above except that
// it does not use the cbm->getNextByte() interface.
// instead it uses its local rdptr until it runs out
// of available buffer space to parse!
// It will assume that any partial frames have already been processed
// so running into the endOfBuffer means that you are at the last frame
// (there's no following PSC)

#define ADVANCE_RDPTR(VALUE)				\
	if (rdptr < endOfBuffer)			\
	    VALUE = *rdptr++;				\
	else						\
	    return rdptr;

Xil_unsigned8* H261DecompressorData::findNextPSC()

    {	// return pointer to 1st complete byte in 1st PSC encountered 
	// after any non-PSC byte, or null if no PSC found.
 
    Xil_unsigned8	*ret, test, tmp;
    int		 	 run, index, failed_once=0;

    ADVANCE_RDPTR(test)
    while (1) {
	run = 0;
	while (run < 15) {	// look for 15 zeros
	    while (test) {
		run = tail[test];
		ADVANCE_RDPTR(test)
		failed_once = 1;
	   	}
	    while (!test) {
		run += 8;
		ADVANCE_RDPTR(test)
		}
	    ret = rdptr-2;
	    run += head[test];
	    }
	index = head[test];		// we have 15 zeros
	tmp = (test << (index+1));
	if (index > 3) {
	    ADVANCE_RDPTR(test)
	    tmp |= test >> (7-index);
	    }
	if (!(tmp & 0xf0) && failed_once)
	    return ret;
	else
	    failed_once = 1;
	}
    }

int H261DecompressorData::findNextFrameBoundary()
{
    Xil_unsigned8	*bndry;

    if (bndry = findPSC())
	return cbm->foundNextFrameBoundary(bndry);
    else
	return XIL_UNRESOLVED;
}


int H261DecompressorData::syncPSC()
{
   int zeros;
   int psc;
   int xbits;

   zeros = 0;
   GETBITS(8,psc);

   if(!psc) {
     GETBITS(8,psc);                  // first byte all zeros, get next byte
     if (!psc) {
       // too many zeros before PSC "1", illegal PSC
       return XIL_FAILURE;
     }
   }

   // The PSC has 4 "0"'s which follow the "1" bit
   // we need to find out if they are in the current byte,
   // or part of the following byte.  


   zeros = head[psc];    // find # "0"`s before the "1"

   if (zeros >3) {                // if the "1" =  bit (3,2,1,0), we
     GETBITS((zeros-3),psc)       // need some additional bits of next byte
   }
   else if (zeros < 3) {
     // we may have extra bits from the current byte
     xbits = 3-zeros;           // the "1" = bit (7,6,5), we have
     psc &= ((1<<xbits)-1);     // extra bits in the current byte
     putbits(xbits,psc);        // restore for next "get"
   }

   return XIL_SUCCESS;

 ErrorReturn:
   return XIL_FAILURE;                     // in case searched beyond buffer
}
