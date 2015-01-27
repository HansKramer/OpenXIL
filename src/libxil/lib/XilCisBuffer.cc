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
//  File:	XilCisBuffer.cc
//  Project:	XIL
//  Revision:	1.15
//  Last Mod:	10:08:59, 03/10/00
//
//  Description:
//		
//	Implementation of XilCisBuffer Object
//	
//------------------------------------------------------------------------
//
//	Copyright (c) 1992, 1993, 1994, by Sun Microsystems, Inc.
//
//------------------------------------------------------------------------

#pragma ident	"@(#)XilCisBuffer.cc	1.15\t00/03/10  "


#include "_XilDefines.h"
#include "_XilCisBuffer.hh"
#include "_XilSystemState.hh"
#include "XiliUtils.hh"

//------------------------------------------------------------------------
//
//  Function:	XilCisBuffer::XilCisBuffer(unsigned int buf_size,
//                                         int approx_nframes)
//  Created:	92/04/14
//
//  Description:
//	
//	XilCisBuffer constructor: sets the buffer size to the parameter
//        buf_size and then creates the buffer. Since this instance
//        created the buffer, it then maintains control over it and
//        thus the setting of the control flag to true.
//
//        Initially:
//          - the write buffer pointer points to the start of the buffer
//               as does the write frame and read frame pointers. 
//	    - the number of frames contained in the buffer is zero
//          - zero bytes have been added to the buffer
//          - the identifier for the start frame is unknown
//          - there exist no data, therfore the frame_list is
//               empty and the read frame position is -1
//
//      The parameter approx_nframes is used to set the the size of
//      the frame list. Since this is an array list which must grow if
//      the size gets bigger than expected (which means a list copy) its
//      best to get this size as close if not larger than the expected
//      maximum number of frames in this buffer.
//
//  Parameters:
//	
//	unsigned int buf_size:  size of buffer
//	int approx_nframes: size to make frame list
//
//------------------------------------------------------------------------

XilCisBuffer::XilCisBuffer(unsigned int buf_size,
                           int          approx_nframes)
{
  isOKFlag = FALSE;

  buffer_size = buf_size;
  control_buffer = 1;
  num_frames = 0;
  start_frame_id = XIL_CIS_UNKNOWN_FRAME_ID;
  partial_frame = FALSE;
  wptr = wfptr = rfptr = buffer = NULL;
  frame_list = NULL;  

  // for an array list, positions are positive integers (from 0). Thus
  // an invalid position would be any negative number. -1 was chosen.
  
  rfpos = -1;
  

  buffer_alloc = buffer = new Xil_unsigned8[buffer_size];

  
  if (buffer == NULL) {
    XIL_ERROR( NULL, XIL_ERROR_RESOURCE,"di-1",TRUE);
    return;
  }

  wptr = wfptr = rfptr = buffer;

  // since approx_nframes is only an approximation, approx_nframes/2
  // was chosen as a grow factor for the array frame list.
  // TODO: make sure growth factor never "0" (check)!  
  frame_list = new XiliFrameInfoAList(approx_nframes,approx_nframes/2);

  
  // check memory alloc for object
  if (frame_list == NULL) { 
    XIL_ERROR( NULL, XIL_ERROR_RESOURCE,"di-1",TRUE); 
    return; 
  }
  // check object creation
  frame_list = frame_list->ok();
  if (frame_list == NULL) {
    // di-277:  Could not create Object
    XIL_ERROR(NULL, XIL_ERROR_RESOURCE, "di-277", FALSE);
    return;
  }

  isOKFlag = TRUE;
}

//------------------------------------------------------------------------
//
//  Function:	XilCisBuffer::XilCisBuffer(unsigned nbytes, int nframes,
//                                         Xil_unsigned8* buf, int frame_id,
//                                         XIL_FUNCPTR_DONE_WITH_DATA done_data
//                                         int approx_nframes = 0);
//
//  Created:	92/04/14
//
//  Description:
//	
//	XilCisBuffer constructor: sets the buffer size to the parameter
//        buf_size and then sets buffer to point to the supplied buffer.
//        Since this instance did not created the buffer, maintenance
//        control is determines by the control flag: default false.
//
//        Initially:
//
//          - the write buffer pointer and the write frame
//               pointer are NULL
//          - the read frame pointer points to the start of the buffer
//	    - the number of frames contained in the buffer is nframes
//          - nbytes bytes will be in the buffer
//          - the identifier for the start frame is unknown
//          - there is one chunk of data in the buffer representing
//               nframes frames. Thus a single XiliFrameInfo object is
//               created that reflects this and the read frame pos
//               is set to this single position in the frame list.
//
//  Parameters:
//
//      Xil_unsigned8* buf:     pointer to a buffer of bytes
//      int int nframes:        num frames in supplied buffer
//	unsigned int nbytes:    num bytes in supplied buffer
//      int frame_id:           start frame id
//      int approx_nframes:     if the number of frames is not known,
//                               this is an approximatiion
//      XIL_FUNCPTR_DONE_WITH_DATA done_data: function to call upon deletion
//
//------------------------------------------------------------------------

XilCisBuffer::XilCisBuffer(unsigned nbytes,int nframes,Xil_unsigned8* buf,int frame_id, XIL_FUNCPTR_DONE_WITH_DATA done_data, int approx_nframes)
{
  isOKFlag = FALSE;

  buffer_size = nbytes;
  buffer_alloc = buffer = buf;
  control_buffer = 0;
  partial_frame = FALSE;
  num_frames = 0;

  // set pointers to initial states
  rfptr = buffer;
  wptr = wfptr = buffer + nbytes;
  frame_list = NULL;

  // set start frame id
  start_frame_id = frame_id;

  // set call back function
  done_with_data = done_data;

  rfpos = -1;	// init list position

  // here we may know the exact number of frames in this buffer
  // and thus can set the size of the frame list exactly. If
  // we don't known the number of frames, we use an approx.

  if (nframes > 0) {
    frame_list = new XiliFrameInfoAList(nframes);

    // check memory alloc for object
    if (frame_list == NULL) {
      XIL_ERROR( NULL, XIL_ERROR_RESOURCE,"di-1",TRUE);
      return;    
    }
    // check object creation
    frame_list = frame_list->ok();
    if (frame_list == NULL) {
      XIL_ERROR( NULL, XIL_ERROR_RESOURCE,"di-277",FALSE);
      return;    
    }

    num_frames = nframes;
  } else {
    // TODO: make sure this growth factor is never "0" (check!)
    frame_list = new XiliFrameInfoAList(approx_nframes,approx_nframes/2);

    // check memory alloc for object
    if (frame_list == NULL) {
      XIL_ERROR( NULL, XIL_ERROR_RESOURCE,"di-1",TRUE);
      return;    
    }
    // check object creation
    frame_list = frame_list->ok();
    if (frame_list == NULL) {
      XIL_ERROR( NULL, XIL_ERROR_RESOURCE,"di-277",FALSE);
      return;    
    }

    num_frames = XIL_CIS_UNKNOWN_NUMBER_OF_FRAMES;
  }

  // create new frame info object for data
  XiliFrameInfo* fi = new XiliFrameInfo(rfptr,getNumBytes(),num_frames,frame_id,0);

  // check memory  alloc for object
  if (fi == NULL) { 
    XIL_ERROR( NULL, XIL_ERROR_RESOURCE,"di-1",TRUE); 
    return;     
  } 

  // cannot fail append since initial frame_list has size > 0 
  rfpos = frame_list->append(fi);

  isOKFlag = TRUE;
}

//------------------------------------------------------------------------
//
//  Function:	XilCisBuffer::isOK()
//  Created:	92/10/26
//
//  Description:
//
//     	Ensures the creation of the XilCisBuffer was successful.
//
//  Returns:
//
// 	TRUE: if object creation succeeded.
//	FALSE: otherwise.
//
//------------------------------------------------------------------------

Xil_boolean
XilCisBuffer::isOK()
{
    _XIL_ISOK_TEST();
}


//------------------------------------------------------------------------
//
//  Function:	XilCisBuffer::~XilCisBuffer()
//  Created:	92/04/14
//
//  Description:
//	
//    XilCisBuffer destructor: if the XilCisBuffer has control over its buffer,
//     then it deletes it. The buffer and frame pointers are then set to
//     NULL and the control flag set to false (in case of dangling references).
//	
//  Notes:
//
//     Is (possible) deletion of buffer enough or does the rest of the
//     destructor body also make sense.
//	
//------------------------------------------------------------------------

