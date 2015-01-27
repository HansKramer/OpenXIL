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
*    File:	xilcis_color.c
*    Project:	XIL Movie Player Example
*    Revision:	1.4
*    Last Mod:	10:24:15, 03/10/00
*  
*
*   Description:
*
*	This file contains routines specific to CELL or JPEG colormap
*	creation and installation.
*
*------------------------------------------------------------------------
*
*    COPYRIGHT
*
*----------------------------------------------------------------------*/
#ifndef lint
static	char     sccsid[] = "@(#)xilcis_color.c	1.4\t00/03/10  ";
#endif

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <xil/xil.h>

enum {JPEG, CELL};

XilLookup create_cmap(XilSystemState State, XilCis cis,
                      Display *display, Window window, int screen,
                      Colormap *cmap, int type, XilIndexList *ilist, 
                      XilLookup yuvtorgb, XilLookup colorcube)
{
    unsigned long tmp_colors[256], pixels[256], mask;
    XColor cdefs[256];
    /*
     *  colormap indices 255 and 254
     */    
    int top_colors = 2;	
    int i, j, t;
    Xil_unsigned8 *data;
    XilLookup lut;
    int cmapsize;
    
    /*
     *  Get colormap size
     */
    switch(type)
    {
      case JPEG:
        cmapsize = xil_lookup_get_num_entries(yuvtorgb);
        break;
        
      case CELL:
        xil_cis_get_attribute(cis,"DECOMPRESSOR_MAX_CMAP_SIZE", (void**)&cmapsize);
        break;
    } 
    
    /*
     *  Create an X colormap for the cis.
     */
    *cmap = XCreateColormap(display, window, DefaultVisual(display, screen), 
                            AllocNone);
    
    /*
     *  Allocate X Colormap cells
     *
     *  If we do not need the entire colormap, allocate `cmapsize'
     *  entries just below the top of the colormap.  Here's how:
     *    Temporarily allocate some entries at the front of the cmap.  
     *    Don't allocate the top_colors (the top two color indices 
     *	  are often used by other applications). 
     *    Allocate the needed entries in the next cmap section
     *    Free the temporary entries.
     *  This allows the X-Window manager to use them and reduces the
     *  chances of of your window colormap flashing.
     */
    
    if(cmapsize < 256-top_colors) {
        if(!XAllocColorCells(display, *cmap, 0, &mask, 0,
                              tmp_colors, 256 - cmapsize - top_colors)) {
            fprintf(stderr, " XAllocColorCells for cmap_create failed(1)\n");
            exit(1);
        }
    }
    
    if(!XAllocColorCells(display, *cmap, 0, &mask, 0, pixels, cmapsize)) {
        fprintf(stderr, " XAllocColorCells for cmap_create failed(2)\n");
        exit(1);
    }

    /*
     *  The remaining code assumes that the values returned in pixels[0] through
     *  pixels[cmapsize-1] are a contiguous range.
     */
    
    /*
     *  Free the unused colors in the front
     */
    if(cmapsize < 256-top_colors)
        XFreeColors(display, *cmap, tmp_colors, 256 - cmapsize - top_colors, 0);
    
    if(type == CELL) {
        
        /*
         *  Initialize the XilIndexList to use when setting the RDWR_INDICES
         *  attribute.  In this example, we make all of the indices writable.
         */
        if( (ilist->pixels = (Xil_unsigned32 *)
              malloc(sizeof(Xil_unsigned32) * cmapsize) ) == NULL ) {
            fprintf(stderr, " out of memory for ilist->pixels create\n");
            exit(1);
        }
        ilist->ncolors = cmapsize;
        
        /*
         *  Copy the color cells returned by XAllocColorCells into the ilist
         */
        for(i = 0; i < cmapsize; i++)
            ilist->pixels[i] = (Xil_unsigned32) pixels[i];
    }
    
    /*
     *  Allocate memory to hold colormap data.
     */
    if( (data = (Xil_unsigned8 *)
         malloc(sizeof(Xil_unsigned8) * cmapsize * 3) ) == NULL ) {
        fprintf(stderr, "xilcis_color: out of memory for cmap data create\n");
        exit(1);
    }  
    
    /*
     *  Get the entries for the colormap. The method depends on the compression
     *  type.  For CELL, get the entries in the current default colormap.
     *  For JPEG, get the entries from the standard lookup table
     *  yuv_to_rgb.
     */
    switch(type) {
      case CELL:
        
        /*
         *  Get the current values in the colormap
         */
        for(i = 0; i < cmapsize; i++)
            cdefs[i].pixel = i + pixels[0];
        
        XQueryColors(display, DefaultColormap(display, screen), cdefs,
                     cmapsize);
        
        /*
         *  Convert the values read from the colormap to an array that can
         *  be read by xil_lookup_create.  Note that the colormap values are
         *  are stored in the XilLookup in BGR order.
         */
        for(i = 0, j = 0; i < cmapsize; i++, j += 3) {
            data[j] = cdefs[i].blue >> 8;
            data[j + 1] = cdefs[i].green >> 8;
            data[j + 2] = cdefs[i].red >> 8;
        }
        
        lut = xil_lookup_create(State, XIL_BYTE, XIL_BYTE, 3, cmapsize,
				(int)pixels[0], data);
        break;
        
      case JPEG:
        xil_lookup_get_values(yuvtorgb, xil_lookup_get_offset(yuvtorgb),
                              cmapsize, data);
        xil_lookup_set_offset(colorcube, (unsigned int)pixels[0]);
        for(i = 0, t = 0; i < cmapsize; i++, t += 3) {
            cdefs[i].pixel = pixels[i];
            cdefs[i].flags = DoRed | DoGreen | DoBlue;
            cdefs[i].blue = data[t] << 8;  
            cdefs[i].green = data[t+1] << 8; 
            cdefs[i].red = data[t+2] << 8;  
        }
        XStoreColors(display, *cmap, cdefs, cmapsize);
        lut = yuvtorgb;
        break;
    }
    
    /*
     *  Install colormap before cursor is placed in window
     */
    XInstallColormap(display, *cmap);
    XSync(display, False);
    
    XSetWindowColormap(display, window, *cmap);
    
    free(data);
    
    return(lut);
}


