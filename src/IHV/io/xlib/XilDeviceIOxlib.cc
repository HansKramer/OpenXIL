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
//  File:	XilDeviceIOxlib.cc
//  Project:	XIL
//  Revision:	1.53
//  Last Mod:	10:13:49, 03/10/00
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
//  MT-level:  <SAFE>
//	
//------------------------------------------------------------------------
//	COPYRIGHT
//------------------------------------------------------------------------
#pragma ident	"@(#)XilDeviceIOxlib.cc	1.53\t00/03/10  "

#ifndef SOLARIS
#define _XOPEN_SOURCE 600
#endif

#include <stdio.h>
#include <string.h>
#include "XilDeviceIOxlib.hh"
#include "XiliUtils.hh"
#include <pthread.h>
#include <malloc.h>


#ifdef SOLARIS
//
//  There isn' a prototype declared for
//  this function, in Solaris 2.5.1 and below.
//
#if XlibSpecificationRelease < 6
extern "C" {
    Status XSolarisGetVisualGamma(Display* display,
                                  int      screen_number,
                                  Visual*  visual,
                                  double*  gamma);
}
#else
#include <X11/Xmu/XmuSolaris.h>
#endif
#endif // SOLARIS

#if defined(X_NOT_MT_SAFE)
XilMutex XilDeviceIOxlib::XMutex;
#endif

//
//  In this constructor we just save the things we
//  need. The core will call us when they want the
//  controlling image.
//
XilDeviceIOxlib::XilDeviceIOxlib(XilDeviceManagerIO* device_manager,
                                 XilSystemState*     state,
                                 Display*            dpy,
                                 Window              win) :
    XilDeviceIO(device_manager), regionData(64)
{
    stateptr = state;
    displayptr = dpy;
    window = win;
    controllingImage = NULL;
    deviceLookup = NULL;
    parentImage = NULL;
    ximage = NULL;
    gc = NULL;
    colorspace = NULL;
    destroyColorspace = FALSE;
    allocXImageData = FALSE;
    ximagePS = 4;

    //
    //  Initialize cast1->8 variables
    //
    bitXImage = NULL;

    //
    //  Offscreen Buffer
    //
    offScreenBuffer = NULL;
    setDoubleBuffered(FALSE);
    nRegions        = 0;
}

//
//  Destructor, clean up things we created.
//
//  The core cleans up the images we created so we don't destroy them here.
//
XilDeviceIOxlib::~XilDeviceIOxlib()
{
    if(destroyColorspace) {
        colorspace->destroy();
    }

    if(gc) {
        lockX();
        XFreeGC(displayptr, gc);
        unlockX();
    }

    if(ximage) {
        if (allocXImageData) {
            // Free data we allocated for image reformatting
            // HK: nope... XDestroyImage will do this for you, see man page
            // free(ximage->data);
        } else {
            ximage->data = NULL;
        }
        lockX();
        XDestroyImage(ximage);
        unlockX();
    }

    if (offScreenBuffer) {
        XFreePixmap(displayptr, offScreenBuffer);
        offScreenBuffer = NULL;
    }

    if(bitXImage) {
        lockX();
        XFreeGC(displayptr, castGC);
        XFreePixmap(displayptr, bitPixmap);
        bitXImage->data = NULL;
        XDestroyImage(bitXImage);
        unlockX();
    }
}


Xil_boolean
XilDeviceIOxlib::isDoubleBuffered()
{
    return doubleBuffered;
}


void
XilDeviceIOxlib::setDoubleBuffered(Xil_boolean flag)
{
    doubleBuffered = flag;
}


//
// Create the image associated with our device.
//
XilImage*
XilDeviceIOxlib::constructControllingImage()
{
    //
    // find out the window information
    //
    Window root;
    int    x,y;
    unsigned int width, height;
    unsigned int border_width;

    lockX();
    Status status=XGetGeometry(displayptr, window,&root, &x, &y, &width,
                               &height, &border_width, &x_depth);
    unlockX();
    if(status==0) {
        XIL_ERROR(stateptr, XIL_ERROR_SYSTEM,"di-220",TRUE);
        return NULL;
    }

    //
    //  Create a graphics context
    //
    XGCValues gc_val;
    gc_val.foreground = 0;
    gc_val.function   = GXcopy;
    lockX();
    gc = XCreateGC(displayptr,this->window, GCForeground|GCFunction, &gc_val);
    unlockX();

    if(gc == NULL) {
        XIL_ERROR(stateptr, XIL_ERROR_SYSTEM, "di-162", TRUE);
        return NULL;
    }

    //
    //  Create the X image
    //
    lockX();
    if(!XGetWindowAttributes(displayptr, window, &window_attr)) {
        XIL_ERROR(stateptr, XIL_ERROR_SYSTEM, "di-249", TRUE);
        XFreeGC(displayptr, gc);
        unlockX();
        return NULL;
    }
    unlockX();
   
    lockX();
    ximage= XCreateImage(displayptr,
                         window_attr.visual,
                         window_attr.depth,
                         ZPixmap,
                         0,0, width, height, 8, 0);
    if(ximage == NULL) {
        XIL_ERROR(stateptr, XIL_ERROR_SYSTEM, "di-250", TRUE);
        XFreeGC(displayptr, gc);
        unlockX();
        return NULL;
    }
    unlockX();

    //
    //  Determine LSB positions of each of the RGB fields
    //
    rLSB = findLSB(ximage->red_mask);
    gLSB = findLSB(ximage->green_mask);
    bLSB = findLSB(ximage->blue_mask);

    //
    //  Determine # of bits to trim from each of the RGB values
    //
    rTrim = 8 - countBits(ximage->red_mask, rLSB); 
    gTrim = 8 - countBits(ximage->green_mask, gLSB); 
    bTrim = 8 - countBits(ximage->blue_mask, bLSB); 

    //
    //  At this point, ximage->bits_per_pixel and ximage->byte_order
    //  reflect the values from the X server.
    //
    switch(ximage->bits_per_pixel) {
      case 1:
        controllingImage = stateptr->createXilImage(width, height, 1, XIL_BIT);
        break;
      case 8:
        controllingImage = stateptr->createXilImage(width, height, 1, XIL_BYTE);
        break;

      case 16:
        controllingImage = stateptr->createXilImage(width, height, 1, XIL_SHORT);
        break;

      case 24:
      case 32:
        controllingImage = stateptr->createXilImage(width, height, 3, XIL_BYTE);

        //
        // Allocate a data buffer for the XImage, rather than assign
        // it the image's storage. The Xlib conversion cost when
        // the image's bits_per_pixel doesn't match that of the XImage
        // is too high. We'll do the conversion ourselve's instead.
        //
        ximage->data = (char*)memalign(4, height * ximage->bytes_per_line);
        if(ximage->data == NULL) {
            XIL_ERROR(stateptr, XIL_ERROR_RESOURCE, "di-1", TRUE);
            lockX();
            XDestroyImage(ximage);
            XFreeGC(displayptr, gc);
            unlockX();
            return NULL;
        }
        allocXImageData = TRUE;
        ximagePS = ximage->bits_per_pixel/8;
        break;

      default:
        lockX();
        XDestroyImage(ximage);
        XFreeGC(displayptr, gc);
        unlockX();
        return NULL;
    }


    //
    //  Check for a NULL controlling image
    //
    if(controllingImage == NULL) {
        lockX();
        XDestroyImage(ximage);
        XFreeGC(displayptr, gc);
        unlockX();
        return NULL;
    }

    if (doubleBuffered) 
        offScreenBuffer = XCreatePixmap(displayptr, window, width, height, 24);

    return controllingImage;
}
      