XilCisBuffer::~XilCisBuffer()
{
  if (control_buffer)
    delete buffer_alloc;
  else if (done_with_data != NULL)
    (*done_with_data)( (void*) buffer_alloc);
  
  wptr = wfptr = rfptr = buffer = buffer_alloc = NULL;
  control_buffer = 0;

  // delete frame info objects
  if (frame_list)
    frame_list->deletePtrElements();

  // delete frame list
  delete frame_list;
  frame_list = NULL;
}

//------------------------------------------------------------------------
//
//  Function:	void XilCisBuffer::addByte(int val)
//  Created:	92/04/14
//
//  Description:
//
//      Checks to see if an overflow condition exist. If not, then
//      the incoming byte is placed into the buffer. Otherwise, an
//      error message is issed and the routine returns.
//	
//  Parameters:
//	
//       int b:  byte value to be added
//	
//------------------------------------------------------------------------

void XilCisBuffer::addByte(int b)
{
    if(buffer_size == (unsigned int ) getNumBytes()) {
        // Attempted write beyond allocated buffer space
        XIL_ERROR( NULL, XIL_ERROR_USER, "di-104", TRUE);
        return; 
    }
  
    *wptr++ = b;
}


//------------------------------------------------------------------------
//
//  Function:	XilCisBuffer::addBytes(Xil_unsigned8* b, unsigned n_bytes)
//  Created:	92/04/14
//
//  Description:
//
//      Checks to see if an overflow condition would exist if n_bytes
//      of data were added to the buffer. If not, then the incoming bytes
//      are placed into the buffer via memcpy. Otherwise, no data is
//      added and an error message is issed and the routine returns.
//	
//  Parameters:
//	
//       int* b:           pointer to the buffer of bytes to be added
//	 unsigned n_bytes: number of bytes in supplied buffer b
//
//------------------------------------------------------------------------

void XilCisBuffer::addBytes(Xil_unsigned8* b, unsigned n_bytes)
{
  if (n_bytes + getNumBytes() > buffer_size){

    // Attempted write beyond allocated buffer space
    XIL_ERROR( NULL, XIL_ERROR_USER, "di-104", TRUE);
    return;
  }

  xili_memcpy(wptr,b,n_bytes);
  wptr += n_bytes;
}

//------------------------------------------------------------------------
//
//  Function:	void XilCisBuffer::copyFrames(unsigned nbytes,int nframes,
//                                         Xil_unsigned8* data, int frame_id)
//  Created:	92/04/22
//
//  Description:
//	
//    This routine attempts to copy the buffer (data) which contains
//      nbytes bytes and nframes frames into the buffer at the write
//      pointer. If there is not enough room, an error message will
//      be issued. If there is room but an incomplete frame exist
//      at the write pointer (i.e. the buffer does not end with a
//      complete frame) then an error message will be issued.
//
//    If neither of the error conditions exist, then the data
//      supplied will be copied into the end of the buffer. The
//      write pointer, the number of bytes in the buffer, the
//      number of frames and the number of bytes in all previous
//      frames will be incremented accordingly.
//
//      A new Frame Info object will be created for this new chunk
//      of data and will be appended to the frame list. After it
//      has been appended to the list, the wfptr is set to point
//      to the end of the data in the buffer.
//	
//  Parameters:
//	
//	unsigned nbytes:      number of bytes in data buffer
//      int nframes:          number of frames in data buffer
//      Xil_unsigned8* data:  buffer of frames
//      int frame_id:         starting frame id for this group
//
//------------------------------------------------------------------------

void XilCisBuffer::copyFrames(unsigned nbytes, int nframes, Xil_unsigned8* data, int frame_id)
{
  
  if (nbytes + getNumBytes() > buffer_size){
 
    // Attempted write beyond allocated buffer space
    // ToDo: also could tell user what n_bytes and buffer_size was.
 
    XIL_ERROR( NULL, XIL_ERROR_USER, "di-104", TRUE); 
    return; 
  }

  if (wfptr != wptr){

    // Incomplete frame in buffer, cannot copy until complete
    XIL_ERROR( NULL, XIL_ERROR_USER, "di-114", TRUE); 
    return; 
  }

  // copy in the data and adjust byte count and write pointer
  xili_memcpy(wptr,data,nbytes);
  wptr += nbytes;

  // partial frame also means unknown number of frames
  if (nframes == XIL_CIS_PARTIAL_FRAME)
    nframes = XIL_CIS_UNKNOWN_NUMBER_OF_FRAMES;

  // increment number of frames if known
  if (num_frames != XIL_CIS_UNKNOWN_NUMBER_OF_FRAMES)  
    if (nframes > 0)
      num_frames += nframes;
    else 
      num_frames = XIL_CIS_UNKNOWN_NUMBER_OF_FRAMES;

  // create new frame info object for data being added
  XiliFrameInfo* fi = new XiliFrameInfo(wfptr, nbytes, nframes, frame_id, 0);

  
  if (fi == NULL) { 
    XIL_ERROR( NULL, XIL_ERROR_RESOURCE,"di-1",TRUE); 
    return;     
  } 

  // if the rfpos is not -1, then there are already frames in the buffer
  // so just append the new frame info object to the list of frames. If,
  // however, rfpos is -1 then this implies that no frames have yet been
  // added to the buffer - Since this is the first set of frames, set 
  // rfpos and also set the start_frame_id variable.
  
  if (rfpos >= 0) {
    if (frame_list->append(fi) == -1){
      // failure to append to list
      XIL_ERROR( NULL, XIL_ERROR_RESOURCE,"di-1",FALSE);
      return;
    }
  }
  else {
    // cannot fail append since initial frame list has size > 0 
    rfpos = frame_list->append(fi);
    start_frame_id = frame_id;
  }

  wfptr = wptr;  
  
}

//------------------------------------------------------------------------
//
//  Function:	XilCisBuffer::incrNumCompressedFrames(int frame_id, int nframes, int type)
//  Created:	92/04/21
//
//  Description:
//	
//    This routine is called after a number of frames have been added 
//      to the XilCisBuffer. The routine determines the number of bytes that
//      were just added to the buffer for the frame. It then creates
//      a new XiliFrameInfo object, initializing its start pointer to
//      the start of the frame just written (wfptr), the number of
//      bytes and the number of frames.  Usually this routine is called
//      after 1 complete frame is added to the buffer, but it is also
//      called from isolateUnresolvedPartialFrame(), in which case the 
//      nframes is UNKNOWN.
//
//    This XiliFrameInfo object is then appended to the XilCisBuffer's
//      frame list. The buffer's frame pointer is then set to be
//      at the current write pointer. Finally, the number of
//      frames contained in this buffer is incremented by nframes and
//      the number of bytes in the previous frames (which now includes
//      the frames just compressed) is set to the total number of bytes
//      in the buffer.
//
//  Parameters:
//
//   int frame_id:    integer id of frame just compressed
//   int nframes:     number of frames.  Normally 1 frame compressed at a time,
//                    but may be called from isolateUnresolvedPartialFrame().
//
//
//  Return:
//
//   num_bytes_in_frame :    number of bytes in compressed frame.  If unable
//			     to add frame info node for this new frame, return "0".
//
//------------------------------------------------------------------------

int XilCisBuffer::incrNumCompressedFrames(int frame_id, int nframes, int type)
{
  int num_bytes_in_frame = wptr - wfptr;
  
  XiliFrameInfo* fi = new XiliFrameInfo(wfptr, num_bytes_in_frame, nframes, frame_id, type);

  
  if (fi == NULL) { 
    XIL_ERROR( NULL, XIL_ERROR_RESOURCE,"di-1",TRUE);
    return 0;      
  }


  // if the rfpos is not -1, then there are already frames in the buffer
  // so just append the new frame info object to the list of frames. If,
  // however, rfpos is -1 then this implies that no frames have yet been
  // added to the buffer - Since this is the first frame, set rfpos.
  
  if (rfpos >= 0) {
    if (    frame_list->append(fi) == -1){
      // failure to append to list
      XIL_ERROR( NULL, XIL_ERROR_RESOURCE,"di-1",FALSE);
      return 0;
    }
  }
  else {
    // cannot fail append since initial frame_list has size > 0
    start_frame_id = frame_id;
    rfpos = frame_list->append(fi);
  }
  
  wfptr = wptr;

  if (num_frames != XIL_CIS_UNKNOWN_NUMBER_OF_FRAMES)
    num_frames+= nframes;

  return num_bytes_in_frame;
}

