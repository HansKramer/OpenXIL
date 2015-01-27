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
//  File:       XilDeviceCompression.cc
//  Project:    XIL
//  Revision:   1.21
//  Last Mod:   10:09:04, 03/10/00
//
//  Description:
//  
//    XilDeviceCompression is the base virtual class for the IHV compressors.
//    This file contains the default implementations of the non-pure
//    virtual functions.  In general, they assume a compression
//    without inter-frame encoding.  Compressions with inter-frame
//    encoding must write their own functions for some of these.
//
//------------------------------------------------------------------------
//    COPYRIGHT
//------------------------------------------------------------------------

#pragma ident   "@(#)XilDeviceCompression.cc	1.21\t00/03/10  "

#include "_XilDefines.h"
#include "_XilSystemState.hh"
#include "_XilDeviceCompression.hh"
#include "_XilImage.hh"
#include "_XilCis.hh"
#include "_XilBoxList.hh"
#include "XiliRect.hh"

//
// Constructor - also calls constructor for the CisBufferManager
//
XilDeviceCompression::XilDeviceCompression(
    XilDeviceManagerCompression* xdct,
    XilCis*                      xcis,
    int                          frame_size,
    int                          nfpb)
{
    dcOKFlag = FALSE;

    cbm = new XilCisBufferManager(xcis, frame_size, nfpb);
    if(cbm == NULL) {
        XIL_ERROR(xcis->getSystemState(), XIL_ERROR_RESOURCE, "di-1", TRUE);
    }
    if(! cbm->isOK()) {
        //  Couldn't create CisBufferManager object
        XIL_ERROR(NULL, XIL_ERROR_RESOURCE, "di-274", TRUE);
        return;
    }

    random_access = TRUE;
    mgr           = xdct;
    cis           = xcis;
    system_state  = cis->getSystemState();
    initValues();
    xil_data_type = -1;
    inputType     = NULL;
    outputType    = NULL;

    dcOKFlag      = TRUE;
}

XilDeviceCompression::~XilDeviceCompression() 
{
    delete cbm;
}

Xil_boolean
XilDeviceCompression::deviceCompressionValid()
{
    if(this == NULL) { 
        return FALSE; 
    } else { 
        if(dcOKFlag == TRUE) { 
            return TRUE; 
        } else { 
            delete this; 
            return FALSE; 
        } 
    }
}

void
XilDeviceCompression::generateError(XilErrorCategory category, 
                                    char*            id,
                                    int              primary, 
                                    Xil_boolean      read_invalid,
                                    Xil_boolean      write_invalid,
                                    int              line, 
                                    char*            file) 
{
    cis->generateError(category, id, primary, read_invalid, write_invalid,
                       line, file);
}

void
XilDeviceCompression::generateError(XilErrorCategory category,
                                    char*            id,
                                    int              primary,
                                    int              line,
                                    char*            file)
{
    system_state->notifyError(category, id, primary, line, file, cis, NULL);
}

XilCisBufferManager*
XilDeviceCompression::getCisBufferManager()
{
    return cbm;
}

XilDeviceManager*
XilDeviceCompression::getDeviceManager()
{
    return mgr;
}

XilCis*
XilDeviceCompression::getCis()
{
    return cis;
}

XilSystemState*
XilDeviceCompression::getSystemState()
{
    return cis->getSystemState();
}

XilStatus
XilDeviceCompression::deriveOutputType()
{
    return XIL_FAILURE;
}

