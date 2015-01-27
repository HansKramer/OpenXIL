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
//  File:       IdctFillEntry.cc
//  Project:    XIL
//  Revision:   1.2
//  Last Mod:   10:16:14, 03/10/00
//
//  Description:
//
//    TODO: Enter some descriptive text here
//
//
//------------------------------------------------------------------------
//    COPYRIGHT
//------------------------------------------------------------------------
#pragma ident   "@(#)IdctFillEntry.cc	1.2\t00/03/10  "

#include "IdctSplatter.hh"

#define CSCALE        ((float) 65536.0)
#define COS_11        ((float) 0.9619397663)
#define COS_12        ((float) 0.9061274464)
#define COS_13        ((float) 0.8154931568)
#define COS_14        ((float) 0.6935199227)
#define COS_15        ((float) 0.5448951068)
#define COS_16        ((float) 0.3753302775)
#define COS_17        ((float) 0.1913417162)
#define COS_22        ((float) 0.8535533906)
#define COS_23        ((float) 0.7681777567)
#define COS_24        ((float) 0.6532814824)
#define COS_25        ((float) 0.5132799672)
#define COS_26        ((float) 0.3535533906)
#define COS_27        ((float) 0.1802399555)
#define COS_33        ((float) 0.6913417162)
#define COS_34        ((float) 0.5879378012)
#define COS_35        ((float) 0.4619397663)
#define COS_36        ((float) 0.3181896451)
#define COS_37        ((float) 0.1622116744)
#define COS_44        ((float) 0.5000000000)
#define COS_45        ((float) 0.3928474792)
#define COS_46        ((float) 0.2705980501)
#define COS_47        ((float) 0.1379496896)
#define COS_55        ((float) 0.3086582838)
#define COS_56        ((float) 0.2126075237)
#define COS_57        ((float) 0.1083863757)
#define COS_66        ((float) 0.1464466094)
#define COS_67        ((float) 0.0746578341)
#define COS_77        ((float) 0.0380602337)


void fillentry(int *cptr, int level, int index)
{
    register float *Cosine;
    register float cscale;
    static float CosTable[ ] = {
        COS_44,
        COS_24, COS_46,
        COS_14, COS_34, COS_45, COS_47,
        COS_12, COS_23, COS_25, COS_27, COS_16, COS_36, COS_56, COS_67,
        COS_22, COS_26, COS_66,
        COS_11, COS_13, COS_15, COS_17, COS_33,
        COS_35, COS_37, COS_55, COS_57, COS_77
    };

    cscale = CSCALE;
    Cosine = CosTable;
    switch (index) {
      case 0:
        cptr[0] = (int) (Cosine[0] * level * cscale);
        break;
      case 1:
        cptr[0] = (int) (Cosine[1] * level * cscale);
        cptr[1] = (int) (Cosine[2] * level * cscale);
        break;
      case 2:
        cptr[0] = (int) (Cosine[3] * level * cscale);
        cptr[1] = (int) (Cosine[4] * level * cscale);
        cptr[2] = (int) (Cosine[5] * level * cscale);
        cptr[3] = (int) (Cosine[6] * level * cscale);
        break;
      case 3:
        cptr[0] = (int) (Cosine[7] * level * cscale);
        cptr[1] = (int) (Cosine[8] * level * cscale);
        cptr[2] = (int) (Cosine[9] * level * cscale);
        cptr[3] = (int) (Cosine[10] * level * cscale);
        cptr[4] = (int) (Cosine[11] * level * cscale);
        cptr[5] = (int) (Cosine[12] * level * cscale);
        cptr[6] = (int) (Cosine[13] * level * cscale);
        cptr[7] = (int) (Cosine[14] * level * cscale);
        break;
      case 4:
        cptr[0] = (int) (Cosine[15] * level * cscale);
        cptr[1] = (int) (Cosine[16] * level * cscale);
        cptr[2] = (int) (Cosine[17] * level * cscale);
        break;
      case 5:
        cptr[0] = (int) (Cosine[18] * level * cscale);
        cptr[1] = (int) (Cosine[19] * level * cscale);
        cptr[2] = (int) (Cosine[20] * level * cscale);
        cptr[3] = (int) (Cosine[21] * level * cscale);
        cptr[4] = (int) (Cosine[22] * level * cscale);
        cptr[5] = (int) (Cosine[23] * level * cscale);
        cptr[6] = (int) (Cosine[24] * level * cscale);
        cptr[7] = (int) (Cosine[25] * level * cscale);
        cptr[8] = (int) (Cosine[26] * level * cscale);
        cptr[9] = (int) (Cosine[27] * level * cscale);
        break;
    }
}
