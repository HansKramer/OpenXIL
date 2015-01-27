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
//  File:       IdctSplatter.hh
//  Project:    XIL
//  Revision:   1.4
//  Last Mod:   10:23:44, 03/10/00
//
//  Description:
//
//    TODO: Enter some descriptive text here
//
//
//------------------------------------------------------------------------
//    COPYRIGHT
//------------------------------------------------------------------------
#pragma ident   "@(#)IdctSplatter.hh	1.4\t00/03/10  "

#ifndef IDCT_SPLATTER_H
#define IDCT_SPLATTER_H

#define CACHE_ENTRIES        4096

typedef struct { 
    int key;
    int entry;
} CacheEntry;


// This may be either Sparc assembly code or C code
#ifdef _WINDOWS
extern void idct(int *result, int *coefflist);
#else
extern "C" {
    void idct(int *result, int *coefflist);
}
#endif

class IdctSplatter  {
public:
    IdctSplatter();
    ~IdctSplatter();

protected:
    CacheEntry* cache;
    int*        saveptr;     // splat kernel cache
    int*        cosine;      // pointers into splat kernel cache
    int         splatterOk;  // constructor success/failure

};

#endif // IDCT_SPLATTER_H
