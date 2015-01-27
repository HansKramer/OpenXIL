#include <stdio.h>

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
//  File:   UnsharpIC.cc
//  Project:    XIL
//  Revision:   
//  Last Mod:   
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
//  MT-level:  SAFE
//  
//------------------------------------------------------------------------
//  COPYRIGHT
//------------------------------------------------------------------------
#pragma ident   "@(#)UnsharpIC.cc "


#include "XilDeviceManagerComputeSHORT.hh"
#include "ComputeInfo.hh"

#include <pthread.h>
#include <unistd.h>


typedef unsigned int    uint_t;


struct thread_data_t {
    Xil_signed16* dst_data; 
    uint_t        dst_scanline_stride; 
    uint_t        dst_pixel_stride;
    Xil_signed16* src_data; 
    uint_t        src_scanline_stride; 
    uint_t        src_pixel_stride;
    uint_t        src_xsize; 
    uint_t        src_ysize; 
    int           nbands; 
    int           size;
    float         alpha;
    float         beta;
    float         gamma;
    float*        weight;
    int           method;
};



static XilStatus
SlidingWindows(Xil_signed16* dest_data, uint_t dest_scanline_stride, uint_t dest_pixel_stride,
               Xil_signed16* src1_data, uint_t src1_scanline_stride, uint_t src1_pixel_stride,
               uint_t src1_xsize, uint_t src1_ysize, int nbands, int size, 
               float alpha, float beta, float gamma, float *weight)
{
    int i, j, line;
    float  fsum;
    Xil_signed16 *srcptr;
    Xil_signed16 *dstptr;

    float norm  = (float) alpha/(size*size);
    int   width = size/2;

    //
    //  3x3 kernels are not supported here
    //  uses other 3x3 optimizations
    //
    float *result = new float[src1_xsize*size];
    float *tmpsav = result;

    for (int b = 0; b < nbands; b++) {
        srcptr = src1_data - (width*src1_scanline_stride) - (width*src1_pixel_stride) + b;
        dstptr = dest_data + b;
        result = tmpsav;

            Xil_signed16 *src = srcptr;
            Xil_signed16 *dst = dstptr;
            Xil_signed16 *tmp = srcptr;

            //  first (size x src1_xsize) block (horizontal convolve)
            Xil_signed16 *current      = src1_data;
            Xil_signed16 *current_line = src1_data;

            int kw;
            int kh = size;
            while (kh--) {
                for (j = 0; j < src1_xsize; j++) {
                    fsum = 0.0;
                    kw   = size;

                    while( kw-- ) {
                        fsum += (float)(*src);
                        src  += src1_pixel_stride;
                    }

                    *result = fsum;

                    result += 1;
                    tmp    += src1_pixel_stride;
                    src     = tmp;
                }

                srcptr += src1_scanline_stride;
                src = srcptr;
                tmp = src;
            }

            result = tmpsav;

            //  first block (vertical convolve)
            for (i = 0; i < src1_xsize; i++) {
                fsum = 0.0;
   
                for (j = 0; j < size; j++) 
                    fsum += *(result + j*src1_xsize + i);

                //*dst = _XILI_ROUND_S16(fsum*norm + *current*beta + gamma);
                *dst = _XILI_ROUND_S16(fsum*norm*weight[*current] + *current*beta + gamma);

                dst     += dest_pixel_stride;
                current += src1_pixel_stride;
            }

            //  sliding window
            dstptr += dest_scanline_stride;
            dst = dstptr;
            src = srcptr;
            tmp = srcptr;

            current_line += src1_scanline_stride;
            current       = current_line;

            for (i = 0; i < src1_ysize-1; i++) {
                //  horizontal pass (line emulates a rolling buffer)
                line = i % size;

                for (j = 0; j < src1_xsize; j++) {
                    fsum = 0.0;
                    kw   = size;

                    while( kw-- ) {
                        fsum += (float)(*src);
                        src  += src1_pixel_stride;
                    }

                    *(result + line*src1_xsize + j) = fsum;

                    tmp += src1_pixel_stride;
                    src  = tmp;
                }

                for (j = 0; j < src1_xsize; j++) {
                    fsum = 0.0;

                    for (int m = 0; m < size; m++) {
                        fsum += *(result + m*src1_xsize + j);
                    }

                    //*dst = _XILI_ROUND_S16(fsum*norm + *current*beta + gamma);
                    *dst = _XILI_ROUND_S16(fsum*norm*weight[*current] + *current*beta + gamma);

                    dst += dest_pixel_stride;
                    current += src1_pixel_stride;
                }

                srcptr += src1_scanline_stride;
                dstptr += dest_scanline_stride;

                src = srcptr;
                dst = dstptr;
                tmp = src;

                current_line += src1_scanline_stride;
                current       = current_line;
            }

/*
        }
*/
    }

    result = tmpsav;
    delete[] result;

    return XIL_SUCCESS;
}


static XilStatus
Zamboni(Xil_signed16* dest_data, uint_t dest_scanline_stride, uint_t dest_pixel_stride,
        Xil_signed16* src1_data, uint_t src1_scanline_stride, uint_t src1_pixel_stride,
        uint_t src1_xsize, uint_t src1_ysize, int nbands, int size, 
        float alpha, float beta, float gamma, float *weight)
{
    int i, j, line;
    float fsum;
    Xil_signed16 *srcptr;
    Xil_signed16 *dstptr;

    float norm  = (float) alpha/(size*size);
    int   width = size/2;

    float *result = new float[src1_xsize*size];
    float *tmpsav = result;

    for (int b = 0; b < nbands; b++) {
        srcptr = src1_data - (width*src1_scanline_stride) - (width*src1_pixel_stride) + b;
        dstptr = dest_data + b;
        result = tmpsav;

        Xil_signed16 *src = srcptr;
        Xil_signed16 *dst = dstptr;
        Xil_signed16 *tmp = srcptr;

        //  first (size x src1_xsize) block (horizontal convolve)
        Xil_signed16 *current      = src1_data;
        Xil_signed16 *current_line = src1_data;

        int kw;
        int kh = size;
        while (kh--) {
            for (j = 0; j < src1_xsize; j++) {
                fsum = 0.0;
                kw   = size;

                while( kw-- ) {
                    fsum += (float)(*src);
                    src  += src1_pixel_stride;
                }

                *result = fsum;

                result += 1;
                tmp    += src1_pixel_stride;
                src     = tmp;
            }

            srcptr += src1_scanline_stride;
            src = srcptr;
            tmp = src;
        }

        result = tmpsav;

        //  first block (vertical convolve)
        for (i = 0; i < src1_xsize; i++) {
            fsum = 0.0;

            for (j = 0; j < size; j++) 
                fsum += *(result + j*src1_xsize + i);

            //*dst = _XILI_ROUND_S16(fsum*norm + *current*beta + gamma);
            *dst = _XILI_ROUND_S16(fsum*norm*weight[*current] + *current*beta + gamma);

            dst     += dest_pixel_stride;
            current += src1_pixel_stride;
        }

        //  sliding window
        dstptr += dest_scanline_stride;
        dst = dstptr;
        src = srcptr;
        tmp = srcptr;

        current_line += src1_scanline_stride;
        current       = current_line;

        for (i = 0; i < src1_ysize-1; i++) {
            //  horizontal pass (line emulates a rolling buffer)
            line = i % size;

            fsum = 0.0;
            kw   = size;
            while( kw-- ) {
                    fsum += (float)(*src);
                    src  += src1_pixel_stride;
            }

            *(result + line*src1_xsize) = fsum;

            Xil_signed16 *tail = tmp;
            Xil_signed16 *head = tmp + size*src1_pixel_stride;

            for (j = 1; j < src1_xsize; j++) {
                fsum += *head - *tail;

                *(result + line*src1_xsize + j) = fsum;

                tail += src1_pixel_stride;
                head += src1_pixel_stride;
            }

            for (j = 0; j < src1_xsize; j++) {
                fsum = 0.0;

                for (int m = 0; m < size; m++) {
                    fsum += *(result + m*src1_xsize + j);
                }

                //*dst = _XILI_ROUND_S16(fsum*norm + *current*beta + gamma);
                *dst = _XILI_ROUND_S16(fsum*norm*weight[*current] + *current*beta + gamma);

                dst += dest_pixel_stride;
                current += src1_pixel_stride;
            }

            srcptr += src1_scanline_stride;
            dstptr += dest_scanline_stride;

            src = srcptr;
            dst = dstptr;
            tmp = src;

            current_line += src1_scanline_stride;
            current       = current_line;
        }
    }

    result = tmpsav;
    delete[] result;

    return XIL_SUCCESS;
}


