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
//  File:       Huffman_Encoder.cc
//  Project:    XIL
//  Revision:   1.6
//  Last Mod:   10:16:08, 03/10/00
//
//  Description:
//
//    Class to perform Huffman variable length coding of
//    a stream of symbols.
//
//
//------------------------------------------------------------------------
//    COPYRIGHT
//------------------------------------------------------------------------
#pragma ident   "@(#)Huffman_Encoder.cc	1.6\t00/03/10  "

#include "Huffman_Encoder.hh"

//------------------------------------------------------------------------
//
//  Function:        Huffman_Encoder::Init
//
//  Description:
//        
//        initializes the abstract class (like a constructor)
//
//------------------------------------------------------------------------

int 
Huffman_Encoder::Init(unsigned int  nt, 
                      Xil_unsigned8 mark, 
                      SingleBuffer* buf)
{
    unsigned int i;

    isOKFlag       = FALSE ;

    num_tables     = nt;
    cur_num_tables = 0;  

    htables = new HTable*[num_tables];
    htable_ids = new int[num_tables];  

    // create table usage array
    tables_in_use = new int[num_tables];
    tables_loaded = new int[num_tables];
    table_outputted = new int[num_tables];  

    if(htables == NULL || htable_ids == NULL || tables_in_use == NULL ||
     tables_loaded == NULL || table_outputted == NULL) {
        XIL_ERROR(NULL,XIL_ERROR_RESOURCE,"di-1",TRUE);
        return XIL_FAILURE;
    }
    for(i = 0; i < num_tables; i++) {
        htables[i] = NULL;
        htable_ids[i] = -1;
        tables_in_use[i] = 0;
        tables_loaded[i] = 0;
        table_outputted[i] = 0;        
    }

    marker = mark;
    buffer = buf;

    bits_leftover = 0;
    lastbyte = 0;
    invalid_code = 0xff;
    stuffer = 0x00;

    isOKFlag = TRUE;

    return XIL_SUCCESS ;
}


//------------------------------------------------------------------------
//
//  Function:        Huffman_Encoder::Delete()
//
//  Description:
//        
//        Like a destructor
//
//------------------------------------------------------------------------

void 
Huffman_Encoder::Delete()
{
    int i;

    for(i=0; i< num_tables; i++) {
        if(htables[i] != NULL && htables[i]->getDeleteByCreator() == FALSE) {
            delete htables[i];
            htables[i] = NULL;
        }
    }

    delete []htables;
    delete []htable_ids;
    delete []tables_in_use;
    delete []tables_loaded;
    delete []table_outputted;
}

//------------------------------------------------------------------------
//
//  Function:        Huffman_Encoder::Table(int table_id) const
//
//  Description:
//        
//        Look and return a htable given its id
//
//------------------------------------------------------------------------

HTable* 
Huffman_Encoder::Table(int table_id) const
{
    int i;

    for(i=0; i< num_tables; i++) {
        if(htable_ids[i] == table_id) {
            return htables[i];
        }
    }
    return NULL;
}

//------------------------------------------------------------------------
//
//  Function:        Huffman_Encoder::tableIndex( int table_id ) const
//
//  Description:
//        
//        Look up and return the index of a table given its id
//
//------------------------------------------------------------------------

int  
Huffman_Encoder::tableIndex(int table_id ) const
{
    int i;

    for(i=0; i< num_tables; i++) {
        if(htable_ids[i] == table_id) {
            return i;
        }
    }
    return -1;
}

//------------------------------------------------------------------------
//
//  Function:        Huffman_Encoder::Add_Table(HTable* ht, int t_id)
//
//  Description:
//        
//        Add a table or insert a table given a table id. If the
//      table id is already in use, then we insert this new table
//      into the other table of the same id's slot. Otherwise, we
//      add this table into the next avail slot.
//
//------------------------------------------------------------------------