XilStatus
XilDeviceIOxlib::display(XilOp*       op,
                         unsigned int ,
                         XilRoi*      roi,
                         XilBoxList*  bl)
{
    //
    //  Split the list of XilBoxes to take tile boundaries into account.  This
    //  will work to ensure that no cobbling of the data is required because
    //  the boxes will not cross tile boundaries in the source images.
    //
    if(op->splitOnTileBoundaries(bl) == XIL_FAILURE) {
        return XIL_FAILURE;
    }
    
    //
    //  Get the image passed to us on the op
    //
    XilImage*    srcImage = op->getSrcImage(1);

    //
    //  Get information about the input image
    //
    unsigned int   nbands = srcImage->getNumBands();
    XilDataType    data_type = srcImage->getDataType();
    XilStorageType storage_type = data_type == XIL_BIT ?
        XIL_BAND_SEQUENTIAL : XIL_PIXEL_SEQUENTIAL;
    
    //
    //  A local copy of the XImage used only by this display operation.
    //
    XImage x_image = *ximage;

    //
    //  Loop over each of the boxes of storage we are to process.
    //
    XilBox* src_box;
    XilBox* dst_box;
    while(bl->getNext(&src_box, &dst_box)) {
        //
        //  Aquire our storage from the images.
        //
        XilStorage  src_storage(srcImage);

        //
        // We always ask for data in band/pixel sequential form. The
        // storage device can convert the data format faster
        // than trying to reformat it here.
        //
        if(srcImage->getStorage(&src_storage, op, src_box, "XilMemory",
                                XIL_READ_ONLY, storage_type) == XIL_FAILURE) {
            //
            //  Mark this box as failed and if that succeeds, continue
            //  processing the next box.  Otherwise, return XIL_FAILURE now.
            //
            if(bl->markAsFailed() == XIL_FAILURE) {
                return XIL_FAILURE;
            } else {
                continue;
            }
        }

        //
        // Get the co-ordinates of the box, we need to adjust to
        // absolute image co-ordinates in the destination.
        //
        int          dst_x,src_x;
        int          dst_y,src_y;
        unsigned int abs_w;
        unsigned int abs_h;
        src_box->getAsRect(&src_x, &src_y, &abs_w, &abs_h);
        dst_box->getAsRect(&dst_x, &dst_y, &abs_w, &abs_h);

        //
        // ximage's are pixel sequential and so as we asked for
        // pixel sequential storage this is ok.
        //
        unsigned int   src_pixel_stride;
        unsigned int   src_scanline_stride;
        Xil_unsigned8* src_data;
        src_storage.getStorageInfo(&src_pixel_stride,
                                   &src_scanline_stride,
                                   NULL, NULL,
                                   (void**)&src_data);

        //
        //  Set the source data information into the XImage structure.
        //  For this case, the src image is the controlling image.
        //  For 24 bpp and 32 bpp displays, reformat the image
        //  into the xImage rather than using the image storage itself.
        //  Xlib bpp conversion is too expensive (uses getPixel/setPixel).
        //
        unsigned char* pSrc;
        switch(data_type) {
          case XIL_BIT:
            x_image.data           = (char*)src_data;
            x_image.bytes_per_line = (int)src_scanline_stride;
            break;

          case XIL_BYTE:
            if (allocXImageData) {
                // 
                // Use the allocated XImage storage for this case
                //
                pSrc = src_data;
                x_image.data += dst_y*x_image.bytes_per_line +
                                dst_x*ximagePS;
                reformatToXImage(pSrc, src_pixel_stride, src_scanline_stride,
                                 (unsigned char*)x_image.data, 
                                 x_image.bytes_per_line, abs_w, abs_h);
            } else {
                x_image.data           = (char*)src_data;
                x_image.bytes_per_line = (int)src_scanline_stride;
            }
            break;

          case XIL_SHORT:
            x_image.data           = (char*)src_data;
            x_image.bytes_per_line = (int)src_scanline_stride*sizeof(short);
            break;

          case XIL_FLOAT:
            return XIL_FAILURE;
        }

        //
        // Loop over the rectangles in the ROI
        // all of the regions are guaranteed not to go outside
        // the image
        //
        XilRectList  rl(roi, dst_box);

        int            x;
        int            y;
        unsigned int   x_size;
        unsigned int   y_size;
 
        int index = nRegions;
        nRegions += rl.getNumRects();
        if (regionData.size() < nRegions)
            regionData.resize(nRegions);

        while(rl.getNext(&x, &y, &x_size, &y_size)) {
            lockX();
            if (doubleBuffered) {
                regionData[index].src_x  = x+src_x;
                regionData[index].src_y  = y+src_y;
                regionData[index].dst_x  = x+dst_x;
                regionData[index].dst_y  = y+dst_y;
                regionData[index].x_size = x_size;
                regionData[index].y_size = y_size;

                XPutImage(displayptr, offScreenBuffer, gc, &x_image, x, y,
                          (int)x+dst_x, (int)y+dst_y, x_size, y_size);
                index++;
            } else 
                XPutImage(displayptr, window, gc, &x_image, x, y,
                          (int)x+dst_x, (int)y+dst_y, x_size, y_size);
            unlockX();
        }
    }
        
    lockX();
    XFlush(displayptr);
    unlockX();

    return XIL_SUCCESS;
}


#include <stdio.h>
XilStatus
XilDeviceIOxlib::swapBuffers()
{
    if (! doubleBuffered)
        return XIL_SUCCESS;

    if (nRegions == 0)
        return XIL_SUCCESS;

    lockX();
    for (int index=0; index<nRegions; index++) {
            XCopyArea(displayptr, offScreenBuffer, window, gc,
                      regionData[index].src_x,
                      regionData[index].src_y,
                      regionData[index].x_size,
                      regionData[index].y_size,
                      regionData[index].dst_x,
                      regionData[index].dst_y
            );

    }
    unlockX();

    nRegions = 0;

    return XIL_SUCCESS;
}


