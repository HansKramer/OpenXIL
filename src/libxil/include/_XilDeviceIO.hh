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
//  File:	_XilDeviceIO.hh
//  Project:	XIL
//  Revision:	1.18
//  Last Mod:	10:21:03, 03/10/00
//
//  Description:
//	The class which abstracts an Input/Output device for XIL.
//	
//	
//	
//	
//	
//  MT-level:  SAFE
//	
//------------------------------------------------------------------------
//	COPYRIGHT
//------------------------------------------------------------------------
#pragma ident	"@(#)_XilDeviceIO.hh	1.18\t00/03/10  "

#ifndef _XIL_DEVICE_IO_HH
#define _XIL_DEVICE_IO_HH

//
//  XIL Includes
//
#include "_XilDefines.h"

//
//  Indicates what type of device it is.
//
typedef enum __XilDeviceIOType {
    XIL_DEVICE_IO_PULL,
    XIL_DEVICE_IO_PUSH
} XilDeviceIOType;

class XilDeviceIO {
public:
    //
    //  For display, capture, getPixel, and setPixel returning a status allows
    //  the XIL core to use a fallback device.  In the case of framebuffers
    //  the fallback is usually Xlib.
    //

    //
    //  Which type of device is it?
    //
    //  This function is called when a capture is required to determine which
    //  type of capture XIL is to use.
    //
    //  If getType() returns XIL_DEVICE_IO_PULL, then the capture() function
    //  is used to grab portions of the device.  Most devices that would sport
    //  this type are freely random access -- like framebuffers.
    //
    //  If getType() returns XIL_DEVICE_IO_PUSH, then the startCapture()
    //  function is used to request the entire boundary of area the XIL core
    //  needs from the device for this capture.  Due to ROIs being set or
    //  the type of operation, XIL may not request the entire image to be
    //  captured.
    //
    //  By default, getType() returns XIL_DEVICE_IO_PULL.
    //
    virtual XilDeviceIOType getType();
    
    //
    //  Return the image created by the device to the core, which can then
    //  attach the device to it.  This routine constructs a new controlling
    //  image for this device. 
    //
    virtual XilImage*       constructControllingImage()=0;

    //
    //  Is the device readable?
    //
    virtual Xil_boolean     isReadable()=0;

    //
    //  Is the device writable?
    //
    virtual Xil_boolean     isWritable()=0;

    //
    //  Set an attribute on the device
    //
    virtual XilStatus       setAttribute(const char* attribute_name,
                                         void*       value)=0;

    //
    //  Get an attribute from the device
    //
    virtual XilStatus       getAttribute(const char*  attribute_name,
                                         void**       value)=0;

    //
    //  Capture a portion of the image from the device and put in into the
    //  controlling image (device backing image).
    //
    //  This is used on devices that advertise themselves as
    //  XIL_DEVICE_IO_PULL type devices.
    //
    virtual XilStatus       capture(XilOp*       op,
                                    unsigned int op_count,
                                    XilRoi*      roi,
                                    XilBoxList*  bl);

    //
    //  These capture routines are used for devices that advertise themselves
    //  as XIL_DEVICE_IO_PUSH type devices.
    //
    //
    //  Start pushing data for a new capture.
    //
    virtual XilStatus       startCapture(XilBox* box);

    //
    //  Stop pushing data for the currently running capture.  This is not a
    //  pause, it is more like a "stop and reset".
    //
    virtual XilStatus       stopCapture();

    //
    //  Display a portion of the image on the device for both PUSH and PULL
    //  devices.  Only needed for writable devices.
    //
    virtual XilStatus       display(XilOp*       op,
                                    unsigned int op_count,
                                    XilRoi*      roi,
                                    XilBoxList*  bl);

    
    //
    //  Get a pixel from the device
    //
    virtual XilStatus       getPixel(unsigned int x,
                                     unsigned int y,
                                     float*       data,
                                     unsigned int offset_band,
                                     unsigned int num_bands);
    
