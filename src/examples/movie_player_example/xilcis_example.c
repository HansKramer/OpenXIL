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
*    File:	xilcis_example.c
*    Project:	XIL Movie Player Example
*    Revision:	1.8
*    Last Mod:	14:11:49, 03/14/00
*  
*   Description:
*
*	XIL CIS (Compressed Image Sequence) Example.
*
*	This program will display either single frames or movie sequences
*	of JPEG or CELL encoded images. 
*   
*   Usage:
*
* 	% xilcis_example -i file.movie  
*           -i file  input filename (default earth.jpg).
*           -c       display CELL CIS (default JPEG).
*           -cb      display Cellb CIS
*           -s w h   Input width and height, if using cellb
*           -m       display MPEG cis
*           -h       Display H.261 cis
*   
*------------------------------------------------------------------------
*
*    COPYRIGHT
*
*----------------------------------------------------------------------*/
#ifndef lint
static	char     sccsid[] = "@(#)xilcis_example.c	1.8\t00/03/14  ";
#endif

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>
#include <xil/xil.h>
#include "memmap.h"


enum compressor_type { JPEG, CELL, H261, MPEG, CELLB };
enum history_state {OFF,ON};
enum visual_type {PSEUDOCOLOR, TRUECOLOR};

extern XilLookup create_cmap(XilSystemState State, XilCis cis,
                             Display * display, Window window,
                             int screen, Colormap *rcmap,
                             int type, XilIndexList * ilist,
                             XilLookup yuvtorgb,
                             XilLookup colorcube);

void find_quit(Display* display, Atom quitAtom);
Xil_boolean error_handler(XilError error);

extern void
cell_install_cmap(Colormap, XilImage, XilLookup, XilIndexList*);

