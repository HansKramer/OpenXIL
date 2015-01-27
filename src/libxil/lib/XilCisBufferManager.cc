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
//  File:       XilCisBufferManager.cc
//  Project:    XIL
//  Revision:   1.17
//  Last Mod:   10:09:05, 03/10/00
//
//  Description:
//
//	Implementation of XilCisBufferManager Object
//
//
//------------------------------------------------------------------------
//    COPYRIGHT
//------------------------------------------------------------------------
#pragma ident   "@(#)XilCisBufferManager.cc	1.17\t00/03/10  "

#include "_XilDefines.h"
#include "_XilCis.hh"
#include "_XilCisBufferManager.hh"
#include "_XilSystemState.hh"

// this compression type has no end of frame marker 
// so last frame is unresolved...if the user told us
// the buffer contained partial frames, then        
// status = XIL_FAILURE.  However, if there are no partial frames,
// then the last frame boundary = end of buffer, status = SUCCESS!

#define CHECK_STATUS_MAYBE(STATUS)                                           \
   if (STATUS == XIL_UNRESOLVED)                                             \
       if (r_buffer->hasPartialFrame() == TRUE)                              \
         STATUS = XIL_FAILURE;                                               \
       else {                                                                \
         next_frame_ptr = r_buffer->getWptr();                               \
         STATUS = foundNextFrameBoundary(next_frame_ptr);                    \
       }                                                                     \


//------------------------------------------------------------------------
//
//  Function:	XilCisBufferManager::XilCisBufferManager(int mfs,int nfpb)
//  Created:	92/04/20
//
//  Description:
//	
//    Constructor for XilCisBufferManager object. The initValues() routine
//    is called to initialize start, read, and write pointers. The maximum frame 
//    size and number of frames per buffer are set to input parameters.  The
//    New buffer linked list is instantiated. 
//	
//  Parameters:
//	
//	int nfpb:   number of frames per buffer
//      int mfs:    maximum number of frames
//
//  Notes:
//      Check successful creation of this object using the isOK() member function
//
//------------------------------------------------------------------------

XilCisBufferManager::XilCisBufferManager(XilCis* xcis, int mfs, int nfpb)
{
  isOKFlag = FALSE;

  cis = xcis;

  

  initValues();
  seek_to_start_frame_flag = TRUE;
  max_frame_size = mfs;
  num_frames_per_buffer = nfpb;
  buffer_list = NULL;

  buffer_list = new XiliCisBufferLList;

  // check memory alloc for object;
  if (buffer_list == NULL) {
    XIL_ERROR( NULL, XIL_ERROR_RESOURCE,"di-1",TRUE);
    return;
  }
  // check object creation
  buffer_list = buffer_list->ok();
  if (buffer_list == NULL) {
    // di-277:  Could not create Object
    XIL_ERROR(NULL, XIL_ERROR_RESOURCE, "di-277", FALSE);
    return;
  }

  isOKFlag = TRUE;
}

Xil_boolean
XilCisBufferManager::isOK()
{
    _XIL_ISOK_TEST();
}


//------------------------------------------------------------------------
//
//  Function:	XilCisBufferManager::~XilCisBufferManager()
//  Created:	92/04/20
//
//  Description:
//
//     Destructor for XilCisBufferManager object.
//
//------------------------------------------------------------------------

XilCisBufferManager::~XilCisBufferManager()
{
  // call upon the deletion of all XilCisBuffers on the buffer list
  if (buffer_list)
    buffer_list->deletePtrElements();
  
  // now delete the list
  delete buffer_list;
  buffer_list = NULL;
}

//------------------------------------------------------------------------
//
//  Function:	XilCisBufferManager::initValues()
//  Created:	92/10/22
//
//  Description:
//
//     Shared initialization for constructor and reset routines.
//     Set all start, read and write pointers to NULL. Clear the w_buffer_locked
//     flag and the buffers_with_partial_frames count.  
//
//------------------------------------------------------------------------
void 
XilCisBufferManager::initValues()
{
  s_buffer = NULL;
  r_buffer = NULL;
  w_buffer = NULL;
  
  s_buffer_pos = NULL;
  r_buffer_pos = NULL;
  w_buffer_pos = NULL;
  
  w_buffer_locked = FALSE;

  pf_ptr = NULL;
  pf_end_ptr = NULL;
  pf_buffer = NULL;  
  pf_buffer_pos = NULL;
  pf_bytes_to_process = 0;
  finding_next_frame_boundary = FALSE;
  
  write_frame_id = 0;
  s_buffer_frame_offset  = 0;
  next_decompress_frame  = 0;

  recovery_frame_id = -1;  
  buffers_with_partial_frames = 0;
  
  return;
}

//------------------------------------------------------------------------
//
//  Function:	XilCisBufferManager::setupPartialFramePtrs(Xil_boolean need_EOF)
//  Created:	92/10/22
//
//  Description:
//
//     Setup for partial frame pointers shared by several routines.
//
//------------------------------------------------------------------------
void 
XilCisBufferManager::setupPartialFramePtrs(Xil_boolean need_EOF)
{
  pf_ptr = r_buffer->getRfptr();
  pf_end_ptr = r_buffer->getWptr();
  pf_buffer = r_buffer;          
  pf_buffer_pos = r_buffer_pos;
  pf_bytes_to_process = 0;
  pf_need_EOF = need_EOF;
}

//------------------------------------------------------------------------
//
//  Function:	XilCisBuffer* XilCisBufferManager::nextBuffer()
//  Created:	92/04/20
//
//  Description:
//
//    This routine is meant to be called by a compressor. It
//      returns a pointer to a XilCisBuffer to which the compressor
//      can add bytes to.
//
//    If there is at least max_frame_size of space left in the current
//     write buffer, then the write buffer will be returned. Else,
//     a new buffer of size max_frame_size*num_frames_per_buffer will
//     be created.
//
//    If this is the first time this routine is called (all buffer
//     pointers will be NULL), then the read, start and write
//     pointers should all be set.
//
//  Returns:
//	
//	XilCisBuffer* :   pointer to a XilCisBuffer to which bytes
//                      may be added to.
//
//------------------------------------------------------------------------

XilCisBuffer* 
XilCisBufferManager::nextBuffer()
{
  
  
  if (w_buffer_locked){

    // CIS write buffer locked,
    // Internal error: probably caused by an error in XilDeviceCompression.

    XIL_OBJ_ERROR( cis->getSystemState(), XIL_ERROR_OTHER,"di-95",FALSE, cis);
    return NULL;
  }
  
  if (w_buffer) {
    
    // if there is not enough room in the current buffer
    // then we need to create a new buffer
    
    if (w_buffer->getBufferSize()-w_buffer->getNumBytes() < max_frame_size){
      
      // we need to create a new buffer, setup:
      //   buf_size = max_frame_size*num_frames_per_buffer
      //   approx_nframes = typical nframes of average size per max_frame_size*
      //		    num_frames_per_buffer
      // NOTE:  
      // XIL_CIS_AVG_PER_MAX was chosen from gathering statistics on the JPEG
      // compressor.  May need to explore effects on CELL, other codecs.

      XilCisBuffer* buffer;
      buffer = new XilCisBuffer(max_frame_size*num_frames_per_buffer,
				XIL_CIS_AVG_PER_MAX*num_frames_per_buffer);
      
      
      // check creation of object
      if (! buffer->isOK()) {
        XIL_ERROR( NULL, XIL_ERROR_RESOURCE,"di-277",FALSE);
        return(NULL);
      }

      // check to see if the read buffer is at the location of
      // the write buffer and if the read frame is at the end of
      // this buffer. The following test checks the opposite:
      
      if (r_buffer != w_buffer || r_buffer->frameAtRfptr()){

        // the read frame is not at the write frame position
        // of the write buffer. Thus, only set w_ members 

        w_buffer = buffer;  
        w_buffer_pos = buffer_list->append(buffer);

      } else {

        // the read frame is at the write frame position
        // of the write buffer. Thus, set w_ and r_ members 

        // if the r_buffer == s_buffer and there are no frames present in
        // the start buffer, remove it, and set s_ members
        if ((s_buffer == r_buffer) && ((s_buffer->getNumFrames()) == 0)) {
          XilCisBuffer* del_buf = buffer_list->remove(s_buffer_pos);
          delete del_buf;
          s_buffer = r_buffer = w_buffer = buffer;  
          s_buffer_pos = r_buffer_pos = w_buffer_pos = buffer_list->append(buffer);
       }
        else {
          r_buffer = w_buffer = buffer;  
          r_buffer_pos = w_buffer_pos = buffer_list->append(buffer);
       }
      }
    } 
  } else {
    
    // first frame: no buffers exist yet so create a new buffer
    // of the desired size and set w_, r_ and s_ members:
    
    XilCisBuffer* buffer;
    buffer = new XilCisBuffer(max_frame_size*num_frames_per_buffer,
                              num_frames_per_buffer*2);
    
    // check creation of object
    if (! buffer->isOK()) {
      XIL_ERROR( NULL, XIL_ERROR_RESOURCE,"di-277",FALSE); 
      return(NULL);  
    }  
    
    
    // set all members to this buffer
    s_buffer = r_buffer = w_buffer = buffer;  
    s_buffer_pos=r_buffer_pos=w_buffer_pos=buffer_list->append(buffer);
  }
  
  return w_buffer;
}

//------------------------------------------------------------------------
//
//  Function:	XilCisBufferManager::nextBufferSpace()
//  Created:	92/05/27
//
//  Description:
//	
//	Like nextBuffer, this routine returns space that can be
//      used by a compressor to write data into. Unlike nextBuffer
//      (which returns a pointer to a XilCisBuffer object), this
//      routine returns a pointer to the data space associated
//      with the write buffer.
//
//      The routine first makes a request for the next buffer
//      and then, after setting the write buffer lock flag,
//      retrieves and returns the write buffer's write pointer.
//	
//  Returns:
//	
//	Xil_unsigned8*: pointer to a buffer space
//
//------------------------------------------------------------------------
Xil_unsigned8* 
XilCisBufferManager::nextBufferSpace()
{
  
  // get the next buffer by making a call to nextBuffer
  XilCisBuffer* nbuf = nextBuffer();
  
  // check to see if buffer valid
  if (nbuf == NULL)
    return NULL;
  
  // lock the buffer
  w_buffer_locked = TRUE;
  
  // return the buffer's write pointer
  return nbuf->getWptr();
}

//------------------------------------------------------------------------
//
//  Function:	void XilCisBufferManager::compressedFrame(int type)
//  Created:	92/04/20
//
//  Description:
//	
//     This routine is meant to be called by a compressor after it
//       has completely finished compressing a single frame of data.
//       The routine will then increment the number of frames in the write
//       buffer appropriately.
//
//     The optional parameter 'type' can be set to a specific frame
//     type which has meaning only to the codec's and is used to
//     mark key frames.  It defaults to XIL_CIS_DEFAULT_FRAME_TYPE.
//
//  Returns:
//
//	int: bytes_in_frame.
//
//------------------------------------------------------------------------
int 
XilCisBufferManager::compressedFrame(int type)
{
  int bytes_in_frame = 0;
  
  if (w_buffer){
    bytes_in_frame = w_buffer->incrNumCompressedFrames(write_frame_id, 1, type);

    // only update the write frame id if already known
    if (write_frame_id != XIL_CIS_UNKNOWN_NUMBER_OF_FRAMES)
      write_frame_id++;
  } else {
    
    // Write buffer does not exist. Not possible to have compressed
    // a frame.  Did you call nextBuffer or nextBufferSpace?
    // Internal error: probably from XilDeviceCompression.
    
    XIL_ERROR( NULL, XIL_ERROR_SYSTEM,"di-95",FALSE);
  }

  return bytes_in_frame;
}

//------------------------------------------------------------------------
//
//  Function:	XilCisBufferManager::doneBufferSpace(int nbytes, int type)
//  Created:	92/05/27
//
//  Description:
//	
//	This routine is called after a compressor has completely finished
//      a frame of data and only if the compressor had previously called
//      nextBufferSpace. The compressor must inform this routine of the
//      number of bytes written into the CIS. Errors may occur if this
//      amount is larger than frame size.
//	
//  Parameters:
//	
//	int nbytes: number of bytes written since last call to
//                  nextBufferSpace.  If this is "-1", flag to
//                  cancel request for nextBufferSpace. Necessary
//                  to allow molecules to fail and follow atomic path.
//
//      int type:   type of frame that was compressed (optional)
//		    defaults to XIL_CIS_DEFAULT_FRAME_TYPE.
//  Returns:
//
//	int: bytes_in_frame.  If an error occurs return "0".
//
//  Side Effects/Notes:
//	
//	If the number of bytes written into the buffer is greater than
//      max_frame_size an error is reported, even if the buffer has the
//	space available.  It is illegal to write more than max_frame_size bytes
//	for a single frame.
//      If (nbytes == -1) this means the compressor cancels its 
//      request for nextBufferSpace.  Provided to allow molecules to
//      fail and follow atomic path. Otherwise, the compressor must
//      make sure the compress succeeds into a private data buffer and
//      then copy into the buffer space, which is SLOW.
//
//------------------------------------------------------------------------

