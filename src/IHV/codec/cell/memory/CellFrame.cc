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
//  File:   CellFrame.cc
//  Project:    XIL
//  Revision:   1.3
//  Last Mod:   10:15:42, 03/10/00
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
#pragma ident   "@(#)CellFrame.cc	1.3\t00/03/10  "

#include "CellFrame.hh"
#include "XiliUtils.hh"

unsigned int bitson_table[256] = 
{
    0,1,1,2, 1,2,2,3, 1,2,2,3, 2,3,3,4,
    1,2,2,3, 2,3,3,4, 2,3,3,4, 3,4,4,5,
    1,2,2,3, 2,3,3,4, 2,3,3,4, 3,4,4,5,
    2,3,3,4, 3,4,4,5, 3,4,4,5, 4,5,5,6,

    1,2,2,3, 2,3,3,4, 2,3,3,4, 3,4,4,5,
    2,3,3,4, 3,4,4,5, 3,4,4,5, 4,5,5,6,
    2,3,3,4, 3,4,4,5, 3,4,4,5, 4,5,5,6,
    3,4,4,5, 4,5,5,6, 4,5,5,6, 5,6,6,7,

    1,2,2,3, 2,3,3,4, 2,3,3,4, 3,4,4,5,
    2,3,3,4, 3,4,4,5, 3,4,4,5, 4,5,5,6,
    2,3,3,4, 3,4,4,5, 3,4,4,5, 4,5,5,6,
    3,4,4,5, 4,5,5,6, 4,5,5,6, 5,6,6,7,

    2,3,3,4, 3,4,4,5, 3,4,4,5, 4,5,5,6,
    3,4,4,5, 4,5,5,6, 4,5,5,6, 5,6,6,7,
    3,4,4,5, 4,5,5,6, 4,5,5,6, 5,6,6,7,
    4,5,5,6, 5,6,6,7, 5,6,6,7, 6,7,7,8,
};

CellFrame::CellFrame(unsigned int init_width,
                     unsigned int init_height)
: frame_height(init_height), 
  frame_width(init_width)
{
    isok = FALSE;

    //
    //  Set them to NULL to the destructor doesn't segv
    //
    frame = NULL;
    flipped = NULL;
    userData = NULL;

    frame = new (Cell*[frame_height]);
    if (frame == NULL) {
      // out of memory error
      XIL_ERROR( NULL, XIL_ERROR_RESOURCE,"di-1",TRUE);
      return;
    }

    for (int i=0; i<frame_height; i++) {
       frame[i] = NULL;
    }

    for (i=0; i<frame_height; i++) {
       frame[i] = new Cell[frame_width];

       if (frame[i] == NULL) {   
         // out of memory error 
         XIL_ERROR( NULL, XIL_ERROR_RESOURCE,"di-1",TRUE);
         return;
       } 
    }

    flipped = new (Xil_boolean*[frame_height]);
    if (flipped == NULL) {   
      // out of memory error 
      XIL_ERROR( NULL, XIL_ERROR_RESOURCE,"di-1",TRUE);
      return;
    } 
    
    for (i=0; i<frame_height; i++) {
       flipped[i] = NULL;
    }
    
    for (i=0; i<frame_height; i++) {
       flipped[i] = new Xil_boolean[frame_width];
    
       if (flipped[i] == NULL) {    
         // out of memory error  
         XIL_ERROR( NULL, XIL_ERROR_RESOURCE,"di-1",TRUE);
         return;
       }    

       //
       //  Could be replaced with a memset.
       //
       // for(int j=0; j<frame_width; j++)
       //    flipped[i][j] = FALSE;
        
       xili_memset(flipped[i], 0, sizeof(Xil_boolean)*frame_width);
    }

    //
    //  Initialize cmapChanged and drop_flag
    //
    cmapChanged = FALSE;
    drop_flag = FALSE;
    isok = TRUE;
}

CellFrame::~CellFrame(void) 
{
    if (frame) {
      for (int i=0; i<frame_height; i++) {
         delete frame[i];
      }
      delete frame;
    }
    
    if (flipped) {
      for (int i=0; i<frame_height; i++) {
         delete flipped[i];
      }
      delete flipped;
    }

    if (userData) {
      delete userData->data;
      delete userData;
    }
}

void
CellFrame::clearFrame(void) 
{
    for (int i=0; i<frame_height; i++) {
       for (int j=0; i<frame_width; j++) {
          frame[i][j].clearCell();
       }
    }
}

void
CellFrame::setFrame(const Cell& set_val) 
{
    for (int i=0; i<frame_height; i++) {
      for (int j=0; i<frame_width; j++) {
        frame[i][j] = set_val;
      }
    }
}

