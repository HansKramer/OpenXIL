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
//  File:	XilDeviceIO.cc
//  Project:	XIL
//  Revision:	1.18
//  Last Mod:	10:08:45, 03/10/00
//
//  Description:
//	
//	
//	
//	
//	
//	
//	
//	
//  MT-level:  <??????>
//	
//------------------------------------------------------------------------
//	COPYRIGHT
//------------------------------------------------------------------------
#pragma ident	"@(#)XilDeviceIO.cc	1.18\t00/03/10  "

#include "_XilDefines.h"
#include "_XilDeviceIO.hh"
#include "_XilOp.hh"
#include "_XilImage.hh"
#include "_XilBox.hh"
#include "_XilDeviceManagerIO.hh"

#include "XiliRect.hh"

XilDeviceIO::XilDeviceIO(XilDeviceManagerIO* device_manager) :
    deviceManager(device_manager)
{
    captureOp                 = NULL;
    controllingImage          = NULL;
    framebufferDeviceFlag     = FALSE;
    doubleBufferingDeviceFlag = FALSE;
    stereoDeviceFlag          = FALSE;
}

XilDeviceIO::~XilDeviceIO()
{
}

XilDeviceManagerIO*
XilDeviceIO::getDeviceManager()
{
    return deviceManager;
}

XilDeviceIOType
XilDeviceIO::getType()
{
    return XIL_DEVICE_IO_PULL;
}

XilStatus
XilDeviceIO::display(XilOp*       op,
                     unsigned int ,
                     XilRoi*      ,
                     XilBoxList*  )
{
    XIL_ERROR_WITH_ARG(op->getSystemState(), XIL_ERROR_SYSTEM,
                       "di-195", TRUE, (void*)deviceManager->getDeviceName());
    return XIL_FAILURE;
}

XilStatus
XilDeviceIO::capture(XilOp*       op,
                     unsigned int ,
                     XilRoi*      ,
                     XilBoxList*  )
{
    XIL_ERROR_WITH_ARG(op->getSystemState(), XIL_ERROR_SYSTEM,
                       "di-194", TRUE, (void*)deviceManager->getDeviceName());
    return XIL_FAILURE;
}

XilStatus
XilDeviceIO::startCapture(XilBox*)
{
    return XIL_FAILURE;
}

XilStatus
XilDeviceIO::stopCapture()
{
    return XIL_FAILURE;
}

XilStatus
XilDeviceIO::getPixel(unsigned int ,
                      unsigned int ,
                      float*       ,
                      unsigned int ,
                      unsigned int )
{
    return XIL_FAILURE;
}
    
XilStatus
XilDeviceIO::setPixel(unsigned int ,
                      unsigned int ,
                      float*       ,
                      unsigned int ,
                      unsigned int )
{
    return XIL_FAILURE;
}

XilBufferId
XilDeviceIO::getActiveBuffer()
{
    return XIL_BACK_BUFFER;
}
    
XilStatus
XilDeviceIO::setActiveBuffer(XilBufferId)
{
    return XIL_FAILURE;
}

XilStatus
XilDeviceIO::swapBuffers()
{
    return XIL_FAILURE;
}

Xil_boolean
XilDeviceIO::hasSubPixelCapture()
{
    return FALSE;
}

Xil_boolean
XilDeviceIO::hasSubPixelDisplay()
{
    return FALSE;
}

XilStatus
XilDeviceIO::reinitControllingImage(XilImageFormat* image_format)
{
    if(controllingImage == NULL || image_format == NULL) {
        //
        //  TODO:  jlf  4/23/96  generate an error...
        //
        return XIL_FAILURE;
    }

    return controllingImage->reinit(image_format);
}

XilStatus
XilDeviceIO::dataReady(unsigned int x,
                       unsigned int y,
                       unsigned int xsize,
                       unsigned int ysize)
{
    if(captureOp == NULL) {
        return XIL_FAILURE;
    }


    XiliRectInt rect(x, y, (x+xsize-1), (y+ysize-1));

    XilStatus status =
        captureOp->flushForward(&rect, 0, bottomOp, bottomTileNumber);
    
    return status;
}

XilStatus
XilDeviceIO::getCaptureStorage(XilStorage*      storage,
                               unsigned int     x,
                               unsigned int     y,
                               unsigned int     xsize,
                               unsigned int     ysize,
                               char*            storage_dev,
                               XilStorageType   type_requested,
                               void*            attribs)
{
    //
    //  Verify the capture op is set and then get the destination image.
    //
    if(captureOp == NULL) {
        //
        //  TODO:  3/11/96 jlf  Should generate internal error.
        //
        return XIL_FAILURE;
    }

    XilImage* image = captureOp->getDstImage(1);

    //
    //  Create a box that encapsulates the given region.
    //
    XilBox    box(x, y, xsize, ysize, 0);

    //
    //  Get the image storage...
    //
    storage->setImage(image);
    if(image->getStorage(storage, captureOp, &box, storage_dev,
                         XIL_WRITE_ONLY, type_requested, attribs) == XIL_FAILURE) {
        return XIL_FAILURE;
    }

    return XIL_SUCCESS;
}