int 
XilCisBufferManager::doneBufferSpace(int nbytes, int type)
{
  int bytes_in_frame = 0;
  XiliCisBufferLListPositionType  buffer_pos;  

  if (w_buffer){
    
    if (nbytes>max_frame_size){
      
        // Wrote more than max_frame_size into buffer space, illegal
        XIL_ERROR( NULL, XIL_ERROR_SYSTEM, "di-337", TRUE);
        return bytes_in_frame;
    }
    
    if (nbytes<0){
       if (nbytes==-1){
          // Compressor cancels the request of nextBufferSpace
          // Restore w_buffer (and possibly r_buffer,s_buffer members)
          // in case the w_buffer was allocated because of nextBufferSpace()
          if (w_buffer->getNumFrames() == 0) {
             // Buffer is empty, allocated because previous buffer did
             // not have room for possible frame OR because buffer list
             // was empty
             if (w_buffer_pos == s_buffer_pos) {
                // This is only buffer in list, 
                // remove it and restore (w_,r_,s_)buffer members
                // for empty list
                XilCisBuffer* del_buf = buffer_list->remove(w_buffer_pos);
                delete del_buf;
                s_buffer = r_buffer = w_buffer = NULL;
                s_buffer_pos=r_buffer_pos=w_buffer_pos=NULL;
             }
             else {
                // Multiple buffers in list
                // Get previous buffer position, delete this w_buffer
                buffer_pos = buffer_list->previous(w_buffer_pos);
                XilCisBuffer* del_buf = buffer_list->remove(w_buffer_pos);
                delete del_buf;
                if (r_buffer_pos == w_buffer_pos) {
                   // r_buffer was also at last buffer in list
                   // restore to end of previous buffer
                   r_buffer_pos = buffer_pos;
                   r_buffer = buffer_list->retrieve(r_buffer_pos);
                   r_buffer->unRead();
                }
                w_buffer_pos = buffer_pos;
                w_buffer = buffer_list->retrieve(w_buffer_pos);
             }
             
          }  // end if (w_buffer->getNumFrames() == 0)
          
          bytes_in_frame = -1;
       } 
       else {
          // Wrote negative number of bytes into buffer space, illegal
          XIL_ERROR( NULL, XIL_ERROR_SYSTEM, "di-342", TRUE);
          return bytes_in_frame;
       }
     }
     else {
       // set the new write pointer.
       w_buffer->setWptr(w_buffer->getWptr()+nbytes);
    
       // now increment the compressed frame
       bytes_in_frame = w_buffer->incrNumCompressedFrames(write_frame_id, 1, type);

       // only update write frame id if already known
       if (write_frame_id != XIL_CIS_UNKNOWN_NUMBER_OF_FRAMES)
         write_frame_id++;

     } // end if/else (nbytes==-1)

     // unlock buffer
     w_buffer_locked = FALSE;

  } else {
    
    // Write buffer does not exist. Not possible to have compressed
    // a frame.  Did you call nextBuffer or nextBufferSpace.
    // (this could be a secondary error from a failure of a `new' above).
    // Internal error.
    
    XIL_ERROR( NULL, XIL_ERROR_SYSTEM, "di-95", FALSE );
  }

  return bytes_in_frame;
}

//------------------------------------------------------------------------
//
//  Function:	Xil_unsigned8* XilCisBufferManager::nextFrame(Xil_unsigned8** r_buffer_end, Xil_boolean need_EOF)
//  Created:	92/04/20
//
//  Description:
//	
//    This routine is meant to be called by a decompressor when it
//      requires a pointer to the first byte the next frame to
//      decompress. If the read buffer is NULL, the CIS is empty and
//      NULL is returned. If the read buffer does exist and if there 
//	is a complete frame in the read buffer at the read position, 
//	then a pointer to the start of this frame is returned. 
//	If a partial frame possibly exists at the end of the buffer,
//	and if it can not be determined if the next frame is complete, 
//	then partial frame handling will be invoked.
//	
//  optional Parameters:
//	
//    Xil_unsigned8** r_buffer_end : a pointer to the last valid byte of 
//      the current r_buffer.
//    Xil_boolean need_EOF : TRUE if need r_buffer_end to point to the
//      end of current frame instead of end of buffer.
//	
//  Returns:
//	
//    Xil_unsigned8* : a pointer to the first byte of the next frame to
//                     decompressed as reflected by the read buffer and
//                     the read pointer within this buffer. NULL if the
//                     read buffer does not exist (CIS empty), there is
//		       no frame at read pointer (read to end of CIS), 
//		       or failed search for next frame boundary.
//
//------------------------------------------------------------------------
Xil_unsigned8* 
XilCisBufferManager::nextFrame(Xil_unsigned8** r_buffer_end,
                                              Xil_boolean need_EOF)
{
  
  if (r_buffer == NULL){
    
    // this would occur only if CIS empty
    // No data to decompress
        
    if (r_buffer_end) {
      *r_buffer_end = NULL;
    }
    return NULL;
    
  } else {
    
    // The read buffer exist, does it contain data at the read position? 
    
    if ( (r_buffer->frameAtRfptr()) ) {
      
      // check partial frame flag of buffer
      if (r_buffer->hasPartialFrame() == FALSE) {
        if (r_buffer_end) {
          *r_buffer_end = r_buffer->getWfptr();
        }
        return r_buffer->getRfptr();
      }
      else {
        
        // the buffer may have a partial frame. check to see if a 
        // frame boundary possibly lies after this current frame
        
        if (r_buffer->frameAfterRfptr(max_frame_size,need_EOF)) {
          if (r_buffer_end) {
               if (need_EOF == TRUE)
                 *r_buffer_end = (r_buffer->getRfptr() + r_buffer->getNumBytesInRFrame());
               else
                 *r_buffer_end = r_buffer->getWfptr();
          }
          return r_buffer->getRfptr();
        } else {
          
          // need to check for frame boundary, first initialize the
          // partial frame byte pointer and the buffer position in
          // which it exist. The call upon the device compression to
          // find the next frame boundary.
          
	  setupPartialFramePtrs(need_EOF);
          finding_next_frame_boundary = TRUE;
          
          int status = device_compression->findNextFrameBoundary();

          CHECK_STATUS_MAYBE(status);

          if ( status == XIL_FAILURE) {
            
            finding_next_frame_boundary = FALSE;
            return NULL;

          } else {
            
            // a frame boundary was found. At this point, the read buffer
            // and the read frame should be ok so just return the pointer
            // of the read frame.
            finding_next_frame_boundary = FALSE;
            if (r_buffer_end) {
               if (need_EOF == TRUE)
                 *r_buffer_end = (r_buffer->getRfptr() + r_buffer->getNumBytesInRFrame());
               else
                 *r_buffer_end = r_buffer->getWfptr();
            }
            return r_buffer->getRfptr();
            
          }
        }
      }
    } else {
      
      // No data at read position so no data to decompress
      if (r_buffer_end) {
        *r_buffer_end = NULL;
      }
      return NULL;
    }
  }
}

//------------------------------------------------------------------------
//
//  Function:	Xil_unsigned8* XilCisBufferManager::hasFrame()
//  Created:	92/04/20
//
//  Description:
//	
//    This routine determines if a complete frame exist at the read
//    position. If partial frames are in the read buffer, the routine
//    findNextFrameBoundary may be called.
//	
//  Returns:
//	
//    Xil_boolean :   TRUE if a complete frame exists at the read position,
//                    else FALSE.
//
//------------------------------------------------------------------------
Xil_boolean 
XilCisBufferManager::hasFrame()
{
  if (r_buffer == NULL)
    return FALSE;
  else {
    
    // The read buffer exist, does it contain data at the read position? 
    
    if (r_buffer->frameAtRfptr()){
      // check partial frame flag of buffer
      if (r_buffer->hasPartialFrame() == FALSE)
        return TRUE;
      else 
        
        // the buffer may have a partial frame. check to see if a 
        // frame boundary possibly lies after this current frame
        
        if (r_buffer->frameAfterRfptr(max_frame_size))
          return TRUE;
        else {
          
          // need to check for frame boundary, first initialize the
          // partial frame byte pointer and the buffer position in
          // which it exist. Then call upon the device compressor to
          // find the next frame boundary.
          
	  setupPartialFramePtrs();
          finding_next_frame_boundary = TRUE;
          
          int status = device_compression->findNextFrameBoundary();

          CHECK_STATUS_MAYBE(status);

          if ( status == XIL_FAILURE) {
            finding_next_frame_boundary = FALSE;
            return FALSE;
          } else {
            
            // a frame boundary was found. At this point, the read buffer
            // and the read frame should be ok so just return the pointer
            // of the read frame.

            finding_next_frame_boundary = FALSE;
            return TRUE;
          }
        }
    } else 
      return FALSE;
  }
}

//------------------------------------------------------------------------
//
//  Function: void XilCisBufferManager::decompressedFrame(Xil_unsigned8* ptr,
//                    int type, void* user_ptr, Xil_boolean update_next)
//  Created:	92/04/20
//
//  Description:
//
//     This routine should be called when a decompressor finishes
//       decompressing a frame. The decompressor should send the
//       byte pointer (ptr) that it received when it made its last
//       call to nextFrame(). If all frames in the current read
//       buffer have been decompressed, then this routine will
//       set the read buffer to the next available buffer.  This routine
//       is also called to register a found frame boundary, either from
//       the device compression or during partial frame handling.  In 
//       this case the flag update_next will indicate if the current frame 
//       has actually been decompressed.
//
//  Parameters:
//	
//	Xil_unsigned8* ptr: the current pointer that the decompressor
//                          used to decompress the last frame.
//
//      int type:   type of frame that was compressed (optional)
//      void* user_ptr:   ptr to user data associated with frame (optional)
//      Xil_boolean update_next:   update next_decompress_frame, ie,
//                                 this frame actually decompressed (optional)
//
//------------------------------------------------------------------------
void 
XilCisBufferManager::decompressedFrame(Xil_unsigned8* ptr, int type, void* user_ptr, Xil_boolean update_next)
{
  
  r_buffer->incrNumDecompressedFrames(ptr, type, user_ptr);
  
  // detect if we are at end of buffer (all frames read)
  
  // if there is not a frame at the read frame position,
  // then increment the read buffer to the next buffer if
  // one exists then reRead the buffer.
  
  if (!r_buffer->frameAtRfptr()){
    
    // update r_ members if another buffer exists, otherwise,
    // the read buffer stays at the write buffer.
    
    if (buffer_list->next(r_buffer_pos) != buffer_list->end()){

      int next_frame_id = r_buffer->getRFrameId();
      r_buffer_pos = buffer_list->next(r_buffer_pos);
      r_buffer = buffer_list->retrieve(r_buffer_pos);
      r_buffer->reRead();
      
      // set start frame id of this buffer if not already known
      
      if (r_buffer->getStartFrameId() == XIL_CIS_UNKNOWN_FRAME_ID &&
          next_frame_id != XIL_CIS_UNKNOWN_FRAME_ID)
        r_buffer->adjustStartFrameId(next_frame_id);

    } else 

      // we have reached the end of all buffers, thus we now
      // the number of frames if not known before. This is
      // equal to the current value of the read frame id.

      if (write_frame_id == XIL_CIS_UNKNOWN_FRAME_ID){
        
        // set the value for the number of frames 
        write_frame_id = r_buffer->getRFrameId();
        
        // clear the read buffer of its partial
        // frame flag if it has been set:
        r_buffer->setPartialFrame(FALSE);
        buffers_with_partial_frames = 0;
        
        // set the buffers number of fames
        r_buffer->setNumFrames(write_frame_id - r_buffer->getStartFrameId());
      }
  }

  if (update_next == TRUE)
    next_decompress_frame = r_buffer->getRFrameId();    

  return;
}

//------------------------------------------------------------------------
//
//  Function:	int XilCisBufferManager::hasData()
//  Created:	92/04/20
//
//  Description:
//	
//    If the read buffer exist, this routine determine the number
//      of total bytes in CIS by suming the number of bytes in
//      each buffer starting at the read buffer and ending with 
//      the write buffer.
//	
//  Returns:
//	
//    int:   total number of bytes contained in frames 
//
//------------------------------------------------------------------------
int 
XilCisBufferManager::hasData()
{
  
  // initialize number of bytes to zero
  int num_bytes = 0;
  
  // if read buffer exist, sum the number of bytes in each
  // buffer from the r_buffer to the end of the buffer list
  
  if (r_buffer != NULL){
    
    XiliCisBufferLListPositionType pos;
    XilCisBuffer* buf;
    
    // some bytes in the read buffer may have already
    // been read, therefore we must check on the
    // number of available bytes in the read buffer
    // by making a call to numAvailBytes()
    
    pos = r_buffer_pos;
    buf = r_buffer;
    num_bytes += buf->numAvailBytes();
    
    // all of the bytes in the rest of the buffers
    // are ok to count as available. Therefore just
    // use the call to getNumBytes() for rest
    
    for (pos=buffer_list->next(pos);
         pos!=buffer_list->end();
         pos=buffer_list->next(pos)){
      
      buf = buffer_list->retrieve(pos);
      num_bytes += buf->getNumBytes();
    }
  }
  
  return num_bytes;
}

//------------------------------------------------------------------------
//
//  Function:	int XilCisBufferManager::numberOfFrames()
//  Created:	92/04/20
//
//  Description:
//	
//    If the read buffer exist, this routine determine the number
//      of total frames in CIS by suming the number of frames in
//      each buffer starting at the read buffer and ending with 
//      the write buffer. If any buffer has the possiblity of
//      having partial frames, these buffers must be resolved
//      using the partial frame handling.
//	
//  Returns:
//	
//    int:   total number of complete frames from read to end of
//           write buffer
//
//------------------------------------------------------------------------
int 
XilCisBufferManager::numberOfFrames()
{
  
  // initialize number of frames to zero
  int nframes = 0;
  int init_read_frame_id;
  int last_frame_id;
  Xil_boolean end_partial = FALSE;
  
  // if read buffer exist, sum the number of frames in each
  // buffer from the r_buffer to the end of the buffer list
  
  if (r_buffer != NULL){
    
    XiliCisBufferLListPositionType pos;
    XilCisBuffer* buf;
    
    // save the initial read frame id.
    init_read_frame_id = r_buffer->getRFrameId();
    
    // step through buffer list from read buffer to write buffer
    // and determine if the number of frames in each buffer is
    // known. If not, then it must be determined. When done with
    // this process, the number of frames should be equal to the
    // last known frame id minus the initial read frame id. 
    
    pos=r_buffer_pos;
    buf = r_buffer;
    
    while (r_buffer != w_buffer){
      if (r_buffer->getNumFrames() == XIL_CIS_UNKNOWN_NUMBER_OF_FRAMES) {
        if (handlePartialFrame() == XIL_FAILURE) {
          if (pf_ptr == w_buffer->getWfptr()) {
            // failed because partial frame with multiple segments at 
            // end of cis.  Break to handle below.
            end_partial = TRUE;
            break;
          }
          else {
            // partial frame not at end of cis, cannot handle here!
            return XIL_CIS_UNKNOWN_NUMBER_OF_FRAMES;
          }
        }
      }
      if (buffer_list->next(r_buffer_pos) != buffer_list->end()) {
        r_buffer_pos = buffer_list->next(r_buffer_pos);
        r_buffer = buffer_list->retrieve(r_buffer_pos);
        // reset the next read buffer
        r_buffer->reRead();
      }
    }
    
    // we should now be at the write buffer position. A failure
    // at handling a partial frame in the write buffer does not
    // mean that the number of frames is unknown. Instead, it
    // just means that an incomplete frame lies at the end of
    // the cis. This routine does not count that partial frame
    // in its sum.
    
    
    if (r_buffer->getNumFrames() == XIL_CIS_UNKNOWN_NUMBER_OF_FRAMES) {
      if ((end_partial == TRUE) || (handlePartialFrame() == XIL_FAILURE)){
        // since we did fail, a partial frame was at end of cis.
        // The routine handlePartialFrame() will not have readjusted
        // the read frame within the write buffer at all. This means
        // that the id of the last known frame is equal to the current
        // read frame id.
        last_frame_id = r_buffer->getRFrameId();
      } else {
        // the handling of partial frames was a success. Thus the
        // routine handlePartialFrame() has alread readjusted the
        // read/write buffer back to its reread position. The last
        // frame id is then the start frame id of this buffer plus
        // the buffer's number of frames (which is now known).
        last_frame_id = r_buffer->getNumFrames() + r_buffer->getStartFrameId();
      }
    } else {
      // the number of frames in the last buffer are known. Thus 
      // the last frame id is then the start frame id of this 
      // buffer plus the buffer's number of frames
      last_frame_id = r_buffer->getNumFrames() + r_buffer->getStartFrameId();
    }
    
    nframes = last_frame_id - init_read_frame_id;
    seek(init_read_frame_id,XIL_CIS_ANY_FRAME_TYPE);
    
  }
  
  return nframes;
}

