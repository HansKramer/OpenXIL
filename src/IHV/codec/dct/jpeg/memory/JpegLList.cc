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
//  File:       JpegLList.cc
//  Project:    XIL
//  Revision:   1.4
//  Last Mod:   10:14:26, 03/10/00
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
#pragma ident   "@(#)JpegLList.cc	1.4\t00/03/10  "


#include "xil/xilGPI.hh"
#include "JpegLList.hh"


JpegLList::JpegLList()
{
    list_head   = NULL;
    list_tail   = NULL;
    list_length = 0;
}

JpegLList::~JpegLList()
{
    emptyList();
}

JpegNode*
JpegLList::append(void* element) {

    JpegNode* node = new JpegNode;
    if(node == NULL) {
        return NULL;
    }

    node->dataPtr = element;

    if(list_length == 0) {
        list_head = node;
        list_tail = node;
        list_head->next = NULL;
    } else {
        node->next = NULL;
        list_tail->next = node;
        list_tail       = node;
    }

    list_length++;

    return list_tail;
}

void
JpegLList::emptyList()
{
    for(JpegNode* node=list_head; node!= NULL; node = node->next) {
        delete node->dataPtr;
        delete node;
    }
    list_head = list_tail = NULL;
    list_length = 0;
}


JpegNode*
JpegLList::head() {
    return list_head;
}

JpegNode*
JpegLList::tail() {
    return list_tail;
}

unsigned int
JpegLList::length() {
    return list_length;
}

