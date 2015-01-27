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
//  File:       XilDeviceCompressionH261.cc
//  Project:    XIL
//  Revision:   1.6
//  Last Mod:   10:15:17, 03/10/00
//
//  Description:
//
//    H261 Device Compression Class Implementation
//
//
//------------------------------------------------------------------------
//    COPYRIGHT
//------------------------------------------------------------------------
#pragma ident   "@(#)XilDeviceCompressionH261.cc	1.6\t00/03/10  "

#include <stdio.h>
#include <xil/xilGPI.hh>
#include "XilDeviceCompressionH261.hh"
#include "XilDeviceManagerCompressionH261.hh"

#define NOT_H261_FRAME_TYPE 1

// XXX What is the right value for H261??
#define FRAMES_PER_BUFFER 2

XilDeviceCompressionH261::XilDeviceCompressionH261(
    XilDeviceManagerCompression* xdct, 
    XilCis*                      xcis)
: XilDeviceCompression(xdct, xcis, 0, FRAMES_PER_BUFFER)
{

    isOKFlag = FALSE;

    ditherBuf = NULL;

    //
    // Start out with the no rescale case
    //
    current_scale[0]  = 1.0F;
    current_scale[1]  = 1.0F;
    current_scale[2]  = 1.0F;
    current_offset[0] = 0.0F;
    current_offset[1] = 0.0F;
    current_offset[2] = 0.0F;

    if(! deviceCompressionValid()) {
        return;
    }

    if (initValues() == XIL_FAILURE) {
        return;
    }

    //
    // Create a buffer for the results of the upsample operation
    // so that this can be fed into the dither function. 
    // This buffer is large enough to hold a CIF frame (352x288),
    // the largest allowed for H.261 codecs.
    //
    ditherBuf = new Xil_unsigned8[352*288*3];
    if(ditherBuf == NULL) {
        return;
    }

    cbm->setSeekToStartFrameFlag(TRUE);
    decompData.cbm = cbm;
    
    isOKFlag = TRUE;
}

XilDeviceCompressionH261::~XilDeviceCompressionH261()
{
    delete ditherBuf;

    //
    // Destroy the ImageFormat object(s).
    //
    if(inputType != outputType) {
        outputType->destroy();
    }

    inputType->destroy();

}

void 
XilDeviceCompressionH261::setBitsPerImage(int value) 
{
  if (value < 0) {
    XIL_ERROR(system_state, XIL_ERROR_USER, "di-324", TRUE);
  } else {
    compData.BitsPerImage = value;
    compData.version++;
  }
}

int 
XilDeviceCompressionH261::getBitsPerImage() 
{
  return compData.BitsPerImage;
}

void 
XilDeviceCompressionH261::setImageSkip(int value) 
{
  if (value < 0 || value > 31) {
    XIL_ERROR(system_state, XIL_ERROR_USER, "di-324", TRUE);
  } else {
    compData.ImageSkip = value;
    compData.version++;
  }
}

int 
XilDeviceCompressionH261::getImageSkip() 
{
  return compData.ImageSkip;
}

void 
XilDeviceCompressionH261::setSearchRange(XilH261MVSearchRange* value) 
{
  if (value->x < 0 || value->x > 15 ||
      value->y < 0 || value->y > 15) {
    XIL_ERROR(system_state, XIL_ERROR_USER, "di-324", TRUE);
  } else {
    compData.searchX = value->x;
    compData.searchY = value->y;
    compData.version++;
  }
}

XilH261MVSearchRange*
XilDeviceCompressionH261::getSearchRange() 
{
  XilH261MVSearchRange* range = new XilH261MVSearchRange;

  range->x = compData.searchX;
  range->y = compData.searchY;
  return range;
}

void 
XilDeviceCompressionH261::setLoopFilter(Xil_boolean value) 
{
  if (value != TRUE && value != FALSE) {
    XIL_ERROR(system_state, XIL_ERROR_USER, "di-318", TRUE);
  } else {
    compData.LoopFilter = value;
    compData.version++;
  }
}