//------------------------------------------------------------------------
//
//  Function:	void* XilCisBufferManager::getBitsPtr(int* nbytes,
//                                                    int* nframes)
//  Created:	92/04/20
//
//  Description:
//	
//   If the read buffer exist, this routine gets number of bytes
//     and frames in this buffer and then returns a pointer to the
//     start of the data within the buffer. Only complete frames are
//     are counted.
//
//   If the read buffer does not exist, nframes and nbytes are set
//     to 0 and NULL is returned.
//
//   Since only complete frames can be returned by this routine,
//   a buffer containing partial frames must be resolved before
//   data can be retrieved from it.
//  
//  Parameters:
//	
//     int* nbytes:   number of bytes
//     int* nframes:  number of frames
//
//  Returns:
//	
//     void*:  pointer to byte data
//
//------------------------------------------------------------------------
void* 
XilCisBufferManager::getBitsPtr(int* nbytes, int* nframes)
{
  
  // make sure there is data in the cis
  if (r_buffer != NULL){

    // check to see if unknown number frames
    if (r_buffer->getNumFrames() == XIL_CIS_UNKNOWN_NUMBER_OF_FRAMES) {

      // check to see if read buffer might have a partial frame at its end
      if (handlePartialFrame() == XIL_FAILURE) {
          *nbytes = 0;
          *nframes = 0;
          return 0;
      }
    }

    // set nbytes, nframes and buf
    Xil_unsigned8* buf = r_buffer->getAvailData(nbytes,nframes);
    
    // update r_ members
    
    if (buffer_list->next(r_buffer_pos) != buffer_list->end()){
      r_buffer_pos = buffer_list->next(r_buffer_pos);
      r_buffer = buffer_list->retrieve(r_buffer_pos);
      
      // reset the next read buffer
      r_buffer->reRead();
    } 
    
    return buf;
    
  } else {
    
    // the cis is empty, set number of bytes
    // and frames to zero and return 0 data
    
    *nbytes = 0;
    *nframes = 0;
    return 0;
  }
}

//------------------------------------------------------------------------
//
//  Function:	void XilCisBufferManager::updateWriteFrameId()
//  Created:	92/07/20
//
//  Description:
//	
//	Determines write frame id if known and updates this value
//      and the manager's count of the number of frames in the
//      buffers. This routine is called only by the two putBit
//      routines. The routine also sets the partial frame flag
//      in the write buffer if the nframes value sent to the
//      putsBits routines was set to XIL_CIS_PARTIAL_FRAME.
//	
//  Parameters:
//	
//	int nframes:   the value sent to the putBit routines:
//                     valid values: > 0
//                                   XIL_CIS_UNKNOWN_NUMBER_OF_FRAMES (-1)
//                                   XIL_CIS_PARTIAL_FRAME (0)
//
//------------------------------------------------------------------------
void 
XilCisBufferManager::updateWriteFrameId(int nframes)
{
  // check for unknown number of frames and partial frames
  if (nframes == XIL_CIS_UNKNOWN_NUMBER_OF_FRAMES)
    write_frame_id = XIL_CIS_UNKNOWN_FRAME_ID;
  else if (nframes == XIL_CIS_PARTIAL_FRAME){
    w_buffer->setPartialFrame(TRUE);
    buffers_with_partial_frames++;
    write_frame_id = XIL_CIS_UNKNOWN_FRAME_ID;
  } else 
    // if we know what the current write frame id is, we can increment
    // the number of frames in the buffers and set the new w frame id
    if (write_frame_id != XIL_CIS_UNKNOWN_FRAME_ID)
      write_frame_id += nframes;
}

//------------------------------------------------------------------------
//
//  Function:	void XilCisBufferManager::putBits(int nbytes,int nframes,
//                                             void* data)
//  Created:	92/04/20
//
//  Description:
//	
//    This routine may create a new buffer of size nbytes to which
//      data can be added. After updating all the write pointers, nbytes
//      of data is added (copied) into the buffer. Then the buffer is set 
//      to have nframes frames. If this is the first buffer created, the
//      read and start buffers are also set. 
//
//    If nframes is equal to XIL_CIS_PARTIAL_FRAME, this means than an
//      uknown number of frames exist in the bytes being supplied and
//      that a partial frame may exist at the end.
//
//  Parameters:
//	
//	int nbytes:   number of bytes of data
//      int nframes:  number of frames data represents
//      void* data:   pointer to data
//
//------------------------------------------------------------------------
void 
XilCisBufferManager::putBits(int nbytes, int nframes, void* data)
{
  // Check to see if we need a new buffer for the incoming bytes.
  // We need a new buffer if this is the first set of frames (w_buffer
  // is NULL) or if there is not enough room in the current buffer for
  // the supplied number of bytes
  
  Xil_boolean num_frames_known = (nframes>0) ? TRUE : FALSE;
  
  if (w_buffer == NULL ||
      (w_buffer->getBufferSize() - w_buffer->getNumBytes() < nbytes) ||
      (w_buffer->hasPartialFrame() == TRUE)) {
    
    // create new buffer of size nbytes 
    
    XilCisBuffer* buffer;
    
    // if the number of frames is not known, try to make an approximation
    
    if (num_frames_known) {
      buffer = new XilCisBuffer(nbytes,nframes);
      
      // check creation of object
      if (! buffer->isOK()) {
        XIL_ERROR( NULL, XIL_ERROR_RESOURCE,"di-277",FALSE);
        return;
      }

    } else {
      
      // to avoid the growth penalty of the array frame
      // list in the buffer, set the approx number of
      // frames to a high value.  default value comes from
      // movie stats.
      
      buffer = new XilCisBuffer(nbytes,XIL_CIS_DEFAULT_PUTBITS_NFRAMES);
      
      // check creation of object
      if (! buffer->isOK()) {
        XIL_ERROR( NULL, XIL_ERROR_RESOURCE,"di-277",FALSE);
        return;
      }
    }
    
    // check to see if the current read position is at the last frame
    if (r_buffer && (r_buffer != w_buffer || r_buffer->frameAtRfptr())){
      // not at last frame: set w_ members to this buffer
      w_buffer = buffer;  
      w_buffer_pos = buffer_list->append(buffer);
    } else {
      // the read frame is at last valid frame:
      // thus set w_ and r_ members to this buffer
      r_buffer = w_buffer = buffer;  
      r_buffer_pos = w_buffer_pos = buffer_list->append(buffer);
    }
    
    // now copy frames into buffer
    w_buffer->copyFrames(nbytes, nframes, (Xil_unsigned8*)data,write_frame_id);
    
    // update the write frame id if known    
    updateWriteFrameId(nframes);
    
    // if first frame
    if (s_buffer == NULL){
      s_buffer = w_buffer;
      s_buffer_pos = w_buffer_pos;
    }
    
  } else {
    // there is enough room in this buffer for the incoming
    // number of frames. So no need to create a new buffer.
    
    // now copy frames into buffer
    w_buffer->copyFrames(nbytes, nframes, (Xil_unsigned8*)data,write_frame_id);
    
    // update the write frame id if known    
    updateWriteFrameId(nframes);
  }
}


//------------------------------------------------------------------------
//
//  Function:	void XilCisBufferManager::putBitsPtr(int nbytes,int nframes,
//                                                   void* data,
//                                                   XIL_FUNCPTR_DONE_WITH_DATA done_data)
//  Created:	92/04/20
//
//  Description:
//
//    This routine always create a new buffer of size nbytes and sets
//      it to use data as its buffer area. This data is not copied into
//      the buffer, the buffer actually points to this data. Thus it is
//      important that this data remain valid.
//
//      After updating all the write pointers, the buffer is set 
//      to have nframes frames. If this is the first buffer created, the
//      read and start buffers are also set. 
//
//    If nframes is equal to XIL_CIS_PARTIAL_FRAME, this means than an
//      uknown number of frames exist in the bytes being supplied and that
//      a partial frame may exist at the end.
//
//  Parameters:
//	
//	int nbytes:   number of bytes of data
//      int nframes:  number of frames data represents
//      void* data:   pointer to data
//      XIL_FUNCPTR_DONE_WITH_DATA done_data: pointer function that
//                    will be called when data is no longer needed
//                    by this cis.
//
//------------------------------------------------------------------------
void 
XilCisBufferManager::putBitsPtr(int nbytes, int nframes, void* data, XIL_FUNCPTR_DONE_WITH_DATA done_data)
{
  // create new buffer and make it use supplied buffer
  XilCisBuffer* buffer;
  
  // if the number of frames is not known, try to make an approximation
  
  if (nframes > 0) {
    buffer = new XilCisBuffer(nbytes,nframes,
                              (Xil_unsigned8*)data,write_frame_id,done_data);
    
    // check object creation
    if (! buffer->isOK()) {
      XIL_ERROR( NULL, XIL_ERROR_RESOURCE,"di-277",FALSE);
      return;
    }  
    
  } else {

    // to avoid the growth penalty of the array frame
    // list in the buffer, set the approx number of
    // frames to a high value. default comes from movie stats.

    buffer = new XilCisBuffer(nbytes,nframes, (Xil_unsigned8*)data,write_frame_id,done_data,XIL_CIS_DEFAULT_PUTBITS_NFRAMES);
    
    
    // check object creation
    if (! buffer->isOK()) {
      XIL_ERROR( NULL, XIL_ERROR_RESOURCE,"di-277",FALSE);
      return;
    }  
    
  }
  
  // check to see if the current read position is at the last frame  
  if (r_buffer && (r_buffer != w_buffer || r_buffer->frameAtRfptr())){
    // not at last frame: set w_ members to this buffer    
    w_buffer = buffer;  
    w_buffer_pos = buffer_list->append(buffer);
  } else {
    // at last frame: set both r_ and w_ members to new buffer
    r_buffer = w_buffer = buffer;
    r_buffer_pos = w_buffer_pos = buffer_list->append(buffer);
  }
  
  // update the write frame id if known
  updateWriteFrameId(nframes);   
  
  // set start if not already set (which occurs when cis empty)
  if (s_buffer == NULL){
    s_buffer = w_buffer;
    s_buffer_pos = w_buffer_pos;    
  }
  
}

//------------------------------------------------------------------------
//
//  Function:	int XilCisBufferManager::seek(int framecount, int type)
//  Created:	92/05/ 4
//
//  Description:
//	
//	Determines the seek direction and call the appropriate seek
//      routine to seek as close to the given framenumber as possible.
//
//      Once this position is reached (and if no errors had occured)
//      the position is checked to see if the frame type of this
//      position matches that of the given (parameter) type.  The
//      default for this parameter is XIL_CIS_ANY_FRAME_TYPE which
//      matches any frame type.
//
//      If the frame type is not matched, then a seek backwards is
//      performed to find the first occurance of the given type.
//
//  Parameters:
//	
//	int framecount:    number of frames to seek
//      int type:          type of frame to seek to.
//
//  Returns:
//	
//	int:               positive values mean seek could not
//                           complete and therefore a burn is
//                           necessary.
//
//                         negative values reflect some type of
//                           error.
//
//  Notes:
//
//      See class header file doc for more information
//
//------------------------------------------------------------------------

