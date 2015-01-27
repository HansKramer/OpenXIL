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

/* xil_tiff.c 1.3 10:24:17, 03/10/00 */

#include "xil_tiff.h"
#include <string.h>

/*
 * Type definitions
 */
typedef struct __supply_args {
    TIFF*          tiff_info;
    unsigned short photometric;
    unsigned short planar_configuration;
    unsigned int   nbands;
    XilDataType    datatype;
} tiff_supply_args_t;

/*
 * Forward declarations of local routines
 */
static Xil_boolean
file_is_TIFF(const char* pathname);
static XilImage
TIFF_createImage(XilSystemState systemState, TIFF* tiff_info);
static XilStatus
TIFF_readInImage(XilSystemState systemState, TIFF* tiff_info, XilImage image);
static XilStatus
TIFF_writeOutImage(TIFF* tiff_info, XilImage image);

static int
tiff_data_supply(XilImage     image,
                 XilStorage   storage,
                 unsigned int x,
                 unsigned int y,
                 unsigned int xsize,
                 unsigned int ysize,
                 void        *args);
static void
tiff_data_release(void *data, void *args);

/*
 * Global routine defintions
 */
XilImage
xil_load_tiff(XilSystemState systemState, const char *pathname)
{
    TIFF*        tiff_info;
    XilImage     image;
    Xil_boolean  file_is_open = FALSE;

    if(file_is_TIFF(pathname) != TRUE) {
        return NULL;
    }

    if((tiff_info = TIFFOpen(pathname, "r")) == NULL) {
        return NULL;
    }

    if((image = TIFF_createImage(systemState, tiff_info)) == NULL) {
        TIFFClose(tiff_info);
        return NULL;
    }

    if(TIFF_readInImage(systemState, tiff_info, image) == XIL_FAILURE) {
        xil_destroy(image);
        TIFFClose(tiff_info);
        return NULL;
    }

    if(xil_get_attribute(image, "FILE_IS_OPEN", (void *)&file_is_open)
        != XIL_SUCCESS || file_is_open != TRUE) {
        TIFFClose(tiff_info);
    }

    return image;
}

XilStatus
xil_save_tiff(XilImage image, const char *pathname)
{
    TIFF* tiff_info;

    if((tiff_info = TIFFOpen(pathname, "w")) == NULL) {
        return FALSE;
    }

    if(TIFF_writeOutImage(tiff_info, image) == XIL_FAILURE) {
        TIFFClose(tiff_info);
        return XIL_FAILURE;
    }
    
    TIFFClose(tiff_info);

    return XIL_SUCCESS;
}

void
xil_close_tiff(XilImage image)
{
    Xil_boolean file_is_open = FALSE;

    if(xil_get_attribute(image, "FILE_IS_OPEN", (void *)&file_is_open)
        == XIL_SUCCESS && file_is_open == TRUE) {
        TIFF* tiff_info;
        xil_get_attribute(image, "TIFF_HANDLE", (void **)&tiff_info);
        TIFFClose(tiff_info);
    }
}

/*
 * Local routine deifnitions
 */
#define BYTES_TO_CHECK 6

static Xil_boolean
file_is_TIFF(const char* pathname)
{
    FILE* file;
    char  bytes[BYTES_TO_CHECK];

    /*
     * Check the suffix
     */
    if(strstr(pathname, ".tif")) {
        return TRUE;
    }

    /*
     *  Didn't match by suffix so open the file and read the first few bytes.
     */
    if ((file = fopen(pathname, "r")) == NULL) {
        return FALSE;
    }

    if (fread(bytes, sizeof(char), BYTES_TO_CHECK, file) != BYTES_TO_CHECK) {
        fclose(file);
        return FALSE;
    }

    if (! strcmp(bytes, "\115\115") ||
        ! strcmp(bytes, "\111\111")) {
        /*
         *  It's a TIFF file.
         */
        fclose(file);
        return TRUE;
    }

    return FALSE;
}

#undef BYTES_TO_CHECK