//------------------------------------------------------------------------
//
//  Function:	XilCisBuffer::incrNumDecompressedFrames(Xil_unsigned8* ptr,
//                                                    int type, void* user_ptr)
//  Created:	92/04/21
//
//  Description:
//	
//    This routine is called after a complete frame has been read 
//      from a XilCisBuffer.
//
//      If the number of frames contained in the read frame_info 
//      object (rfinfo), which comes from the read frame position 
//      in the frame list, is greater than 1, then:
//
//        information about the frame just decompressed is separated
//        out from this Frame Info object and placed into its own
//        Frame Info object. The previous frame information object
//        at the read position is then modified (having info removed/
//        subtracted out of it). The new frame info object is stored
//        at the read position in the frame list and the old object
//        is inserted after it. The new read position then becomes
//        the new position of the old object. Finally, the read
//        pointer is set appropriately.
//
//      Else:
//
//        there being only one frame in frame info object, the
//        read frame position is set to the next position in
//        the frame list. If this equated to a valid frame
//        data section, then the read frame pointer is set
//        to the first byte in this next frame. Otherwise it
//        is set to the wfptr indicating being at the end of
//        the buffer.
//
//------------------------------------------------------------------------

void XilCisBuffer::incrNumDecompressedFrames(Xil_unsigned8* ptr, int type,
                                             void* user_ptr)
{

  if (rfpos >=0) {

    XiliFrameInfo* rfinfo = frame_list->retrieve(rfpos);

    // sanity check assures that the user just 
    // decompressed the read frame of this buffer.

    
    if (rfinfo->getStart() != rfptr){

      // Frame info error. Start addresses do not match
      // Internal error
      XIL_ERROR( NULL, XIL_ERROR_SYSTEM, "di-95", TRUE); 

      return;
    }

    // get numer of frames in segment
    int nframes = rfinfo->getNumFrames();

    // get the frame id of this frame
    int frame_id = rfinfo->getFrameId();
    
    // determine number of bytes of frame just decompressed
    int num_bytes_in_frame = ptr - rfptr;

    // get the type of the frame
    int frame_type = rfinfo->getFrameType();


    // determine if there was more than one
    // frame in the current frame info segment.
    
    if (nframes > 1 || ( nframes == XIL_CIS_UNKNOWN_NUMBER_OF_FRAMES &&
                         rfinfo->getNumBytes() > num_bytes_in_frame)){
      
      // create a new XiliFrameInfo object for decompressed frame

      XiliFrameInfo* fi = new XiliFrameInfo(rfptr, num_bytes_in_frame, 1, frame_id, type, user_ptr);
     
      
      if (fi == NULL) { 
        XIL_ERROR( NULL, XIL_ERROR_RESOURCE,"di-1",TRUE);
        return;      
      }
 
      // update rfinfo object
      
      // set the start point and the number of bytes
      rfinfo->setStart(ptr);
      rfinfo->setNumBytes( rfinfo->getNumBytes() - num_bytes_in_frame);
      
      // if the frame_id was known, reset the frame id for
      // the next chunk to one plus the last frame_id.
      if (frame_id != XIL_CIS_UNKNOWN_FRAME_ID)
        rfinfo->setFrameId( frame_id + 1);
      
      // decrement the number of frames if known
      if (nframes != XIL_CIS_UNKNOWN_NUMBER_OF_FRAMES)
        rfinfo->setNumFrames( rfinfo->getNumFrames() - 1);

      // store new Frame Info at rfpos and insert
      // modified (old rfpos's frame info object)
      // after rfpos. Set rfpos to this new position.
      
      frame_list->store(fi,rfpos);
      rfpos = frame_list->insertAfter(rfinfo,rfpos);
      if (rfpos == -1) {
	// failure to insert in list
	XIL_ERROR( NULL, XIL_ERROR_RESOURCE,"di-1",FALSE);
	return;
      }

      // set rfptr to the start of the next frame
      rfptr = ptr;
      
    } else {

      if (nframes != 1) 
        // there was only frame in the last segment although this
        // wasn't a known fact. Now that we know, make it known.
        rfinfo->setNumFrames(1);

      // there was only one frame in the last buffer section.
      // Increment rfpos and set rfptr appropriately. Also set
      // the frame id if not already known.
      // Also, reset the frame type to the new type

      rfinfo->setFrameType(type);
      if (user_ptr)
        rfinfo->setUserPtr(user_ptr);

      rfpos = frame_list->next(rfpos);
      rfinfo = (rfpos == frame_list->end()) ?
        NULL : frame_list->retrieve(rfpos);
      
      if (rfinfo) {
        rfptr = rfinfo->getStart();
        if (rfinfo->getFrameId() == XIL_CIS_UNKNOWN_FRAME_ID)
          rfinfo->setFrameId(frame_id+1);
      } else {
        // we are at then end of the buffer - thus we now know
        // how many frames are contained in this buffer.
        num_frames = frame_id - start_frame_id + 1;

        // set the read frame pointer to the write frame pointer
        // which indicates that we are at the end of the buffer.
        rfptr = wfptr;
      }
    }
  } else {

    // Current read frame is unknown
    XIL_ERROR( NULL, XIL_ERROR_USER, "di-293", TRUE); 
    return;
  }
}
//------------------------------------------------------------------------
//
//  Function:	XilCisBuffer::frameAtRfptr() const
//  Created:	92/04/22
//
//  Description:
//	
//    Determines if there is a complete frame at the read
//      frame pointer. If the read frame pointer (rfptr) is
//      NULL, there is no data. If the pointer is equal to
//      the same location as the write frame pointer (wfptr)
//      then, even though data may exist for the current frame,
//      this frame is not yet complete. In this case, it is
//      counted as there being no data available. If neither
//      of the two previous conditions exists, then there is
//      data at the read frame pointer.
//	
//	
//  Returns:
//
//    int:   TRUE if data, FALSE otherwise (see above)
//	
//------------------------------------------------------------------------
int XilCisBuffer::frameAtRfptr() const
{
  return ((rfptr == wfptr) || (rfptr == NULL)) ? FALSE : TRUE;
}

//------------------------------------------------------------------------
//
//  Function:	XilCisBuffer::frameAfterRfptr(int max_frame_size,
//                                            Xil_boolean need_EOF) const
//  Created:	92/04/22
//
//  Description:
//	
//    Determines if there is a frame after the read frame pointer. 
//
//  Parameters:
//
//   int max_frame_size:    maximum size in bytes a frame can be
//   Xil_boolean need_EOF:  need to know end of frame, canot rely on max_frame_size
//
//  Returns:
//
//    int:   TRUE if data, FALSE otherwise (see above)
//	
//------------------------------------------------------------------------
int XilCisBuffer::frameAfterRfptr(int max_frame_size, Xil_boolean need_EOF) const
{
  // if there is a node on the frame list for a frame
  // past the read frame position, the read frame
  // does lie completely within this buffer.
  
  if (frame_list->next(rfpos) != frame_list->end())
    return TRUE;

  // if there are at least max_frame_size bytes in buffer beyond
  // the read frame pointer, then a complete frame must be here
  
  if (max_frame_size && (rfptr + max_frame_size <= wfptr) && (need_EOF==FALSE))
    return TRUE;
  
  return FALSE;
}

//------------------------------------------------------------------------
//
//  Function:	int XilCisBuffer::numAvailBytes() const
//  Created:	92/05/27
//
//  Description:
//	
//	Determine the number bytes from the read position to
//      the end of the buffer (inclusive of any incomplete frame
//      at the write position).
//	
//------------------------------------------------------------------------

int XilCisBuffer::numAvailBytes() const
{
  // if rfpos is equal to -1, then this means that
  // no frames have been added to this buffer yet
  
  if (rfpos == -1)
    return 0;

  // if rfpos is at end of frame list, the number of bytes is equal to
  // the number of bytes between the write pointer and the write frame
  // pointer
  
  if (rfpos == frame_list->end())
    return (wptr - wfptr);

  // here, rfpos is valid somewhere in the middle of the frame list.
  // Thus the number of bytes is equal to the write pointer minus the
  // address of the start of the read frame.

  return (wptr - frame_list->retrieve(rfpos)->getStart());

}

//------------------------------------------------------------------------
//
//  Function:	Xil_unsigned8* XilCisBuffer::getAvailData(int* nb, int* nf)
//  Created:	92/05/27
//
//  Description:
//	
//	Returns a pointer to the avail data from the read position up
//      to the write frame (does not include data in write space).
//	
//  Parameters:
//	
//	int* nb:   modified to be number of bytes in returned segment
//      int* nf:   modified to be number of frames in returned segment
//
//  Returns:
//	
//      Xil_unsigned8*: pointer to data
//
//  Side Effects:
//	
//	parameters are modified (read frame moved up to write frame position)
//
//------------------------------------------------------------------------