int 
XilCisBufferManager::seek(int framenumber, int type)
{
  // save address of current read frame pointer
  // init ptrs, flags to invalid values
  int frames_to_burn = 0;
  int current_frame = -2;
  int cur_frame_type = -2;  
  int  read_frame_type;
  int* read_frame_type_ptr;  
  
  // make sure CIS is not empty
  
  if (r_buffer != NULL){
    
    current_frame = r_buffer->getRFrameId();
    
    if (framenumber == current_frame ) {

      // we are at the desired number, check the type
      // NOTE: next_decompress_frame has type "ANY_TYPE"
      if ((current_frame == next_decompress_frame) ||
          (type == XIL_CIS_ANY_FRAME_TYPE) ||
          (type == XIL_CIS_NO_BURN_TYPE) ||
          (r_buffer->getRFrameType() == type))
        // we are at the desired frame and type
        return 0;

    }

    // sanity check: we should always know what frame we are on
    
    
    if (current_frame == XIL_CIS_UNKNOWN_FRAME_ID){
      
      // Current read frame identifier unknown.
      // Internal error.
      
      XIL_ERROR( NULL, XIL_ERROR_USER, "di-95", FALSE);
      
      return -1;
    }
    
    // check to see if frame number exceeds number of frames in the CIS.
    
    if (write_frame_id != XIL_CIS_UNKNOWN_FRAME_ID &&
        framenumber >= write_frame_id){
      
      // SPECIAL CASE:: if the frame number is equal to the write frame
      // identifier, then just seek to then end of the CIS.
      
      if (framenumber == write_frame_id){
        
        // Set the r_buffer to the last buffer in buffer list
        r_buffer_pos = buffer_list->previous(buffer_list->end());
        
        // Check to make sure that this is a valid buffer (it wouldn't
        // be if the buffer list was empty).
        
        if (r_buffer_pos){
          r_buffer = buffer_list->retrieve(r_buffer_pos);
          
          // Seek to last frame
          r_buffer->unRead();
        }
        if ((type == XIL_CIS_ANY_FRAME_TYPE) ||
            (type == XIL_CIS_NO_BURN_TYPE) ||
            (next_decompress_frame == write_frame_id)) {
           // if we are looking for any type (no burn for type)
           // or if write frame is our "special marked frame"
           frames_to_burn = 0;    // at write frame, no burn
        }
        else {
           // backup to next_decompress_frame and burn forward
           if (seekBackward(next_decompress_frame) != 0)
             // ERROR occurred, never burn frames to next_decompress_frame
             frames_to_burn = -1;
           else 
             frames_to_burn = write_frame_id - next_decompress_frame;
        }
      } else {
        
        // "Desired seek frame %d not in CIS\n",framenumber 
        // TODO: pass framenumber as an op to error handler.
        
        XIL_ERROR( NULL, XIL_ERROR_USER, "di-105", TRUE); 
        frames_to_burn = -1;
      }

      return frames_to_burn;
      

    } 



    // Temporarily mark next_decompress_frame's type as XIL_CIS_ANY_FRAME_TYPE
    
    if (next_decompress_frame != write_frame_id) {
      if (current_frame > next_decompress_frame) {
        if (seekBackward(next_decompress_frame) != 0)
          // ERROR occurred, should never burn frames to next_decompress_frame
          return(-1);
        current_frame = next_decompress_frame;
      } else if (current_frame < next_decompress_frame) {
        if (seekForward(next_decompress_frame) != 0)
          // ERROR occurred, should never burn frames to next_decompress_frame
          return(-1);
        current_frame = next_decompress_frame;
      }

    }

    if ((read_frame_type_ptr = r_buffer->getRFrameTypePtr())!=NULL){
      read_frame_type = *read_frame_type_ptr;
      *read_frame_type_ptr = XIL_CIS_ANY_FRAME_TYPE;
    }

    if (framenumber > current_frame){
      
      // The desired frame lies between the current frame and the last frame.
      
      frames_to_burn = seekForward(framenumber);
      
    } else {
      
      // The desired frame lies between the current frame and frame 0.
      
      // Determine the frame id for the first frame in CIS
      
      int start_frame = s_buffer->getStartFrameId();
      int first_frame = start_frame + s_buffer_frame_offset;
      
      if (framenumber < start_frame){
        
        // Desired seek frame # not in CIS  
        // TODO: pass framenumber as an op to error handler. 
        
        XIL_ERROR( NULL, XIL_ERROR_USER, "di-105", TRUE);  
        frames_to_burn = -1;
        
      } else if (framenumber == first_frame){

        // SPECIAL CASE: we want to seek to first frame in CIS
        // Therefore, set r_buffer to first buffer in buffer list.
        
        r_buffer_pos = buffer_list->start();
        r_buffer = buffer_list->retrieve(r_buffer_pos);
        
        // Reset this buffer so that it may be read again. This places
        // us at the first frame. Thus, the number of frames to burn is
        // equal to s_buffer_frame_offset. 
        
        r_buffer->reRead();
        if (s_buffer_frame_offset)
          frames_to_burn = seekForward(framenumber);            
        else
          frames_to_burn = 0;

      } else
        // Desired frame lies before current frame
        frames_to_burn = seekBackward(framenumber);
    }
    
  } else {
    
    // read buffer == NULL means CIS is empty
    
    // SPECIAL CASE: if framenumber is equal to 0 do not issue an error
    // message. Just return 0 as the number of frames to burn.
    
    if (framenumber == 0)
      return 0;
    else {
      
      // "CIS empty. Can not seek to frame %d\n",framenumber
      // TODO: pass framenumber as an op to error handler.
      
      XIL_ERROR( NULL, XIL_ERROR_USER, "di-107", TRUE);
      return -1;
    }
  }
  
  // check for error
  
  // if caller asked to seek to a frame position without burn frames,
  // skip extra burn for type (not necessary).
  if (!((frames_to_burn == 0) && (type == XIL_CIS_NO_BURN_TYPE))) {

     if (frames_to_burn >= 0){
        
        // no error occured. Check to see if frame type matches. If not, we must
        // seek backwards to the first occurance of the frame_type, incrementing
        // the variable frames_to_burn as we go
        
        // Note: that if type == XIL_CIS_ANY_FRAME_TYPE, the optional parameter
        // setting , we do no further processing. This type matches any type.
        
        // Recall that the type of the next_decompress_frame upon entry to 
        // this routine was temporarily changed to ANY_TYPE
        
        cur_frame_type = r_buffer->getRFrameType();
        
        if ( r_buffer->getRFrameId() != s_buffer->getStartFrameId()){
           if ((type != XIL_CIS_ANY_FRAME_TYPE) &&
               (type != cur_frame_type) &&
               (cur_frame_type != XIL_CIS_ANY_FRAME_TYPE)){
              
              int extra_frames_to_burn = seekBackToFrameType(type);
              
              // check for error
              if (extra_frames_to_burn < 0)
                frames_to_burn = extra_frames_to_burn;
              else 
                frames_to_burn += extra_frames_to_burn;
           }
        }
     }
  }
  
  // reset the frame type that may have been changed
  if (read_frame_type_ptr)
    *read_frame_type_ptr = read_frame_type;
  
  // check for overall error
  
  if (frames_to_burn >= 0){
    // no error occurred
    return frames_to_burn;
  } else {
    // an error occurred: return negative burn number
    return -1;
  } 
}

//------------------------------------------------------------------------
//
//  Function:	XilCisBufferManager::seekBackward(int framenumber)
//  Created:	92/07/ 8
//
//  Description:
//	
//	This routine keeps setting the read buffer (r_buffer) to the previous
//      buffer until the desired frame number (framenumber) lies within the
//      read buffer.
//
//      This routine expects:
//
//       o the r_buffer to be valid
//       o the desired frame number to be in either the current read buffer
//           or in a buffer previous to this buffer.
//	
//  Parameters:
//	
//	int framenumber: the desired frame number
//
//  Returns:
//	
// 	int: number of frames to burn to reach desired frame
//           ( -1 if an error occurred)
//
//------------------------------------------------------------------------

int 
XilCisBufferManager::seekBackward(int framenumber)
{
  
  int start_id;
  
  // get start id of current read buffer
  start_id = r_buffer->getStartFrameId();
  
  // while the desired framenumber is less than the start id
  // of the current buffer and while we have not yet reached the
  // beginning of the buffer list: get the previous position,
  // retrieve the buffer from that position and get its start id.
  
  while (framenumber < start_id && r_buffer_pos != s_buffer_pos){
    r_buffer_pos = buffer_list->previous(r_buffer_pos);
    r_buffer = buffer_list->retrieve(r_buffer_pos);
    start_id = r_buffer->getStartFrameId();
  }
  
  // framenumber should now be between the start id and this
  // value plus the number of frames in the current read buffer.
  // Even if the number of frames is unknown, we should still
  // be at the correct buffer.
  
  return r_buffer->seekTo(framenumber);
}

//------------------------------------------------------------------------
//
//  Function:	XilCisBufferManager::seekForward(int framenumber)
//  Created:	92/07/ 8
//
//  Description:
//	
//	This routine keeps setting the read buffer (r_buffer) to the next
//      buffer until the desired frame number (framenumber) lies within the
//      read buffer.
//
//      This routine expects:
//
//       o the r_buffer to be valid
//       o the desired frame number to be in either the current read buffer
//           or in a buffer after this buffer.
//	 o that the case of framenumber 'write_frame_id' is taken care of by
//           XilCisBufferManager::seek
//
//  Parameters:
//	
//	int framenumber: the desired frame number
//
//  Returns:
//	
// 	int: number of frames to burn to reach desired frame
//           ( -1 if an error occurred)
//
//------------------------------------------------------------------------

int 
XilCisBufferManager::seekForward(int framenumber)
{
  
  int start_id;
  int last_id;
  int nframes;
  
  // get start id of read buffer
  start_id =  r_buffer->getStartFrameId();
  
  // calculate last id of read buffer
  nframes = r_buffer->getNumFrames();
  last_id = (nframes != XIL_CIS_UNKNOWN_NUMBER_OF_FRAMES) ?
    (start_id + nframes - 1) : XIL_CIS_UNKNOWN_FRAME_ID;
  
  // while the desired framenumber is greater than the last id
  // of the current buffer and while we have not yet reached the
  // end of the buffer list: get the next position, retrieve the
  // buffer from that position and get its start and last id.
  
  while (last_id != XIL_CIS_UNKNOWN_FRAME_ID &&
         framenumber > last_id && r_buffer_pos != w_buffer_pos){
    r_buffer_pos = buffer_list->next(r_buffer_pos);
    r_buffer = buffer_list->retrieve(r_buffer_pos);
    start_id = r_buffer->getStartFrameId();
    nframes = r_buffer->getNumFrames();
    last_id = (nframes != XIL_CIS_UNKNOWN_NUMBER_OF_FRAMES) ?
      (start_id + nframes - 1) : XIL_CIS_UNKNOWN_FRAME_ID;
  }
  
  // framenumber should now be between the start id and the last id
  
  if (framenumber >= start_id &&
      (last_id == XIL_CIS_UNKNOWN_FRAME_ID || framenumber <= last_id)){
    return r_buffer->seekTo(framenumber);
  } else {
    
    // Desired framenumber not found
    // TODO: pass framenumber as an op to error handler.
    
    XIL_ERROR( NULL, XIL_ERROR_USER, "di-105", TRUE);
    return -1;
    
  }
}

//------------------------------------------------------------------------
//
//  Function:	XilCisBufferManager::seekBackToFrameType(int type)
//  Created:	92/05/27
//
//  Description:
//	
//	This routine seeks backward from the current read frame
//      until it finds a frame of the given type. The routine
//      then returns a value that represents the number of frames
//      from this new found 'key' frame to the initial read frame
//      position when this routine was called.
//
//      This routine expects:
//
//       o the r_buffer to be valid
//
//  Parameters:
//	
//	int type:  type of frame to locate
//
//  Returns:
//	
//	int: number of frames from frame of given type to initial
//           read frame. If negative, it was because of an error.
//
//------------------------------------------------------------------------
int     
XilCisBufferManager::seekBackToFrameType(int type)
{
  int nframes;             
  int frames_to_type;      
  int frames_to_burn = 0;
  
  // nframes: a count of the frames within a buffer
  // frames_to_type: possible count of number of frames from the
  //                 desired type to the desired frame
  // frames_to_burn: running count of the number of frames to the
  //                 desired frame 
  
  // Determine the number of frames previous to the read frame
  // in current read buffer. This value is equal to the total
  // number of frames in the read buffer that have already been
  // read.
  
  // determine number of frames read
  if (!r_buffer->frameAtRfptr())
    nframes = r_buffer->getNumFrames();
  else
    nframes = (r_buffer->getRFrameId() - r_buffer->getStartFrameId());
  
  // Determine if a frame of the given type lies prior to 
  // the read frame and the beginning of the read buffer
  
  frames_to_type = r_buffer->seekBackwardToFrameType(type);
  
  // If the above operation results in a non negative 
  // value, then this number reflects the number of 
  // frames to burn to get to the selected frame number.
  // This means that the current read frame is now on
  // the first previous frame of the desired type and
  // it is frames_to_type frames before the desired frame.
  
  if (frames_to_type >= 0)
    return frames_to_type;
  
  // If frames_to_type is negative, then a frame of the
  // desired type before the selected frame number was
  // not found in the current read buffer. The value
  // nframes then represents the number of frames in the
  // read buffer that must be burned to get to the dersired
  // frame number.
  
  // We must now continue back through the frame buffers
  // to find a frame of the given type.
  
  while (frames_to_type < 0){
    
    // On the first pass through this loop, nframes is the number
    // of frames in the initial read buffer from its start to the
    // desired seek frame. On all other passes, it is the total
    // number of frames in the last buffer processed. In both cases,
    // this value is added to the running total of the number of frames
    // to burn to get to the desired seek frame.
    
    frames_to_burn += nframes;
    
    // Decrement r_buffer to the previous position in the buffer list
    // if possible. If not, we have go as far back as we can go.
    
    
    if (r_buffer_pos != buffer_list->start()){
      
      // we can go farther
      r_buffer_pos = buffer_list->previous(r_buffer_pos);
      r_buffer = buffer_list->retrieve(r_buffer_pos);
      
    } else {
      
      // we are at the start, can't go back any more
      // Can't go back far enough: no previous (desired) frame
      // to seek backward to.  Check to see if we can just leave
      // device compression at the start frame (essentially means
      // the start frame treated as XIL_CIS_ANY_FRAME_TYPE)

      if (seek_to_start_frame_flag == TRUE) {
        // unable to find specified type, OK to use start frame
        r_buffer->reRead();      // setup rfpos = start, rfptr = buffer start
        frames_to_type = 0;      // at the start frame
        return frames_to_burn;   // burn frames to get to desired framenumber
      }
      else {      
        XIL_ERROR( NULL, XIL_ERROR_USER, "di-108", TRUE);

        return -1;
      }
    }
    
    // Since we are seeking backwards, unread the current buffer. This
    // will place all buffer read pointers to the end of the buffer
    r_buffer->unRead();
    
    // Determine num frames in current read buffer
    nframes = r_buffer->getNumFrames();     
    
    // seek backward in the attempt to find the
    // last occurance of a frame of the given type
    
    frames_to_type = r_buffer->seekBackwardToFrameType(type);
    
  }
  
  // At this point, frames_to_type is non negative and
  // reflects the number of frames from a frame of
  // the desired type to the end of the current buffer.  
  // Addinf this to the frames_to_burn equals a total
  // on the number of frames that must be burned to
  // get from the current frame of the desired type
  // to the framenumber specified by the call to seek.
  
  frames_to_burn += frames_to_type;
  
  return frames_to_burn;
}