//
// Default behavior is to set all the fields that are 0 in the inputType.
// generate an error if any type changes.  If inputType && outputType are
// the same, then update outputType;
//
XilStatus
XilDeviceCompression::setInputType(XilImageFormat* image_format)
{
    //
    // Get target image type info
    //
    unsigned int tw, th, tn;
    XilDataType td;
    image_format->getInfo(&tw, &th, &tn, &td);

    //
    // Get stored type info
    //
    unsigned int iw, ih, in;
    XilDataType id;
    inputType->getInfo(&iw, &ih, &in, &id);

    //
    // Verify that input and output image types are the same
    // (that width, height and nbands match)
    //
    if( (iw && (iw != tw)) ||
        (ih && (ih != th)) ||
        (in && (in != tn)) ||
        ((xil_data_type != -1) && (xil_data_type != (int)td)) ) {
        XIL_ERROR(system_state, XIL_ERROR_USER, "di-2", TRUE);
        return XIL_FAILURE;
    };

    Xil_boolean input_equal_output = (inputType == outputType);
    inputType->destroy();
    inputType = system_state->createXilImageFormat(tw, th, tn, td);
    if(input_equal_output) {
        outputType = inputType;
    }

    int mfs = getMaxFrameSize();
    cbm->setFrameSize(mfs);
    xil_data_type = (int)td;
  
    return XIL_SUCCESS;
}

int
XilDeviceCompression::hasData()
{
    return cbm->hasData();
}


int
XilDeviceCompression::numberOfFrames()
{
    return cbm->numberOfFrames();
}

Xil_boolean
XilDeviceCompression::hasFrame()
{
    return cbm->hasFrame();
}

int
XilDeviceCompression::adjustStart(int new_start_frame)
{
    return cbm->adjustStart(new_start_frame);
}

//
//  An alternative way to implement the decompress a previous frame
//  is to force deferred execution to always flush the cis if anything
//  tries to depend on a cis with a frame earlier than R-1.  Then you
//  must save enough history to generate the last frame again if
//  requested.   This version works for any compression where the
//  seek is guarenteed to work (i.e., no history, or history is such
//  that key-frames are retained and respected within seek).
//  TODO:  Handle dependencies such that compressions which can not
//  always guarantee a seek still work.

// For cell (and possibly others), we need to read the header information
// for a frame to update the attributes before we put the decompress 
// operation on the dag.  This provides the entry point for that and a 
// default implementation for compressions which do not need the 
// decompressHeader functionality.  It should return XIL_FAILURE if the 
// decompressHeader gets an error.
//
XilStatus
XilDeviceCompression::decompressHeader()
{
    return XIL_SUCCESS;
}

//
// Any compression which actually DOES something on flush should
// reimplement this virtual function
//
void
XilDeviceCompression::flush()
{
}

void XilDeviceCompression::initValues()
{
    xil_data_type = -1;
    in_molecule   = FALSE;
}


void
XilDeviceCompression::reset()
{
    initValues();
    cbm->reset();
}


//
// The default virtual function implements the most straight-forward
// seek, where no history needs to be maintained.  If the compression
// must maintain history, then it must seek to the closest key-frame
// and then burn forward to the actual frame.  Therefore the actual value
// of the parameter history_update is ignored by this routine, which
// treats it as FALSE.  The seek is to position only, no specific frame type.
//
void
XilDeviceCompression::seek(int         framenumber, 
                           Xil_boolean )
{
    int frames_to_burn;

    frames_to_burn = cbm->seek(framenumber);
    if(frames_to_burn > 0) {
        burnFrames(frames_to_burn);
    }
}


void*
XilDeviceCompression::getBitsPtr(int* nbytes, 
                                 int* nframes)
{
    return cbm->getBitsPtr(nbytes, nframes);
}


void
XilDeviceCompression::putBits(int   nbytes, 
                              int   nframes, 
                              void* data)
{
    cbm->putBits(nbytes, nframes, data);
}


void
XilDeviceCompression::putBitsPtr(int                        nbytes, 
                                 int                        nframes, 
                                 void*                      data, 
                                 XIL_FUNCPTR_DONE_WITH_DATA done_with_data)
{
    cbm->putBitsPtr(nbytes, nframes, data, done_with_data);
}