XilStatus
XilDeviceIOxlib::capture(XilOp*       op,
                         unsigned int ,
                         XilRoi*      roi,
                         XilBoxList*  bl)
{
    //
    // Get the image passed to us on the op
    //
    XilImage*    dstImage  = op->getDstImage(1);

    //
    // If we can't get the window attributes and its not
    // mapped just return.
    //
    XWindowAttributes win_info;
    
    lockX();
    if(!XGetWindowAttributes(displayptr, window, &win_info)) {
        XIL_ERROR(dstImage->getSystemState(),XIL_ERROR_SYSTEM,"di-249",TRUE);
        unlockX();
        return XIL_FAILURE;
    }
    unlockX();

    if((win_info.map_state == IsUnmapped) ||
       (win_info.map_state == IsUnviewable)) {
        return XIL_SUCCESS;
    }

    //
    //  Get information about the input image
    //
    unsigned int   nbands = dstImage->getNumBands();
    XilDataType    data_type = dstImage->getDataType();
    XilStorageType storage_type = data_type == XIL_BIT ?
        XIL_BAND_SEQUENTIAL : XIL_PIXEL_SEQUENTIAL;
    
    //
    //  Split the list of XilBoxes to take tile boundaries into account.  This
    //  will work to ensure that no cobbling of the data is required because
    //  the boxes will not cross tile boundaries in the source images.
    //
    if(op->splitOnTileBoundaries(bl) == XIL_FAILURE) {
        return XIL_FAILURE;
    }

    //
    //  Determine the visible portion of the screen
    //
    lockX();
    int screen = XScreenNumberOfScreen(win_info.screen);
    unlockX();

    //
    //  Upper left corner relative to the root window
    //
    int    wabs_x;
    int    wabs_y;
    Window dwin;
    lockX();
    if(!XTranslateCoordinates(displayptr, window,
                              RootWindow(displayptr, screen),
                              0, 0, &wabs_x, &wabs_y, &dwin)) {
        XIL_ERROR(dstImage->getSystemState(), XIL_ERROR_SYSTEM, "di-249", TRUE);
        unlockX();
        return XIL_FAILURE;
    }
    unlockX();

    //
    //  Setup window information
    //
    win_info.x = wabs_x;
    win_info.y = wabs_y;
    unsigned int clipwidth = win_info.width;
    unsigned int clipheight = win_info.height;

    //
    //  Screen width height
    //
    int dwidth = DisplayWidth(displayptr, screen);
    int dheight = DisplayHeight(displayptr, screen);

    //
    //  Adjust width, height start coordinate
    //
    if(wabs_x < 0) {
        clipwidth += wabs_x;
        wabs_x = 0;
    }
    if(wabs_y < 0) {
        clipwidth += wabs_y;
        wabs_y = 0;
    }
    if((wabs_x + clipwidth) > dwidth) {
        clipwidth = dwidth - wabs_x;
    }
    if((wabs_y + clipheight) > dheight) {
        clipheight = dheight - wabs_y;
    }

    int clipx = wabs_x - win_info.x;
    int clipy = wabs_y - win_info.y;

    //
    //  Create a rectlist for use in the while box's loop
    //  This constrains the capture to the visible area of
    //  the image.
    //
    XilRectList crl(roi, clipx, clipy, clipwidth, clipheight);

    //
    //  A local copy of the XImage used only by this display operation.
    //
    XImage x_image = *ximage;

    //
    //  Loop over each of the boxes of storage we are to process.
    //
    XilBox*    src_box;
    XilBox*    dst_box;
    while(bl->getNext(&src_box, &dst_box)) {
        //
        //  Aquire our storage from the images.
        //
        XilStorage  dst_storage(dstImage);

        //
        // Always asking for BAND/PIXEL_SEQUENTIAL is quicker than trying
        // to handle the other storage types. As X data is inherently
        // PIXEL_SEQUENTIAL
        //
        if(dstImage->getStorage(&dst_storage, op, dst_box, "XilMemory",
                                XIL_READ_ONLY, storage_type) == XIL_FAILURE) {
            //
            //  Mark this box as failed and if that succeeds, continue
            //  processing the next box.  Otherwise, return XIL_FAILURE now.
            //
            if(bl->markAsFailed() == XIL_FAILURE) {
                return XIL_FAILURE;
            } else {
                continue;
            }
        }

        //
        // Get the co-ordinates of the box, we need to adjust to
        // absolute image co-ordinates in the destination.
        //
        int          dst_x;
        int          dst_y;
        unsigned int abs_w;
        unsigned int abs_h;
        src_box->getAsRect(&dst_x, &dst_y, &abs_w, &abs_h);

        //
        // Needs to be done on the current box
        //
        unsigned int   dst_pixel_stride;
        unsigned int   dst_scanline_stride;
        Xil_unsigned8* dst_data;

        dst_storage.getStorageInfo(&dst_pixel_stride,
                                   &dst_scanline_stride,
                                   NULL, NULL,
                                   (void**)&dst_data);

        Xil_unsigned8* pDst;
        switch(data_type) {
          case XIL_BIT:
            x_image.data           = (char*)dst_data;
            x_image.bytes_per_line = (int)dst_scanline_stride;
            break;
        
          case XIL_BYTE:

            if (allocXImageData) {
                //
                // Use the storage allocated for the XImage
                //
                x_image.data += dst_y*x_image.bytes_per_line +
                                dst_x*ximagePS;
                               
            } else {
                //
                // Use the storage of the dst image
                //
                x_image.data           = (char*)dst_data;
                x_image.bytes_per_line = (int)dst_scanline_stride;
            }
            break;


          case XIL_SHORT:
            x_image.data           = (char*)dst_data;
            x_image.bytes_per_line =
                (int)dst_scanline_stride*sizeof(Xil_signed16); 
            break;

          case XIL_FLOAT:
            return XIL_FAILURE;
        }

        //
        // Loop over the rectangles in the ROI
        // all of the regions are guaranteed not to go outside
        // the image
        //
        XilRectList  rl(&crl, src_box);

        int          roi_x;
        int          roi_y;
        unsigned int x_size;
        unsigned int y_size;
        while(rl.getNext(&roi_x, &roi_y, &x_size, &y_size)) {
            lockX();
            XGetSubImage(displayptr, window,
                         (roi_x+dst_x), (roi_y+dst_y), x_size, y_size,
                         0xFFFFFFFF, ZPixmap, &x_image, roi_x, roi_y);
            unlockX();
        }

        lockX();
        //XFlush(displayptr);
        //XSync(displayptr, FALSE);
        unlockX();


        if (allocXImageData) {
            pDst = dst_data;
            reformatFromXImage((unsigned char*)x_image.data, 
                               x_image.bytes_per_line,
                               pDst, dst_pixel_stride, 
                               dst_scanline_stride, abs_w, abs_h);
        }
    }

    return XIL_SUCCESS;
}

