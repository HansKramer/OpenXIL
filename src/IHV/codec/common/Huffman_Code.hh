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
//  File:       Huffman_Code.hh
//  Project:    XIL
//  Revision:   1.2
//  Last Mod:   10:23:38, 03/10/00
//
//  Description:
//
//    A Huffman_Code is a simple structure that contains a code and a
//    length which specifies the size in bits of the code. There is no
//    constructor which enables one to instantiate and initialize
//    an array of Huffman_Codes
//
//  Notes:
//
//    The Set and Get functions are not needed since the data members
//    are public but have left them in for consistency.
//
//------------------------------------------------------------------------
//    COPYRIGHT
//------------------------------------------------------------------------
#pragma ident   "@(#)Huffman_Code.hh	1.2\t00/03/10  "

#ifndef HUFFMAN_CODE_HH
#define HUFFMAN_CODE_HH


class Huffman_Code{
public:

    unsigned int length;
    unsigned int code;

    void Set_Length(unsigned int l) { length = l; }
    void Set_Code(unsigned int c)   { code = c;   }
    void Set(unsigned int l, unsigned int c)   { length = l; code = c;   }  

    unsigned int Get_Length() const { return length; }
    unsigned int Get_Code()   const { return code;   }

    void operator=( const Huffman_Code&);

    friend int     operator==( const Huffman_Code&, const Huffman_Code&);
    friend int     operator!=( const Huffman_Code&, const Huffman_Code&);  

};

#endif // HUFFMAN_CODE_HH