static XilStatus
Zamboni_integer(Xil_signed16* dest_data, uint_t dest_scanline_stride, uint_t dest_pixel_stride,
        Xil_signed16* src1_data, uint_t src1_scanline_stride, uint_t src1_pixel_stride,
        uint_t src1_xsize, uint_t src1_ysize, int nbands, int size, 
        float alpha, float beta, float gamma, float *weight)
{
    int i, j, line;
    Xil_signed32 fsum;
    Xil_signed16 *srcptr;
    Xil_signed16 *dstptr;

    float norm  = (float) alpha/(size*size);
    int   width = size/2;

    Xil_signed32 *result = new Xil_signed32[src1_xsize*size];
    Xil_signed32 *tmpsav = result;

    for (int b = 0; b < nbands; b++) {
        srcptr = src1_data - (width*src1_scanline_stride) - (width*src1_pixel_stride) + b;
        dstptr = dest_data + b;
        result = tmpsav;

        Xil_signed16 *src = srcptr;
        Xil_signed16 *dst = dstptr;
        Xil_signed16 *tmp = srcptr;

        //  first (size x src1_xsize) block (horizontal convolve)
        Xil_signed16 *current      = src1_data;
        Xil_signed16 *current_line = src1_data;

        int kw;
        int kh = size;
        while (kh--) {
            for (j = 0; j < src1_xsize; j++) {
                fsum = 0.0;
                kw   = size;

                while( kw-- ) {
                    fsum += (Xil_signed32)(*src);
                    src  += src1_pixel_stride;
                }

                *result = fsum;

                result += 1;
                tmp    += src1_pixel_stride;
                src     = tmp;
            }

            srcptr += src1_scanline_stride;
            src = srcptr;
            tmp = src;
        }

        result = tmpsav;

        //  first block (vertical convolve)
        for (i = 0; i < src1_xsize; i++) {
            fsum = 0.0;

            for (j = 0; j < size; j++) 
                fsum += *(result + j*src1_xsize + i);

            //*dst = _XILI_ROUND_S16(fsum*norm + *current*beta + gamma);
            *dst = _XILI_ROUND_S16(fsum*norm*weight[*current] + *current*beta + gamma);

            dst     += dest_pixel_stride;
            current += src1_pixel_stride;
        }

        //  sliding window
        dstptr += dest_scanline_stride;
        dst = dstptr;
        src = srcptr;
        tmp = srcptr;

        current_line += src1_scanline_stride;
        current       = current_line;

        for (i = 0; i < src1_ysize-1; i++) {
            //  horizontal pass (line emulates a rolling buffer)
            line = i % size;

            fsum = 0.0;
            kw   = size;
            while( kw-- ) {
                    fsum += (Xil_signed32)(*src);
                    src  += src1_pixel_stride;
            }

            *(result + line*src1_xsize) = fsum;

            Xil_signed16 *tail = tmp;
            Xil_signed16 *head = tmp + size*src1_pixel_stride;

            for (j = 1; j < src1_xsize; j++) {
                fsum += *head - *tail;

                *(result + line*src1_xsize + j) = fsum;

                tail += src1_pixel_stride;
                head += src1_pixel_stride;
            }

            for (j = 0; j < src1_xsize; j++) {
                fsum = 0.0;

                for (int m = 0; m < size; m++) {
                    fsum += *(result + m*src1_xsize + j);
                }

                // *dst = _XILI_ROUND_S16(fsum*norm + *current*beta + gamma);
                *dst = _XILI_ROUND_S16(fsum*norm*weight[*current] + *current*beta + gamma);

                dst += dest_pixel_stride;
                current += src1_pixel_stride;
            }

            srcptr += src1_scanline_stride;
            dstptr += dest_scanline_stride;

            src = srcptr;
            dst = dstptr;
            tmp = src;

            current_line += src1_scanline_stride;
            current       = current_line;
        }
    }

    result = tmpsav;
    delete[] result;

    return XIL_SUCCESS;
}


static void worker_thread(thread_data_t *data)
{
    if (data->method == 0)
        SlidingWindows(data->dst_data, data->dst_scanline_stride, data->dst_pixel_stride,
                       data->src_data, data->src_scanline_stride, data->src_pixel_stride,
                       data->src_xsize, data->src_ysize, data->nbands, data->size, 
                       data->alpha, data->beta, data->gamma, data->weight);
    else if (data->method & XIL_UNSHARP_INTEGER)
        Zamboni_integer(data->dst_data, data->dst_scanline_stride, data->dst_pixel_stride,
                        data->src_data, data->src_scanline_stride, data->src_pixel_stride,
                        data->src_xsize, data->src_ysize, data->nbands, data->size, 
                        data->alpha, data->beta, data->gamma, data->weight);
    else
        Zamboni(data->dst_data, data->dst_scanline_stride, data->dst_pixel_stride,
                data->src_data, data->src_scanline_stride, data->src_pixel_stride,
                data->src_xsize, data->src_ysize, data->nbands, data->size, 
                data->alpha, data->beta, data->gamma, data->weight);
}


static inline void *wordfill(Xil_signed16 *buf, size_t bufsize, Xil_signed16 *pat) 
{
    for (;bufsize>0; bufsize--, buf++) 
        memcpy(buf, pat, 2);

    return buf;
}


