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
//  File:        XilCis.cc
//  Project:     XIL
//  Revision:    1.30
//  Last Mod:    10:08:38, 03/10/00
//
//  Description:
//        
//        Implementation of virtual functions for codecs
//        
//------------------------------------------------------------------------
//        COPYRIGHT
//------------------------------------------------------------------------

#pragma ident        "@(#)XilCis.cc	1.30\t00/03/10  "

#include "_XilDefines.h"
#include "_XilCis.hh"
#include "_XilDeviceCompression.hh"
#include "_XilDeviceManagerCompression.hh"
#include "_XilImage.hh"
#include "_XilOp.hh"
#include "_XilSystemState.hh"

#define XIL_CIS_GET_ATOMIC(return_type, variable) \
    return_type atomic_tmp = (variable); \
    return atomic_tmp;

XilObject*
XilCis::createCopy()
{
    // TODO: Implement this
    return NULL;
}

void 
XilCis::flush()
{
    sync();
    deviceCompression->flush();
}

void 
XilCis::initValues()
{
    read_frame         = 0;
    write_frame        = 0;
    start_frame        = 0;
    keep_frames        = -1;
    max_frames         = -1;
    dependentCount     = 0;
    autorecover        = FALSE;
    dependents         = NULL;
    compressOp         = NULL;
    read_invalid       = FALSE;
    write_invalid      = FALSE;

    //
    //  Initialize the global space roi to NULL so it can be created when
    //  requested.
    //
    globalSpaceRoi     = NULL;
}

void 
XilCis::reset()
{
    sync();

    deviceCompression->reset();
    globalSpaceRoi->destroy();
    initValues();
}

void 
XilCis::seek(int framenumber, 
             int relative_to)
{
    //
    //  Update the version information prior to changing the state of the CIS.
    //
    newVersion();
    
    int goal_frame = -1;
    read_invalid   = FALSE;

    switch (relative_to)
    {
      case 0: // Relative to frame zero 
        goal_frame = framenumber;
        break;

      case 1: // Relative to current read frame
        goal_frame = read_frame + framenumber;
        break;

      case 2: // Relative to end of cis
        if(write_frame >= 0) {
            goal_frame = write_frame + framenumber;
        } else {
            // Determine if the cbm knows the write frame id
            int last_frame = deviceCompression->getWriteFrame();

            if(last_frame < 0) {
                //
                // Call to get the side-effect of finding the last frame
                // and updating write_frame if appropriate.
                //
                (void)deviceCompression->numberOfFrames();
                last_frame = deviceCompression->getWriteFrame();
            }

            if(last_frame != -1) {
                write_frame = last_frame;
                goal_frame = write_frame + framenumber;

            } else  

            //
            // Should never happen since we\'ve called numberOfFrames above
            // Seek to framenumber not in CIS
            //  
            XIL_CIS_UNCOND_ERROR(XIL_ERROR_SYSTEM, "di-105", 
                                   TRUE, this, TRUE, FALSE);

        }
        break;

      default:
        // Illegal relative_to value in xil_cis_seek
        XIL_CIS_UNCOND_ERROR(XIL_ERROR_USER, "di-124", 
                                 TRUE, this, FALSE, FALSE);
        return;
    } // End switch

    if(deviceCompression->getRandomAccess() == FALSE &&
       goal_frame < read_frame) {
        //  XilCis: Illegal seek backward in a non-random access CIS
        XIL_CIS_UNCOND_ERROR(XIL_ERROR_USER, "di-304", TRUE,
                             this, FALSE, FALSE);
        return;
    }


    if(goal_frame >= 0) {
        if(goal_frame < start_frame || 
           ((write_frame != -1) && goal_frame > write_frame)) {
            //  XilCis: Seek to framenumber not in CIS
            XIL_CIS_UNCOND_ERROR(XIL_ERROR_USER, "di-105", 
                                 TRUE, this, FALSE, FALSE);
            return;
        } else {
            read_frame = goal_frame;
            //
            //  Determine if we must adjust our perception of the 
            //  CIS starting frame number because keep_frames has been set.
            //
            if(keep_frames > 0 && (read_frame - start_frame) >= keep_frames) {
                start_frame = read_frame - keep_frames;

                flushPriorDecompressOps(start_frame);
                //
                // Call the device compression to handle the removal of any
                // data associated with changing to the new start frame.
                //
                deviceCompression->adjustStart(start_frame);
            }
        }
    }
}


