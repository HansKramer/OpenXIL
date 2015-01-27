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
*    File:	encode.c
*    Project:	XIL Movie Maker Example
*    Revision:	1.4
*    Last Mod:	10:24:13, 03/10/00
*  
*    Description:
*
*        This program creates a JPEG, Cell, or CellB movie from
*        a series of frames.
*
*    Usage:
*
*        % encode [-c | -cb] file-list [output-file]
*            -c         Make a Cell movie (default is JPEG)
*            -cb        Make a CellB movie (default is JPEG)
*            file-list  Name of a file that contains a list of
*                       files (images) to be processed. The list of
*                       files supplied with this example is called
*                       mifkin.list.
*            output-file        Name of file to write movie to.
*                               By default, output is written to
*                               out.jpg.
*        
*------------------------------------------------------------------------
*
*    COPYRIGHT
*
*----------------------------------------------------------------------*/
#ifndef lint
static	char     sccsid[] = "@(#)encode.c	1.4\t00/03/10  ";
#endif

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <xil/xil.h>

extern XilImage load_example_image_file(XilSystemState state,
                                        char*          pathname);

enum comp_type {JPEG, CELL, CELLB};

XilImage image_transform_colorspace(XilSystemState state,
                                    XilImage image,
                                    int nbands,
                                    char* colorspace);

void write_file(XilCis cis,
                int *total_nbytes,
                int *total_nframes,
                FILE *out);

