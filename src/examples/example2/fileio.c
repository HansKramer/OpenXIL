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
*    Project:	XIL Example #2
*    Revision:	1.6
*    Last Mod:	10:24:10, 03/10/00
*  
*    Description:
*        Reads a file in the XIL example file format (.header/.data)
*        and returns the corresponding XIL image.
*
*        This example uses the full tiling interface to load the image
*        into a potentially tiled image.
*
*        All data types are supported.
*        
*------------------------------------------------------------------------
*
*    COPYRIGHT
*
*----------------------------------------------------------------------*/
#ifndef lint
static	char     sccsid[] = "@(#)fileio.c	1.6\t00/03/10  ";
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
     *  Tile information.
     */
    unsigned int tile_xsize;
    unsigned int tile_ysize;

    /*
     *  Counters...
     */
    int          x;
    int          y;

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
     *  Turn on tiling in XIL
     *  The code would also work in the default XIL_WHOLE_IMAGE
     *  tiling mode, but the image would only ever be treated
     *  as one tile regardless of how large it was.
     */
    xil_state_set_default_tiling_mode(state,XIL_TILING);

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
     *  Export the image so we can gain access and control of the image data
     *  as well as determining its tile size.
     */
    if(xil_export(image) == XIL_FAILURE) {
        /*
         *  XIL's default error handler will print an error msg to stderr
         */
        munmap((caddr_t)mmap_ptr, statbuf.st_size);
	return NULL;
    }

    /*
     *  Get the image's tile size.
     */
    xil_get_tilesize(image, &tile_xsize, &tile_ysize);

    /*
     *  Loop through the tiles in the image and copy the storage from disk
     *  into each tile.
     */
    for(y = 0; y < height; y += tile_ysize) {
        for(x = 0; x < width; x += tile_xsize) {
            /*
             *  Loop counters.
             */
            int            i, j, k;

            /*
             *  Calculate the start address that corresponds to the tile area
             *  in our contiguous disk storage.
             */
            unsigned int   src_scanline_stride = width*nbands;
            Xil_unsigned8* src_start = ((Xil_unsigned8*)mmap_ptr) +
                y * src_scanline_stride + x * nbands;
            Xil_unsigned8* src_scanline;
            Xil_unsigned8* src_pixel;
            Xil_unsigned8* src_band;

            /*
             *  Variables describing the destination tile's storage.
             */
            Xil_unsigned8* dst_start;
            Xil_unsigned8* dst_scanline;
            Xil_unsigned8* dst_pixel;
            Xil_unsigned8* dst_band;
            unsigned int   dst_scanline_stride;
            unsigned int   dst_pixel_stride;
            unsigned int   dst_band_stride;

            
            /*
             *  Get the storage for this tile.
             */
            if(xil_get_tile_storage(image, x, y, storage) == FALSE) {
                fprintf(stderr,
                        "ERROR: Failed to aquire storage for tile (%d, %d)\n",
                        x, y);
                munmap((caddr_t)mmap_ptr, statbuf.st_size);
                return NULL;
            }

            /*
             *  Determine its type (XIL_PIXEL_SEQUENTIAL, XIL_BAND_SEQUENTIAL
             *  or XIL_GENERAL) and copy the storage from disk into the tile.
             */
            if(xil_storage_is_type(storage, XIL_PIXEL_SEQUENTIAL)) {
                /*
                 *  Setup to copy into pixel-sequential storage by pointing at
                 *  the first band.  By definition, the band stride is 1.
                 */
                dst_start           = (Xil_unsigned8*)
                    xil_storage_get_data(storage, 0);

                dst_scanline_stride = xil_storage_get_scanline_stride(storage, 0);

                dst_pixel_stride    = xil_storage_get_pixel_stride(storage, 0);

                src_scanline = src_start;
                dst_scanline = dst_start;

                for(i = 0; i < tile_ysize; i++) {
                    src_pixel = src_scanline;
                    dst_pixel = dst_scanline;

                    for(j = 0; j < tile_xsize; j++) {
                        src_band = src_pixel;
                        dst_band = dst_pixel;

                        for(k = 0; k < nbands; k++) {
                            *dst_band++ = *src_band++;
                        }

                        src_pixel += nbands;
                        dst_pixel += dst_pixel_stride;
                    }

                    src_scanline += src_scanline_stride;
                    dst_scanline += dst_scanline_stride;
                }
            } else if(xil_storage_is_type(storage, XIL_BAND_SEQUENTIAL)) {
                /*
                 *  Setup to copy into band-sequential storage by pointing at
                 *  the first band.  By definition, the pixel stride is 1.
                 */
                dst_start           = (Xil_unsigned8*)
                    xil_storage_get_data(storage, 0);

                dst_scanline_stride = xil_storage_get_scanline_stride(storage, 0);

                dst_band_stride     = xil_storage_get_band_stride(storage);

                src_band = src_start;
                dst_band = dst_start;

                for(k = 0; k < nbands; k++) {
                    src_scanline = src_band;
                    dst_scanline = dst_band;

                    for(i = 0; i < tile_ysize; i++) {
                        src_pixel = src_scanline;
                        dst_pixel = dst_scanline;

                        for(j = 0; j < tile_xsize; j++) {
                            *dst_pixel = *src_pixel;

                            src_pixel += nbands;
                            dst_pixel += 1;
                        }

                        src_scanline += src_scanline_stride;
                        dst_scanline += dst_scanline_stride;
                    }

                    src_band += 1;
                    dst_band += dst_band_stride;
                }
            } else {
                /*
                 *  Setup to copy into XIL_GENERAL storage which means the
                 *  data location and layout can be different in each band.
                 */

                src_band = src_start;

                for(k = 0; k < nbands; k++) {
                    /*
                     *  Get the destination information for this band.
                     */
                    dst_scanline        = (Xil_unsigned8*)
                        xil_storage_get_data(storage, k);
                    dst_scanline_stride = xil_storage_get_scanline_stride(storage, k);
                    dst_pixel_stride    = xil_storage_get_pixel_stride(storage, k);
                    
                    for(i = 0; i < tile_ysize; i++) {
                        src_pixel = src_scanline;
                        dst_pixel = dst_scanline;

                        for(j = 0; j < tile_xsize; j++) {
                            *dst_pixel = *src_pixel;

                            src_pixel += nbands;
                            dst_pixel += dst_pixel_stride;
                        }

                        src_scanline += src_scanline_stride;
                        dst_scanline += dst_scanline_stride;
                    }

                    src_band += 1;
                }
            }
        }
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
