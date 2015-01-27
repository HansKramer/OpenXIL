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
*    File:	display.c
*    Project:	XIL Display Example 
*    Revision:	1.7
*    Last Mod:	10:24:12, 03/10/00
*  
*    Description:
*        This example reads an image from a file and displays
*        the image in an X window using XIL.
*        
*------------------------------------------------------------------------
*
*    COPYRIGHT
*
*----------------------------------------------------------------------*/
#ifndef lint
static	char     sccsid[] = "@(#)display.c	1.7\t00/03/10  ";
#endif

#include <stdio.h>
#include <stdlib.h>
#include <xil/xil.h>

#define CMAPSIZE 256		/* total entries in the colormap */
#define	NCOLORS	216		/* colormap entries uesd by this program */
#define SHORTSIZE 32768         /* number of values in a short datatype */
#define BYTESIZE 256            /* number of values in a byte datatype */
#define BITSIZE	2               /* number of values in a bit datatype */

extern XilImage load_example_image_file(XilSystemState state,
                                        char*          pathname);

extern XilImage open_window(XilSystemState state,
                            unsigned int   width,
                            unsigned int   height,
                            unsigned int   nbands,
                            XilDataType    datatype,
                            XVisualInfo*   vinfo,
                            Xil_boolean    prefer_depth_24);

void     set_colormap(XilImage      display_image,
                      Display       *xdisplay,
                      Window        xwindow,
                      XilLookup     cmap);

