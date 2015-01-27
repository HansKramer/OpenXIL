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
//  File:	XilDeviceIOcg6.cc
//  Project:	XIL
//  Revision:	1.34
//  Last Mod:	10:13:52, 03/10/00
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
#pragma ident	"@(#)XilDeviceIOcg6.cc	1.34\t00/03/10  "

#include <stdio.h>
#include "XilDeviceIOcg6.hh"
#include "XiliUtils.hh"

//
// In this constructor we just save the things we
// need. The core will call us when they want the
// controlling image.
//
XilDeviceIOcg6::XilDeviceIOcg6(XilSystemState*         state,
			       Display*                dpy,
			       Window                  win,
			       XilDeviceManagerIOcg6*  mgr) :
    XilDeviceIO(mgr)
{
    isOKFlag      = FALSE;

    //
    //  Save the passed in information
    //
    stateptr      = state;
    displayptr    = dpy;
    window        = win;
    deviceManager = mgr;

    //
    //  NULL out the controllingImage and deviceLookup
    //
    controllingImage = NULL;
    deviceLookup     = NULL;

    //
    //  Colorspace attribute information.
    destroyColorspace = FALSE;
    colorspace        = NULL;

    //
    //  Initialize the colormap variables for this window.  The xcmap
    //  and the dga_cmap are initially set to NULL values to indicate
    //  that the user has not set the X_COLORMAP device attribute.
    //  The colormap information is initialized each time the user
    //  calls X_COLORMAP.
    //
    xcmap = 0;
    dga_cmap = NULL;

    //
    //  Here we connect to DGA and turn the window we've been given
    //  into a DGA window so we can access the hardware directly.
    //
    dga_draw = XDgaGrabDrawable(displayptr, window);
    if(dga_draw == NULL) {
        XIL_ERROR(stateptr,XIL_ERROR_SYSTEM,"di-219",TRUE);
        return;
    }

    if(dga_draw_rtngrab(dga_draw)) {
	retained_grabbed = TRUE;
    } else {
	dga_draw_rtnungrab(dga_draw);
	retained_grabbed = FALSE;
    }

    //
    //  Create or get the already created device information from the
    //   CG6 type.
    //
    CG6Description* cg6_description;

    cg6_description = mgr->getCG6Description(dga_draw_devname(dga_draw),
					     stateptr);
    if(!cg6_description) {
        //
        //  TODO: generate an appropriate error
        //
        return;
    }

    //
    //  Initialize our own variables.
    //
    fd        = cg6_description->fd;
    fb_width  = cg6_description->fb_width;
    fb_height = cg6_description->fb_height;
    fb_mem    = cg6_description->fb_mem;
    fb_fbc    = cg6_description->fb_fbc;

    //
    //  Retained window stuff
    //
    modif_flag     = 0;
    rtnchg_flag    = 0;
    rtnactive_flag = 0;
    rtn_width      = 0;
    rtn_height     = 0;
    rtn_linebytes  = 0;
    rtn_cached     = 0;
    bs_ptr         = NULL;
    
    dgaLockRefCnt = 0;
    
    isOKFlag      = TRUE;
}

//
// Destructor, clean up things we created.
//
XilDeviceIOcg6::~XilDeviceIOcg6()
{
    if(retained_grabbed) {
	dga_draw_rtnungrab(dga_draw);
	retained_grabbed = FALSE;
    }

    if(destroyColorspace) {
        colorspace->destroy();
    }

    XDgaUnGrabDrawable(dga_draw);
}

