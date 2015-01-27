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
//  File:	XilDeviceCompressionIdentity.cc
//  Project:	XIL
//  Revision:	1.5
//  Last Mod:	10:14:05, 03/10/00
//
//  Description:
//	Contains the member functions of XilDeviceCompressionIdentity.
//	
//	
//	
//	
//	
//	
//	
//------------------------------------------------------------------------
#pragma ident	"@(#)XilDeviceCompressionIdentity.cc	1.5\t00/03/10  "

#include "XilDeviceCompressionIdentity.hh"

//
//  FRAMES_PER_BUFFER is a recommendation on the size of each buffer 
//  inside the CBM.
//

XilDeviceCompressionIdentity::XilDeviceCompressionIdentity(
    XilDeviceManagerCompression* xdmc,
    XilCis*                      xcis)
: XilDeviceCompression(xdmc, xcis, 0, FRAMES_PER_BUFFER)
{
    if(initValues() == XIL_FAILURE) {
        //  Couldn't create internal Identity compressor object
        XIL_ERROR(xcis->getSystemState(), XIL_ERROR_SYSTEM,"di-275", FALSE);
        return;
    }
}

XilDeviceCompressionIdentity::~XilDeviceCompressionIdentity(void) 
{ 
}

int            
XilDeviceCompressionIdentity::getMaxFrameSize(void) 
{
    unsigned int w, h, nbands;
    XilDataType datatype;

    inputType->getInfo(&w, &h, &nbands, &datatype);

    return w * h * nbands + 3 * sizeof(Xil_unsigned32);
}

XilStatus
XilDeviceCompressionIdentity::initValues()
{
    XilImageFormat* t = 
    system_state->createXilImageFormat(0, 0, 0, XIL_BYTE);
    
    if(t == NULL) {
        // out of memory
        XIL_ERROR(system_state, XIL_ERROR_RESOURCE,"di-1", TRUE);
        return XIL_FAILURE;
    }

    inputType = outputType = t;

    // output type has not yet been derived from bitstream
    isDerivedType = FALSE;

    // reset any attributes to default state
    comp_quality   = 0;
    decomp_quality = 0;
    
    return XIL_SUCCESS;
}

void
XilDeviceCompressionIdentity::reset()
{
    if(inputType != outputType) {
	outputType->destroy();
    }
    inputType->destroy();
    
    initValues();
    XilDeviceCompression::reset();
}

//
//  Function to read header and fill in the Type (ImageFormat) information
//
XilStatus
XilDeviceCompressionIdentity::deriveOutputType(void)
{
   // isDerivedType flags if the type has been derived from 
   // the bitstream--prevents an infinite loop when neither the
   // boundary nor type of the first frame in the CIS have been
   // established
    if(! isDerivedType) {
       //
       //  This call will ensure that there is an entire frame for me to
       //  look through.  If necessary, the cbm will call this class's
       //  findNextFrameBoundary to parse the bitstream and
       //  locate the end of the frame.
       //
       Xil_unsigned32* bp32 = (Xil_unsigned32*)cbm->nextFrame();
       if(bp32 == NULL) {
          return XIL_FAILURE;
       }
       
       //
       //  NOTE:  This doesn't produce an endian-portable bit-stream.
       //
       unsigned int image_width  = *bp32++;
       unsigned int image_height = *bp32++;
       unsigned int image_bands  = *bp32++;
       
       if(image_width && image_height && image_bands) {
          XilImageFormat*  new_format =
	    getCis()->getSystemState()->createXilImageFormat(image_width, image_height,
                                                   image_bands, XIL_BYTE);
          
          if(new_format == NULL) {
             XIL_ERROR(getCis()->getSystemState(), XIL_ERROR_RESOURCE,"di-1", TRUE);
             return XIL_FAILURE;
          }
          
          //
          //  This will also set the outputType as a side-effect
          //
          setInputType(new_format);
          new_format->destroy();	// destroy copy
          isDerivedType = TRUE;
       }
    }    
    return XIL_SUCCESS;
}


void
XilDeviceCompressionIdentity::burnFrames(int nframes)
{
    int frame_type;

    // In order to illustrate "key" frames, 
    // this codec marks even frames with its own frame type
    // This illustrates the use of frame type with the
    // compressedFrame/decompressFrame/seek/adjustStart routines
    // (Of course, codecs generally have a much better reason
    // to mark a frame as a "key" frame!)

    //
    //  Get the information about the CIS image type.
    //
    XilImageFormat*  cis_outtype = getOutputType();
    unsigned int     cis_width   = cis_outtype->getWidth();
    unsigned int     cis_height  = cis_outtype->getHeight();
    unsigned int     cis_bands   = cis_outtype->getNumBands();
    
    //
    //  Compute how far the next frame should be...
    //
    unsigned int  frame_size =
        cis_width*cis_height*cis_bands + 3*sizeof(Xil_unsigned32);
    
    for(int i=0; i<nframes; i++) {
	
       Xil_unsigned8* bp = (Xil_unsigned8*)cbm->nextFrame();

       // Get the frame number of the burn frame
       if(cbm->getRFrameId() & 0x1) {
         // odd frame, no special frame type
         frame_type = XIL_CIS_DEFAULT_FRAME_TYPE;
       } else {
         // even frame, mark it as our key frame
         frame_type = IDENTITY_FRAME_TYPE;
       }

       bp += frame_size;
       cbm->decompressedFrame(bp, frame_type);
    }
}