static XilImage
TIFF_createImage(XilSystemState systemState, TIFF* tiff_info)
{
    XilDataType   datatype;
    short         bitspersample;
    short         samplesperpixel;
    unsigned int  width;
    unsigned int  height;
    XilTilingMode tiling_mode;
    XilImage      image;
    
    if(TIFFGetField(tiff_info,
                    TIFFTAG_BITSPERSAMPLE, &bitspersample) > 0) {
        if(bitspersample == 1) {
            datatype = XIL_BIT;
        } else if(bitspersample <= 8) {
            datatype = XIL_BYTE;
        } else {
            unsigned short sampleformat;
            if(TIFFGetField(tiff_info,
                            TIFFTAG_SAMPLEFORMAT, &sampleformat) > 0) {
                if(bitspersample <= 16 &&
                   sampleformat == SAMPLEFORMAT_INT) {
                    datatype = XIL_SHORT;
                } else if(bitspersample == 32 &&
                          sampleformat == SAMPLEFORMAT_IEEEFP) {
                    datatype = XIL_FLOAT;
                } else {
                    return NULL;
                }
            } else {
                return NULL;
            }
        }
    }
    
    if(TIFFGetField(tiff_info,
                    TIFFTAG_SAMPLESPERPIXEL, &samplesperpixel) == 0) {
        return NULL;
    }
            
    if(TIFFGetField(tiff_info, TIFFTAG_IMAGEWIDTH, &width) == 0) {
        return NULL;
    }
    
    if(TIFFGetField(tiff_info, TIFFTAG_IMAGELENGTH, &height) == 0) {
        return NULL;
    }

    if(TIFFIsTiled(tiff_info) &&
        (tiling_mode = xil_state_get_default_tiling_mode(systemState))
            != XIL_TILING) {
        xil_state_set_default_tiling_mode(systemState, XIL_TILING);
    }

    image = xil_create(systemState,
                       width, height, samplesperpixel, datatype);

    /* TODO: Error checking in this if-block */
    if(TIFFIsTiled(tiff_info)) {
        if(image != (XilImage)NULL && xil_export(image) != XIL_FAILURE) {
            unsigned int txsize, tysize;
            TIFFGetField(tiff_info, TIFFTAG_TILEWIDTH, &txsize);
            TIFFGetField(tiff_info, TIFFTAG_TILELENGTH, &tysize);
            xil_set_tilesize(image, txsize, tysize);
            xil_import(image, FALSE);
        }
        if(tiling_mode != XIL_TILING) {
            xil_state_set_default_tiling_mode(systemState, tiling_mode);
        }
    }

    if(image != (XilImage)NULL) {
        xil_set_attribute(image, "TIFF_HANDLE", (void *)tiff_info);
    }

    return image;
}