static void init_weight(float *w, int type, int window, int level)
{
    w += 32768;
    if (type == XIL_UNSHARP_REGULAR) {
        int i;
        int c = level - window/2;
        int d = level + window/2;

        for (i=-32768; i<32768 && i < c; i++)
           w[i] = 0.0; 

        for (; i<32768 && i < d; i++)
           w[i] = ((float) (i - c))/window;

        for (; i<32768; i++)
           w[i] = 1.0;
    } else if (type == XIL_UNSHARP_INVERTED) {
        int i;
        int c = level - window/2;
        int d = level + window/2;

        for (i=-32768; i<32768 && i < c; i++)
           w[i] = 1.0; 

        for (; i<32768 && i < d; i++)
           w[i] = 1.0 - ((float) (i - c))/window;

        for (; i<32768; i++)
           w[i] = 0.0;
    } else if (type == XIL_UNSHARP_TRIANGLE) {
        int i;
        int c = level - window/2;
        int d = level + window/2;

        for (i=-32768; i<32768 && i < c; i++)
           w[i] = 0.0; 

        for (; i<32768 && i < level; i++)
           w[i] = ((float) 2*(i - c))/window;

        for (; i<32768 && i < d; i++)
           w[i] = 2* (1.0 - ((float) (i - c))/window);

        for (; i<32768; i++)
           w[i] = 0.0;
    }

//int i;
//for (i=-32768; i<32768; i+=100)
//     printf("%d %f\n", i, w[i]);
//puts("----");    
}