//
// Check code for ROI setup
//
XilStatus
XilDeviceIOxlib::getPixel(unsigned int x,
                          unsigned int y,
                          float*       values,
                          unsigned int offset_band,
                          unsigned int nbands)
{
    offset_band = offset_band;

    //
    //  If we can't get the window attributes and its not
    //  mapped just return.
    //
    XWindowAttributes win_info;
    
    lockX();
    if(!XGetWindowAttributes(displayptr, window, &win_info)) {
        XIL_ERROR(stateptr,XIL_ERROR_SYSTEM,"di-249",TRUE);
        unlockX();
        return XIL_FAILURE;
    }
    unlockX();

    if((win_info.map_state == IsUnmapped) ||
       (win_info.map_state == IsUnviewable)) {
        return XIL_SUCCESS;
    }

    //
    //  Determine the visible portion of the screen
    //
    lockX();
    int screen = XScreenNumberOfScreen(win_info.screen);
    unlockX();

    //
    //  Upper left corner relative to the root window
    //
    int    wabs_x;
    int    wabs_y;
    Window dwin;
    lockX();
    if(!XTranslateCoordinates(displayptr, window,
                              RootWindow(displayptr, screen),
                              0, 0, &wabs_x, &wabs_y, &dwin)) {
        XIL_ERROR(stateptr, XIL_ERROR_SYSTEM, "di-249", TRUE);
        unlockX();
        return XIL_FAILURE;
    }
    unlockX();

    //
    //  Setup window information
    //
    win_info.x     = wabs_x;
    win_info.y     = wabs_y;
    int clipwidth  = win_info.width;
    int clipheight = win_info.height;

    //
    //  Screen width height
    //
    int dwidth  = DisplayWidth(displayptr, screen);
    int dheight = DisplayHeight(displayptr, screen);

    //
    //  Adjust width, height start coordinate
    //
    if(wabs_x < 0) {
        clipwidth += wabs_x;
        wabs_x = 0;
    }
    if(wabs_y < 0) {
        clipwidth += wabs_y;
        wabs_y = 0;
    }
    if((wabs_x + clipwidth) > dwidth) {
        clipwidth = dwidth - wabs_x;
    }
    if((wabs_y + clipheight) > dheight) {
        clipheight = dheight - wabs_y;
    }

    int clipx = wabs_x - win_info.x;
    int clipy = wabs_y - win_info.y;

    if(((int)x <  clipx)             ||
       ((int)x >= (clipx+clipwidth)) ||
       ((int)y <  clipy)             ||
       ((int)y >= (clipy+clipheight))) {
        //
        //  Pixel isn't on the screen.
        //
        //  API says that the values of occluded windows is undefined so we
        //  don't touch the values.
        //
    } else {
        //
        //  Get a single pixel image
        //
        lockX();
        XImage* image = XGetImage(displayptr, window,
                                  x, y, 1, 1, 0xFFFFFF, ZPixmap);
        unlockX();
        if(image == NULL) {
            XIL_ERROR(stateptr, XIL_ERROR_SYSTEM, "di-316", TRUE);
            return XIL_FAILURE;
        }

        //
        //  Get the pixel data from the X image.
        //
        //  SOL64:  XGetPixel returns a u_long we cast here
        //          to remove use of long for Solaris64
        //
        lockX();
        unsigned int pixel = (unsigned int)XGetPixel(image, 0, 0);
        XDestroyImage(image);
        unlockX();

        //
        //  Set up the return value(s)
        //
        switch(x_depth) {
          case 1:
          case 4:
          case 8:
          case 16:
            values[0] = (float)pixel;
            break;
          case 24:
            for (int i=0; i<nbands; i++) {
                switch(i) {
                  case 0:
                    values[0] = (float)((pixel >> bLSB) & 0xff);
                    break;
                  case 1:
                    values[1] = (float)((pixel >> gLSB) & 0xff);
                    break;
                  case 2:
                    values[2] = (float)((pixel >> rLSB) & 0xff);
                    break;
                }
            }
          break;
        }
    }

    return XIL_SUCCESS;
}

   
//
//  Set the pixel
//
XilStatus
XilDeviceIOxlib::setPixel(unsigned int x,
                          unsigned int y,
                          float*       values,
                          unsigned int offset_band,
                          unsigned int nbands)
{
    unsigned int pixel;
    unsigned int mask;

    offset_band = offset_band;

    switch(x_depth) {
      case 1:
        if(values[0] < 0.5) {
            pixel = 0;
        } else {
            pixel = 1;
        }
        mask = 1;
        break;

      case 4:
      case 8:
        pixel = _XILI_ROUND_U8(values[0]);
        mask  = 0xFF;
        break;

      case 16:
        pixel = _XILI_ROUND_U16(values[0]);
        mask  = 0xFFFF;
        break;

      case 24:
        pixel = 0;
        mask  = 0;

        //
        //  This *should* always be 3 bands, but we can't
        //  necessarily count on it. So check the band.
        //
        for(int i=0; i<nbands; i++) {
            switch(i) {
              case 0:
                pixel |= (_XILI_ROUND_U8(values[0])) << bLSB;
                mask |= ximage->blue_mask;
                break;

              case 1:
                pixel |= (_XILI_ROUND_U8(values[1])) << gLSB;
                mask |= ximage->green_mask;
                break;

              case 2:
                pixel |= (_XILI_ROUND_U8(values[2])) << rLSB;
                mask |= ximage->red_mask;
                break;
            }
        }
        break;
    }

    lockX();
    XSetPlaneMask(displayptr, gc, mask);
    XSetForeground(displayptr, gc, pixel);
    XDrawPoint(displayptr, window, gc, x, y);
    XSetPlaneMask(displayptr, gc, 0xFFFFFFFF);
    unlockX();

    return XIL_SUCCESS;
}

XilStatus
XilDeviceIOxlib::setAttribute(const char* attribute_name,
                  void*       value)
{
    if(!strcmp(attribute_name,"XCOLORMAP")) {
        XilColorList* clist = (XilColorList*)value;
    
        lockX();
        XStoreColors(displayptr, clist->cmap, clist->colors, clist->ncolors);
        XFlush(displayptr);
        unlockX();

        return XIL_SUCCESS;
    }

    return XIL_FAILURE;   
}