//------------------------------------------------------------------------
//
//  Function:	getSFrameId()
//  Created:	92/05/ 4
//
//  Description:
//	
//	Return the first frame id in the start buffer
//	
//  Returns:
//	
//	int: frame id or -1 if error
//	
//------------------------------------------------------------------------


int XilCisBufferManager::getSFrameId()
{
  
  if (s_buffer)
    return s_buffer->getStartFrameId() + s_buffer_frame_offset;
  else {
    
    // No start buffer
    
    return -1;
  }
}

//------------------------------------------------------------------------
//
//  Function:	getRFrameId()
//  Created:	92/05/ 4
//
//  Description:
//	
//	Return frame id of the read frame in the read buffer.
//	
//  Returns:
//	
//	int: frame id or -1 if error
//
//------------------------------------------------------------------------

int 
XilCisBufferManager::getRFrameId()
{
  // check to see if CIS not empty
  if (r_buffer != NULL){
    return r_buffer->getRFrameId();
  } else{
    // CIS is empty
    return 0;
  }
}

//------------------------------------------------------------------------
//
//  Function:	getRFrameType()
//  Created:	92/05/ 4
//
//  Description:
//	
//	Return frame type of the read frame in the read buffer.
//	
//  Returns:
//	
//	int: frame type
//
//------------------------------------------------------------------------

int XilCisBufferManager::getRFrameType()
{
  // check to see if CIS not empty
  if (r_buffer != NULL){
    return r_buffer->getRFrameType();
  } else{
    // CIS is empty
    return XIL_CIS_ANY_FRAME_TYPE;
  }
}

//------------------------------------------------------------------------
//
//  Function:	getRFrameUserPtr()
//  Created:	93/03/26
//
//  Description:
//	
//	Return the user ptr of the read frame in the read buffer.
//	
//  Returns:
//	
//	void* uptr: user ptr 
//      in case of failure, (0)
//------------------------------------------------------------------------

void* 
XilCisBufferManager::getRFrameUserPtr()
{
  // check to see if CIS not empty
  if (r_buffer != NULL){
    return r_buffer->getRFrameUserPtr();
  } else{
    // CIS is empty
    return 0;
  }
}

//------------------------------------------------------------------------
//
//  Function:	XilCisBufferManager::setFrameSize(int size)
//  Created:	92/05/27
//
//  Description:
//	
//	Sets the frame size if the write  buffer is not locked. It
//      is illegal to attempt to change the buffer size while a
//      buffer has been checked out and locked.
//	
//  Parameters:
//	
//	int size: new size for buffers
//
//  Returns:
//	
//	int: success or failure flag
//
//  Deficiencies/ToDo:
//
//      is buffers_with_partial_frames incremented and decremented ok?
//
//------------------------------------------------------------------------

int 
XilCisBufferManager::setFrameSize(int size)
{
  
  if (!w_buffer_locked){
    
    // if partial frames exist, then you can not shrink the
    // frame size because it is used as a test for possible
    // frame boundaries.
    
    if (buffers_with_partial_frames && size < max_frame_size){
      
      // Cannot shrink frame size while a possible partial frame exists
      // Internal error. 
      
      XIL_ERROR( NULL, XIL_ERROR_SYSTEM, "di-95", FALSE);
      
      return XIL_FAILURE;
    } else {
      max_frame_size = size;
      return XIL_SUCCESS;
    }
  }  else {
    
    // Write buffer is locked. Can't change frame size while locked
    // Internal error.
    
    XIL_ERROR( NULL, XIL_ERROR_SYSTEM, "di-95", FALSE);
    
    return XIL_FAILURE;
  }
}

//------------------------------------------------------------------------
//
//  Function:	XilCisBufferManager::setRFrameUserPtr(void* uptr)
//  Created:	93/03/26
//
//  Description:
//	
//	Sets the user ptr of the read frame to the input value.
//	
//  Parameters:
//	
//	void* uptr: user ptr to store
//
//  Returns:
//	
//	int: success(0) or failure(-1) flag
//
//  Deficiencies/ToDo:
//
//      do we have to check locked flag?
//
//------------------------------------------------------------------------

int 
XilCisBufferManager::setRFrameUserPtr(void* uptr)
{

   if (r_buffer != NULL){
      return r_buffer->setRFrameUserPtr(uptr);
   } else {
      // CIS is empty
      return -1;
   }
}

//------------------------------------------------------------------------
//
//  Function:	XilCisBufferManager::reset()
//  Created:	92/05/ 4
//
//  Description:
//	
//	This routine frees all XilCisBuffer buffer objects associated
//      with this object. This is done by making a call on the buffer
//      list to deletePtrElements which transverses the list and calls
//      delete on all the objects pointed to in the list. The buffer
//      list is then made null and all attributes are reset.
//	
//------------------------------------------------------------------------

void 
XilCisBufferManager::reset()
{
  // force deletion of pointer elements (CisBuffers)
  // on the buffer list. Then call makeNull to empty
  // the list
  
  if (buffer_list) {
    buffer_list->deletePtrElements();
    buffer_list->makeNull();
  }

  // reset attributes
  initValues();  
}

//------------------------------------------------------------------------
//
//  Function:	XilCisBufferManager::getNextByte()
//  Created:	92/08/20
//
//  Description:
//	
//    Returns a pointer to the next byte to be examined during
//      partial frame handling. If the next byte exist in the
//      next buffer, advance buffers. 
//	
//  Returns:
//	
//	valid pointer or NULL if no more data exist
//
//  Notes:
//	
//	That the variable pf_bytes_to_process is set the number
//      of bytes that can be examined (1 in this routine). After a
//      call to this routine, a call to this routine or getNextBytes
//      will assume that this byte was examined.
//	
//------------------------------------------------------------------------

Xil_unsigned8* 
XilCisBufferManager::getNextByte()
{
  // increment partial frame pointer
  pf_ptr = pf_ptr + pf_bytes_to_process;
  
  // getNextByte processes one byte
  pf_bytes_to_process = 1;
  
  // check to see if more bytes in this buffer
  if (pf_ptr < pf_end_ptr)
    return pf_ptr;
  
  else if (pf_ptr == pf_end_ptr){
    
    // we are at the end of the buffer. Attempt to advance to
    // the next buffer. If possible, return the next byte to
    // process, else set pf_bytes_to_process to 0 and return NULL
    
    if (buffer_list->next(pf_buffer_pos) != buffer_list->end()){
      pf_buffer_pos = buffer_list->next(pf_buffer_pos);
      pf_buffer = buffer_list->retrieve(pf_buffer_pos);
      pf_buffer->reRead();
      
      pf_ptr = pf_buffer->getRfptr();
      pf_end_ptr = pf_buffer->getWptr();
      
      if (pf_ptr == pf_end_ptr){
        pf_bytes_to_process = 0;
        return NULL;
      } else
        return pf_ptr;
    } else {
      
      // no more buffers left      
      pf_bytes_to_process = 0;
      return NULL;
    }
  } else {
    pf_bytes_to_process = 0;
    return NULL;
  }
}

//------------------------------------------------------------------------
//
//  Function:	XilCisBufferManager::getNextBytes(int* nbytes)
//  Created:	92/08/20
//
//  Description:
//	
//	Returns a pointer to the next byte to be examined during
//      partial frame handling. If the next byte exist in the
//      next buffer, advance buffers. Set the parameter nbytes
//      to the number of contiguous bytes from the byte being
//      examined to the end of the current buffer.
//	
//  Parameters:
//	
//	int* nbytes:  set to the number of bytes that can be examined
//                    given the returned byte pointer.
//  Returns:
//	
//	valid pointer or NULL if no more data exist.
//	
//  Notes:
//	
//	That the variable pf_bytes_to_process is set the number
//      of bytes that can be examined. After a call to this
//      routine, a call to this routine or getNextByte will
//      assume that ALL of these bytes were examined.
//	
//------------------------------------------------------------------------

Xil_unsigned8* 
XilCisBufferManager::getNextBytes(int* nbytes)
{
  // increment partial frame pointer
  pf_ptr = pf_ptr + pf_bytes_to_process;
  
  
  // check to see if more bytes in this buffer
  if (pf_ptr < pf_end_ptr){
    // determine the number of bytes left in this buffer
    *nbytes = pf_bytes_to_process = pf_end_ptr - pf_ptr;
    return pf_ptr;
  } else if (pf_ptr == pf_end_ptr){
    
    // we are at the end of the buffer. Attempt to advance to
    // the next buffer. If possible, return the next byte to
    // process, else set pf_bytes_to_process to 0 and return NULL
    
    if (buffer_list->next(pf_buffer_pos) != buffer_list->end()){
      pf_buffer_pos = buffer_list->next(pf_buffer_pos);
      pf_buffer = buffer_list->retrieve(pf_buffer_pos);
      pf_buffer->reRead();
      
      pf_ptr = pf_buffer->getRfptr();
      pf_end_ptr = pf_buffer->getWptr();
      
      // make sure there are bytes in this buffer
      
      if (pf_buffer->getNumBytes() <= 0){
        *nbytes = pf_bytes_to_process = 0;
        return NULL;
      } else {
        *nbytes = pf_bytes_to_process = pf_end_ptr - pf_ptr;
        return pf_ptr;
      }
    } else {
      
      // no more buffers left
      *nbytes = pf_bytes_to_process = 0;
      return NULL;
    }
  } else {
    *nbytes = pf_bytes_to_process = 0;
    return NULL;
  }  
}

//------------------------------------------------------------------------
//
//  Function:	XilCisBufferManager::foundNextFrameBoundary(Xil_unsigned8*
//                                                          frame_boundary_ptr)
//  Created:	92/08/20
//
//  Description:
//	
//    This routine is meant to be called when the device compressor has
//      found the frame boundary If the frame boundaryis not found then
//      this routine should obviously not be called - the device compressor
//      should return XIL_FAILURE from findNextFrameBoundary to nextFrame.
//
//      The device compressor should pass in the pointer it was using to
//      examine the bit stream for the end of the frame. It should point
//      to one byte beyond the last byte in the processed frame. This
//      routine adjusts the buffers in the buffer list appropriately so
//      that the possibly segmented frame lies within a single allocated
//      buffer space. This basically means copying the segemented parts
//      of the partial frame into a single buffer, deleting any buffers
//      that may have only contained parts of the partial frame and
//      adjusting the buffer that contained the start of the partial
//      frame and the buffer that contained the end of the partial frame.
//      Obviously, if the frame being tested for partiality (<- new word?)
//      was contained in a single buffer, no work needs to be done.
//	
//  Parameters:
//	
//	Xil_unsigned8* frame_boundary_ptr: pointer to byte beyond end of frame
//
//  Returns:
//	
//	XIL_SUCCESS if no errors occurred - else XIL_FAILURE
//
//  Deficiencies/ToDo:
//
//      Part of this routine determines the size of the partial frame
//      so as to allocate only that size of buffer. We could just
//      create a buffer of max_frame_size and not worry about wasted
//      space.
//
//------------------------------------------------------------------------