Xil_unsigned8* XilCisBuffer::getAvailData(int* nb, int* nf)
{

  *nb = 0;
  *nf = 0;
  Xil_unsigned8* buf = NULL;

  // if rfpos is equal to -1, then this means that
  // no frames have been added to this buffer yet
  
  if (rfpos == -1)
    return NULL;
  
  XiliFrameInfoAListPositionType fipos = rfpos;
  XiliFrameInfo* fi = frame_list->retrieve(fipos);

  if (fipos != frame_list->end()){

    buf = fi->getStart();

    while (fipos != frame_list->end()){
      *nb = *nb + fi->getNumBytes();
      *nf = *nf + fi->getNumFrames();    
      fipos = frame_list->next(fipos);
      fi = frame_list->retrieve(fipos);
    }

    rfpos = fipos;
    rfptr = wfptr;
  }

  return buf;
}

//------------------------------------------------------------------------
//
//  Function:	Xil_unsigned8* XilCisBuffer::getNumBytesInRFrame()
//  Created:	93/04/19
//
//  Description:
//	
//	Returns num bytes in the current read frame, used after a frame boundary
//      found to establish end of frame pointer = rfptr + num_bytes_in_read_frame.
//	
//  Parameters:
//	
//  Returns:
//	
//      int: number of bytes in read frame info node.
//           if node does not exist, return 0.
//
//------------------------------------------------------------------------

int XilCisBuffer::getNumBytesInRFrame() const
{
  int nb;


  // if rfpos is equal to -1, then this means that
  // no frames have been added to this buffer yet
  
  nb = 0;

  if (rfpos == -1)
    return NULL;
  
  if (rfpos != frame_list->end()) {

    XiliFrameInfo* fi = frame_list->retrieve(rfpos);

    if (fi != NULL) {

      nb = fi->getNumBytes();
    }
  }

  return nb;
}

//------------------------------------------------------------------------
//
//  Function:	Xil_unsigned8* XilCisBuffer::getNumBytesToFrame(int end_id, int* nb);
//  Created:	93/04/19
//
//  Description:
//	
//	Returns a pointer to the byte block starting at curr read frame and ending
//      at end_id.
//	
//  Parameters:
//	
//	int* nb:   modified to be number of bytes between read position and end_id,
//                 inclusive.
//
//  Returns:
//	
//      Xil_unsigned8*: pointer to beginning of byte block
//  NOTE:
//      end frame_id must be in this buffer!!!
//      read position updated to end frame_id or last frame in this buffer
//------------------------------------------------------------------------

Xil_unsigned8* XilCisBuffer::getNumBytesToFrame(int end_id, int* nb)
{

  *nb = 0;
  Xil_unsigned8* buf = NULL;

  // if rfpos is equal to -1, then this means that
  // no frames have been added to this buffer yet
  
  if (rfpos == -1)
    return NULL;
  
  XiliFrameInfoAListPositionType fipos = rfpos;

  XiliFrameInfo* fi = frame_list->retrieve(fipos);

  if (fipos != frame_list->end()){

    buf = fi->getStart();
    
    while ((fipos != frame_list->end()) && (fi->getFrameId() <= end_id)){
      *nb = *nb + fi->getNumBytes();
      fipos = frame_list->next(fipos);
      fi = frame_list->retrieve(fipos);
    }

    rfpos = fipos;
    if (fipos == frame_list->end())
      rfptr = wfptr;
    else
      rfptr = fi->getStart();

  }

  return buf;
}
//------------------------------------------------------------------------
//
//  Function:	void XilCisBuffer::reRead()
//  Created:	92/05/27
//
//  Description:
//	
//	This routine resets the read frame pointer rfptr to the
//      beginning of the buffer. The read frame position is set
//      to the start position of the frame information list.
//	
//  Side Effects:
//	
//	rfptr and rfpos are modified.
//
//------------------------------------------------------------------------

void XilCisBuffer::reRead()
{
  rfptr = buffer;
  rfpos = frame_list->start();
}

//------------------------------------------------------------------------
//
//  Function:	void XilCisBuffer::unRead()
//  Created:	92/05/27
//
//  Description:
//	
//	This routine sets the read frame pointer rfptr to the
//      wfptr of the buffer. The read frame position is set
//      to the last position of the frame information list.
//	
//  Side Effects:
//	
//	rfptr and rfpos are modified.
//
//------------------------------------------------------------------------

void XilCisBuffer::unRead()
{
  rfptr = wfptr;
  rfpos = frame_list->end();
}


//------------------------------------------------------------------------
//
//  Function:	XilCisBuffer::adjustStart(Xil_unsigned8* fb_ptr, int new_start_id)
//  Created:	92/07/16
//
//  Description:
//	
//	Adjust where the start of the internal/external buffer exist due
//      to partial frame collapsing.
//
//  Parameters:
//	
//	Xil_unsigned8* fb_ptr: new start address: (frame boundary pointer)
//      int new_start_id: the corrected start id for the first frame
//                        in this buffer
//
//  Returns:
//
//      int:  removal flag. If the buffer is empty after adjustment,
//            this flag will let the buffer manager know that this
//            buffer is no longer needed.
//
//  Side Effects:
//	
//	buffer's initial position changed
//
//  Notes:
//
//      This routine should have been called only because a partial
//      frame was distributed over multiple buffers. This buffer
//      contains the last portion of the frame. It would have been
//      an error on the user's end if this buffer is not marked as
//      having a partial frame. 
//
//------------------------------------------------------------------------

int XilCisBuffer::adjustStart(Xil_unsigned8* fb_ptr, int new_start_id)
{
  // sanity check: we should be at first frame in the buffer

  
  if (rfpos != 0){

    // Not at first frame of buffer
    // Partial frame start adjustment error on frame position
    XIL_ERROR( NULL, XIL_ERROR_SYSTEM, "di-290", TRUE); 
    return -1;
  }

  // user sanity check: partial frame flag should be set
  
  
  if (!partial_frame){

    // Partial frame flag must be set for buffer 
    // Partial frame flag not set on buffer with partial frame
    XIL_ERROR( NULL, XIL_ERROR_SYSTEM, "di-291", TRUE); 
    return -1;

  }

  // realign first frame info marker

  XiliFrameInfo* rfinfo = frame_list->retrieve(rfpos);

  if (frame_list->next(rfpos) == frame_list->end()){
    // there is only one segment in this buffer

    // if the frame boundary pointer passed into this routine
    // is at the end of the buffer, then the partial frame
    // completely fills this buffer. Thus, this buffer is no
    // longer needed.

    if (fb_ptr == wptr)
      return 1;

    // adjust the size of the buffer
    buffer_size -= fb_ptr - buffer;

    // adjust frame and buffer pointers
    rfptr = fb_ptr;
    buffer = fb_ptr;

     
    // else other frame(s) exist in this buffer: adjust first frame

    rfinfo->setStart(fb_ptr);
    rfinfo->setNumBytes(getNumBytes());
    rfinfo->setFrameId(new_start_id);
    
    start_frame_id = new_start_id;
    
    return 0;
    
  } else {
    // indicate failure
    // this should never happen since putBits creates a buffer of the exact size
    // for the partial frame segment
    // Partial frame start adjustment error--mismatch with buffer start
    XIL_ERROR( NULL, XIL_ERROR_SYSTEM, "di-292", TRUE); 
     return -1;  
  }

}

//------------------------------------------------------------------------
//
//  Function:	XilCisBuffer::adjustEnd(Xil_unsigned8* fb_ptr)
//  Created:	92/07/16
//
//  Description:
//	
//	Adjust where the end of the internal/external buffer exist due
//      to partial frame collapsing.
//
//  Parameters:
//	
//	Xil_unsigned8* fb_ptr: new end address: (frame boundary pointer)
//
//  Returns:
//
//      int:  the last frame id which is now being moved out of the
//            buffer and into the new collapsed buffer by the buffer
//            manager.
//
//  Side Effects:
//	
//	buffer's end position changed
//
//  Notes:
//
//      the routine expects that the read frame to be at the position
//      of the passed pointer. An internal error results if this is
//      not so.
//
//------------------------------------------------------------------------

int XilCisBuffer::adjustEnd(Xil_unsigned8* fb_ptr)
{
  int last_frame_id;
  
  // sanity check

  
  if (rfpos == -1 || frame_list->retrieve(rfpos)->getStart() != fb_ptr){

    // Partial frame end adjustment error on frame position
    XIL_ERROR( NULL, XIL_ERROR_USER, "di-118", TRUE); 
    return -1;
  }

  // another sanity check

  
  if (frame_list->next(rfpos) != frame_list->end()){

    // Partial frame end adjustment error on frame list 
    XIL_ERROR( NULL, XIL_ERROR_USER, "di-119", TRUE);  
    return -1;
  }
  
  // adjust write and read pointers
  wptr = wfptr = rfptr = fb_ptr;

  // remove the read frame info position, save its id
  // and then delete the object.
  
  XiliFrameInfo* fi = frame_list->remove(rfpos);
  last_frame_id = fi->getFrameId();
  delete fi;

  // set rfpos to the end of the frame list
  rfpos = frame_list->end();

  // set the number of frames in this buffer
  num_frames = last_frame_id - start_frame_id;

  // return the last frame id
  return last_frame_id;
}

