#include <stdio.h>
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
//  File:	XilDeviceIOxshm.cc
//  Project:	XIL
//  Revision:	1.22
//  Last Mod:	10:13:59, 03/10/00
//  SID:	%Z% %F% %I% %U% %E%
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
//  MT-level:  SAFE
//	
//------------------------------------------------------------------------
//	COPYRIGHT
//------------------------------------------------------------------------
#pragma ident	"@(#)XilDeviceIOxshm.cc	1.22\t00/03/10  "

#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <pthread.h>

#include "XilDeviceIOxshm.hh"
#include "XiliUtils.hh"

#ifdef SOLARIS
//
// There isn't a prototype declared for
// this function, in Solaris 2.5.1 and below.
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

#if defined(SOLARIS) || defined(IRIX) || defined(HPUX)
extern "C" {
    //
    //  Solaris and IRIX don't seem to delare this in XShm.h
    //
    int         XShmGetEventBase(Display* display);
}
#endif // SOLARIS  || IRIX


extern "C" {
    //
    //  A function that tests to see if the event we want is on the queue.
    //
    static
    Bool
    xili_check_if_xshm_event(Display* ,
                             XEvent*  xevent,
                             char*    arg)
    {
        if(xevent->type == *((int*)arg)) {
            return TRUE;
        } else {
            return FALSE;
        }
    }
    
    //
    //  Trap a BadAccess error for XShmAttach() and set a global flag.
    //  If we get another type of error, call the old errorHandler.
    //
    static
    void
    xili_xshm_attach_error_handler(Display*     dpy,
                                   XErrorEvent* errEvent)
    {
        if(errEvent->error_code   == BadAccess &&
           errEvent->request_code == xili_xshm_attach_info.major_code &&
           errEvent->minor_code   == X_ShmAttach) {
            xili_xshm_attach_info.attachFailed = True;
        } else {
            (*xili_xshm_attach_info.oldHandler)(dpy, errEvent);
        }
    }
}

//
//  The global struct that holds some xshm attach information that's
//  shared between the manager and the device.
//
XiliXShmAttachInfo xili_xshm_attach_info;

//
//  In this constructor we just save the things we
//  need. The core will call us when they want the
//  controlling image.
//
XilDeviceIOxshm::XilDeviceIOxshm(XilDeviceManagerIO* device_manager,
                                 XilSystemState*     state,
				 Display*            display,
				 Window              window) :
    XilDeviceIO(device_manager), regionData(16)
{
    isOKFlag          = FALSE;

    //
    //  Since we want to get events from the server, we need to open our own
    //  connection to the server that ensures we have a seperate event queue
    //  from the one the application or other XShm windows is using.
    //
    xDisplay          = XOpenDisplay(DisplayString(display));
    if(xDisplay == NULL) {
        return;
    }

    systemState       = state;
    origDisplay       = display;
    xWindow           = window;
    xImage            = NULL;
    xGC               = NULL;
    xshmCompleteType  = XShmGetEventBase(xDisplay) + ShmCompletion;

    parentImage       = NULL;
    controllingImage  = NULL;
    deviceLookup      = NULL;
    colorspace        = NULL;
    destroyColorspace = FALSE;
    controllingImageBandOffset  = 0;

    //
    //  This needs to be negative so if constructControllingImage can't complete,
    //  the destructor will know that no memory was actually shared.
    //
    xshmSegmentInfo.shmid = -1;

    //
    //  Initialize cast1->8 variables
    //
    bitXImage = NULL;

    // 
    //  Offscreen Buffer
    //
    offScreenBuffer = NULL;
    setDoubleBuffered(FALSE);
    nRegions   = 0;

    isOKFlag          = TRUE;
}