void
cell_install_cmap(Colormap x_cmap, XilImage displayimage,
                  XilLookup cmap, XilIndexList *ilist)
{
    int i, j, t;
    XColor cdefs[256];
    unsigned char cmap_data[3 * 256];
    unsigned short cmapsize;
    int cmapoffset;
    
    XilColorList clist;
    
    cmapsize = xil_lookup_get_num_entries(cmap);
    cmapoffset = xil_lookup_get_offset(cmap);
    xil_lookup_get_values(cmap, cmapoffset, cmapsize, cmap_data);
    
    for(i = 0, j = 0; i < (int) ilist->ncolors; i++, j += 3) {
        t = (ilist->pixels[i] - cmapoffset) * 3;
        
        cdefs[i].pixel = ilist->pixels[i];
        cdefs[i].flags = DoRed | DoGreen | DoBlue;
        cdefs[i].blue = cmap_data[t] << 8;
        cdefs[i].green = cmap_data[t + 1] << 8;
        cdefs[i].red = cmap_data[t + 2] << 8;
    }
    
    clist.ncolors = ilist->ncolors; 
    clist.colors  = cdefs; 
    clist.cmap    = x_cmap; 
    
    /*
     *  Setting XCOLORMAP accomplishes the same thing as: 
     *    XStoreColors(display, x_cmap, cdefs, ilist->ncolors);
     *    XFlush( display );
     *  but goes through DGA (Direct Graphics Access). This is 
     *  much faster.
     */
    xil_set_device_attribute(displayimage, "XCOLORMAP", &clist );
}