static XilStatus
TIFF_readInImage(XilSystemState systemState, TIFF* tiff_info, XilImage image)
{
    XilMemoryStorage storage;
    XilDataType      datatype;
    unsigned         xsize;
    unsigned         ysize;
    unsigned         nbands;

    Xil_unsigned8*   data_buf = NULL;
    unsigned         data_size;

    unsigned short   photometric;
    TIFFGetField(tiff_info, TIFFTAG_PHOTOMETRIC, &photometric);

    switch(photometric) {
        case PHOTOMETRIC_MINISWHITE:
        case PHOTOMETRIC_MINISBLACK:
        case PHOTOMETRIC_PALETTE:
        case PHOTOMETRIC_RGB:
        break;

        default:
        return XIL_FAILURE;
    }

    xil_get_info(image, &xsize, &ysize, &nbands, &datatype);

#ifndef BIT_IO
    if(datatype == XIL_BIT) return XIL_FAILURE; /* TODO: Bit input */
#endif

    if(TIFFIsTiled(tiff_info)) {
        tiff_supply_args_t *targs =
            (tiff_supply_args_t*)malloc(sizeof(tiff_supply_args_t));
        /* TODO: if targs == NULL ... */

        targs->tiff_info = tiff_info;
        TIFFGetField(tiff_info, TIFFTAG_PLANARCONFIG, /* TODO: check return */
                     &targs->planar_configuration);
        targs->photometric = photometric;
        targs->nbands      = nbands;
        targs->datatype    = datatype;
        xil_set_data_supply_routine(image, tiff_data_supply, (void*)targs);

        xil_set_attribute(image, "FILE_IS_OPEN", (void *)TRUE);
        xil_set_attribute(image, "TIFF_HANDLE", (void *)tiff_info);

        return XIL_SUCCESS;
    }

    switch(datatype) {
        case XIL_BIT:
        data_size = xsize*sizeof(Xil_unsigned8);
        data_buf  = (Xil_unsigned8*)malloc(data_size);
        break;
        
        case XIL_BYTE:
        data_size = xsize*nbands*sizeof(Xil_unsigned8);
        data_buf  = (Xil_unsigned8*)malloc(data_size);
        break;
        
        case XIL_SHORT:
        data_size = xsize*nbands*sizeof(Xil_signed16);
        data_buf  = (Xil_unsigned8*)malloc(data_size);
        break;
        
        case XIL_FLOAT:
        data_size = xsize*ysize*nbands*sizeof(Xil_float32);
        data_buf  = (Xil_unsigned8*)malloc(data_size);
        break;
    }
    
    if(!data_buf) return XIL_FAILURE;

    if(xil_export(image) == XIL_FAILURE) return XIL_FAILURE;
    
    if(datatype != XIL_FLOAT) {
        if(xil_get_memory_storage(image, &storage) == FALSE) {
            xil_import(image, FALSE);
            return XIL_FAILURE;
        }
    }

    switch(photometric) {
        case PHOTOMETRIC_MINISWHITE:
        case PHOTOMETRIC_MINISBLACK:
        switch(datatype) {
            case XIL_BIT:
            {
                Xil_unsigned8* src_scanline = data_buf + (storage.bit.offset/8);
                Xil_unsigned8* dst_scanline = storage.bit.data + (storage.bit.offset/8);
                unsigned int   offset = (storage.bit.offset % 8);
                int y;
            
                for(y=0; y<ysize; y++) {
                    memset(data_buf, 0, data_size);
                
                    if(!TIFFReadScanline(tiff_info, data_buf, y, 0)) {
                        break;
                    }
                            
#ifdef BIT_IO
                    xil_bit_memcpy(src_scanline, dst_scanline,
                                   xsize, offset, offset);
#endif

                    dst_scanline += storage.bit.scanline_stride;
                }
            }
            break;

            case XIL_BYTE:
            {
                Xil_unsigned8*  scanline;
                int x, y;

                scanline = storage.byte.data;
            
                for(y=0; y<ysize && TIFFReadScanline(tiff_info,data_buf,y,0) > 0; y++)
                {
                    Xil_unsigned8* indata = data_buf;
                    
                    if(storage.byte.pixel_stride == 1) {
                        memcpy((void*)scanline, (void*)indata, xsize);
                    } else {
                        Xil_unsigned8* pixel = scanline;
                        
                        for(x=0; x<xsize; x++) {
                            *pixel = *indata++;
                            
                            pixel += storage.byte.pixel_stride;
                        }
                    }
                    
                    scanline += storage.byte.scanline_stride;
                }
            }
            break;

            case XIL_SHORT:
            {
                Xil_signed16*  scanline;
                int x, y;

                scanline = storage.shrt.data;
            
                for(y=0; y<ysize && TIFFReadScanline(tiff_info,data_buf,y,0) > 0; y++)
                {
                    Xil_signed16* indata = (Xil_signed16*)data_buf;
                    
                    if(storage.shrt.pixel_stride == 1) {
                        memcpy((void*)scanline, (void*)indata,
                               xsize*sizeof(Xil_signed16));
                    } else {
                        Xil_signed16* pixel = scanline;
                        
                        for(x=0; x<xsize; x++) {
                            *pixel = *indata++;
                            
                            pixel += storage.shrt.pixel_stride;
                        }
                    }
                    
                    scanline += storage.shrt.scanline_stride;
                }
            }
            break;

            case XIL_FLOAT:
            {
                XilStorage storage;
                Xil_float32* scanline;
                unsigned int scanline_stride;
                int y;

                /*
                 *  Create a contiguous copy of the image's data for reading
                 */
                storage = xil_storage_create(systemState, image);
                if(storage == NULL) {
                    return XIL_FAILURE;
                }

                /*
                 *  Read in the entire image (yikes!)
                 */
                scanline = (Xil_float32*)data_buf;
                scanline_stride = xsize*nbands;
                for(y=0; y<ysize &&
                         TIFFReadScanline(tiff_info, scanline, y, 0) > 0; y++) {
                    scanline += scanline_stride;
                }

                /*
                 * Set storage pixel and scanline strides and data pointer
                 */
                xil_storage_set_pixel_stride(storage, 0, nbands);
                xil_storage_set_scanline_stride(storage, 0, scanline_stride);
                xil_storage_set_data(storage, 0, (void *)data_buf);

                /*
                 * Set the image storage
                 */
                xil_export(image);
                xil_set_storage_with_copy(image, storage);
                xil_storage_destroy(storage);
            }
            break;

            default:
            {
                free(data_buf);
                xil_import(image, FALSE);
                return XIL_FAILURE;
            }
        }
        break;

        case PHOTOMETRIC_PALETTE:
        {
            int x, y;
            Xil_unsigned8* scanline = storage.byte.data;
            
            for(y=0; y<ysize && TIFFReadScanline(tiff_info,data_buf,y,0) > 0; y++)
                {
                    Xil_unsigned8* indata = data_buf;
                    
                    if(storage.byte.pixel_stride == 1) {
                        memcpy((void*)scanline, (void*)indata, xsize);
                    } else {
                        Xil_unsigned8* pixel = scanline;
                        
                        for(x=0; x<xsize; x++) {
                            *pixel = *indata++;
                            
                            pixel += storage.byte.pixel_stride;
                        }
                    }
                    
                    scanline += storage.byte.scanline_stride;
                }
        }
        break;
        
        case PHOTOMETRIC_RGB:
        {
            switch(datatype) {
                case XIL_BYTE:
                {
                    Xil_unsigned8* scanline = storage.byte.data;
                    int x, y;
            
                    for(y=0; y<ysize &&
                        TIFFReadScanline(tiff_info,data_buf,y,0) > 0; y++) {
                        Xil_unsigned8* pixel = scanline;
                        Xil_unsigned8* indata = data_buf;
                    
                        for(x=0; x<xsize; x++) {
                            Xil_unsigned8* band = pixel;
                        
                            *band++ = *(indata + 2);
                            *band++ = *(indata + 1);
                            *band++ = *(indata + 0);
                        
                            indata += 3;
                        
                            pixel += storage.byte.pixel_stride;
                        }
                    
                        scanline += storage.byte.scanline_stride;
                    }
                }
                break;

                case XIL_SHORT:
                {
                    Xil_signed16* scanline = storage.shrt.data;
                    int x, y;
            
                    for(y=0; y<ysize &&
                        TIFFReadScanline(tiff_info,data_buf,y,0) > 0; y++) {
                        Xil_signed16* pixel = scanline;
                        Xil_signed16* indata = (Xil_signed16*)data_buf;
                    
                        for(x=0; x<xsize; x++) {
                            Xil_signed16* band = pixel;
                        
                            *band++ = *(indata + 2);
                            *band++ = *(indata + 1);
                            *band++ = *(indata + 0);
                        
                            indata += 3;
                        
                            pixel += storage.shrt.pixel_stride;
                        }
                    
                        scanline += storage.shrt.scanline_stride;
                    }
                }
                break;

                case XIL_FLOAT:
                {
                    XilStorage storage;
                    Xil_float32* scanline;
                    unsigned int scanline_stride;
                    int x, y;

                    /*
                     *  Create a contiguous copy of the image's data for reading
                     */
                    storage = xil_storage_create(systemState, image);
                    if(storage == NULL) {
                        return XIL_FAILURE;
                    }

                    /*
                     *  Read in the entire image (yikes!)
                     */
                    scanline = (Xil_float32*)data_buf;
                    scanline_stride = xsize*nbands;
                    for(y=0; y<ysize &&
                             TIFFReadScanline(tiff_info, scanline, y, 0) > 0;
                        y++) {
                        Xil_float32* pixel = scanline;
                    
                        for(x=0; x<xsize; x++) {
                            Xil_float32 btmp;
                        
                            btmp = *pixel;
                            *pixel = *(pixel + 2);
                            *(pixel+2) = btmp;
                        
                            pixel += nbands;
                        }
                        scanline += scanline_stride;
                    }

                    /*
                     * Set storage pixel and scanline strides and data pointer
                     */
                    xil_storage_set_pixel_stride(storage, 0, nbands);
                    xil_storage_set_scanline_stride(storage, 0, scanline_stride);
                    xil_storage_set_data(storage, 0, (void *)data_buf);

                    /*
                     * Set the image storage
                     */
                    xil_export(image);
                    xil_set_storage_with_copy(image, storage);
                    xil_storage_destroy(storage);
                }
                break;
            }
        }
        break;
        
        default:
        {
            free(data_buf);
            xil_import(image, FALSE);
            return XIL_FAILURE;
        }
    }

    free(data_buf);
    xil_import(image, TRUE);

    if(photometric == PHOTOMETRIC_PALETTE) {
        float pixel_min, pixel_max;
        Xil_unsigned16* rtmp;
        Xil_unsigned16* gtmp;
        Xil_unsigned16* btmp;
        unsigned int num_entries;
        Xil_unsigned8*  t;
        int i;

        /*
         * Read the colormap and create a lookup table equivalent to it.
         * Limit the number of entries in the colormap to the number
         * actually required (as opposed to 256) which is determined by
         * calculating the image pixel value extrema. This can prevent
         * the colormap from running out of entries (as has been
         * seen to occur).
         */
        xil_extrema(image, &pixel_max, &pixel_min);
        num_entries = (unsigned int)(pixel_max - pixel_min);

        if(TIFFGetField(tiff_info, TIFFTAG_COLORMAP,
                        &rtmp, &gtmp, &btmp) != 0) {
            XilLookup *colormap = (XilLookup*)malloc(sizeof(XilLookup*));
            Xil_unsigned8* lut_data =
                (Xil_unsigned8*)malloc(num_entries*3*sizeof(Xil_unsigned8));
                
            if(!lut_data) {
                return XIL_SUCCESS; /* TODO: warning message */
            }
                
            t = lut_data;
            for(i=(int)pixel_min; i<(int)pixel_max; i++) {
                t[0]  = (Xil_unsigned8)(btmp[i]>>8);
                t[1]  = (Xil_unsigned8)(gtmp[i]>>8);
                t[2]  = (Xil_unsigned8)(rtmp[i]>>8);
                t += 3;
            }
                
            *colormap = 
                xil_lookup_create(systemState,
                                  XIL_BYTE, XIL_BYTE,
                                  3, num_entries, (short)pixel_min,
                                  (void*)lut_data);
            xil_set_attribute(image, "COLORMAP", *colormap);
            free(lut_data);
        }
    }

    return XIL_SUCCESS;
}