//
//  Destructor, clean up things we created.
//
//  The core cleans up the images we created so we don't destroy them here.
//
XilDeviceIOxshm::~XilDeviceIOxshm()
{
    if(destroyColorspace) {
        colorspace->destroy();
    }

    if(xGC) {
	XFreeGC(xDisplay, xGC);
    }

    if (oGC) {
	XFreeGC(origDisplay, oGC);
    }

    if(bitXImage) {
        XFreeGC(xDisplay, castGC);
        XFreePixmap(xDisplay, bitPixmap);
        bitXImage->data = NULL;
        XDestroyImage(bitXImage);
    }

    //
    //  Detach the XShm area, destroy the image and release the shared memory
    //  resources.
    //
    if(xshmSegmentInfo.shmid >= 0) {
        
        XShmDetach(xDisplay, &xshmSegmentInfo);
        
        //
        //  Let the server destroy the segment.
        //
        if (xImage != NULL)   // HK July 13, 2003: better safe than segv
            XDestroyImage(xImage);
        
        //
        //  Release the shared memory resources.
        //
        shmdt(xshmSegmentInfo.shmaddr);
    } else if (xImage != NULL) // HK uly 13, 2003: see comment below
        XDestroyImage(xImage);

    //
    //  Pick up the case where the constructControllingImage failed and there was
    //  no shared memory
    //
/*  HK July 13, 2003: xImage might already be destroyed, hence I put this
    statement in the above created else clause. This bug caused random segv on linux
    if(xImage != NULL) {
            XDestroyImage(xImage);
    }
*/

    if (offScreenBuffer) {
        XFreePixmap(origDisplay, offScreenBuffer);
        //XFreePixmap(xDisplay, offScreenBuffer);
        offScreenBuffer = NULL;
    }

    //
    //  Unlike the Xlib I/O handler, we create our own connection to the X
    //  server instead of sharing the application's connection.  The manager
    //  does this and we're expected to close the connection when we're done.
    //
    if(xDisplay) {
        XCloseDisplay(xDisplay);
    }
}


Xil_boolean 
XilDeviceIOxshm::isDoubleBuffered()
{
    return doubleBuffered;
}


void 
XilDeviceIOxshm::setDoubleBuffered(Xil_boolean flag)
{
    doubleBuffered = flag;
}