XilStatus
XilCisBufferManager::foundNextFrameBoundary(Xil_unsigned8* frame_boundary_ptr)
{
  int init_id;
  int type;
  
  // save the location of the frame boundary. This may be used by get bits
  next_frame_ptr = frame_boundary_ptr;
  
  // if the frame just processed was in just one buffer,
  // then no work is required in collapsing buffers
  
  if (pf_buffer == r_buffer) {
    if (pf_need_EOF==TRUE) {
       // we must establish frame boundary for this frame!
       init_id = r_buffer->getRFrameId();
       type = r_buffer->getRFrameType();
       r_buffer->incrNumDecompressedFrames(frame_boundary_ptr,type);
       r_buffer->seekTo(init_id);
    }
    return XIL_SUCCESS;
  }

  // we need to collapse buffers
  
  // ------ determine the size of the new buffer that is needed ----------
  
  // initialize number of bytes to zero and buffers to read position
  int num_bytes = 0;
  XiliCisBufferLListPositionType pos = r_buffer_pos;
  XilCisBuffer* buf = r_buffer;
  
  // by making a call to numAvailBytes() we get the number
  // of bytes from the start of the read frame to the end of
  // the buffer.
  
  num_bytes += buf->numAvailBytes();
  
  // until we reach the partial frame buffer position, all
  // bytes in successive frames are required:
  
  for (pos=buffer_list->next(pos);
       pos != pf_buffer_pos && pos!=buffer_list->end();
       pos=buffer_list->next(pos)){
    
    buf = buffer_list->retrieve(pos);
    num_bytes += buf->getNumBytes();
  }
  
  // we should now be at the partial frame buffer position
  // otherwise some kind of internal error occurred:
  
  
  if (pos == buffer_list->end()){
    
    // Frame boundary not at partial frame position
    // Internal error.
    
    XIL_ERROR( NULL, XIL_ERROR_SYSTEM,"di-95",FALSE);
    return XIL_FAILURE;
  }
  
  // only bytes up to the pf_ptr are valid:
  buf = pf_buffer;
  pf_ptr = frame_boundary_ptr;
  num_bytes += pf_ptr - buf->getBuffer();
  
  // we now know the number of bytes we need
  
  // -------------- create a buffer of the needed size -------------------
  
  // create a buffer of this size and 1 frame
  XilCisBuffer* new_buffer; 
  XiliCisBufferLListPositionType new_buffer_pos;   
  new_buffer = new XilCisBuffer(num_bytes,1);
  
  // check object creation
  if (! new_buffer->isOK()) {
    XIL_ERROR( NULL, XIL_ERROR_RESOURCE,"di-277",FALSE);
    return XIL_FAILURE;
  }
  
  // -------------------- copy the frame into it -------------------------
  
  pos = r_buffer_pos;
  buf = r_buffer;
  Xil_unsigned8* r_ptr = r_buffer->getRfptr();
  
  num_bytes = buf->numAvailBytes();
  
  new_buffer->addBytes(r_ptr, num_bytes);
  
  for (pos=buffer_list->next(pos);
       pos != pf_buffer_pos && pos!=buffer_list->end();
       pos=buffer_list->next(pos)){
    
    buf = buffer_list->retrieve(pos);
    buf->reRead();
    num_bytes = buf->getNumBytes();
    r_ptr = buf->getRfptr();
    new_buffer->addBytes(r_ptr, num_bytes);
  }
  
  buf = buffer_list->retrieve(pos);
  buf->reRead();
  num_bytes = pf_ptr - buf->getBuffer();
  r_ptr = buf->getRfptr();
  
  new_buffer->addBytes(r_ptr, num_bytes);
  
  // -------------------- collapse buffer list -------------------------
  
  // insert new buffer into buffer list 
  new_buffer_pos = buffer_list->insertAfter(new_buffer, r_buffer_pos);
  
  // adjust the end of the read buffer (its write pointers)
  
  int last_frame_id;
  
  if ((last_frame_id = r_buffer->adjustEnd(r_buffer->getRfptr())) < 0 ){
    // an internal error occurred
    XIL_ERROR( NULL, XIL_ERROR_SYSTEM,"di-95",FALSE);
    return XIL_FAILURE;
  }
  
  // check to see if all data removed out of read buffer
  if (last_frame_id == r_buffer->getStartFrameId()){
    
    // if so, we need to delete this buffer
    XilCisBuffer* del_buf = buffer_list->remove(r_buffer_pos);
    
    // check to see if read buffer also start buffer
    if (r_buffer == s_buffer){
      // make start buffers equal to new buffer
      s_buffer = new_buffer;
      s_buffer_pos = new_buffer_pos;
    }
    
    // now delete old read buffer, decr count of buffer with partial frame
    delete del_buf;
    
  } else {
    // clear the partial frame flag on the old read buffer
    r_buffer->setPartialFrame(FALSE);
    buffers_with_partial_frames--;
  }
  
  // set r_buffer to new buffer
  r_buffer = new_buffer;
  r_buffer_pos = new_buffer_pos;
  
  new_buffer->incrNumCompressedFrames(last_frame_id,1);
  last_frame_id++;
  
  // remove and free buffers from new read buffer to partial frame buffer
  
  pos=buffer_list->next(r_buffer_pos);
  while (pos != pf_buffer_pos && pos!=buffer_list->end()) {
    XiliCisBufferLListPositionType save_pos;   

    // before delete save following position
    // (don't try to access buffer_list->next(pos) after "pos" removed!!!
    save_pos = buffer_list->next(pos);
    buf = buffer_list->remove(pos);
    delete buf;
    pos = save_pos;
  }
  
  // we should now be at the buffer containing the last section of
  // the partial frame. We need to adjust this buffer's start.
  
  int status = pf_buffer->adjustStart(frame_boundary_ptr,last_frame_id);
  
  // If the adjust routine returns a 1, then this buffer is no
  // longer needed and can therefore be removed. If the adjust routine
  // returns a -1, an error occurred in adjustment.
  
  if (status == 1){
    // if the buffer to be deleted is the w_buffer (which means
    // that the all data in the write buffer is part of the
    // partial frame and thus has moved into the newly created
    // buffer ), then we must set the write buffer to the newly
    // created buffer.
    if (pf_buffer_pos == w_buffer_pos){

      // set w_buffer to new buffer
      w_buffer = new_buffer;
      w_buffer_pos = new_buffer_pos;

      // we now know the write_frame_id
      write_frame_id = w_buffer->getRFrameId() + 1;
      
    } else {
      // it wasn't the write buffer. We still need to set
      // the id of the buffer following the newly created one.
      buf = buffer_list->retrieve(buffer_list->next(pf_buffer_pos));
      buf->adjustStartFrameId(last_frame_id);
    }
    // now delete last buffer
    buf = buffer_list->remove(pf_buffer_pos);
    delete buf;
  } else if (status == -1)
    // an error occurred
    return XIL_FAILURE;
  
  return XIL_SUCCESS;  
}

//------------------------------------------------------------------------
//
//  Function:	XilCisBufferManager::isolateUnresolvedPartialFrame()
//  Created:	93/01/22
//
//  Description:
//	
//    This routine is meant to be called when handlePartialFrame has failed
//      to resove the partial Frame, but other valid frames are present in
//      the current read buffer.  We need to move the unresolved partial
//      frame into its own buffer so that the valid frames in the read buffer
//      are accessible to getBitsPtr() and numberOfFrames() routines.
//      The pf_ members are setup during the device compressions's 
//      findNextFrameBoundary() routine, and used by this routine to move 
//      from the read_buffer to the last byte associated with the partial 
//      frame.
//
//      The partial frame may only be in one buffer,  pf_buffer = r_buffer,
//      or the partial frame may be spread out over several buffers, 
//      pf_buffer != r_buffer.  In either case, this routine should create 
//      a new buffer, copy the segments of the partial frame to this single 
//      buffer, and adjust the read buffer for the removal of the partial 
//      frame.  Then insert the new buffer in the buffer list, and remove 
//      any buffers which contained only partial frame segments.  Mark the
//      read buffer with partial = FALSE, and the new buffer with 
//      partial = TRUE.
//
//	
//  Parameters:
//	(none--uses the pf_ members set up by device compression's 
//       findNextFrameBoundary().)
//
//  Returns:
//	
//	XIL_SUCCESS if no errors occurred - else XIL_FAILURE
//
//  Deficiencies/ToDo:
//
//
//------------------------------------------------------------------------

int 
XilCisBufferManager::isolateUnresolvedPartialFrame()
{
  
  // save the location of the frame boundary. This may be used by get bits
  next_frame_ptr = pf_end_ptr;
  
  // we need to move partial frame into its own buffer
  
  // ------ determine the size of the new buffer that is needed ----------
  
  // initialize number of bytes to zero and buffers to read position
  int num_bytes = 0;
  XiliCisBufferLListPositionType pos = r_buffer_pos;
  XilCisBuffer* buf = r_buffer;
  
  // by making a call to numAvailBytes() we get the number
  // of bytes from the start of the read frame to the end of
  // the buffer.
  
  num_bytes += buf->numAvailBytes();
  
  // if partial frame spread over multiple buffers,
  // until we reach the partial frame buffer position, all
  // bytes in successive frames are required:
  
  if (pf_buffer != r_buffer) {

    for (pos=buffer_list->next(pos);
       pos != pf_buffer_pos && pos!=buffer_list->end();
       pos=buffer_list->next(pos)){
    
      buf = buffer_list->retrieve(pos);
      num_bytes += buf->getNumBytes();
    }
  
    // we should now be at the partial frame buffer position
    // otherwise some kind of internal error occurred:
  
  
    if (pos == buffer_list->end()){
    
      // Frame boundary not at partial frame position
      // Internal error.
    
      XIL_ERROR( NULL, XIL_ERROR_SYSTEM,"di-95",FALSE);
      return XIL_FAILURE;
    }
  
    buf = buffer_list->retrieve(pos);
    num_bytes += buf->getNumBytes();
  }

  // we now know the number of bytes we need
  
  // -------------- create a buffer of the needed size -------------------
  
  // create a buffer of this size, approx nframes = 1
  XilCisBuffer* new_buffer; 
  XiliCisBufferLListPositionType new_buffer_pos;   
  new_buffer = new XilCisBuffer(num_bytes,1);
  
  // check object creation
  if (! new_buffer->isOK()) {
    XIL_ERROR( NULL, XIL_ERROR_RESOURCE,"di-277",FALSE);
    return XIL_FAILURE;
  }
  
  // -------------------- copy the frame into it -------------------------
  
  pos = r_buffer_pos;
  buf = r_buffer;
  Xil_unsigned8* r_ptr = r_buffer->getRfptr();
  
  num_bytes = buf->numAvailBytes();
  
  new_buffer->addBytes(r_ptr, num_bytes);
  
  if (pf_buffer != r_buffer) { 
    for (pos=buffer_list->next(pos);
       pos != pf_buffer_pos && pos!=buffer_list->end();
       pos=buffer_list->next(pos)){
    
      buf = buffer_list->retrieve(pos);
      buf->reRead();
      num_bytes = buf->getNumBytes();
      r_ptr = buf->getRfptr();
      new_buffer->addBytes(r_ptr, num_bytes);
    }
  
    buf = buffer_list->retrieve(pos);
    buf->reRead();
    num_bytes =  buf->getNumBytes();
    r_ptr = buf->getRfptr();
    new_buffer->addBytes(r_ptr, num_bytes);
  }

  // -------------------- collapse buffer list -------------------------
  
  // insert new buffer into buffer list 
  new_buffer_pos = buffer_list->insertAfter(new_buffer, r_buffer_pos);
  
  // adjust the end of the read buffer (its write pointers)
  
  int last_frame_id;
  
  if ((last_frame_id = r_buffer->adjustEnd(r_buffer->getRfptr())) < 0 ){
    // an internal error occurred
    XIL_ERROR( NULL, XIL_ERROR_SYSTEM,"di-95",FALSE);
    return XIL_FAILURE;
  }
  
  // check to see if all data removed out of read buffer
  if (last_frame_id == r_buffer->getStartFrameId()){
    
    // an internal error occurred, should still be valid read buffer frames
    XIL_ERROR( NULL, XIL_ERROR_SYSTEM,"di-95",FALSE);
    return XIL_FAILURE;

  }

  // clear partial frame flag on the old read buffer
  r_buffer->setPartialFrame(FALSE);

  // set partial frame flag on the new buffer, set unknown num_frames
  new_buffer->setPartialFrame(TRUE);
  new_buffer->setNumFrames(XIL_CIS_UNKNOWN_NUMBER_OF_FRAMES);
  
  // create frame info node for partial frame data
  new_buffer->incrNumCompressedFrames(last_frame_id,XIL_CIS_UNKNOWN_NUMBER_OF_FRAMES);

  if (w_buffer == pf_buffer) {
    // update w_ members to new buffer
    w_buffer = new_buffer;
    w_buffer_pos = new_buffer_pos;
     
  }

  if (r_buffer != pf_buffer) {
    // remove and free buffers from the new buffer to pf_buffer
    // (copied into new buffer already)
     
    pos=buffer_list->next(new_buffer_pos);
    while (pos != pf_buffer_pos && pos!=buffer_list->end()) {
       XiliCisBufferLListPositionType save_pos;   
       
       // before delete save following position
       // (don't try to access buffer_list->next(pos) after "pos" removed!!!
       save_pos = buffer_list->next(pos);
       buf = buffer_list->remove(pos);
       delete buf;
       pos = save_pos;
       
    }
    // includes removal of "pf_buffer_pos" in buffer_list
    // since this now copied into new buffer
    buf = buffer_list->remove(pos);
    delete buf;
  }  

  // set r_buffer to new buffer
  r_buffer = new_buffer;
  r_buffer_pos = new_buffer_pos;
  
  return XIL_SUCCESS;  
}

//------------------------------------------------------------------------
//
//  Function:	XilCisBufferManager::handlePartialFrame()
//  Created:	92/08/20
//
//  Description:
//	
//    This routine is used to handle the possibilty of having a partial
//      frame in the read buffer. When done, any partial frame that
//      may have existed in the read buffer is moved out into it's
//      own buffer.
//	
//  Returns:
//	
//	XIL_SUCCESS if no errors occurred else XIL_FAILURE
//
//------------------------------------------------------------------------

int 
XilCisBufferManager::handlePartialFrame()
{
  // the read buffer may have a partial frame at its end.
  // We need to resolve this issue and move partial frame
  // into its own buffer. This is done by using the routine
  // findNextFrameBoundary of the device compressor.
  
  // save information about the current position
  XilCisBuffer* init_read_buffer = r_buffer;
  XiliCisBufferLListPositionType init_read_buffer_pos = r_buffer_pos;
  int init_read_frame_id = r_buffer->getRFrameId();
  
  // start findNextFrameBoundary process: we need to continue
  // to find the nextFrameBoundary until the boundary lies
  // outside of the initial read buffer.
  
  setupPartialFramePtrs();
  next_frame_ptr = NULL;
  
  while (pf_buffer == r_buffer && next_frame_ptr != pf_end_ptr){

    finding_next_frame_boundary = TRUE;

    int status = device_compression->findNextFrameBoundary();

    CHECK_STATUS_MAYBE(status);

    if ( status == XIL_FAILURE){
      // check to see if any "hidden" complete frames were found 
      if (init_read_frame_id == r_buffer->getRFrameId()) {
        // no other frames, could not resolve this partial frame
        finding_next_frame_boundary = FALSE;
        return XIL_FAILURE;
      }
      else {
        // isolate this unresolved partial frame from complete frames
        // by moving partial frame from read buffer into new buffer
        if (isolateUnresolvedPartialFrame() == XIL_FAILURE) {
          finding_next_frame_boundary = FALSE;
          return XIL_FAILURE;
        }
      }
    }
    finding_next_frame_boundary = FALSE;
    
    // all bytes may not have been processed, so adjust this value
    pf_bytes_to_process = next_frame_ptr - pf_ptr;
    
    // if we are still in our initial read buffer, no collapse
    // has occured yet.
    
    if (pf_buffer == r_buffer) 
      
      // we have basically found a frame boundary within the initial
      // read buffer that may not have been known before. So we can 
      // pretend to have decompressed this frame. This advances the 
      // read pointer in the read buffer. When we are done with this 
      // while loop, we will seek back to our initial position.
      
      decompressedFrame(next_frame_ptr,XIL_CIS_DEFAULT_FRAME_TYPE,NULL,FALSE);
    
  }
  
  // we have left the above loop because either the read buffer
  // has changed (which means a partial frame existed at the
  // end of the buffer and a collapse occurred) or because a
  // frame boundary did exist at the end of the buffer.
  
  if (pf_buffer != r_buffer){

    // the read buffer changed. We need to reset it if
    // the initial read buffer still exist. This will not be
    // so if the initial read buffer had only a partial frame
    // in it. In that case, all data would have been moved out
    // of it and the buffer would have been deleted.

    if (buffer_list->previous(r_buffer_pos) == init_read_buffer_pos){
      r_buffer = init_read_buffer;
      r_buffer_pos = init_read_buffer_pos;
    }
  }

  // At this point,  partial frame does not exist in the read buffer.
  // Thus clear the partial frame flag on the read buffer, if needed

  if (r_buffer->hasPartialFrame() == TRUE) {
    r_buffer->setPartialFrame(FALSE);
    buffers_with_partial_frames--;
  }

  // seek back to original position
  r_buffer->seekTo(init_read_frame_id);
  
  return XIL_SUCCESS; 
}

