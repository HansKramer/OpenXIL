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
//  File:   CellBFrame.cc
//  Project:    XIL
//  Revision:   1.2
//  Last Mod:   10:15:28, 03/10/00
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
//  COPYRIGHT
//------------------------------------------------------------------------
#pragma ident   "@(#)CellBFrame.cc	1.2\t00/03/10  "

#include "xil/xilGPI.hh"

#include "CellBFrame.hh"

CellBFrame::CellBFrame(
    unsigned short init_width,
    unsigned short init_height)
: frame_height(init_height), frame_width(init_width)
{
    isOKFlag = FALSE;

    //
    //  Set them to NULL to the destructor doesn't segv
    //
    frame = NULL;

    frame = new (CellB*[frame_height]);
    if (frame == NULL) {
      // out of memory error
      XIL_ERROR(NULL, XIL_ERROR_RESOURCE, "di-1", TRUE);
      return;
    }

    for(int i=0; i<frame_height; i++) {
        frame[i] = NULL;
    }

    for(i=0; i<frame_height; i++) {
      frame[i] = new CellB[frame_width];

      if (frame[i] == NULL) {   
        // out of memory error 
        XIL_ERROR( NULL, XIL_ERROR_RESOURCE, "di-1", TRUE);
        return;
      } 
    }

    isOKFlag = TRUE;
}

CellBFrame::~CellBFrame(void) {
    if(frame) {
        for(int i=0; i<frame_height; i++) {
            delete frame[i];
        }
        delete frame;
    }
}

CellBFrame&
CellBFrame::operator= (const CellBFrame& rval) {

    int assert_fail=FALSE;

    if (assert_fail || (frame_height != rval.frame_height) ) {
      // internal error       
      XIL_ERROR( NULL, XIL_ERROR_SYSTEM, "di-95", TRUE);
      return *this;
    }

    if (assert_fail || (frame_width  != rval.frame_width) ) {
      // internal error    
      XIL_ERROR( NULL, XIL_ERROR_SYSTEM, "di-95", TRUE); 
      return *this;
    }
    
    for(int i=0; i<frame_height; i++) {
      for(int j=0; j<frame_width; j++) {
        frame[i][j] = rval.frame[i][j];
      }
    }

    return *this;
}

CellBFrame*
CellBFrame::ok(void) {
    if(this == NULL) {
        return NULL;
    } else {
        if(isOKFlag == TRUE) {
            return this;
        } else {
            delete this;
            return NULL;
        }
    }
}    

