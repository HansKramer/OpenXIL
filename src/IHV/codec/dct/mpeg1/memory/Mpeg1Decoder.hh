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
//  File:       Mpeg1Decoder.hh
//  Project:    XIL
//  Revision:   1.2
//  Last Mod:   10:22:59, 03/10/00
//
//  Description:
//
//    Header for the Mpeg1Decoder Class
//
//
//------------------------------------------------------------------------
//    COPYRIGHT
//------------------------------------------------------------------------
#pragma ident   "@(#)Mpeg1Decoder.hh	1.2\t00/03/10  "

#ifndef MPEG1DECODER_HH
#define MPEG1DECODER_HH

#include "IdctDecoder.hh"

const int EOBP            = 0xfffc;
const int STUFF           = 0xfffd;
const int ESCAPE          = 0xfffe;
const int INVALID         = 0xffff;

const int CODE_C          = 0x01;
const int CODE_F          = 0x02;
const int CODE_B          = 0x04;
const int CODE_M          = 0x07;
const int CODE_Q          = 0x08;
const int CODE_I          = 0x10;

class Mpeg1Decoder : public IdctDecoder {
public:
    //
    // Constructor/destructor
    //
    Mpeg1Decoder();
    ~Mpeg1Decoder();

    int  isok;

    int  allocOk()       {return isok; }

    int  getMBA();
    int  getMVC();
    int  getMBTI();
    int  getMBTP();
    int  getMBTB();
    int  getMVD(int last, int *modulo);
    int  checkforstart();
    void alignbits();
    int  gettostart();

protected:
    int* MBA_HuffTable;
    int* MBTI_HuffTable;
    int* MBTP_HuffTable;
    int* MBTB_HuffTable;
    int* DCL_HuffTable;
    int* DCC_HuffTable;
    int* ACF_HuffTable;
    int* ACR_HuffTable;
    int* MVD_HuffTable;
    int* MVC_HuffTable;

private:
    static Codes MBAtable[35];

    static Codes MBTItable[2];
    static Codes MBTPtable[7];
    static Codes MBTBtable[11];

    static Codes DCCtable[9];
    static Codes DCLtable[9];

    static Codes MVDtable[33];
    static Codes MVCtable[63];

    static Codes TCOEFF_F[223];
    static Codes TCOEFF_R[224];
};

#endif /* MPEG1DECODER_HH */
