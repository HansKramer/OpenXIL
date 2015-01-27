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
//  File:       Mpeg1Decoder.cc
//  Project:    XIL
//  Revision:   1.4
//  Last Mod:   10:14:43, 03/10/00
//
//  Description:
//
//    Tables for Mpeg decoding
//
//
//------------------------------------------------------------------------
//    COPYRIGHT
//------------------------------------------------------------------------
#pragma ident   "@(#)Mpeg1Decoder.cc	1.4\t00/03/10  "

#include <stdio.h>
#include <math.h>
#include "xil/xilGPI.hh"
#include "Mpeg1Decoder.hh"

Mpeg1Decoder::Mpeg1Decoder()
{
    isok = 0;

    MBA_HuffTable = create_decode_table(0, MBAtable, 
                                   sizeof(MBAtable)/sizeof(Codes));

    MBTI_HuffTable = create_decode_table(0, MBTItable, 
                                   sizeof(MBTItable)/sizeof(Codes));

    MBTP_HuffTable = create_decode_table(0, MBTPtable, 
                                   sizeof(MBTPtable)/sizeof(Codes));

    MBTB_HuffTable = create_decode_table(0, MBTBtable, 
                                   sizeof(MBTBtable)/sizeof(Codes));

    DCL_HuffTable = create_decode_table(0, DCLtable, 
                                   sizeof(DCLtable)/sizeof(Codes));

    DCC_HuffTable = create_decode_table(0, DCCtable, 
                                   sizeof(DCCtable)/sizeof(Codes));

    ACF_HuffTable = create_decode_table(0, TCOEFF_F, 
                                   sizeof(TCOEFF_F)/sizeof(Codes));

    ACR_HuffTable = create_decode_table(0, TCOEFF_R, 
                                   sizeof(TCOEFF_R)/sizeof(Codes));

    MVD_HuffTable = create_decode_table(0, MVDtable, 
                                   sizeof(MVDtable)/sizeof(Codes));

    MVC_HuffTable = create_decode_table(0, MVCtable, 
                                   sizeof(MVCtable)/sizeof(Codes));


    isok = 1;
}

Mpeg1Decoder::~Mpeg1Decoder()
{
    // Free decode tables
    free_decode_table(MBA_HuffTable);
    free_decode_table(MBTI_HuffTable);
    free_decode_table(MBTP_HuffTable);
    free_decode_table(MBTB_HuffTable);
    free_decode_table(DCL_HuffTable);
    free_decode_table(DCC_HuffTable);
    free_decode_table(ACF_HuffTable);
    free_decode_table(ACR_HuffTable);
    free_decode_table(MVD_HuffTable);
    free_decode_table(MVC_HuffTable);

}


int 
Mpeg1Decoder::checkforstart()
{
    unsigned int tmp,foo;
    int next;

    if(savedBits) {
        return 0;
    }

    if(rdptr > (endOfBuffer-4)) {
        return 1;
    }

    if(nbits < 8) {
        tmp = rdptr[0];
        tmp = (tmp<<8) | rdptr[1];
        tmp = (tmp<<8) | rdptr[2];
        tmp = (tmp<<8) | rdptr[3];
        next = 4;
    } else if(nbits < 16) {
        tmp = savedBits & 0xff;
        tmp = (tmp<<8) | rdptr[0];
        tmp = (tmp<<8) | rdptr[1];
        tmp = (tmp<<8) | rdptr[2];
        next = 3;
    } else if(nbits < 24) {
        tmp = savedBits & 0xffff;
        tmp = (tmp<<8) | rdptr[0];
        tmp = (tmp<<8) | rdptr[1];
        next = 2;
    } else {
        tmp = savedBits & 0xffffff;
        tmp = (tmp<<8) | rdptr[0];
        next = 1;
    }

    if(tmp < 0x00000100) {
        while(tmp < 0x00000100) {
            if(rdptr > (endOfBuffer-next)) {
                return 1;
            }
            tmp = (tmp<<8) | rdptr[next];
            next++;
        }
    }

    foo = tmp & 0xffffff00;
    if(foo == 0x00000100) {
        nbits = 0;
        savedBits = 0;
        rdptr = rdptr + (next-4);
        return 1;
    } else {
        return 0;
    }
}

