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
//  File:	XilCisBufferManagerPrivate.hh
//  Project:	XIL
//  Revision:	1.8
//  Last Mod:	10:21:52, 03/10/00
//
//  Description:
//		
//	Private definitions for XilCisBufferManager Object
//	
//------------------------------------------------------------------------
//	COPYRIGHT
//------------------------------------------------------------------------


#ifdef _XIL_PRIVATE_INCLUDES

#include "XiliCisBufferLList.hh"

#endif // _XIL_PRIVATE_INCLUDES


#ifdef _XIL_PRIVATE_DATA

  // the linked list of buffers
  XiliCisBufferLList* buffer_list;

  // positions of important buffers in buffer list
  XiliCisBufferLListPositionType s_buffer_pos;
  XiliCisBufferLListPositionType r_buffer_pos;
  XiliCisBufferLListPositionType w_buffer_pos;

  // current buffer position for partial frame search
  XiliCisBufferLListPositionType pf_buffer_pos;

  // constructor success flag
  Xil_boolean isOKFlag;

  // For error reporting
  XilSystemState* system_state;

  // pointers to important buffers
  XilCisBuffer*  s_buffer;
  XilCisBuffer*  r_buffer;
  XilCisBuffer*  w_buffer;


  // max frame size
  int    max_frame_size;

  // minimum number of frames per buffer
  int    num_frames_per_buffer;

  // flag used to control write buffer locking
  Xil_boolean   w_buffer_locked;

  // ----- added to handle seekBackToFrameType failure for JPEG -------
  Xil_boolean seek_to_start_frame_flag;

  // ------------ required for partial frames --------------

  // counter used to indicate number of buffers
  // having possible partial frames
  int buffers_with_partial_frames;

  // count of the number of bytes to process
  int pf_bytes_to_process;

  // partial frame byte pointer
  Xil_unsigned8* pf_ptr;

  // last byte in current buffer
  Xil_unsigned8* pf_end_ptr;

  // current buffer for partial frame search
  XilCisBuffer*  pf_buffer;

  Xil_boolean pf_need_EOF;

  // pointer back to device compression in order to call findNextFrameBoundary
  XilDeviceCompression* device_compression;

  // pointer to the start of the next frame given by a device compression
  // through the routine foundNextFrameBoundary
  Xil_unsigned8* next_frame_ptr;

  // flag indicating if findNextFrameBoundary is in process
  Xil_boolean finding_next_frame_boundary;

  // ------- required for unknown number of frames -------
  int         write_frame_id;

  // ------- required for max / keep frames -------
  int         s_buffer_frame_offset;

  // ------- required for error recovery -------
  int         recovery_frame_id;

  // -------- required for distinction of read frame for decompress/getBits -- -
  int         next_decompress_frame;

    //
    //  TODO:  1/30/96 jlf  Put here temporarily to make XIL_ERROR calls work.
    //
    XilCis* cis;

  // -------- Private Routines ---------

  int     seekForward(int framecount);
  int     seekBackward(int framecount);

  void    updateWriteFrameId(int nframes);
  int     handlePartialFrame();
  void    setupPartialFramePtrs(Xil_boolean need_EOF = FALSE);
  int     isolateUnresolvedPartialFrame();

  void    initValues();         // common initialization/reset actions

  void   checkStatusMaybe(int& status); // check for partial frames



#endif // _XIL_PRIVATE_DATA
