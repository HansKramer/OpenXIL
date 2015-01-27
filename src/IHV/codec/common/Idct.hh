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
//  File:       Idct.hh
//  Project:    XIL
//  Revision:   1.2
//  Last Mod:   10:23:37, 03/10/00
//
//  Description:
//
//      Utility macros for the Idct
//
//
//------------------------------------------------------------------------
//    COPYRIGHT
//------------------------------------------------------------------------
#pragma ident   "@(#)Idct.hh	1.2\t00/03/10  "

#ifndef IDCT_H
#define IDCT_H

//
// IDCT_FRAC_BITS is number of fractional bits maintained by IDCT
//
#define IDCT_FRAC_BITS	6

//
// Mask for IDCT fractional bits
//
#define IDCT_FRAC_MASK       ((1<<IDCT_FRAC_BITS) - 1)
 
//
// '1/2' expressed in IDCT fractional bits.
// Used for rounding
//
#define IDCT_HALF            (1<<(IDCT_FRAC_BITS-1))

#ifdef XIL_LITTLE_ENDIAN
#define ASSIGN_FIRST_BYTE(dest,value)	dest = value
#define ASSIGN_SECOND_BYTE(dest,value)	dest |= (value) << 8
#define ASSIGN_THIRD_BYTE(dest,value)	dest |= (value) << 16
#define ASSIGN_FOURTH_BYTE(dest,value)	dest |= (value) << 24

#else

#define ASSIGN_FIRST_BYTE(dest,value)	dest = value << 24
#define ASSIGN_SECOND_BYTE(dest,value)	dest |= (value) << 16
#define ASSIGN_THIRD_BYTE(dest,value)	dest |= (value) << 8
#define ASSIGN_FOURTH_BYTE(dest,value)	dest |= value

#endif

#endif // IDCT_H
