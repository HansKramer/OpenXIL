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
//  File:       JpegOptHuffmanEncoder.hh
//  Project:    XIL
//  Revision:   1.5
//  Last Mod:   10:22:52, 03/10/00
//
//  Description:
//
//    TODO: Enter some descriptive text here
//
//
//------------------------------------------------------------------------
//    COPYRIGHT
//------------------------------------------------------------------------
#pragma ident   "@(#)JpegOptHuffmanEncoder.hh	1.5\t00/03/10  "

#ifndef JPEGOPTHUFFMANENCODER_H
#define JPEGOPTHUFFMANENCODER_H

#include "SingleBuffer.hh"
#include "Jpeg_Huffman_Encoder.hh"
#include "JpegLList.hh"

struct JpegHuffmanBlock {
    int dc_table_id;
    int ac_table_id;
    int component;
    int block[64];
};

//------------------------------------------------------------------------
//
//  Class:        JpegOptHuffmanEncoder
//  Created:        92/03/23
//
// Description:
//
//    Derived from the standard Jpeg Huffman Encoder, this object
//    gathers frequency statistics during the normal encoding process
//    and when flushed, uses these frequencies to generate optimal
//    huffman tables.
//        
//------------------------------------------------------------------------

class JpegOptHuffmanEncoder : public Jpeg_Huffman_Encoder {
public:

    JpegOptHuffmanEncoder(unsigned int nt, unsigned int nc,
                       Xil_unsigned8 m = 0, SingleBuffer* buf = NULL);

    ~JpegOptHuffmanEncoder();

    void Encode(int* b, int component, int dc_table_id, int ac_table_id);

    void Flush_Codes();
    void generateTables(int dc_id = -1, int ac_id = -1);  

private:
    // caching indexes
    int         last_dc_index;
    int         last_ac_index;
    int         last_dc_table;
    int         last_ac_table;

    int*        dc_table_ids;
    int*        ac_table_ids; 

    // tables used in optimal table generation

    int*        symbol_count;    // the number of symbols in a given table
    int**       symbol_freq;     // the frequency of a given symbol in a table
    int**       code_size;       // the code side for a given symbol in a table
    int**       others;          // indexes to other symbols of same code size
    int**       bits;            // the number of bits in a symbol's code
    int**       huffval;         // the huffman value for a symbol
    int**       huffsize;        // the size of the huffman value in bits
    int**       huffcode;        // the huffman code
    int**       ehuffsi;         // the encoded huffman size
    int**       ehuffco;         // the encoded huffman code
    int*        lastk;           // a counter

    JpegLList*  hblocks;         // list of blocks used during encoding
    JpegLList*  cur_htables;     // list of current huffman tables


    int  getDcStats(int bits_in_code, int dc_table_id);
    int  getAcStats(int zeros, int bits_in_code, int ac_table_id);

    void initTables();
    void findHuffmanCodeSizes(int t);
    void findNumberOfCodes(int t);
    void limitCodeLengths(int t);
    void sortByCodeSize(int t);
    void generateHuffmanCodeSizes(int t);
    void generateHuffmanCodes(int t);
    void createCodeTables(int t);

    enum {
        NUM_SYMBOLS = 256
    };

};

#endif

