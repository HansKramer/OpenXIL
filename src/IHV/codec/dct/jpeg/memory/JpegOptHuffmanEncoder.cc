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
//  File:       JpegOptHuffmanEncoder.cc
//  Project:    XIL
//  Revision:   1.6
//  Last Mod:   10:14:29, 03/10/00
//
//  Description:
//
//    Implementation of Optimal Jpeg Huffman Encoder Object
//
//
//------------------------------------------------------------------------
//    COPYRIGHT
//------------------------------------------------------------------------
#pragma ident   "@(#)JpegOptHuffmanEncoder.cc	1.6\t00/03/10  "



#include <xil/xilGPI.hh>
#include <string.h>
#include "JpegOptHuffmanEncoder.hh"
#include "JpegMacros.hh"
#include "ZigZag.hh"

JpegOptHuffmanEncoder::JpegOptHuffmanEncoder(unsigned int  nt,
                                             unsigned int  nc, 
                                             Xil_unsigned8 mark, 
                                             SingleBuffer* buf) 
 : Jpeg_Huffman_Encoder(nt, nc, mark, buf)
{
    int i,j;

    // intialize cache indexes to an invalid index (-1)
    last_dc_index = -1;
    last_ac_index = -1;
    last_dc_table = -1;
    last_ac_table = -1;  

    dc_table_ids  = new int[num_components];  
    ac_table_ids  = new int[num_components];
    symbol_count  = new int[nt];
    symbol_freq   = new int*[nt];
    code_size     = new int*[nt];
    others        = new int*[nt];
    bits          = new int*[nt];
    huffval       = new int*[nt];
    huffsize      = new int*[nt];
    huffcode      = new int*[nt];
    ehuffsi       = new int*[nt];
    ehuffco       = new int*[nt];
    lastk         = new int[nt];
 
    hblocks       = new JpegLList;
    cur_htables   = new JpegLList;

    for(i = 0; i < nc; i++) {
        ac_table_ids[i] = -1;
        dc_table_ids[i] = -1;
    }

    for(i = 0; i < nt; i++) {

        symbol_count[i] = 0;

        // NUM_SYMBOLS+1 to reserve one code point    

        symbol_freq[i] = new int[NUM_SYMBOLS+1];
        code_size[i]   = new int[NUM_SYMBOLS+1];
        others[i]      = new int[NUM_SYMBOLS+1];
        huffval[i]     = NULL;
        huffsize[i]    = NULL;
        huffcode[i]    = NULL;
        ehuffsi[i]     = NULL;
        ehuffco[i]     = NULL;

        // algorithms assume that bits will never exceed 32
        // thus make array indexes from 0 to 32:

        bits[i] = new int[33];

        // initialize frequencies to 0

        for(j=0; j<NUM_SYMBOLS; j++) {
            symbol_freq[i][j] = 0;
        }

        // setting frequency of NUM_SYMBOLS to 1 to reserve one code point
        symbol_freq[i][NUM_SYMBOLS] = 1;
    }

}

JpegOptHuffmanEncoder::~JpegOptHuffmanEncoder()
{
    delete []ac_table_ids;
    delete []dc_table_ids;
    delete []symbol_count;
    delete []lastk;

    for(int i = 0; i < num_tables; i++) {
        delete symbol_freq[i];
        delete code_size[i];
        delete others[i];
        delete bits[i];
        if(huffval[i]) {
            delete huffval[i];
            delete huffsize[i];
            delete huffcode[i];
            delete ehuffsi[i];
            delete ehuffco[i];
        }
    }

    delete []symbol_freq;
    delete []code_size;
    delete []others;
    delete []bits ;
    delete []huffval;
    delete []huffsize;
    delete []huffcode;
    delete []ehuffsi;
    delete []ehuffco;

    delete hblocks;

    delete cur_htables;
}