//------------------------------------------------------------------------
//
//  Function:	XilCisBufferManager::adjustStart(int framenumber, int type)
//  Created:	92/08/20
//
//  Description:
//	
//    This routine is used to adjust the start frame within the buffer
//      lists. Since the new desired start frame may not be able to be
//      processed without addition previous frame information, this
//      routine can also take as a parameter a frame type. Any frames from
//      the desired frame back to a frame of the given type will be kept
//      within the cis although they may not be accessed. They are only
//      used to process the new start frame (and frames that may follow
//      the new start frame). The variable s_buffer_frame_offset is the
//      number of frames from the frame of the required type to the desired
//      start frame. Once the adjustment has been made, any buffers prior
//      to the new start buffer are destroyed. Any frame data within the
//      new start buffer but prior to the "type'ed" frame becomes inaccessible.
//	
//  Parameters:
//	
//	int framenumber:  the desired new start frame
//      int type:         the type of frame that must be kept in order to
//                        guarantee decompression of the desired new start
//                        frame.
//
//  Returns:
//	
//	XIL_SUCCESS if no error occurr else XIL_FAILURE
//	
//------------------------------------------------------------------------
int 
XilCisBufferManager::adjustStart(int framenumber, int type)
{
  int start_adjustment;
  
  // if no data in buffer, cannot adjust start!
  if (r_buffer == NULL)
    return XIL_FAILURE;

  // save information about the current position
  int init_read_frame_id = r_buffer->getRFrameId();
  int new_start_frame_id;
  int save_next_decompress_frame;

  // save next_decompress_frame, because its ANY_TYPE may interefere with start frame type
  save_next_decompress_frame = next_decompress_frame;
  next_decompress_frame = s_buffer->getStartFrameId();
 
  // seek to desired frame and type
  start_adjustment = seek(framenumber,type);
  
  if (start_adjustment >= 0){
    
    // The value of start adjustment is the number of frames
    // from the desired frame to a frame of desired type.
    // The read frame should be the frame of the desired type.
    // The read buffer should be the buffer in which this frame
    // exist.
    
    // All previous buffers may be deleted.
    
    XiliCisBufferLListPositionType prev_pos;
    XilCisBuffer* buf;
    
    while (r_buffer_pos != buffer_list->start()){
      prev_pos = buffer_list->previous(r_buffer_pos);
      buf = buffer_list->remove(prev_pos);
      delete buf;
    }
    
    // The start buffer is now the r_buffer
    s_buffer = r_buffer;
    s_buffer_pos = r_buffer_pos;
    
    // the 'real' start frame id is the id of the first frame
    // in the start buffer plus the start_adjustment
    
    s_buffer_frame_offset = start_adjustment;
    
    // We need to adjust the start of the allocated buffer
    // space in the read buffer to the read frame position.
    new_start_frame_id = s_buffer->adjustStartToRFrame();

    // restore next_decompress_frame
    // if next_decompress_frame less than new start, update
    // (for device compressions without interframe encoding)
    if (save_next_decompress_frame <= new_start_frame_id)
      next_decompress_frame = new_start_frame_id;
    else
      next_decompress_frame = save_next_decompress_frame;
      
    // reset the read frame to original position 
    if (init_read_frame_id > new_start_frame_id)
      seek(init_read_frame_id,XIL_CIS_ANY_FRAME_TYPE);

  } else {
    // an error occurred in seek
    XIL_ERROR(NULL,XIL_ERROR_SYSTEM,"di-184",TRUE);
    return XIL_FAILURE;
  }
  
  
  return XIL_SUCCESS;
}

//------------------------------------------------------------------------
//
//  Function:	XilCisBufferManager::ungetBytes(Xil_unsigned8* curr_ptr, 
//                                              int nbytes)
//  Created:	93/04/01
//
//  Description:
//	This routine is used during partial frame handling. It is called
//	when a device compression has read past the current frame_boundary
//      and needs to backup.  The buffer manager must check for a buffer
//      transition and update the pf_buffer and pf_ptr correctly.  This
//      routine is required for bitstreams which have no end of frame markers;
//      the device compression has read ahead to the next start of frame
//      to determine the frame boundary.
//	
//  Parameters:
//	
//	Xil_unsigned8* curr_ptr:  pointer to current byte (may not be
//              equal to the pf_ptr if used the getNextBytes interface).
//
//	int nbytes:  the number of bytes from curr_ptr to the last byte 
//              of the frame.  So if the bitstream was in one chunk, 
//              (curr_ptr-nbytes) would point to the last byte of the frame.
//  Returns:
//	pointer to the frame boundary (one past the last byte of frame)
//      or NULL if error occurred.
//	
//  Notes:
//	Updates the pf_ptr and pf_buffer to ensure partial frame handling.
//	
//	The variable pf_bytes_to_process is set to 0 if pf_ptr is reset.
//	
//------------------------------------------------------------------------

Xil_unsigned8* 
XilCisBufferManager::ungetBytes(Xil_unsigned8* curr_ptr,
                                               int nbytes)
{
  int delta;

  // sanity check--make sure within current pf_buffer
  if (curr_ptr< pf_end_ptr) {

    // check to see if backup by nbytes stays within same buffer
    delta = (curr_ptr - pf_buffer->getBuffer());
    if (delta >= nbytes) {
      // last byte of frame within same pf_buffer, no special action
      pf_bytes_to_process = 0;
      pf_ptr = curr_ptr-nbytes;
      return(pf_ptr+1);
    }
    else {
      // changing pf_buffer to previous buffer
      delta = (nbytes-delta);

      // Attempt to backup to the prev buffer. if successful, return
      // ptr to frame_boundary (one past the last byte of frame),
      // else return NULL.
    
      if (pf_buffer_pos != s_buffer_pos) {
        pf_buffer_pos = buffer_list->previous(pf_buffer_pos);
        pf_buffer = buffer_list->retrieve(pf_buffer_pos);
      
        pf_end_ptr = pf_buffer->getWptr();
      
        // make sure there are bytes in this buffer
      
        if (pf_buffer->getNumBytes() <= 0){
          return NULL;
        } else {
          // backup from end by delta bytes
          pf_bytes_to_process = 0;
          pf_ptr = pf_end_ptr - delta;
          return (pf_ptr+1);
        }
      } else {
      
        // no previous buffers
        return NULL;
      }
    }
  }
  else {
     // curr_ptr not within current pf_buffer
     return NULL;
  }


}
//------------------------------------------------------------------------
//
//  Function: void XilCisBufferManager::byteError(Xil_unsigned8* bptr);
//  Created:	92/09/18
//
//  Description:
//
//     This routine should be called when a decompressor finds a
//       bit-stream error during decompression or findNextFrameBoundary.
//       The decompressor should send the byte pointer of the place where
//       the error occured.  This should set up the partial frame information
//       such that the next call to getNextByte will give the error handler
//       a byte ptr just after the error.
//
//  Parameters:
//	
//	Xil_unsigned8* bptr: the current pointer that the decompressor
//                          found the error.
//
//------------------------------------------------------------------------
void 
XilCisBufferManager::byteError(Xil_unsigned8* bptr)
{
  // set up state required by byte retrieval so that
  // next call to getNextByte or getNextBytes will
  // return a pointer to the next byte in the stream.
  
  if (finding_next_frame_boundary){

    // an error occurred during findNextFrameBoundary
    pf_bytes_to_process =  (bptr - pf_ptr) + 1;
    
  } else {

    // an error occurred decompression
    pf_ptr = bptr;
    pf_end_ptr = r_buffer->getWptr();
    pf_buffer = r_buffer;          
    pf_buffer_pos = r_buffer_pos;
    pf_bytes_to_process = 1;
    next_frame_ptr = NULL;
  }

  // set the id of the frame the error occurred on
  recovery_frame_id = r_buffer->getRFrameId();
}


//------------------------------------------------------------------------
//
//  Function: void XilCisBufferManager::nextKnownFrameBoundary(
//                                      Xil_unsigned8* cptr,
//                                      Xil_unsigned8** fptr, int* num_frames);
//  Created:	92/09/18
//
//  Description:
//
//     This routine returns the pointer to the next known frame boundary
//       to the given cptr - a pointer within the current buffer
//       being processed through the getNextByte routines.
//
//       It also returns the number of frames that lie between the current
//       position and that known frame boundary.  NULL is returned if
//       there is no known frame boundary.
//
//  Parameters:
//	Xil_unsigned8* cptr: current pointer 
//	Xil_unsigned8** ptr: return the pointer to the next known
//                           frame boundary
//
//      int* num_frames:     return the number of frames that lie between
//                           the current position and the frame boundary 
//                           (including the current frame)
//
//------------------------------------------------------------------------
void 
XilCisBufferManager::nextKnownFrameBoundary(Xil_unsigned8* cptr, Xil_unsigned8** fptr, int* num_frames)
{
  XilCisBuffer* buf;
  XiliCisBufferLListPositionType pos;

  *fptr = NULL;
  *num_frames = 0;

  pos = pf_buffer_pos;
  buf = pf_buffer;

  while (pos != buffer_list->end() &&
         !buf->frameBoundaryAfter(cptr,fptr,num_frames)){
    pos = buffer_list->next(pos);
    buf = buffer_list->retrieve(pos);
    if (pos != buffer_list->end())
      cptr = buf->getBuffer() - 1;
  }

  if (pos == buffer_list->end()){
    *fptr = NULL;
    *num_frames = 0;
  }
}


//------------------------------------------------------------------------
//
//  Function: void XilCisBufferManager::errorRecoveryDone(
//                         Xil_unsigned8* ptr, int num_frames, Xil_boolean fixed);
//  Created:	92/09/18
//
//  Description:
//
//     This routine is called just before xil_cis_attempt_recovery completes.
//     It is sent the current pointer, the number of frames that have been
//     parsed, and whether or not the recovery was successful.
//
//  Parameters:
//	
//	Xil_unsigned8* ptr:  the current data pointer
//
//      int num_frames:      the number of frames that have been
//                           processed in this call to xil_cis_attempt_recovery.
//
//      Xil_boolean fixed:   whether or not the recovery was successful
//                           If fixed, the RFrameId should be equal to
//                           the newly discovered valid frame
//
//------------------------------------------------------------------------
void 
XilCisBufferManager::errorRecoveryDone(Xil_unsigned8* fptr, int num_frames, Xil_boolean fixed)
{
  if (!fixed){
    
    // set up pf pointers for getNextByte routines
    // (i.e. next call to the routines will return
    //  byte just after fptr).
    
    pf_bytes_to_process =  (fptr - pf_ptr) + 1;
    
  } else {

    // recovery was a success. From the initial read position
    // to fptr represents num_frames (this may be a guess).
    // Thus the frame id of frame that starts at fptr should
    // be the rframe_id + num_frames.

    int recovery_frame = r_buffer->getRFrameId() + num_frames;
    
    // Case A:
    // It is possible that, due to put_pits of unknown or
    // partial frames, that the frame id associated with
    // fptr is unknown. In this case we set its frame id
    // to rframe_id + num_frames.

    // Case B:
    // It is possible that, knowing the frame id associated
    // with fptr, it is less than  rframe_id + num_frames.
    // In this case, we adjust it to its new value.

    // Case C:
    // It is possible that, knowing the frame id associated
    // with fptr, it is greater than  rframe_id + num_frames.
    // In this case, we are marking this as an internal error.
    // This could indicate that the error recovery module
    // went further than the user allowed it to.
    // It is the responsibility of the errorRecovery module
    // of a compressor to make sure that this doesn\'t happen

    // In all cases, the read buffer is set to the pf_buffer.

    r_buffer = pf_buffer;
    r_buffer_pos = pf_buffer_pos;

    if (r_buffer->recoveryFrame(fptr,recovery_frame) == XIL_FAILURE){
      // CASE C: error message:
      XIL_ERROR( NULL, XIL_ERROR_SYSTEM, "di-95", TRUE);
    }
  }
}


//------------------------------------------------------------------------
//
//  Function:	int XilCisBufferManager::nextSeek(int framenumber, int type)
//  Created:	92/09/18
//
//  Description:
//	
//	Determines the closest frame >= framenumber of given type.
//      return -1 if no such seekable frame exists.
//
//      The default for type is XIL_CIS_ANY_FRAME_TYPE which
//      matches any frame type.
//
//  Parameters:
//	
//	int framenumber:   frame to start from
//      int type:          type of frame to look for
//
//  Returns:
//	
//	int:               non-negative values means that the
//			     return value is the closest frame that
//                           is seekable.
//
//                         negative values reflect that no frame 
//			     was found
//
//  Notes:
//
//      See class header file doc for more information
//
//      Note that an error handler may use nextSeek to find a frame
//      to seek to, do a seek to that frame, and then expect to use
//      getNextByte to get values the sought frame.
//
//------------------------------------------------------------------------

int 
XilCisBufferManager::nextSeek(int framenumber, int type)
{
  XilCisBuffer* buf;
  XiliCisBufferLListPositionType pos;
  int frame_id;

  // find the buffer that contains frame frame number

  frame_id = r_buffer->getStartFrameId();
  
  pos = (frame_id != XIL_CIS_UNKNOWN_FRAME_ID &&
         framenumber >= frame_id) ? r_buffer_pos : s_buffer_pos;
  
  while (pos != buffer_list->end() && (frame_id = buffer_list->retrieve(pos)->getStartFrameId()) != XIL_CIS_UNKNOWN_FRAME_ID && framenumber >= frame_id)
    pos = buffer_list->next(pos);

  // frame framenumber should be in previous buffer
  frame_id = -1;
  
  if (pos != s_buffer_pos)
    pos = buffer_list->previous(pos);
    
  buf = buffer_list->retrieve(pos);

  // now look forward in buffer list for desired frame
  while (pos !=  buffer_list->end() &&
         (frame_id = buf->nextPossibleFrame(framenumber,type)) < 0){
    pos=buffer_list->next(pos);
    buf = buffer_list->retrieve(pos);
  }
  
  return frame_id;
}

