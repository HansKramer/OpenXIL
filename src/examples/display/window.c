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

/*-----------------------------------------------------------------------
* 
*    File:	window.c
*    Project:	XIL Example #1
*    Revision:	1.4
*    Last Mod:	10:24:11, 03/10/00
*  
*    Description:
*        This example opens an X window and creates and XIL
*        display image.
*        In addition, this file catches 3 banded image cases
*        and turns GrayScale into PsuedoColor.
*        
*------------------------------------------------------------------------
*
*    COPYRIGHT
*
*----------------------------------------------------------------------*/
#define MAXCOLORS 256

#include <stdio.h>
#include <stdlib.h>
#include <xil/xil.h>

#define NCOLORS 216


XilImage
open_window(XilSystemState state,
            unsigned int   width,
            unsigned int   height,
            unsigned int   nbands,
            XilDataType    datatype,
            XVisualInfo*   vinfo,
            Xil_boolean    prefer_depth_24)
{
    /*
     *  X Widnow and Display Information
     */
    Display*             xdisplay;
    Window               xwindow;
    XEvent               xevent;

    /*
     *  X structure for setting the window attributes
     */
    XSetWindowAttributes winattr;
    int                  attribmask;

    /*
     *  X colormap
     */
    Colormap             colormap;
    XColor               colors[MAXCOLORS];
    unsigned long        pixels[MAXCOLORS];
    unsigned long        pmask;
    unsigned long        pixel;
    int                  ncolors;
    int                  index;
    int                  base;

    /*
     *  Incrementing Variables
     */
    int                  i;
    int                  j;
    int                  k;

    /*
     *  XIL image we provide
     */
    XilImage             display_image;

    /*
     *  Connect to the X Server
     */
    xdisplay = XOpenDisplay(NULL);
    if(xdisplay == NULL) {
        fprintf(stderr, "open_window:  Unable to connect to X server\n");
        return NULL;
    }

    /*
     *  Find the "best" visual...
     */
    if(XMatchVisualInfo(xdisplay, DefaultScreen(xdisplay), 24,
                        TrueColor, vinfo) && prefer_depth_24) {
    } else if(XMatchVisualInfo(xdisplay, DefaultScreen(xdisplay), 8, GrayScale, vinfo)) {
    } else if(XMatchVisualInfo(xdisplay, DefaultScreen(xdisplay), 8, StaticGray, vinfo)) {
    } else if(XMatchVisualInfo(xdisplay, DefaultScreen(xdisplay), 1, StaticGray, vinfo)) {
    } else if(XMatchVisualInfo(xdisplay, DefaultScreen(xdisplay), 24, TrueColor, vinfo)) {
        /*
         *  No visual we can support found.
         */
        fprintf(stderr, "open_window:  Could not locate a supported visual\n");
	return NULL;
    }

    /*
     *  In the case of 3 banded images, turn grayscale into PseudoColor
     */
    if((vinfo->class == GrayScale) && (nbands == 3)) {
	if(XMatchVisualInfo(xdisplay, DefaultScreen(xdisplay), 8, PseudoColor, vinfo)) {
	} else if(XMatchVisualInfo(xdisplay, DefaultScreen(xdisplay), 8, GrayScale, vinfo)) {
	}
    }

    /*
     *  Construct a colormap for the window
     */
    colormap = XCreateColormap(xdisplay, DefaultRootWindow(xdisplay), 
                               vinfo->visual, AllocNone);

    /*
     *  For certain visuals (i.e. 24-bit from a root of 8-bit) we need to set
     *  the border and back pixel.
     */
    attribmask = CWColormap | CWBorderPixel | CWBackPixel;

    winattr.colormap         = colormap;
    winattr.border_pixel     = WhitePixel(xdisplay, DefaultScreen(xdisplay));
    winattr.background_pixel = BlackPixel(xdisplay, DefaultScreen(xdisplay));

    /*
     *  Install a gray ramp for a colormap if appropriate.
     */
    if(vinfo->class == GrayScale) {
        if(datatype == XIL_BIT) {
            ncolors = 2;
        } else {
            ncolors = NCOLORS;
        }
        base = MAXCOLORS - ncolors;

        /*
         *  Start by allocating all of the colors available to us.
         */
        i = XAllocColorCells(xdisplay, winattr.colormap, 1, &pmask,
                             0, pixels, MAXCOLORS);

        /*
         *  Free those we don't need.
         */
        for(pixel = 0; pixel < base; pixel++) {
            XFreeColors(xdisplay, winattr.colormap, &pixel, 1, 0);
        }

        /*
         *  Fill the colormap
         */
        index = 0;
        for(i = 0; i < ncolors; i++) {
            colors[index].pixel = index + base;
            colors[index].blue  = colors[index].green = colors[index].red =
                (255 * i / (ncolors - 1)) * 256;
            colors[index].flags = DoRed | DoGreen | DoBlue;
            index++;
        }

        XStoreColors(xdisplay, winattr.colormap, colors, ncolors);
    }

    /*
     *  Create the X Window
     */
    xwindow = XCreateWindow(xdisplay, DefaultRootWindow(xdisplay),
                            0, 0, width, height, 0,
                            vinfo->depth, InputOutput, vinfo->visual,
                            attribmask, &winattr);

    if(xwindow == NULL) {
        fprintf(stderr, "Unable to create window\n");
        return(0);
    }

    /*
     *  List window events used by example programs.
     */
    XSelectInput(xdisplay, xwindow,
                 ExposureMask | ButtonPressMask | KeyPressMask);

    /*
     *  Make the window visible -- XIL can only create from mapped and
     *  visible windows.
     */
    XMapWindow(xdisplay, xwindow);

    /*
     *  Wait for the window to be mapped (an Expose event)
     */
    while(1) {
        XNextEvent(xdisplay, &xevent);

        if(xevent.xany.type      == Expose &&
           xevent.xexpose.window == xwindow) {
            break;
        }
    }

    /*
     *  Create the XIL display image
     */
    display_image = xil_create_from_window(state, xdisplay, xwindow);

    return display_image;
}

