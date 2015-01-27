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
//  File:	XilCisBufferPrivate.hh
//  Project:	XIL
//  Revision:	1.10
//  Last Mod:	10:22:01, 03/10/00
//
//  Description:
//		
//	Definition of Private Portion of XilCisBuffer Object
//	
//------------------------------------------------------------------------
//  COPYRIGHT
//------------------------------------------------------------------------

#ifdef _XIL_PRIVATE_INCLUDES

#include "XiliFrameInfoAList.hh"

#endif // _XIL_PRIVATE_INCLUDES



#ifdef _XIL_PRIVATE_DATA

//
// Make these functions public to the Core only
// So that the XilCisBufferManager can access them.
// Otherwise this class will have to be a friend.
//
public:

    //
    // Supply access to the system state for error reporting
    //
    void setSystemState(XilSystemState* state) {system_state = state;}
    XilSystemState* getSystemState() {return system_state;}

  //------------------------ Private Attributes Access --------------------

  void     setNumFrames(int nf)              { num_frames    = nf; }
  void     setStartFrameId(int sfid)         { start_frame_id = sfid; }
  void     setRfptr(Xil_unsigned8* ptr)      { rfptr        = ptr; }
  void     setWptr(Xil_unsigned8* ptr)       { wptr         = ptr; }
  void     setPartialFrame(int pf_flag)      { partial_frame = pf_flag; }

  Xil_unsigned8*    getBuffer()          const  { return buffer; }
  Xil_unsigned8*    getWptr()            const  { return wptr; }
  Xil_unsigned8*    getRfptr()           const  { return rfptr; }
  Xil_unsigned8*    getWfptr()           const  { return wfptr; }
  Xil_unsigned8*    getBufferAlloc()     const  { return buffer_alloc; }
  int               hasPartialFrame()    const  { return partial_frame; } 

  //------------------------ Private Member Functions ---------------------

  int      incrNumCompressedFrames(int frame_id = XIL_CIS_UNKNOWN_FRAME_ID,
                                   int nframes  = 1,
                                   int type     = 0);

  void     incrNumDecompressedFrames(Xil_unsigned8* ptr,
                                     int            type     = 0,
                                     void*          user_ptr = NULL);


  void     copyFrames(unsigned       nbytes,
                      int            nframes,
                      Xil_unsigned8* data,
                      int            frame_id = XIL_CIS_UNKNOWN_FRAME_ID);

  void     reRead();    // reset ptrs to beginning of buffer
  void     unRead();    // reset ptrs to end of buffer


  //----------- Partial Frames & Max_Frames/ Keep_Frames --------------------
  int      adjustStart(Xil_unsigned8* ptr, int new_start_id);
  int      adjustEnd(Xil_unsigned8* ptr);
  void     adjustStartFrameId(int sfid);
  int      adjustStartToRFrame();

  //----------- Seek  --------------------
  int      skipForward(int framecount);
  int      seekTo(int desired_frameid);
  int      seekForwardTo(int desired_frameid);
  int      seekBackwardTo(int desired_frameid);
  int      seekBackwardToFrameType(int type);

  //----------- Error Recovery  --------------------
  int      nextPossibleFrame(int framenumber, int type);
  int      prevPossibleFrame(int framenumber, int type);
  int      recoveryFrame(Xil_unsigned8* ptr, int recover_frame_id);
  int      frameBoundaryAfter(Xil_unsigned8* cptr, Xil_unsigned8** fptr,
                              int* num_frames_to_boundary);
  void     foundFrameDuringRecovery(Xil_unsigned8* fptr, 
                                    int recovery_frame_id);
  int      getRFrameId();
  int      getRFrameType();
  int*     getRFrameTypePtr();
  void*    getRFrameUserPtr();

  Xil_unsigned8*    getAvailData(int* nb,int* nf);

  //----------- Access to user data ptr  --------------------
  int      setRFrameUserPtr(void* uptr);

#if defined(GCC) || defined(_WINDOWS) || defined(HPUX)
    //
    //  For placating explicit template instantiation.
    //
    int operator == (XilCisBuffer&) {
        return TRUE;
    }
    XilCisBuffer() { }
#endif

//
// Private data members
//
private:

  Xil_boolean   isOKFlag;              // constructor creation flag
  int           num_frames;             // num of completed frames
  int           start_frame_id;        // identifier of first frame
  int           numBytesPrevFrames; // number of bytes in completed frames

  unsigned      buffer_size;           // buffer size

  Xil_unsigned8*    wptr;         // ptr to next writable byte in buffer
  Xil_unsigned8*    wfptr;        // ptr to start of frame being written into
  Xil_unsigned8*    rfptr;        // ptr to start of frame being read out of
  Xil_unsigned8*    buffer;       // ptr to start of buffer
  Xil_unsigned8*    buffer_alloc; // ptr to start of the allocated buffer

  Xil_boolean       control_buffer;
  Xil_boolean       partial_frame;    // partial frames flag

  XIL_FUNCPTR_DONE_WITH_DATA    done_with_data; // call back function pointer

  XilSystemState*   system_state;

  XiliFrameInfoAList*            frame_list; // list of frame info objects
  XiliFrameInfoAListPositionType rfpos;      // curr pos of read frame info


#endif // _XIL_PRIVATE_DATA
