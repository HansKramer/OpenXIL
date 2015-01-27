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
//  File:       HTable.cc
//  Project:    XIL
//  Revision:   1.4
//  Last Mod:   10:16:06, 03/10/00
//
//  Description:
//
//    Implementation of HTable Class
//
//
//------------------------------------------------------------------------
//    COPYRIGHT
//------------------------------------------------------------------------
#pragma ident   "@(#)HTable.cc	1.4\t00/03/10  "


#include "HTable.hh"

//------------------------------------------------------------------------
//
//  Function:   HTable::HTable( unsigned int nl, unsigned int table_size)
//
//  Description:
//        
//   Constructor for Huffman Table. A HTable is composed of:
//
//    htable: list of Huffman_Codes
//
//    symbols: an array of lists containing unsigned bytes. The array
//             index represent the length of the codes in list it points
//             to. The value in the 
//        
//        
//------------------------------------------------------------------------

HTable::HTable(unsigned int nl, 
               unsigned int ,
               Xil_boolean  dbc)
{
    isOKFlag = FALSE;

    //
    // Initialize HTable
    //
    num_codes         = 0;
    num_symbols       = 0;
    num_lengths       = nl;
    delete_by_creator = dbc;

    //
    // Use a list of arrays to hold the symbols.
    // Array symbols[i] holds the symbols of length i. 
    // Symbol[0] is empty. For Jpeg num_lengths is 16.
    // Here we take advantage of the fact that the maximum
    // number of symbols is 256. Therefore, no symbol array
    // can be bigger than that. The maximum size this whole
    // structure can get to is 10x256 = 2560, so this seems small
    // enough that the wasted space shouldn't be a concern.
    //

    // Allocate array of arrays
    symbols = new JpegByteArray*[num_lengths+1];
    if(symbols == NULL) {
        XIL_ERROR(NULL, XIL_ERROR_RESOURCE, "di-1", TRUE);
        return;
    }

    // Allocate individual arrays
    symbols[0] = NULL; // Empty slot
    for(int nbits=1; nbits<=num_lengths; nbits++) {
        int nbins = 1 << nbits;
        int nsyms = (nbins < 256) ? nbins : 256;
        symbols[nbits] = new JpegByteArray(nsyms);
        if(symbols[nbits] == NULL) {
            XIL_ERROR(NULL, XIL_ERROR_RESOURCE, "di-1", TRUE);
            return;
        }
    }

    // 
    // Change for v1.3
    //
    // Allocate the Array of HuffmanCodes as a simple array
    // rather than a list. This is completely safe because 
    // Jpeg limits the symbol size to 8 bits, even for the
    // extended precision mode (which we DON'T support now).
    //
    htable = new Huffman_Code*[256];
    if(htable == NULL) {
        XIL_ERROR(NULL, XIL_ERROR_RESOURCE, "di-1", TRUE);
        return;
    }

    //
    // Initialize all pointers to NULL.
    // This simplifies the destructor, which will always
    // just delete all 256 elements.
    //
    for(int i=0; i<256; i++) {
        htable[i] = NULL;
    }

    isOKFlag = TRUE;
}

HTable::~HTable( void )
{
    //
    // Delete the Huffman code elements
    // Delete thru NULL ptr is guaranteed harmless
    //
    for(int i=0; i<256; i++) {
        delete htable[i];
    }

    // Delete the array of Huffman Codes
    delete []htable;

    // Delete each of the symbol tables
    for(i=1; i<=num_lengths; i++) {
        delete symbols[i];
    }

    // Delete the symbol table array
    delete []symbols;
}

//------------------------------------------------------------------------
//
//  Function:   HTable::Add_Code(unsigned int  hc_array_size, 
//                               Huffman_Code* hc_array )
//
//  Description:
//        
//        Add codes given in the array to the htable.
//        
//------------------------------------------------------------------------

void 
HTable::Add_Code(unsigned int  hc_array_size, 
                 Huffman_Code* hc_array )
{
    int length;
    int i;
    Huffman_Code* hc;

    for(i=0; i<hc_array_size; i++) {

        // get length of code
        length = hc_array[i].length;

        // check to see if in correct range
        if(length <= num_lengths) {

            // create new huffman code and add to htable
            hc = new Huffman_Code;
            *hc = hc_array[i];
            htable[i] = hc;

            if(length>0) {
                Xil_unsigned8 sym = num_codes & 0xff;
                symbols[length]->append(sym);                  
                num_symbols++;
            }
            num_codes++;
        } else {
            // Invalid huffman code length
            XIL_ERROR( NULL, XIL_ERROR_SYSTEM, "di-69", FALSE); 
        }
    }
}

