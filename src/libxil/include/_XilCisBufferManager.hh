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
//  File:       _XilCisBufferManager.hh
//  Project:    XIL
//  Revision:   1.8
//  Last Mod:   10:22:09, 03/10/00
//
//  Description:
//
//	Definition of XilCisBufferManager Object
//
//    A XilCisBufferManager manages XilCisBuffer Objects for a Cis - it
//      maintains a list of buffers in which compressed data is kept.
//      There are three important positions within this list of buffers:
//
//         s_ position represents the start of the list
//
//         r_ position represents the position in the list which data
//            will be read from next (upon certain operations)
//
//         w_ position represents the current write buffer to which
//            data is placed into either by the application or by
//            a compressor.
//
//      For each of the three positions above, there exist two types
//      of data members:
//
//         buffer:      pointer to the XilCisBuffer object for the position
//         buffer_pos:  actual position object of XilCisBuffer in buffer list
//
//    A XilCisBufferManager has a frame size and a number frames per buffer
//      value associated with it. Whenever the manager deems it is necessary
//      to create a new XilCisBuffer object with its own data storage (versus
//      a XilCisBuffer with external storage) it will create it such that 
//      the XilCisBuffer has a buffer the size of:
//
//                      max_frame_size*num_frames_per_buf.
//
//                       - - - - - - - - - - - - -
//
//    COMPRESSION:
//
//      Method 1:
//
//     A compressor calls nextBuffer each time it needs to compresses
//      a frame. The XilCisBufferManager will return a pointer to a 
//      XilCisBuffer to which a compressor can add bytes to (see interface 
//      for XilCisBuffer). If there is enough buffer space in the current
//      write buffer (at least max_frame_size), the manager will return a
//      pointer to the current write buffer. Otherwise, a new XilCisBuffer
//      will be created and the write buffer advances.
//
//     A compressor calls compressedFrame each time it has compressed a frame.
//      An optional parameter to this routine can associate a 'type' with
//      the frame just compressed. This type defaults to XIL_CIS_DEFAULT_FRAME_TYPE,
//      which has no meaning unless a codec desires it to.
//
//      Method 2:
//
//    Instead of getting a XilCisBuffer object to add data to, a compressor
//      can opt to make a call to nextBufferSpace which will return a
//      pointer to the first available byte within the XilCisBuffer that
//      normally would have been returned by nextBuffer. Thus, a device compression
//	can use this byte/Xil_unsigned8 pointer to add data to the byte stream.
//      The data space available for writing into will be at least
//      max_frame_size bytes large. It is the device compression's responsibility,
//      however, to ensure that no more than max_frame_size bytes are written.
//
//    The duo to compressedFrame for nextBufferSpace is doneBufferSpace.
//      This routine takes at least one argument: the number of bytes
//      written using the pointer retrieved by nextBufferSpace. It is
//      important that this be accurate since a pointer to the end of
//      what was just compressed will be determined by the sum of the
//      initial pointer (retrieved by nextBufferSpace) and this byte
//      count.
//
//    Until doneBufferSpace is called, the flag w_buffer_locked is will
//      set so that no one can attempt to retrieve a pointer to the
//      data space into which the user may be writing into.
//
//                       - - - - - - - - - - - - -
//
//    DECOMPRESSION:
//
//     A decompressor calls nextFrame() each time it wants to decompress
//      a frame. The XilCisBufferManager will return a pointer to the first
//      byte of the next frame to decompress. This next frame is determined
//      by the read buffer. Any partial frame handling is done within this
//      routine.  The default parameter, r_buffer_end, can be loaded with
//      the pointer to the last valid byte in the current frame's read buffer.
//
//     If the buffer manager can not determine that a full frame exists at
//      the read position, it will ask the device compressor to find the
//      next frame boundary (via findNextFrameBoundary). The set of routines
//      associated with this process are getNextByte, getNextBytes and
//      foundNextFrameBoundary. The get byte routines are used by the device
//      compressor to retrieve bytes from this cis in order for the device
//      compressor to locate the end of the current frame. The routine
//      getNextByte returns a pointer to a single byte while getNextBytes
//      returns a pointer to the next byte and modifies by reference a
//      parameter indicating the number of bytes available for examination
//      from the returned pointer on. Both routines return NULL if no more
//      bytes are available in the cis for examination. If the frame boundary
//      is not found - i.e. the available bytes run out before the boundary
//      is found, the device compressor should return XIL_FAILURE from
//      findNextFrameBoundary to nextFrame. The routine nextFrame will then
//      return NULL to its caller indicating that there are no enough data
//      left to decompress. If, however, the frame boundary is found, the
//      pointer via getNextByte(s) is passed through foundNextFrameBoundary.
//      This pointer should point to one byte beyond the last byte in the
//      processed frame. The routine foundNextFrameBoundary will adjust
//      buffers appropriately so that the possibly segmented frame lies
//      within a single allocated buffer space.
//
//     A decompressor should call decompressedFrame(byte *) every time 
//      it has decompressed a frame. It should also return via this routine,
//      the pointer that it received from nextFrame which should now point to
//      one byte beyond the last byte in the decompressed frame. A second
//      optional parameter (not shown above) is the type of frame just
//      decompressed, which defaults to XIL_CIS_DEFAULT_FRAME_TYPE.
//
//                       - - - - - - - - - - - - -
//
//    HAS DATA / GET & PUT BITS:
//
//     The XilCisBufferManager can be queried for the number of bytes 
//      contained in its buffers from the read position to the write
//      position. This call is hasData(). The number of bytes is inclusive
//      and thus may contain any sum of bytes in a partial frame in the
//      write buffer.
//
//     If a hasData() returns some number of bytes > 0, then a call
//      to getBitsPtr is valid. Supplied as arguments, the number of
//      bytes and the integer number of frames contained in the data
//      returned by getBitsPtr will be set. getBitsPtr then returns a
//      pointer to the data. Note that if a partial frame exist in the
//      write buffer, it may not be possible to retreive this data
//      although hasData returns a value > 0.
//
//    Since the implementation of hasData, a better test has been designed
//      for testing if a frame exist at the read position. The routine is
//      called hasFrame. It returns true if a complete frame exist at
//      the read position and false otherwise. Note that partial frame
//      handling may be required if the read buffer has been marked
//      as having partial frames.
//
//    The routines putBits and putBitsPtr can be used to put data into
//      a buffer. The main difference between the two is that putBits
//      results in a copy of the data and putBitsPtr does not. A call
//      to putBits will copy the data supplied into the current write
//      buffer if there is enough room else a new buffer of the exact
//      size will be created. A call to putBitsPtr always causes the
//      creation of a new XilCisBuffer. However, the buffer space for
//      this object will not be allocated. Instead, the objects pointer
//      to a buffer space will be set to the data pointer passed to
//      putBitsPtr. Thus, the object will use external storage.
//
//    The routine putBitsPtr also has an additional, optional parameter.
//      This parameter is a pointer to a function (the type is defined
//      as XIL_FUNCPTR_DONE_WITH_DATA in XilDefines.h) which will be
//      called when the XilCisBuffer object that maintains the external
//      data pointer is destroyed. This can occur because of a reset on
//      the cis, the destruction of buffers due to keep and max frames,
//      the collapsing of a partial frame into a single buffer which may
//      cause the removal of an uncessary buffer, or because the cis was
//      destroyed. Thus the call-back can be used to reclaim data space.
//      The default is NULL in which case no call back will be made.
//
//                       - - - - - - - - - - - - -
//  SEEK:
//
//    The seek routine can be used to seek to a specific frame within the
//      buffer manager. A seek is performed by attempting to get as close
//      to the desired frame as possible. This may result in some number of
//      frames that must be 'burned' to get to the desired frame.
//
//    A frame type can optionally be specified with the default being
//     XIL_CIS_ANY_FRAME_TYPE (which matches any frame type). The seek
//     routine always attempts to seek first to the desired frame (which
//     may result in some number of burn frames). Then, if no errors
//     occurred, the frame which is now the 'read frame' is checked for
//     its frame type. If this type doesn't match the desired frame type,
//     then a seek backwards to a frame of the desired type occurrs.
//
//     NOTE: There are two special cases in which a seek back to a frame
//       type will not occur. 1) if the current read frame id is 0 (Frame
//       zero is always considered a frame of ANY TYPE). 2) if read frame
//       did not change during the seek operation (The current read frame
//       before a seek is called is always considered a frame of ANY TYPE).
//
//     NOTE: When seek is called, the current read frame's frame type is
//       marked as ANY TYPE (following exception 2 in above note). Before
//       seek returns, the type of this frame is changed back to what it
//       was originally. Thus, on a seek to a type, if a seek takes us
//       forward from the current frame to a new frame that doesn't match
//       the desired type, then in the worse case, a seek back to the desired
//       type will bring us back to the original current frame.
//
//
//  KEEP / MAX FRAMES:
//
//    A XilCisBufferManager maintains a list of XilCisBuffers. The max number
//      of frames present in this list of buffers may optionally be set.
//      A limit may also be optionally placed on the number of frames kept 
//      between the read position and the start frame. The routine adjustStart
//      is used to adjust the start frame within the buffer lists. Since the
//      new desired start frame may not be able to be processed without
//      addition previous frame information, adjustStart routine can also
//      take as a parameter a frame type. Any frames from the desired frame
//      back to a frame of the given type will be kept within the cis although
//      they may not be accessed. They are only used to process the new start
//      frame (and frames that may follow the new start frame). The variable
//      s_buffer_frame_offset is the number of frames from the frame of the
//      required type to the desired start frame. Once the adjustment has been
//      made, any buffers prior to the new start buffer are destroyed. Any
//      frame data within the new start buffer but prior to the "type'ed" frame
//      becomes inaccessible.
//
//
//------------------------------------------------------------------------
//    COPYRIGHT
//------------------------------------------------------------------------
#pragma ident   "@(#)_XilCisBufferManager.hh	1.8\t00/03/10  "


