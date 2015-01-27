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
//  File:       MacroBlockBlt.hh
//  Project:    XIL
//  Revision:   1.1
//  Last Mod:   14:38:17, 04/03/96
//
//  Description:
//
//    TODO: Enter some descriptive text here
//
//
//------------------------------------------------------------------------
//    COPYRIGHT
//------------------------------------------------------------------------
#pragma ident   "@(#)MacroBlockBlt.hh	1.1\t96/04/03  "

#ifndef MACROBLOCKBLT_H
#define MACROBLOCKBLT_H

#include <xil/xilGPI.hh>

void getblocks(Xil_unsigned8 *yp, Xil_unsigned8 *up, Xil_unsigned8 *vp, 
                short *blold[6], Xil_unsigned8 *blnew[6], int yyad, int uvad,
                int mvy, int mvx, int width, int cbp);
void putblocks(Xil_unsigned8 *yp, Xil_unsigned8 *up, Xil_unsigned8 *vp,
                Xil_unsigned8 *blold[6],
                int yyad, int uvad, int width);
void copyblocks(Xil_unsigned8 *ysrc, Xil_unsigned8 *usrc, Xil_unsigned8 *vsrc,
                Xil_unsigned8 *ydst, Xil_unsigned8 *udst, Xil_unsigned8 *vdst,
                int yyad, int uvad, int width);

#ifdef LATER
void copyblock(Xil_unsigned8 *o, Xil_unsigned8 *n, int w);
void setblock (Xil_unsigned8 *n, int w, int v);
void getputblocks(short *, short *, short *, short *, short *, short *, short *blold[6], int, int, int, int, int);
#endif // LATER

#endif // MACROBLOCKBLT_H