int 
Huffman_Encoder::Add_Table(HTable* ht, 
                           int     t_id)
{
    int table_slot;

    if((table_slot = tableIndex(t_id)) == -1) {  
        if(ValidTable(cur_num_tables)) {
            htables[cur_num_tables] = ht;
            htable_ids[cur_num_tables] = t_id;
            tables_loaded[cur_num_tables] = 1;
            table_outputted[cur_num_tables] = 0;
            cur_num_tables += 1;
            return cur_num_tables - 1;
        }
        else {
            // Exceeded number of htables
            // Jpeg bitstream error: tableindex too large
            XIL_ERROR( NULL, XIL_ERROR_USER, "di-94", TRUE);
            return -1;
        }
    } else {

        // the slot for the table is already filled by another
        // huffman table. If this table is not flaged as being
        // delete_by_creator, then go ahead and delete the table.

        if(htables[table_slot] != NULL &&
           htables[table_slot]->getDeleteByCreator() == FALSE) {
            delete htables[table_slot];
        }

        // swap in new table

        htables[table_slot] = ht;
        tables_loaded[table_slot] = 1;
        table_outputted[table_slot] = 0;
        return table_slot;
    }
}


//------------------------------------------------------------------------
//
//  Function:        Huffman_Encoder::Add_Table(HTable& ht, int t_id)
//
//  Description:
//        
//        Add a table or insert a table given a table id. If the
//      table id is already in use, then we insert this new table
//      into the other table of the same id's slot. Otherwise, we
//      add this table into the next avail slot.
//
//------------------------------------------------------------------------

int 
Huffman_Encoder::Add_Table(HTable& ht, 
                           int     t_id)
{
    int table_slot;

    if((table_slot = tableIndex(t_id)) == -1) {  
        if(ValidTable(cur_num_tables)) {
            htables[cur_num_tables] = &ht;
            htable_ids[cur_num_tables] = t_id;
            tables_loaded[cur_num_tables] = 1;
            table_outputted[cur_num_tables] = 0;      
            cur_num_tables += 1;
            return cur_num_tables - 1;
        }
        else {
            // Exceeded number of htables 
            // Jpeg bitstream error: tableindex too large 
            XIL_ERROR( NULL, XIL_ERROR_USER, "di-94", TRUE); 
            return -1;
        }
    } else {


        // the slot for the table is already filled by another
        // huffman table. If this table is not flaged as being
        // delete_by_creator, then go ahead and delete the table.

        if(htables[table_slot]->getDeleteByCreator() == FALSE) {
            delete htables[table_slot];
        }

        // swap in new table

        htables[table_slot] = &ht;
        tables_loaded[table_slot] = 1;
        table_outputted[table_slot] = 0;
        return table_slot;
    }
}


//------------------------------------------------------------------------
//
//  Function:        Huffman_Encoder::nonsignbits(int x)
//
//  Description:
//        
//        Return the number of sig bits in a value
//
//------------------------------------------------------------------------

int 
Huffman_Encoder::nonsignbits(int x)
{
    int count;

    if(x < 0) {
        x = -x;
    }

    count = 0;
    while(x != 0) {
        x >>= 1;
        count += 1;
    }
    return (count);
}


//------------------------------------------------------------------------
//
//  Function:        Huffman_Encoder::Put_Code(int code, int codelen,
//                                        int bits, int value)
//
//  Description:
//        
//        Put a code out to the byte-stream. This routine handles the
//      bit buffering and outputting bytes at a time.
//
//------------------------------------------------------------------------

void 
Huffman_Encoder::Put_Code(int code, 
                          int codelen, 
                          int bits, 
                          int value)
{
    int databyte, shift;

    // The value is encoded as a variable length code.  If the
    // value is positive, it is already coded,
    // if value is negative mask off lower bits and subtract
    //  one  (same as adding (2**bits - 1))
    // concatenate value onto the end of the code word

    value = (value < 0) ? ((value & ((1 << bits) - 1)) - 1) : value;
    code = (code << bits) | value;

    // adjust length to comphensate for 
    // leftover bits and the bit of "value"

    codelen += bits_leftover + bits;

    //        stick code into bytes
    databyte = lastbyte;
    while(codelen >= 8) {
        // need to break up code into byte-sized chunks
        shift = codelen - 8;
        databyte = databyte | (code >> shift);
        code = code & ((1 << shift) - 1);
        codelen = shift;

        Output_Byte(databyte);
        databyte = 0;
    }

    if(codelen > 0) {
        // merge remainder into last "leftover" byte
        shift = 8 - codelen;
        databyte = databyte | (code << shift);

    }
    bits_leftover = codelen;
    lastbyte = databyte;
}

//------------------------------------------------------------------------
//
//  Function:        Huffman_Encoder::Output_Byte(int val)
//
//  Description:
//        
//        Outputs a byte to the byte-stream. If the byte is equiv to the
//      marker code, add a stuffer byte.
//
//------------------------------------------------------------------------