//
//  Create the image associated with our device.
//
XilImage* 
XilDeviceIOxshm::constructControllingImage()
{
    //
    //  Query to aquire the window information.
    //
    Window       root;
    int          x;
    int          y;
    unsigned int width;
    unsigned int height;
    unsigned int border_width;
    Status       status = XGetGeometry(xDisplay, xWindow,&root, &x, &y, &width,
                                       &height, &border_width, &xDepth);
    if(status == 0) {
	return NULL;
    }

    //
    //  Create a graphics context
    //
    XGCValues gc_val;
    gc_val.foreground = 0;
    gc_val.function   = GXcopy;

    xGC = XCreateGC(xDisplay, xWindow, GCForeground|GCFunction, &gc_val);
    if(xGC == NULL) {
	return NULL;
    }
    oGC = XCreateGC(origDisplay, xWindow, GCForeground|GCFunction, &gc_val);
    if(oGC == NULL) {
        XFreeGC(xDisplay, xGC);
	return NULL;
    }

    //
    //  Get the window attributes and create the shared X image
    //
    if(!XGetWindowAttributes(xDisplay, xWindow, &xWindowAttribs)) {
	XFreeGC(xDisplay, xGC);
        XFreeGC(origDisplay, oGC);
	return NULL;
    }

    xImage = XShmCreateImage(xDisplay,
                             xWindowAttribs.visual,
                             xWindowAttribs.depth,
                             ZPixmap,
                             NULL, &xshmSegmentInfo, width, height);
    if(xImage == NULL) {
	return NULL;
    }

    //
    //  Detect two common planemask consigurations
    //
    isXBGR = (xImage->blue_mask  == 0xFF0000) &&
             (xImage->green_mask == 0xFF00) &&
             (xImage->red_mask   == 0xFF);

    isXRGB = (xImage->blue_mask  == 0xFF) &&
             (xImage->green_mask == 0xFF00) &&
             (xImage->red_mask   == 0xFF0000);

#ifdef XIL_LITTLE_ENDIAN 
// Don't bail out of XSHM if not 24 bits visual
// HK April 13, 2006
//    if(isXRGB) {
        //
        // This is the typical Wintel xRGB layout.
        // However, in a little-endian machine, it looks like BGRX.
        //
        bLSB = 0;
        gLSB = 8;
        rLSB = 16;
        controllingImageBandOffset = 0;
//    } else {
//        return NULL;
//    }
#else
//    if (isXBGR) {
        //
        // FFB, AFB, CG14, etc
        //
        bLSB = 16;
        gLSB = 8;
        rLSB = 0;
        controllingImageBandOffset = 1;
//    } else {
//        return NULL;
//    }
#endif

    //
    //  At this point, xImage->bits_per_pixel reflects the values 
    //  returned from the X server.
    //
    //  Shared memory only supports 8-bit and 32-bit displays.
    //
    XilImage* storage_image;
    switch(xImage->bits_per_pixel) {
      case 8:
	controllingImage = systemState->createXilImage(width, height, 1,
                                                       XIL_BYTE);
        storage_image = controllingImage;
	break;

      case 32:
        parentImage = systemState->createXilImage(width, height, 4,
                                               XIL_BYTE);
        if(parentImage == NULL) {
            XDestroyImage(xImage);
            return NULL;
        }

        controllingImage =
            parentImage->createChild(0, 0, width, height, 
                                     controllingImageBandOffset, 3);

        storage_image = parentImage;
        break;

      default:
	return NULL;
    }

    //
    //  Check for a NULL controlling image
    //
    if(controllingImage == NULL) {
        return NULL;
    }

    //
    //  Now, create the shared memory segment.
    //
    xshmSegmentInfo.shmid = shmget(IPC_PRIVATE,
                                   xImage->bytes_per_line*height,
                                   IPC_CREAT|0777);
    if(xshmSegmentInfo.shmid < 0) {
        parentImage->destroy();
        return NULL;
    }


        
    xshmSegmentInfo.shmaddr = xImage->data =
        (char*)shmat(xshmSegmentInfo.shmid,0,0);
    xshmSegmentInfo.readOnly = False;

    if (doubleBuffered)
        offScreenBuffer = XCreatePixmap(origDisplay, xWindow, width, height, xWindowAttribs.depth);
        //offScreenBuffer = XCreatePixmap(xDisplay, xWindow, width, height, xWindowAttribs.depth);

    status = 0;
    if(xshmSegmentInfo.shmaddr != (char*)-1) {
        XSync(xDisplay, False);

        xili_xshm_attach_info.oldHandler = 
            XSetErrorHandler((XErrorHandler)xili_xshm_attach_error_handler);

        xili_xshm_attach_info.attachFailed = False;

        status = XShmAttach(xDisplay, &xshmSegmentInfo);

        XSync(xDisplay, False);

        XSetErrorHandler(xili_xshm_attach_info.oldHandler);
    } else {
        shmdt(xshmSegmentInfo.shmaddr);
        shmctl(xshmSegmentInfo.shmid, IPC_RMID, 0);

        //
        //  Set the xshmSegmentInfo.shmid to -1 to indicate to the constructor
        //  not to release any of the shared emmroy resources since they've
        //  been released.
        //
        xshmSegmentInfo.shmid = -1;

        return NULL;
    }

    //
    //  If we attached or not, we want to delete the shared
    //  memory segment.  In the case where we successfully
    //  attached, the server keeps a reference to it so it will not
    //  really be deleted until the client dies.  If we didn't
    //  attach successfully, we don't want it around anymore.
    //
    shmctl(xshmSegmentInfo.shmid, IPC_RMID, 0);


    if(!status || xili_xshm_attach_info.attachFailed) {
        shmdt(xshmSegmentInfo.shmaddr);

        //
        //  Set the xshmSegmentInfo.shmid to -1 to indicate to the constructor
        //  not to release any of the shared emmroy resources since they've
        //  been released.
        //
        xshmSegmentInfo.shmid = -1;

        //
        //  The destructor will take care of releasing resources.
        //
        xshmSegmentInfo.shmid = -1;
        return NULL;
    }

    //
    //  Ok, now we handle getting the storage set on the controlling image.
    //
    //  We set the controlling image storage as XIL_KEEP_STATIONARY,
    //  so it doesn't change or get moved around.
    //
    if(storage_image->exportStorage() == XIL_FAILURE) {
        return NULL;
    }

    //
    //  First, set the image's tile size to be the entire image so it's not
    //  trying to tile our backing storage
    //
    if(storage_image->setExportedTileSize(width, height) == XIL_FAILURE) {
        return NULL;
    }

    //
    //  Now, construct an API storage object for setting the storage for the
    //  image...
    //
    XilStorage storage(storage_image);

    //
    //  Ok, describe the shared memory segment storage and set the image's
    //  storage.
    //
    storage.setCoordinates(0, 0);
    storage.setPixelStride(xImage->bits_per_pixel >> 3);
    storage.setScanlineStride(xImage->bytes_per_line);
    storage.setDataPtr(xImage->data);

    if(storage_image->setExportedTileStorage(&storage) == XIL_FAILURE) {
        return NULL;
    }

    //
    //  Set the storage movement flag and then re-import the image.
    //
    storage_image->setStorageMovement(XIL_KEEP_STATIONARY);
    storage_image->import(TRUE);

    return controllingImage;
}

