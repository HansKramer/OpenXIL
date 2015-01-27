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
//  File:       Jpeg_Huffman_Encoder_Base.cc
//  Project:    XIL
//  Revision:   1.3
//  Last Mod:   10:16:20, 03/10/00
//
//  Description:
//
//    Implementation of Jpeg_Huffman_Encoder_Base base class functions
//
//
//------------------------------------------------------------------------
//    COPYRIGHT
//------------------------------------------------------------------------
#pragma ident   "@(#)Jpeg_Huffman_Encoder_Base.cc	1.3\t00/03/10  "
 
  
#include <stdio.h>
#include "Jpeg_Huffman_Encoder_Base.hh"

Jpeg_Huffman_Encoder_Base::Jpeg_Huffman_Encoder_Base(unsigned int  nt,
                                                     unsigned int  nc,
                                                     Xil_unsigned8 mark,
                                                     SingleBuffer* buf)
{
    isOKFlag = FALSE;

    components = NULL;

    if(Init(nt, mark, buf) == XIL_FAILURE)  {
        XIL_ERROR( NULL, XIL_ERROR_RESOURCE,"di-1",FALSE);
        return;
    }

    num_components = nc;

    if(num_components) {
        components = new int[num_components];
        if(components==NULL) {
            XIL_ERROR( NULL, XIL_ERROR_RESOURCE,"di-1",TRUE);
            return;
        }
        for(int i=0; i<num_components; i++) {
            components[i] = 0;
        }
    }

    lookup_bit_table = new int[MAX_LOOKUP_SIZE];
    if(lookup_bit_table == NULL) {
        XIL_ERROR( NULL, XIL_ERROR_RESOURCE,"di-1",TRUE);
        delete [] components;
        return;
    }
    createBitSizeTableLookup(lookup_bit_table, MAX_LOOKUP_SIZE);

    isOKFlag = TRUE;
}

Jpeg_Huffman_Encoder_Base::~Jpeg_Huffman_Encoder_Base()
{
    Delete();

    delete []components;
    delete []lookup_bit_table ;
}

Xil_boolean
Jpeg_Huffman_Encoder_Base::isOK()
{
    if(this == NULL) {
        return FALSE;
    } else {
        if(isOKFlag == TRUE) {
            return TRUE;
        } else {
            delete this;
            return FALSE;
        }
    }
}

void 
Jpeg_Huffman_Encoder_Base::createBitSizeTableLookup(int*         ltable,
                                               unsigned int tableSize)
{
    int i;
    unsigned int n = 0 ;
    unsigned int temp = tableSize -1;

    while(temp != 0) {
        temp >>= 1;    // Get initial value of ltable
        ++n ;
    }
    ltable[0] = 0;
    ltable[1] = 1;   // Hard code the initial values;
    ltable[2] = 2;  // for speed's sake
    ltable[3] = 2;
    temp = n;
    for(i=4; i<tableSize; i++) {
        while(i <  (1 << temp)) {
            --temp ;
        }
        ltable[i] = temp+1;
        temp = n;
    }
}

//------------------------------------------------------------------------
//
//  Function:        Jpeg_Huffman_Encoder_Base::Output(int table_id)
//
//  Description:
//        
//        Output tables.
//        
//------------------------------------------------------------------------

void 
Jpeg_Huffman_Encoder_Base::Output(int table_id)
{
    int num_codes = 0;
    int num_lengths = 0;
    int i;
    HTable* table;
    JpegByteArray* symtab;  

    if(buffer == NULL) {
        //
        // Buffer does not exist - Internal error 
        //
        XIL_ERROR( NULL, XIL_ERROR_SYSTEM, "di-95", TRUE); 
        return;
    }

    if(table_id == ALL_HTABLES) {
        for(i=0; i< num_tables; i++) {
            if(tables_in_use[i]) {
                Output(htable_ids[i]);
            }
        }
        return;
    }

    int table_index = tableIndex(table_id);
    if(table_index == -1) {

        // Jpeg bitstream error: invalid table identifier
        XIL_ERROR( NULL, XIL_ERROR_USER, "di-82", TRUE);

        return;
    } else {
        table = htables[table_index];
    }

    if(! tableInUse(table_index) ) {

        // Jpeg bitstream error: table identifier not in use 
        XIL_ERROR( NULL, XIL_ERROR_USER, "di-81", TRUE);

        return; 
    }

    // define huffman table
    buffer->addByte(MARKER);
    buffer->addByte(marker);

    // determine number of codes
    num_codes = table->Num_Symbols();

    // output huffman table size 
    buffer->addShort(num_codes + 19);

    // output table id
    buffer->addByte(table_id);

    // output number of codes per bit length
    num_lengths = table->Num_Lengths();
    for(i=1; i<=num_lengths; i++) {
        buffer->addByte(table->SymbolTable(i)->length());
    }

    // output codes
    for(i=1; i<=num_lengths; i++) {
        symtab = table->SymbolTable(i);
        for(int pos=0; pos<symtab->length(); pos++) {
            buffer->addByte(symtab->retrieve(pos));
        }
    }

    table_outputted[table_index] = 1;
}

//------------------------------------------------------------------------
//
//  Function:        Jpeg_Huffman_Encoder_Base::OutputChanges(int table_id)
//
//  Description:
//        
//        Output tables if the have changed since they were last put
//      out to the byte stream.
//        
//------------------------------------------------------------------------

void 
Jpeg_Huffman_Encoder_Base::OutputChanges(int table_id)
{
    int i;

    if(table_id == ALL_HTABLES) {
        for(i=0; i< num_tables; i++) {
            if(tables_in_use[i] && !table_outputted[i]) {
                Output(htable_ids[i]);
            }
        }
        return;
    }

    int table_index;

    table_index = tableIndex(table_id);
    if(table_index == -1) {
        // Jpeg bitstream error: invalid table identifier  
        XIL_ERROR( NULL, XIL_ERROR_USER, "di-82", TRUE);   
        return;
    }

    if(tables_in_use[table_index] && !table_outputted[table_index]) {
        Output(table_id);
    }
}

//------------------------------------------------------------------------
//
//  Function:        Jpeg_Huffman_Encoder_Base::Reset()
//
//  Description:
//        
//        Reset the state of this object.
//        
//------------------------------------------------------------------------

void 
Jpeg_Huffman_Encoder_Base::Reset()
{
    for(int i=0; i<num_components; i++) {
        components[i] = 0;
    }

    Huffman_Encoder::Reset();
}