Xil_boolean 
XilDeviceCompressionH261::getLoopFilter() 
{
  return compData.LoopFilter;
}

void 
XilDeviceCompressionH261::setEncodeIntra(Xil_boolean value) 
{
  if (value != TRUE && value != FALSE) {
    XIL_ERROR(system_state, XIL_ERROR_USER, "di-318", TRUE);
  } else {
    compData.EncodeIntra = value;
    compData.version++;
  }
}

Xil_boolean 
XilDeviceCompressionH261::getEncodeIntra() 
{
  return compData.EncodeIntra;
}

void 
XilDeviceCompressionH261::setCompFreezeRelease(Xil_boolean value) 
{
  if (value != TRUE && value != FALSE) {
    XIL_ERROR(system_state, XIL_ERROR_USER, "di-318", TRUE);
  } else {
    compData.FreezeRelease = value;
    compData.version++;
  }
}

Xil_boolean 
XilDeviceCompressionH261::getCompFreezeRelease() 
{
  return compData.FreezeRelease;
}

void 
XilDeviceCompressionH261::setCompSplitScreen(Xil_boolean value) 
{
  if (value != TRUE && value != FALSE) {
    XIL_ERROR(system_state, XIL_ERROR_USER, "di-318", TRUE);
  } else {
    compData.SplitScreen = value;
    compData.version++;
  }
}

Xil_boolean 
XilDeviceCompressionH261::getCompSplitScreen() 
{
  return compData.SplitScreen;
}

void 
XilDeviceCompressionH261::setCompDocCamera(Xil_boolean value) 
{
  if (value != TRUE && value != FALSE) {
    XIL_ERROR(system_state, XIL_ERROR_USER, "di-318", TRUE);
  } else {
    compData.DocCamera = value;
    compData.version++;
  }
}

Xil_boolean 
XilDeviceCompressionH261::getCompDocCamera() 
{
  return compData.DocCamera;
}

void 
XilDeviceCompressionH261::setIgnoreHistoryFlag(Xil_boolean value)
{

  if (value != TRUE && value != FALSE) {
    XIL_ERROR(system_state, XIL_ERROR_USER, "di-318", TRUE);
  } else {
    // if we are moving into the state "ignorehistory = TRUE",
    // then flush any prior ops so that they are performed under
    // the attribute's previous state (probably FALSE, which would
    // mean we must correctly update the history buffer)
    if (value)
	cis->flushPriorDecompressOps(cis->getReadFrame());
    decompData.ignoreHistory = value;
    decompData.version++;
    setRandomAccess(value);
  }
}

Xil_boolean 
XilDeviceCompressionH261::getIgnoreHistoryFlag()
{
   return decompData.ignoreHistory;
}

Xil_boolean 
XilDeviceCompressionH261::getScreenFlag()
{
   return decompData.splitScreen;
}

Xil_boolean 
XilDeviceCompressionH261::getCameraFlag()
{
   return decompData.documentCamera;
}

Xil_boolean 
XilDeviceCompressionH261::getFreezeFlag()
{
   return decompData.freezeFrame;
}

XilH261SourceFormat 
XilDeviceCompressionH261::getCifFlag()
{
   if (decompData.isCif)
       return CIF;
    else
       return QCIF;
}

int 
XilDeviceCompressionH261::getTemporalRef()
{
   return decompData.temporalReference;
}

int
XilDeviceCompressionH261::initValues()
{
    XilImageFormat* t = 
      system_state->createXilImageFormat(0U, 0U, 3U, XIL_BYTE);

    if(t == NULL) {
      XIL_ERROR(system_state, XIL_ERROR_RESOURCE,"di-1",TRUE);
      return XIL_FAILURE;
    }

    outputType = inputType = t;
  
    inputOutputType = FALSE;
    imageWidth = 0;
    imageHeight = 0;
    setRandomAccess(FALSE);
    return XIL_SUCCESS;
}


void
XilDeviceCompressionH261::reset()
{
    if (inputType != outputType)
	outputType->destroy();
    inputType->destroy();
    
    initValues();
    decompData.reset();
    compData.reset();
    XilDeviceCompression::reset();
}