XilStatus
XilDeviceIOxlib::getAttribute(const char*  attribute_name,
                                    void** value)
{
    //
    //  These are required I/O Device Attributes.
    //
    if(!strcmp(attribute_name, "WINDOW")) {
        *value = (void*)window;
    } else if(!strcmp(attribute_name, "DISPLAY")) {
        *value = (void*)displayptr;
    } else if(!strcmp(attribute_name, "DRAWABLE")) {
        if (doubleBuffered)
            *value = (void*) offScreenBuffer;
        else
            *value = (void*) window;
    } else if(!strcmp(attribute_name, "COLORSPACE")) {
        //
        //  Get a colorspace object that represents our device.
        //
        mutex.lock();

        if(colorspace != NULL) {
            *value = (void*)colorspace;

            mutex.unlock();

            return XIL_SUCCESS;
        }

#ifdef SOLARIS
        //
        //  This is a utility constructor that checks for
        //  the presence of an ICC profile for the particular
        //  visual we are interested in
        //
        int screen = DefaultScreen(displayptr);

        colorspace = stateptr->createXilColorspace(displayptr, screen,
                                                   window_attr.visual);
        if(colorspace == NULL) {
            //
            //  No colorspace set on the window create a default
            //  check the gamma value to determine whether to use
            //  the linear version of the colorspace
            //
            double gamma;
            XSolarisGetVisualGamma(displayptr, screen, window_attr.visual,
                                   &gamma);

            if(XILI_FLT_EQ(gamma, 1.0)) {
                colorspace = (XilColorspace*)stateptr->getObjectByName("rgblinear",
                                                                       XIL_COLORSPACE);
            } else {
                colorspace = (XilColorspace*)stateptr->getObjectByName("rgb709",
                                                                       XIL_COLORSPACE);
            }
        } else {
            //
            //  Indicate we didn't get it "by name" so destroy it when we go away...
            //
            destroyColorspace = TRUE;
        }
#else
        colorspace = (XilColorspace*)stateptr->getObjectByName("rgb709",
                                                               XIL_COLORSPACE);
#endif

        *value = (void*)colorspace;

        mutex.unlock();
    } else if(!strcmp(attribute_name, "COLORMAP")) {
        //
        //  Return a lookup that represents the layout of the lookup
        //  associated with the device.  For 3 banded images, this is
        //  meaningless.
        //
        // NOTE! Access to Xlib in the block below is already serialized
        // by holding the colormap mutex; don't need to lockX()
        if(controllingImage->getNumBands() == 1) {
            mutex.lock();

            if(deviceLookup == NULL) {
                //
                //  Create a lookup that represents the layout of an
                //    X Colormap.
                //
                XilSystemState* state = controllingImage->getSystemState();
                deviceLookup =
                    state->createXilLookupSingle(XIL_BYTE, XIL_BYTE,
                                                 3, 256, 0, lookupData);
            }

            XWindowAttributes win_attr;
            if(!XGetWindowAttributes(displayptr, window, &win_attr)) {
                XIL_ERROR(controllingImage->getSystemState(),
                          XIL_ERROR_SYSTEM, "di-249", TRUE);
                mutex.unlock();
                return XIL_FAILURE;
            }

            int i, j;
            XColor colors[256];
            for(i=0; i<256; i++) {
                colors[i].pixel = i;
            }

            XQueryColors(displayptr, win_attr.colormap, colors, 256);

            for(i=0, j=0; i<256; i++, j+=3) {
                lookupData[j]   = colors[i].blue  >> 8;
                lookupData[j+1] = colors[i].green >> 8;
                lookupData[j+2] = colors[i].red   >> 8;
            }

            deviceLookup->setValues(0, 256, lookupData);

            mutex.unlock();

            *value = (void*)deviceLookup;
        } else {
            *value = (void*)NULL;
        }
    } else {
        return XIL_FAILURE;
    }
    
    return XIL_SUCCESS;
}

Xil_boolean
XilDeviceIOxlib::isReadable()
{
    return TRUE;
}

Xil_boolean
XilDeviceIOxlib::isWritable()
{
    return TRUE;
}

//------------------------------------------------------------------------
//
//  Function:    Xlib Molecules
//
//  Description:
//    
//    Molecules for operations->display
//    
//    
//    
//    
//  MT-level:  SAFE
//    
//------------------------------------------------------------------------
XilStatus
XilDeviceIOxlib::copyDisplayPreprocess(XilOp*        op,
                                       unsigned int  ,
                                       XilRoi*       ,
                                       void**        ,
                                       unsigned int* )
{
    //
    //  We don't handle copying images with child band offsets yet.
    //
    unsigned int num_disp_bands;
    op->getParam(1, &num_disp_bands);

    unsigned int child_band_offset;
    op->getParam(2, &child_band_offset);

    if(controllingImage->getNumBands() != num_disp_bands ||
       child_band_offset               != 0) {
        return XIL_FAILURE;
    }

    return XIL_SUCCESS;
}

XilStatus
XilDeviceIOxlib::copyDisplay(XilOp*       op,
                             unsigned int ,
                             XilRoi*      roi,
                             XilBoxList*  bl)
{
    //
    //  Split the list of XilBoxes to take tile boundaries into account.  This
    //  will work to ensure that no cobbling of the data is required because
    //  the boxes will not cross tile boundaries in the source images.
    //
    if(op->splitOnTileBoundaries(bl) == XIL_FAILURE) {
        return XIL_FAILURE;
    }
    
    //
    //  Get the source image passed to us on the op...
    //
    XilOp*    srcOp    = op->getOpList()[1];
    XilImage* srcImage = srcOp->getSrcImage(1);

    //
    //  Get information about the input image
    //
    unsigned int   nbands = srcImage->getNumBands();
    XilDataType    data_type = srcImage->getDataType();
    XilStorageType storage_type = data_type == XIL_BIT ?
        XIL_BAND_SEQUENTIAL : XIL_PIXEL_SEQUENTIAL;
    
    //
    //  A local copy of the XImage used only by this display operation.
    //
    XImage x_image = *ximage;

    //
    //  Loop over each of the boxes of storage we are to process.
    //
    XilBox* src_box;
    XilBox* dst_box;
    while(bl->getNext(&src_box, &dst_box)) {
        //
        //  Aquire our storage from the images.
        //
        XilStorage  src_storage(srcImage);

        //
        //  We always ask for data in band/pixel sequential form. The
        //  storage device can convert the data format faster
        //  than trying to reformat it here.
        //
        if(srcImage->getStorage(&src_storage, srcOp, src_box, "XilMemory",
                                XIL_READ_ONLY, storage_type) == XIL_FAILURE) {
            //
            //  Mark this box as failed and if that succeeds, continue
            //  processing the next box.  Otherwise, return XIL_FAILURE now.
            //
            if(bl->markAsFailed() == XIL_FAILURE) {
                return XIL_FAILURE;
            } else {
                continue;
            }
        }

        //
        // Get the co-ordinates of the box, we need to adjust to
        // absolute image co-ordinates in the destination.
        //
        int          dst_x;
        int          dst_y;
        unsigned int abs_w;
        unsigned int abs_h;
        dst_box->getAsRect(&dst_x, &dst_y, &abs_w, &abs_h);

        //
        // ximage's are pixel sequential and so as we asked for
        // pixel sequential storage this is ok.
        //
        unsigned int   src_pixel_stride;
        unsigned int   src_scanline_stride;
        Xil_unsigned8* src_data;
        src_storage.getStorageInfo(&src_pixel_stride,
                                   &src_scanline_stride,
                                   NULL, NULL,
                                   (void**)&src_data);

        //
        //  Set the source data information into the ximage structure.
        //  For this case, the src image is the src for the copy into
        //  the controlling image, not the controlling image itself.
        //
        unsigned char* pSrc;
        switch(data_type) {
          case XIL_BIT:
            x_image.data           = (char*)src_data;
            x_image.bytes_per_line = (int)src_scanline_stride;
            break;

          case XIL_BYTE:
            if (allocXImageData) {
                pSrc = src_data;
                x_image.data += dst_y*x_image.bytes_per_line +
                                dst_x*ximagePS;

                reformatToXImage(pSrc, src_pixel_stride, src_scanline_stride,
                                 (unsigned char*)x_image.data,
                                 x_image.bytes_per_line, abs_w, abs_h);
            } else {
                x_image.data           = (char*)src_data;
                x_image.bytes_per_line = (int)src_scanline_stride;
            }
            break;

          case XIL_SHORT:
            //
            //  XPutImage doesn't know how to handle non-contiguous data.
            //
            if(src_pixel_stride != nbands) {
                //
                //  Mark this box as failed and if that succeeds, continue
                //  processing the next box.  Otherwise, return XIL_FAILURE now.
                //
                if(bl->markAsFailed() == XIL_FAILURE) {
                    return XIL_FAILURE;
                } else {
                    continue;
                }
            }

            x_image.data           = (char*)src_data;
            x_image.bytes_per_line = (int)src_scanline_stride*sizeof(short);
            break;

          case XIL_FLOAT:
            return XIL_FAILURE;
        }

        //
        // Loop over the rectangles in the ROI
        // all of the regions are guaranteed not to go outside
        // the image
        //
        XilRectList  rl(roi, dst_box);

        int          x;
        int          y;
        unsigned int x_size;
        unsigned int y_size;
        while(rl.getNext(&x, &y, &x_size, &y_size)) {
            lockX();
            XPutImage(displayptr, window, gc, &x_image, x, y,
                      (int)x+dst_x, (int)y+dst_y, x_size, y_size);
            unlockX();
        }
    }

    lockX();
    XFlush(displayptr);
    unlockX();

    return XIL_SUCCESS;
}