    //
    //  Set a pixel on the device
    //
    virtual XilStatus       setPixel(unsigned int x,
                                     unsigned int y,
                                     float*       data,
                                     unsigned int offset_band,
                                     unsigned int num_bands);
    
    //
    //  Set and get the active buffer state for the device and swap back
    //  buffer an the front buffer.  After a swap the contents of the back
    //  buffer are UNDEFINED.  So, it's really a swap back to front.
    //
    //  These are only valid for devices created as a double buffering device
    //  via the xil_create_double_buffered_window() call.
    //
    virtual XilStatus       setActiveBuffer(XilBufferId active_buffer);
    virtual XilBufferId     getActiveBuffer();
    virtual XilStatus       swapBuffers();

    //
    //  Indicate whether the device's capture and display routines
    //  support sub-pixels.
    //
    //  This means the capture routine supports using the band_offset and
    //  num_bands arguments provided.  The default is that routines do not
    //  support writing sub portions of pixels to the device. 
    //
    virtual Xil_boolean     hasSubPixelCapture();
    virtual Xil_boolean     hasSubPixelDisplay();
    
    //
    //  Return the device manager which created this device.
    //
    XilDeviceManagerIO*     getDeviceManager();

    //
    //  Constructor...
    //
                            XilDeviceIO(XilDeviceManagerIO* device_manager);

    //
    //  Destructor...
    //
    virtual                 ~XilDeviceIO();

protected:
    //
    //  Give XIL the data which has been captured and is now ready to be
    //  passed on down the line.   Derived classes call this method when
    //  performing a capture to to present each buffer of new data to the XIL
    //  core.
    //
    //  The regions must be non-overlapping.  If the same region or a portion
    //  of a previously provided region is given to XIL, this function returns
    //  XIL_FAILURE and stopCapture() will be called.  The application will be
    //  told that the device failed to perform the capture.
    //
    XilStatus               dataReady(unsigned int x,
                                      unsigned int y,
                                      unsigned int xsize,
                                      unsigned int ysize);

    //
    //  Reinit the controlling image associated with the device to a new image
    //  format.  Only the controlling image is modified by this call, not its
    //  parent image (if there is one).
    //
    XilStatus               reinitControllingImage(XilImageFormat* image_format);

    //
    //  Get the storage to fill when in the process of doing a push style
    //  capture.
    //
    //  This will allocate a chunk of storage on the specified device for
    //  reading and writing that matches the controlling image type.  The
    //  storage information is set on the provided storage object.
    //
    //  If the given region crosses a tile boundary, the storage is cobbled.
    //  The I/O device can control the tile size by changing the tile size on
    //  the controlling image.
    //
    XilStatus               getCaptureStorage(XilStorage*      storage,
                                              unsigned int     x,
                                              unsigned int     y,
                                              unsigned int     xsize,
                                              unsigned int     ysize,
                                              char*            storage_dev,
                                              XilStorageType   type_requested = XIL_STORAGE_TYPE_UNDEFINED,
                                              void*            attribs = NULL);
    

private:
#ifdef _XIL_LIBXIL_PRIVATE
#define _XIL_PRIVATE_DATA
    
#include "XilDeviceIOPrivate.hh"
    
#undef  _XIL_PRIVATE_DATA
#endif
    
    XilDeviceManagerIO*     deviceManager;

    XilOp*                  captureOp;
    XilOp*                  bottomOp;
    XilTileNumber           bottomTileNumber;

    XilImage*               controllingImage;

    Xil_boolean             framebufferDeviceFlag;
    Xil_boolean             doubleBufferingDeviceFlag;
    Xil_boolean             stereoDeviceFlag;
    
    //
    //  Base member data
    //  (This has been decremented by one from its initial value 
    //   of 256, so that existing IO pipelines will continue to work).
    //
    void* _extra_data[255];
};

#endif // _XIL_DEVICE_IO_HH