//------------------------------------------------------------------------
//
//  Function:        JpegOptHuffmanEncoder::Encode(int* block,
//                                           int component, int dc_table_id, 
//                                           int ac_table_id)
//  Created:        92/09/ 1
//
//  Description:
//        
//        Much of this routine is much like the encode routine of the 
//      ordinary Jpeg_Huffman_Encoder object except encoded data is
//      not written out to the byte stream. Instead, this routine
//      is used to gather frequency information about the block
//      to be encoded. The parameters to this routine, including
//      the block itself, are save into a an object called a huffman
//      block and appended to a list of such objects. At a later time
//      after optimal tables have been generated, these blocks will be
//      removed in order of addition and re-encoded using the new optimal
//      tables.
//        
//  Parameters:
//        
//      int* block:              block resulting from DCT and quantization.
//      int component:           specifies which component the block originated
//                                  from.
//      int dc_table_id:         the identifier for the dc table to encode with
//      int ac_table_id:         the identifier for the ac table to encode with
// 
//------------------------------------------------------------------------

void JpegOptHuffmanEncoder::Encode(int* block, 
                                   int component,
                                   int dc_table_id, 
                                   int ac_table_id)
{

    int table_slot;

    // assertion check for valid component
    if(!ValidComponent(component)) {
        // Jpeg bitstream error: invalid component identifier 
        XIL_ERROR( NULL, XIL_ERROR_USER, "di-86", TRUE);
        return;    
    }

    // check for dc table cache in order to avoid lookup
    if(dc_table_id != dc_table_ids[component]) {

        // check to see if the dc table id for the
        // given component has been set yet

        if(dc_table_ids[component] == -1) {

            // the identifier has not yet been set, thus set it
            dc_table_ids[component] = dc_table_id;

            // look up the table index given the table identifier
            if((table_slot = tableIndex(dc_table_id)) == -1) {

                // we have not reserved a table for this slot so do it.
                table_slot = Add_Table(NULL,dc_table_id);
            }

            // The DC table can't have more than 16 symbols
            symbol_count[table_slot] = 16;

            // reserve the last code point
            symbol_freq[table_slot][16] = 1;

            // if we haven't created the other tables yet do so

            if(!huffval[table_slot]) {
                huffval[table_slot] = new int[symbol_count[table_slot]];
                huffsize[table_slot] = new int[symbol_count[table_slot]];
                huffcode[table_slot] = new int[symbol_count[table_slot]];
                ehuffsi[table_slot] = new int[symbol_count[table_slot]];
                ehuffco[table_slot] = new int[symbol_count[table_slot]];
            }
        } else {
            // dc table id does not match previous id given
            // XIL_ERROR( NULL, XIL_ERROR_SYSTEM, "di-??", TRUE);
            return;
        }
    }

    // check for ac table cache in order to avoid lookup
    if(ac_table_id != ac_table_ids[component]) {

        // check to see if the ac table id for the
        // given component has been set yet

        if(ac_table_ids[component] == -1) {

            // the identifier has not yet been set, thus set it
            ac_table_ids[component] = ac_table_id;

            // look up the table index given the table identifier
            if((table_slot = tableIndex(ac_table_id)) == -1) {

                // we have not reserved a table for this slot so do it.
                table_slot = Add_Table(NULL,ac_table_id);

            }

            // The AC table can't have more than NUM_SYMBOLS symbols
            // The reserved code point has already been made.
            symbol_count[table_slot] = NUM_SYMBOLS;

            // if we haven't created the other tables yet do so
            if(!huffval[table_slot]) {
                huffval[table_slot] = new int[symbol_count[table_slot]];      
                huffsize[table_slot] = new int[symbol_count[table_slot]];
                huffcode[table_slot] = new int[symbol_count[table_slot]];
                ehuffsi[table_slot] = new int[symbol_count[table_slot]];
                ehuffco[table_slot] = new int[symbol_count[table_slot]];
            }
        } else {
            // ac table id does not match previous id given
            // XIL_ERROR( NULL, XIL_ERROR_SYSTEM, "di-??", TRUE);
            return;      
        }
    }

    // create a new huffman encoded block
    JpegHuffmanBlock* hblock = new JpegHuffmanBlock;

    // copy the data to be encoded in to hblocks block
    memcpy(hblock->block, block, sizeof(JpegHuffmanBlock));

    // the the component, dc and ac identifiers used to encode
    // this block. They will be used later to re-encoded the
    // block after the optimal tables have been generated.

    hblock->component = component;  
    hblock->dc_table_id = dc_table_id;
    hblock->ac_table_id = ac_table_id;

    // append this block to the list of blocks to re-encode.
    hblocks->append(hblock);

    // perform the encoding process but actually only
    // gather AC and DC frequency statistics

    ZigZagArray zigzag;
    Xil_unsigned8* iptr = zigzag.getArray();
    int i, index;
    int data, diff, code;
    int zeros = 0;


    for(i = 0; i < 64; i++) {
        index = *iptr++;
        data = block[index];

        if(index == 0) {

            // dc coefficient, find forward prediction,
            // count bits, and output code 

            diff = data - components[component];
            components[component] = data;
            code = nonsignbits(diff);

            getDcStats(code, dc_table_id);

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
                    getAcStats(15, 0, ac_table_id);

                    zeros -= 16;
                }
                code = nonsignbits(data);
                getAcStats(zeros, code, ac_table_id);

                zeros = 0;
            }
        }
    }
    if(zeros > 0) {
        // output an EOB (End Of Block) if one
        // or more zeros are left on the end
        getAcStats(0, 0, ac_table_id);
    }  
}