//
// Create the image associated with our device.
//
XilImage*
XilDeviceIOcg6::constructControllingImage()
{
    //
    // find out the window information
    //
    Window root;
    int    x,y;
    unsigned int width, height;
    unsigned int border_width;
    unsigned int x_depth;
    
    Status status = XGetGeometry(displayptr, window,&root, &x, &y, &width,
				 &height, &border_width, &x_depth);
    if(status==0) {
	XIL_ERROR(stateptr,XIL_ERROR_SYSTEM,"di-220",TRUE);
	return NULL;
    }

    //
    // Create the controlling image
    //
    controllingImage = stateptr->createXilImage(width, height, 1,
						XIL_BYTE);
    if(controllingImage == NULL) {
	XIL_ERROR(stateptr,XIL_ERROR_SYSTEM,"di-TODO",TRUE);
	return NULL;
    }

    return controllingImage;
}
      
XilStatus
XilDeviceIOcg6::display(XilOp*       op,
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
    // Get our backing store image. The display op
    // guarantees that if we have a parent image it
    // will pass it in on the op. We don't ask or use
    // the offset band that is also passed to us as
    // we are a single band image.
    //
    XilImage*    srcImage = op->getSrcImage(1);

    //
    //  Loop over each of the boxes of storage we are to process.
    //
    XilBox*    src_box;
    XilBox*    dst_box;
    while(bl->getNext(&src_box, &dst_box)) {
	//
        //  Aquire our storage from the source image.  The storage returned is
        //  valid for the box given.  Thus, any origins or child offsets have
        //  been taken into account.
        //
        XilStorage src_storage(srcImage);
	if(srcImage->getStorage(&src_storage, op, src_box, "XilMemory",
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
	int          abs_x;
	int          abs_y;
	unsigned int abs_w;
	unsigned int abs_h;
	dst_box->getAsRect(&abs_x, &abs_y, &abs_w, &abs_h);

        //
        //  Lock the DGA window for our use -- we don't need the registers
        //  locked since we're not modifying them.
        //
        lockDGA(TRUE, FALSE);
    
        //
        //  Intersect the cliplist with the ROI, to give us a RectList
        //  for the entire image.
        //
        //  The cliplist is relative to the framebuffer and we need
        //  the window x, y to generate a good XilRectList
        //
        XilRectList crl(roi, dgaClipList, winX, winY);

	//
	// The cg6 only supports single banded images which
	// default to pixel sequential so no check is made
	// here for storage type.
	// 
	unsigned int   src_pixel_stride;
	unsigned int   src_scanline_stride;
	Xil_unsigned8* src_data;
	src_storage.getStorageInfo(&src_pixel_stride,
				   &src_scanline_stride,
				   NULL, NULL,
				   (void**)&src_data);

        if(src_pixel_stride == 1) {
            //
            //  Update the retained image if needed, we only check the
            //  bs_ptr as only when this is valid do we actually need
            //  to do anything.
            //
            if(bs_ptr != NULL) {
                //
                //  Note the rectlist is generated from the intersection
                //  of the ROI and the box.
                //
                XilRectList  rl(roi, dst_box);

                int          x;
                int          y;
                unsigned int x_size;
                unsigned int y_size;
                while(rl.getNext(&x, &y, &x_size, &y_size)) {
                    unsigned char* src =
                        src_data + y*src_scanline_stride + x;
                    unsigned char* dst =
                        bs_ptr + y*rtn_linebytes + x;

                    do {
                        xili_memcpy(dst, src, x_size);

                        dst += rtn_linebytes;
                        src += src_scanline_stride;
                    } while(--y_size);
                }
            }

            //
            //  Note the rectlist is generated from the intersection
            //  of the clip rectlist and the box.
            //
            XilRectList  rl(&crl, dst_box);

            int          x;
            int          y;
            unsigned int x_size;
            unsigned int y_size;

            abs_x += winX;
            abs_y += winY;
            while(rl.getNext(&x, &y, &x_size, &y_size)) {
                unsigned char* src =
                    src_data + y*src_scanline_stride + x;
                unsigned char* dst =
                    fb_mem + (y+abs_y)*fb_width + (x+abs_x);

                do {
                    xili_memcpy(dst, src, x_size);

                    dst += fb_width;
                    src += src_scanline_stride;
                } while(--y_size);
            }
        } else {
            //
            //  Update the retained image if needed, we only check the
            //  bs_ptr as only when this is valid do we actually need
            //  to do anything.
            //
            if(bs_ptr != NULL) {
                //
                //  Note the rectlist is generated from the intersection
                //  of the ROI and the box.
                //
                XilRectList  rl(roi, dst_box);

                int          x;
                int          y;
                unsigned int x_size;
                unsigned int y_size;

                while(rl.getNext(&x, &y, &x_size, &y_size)) {
                    unsigned char* src =
                        src_data + y*src_scanline_stride + x;
                    unsigned char* dst =
                        bs_ptr + y*rtn_linebytes + x;

                    do {
                        Xil_unsigned8* src_pixel = src;
                        Xil_unsigned8* dst_pixel = dst;
                        unsigned int   width     = x_size;

                        do {
                            *dst_pixel++ = *src_pixel;

                            src_pixel += src_pixel_stride;
                        } while(--width);

                        dst += rtn_linebytes;
                        src += src_scanline_stride;
                    } while(--y_size);
                }
            }

            //
            //  Note the rectlist is generated from the intersection
            //  of the clip rectlist and the box.
            //
            XilRectList  rl(&crl, dst_box);

            int          x;
            int          y;
            unsigned int x_size;
            unsigned int y_size;

            abs_x += winX;
            abs_y += winY;
            while(rl.getNext(&x, &y, &x_size, &y_size)) {
                unsigned char* src =
                    src_data + y*src_scanline_stride + x;
                unsigned char* dst =
                    fb_mem + (y+abs_y)*fb_width + (x+abs_x);

                do {
                    Xil_unsigned8* src_pixel = src;
                    Xil_unsigned8* dst_pixel = dst;
                    unsigned int   width     = x_size;

                    do {
                        *dst_pixel++ = *src_pixel;

                        src_pixel += src_pixel_stride;
                    } while(--width);

                    dst += fb_width;
                    src += src_scanline_stride;
                } while(--y_size);
            }
        }
	
        //
        // Unlock the DGA window -- not the registers.
        //
        unlockDGA(FALSE);
    }

    return XIL_SUCCESS;
}

XilStatus
XilDeviceIOcg6::capture(XilOp*       op,
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
    // Get our backing store image. The capture op
    // guarantees that if we have a parent image it
    // will pass it in on the op.
    //
    XilImage*    dstImage = op->getDstImage(1);
    
    //
    //  Loop over each of the boxes of storage we are to process.
    //
    XilBox* src_box;
    XilBox* dst_box;
    while(bl->getNext(&src_box, &dst_box)) {
	//
        //  Aquire our storage from the images.  The storage returned is valid
        //  for the box given.  Thus, any origins or child offsets have been
        //  taken into account.
        //
        XilStorage  dst_storage(dstImage);
	if(dstImage->getStorage(&dst_storage, op, dst_box, "XilMemory",
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
	src_box->getAsRect(&abs_x, &abs_y, &abs_w, &abs_h);
	
	//
	//  Lock the DGA window for our use.
	//
	lockDGA();

	//
	//  Intersect the cliplist with the ROI, to give us a RectList
	//  for the entire image.
	//
	//  The cliplist is relative to the framebuffer and we need
	//  the window x, y to generate a good XilRectList
	//
	XilRectList    crl(roi, dgaClipList, winX, winY);

	//
	// The cg6 only supports single banded images which
	// default to pixel sequential so no check is made
	// here for storage type.
	// 
	unsigned int   dst_pixel_stride;
	unsigned int   dst_scanline_stride;
	Xil_unsigned8* dst_data;
	dst_storage.getStorageInfo(&dst_pixel_stride,
				   &dst_scanline_stride,
				   NULL, NULL,
				   (void**)&dst_data);

	//
	// Loop over the rectangles in the ROI
	// all of the regions are guaranteed not to go outside
	// the image
	//
	XilRectList  rl(&crl, src_box);
	
	int          x;
	int          y;
	unsigned int x_size;
	unsigned int y_size;
	unsigned char* src;
	unsigned char* dst;
	
	abs_x += winX;
	abs_y += winY;

	while(rl.getNext(&x, &y, &x_size, &y_size)) {
	    src = fb_mem + (y+abs_y)*fb_width + (x+abs_x);
	    
	    dst = dst_data + y*dst_scanline_stride + x*dst_pixel_stride;

	    //
	    // 1.2 had a check in here for Sun4 based P4
	    // cg6 cards. Solaris 2.5 dropped support for
	    // Sun4 machines, so I'm dropping it in here too.
	    //

	    if(dst_pixel_stride != 1) {
		//
		// The source image is a 1-banded image, and therefore
		// the BAND_SEQUENTIAL and GENERAL cases reduce to
		// PIXEL_SEQUENTIAL except in the case where the pixel
		// stride is not 1.
		// 
		for(int i=0; i<y_size; i++) {
		    unsigned char* src_scanline = src;
		    Xil_unsigned8* dst_scanline = dst;
		    for(int j=0; j<x_size; j++) {
			unsigned char* src_pixel = src_scanline;
			Xil_unsigned8* dst_pixel = dst_scanline;

			*dst_pixel = *src_pixel;
			dst_pixel += dst_pixel_stride;
			src_pixel++;
		    }
		    src_scanline += fb_width;
		    dst_scanline += dst_scanline_stride;
		}
	    } else {
		for(int i=0; i<y_size; i++) {
		    xili_memcpy(dst, src, x_size);
		
		    src += fb_width;
		    dst += dst_scanline_stride;
		}
	    }
	}

	//
	//  Unlock the DGA window.
	//
	unlockDGA();
    }
    
    return XIL_SUCCESS;
}

//
// Set a pixel, always a single banded image, child images
// must therefore always be single banded.
//
XilStatus
XilDeviceIOcg6::setPixel(unsigned int x,
			 unsigned int y,
			 float*       values,
			 unsigned int,
			 unsigned int)
{
    //
    //  Lock the DGA window so I can access the framebuffer...need retained
    //  grab locked so we can update it at the same time. 
    //
    lockDGA(TRUE);

    //
    //  Transform the point into screen space.
    //
    x += winX;
    y += winY;
    
    //
    //  Loop over the cliplist to figure out which rectangle contains
    //  the pixel we're looking for.
    //
    short* cliplist = dgaClipList;

    while(*cliplist != DGA_Y_EOL) {
        if((y >= (unsigned short)cliplist[0]) &&
           (y <= (unsigned short)cliplist[1])) {
            cliplist += 2;
            while(*cliplist != DGA_X_EOL) {
                if((x >= (unsigned short)cliplist[0]) &&
                   (x <= (unsigned short)cliplist[1])) break;
                cliplist += 2;
            }
            if(*cliplist != DGA_X_EOL) break;
            cliplist++;
        } else {
            cliplist += 2;
            while(*cliplist != DGA_X_EOL) cliplist += 2;
            cliplist++;
        }
    }

    //
    //  TODO:  11/13/96 jlf  Update retained window.
    //
    
    if(*cliplist != DGA_Y_EOL) {
        //
        //  Set the pixel now that we've found the right rectangle on
        //    the screen.
        //
        Xil_unsigned8* dst = fb_mem + y*fb_width + x;

        //
        //  Round the pixel up.
        //
        float value = values[0] + 0.5;
        
        //
        //  Clip the value properly and set it on the screen.
        //
        if(value > 255.0) {
            *dst = 255;
        } else if (value < 0.0) {
            *dst = 0;
        } else {
            *dst = (unsigned char) value;
        }
    }

    //
    //  Unlock the window...
    //
    unlockDGA();
    
    return XIL_SUCCESS;
}

//
// get a pixel, always a single banded image, child images
// must therefore always be single banded.
//
XilStatus
XilDeviceIOcg6::getPixel(unsigned int x,
			 unsigned int y,
			 float*       values,
			 unsigned int,
			 unsigned int)
{
    //
    //  Lock the DGA window so I can access the framebuffer...don't need
    //  retained grab locked for this. 
    //
    lockDGA(FALSE);

    //
    //  Transform the point into screen space.
    //
    x += winX;
    y += winY;
    
    //
    //  Loop over the cliplist to figure out which rectangle contains
    //  the pixel we're looking for.
    //
    short* cliplist = dgaClipList;

    while(*cliplist != DGA_Y_EOL) {
        if((y >= (unsigned short)cliplist[0]) &&
           (y <= (unsigned short)cliplist[1])) {
            cliplist += 2;
            while(*cliplist != DGA_X_EOL) {
                if((x >= (unsigned short)cliplist[0]) &&
                   (x <= (unsigned short)cliplist[1])) break;
                cliplist += 2;
            }
            if(*cliplist != DGA_X_EOL) break;
            cliplist++;
        } else {
            cliplist += 2;
            while(*cliplist != DGA_X_EOL) cliplist += 2;
            cliplist++;
        }
    }
    
    if(*cliplist != DGA_Y_EOL) {
        //
        //  Return what's on the screen.
        //
        Xil_unsigned8* src = fb_mem + y*fb_width + x;
	values[0] = *src;
    } else {
	//
        //  Data point is obscured by another window so return 0.0
        //
        values[0] = 0.0;
    }

    //
    //  Unlock the window...
    //
    unlockDGA();

    return XIL_SUCCESS;
}

XilStatus
XilDeviceIOcg6::setAttribute(const char* attribute_name,
			     void*       value)
{
    if(!strcmp(attribute_name, "XCOLORMAP")) {
	XilColorList*  clist   = (XilColorList*)value;
        
	if(clist->cmap != xcmap) {
	    if(clist->cmap != 0) {
		//
		//  UnGrab Cmap Grabber
		//
		XDgaUnGrabColormap(displayptr, xcmap);
		dga_cmap = NULL;
	    }
	    xcmap = 0;
	    
	    //
	    //  Connect to the Cmap Grabber
	    //
	    Dga_token dga_token = XDgaGrabColormap(displayptr, clist->cmap);
	    if(dga_token==NULL) {
      		XIL_ERROR(stateptr,XIL_ERROR_SYSTEM,"di-219",TRUE);
		return XIL_FAILURE;
	    }
	    if((dga_cmap =
                ((Dga_cmap) dga_cm_grab(dga_draw_devfd(dga_draw),
                                        dga_token))) == NULL) {
      		XIL_ERROR(stateptr,XIL_ERROR_SYSTEM,"di-219",TRUE);
		XDgaUnGrabColormap(displayptr, clist->cmap);
		return XIL_FAILURE;
	    }

	    //
	    //  Mmap Hardware
	    //
	    if((cg6cmap = (struct cg6_cmap*)
		mmap(NULL, CG6_CMAP_SZ, PROT_READ|PROT_WRITE,
		     MAP_SHARED, fd, CG6_VADDR_CMAP)) ==
               (struct cg6_cmap*)-1) {
      		XIL_ERROR(stateptr,XIL_ERROR_SYSTEM,"di-215",TRUE);
		return XIL_FAILURE;
	    }

            dga_cm_set_client_infop(dga_cmap, cg6cmap);

	    xcmap = clist->cmap;
	}
	
	XColor*        colors  = clist->colors;
	Xil_unsigned32 ncolors = clist->ncolors;
	int            index   = colors[0].pixel;

	//
	// Check to see if the colors are linear and convert
	//
	static unsigned char    red[256];
	static unsigned char    green[256];
	static unsigned char    blue[256];
	for(int i=index,j=0; i<(ncolors+index); i++,j++) {
	    if(colors[i-index].pixel != i) {
		XStoreColors(displayptr, clist->cmap, colors, ncolors);
		return XIL_SUCCESS;
	    }
            
	    red[j]   = colors[j].red>>8;
	    green[j] = colors[j].green>>8;
	    blue[j]  = colors[j].blue>>8;
	}

	//
	//  Install the new colormap.
	//
	void    install_cmap(Dga_cmap       dga_cmap,
			     int            index,
			     int            count,
			     unsigned char* red,
			     unsigned char* green,
			     unsigned char* blue);
        dga_cm_write(dga_cmap, index, (int)ncolors,
                     red, green, blue, install_cmap);
        
        return XIL_SUCCESS;
    }
    
    return XIL_FAILURE;
}

XilStatus
XilDeviceIOcg6::getAttribute(const char* attribute_name,
			     void**      value)
{
    //
    //  The first four attributes are required to exist.
    //
    if(!strcmp(attribute_name,"WINDOW")) {
	Window *win = (Window *) value;
        *win= window;
    } else if(!strcmp(attribute_name,"DISPLAY")) {
        *value= (void*)displayptr;
    } else if(!strcmp(attribute_name, "COLORSPACE")) {
        //
        //  Get a colorspace object that represents our device.
        //
        deviceMutex.lock();

        if(colorspace != NULL) {
            *value = (void*)colorspace;

            deviceMutex.unlock();

            return XIL_SUCCESS;
        }

#ifdef SOLARIS
        //
        //  This is a utility constructor that checks for
        //  the presence of an ICC profile for the particular
        //  visual we are interested in
        //
        int screen = DefaultScreen(displayptr);

        XWindowAttributes win_attr;
        if(!XGetWindowAttributes(displayptr, window, &win_attr)) {
            XIL_ERROR(controllingImage->getSystemState(),
                      XIL_ERROR_SYSTEM, "di-249", TRUE);
            return XIL_FAILURE;
        }

        colorspace = stateptr->createXilColorspace(displayptr, screen,
                                                   win_attr.visual);
        if(colorspace == NULL) {
            colorspace = (XilColorspace*)stateptr->getObjectByName("rgb709",
                                                                   XIL_COLORSPACE);
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

        deviceMutex.unlock();
    } else if(!strcmp(attribute_name, "COLORMAP")) {
        //
        //  Return a lookup that represents the layout of the lookup
        //  associated with the device.  For 3 banded images, this is
        //  meaningless.
        //
        if(controllingImage->getNumBands() == 1) {
            deviceMutex.lock();

            if(deviceLookup == NULL) {
                //
                //  Create a lookup that represents the layout of an
                //    X Colormap.
                //
                deviceLookup =
                    stateptr->createXilLookupSingle(XIL_BYTE, XIL_BYTE,
                                                 3, 256, 0, lookupData);
            }

            XWindowAttributes win_attr;
            if(!XGetWindowAttributes(displayptr, window, &win_attr)) {
                XIL_ERROR(controllingImage->getSystemState(),
                          XIL_ERROR_SYSTEM, "di-249", TRUE);
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

            deviceMutex.unlock();

            *value = (void*)deviceLookup;
        } else {
            *value = (void*)NULL;
        }
    } else if(!strcmp(attribute_name,"FBC")) {
        *value= (void*)fb_fbc;
    } else if(!strcmp(attribute_name,"DGA_WIN")) {
        *value= (void*)dga_draw;
    } else if(!strcmp(attribute_name,"DGA_DRAW")) {
        *value= (void*)dga_draw;
    } else {
        return XIL_FAILURE;
    }
    
    return XIL_SUCCESS;
}

Xil_boolean
XilDeviceIOcg6::isReadable()
{
    return TRUE;
}

Xil_boolean
XilDeviceIOcg6::isWritable()
{
    return TRUE;
}

//
// Private methods/callbacks
//
static
void
install_cmap(Dga_cmap       dga_cmap,
	     int            index,
	     int            count,
	     unsigned char* red,
	     unsigned char* green,
	     unsigned char* blue)
{
    cg6_cmap* cg6cmap = (cg6_cmap*)dga_cm_get_client_infop(dga_cmap);
    
    //
    //  Store colors side-by-side
    //
    static unsigned char cmap[3*256];
    for(int i=0,j=0; j<count; i+=3,j++) {
        cmap[i]   = red[j];
        cmap[i+1] = green[j];
        cmap[i+2] = blue[j];
    }
    
    //
    //  cg6 Cmap
    //
    cg6cmap->addr = index << 24;
    
    int             nument  = (((count<<1)+count)>>2);  // ncolors*3
    volatile u_int* hw_cmap = &cg6cmap->cmap;
    int*            incmap = (int*) cmap;
    for(i=0; i<nument; i++, incmap++) {
        *hw_cmap = *incmap;
        *hw_cmap = *incmap << 8;
        *hw_cmap = *incmap << 16;
        *hw_cmap = *incmap << 24;
    }
}

#include <thread.h>

//
//  This method checks to see if the window needs to
//  be locked using DGA_DRAW_LOCK(), it also saves the
//  modif flag and other retained information because we only
//  need to ask for it once. The modif flag is also cleared after the
//  first read.
//
XilMutex XilDeviceIOcg6::dgaRegistersMutex;

void
XilDeviceIOcg6::lockDGA(Xil_boolean lock_retained_window,
                        Xil_boolean lock_registers)
{
    if(lock_registers) {
        dgaRegistersMutex.lock();
    } else {
        dgaLockMutex.lock();
    }

    if(dgaLockRefCnt++ == 0) {
        DGA_DRAW_LOCK(dga_draw, -1);

        if(lock_retained_window == TRUE && retained_grabbed == TRUE) {
            modif_flag = DGA_DRAW_MODIF(dga_draw);
            if(modif_flag) {
                rtnchg_flag = dga_draw_rtnchg(dga_draw);
                if(rtnchg_flag) {
                    rtnactive_flag = dga_draw_rtnactive(dga_draw);
                    if(rtnactive_flag) {
                        dga_draw_rtndimensions(dga_draw, &rtn_width, &rtn_height,
                                               &rtn_linebytes);

                        rtn_cached = dga_draw_rtncached(dga_draw);
                        if(rtn_cached == DGA_RTN_NOT_CACHED) {
                            bs_ptr = (unsigned char*)dga_draw_rtnpixels(dga_draw);
                        }
                    }
                }
            }
        }

        //
        //  We grab the cliplist every time since checking it may have
        //  cause problems with molecules that go directly to the dispaly
        //  which need to know if the window has been modified.  It's
        //  cheap enough.
        //
        //
        dgaClipList = dga_draw_clipinfo(dga_draw);

        dga_draw_bbox(dga_draw, &winX, &winY, &winWidth, &winHeight);
    }        

    if(! lock_registers) {
        dgaLockMutex.unlock();
    }
}

//
//  This method checks to see if the window needs to
//  be unlocked using DGA_DRAW_LOCK(), if the dga_lock_count
//  drops to 0.
//
void
XilDeviceIOcg6::unlockDGA(Xil_boolean unlock_registers)
{
    if(! unlock_registers) {
        dgaLockMutex.lock();
    }

    if(--dgaLockRefCnt == 0) {
        DGA_DRAW_UNLOCK(dga_draw);

        //
        // Reset the retained values
        //
        if(retained_grabbed == TRUE) {
            modif_flag = 0;
            rtnchg_flag = 0;
            rtnactive_flag = 0;
            rtn_width = 0;
            rtn_height = 0;
            rtn_linebytes = 0;
            rtn_cached = 0;
            bs_ptr = NULL;
        }
    }

    if(unlock_registers) {
        dgaRegistersMutex.unlock();
    } else {
        dgaLockMutex.unlock();
    }
}
