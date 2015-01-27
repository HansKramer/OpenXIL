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
//  File:       JpegLL_Huffman_Encoder.cc
//  Project:    XIL
//  Revision:   1.5
//  Last Mod:   10:15:03, 03/10/00
//
//  Description:
//
//    Implementation of JpegLL_Huffman_Encoder
//
//
//------------------------------------------------------------------------
//    COPYRIGHT
//------------------------------------------------------------------------
#pragma ident   "@(#)JpegLL_Huffman_Encoder.cc	1.5\t00/03/10  "
 
  
#include "xil/xilGPI.hh"
#include "JpegLL_Huffman_Encoder.hh"

JpegLL_Huffman_Encoder::JpegLL_Huffman_Encoder(unsigned int  nt,
                                               unsigned int  nc,
                                               Xil_unsigned8 mark,
                                               SingleBuffer* buf)
: Jpeg_Huffman_Encoder_Base(nt, nc, mark, buf)
{
    if( ! isOK()) {
        return;
    }

    isOKFlag = FALSE;

    dc_table = NULL;  
    last_dc_table = -1;

    isOKFlag = TRUE;
}

JpegLL_Huffman_Encoder::~JpegLL_Huffman_Encoder()
{
}

void JpegLL_Huffman_Encoder::Encode_ll(Xil_signed16* diff,
                                       unsigned int width,
                                       unsigned int nbands,
                                       JpegBandInfo* banddata)
{
    //
    // Record table lookups (max of 4)
    //
    HTable* table[4];
    for(int i=0; i<nbands; i++) {
        table[i] = htables[tableIndex(banddata[i].getHtableId())];
    }

    for( ; width--; ) {
        for(int band=0; band<nbands; band++) {
            int difference = *diff++;

            //
            // Look for bit size;
            //
            int s_bits;
            unsigned int x = (difference >  0) ? difference : -difference ;
            if(x >> 8) {
                s_bits = 8 + lookup_bit_table[x >> 8] ;
            } else {
                s_bits = lookup_bit_table[x] ;
            }

            //
            // Finally find Huffman Code and put into buffer
            //
            Huffman_Code *hcode ;
            hcode = table[band]->Get_Code(s_bits);
            if(s_bits == 16) {
                Put_Code(hcode->code, hcode->length, 0, 0) ;
            } else {
                Put_Code(hcode->code, hcode->length, s_bits, difference) ;
            }
        }
    }
} 


//------------------------------------------------------------------------
//
//  Function:        JpegLL_Huffman_Encoder::Reset()
//
//  Description:
//        
//        Reset the state of this object.
//        
//------------------------------------------------------------------------

void 
JpegLL_Huffman_Encoder::Reset()
{
    Jpeg_Huffman_Encoder_Base::Reset();

    dc_table      = NULL;
    last_dc_table = -1;

    Huffman_Encoder::Reset();
}