XilStatus
XilDeviceIOxshm::display(XilOp*       op,
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
    //  Get the image passed to us on the op -- guarenteed to be the
    //  controlling image we constructed earlier.
    //
    XilImage* image = op->getSrcImage(1);

    //
    //  If for some reason it is not the controllingImage, then return failure
    //  because we set the shared memory backing store on the
    //  controllingImage. 
    //
    if(image != controllingImage) {
        return XIL_FAILURE;
    }

    //
    //  Loop over each of the boxes of storage we are to process.
    //
    XilBox* src_box;
    XilBox* dst_box;
    while(bl->getNext(&src_box, &dst_box)) {
	//
        //  Aquire our storage from the images.
        //
        XilStorage  src_storage(image);

        //
        //  Since we set the storage on the image and requested it remain
        //  stationary, we know it's pixel-sequential and it points into our
        //  shared memory buffer.
	if(image->getStorage(&src_storage, op, src_box, "XilMemory",
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
	//  Get the co-ordinates of the box, we need to adjust to
	//  absolute image co-ordinates in the destination.
	//
	int          src_x;
	int          src_y;
	unsigned int src_w;
	unsigned int src_h;
	src_box->getAsRect(&src_x, &src_y, &src_w, &src_h);


	int          dst_x;
	int          dst_y;
	unsigned int dst_w;
	unsigned int dst_h;
	dst_box->getAsRect(&dst_x, &dst_y, &dst_w, &dst_h);

	//
	//  Loop over the rectangles in the ROI.
        //
	//  All of the regions are guaranteed not to go outside the image.
	//
	XilRectList  rl(roi, dst_box);

	int          x;
	int          y;
	unsigned int x_size;
	unsigned int y_size;
        Xil_boolean doPut = FALSE;

        int index = nRegions;
        nRegions += rl.getNumRects();
        if (regionData.size() < nRegions)
            regionData.resize(nRegions);

	while (rl.getNext(&x, &y, &x_size, &y_size)) {
            doPut = TRUE;
            if (doubleBuffered) {
                regionData[index].src_x  = x+src_x;
                regionData[index].src_y  = y+src_y;
                regionData[index].dst_x  = x+dst_x;
                regionData[index].dst_y  = y+dst_y;
                regionData[index].x_size = x_size;
                regionData[index].y_size = y_size;

	        XShmPutImage(xDisplay, offScreenBuffer, xGC, xImage, 
                             regionData[index].src_x,  regionData[index].src_y,
                             regionData[index].dst_x,  regionData[index].dst_y,
                             regionData[index].x_size, regionData[index].y_size, 
                             False
                );
                index++;
            } else 
	        XShmPutImage(xDisplay, xWindow, xGC, xImage,
                             (int)x+src_x, (int)y+src_y,
                             (int)x+dst_x, (int)y+dst_y,
                             x_size, y_size, False
                );
	}
        //
        //  We do a final put of a 1x1 area that will get us our signal
        //  they're all done since they're handled sequentially by the X
        //  server.
        //  Fix for BugId 4174709:
        //  Do the final 1x1 XShmPutImage only if a previous put
        //  was done above. Otherwise, the values of x and y are
        //  uninitialized
        //
        if(doPut) {
            if (doubleBuffered)
                XShmPutImage(xDisplay, offScreenBuffer, xGC, xImage,
                             (int)x+src_x, (int)y+src_y,
                             (int)x+dst_x, (int)y+dst_y,
                             1, 1, True);
            else
                XShmPutImage(xDisplay, xWindow, xGC, xImage,
                             (int)x+src_x, (int)y+src_y,
                             (int)x+dst_x, (int)y+dst_y,
                             1, 1, True);

            //  Now, wait until the XShmPutImage is done.
            XEvent xevent;
            XIfEvent(xDisplay, &xevent, xili_check_if_xshm_event, (char*) &xshmCompleteType);
        }
    }

    return XIL_SUCCESS;
}


static bool contains(RegionData src, RegionData dst)
{
    if (src.src_x > dst.src_x)
        return False;
    if (dst.src_x+dst.x_size > src.src_x+src.x_size)
        return False;
    if (src.src_y > dst.src_y)
        return False;
    if (dst.src_y+dst.y_size > src.src_y+src.y_size)
        return False;

    return True;
}


XilStatus
XilDeviceIOxshm::swapBuffers()
{
    if (! doubleBuffered)
        return XIL_SUCCESS;

    if (nRegions == 0)
        return XIL_SUCCESS;

    for (int index=0; index<nRegions; index++) {
        bool skip = False;
        for (int i=0; i<index; i++)
            if (contains(regionData[i], regionData[index])) {
                skip = True;
                break;
            }
        if (! skip) 
            XCopyArea(origDisplay, offScreenBuffer, xWindow, oGC,
                      regionData[index].src_x,
                      regionData[index].src_y,
                      regionData[index].x_size,
                      regionData[index].y_size,
                      regionData[index].dst_x,
                      regionData[index].dst_y
            );
    }
    
/*
    XShmPutImage(xDisplay, xWindow, xGC, xImage,
             regionData[index][0], regionData[index][1],  regionData[index][2], regionData[index][3],
             1, 1, True);
    XEvent xevent;
    XIfEvent(xDisplay, &xevent, xili_check_if_xshm_event, (char*)&xshmCompleteType);
*/

    nRegions = 0;

    return XIL_SUCCESS;
}


XilStatus
XilDeviceIOxshm::capture(XilOp*       op,
                         unsigned int ,
			 XilRoi*      roi,
			 XilBoxList*  bl)
{
    //
    //  Get the image passed to us on the op -- guarenteed to be the
    //  controlling image we constructed earlier.
    //
    XilImage* image = op->getSrcImage(1);

    //
    //  If for some reason it is not the controllingImage, then return failure
    //  because we set the shared memory backing store on the
    //  controllingImage. 
    //
    if(image != controllingImage) {
        return XIL_FAILURE;
    }

    //
    //  If we can't get the window attributes and its not
    //  mapped just return.
    //
    XWindowAttributes win_info;

    if (doubleBuffered) {
         if (!XGetWindowAttributes(origDisplay, xWindow, &win_info))
             return XIL_FAILURE;
    } else
         if (!XGetWindowAttributes(xDisplay, xWindow, &win_info))
             return XIL_FAILURE;

    if((win_info.map_state == IsUnmapped) ||
       (win_info.map_state == IsUnviewable)) {
        return XIL_SUCCESS;
    }

    //
    //  Determine the visible portion of the screen
    //
    int screen = XScreenNumberOfScreen(win_info.screen);

    //
    //  Upper left corner relative to the root window
    //
    int    wabs_x;
    int    wabs_y;
    Window dwin;
    if(!XTranslateCoordinates(xDisplay, xWindow,
			      RootWindow(xDisplay, screen),
			      0, 0, &wabs_x, &wabs_y, &dwin)) {
	return XIL_FAILURE;
    }

    //
    //  Setup window information
    //
    win_info.x = wabs_x;
    win_info.y = wabs_y;

    unsigned int clipwidth  = win_info.width;
    unsigned int clipheight = win_info.height;

    //
    //  Screen width height
    //
    int dwidth  = DisplayWidth(xDisplay, screen);
    int dheight = DisplayHeight(xDisplay, screen);

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
    //  Create a rect list for use in the while box's loop.
    //
    //  This constrains the capture to the visible area of the image.
    //
    XilRectList crl(roi, clipx, clipy, clipwidth, clipheight);

    //
    //  Use a local XImage structure so the XShm one isn't modified.
    //
    XImage local_ximage = *xImage;

    //
    //  Loop over each of the boxes of storage we are to process.
    //
    XilBox* src_box;
    XilBox* dst_box;
    while(bl->getNext(&src_box, &dst_box)) {
	//
        //  Aquire our storage from the images.
        //
        XilStorage  storage(image);

	//
	//  Always asking for PIXEL_SEQUENTIAL is quicker than trying
	//  to handle the other storage types. As X data is inherently
	//  PIXEL_SEQUENTIAL
	//
	if(image->getStorage(&storage, op, dst_box, "XilMemory",
                                XIL_READ_ONLY, XIL_PIXEL_SEQUENTIAL) == XIL_FAILURE) {
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
	//  Get the co-ordinates of the box, we need to adjust to
	//  absolute image co-ordinates in the destination.
	//
	int          abs_x;
	int          abs_y;
	unsigned int abs_w;
	unsigned int abs_h;
	src_box->getAsRect(&abs_x, &abs_y, &abs_w, &abs_h);

	//
	//  Needs to be done on the current box.
        //
        //  The data pointer returned here is wrt to controlling image.
        //
        //  In the case of a 3-band child of 4-band parent, we need to remove
        //  the child offset.
	//
        if(parentImage != NULL) {
            local_ximage.data = ((char*)storage.getDataPtr()) - 
                                controllingImageBandOffset;
        } else {
            local_ximage.data = (char*)storage.getDataPtr();
        }

        local_ximage.bytes_per_line = (int)storage.getScanlineStride();

	//
	//  Loop over the rectangles in the ROI all of the regions are
        //  guaranteed not to go outside the window now.
	//
	XilRectList  rl(&crl, src_box);

	int          roi_x;
	int          roi_y;
	unsigned int x_size;
	unsigned int y_size;
	while(rl.getNext(&roi_x, &roi_y, &x_size, &y_size)) {
	    XGetSubImage(xDisplay, xWindow,
			 (roi_x+abs_x), (roi_y+abs_y), x_size, y_size,
			 0xFFFFFFFF, ZPixmap, &local_ximage,
			 roi_x, roi_y);
	}
        XFlush(xDisplay);
    }

    return XIL_SUCCESS;
}

//
//  Check code for ROI setup
//
XilStatus
XilDeviceIOxshm::getPixel(unsigned int x,
			  unsigned int y,
			  float*       values,
			  unsigned int ,       // offset_band,
			  unsigned int nbands)
{
    //
    //  If we can't get the window attributes and its not
    //  mapped just return.
    //
    XWindowAttributes win_info;
	
    if(!XGetWindowAttributes(xDisplay, xWindow, &win_info)) {
        XIL_ERROR(systemState,XIL_ERROR_SYSTEM,"di-249",TRUE);
        return XIL_FAILURE;
    }

    if((win_info.map_state == IsUnmapped) ||
       (win_info.map_state == IsUnviewable)) {
        return XIL_SUCCESS;
    }

    //
    //  Determine the visible portion of the screen
    //
    int screen = XScreenNumberOfScreen(win_info.screen);

    //
    //  Upper left corner relative to the root window
    //
    int    wabs_x;
    int    wabs_y;
    Window dwin;
    if(!XTranslateCoordinates(xDisplay, xWindow,
			      RootWindow(xDisplay, screen),
			      0, 0, &wabs_x, &wabs_y, &dwin)) {
	XIL_ERROR(systemState, XIL_ERROR_SYSTEM, "di-249", TRUE);
	return XIL_FAILURE;
    }

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
    int dwidth  = DisplayWidth(xDisplay, screen);
    int dheight = DisplayHeight(xDisplay, screen);

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
	//  Pixel is on the screen.
        //

	//
	//  Get a single pixel image
	//
	XImage* image = XGetImage(xDisplay, xWindow,
				  x, y, 1, 1, 0xFFFFFF, ZPixmap);
	if(image == NULL) {
	    XIL_ERROR(systemState, XIL_ERROR_SYSTEM, "di-316", TRUE);
	    return XIL_FAILURE;
	}

	//
	//  Get the pixel data from the X image.
        //
	//  SOL64:  XGetPixel returns a u_long we cast here
	//          to remove use of long for Solaris64
	//
	unsigned int pixel = (unsigned int)XGetPixel(image, 0, 0);

	XDestroyImage(image);

        //
	//  Set up the return value(s)
        //
	switch(xDepth) {
	  case 1:
	  case 4:
	  case 8:
	  case 16:
	    values[0] = (float)pixel;
	    break;
	    
	  case 24:
          {
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
XilDeviceIOxshm::setPixel(unsigned int x,
			  unsigned int y,
			  float*       values,
			  unsigned int ,        // offset_band,
			  unsigned int nbands)
{
    unsigned int pixel;
    unsigned int mask;

    switch(xDepth) {
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

        for(int i=0; i<nbands; i++) {
            switch(i) {
              case 0:
                pixel|= (_XILI_ROUND_U8(values[i])) << bLSB;
                mask |= 0xFF << bLSB;
                break;

              case 1:
                pixel|= (_XILI_ROUND_U8(values[i])) << gLSB;
                mask |= 0xFF << gLSB;
                break;

              case 2:
                pixel|= (_XILI_ROUND_U8(values[i])) << rLSB;
                mask |= 0xFF << rLSB;
                break;
            }
        }
        break;
    }

    XSetPlaneMask(xDisplay, xGC, mask);

    XSetForeground(xDisplay, xGC, pixel);

    XDrawPoint(xDisplay, xWindow, xGC, x, y);

    XSetPlaneMask(xDisplay, xGC, 0xffffffff);

    return XIL_SUCCESS;
}

XilStatus
XilDeviceIOxshm::setAttribute(const char* attribute_name,
			      void*       value)
{
    
    if(!strcmp(attribute_name,"XCOLORMAP")) {
	XilColorList* clist = (XilColorList*)value;
	
	XStoreColors(xDisplay, clist->cmap, clist->colors, clist->ncolors);
        XFlush(xDisplay);

        return XIL_SUCCESS;
    }

    return XIL_FAILURE;   
}


XilStatus
XilDeviceIOxshm::getAttribute(const char* attribute_name,
			      void**      value)
{
    //
    //  These are required I/O Device Attributes.
    //
    if(!strcmp(attribute_name, "WINDOW")) {
	*value = (void*)xWindow;
    } else if(!strcmp(attribute_name, "DISPLAY")) {
        *value = (void*)origDisplay;
    } else if(!strcmp(attribute_name, "DRAWABLE")) {
        if (doubleBuffered)
            *value = (void*)offScreenBuffer;
        else
            *value = (void*)xWindow;
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
        int screen = DefaultScreen(xDisplay);

        colorspace = systemState->createXilColorspace(xDisplay, screen,
                                                      xWindowAttribs.visual);
        if(colorspace == NULL) {
            //
            //  No colorspace set on the window create a default
            //  check the gamma value to determine whether to use
            //  the linear version of the colorspace
            //
            double gamma;
            XSolarisGetVisualGamma(xDisplay, screen,
                                   xWindowAttribs.visual, &gamma);

            if(XILI_FLT_EQ(gamma, 1.0)) {
                colorspace = (XilColorspace*)systemState->getObjectByName("rgblinear",
                                                                       XIL_COLORSPACE);
            } else {
                colorspace = (XilColorspace*)systemState->getObjectByName("rgb709",
                                                                       XIL_COLORSPACE);
            }
        } else {
            //
            //  Indicate we didn't get it "by name" so destroy it when we go away...
            //
            destroyColorspace = TRUE;
        }
#else
        colorspace = (XilColorspace*)systemState->getObjectByName("rgb709",
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
            if(!XGetWindowAttributes(xDisplay, xWindow, &win_attr)) {
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

            XQueryColors(xDisplay, win_attr.colormap, colors, 256);

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
XilDeviceIOxshm::isReadable()
{
    return TRUE;
}

Xil_boolean
XilDeviceIOxshm::isWritable()
{
    return TRUE;
}

//------------------------------------------------------------------------
//
//  Function:	Xshm Molecules
//
//  Description:
//	
//	Molecules for operations->display
//	
//	
//	
//	
//  MT-level:  SAFE
//	
//------------------------------------------------------------------------
XilStatus
XilDeviceIOxshm::setValueDisplayPreprocess(XilOp*        op,
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

    unsigned int child_offset;
    op->getParam(2, &child_offset);

    if(controllingImage->getNumBands() != num_disp_bands || child_offset != 0) {
        return XIL_FAILURE;
    }

    return XIL_SUCCESS;
}

XilStatus
XilDeviceIOxshm::setValueDisplay(XilOp*       op,
                                 unsigned int ,
                                 XilRoi*      roi,
                                 XilBoxList*  bl)
{
    //
    //  Get the set_value colors...
    //
    XilOp* src_op = op->getOpList()[1];

    //
    //  Get information about the output image
    //
    unsigned int   nbands = controllingImage->getNumBands();
    XilDataType    data_type = controllingImage->getDataType();

    //
    //  Get the values...
    //
    void* values;
    src_op->getParam(1, &values);

    //
    //  Loop over each of the boxes of storage we are to process.
    //
    XilBox* dst_box;
    while(bl->getNext(&dst_box)) {
	//
	// Get the co-ordinates of the box, we need to adjust to
	// absolute image co-ordinates in the destination.
	//
	int          abs_x;
	int          abs_y;
	unsigned int abs_w;
	unsigned int abs_h;
	dst_box->getAsRect(&abs_x, &abs_y, &abs_w, &abs_h);

        //
        //  Determine the color to set the XGC.
        //
        //  TODO: 12/5/96 jlf  For now, we only support XIL_BYTE.
        //
        Xil_unsigned8* vals  = (Xil_unsigned8*)values;
        unsigned long  color = 0;

        if (xDepth == 24) {
            for(int i=0; i<nbands; i++) {
                switch(i) {
                  case 0:
                    color |= vals[0] << bLSB;
                    break;
                  case 1:
                    color |= vals[1] << gLSB;
                    break;
                  case 2:
                    color |= vals[2] << rLSB;
                    break;
                }
            }

        } else {
            color = vals[0];
        }

        XSetForeground(xDisplay, xGC, color);

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
	    XFillRectangle(xDisplay, xWindow, xGC,
                           (int)x+abs_x, (int)y+abs_y, x_size, y_size);
	}
    }
	    
    return XIL_SUCCESS;
}


XilStatus
XilDeviceIOxshm::cast1to8DisplayPreprocess(XilOp*        op,
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
	bitXImage = XCreateImage(xDisplay, xWindowAttribs.visual, 
                                 1, XYBitmap,
                                 0, 0, width, height, 8, width);
	if(bitXImage == NULL) {
	    return XIL_FAILURE;
	}

        //
	//  Data in is always MSBFirst
        //
	bitXImage->bitmap_bit_order = MSBFirst;
	bitXImage->byte_order = MSBFirst;

	bitPixmap = XCreatePixmap(xDisplay, xWindow, width, height, 1);

        //
        //  Create the GC for correct copying of the data into the pixmap.
        //
        XGCValues gc_val;

        gc_val.background = BlackPixel(xDisplay, DefaultScreen(xDisplay));
        gc_val.foreground = WhitePixel(xDisplay, DefaultScreen(xDisplay));
        gc_val.function= GXcopy;
        gc_val.graphics_exposures = False;

        castGC = XCreateGC(xDisplay, bitPixmap, 
                           GCForeground|GCBackground|GCFunction|GCGraphicsExposures, 
                           &gc_val);
        if(!castGC) {
            return XIL_FAILURE;
        }
    }

    return XIL_SUCCESS;
}

XilStatus
XilDeviceIOxshm::cast1to8Display(XilOp*       op,
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
	int          abs_x;
	int          abs_y;
	unsigned int abs_w;
	unsigned int abs_h;
	dst_box->getAsRect(&abs_x, &abs_y, &abs_w, &abs_h);

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
            XPutImage(xDisplay, bitPixmap, castGC, &x_image,
                      (int)x, (int)y, (int)x, (int)y, x_size, y_size);

            //
            //  Copy the bit pixmap into the X window.
            //
            XCopyPlane(xDisplay, bitPixmap, xWindow, xGC,
                       (int)x, (int)y, x_size, y_size,
                       (int)x+abs_x, (int)y+abs_y, 1);
	}
    }

    XFlush(xDisplay);

    return XIL_SUCCESS;
}