#ifndef XILCISBUFFERMANAGER_H
#define XILCISBUFFERMANAGER_H

class XilDeviceCompression;

#include "_XilDefines.h"
#include "_XilCisBuffer.hh"

#ifdef _XIL_LIBXIL_PRIVATE
#define _XIL_PRIVATE_INCLUDES

#include "XilCisBufferManagerPrivate.hh"

#undef _XIL_PRIVATE_INCLUDES
#endif // _XIL_LIBXIL_PRIVATE

#define XIL_UNRESOLVED -1

//
// Note: XIL_CIS_PARTIAL_FRAME is defined in XilCisBuffer.h as 0
//       XIL_CIS_UNKNOWN_NUMBER_OF_FRAMES is defined in XilFrameInfo.h as -1
//

class XilCisBufferManager {
public:

    //
    // Constructor / Destructor
    //
                          XilCisBufferManager(XilCis* xcis,
                                              int     mfs,
                                              int     nfpb);

                          ~XilCisBufferManager();

    //
    // Test for construction success
    //
    Xil_boolean           isOK();

    int                   getNumFrames()           const;
    int                   getFrameSize()           const;
    int                   getNumFramesPerBuffer()  const;

    int                   getSFrameId();
    int                   getRFrameId();
    int                   getWFrameId()            const;
    int                   getNextDecompressId()    const;
    int                   getRFrameType();
    void*                 getRFrameUserPtr();