main(int argc, char **argv)
{
    
    int i;
    
    /*
     *  File names and pointers
     */
    char infile[80];
    char outfile[80];
    char cis_type[10];
    char *c;
    FILE *out;
    FILE *file_list = NULL;
    enum comp_type comp = JPEG;
    int cmap_size = 240;
    
    /*
     *  XIL related objects
     */
    XilSystemState state;  
    XilImage src;
    XilImage child_clip;
    XilCis cis;
    
    /*
     *  Variables used to retrieve data
     */
    int total_nbytes = 0;
    int total_nframes = 0;
    
    /*
     *  Initialize input/output file names
     */
    strcpy(infile,"");
    strcpy(outfile,"out.jpg");
    strcpy(cis_type,"Jpeg");
    
    /*
     *  Parse command line options
     */
    for(i = 1; i < argc; i++) {
        if(argv[i][0] == '-') {
            if(!strcmp("-cb",argv[i])) {
                strcpy(cis_type,"CellB");
                strcpy(outfile,"out.cellb");
                comp = CELLB;
            }
            else if(!strcmp("-c",argv[i])) {
                strcpy(cis_type,"Cell");
                strcpy(outfile,"out.cell");
                comp = CELL;
            }
        }
        else {
            if(file_list == NULL) {
                if((file_list = fopen(argv[i], "r")) == NULL) {
                    fprintf(stderr, "Could not open file list %s\n", argv[i]);
                    exit(1);
                }
            } else 
                strcpy(outfile, argv[i]);
        }
    }
    
    /*
     *  Check input file list
     */
    if(file_list == NULL) {
        fprintf(stderr, "No input file supplied\n");
        exit(1);
    }
    
    /*
     *  Open output file
     */
    if((out = fopen(outfile, "w")) == NULL) {
        fprintf(stderr, "Cannot open output file %s\n", outfile);
        exit(1);
    }
    
    /*
     *  Open XIL and create state
     */
    state = xil_open();
    if(state == NULL) {
        fprintf(stderr, "Error initializing XIL\n");
        exit(1);
    }
    
    /*
     *  Create a  compressor
     */
    cis = xil_cis_create(state, cis_type);
    if(comp == JPEG) {
        xil_cis_set_attribute(cis, "ENCODE_411_INTERLEAVED", (void *)TRUE);
    }
    
    if(comp == CELL) {
        xil_cis_set_attribute(cis, "COMPRESSOR_MAX_CMAP_SIZE", (void *)cmap_size);
    }
    
    /*
     *  Tell the CIS not to buffer more than 100 frames
     */
    xil_cis_set_max_frames(cis, 100);
    
    /*
     *  Retrieve the next input name from the file list until none are left
     */
    while(fgets(infile, 80, file_list) != NULL) {
        
        /*
         *  Strip newline off of filename if necessary
         */
        if((c = strchr(infile, '\n')) != NULL)
            *c = '\0';
        
        /*
         *  Load image
         */
        src = load_example_image_file(state, infile);
        if(src == NULL) {
            fprintf(stderr, "Unable to load %s\n", infile);
            exit(1);
        }
        
        /*
         *  Cell video format requires 3 banded, "rgb709" image
         */
        
        if(comp == CELL) {
            src = image_transform_colorspace(state,src,3,"rgb709");
        }
        
        /*
         *  JPEG 411 fast decompress and cellb fast decompress
         *  assumes 3-banded, YCC image
         */
        else {
            src = image_transform_colorspace(state, src, 3, "ycc601");
        }
        if(src == NULL) {
            fprintf(stderr, "Unable to create image in image_transform_colorspace()\n");
            exit(1);
        }
        
        
        /*
         *  Clip image to multiples of 16 for JPEG and
         *  multiples of four for cellb
         */
        
        if(comp == JPEG)
            child_clip = xil_create_child(src, 0, 0,
                                          xil_get_width(src) & ~0xf,
                                          xil_get_height(src) & ~0xf,
                                          0, 3);
        else
            child_clip = xil_create_child(src, 0, 0,
                                          xil_get_width(src) & ~0x3,
                                          xil_get_height(src) & ~0x3,
                                          0, 3);          
        
        xil_compress(child_clip, cis);
        
        /*
         *  We are responsible for destroying all of the XIL objects we created.
         *  Like free(), XIL supports destroying NULL objects.
         */
        xil_destroy(child_clip);
        xil_destroy(src);
        
        /*
         *  Write compressed data to output file
         */
        write_file(cis, &total_nbytes, &total_nframes, out);
        
        printf("Compressed File %s\n", infile);
    }
    
    /*
     *  Tell the CIS to finish any pending compression operations and
     *  and write any resulting compressed data to the output file
     */
    xil_cis_flush(cis);
    write_file(cis, &total_nbytes, &total_nframes, out);
    
    printf("Retrieved %d frames, %d bytes\n", total_nframes, total_nbytes);  
    
    /*
     *  Close output file
     */
    fclose(out);
    
    /*
     *  Destroy CIS and close XIL
     */
    xil_cis_destroy(cis);
    xil_close(state);
}

XilImage
image_transform_colorspace(XilSystemState state,
                           XilImage image,
                           int desired_nbands,
                           char* desired_colorspace)
{
    XilImage new_image;
    unsigned int width, height, nbands;
    XilDataType datatype;
    
    xil_get_info(image, &width, &height, &nbands, &datatype);
    new_image = xil_create (state, width, height, desired_nbands, datatype);
    if(new_image == NULL) {
	/*
         *  XIL sends an error msg to stderr if image create fails
         */
	return(NULL);
    }
    xil_set_colorspace(new_image, xil_colorspace_get_by_name(state, desired_colorspace));
    xil_color_convert(image, new_image);
    xil_destroy(image);
    return(new_image);
}

void write_file(XilCis cis, int *total_nbytes, int *total_nframes,
                FILE *out)
{
    Xil_unsigned8 *data;
    int nbytes, nframes;
    
    while(xil_cis_has_frame(cis) == TRUE) {
        data = (Xil_unsigned8 *)xil_cis_get_bits_ptr(cis,
                                                     &nbytes, &nframes);
        *total_nbytes += nbytes;
        *total_nframes += nframes;
        if(nbytes > 0) {
            if(fwrite((char *)data, sizeof(Xil_unsigned8), nbytes,
                       out) != nbytes) {
                fprintf(stderr, "Write failed\n");
                exit(1);
            }
        }
    }
}