XilStatus
XilDeviceIOxlib::setValueDisplayPreprocess(XilOp*        op,
                                           unsigned int  ,
                                           XilRoi*       ,
                                           void**        ,
                                           unsigned int* )
{
    //
    //  We don't handle images with child band offsets yet.
    //
    unsigned int num_disp_bands;
    op->getParam(1, &num_disp_bands);

    unsigned int child_band_offset;
    op->getParam(2, &child_band_offset);

    if(controllingImage->getNumBands() != num_disp_bands ||
       child_band_offset               != 0 ||
       controllingImage->getDataType() != XIL_BYTE) {
        return XIL_FAILURE;
    }

    return XIL_SUCCESS;
}

XilStatus
XilDeviceIOxlib::setValueDisplay(XilOp*       op,
                                 unsigned int ,
                                 XilRoi*      roi,
                                 XilBoxList*  bl)
{
    //
    //  Get the set_value colors...
    //
    XilOp* srcOp = op->getOpList()[1];

    //
    //  Get information about the output image
    //
    unsigned int   nbands = controllingImage->getNumBands();
    XilDataType    data_type = controllingImage->getDataType();

    //
    //  Get the values...
    //
    Xil_unsigned8* values;
    srcOp->getParam(1, (void**)&values);

    //
    //  Determine the color to set the GC.
    //
    unsigned long color = 0;

    if (x_depth == 24) {
        for(int i=0; i<nbands; i++) {
            switch(i) {
              case 0:
                color |= values[0] << bLSB;
                break;
              case 1:
                color |= values[1] << gLSB;
                break;
              case 2:
                color |= values[2] << rLSB;
                break;
            }
        }

    } else {
        color = values[0];
    }

    lockX();
    XSetForeground(displayptr, gc, color);
    unlockX();

    //
    //  Loop over each of the boxes of storage we are to process.
    //
    XilBox* dst_box;
    while(bl->getNext(&dst_box)) {
        //
        // Get the co-ordinates of the box, we need to adjust to
        // absolute image co-ordinates in the destination.
        //
        int          dst_x;
        int          dst_y;
        unsigned int abs_w;
        unsigned int abs_h;
        dst_box->getAsRect(&dst_x, &dst_y, &abs_w, &abs_h);

        //
        // Loop over the rectangles in the ROI
        // all of the regions are guaranteed not to go outside
        // the image
        //
        XilRectList  rl(roi, dst_box);

        int          x;
        int          y;
        unsigned int x_size;
        unsigned int y_size;
        while(rl.getNext(&x, &y, &x_size, &y_size)) {
            lockX();
            XFillRectangle(displayptr, window, gc,
                           (int)x+dst_x, (int)y+dst_y, x_size, y_size);
            unlockX();
        }
    }

    lockX();
    XFlush(displayptr);
    unlockX();

    return XIL_SUCCESS;
}

XilStatus
XilDeviceIOxlib::cast1to8DisplayPreprocess(XilOp*        op,
                                           unsigned int  ,
                                           XilRoi*       ,
                                           void**        ,
                                           unsigned int* )
{
    unsigned int ci_nbands = controllingImage->getNumBands();

    //
    //  Only 1 banded bit to 1 banded byte.
    //
    if(ci_nbands != 1) {
        return XIL_FAILURE;
    }

    //
    //  We don't handle casting images with child band offsets yet.
    //
    unsigned int num_disp_bands;
    op->getParam(1, &num_disp_bands);

    unsigned int child_band_offset;
    op->getParam(2, &child_band_offset);

    if(ci_nbands         != num_disp_bands ||
       child_band_offset != 0) {
        return XIL_FAILURE;
    }

    //
    //  If it's not already create, create the bit image (size of the
    //  controlling image) and pixmap that will hold the bit image
    //  representation as well as the graphics context for copying the 1-bit
    //  data to 8-bit data.
    //
    if(bitXImage == NULL) {
        unsigned int width  = controllingImage->getWidth();
        unsigned int height = controllingImage->getHeight();
        
        //
        //  Create a 1-bit XImage for the src data
        //
        lockX();
        bitXImage = XCreateImage(displayptr, visual, 
                                 1, XYBitmap,
                                 0, 0, width, height, 8, width);
        unlockX();
        if(bitXImage == NULL) {
            return XIL_FAILURE;
        }

        //
        //  Data in is always MSBFirst
        //
        bitXImage->bitmap_bit_order = MSBFirst;
        bitXImage->byte_order = MSBFirst;

        lockX();
        bitPixmap = XCreatePixmap(displayptr, window, width, height, 1);
        unlockX();

        //
        //  Create the GC for correct copying of the data into the pixmap.
        //
        XGCValues gc_val;

        gc_val.background = BlackPixel(displayptr, DefaultScreen(displayptr));
        gc_val.foreground = WhitePixel(displayptr, DefaultScreen(displayptr));
        gc_val.function   = GXcopy;
        gc_val.graphics_exposures = False;

        lockX();
        castGC = XCreateGC(displayptr, bitPixmap, 
                           GCForeground|GCBackground|GCFunction|GCGraphicsExposures, 
                           &gc_val);
        unlockX();
        if(!castGC) {
            return XIL_FAILURE;
        }
    }

    return XIL_SUCCESS;
}

