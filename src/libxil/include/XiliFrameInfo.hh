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
//  File:       XiliFrameInfo.hh
//  Project:    XIL
//  Revision:   1.5
//  Last Mod:   10:22:12, 03/10/00
//
//  Description:
//
//    Class definition for XiliFrameInfo object
//
//
//------------------------------------------------------------------------
//    COPYRIGHT
//------------------------------------------------------------------------
#pragma ident   "@(#)XiliFrameInfo.hh	1.5\t00/03/10  "


#ifndef XILIFRAMEINFO_H
#define XILIFRAMEINFO_H

#include "_XilDefines.h"

//
// Number of average size frames which will fit 
// in a max size buffer
//
#define XIL_CIS_AVG_PER_MAX 8

#define XIL_CIS_DEFAULT_FRAME_TYPE 0
#define XIL_CIS_ANY_FRAME_TYPE -1
#define XIL_CIS_NO_BURN_TYPE -2
#define XIL_CIS_UNKNOWN_FRAME_ID -1
#define XIL_CIS_UNKNOWN_NUMBER_OF_FRAMES -1
#define XIL_CIS_DEFAULT_PUTBITS_NFRAMES 1000

class XiliFrameInfo {
public:

    XiliFrameInfo(Xil_unsigned8* s     = NULL, 
                  int            nb    = 0, 
                  int            nf    = 0, 
                  int            fid   = -1, 
                  int            ftype = XIL_CIS_DEFAULT_FRAME_TYPE, 
                  void*          uptr  = NULL)
    {
      start      = s;
      num_bytes  = nb;
      frame_id   = fid;
      num_frames = nf;
      frame_type = ftype;
      user_ptr   = uptr;
    }

    ~XiliFrameInfo()
    {
       if (user_ptr) {
         delete user_ptr;   // delete data assoc'd with this frame
         user_ptr = NULL;
       }
    }
  
    Xil_unsigned8* getStart()           const { return start; }
    int            getNumBytes()        const { return num_bytes; }
    int            getFrameId()         const { return frame_id; }
    int            getNumFrames()       const { return num_frames; }
    int            getFrameType()       const { return frame_type; }    
    void*          getUserPtr()               { return user_ptr; }    
    
    void          setStart(Xil_unsigned8* s)  { start = s; }
    void          setNumBytes(int nb)         { num_bytes = nb; }
    void          setFrameId(int fid)         { frame_id = fid; }
    void          setNumFrames(int nf)        { num_frames = nf; }
    void          setFrameType(int ftype)     { frame_type = ftype; }
    void          setUserPtr(void* uptr)      { user_ptr = uptr; }

    int*           getFrameTypePtr()          { return &frame_type; }  

#if defined(GCC) || defined(_WINDOWS) || defined(HPUX)
public:
    //
    //  For placating explicit template instantiation.
    //
    int operator == (XiliFrameInfo&) {
        return TRUE;
    }
#endif

private:
    Xil_unsigned8* start;      // pointer to start of frame(s)
    int            num_bytes;  // number of bytes in frame(s)
    int            frame_id;   // frame if of first frame in segment
    int            num_frames; // number of frames in this object
    int            frame_type; // the type of the first frame in segment
    void*          user_ptr;   // device compression can load ptr to 
                               // data associated with this frame.  
                               // It must alloc space for the data.
};

#endif