//------------------------------------------------------------------------
//
//  Function:	int XilCisBuffer::seekForwardTo(int desired_frameid)
//  Created:	92/07/ 6
//
//  Description:
//	
//	This routine seeks forward in the buffer to a specific frame
//      id.  
//	
//  Parameters:
//	
//	int desired_frameid: the desired frame id to seek to
//
//  Returns:
//	
//      int: number of frames to burn to reach desired frame. If negative,
//           an error occurred during the seek operation.
//
//------------------------------------------------------------------------

int XilCisBuffer::seekForwardTo(int desired_frameid)
{
  int current_frame_id;     
  XiliFrameInfo* rfinfo;
  XiliFrameInfo* finfo;
  int nframes;
  
  // finfo_pos is initialized to the position of the read frame (rfpos)  
  XiliFrameInfoAListPositionType finfo_pos = rfpos;
  
  // determine if we are at the end of the frame list
  if (finfo_pos != frame_list->end()){

    // since we are not at then end of the frame list, we can
    // further traverse this list until either we reach the
    // end or when the desired frame enters into the current
    // frame info segment.
    
    do {

      // get the next frame info segment  and then test to see if
      // desired frame within the segment (and going on to the
      // next segment if not in this one).
      
      finfo = frame_list->retrieve(finfo_pos);
      nframes = finfo->getNumFrames();
      
    } while (nframes != XIL_CIS_UNKNOWN_NUMBER_OF_FRAMES &&
             desired_frameid >= finfo->getFrameId() + nframes &&
             ((finfo_pos = frame_list->next(finfo_pos)) != frame_list->end()));
    
    // set the read frame position to the current value of finfo_pos
    // then retrieve the frame information from this node on the list
    // (setting it to NULL if at the list's end)
    
    rfpos = finfo_pos;
    rfinfo = (rfpos==frame_list->end()) ? NULL : frame_list->retrieve(rfpos);
    
    // if the read frame info object, rfinfo, is valid then set the read
    // frame pointer, rfptr to the info object's start pointer. The id
    // of the current frame is thus this frame info's frame id.
    
    if (rfinfo){
      rfptr = rfinfo->getStart();
      current_frame_id = rfinfo->getFrameId();
    } else {
      
      // rfinfo being invalid means all frames in this buffer have been
      // skipped. Thus set rfptr to the wfptr and the current frame id to
      // the start_frame_id + the number of frames in this buffer.
      
      rfptr = wfptr;
      current_frame_id = start_frame_id + num_frames;
    }
  } else {

    // here we were initially at the end of the buffer, thus
    // the current frame id is equal to the start id plus the
    // number of frames in this buffer.
    
    current_frame_id = start_frame_id + num_frames;
  }

  // the return value is equal to the desired frame id minus
  // the current frame id. Id the two are equal, the value
  // returned is zero - thus no frames need to be burned. If
  // the value is positive, some number of frames will need
  // to be burned. The value should not be negative.
  
  return (desired_frameid - current_frame_id);
}

//------------------------------------------------------------------------
//
//  Function:	int XilCisBuffer::seekBackwardTo(int desired_frameid)
//  Created:	92/07/ 6
//
//  Description:
//	
//	This routine seeks backward in the buffer to a specific frame
//      id.  
//	
//  Parameters:
//	
//	int desired_frameid: the desired frame id to seek to
//
//  Returns:
//	
//      int: number of frames to burn to reach desired frame. If negative,
//           an error occurred during the seek operation.
//
//------------------------------------------------------------------------

int XilCisBuffer::seekBackwardTo(int desired_frameid)
{
  int current_frame_id;
  XiliFrameInfo* rfinfo;
  XiliFrameInfo* finfo;

  // finfo_pos is initialized to the position of the read frame (rfpos)
  XiliFrameInfoAListPositionType finfo_pos = rfpos;

  // if we are at the end of the frame list, back up
  // one segment.

  if (finfo_pos == frame_list->end())
    finfo_pos = frame_list->previous(finfo_pos);

  // if finfo_pos is < 0, we can not seek backwards.
  // (this position being negative means we are in 
  // front of the beginning of the frame list).
 
  
  if (finfo_pos >= 0){
    
    // retrieve the frame info object for this position
    finfo = frame_list->retrieve(finfo_pos);
    
    // get the frame id of the start of this frame segment
    current_frame_id = finfo->getFrameId();
    
    // While the desired frame id is < then the frame id of the
    // current frame info segment and we have not yet reached the
    // beginning of the frame list, go back a frame segment and
    // get the next frame id.
    
    while (desired_frameid < current_frame_id &&
           finfo_pos != frame_list->start()){
      
      finfo_pos = frame_list->previous(finfo_pos);
      finfo = frame_list->retrieve(finfo_pos);
      current_frame_id = finfo->getFrameId();
    }
    
    // set the read frame position to the current value of finfo_pos
    
    rfpos = finfo_pos;
    rfinfo = finfo;

    rfptr = rfinfo->getStart();
    current_frame_id = rfinfo->getFrameId();
    
    // The desired frame id should be in the current segment.
    // The number of frames to burn is equal to the desired
    // frame id minus the frame id of the current frame info
    // segment.
   
    
    if (desired_frameid >= current_frame_id)
      return (desired_frameid - current_frame_id);
    else {

      // Desired framenumber no longer in CIS
      // (Desired frame not in this buffer)

      XIL_ERROR( NULL, XIL_ERROR_USER, "di-106", TRUE); 

      return -1;
    }
  } else {


    // Desired framenumber no longer in CIS
    // (Desired frame not in this buffer)

    XIL_ERROR( NULL, XIL_ERROR_USER, "di-106", TRUE);

    return -1;
  }
}

//------------------------------------------------------------------------
//
//  Function:	int XilCisBuffer::seekTo(int desired_frameid)
//  Created:	92/07/ 6
//
//  Description:
//	
//	Determine which direction is best to seek in and then call
//      the appropriate seek routine.
//	
//  Parameters:
//	
//	int desired_frameid: frame to seek to
//
//  Returns:
//	
//	number of frames to burn to get to desired frame
//
//  Side Effects:
//	
//	during the seek operation, the read frame can change
//
//------------------------------------------------------------------------

int XilCisBuffer::seekTo(int desired_frameid)
{
  int current_frame_id = getRFrameId();

  if (desired_frameid == current_frame_id)
    // we are at the desired frame
    return 0;

  if (desired_frameid == start_frame_id){
    // we want to goto the start of this buffer
    rfptr = buffer;
    rfpos = frame_list->start();
    return 0;
  }

  if (desired_frameid > start_frame_id){

    if (current_frame_id >= 0){
      
      // determine direction of seek

      if (desired_frameid > current_frame_id){

        // desired frame between current frame and last frame
        
        if (desired_frameid <= (current_frame_id + (start_frame_id + 
		num_frames - current_frame_id)/2) || 
		num_frames == XIL_CIS_UNKNOWN_FRAME_ID)
          return seekForwardTo(desired_frameid);
        else {
          rfptr = wfptr;
          rfpos = frame_list->end();
          return seekBackwardTo(desired_frameid);
        }
        
      } else {

        // desired frame between first frame and current frame

        if (desired_frameid >= (start_frame_id + (current_frame_id - start_frame_id)/2))
          return seekBackwardTo(desired_frameid);
        else {
          rfptr = buffer;
          rfpos = frame_list->start();
          return seekForwardTo(desired_frameid);
        }
        
      }
    } else {
      // current_frame_id is negative. Thus an error occurred.
      return -1;
    }

  } else {

    // Seek to framenumber not in CIS 
    // (Desired frame not in this buffer)
 
    XIL_ERROR( NULL, XIL_ERROR_USER, "di-105", TRUE);
 
    return -1;
  }
}

//------------------------------------------------------------------------
//
//  Function:	int      XilCisBuffer::seekBackwardToFrameType(int type)
//  Created:	92/05/27
//
//  Description:
//	
//	This routine seeks backward until it finds a frame of the desired
//      type.
//	
//  Parameters:
//	
//	int type: the desired frame type
//
//  Returns:
//	
//      int: number of frames to burn to get to desired frame type
//           or -1 if error occurred.
//
//------------------------------------------------------------------------

