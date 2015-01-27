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
//  File:       Jpeg_Huffman_Encoder_Base.hh
//  Project:    XIL
//  Revision:   1.3
//  Last Mod:   10:23:50, 03/10/00
//
//  Description:
//
//   Definition of Jpeg_Huffman_Encoder_base Class
//
//   This class, derived off of the base abstract class Huffman_
//   Encoder, defines the encoding and output routines common
//   to both Jpeg and Jpeg Lossless.
//
//------------------------------------------------------------------------
//    COPYRIGHT
//------------------------------------------------------------------------
#pragma ident   "@(#)Jpeg_Huffman_Encoder_Base.hh	1.3\t00/03/10  "


#ifndef JPEG_HUFFMAN_ENCODER_BASE_H
#define JPEG_HUFFMAN_ENCODER_BASE_H

#include <xil/xilGPI.hh>
#include "SingleBuffer.hh"
#include "Huffman_Encoder.hh"
#include "JpegMacros.hh"

#define MAX_NUM_HTABLES  8
#define MAX_HUF_CODE 16
#define MAX_LOOKUP_SIZE 256

 
class Jpeg_Huffman_Encoder_Base : public Huffman_Encoder {
public:
    Jpeg_Huffman_Encoder_Base(unsigned int  nt, 
                              unsigned int  nc,
                              Xil_unsigned8 m = 0, 
                              SingleBuffer* buf = NULL);

    ~Jpeg_Huffman_Encoder_Base();

    void Output(int table_id = ALL_HTABLES);
    void OutputChanges(int table_id = ALL_HTABLES);  
    void Reset();
    Xil_boolean isOK();


protected:
    int*         components;
    unsigned int num_components;
    HTable*      dc_table;
    int          last_dc_table;
    Xil_boolean  isOKFlag;

    int*         lookup_bit_table ;

    int  ValidComponent(unsigned int c) const
        { return ((int)c>=0 && c<num_components) ? 1 : 0; }

    void createBitSizeTableLookup(int *ltable, unsigned int tableSize) ;

};

#endif // JPEG_HUFFMAN_ENCODER_BASE_H