void 
XilCis::attemptRecovery(unsigned int nframes, 
                        unsigned int nbytes)
{
    int was_read_invalid  = read_invalid;
    int was_write_invalid = write_invalid;

    deviceCompression->attemptRecovery(nframes, nbytes, 
                                       read_invalid, write_invalid);
    if(was_read_invalid && !read_invalid) {
        read_frame = deviceCompression->getReadFrame();
    }

    if(was_write_invalid && !write_invalid) {
        write_frame = deviceCompression->getWriteFrame();
    }
}


void 
XilCis::generateError(XilErrorCategory category, 
                      const char*      id,
                      int              primary, 
                      Xil_boolean      read_invalid_arg,
                      Xil_boolean      write_invalid_arg, 
                      int              line, 
                      const char*      file)
{
    //
    //  Need to reflect the actual device compression status in the
    //  error handler, not the perceived (deferred) status
    //
    //  However, if the read_frame changes while in the error handler, then
    //  we want to update our perceived read frame to that (since either a
    //  seek or an xil_cis_error_recover occurred).
    //

    int cis_rf = read_frame;
    int cis_wf = write_frame;
    int cis_sf = start_frame;
    int cbm_rf = read_frame = deviceCompression->getReadFrame();
    int cbm_wf = deviceCompression->getWriteFrame();
    int cbm_sf = deviceCompression->getStartFrame();

    if(read_invalid_arg) {
        this->read_invalid = TRUE;
    }
    if(write_invalid_arg) {
        this->write_invalid = TRUE;
    }
    deviceCompression->generateError(category, (char*)id,
                                     primary, line, (char*)file);
    if(cbm_rf == read_frame) {
        read_frame = cis_rf;
    }
    if(cbm_wf == write_frame) {
        write_frame = cis_wf;
    }
    if(cbm_sf == start_frame) {
        start_frame = cis_sf;
    }
}
    

void* 
XilCis::getBitsPtr(int* nbytes, 
                   int* nframes)
{
    deviceCompression->seek(read_frame, FALSE);
    void* tmp = deviceCompression->getBitsPtr( nbytes, nframes );
    read_frame += *nframes;

    return tmp;
}


void 
XilCis::putBits(int   nbytes, 
                int   nframes, 
                void* data)
{
    //
    //  Update the version information prior to changing the state of the CIS.
    //
    newVersion();
    
    if(nframes <= 0) {
        write_frame = -1;
    } else {
        if(write_frame >= 0) {
            write_frame += nframes;
        }
    }

    deviceCompression->putBits(nbytes, nframes, data);
}


void 
XilCis::putBitsPtr(int                        nbytes, 
                   int                        nframes, 
                   void*                      data, 
                   XIL_FUNCPTR_DONE_WITH_DATA done_with_data)
{
    //
    //  Update the version information prior to changing the state of the CIS.
    //
    newVersion();
    
    if(nframes <= 0) {
        write_frame = -1;
    } else {
        if(write_frame >= 0) {
            write_frame += nframes;
        }
    }

    deviceCompression->putBitsPtr(nbytes, nframes, data, done_with_data);
}

//
// This used deviceCompression == NULL as a symbol that the
// create failed.
//
XilCis::XilCis(XilSystemState* system_state, 
               const char*     compression_name)
: XilDeferrableObject(system_state, XIL_CIS)
{
    isOKFlag               = FALSE;

    deviceCompression      = NULL;
    compressFunctionName   = NULL;
    decompressFunctionName = NULL;
    compressOpNumber       = -1;
    decompressOpNumber     = -1;

    //
    //  Clear all of the variables and set necessary pointers to NULL
    //
    initValues();

    //
    //  Did the XilObject construction work?
    //
    if(getSystemState() == NULL) {  
        // XilObject will have already generated enough of an error
        return;
    }

    //
    //  Get the shared deviceManagerCompression object
    //
    XilGlobalState* xgs = XilGlobalState::getXilGlobalState();
    deviceManagerCompression =
    xgs->getDeviceManagerCompression(compression_name);

    if(deviceManagerCompression == NULL) {
        //
        // Compression device is unavailable
        // Can not use XIL_CIS_ERROR since cis is not created properly
        //
        XIL_ERROR(system_state, XIL_ERROR_OTHER, "di-125", FALSE);
        deviceCompression = NULL;
        return;
    }

    //
    // Create the compression object
    //
    deviceCompression = deviceManagerCompression->constructNewDevice(this);
    if(deviceCompression == NULL) {
        //
        // Couldn't create compression device. 
        // Can not use XIL_CIS_ERROR since cis is not created properly
        //
        XIL_ERROR(system_state, XIL_ERROR_OTHER, "di-126", FALSE);
        return;
    }
    // 
    // Record the CisBuffermanager
    //
    cbm = deviceCompression->getCisBufferManager();

    //
    // Stash a reference to the deviceCompression in the cbm
    //
    cbm->setXilDeviceCompression(deviceCompression);

    isOKFlag = TRUE;
}