/*
 * main() -- example program to read an image from a file and put
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
    Window         xwindow;

    /*
     *  Assorted XIL images.
     */
    XilImage       image          = NULL;
    XilImage       bit_image      = NULL;
    XilImage       byte_image     = NULL;
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
     *  Range of data
     */
    float          low;
    float          high;

    /*
     *  Colorcube/Colormap Information
     *
     *  The multiplier of -1 and offset of 0 assumes entry 0 of colormap is
     *  WHITE and entry 1 of colormap is black. 
     */
    XilLookup      colorcube   = NULL;
    int            multipliers = -1;
    unsigned int   dimensions  =  2;

    XilLookup      lookup       = NULL;
    Xil_unsigned8  lookupdata[] = {0, 0, 0, 255, 255, 255};

    /*
     *  Colorspace information
     */ 
    XilColorspace  rgb_cspace;
    XilColorspace  ylinear_cspace;

    /*
     *  Ordered dither mask
     */
    XilDitherMask  dithermask;

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
                fprintf(stderr, "Usage: display [header pathname]\n");
                break;
            }
        } else {
            pathname = *argv;
        }
    }

    if(pathname == NULL) {
        fprintf(stderr, "Usage: display [header pathname]\n");
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

    if((nbands != 1) && (nbands != 3)) {
        fprintf(stderr, "This example program displays images with 1 or 3 bands;\n");
        fprintf(stderr, "Display of %d banded images not supported.\n",nbands);
        exit(1);
    }

    /*
     *  Use the open_window() utility to construct a display image.  We would
     *  prefer an 8-bit visual over a 24-bit visual.
     */
    display_image = open_window(state, width, height,
                                nbands, datatype, &vinfo, FALSE);

    if(display_image == NULL) {
        fprintf(stderr, "Failed to construct a display image\n"); 
	exit(1);
    }


    /*
     *  Get the X Display for the display image so we can aquire X events
     */
    if(xil_get_device_attribute(display_image, "DISPLAY",
                                (void**)&xdisplay) == XIL_FAILURE) {
        fprintf(stderr, "could not get XDISPLAY attribute of display_image\n");
        exit(1);
    }

    /*
     *  Modify the source image so that it can directly be copied into the
     *  display image.
     *
     *  retained_image is the image we copy into the display image below.
     */
    if(datatype == XIL_BIT) {
        if(nbands == 1) {
            switch(vinfo.class) {
	    case TrueColor:
		retained_image = xil_create(state, width, height, 3, XIL_BYTE);

		lookup = xil_lookup_create(state, XIL_BIT, XIL_BYTE, 3,
					   BITSIZE, 0, lookupdata);
		xil_lookup(image, retained_image, lookup);
		break;
	    case PseudoColor:
		fprintf(stderr,
		   "1 band images not supported for pseudocolor visuals\n");
		exit(1);
	    case GrayScale:
		retained_image = xil_create(state, width, height, nbands, XIL_BYTE);

                /*
                 *  Cast the bit image into a byte image
                 */
		xil_cast(image, retained_image);

                /*
                 *  Move the values into better values in the colormap  
                 */
		offset = CMAPSIZE - BITSIZE;
		xil_add_const(retained_image, &offset, retained_image);
		break;
	    case StaticGray:
		if(vinfo.depth == 8) {
		    retained_image = xil_create(state, width, height, nbands, XIL_BYTE);
                    
                    /*
                     *  Cast the bit image into a byte image
                     */
		    xil_cast(image, retained_image);
                    
		    /*
                     *  Remap the image to fill NCOLORS entries 
                    */
		    mult = CMAPSIZE - 1;
		    xil_multiply_const(retained_image, &mult, retained_image);
		}
		else if(vinfo.depth == 1) {
		    xil_not(image, image);
		    retained_image = image;
		}
		break;
	    }
	}
	else if(nbands == 3) {
	    switch(vinfo.class) {
              case TrueColor:
                fprintf(stderr,
                        "Multibanded bit images not supported by this program\n");
		exit(1);
              case PseudoColor:
              case GrayScale:
		/* GrayScale is handled by PseudoColor visuals */
		fprintf(stderr,
                        "Multibanded bit images not supported by this program\n");
		exit(1);
              case StaticGray:
		if (vinfo.depth == 8) {
		    /* not implemented */
		    exit(0);
		}
		else if(vinfo.depth == 1) {
		    fprintf(stderr,
			    "Multibanded bit images not supported by this program\n");
		    exit(1);
		}
	    }
	}
    }
    else if (datatype == XIL_BYTE) {
	if (nbands == 1) {
	    switch(vinfo.class) {
              case TrueColor:
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
                break;
              case PseudoColor:
		fprintf(stderr,
                        "1 band images not supported for pseudocolor visuals\n");
		exit(1);
              case GrayScale:
                /*
                 *  Rescale the image to fill NCOLORS entries
                 */
                mult      = (NCOLORS - 1)/(BYTESIZE -1);
                offset    = CMAPSIZE - NCOLORS;
                
                xil_rescale(image, image, &mult, &offset);
                
                retained_image = image;
                break;
              case StaticGray:
		if(vinfo.depth == 8) {
		    retained_image = image;
		} else if(vinfo.depth == 1) {
                    
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
		}
		break;
	    }
	}
	else if(nbands == 3) {
	    switch (vinfo.class) {
              case TrueColor:
		retained_image = image;
		break;
              case PseudoColor:
              {
                  retained_image = xil_create(state, width, height, 1, XIL_BYTE);
                  
                  colorcube = xil_lookup_get_by_name(state, "cc496");
                  dithermask = xil_dithermask_get_by_name(state, "dm443"); 
                  
                  if(xil_get_device_attribute(display_image, "WINDOW",
                                              (void**)&xwindow) == XIL_FAILURE) {
                      fprintf(stderr, "could not get WINDOW attribute of display_image\n");
                      exit(1);
                  }
                  set_colormap(display_image, xdisplay, xwindow, colorcube);
                  
                  xil_ordered_dither(image, retained_image, colorcube, dithermask);
              }
              break;
              case GrayScale:
		/* This case is handled by PseudoColor visuals */
                exit(0);
              case StaticGray:
		if(vinfo.depth == 8) {
		    /* not implemented */
                    exit(0);
		} else if(vinfo.depth == 1) {
		    bit_image = xil_create(state, width, height, 1, XIL_BYTE);
		    if (bit_image == NULL) {
                        /*
                         *  XIL's default error handler will print an error msg on stderr
                         */
			exit(1);
		    }
                    
                    rgb_cspace = xil_colorspace_get_by_name(state,"rgblinear");
		    xil_set_colorspace(image, rgb_cspace);
                    ylinear_cspace = xil_colorspace_get_by_name(state, "ylinear");
		    xil_set_colorspace(bit_image, ylinear_cspace); 
		    xil_color_convert(image, bit_image);
                    
		    retained_image = xil_create(state, width, height, 1, XIL_BIT);

		    distribution = xil_kernel_get_by_name(state, "floyd-steinberg");
		    colorcube = xil_colorcube_create(state, XIL_BIT, XIL_BYTE,
                                                     1, 0, &multipliers, &dimensions);

		    xil_error_diffusion(bit_image, retained_image, colorcube,
                                        distribution);
		}
		break;
	    }
	}
    }
    else if (datatype == XIL_SHORT) {
	if (nbands == 1) {
	    switch (vinfo.class) {
              case TrueColor:
		retained_image = xil_create(state, width, height, 3, XIL_BYTE);

		band0 = xil_create_child(retained_image, 0, 0, width, height,
					 0, 1);
		band1 = xil_create_child(retained_image, 0, 0, width, height,
					 1, 1);
		band2 = xil_create_child(retained_image, 0, 0, width, height,
					 2, 1);
                
		/*
                 *  Remap the range of image data to 0 to 255
                 */
		xil_extrema(image, &high, &low);
		mult = (CMAPSIZE - 1) / (high - low);
		offset = -((low * (CMAPSIZE - 1)) / (high - low));
		xil_rescale(image, image, &mult, &offset);

                /*
                 *  Cast the remapped image into each band of a 3 banded byte image
                 */
		xil_cast(image, band0);
		xil_cast(image, band1);
		xil_cast(image, band2);
		break;
              case PseudoColor:
		fprintf(stderr,
                        "1 band images not supported for pseudocolor visuals\n");
		exit(1);
              case GrayScale:
		retained_image = xil_create(state, width, height, 1, XIL_BYTE);
                
		/*
                 *  Remap the range of image data to 0 to 255
                 */
		xil_extrema(image, &high, &low);
		mult = (NCOLORS - 1) / (high - low);
		offset = -((low * (NCOLORS - 1)) /
			      (high - low)) + (CMAPSIZE - NCOLORS);
		xil_rescale(image, image, &mult, &offset);

                /*
                 *  Cast the remapped image into a byte image
                 */
		xil_cast(image, retained_image);
		break;
              case StaticGray:
		if(vinfo.depth == 8) {
		    retained_image = xil_create(state, width, height, 1, XIL_BYTE);
                    
                    /*
                     *  Remap the range of image data to 0 to 255
                     */
		    xil_extrema(image, &high, &low);
		    mult = (CMAPSIZE - 1) / (high - low);
		    offset = -((low * (CMAPSIZE - 1)) /
				  (high - low));
		    xil_rescale(image, image, &mult, &offset);

                    /*
                     *  Cast the remapped image into a byte image
                     */
		    xil_cast(image, retained_image);
		}
		else if(vinfo.depth == 1) {
		    bit_image = xil_create(state, width, height, 1, XIL_BIT);
		    if (bit_image == NULL) {
                        /*
                         *  XIL's default error handler will print an error msg on stderr
                         */
			exit(1);
		    }

		    byte_image = xil_create(state, width, height, 1, XIL_BYTE);
		    if (byte_image == NULL) {
                        /*
                         *  XIL's default error handler will print an error msg on stderr
                         */
			exit(1);
		    }
                    
		    /*
                     *  Remap the range of image data to fill XIL_SHORT range
                     */
		    xil_extrema(image, &high, &low);
		    mult = (CMAPSIZE - 1) / (high - low);
		    offset = -((low * (CMAPSIZE - 1)) /
				  (high - low));
		    xil_rescale(image, image, &mult, &offset);

                    /*
                     *  Cast the remapped image into a byte image
                     */
		    xil_cast(image, byte_image);
                    
                    distribution = xil_kernel_get_by_name(state, "floyd-steinberg");
		    colorcube = xil_colorcube_create(state, XIL_BIT, XIL_BYTE,
                                                    1, 0, &multipliers, &dimensions);
		    xil_error_diffusion(byte_image, bit_image, colorcube, distribution);

		    retained_image = bit_image;
		}
		break;
	    }
	}
	else if(nbands == 3) {
	    switch (vinfo.class) {
              case TrueColor:
		fprintf(stderr,
                        "Multibanded short images not supported by this program\n");
		exit(1);
              case PseudoColor:
		fprintf(stderr,
                        "Multibanded short images not supported by this program\n");
		exit(1);
              case GrayScale:	
		/* This case is handled by PseudoColor visuals */
                exit(0);
              case StaticGray:
		if (vinfo.depth == 8) {
		    /* not implemented */
		    exit(0);
		}
		else if (vinfo.depth == 1) {
		    fprintf(stderr,
			    "Multibanded short images not supported by this program\n");
		    exit(1);
		}
		break;
	    }
	}
    }


    /*
     *  Put the image onto the display.
     */
    xil_copy(retained_image, display_image);


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
    xil_destroy(byte_image);
    xil_destroy(band0);
    xil_destroy(band1);
    xil_destroy(band2);

    if(retained_image != image) {
        xil_destroy(retained_image);
    }

    xil_lookup_destroy(colorcube);
    xil_lookup_destroy(lookup);

    /*
     *  We do not destroy the XilKernel because we didn't create it, we
     *  aquired it using the xil_kernel_get_by_name() call.
     */

    xil_close(state);

    XCloseDisplay(xdisplay);
}