main(int argc, char **argv)
{
    
    /*
     *  Default values
     */
    char *infile = "../../../data/images/earth.jpg";
    char *cis_type = "Jpeg";
    enum compressor_type docis = JPEG;
    enum history_state ignore_history = OFF;
    enum visual_type default_visual = PSEUDOCOLOR;

    /*
     *  A frame count of -1 indicates that the number of frames in the
     *  movie is unknown
     */
    int frame_count = -1;
    unsigned int cis_xsize, cis_ysize, cis_nbands;
    unsigned int cellb_width = 0;
    unsigned int cellb_height = 0;
    
    /*
     *  The MFILE structure is defined in memmap.h
     */    
    MFILE *memfile; 
    int i, j;
    
    /*
     *  X-Window variables
     */
    Display *display;
    Window window;
    Colormap x_cmap;
    XEvent event;
    int screen_num;
    Atom closedownAtom;
    Visual *visual;
    VisualID visualid;
    XVisualInfo visual_template;
    int matching_visuals;
    
    /*
     *  XIL variables
     */
    XilSystemState State;
    XilImage displayimage = NULL;
    XilCis cis;
    XilDataType cis_datatype;
    XilImageType outputtype;
    
    /*
     *  Process command line arguments
     */
    for(i = 1; i < argc; i++)
    {
        if(!strcmp(argv[i], "-i"))
            infile = argv[++i];
        if(!strcmp(argv[i], "-c"))
        {
            docis = CELL;
            cis_type = "Cell";
        }
        if(!strcmp(argv[i], "-h"))
        {
            docis = H261;
            cis_type = "H261";
        }        
        if(!strcmp(argv[i], "-m"))
        {
            docis = MPEG;
            cis_type = "Mpeg1";
        }
        if(!strcmp(argv[i], "-cb"))
        {
            docis = CELLB;
            cis_type = "CellB";
        }
        if(!strcmp(argv[i], "-s"))
        {
            cellb_width = atoi(argv[++i]);
            cellb_height = atoi(argv[++i]);
        }
    }
    
    display = XOpenDisplay(NULL);
    if(display == NULL)
    {
        fprintf(stderr, "Unable to to connect to X server.\n");
        exit(1);
    }
    
    /*
     *  Cellb needs width and height specified in order to decompress
     *   properly
     */
    
    
    if(docis == CELLB && (!cellb_width || !cellb_height)) {
        fprintf(stderr, "Please specify input width and height for CellB cis. \n");
        exit(1);
    }
    
    if((State = xil_open()) == NULL)
    {
        /*
         *  XIL sends an error message to stderr if xil_open fails
         */
        exit(1);
    }
    
    /*
     *  Install error handler
     */
    if(xil_install_error_handler(State, error_handler) == XIL_FAILURE)
        fprintf(stderr, "Unable to install error handler\n"); 
    
    /*
     *  Memory-map file containing Compressed Image Sequence (CIS) data
     */
    if( (memfile = (MFILE *) malloc(sizeof(MFILE)) ) == NULL )
    {
        fprintf(stderr, "%s: out of memory for memfile create\n", argv[0]);
        exit(1);
    }  
    init_memfile(memfile);
    attach_file(memfile, infile);
    
    /*
     *  Create the CIS
     */
    cis = xil_cis_create(State, cis_type);
    if(!cis)
    {
        /*
         *  XIL sends an error message to stderr if xil_cis_create fails
         */
        exit(1);
    }
    
    /*
     *  Set decompression quality for faster speed for Mpeg1
     */
    if(cis_type == "Mpeg1")
        xil_cis_set_attribute(cis, "DECOMPRESSOR_QUALITY", (void *)90);
    
    /*
     *  Set the input widtht and height for cellb
     */
    if(docis == CELLB) {
        xil_cis_set_attribute(cis, "WIDTH", (void *)cellb_width);
        xil_cis_set_attribute(cis, "HEIGHT", (void *)cellb_height);
    }
    
    /*
     *  Give the CIS a pointer to the data
     */
    xil_cis_put_bits_ptr(cis, memfile->mlen, frame_count,
                         memfile->mstart, NULL);
    
    /*
     *  Get information from the CIS for dimensions
     */
    outputtype = xil_cis_get_output_type(cis);
    xil_imagetype_get_info(outputtype, &cis_xsize, &cis_ysize,
			   &cis_nbands, &cis_datatype);
    if(cis_nbands != 3) {
	fprintf(stderr,
                "Program cannot play JPEG movies made from %d band images\n",
                cis_nbands);
	exit(1);
    }
    
    /*
     *  Create an X-Window to put the movie in
     */
    screen_num = DefaultScreen(display);
    window = XCreateSimpleWindow(display, RootWindow(display, screen_num),
                                 0, 0, cis_xsize, cis_ysize, 0, BlackPixel(display, screen_num),
                                 WhitePixel(display, screen_num));
    if(!window)
    {
        fprintf(stderr, "%s: unable to create window\n", argv[0]);
        exit(1);
    }
    
    visual = DefaultVisual(display, screen_num);
    visualid = XVisualIDFromVisual(visual);
    visual_template.visualid = visualid;
    visual_template.depth = 24;
    visual_template.class = TrueColor;
    XGetVisualInfo(display, VisualIDMask | VisualDepthMask | VisualClassMask,
                   &visual_template, &matching_visuals);
    if(matching_visuals == 1)
        default_visual = TRUECOLOR;
    else {
        visual_template.depth = 8;
        visual_template.class = PseudoColor;
        XGetVisualInfo(display,VisualIDMask | VisualDepthMask | VisualClassMask,
                       &visual_template, &matching_visuals);
        if(matching_visuals != 1) {
            fprintf(stderr, "Default visual for this screen must be an "); 
            fprintf(stderr, "8-bit PseudoColor or 24-bit TrueColor visual\n");
            exit(1);
        }
    }
    
    
    if(closedownAtom = XInternAtom(display, "WM_DELETE_WINDOW", False))
        XSetWMProtocols(display, window, &closedownAtom, 1);
    
    /*
     *  Need to get window exposure and button press events.
     */
    XSelectInput(display, window, ExposureMask | ButtonPressMask);
    
    /*
     *  Make this window visible
     */
    XMapWindow(display, window);
    
    /*
     *  Wait for an expose event to make sure window has been
     *  properly displayed
     */
    while(1)
    {
        XNextEvent(display, &event);
        if(event.xany.type == Expose &&
            event.xexpose.window == window) break;
    }
    
    /*
     *  Create an XIL display image
     */
    displayimage = xil_create_from_window(State, display, window);
    if(!displayimage)
    {
        /*
         *  XIL sends error message to stderr if xil_create_from_window fails
         */
        exit(1);
    }
    
    if(docis == CELL) {

        /*
         *  CELL variables
         */
        XilImage image24 = NULL;
        XilLookup xil_cmap;
        XilVersionNumber lu_version;
        XilIndexList *ilist;
        
        if(default_visual == PSEUDOCOLOR) {
            
            /*
             *  Create colormap index list
             */
 	    if((ilist = (XilIndexList *) malloc(sizeof(XilIndexList)))
                == NULL ) { 
                fprintf(stderr, "Out of memory for ilist create\n");
                exit(1);
            }
            
            /* 
             *  This routine is in the file xilcis_color.c. The Cell
             *  colormap is created.
             */
            xil_cmap = create_cmap(State, cis, display, window,
                                   DefaultScreen(display), &x_cmap,
                                   CELL, ilist, NULL, NULL);
            
            /*
             *  Get the colormap version number so that we don't have to
             *  reload the colormap except when it changes
             */  
            lu_version = xil_lookup_get_version( xil_cmap );
            
            /*
             *  Tell the decompressor which colormap to manage and then
             *  give the decompressor permission to change colormap
             *  indices in ilist. The setting of RDWR_INDICES may cause
             *  the decompressor to update xil_cmap with its current
             *  colormap from the byte-stream.
             */
            xil_cis_set_attribute(cis, "DECOMPRESSOR_COLORMAP", xil_cmap );
            xil_cis_set_attribute(cis, "RDWR_INDICES", ilist);
            
             /*
              *  Create 24-bit image to receive the decompressed Cell
              *  CIS frames. These will be converted into 8-bits for
              *  display by the xil_nearest_color function.
              */
            image24 = xil_create(State, cis_xsize, cis_ysize, 3, XIL_BYTE);
            
            while(xil_cis_has_frame(cis)) {
                xil_decompress(cis, image24);
                
                /*
                 * Look at the LookUp version number to see if it is time
                 * to re-install the colormap.
                 */
                if(lu_version != xil_lookup_get_version( xil_cmap )) {
                    cell_install_cmap(x_cmap, displayimage, xil_cmap, ilist );
                    lu_version = xil_lookup_get_version( xil_cmap );
                }
                xil_nearest_color(image24, displayimage, xil_cmap);
                find_quit(display, closedownAtom);
            }
        }
        if(default_visual == TRUECOLOR)
            while(xil_cis_has_frame(cis)) {
                xil_decompress(cis, displayimage);
                find_quit(display, closedownAtom);
            }
        
        /*
         *  Determine number of frames played
         */
        frame_count = xil_cis_get_read_frame(cis) - 1;

        /*
         *  Wait for user to press button.  Also, refresh the window if
         *  there is an exposure event.
         */
        while(1) {
            XNextEvent(display, &event);
            
            /*
             *  Exit CELL decompression when user presses button in window
             */
            if(event.xany.type == ButtonPress) break;
            
            if(event.xany.type == ClientMessage &&
                ((XClientMessageEvent *)&event)->data.l[0] == closedownAtom)
                break;
            
            if(event.xany.type == Expose) {
                
                /*
                 *  Go to the last frame of the movie.
                 */
                xil_cis_seek(cis, frame_count, 0); 
                if(default_visual == PSEUDOCOLOR) {
                    xil_decompress(cis, image24);
                    xil_nearest_color(image24, displayimage, xil_cmap);
                }
                else
                    xil_decompress(cis, displayimage);
            }
        }
        if( image24 )
            xil_destroy( image24 );
    }
    /*
     *  End of CELL decompression.
     */
    else
    {
        /*
         *  JPEG variables 
         */
        XilLookup colorcube;
        XilDitherMask dmask;
        XilImage imageYCC = NULL;
        XilLookup xil_cmap;
        float scale[3], offset[3];
        XilColorspace ycc, rgb;
        
        /*
         *  imageYCC is the XilImage that the cis will be decompressed
         *  into.
         */
        imageYCC = xil_create(State, cis_xsize, cis_ysize, cis_nbands,
                              cis_datatype);
        if(imageYCC == NULL) {
            /*
             * XIL sends error message to stderr if xil_create fails
             */
            exit(1);
        }
        
        if(default_visual == PSEUDOCOLOR) {
            scale[0] = 255.0 / (235.0 - 16.0);
            scale[1] = 255.0 / (240.0 - 16.0);
            scale[2] = 255.0 / (240.0 - 16.0);
            offset[0] = -16.0 * scale[0];
            offset[1] = -16.0 * scale[1];
            offset[2] = -16.0 * scale[2];
            
            colorcube = xil_lookup_get_by_name(State, "cc855");
            dmask = xil_dithermask_get_by_name(State, "dm443");
            
            /*
             *  This routine is in the file: xilcis_color.c. Inside this
             *  routine the colormap is installed through X, and the color
             *  cube offset is set to the cmap offset.
             */
            xil_cmap = create_cmap(State, cis, display, window,
                                   DefaultScreen(display), &x_cmap, JPEG, NULL, 
                                   xil_lookup_get_by_name(State, "yuv_to_rgb"),
                                   colorcube);
            while(xil_cis_has_frame(cis)) {
                
                /*
                 * Begin XIL Molecule...
                 */
                xil_decompress( cis, imageYCC );
                xil_rescale(imageYCC, imageYCC, scale, offset);
                xil_ordered_dither( imageYCC, displayimage, colorcube, dmask);
	        /*
                 * End of XIL Molecule
                 */
                find_quit(display, closedownAtom);
            }
        }
        
        if(default_visual == TRUECOLOR) {
            ycc = xil_colorspace_get_by_name(State, "ycc601");
            rgb = xil_colorspace_get_by_name(State, "rgb709");
            xil_set_colorspace(imageYCC, ycc);
            xil_set_colorspace(displayimage, rgb); 
            while(xil_cis_has_frame(cis)) {
                
                /*
                 *  Begin XIL molecule
                 */
                xil_decompress(cis, imageYCC);
                xil_color_convert(imageYCC, displayimage);
                /*
                 *  End of XIL molecule
                 */
                find_quit(display, closedownAtom);
            }
        }
        
        /*
         *  Determine number of frames played
         */
        frame_count = xil_cis_get_read_frame(cis) - 1;
        
        /*
         *  Wait for user to press button.  Also, refresh the window if there
         *  is an exposure event.
         */
        while(1) {
            XNextEvent(display, &event);
            
            /*
             *  Exit JPEG decompression when user presses button in window
             */
            if(event.xany.type == ButtonPress) break;
            
            if(event.xany.type == ClientMessage &&
                ((XClientMessageEvent *)&event)->data.l[0] == closedownAtom)
                break;
            
            if(event.xany.type == Expose) {
                
                /*
                 *  Go to the last frame of the movie.
                 *  For CellB and H261, must use the ignore history attribute
                 *  to backup 1 frame.  This frame is "special" because it
                 *  is the last decompressed frame and our history buffer
                 *  is 1 deep. So we are able to re-decompress/display the
                 *  last frame with valid history.
                 */
                if((docis == CELLB) || (docis == H261)){
                    ignore_history = ON;
                    xil_cis_set_attribute(cis,"IGNORE_HISTORY",
                                          (void *)ignore_history);
                    xil_cis_seek(cis, -1, 2);
                    ignore_history = OFF;
                    xil_cis_set_attribute(cis,"IGNORE_HISTORY",
                                          (void *)ignore_history);
                }
                else 
                    xil_cis_seek(cis, -1, 2);
                
                if(default_visual == PSEUDOCOLOR) {
                    /*
                     *  Begin XIL Molecule
                     */
                    xil_decompress(cis, imageYCC);
                    xil_rescale(imageYCC, imageYCC, scale, offset);
                    xil_ordered_dither(imageYCC, displayimage, colorcube,dmask);
                    /*
                     *  End of XIL Molecule
                     */
                }
                else {
                    xil_decompress(cis, imageYCC);
                    xil_color_convert(imageYCC, displayimage);
                }    
            }
        }
        if( imageYCC )
            xil_destroy( imageYCC );
    }
    /*
     *  End of JPEG decompression
     */
    
    
    /*
     *  Clean up
     */
    if( displayimage )
        xil_destroy( displayimage );
    xil_cis_destroy(cis);
    xil_close(State);
    XCloseDisplay(display);
    exit(0);
}

void find_quit(Display* display, Atom quitAtom)
{
    XEvent event;
    
    if(XPending(display) > 0) {
        XNextEvent(display, &event);
        if(event.xany.type == ClientMessage &&
            ((XClientMessageEvent *)&event)->data.l[0] == quitAtom)
            exit(0);
    }
}

Xil_boolean error_handler(XilError error)
{
    xil_call_next_error_handler(error);
    fprintf(stderr, "\n***ERROR received: example exiting\n");
    exit(1);
}
