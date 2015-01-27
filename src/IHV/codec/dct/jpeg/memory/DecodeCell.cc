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
//  File:       DecodeCell.cc
//  Project:    XIL
//  Revision:   1.5
//  Last Mod:   10:14:24, 03/10/00
//
//  Description:
//
//    Decode a single 8x8 DCT block
//
//
//------------------------------------------------------------------------
//    COPYRIGHT
//------------------------------------------------------------------------
#pragma ident   "@(#)DecodeCell.cc	1.5\t00/03/10  "

#include "xil/xilGPI.hh"
#include "JpegDecompressorData.hh"

//------------------------------------------------------------------------
//
//  Function:   decode8x8
//
//  Description:
//      Decode (Huffman decode/dequant/IDCT) an 8x8 block
//
//  Parameters:
//      tile        array of 64 shorts.  destination for decoded pixels
//      dcpred      Reconstructed value of previous 8x8 block for this band
//      qindex      id of the quantization table for this band.
//      dctable     DC Huffman decoding table
//      actable     AC Huffman decoding table
//      
//  Returns:
//      dcpred  -- The cumulative DC coefficient value, which is used
//                 to compute the next DC coefficient (DPCM encoded).
//      
//------------------------------------------------------------------------

int 
JpegDecompressorData::decode8x8(Xil_signed16* tile, 
                                int           dcpred,
                                int           qindex,
                                int*          dctable, 
                                int*          actable)
{
    int    kount;
    short* qmtx;
    int    results[65];
    int    last;

    /*
    * Huffman decode the block
    */
    last = huffmandecoder.decode(&rdptr, results, dctable, 
                                 actable, endOfBuffer, &dcpred);

    kount = maxkount;
    qmtx = &quantizer[qindex][0];

    if(last > 0) {
        int* presults = results;
        int i, index, key;
        int level, qval;
        CacheEntry* pcache = cache;
        int* pcosine = cosine;

        while((i = *presults) != 0) {
            level = i >> 8;
            i = i & 63;
            if(i > kount) {
                *presults = 0;
                // Special case: kount has caused us to create an empty
                // results array (the first non-zero result has an index
                // greater than kount).
                if(presults == results) {
                    last = 0;
                }
                break;
            }
            qval = qmtx[i];
            key = (level<<8) + qval;   // PACK
            index = ((level << 6) + (qval&0x3f)) & 0xfff; // HASH
            if(pcache[index].key == key) {
                level = pcache[index].entry;
            } else {
                level = level * qval;
                /*
                * To deal with bad bitstreams, use the Mpeg range test here
                * (The jpeg range test is even more restrictive)
                */
                if(level > 2047) {
                    level = 2047;
                } else {
                    if(level < -2048) {
                        level = -2048;
                    }
                }
                level = (int) &pcosine[level * 32];
                pcache[index].key = key;
                pcache[index].entry = level;
            }
            level += i;
            *presults = level;
            presults += 1;
        }
    }

    if(last > 0) {
        idct((int*)tile,results);
    } else {
        int* hptr = (int *) tile;
        for(int indx = 0; indx < 8; indx++) {
            hptr[0] = 0;
            hptr[1] = 0;
            hptr[2] = 0;
            hptr[3] = 0;
            hptr += 4;
        }
    }

    return dcpred;
}