//------------------------------------------------------------------------
//
//  Function:        JpegOptHuffmanEncoder::getDcStats(int bits_in_code,
//                                                int dc_table_id)
//  Created:        92/09/ 1
//
//  Description:
//        
//        The parameter bits_in_code is what is used later to encode the
//      desired value (value not needed at this phase). Thus we just
//      increments the frequency of bits_in_code by one.
//        
//  Parameters:
//
//    int bits_in_code: number of significant bits in value to dc encode
//    int dc_table_id:  the identifier for the dc table
//
//  Returns:
//        
//        XIL_FAILURE if an error occurred else XIL_SUCCESS
//
//  Notes:
//
//      Return value ignored.
//  
//------------------------------------------------------------------------

int JpegOptHuffmanEncoder::getDcStats(int bits_in_code, int dc_table_id)
{
    // check cache 
    if(dc_table_id != last_dc_table) {
        // cache miss, look up index
        if((last_dc_index = tableIndex(dc_table_id)) == -1) {
            // Jpeg bitstream error: invalid dc table identifier  
            XIL_ERROR( NULL, XIL_ERROR_USER, "di-84", TRUE);
            return XIL_FAILURE;
        } else {
            last_dc_table = dc_table_id;
        }
    }

    //  check assertion that bits < 16
    if(bits_in_code >= 16) {
        //fprintf(stderr,"invalid dc symbol %d\n",bits_in_code);
        return XIL_FAILURE;
    }

    // increment bits_in_code frequency
    symbol_freq[last_dc_index][bits_in_code] = symbol_freq[last_dc_index][bits_in_code] + 1;
    return XIL_SUCCESS;  
}

//------------------------------------------------------------------------
//
//  Function:        JpegOptHuffmanEncoder::getAcStats(int zeros, int bits_in_code,
//                                                int dc_table_id)
//  Created:        92/09/ 1
//
//  Description:
//        
//        The parameter bits_in_code summed to the value of zeros left
//      shifted to the left four is what is used to later encode the
//      desired value (value not needed at this phase). Thus we just
//      increments the frequency of this symbol by one.
//        
//  Parameters:
//
//    int zeros:        number of runs of zeros preceding value to encode
//    int bits_in_code: number of significant bits in value to ac encode
//    int dc_table_id:  the identifier for the dc table
//
//  Returns:
//        
//        XIL_FAILURE if an error occurred else XIL_SUCCESS
//
//  Notes:
//
//      Return value ignored.
//  
//------------------------------------------------------------------------

int JpegOptHuffmanEncoder::getAcStats(int zeros, int bits_in_code, int ac_table_id)
{
    // check cache 
    if(ac_table_id != last_ac_table) {
        // cache miss, look up index
        if((last_ac_index = tableIndex(ac_table_id)) == -1) {
            // Jpeg bitstream error: invalid ac table identifier  
            XIL_ERROR( NULL, XIL_ERROR_USER, "di-83", TRUE);
            return XIL_FAILURE;
        } else {
            last_ac_table = ac_table_id;
        }
    }

    // get symbol
    int symbol = (zeros << 4) + bits_in_code;

    //  check assertion that bits < NUM_SYMBOLS
    if(symbol >= NUM_SYMBOLS) {
        // fprintf(stderr,"invalid ac symbol %d\n",symbol);
        return XIL_FAILURE;
    }

    // increment symbol frequency
    symbol_freq[last_ac_index][symbol] = symbol_freq[last_ac_index][symbol] + 1;

    return XIL_SUCCESS;
}