XilStatus
XilDeviceIOxlib::cast1to8Display(XilOp*       op,
                                 unsigned int ,
                                 XilRoi*      roi,
                                 XilBoxList*  bl)
{
    //
    //  Split the list of XilBoxes to take tile boundaries into account.  This
    //  will work to ensure that no cobbling of the data is required because
    //  the boxes will not cross tile boundaries in the source images.
    //
    if(op->splitOnTileBoundaries(bl) == XIL_FAILURE) {
        return XIL_FAILURE;
    }
    
    //
    //  Get the source image passed to us on the op...
    //
    XilOp*    srcOp    = op->getOpList()[1];
    XilImage* srcImage = srcOp->getSrcImage(1);

    //
    //  Get information about the input image
    //
    unsigned int   nbands    = srcImage->getNumBands();
    XilDataType    data_type = srcImage->getDataType();
    
    //
    //  A local copy of the XImage used only by this display operation.
    //
    XImage x_image = *bitXImage;

    //
    //  Loop over each of the boxes of storage we are to process.
    //
    XilBox* src_box;
    XilBox* dst_box;
    while(bl->getNext(&src_box, &dst_box)) {
        //
        //  Aquire our storage from the images.
        //
        XilStorage  src_storage(srcImage);

        //
        //  Since the source is 1 banded, we don't care what the storage
        //  format is -- they're the same for 1 banded images.
        //
        if(srcImage->getStorage(&src_storage, srcOp, src_box, "XilMemory",
                                XIL_READ_ONLY) == XIL_FAILURE) {
            //
            //  Mark this box as failed and if that succeeds, continue
            //  processing the next box.  Otherwise, return XIL_FAILURE now.
            //
            if(bl->markAsFailed() == XIL_FAILURE) {
                return XIL_FAILURE;
            } else {
                continue;
            }
        }

        //
        // Get the co-ordinates of the box, we need to adjust to
        // absolute image co-ordinates in the destination.
        //
        int          dst_x;
        int          dst_y;
        unsigned int abs_w;
        unsigned int abs_h;
        dst_box->getAsRect(&dst_x, &dst_y, &abs_w, &abs_h);

        //
        // ximage's are pixel sequential and so as we asked for
        // pixel sequential storage this is ok.
        //
        unsigned int   src_pixel_stride;
        unsigned int   src_scanline_stride;
        Xil_unsigned8* src_data;
        src_storage.getStorageInfo(&src_pixel_stride,
                                   &src_scanline_stride,
                                   NULL, NULL,
                                   (void**)&src_data);

        //
        //  Set the source data information into the ximage structure.
        //
        x_image.data           = (char*)src_data;
        x_image.bytes_per_line = (int)src_scanline_stride;

        
        //
        //  Loop over the rectangles in the ROI...
        //
        XilRectList  rl(roi, dst_box);

        int          x;
        int          y;
        unsigned int x_size;
        unsigned int y_size;
        while(rl.getNext(&x, &y, &x_size, &y_size)) {
            //
            //  Copy the region into the pixmap.
            //
            lockX();
            XPutImage(displayptr, bitPixmap, castGC, &x_image,
                      (int)x, (int)y, (int)x, (int)y, x_size, y_size);

            //
            //  Copy the bit pixmap into the window.
            //
            XCopyPlane(displayptr, bitPixmap, window, gc,
                       (int)x, (int)y, x_size, y_size,
                       (int)x+dst_x, (int)y+dst_y, 1);
            unlockX();
        }
    }

    lockX();
    XFlush(displayptr);
    unlockX();

    return XIL_SUCCESS;
}

void
XilDeviceIOxlib::reformatToXImage(Xil_unsigned8* src,
                                  unsigned int   srcPS,
                                  unsigned int   srcSS,
                                  Xil_unsigned8* dst,
                                  unsigned int   dstSS,
                                  unsigned int   width,
                                  unsigned int   height) {

    switch (ximage->bits_per_pixel) {
      case 24:
        if (ximage->byte_order == MSBFirst) {
            reformatTo24MSB(src, srcPS, srcSS, dst, dstSS, width, height);
        } else {
            reformatTo24LSB(src, srcPS, srcSS, dst, dstSS, width, height);
        }
        break;
      case 32:
        if (ximage->byte_order == MSBFirst) {
            reformatTo32MSB(src, srcPS, srcSS, dst, dstSS, width, height);
        } else {
            reformatTo32LSB(src, srcPS, srcSS, dst, dstSS, width, height);
        }
        break;
    }
}


void
XilDeviceIOxlib::reformatTo24MSB(Xil_unsigned8* src,
                             unsigned int   srcPS,
                             unsigned int   srcSS,
                             Xil_unsigned8* dst,
                             unsigned int   dstSS,
                             unsigned int   width,
                             unsigned int   height) {

    for (int line=height; line!=0; line--) {
        Xil_unsigned8* pSrc = src; 
        Xil_unsigned8* pDst = dst; 
        for (int samp=width; samp!=0; samp--) {
            int b = pSrc[0] >> bTrim;
            int g = pSrc[1] >> gTrim;
            int r = pSrc[2] >> bTrim;
            pSrc += srcPS;

            Xil_unsigned32 pixel = (r<<rLSB) | (g<<gLSB) | (b<<bLSB);

            pDst[0] = (pixel>>16) & 0xff;
            pDst[1] = (pixel>>8) & 0xff;
            pDst[2] = pixel & 0xff;
            pDst += 3;
        }
        src += srcSS;
        dst += dstSS;
    }
}

void
XilDeviceIOxlib::reformatTo24LSB(Xil_unsigned8* src,
                             unsigned int   srcPS,
                             unsigned int   srcSS,
                             Xil_unsigned8* dst,
                             unsigned int   dstSS,
                             unsigned int   width,
                             unsigned int   height) {

    for (int line=height; line!=0; line--) {
        Xil_unsigned8* pSrc = src; 
        Xil_unsigned8* pDst = dst; 
        for (int samp=width; samp!=0; samp--) {
            int b = pSrc[0] >> bTrim;
            int g = pSrc[1] >> gTrim;
            int r = pSrc[2] >> bTrim;
            pSrc += srcPS;

            Xil_unsigned32 pixel = (r<<rLSB) | (g<<gLSB) | (b<<bLSB);

            pDst[0] = pixel & 0xff;
            pDst[1] = (pixel>>8) & 0xff;
            pDst[2] = (pixel>>16) & 0xff;
            pDst += 3;
        }
        src += srcSS;
        dst += dstSS;
    }
}

void
XilDeviceIOxlib::reformatTo32MSB(Xil_unsigned8* src,
                             unsigned int   srcPS,
                             unsigned int   srcSS,
                             Xil_unsigned8* dst,
                             unsigned int   dstSS,
                             unsigned int   width,
                             unsigned int   height) {

    for (int line=height; line!=0; line--) {
        Xil_unsigned8* pSrc = src; 
        Xil_unsigned8* pDst = dst; 
        for (int samp=width; samp!=0; samp--) {
            int b = pSrc[0] >> bTrim;
            int g = pSrc[1] >> gTrim;
            int r = pSrc[2] >> bTrim;
            pSrc += srcPS;

            Xil_unsigned32 pixel = (r<<rLSB) | (g<<gLSB) | (b<<bLSB);

            pDst[0] = (pixel>>24) & 0xff;
            pDst[1] = (pixel>>16) & 0xff;
            pDst[2] = (pixel>>8) & 0xff;
            pDst[3] = pixel & 0xff;
            pDst += 4;
        }
        src += srcSS;
        dst += dstSS;
    }
}