//
//  Function to find the next frame boundry
//
int
XilDeviceCompressionIdentity::findNextFrameBoundary(void)
{
    Xil_unsigned8* bp;
    unsigned int  frame_size;
    
    if(! isDerivedType) {
       unsigned int   image_dimensions[3] = {0, 0, 0};
       
       // not yet derived input/output type
       // cannot call getOutputType because we will recurse on this routine!
       // parse bitstream bytes to get width/height/bands
       for (int i=0; i<3; i++) {
          for (int j=0; j<sizeof(Xil_unsigned32); j++) {
             if((bp = cbm->getNextByte()) == NULL) {
               // here if no more bytes in buffer--failed!
               return XIL_FAILURE;
             }
             // accumulate bytes into current dimension
             image_dimensions[i] = (image_dimensions[i]*256) + *bp;
          }
       }      
       //
       //  Compute how far we have to advance the pointer.
       //
       frame_size =
         image_dimensions[0]*image_dimensions[1]*image_dimensions[2];
    } else {
       //
       //  Get the information about the CIS image type.
       //  will cause deriveOutputType() to be called if 
       //  outputType not yet established.
       //
       XilImageFormat*  cis_outtype    = getOutputType();
       unsigned int     image_width    = cis_outtype->getWidth();
       unsigned int     image_height   = cis_outtype->getHeight();
       unsigned int     image_bands    = cis_outtype->getNumBands();
       
       //
       //  Compute how far we have to advance the pointer.
       //
       frame_size =
         image_width*image_height*image_bands + 3*sizeof(Xil_unsigned32);
    }

    //
    //  Run through the frame one byte at a time upto the second to
    //  last byte in the frame.  The final byte will be set to the
    //  return value of getNextByte() -- as opposed to updating it
    //  on every cycle of the loop.
    //
    
    for(int i=0; i<frame_size - 1; i++) {
        if(cbm->getNextByte() == NULL) {
            return XIL_FAILURE;
        }
    }
    if((bp = cbm->getNextByte()) == NULL) {
        return XIL_FAILURE;
    }

    //
    //  Tell the CisBufferManager where the frame boundary is...
    //
    return cbm->foundNextFrameBoundary(bp + 1);
}

void
XilDeviceCompressionIdentity::seek(int         framenumber, 
                                   Xil_boolean history_update)
{
    int frames_to_burn;

    if(history_update == TRUE) {
      // when history_update is true, if we have key frames
      // then we must seek with repect to the key frame.
      // The "frames_to_burn" returned by the cbm
      // will start from a key frame, which means our history remains
      // correct
      frames_to_burn = cbm->seek(framenumber, IDENTITY_FRAME_TYPE);
    } else {
      // when history_update is false, then we are interested
      // in position only for this seek.  Flag the cbm that
      // there should be no burn frames for frame type.
      frames_to_burn = cbm->seek(framenumber, XIL_CIS_NO_BURN_TYPE);
    }

    if(frames_to_burn > 0) {
	burnFrames(frames_to_burn);
    }
}


int
XilDeviceCompressionIdentity::adjustStart(int new_start_frame)
{
   //	Called by the compression core to indicate that existing
   //	frames prior to the frame number given are not to be retained
   //	any longer due to KEEPFRAMES or MAXFRAMES requirements.
   //	We'll just simply call the XilCisBufferManager and tell it
   //	which type of frame MUST be kept and let it do any actual
   //	deleting of data.

    return cbm->adjustStart(new_start_frame, IDENTITY_FRAME_TYPE);
}

// NOTE: the Identity codec does not make use of
// "quality" ...these routines are here to illustrate
// the XilDeviceManagerCompressionIdentity registerAttr mechanism.

void
XilDeviceCompressionIdentity::setCompressionQuality(int value)
{
   comp_quality = value;
}

int
XilDeviceCompressionIdentity::getCompressionQuality()
{
   return comp_quality;
}

void
XilDeviceCompressionIdentity::setDecompressionQuality(int value)
{
   decomp_quality = value;
}

int
XilDeviceCompressionIdentity::getDecompressionQuality()
{
   return decomp_quality;
}