//------------------------------------------------------------------------
//
//  Function:        JpegOptHuffmanEncoder::initTables()
//  Created:        92/09/ 2
//
//  Description:
//        
//        Deletes values old huffman tables and then resets values in
//      tables used during huffman table generation.
//
//      Initial values and tables taken from ISO spec.
//        
//------------------------------------------------------------------------

void JpegOptHuffmanEncoder::initTables()
{
    int t,j;
    JpegNode* p;
    HTable* table;

    for(p = cur_htables->head(); p!=NULL; p = p->next) {
        table = (HTable*) p->dataPtr;
        for(t=0; t<cur_num_tables; t++) {
            if(htables[t] == table) {
                htables[t] = NULL ;
            }
        }
    }
    cur_htables->emptyList();

    for(t = 0; t < num_tables; t++) {

        for(j = 0; j < NUM_SYMBOLS; j++) {
            code_size[t][j] = 0;
            others[t][j] = -1;
        }

        symbol_freq[t][NUM_SYMBOLS] = 1;
        code_size[t][NUM_SYMBOLS] = 0;
        others[t][NUM_SYMBOLS] = -1;

        for(j = 0; j < 33; j++)
        bits[t][j] = 0;
    }
}

//------------------------------------------------------------------------
//
//  Function:        JpegOptHuffmanEncoder::findHuffmanCodeSizes(int t)
//  Created:        92/09/ 2
//
//  Description:
//        
//        Alg K.1 of ISO Jpeg Spec: given a list of frequencies at which
//      a symbol occurs, this algorithm assigns sizes to the symbols
//      such that the symbol with the largest frequency has the smallest
//      size.
//        
//  Parameters:
//        
//        int t:  table index
//
//------------------------------------------------------------------------

void JpegOptHuffmanEncoder::findHuffmanCodeSizes(int t)
{
    int done1,done2,done3;
    int v1,v2;
    int j;

    done1 = FALSE;
    while(!done1) {

        v1 = -1;
        v2 = -1;

        for(j=0; j <= symbol_count[t]; j++)
        if(symbol_freq[t][j] > 0) {
            if(v1 == -1) {
                v1 = j;
            } else {
                if(symbol_freq[t][j] <= symbol_freq[t][v1]) {
                    v2 = v1;
                    v1 = j;
                } else {
                    if(v2 == -1) {
                        v2 = j;
                    } else {
                        if(symbol_freq[t][j] <= symbol_freq[t][v2]) {
                            v2 = j;                  
                        }
                    }
                }
            }
        }
        if(v2 != -1) {

            symbol_freq[t][v1] = symbol_freq[t][v1] + symbol_freq[t][v2];
            symbol_freq[t][v2] = 0;

            done2 =  FALSE;
            while(!done2) {
                code_size[t][v1] = code_size[t][v1] + 1;

                if(others[t][v1] != -1) {
                    v1 = others[t][v1];
                } else {
                    done2 = TRUE;
                }
            }

            others[t][v1] = v2;

            done3 = FALSE;        
            while(!done3) {
                code_size[t][v2] = code_size[t][v2] + 1;
                if(others[t][v2] != -1) {
                    v2 = others[t][v2];
                } else {
                    done3 = TRUE;
                }
            }

        } else {
            done1 = TRUE;
        }
    }
}

//------------------------------------------------------------------------
//
//  Function:        JpegOptHuffmanEncoder::findNumberOfCodes(int t)
//  Created:        92/09/ 2
//
//  Description:
//        
//        Alg K.2 of ISO jpeg spec: Determines the number of codes of each
//      size. This count is saved in table bits.
//        
//  Parameters:
//        
//        int t: table index
//
//------------------------------------------------------------------------

