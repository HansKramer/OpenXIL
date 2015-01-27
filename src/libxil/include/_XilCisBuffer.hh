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
//  File:       _XilCisBuffer.hh
//  Project:    XIL
//  Revision:   1.9
//  Last Mod:   10:21:39, 03/10/00
//
//  Description:
//
//    Definition of XilCisBuffer Object
//    The XilCisBufferManager class manages multiple
//    XilCisBuffers to implement compression/decompression
//    bufferring.
//
//    A XilCisBuffer object basically acts as a buffer for compressed 
//      data. An instance can be told to create a buffer of a certain
//      size or can be handed a buffer to which it will act upon.
//
//	A XilCisBuffer contains information about the:
//
//       o number of complete frames contained within its buffer 
//       o identifier associated with the first frame in the buffer 
//       o number of bytes within the buffer
//       o the size of the buffer
//
//      A XilCisBuffer maintains a:
//
//       o a pointer to the next available byte in the buffer (wptr)
//       o a pointer to the start of the frame being written into (wfptr)
//       o a pointer to the start of the frame being read out of (rfptr)
//
//           If this equals the wfptr then a complete frame does not
//           exist at the read pointer. The routine frameAtRfptr()
//           will return false under this condition. If these 
//           pointers are not equal, then this implies that the a 
//           complete frame may exist at the read pointer. Thus,
//           frameAtRfptr() will return true. The partial frame 
//           flag should still be tested because there may still
//           not be a complete frame at the buffer's end.
//
//       o a pointer (buffer_alloc) in which compress data is placed
//           (may be external)
//
//       o a pointer (buffer) to the start of the accessible portion
//          of buffer_alloc. (May be adjusted because of keep/max
//          frames).
//
//       o a control_buffer flag is also kept which if true, implies
//          that the XilCisBuffer instance has control over its buffer
//          and can destroy it if necessary. If the flag is not true
//          then an instance may not delete the storage associated 
//          with the buffer. A call back may be set up to free this
//          external storage.
//
//       o a list of XilFrameInfo objects that contain information about
//          each frame within this buffer (such as a pointer to the
//          starting byte of the frame within the buffer and number of
//          bytes in the frame). This list is built up by the compressor
//          each time it calls compressedFrame which in turn calls
//          incrNumCompressedFrames of the XilCisBuffer. The list can
//          also be built up by the decompressor when it calls the routine
//          decompressedFrame which calls incrNumDecompressedFrames. 
//          
//       o the position on the frame list that represents the frame
//          currently being read out of.
//
//       o a flag, partial_frames, which if true states that the buffer
//         may contain a partial frame at its end.
//
//   Member functions include standard get/set attribute routines and
//      methods for adding byte(s) and short(s) to the internal buffer.
//      A buffer of complete frames may also be 'copied' into the buffer.
//
//
//
//------------------------------------------------------------------------
//    COPYRIGHT
//------------------------------------------------------------------------
#pragma ident   "@(#)_XilCisBuffer.hh	1.9\t00/03/10  "


#ifndef XILCISBUFFER_H
#define XILCISBUFFER_H

#include "_XilDefines.h"

//
// Define avg #frames which will fit in a 
// max buffer size = max_frame_size
//
#define XIL_CIS_AVG_PER_MAX               8

//
// CIS defines for frame types
//
#define XIL_CIS_DEFAULT_FRAME_TYPE        0
#define XIL_CIS_ANY_FRAME_TYPE           -1
#define XIL_CIS_NO_BURN_TYPE             -2
#define XIL_CIS_PARTIAL_FRAME             0

#define XIL_CIS_UNKNOWN_FRAME_ID         -1
#define XIL_CIS_UNKNOWN_NUMBER_OF_FRAMES -1
#define XIL_CIS_DEFAULT_PUTBITS_NFRAMES   1000  // From movie stats


#ifdef _XIL_LIBXIL_PRIVATE
#define _XIL_PRIVATE_INCLUDES

#include "XilCisBufferPrivate.hh"

#undef _XIL_PRIVATE_INCLUDES
#endif // _XIL_LIBXIL_PRIVATE

class XilCisBuffer {
public:
  
  //
  // Constructors / destructor
  //
                 XilCisBuffer(unsigned int buf_size, int approx_nframes);

                 XilCisBuffer(unsigned int               nbytes,
                              int                        nframes,
                              Xil_unsigned8*             buf,
                              int                        frame_id,
                              XIL_FUNCPTR_DONE_WITH_DATA done_data,
                              int              approx_nframes = 0);


                 ~XilCisBuffer();

  //
  // Functions to query buffer contents
  //
  int            getNumFrames()        const;
  int            getStartFrameId()     const;
  int            getNumBytes()         const;
  unsigned int   getBufferSize()       const;
  int            getNumBytesInWFrame() const;
  int            getNumBytesInRFrame() const;
  Xil_unsigned8* getNumBytesToFrame(int  end_id,
                                    int* nbytes);

  int            frameAtRfptr() const;
  int            frameAfterRfptr(int         max_frame_size,
                                 Xil_boolean need_EOF = FALSE) const;
  int            numAvailBytes() const;

  //
  // Functions to add data to a buffer
  //
  void           addByte(int b );
  void           addShort(int s);
  void           addBytes(Xil_unsigned8* b,
                          unsigned int   nbytes );
  void           addShorts(short*       s,
                           unsigned int m_shorts );

  //
  // Functions to update buffers
  //
  int            removeStartFrame();
  int            updateLastFrame();

  //
  // Test for successful construction
  //
  Xil_boolean    isOK();


#ifdef _XIL_LIBXIL_PRIVATE
#define _XIL_PRIVATE_DATA

#include "XilCisBufferPrivate.hh"

#undef _XIL_PRIVATE_DATA
#endif // _XIL_LIBXIL_PRIVATE
  
};

#endif