XilCis::~XilCis()
{
    isOKFlag = FALSE;

    deviceCompression->destroy();
    globalSpaceRoi->destroy();
}


XilDeviceCompression* 
XilCis::getDeviceCompression(void)
{
    return deviceCompression;
}


XilDeviceManagerCompression* 
XilCis::getDeviceManagerCompression(void)
{
    return deviceManagerCompression;
}

void 
XilCis::flushPriorDecompressOps(int frame_no)
{
    int frame;
    for(unsigned int i=0; i<dependentCount; i++) {
        dependents[i]->getParam(1, &frame);
        if(frame < frame_no) {
            // Since this can be called from within execute routines
            // [see CellB and H261], we don't know whether or not
            // the op gets cleaned up here or not.
            XilOp* tmp = dependents[i];
            dependents[i]->flush();
            if((dependentCount > 0) && (tmp != dependents[i])) {
                i--;
            }
        }
    }
}

//
// Get/Set methods
//

int 
XilCis::getKeepFrames()
{
    XIL_CIS_GET_ATOMIC(int, keep_frames);
}


int 
XilCis::getMaxFrames()
{
    XIL_CIS_GET_ATOMIC(int, max_frames);
}

void 
XilCis::setKeepFrames(int k)
{
    //
    //  Update the version information prior to changing the state of the CIS.
    //
    newVersion();
    
    if(k < max_frames || max_frames <= 0) {
        keep_frames = k;
    } else {
        keep_frames = max_frames;
    }
}

void 
XilCis::setMaxFrames(int m)
{
    //
    //  Update the version information prior to changing the state of the CIS.
    //
    newVersion();
    
//
// Comment out this section. Changes to revert to the 1.2 list
// handler appear to have fixed this bug, so we can go back to 
// allowing max_frames to be 1.
//
#if 0
    //
    // TODO: Bug# 4024749 (Showme bug)
    // Temp fix for bug in maxFrames/keepFrames logic (lperry)
    //
    if(m < 2) {
        m = 2;
    }
#endif

    max_frames = m;
    if(m > keep_frames) {
        keep_frames = max_frames;
    }
}


Xil_boolean 
XilCis::getReadInvalid()
{
    XIL_CIS_GET_ATOMIC(Xil_boolean, read_invalid);
}

Xil_boolean 
XilCis::getWriteInvalid()
{
    XIL_CIS_GET_ATOMIC(Xil_boolean, write_invalid);
}

Xil_boolean 
XilCis::getAutorecover()
{
    XIL_CIS_GET_ATOMIC(Xil_boolean, autorecover);
}

void 
XilCis::setAutorecover(Xil_boolean on_off)
{
    //
    //  Update the version information prior to changing the state of the CIS.
    //
    newVersion();
    
    autorecover = on_off;
}

char* 
XilCis::getCompressor()
{
    return deviceCompression->getCompressor(); 
}

char* 
XilCis::getCompressionType()
{
    return deviceCompression->getCompressionType();
}


Xil_boolean 
XilCis::getRandomAccess()
{
    return deviceCompression->getRandomAccess();
}


int 
XilCis::getStartFrame()
{
    XIL_CIS_GET_ATOMIC(int, start_frame);
}

void 
XilCis::setStartFrame(int frame)
{
    //
    //  Update the version information prior to changing the state of the CIS.
    //
    newVersion();
    
    start_frame = frame;
}

int 
XilCis::getReadFrame()
{
    XIL_CIS_GET_ATOMIC(int, read_frame);
}

void 
XilCis::setReadFrame(int frame)
{
    //
    //  Update the version information prior to changing the state of the CIS.
    //
    newVersion();
    
    read_frame = frame;
}