//
//  Routines to get references to the Compressor, Decompressor and
//  Attribute specific classes.
//

H261DecompressorData*  
XilDeviceCompressionH261::getH261DecompressorData(void)
{
  return &decompData;
}

H261CompressorData*  
XilDeviceCompressionH261::getH261CompressorData(void)
{
  return &compData;
}

int 
XilDeviceCompressionH261::findNextFrameBoundary()
{
  return decompData.findNextFrameBoundary();
}

void 
XilDeviceCompressionH261::burnFrames(int nframes)
{
  if (nframes > 0)
    decompData.burnFrames(nframes);
}

#define MAX_P	30
#define FUDGE	1000
int 
XilDeviceCompressionH261::getMaxFrameSize()
{

  // XXX This is bogus.  Figure out what it should be.

  // This number is ((256kbits/sec * p * 64kbits/sec ) * 1 sec ) / 8 bits/byte
  return (FUDGE + ((262143 + MAX_P * 65535) >> 3));
}

//
//  Function to read header and fill in the header information --
//  specifically width and height
//
XilStatus 
XilDeviceCompressionH261::deriveOutputType()
{

  int width, height, nbands;
  XilDataType datatype;

  // input and output types are assumed to be the same
  if (inputOutputType == FALSE) {
      // type not initialized from bitstream
      if (!decompData.getOutputType(&width, &height, &nbands, &datatype)) {
	  return XIL_FAILURE;
      }

      XilImageFormat* newtype = 
        system_state->createXilImageFormat(width, height,
                                      nbands, datatype);
      if (newtype == NULL) {
        XIL_ERROR(system_state,XIL_ERROR_RESOURCE,"di-1",TRUE);
        return XIL_FAILURE;
      }

      imageWidth    = width;
      imageHeight   = height;
      imageNbands   = nbands;
      imageDatatype = datatype;

      setInputType(newtype);  // sets outputType as a sideaffect
      newtype->destroy();
      inputOutputType = TRUE;
  }

  return XIL_SUCCESS;
}

Xil_boolean
XilDeviceCompressionH261::isOK()
{
    if(this == NULL) {
        return FALSE;
    } else {
        if(isOKFlag == TRUE) {
            return TRUE;
        } else {
            delete this;
            return FALSE;
        }
    }
}
    

//------------------------------------------------------------------------
//
//  Function:	XilDeviceCompressionH261::seek
//
//  Description: We allow 2 modes of seeking in H261, selectable by the user.
//  The random_access attribute indicates if backward seeks are legal for
//  the cis; it is "gettable" only, and determined by the state of the
//  user "settable" ignore_history attribute.  When ignore_history is FALSE, the seek
//  backward is disabled and any forward seeks keep the history buffers
//  accurate by burning through frames.  When ignore_history is TRUE, the seek
//  backward is enabled, and any seek (either forward or backward), takes 
//  you to the desired frame without updating the history buffers.
//	
//  DEFERRED EXECUTION NOTE:
//	If we are not ignoring history, then we can get two cases which
//  are not handled in a straightforward manner.  1) We need to decompress
//  the same frame we just finished decompressing.  Since we have the prev
//  and curr y,u,v blocks stored in the decompressor, we handle it by just
//  seeking to the desired location, and swapping the prev and curr block
//  pointers, so that we are ready to decompress into the curr block using
//  the prev block as the predictor.  2) If we need to decompress a frame
//  further back than read_frame-1, we are sunk.  We have to handle that
//  elsewhere by actually flushing any such decompresses from the dag.
//  This is implemented using seekFlush() as the seek within each molecule
//  and atom.  It cannot be part of seek() because seek is called from
//  the non-deferred part of decompress as well.
//      These situations are handled in very similar fashion by the 
//  CellB decompressor.
//
//  Parameters:
//	int framenumber    desired framenumber 
//	
//  Returns:
//	void
//	
//------------------------------------------------------------------------