//------------------------------------------------------------------------
//
//  Function:	int XilCisBufferManager::prevSeek(int framenumber, int type)
//  Created:	92/09/18
//
//  Description:
//	
//	Determines the closest frame <= framenumber of given type.
//      return -1 if no such seekable frame exists.
//
//      The default for type is XIL_CIS_ANY_FRAME_TYPE which
//      matches any frame type.
//
//  Parameters:
//	
//	int framenumber:   frame to start from
//      int type:          type of frame to look for
//
//  Returns:
//	
//	int:               non-negative values means that the
//			     return value is the closest frame that
//                           is seekable.
//
//                         negative values reflect that no frame 
//			     was found
//
//  Notes:
//
//      See class header file doc for more information
//
//      Note that an error handler may use nextSeek to find a frame
//      to seek to, do a seek to that frame, and then expect to use
//      getNextByte to get values the sought frame.
//
//------------------------------------------------------------------------

int 
XilCisBufferManager::prevSeek(int framenumber, int type)
{
  XilCisBuffer* buf;
  XiliCisBufferLListPositionType pos;
  int frame_id;
  
  // find the buffer that contains frame frame number
  frame_id = r_buffer->getStartFrameId();
  
  pos = (frame_id != XIL_CIS_UNKNOWN_FRAME_ID &&
         framenumber <= frame_id) ? r_buffer_pos : w_buffer_pos;
  
  while (pos != buffer_list->start() && (frame_id = buffer_list->retrieve(pos)->getStartFrameId()) != XIL_CIS_UNKNOWN_FRAME_ID && framenumber < frame_id)
    pos = buffer_list->previous(pos);

  // frame framenumber should in current buffer
  frame_id = -1;
  buf = buffer_list->retrieve(pos);

  // now look backward in buffer list for desired frame
  while ((frame_id = buf->prevPossibleFrame(framenumber,type)) < 0){
    if (pos ==  buffer_list->start())
      return -1;
    pos=buffer_list->previous(pos);
    buf = buffer_list->retrieve(pos);
  }
  
 return frame_id;

}




//------------------------------------------------------------------------
//
//  Function: void XilCisBufferManager::foundFrameDuringRecovery(
//                         Xil_unsigned8* fptr)
//  Created:	92/09/18
//
//  Description:
//
//
//  Parameters:
//	
//	Xil_unsigned8* fptr: pointer to the start of a frame
//
//------------------------------------------------------------------------
void 
XilCisBufferManager::foundFrameDuringRecovery(Xil_unsigned8* fptr)
{
  recovery_frame_id++;
  
  pf_buffer->foundFrameDuringRecovery(fptr,recovery_frame_id);
}
  

//------------------------------------------------------------------------
//
//  Function:	Xil_unsigned8* getNumBytesToFrame(int end_id, int* nbytes)
//  Created:	93/04/19
//
//  Description:
//	This routine is called when the device compression has determined that
//      only a portion of the current read buffer can be provided to the user.
//	Determine num bytes in buffer from current read frame to frame end_id,
//      inclusive, and return the starting address of this byte block.
//	
//  Returns:
//	ptr to the start of the byte block in this range
//
//  NOTE:
//      End must be in the current cis r_buffer.
//
//------------------------------------------------------------------------

Xil_unsigned8* XilCisBufferManager::getNumBytesToFrame(int end_id,int* nbytes)
{
  // check to see if CIS not empty
  if (r_buffer != NULL){
    // set nbytes, nframes and buf
    Xil_unsigned8* buf = r_buffer->getNumBytesToFrame(end_id, nbytes);
    
    // update r_ members if read entire buffer...
    if (r_buffer->frameAtRfptr()==FALSE) {
      if (buffer_list->next(r_buffer_pos) != buffer_list->end()){
        r_buffer_pos = buffer_list->next(r_buffer_pos);
        r_buffer = buffer_list->retrieve(r_buffer_pos);
      
        // reset the next read buffer
        r_buffer->reRead();
      } 
    }
    return buf;
    
  } else{
    // CIS is empty
    *nbytes = 0;
    return NULL;
  }
}



//------------------------------------------------------------------------
//
//  Function:	Xil_unsigned8* moveEndStartOneBuffer();
//  Created:	93/04/19
//
//  Description:
//	This routine is called when the device compression has determined that
//      the end frame of the current buffer must be moved into the same buffer
//	as the start frame of the next buffer.  These frames must have their
//      frame boundaries established, ie neither can be an incomplete
//      (partial) frame.
//
//  Returns:
//	status: XIL_SUCCESS or XIL_FAILURE
//
//  NOTE:
//      Assumed that the current read frame is the "end" of the current buffer.
//      The first frame of the next buffer in the buffer list is the "start".
//      The "end" and "start" frames will be excised from their respective 
//      buffers and placed in a newly created buffer of the proper size, which
//      is then correctly positioned in the buffer list.  The read frame will
//      be restored to the "end" frame_id, which is the first frame of the
//      new buffer.
//------------------------------------------------------------------------

int 
XilCisBufferManager::moveEndStartOneBuffer()
{
  int last_frame_id;
  int buf1_id,buf1_size,buf1_type;
  int buf2_id,buf2_size,buf2_type;
  XilCisBuffer *buf1,*buf2;
  XiliCisBufferLListPositionType buf1_pos, buf2_pos;
  void *buf1_user_ptr,*buf2_user_ptr;
  Xil_unsigned8 *buf1_ptr,*buf2_ptr;
  
  // setup location of the frame boundary in pf_ptr. 

  buf1_id = r_buffer->getRFrameId();
  buf1 = r_buffer;
  buf1_pos = r_buffer_pos;

  buf1_ptr = r_buffer->getRfptr();
  buf1_size = r_buffer->getNumBytesInRFrame();
  buf1_user_ptr = r_buffer->getRFrameUserPtr();
  r_buffer->setRFrameUserPtr(0);
  buf1_type = r_buffer->getRFrameType();

  last_frame_id = r_buffer->getStartFrameId() + r_buffer->getNumFrames() - 1;
  if (buf1_id != last_frame_id) {
    // incorrect end frame position--must be at end of current buffer
    XIL_ERROR( NULL, XIL_ERROR_RESOURCE,"di-338",TRUE);
    return XIL_FAILURE;
  }

  if (buffer_list->next(r_buffer_pos) == buffer_list->end()) {
    // incorrect buffer position--no "next" buffer for start frame
    XIL_ERROR( NULL, XIL_ERROR_RESOURCE,"di-339",TRUE);
    return XIL_FAILURE;
  }
  
  r_buffer_pos = buffer_list->next(r_buffer_pos);
  r_buffer = buffer_list->retrieve(r_buffer_pos);
  r_buffer->reRead();

  buf2 = r_buffer;
  buf2_pos = r_buffer_pos;

  buf2_id = r_buffer->getStartFrameId();
  if (buf2_id != buf1_id+1) {
    // incorrect start frame position--
    // must be sequential to end frame of previous buffer
    XIL_ERROR( NULL, XIL_ERROR_RESOURCE,"di-95",TRUE);
    return XIL_FAILURE;
  }

  buf2_ptr = r_buffer->getRfptr();
  buf2_size = r_buffer->getNumBytesInRFrame();
  buf2_user_ptr = r_buffer->getRFrameUserPtr();
  r_buffer->setRFrameUserPtr(0);
  buf2_type = r_buffer->getRFrameType();


  // -------------- create a buffer of the needed size -------------------
  
  // create a buffer of this size and 2 frames
  XilCisBuffer* new_buffer; 
  XiliCisBufferLListPositionType new_buffer_pos;   
  new_buffer = new XilCisBuffer((buf1_size+buf2_size),2);
  
  // check object creation
  if (! new_buffer->isOK()) {
    XIL_ERROR( NULL, XIL_ERROR_RESOURCE,"di-277",FALSE);
    return XIL_FAILURE;
  }
  
  // -------------------- copy the frames into it -------------------------

  
  new_buffer->addBytes(buf1_ptr, buf1_size);
  new_buffer->incrNumCompressedFrames(buf1_id,1,buf1_type);
    
  new_buffer->addBytes(buf2_ptr, buf2_size);
  new_buffer->incrNumCompressedFrames((buf2_id),1,buf2_type);

  new_buffer->seekTo(buf1_id);
  new_buffer->setRFrameUserPtr(buf1_user_ptr);
  new_buffer->seekTo(buf2_id);
  new_buffer->setRFrameUserPtr(buf2_user_ptr);

  // -------------------- collapse buffer list -------------------------
  
  // insert new buffer into buffer list 
  new_buffer_pos = buffer_list->insertAfter(new_buffer, buf1_pos);
  
  // check buffer containing I/P frame
  // adjust the end of the read buffer (its write pointers)
  
  buf1->seekTo(buf1_id);

  if ((last_frame_id = buf1->adjustEnd(buf1->getRfptr())) < 0 ){
    // an internal error occurred
    XIL_ERROR( NULL, XIL_ERROR_SYSTEM,"di-95",FALSE);
    return XIL_FAILURE;
  }
  
  // check for only frame in buffer
  if (last_frame_id == buf1->getStartFrameId()){
    
    // if so, we need to delete this buffer
    XilCisBuffer* del_buf = buffer_list->remove(buf1_pos);
    
    // check to see if read buffer also start buffer
    if (buf1 == s_buffer){
      // make start buffers equal to new buffer
      s_buffer = new_buffer;
      s_buffer_pos = new_buffer_pos;
    }
    
    // now delete old read buffer, decr count of buffer with partial frame
    delete del_buf;
    
  }
  
  // setup read buffer to newly created buffer since it now contains
  // the original read frame, buf1_id
  r_buffer = new_buffer;
  r_buffer_pos = new_buffer_pos;
  r_buffer->reRead();

  // we should now be at the buffer containing the B frame
  // We need to adjust this buffer's start.
  
  int status = buf2->removeStartFrame();
  
  // If the adjust routine returns a 1, then this buffer is no
  // longer needed and can therefore be removed. If the adjust routine
  // returns a -1, an error occurred in adjustment.
  
  if (status == 1){
    // if the buffer to be deleted is the w_buffer (which means
    // that the all data in the write buffer has moved into the newly created
    // buffer ), then we must set the write buffer to the newly
    // created buffer.
    if (buf2_pos == w_buffer_pos){

      // set w_buffer to new buffer
      w_buffer = new_buffer;
      w_buffer_pos = new_buffer_pos;

    }

    // now delete last buffer
    XilCisBuffer* del_buf = buffer_list->remove(buf2_pos);
    delete del_buf;

  } else if (status == -1)
    // an error occurred
    return XIL_FAILURE;

    
  return XIL_SUCCESS;  
}






//------------------------------------------------------------------------
//
//  Function:	int  addToLastFrame(Xil_unsigned8* ptr, int nbytes)
//  Created:	93/05/26
//
//  Description:
//	This routine is called when the device compression has nbytes of data
//      at "data" which must be added to the last frame in the cis.  This
//      routine will add the bytes to the last frame (which is defined
//      as the frame prior to the write frame), updating its number
//      of bytes and the buffer's wptr/wfptr.  The buffer manager
//      assumes that any extra bytes which are added to the frame have
//      been included in the estimate for max_frame_size, which is
//      essential for the correct determination of frame boundaries.
//
//  Input:
//	Xil_unsigned8* data:   start of data to be added to the last frame
//      int         nbytes:   num of bytes of data
//
//  Returns:
//	status: XIL_SUCCESS or XIL_FAILURE
//
//  NOTE:
//      As stated above, the max_frame_size must include any extra bytes
//      which may be added in this manner to ensure frame boundaries
//      are found correctly by the decompressor.
//
//------------------------------------------------------------------------

int XilCisBufferManager::addToLastFrame(Xil_unsigned8* data, int nbytes)
{
   // check if there is room in the current buffer to add bytes
   if (w_buffer) {
        if (write_frame_id != 0) {
           if (w_buffer->getWptr() == w_buffer->getWfptr()) {
             // copy nbytes into buffer at wptr.
             // max_frame_size must include the extra bytes; if this is true,
             // then the buffer had enough bytes for the last frame to be 
             // this large.  If we attempt to add nbytes to the buffer and 
             // there is not room, then we were given an inaccurate 
             // max_frame_size.
             if ((nbytes + w_buffer->getNumBytes()) > 
                 w_buffer->getBufferSize()){
                 // Attempted write beyond allocated buffer space
                 XIL_ERROR( NULL, XIL_ERROR_USER, "di-104", TRUE);
                 return XIL_FAILURE;
             }
             w_buffer->addBytes(data, nbytes);
             // update buffer wfptr and frame info node num_bytes
             if (w_buffer->updateLastFrame() == XIL_FAILURE) {
               return XIL_FAILURE;
             }
          }
          else {
            // write frame incomplete, cannot add bytes until complete
            XIL_ERROR(NULL,XIL_ERROR_SYSTEM,"di-114",TRUE);
            return XIL_FAILURE;
          }
        }
        else {
          // write frame = 0, buffer empty
          XIL_ERROR(NULL,XIL_ERROR_SYSTEM,"di-107",TRUE);
          return XIL_FAILURE;
        }
   }
   else {
       // no write buffer, empty cis
       XIL_ERROR(NULL,XIL_ERROR_SYSTEM,"di-107",TRUE);
       return XIL_FAILURE;
   }

   return XIL_SUCCESS;
}

void
XilCisBufferManager::setXilDeviceCompression(XilDeviceCompression* dc)
{
    device_compression = dc;
}

XilDeviceCompression*
XilCisBufferManager::getXilDeviceCompression()
{
    return device_compression;
}

int   
XilCisBufferManager::getNumFrames()           const    
{ 
    return write_frame_id; 
}

int   
XilCisBufferManager::getFrameSize()           const    
{ 
    return max_frame_size; 
}

int   
XilCisBufferManager::getNumFramesPerBuffer()  const    
{ 
    return num_frames_per_buffer;
}

int   
XilCisBufferManager::getWFrameId()            const    
{ 
    return write_frame_id; 
}

int   
XilCisBufferManager::getNextDecompressId()    const    
{ 
    return next_decompress_frame;
}

void  
XilCisBufferManager::setNumFramesPerBuffer(int nfpb)
{ 
    num_frames_per_buffer = nfpb; 
}


void 
XilCisBufferManager::setSeekToStartFrameFlag(Xil_boolean value) 
{
    seek_to_start_frame_flag = value;
}


Xil_unsigned8* 
XilCisBufferManager::getRBuffer()       
{ 
    return ((Xil_unsigned8 *) r_buffer); 
}


