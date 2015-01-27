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
//  File:       JpegLList.hh
//  Project:    XIL
//  Revision:   1.3
//  Last Mod:   10:22:51, 03/10/00
//
//  Description:
//
//    Simple Generic Linked List.
//    Only handles append and retrieve
//
//
//------------------------------------------------------------------------
//    COPYRIGHT
//------------------------------------------------------------------------
#pragma ident   "@(#)JpegLList.hh	1.3\t00/03/10  "

#ifndef _JPEG_LLIST_HH
#define _JPEG_LLIST_HH

#include <stdlib.h>

class JpegNode {
public:
    void*     dataPtr;
    JpegNode* next;
};

class JpegLList {
public:
    JpegLList();
    ~JpegLList();

    JpegNode*    append(void* element);
    void         emptyList();
    JpegNode*    head();
    JpegNode*    tail();
    unsigned int length();

private:
    JpegNode*    list_head;
    JpegNode*    list_tail;
    unsigned int list_length;
};

#endif // _JPEG_LLIST_HH