XilStatus
XilDeviceManagerComputeSHORT::UnsharpIC(XilOp*       op,
                                        unsigned     op_count,
                                        XilRoi*      roi,
                                        XilBoxList*  bl)
{
//puts("XilDeviceManagerComputeSHORT::UnsharpIC");
    if (op->splitOnTileBoundaries(bl) == XIL_FAILURE) 
        return XIL_FAILURE;

    XilImage* src_image = op->getSrcImage(1);
    XilImage* dst_image = op->getDstImage(1);

    unsigned int size;
    op->getParam(1, &size);

    float alpha, beta, gamma;
    op->getParam(2, &alpha);
    op->getParam(3, &beta);
    op->getParam(4, &gamma);

    int type;
    int window;
    int level; 
    op->getParam(5, &type);
    op->getParam(6, &window);
    op->getParam(7, &level);
    
//printf("XilDeviceManagerComputeSHORT::UnsharpIC : %d %d %d\n", type, window, level);

    int mode;
    op->getParam(8, &mode);

    float norm  = (float) alpha/(size*size);
    int   width = size/2;

    float weight[65536];
    init_weight(weight, type, window, level);

    unsigned int nbands = dst_image->getNumBands();

    int no_cpus = 1;

    if ((mode & XIL_UNSHARP_MT) == XIL_UNSHARP_MT)
        no_cpus = dst_image->getSystemState()->get_no_cpus();
    if ((mode & XIL_UNSHARP_HYPER) == XIL_UNSHARP_HYPER)
        no_cpus = sysconf(_SC_NPROCESSORS_ONLN);

    int i;
    int j;
    
    //  Loop over each of the boxes of storage we are to process.
    XilBox* src_box;
    XilBox* dst_box;
    while (bl->getNext(&src_box, &dst_box)) {
        //  Get the tag associated with this box which indicates where the box
        //  resides in the image.  Verify that the tag is of a type we know.
        XilBoxAreaType tag = (XilBoxAreaType) ((long) dst_box->getTag());
        switch (tag) {
          case XIL_AREA_TOP_LEFT_CORNER:
          case XIL_AREA_TOP_EDGE:
          case XIL_AREA_TOP_RIGHT_CORNER:
          case XIL_AREA_RIGHT_EDGE:
          case XIL_AREA_CENTER:
          case XIL_AREA_LEFT_EDGE:
          case XIL_AREA_BOTTOM_LEFT_CORNER:
          case XIL_AREA_BOTTOM_EDGE:
          case XIL_AREA_BOTTOM_RIGHT_CORNER:
            break;

          case XIL_AREA_SMALL_SOURCE:
            //  We do not currently support convolve implementation
            //  for sources smaller than the kernel.
            //  Don't mark the box as failed since there's no
            //  other implementation to fall to.
            // FIX THIS
            XIL_ERROR(dst_image->getSystemState(), XIL_ERROR_INTERNAL, "di-447", TRUE);
            continue;

          default:
            if (bl->markAsFailed() == XIL_FAILURE) 
                return XIL_FAILURE;
            else 
                continue;
        }

        XilStorage  src_storage(src_image);
        XilStorage  dst_storage(dst_image);

        if ((src_image->getStorage(&src_storage, op, src_box, "XilMemory", XIL_READ_ONLY)  == XIL_FAILURE) || 
            (dst_image->getStorage(&dst_storage, op, dst_box, "XilMemory", XIL_WRITE_ONLY) == XIL_FAILURE)) {
            //  Mark this box entry as having failed.  If marking the box
            //  returns XIL_FAILURE, then we return XIL_FAILURE.
            if (bl->markAsFailed() == XIL_FAILURE) 
                return XIL_FAILURE;
            else 
                continue;
        }

        //  Get the image coordinates of our source from the box.
        int          src_box_x;
        int          src_box_y;
        unsigned int src_box_xsize;
        unsigned int src_box_ysize;

        src_box->getAsRect(&src_box_x, &src_box_y, &src_box_xsize, &src_box_ysize);

        if (tag != XIL_AREA_CENTER) {
            XilRectList  rl(roi, dst_box);
        
            int          x;
            int          y;
            unsigned int xsize;
            unsigned int ysize;
            while (rl.getNext(&x, &y, &xsize, &ysize)) {
                int band;

                switch(tag) {
                  case XIL_AREA_TOP_LEFT_CORNER:
                    for (band=0; band<nbands; band++) {
                        unsigned int  src_pixel_stride;
                        unsigned int  src_scanline_stride;
                        Xil_signed16* src_data;

                        src_storage.getStorageInfo(band, &src_pixel_stride, &src_scanline_stride, NULL, (void**) &src_data);

                        unsigned int  dst_pixel_stride;
                        unsigned int  dst_scanline_stride;
                        Xil_signed16* dst_data;

                        dst_storage.getStorageInfo(band, &dst_pixel_stride, &dst_scanline_stride, NULL, (void**) &dst_data);
            
                        Xil_signed16* src_scanline = src_data + (y*src_scanline_stride) + (x*src_pixel_stride);

                        Xil_signed16* dst_scanline = dst_data + (y*dst_scanline_stride) + (x*dst_pixel_stride);

                        int ptrX = src_box_x + x;
                        int ptrY = src_box_y + y;
                        int kulX = ptrX - width;
                        int kulY = ptrY - width;

                        if (kulX < 0) 
                            kulX = 0;
                        if (kulY < 0) 
                            kulY = 0;

                        Xil_signed16* up_left = src_scanline + ((kulX - ptrX) * src_pixel_stride) + ((kulY - ptrY) * src_scanline_stride);
            
                        // Do the cases where the kernel extends above or meets the top
                        // of the image.
                        Xil_signed16* save = src_data;
                        for (j = 0; j < ysize; j++) {
                            // point to the first pixel of the scanline 
                            Xil_signed16* src_pixel = src_scanline;
                            Xil_signed16* dst_pixel = dst_scanline;
                
                            for (i = 0; i < xsize; i++) {
                                Xil_signed16* dst = dst_pixel;
                            
                                float fsum  = 0.0;

                                Xil_signed16* corner = up_left;
                                Xil_signed16* sptr;
                                int kh;
                                int kw;
                                for (kh = 0; kh < width - ptrY - j; kh++) {
                                    sptr = corner;
                                    for (kw = 0; kw < width - ptrX - i; kw++) 
                                        fsum += ((float) *sptr);
                    
                                    for (kw = width - ptrX - i; kw < size; kw++) {
                                        fsum += ((float) *sptr);
                                        sptr += src_pixel_stride;
                                    }
                                }

                                sptr = corner;
                                for (kh = width - ptrY - j; kh < size; kh++) {
                                    for (kw = 0; kw < width - ptrX - i; kw++) 
                                        fsum += ((float) *sptr);
                    
                                    for (kw = width - ptrX - i; kw < size; kw++) {
                                        fsum += ((float) *sptr);
                                        sptr += src_pixel_stride;
                                    }
                    
                                    corner += src_scanline_stride;
                                    sptr    = corner;
                                }
                
                                // *dst = _XILI_ROUND_S16(fsum*norm + *src_data*beta + gamma);
                                *dst = _XILI_ROUND_S16(fsum*norm*weight[*src_data] + *src_data*beta + gamma);
                                /* move to the next pixel */
                                src_pixel += src_pixel_stride;
                                src_data  += src_pixel_stride;
                                dst_pixel += dst_pixel_stride;
                            }
                
                            save     += src_scanline_stride;
                            src_data  = save;

                            /* move to the next scanline */
                            src_scanline += src_scanline_stride;
                            dst_scanline += dst_scanline_stride;
                        }
                    }
                    break; 
                  case XIL_AREA_TOP_EDGE:
                    for (band=0; band<nbands; band++) {
                        unsigned int  src_pixel_stride;
                        unsigned int  src_scanline_stride;
                        Xil_signed16* src_data;
                        src_storage.getStorageInfo(band, &src_pixel_stride, &src_scanline_stride, NULL, (void**) &src_data);
        
                        unsigned int  dst_pixel_stride;
                        unsigned int  dst_scanline_stride;
                        Xil_signed16* dst_data;
                        dst_storage.getStorageInfo(band, &dst_pixel_stride, &dst_scanline_stride, NULL, (void**) &dst_data);
            
                        Xil_signed16* src_scanline = src_data + (y*src_scanline_stride) + (x*src_pixel_stride);
        
                        Xil_signed16* dst_scanline = dst_data + (y*dst_scanline_stride) + (x*dst_pixel_stride);

                        int ptrX = src_box_x + x;
                        int ptrY = src_box_y + y;
                        int kulX = ptrX - width;
                        int kulY = ptrY - width;
                        if (kulX < 0) 
                            kulX = 0;
                        if (kulY < 0) 
                            kulY = 0;

                        Xil_signed16* kernel_start = src_scanline + ((kulX - ptrX) * src_pixel_stride) + ((kulY - ptrY) * src_scanline_stride);
                        // Do the cases where the kernel extends above or meets the top
                        // of the image.
                        Xil_signed16* save = src_data;
                        for (j = 0; j < ysize; j++) {
                            // point to the first pixel of the scanline 
                            Xil_signed16* src_pixel = src_scanline;
                            Xil_signed16* dst_pixel = dst_scanline;
                
                            for (i = 0; i < xsize; i++) {
                                Xil_signed16* dst = dst_pixel;
                
                                float fsum = 0.0;
                
                                Xil_signed16* sptr;
                                Xil_signed16* corner = kernel_start + (i * src_pixel_stride);
                                int kh;
                                int kw;
                                for (kh = 0; kh < width - ptrY - j; kh++) {
                                    sptr = corner;
                                    for (kw = 0; kw < size; kw++) {
                                        fsum += ((float) *sptr);
                                        sptr += src_pixel_stride;
                                    }
                                }
    
                                sptr = corner;
                                for (kh = width - ptrY - j; kh < size; kh++) {
                                    for (kw = 0; kw < size; kw++) {
                                        fsum += ((float) *sptr);
                                        sptr += src_pixel_stride;
                                    }
                        
                                    corner += src_scanline_stride;
                                    sptr    = corner;
                                }
                    
                                //*dst = _XILI_ROUND_S16(fsum*norm + *src_data*beta + gamma);
                                *dst = _XILI_ROUND_S16(fsum*norm*weight[*src_data] + *src_data*beta + gamma);
                    
                                /* move to the next pixel */
                                src_data  += src_pixel_stride;
                                src_pixel += src_pixel_stride;
                                dst_pixel += dst_pixel_stride;
                            }
                    
                            /* move to the next scanline */
                            save         += src_scanline_stride;
                            src_data      = save;
                            src_scanline += src_scanline_stride;
                            dst_scanline += dst_scanline_stride;
                        }
                    }
                    break;
                  case XIL_AREA_TOP_RIGHT_CORNER:
                    for (band=0; band<nbands; band++) {
                        unsigned int  src_pixel_stride;
                        unsigned int  src_scanline_stride;
                        Xil_signed16* src_data;
                        src_storage.getStorageInfo(band, &src_pixel_stride, &src_scanline_stride, NULL, (void**) &src_data);
        
                        unsigned int  dst_pixel_stride;
                        unsigned int  dst_scanline_stride;
                        Xil_signed16* dst_data;
                        dst_storage.getStorageInfo(band, &dst_pixel_stride, &dst_scanline_stride, NULL, (void**) &dst_data);
            
                        Xil_signed16* src_scanline = src_data + (y*src_scanline_stride) + (x*src_pixel_stride);

                        Xil_signed16* dst_scanline = dst_data + (y*dst_scanline_stride) + (x*dst_pixel_stride);

                        int ptrX = src_box_x + x;
                        int ptrY = src_box_y + y;
                        int kulX = ptrX + size - width - 1;
                        int kulY = ptrY - width;

                        int image_width = src_image->getWidth();
                        if (kulX > image_width - 1)
                            kulX = image_width - 1;
                        if (kulY < 0)
                            kulY = 0;

                        Xil_signed16* top_right = src_scanline + ((kulX - ptrX) * src_pixel_stride) + ((kulY - ptrY) * src_scanline_stride);

                        //
                        // Do the cases where the kernel extends above or meets the top
                        // of the image.
                        //
                        Xil_signed16* save = src_data;
                        for (j = 0; j < ysize; j++) {
                            // point to the first pixel of the scanline 
                            Xil_signed16* src_pixel = src_scanline;
                            Xil_signed16* dst_pixel = dst_scanline;
                            for (i = 0; i < xsize; i++) {
                                Xil_signed16* dst = dst_pixel;
                
                                float fsum = 0.0;

                                Xil_signed16* corner = top_right;
                                Xil_signed16* sptr;
                                int kh;
                                int kw;
                                for (kh = 0; kh < width - ptrY - j; kh++) {
                                    sptr = corner;
                                    for (kw = width + image_width - ptrX - i; kw < size; kw++) {
                                        fsum += ((float) *sptr);
                                    }
                    
                                    for (kw = width + image_width - ptrX - i - 1; kw >= 0; kw--) {
                                        fsum += ((float) *sptr);
                                        sptr -= src_pixel_stride;
                                    }
                                }
                
                                sptr = corner;
                                for (kh = width - ptrY - j; kh < size; kh++) {
                                    for (kw = width + image_width - ptrX - i; kw < size; kw++) {
                                        fsum += ((float) *sptr);
                                    }
                     
                                    for (kw = width + image_width - ptrX - i - 1; kw >= 0; kw--) {
                                        fsum += ((float) *sptr);
                                        sptr -= src_pixel_stride;
                                    }
                     
                                    corner += src_scanline_stride;
                                    sptr    = corner;
                                }
                
                                //*dst = _XILI_ROUND_S16(fsum*norm + *src_data*beta + gamma);
                                *dst = _XILI_ROUND_S16(fsum*norm*weight[*src_data] + *src_data*beta + gamma);
                
                                /* move to the next pixel */
                                src_data  += src_pixel_stride;
                                src_pixel += src_pixel_stride;
                                dst_pixel += dst_pixel_stride;
                            }
                
                            save    += src_scanline_stride;
                            src_data = save;
                            /* move to the next scanline */
                            src_scanline += src_scanline_stride;
                            dst_scanline += dst_scanline_stride;
                        }
                    }
                    break;
                  case XIL_AREA_LEFT_EDGE:
                    for (band=0; band<nbands; band++) {
                        unsigned int  src_pixel_stride;
                        unsigned int  src_scanline_stride;
                        Xil_signed16* src_data;
                        src_storage.getStorageInfo(band, &src_pixel_stride, &src_scanline_stride, NULL, (void**) &src_data);
            
                        unsigned int  dst_pixel_stride;
                        unsigned int  dst_scanline_stride;
                        Xil_signed16* dst_data;
                        dst_storage.getStorageInfo(band, &dst_pixel_stride, &dst_scanline_stride, NULL, (void**) &dst_data);
            
                        Xil_signed16* src_scanline = src_data + + (x*src_pixel_stride) + (y*src_scanline_stride);

                        Xil_signed16* dst_scanline = dst_data + + (x*dst_pixel_stride) + (y*dst_scanline_stride);

                        int ptrX = src_box_x + x;
                        int ptrY = src_box_y + y;
                        int kulX = ptrX - width;
                        int kulY = ptrY - width;
                        // Clamp kernel top left
                        // Y should be clamping to 0.
                        // X should clamp to 0
                        if (kulX < 0)
                            kulX = 0;
                        if (kulY < 0)
                            kulY = 0;
                        Xil_signed16* kernel_start = src_scanline + ((kulX - ptrX) * src_pixel_stride) + ((kulY - ptrY) * src_scanline_stride);
            
                        Xil_signed16* save = src_data;
                        for (j = 0; j < ysize; j++) {
                            Xil_signed16* dst_pixel = dst_scanline;
                
                            for (i = 0; i < xsize; i++) {
                                Xil_signed16* dst = dst_pixel;
                
                                float fsum = 0.0;

                                Xil_signed16* sptr = kernel_start;
                                Xil_signed16* sptr_save = sptr;
                                int kh;
                                int kw;
                                for (kh = 0; kh < size; kh++) {
                                    for (kw = 0; kw < width - ptrX - i; kw++) 
                                        fsum += ((float) *sptr);
                    
                                    for (kw = width - ptrX - i; kw < size; kw++) {
                                        fsum += ((float) *sptr);
                                        sptr += src_pixel_stride;
                                    }
                    
                                    sptr_save += src_scanline_stride;
                                    sptr       = sptr_save;
                                }

                                //*dst = _XILI_ROUND_S16(fsum*norm + *src_data*beta + gamma);
                                *dst = _XILI_ROUND_S16(fsum*norm*weight[*src_data] + *src_data*beta + gamma);
                    
                                /* move to the next pixel */
                                src_data  += src_pixel_stride;
                                dst_pixel += dst_pixel_stride;
                            }
                
                            save    += src_scanline_stride;
                            src_data = save;
                            /* move to the next scanline */
                            src_scanline += src_scanline_stride;
                            dst_scanline += dst_scanline_stride;
                            kernel_start += src_scanline_stride;
                        }
                    }
                    break;
                  case XIL_AREA_RIGHT_EDGE:
            for (band=0; band<nbands; band++) {
                unsigned int  src_pixel_stride;
                unsigned int  src_scanline_stride;
                Xil_signed16* src_data;

                src_storage.getStorageInfo(band, &src_pixel_stride, &src_scanline_stride, NULL, (void**) &src_data);
        
                unsigned int  dst_pixel_stride;
                unsigned int  dst_scanline_stride;
                Xil_signed16* dst_data;

                dst_storage.getStorageInfo(band, &dst_pixel_stride, &dst_scanline_stride, NULL, (void**) &dst_data);
                
                Xil_signed16* src_scanline = src_data + (y*src_scanline_stride) + (x*src_pixel_stride);
    
                Xil_signed16* dst_scanline = dst_data + (y*dst_scanline_stride) + (x*dst_pixel_stride);

                int ptrX = src_box_x + x;
                int ptrY = src_box_y + y;
                int kulX = ptrX + size - width - 1;
                int kulY = ptrY - width;

                if (kulX > src_image->getWidth() - 1)
                    kulX = src_image->getWidth() - 1;
                if (kulY < 0)
                    kulY = 0;

                Xil_signed16* kernel_start = src_scanline + ((kulX - ptrX) * src_pixel_stride) + ((kulY - ptrY) * src_scanline_stride);

                Xil_signed16* save = src_data;
                for (j = 0; j < ysize; j++) {
                    // point to the first pixel of the scanline 
                    Xil_signed16* dst_pixel = dst_scanline;
                
                    for (i = 0; i < xsize; i++) {
                        Xil_signed16* dst = dst_pixel;
                
                        float fsum = 0.0;

                        Xil_signed16* sptr = kernel_start;
                        Xil_signed16* sptr_save = sptr;
                        int kh;
                        int kw;
                        for (kh = 0; kh < size; kh++) {
                            for (kw = kulX - (ptrX-width) - i + 1; kw < size; kw++)
                                fsum += ((float) *sptr);
                    
                            for (kw = kulX - (ptrX-width) - i; kw >= 0; kw--) {
                                fsum += ((float) *sptr);
                                sptr -= src_pixel_stride;
                            }
                
                            sptr_save += src_scanline_stride;
                            sptr       = sptr_save;
                        }
                        // *dst = _XILI_ROUND_S16(fsum*norm + *src_data*beta + gamma);
                        *dst = _XILI_ROUND_S16(fsum*norm*weight[*src_data] + *src_data*beta + gamma);
                
                        /* move to the next pixel */
                        src_data  += src_pixel_stride;
                        dst_pixel += dst_pixel_stride;
                    }
                
                    save    += src_scanline_stride;
                    src_data = save;
                    /* move to the next scanline */
                    src_scanline += src_scanline_stride;
                    dst_scanline += dst_scanline_stride;
                    kernel_start += src_scanline_stride;
                }
            }
            break;
          case XIL_AREA_BOTTOM_LEFT_CORNER:
            for (band=0; band<nbands; band++) {
            unsigned int  src_pixel_stride;
            unsigned int  src_scanline_stride;
            Xil_signed16* src_data;

            src_storage.getStorageInfo(band, &src_pixel_stride, &src_scanline_stride, NULL, (void**) &src_data);
        
            unsigned int  dst_pixel_stride;
            unsigned int  dst_scanline_stride;
            Xil_signed16* dst_data;

            dst_storage.getStorageInfo(band, &dst_pixel_stride, &dst_scanline_stride, NULL, (void**) &dst_data);
            
            Xil_signed16* src_scanline = src_data + (y*src_scanline_stride) + (x*src_pixel_stride);

            Xil_signed16* dst_scanline = dst_data + (y*dst_scanline_stride) + (x*dst_pixel_stride);

            int ptrX = src_box_x + x;
            int ptrY = src_box_y + y;
            int kulX = ptrX - width;
            int kulY = ptrY + size - width - 1;

            int image_height = src_image->getHeight();
            if(kulX < 0)
                kulX = 0;
            if(kulY > image_height - 1)
                kulY = image_height - 1;

            Xil_signed16* lower_left = src_scanline + ((kulX - ptrX) * src_pixel_stride) + ((kulY - ptrY) * src_scanline_stride);

            //
            // Do the cases where the kernel extends above or meets the top
            // of the image.
            //
                        Xil_signed16* save = src_data; 
            for (j = 0; j < ysize; j++) {
                // point to the first pixel of the scanline 
                Xil_signed16* src_pixel = src_scanline;
                Xil_signed16* dst_pixel = dst_scanline;
                
                for(i = 0; i < xsize; i++) {
                Xil_signed16* dst = dst_pixel;
                
                float fsum = 0.0;

                Xil_signed16* corner = lower_left;
                Xil_signed16* sptr = corner;
                Xil_signed16* sptr_save = corner;
                int kh;
                int kw;
                for (kh = width + image_height - ptrY - j - 1; kh >= 0; kh--) {
                    for (kw = 0; kw < width - ptrX - i; kw++) 
                    fsum += ((float) *sptr);
                    
                    for (kw = width - ptrX - i; kw < size; kw++) {
                        fsum += ((float) *sptr);
                        sptr += src_pixel_stride;
                    }

                    sptr = (sptr_save -= src_scanline_stride);
                }

                for (kh = width + image_height - ptrY - j; kh < size; kh++) {
                    sptr = corner;
                    for (kw = 0; kw < width - ptrX - i; kw++)
                        fsum += ((float) *sptr);
                    
                    for (kw = width - ptrX - i; kw < size; kw++) {
                        fsum += ((float) *sptr);
                        sptr += src_pixel_stride;
                    }
                }
                
                // *dst = _XILI_ROUND_S16(fsum*norm + *src_data*beta + gamma);
                *dst = _XILI_ROUND_S16(fsum*norm*weight[*src_data] + *src_data*beta + gamma);
                
                /* move to the next pixel */
                src_data  += src_pixel_stride;
                src_pixel += src_pixel_stride;
                dst_pixel += dst_pixel_stride;
                }
                
                save    += src_scanline_stride;
                src_data = save;
                /* move to the next scanline */
                src_scanline += src_scanline_stride;
                dst_scanline += dst_scanline_stride;
            }
            }
            break; 
          case XIL_AREA_BOTTOM_EDGE:
            for (band=0; band<nbands; band++) {
                unsigned int  src_pixel_stride;
                unsigned int  src_scanline_stride;
                Xil_signed16* src_data;

                src_storage.getStorageInfo(band, &src_pixel_stride, &src_scanline_stride, NULL, (void**) &src_data);
        
                unsigned int  dst_pixel_stride;
                unsigned int  dst_scanline_stride;
                Xil_signed16* dst_data;

                dst_storage.getStorageInfo(band, &dst_pixel_stride, &dst_scanline_stride, NULL, (void**) &dst_data);
            
                Xil_signed16* src_scanline = src_data + (y*src_scanline_stride) + (x*src_pixel_stride);

                Xil_signed16* dst_scanline = dst_data + (y*dst_scanline_stride) + (x*dst_pixel_stride);

                int ptrX = src_box_x + x;
                int ptrY = src_box_y + y;
                int kulX = ptrX - width;
                int kulY = ptrY + size - width - 1;

                if (kulX < 0)
                    kulX = 0;
                if (kulY > src_image->getHeight() - 1)
                    kulY = src_image->getHeight() - 1;

                Xil_signed16* kernel_start = src_scanline + ((kulX - ptrX) * src_pixel_stride) + ((kulY - ptrY) * src_scanline_stride);

                Xil_signed16* save = src_data;
                for (j = 0; j < ysize; j++) {
                    // point to the first pixel of the scanline 
                    Xil_signed16* src_pixel  = src_scanline;
                    Xil_signed16* dst_pixel  = dst_scanline;
                    Xil_signed16* lower_left = kernel_start;
                
                    for (i = 0; i < xsize; i++) {
                        Xil_signed16* dst = dst_pixel;
                
                        float fsum = 0.0;

                        Xil_signed16* sptr      = lower_left;
                        Xil_signed16* sptr_save = lower_left;
                        int kh;
                        int kw;
                        for (kh = kulY - (ptrY-width) - j; kh >= 0; kh--) {
                            for (kw = 0; kw < size; kw++) {
                                fsum += ((float) *sptr);
                                sptr += src_pixel_stride;
                        }
                        sptr = (sptr_save -= src_scanline_stride);
                    }

                    for (kh = kulY - (ptrY-width) - j + 1; kh < size; kh++) {
                        sptr = lower_left;
                        for (kw = 0; kw < size; kw++) {
                            fsum += ((float) *sptr);
                            sptr += src_pixel_stride;
                        }
                    }

                   // *dst = _XILI_ROUND_S16(fsum*norm + *src_data*beta + gamma);
                    *dst = _XILI_ROUND_S16(fsum*norm*weight[*src_data] + *src_data*beta + gamma);
                    
                    /* move to the next pixel */
                    src_data   += src_pixel_stride;
                    src_pixel  += src_pixel_stride;
                    dst_pixel  += dst_pixel_stride;
                    lower_left += src_pixel_stride;
                }
                    
                save    += src_scanline_stride;
                src_data = save;
                /* move to the next scanline */
                src_scanline += src_scanline_stride;
                dst_scanline += dst_scanline_stride;
            }
                }
                break;
          case XIL_AREA_BOTTOM_RIGHT_CORNER:
                for (band=0; band<nbands; band++) {
                    unsigned int  src_pixel_stride;
                    unsigned int  src_scanline_stride;
                    Xil_signed16* src_data;

                    src_storage.getStorageInfo(band, &src_pixel_stride, &src_scanline_stride, NULL, (void**) &src_data);
            
                    unsigned int  dst_pixel_stride;
                    unsigned int  dst_scanline_stride;
                    Xil_signed16* dst_data;

                    dst_storage.getStorageInfo(band, &dst_pixel_stride, &dst_scanline_stride, NULL, (void**) &dst_data);
                
                    Xil_signed16* src_scanline = src_data + (y*src_scanline_stride) + (x*src_pixel_stride);

                    Xil_signed16* dst_scanline = dst_data + (y*dst_scanline_stride) + (x*dst_pixel_stride);

                    int ptrX = src_box_x + x;
                    int ptrY = src_box_y + y;
                    int kulX = ptrX + size - width - 1;
                    int kulY = ptrY + size - width - 1;

                    int image_width  = src_image->getWidth();
                    int image_height = src_image->getHeight();
                    if(kulX > image_width - 1)
                        kulX = image_width - 1;
                    if(kulY > image_height - 1)
                        kulY = image_height - 1;
                    Xil_signed16* lower_right = src_scanline + ((kulX - ptrX) * src_pixel_stride) + ((kulY - ptrY) * src_scanline_stride);
                    // Do the cases where the kernel extends above or meets the top
                    // of the image.
                    Xil_signed16* save = src_data;
                    for (j = 0; j < ysize; j++) {
                        // point to the first pixel of the scanline 
                        Xil_signed16* src_pixel = src_scanline;
                        Xil_signed16* dst_pixel = dst_scanline;
                    
                        for (i = 0; i < xsize; i++) {
                            Xil_signed16* dst = dst_pixel;
                    
                            float fsum = 0.0;

                            Xil_signed16* corner = lower_right;
                            Xil_signed16* sptr = corner;
                            Xil_signed16* sptr_save = corner;
                            int kh;
                            int kw;
                            for (kh = width + image_height - ptrY - j - 1; kh >= 0; kh--) {
                                for (kw = width + image_width - ptrX - i; kw < size; kw++) 
                                    fsum += ((float) *sptr);
                        
                                for (kw = width + image_width - ptrX - i - 1; kw >= 0; kw--) {
                                    fsum += ((float) *sptr);
                                    sptr -= src_pixel_stride;
                                }
                                sptr = (sptr_save -= src_scanline_stride);
                            }
                    
                            for (kh = width + image_height - ptrY - j; kh < size; kh++) {
                                sptr = corner;
                                for (kw =width + image_width - ptrX - i ; kw < size; kw++)
                                    fsum += ((float) *sptr);
                        
                                for (kw =width + image_width - ptrX - i - 1; kw >= 0; kw--) {
                                    fsum += ((float) *sptr);
                                    sptr -= src_pixel_stride;
                                }
                        
                                }
                        
                                // *dst = _XILI_ROUND_S16(fsum*norm + *src_data*beta + gamma);
                                *dst = _XILI_ROUND_S16(fsum*norm*weight[*src_data] + *src_data*beta + gamma);
                    
                                /* move to the next pixel */
                                src_data  += src_pixel_stride;
                                src_pixel += src_pixel_stride;
                                dst_pixel += dst_pixel_stride;
                            }
                    
                            save    += src_scanline_stride;
                            src_data = save;
                            /* move to the next scanline */
                            src_scanline += src_scanline_stride;
                            dst_scanline += dst_scanline_stride;
                        }
                    }
                    break;
                }
            }
        } else {
            //  The XIL_AREA_CENTER Case.

            //  Test to see if all of our storage is of type XIL_PIXEL_SEQUENTIAL.
            //  If so, implement an loop optimized for pixel-sequential storage.

           // Edge optimization only implemented for nbands == 1 and src_pixel_stride == 1
           if ((mode & XIL_UNSHARP_EDGE_OPT) == XIL_UNSHARP_EDGE_OPT && nbands == 1) {
               thread_data_t data;

               data.size   = size;
               data.nbands = nbands;
               data.alpha  = alpha;
               data.beta   = beta;
               data.gamma  = gamma;
               data.weight = weight;
               data.method = mode & (XIL_UNSHARP_ZAMBONI | XIL_UNSHARP_INTEGER);

               Xil_signed16* src_data;
               uint_t        src_pixel_stride;
               uint_t        src_scanline_stride;
               src_storage.getStorageInfo(&src_pixel_stride, &src_scanline_stride, NULL, NULL, (void**) &src_data);
 
               if (src_pixel_stride == 1) {
                   int w       = src_image->getWidth();
                   int h       = src_image->getHeight();

                   data.src_pixel_stride    = 1;
                   data.src_scanline_stride = w + size - 1; 
                   Xil_signed16 *image      = new Xil_signed16[data.src_scanline_stride*(h+size-1)*2];

                   Xil_signed16 *left    = image;
                   Xil_signed16 *middle  = left   + width;
                   Xil_signed16 *right   = middle + w;

                   for (j=0; j<width; j++) {
                       wordfill(left, width, src_data);
                       memcpy(middle, src_data, sizeof(Xil_signed16)*w); 
                       wordfill(right, width, src_data+w-1);

                       middle += data.src_scanline_stride;
                       left   += data.src_scanline_stride;
                       right  += data.src_scanline_stride;
                   }

                   for (j=0; j<h; j++) {
                       wordfill(left, width, src_data);
                       memcpy(middle, src_data, sizeof(Xil_signed16)*w);
                       wordfill(right, width, src_data+w-1);

                       middle += data.src_scanline_stride; 
                       left   += data.src_scanline_stride;
                       right  += data.src_scanline_stride;
                       src_data += src_scanline_stride;
                   }
           
                   src_data -= src_scanline_stride;
                   for (j=0; j<width; j++) {
                       wordfill(left, width, src_data);
                       memcpy(middle, src_data, sizeof(Xil_signed16)*w);
                       wordfill(right, width, src_data+w-1);

                       middle += data.src_scanline_stride;
                       left   += data.src_scanline_stride;
                       right  += data.src_scanline_stride;
                   }

                   unsigned int  dst_pixel_stride;
                   unsigned int  dst_scanline_stride;
                   Xil_signed16* dst_data;
                   dst_storage.getStorageInfo(&data.dst_pixel_stride, &data.dst_scanline_stride, NULL, NULL, (void**) &dst_data);

                   XilRectList rl(roi, dst_box);

                   if (no_cpus > 1) {  // Multi-threading or hyper-threading
                       pthread_t     *threads = new pthread_t[no_cpus];
                       thread_data_t *tdata   = new thread_data_t[no_cpus];
    
                       int x, y;
                       while (rl.getNext(&x, &y, &data.src_xsize, &data.src_ysize)) {
                           Xil_signed16* dst_data_ptr = dst_data + y*dst_scanline_stride          + x*dst_pixel_stride;
                           Xil_signed16* src_data_ptr = image    + width*data.src_scanline_stride + width*data.src_pixel_stride;

                           int slice_size = data.src_ysize/no_cpus;
                           int slice;
                           for (slice=0; slice<no_cpus-1; slice++) {
                               tdata[slice] = data;
                               tdata[slice].src_ysize = slice_size;
                               tdata[slice].dst_data  = dst_data_ptr;
                               tdata[slice].src_data  = src_data_ptr;
                               pthread_create(&threads[slice], NULL, (void*(*)(void*)) worker_thread, &tdata[slice]);
                               dst_data_ptr += slice_size * data.dst_scanline_stride;
                               src_data_ptr += slice_size * data.src_scanline_stride;
                           }
                           tdata[slice] = data;
                           tdata[slice].src_ysize = data.src_ysize - (no_cpus-1)*slice_size;
                           tdata[slice].dst_data  = dst_data_ptr;
                           tdata[slice].src_data  = src_data_ptr;
                           pthread_create(&threads[slice], NULL, (void*(*)(void*)) worker_thread, &tdata[slice]);

                           for (slice=0; slice<no_cpus; slice++) 
                               pthread_join(threads[slice], NULL);
                       }

                       delete[] tdata;
                       delete[] threads;
                   } else {
                       int x, y;
                       while (rl.getNext(&x, &y, &data.src_xsize, &data.src_ysize)) {
                           data.dst_data = dst_data + (y*dst_scanline_stride) + (x*dst_pixel_stride);
                           data.src_data = image + width*data.src_scanline_stride + width*data.src_pixel_stride;

                           worker_thread(&data);
                       }
                   }

                   delete[] image;

                   return XIL_SUCCESS;
               }    
           }

           if (src_storage.isType(XIL_PIXEL_SEQUENTIAL) && dst_storage.isType(XIL_PIXEL_SEQUENTIAL)) {
                thread_data_t data;

                data.size   = size;
                data.nbands = nbands;
                data.alpha  = alpha;
                data.beta   = beta;
                data.gamma  = gamma;
                data.weight = weight;
                data.method = mode & (XIL_UNSHARP_ZAMBONI | XIL_UNSHARP_INTEGER);

                src_storage.getStorageInfo(&data.src_pixel_stride, &data.src_scanline_stride, NULL, NULL, (void**) &data.src_data);
 
                dst_storage.getStorageInfo(&data.dst_pixel_stride, &data.dst_scanline_stride, NULL, NULL, (void**) &data.dst_data);
 
                XilRectList rl(roi, dst_box);
 
                if (no_cpus > 1) {  // Multi-threading or hyperthreading
                    pthread_t     *threads = new pthread_t[no_cpus];
                    thread_data_t *tdata   = new thread_data_t[no_cpus];
     
                    int x, y;
                    while (rl.getNext(&x, &y, &data.src_xsize, &data.src_ysize)) {
                        Xil_signed16* dst_data_ptr = data.dst_data;
                        Xil_signed16* src_data_ptr = data.src_data;
 
                        int slice_size = data.src_ysize/no_cpus;
                        int slice;
                        for (slice=0; slice<no_cpus-1; slice++) {
                            tdata[slice] = data;
                            tdata[slice].src_ysize = slice_size;
                            tdata[slice].dst_data  = dst_data_ptr;
                            tdata[slice].src_data  = src_data_ptr;
                            pthread_create(&threads[slice], NULL, (void*(*)(void*)) worker_thread, &tdata[slice]);
                            dst_data_ptr += slice_size * data.dst_scanline_stride;
                            src_data_ptr += slice_size * data.src_scanline_stride;
                        }
                        tdata[slice] = data;
                        tdata[slice].src_ysize = data.src_ysize - (no_cpus-1)*slice_size;
                        tdata[slice].dst_data  = dst_data_ptr;
                        tdata[slice].src_data  = src_data_ptr;
                        pthread_create(&threads[slice], NULL, (void*(*)(void*)) worker_thread, &tdata[slice]);
 
                        for (slice=0; slice<no_cpus; slice++) 
                            pthread_join(threads[slice], NULL);
                    }
 
                    delete[] tdata;
                    delete[] threads;
                } else {
                    int x, y;
                    while (rl.getNext(&x, &y, &data.src_xsize, &data.src_ysize)) 
                        worker_thread(&data);
                }
            } else {
                // General Storage Implementation.
                XilRectList  rl(roi, dst_box);
          
                int          x;
                int          y;
                unsigned int xsize;
                unsigned int ysize;
                while(rl.getNext(&x, &y, &xsize, &ysize)) {
                    //  Each Band...
                    for(unsigned int  band=0; band<nbands; band++) {
                        unsigned int  src_pixel_stride;
                        unsigned int  src_scanline_stride;
                        Xil_signed16* src_data;
  
                        src_storage.getStorageInfo(band, &src_pixel_stride, &src_scanline_stride, NULL, (void**) &src_data);
  
                        unsigned int  dst_pixel_stride;
                        unsigned int  dst_scanline_stride;
                        Xil_signed16* dst_data;
                        dst_storage.getStorageInfo(band, &dst_pixel_stride, &dst_scanline_stride, NULL, (void**) &dst_data);
              
                        Xil_signed16* src_scanline = src_data + (y*src_scanline_stride) + (x*src_pixel_stride);
              
                        Xil_signed16* dst_scanline = dst_data + (y*dst_scanline_stride) + (x*dst_pixel_stride);
              
                        unsigned int scanline_count = ysize;
              
                        Xil_signed16* kernel_scanline  = src_scanline - (width * src_pixel_stride) - (width * src_scanline_stride);
                            //
                            //  Each Scanline...
                            //
                        Xil_signed16* save = src_data;
  
                        do {
                            Xil_signed16* src_pixel = src_scanline;
                            Xil_signed16* dst_pixel = dst_scanline;
                            Xil_signed16* kernel_pixel = kernel_scanline;
                            unsigned int pixel_count = xsize;
                  
                            //
                            //  Each Pixel...
                            //
                            do {
                                float fsum = 0.0;
                                Xil_signed16* sptr      = kernel_pixel;
                                Xil_signed16* sptr_save = sptr;
                  
                                for (int kh = 0; kh < size; kh++) {
                                    for (int kw = 0; kw < size; kw++) {
                                        fsum += ((float) *sptr);
                                        sptr += src_pixel_stride;
                                    }
                                    sptr = (sptr_save += src_scanline_stride);
                                }
                  
                                //*dst_pixel = _XILI_ROUND_S16(fsum*norm + *src_data*beta + gamma);
                                *dst_pixel = _XILI_ROUND_S16(fsum*norm*weight[*src_data] + *src_data*beta + gamma);
  
                                src_data   += src_pixel_stride;
                                src_pixel  +=  src_pixel_stride;
                                dst_pixel  +=  dst_pixel_stride;
                                kernel_pixel += src_pixel_stride;
                            } while (--pixel_count);
                            save    += src_scanline_stride;
                            src_data = save;
  
                            src_scanline += src_scanline_stride;
                            dst_scanline += dst_scanline_stride;
                            kernel_scanline += src_scanline_stride;
                        } while(--scanline_count);
                    }
                }
            }
        }
    }

    return XIL_SUCCESS;
}