void
XilDeviceCompressionH261::seek(int framenumber, Xil_boolean history_update)
{
    int frames_to_burn;
    int frames_to_skip;
    int frame_type;
    Xil_boolean ignore_history;

    // Check the ignore_history attribute to determine seek mode.
    ignore_history = decompData.getIgnoreHistoryFlag();
    if (ignore_history == TRUE) {
       // here if ignore_history mode, may seek either forward or backward,
       // without burning frames--history buffer retains last decompressed
       // frame!  
       frames_to_skip = cbm->seek(framenumber, XIL_CIS_ANY_FRAME_TYPE);
       if(frames_to_skip>0) {
         decompData.skipFrames(frames_to_skip);
       }
       else if (frames_to_skip <0) {
         // error in seeking
         XIL_ERROR(system_state, XIL_ERROR_SYSTEM,"di-317",TRUE);
         return;
       }
    } else {
       // here if NOT ignore_history mode, backward seek disabled.  
       // May still have backward seeks because of 
       //    1. deferred decompress
       //    2. previous ignore history set/seek back/reset 
       // If history_update  is TRUE, then seek must maintain history
       // correctly.  A forward seek uses a frame type which
       // does not exist in cis, to force burnFrames.  A backward (-1)
       // or frame seek uses ANY_FRAME_TYPE.  If history_update
       // is FALSE, then the seek is concerned with position in the cis
       // rather than specific frame type.
       if (history_update == TRUE)
         if (framenumber == (cbm->getNextDecompressId()-1))
           // special case of seek back by 1
           frame_type = XIL_CIS_ANY_FRAME_TYPE;
         else
           // normal seek
           frame_type = NOT_H261_FRAME_TYPE;
       else
         frame_type = XIL_CIS_NO_BURN_TYPE;
       frames_to_burn = cbm->seek(framenumber, frame_type);
       if (frames_to_burn>0) {
         burnFrames(frames_to_burn);
       }
       else if (frames_to_burn<0) {
         // error in seeking
         XIL_ERROR(system_state, XIL_ERROR_SYSTEM,"di-317",TRUE);
         return;
       }
    }
    return;
 }


void
XilDeviceCompressionH261::seekFlush(int framenumber)
{
    if (!decompData.ignoreHistory)
	cis->flushPriorDecompressOps(framenumber);
    seek(framenumber);
}


//
// Function to retrieve or create dither tables.
// This is located in the DeviceCompression because
// different CIS'es may be using different dither matrices.
// So each CIS must have its own table.
//
XiliOrderedDitherLut*
XilDeviceCompressionH261::getDitherTable(XilLookupColorcube* cmap,
                                         XilDitherMask*      dmask,
                                         float*              scale,
                                         float*              offset)
{
    //
    // We can potentially use the existing table.
    // But first, check if the cmap and dmask versions have changed.
    // If so, then we need to create a new table.
    // This is also true for the first call. 
    // Use a mutex lock to serialize access here.
    //
    //
    mutex.lock();
    if(ditherTable == NULL                        ||
       !cmap->isSameAs(&current_cmap_version)     ||
       !dmask->isSameAs(&current_dmask_version)   ||
       scale[0]  != current_scale[0]              ||
       scale[1]  != current_scale[1]              ||
       scale[2]  != current_scale[2]              ||
       offset[0] != current_offset[0]             ||
       offset[1] != current_offset[1]             ||
       offset[2] != current_offset[2] ) {

        ditherTable = new XiliOrderedDitherLut(cmap, dmask, scale, offset, 0);
        if(! ditherTable->isOK()) {
            mutex.unlock();
            return NULL;
        }

        //
        // Record new version info
        //
        cmap->getVersion(&current_cmap_version);
        dmask->getVersion(&current_dmask_version);

        //
        // Record new scale, offset info
        //
        current_scale[0]  = scale[0];
        current_scale[1]  = scale[1];
        current_scale[2]  = scale[2];
        current_offset[0] = offset[0];
        current_offset[1] = offset[1];
        current_offset[2] = offset[2];
    }

    mutex.unlock();

    return ditherTable;
}