//------------------------------------------------------------------------
//
//  Function:  HTable::Add_Code(unsigned short length, unsigned short code)
//
//  Description:
//        
//        Add a given code to the htable.
//        
//------------------------------------------------------------------------

void 
HTable::Add_Code(unsigned short length, 
                 unsigned short code)
{
    // check to see if in correct range  
    if(length <= num_lengths) {

        // create new huffman code and add to htable         
        Huffman_Code* hc = new Huffman_Code;
        hc->Set(length,code);
        htable[num_codes] = hc;

        if(length>0) {
            Xil_unsigned8 sym = num_codes & 0xff;
            symbols[length]->append(sym);                  
            num_symbols++;        
        }
        num_codes++;
    } else {
        // Invalid huffman code length 
        XIL_ERROR( NULL, XIL_ERROR_SYSTEM, "di-69", FALSE);
    }
}

//------------------------------------------------------------------------
//
//  Function:        HTable::Add_Code(const Huffman_Code& huff_code)
//
//  Description:
//        
//        Add a huffman code to the htable
//        
//------------------------------------------------------------------------

void 
HTable::Add_Code(const Huffman_Code& huff_code)
{

    // get length of huffman code
    unsigned int length = huff_code.Get_Length();

    // check to see if in correct range    
    if(length <= num_lengths) {

        // create new huffman code and add to htable                  
        Huffman_Code* hc = new Huffman_Code;
        *hc = huff_code;
        htable[num_codes] = hc;


        if(length>0) {
            Xil_unsigned8 sym = num_codes & 0xff;
            symbols[length]->append(sym);                  
            num_symbols++;
        }
        num_codes++;
    } else {
        // Invalid huffman code length 
        XIL_ERROR( NULL, XIL_ERROR_SYSTEM, "di-69", FALSE);
    }
}


//------------------------------------------------------------------------
//
//  Function:        HTable::SymbolTable(int t)
//
//  Description:
//        
//        Return the symbol table for size t
//        
//------------------------------------------------------------------------

JpegByteArray*
HTable::SymbolTable(int t)
{
    if(ValidTable(t)) {
        return symbols[t];
    } else {
        // Invalid symbol table id  
        XIL_ERROR( NULL, XIL_ERROR_SYSTEM, "di-68", FALSE); 
        return NULL;
    }
}

//------------------------------------------------------------------------
//
//  Function:        HTable::Num_Codes() const
//
//  Description:
//        
//        return number of codes (SHOULD BE INLINED)
//        
//        
//------------------------------------------------------------------------

int 
HTable::Num_Codes() const
{
    return num_codes;
}

//------------------------------------------------------------------------
//
//  Function:        HTable::Num_Symbols() const
//
//  Description:
//        
//        
//        return number of symbols (SHOULD BE INLINED)
//        
//------------------------------------------------------------------------

int 
HTable::Num_Symbols() const
{
    return num_symbols;
}

//------------------------------------------------------------------------
//
//  Function:        HTable::Get_Code(int v)
//
//  Description:
//        
//        Look up a code given a value.
//        
//------------------------------------------------------------------------

Huffman_Code* 
HTable::Get_Code(int v)
{
    if(v < num_codes) {
        return htable[v];
    } else {
        // Invalid huffman code value index   
        XIL_ERROR( NULL, XIL_ERROR_SYSTEM, "di-67", FALSE);
        return NULL;
    }
}


//------------------------------------------------------------------------
//
//  Function:        HTable::operator[](int v)
//
//  Description:
//        
//        same as Get_Code (CAN JUST CALL Get_Code)
//        
//        
//------------------------------------------------------------------------

Huffman_Code* 
HTable::operator[](int v)
{
    if(v < num_codes) {
        return htable[v];
    } else {
        // Invalid huffman code value index    
        XIL_ERROR( NULL, XIL_ERROR_SYSTEM, "di-67", FALSE);
        return NULL;
    }
}

Xil_boolean
HTable::isOK()
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