void JpegOptHuffmanEncoder::findNumberOfCodes(int t)
{
    int i;
    int size;

    int count = symbol_count[t];
    for(i=0; i <= count; i++) {
        size = code_size[t][i];
        if(size != 0) {
            bits[t][size] += 1;
        }
    }
}

//------------------------------------------------------------------------
//
//  Function:        JpegOptHuffmanEncoder::limitCodeLengths(int t)
//  Created:        92/09/ 2
//
//  Description:
//        
//        Alg K.3 of ISO jpeg spec: Code lengths can not be greater than
//      16 bits. This algorithm is used to limit the code lenghts to
//      16 bits.
//        
//  Parameters:
//        
//        int t: table index
//        
//------------------------------------------------------------------------

void JpegOptHuffmanEncoder::limitCodeLengths(int t)
{
    int j;
    int i = 32;
    int done = FALSE;

    while(!done) {
        if(bits[t][i] > 0) {

            j = i - 1;

            do 
            j = j - 1;
            while(bits[t][j] <= 0);

            bits[t][i] = bits[t][i] - 2;
            bits[t][i-1] = bits[t][i-1] + 1;
            bits[t][j+1] = bits[t][j+1] + 2;
            bits[t][j] = bits[t][j] - 1;

        } else {

            i = i - 1;
            if(i == 16) {

                while(i >= 0 && bits[t][i] == 0)
                i = i - 1;

                if(i >= 0) {
                    bits[t][i] = bits[t][i] - 1;
                }

                done = TRUE;
            }
        }
    }
}

//------------------------------------------------------------------------
//
//  Function:        JpegOptHuffmanEncoder::sortByCodeSize(int t)
//  Created:        92/09/ 2
//
//  Description:
//        
//        Alg. K.4 of ISO jpeg spec: The input values are sorted accdoring
//      code size.
//        
//  Parameters:
//        
//        int t: table index
//        
//------------------------------------------------------------------------

void JpegOptHuffmanEncoder::sortByCodeSize(int t)
{
    int j;
    int i = 1;
    int k = 0;

    do {
        j = 0;

        do {
            if(code_size[t][j] == i) {
                huffval[t][k] = j;
                k = k + 1;
            }
            j = j + 1;
        } while(j <= symbol_count[t] - 1);

        i = i + 1;

    } while(i<=32);
}

//------------------------------------------------------------------------
//
//  Function:        JpegOptHuffmanEncoder::generateHuffmanCodeSizes(int t)
//  Created:        92/09/ 2
//
//  Description:
//        
//        Alg. C.1 ISO jpeg spec: The previous algorithms formed huffman
//      tables in an interchange format. The next set of algorithms,
//      including this one, convert these tables into tables of codes
//      and code lenghts. This routine generates code sizes.
//        
//  Parameters:
//        
//        int t: table index
//        
//------------------------------------------------------------------------

void JpegOptHuffmanEncoder::generateHuffmanCodeSizes(int t)
{
    int k = 0;
    int i,j;

    i = j = 1;

    do {

        while(j <= bits[t][i]) {

            huffsize[t][k] = i;
            k++;
            j++;
        }

        i++;
        j = 1;

    } while( i <= 16 );

    huffsize[t][k] = 0;
    lastk[t] = k;
}

//------------------------------------------------------------------------
//
//  Function:        JpegOptHuffmanEncoder::generateHuffmanCodes(int t)
//  Created:        92/09/ 2
//
//  Description:
//        
//
//        Alg. C.2 ISO jpeg spec: The previous algorithms formed huffman
//      tables in an interchange format. The next set of algorithms,
//      including this one, convert these tables into tables of codes
//      and code lenghts. This routine generates codes given the
//      code sizes generated from the generateHuffmanSizes.
//        
//  Parameters:
//        
//        int t: table index
//        
//------------------------------------------------------------------------

void JpegOptHuffmanEncoder::generateHuffmanCodes(int t)
{
    int k, code;
    int si = huffsize[t][0];
    int done = FALSE;

    k = code = 0;

    while(!done) {
        do {

            huffcode[t][k] = code;
            code++;
            k++;

        } while(huffsize[t][k] == si);

        if(huffsize[t][k] == 0) {
            done = TRUE;
        } else {

            do {

                code = code << 1;
                si++;

            } while(huffsize[t][k] != si);
        }
    }
}

