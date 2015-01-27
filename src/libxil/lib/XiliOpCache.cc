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
//  File:	XiliOpCache.cc
//  Project:	XIL
//  Revision:	1.10
//  Last Mod:	10:09:07, 03/10/00
//
//  Description:
//	
//	Implementation of the XiliOpCache class.
//	
//	
//  MT-level:  SAFE
//	
//------------------------------------------------------------------------
//	COPYRIGHT
//------------------------------------------------------------------------
#pragma ident	"@(#)XiliOpCache.cc	1.10\t00/03/10  "

#include "_XilDefines.h"
#include "XiliOpCache.hh"

int
XiliOpCache::lookup(XilDataType datatype)
{
    int tmp = -1;
    
    opCacheMutex.lock();
    tmp = opCache[datatype];
    opCacheMutex.unlock();
        
    return tmp;
}

int
XiliOpCache::set(XilDataType datatype, int op_num)
{
    opCacheMutex.lock();
    opCache[datatype] = op_num;
    opCacheMutex.unlock();

    return op_num;
}

int
XiliOpCache::lookup(XilDataType src_datatype, XilDataType dst_datatype)
{
    int tmp = -1;
    
    opCacheMutex.lock();
    tmp = opCache[(src_datatype*XIL_DATATYPES)+dst_datatype];
    opCacheMutex.unlock();
        
    return tmp;
}

int
XiliOpCache::set(XilDataType src_datatype, XilDataType dst_datatype, int op_num)
{
    opCacheMutex.lock();
    opCache[(src_datatype*XIL_DATATYPES)+dst_datatype] = op_num;
    opCacheMutex.unlock();

    return op_num;
}

//
//  TODO:  Still potential race condition that a member function is used
//           before the constructor completes?
//
XiliOpCache::XiliOpCache(unsigned int num_datatype_differences)
{
    opCacheMutex.lock();
    int size = XIL_DATATYPES;
    if(num_datatype_differences) {
        for(unsigned int i=0; i<num_datatype_differences; i++) {
            size *= XIL_DATATYPES;
        }
    }

    opCache = new int[size];
    
    for(int i=0; i<size; i++) {
        opCache[i] = -1;
    }
    opCacheMutex.unlock();
}

XiliOpCache::~XiliOpCache()
{
    delete [] opCache;
}
