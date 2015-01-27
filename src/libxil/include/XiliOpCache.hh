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
//  File:	XiliOpCache.hh
//  Project:	XIL
//  Revision:	1.5
//  Last Mod:	10:21:49, 03/10/00
//
//  Description:
//	Caches op numbers to avoid hash lookups.  It is used by the Ops
//      to hand to XilVerifyArgs which will set and lookup the values
//      using this class.
//
//  TODO:  The number of op numbers has to be variable and not
//         tied to XIL_DATATYPES because this number may change in the
//         future.  
//	
//  MT-level:  Safe
//	
//------------------------------------------------------------------------
//	COPYRIGHT
//------------------------------------------------------------------------
#pragma ident	"@(#)XiliOpCache.hh	1.5\t00/03/10  "

#ifndef _XILI_OP_CACHE_HH
#define _XILI_OP_CACHE_HH

#include "_XilMutex.hh"

class XiliOpCache {
public:
    int    lookup(XilDataType datatype);
    int    lookup(XilDataType src_datatype, XilDataType dst_datatype);

    int    set(XilDataType datatype, int op_num);
    int    set(XilDataType src_datatype, XilDataType dst_datatype, int op_num);
    
    XiliOpCache(unsigned int num_datatype_differences = 0);
    ~XiliOpCache();
    
private:
    XilMutex  opCacheMutex;
    int*      opCache;
};

//
//  TODO:  9/11/96 jlf  For backward compatibility with all of our op
//                      routines, I'll ifdef to XilOpCache.
//
#define XilOpCache XiliOpCache

#endif // _XILI_OP_CACHE_HH