//------------------------------------------------------------------------
//
//  Function:        JpegOptHuffmanEncoder::createCodeTables(int t)
//  Created:        92/09/ 2
//
//  Description:
//        
//      Alg. C.3 of ISO jpeg spec: given the list of huffman code
//      sizes and codes, this algorithm orders the codes and sizes
//      and then adds them into a huffman table. The huffman table
//      is then inserted into its slot and a pointer to the table
//      is appened to the htable list (The elements on the htable
//      will be removed after the encoder is flushed).
//        
//  Parameters:
//        
//        int t: table index
//        
//------------------------------------------------------------------------

void JpegOptHuffmanEncoder::createCodeTables(int t)
{
    HTable* opt_htable = new HTable(16,symbol_count[t]);

    // INITIALZE CODES AND SIZES
    for(int i=0; i<symbol_count[t]; i++) {
        ehuffco[t][i] = 0;
        ehuffsi[t][i] = 0;
    }

    int k = 0;

    do{
        i = huffval[t][k];
        ehuffco[t][i] = huffcode[t][k];
        ehuffsi[t][i] = huffsize[t][k];
        k++;
    } while( k < lastk[t] );

    for(i=0; i<symbol_count[t]; i++) {
        opt_htable->Add_Code((unsigned short)ehuffsi[t][i],
                             (unsigned short)ehuffco[t][i]);
    }

    Add_Table(opt_htable,htable_ids[t]);

    cur_htables->append(opt_htable);
}

//------------------------------------------------------------------------
//
//  Function:        JpegOptHuffmanEncoder::generateTables()
//  Created:        92/09/ 2
//
//  Description:
//        
//      For each table in use, this routine calls upon the routines
//      to generate huffman tables. The statistics required should
//      have already been aquired via the pseudo encoding process.
//
//      Once the tables are generated, the symbol frequencies are
//      reset.
//        
//------------------------------------------------------------------------

void JpegOptHuffmanEncoder::generateTables(int dc_id, int ac_id)
{

    int t,j;

    initTables();

    for(t= 0; t<num_tables; t++) {
        if(tableInUse(t)) {
            if((dc_id == -1 || tableIndex(dc_id) == t) ||
            (ac_id == -1 || tableIndex(ac_id) == t)) {

                findHuffmanCodeSizes(t);
                findNumberOfCodes(t);
                limitCodeLengths(t);
                sortByCodeSize(t);
                generateHuffmanCodeSizes(t);
                generateHuffmanCodes(t);      
                createCodeTables(t);
            }
        }
    }

    // reset symbol frequencies for next time around
    for(t= 0; t<num_tables; t++) {
        for(j=0; j<NUM_SYMBOLS; j++) {
            symbol_freq[t][j] = 0;
        }

        symbol_freq[t][NUM_SYMBOLS] = 1;
        symbol_freq[t][symbol_count[t]] = 1;
    }

    // reset the huffman encoder 
    Jpeg_Huffman_Encoder::Reset();
}


//------------------------------------------------------------------------
//
//  Function:        JpegOptHuffmanEncoder::Flush_Codes()
//  Created:        92/09/ 2
//
//  Description:
//        
//        The optimal tables should have been generated before this routine.
//      This routine simply re-encodes the blocks that were previously
//      processed. The normal jpeg huffman encoding routine is used.
//
//      Once done, the huffman encoder is flushed and all blocks are
//      deleted.
//        
//------------------------------------------------------------------------

void JpegOptHuffmanEncoder::Flush_Codes()
{
    JpegNode* pos;
    JpegHuffmanBlock* hblock;

    for(pos = hblocks->head();
    pos != NULL;
    pos = pos->next) {
        hblock = (JpegHuffmanBlock*) pos->dataPtr;
        Jpeg_Huffman_Encoder::Encode(hblock->block, 
        hblock->component, 
        hblock->dc_table_id, 
        hblock->ac_table_id);
    }

    Huffman_Encoder::Flush_Codes();

    hblocks->emptyList();
}