CellFrame&
CellFrame::operator= (const CellFrame& rval) 
{
    int assert_fail=FALSE;
    if (assert_fail || (frame_height != rval.frame_height) ) {
      // internal error       
      XIL_ERROR( NULL, XIL_ERROR_SYSTEM, "di-95", TRUE);
      return *this;
    }

    if ( assert_fail || (frame_width  != rval.frame_width) ) {
      // internal error    
      XIL_ERROR( NULL, XIL_ERROR_SYSTEM, "di-95", TRUE); 
      return *this;
    }
    
    for (int i=0; i<frame_height; i++) {
      for (int j=0; j<frame_width; j++) {
         frame[i][j] = rval.frame[i][j];
      }
    }

    for (i=0; i<frame_height; i++) {
      for(int j=0; j<frame_width; j++) {
        flipped[i][j] = rval.flipped[i][j];
      }
    }

    cmap = rval.cmap;
    cmapChanged = rval.cmapChanged;
    
    return *this;
}

void  CellFrame::clearFlipped(void) 
{
    for(int i=0; i<frame_height; i++) {
      for(int j=0; j<frame_width; j++) {
        flipped[i][j] = FALSE;
      }
    }
}

CellFrame*
CellFrame::ok(void) 
{
  if (this == NULL) {
    return NULL;
  } else {
    if (isok == TRUE) {
      return this;
    } else {
      delete this;
      return NULL;
    }
  }
}    

//------------------------------------------------------------------------
//
//  Class:    ErrorInfoFrame
//
// Description:
//    ErrorInfo and ErrorInfoFrame class member functions.
//    
//    
//------------------------------------------------------------------------

ErrorInfo::ErrorInfo(const int size) 
{
    isok = FALSE;
    
    if (size != 0) {
      err_array = new SubErrorInfo[size];
      if (err_array == NULL) {
        // out of memory error
        XIL_ERROR( NULL, XIL_ERROR_RESOURCE,"di-1",TRUE);
        return;
      }
    } else {
      err_array = NULL;
    }
    
    numEntries = size;
    partition = ~0;  // everything is on
    
    isok = TRUE;
}

void
ErrorInfo::resize(const int size) 
{
    isok = FALSE;

    delete err_array;
    err_array = new SubErrorInfo[size];
    if (err_array == NULL) {
      // out of memory error
      XIL_ERROR( NULL, XIL_ERROR_RESOURCE,"di-1",TRUE);
      return;
    }

    numEntries = size;
    isok = TRUE;
}

ErrorInfo*
ErrorInfo::ok(void) 
{
    if (this == NULL) {
      return NULL;
    } else {
      if (isok == TRUE) {
        return this;
      } else {
        delete this;
        return NULL;
      }
    }
}    
    
ErrorInfoFrame::ErrorInfoFrame(unsigned int init_width,
                               unsigned int init_height,
                               unsigned int array_size)
: frame_height(init_height), 
  frame_width(init_width)
{
    isok = FALSE;
    
    frame = NULL;
    frame = new (ErrorInfo*[frame_height]);

    if (frame == NULL) {
      // out of memory error
      XIL_ERROR( NULL, XIL_ERROR_RESOURCE,"di-1",TRUE);
      return;
    }

    for (int i=0; i<frame_height; i++) {
       frame[i] = NULL;
    }

    for (i=0; i<frame_height; i++) {
       frame[i] = new ErrorInfo[frame_width];

       if (frame[i] == NULL) {   
         // out of memory error 
         XIL_ERROR( NULL, XIL_ERROR_RESOURCE,"di-1",TRUE);
         return;
       } 
    }

    //
    //  ARGH.  Why can't new'ing an array take initialization arguments?
    //  Oh well, now we'll really create the ErrorInfo.
    //
    for (int j=0; j<frame_height; j++) {
       for (int k=0; k<frame_width; k++) {
          frame[j][k].resize(array_size);
          if (frame[j][k].ok() == NULL) {
            // Couldn't create internal Cell compressor object 
            XIL_ERROR(NULL, XIL_ERROR_SYSTEM,"di-275",FALSE);
            return;
          }
        }
    }
    isok = TRUE;
}

ErrorInfoFrame::~ErrorInfoFrame(void) 
{
    for (int i=0; i<frame_height; i++) {
      delete[] frame[i];
    }
    delete[] frame;
}

ErrorInfoFrame*
ErrorInfoFrame::ok(void) 
{
    if (this == NULL) {
      return NULL;
    } else {
      if (isok == TRUE) {
        return this;
      } else {
        delete this;
        return NULL;
      }
    }
}    
