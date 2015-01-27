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
//  File:	xili_memset.cc
//  Project:	XIL
//  Revision:	1.8
//  Last Mod:	10:16:32, 03/10/00
//
//  Description:
//	A C version of xil_memset which just calls the memset library
//  routine. 
//	
//	
//------------------------------------------------------------------------
//  COPYRIGHT
//------------------------------------------------------------------------
#pragma ident	"@(#)xili_memset.cc	1.8\t00/03/10  "

#include <string.h>
#include "XiliUtils.hh"

#ifdef _XIL_USE_SYSTEM_MEMCPY
#undef xili_memset
extern "C" {
    void*
    xili_memset(void* s1, int c, size_t n);
}
#endif

void*
xili_memset(void* s1, int c, size_t n) {
    return memset(s1,c,n);
}