void
XilDeviceIOxlib::reformatTo32LSB(Xil_unsigned8* src,
                             unsigned int   srcPS,
                             unsigned int   srcSS,
                             Xil_unsigned8* dst,
                             unsigned int   dstSS,
                             unsigned int   width,
                             unsigned int   height) {

    for (int line=height; line!=0; line--) {
        Xil_unsigned8* pSrc = src; 
        Xil_unsigned8* pDst = dst; 
        for (int samp=width; samp!=0; samp--) {
            int b = pSrc[0] >> bTrim;
            int g = pSrc[1] >> gTrim;
            int r = pSrc[2] >> bTrim;
            pSrc += srcPS;

            Xil_unsigned32 pixel = (r<<rLSB) | (g<<gLSB) | (b<<bLSB);

            pDst[0] = pixel & 0xff;
            pDst[1] = (pixel>>8) & 0xff;
            pDst[2] = (pixel>>16) & 0xff;
            pDst[3] = (pixel>>24) & 0xff;
            pDst += 4;
        }
        src += srcSS;
        dst += dstSS;
    }
}

void
XilDeviceIOxlib::reformatFromXImage(Xil_unsigned8* src,
                                    unsigned int   srcSS,
                                    Xil_unsigned8* dst,
                                    unsigned int   dstPS,
                                    unsigned int   dstSS,
                                    unsigned int   width,
                                    unsigned int   height) {

    switch (ximage->bits_per_pixel) {
      case 24:
        if (ximage->byte_order == MSBFirst) {
            reformatFrom24MSB(src, srcSS, dst, dstPS, dstSS, width, height);
        } else {
            reformatFrom24LSB(src, srcSS, dst, dstPS, dstSS, width, height);
        }
        break;
      case 32:
        if (ximage->byte_order == MSBFirst) {
            reformatFrom32MSB(src, srcSS, dst, dstPS, dstSS, width, height);
        } else {
            reformatFrom32LSB(src, srcSS, dst, dstPS, dstSS, width, height);
        }
        break;
    }
}

void
XilDeviceIOxlib::reformatFrom24MSB(Xil_unsigned8* src,
                                   unsigned int   srcSS,
                                   Xil_unsigned8* dst,
                                   unsigned int   dstPS,
                                   unsigned int   dstSS,
                                   unsigned int   width,
                                   unsigned int   height) {

    for (int line=height; line!=0; line--) {
        Xil_unsigned8* pSrc = src; 
        Xil_unsigned8* pDst = dst; 
        for (int samp=width; samp!=0; samp--) {

            Xil_unsigned32 pixel = (pSrc[0]<<16) | (pSrc[1]<<8) | pSrc[2];
            pSrc += 3;

            pDst[0] = (pixel >> bLSB) & 0xff;
            pDst[1] = (pixel >> gLSB) & 0xff;
            pDst[2] = (pixel >> rLSB) & 0xff;
            pDst += dstPS;
        }
        src += srcSS;
        dst += dstSS;
    }
}

void
XilDeviceIOxlib::reformatFrom24LSB(Xil_unsigned8* src,
                                   unsigned int   srcSS,
                                   Xil_unsigned8* dst,
                                   unsigned int   dstPS,
                                   unsigned int   dstSS,
                                   unsigned int   width,
                                   unsigned int   height) {

    for (int line=height; line!=0; line--) {
        Xil_unsigned8* pSrc = src; 
        Xil_unsigned8* pDst = dst; 
        for (int samp=width; samp!=0; samp--) {

            Xil_unsigned32 pixel = pSrc[0] | (pSrc[1]<<8) | (pSrc[2]<<16);
            pSrc += 3;

            pDst[0] = (pixel >> bLSB) & 0xff;
            pDst[1] = (pixel >> gLSB) & 0xff;
            pDst[2] = (pixel >> rLSB) & 0xff;
            pDst += dstPS;

        }
        src += srcSS;
        dst += dstSS;
    }
}


void
XilDeviceIOxlib::reformatFrom32MSB(Xil_unsigned8* src,
                                   unsigned int   srcSS,
                                   Xil_unsigned8* dst,
                                   unsigned int   dstPS,
                                   unsigned int   dstSS,
                                   unsigned int   width,
                                   unsigned int   height) {

    for (int line=height; line!=0; line--) {
        Xil_unsigned8* pSrc = src; 
        Xil_unsigned8* pDst = dst; 
        for (int samp=width; samp!=0; samp--) {

            Xil_unsigned32 pixel = (pSrc[0]<<24) | (pSrc[1]<<16) | 
                                   (pSrc[2]<<8) | pSrc[3];
            pSrc += 4;

            pDst[0] = (pixel >> bLSB) & 0xff;
            pDst[1] = (pixel >> gLSB) & 0xff;
            pDst[2] = (pixel >> rLSB) & 0xff;
            pDst += dstPS;
        }
        src += srcSS;
        dst += dstSS;
    }
}

void
XilDeviceIOxlib::reformatFrom32LSB(Xil_unsigned8* src,
                                   unsigned int   srcSS,
                                   Xil_unsigned8* dst,
                                   unsigned int   dstPS,
                                   unsigned int   dstSS,
                                   unsigned int   width,
                                   unsigned int   height) {

    for (int line=height; line!=0; line--) {
        Xil_unsigned8* pSrc = src; 
        Xil_unsigned8* pDst = dst; 
        for (int samp=width; samp!=0; samp--) {

            Xil_unsigned32 pixel = pSrc[0] | (pSrc[1]<<8) | 
                                   (pSrc[2]<<16) | (pSrc[3]<<24);
            pSrc += 4;

            pDst[0] = (pixel >> bLSB) & 0xff;
            pDst[1] = (pixel >> gLSB) & 0xff;
            pDst[2] = (pixel >> rLSB) & 0xff;
            pDst += dstPS;

        }
        src += srcSS;
        dst += dstSS;
    }
}


int
XilDeviceIOxlib::findLSB(unsigned long planemask) {

    unsigned long bitmask = 1;
    for(int i=0; i<32; i++) {
        if (planemask & bitmask) {
            return i;
        }
        bitmask <<= 1;
    }

    return 0; // This should never happen
}

int
XilDeviceIOxlib::countBits(unsigned long planemask,
                           int           lsb) {

    unsigned long bitmask = 1 << lsb;
    for(int i=lsb; i<32; i++) {
        if ((planemask & bitmask) == 0) {
            return i - lsb;
        }
        bitmask <<= 1;
    }

    return 0; // This should never happen
}