int      XilCisBuffer::seekBackwardToFrameType(int type)
{
  
  int total_nframes = 0;

  XiliFrameInfoAListPositionType fipos;
  XiliFrameInfo* fi;
  
  // check to see if rfpos valid (it should 
  // if buffer contains at least one frame.
  // if rfpos is equal to -1, then this means that
  // no frames have been added to this buffer yet
 
  
  if (rfpos == -1){

    // Current read frame is unknown
    // (No read frame position on seek back to frame type)
    XIL_ERROR( NULL, XIL_ERROR_SYSTEM, "di-293", TRUE);

    return -1;
  }

  // check to see if we are start of buffer list
  
  if (rfpos == frame_list->start()){
    // we are at the first frame in buffer. Thus we can not
    // seek backwards from here. Thus a frame of the desired
    // type can not be found. Return -1.
    return -1;
  }

  // start with the previous frame from the current one
  fipos = frame_list->previous(rfpos);
  fi = frame_list->retrieve(fipos);

  // now traverse the frame list backwards until a frame
  // of the desired type is found. Keep track of the number
  // of frames that we go back past until the frame is
  // found. Do not go back past the first frame (which
  // occurs when fipos becomes -1)

  // get number of frames in this node
  total_nframes += fi->getNumFrames();

  while (fipos != -1 &&
         type != fi->getFrameType() &&
         fi->getFrameType() != XIL_CIS_ANY_FRAME_TYPE){

  
    // advance frame info to previous position
    fipos = frame_list->previous(fipos);

    if (fipos != -1){
      fi = frame_list->retrieve(fipos);
      total_nframes += fi->getNumFrames();
    } else 
      fi = NULL;
  }

  // if fipos is not -1, then a frame of the desired type was found.

  if ( fipos != -1 ){
    
    // The total_nframes represents the number of frames from the
    // desired frame type to the initial read frame position when
    // this routine was called.
    
    // set rfpos to the desired type frame 
    
    rfpos = fipos;
    rfptr = fi->getStart();

    return total_nframes;
    
  } else {

    // Here, fipos was -1 which means a frame of the desired type
    // was not found; thus, return a negative number.

    
    return -1;
  }   
}

//------------------------------------------------------------------------
//
//  Function:	int XilCisBuffer::getRFrameId()
//  Created:	92/05/ 4
//
//  Description:
//	
//	Return the id of the read frame. 
//	
//------------------------------------------------------------------------

int XilCisBuffer::getRFrameId()
{
  // if rfpos is equal to -1, then this means that
  // no frames have been added to this buffer yet

  if (rfpos >= 0){
    XiliFrameInfo* rfinfo = frame_list->retrieve(rfpos);
    if (rfinfo == NULL)
      return start_frame_id + num_frames;
    else
      return rfinfo->getFrameId();
  } else {

    // Current read frame is unknown
    // (Read position does not exist)

    XIL_ERROR( NULL, XIL_ERROR_USER, "di-293", TRUE);

    return -1;
  }
}

//------------------------------------------------------------------------
//
//  Function:	int XilCisBuffer::getRFrameType()
//  Created:	92/05/27
//
//  Description:
//	
//	Return the frame tpye of the read frame
//	
//------------------------------------------------------------------------

int XilCisBuffer::getRFrameType()
{
  // if rfpos is equal to -1, then this means that
  // no frames have been added to this buffer yet
 
  
  if (rfpos >= 0){
    XiliFrameInfo* rfinfo = frame_list->retrieve(rfpos);
    if (rfinfo == NULL)
      // This should occur only if this buffer is the last buffer in
      // the cis buffer manager.
      return -1; // this matches XIL_CIS_ANY_FRAME_TYPE 
    else
      return rfinfo->getFrameType();
  } else {

    // Current read frame is unknown 
    // (Read position does not exist) 
 
    XIL_ERROR( NULL, XIL_ERROR_USER, "di-293", TRUE); 
 
    return -1;   
  }
}

//------------------------------------------------------------------------
//
//  Function:	int* XilCisBuffer::getRFrameTypePtr()
//  Created:	92/07/ 9
//
//  Description:
//	
//	Return a pointer to the frame type of the read frame. This
//      is used by the seek routine of the XilCisBufferManager to
//      temporarily change the type of a frame during seeks.
//
//  NOTE:
//
//	caller expects NULL return value if this buffer is the last
//	buffer in the cis (at the end of the cis, no real frame there).
// 
//------------------------------------------------------------------------

int* XilCisBuffer::getRFrameTypePtr()
{
  // if rfpos is equal to -1, then this means that
  // no frames have been added to this buffer yet
 
  
  if (rfpos >= 0){
    XiliFrameInfo* rfinfo = frame_list->retrieve(rfpos);
    if (rfinfo == NULL)
      // This should occur only if this buffer is the last buffer in
      // the cis buffer manager
      return NULL; // this matches XIL_CIS_ANY_FRAME_TYPE
    else
      return rfinfo->getFrameTypePtr();
  } else {

    // Current read frame is unknown  
    // (Read position does not exist)  
  
    XIL_ERROR( NULL, XIL_ERROR_USER, "di-293", TRUE);  
  
    return NULL;
  }
}

//------------------------------------------------------------------------
//
//  Function:	void* XilCisBuffer::getRFrameUserPtr()
//  Created:	93/03/26
//
//  Description:
//	
//	Return the user ptr stored for the read frame.
//
//  NOTE:
//
//	caller expects NULL return value if no real frame info node
//      established for the read frame (at the end of the cis, for example).
// 
//------------------------------------------------------------------------

void* XilCisBuffer::getRFrameUserPtr()
{
  // if rfpos is equal to -1, then this means that
  // no frames have been added to this buffer yet
 
  
  if (rfpos >= 0){
    XiliFrameInfo* rfinfo = frame_list->retrieve(rfpos);
    if (rfinfo == NULL)
      // This would occur if we have not yet decompressed this frame or
      // we are at the very end of the cis
      return NULL; 
    else
      return rfinfo->getUserPtr();
  } else {

    // Current read frame is unknown  
    // (Read position does not exist)  
  
    XIL_ERROR( NULL, XIL_ERROR_USER, "di-293", TRUE);  
  
    return NULL;
  }
}

//------------------------------------------------------------------------
//
//  Function:	int XilCisBuffer::setRFrameUserPtr(void *uptr)
//  Created:	93/03/26
//
//  Description:
//	
//	Store the user ptr associated with the read frame.
//
//  Parameters:
//       void *uptr: user ptr to store at read frame info node
//
//  Return:
//       success(0), or failure(-1)
// 
//------------------------------------------------------------------------

int XilCisBuffer::setRFrameUserPtr(void* uptr)
{
  // if rfpos is equal to -1, then this means that
  // no frames have been added to this buffer yet
 
  
  if (rfpos >= 0){
    XiliFrameInfo* rfinfo = frame_list->retrieve(rfpos);
    if (rfinfo == NULL) {
  
      // This would occur if there is no frame info node for the frame
      XIL_ERROR( NULL, XIL_ERROR_SYSTEM, "di-95", TRUE);  
      return -1;
    }
    else {
      rfinfo->setUserPtr(uptr);
      return 0;
    }
  } else {

    // Current read frame is unknown  
    // (Read position does not exist)  
  
    XIL_ERROR( NULL, XIL_ERROR_USER, "di-293", TRUE);  
    return -1;
  }
}

//------------------------------------------------------------------------
//
//  Function:	void XilCisBuffer::adjustStartFrameId(int new_start_id)
//  Created:	92/07/17
//
//  Description:
//	
//	Sets the start id of this buffer to a new value. The routine also
//      scans through the frame info nodes in the buffer and readjust
//      frame ids if possible.
//	
//  Parameters:
//	
//	int new_start_id: new frame id for first frame in buffer
//
//  Side Effects:
//	
//      frame ids can be modified	
//	
//------------------------------------------------------------------------

