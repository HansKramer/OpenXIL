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
//  File:       HTable.hh
//  Project:    XIL
//  Revision:   1.2
//  Last Mod:   10:23:37, 03/10/00
//
//  Description:
//
//   An HTable, or Huffman Table, is an object that stores a table
//   of variable length values. It also stores an array of symbol
//   lists (list of unsigned bytes). The index of the array repre-
//   sents the size of the huffman codes represented in the list.
//
//   Each HTable must be assigned a unique identifier by which it
//   can be distinguished from other HTables by a Huffman_Encoder
//   object. Routines exist to set and retrieve (by val or ref)
//   this id.
//
//   "Add" routines allow one to add codes in a variety of formats
//   to the huffman table. The symbols for the huffman table are
//   determined by the order in which codes are added to the table.
//
//------------------------------------------------------------------------
//    COPYRIGHT
//------------------------------------------------------------------------
#pragma ident   "@(#)HTable.hh	1.2\t00/03/10  "

#ifndef HTABLE_H
#define HTABLE_H

class Huffman_Encoder;

#include <xil/xilGPI.hh>
#include "Huffman_Code.hh"
#include "JpegByteArray.hh"

class HTable {
public:

    HTable(unsigned int nl         = 16, 
           unsigned int table_size = 0, 
           Xil_boolean dbc         = TRUE);

    ~HTable();

    void           Add_Code(const Huffman_Code &hc);
    void           Add_Code(unsigned short length, unsigned short code);
    void           Add_Code(unsigned int hc_array_size, Huffman_Code *hc_array );

    int            Num_Codes() const;
    int            Num_Symbols() const;        
    int            Num_Lengths() const           { return num_lengths; }

    JpegByteArray* SymbolTable(int t);        

    Huffman_Code*  Get_Code(int v);                
    Huffman_Code*  operator[](int v);

    Xil_boolean    getDeleteByCreator() const  {return delete_by_creator ; } 

    Xil_boolean    isOK();

    friend class   Huffman_Encoder;

private:
    JpegByteArray** symbols; // list of symbol values
    Huffman_Code**  htable;  // entire huffman table

    unsigned int    num_symbols;        
    unsigned int    num_codes;
    unsigned int    num_lengths;
    Xil_unsigned8   size_flag;
    Xil_boolean     delete_by_creator;
    Xil_boolean     isOKFlag;

    Xil_boolean     ValidTable(unsigned int t) const
        { return (t>=1 && t<=num_lengths); }

};

#endif
