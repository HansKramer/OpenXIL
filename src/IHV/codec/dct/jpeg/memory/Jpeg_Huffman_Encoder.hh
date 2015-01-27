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
//  File:       Jpeg_Huffman_Encoder.hh
//  Project:    XIL
//  Revision:   1.5
//  Last Mod:   10:22:54, 03/10/00
//
//  Description:
//
//   Definition of Jpeg_Huffman_Encoder Class
//
//   This class, derived off of the base abstract class Huffman_
//   Encoder, defines the encoding and output routines specific
//   to jpeg.
//
//------------------------------------------------------------------------
//    COPYRIGHT
//------------------------------------------------------------------------
#pragma ident   "@(#)Jpeg_Huffman_Encoder.hh	1.5\t00/03/10  "


#ifndef JPEG_HUFFMAN_ENCODER_H
#define JPEG_HUFFMAN_ENCODER_H

#include <xil/xilGPI.hh>
#include "Jpeg_Huffman_Encoder_Base.hh"
#include "ZigZag.hh"

class Jpeg_Huffman_Encoder : public Jpeg_Huffman_Encoder_Base {
public:
    Jpeg_Huffman_Encoder(unsigned int  nt, 
                         unsigned int  nc,
                         Xil_unsigned8 m = 0, 
                         SingleBuffer* buf = NULL);

    ~Jpeg_Huffman_Encoder();
    void Encode(int* b, int component, 
                int dc_table_id, int ac_table_id);
    void Encode_ll(signed short difference, int dc_table_id);
    void Reset();

protected:
    HTable*      ac_table;
    int          last_ac_table;

    void Encode_dc(int bits, int value, HTable* dc_table);
    void Encode_ac(int zeros, int bits, int value, HTable* ac_table);
};

#endif