XilImageFormat*
XilDeviceCompression::getOutputType()
{
    unsigned int w, h, n;
    XilDataType d;
    outputType->getInfo(&w, &h, &n, &d);

    //
    // If we don't know one of the elements of the type 
    // and we have data in the CIS (i.e. when start_frame 
    // and write_frame are not equal to 0) then call
    // deriveOutputType to find out the type.
    //
    if((!w || !h || !n) &&
        !((cis->getStartFrame() == 0) && (cis->getWriteFrame() == 0))) {
        deriveOutputType();
    }
    return outputType;
}

XilImageFormat*
XilDeviceCompression::getOutputTypeHoldTheDerivation()
{
    return outputType;
}

int
XilDeviceCompression::findNextFrameBoundary()
{
    //
    // Device Compression has not implemented findNextFrameBoundary()
    // Operation not implemented
    //
    XIL_ERROR(system_state, XIL_ERROR_SYSTEM, "di-5", TRUE); 

    return XIL_FAILURE;
}

void 
XilDeviceCompression::attemptRecovery(unsigned int, 
                                      unsigned int,
                                      Xil_boolean&, 
                                      Xil_boolean&)
{
}

//
// Miscellaneous get/set functions
//
char*
XilDeviceCompression::getCompressor()
{
    return  mgr->getCompressor();
}

char*
XilDeviceCompression::getCompressionType()
{
    return mgr->getCompressionType();
}

Xil_boolean
XilDeviceCompression::getRandomAccess()
{
    return random_access;
}

void
XilDeviceCompression::setRandomAccess(Xil_boolean on_off)
{
    random_access = on_off;
}

XilImageFormat*
XilDeviceCompression::getInputType()
{ 
    return inputType;
}

int
XilDeviceCompression::getStartFrame()
{
    return cbm->getSFrameId();
}

int
XilDeviceCompression::getReadFrame()
{
    return cbm->getRFrameId();
}        

int
XilDeviceCompression::getWriteFrame()
{
    return cbm->getWFrameId(); 
}

XilStatus
XilDeviceCompression::getAttribute(const char* name, 
                                   void**      value)
{
    return mgr->getAttr(this, name, value); 
}

XilStatus
XilDeviceCompression::setAttribute(const char* name, 
                                   void*       value)
{
    return mgr->setAttr(this, name, value); 
}

void
XilDeviceCompression::destroy() 
{
    delete this; 
}

Xil_boolean
XilDeviceCompression::inMolecule() 
{
    return in_molecule; 
}

void
XilDeviceCompression::setInMolecule(Xil_boolean on_off)
{
    in_molecule = on_off; 
}

//
// Determine if the decompressed frame can be written directly
// to the destination image, without needing to account for ROIs.
// Then a decompression to a temporary buffer is not required.
//
Xil_boolean
XilDeviceCompression::cisFitsInDst(XilRoi*     roi)
{
    unsigned int cis_w = getOutputTypeHoldTheDerivation()->getWidth();
    unsigned int cis_h = getOutputTypeHoldTheDerivation()->getHeight();

    //
    //  Does the ROI have more than one rect?
    //  Since we're not prepared to do a complex containment
    //  test here, we'll return FALSE and let the rectangle handling
    //  code take care of this.
    //
    if(roi->numRects() > 1 || roi->numRegions() > 1) {
        return FALSE;
    }

    //
    //  Get the bounding box of the ROI as a rect
    //
    XiliRect* dst_rect = roi->getBoundingBox();
    if(dst_rect == NULL) {
        return FALSE;
    }

    //
    // Convert to width/height form
    //
    int dst_x1, dst_x2, dst_y1, dst_y2;
    int dst_w, dst_h;
    dst_rect->get(&dst_x1, &dst_y1, &dst_x2, &dst_y2);
    dst_w = dst_x2 - dst_x1 + 1;
    dst_h = dst_y2 - dst_y1 + 1;

    //
    // Does the cis rectangle fit in the destination rectangle
    //
    if(cis_w > (unsigned int) dst_w || cis_h > (unsigned int) dst_h) {
        return FALSE;
    }

    return TRUE;
}

