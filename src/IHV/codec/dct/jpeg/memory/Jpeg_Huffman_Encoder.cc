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
//  File:       Jpeg_Huffman_Encoder.cc
//  Project:    XIL
//  Revision:   1.5
//  Last Mod:   10:14:38, 03/10/00
//
//  Description:
//
//    Implementation of Jpeg_Huffman_Encoder
//
//
//------------------------------------------------------------------------
//    COPYRIGHT
//------------------------------------------------------------------------
#pragma ident   "@(#)Jpeg_Huffman_Encoder.cc	1.5\t00/03/10  "
 
  
#include <stdio.h>
#include "Jpeg_Huffman_Encoder.hh"

Jpeg_Huffman_Encoder::Jpeg_Huffman_Encoder(unsigned int  nt,
                                           unsigned int  nc,
                                           Xil_unsigned8 mark,
                                           SingleBuffer* buf)
: Jpeg_Huffman_Encoder_Base(nt, nc, mark, buf)
{
    if( ! isOK()) {
        return;
    }

    isOKFlag = FALSE;

    ac_table = NULL;
    dc_table = NULL;  
    last_ac_table = -1;
    last_dc_table = -1;

    isOKFlag = TRUE;
}

Jpeg_Huffman_Encoder::~Jpeg_Huffman_Encoder()
{
}

//------------------------------------------------------------------------
//
//  Function: Jpeg_Huffman_Encoder::Encode_dc(int bits, 
//                                            int value, HTable* htable)
//
//  Description:
//        
//        Encode dc coeff.
//        
//------------------------------------------------------------------------

void 
Jpeg_Huffman_Encoder::Encode_dc(int     bits, 
                                int     value, 
                                HTable* htable)
{
    Huffman_Code *hcode;
    int codelen, code;

    //offset into huffman code table, get 
    // variable length code and code's length 

    if((hcode = htable->Get_Code(bits)) != NULL) {
        code = hcode->code;
        codelen = hcode->length;

        Put_Code(code, codelen, bits, value);
    }
}

//------------------------------------------------------------------------
//
//  Function:        Jpeg_Huffman_Encoder::Encode_ac(int zeros, int bits, int value, HTable* htable)
//
//  Description:
//        
//        Encode ac coeff.
//        
//------------------------------------------------------------------------

void 
Jpeg_Huffman_Encoder::Encode_ac(int     zeros, 
                                int     bits, 
                                int     value, 
                                HTable* htable)
{
    Huffman_Code *hcode;
    int codelen, code;

    //offset into huffman code table, get 
    // variable length code and code's length 

    if((hcode = htable->Get_Code((zeros << 4) + bits)) != NULL) {
        code = hcode->code;
        codelen = hcode->length;

        Put_Code(code, codelen, bits, value);
    }
}

//------------------------------------------------------------------------
//
//  Function:  Jpeg_Huffman_Encoder::Encode(int* block, 
//                                          int component,
//                                          int dc_table_id, 
//                                          int ac_table_id)
//
//  Description:
//        
//        Encode a block.
//        
//------------------------------------------------------------------------

void 
Jpeg_Huffman_Encoder::Encode(int* block, 
                             int  component,
                             int  dc_table_id, 
                             int  ac_table_id)
{
    
    ZigZagArray zigzag;
    Xil_unsigned8* iptr = zigzag.getArray();
    int i, index;
    int data, diff, code;
    int zeros = 0;

    // output the entire 8x8 matrix of
    // coefficients in zigzag order

    if(!ValidComponent(component)) {

        // Jpeg bitstream error: invalid component identifier 
        XIL_ERROR( NULL, XIL_ERROR_USER, "di-86", TRUE);

        return;
    }

    int table_index;

    // Cache AC and DC table lookups

    if(last_dc_table != dc_table_id) {

        if((table_index = tableIndex(dc_table_id)) == -1) {

            // Jpeg bitstream error: invalid dc table identifier 
            XIL_ERROR( NULL, XIL_ERROR_USER, "di-85", TRUE); 

            return;
        }

        if(tableInUse(table_index)) {
            dc_table = htables[table_index];
            last_dc_table = ac_table_id;
        } else {

            // Jpeg bitstream error: dc table identifier not in use 
            XIL_ERROR( NULL, XIL_ERROR_USER, "di-84", TRUE);  

            return;
        }
    }

    if(last_ac_table != ac_table_id) {
        if((table_index = tableIndex(ac_table_id)) == -1) {

            // Jpeg bitstream error: invalid ac table identifier  
            XIL_ERROR( NULL, XIL_ERROR_USER, "di-83", TRUE);  

            return;
        }

        if(tableInUse(table_index)) {
            ac_table = htables[table_index];
            last_ac_table = ac_table_id;
        } else {

            // ToDo: should't this be "Ac Table identifier not in use??
            // Jpeg bitstream error: Dc Table identifier not in use

            XIL_ERROR( NULL, XIL_ERROR_USER, "di-84", TRUE);      

            return;
        }
    }

    for(i = 0; i < 64; i++) {
        index = *iptr++;
        data = block[index];

        if(index == 0) {

            // dc coefficient, find forward prediction,
            // count bits, and output code 

            diff = data - components[component];
            components[component] = data;
            unsigned int x = (diff > 0) ? diff : -diff ;
            if(x >> 8) {
                code = 8 + lookup_bit_table[x >> 8] ;
            } else {
                code = lookup_bit_table[x] ;
            }


            Encode_dc(code, diff, dc_table);

        } else {

            // ac coefficient
            if(data == 0) {
                // count zeros
                zeros += 1;
            } else {

                // count the bits in and then
                //  output nonzero coefficients

                while(zeros > 15) {

                    // output ZRL (Zero Run Length = 16 '0's) 
                    Encode_ac(15, 0, 0, ac_table);

                    zeros -= 16;
                }
                code = nonsignbits(data);
                Encode_ac(zeros, code, data, ac_table);

                zeros = 0;
            }
        }
    }
    if(zeros > 0) {
        // output an EOB (End Of Block) if one
        // or more zeros are left on the end
        Encode_ac(0, 0, 0, ac_table);
    }
}

void Jpeg_Huffman_Encoder::Encode_ll(signed short difference,
                                     int          dc_table_id)
{
    //
    // Cache table lookups
    //
    if(last_dc_table != dc_table_id) {
        dc_table = htables[tableIndex(dc_table_id)];
        last_dc_table = dc_table_id;
    }

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

    // Finally find Huffman Code and put into buffer

    Huffman_Code *hcode ;
    if((hcode = dc_table->Get_Code(s_bits)) != NULL  && 
                s_bits != MAX_HUF_CODE) {
        Put_Code(hcode->code, hcode->length, s_bits, difference) ;
    } else if(hcode != NULL)  {
        Put_Code(hcode->code, hcode->length, 0, 0) ;
    }
} 


//------------------------------------------------------------------------
//
//  Function:        Jpeg_Huffman_Encoder::Reset()
//
//  Description:
//        
//        Reset the state of this object.
//        
//------------------------------------------------------------------------

void 
Jpeg_Huffman_Encoder::Reset()
{
    Jpeg_Huffman_Encoder_Base::Reset();

    ac_table      = NULL;
    dc_table      = NULL;  
    last_ac_table = -1;
    last_dc_table = -1;  

    Huffman_Encoder::Reset();
}