void XilCisBuffer::adjustStartFrameId(int new_start_id)
{
  start_frame_id = new_start_id;

  // go through frame list and adjust frame id's

  int cur_id = new_start_id;
  int nframes;
  int total_nframes = 0;
  XiliFrameInfo* finfo;
  XiliFrameInfoAListPositionType finfo_pos;
  
  for (finfo_pos = frame_list->start();
       finfo_pos != frame_list->end();
       finfo_pos = frame_list->next(finfo_pos)){

    finfo = frame_list->retrieve(finfo_pos);

    finfo->setFrameId(cur_id);

    // get number of frames
    nframes = finfo->getNumFrames();
    
    // increment the current id if possible

    if (nframes != XIL_CIS_UNKNOWN_NUMBER_OF_FRAMES){
      total_nframes += nframes;
      if (cur_id != XIL_CIS_UNKNOWN_FRAME_ID)
        cur_id = cur_id + nframes;
    } else {

      // the number of frames in this node is unknown, thus
      // we don't know how much to increment the frame id
      // count by.
      
      cur_id = XIL_CIS_UNKNOWN_FRAME_ID;
      total_nframes = XIL_CIS_UNKNOWN_NUMBER_OF_FRAMES;
      num_frames = XIL_CIS_UNKNOWN_NUMBER_OF_FRAMES;
    }
  }

  // make sure frame count in this buffer is correct
  if (num_frames != total_nframes);
    num_frames = total_nframes;
  
}

//------------------------------------------------------------------------
//
//  Function:	XilCisBuffer::adjustStartToRFrame()
//  Created:	92/08/24
//
//  Description:
//	
//    This routine is called during the keep/max frame processing. It
//      is used to adjust the start frame of the start buffer. The
//      routine adjusts the start frame to the current position of
//      the read frame. All frames before this position can no longer
//      be accessed and therefore their frame info nodes can be removed.
//	
//  Returns:
//	
//	int: the start frame id after adjustment
//	
//------------------------------------------------------------------------

int XilCisBuffer::adjustStartToRFrame()
{
  Xil_unsigned8 *old_buffer;
  int bytes;
  int frames;

  // the start frame in this buffer should be moved 
  // to the location of the read frame position.

  XiliFrameInfo* rfinfo = frame_list->retrieve(rfpos);

  // re-adjust the start of the frame list. Since this is an
  // array list, we don't want to do multiple removes since
  // each means a shift every element in the array. Thus, the
  // adjustStartTo routine was implemented to do this operation
  // quickly. See the XilGAList implementation for details.
  
  if (rfinfo == NULL) {
    // special case: only 1 frame in frame list, cannot adjust to next frame node
    rfinfo = frame_list->retrieve(frame_list->start());    // retrieve start frame
    bytes = rfinfo->getNumBytes();    // store size of frame
    frames = rfinfo->getNumFrames(); // decr num frames
    frame_list->adjustStartTo(rfpos);    // adjust Start
    rfpos = frame_list->start();     // update rfpos
    num_frames -= frames;            // decr num frames
    start_frame_id += frames;        // incr start frame
    buffer += bytes;                 // update buffer start
    buffer_size -= bytes;            // decr buffer size
   
  }
  else {
    // find old start address of the buffer
    old_buffer = buffer;

    frame_list->adjustStartTo(rfpos);

    rfpos = frame_list->start();

    // adjust the count of the number of frames
    if (num_frames != XIL_CIS_UNKNOWN_NUMBER_OF_FRAMES)
      num_frames = num_frames - (rfinfo->getFrameId()-start_frame_id);

    // adjust the id of the first frame
    start_frame_id = rfinfo->getFrameId();

    // adjust the start address of the buffer
    buffer = rfinfo->getStart();

    // adjust the size of the buffer
    buffer_size -= buffer - old_buffer;
 }
 
  return start_frame_id;
}     

//------------------------------------------------------------------------
//
//  Function:	XilCisBuffer::nextPossibleFrame(int framenumber, int type)
//  Created:	92/09/21
//
//  Description:
//	
//	This routine searches through the frame list in an attempt to
//      find a frame whose id is  greater than or equal to framenumber
//      and whose type matches the desired type. 
//	
//  Parameters:
//	
//	int framenumber:  want a frame id >= to this
//      int type:         type of frame to look for
//
//  Returns:
//	
//	int:              non-negative values means that the
//			    return value is the closest frame that
//                          is seekable.
//
//                        -1 reflect that no frame was found
//
//------------------------------------------------------------------------

int      XilCisBuffer::nextPossibleFrame(int framenumber, int type)
{
  // find frame framenumber
  XiliFrameInfoAListPositionType pos;
  XiliFrameInfo* finfo;
  Xil_boolean found = FALSE;

  // determine the best place to start looking
  pos = (framenumber >= getRFrameId()) ? rfpos : frame_list->start();

  while (!found && pos != frame_list->end()){
    finfo = frame_list->retrieve(pos);
    if (framenumber <= finfo->getFrameId() &&
        (type == XIL_CIS_ANY_FRAME_TYPE ||
         finfo->getFrameType() == XIL_CIS_ANY_FRAME_TYPE ||
         type == finfo->getFrameType()))
        found = TRUE;
    else
      pos = frame_list->next(pos);
  }

  return (found) ? finfo->getFrameId() : -1;
}


//------------------------------------------------------------------------
//
//  Function:	XilCisBuffer::prevPossibleFrame(int framenumber, int type)
//  Created:	92/09/21
//
//  Description:
//	
//  Description:
//	
//	This routine searches through the frame list in an attempt to
//      find a frame whose id is  less than or equal to framenumber
//      and whose type matches the desired type. 
//	
//  Parameters:
//	
//	int framenumber:  want a frame id <= to this
//      int type:         type of frame to look for
//
//  Returns:
//	
//	int:              non-negative values means that the
//			    return value is the closest frame that
//                          is seekable.
//
//                        -1 reflect that no frame was found
//
//------------------------------------------------------------------------

int      XilCisBuffer::prevPossibleFrame(int framenumber, int type)
{
  // find frame framenumber
  XiliFrameInfoAListPositionType pos;
  XiliFrameInfo* finfo;
  Xil_boolean found = FALSE;
  
  pos = (framenumber <= getRFrameId()) ? rfpos : frame_list->previous(frame_list->end());

  // note: make assumption that pos previous to start of list is == -1
  
  while (!found && pos != -1){
    finfo = frame_list->retrieve(pos);
    if (framenumber >= finfo->getFrameId() &&
        (type == XIL_CIS_ANY_FRAME_TYPE ||
         finfo->getFrameType() == XIL_CIS_ANY_FRAME_TYPE ||
         type == finfo->getFrameType()))
        found = TRUE;
    else
      pos = frame_list->previous(pos);
  }

  return (found) ? finfo->getFrameId() : -1;
}

//------------------------------------------------------------------------
//
//  Function:	XilCisBuffer::frameBoundaryAfter(Xil_unsigned8* cptr,
//                                       Xil_unsigned8** fptr, int* num_frames)
//  Created:	92/09/21
//
//  Description:
//	
//	This routine searches the frame list for a start address that
//      is greater than the current ptr (cptr). The number of frames
//      to this segment is calculated.
//	
//  Parameters:
//	
//	Xil_unsigned8* cptr: current pointer 
//	Xil_unsigned8** ptr: return the pointer to the next known
//                           frame boundary
//
//      int* num_frames:     return the number of frames that lie between
//                           the current position and the frame boundary 
//                           (including the current frame)
//
//------------------------------------------------------------------------
int      XilCisBuffer::frameBoundaryAfter(Xil_unsigned8* cptr,
Xil_unsigned8** fptr, int* num_frames_arg)
{
  XiliFrameInfoAListPositionType pos;
  XiliFrameInfo* finfo;
  Xil_boolean found = FALSE;
  int nframes;

  for (pos = frame_list->start();
       pos != frame_list->end() && !found;
       pos = frame_list->next(pos)){

    finfo = frame_list->retrieve(pos);
    if (finfo->getStart() > cptr){
      *fptr = finfo->getStart();
      found = TRUE;
    } else if (finfo->getStart() + finfo->getNumBytes() >= cptr) {
      // error occurred within this segment
      nframes = finfo->getNumFrames();
      if (nframes != XIL_CIS_UNKNOWN_NUMBER_OF_FRAMES)
        *num_frames_arg += nframes;
      else
        *num_frames_arg = XIL_CIS_UNKNOWN_NUMBER_OF_FRAMES;
    }
  }

  return (found) ? TRUE : FALSE;
  
}