    int                   setFrameSize(int fs);
    int                   setRFrameUserPtr(void* uptr);

    void                  setNumFramesPerBuffer(int nfpb);

    void                  setXilDeviceCompression(XilDeviceCompression* dc);

    XilDeviceCompression* getXilDeviceCompression();

    int                   hasData();
    int                   numberOfFrames();
    Xil_boolean           hasFrame();
    int                   seek(int framenumber,
                               int type = XIL_CIS_ANY_FRAME_TYPE);
    void                  reset();



    //
    // Functions for compression
    //
    XilCisBuffer*         nextBuffer();
    int                   compressedFrame(int type = XIL_CIS_DEFAULT_FRAME_TYPE); 

    Xil_unsigned8*         nextBufferSpace();
    int                    doneBufferSpace(int nbytes,
                                           int type = XIL_CIS_DEFAULT_FRAME_TYPE);

    //
    // Added to handle seekBackToFrameType failure for JPEG
    //
    void                   setSeekToStartFrameFlag(Xil_boolean value);

    //
    // Added to allow MPEG to add end of sequence to existing frame
    //
    int                   addToLastFrame(Xil_unsigned8* data,
                                         int            nbytes);

    //
    // Functions for decompression
    //
    Xil_unsigned8*         nextFrame(Xil_unsigned8** r_buffer_end = NULL,
                                     Xil_boolean     need_EOF = FALSE);

