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
*    File:	fileio.c
*    Project:	XIL Example #1
*    Revision:	1.5
*    Last Mod:	10:24:09, 03/10/00
*  
*    Description:
*        Reads a file in the XIL example file format (.header/.data)
*        and returns the corresponding XIL image.
*        
*        All data types are supported.
*        
*------------------------------------------------------------------------
*
*    COPYRIGHT
*
*----------------------------------------------------------------------*/
#ifndef lint
static	char     sccsid[] = "@(#)fileio.c	1.5\t00/03/10  ";
#endif

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <xil/xil.h>

XilImage 
load_example_image_file(XilSystemState state,
                        char*          pathname)
{
    /*
     *  XIL Image information and dimensions
     */
    XilImage     image;
    unsigned int width;
    unsigned int height;
    unsigned int nbands;
    unsigned int depth;
    XilDataType  datatype;

    /*
     *  XIL Storage object for setting storage
     */
    XilStorage   storage;

    /*
     *  Header loading information.
     */
    FILE* header_file;
    char  datafile_pathname[BUFSIZ];

    /*
     *  Data loading variables
     */
    int         data_fd;
    void*       mmap_ptr;
    struct stat statbuf;
    
    /*
     *  Open the example image header file
     */
    header_file = fopen(pathname, "r");
    if(header_file == NULL) {
        perror("opening header:  ");
        return NULL;
    }

    /*
     *  Read the header information
     */
    fscanf(header_file, "%d%d%d%d", &width, &height, &depth, &nbands);
    fscanf(header_file, "%s", datafile_pathname);

    /*
     *  Convert depth to XIL datatype
     */
    switch(depth) {
      case 1:     datatype = XIL_BIT;   break;
      case 8:     datatype = XIL_BYTE;  break;
      case 16:    datatype = XIL_SHORT; break;
      case 32:    datatype = XIL_FLOAT; break;
      default:
        fprintf(stderr, "unknown data depth %d\n", depth);
        return NULL;
    }

        
    /*
     *  Close the header file
     */
    fclose(header_file);

    /*
     *  Open the specified data file
     */
    data_fd = open(datafile_pathname, O_RDONLY);
    if(data_fd == -1) {
        perror("opening datafile:  ");
        return NULL;
    }

    /*
     *  Create the image to read the data into.
     */
    image = xil_create(state, width, height, nbands, datatype);
    if(image == NULL) {
        /*
         *  XIL's default error handler will print an error msg to stderr
         */
        close(data_fd);
        return NULL;
    }

    /*
     *  Export the image so we have access and control of the image data
     */
    if(xil_export(image) == XIL_FAILURE) {
        /*
         *  XIL's default error handler will print an error msg to stderr
         */
        close(data_fd);
	return NULL;
    }

    /*
     *  Get the size of the data file.
     */
    if(fstat(data_fd, &statbuf) == -1) {
        perror("stat of datafile:  ");
        close(data_fd);
        return NULL;
    }

    /*
     *  Memory map the data file and then instruct XIL to copy the data
     *  into the XIL image.
     */
    mmap_ptr = mmap(NULL, statbuf.st_size, PROT_READ, MAP_SHARED, data_fd, 0);
    if(mmap_ptr == (void*)-1) {
        perror("mmap of datafile:  ");
        close(data_fd);
        return NULL;
    }

    /*
     *  We have a mapping so we no longer need the file descriptor.
     */
    close(data_fd);

    /*
     *  Construct a storage object for setting storage on the image
     */
    storage = xil_storage_create(state, image);
    if(storage == NULL) {
        /*
         *  XIL's default error handler will print an error msg to stderr
         */
        munmap((caddr_t)mmap_ptr, statbuf.st_size);
	return NULL;
    }

    /*
     *  Describe the storage to XIL.
     */
    xil_storage_set_pixel_stride(storage, 0, nbands);
    xil_storage_set_scanline_stride(storage, 0, nbands*width);
    xil_storage_set_data(storage, 0, mmap_ptr);

    /*
     *  Instruct XIL to set the image storage by copying the data.  We use
     *  this convenience routine so if there are multiple tiles in the image,
     *  we don't have to handle writing a routine to convert the single-buffer
     *  of image storage on disk to multiple tiles.
     */
    if(xil_set_storage_with_copy(image, storage) == XIL_FAILURE) {
        /*
         *  XIL's default error handler will print an error msg to stderr
         */
        munmap((caddr_t)mmap_ptr, statbuf.st_size);
	return NULL;
    }

    /*
     *  Cleanup by destroying the storage object and unmapping the file.
     */
    xil_storage_destroy(storage);
    munmap((caddr_t)mmap_ptr, statbuf.st_size);

    /*
     *  Give control of the image back to XIL making it available for the
     *  maximum available XIL processing performance.  Indicate the image was
     *  modified while it was exported.
     */
    xil_import(image, TRUE);

    return image;
}
