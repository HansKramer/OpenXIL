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
//  File:	Transpose.hh
//  Project:	XIL
//  Revision:	1.6
//  Last Mod:	10:22:20, 03/10/00
//
//  Description: prototypes for block transpose functions
//	
//	
//  MT-level:  <??????>
//	
//------------------------------------------------------------------------
//	COPYRIGHT
//------------------------------------------------------------------------
#pragma ident	"@(#)Transpose.hh	1.6\t00/03/10  "

#ifndef _TRANSPOSE_HH
#define _TRANSPOSE_HH

#include "XiliUtils.hh"


//
//
//

#define LOAD_REG(R)			\
	R = src_blk[0];			\
	R = (R << 8) | src_blk[1];	\
	src_blk += src_stride;		\
	R = (R << 8) | src_blk[0];	\
	R = (R << 8) | src_blk[1];	\
	src_blk += src_stride;

#define LOAD_REG_UNALIGNED(R)						\
	R = src_blk[0];							\
	R = (R << 8) | src_blk[1];					\
	R = (R << 8) | src_blk[2];					\
	src_blk += src_stride;						\
	R = (R & (0xffff00 >> offset)) | (src_blk[0] & (0xff >> offset));\
	R = (R << 8) | src_blk[1];					\
	R = (R << offset) | (src_blk[2] >> (8-offset));			\
	src_blk += src_stride;

#define STORE_REG(R)					\
	dst[0] = R >> 24;				\
	dst[1] = R >> 16;	dst += dst_stride;	\
	dst[0] = R >> 8;				\
	dst[1] = R;		dst += dst_stride;

#define STORE_REG_UNALIGNED(R)					\
	tmp1 = R >> offset;					\
	tmp2 = R << (8 - offset);				\
	dst[0] = (dst[0] & keep) | ((tmp1 >> 24) & change);	\
	dst[1] = tmp1 >> 16;					\
	dst[2] = (dst[2] & change) | ((tmp2 >> 16) & keep);	\
	dst += dst_stride;					\
	dst[0] = (dst[0] & keep) | ((tmp1 >> 8) & change);	\
	dst[1] = tmp1;						\
	dst[2] = (dst[2] & change) | (tmp2 & keep);		\
	dst += dst_stride;		dst += dst_stride;

//
// Forward declarations
//

static void blk_flip_x(Xil_unsigned8*, Xil_unsigned8*,
                       unsigned int, unsigned int, unsigned int);
static void blk_flip_y(Xil_unsigned8*, Xil_unsigned8*,
                       unsigned int, unsigned int, unsigned int);
static void blk_flip_d(Xil_unsigned8*, Xil_unsigned8*,
                       unsigned int, unsigned int, unsigned int);
static void blk_flip_a(Xil_unsigned8*, Xil_unsigned8*,
                       unsigned int, unsigned int, unsigned int);
static void blk_flip_90(Xil_unsigned8*, Xil_unsigned8*,
			 unsigned int, unsigned int, unsigned int);
static void blk_flip_180(Xil_unsigned8*, Xil_unsigned8*,
			 unsigned int, unsigned int, unsigned int);
static void blk_flip_270(Xil_unsigned8*, Xil_unsigned8*,
			 unsigned int, unsigned int, unsigned int);

static void transpose_bit_region(XilOp* op,
				 XilStorage&, XilStorage&, XilBox*, XilBox*,
				 XilRectList&, XilFlipType, unsigned int);

static void xil_flip_x_axis(Xil_unsigned8*, Xil_unsigned8*,
			    unsigned int, unsigned int,
			    unsigned int, unsigned int, int, int);

static void xil_flip_y_axis(Xil_unsigned8*, Xil_unsigned8*,
			    unsigned int, unsigned int,
			    unsigned int, unsigned int, int, int);

static void xil_flip_main_diagonal(Xil_unsigned8*, Xil_unsigned8*,
				   unsigned int, unsigned int,
				   unsigned int, unsigned int, int, int);

static void xil_flip_anti_diagonal(Xil_unsigned8*, Xil_unsigned8*,
				   unsigned int, unsigned int,
				   unsigned int, unsigned int, int, int);

static void xil_flip_90(Xil_unsigned8*, Xil_unsigned8*,
			unsigned int, unsigned int, 
			unsigned int, unsigned int, int, int);

static void xil_flip_180(Xil_unsigned8*, Xil_unsigned8*,
			 unsigned int, unsigned int,
			 unsigned int, unsigned int, int, int);

static void xil_flip_270(Xil_unsigned8*, Xil_unsigned8*,
			 unsigned int, unsigned int,
			 unsigned int, unsigned int, int, int);

#endif // _TRANSPOSE_HH