static XilStatus
TIFF_writeOutImage(TIFF* tiff_info, XilImage image)
{
    XilMemoryStorage storage;

    XilDataType      datatype;
    unsigned         xsize;
    unsigned         ysize;
    unsigned         nbands;

    Xil_unsigned8*   data_buf = NULL;
    unsigned         data_size;

    short photometric;

#ifndef BIT_IO
    if(xil_get_datatype(image) == XIL_BIT) return XIL_FAILURE; /* TODO: Bit output */
#endif

    if(xil_export(image) == XIL_FAILURE) {
	fprintf(stderr, "TIFF FILEIO ERROR:  Couldn't export image\n");
	return XIL_FAILURE;
    }
    
    xil_get_info(image, &xsize, &ysize, &nbands, &datatype);

    if(datatype != XIL_FLOAT) {
        if(xil_get_memory_storage(image, &storage) == FALSE) {
    	    fprintf(stderr,
                "TIFF FILEIO ERROR:  Couldn't get image memory storage\n");
            xil_import(image, FALSE);
            return XIL_FAILURE;
        }
    }

    TIFFSetField(tiff_info, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
    TIFFSetField(tiff_info, TIFFTAG_COMPRESSION, COMPRESSION_NONE);

    switch(datatype) {
        case XIL_BIT:
        if(nbands != 1) {
	    fprintf(stderr, "TIFF FILEIO ERROR:  Only support 1 band BIT images\n");
            return XIL_FAILURE;
        }
        
        TIFFSetField(tiff_info, TIFFTAG_PHOTOMETRIC, 
                     PHOTOMETRIC_MINISBLACK);
        TIFFSetField(tiff_info, TIFFTAG_BITSPERSAMPLE, 1);
        TIFFSetField(tiff_info, TIFFTAG_SAMPLESPERPIXEL, 1);
        TIFFSetField(tiff_info, TIFFTAG_ROWSPERSTRIP, 1);
        TIFFSetField(tiff_info, TIFFTAG_RESOLUTIONUNIT, RESUNIT_NONE);
        data_size = xsize * sizeof(Xil_unsigned8);
        data_buf  = (Xil_unsigned8*)malloc(data_size);
        break;
        
        case XIL_BYTE:
        if(nbands == 1) {
            XilLookup colormap;

            TIFFSetField(tiff_info, TIFFTAG_PHOTOMETRIC, 
                         PHOTOMETRIC_MINISBLACK);
            TIFFSetField(tiff_info, TIFFTAG_BITSPERSAMPLE, 8);
            TIFFSetField(tiff_info, TIFFTAG_SAMPLESPERPIXEL, 1);
            TIFFSetField(tiff_info, TIFFTAG_ROWSPERSTRIP, 1);
            TIFFSetField(tiff_info, TIFFTAG_RESOLUTIONUNIT,
                         RESUNIT_NONE);

            /*
             * If the FIO colormap is set, write the colormap tag.
             */
            if(xil_get_attribute(image, "COLORMAP", (void**)&colormap)
                == XIL_SUCCESS) {
                unsigned int num_entries = xil_lookup_get_num_entries(colormap);
                Xil_unsigned16 rtmp[256];
                Xil_unsigned16 btmp[256];
                Xil_unsigned16 gtmp[256];
                Xil_unsigned8  *data;
                Xil_unsigned8  *t;
                short offset;
                unsigned int i;

                data = (Xil_unsigned8 *)malloc(num_entries*3*
                                               sizeof(Xil_unsigned8));
                if(data == NULL) {
                    fprintf(stderr, "Memory allocation failure\n");
                    return XIL_FAILURE;
                }

                offset = xil_lookup_get_offset(colormap);
                xil_lookup_get_values(colormap, offset, num_entries, data);

                memset((void *)rtmp, (int)0, sizeof(rtmp));
                memset((void *)btmp, (int)0, sizeof(btmp));
                memset((void *)gtmp, (int)0, sizeof(gtmp));
                for(i = offset, t = data; i < num_entries; i++, t += 3) {
                    btmp[i] = t[0] << 8;
                    gtmp[i] = t[1] << 8;
                    rtmp[i] = t[2] << 8;
                }

                TIFFSetField(tiff_info, TIFFTAG_PHOTOMETRIC, 
                             PHOTOMETRIC_PALETTE);

                TIFFSetField(tiff_info, TIFFTAG_COLORMAP, rtmp, gtmp, btmp);

                free(data);
            }
        } else {
            TIFFSetField(tiff_info,
                         TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB); 
            TIFFSetField(tiff_info, TIFFTAG_BITSPERSAMPLE, 8);
            TIFFSetField(tiff_info, TIFFTAG_SAMPLESPERPIXEL, nbands);
            TIFFSetField(tiff_info, TIFFTAG_ROWSPERSTRIP, 1);
            TIFFSetField(tiff_info, TIFFTAG_RESOLUTIONUNIT,
                         RESUNIT_NONE);
        }
        
        data_size = xsize*nbands*sizeof(Xil_unsigned8);
        data_buf = (Xil_unsigned8*)malloc(data_size);
        break;

        case XIL_SHORT:
        case XIL_FLOAT:
        TIFFSetField(tiff_info, TIFFTAG_BITSPERSAMPLE,
                     datatype == XIL_FLOAT ? 32 : 16);
        TIFFSetField(tiff_info, TIFFTAG_SAMPLESPERPIXEL, nbands);
        TIFFSetField(tiff_info, TIFFTAG_ROWSPERSTRIP, 1);
        TIFFSetField(tiff_info, TIFFTAG_RESOLUTIONUNIT,
                     RESUNIT_NONE);
        TIFFSetField(tiff_info, TIFFTAG_PHOTOMETRIC, 
                     nbands == 1 ? PHOTOMETRIC_MINISBLACK : PHOTOMETRIC_RGB);
        TIFFSetField(tiff_info, TIFFTAG_SAMPLEFORMAT,
                     datatype == XIL_FLOAT ?
                     SAMPLEFORMAT_IEEEFP : SAMPLEFORMAT_INT);
        data_size = xsize*nbands*(datatype == XIL_FLOAT ?
                        sizeof(Xil_float32) : sizeof(Xil_signed16));
        data_buf = (Xil_unsigned8*)malloc(data_size);
        break;
    }
    
    TIFFSetField(tiff_info, TIFFTAG_IMAGEWIDTH, xsize);
    TIFFSetField(tiff_info, TIFFTAG_IMAGELENGTH, ysize);
    
    
    switch(datatype) {
        case XIL_BIT:
        {
            Xil_unsigned8* src_scanline = storage.bit.data;
            Xil_unsigned8* dst_scanline = data_buf;
            
            unsigned int   offset = 0;

            int y, x;
            
            for(y=0; y<ysize; y++) {
                memset(data_buf, 0, data_size);
                
#ifdef BIT_IO
                xil_bit_memcpy(src_scanline, dst_scanline,
                               xsize, offset, offset);
#endif
                
                TIFFWriteScanline(tiff_info, data_buf, y, 0);
                            
                src_scanline += storage.bit.scanline_stride;
            }
        }
        break;
                    
        case XIL_BYTE:
        {
            Xil_unsigned8* scanline = storage.byte.data;
            int y, x, b;
            
            for(y=0; y<ysize; y++) {
                Xil_unsigned8* pixel =   scanline;
                Xil_unsigned8* outdata = data_buf;
                            
                memset(data_buf, 0, data_size);
                
                for(x=0; x<xsize; x++) {
                    Xil_unsigned8* band = pixel;

                    if(nbands == 3) { /* swap B and R */
                        *outdata++ = *(band + 2);
                        *outdata++ = *(band + 1);
                        *outdata++ = *(band + 0);
                    } else {
                        for(b=0; b<nbands; b++) {
                            *outdata++ = *band++;
                        }
                    }
                                
                    pixel += storage.byte.pixel_stride;
                }

                if(TIFFWriteScanline(tiff_info,data_buf,y,0) <= 0) {
	            fprintf(stderr, "TIFF FILEIO ERROR:  TIFFWriteScanline failed\n");
                    return XIL_FAILURE;
                }
                            
                scanline += storage.byte.scanline_stride;
            }
        }
        break;

        case XIL_SHORT:
        {
            Xil_signed16* scanline = storage.shrt.data;
            int y, x, b;
            
            for(y=0; y<ysize; y++) {
                Xil_signed16* pixel =   scanline;
                Xil_signed16* outdata = (Xil_signed16*)data_buf;
                            
                memset(data_buf, 0, data_size);
                
                for(x=0; x<xsize; x++) {
                    Xil_signed16* band = pixel;

                    if(nbands == 3) { /* swap B and R */
                        *outdata++ = *(band + 2);
                        *outdata++ = *(band + 1);
                        *outdata++ = *(band + 0);
                    } else {
                        for(b=0; b<nbands; b++) {
                            *outdata++ = *band++;
                        }
                    }
                                
                    pixel += storage.byte.pixel_stride;
                }

                if(TIFFWriteScanline(tiff_info,data_buf,y,0) <= 0) {
	            fprintf(stderr, "TIFF FILEIO ERROR:  TIFFWriteScanline failed\n");
                    return XIL_FAILURE;
                }
                            
                scanline += storage.byte.scanline_stride;
            }
        }
        break;

        case XIL_FLOAT:
        {
            XilStorage storage;
            Xil_float32* scanline;
            unsigned int pixel_stride, scanline_stride;
            int y, x, b;

            /*
             *  Get a contiguous copy of the image's data for writing to disk.
             */
            if((storage = xil_get_storage_with_copy(image)) == NULL) {
                return XIL_FAILURE;
            }
            if(! xil_storage_is_type(storage, XIL_PIXEL_SEQUENTIAL)) {
                xil_storage_destroy(storage);
                return XIL_FAILURE;
            }

            pixel_stride = xil_storage_get_pixel_stride(storage,0);
            scanline_stride = xil_storage_get_scanline_stride(storage,0);
            scanline = (Xil_float32*)xil_storage_get_data(storage,0);

            for(y=0; y<ysize; y++) {
                Xil_float32* pixel =   scanline;
                Xil_float32* outdata = (Xil_float32*)data_buf;
                            
                memset(data_buf, 0, data_size);
                
                for(x=0; x<xsize; x++) {
                    Xil_float32* band = pixel;

                    if(nbands == 3) { /* swap B and R */
                        *outdata++ = *(band + 2);
                        *outdata++ = *(band + 1);
                        *outdata++ = *(band + 0);
                    } else {
                        for(b=0; b<nbands; b++) {
                            *outdata++ = *band++;
                        }
                    }
                                
                    pixel += pixel_stride;
                }

                if(TIFFWriteScanline(tiff_info,data_buf,y,0) <= 0) {
	            fprintf(stderr,
                        "TIFF FILEIO ERROR:  TIFFWriteScanline failed\n");
                    return XIL_FAILURE;
                }
                            
                scanline += scanline_stride;
            }
        }
    }
    
    TIFFFlush(tiff_info);

    free(data_buf);
    xil_import(image, TRUE);

    return XIL_SUCCESS;
}

static int
tiff_data_supply(XilImage     image,
                 XilStorage   storage,
                 unsigned int x,
                 unsigned int y,
                 unsigned int xsize,
                 unsigned int ysize,
                 void        *args)
{
    tiff_supply_args_t *targs = (tiff_supply_args_t *)args;
    unsigned int row_bytes_per_band;
    void *data = NULL;

#ifdef DEBUG
    fprintf(stderr, "Getting %dx%d tile at (%d,%d)\n", xsize, ysize, x, y);
#endif

    /*
     * Set the row bytes per band
     */
    switch(targs->datatype) {
        case XIL_BIT:
            row_bytes_per_band = sizeof(Xil_unsigned8)*
                ((xsize + XIL_BIT_ALIGNMENT - 1)/XIL_BIT_ALIGNMENT);
            break;
        case XIL_BYTE:
            row_bytes_per_band = xsize*sizeof(Xil_unsigned8);
            break;
        case XIL_SHORT:
            row_bytes_per_band = xsize*sizeof(Xil_signed16);
            break;
        case XIL_FLOAT:
            row_bytes_per_band = xsize*sizeof(Xil_float32);
            break;
    }

    /*
     * Allocate memory
     */
    if((data = (void*)malloc(xsize*row_bytes_per_band*targs->nbands)) == NULL) {
        return XIL_FAILURE;
    }

    /*
     * Read the tile data
     */
    TIFFReadTile(targs->tiff_info, data, x, y, 0, 0);

    /*
     * Set the storage information
     */
    if(targs->nbands == 1) { /* !PHOTOMETRIC_RGB */
        xil_storage_set_pixel_stride(storage, 0, targs->nbands);
        xil_storage_set_scanline_stride(storage, 0, row_bytes_per_band);
        xil_storage_set_data(storage, 0, data);
    } else if(targs->planar_configuration == PLANARCONFIG_SEPARATE) {
        int b;
        unsigned int bytes_per_band = xsize*row_bytes_per_band;
        Xil_unsigned8 *dataP = (Xil_unsigned8 *)data;
        /* Swap red and blue channels */
        for(b = 0; b < 3; b++) {
            xil_storage_set_pixel_stride(storage, 2-b, 1);
            xil_storage_set_scanline_stride(storage, 2-b, row_bytes_per_band);
            xil_storage_set_data(storage, 2-b, (void *)dataP);
            dataP += bytes_per_band;
        }
    } else { /* PHOTOMETRIC_RGB && PLANARCONFIG_CONTIG */
        unsigned int scanline_stride = row_bytes_per_band*targs->nbands;

        /* Swap red and blue channels */
        switch(targs->datatype) {
            case XIL_BYTE:
            {
                Xil_unsigned8* scanline = data;
                int i, j;
            
                for(j=0; j<ysize; j++) {
                    Xil_unsigned8* pixel  = scanline;
                    
                    for(i=0; i<xsize; i++) {
                        Xil_unsigned8 btmp;
                        
                        btmp         = *pixel;
                        *pixel       = *(pixel + 2);
                        *(pixel + 2) = btmp;
                        
                        pixel += 3;
                    }
                    
                    scanline += scanline_stride;
                }
            }
            break;

            case XIL_SHORT:
            {
                Xil_signed16* scanline = data;
                int i, j;
            
                for(j=0; j<ysize; j++) {
                    Xil_signed16* pixel  = scanline;
                    
                    for(i=0; i<xsize; i++) {
                        Xil_signed16 btmp;
                        
                        btmp         = *pixel;
                        *pixel       = *(pixel + 2);
                        *(pixel + 2) = btmp;
                        
                        pixel += 3;
                    }
                    
                    scanline += scanline_stride;
                }
            }
            break;

            case XIL_FLOAT:
            {
                Xil_float32* scanline = data;
                int i, j;
            
                for(j=0; j<ysize; j++) {
                    Xil_float32* pixel  = scanline;
                    
                    for(i=0; i<xsize; i++) {
                        Xil_float32 btmp;
                        
                        btmp         = *pixel;
                        *pixel       = *(pixel + 2);
                        *(pixel + 2) = btmp;
                        
                        pixel += 3;
                    }
                    
                    scanline += scanline_stride;
                }
            }
            break;
        }
    }

    /*
     * Set the data release function
     */
    xil_storage_set_data_release(storage,
        (XilDataReleaseFuncPtr)tiff_data_release, (void *)NULL);

    return XIL_SUCCESS;
}

static void
tiff_data_release(void *data, void *args)
{
    free(data);
}