    void                   decompressedFrame(Xil_unsigned8* bfptr,
                                             int            type = XIL_CIS_DEFAULT_FRAME_TYPE,
                                             void*          user_ptr = NULL,
                                             Xil_boolean    update_next = TRUE);

    //
    // Put and Get Bits
    //
    void*                  getBitsPtr(int* nbytes,
                                      int* nframes);
    void                   putBits(int   nbytes,
                                   int   nframes,
                                   void* data);
    void                   putBitsPtr(int   nbytes,
                                      int   nframes,
                                      void* data,
                                      XIL_FUNCPTR_DONE_WITH_DATA = NULL);

    //
    // Routines for Partial Frames
    // NOTE: these are also used in error handling
    //
    Xil_unsigned8*         getNextByte();
    Xil_unsigned8*         getNextBytes(int* nbytes);
    Xil_unsigned8*         ungetBytes(Xil_unsigned8* curr_ptr,
                                      int            nbytes);
    XilStatus              foundNextFrameBoundary(Xil_unsigned8* frame_ptr);

    //
    // Error handling Routines
    //
    void                   byteError(Xil_unsigned8* bptr);
    int                    nextSeek(int framenumber,
                                    int type = XIL_CIS_ANY_FRAME_TYPE);
    int                    prevSeek(int framenumber,
                                    int type = XIL_CIS_ANY_FRAME_TYPE);
    void                   nextKnownFrameBoundary(Xil_unsigned8* cptr,
                                                  Xil_unsigned8** fptr,
                                                  int* num_frames);
    void                   errorRecoveryDone(Xil_unsigned8* fptr,
                                             int            num_frames,
                                             Xil_boolean    fixed);
    void                   foundFrameDuringRecovery(Xil_unsigned8* fptr);

    //
    // Routines for Max/Keep Frames
    //
    int                    adjustStart(int framenumber,
                                       int type = XIL_CIS_ANY_FRAME_TYPE);

     //
     // Routines to allow deviceCompression getBits
     //
     Xil_unsigned8*        getRBuffer();
     Xil_unsigned8*        getNumBytesToFrame(int  end_id,
                                              int* nbytes);
     int                   moveEndStartOneBuffer();

    //
    // Seek back to a specific frame type
    // (e.g. Mpeg non-B frame)
    //
    int                    seekBackToFrameType(int type);

    //
    // Retrieve system state for error reporting
    //
    XilSystemState*        getSystemState();




#ifdef _XIL_LIBXIL_PRIVATE
#define _XIL_PRIVATE_DATA

#include "XilCisBufferManagerPrivate.hh"

#undef _XIL_PRIVATE_DATA
#endif // _XIL_LIBXIL_PRIVATE


};

#include "_XilDeviceCompression.hh"

#endif // XILCISBUFFERMANAGER_H