void
set_colormap(display_image, xdisplay, xwindow, cmap)
    XilImage    display_image;
    Display    *xdisplay;
    Window      xwindow;
    XilLookup   cmap;
{
    XVisualInfo          vinfo;
    XWindowAttributes    winattr;
    unsigned long        pixels[256];
    unsigned long        pmask[1];
    unsigned long        pixel[1];
    int                  step;
    int                  i, j;
    int                  ncolors;
    int                  offset;
    Xil_unsigned8        cmap_data[CMAPSIZE * 4];
    XColor               colors[CMAPSIZE];
    XilColorList         clist;

    step = xil_lookup_get_output_nbands(cmap);
    ncolors = xil_lookup_get_num_entries(cmap);
    offset = xil_lookup_get_offset(cmap);

    xil_lookup_get_values(cmap, offset, ncolors, cmap_data);

    XGetWindowAttributes(xdisplay, xwindow, &winattr);
    if (XAllocColorCells(xdisplay, winattr.colormap, 1, pmask, 0, pixels, offset + ncolors)
	== 0) {
	fprintf(stderr, "XAllocColorCells failed\n");
	return;
    }

    for (pixel[0] = 0; pixel[0] < offset; pixel[0]++) {
	XFreeColors(xdisplay, winattr.colormap, pixel, 1, 0);
    }

    /*
     * fill the colormap
     */
    for (i = 0, j = step - 3; i < ncolors; i++, j += step) {
	colors[i].pixel = offset + i;
	colors[i].flags = DoRed | DoGreen | DoBlue;
	colors[i].blue =  cmap_data[j] << 8;
	colors[i].green = cmap_data[j + 1] << 8;
	colors[i].red =   cmap_data[j + 2] << 8;
    }

    clist.ncolors = ncolors;
    clist.colors  = colors;
    clist.cmap    = winattr.colormap;
    /*
     *  Set the COLORMAP through the XIL display image
     */
    xil_set_device_attribute(display_image, "XCOLORMAP", &clist);
}

