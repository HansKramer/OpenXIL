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
//  File:       IdctDecoder.hh
//  Project:    XIL
//  Revision:   1.2
//  Last Mod:   10:23:39, 03/10/00
//
//  Description:
//
//    IdctDecoder Class definition
//
//
//------------------------------------------------------------------------
//    COPYRIGHT
//------------------------------------------------------------------------
#pragma ident   "@(#)IdctDecoder.hh	1.2\t00/03/10  "

#ifndef IDCTDECODER_H
#define IDCTDECODER_H

#include "IdctParser.hh"

#define EOB_CODE        0xfffc
#define INVALID_DECODE_RETURN	EOB_CODE

typedef struct
{
    unsigned short	code;
    unsigned short	length;
    int		        value;
} Codes;

class IdctDecoder : public IdctParser {

public:
    int* create_decode_table(int *hptr, Codes *huffman, int size);
    void free_decode_table(int *decode_table);
    int  decode(int *ptr);

};

#endif // IDCTDECODER_H





