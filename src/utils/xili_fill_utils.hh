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

//------------------------------------------------------------------------
//
//  File:	xili_fill_utils.hh
//  Project:	XIL
//  Revision:	1.4
//  Last Mod:	10:23:58, 03/10/00
//
//  Description:
//	
//	
//	
//	
//	
//	
//	
//	
//  MT-level:  <??????>
//	
//------------------------------------------------------------------------
//	COPYRIGHT
//------------------------------------------------------------------------
#pragma ident	"@(#)xili_fill_utils.hh	1.4\t00/03/10  "

#ifndef _XILI_FILL_UTILS_HH
#define _XILI_FILL_UTILS_HH

// Used for SoftFill only
const float XILI_SOFTFILL_THRESHOLD = 0.01F;

const int XILI_FILL_MAX_STACK_FRAMES = 25;

//
// Stack control defines
//
#define BFILL_STACK_EMPTY(ctrl_ptr) \
  ( BFILL_STACK_BLOCK_EMPTY((ctrl_ptr)) && BFILL_STACK_NO_PREV_BLOCK((ctrl_ptr)))

#define BFILL_STACK_BLOCK_EMPTY(ctrl_ptr) \
  ( ((ctrl_ptr)->curr_idx <= 0) ? TRUE : FALSE )

#define BFILL_STACK_NO_PREV_BLOCK(ctrl_ptr) \
  ( ((ctrl_ptr)->curr_block->prev == NULL) ? TRUE : FALSE )

#define BFILL_STACK_FULL(ctrl_ptr) \
  ( BFILL_STACK_BLOCK_FULL((ctrl_ptr)) && BFILL_STACK_NO_NEXT_BLOCK((ctrl_ptr)) )

#define BFILL_STACK_BLOCK_FULL(ctrl_ptr) \
  ( ((ctrl_ptr)->curr_idx >= XILI_FILL_MAX_STACK_FRAMES) ? TRUE : FALSE )

#define BFILL_STACK_NO_NEXT_BLOCK(ctrl_ptr) \
  ( ((ctrl_ptr)->curr_block->next == NULL) ? TRUE : FALSE )

//
// Writemask defines
//
#define BFILL_MASK_ON(scanline,offset) \
  ( XIL_BMAP_TST(scanline, offset))

#define BFILL_MASK_SET(scanline,offset) \
  ( XIL_BMAP_SET(scanline,offset))

#endif	// _XILI_FILL_UTILS_HH
