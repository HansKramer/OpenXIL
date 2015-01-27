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
//  File:       Huffman_Encoder.hh
//  Project:    XIL
//  Revision:   1.4
//  Last Mod:   10:23:36, 03/10/00
//
//  Description:
//
//   A Huffman_Encoder is an abstract class that maintains set of
//   Huffman tables (HTables) and their identifiers. The class is
//   meant for derivation which should define Encode and Output
//   routines. The Output routine is the only one defined here
//   and is made pure. Therefore, if a derived class does not
//   defines Output, it shall also be abstract.
//
//   The class uses a XilCisBuffer object through which to do output.
//   A pointer to a created XilCisBuffer object must be supplied
//   either through the Init routine or via normal set_ function
//   before operatinf with the encoder.
//
//   This class provides methods for "putting" variable lengths 
//   codes, putting bytes and flushing codes output to the 
//   outstream. All of these routines are virtual and therefore 
//   may be redefined by the derived class.
//
//   For output, the class maintains three types of markers. The
//   first, called a marker, defines the marker code for Huffman
//   tables. The invalid_code marker can be set to a code which
//   should be treated as invalid. When this code appears in the
//   byte stream, the default Output_Byte routine will stuff the
//   stuffer marker byte into the stream instead of the invalid
//   code. Again, these routines are virtual and can be redefined.
//        
// Notes:
//
//   See Jpeg_Huffman_Encoder class for example of using this
//   Huffman_Encoder object.
//        
// Deficiencies/ToDo:
//
//
//------------------------------------------------------------------------
//    COPYRIGHT
//------------------------------------------------------------------------
#pragma ident   "@(#)Huffman_Encoder.hh	1.4\t00/03/10  "

#ifndef HUFFMAN_ENCODER_H
#define HUFFMAN_ENCODER_H

#include "xil/xilGPI.hh"
#include "HTable.hh"
#include "SingleBuffer.hh"

#define ALL_HTABLES -1

//
// Class definition
//
class Huffman_Encoder {
public:

    int     Init(unsigned int nt, Xil_unsigned8 m, SingleBuffer* buf);

    void    Delete();
    int     tableIndex( int t_id ) const;

    int     Add_Table(HTable* ht, int t_id);
    int     Add_Table(HTable& ht, int t_id);

    void    set_Buffer(SingleBuffer* buf)  { buffer = buf; }
    void    set_Marker(Xil_unsigned8 m)          { marker = m; }
    void    set_Stuffer(Xil_unsigned8 s)         { stuffer = s; }
    void    set_Invalid_Code(Xil_unsigned8 i)    { invalid_code = i; }

    void    set_Output_Codes(Xil_unsigned8 m, Xil_unsigned8 s, Xil_unsigned8 i)
    { marker = m; stuffer = s; invalid_code = i; }

    HTable* Table(int table_id) const;  
    HTable* operator[](int table_id) const { return Table(table_id); }

    Xil_unsigned8    Num_Tables() const { return num_tables; }

    void    resetTableUsage();
    void    usingTable(int table_id);
    int     numTablesInUse();
    int     tableLoaded(int table_id);


    virtual void Put_Code(int code, int codelen, int bits, int value);  
    virtual void Flush_Codes();
    virtual void Reset();

    virtual void Output(int table_id = ALL_HTABLES) = 0;

protected:

    HTable**            htables;
    int*                htable_ids; 
    int*                tables_in_use;
    int*                tables_loaded;  
    int*                table_outputted;
    Xil_boolean         isOKFlag;

    Xil_unsigned8       num_tables;
    Xil_unsigned8       cur_num_tables;  


    Xil_unsigned8       marker;
    Xil_unsigned8       invalid_code;  
    Xil_unsigned8       stuffer;

    SingleBuffer*       buffer;

    int                 bits_leftover;
    Xil_unsigned8       lastbyte;

    int          tableInUse(int table);
    int          tableOutputted(int table);

    int          ValidTable(unsigned int t)
    const { return ((int)t>=0 && t<num_tables) ? 1 : 0; }


    int          nonsignbits( int );

    virtual void Output_Byte(int val);
};

#endif