//------------------------------------------------------------------------
//
//  Function:	XilCisBuffer::recoveryFrame(Xil_unsigned8* fptr,
//                                          int recovery_frame_id)
//  Created:	92/09/21
//
//  Description:
//	
//	This routine searches the frame list for a start address that
//      is greater than the current ptr (cptr). The number of frames
//      to this segment is calculated.
//	
//  Parameters:
//	
//	Xil_unsigned8*  ptr:   pointer to start of the recovery frame
//
//      int recovery_frame_id: the frame id of the recovery frame
//
//------------------------------------------------------------------------
int  XilCisBuffer::recoveryFrame(Xil_unsigned8* fptr, int recovery_frame_id)
{
  // search through the frame list for a segment that starts at fptr

  XiliFrameInfoAListPositionType pos;
  XiliFrameInfo* finfo;
  Xil_boolean found = FALSE;

  for (pos = frame_list->start();
       pos != frame_list->end() && !found;
       pos = frame_list->next(pos)){

    finfo = frame_list->retrieve(pos);
    if (finfo->getStart() == fptr){

      //set the read frame to this position
      rfpos = pos;
      rfptr = fptr;

      if (finfo->getFrameId() == XIL_CIS_UNKNOWN_FRAME_ID){
        // CASE A (see XilCisBufferManager::errorRecoveryDone)
        finfo->setFrameId(recovery_frame_id);
        return XIL_SUCCESS;
      } else if (finfo->getFrameId() <= recovery_frame_id) {
        // CASE B (see XilCisBufferManager::errorRecoveryDone)
        finfo->setFrameId(recovery_frame_id);
        return XIL_SUCCESS;
      } else
        // CASE B (see XilCisBufferManager::errorRecoveryDone)
        return XIL_FAILURE;
    } else 
	if (fptr > finfo->getStart() && fptr < finfo->getStart() + finfo->getNumBytes()) {
	    // determine the num bytes in segment
	    int old_nbytes = fptr - finfo->getStart();
	    int new_nbytes = finfo->getNumBytes() - old_nbytes;

	    // set this value
	    finfo->setNumBytes(old_nbytes);
	    
	    XiliFrameInfo* new_info = new XiliFrameInfo(fptr,new_nbytes,-1,recovery_frame_id,0);

            if (new_info == NULL) {
              XIL_ERROR( NULL, XIL_ERROR_RESOURCE,"di-1",TRUE);
              return XIL_FAILURE;
            }
	    rfpos = frame_list->insertAfter(new_info,pos);
      	    if (rfpos == -1) {
		// failure to insert in list
		XIL_ERROR( NULL, XIL_ERROR_RESOURCE,"di-1",FALSE);
		return XIL_FAILURE;
      	    }
	    rfptr = fptr;
          } 		
  }

  return XIL_FAILURE;
}

//------------------------------------------------------------------------
//
//  Function:	
//  Created:	92/09/24
//
//  Description:
//	
//	
//	
//  Parameters:
//	
//	
//  Returns:
//	
//	
//  Side Effects:
//	
//	
//  Notes:
//	
//	
//  Deficiencies/ToDo:
//	
//	
//------------------------------------------------------------------------
void XilCisBuffer::foundFrameDuringRecovery(Xil_unsigned8* fptr, int recovery_frame_id)
{

  // search through the frame list for a segment that starts at fptr

  XiliFrameInfoAListPositionType pos;
  XiliFrameInfo* finfo;
  Xil_boolean found = FALSE;

  for (pos = frame_list->start();
       pos != frame_list->end() && !found;
       pos = frame_list->next(pos)){

    finfo = frame_list->retrieve(pos);
    if (finfo->getStart() == fptr){
      // found a node already for this frame so we can just return
      return;
    } else
      // check to see if frame lies within the current segment
      if (fptr > finfo->getStart() && fptr < finfo->getStart() + finfo->getNumBytes()) {

        // frame is with segment, break segment up.

        // NOTE: we are making the assumption that from the
        // start of the current segment to this new frame 
        // point is equivalent to ONE frame.

        // determine the num bytes and frames in old/new segment
        int old_nbytes = fptr - finfo->getStart();
        int new_nbytes = finfo->getNumBytes() - old_nbytes;
        int new_nframes = finfo->getNumFrames() - 1;

        // if finfo->getNumFrames() returned -1
        // (XIL_CIS_UNKNOWN_NUMBER_OF_FRAMES), then
        // reset new_nframes to this value.
        
        if (new_nframes == -2)
          new_nframes = XIL_CIS_UNKNOWN_NUMBER_OF_FRAMES;
        
        // adjust old segement
        finfo->setNumBytes(old_nbytes);
        finfo->setNumFrames(1);

        // create new frame info segement and insert it after current  pos
        XiliFrameInfo* new_info = new XiliFrameInfo(fptr,new_nbytes,new_nframes,recovery_frame_id,0);
        if (new_info == NULL) {
          XIL_ERROR( NULL, XIL_ERROR_RESOURCE,"di-1",TRUE);
          return;
        }
        frame_list->insertAfter(new_info,pos);
      }
  }
}
        






//------------------------------------------------------------------------
//
//  Function:	XilCisBuffer::removeStartFrame()
//  Created:	93/04/23
//
//  Description:
//	
//	Remove the 1st frame node of the current buffer, start frame.
//      Used by the XilCisBufferManager::moveEndStartOneBuffer() routine
//      after the start frame has been copied to a newly created buffer.
//
//  Parameters:
//	
//  Returns:
//      int:  buffer removal flag--this flag will let the buffer manager 
//            know that this buffer is no longer needed since it only 
//            contained the start frame.  If flag = -1, indicates an
//            error was detected by this routine.
//
//  Side Effects:
//	
//	if buffer contains more than 1 frame,  the buffer's initial position,
//      start_frame_id and num_frames updated.
//
//
//------------------------------------------------------------------------

int XilCisBuffer::removeStartFrame()
{
   // remove first frame node in current buffer

  // sanity check: we should be at first frame in the buffer
  if ((rfpos != 0) || (rfpos == frame_list->end())) {

    // Not at first frame of buffer 
    XIL_ERROR( NULL, XIL_ERROR_SYSTEM, "di-340", TRUE); 
    return -1;
  }

  // check for only 1 frame node
  if (frame_list->next(rfpos) == frame_list->end())
    // only one frame in this buffer, delete entire buffer
    return 1;

  // more than 1 frame node, remove 1st frame node
  XiliFrameInfo* rfinfo = frame_list->remove(rfpos);
  buffer_size -= rfinfo->getNumBytes();
  delete rfinfo;  

  XiliFrameInfo* fi = frame_list->retrieve(rfpos);
  rfptr = fi->getStart();
  buffer = rfptr;
  start_frame_id = fi->getFrameId();
  
  if (num_frames != XIL_CIS_UNKNOWN_NUMBER_OF_FRAMES)
    num_frames -= 1;

  // keep buffer as updated
  return 0;  
}


//------------------------------------------------------------------------
//
//  Function:	int XilCisBuffer::updateLastFrame()
//  Created:	93/05/26
//
//  Description:
//	
//	Update the last frame of the current buffer so that the
//      frame node num_bytes and the buffer wfptr are accurate.
//      This routine is called after bytes have been added to the
//      existing last frame.
//
//  Returns:
//	XIL_SUCCESS or XIL_FAILURE
//
//  Notes:
//
//      The buffer to be operated upon should be the current write
//      buffer of the CBM, the w_buffer.
//
//------------------------------------------------------------------------

int XilCisBuffer::updateLastFrame()
{
  XiliFrameInfoAListPositionType finfo_pos;

  finfo_pos = frame_list->end();
  finfo_pos = frame_list->previous(finfo_pos);

  // if finfo_pos is < 0, we cannot seek backwards.
  // (this position being negative means we are in 
  // front of the beginning of the frame list).
 

  if (finfo_pos < 0) {

    // No previous entry from end, cis is empty

    XIL_ERROR( NULL, XIL_ERROR_USER, "di-107", TRUE);
    return XIL_FAILURE;

  }

  // if bytes were added, then the wptr has moved forward of wfptr
  // update num_bytes in frame info node, and wfptr for buffer
  XiliFrameInfo* finfo = frame_list->retrieve(finfo_pos);
  finfo->setNumBytes( finfo->getNumBytes() + (wptr - wfptr));
  
  if (wfptr==rfptr)
    // adjust buffer read ptr to "new" end of frame
    rfptr = wptr;
  // adjust buffer write ptr to "new" end of frame
  wfptr = wptr;

  return XIL_SUCCESS;
}



int
XilCisBuffer::getNumFrames() const
{
    return num_frames;
}

int
XilCisBuffer::getStartFrameId() const
{
    return start_frame_id;
}

int
XilCisBuffer::getNumBytes() const
{
    return wptr - buffer;
}

unsigned int
XilCisBuffer::getBufferSize() const
{
    return buffer_size;
}

int
XilCisBuffer::getNumBytesInWFrame() const
{
    return wptr - wfptr;
}

void
XilCisBuffer::addShort(int s)
{
    addByte((s) >> 8);
    addByte(s);
}

void
XilCisBuffer::addShorts(short*       s,
                        unsigned int m_shorts)
{
    for(unsigned int i=0; i<m_shorts; i++) {
        addShort((int)(s[i]));
    }
}