void 
Huffman_Encoder::Output_Byte(int val)
{
    buffer->addByte(val); 
    if((val) == invalid_code)  {
        buffer->addByte(stuffer); 
    }
}

//------------------------------------------------------------------------
//
//  Function:        Huffman_Encoder::Flush_Codes( )
//
//  Description:
//        
//        Flush any remaining bytes to the byte-stream padding the last
//      bits with ones.
//
//------------------------------------------------------------------------

void 
Huffman_Encoder::Flush_Codes( )
{
    int bits = bits_leftover;
    int databyte;

    // pad the leftover bits of the lastbyte 
    // with ones until it is byte aligned

    if(bits == 0) {
        return;
    }

    bits = 8 - bits;
    databyte = lastbyte | ((1 << bits) - 1);

    Output_Byte(databyte);
    bits_leftover = 0;
    lastbyte = 0;
}

//------------------------------------------------------------------------
//
//  Function:        Huffman_Encoder::Reset()
//
//  Description:
//        
//        Reset huffman state
//
//------------------------------------------------------------------------

void 
Huffman_Encoder::Reset()
{
    bits_leftover = 0;
    lastbyte = 0;
}


//------------------------------------------------------------------------
//
//  Function:        Huffman_Encoder::resetTableUsage()
//
//  Description:
//        
//        Resets table usage. A table is in use if a band uses it during
//      compression.
//
//------------------------------------------------------------------------

void 
Huffman_Encoder::resetTableUsage()
{
    for(int i = 0; i < num_tables; i++) {
        tables_in_use[i] = 0;
    }
}

//------------------------------------------------------------------------
//
//  Function:        Huffman_Encoder::tableInUse(int table)
//
//  Description:
//        
//        Return the fact that a table is in use by a band of the current
//      image being compressed.
//        
//------------------------------------------------------------------------

int 
Huffman_Encoder::tableInUse(int table)
{
    if(ValidTable(table)) {
        return tables_in_use[table];
    } else {
        // Invalid htable identifier
        XIL_ERROR( NULL, XIL_ERROR_USER, "di-90", TRUE);
        return 0;
    }
}


//------------------------------------------------------------------------
//
//  Function:        Huffman_Encoder::usingTable(int table_id)
//
//  Description:
//        
//        
//        Set the fact that a band is using a given table id.
//        
//------------------------------------------------------------------------

void 
Huffman_Encoder::usingTable(int table_id)
{
    int table_index;

    if((table_index = tableIndex(table_id)) == -1) {

        // Invalid htable identifier 
        XIL_ERROR( NULL, XIL_ERROR_USER, "di-90", TRUE); 
    } else {
        tables_in_use[table_index] = 1;
    }
}


//------------------------------------------------------------------------
//
//  Function:        Huffman_Encoder::numTablesInUse()
//
//  Description:
//        
//        Return the number of tables being used.
//        
//------------------------------------------------------------------------

int 
Huffman_Encoder::numTablesInUse()
{
    int num_tables_in_use = 0;

    for(int t = 0; t < num_tables; t++) {
        if(tables_in_use[t]) {
            num_tables_in_use++;
        }
    }

    return num_tables_in_use;
}


//------------------------------------------------------------------------
//
//  Function:        Huffman_Encoder::tableLoaded(int table_id)
//
//  Description:
//        
//        Return the fact that a table of a given id has been loaded.
//        
//------------------------------------------------------------------------

int 
Huffman_Encoder::tableLoaded(int table_id)
{
    int table_index;

    if((table_index = tableIndex(table_id)) == -1) {
        // Invalid htable identifier 
        XIL_ERROR( NULL, XIL_ERROR_USER, "di-90", TRUE); 
        return 0;
    } else {
        return tables_loaded[table_index];
    }
}

//------------------------------------------------------------------------
//
//  Function:        Huffman_Encoder::tableOutputted(int table)
//
//  Description:
//        
//        Return the fact that a table of a given id has been outputted.
//        
//------------------------------------------------------------------------

int 
Huffman_Encoder::tableOutputted(int table)
{
    if(ValidTable(table)) {
        return table_outputted[table];
    } else {
        //
        // Invalid htable index
        // Jpeg bitstream error: invalid htable identifier 
        //
        XIL_ERROR( NULL, XIL_ERROR_USER, "di-90", TRUE); 
        return 0;
    }
}
