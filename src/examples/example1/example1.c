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
*    File:	example1.c
*    Project:	XIL Example #1
*    Revision:	1.7
*    Last Mod:	10:24:08, 03/10/00
*  
*    Description:
*        This example reads a grayscale image from a file and displays
*        the image in an X window using XIL.
*        
*------------------------------------------------------------------------
*
*    COPYRIGHT
*
*----------------------------------------------------------------------*/
#ifndef lint
static	char     sccsid[] = "@(#)example1.c	1.7\t00/03/10  ";
#endif

#include <stdio.h>
#include <stdlib.h>
#include <xil/xil.h>

#define NCOLORS 216

extern XilImage load_example_image_file(XilSystemState state,
                                        char*          pathname);

extern XilImage open_window(XilSystemState state,
                            unsigned int   width,
                            unsigned int   height,
                            unsigned int   nbands,
                            XilDataType    datatype,
                            XVisualInfo*   vinfo,
                            Xil_boolean    prefer_depth_24);

/*
 * main() -- example program to read a grayscale image from a file and put
 *           it on the display using XIL.
 */
main(int argc, char **argv)
{
    /*
     *  Our XIL system state
     */
    XilSystemState state;

    /*
     *  X Display Information
     */
    Display*       xdisplay;
    XVisualInfo    vinfo;
    XEvent         xevent;

    /*
     *  Assorted XIL images.
     */
    XilImage       image          = NULL;
    XilImage       bit_image      = NULL;
    XilImage       retained_image = NULL;
    XilImage       display_image  = NULL;
    XilImage       band0          = NULL;
    XilImage       band1          = NULL;
    XilImage       band2          = NULL;

    /*
     *  XIL Image dimensions and datatype
     */
    unsigned int   width;
    unsigned int   height;
    unsigned int   nbands;
    XilDataType    datatype;

    /*
     *  Rescale Parameters
     */
    float          mult;
    float          offset;

    /*
     *  Colorcube/Colormap Information
     *
     *  The multiplier of -1 and offset of 0 assumes entry 0 of colormap is
     *  WHITE and entry 1 of colormap is black. 
     */
    XilLookup      colorcube   = NULL;
    int            multipliers = -1;
    unsigned int   dimensions  =  2;

    /*
     *  Error diffusion kernel
     */
    XilKernel      distribution;

    /*
     *  Pathname argument
     */
    char*          pathname = NULL;
    
    /*
     *  Get the pathname from the command line.
     */
    while(--argc) {
        if((*++argv)[0] == '-') {
            switch((*argv)[1]) {
              case 'h':
                fprintf(stderr, "Usage: example1 [header pathname]\n");
                break;
            }
        } else {
            pathname = *argv;
        }
    }

    if(pathname == NULL) {
        fprintf(stderr, "Usage: example1 [header pathname]\n");
        exit(1);
    }

    /*
     *  Open the XIL library
     */
    state = xil_open();
    if(state == NULL) {
        /*
         *  XIL's default error handler will print an error msg on stderr
         */
        exit(1);
    }

    image = load_example_image_file(state, pathname);
    if(image == NULL) {
        /*
         *  XIL and the load_example_image_file() routine will have generated
         *  enough of an error.
         */
        exit(1);
    }

    /*
     *  Get the image dimensions in order to create a window of the
     *  appropriate size.
     */
    xil_get_info(image, &width, &height, &nbands, &datatype);

    if((datatype != XIL_BYTE) || (nbands != 1)) {
        fprintf(stderr, "This example program requires a single band, 8 bit image\n");
        exit(1);
    }

    /*
     *  Use the open_window() utility to construct a display image.  We would
     *  prefer and 8-bit visual over a 24-bit visual.
     */
    display_image = open_window(state, width, height,
                                nbands, datatype, &vinfo, TRUE);

    if(display_image == NULL) {
        fprintf(stderr, "Failed to construct a display image\n"); 
        exit(1);
    }

    /*
     *  Modify the source image so that it can directly be copied into the
     *  display image.
     *
     *  retained_image is the image we copy into the display image below.
     */
    if(vinfo.class == TrueColor) {
        /*
         *  Copy the grayscale image into each of the bands
         */
        retained_image = xil_create(state, width, height, 3, XIL_BYTE);

        band0 = xil_create_child(retained_image, 0, 0, width, height, 0, 1);
        band1 = xil_create_child(retained_image, 0, 0, width, height, 1, 1);
        band2 = xil_create_child(retained_image, 0, 0, width, height, 2, 1);

        xil_copy(image, band0);
        xil_copy(image, band1);
        xil_copy(image, band2);
    } else if(vinfo.class == GrayScale) {
        /*
         *  rescale the image to fill NCOLORS entries
         */
        mult      = NCOLORS/256.0;
        offset    = 256 - NCOLORS;

        xil_rescale(image, image, &mult, &offset);

        retained_image = image;
    } else if((vinfo.class == StaticGray) && (vinfo.depth == 8)) {
        retained_image = image;
    } else if((vinfo.class == StaticGray) && (vinfo.depth == 1)) {
        bit_image = xil_create(state, width, height, 1, XIL_BIT); 

        if(bit_image == NULL) {
            /*
             *  XIL's default error handler will print an error msg on stderr
             */
            exit(1);
        }

        distribution = xil_kernel_get_by_name(state, "floyd-steinberg");
        colorcube    = xil_colorcube_create(state, XIL_BIT, XIL_BYTE, 1, 0, 
                                            &multipliers, &dimensions);

        xil_error_diffusion(image, bit_image, colorcube, distribution);

        retained_image = bit_image;
    } else if(vinfo.class == PseudoColor) {
        /*
         *  rescale the image to fill NCOLORS entries
         */
        mult      = NCOLORS/256.0;
        offset    = 256 - NCOLORS;

        xil_rescale(image, image, &mult, &offset);

        retained_image = image;
    }

    /*
     *  Put the image onto the display.
     */
    xil_copy(retained_image, display_image);

    /*
     *  Get the X Display for the display image so we can aquire X events
     */
    if(xil_get_device_attribute(display_image, "DISPLAY",
                                (void**)&xdisplay) == XIL_FAILURE) {
        fprintf(stderr, "could not get XDISPLAY attribute of display_image\n");
        exit(1);
    }

    /*
     *  Continue to refresh window as necessary till program is terminated by
     *  any button press in the window.  A KeyPress in the window causes a
     *  negative of the image to be displayed.
     */
    while(1) {
        XNextEvent(xdisplay, &xevent);

        if(xevent.xany.type == KeyPress) {
            xil_not(retained_image, retained_image);

            /*
             *  For GrayScale, we need to move the negated image by the offset
             */
            if(vinfo.class == GrayScale) {
                xil_add_const(retained_image, &offset, retained_image);
            }

            xil_copy(retained_image, display_image);
        } else if(xevent.xany.type == Expose) {
            xil_copy(retained_image, display_image);
        } else if(xevent.xany.type == ButtonPress) {
            break;
        }
    }

    /*
     *  We are responsible for destroying all of the XIL objects we created.
     *  Like free(), XIL supports destroying NULL objects.
     */
    xil_destroy(image);
    xil_destroy(display_image);
    xil_destroy(bit_image);
    xil_destroy(band0);
    xil_destroy(band1);
    xil_destroy(band2);

    if(retained_image != image) {
        xil_destroy(retained_image);
    }

    xil_lookup_destroy(colorcube);

    /*
     *  We do not destroy the XilKernel because we didn't create it, we
     *  aquired it using the xil_kernel_get_by_name() call.
     */

    xil_close(state);

    XCloseDisplay(xdisplay);
}