int 
Mpeg1Decoder::gettostart()
{
    int ret = 0;

    alignbits();

    if(rdptr <= (endOfBuffer-4)) {
        int tmp;
        int foo;
        while(1) {
            tmp = rdptr[0];
            tmp = (tmp<<8) | rdptr[1];
            tmp = (tmp<<8) | rdptr[2];
            tmp = (tmp<<8) | rdptr[3];
    
            foo = tmp & 0xffffff00;
            if(foo == 0x00000100) {
                ret = 1;
                break;
            } else {
                rdptr++;
                if(rdptr > (endOfBuffer-4)) {
                    ret = 0;
                    break;
                }
            }
        }
    }

    return ret;
}

void 
Mpeg1Decoder::alignbits()
{
    while(nbits >= 8) {
        rdptr--;
        nbits -= 8;
    }
    nbits = 0;
}

int 
Mpeg1Decoder::getMBA()
{
    int word;
    int incr;

    word = decode(MBA_HuffTable);

    if(word & 0x8000) {
        incr = 0;
        while(word & 0x8000) {
            if(word == ESCAPE) {
                incr += 33;
            }
            word = decode(MBA_HuffTable);
            if(word == INVALID_DECODE_RETURN) {
                return word;
            }
        }
        word += incr;
    }

    return word;
}

int 
Mpeg1Decoder::getMVC()
{
    return decode(MVC_HuffTable);
}

int 
Mpeg1Decoder::getMBTI()
{
    int code;

    int type = decode(MBTI_HuffTable);

    switch (type) {
      case 0:
        code = CODE_I;
        break;
      case 1:
        code = CODE_I | CODE_Q;
        break;
      default:
        return INVALID_DECODE_RETURN;
    }

    return (code<<16) | (type+1);
}


int 
Mpeg1Decoder::getMBTP()
{
    int code;

    int type = decode(MBTP_HuffTable);

    switch(type) {
      case 0:
        code = CODE_F | CODE_C;
        break;
      case 1:
        code = CODE_C;
        break;
      case 2:
        code = CODE_F;
        break;
      case 3:
        code = CODE_I;
        break;
      case 4:
        code = CODE_Q | CODE_F | CODE_C;
        break;
      case 5:
        code = CODE_Q | CODE_C;
        break;
      case 6:
        code = CODE_Q | CODE_I;
        break;
      default:
        return INVALID_DECODE_RETURN;
    }


    return (code<<16) | (type+1);
}


int 
Mpeg1Decoder::getMBTB()
{
    int code;

    int type = decode(MBTB_HuffTable);

    switch(type) {
      case 0:
        code = CODE_F | CODE_B;
        break;
      case 1:
        code = CODE_F | CODE_B | CODE_C;
        break;
      case 2:
        code = CODE_B;
        break;
      case 3:
        code = CODE_B | CODE_C;
        break;
      case 4:
        code = CODE_F;
        break;
      case 5:
        code = CODE_F | CODE_C;
        break;
      case 6:
        code = CODE_I;
        break;
      case 7:
        code = CODE_Q | CODE_F | CODE_B | CODE_C;
        break;
      case 8:
        code = CODE_Q | CODE_F | CODE_C;
        break;
      case 9:
        code = CODE_Q | CODE_B | CODE_C;
        break;
      case 10:
        code = CODE_Q | CODE_I;
        break;
      default:
        return INVALID_DECODE_RETURN;
    }

    return (code<<16) | (type+1);
}

int 
Mpeg1Decoder::getMVD(int last, int *modulo)
{
    int little = decode(MVD_HuffTable);
    if(little == INVALID_DECODE_RETURN) {
        return little;
    }
    little = modulo[little];

    if(little == 0) {
        return last;
    }

    int extra  = modulo[34];
    int tmax   = modulo[35];
    int t32    = modulo[36];
    int code   = modulo[33];
    int remain = 0;

    int wordb;
    GETBITS(extra,wordb);


    if(code != 0) {
        remain = code - wordb;
    }

    int big;
    if(little > 0) {
        little = little - remain;
        big    = little - t32;
    } else if(little < 0) {
        little = little + remain;
        big    = little + t32;
    }

    wordb = last + little;
    if(wordb < -tmax || wordb > tmax-1) {
        wordb = last + big;
    }

    return wordb;

    ErrorReturn:
    return INVALID_DECODE_RETURN;
}