int 
XilCis::getWriteFrame()
{
    //
    // TODO: This comment was in XIL 1.2.
    //       Leaving it here for future reference.
    //
    // this may eventually require decoding the bit stream once we 
    // allow insertion of frames on non-frame boundaries
    //

    //    if(write_frame == -1)
    //
    // this indicates that a put_bits with a non-integral 
    // number of frames occurred.  This is not supported for 
    // alpha, so it should never happen.

    XIL_CIS_GET_ATOMIC(int, write_frame);
}

void 
XilCis::setWriteFrame(int frame)
{
    //
    //  Update the version information prior to changing the state of the CIS.
    //
    newVersion();
    
    write_frame = frame;;
}


int 
XilCis::hasData()
{
    sync();
    deviceCompression->seek(read_frame, FALSE);
    return deviceCompression->hasData();
}

int 
XilCis::numberOfFrames()
{
    sync();
    deviceCompression->seek(read_frame, FALSE);
    return deviceCompression->numberOfFrames();
}

Xil_boolean 
XilCis::hasFrame()
{
    if(read_invalid) {
        return FALSE;
    } else {
        sync();
        deviceCompression->seek(read_frame, FALSE);
        return deviceCompression->hasFrame();
    }
}

XilStatus  
XilCis::getAttribute(const char* attribute_name,
                     void**       value)
{
    sync();
    deviceCompression->seek(read_frame, TRUE);
    XilStatus tmp = deviceCompression->getAttribute(attribute_name, value);

    return tmp;
}

XilStatus  
XilCis::setAttribute (const char* attribute_name,
                      void*       value)
{
    sync();

    deviceCompression->seek(read_frame, TRUE);

    //
    //  Update the version information prior to changing the state of the CIS.
    //
    newVersion();
    
    XilStatus tmp = deviceCompression->setAttribute(attribute_name, value);

    return tmp;
}

XilImageFormat*  
XilCis::getInputType()
{
    deviceCompression->seek(read_frame, TRUE);

    return deviceCompression->getInputType();
}

XilImageFormat*  
XilCis::getOutputType()
{
    //
    //  If the rule that the output type can not change in a CIS
    //  changes, then this must be changed.
    //
    XilImageFormat* otype =
    deviceCompression->getOutputTypeHoldTheDerivation();

    if(otype->getWidth()     == 0 ||
       otype->getHeight()    == 0 ||
       otype->getNumBands()  == 0) {
        deviceCompression->seek(read_frame, TRUE);
        return deviceCompression->getOutputType();
    } else {
        return otype;
    }
}


//------------------------------------------------------------------------
//
//  Function:        getGlobalSpaceRoi()
//
//  Description:
//
//  Create the global space ROI and add the rectangle defined from the
//  input image format to it.
//
//  TODO:  2/8/96 jlf  Problem here where the ROI depends on src/dst.
//
//    The ROI depends on whether the CIS is being used as a source or a
//    destination image.  For now, we assume they're the same size, but
//    they don't have to be and this will generate an incorrect answer in
//    the case where the input type and the output type are of different
//    sizes.
//
//        
//  MT-level:  <??????>
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
XilRoi*
XilCis::getGlobalSpaceRoi()
{
    if(globalSpaceRoi == NULL) {
        globalSpaceRoi = this->getSystemState()->createXilRoi();

        XilImageFormat* input_format = deviceCompression->getInputType();

        globalSpaceRoi->addRect(0, 0,
                                input_format->getWidth(),
                                input_format->getHeight());
    }

    return globalSpaceRoi;
}

XiliRect*
XilCis::getGlobalSpaceRect()
{
    if(globalSpaceRect.isEmpty()) {
        XilImageFormat* input_format = deviceCompression->getInputType();

        globalSpaceRect.set(0, 0,
                            input_format->getWidth(),
                            input_format->getHeight());
    }

    return &globalSpaceRect;
}

//
// Overloaded function for deferred execution
//
XiliOpQueuePosition
XilCis::setOp(XilOp* new_op)
{
    //
    // At most 1 compress operation can be deferred.
    // So check op queue length.
    //
    if(qLength() > 0) {
        //
        // flush the operation
        //
        if(sync() != XIL_SUCCESS) {
            XIL_ERROR(getSystemState(), XIL_ERROR_OTHER, "di-384", FALSE);
            return _XILI_OP_QUEUE_INVALID_POSITION;
        }
    }

    //
    // Insert the Op into the DAG
    //
    return XilDeferrableObject::setOp(new_op);
}
